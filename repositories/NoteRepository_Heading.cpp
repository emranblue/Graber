#include "NoteRepository.h"
#include "MarkdownUtils.h"

#include <QFile>
#include <QTextStream>
#include <QDateTime>
#include <QDir>
#include <QFileInfo>

bool NoteRepository::deleteHeadingSection(const QString &targetFile, const QString &slug, const QString &subjectName, QString &outCapturedLabelText) {
    QFile file(targetFile);
    if (!file.open(QIODevice::ReadWrite | QIODevice::Text)) {
        outCapturedLabelText = "ত্রুটি: ফাইলটি খোলা যায়নি!";
        return false;
    }
    
    QTextStream in(&file);
    QString content = in.readAll();
    file.close();
    
    int start_pos = -1;
    int end_pos = -1;
    bool is_html = false;
    bool found = false;
    
    if (MarkdownUtils::get_heading_bounds(content, slug, start_pos, end_pos, is_html)) {
        found = true;
    } else if (MarkdownUtils::get_subheading_bounds(content, slug, start_pos, end_pos)) {
        found = true;
    }
    
    if (found) {
        QString deleted_chunk = content.mid(start_pos, end_pos - start_pos);
        content.remove(start_pos, end_pos - start_pos);
        
        if (file.open(QIODevice::WriteOnly | QIODevice::Truncate | QIODevice::Text)) {
            QTextStream out(&file);
            out << content;
            file.close();
        }
        
        QString del_filename = QString("%1_%2_%3.txt")
                               .arg(subjectName)
                               .arg(slug)
                               .arg(QDateTime::currentDateTime().toString("yyyyMMdd_hhmmss"));
        
        QString deleted_dir_path = notes_dir_path_ + QDir::separator() + "deleted";
        QString del_filepath = deleted_dir_path + QDir::separator() + del_filename;
        QFileInfo del_file_info(del_filepath);
        QDir del_file_dir = del_file_info.dir();
        if (!del_file_dir.exists()) {
            del_file_dir.mkpath(".");
        }
        
        QFile del_file(del_filepath);
        if (del_file.open(QIODevice::WriteOnly | QIODevice::Text)) {
            QTextStream del_out(&del_file);
            del_out << deleted_chunk;
            del_file.close();
            outCapturedLabelText = QString("মুছে ফেলা হয়েছে এবং ব্যাকআপ রাখা হয়েছে: %1").arg(del_filename);
        } else {
            outCapturedLabelText = "মুছে ফেলা হয়েছে কিন্তু ব্যাকআপ রাখা যায়নি!";
        }
        return true;
    } else {
        outCapturedLabelText = "ত্রুটি: শিরোনাম বা উপ-শিরোনামটি খুঁজে পাওয়া যায়নি!";
        return false;
    }
}

bool NoteRepository::shiftHeadingSection(const QString &targetFile, const QString &sourceSlug, const QString &targetSlug, const QList<NoteItem> &allHeadings, QString &outCapturedLabelText) {
    Q_UNUSED(allHeadings);
    QFile file(targetFile);
    if (!file.open(QIODevice::ReadWrite | QIODevice::Text)) {
        outCapturedLabelText = "ত্রুটি: ফাইলটি খোলা যায়নি!";
        return false;
    }
    
    QTextStream in(&file);
    QString content = in.readAll();
    file.close();

    int src_start = -1;
    int src_end = -1;
    bool is_html = false;
    bool found = false;

    if (MarkdownUtils::get_heading_bounds(content, sourceSlug, src_start, src_end, is_html)) {
        found = true;
    } else if (MarkdownUtils::get_subheading_bounds(content, sourceSlug, src_start, src_end)) {
        found = true;
    }

    if (!found) {
        outCapturedLabelText = "ত্রুটি: স্থানান্তর করার জন্য উৎস সেকশন পাওয়া যায়নি!";
        return false;
    }

    QString source_chunk = content.mid(src_start, src_end - src_start);
    content.remove(src_start, src_end - src_start);

    int insert_pos = -1;
    if (targetSlug.isEmpty()) {
        insert_pos = content.length();
    } else {
        int tgt_start = -1;
        int tgt_end = -1;
        bool tgt_is_html = false;
        
        bool tgt_found_h2 = MarkdownUtils::get_heading_bounds(content, targetSlug, tgt_start, tgt_end, tgt_is_html);
        if (tgt_found_h2) {
            insert_pos = tgt_end;
        } else {
            bool tgt_found_h3 = MarkdownUtils::get_subheading_insert_pos(content, targetSlug, insert_pos);
            if (!tgt_found_h3) {
                outCapturedLabelText = "ত্রুটি: গন্তব্য সেকশন খুঁজে পাওয়া যায়নি!";
                return false;
            }
        }
    }

    if (insert_pos > 0 && content[insert_pos - 1] != '\n') {
        source_chunk.prepend("\n");
    }
    content.insert(insert_pos, source_chunk);

    if (file.open(QIODevice::WriteOnly | QIODevice::Truncate | QIODevice::Text)) {
        QTextStream out(&file);
        out << content;
        file.close();
        outCapturedLabelText = QString("স্থানান্তর সফল হয়েছে: %1").arg(sourceSlug);
        return true;
    } else {
        outCapturedLabelText = "ত্রুটি: ফাইলে স্থানান্তর সম্পন্ন করা যায়নি!";
        return false;
    }
}

#include "NoteRepository.h"
#include "MarkdownUtils.h"
#include "../utils/FileIO.h"
#include "../utils/CrashGuard.h"
#include "../utils/Utils.h"

#include <QDateTime>
#include <QDir>
#include <QFileInfo>

bool NoteRepository::deleteHeadingSection(const QString &targetFile, const QString &slug,
                                          const QString &subjectName, QString &outCapturedLabelText) {
    if (targetFile.isEmpty() || slug.isEmpty()) {
        outCapturedLabelText = QStringLiteral("ত্রুটি: অবৈধ পথ বা স্লাগ!");
        return false;
    }

    return CrashGuard::safeCallValue<bool>([&]() -> bool {
        auto read = FileIO::readText(targetFile);
        if (read.isFail()) {
            outCapturedLabelText = QStringLiteral("ত্রুটি: ফাইলটি খোলা যায়নি!");
            return false;
        }

        QString content = read.takeValue();

        int start_pos = -1;
        int end_pos = -1;
        bool is_html = false;
        bool found = false;

        if (MarkdownUtils::get_heading_bounds(content, slug, start_pos, end_pos, is_html)) {
            found = true;
        } else if (MarkdownUtils::get_subheading_bounds(content, slug, start_pos, end_pos)) {
            found = true;
        }

        if (!found || start_pos < 0 || end_pos < start_pos || end_pos > content.size()) {
            outCapturedLabelText = QStringLiteral("ত্রুটি: শিরোনাম বা উপ-শিরোনামটি খুঁজে পাওয়া যায়নি!");
            return false;
        }

        const QString deleted_chunk = content.mid(start_pos, end_pos - start_pos);
        content.remove(start_pos, end_pos - start_pos);

        if (!FileIO::writeTextAtomic(targetFile, content).isOk()) {
            outCapturedLabelText = QStringLiteral("ত্রুটি: ফাইলে লেখা যায়নি!");
            return false;
        }

        // Safe backup filename: sanitize subject path segments
        QString safeSubject = sanitizeRelativePath(subjectName);
        if (safeSubject.isEmpty())
            safeSubject = QStringLiteral("unknown");
        safeSubject.replace(QLatin1Char('/'), QLatin1Char('_'));

        const QString del_filename = QStringLiteral("%1_%2_%3.txt")
                                         .arg(safeSubject, slug,
                                              QDateTime::currentDateTime().toString(QStringLiteral("yyyyMMdd_hhmmss")));

        const QString deleted_dir_path = notes_dir_path_ + QDir::separator() + QStringLiteral("deleted");
        const QString del_filepath = deleted_dir_path + QDir::separator() + del_filename;

        if (FileIO::writeTextAtomic(del_filepath, deleted_chunk).isOk()) {
            outCapturedLabelText = QStringLiteral("মুছে ফেলা হয়েছে এবং ব্যাকআপ রাখা হয়েছে: %1").arg(del_filename);
        } else {
            outCapturedLabelText = QStringLiteral("মুছে ফেলা হয়েছে কিন্তু ব্যাকআপ রাখা যায়নি!");
        }
        return true;
    }, false, QStringLiteral("deleteHeadingSection"));
}

bool NoteRepository::shiftHeadingSection(const QString &targetFile, const QString &sourceSlug,
                                         const QString &targetSlug, const QList<NoteItem> &allHeadings,
                                         QString &outCapturedLabelText) {
    Q_UNUSED(allHeadings);
    if (targetFile.isEmpty() || sourceSlug.isEmpty()) {
        outCapturedLabelText = QStringLiteral("ত্রুটি: অবৈধ পথ বা স্লাগ!");
        return false;
    }

    return CrashGuard::safeCallValue<bool>([&]() -> bool {
        auto read = FileIO::readText(targetFile);
        if (read.isFail()) {
            outCapturedLabelText = QStringLiteral("ত্রুটি: ফাইলটি খোলা যায়নি!");
            return false;
        }

        QString content = read.takeValue();

        int src_start = -1;
        int src_end = -1;
        bool is_html = false;
        bool found = false;

        if (MarkdownUtils::get_heading_bounds(content, sourceSlug, src_start, src_end, is_html)) {
            found = true;
        } else if (MarkdownUtils::get_subheading_bounds(content, sourceSlug, src_start, src_end)) {
            found = true;
        }

        if (!found || src_start < 0 || src_end < src_start || src_end > content.size()) {
            outCapturedLabelText = QStringLiteral("ত্রুটি: স্থানান্তর করার জন্য উৎস সেকশন পাওয়া যায়নি!");
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

            if (MarkdownUtils::get_heading_bounds(content, targetSlug, tgt_start, tgt_end, tgt_is_html)) {
                insert_pos = tgt_end;
            } else if (!MarkdownUtils::get_subheading_insert_pos(content, targetSlug, insert_pos)) {
                outCapturedLabelText = QStringLiteral("ত্রুটি: গন্তব্য সেকশন খুঁজে পাওয়া যায়নি!");
                return false;
            }
        }

        if (insert_pos < 0 || insert_pos > content.size()) {
            outCapturedLabelText = QStringLiteral("ত্রুটি: অবৈধ ইনসার্ট পজিশন!");
            return false;
        }

        if (insert_pos > 0 && content[insert_pos - 1] != QLatin1Char('\n')) {
            source_chunk.prepend(QLatin1Char('\n'));
        }
        content.insert(insert_pos, source_chunk);

        if (!FileIO::writeTextAtomic(targetFile, content).isOk()) {
            outCapturedLabelText = QStringLiteral("ত্রুটি: ফাইলে স্থানান্তর সম্পন্ন করা যায়নি!");
            return false;
        }

        outCapturedLabelText = QStringLiteral("স্থানান্তর সফল হয়েছে: %1").arg(sourceSlug);
        return true;
    }, false, QStringLiteral("shiftHeadingSection"));
}

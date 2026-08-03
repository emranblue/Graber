#include "NoteRepository.h"
#include "MarkdownUtils.h"
#include "../utils/Utils.h"

#include <QFile>
#include <QTextStream>
#include <QDateTime>
#include <fstream>

bool NoteRepository::appendContentToHeading(const QString &filePath, const QString &slug, const QString &processedText, int formatIndex, const QString &section) {
    QFile file(filePath);
    if (!file.open(QIODevice::ReadWrite | QIODevice::Text)) {
        return false;
    }
    
    QTextStream in(&file);
    QString content = in.readAll();
    file.close();
    
    int start_pos = -1;
    int end_pos = -1;
    bool is_html = false;
    
    if (MarkdownUtils::get_heading_bounds(content, slug, start_pos, end_pos, is_html)) {
        QString to_append = "";
        if (formatIndex == 2) {
            QString sub_slug = formatter_->generateSlug(processedText);
            to_append = QString("\n<h3 id=\"%1\" style=\"color: #2980b9; font-weight: bold; font-style: italic; margin-top: 10px; margin-bottom: 5px;\">%2</h3>\n")
                        .arg(sub_slug, processedText.trimmed());
        } else if (formatIndex == 1) {
            QString main_slug = formatter_->generateSlug(processedText);
            to_append = QString("\n<h2 id=\"%1\" data-section=\"%2\" style=\"color: #e74c3c; font-weight: bold; font-style: italic; margin-bottom: 5px;\">%3</h2>\n")
                        .arg(main_slug, section, processedText.trimmed());
        } else if (formatIndex == 3) {
            QString color = get_random_beautiful_color();
            QString color_alpha = color + "0f";
            to_append = QString("<div class=\"timeline-item\" style=\"border-left: 2px dashed %1; margin-left: 20px; padding-left: 20px; padding-bottom: 12px; position: relative;\"><span style=\"position: absolute; left: -2px; top: 18px; width: 12px; height: 2px; background-color: %1;\"></span><span style=\"position: absolute; left: 8px; top: 13px; color: %1; font-size: 10px; line-height: 1;\">➤</span><div style=\"background-color: %2; border: 1px solid %1; border-radius: 6px; padding: 8px 12px; display: inline-block; box-shadow: 1px 1px 3px rgba(0,0,0,0.05); margin-left: 10px;\"><span style=\"color: %1; font-weight: 600; font-family: 'Segoe UI', 'Kalpurush', sans-serif; font-size: 16px;\">%3</span></div></div>\n")
                        .arg(color, color_alpha, processedText.trimmed());
        } else if (formatIndex == 4) {
            to_append = QString("<p class=\"paragraph-item\" style=\"color: #2f3640; line-height: 1.6; font-family: 'Segoe UI', 'Kalpurush', sans-serif; margin-bottom: 10px; text-align: justify;\">%1</p>\n")
                        .arg(processedText.trimmed());
        } else {
            to_append = QString("- ▣ %1\n\n").arg(processedText.trimmed());
        }
        
        if (end_pos > 0 && content[end_pos - 1] != '\n') {
            to_append.prepend("\n");
        }
        content.insert(end_pos, to_append);
        
        if (file.open(QIODevice::WriteOnly | QIODevice::Truncate | QIODevice::Text)) {
            QTextStream out(&file);
            out << content;
            file.close();
            return true;
        }
    } else {
        int insert_pos = -1;
        if (MarkdownUtils::get_subheading_insert_pos(content, slug, insert_pos)) {
            QString to_append = "";
            if (formatIndex == 2) {
                QString sub_slug = formatter_->generateSlug(processedText);
                to_append = QString("\n<h3 id=\"%1\" style=\"color: #2980b9; font-weight: bold; font-style: italic; margin-top: 10px; margin-bottom: 5px;\">%2</h3>\n")
                            .arg(sub_slug, processedText.trimmed());
            } else if (formatIndex == 1) {
                QString main_slug = formatter_->generateSlug(processedText);
                to_append = QString("\n<h2 id=\"%1\" data-section=\"%2\" style=\"color: #e74c3c; font-weight: bold; font-style: italic; margin-bottom: 5px;\">%3</h2>\n")
                            .arg(main_slug, section, processedText.trimmed());
            } else if (formatIndex == 3) {
                QString color = get_random_beautiful_color();
                QString color_alpha = color + "0f";
                to_append = QString("<div class=\"timeline-item\" style=\"border-left: 2px dashed %1; margin-left: 20px; padding-left: 20px; padding-bottom: 12px; position: relative;\"><span style=\"position: absolute; left: -2px; top: 18px; width: 12px; height: 2px; background-color: %1;\"></span><span style=\"position: absolute; left: 8px; top: 13px; color: %1; font-size: 10px; line-height: 1;\">➤</span><div style=\"background-color: %2; border: 1px solid %1; border-radius: 6px; padding: 8px 12px; display: inline-block; box-shadow: 1px 1px 3px rgba(0,0,0,0.05); margin-left: 10px;\"><span style=\"color: %1; font-weight: 600; font-family: 'Segoe UI', 'Kalpurush', sans-serif; font-size: 16px;\">%3</span></div></div>\n")
                            .arg(color, color_alpha, processedText.trimmed());
            } else if (formatIndex == 4) {
                to_append = QString("<p class=\"paragraph-item\" style=\"color: #2f3640; line-height: 1.6; font-family: 'Segoe UI', 'Kalpurush', sans-serif; margin-bottom: 10px; text-align: justify;\">%1</p>\n")
                            .arg(processedText.trimmed());
            } else {
                to_append = QString("- ▣ %1\n\n").arg(processedText.trimmed());
            }
            
            if (insert_pos > 0 && content[insert_pos - 1] != '\n') {
                to_append.prepend("\n");
            }
            content.insert(insert_pos, to_append);
            
            if (file.open(QIODevice::WriteOnly | QIODevice::Truncate | QIODevice::Text)) {
                QTextStream out(&file);
                out << content;
                file.close();
                return true;
            }
        }
    }
    return false;
}

bool NoteRepository::writeToNote(const QString &targetFile, const QString &processedText, int formatIndex, const QString &section, const QString &selectedSlug, QString &lastDate, QString &outCapturedLabelText) {
    if (targetFile == "নির্বাচিত নয়") return false;

    if (!selectedSlug.isEmpty()) {
        if (appendContentToHeading(targetFile, selectedSlug, processedText, formatIndex, section)) {
            outCapturedLabelText = "শেষ ক্যাপচার (নির্বাচিত শিরোনামে যুক্ত করা হয়েছে): " + processedText;
            return true;
        } else {
            outCapturedLabelText = "ত্রুটি: নির্বাচিত শিরোনামে যুক্ত করা যায়নি!";
            return false;
        }
    }

    lastDate = MarkdownUtils::restore_state_from_file(targetFile);

    std::ofstream outfile;
    outfile.open(targetFile.toStdString(), std::ios_base::app);
    
    if (outfile.is_open()) {
        QDateTime now = QDateTime::currentDateTime();
        QString current_date = now.toString("dd MMMM, yyyy");
        
        if (current_date != lastDate) {
            outfile << "\n### ***" << current_date.toStdString() << "***\n";
            lastDate = current_date;
        }
        
        if (!processedText.isEmpty()) {
            if (formatIndex == 1) {
                QString title = processedText.trimmed();
                std::string slug = formatter_->generateSlug(title).toStdString();
                outfile << "\n<h2 id=\"" << slug << "\" data-section=\"" << section.toStdString() << "\" style=\"color: #e74c3c; font-weight: bold; font-style: italic; margin-bottom: 5px;\">" 
                        << title.toStdString() << "</h2>\n";
            } else if (formatIndex == 2) {
                QString title = processedText.trimmed();
                std::string slug = formatter_->generateSlug(title).toStdString();
                outfile << "\n<h3 id=\"" << slug << "\" style=\"color: #2980b9; font-weight: bold; font-style: italic; margin-top: 10px; margin-bottom: 5px;\">" 
                        << title.toStdString() << "</h3>\n";
            } else if (formatIndex == 3) {
                QString color = get_random_beautiful_color();
                outfile << "<div class=\"timeline-item\" style=\"border-left: 2px dashed " << color.toStdString() << "; margin-left: 20px; padding-left: 20px; padding-bottom: 12px; position: relative;\"><span style=\"position: absolute; left: -2px; top: 18px; width: 12px; height: 2px; background-color: " << color.toStdString() << ";\"></span><span style=\"position: absolute; left: 8px; top: 13px; color: " << color.toStdString() << "; font-size: 10px; line-height: 1;\">➤</span><div style=\"background-color: " << color.toStdString() << "0f; border: 1px solid " << color.toStdString() << "; border-radius: 6px; padding: 8px 12px; display: inline-block; box-shadow: 1px 1px 3px rgba(0,0,0,0.05); margin-left: 10px;\"><span style=\"color: " << color.toStdString() << "; font-weight: 600; font-family: 'Segoe UI', 'Kalpurush', sans-serif; font-size: 16px;\">" << processedText.trimmed().toStdString() << "</span></div></div>\n";
            } else if (formatIndex == 4) {
                outfile << "<p class=\"paragraph-item\" style=\"color: #2f3640; line-height: 1.6; font-family: 'Segoe UI', 'Kalpurush', sans-serif; margin-bottom: 10px; text-align: justify;\">" << processedText.trimmed().toStdString() << "</p>\n";
            } else {
                outfile << "\n ▣ " << processedText.trimmed().toStdString() << "\n\n";
            }
        }
        
        outfile.close();
        outCapturedLabelText = "শেষ ক্যাপচার: " + processedText;
        return true;
    } else {
        outCapturedLabelText = "ত্রুটি: ফাইলে লেখা যায়নি!";
        return false;
    }
}


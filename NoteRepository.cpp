#include "NoteRepository.h"
#include "Utils.h"
#include "MarkdownUtils.h"

#include <QDir>
#include <QDirIterator>
#include <QFileInfo>
#include <QFile>
#include <QTextStream>
#include <QSettings>
#include <QDateTime>
#include <QStandardPaths>
#include <QRegularExpression>
#include <fstream>
#include <iostream>

NoteRepository::NoteRepository() {
    notes_dir_path_ = QStandardPaths::writableLocation(QStandardPaths::HomeLocation) + QDir::separator() + "GraberNotes";
    QDir dir(notes_dir_path_);
    if (!dir.exists()) {
        dir.mkpath(".");
    }
}

QString NoteRepository::notesDirPath() const {
    return notes_dir_path_;
}

QStringList NoteRepository::populateSubjectsFromDisk(QComboBox *sectionDropdown) {
    QStringList all_subjects;
    QDirIterator it(notes_dir_path_, QStringList() << "*.md", QDir::Files, QDirIterator::Subdirectories);
    QDir base_dir(notes_dir_path_);
    while (it.hasNext()) {
        QString filepath = it.next();
        QString relative_path = base_dir.relativeFilePath(filepath);
        
        if (relative_path.startsWith(QString("deleted") + QDir::separator()) || relative_path == "deleted") {
            continue;
        }
        
        normalizeMarkdownFile(filepath);
        updateTocInFile(filepath, sectionDropdown);
        
        relative_path.chop(3); // Remove ".md"
        all_subjects << relative_path;
    }
    all_subjects.sort(Qt::CaseInsensitive);
    return all_subjects;
}

bool NoteRepository::createSubject(const QString &subjectName) {
    if (subjectName.isEmpty()) return false;
    QString filename = notes_dir_path_ + QDir::separator() + subjectName + ".md";
    QFileInfo file_info(filename);
    QDir parent_dir = file_info.dir();
    if (!parent_dir.exists()) {
        parent_dir.mkpath(".");
    }
    if (!QFile::exists(filename)) {
        QFile file(filename);
        if (file.open(QIODevice::WriteOnly)) {
            file.close();
            return true;
        }
    }
    return true;
}

bool NoteRepository::createFolder(const QString &folderPath, QString &outStatusMsg) {
    if (folderPath.isEmpty()) return false;
    QDir dir(notes_dir_path_);
    if (dir.mkpath(folderPath)) {
        outStatusMsg = "অবস্থা: ফোল্ডার তৈরি হয়েছে - " + folderPath;
        return true;
    } else {
        outStatusMsg = "অবস্থা: ফোল্ডার তৈরি করতে ব্যর্থ!";
        return false;
    }
}

void NoteRepository::saveSectionsForSubject(const QString &subjectName, QComboBox *sectionDropdown) {
    if (subjectName.isEmpty() || subjectName == "নির্বাচিত নয়" || !sectionDropdown) return;

    QString ini_path = notes_dir_path_ + QDir::separator() + subjectName + ".ini";
    QFileInfo fileInfo(ini_path);
    QDir().mkpath(fileInfo.absolutePath());

    QSettings settings(ini_path, QSettings::IniFormat);
    settings.beginGroup("Sections");
    settings.remove("");

    QStringList order;
    for (int i = 0; i < sectionDropdown->count(); ++i) {
        QString slug = sectionDropdown->itemData(i).toString();
        QString display_name = sectionDropdown->itemText(i);
        order.append(slug);
        settings.setValue(slug, display_name);
    }
    settings.setValue("_order", order.join(","));
    settings.endGroup();
}

void NoteRepository::loadSectionsForSubject(const QString &subjectName, QComboBox *sectionDropdown) {
    if (!sectionDropdown) return;
    sectionDropdown->clear();

    if (subjectName.isEmpty() || subjectName == "নির্বাচিত নয়") {
        sectionDropdown->addItem("অন্যান্য (Others)", "others");
        return;
    }

    QString ini_path = notes_dir_path_ + QDir::separator() + subjectName + ".ini";
    QFileInfo fileInfo(ini_path);

    if (!fileInfo.exists()) {
        sectionDropdown->addItem("অন্যান্য (Others)", "others");
        saveSectionsForSubject(subjectName, sectionDropdown);
        return;
    }

    QSettings settings(ini_path, QSettings::IniFormat);
    settings.beginGroup("Sections");
    
    QString order_str = settings.value("_order").toString();
    QStringList order = order_str.split(',', Qt::SkipEmptyParts);

    if (!order.isEmpty()) {
        for (const QString &slug : order) {
            QString trimmed_slug = slug.trimmed();
            if (settings.contains(trimmed_slug)) {
                QString display_name = settings.value(trimmed_slug).toString();
                sectionDropdown->addItem(display_name, trimmed_slug);
            }
        }
    } else {
        QStringList keys = settings.childKeys();
        for (const QString &slug : keys) {
            if (slug == "_order") continue;
            QString display_name = settings.value(slug).toString();
            sectionDropdown->addItem(display_name, slug);
        }
    }
    settings.endGroup();

    bool has_others = false;
    for (int i = 0; i < sectionDropdown->count(); ++i) {
        if (sectionDropdown->itemData(i).toString() == "others") {
            has_others = true;
            break;
        }
    }
    if (!has_others) {
        sectionDropdown->addItem("অন্যান্য (Others)", "others");
    }

    if (sectionDropdown->count() == 0) {
        sectionDropdown->addItem("অন্যান্য (Others)", "others");
        saveSectionsForSubject(subjectName, sectionDropdown);
    }
}

QString NoteRepository::getTargetFilePath(const QString &subjectName) const {
    if (subjectName.isEmpty() || subjectName == "নির্বাচিত নয়") {
        return "নির্বাচিত নয়";
    }
    return notes_dir_path_ + QDir::separator() + subjectName + ".md";
}

void NoteRepository::normalizeMarkdownFile(const QString &filePath) {
    QFile file(filePath);
    if (!file.open(QIODevice::ReadWrite | QIODevice::Text)) {
        return;
    }

    QTextStream in(&file);
    QString content = in.readAll();
    file.close();

    int toc_start = content.indexOf("<!-- TOC_START -->");
    int toc_end = content.indexOf("<!-- TOC_END -->");
    QString clean_content = content;
    if (toc_start != -1 && toc_end != -1) {
        QString pre_toc = content.left(toc_start);
        QString post_toc = content.mid(toc_end + QString("<!-- TOC_END -->").length());
        clean_content = pre_toc + post_toc;
    }

    QStringList lines = clean_content.split('\n');
    QStringList output_lines;

    QRegularExpression date_regex("^###\\s*(?:\\*\\*\\*)?\\s*([0-9০-৯]{1,2}\\s+(?:January|February|March|April|May|June|July|August|September|October|November|December|জানুয়ারি|ফেব্রুয়ারি|মার্চ|এপ্রিল|মে|জুন|জুলাই|আগস্ট|সেপ্টেম্বর|অক্টোবর|নভেম্বর|ডিসেম্বর)[,\\s]+[0-9০-৯]{4})\\s*(?:\\*\\*\\*)?$", QRegularExpression::CaseInsensitiveOption);
    QRegularExpression h2_regex("<h2([^>]*)>(.*?)</h2>", QRegularExpression::CaseInsensitiveOption);
    QRegularExpression md_regex("^(#{1,3})\\s+(?!\\*\\*\\*)(.*?)$");
    QRegularExpression md_section_regex("<!--\\s*section:([\\w-]+)\\s*-->");
    QRegularExpression section_attr_regex("data-section=\"([^\"]*)\"", QRegularExpression::CaseInsensitiveOption);
    QRegularExpression style_regex("style=\"([^\"]*)\"", QRegularExpression::CaseInsensitiveOption);

    for (int i = 0; i < lines.size(); ++i) {
        QString line = lines[i];
        QString trimmed_line = line.trimmed();

        QRegularExpressionMatch date_match = date_regex.match(trimmed_line);
        QRegularExpressionMatch h2_match = h2_regex.match(trimmed_line);
        QRegularExpressionMatch md_match = md_regex.match(trimmed_line);

        if (trimmed_line.contains("<div") && (trimmed_line.contains("border") || trimmed_line.contains("background-color")) && !trimmed_line.contains("timeline") && !trimmed_line.contains("bullet")) {
            continue;
        } else if (trimmed_line == "</div>") {
            continue;
        } else if (date_match.hasMatch()) {
            output_lines.append(line);
        } else if (h2_match.hasMatch()) {
            QString attributes = h2_match.captured(1);
            QString title = h2_match.captured(2).trimmed();
            QString slug = QString::fromStdString(MarkdownUtils::generate_slug(title));

            QString style = "color: #e74c3c; font-weight: bold; font-style: italic; margin-bottom: 5px;";
            QRegularExpressionMatch style_match = style_regex.match(attributes);
            if (style_match.hasMatch()) {
                style = style_match.captured(1);
            }

            QString section = MarkdownUtils::detect_section_from_title(title);
            QRegularExpressionMatch section_match = section_attr_regex.match(attributes);
            if (section_match.hasMatch()) {
                section = section_match.captured(1);
            }

            QString rewritten_line = QString("<h2 id=\"%1\" data-section=\"%2\" style=\"%3\">%4</h2>")
                                     .arg(slug, section, style, title);
            output_lines.append(rewritten_line);
        } else if (md_match.hasMatch()) {
            int level = md_match.captured(1).length();
            if (level == 3 && trimmed_line.contains("***")) {
                output_lines.append(line);
                continue;
            }

            QString rest = md_match.captured(2).trimmed();
            QString section = MarkdownUtils::detect_section_from_title(rest);
            QRegularExpressionMatch section_match = md_section_regex.match(rest);
            QString title = rest;
            if (section_match.hasMatch()) {
                section = section_match.captured(1);
                title = rest.left(section_match.capturedStart()).trimmed();
            }

            QString slug = QString::fromStdString(MarkdownUtils::generate_slug(title));

            if (level == 2) {
                QString style = "color: #e74c3c; font-weight: bold; font-style: italic; margin-bottom: 5px;";
                QString html_heading = QString("<h2 id=\"%1\" data-section=\"%2\" style=\"%3\">%4</h2>")
                                       .arg(slug, section, style, title);
                output_lines.append(html_heading);
            } else if (level == 3) {
                QString style = "color: #2980b9; font-weight: bold; font-style: italic; margin-top: 10px; margin-bottom: 5px;";
                QString html_subheading = QString("<h3 id=\"%1\" style=\"%2\">%3</h3>")
                                          .arg(slug, style, title);
                output_lines.append(html_subheading);
            }
        } else {
            output_lines.append(line);
        }
    }

    QString final_content = output_lines.join('\n');

    if (file.open(QIODevice::WriteOnly | QIODevice::Truncate | QIODevice::Text)) {
        QTextStream out(&file);
        out << final_content << "\n";
        file.close();
    }
}

void NoteRepository::updateTocInFile(const QString &filePath, QComboBox *sectionDropdown) {
    QFile file(filePath);
    if (!file.open(QIODevice::ReadWrite | QIODevice::Text)) {
        return;
    }

    QTextStream in(&file);
    QString content = in.readAll();
    file.close();

    QString clean_content = content;
    while (true) {
        int toc_start = clean_content.indexOf("<!-- TOC_START -->");
        int toc_end = clean_content.indexOf("<!-- TOC_END -->");
        if (toc_start != -1 && toc_end != -1 && toc_end > toc_start) {
            QString pre_toc = clean_content.left(toc_start);
            QString post_toc = clean_content.mid(toc_end + QString("<!-- TOC_END -->").length());
            clean_content = pre_toc + post_toc;
        } else {
            break;
        }
    }

    QStringList lines = clean_content.split('\n');
    
    struct HeadingInfo {
        int index;
        QString title;
        QString slug;
        QString date;
        bool is_html;
        QString style;
        int level;
        QString section;
    };
    
    QList<HeadingInfo> headings;
    QString current_date = "";
    
    QRegularExpression date_regex("^###\\s*(?:\\*\\*\\*)?\\s*([0-9০-৯]{1,2}\\s+(?:January|February|March|April|May|June|July|August|September|October|November|December|জানুয়ারি|ফেব্রুয়ারি|মার্চ|এপ্রিল|মে|জুন|জুলাই|আগস্ট|সেপ্টেম্বর|অক্টোবর|নভেম্বর|ডিসেম্বর)[,\\s]+[0-9০-৯]{4})\\s*(?:\\*\\*\\*)?$", QRegularExpression::CaseInsensitiveOption);
    QRegularExpression h2_regex("<h2([^>]*)>(.*?)</h2>", QRegularExpression::CaseInsensitiveOption);
    QRegularExpression h3_regex("<h3([^>]*)>(.*?)</h3>", QRegularExpression::CaseInsensitiveOption);
    QRegularExpression style_regex("style=\"([^\"]*)\"", QRegularExpression::CaseInsensitiveOption);
    QRegularExpression section_attr_regex("data-section=\"([^\"]*)\"", QRegularExpression::CaseInsensitiveOption);
    QRegularExpression md_regex("^(#{2})\\s+(.*?)$");
    QRegularExpression md_sub_regex("^(#{3})\\s+(?!\\*\\*\\*)(.*?)$");
    QRegularExpression md_section_regex("<!--\\s*section:([\\w-]+)\\s*-->");

    QStringList processed_lines;
    int heading_counter = 0;

    for (int i = 0; i < lines.size(); ++i) {
        QString line = lines[i];
        QString trimmed_line = line.trimmed();

        QRegularExpressionMatch date_match = date_regex.match(trimmed_line);
        QRegularExpressionMatch h2_match = h2_regex.match(trimmed_line);
        QRegularExpressionMatch h3_match = h3_regex.match(trimmed_line);
        QRegularExpressionMatch md_match = md_regex.match(trimmed_line);
        QRegularExpressionMatch md_sub_match = md_sub_regex.match(trimmed_line);

        if (date_match.hasMatch()) {
            current_date = date_match.captured(1).trimmed();
            processed_lines.append(line);
        } else if (h2_match.hasMatch()) {
            heading_counter++;
            QString attributes = h2_match.captured(1);
            QString title = h2_match.captured(2).trimmed();
            QString slug = QString::fromStdString(MarkdownUtils::generate_slug(title));

            QString style = "color: #e74c3c; font-weight: bold; font-style: italic; margin-bottom: 5px;";
            QRegularExpressionMatch style_match = style_regex.match(attributes);
            if (style_match.hasMatch()) {
                style = style_match.captured(1);
            }

            QString section = "others";
            QRegularExpressionMatch section_match = section_attr_regex.match(attributes);
            if (section_match.hasMatch()) {
                section = section_match.captured(1);
            }

            if (section.trimmed().isEmpty()) {
                section = "others";
            }

            HeadingInfo info;
            info.index = heading_counter;
            info.title = title;
            info.slug = slug;
            info.date = current_date;
            info.is_html = true;
            info.style = style;
            info.level = 2;
            info.section = section;
            headings.append(info);

            QString rewritten_line = QString("<h2 id=\"%1\" data-section=\"%2\" style=\"%3\">%4</h2>")
                                     .arg(slug, section, style, title);
            processed_lines.append(rewritten_line);
        } else if (h3_match.hasMatch()) {
            QString attributes = h3_match.captured(1);
            QString title = h3_match.captured(2).trimmed();
            QString slug = QString::fromStdString(MarkdownUtils::generate_slug(title));
            QString style = "color: #2980b9; font-weight: bold; font-style: italic; margin-top: 10px; margin-bottom: 5px;";
            QRegularExpressionMatch style_match = style_regex.match(attributes);
            if (style_match.hasMatch()) {
                style = style_match.captured(1);
            }
            
            QString rewritten_line = QString("<h3 id=\"%1\" style=\"%2\">%3</h3>")
                                     .arg(slug, style, title);
            processed_lines.append(rewritten_line);
        } else if (md_match.hasMatch()) {
            heading_counter++;
            QString rest = md_match.captured(2).trimmed();

            QString section = "others";
            QRegularExpressionMatch section_match = md_section_regex.match(rest);
            QString title = rest;
            if (section_match.hasMatch()) {
                section = section_match.captured(1);
                title = rest.left(section_match.capturedStart()).trimmed();
            }

            if (section.trimmed().isEmpty()) {
                section = "others";
            }

            QString slug = QString::fromStdString(MarkdownUtils::generate_slug(title));

            HeadingInfo info;
            info.index = heading_counter;
            info.title = title;
            info.slug = slug;
            info.date = current_date;
            info.is_html = false;
            info.style = "";
            info.level = 2;
            info.section = section;
            headings.append(info);

            processed_lines.append(line);
        } else if (md_sub_match.hasMatch()) {
            processed_lines.append(line);
        } else {
            processed_lines.append(line);
        }
    }

    QString toc_block = "";
    if (!headings.isEmpty()) {
        toc_block += "<!-- TOC_START -->\n";
        toc_block += "## সূচিপত্র (Table of Contents)\n\n";

        struct SectionDef {
            QString key;
            QString bangla;
            QString english;
        };

        QList<SectionDef> sections_list = {
            {"environment", "পরিবেশ", "Environment"},
            {"energy", "জ্বালানি", "Energy"},
            {"economy", "অর্থনীতি", "Economy"},
            {"culture", "সংস্কৃতি", "Culture"},
            {"geography", "ভূগোল", "Geography"},
            {"population", "জনসংখ্যা", "Population"},
            {"law-constitution", "আইন ও সংবিধান", "Law-Constitution"},
            {"politics", "রাজনীতি", "Politics"},
            {"freedom-fight", "মুক্তিযুদ্ধ", "Freedom-Fight"},
            {"agriculture", "কৃষি", "Agriculture"},
            {"history", "ইতিহাস", "History"},
            {"education", "শিক্ষা", "Education"},
            {"health", "স্বাস্থ্য", "Health"},
            {"science-tech", "বিজ্ঞান ও প্রযুক্তি", "Science-Tech"},
            {"foreign-policy", "পররাষ্ট্রনীতি", "Foreign-Policy"},
            {"administration", "প্রশাসন", "Administration"}
        };

        for (const HeadingInfo &info : headings) {
            if (info.section.isEmpty() || info.section == "others") continue;
            bool exists = false;
            for (const SectionDef &sec : sections_list) {
                if (sec.key == info.section) {
                    exists = true;
                    break;
                }
            }
            if (!exists) {
                QString bangla_name = info.section;
                QString english_name = info.section;
                if (sectionDropdown) {
                    for (int d_idx = 0; d_idx < sectionDropdown->count(); ++d_idx) {
                        if (sectionDropdown->itemData(d_idx).toString() == info.section) {
                            QString full_text = sectionDropdown->itemText(d_idx);
                            int paren_idx = full_text.indexOf('(');
                            if (paren_idx != -1) {
                                bangla_name = full_text.left(paren_idx).trimmed();
                                QString eng = full_text.mid(paren_idx + 1);
                                if (eng.endsWith(')')) eng.chop(1);
                                english_name = eng.trimmed();
                            } else {
                                bangla_name = full_text;
                                english_name = full_text;
                            }
                            break;
                        }
                    }
                }
                sections_list.append({info.section, bangla_name, english_name});
            }
        }
        
        sections_list.append({"others", "অন্যান্য", "Others"});

        for (const SectionDef &sec : sections_list) {
            QList<HeadingInfo> sec_headings;
            for (const HeadingInfo &info : headings) {
                if (info.section == sec.key) {
                    sec_headings.append(info);
                }
            }

            if (!sec_headings.isEmpty()) {
                toc_block += QString("### %1 (%2)\n").arg(sec.bangla, sec.english);
                toc_block += "| পৃষ্ঠা (Page) | তারিখ (Date) | আইডি (ID) | শিরোনাম (Chapter/Topic) |\n";
                toc_block += "| :---: | :---: | :---: | :--- |\n";

                for (const HeadingInfo &info : sec_headings) {
                    QString date_str = info.date.isEmpty() ? "---" : info.date;
                    toc_block += QString("| **%1** | %2 | `%3` | [%4](#%5) |\n")
                                 .arg(QString::number(info.index), date_str, info.slug, info.title, info.slug);
                }
                toc_block += "\n";
            }
        }

        toc_block += "---\n";
        toc_block += "<!-- TOC_END -->\n";
    }

    QString final_content = "";
    QString body_content = processed_lines.join('\n').trimmed();
    
    if (!toc_block.isEmpty()) {
        final_content = toc_block + "\n\n" + body_content;
    } else {
        final_content = body_content;
    }
    
    if (file.open(QIODevice::WriteOnly | QIODevice::Truncate | QIODevice::Text)) {
        QTextStream out(&file);
        out << final_content << "\n";
        file.close();
    }
}

void NoteRepository::parseNoteStructure(const QString &filePath, QList<NoteItem> &items, QComboBox *sectionDropdown, const QSet<QString> &customAddedSections, const QString &subjectName) {
    items.clear();
    QFile file(filePath);
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        return;
    }

    QTextStream in(&file);
    QString content = in.readAll();
    file.close();

    int toc_start = content.indexOf("<!-- TOC_START -->");
    int toc_end = content.indexOf("<!-- TOC_END -->");
    QString clean_content = content;
    if (toc_start != -1 && toc_end != -1) {
        QString pre_toc = content.left(toc_start);
        QString post_toc = content.mid(toc_end + QString("<!-- TOC_END -->").length());
        clean_content = pre_toc + post_toc;
    }

    QStringList lines = clean_content.split('\n');
    
    QRegularExpression h2_regex("<h2([^>]*)>(.*?)</h2>", QRegularExpression::CaseInsensitiveOption);
    QRegularExpression h3_regex("<h3([^>]*)>(.*?)</h3>", QRegularExpression::CaseInsensitiveOption);
    QRegularExpression md_regex("^(#{2})\\s+(.*?)$");
    QRegularExpression md_sub_regex("^(#{3})\\s+(?!\\*\\*\\*)(.*?)$");
    QRegularExpression section_attr_regex("data-section=\"([^\"]*)\"", QRegularExpression::CaseInsensitiveOption);

    QString current_h2_slug = "";
    bool new_section_added = false;

    for (int i = 0; i < lines.size(); ++i) {
        QString line = lines[i];
        QString trimmed_line = line.trimmed();

        QRegularExpressionMatch h2_match = h2_regex.match(trimmed_line);
        QRegularExpressionMatch h3_match = h3_regex.match(trimmed_line);
        QRegularExpressionMatch md_match = md_regex.match(trimmed_line);
        QRegularExpressionMatch md_sub_match = md_sub_regex.match(trimmed_line);

        if (h2_match.hasMatch()) {
            QString attributes = h2_match.captured(1);
            QString title = h2_match.captured(2).trimmed().normalized(QString::NormalizationForm_C);
            QString slug = QString::fromStdString(MarkdownUtils::generate_slug(title)).normalized(QString::NormalizationForm_C);
            
            QString section = "others";
            QRegularExpressionMatch section_match = section_attr_regex.match(attributes);
            if (section_match.hasMatch()) {
                section = section_match.captured(1);
            }
            section = section.normalized(QString::NormalizationForm_C);

            if (sectionDropdown && !section.isEmpty() && section != "others") {
                bool exists = false;
                for (int idx = 0; idx < sectionDropdown->count(); ++idx) {
                    if (sectionDropdown->itemData(idx).toString() == section) {
                        exists = true;
                        break;
                    }
                }
                if (!exists) {
                    QString display_name = section;
                    if (display_name.length() > 0) {
                        display_name[0] = display_name[0].toUpper();
                    }
                    sectionDropdown->addItem(QString("%1 (%2)").arg(display_name, section.toUpper()), section);
                    new_section_added = true;
                }
            }

            NoteItem item = {title, slug, "heading", section, ""};
            items.append(item);
            current_h2_slug = slug;
        } else if (md_match.hasMatch()) {
            QString title = md_match.captured(2).trimmed().normalized(QString::NormalizationForm_C);
            QString slug = QString::fromStdString(MarkdownUtils::generate_slug(title)).normalized(QString::NormalizationForm_C);
            QString section = MarkdownUtils::detect_section_from_title(title).normalized(QString::NormalizationForm_C);

            if (sectionDropdown && !section.isEmpty() && section != "others") {
                bool exists = false;
                for (int idx = 0; idx < sectionDropdown->count(); ++idx) {
                    if (sectionDropdown->itemData(idx).toString() == section) {
                        exists = true;
                        break;
                    }
                }
                if (!exists) {
                    QString display_name = section;
                    if (display_name.length() > 0) {
                        display_name[0] = display_name[0].toUpper();
                    }
                    sectionDropdown->addItem(QString("%1 (%2)").arg(display_name, section.toUpper()), section);
                    new_section_added = true;
                }
            }

            NoteItem item = {title, slug, "heading", section, ""};
            items.append(item);
            current_h2_slug = slug;
        } else if (h3_match.hasMatch()) {
            QString title = h3_match.captured(2).trimmed().normalized(QString::NormalizationForm_C);
            QString slug = QString::fromStdString(MarkdownUtils::generate_slug(title)).normalized(QString::NormalizationForm_C);

            NoteItem item = {title, slug, "subheading", "others", current_h2_slug};
            items.append(item);
        } else if (md_sub_match.hasMatch()) {
            QString title = md_sub_match.captured(2).trimmed().normalized(QString::NormalizationForm_C);
            QString slug = QString::fromStdString(MarkdownUtils::generate_slug(title)).normalized(QString::NormalizationForm_C);

            NoteItem item = {title, slug, "subheading", "others", current_h2_slug};
            items.append(item);
        }
    }

    if (sectionDropdown) {
        QSet<QString> used_sections;
        used_sections.insert("others");
        for (const NoteItem &item : items) {
            if (!item.section.isEmpty()) {
                used_sections.insert(item.section);
            }
        }
        for (const QString &custom_sec : customAddedSections) {
            used_sections.insert(custom_sec);
        }

        bool sections_pruned = false;
        for (int i = sectionDropdown->count() - 1; i >= 0; --i) {
            QString slug = sectionDropdown->itemData(i).toString();
            if (!used_sections.contains(slug)) {
                sectionDropdown->removeItem(i);
                sections_pruned = true;
            }
        }

        if ((new_section_added || sections_pruned) && !subjectName.isEmpty()) {
            saveSectionsForSubject(subjectName, sectionDropdown);
        }
    }
}

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
            QString sub_slug = QString::fromStdString(MarkdownUtils::generate_slug(processedText));
            to_append = QString("\n<h3 id=\"%1\" style=\"color: #2980b9; font-weight: bold; font-style: italic; margin-top: 10px; margin-bottom: 5px;\">%2</h3>\n")
                        .arg(sub_slug, processedText.trimmed());
        } else if (formatIndex == 1) {
            QString main_slug = QString::fromStdString(MarkdownUtils::generate_slug(processedText));
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
                QString sub_slug = QString::fromStdString(MarkdownUtils::generate_slug(processedText));
                to_append = QString("\n<h3 id=\"%1\" style=\"color: #2980b9; font-weight: bold; font-style: italic; margin-top: 10px; margin-bottom: 5px;\">%2</h3>\n")
                            .arg(sub_slug, processedText.trimmed());
            } else if (formatIndex == 1) {
                QString main_slug = QString::fromStdString(MarkdownUtils::generate_slug(processedText));
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
                std::string slug = MarkdownUtils::generate_slug(title);
                outfile << "\n<h2 id=\"" << slug << "\" data-section=\"" << section.toStdString() << "\" style=\"color: #e74c3c; font-weight: bold; font-style: italic; margin-bottom: 5px;\">" 
                        << title.toStdString() << "</h2>\n";
            } else if (formatIndex == 2) {
                QString title = processedText.trimmed();
                std::string slug = MarkdownUtils::generate_slug(title);
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

bool NoteRepository::writeImageToNote(const QString &targetFile, const QString &imageFilename, QString &lastDate) {
    if (targetFile == "নির্বাচিত নয়") return false;

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
        
        outfile << "\n![Image](images/" << imageFilename.toStdString() << ")\n\n";
        outfile.close();
        return true;
    }
    return false;
}

bool NoteRepository::injectHeadingToNote(const QString &targetFile, const QString &simplifiedText, const QString &section, QString &lastDate) {
    if (targetFile == "নির্বাচিত নয়") return false;

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
        
        QString title = simplifiedText.trimmed();
        std::string slug = MarkdownUtils::generate_slug(title);
        
        outfile << "\n<h2 id=\"" << slug << "\" data-section=\"" << section.toStdString() << "\" style=\"color: #e74c3c; font-weight: bold; font-style: italic; margin-bottom: 5px;\">" 
                << title.toStdString() << "</h2>\n";
        
        outfile.close();
        return true;
    }
    return false;
}

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

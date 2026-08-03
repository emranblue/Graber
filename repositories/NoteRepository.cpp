#include "NoteRepository.h"
#include "MarkdownDocumentFormatter.h"
#include "IniSectionRepository.h"
#include "../utils/Utils.h"
#include "MarkdownUtils.h"

#include <QDir>
#include <QDirIterator>
#include <QFileInfo>
#include <QFile>
#include <QTextStream>
#include <QDateTime>
#include <QStandardPaths>
#include <fstream>

NoteRepository::NoteRepository() 
    : formatter_(std::make_shared<MarkdownDocumentFormatter>()),
      section_repo_(std::make_shared<IniSectionRepository>()) {
    notes_dir_path_ = QStandardPaths::writableLocation(QStandardPaths::HomeLocation) + QDir::separator() + "GraberNotes";
    QDir dir(notes_dir_path_);
    if (!dir.exists()) {
        dir.mkpath(".");
    }
}

NoteRepository::NoteRepository(std::shared_ptr<IDocumentFormatter> formatter,
                               std::shared_ptr<ISectionRepository> sectionRepo)
    : formatter_(std::move(formatter)),
      section_repo_(std::move(sectionRepo)) {
    notes_dir_path_ = QStandardPaths::writableLocation(QStandardPaths::HomeLocation) + QDir::separator() + "GraberNotes";
    QDir dir(notes_dir_path_);
    if (!dir.exists()) {
        dir.mkpath(".");
    }
}

QString NoteRepository::notesDirPath() const {
    return notes_dir_path_;
}

QStringList NoteRepository::populateFoldersFromDisk() {
    QStringList folders;
    QDir base_dir(notes_dir_path_);
    QDirIterator it(notes_dir_path_, QDir::Dirs | QDir::NoDotAndDotDot, QDirIterator::Subdirectories);
    while (it.hasNext()) {
        QString dir_path = it.next();
        QString rel = base_dir.relativeFilePath(dir_path);
        rel.replace('\\', '/');
        
        if (rel == "deleted" || rel.startsWith("deleted/") ||
            rel == "images" || rel.endsWith("/images") || rel.contains("/images/") ||
            rel == "backup" || rel.startsWith("backup/") ||
            rel.startsWith(".git") || rel.startsWith("build")) {
            continue;
        }
        folders << rel;
    }
    folders.sort(Qt::CaseInsensitive);
    return folders;
}

QStringList NoteRepository::populateSubjectsFromDisk(const QList<SectionItem> &sections, const QString &folderFilter) {
    QStringList all_subjects;
    QDirIterator it(notes_dir_path_, QStringList() << "*.md", QDir::Files, QDirIterator::Subdirectories);
    QDir base_dir(notes_dir_path_);
    while (it.hasNext()) {
        QString filepath = it.next();
        QString relative_path = base_dir.relativeFilePath(filepath);
        relative_path.replace('\\', '/');
        
        if (relative_path == "deleted" || relative_path.startsWith("deleted/")) {
            continue;
        }

        relative_path.chop(3); // Remove ".md"

        if (!folderFilter.isEmpty() && folderFilter != "__ALL__") {
            if (folderFilter == "__ROOT__") {
                if (relative_path.contains('/')) {
                    continue;
                }
            } else {
                QString norm_filter = folderFilter;
                norm_filter.replace('\\', '/');
                QString prefix = norm_filter + "/";
                if (!relative_path.startsWith(prefix)) {
                    continue;
                }
            }
        }
        
        normalizeNoteFile(filepath);
        updateTocInFile(filepath, sections);
        
        all_subjects << relative_path;
    }
    all_subjects.sort(Qt::CaseInsensitive);
    return all_subjects;
}

QList<SubjectItem> NoteRepository::populateSubjectItemsFromDisk(const QList<SectionItem> &sections, const QString &folderFilter) {
    QList<SubjectItem> items;
    QStringList raw_subjects = populateSubjectsFromDisk(sections, folderFilter);
    for (const QString &full : raw_subjects) {
        SubjectItem item;
        item.fullPath = full;
        int last_slash = full.lastIndexOf('/');
        if (last_slash != -1) {
            item.folderPath = full.left(last_slash);
            if (!folderFilter.isEmpty() && folderFilter != "__ALL__" && folderFilter != "__ROOT__") {
                item.displayName = "📄 " + full.mid(last_slash + 1);
            } else {
                item.displayName = "📄 " + item.folderPath + " / " + full.mid(last_slash + 1);
            }
        } else {
            item.folderPath = "";
            if (!folderFilter.isEmpty() && folderFilter != "__ALL__" && folderFilter != "__ROOT__") {
                item.displayName = "📄 " + full;
            } else {
                item.displayName = "📄 " + full + " (Root)";
            }
        }
        items.append(item);
    }
    return items;
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

bool NoteRepository::moveSubject(const QString &oldSubjectName, const QString &newSubjectName, QString &outStatusMsg) {
    if (oldSubjectName.isEmpty() || newSubjectName.isEmpty()) {
        outStatusMsg = "অকার্যকর বিষয়ের নাম!";
        return false;
    }

    QString old_md = notes_dir_path_ + QDir::separator() + oldSubjectName + ".md";
    QString new_md = notes_dir_path_ + QDir::separator() + newSubjectName + ".md";

    if (!QFile::exists(old_md)) {
        outStatusMsg = "মূল বিষয় ফাইলটি পাওয়া যায়নি!";
        return false;
    }

    QFileInfo new_info(new_md);
    QDir().mkpath(new_info.absolutePath());

    if (!QFile::rename(old_md, new_md)) {
        if (!QFile::copy(old_md, new_md) || !QFile::remove(old_md)) {
            outStatusMsg = "নোট ফাইল স্থানান্তর ব্যর্থ হয়েছে!";
            return false;
        }
    }

    // Move associated .ini section config
    QString old_ini = notes_dir_path_ + QDir::separator() + oldSubjectName + ".ini";
    QString new_ini = notes_dir_path_ + QDir::separator() + newSubjectName + ".ini";
    if (QFile::exists(old_ini)) {
        if (!QFile::rename(old_ini, new_ini)) {
            if (QFile::copy(old_ini, new_ini)) {
                QFile::remove(old_ini);
            }
        }
    }

    // Move associated .tree structure file
    QString old_tree = notes_dir_path_ + QDir::separator() + oldSubjectName + ".tree";
    QString new_tree = notes_dir_path_ + QDir::separator() + newSubjectName + ".tree";
    if (QFile::exists(old_tree)) {
        if (!QFile::rename(old_tree, new_tree)) {
            if (QFile::copy(old_tree, new_tree)) {
                QFile::remove(old_tree);
            }
        }
    }

    outStatusMsg = "বিষয় সফলভাবে স্থানান্তর করা হয়েছে!";
    return true;
}

QList<SectionItem> NoteRepository::loadSectionsForSubject(const QString &subjectName) {
    return section_repo_->loadSectionsForSubject(notes_dir_path_, subjectName);
}

void NoteRepository::saveSectionsForSubject(const QString &subjectName, const QList<SectionItem> &sections) {
    section_repo_->saveSectionsForSubject(notes_dir_path_, subjectName, sections);
}

QString NoteRepository::getTargetFilePath(const QString &subjectName) const {
    if (subjectName.isEmpty() || subjectName == "নির্বাচিত নয়") {
        return "নির্বাচিত নয়";
    }
    return notes_dir_path_ + QDir::separator() + subjectName + ".md";
}

void NoteRepository::normalizeNoteFile(const QString &filePath) {
    QFile file(filePath);
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        return;
    }

    QTextStream in(&file);
    QString content = in.readAll();
    file.close();

    QString normalized = formatter_->normalizeContent(content);

    if (file.open(QIODevice::WriteOnly | QIODevice::Truncate | QIODevice::Text)) {
        QTextStream out(&file);
        out << normalized << "\n";
        file.close();
    }
}

void NoteRepository::updateTocInFile(const QString &filePath, const QList<SectionItem> &sections) {
    QFile file(filePath);
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        return;
    }

    QTextStream in(&file);
    QString content = in.readAll();
    file.close();

    QString updated_content = formatter_->updateTocInContent(content, sections);

    if (file.open(QIODevice::WriteOnly | QIODevice::Truncate | QIODevice::Text)) {
        QTextStream out(&file);
        out << updated_content << "\n";
        file.close();
    }
}

QList<NoteItem> NoteRepository::parseNoteStructure(const QString &filePath, const QList<SectionItem> &sections, QSet<QString> &customAddedSections, const QString &subjectName) {
    QFile file(filePath);
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        return {};
    }

    QTextStream in(&file);
    QString content = in.readAll();
    file.close();

    QList<NoteItem> items = formatter_->parseNoteStructure(content, sections, customAddedSections);
    
    QString tree_path = filePath;
    formatter_->saveStructureTree(tree_path, items);

    return items;
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

bool NoteRepository::insertDiagramToNote(const QString &targetFile, const QString &diagramMarkdown, const QString &selectedSlug, QString &lastDate) {
    if (targetFile == "নির্বাচিত নয়") return false;
    if (diagramMarkdown.isEmpty()) return false;

    if (!selectedSlug.isEmpty()) {
        QFile file(targetFile);
        if (!file.open(QIODevice::ReadWrite | QIODevice::Text)) {
            return false;
        }

        QTextStream in(&file);
        QString content = in.readAll();
        file.close();

        int start_pos = -1;
        int end_pos = -1;
        bool is_html = false;
        int insert_pos = -1;

        if (MarkdownUtils::get_heading_bounds(content, selectedSlug, start_pos, end_pos, is_html)) {
            insert_pos = end_pos;
        } else if (!MarkdownUtils::get_subheading_insert_pos(content, selectedSlug, insert_pos)) {
            return false;
        }

        QString to_insert = diagramMarkdown;
        if (insert_pos > 0 && content[insert_pos - 1] != '\n') {
            to_insert.prepend("\n");
        }
        content.insert(insert_pos, to_insert);

        if (file.open(QIODevice::WriteOnly | QIODevice::Truncate | QIODevice::Text)) {
            QTextStream out(&file);
            out << content;
            file.close();
            return true;
        }
        return false;
    }

    // No heading selected: append at the end of file, under the active date
    // section — same placement behaviour as writeImageToNote().
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

        outfile << diagramMarkdown.toStdString();
        outfile.close();
        return true;
    }
    return false;
}

bool NoteRepository::upsertLiveDiagram(const QString &targetFile, const QString &sessionId, const QString &diagramMarkdown, const QString &selectedSlug, QString &lastDate) {
    if (targetFile == "নির্বাচিত নয়") return false;
    if (diagramMarkdown.isEmpty() || sessionId.isEmpty()) return false;

    const QString start_marker = QString("<!-- DIAGRAM:%1 START -->").arg(sessionId);
    const QString end_marker = QString("<!-- DIAGRAM:%1 END -->").arg(sessionId);
    const QString wrapped_block = start_marker + "\n" + diagramMarkdown.trimmed() + "\n" + end_marker;

    QFile file(targetFile);
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        return false;
    }
    QTextStream in(&file);
    QString content = in.readAll();
    file.close();

    int start_idx = content.indexOf(start_marker);
    int end_idx = content.indexOf(end_marker);

    if (start_idx != -1 && end_idx != -1 && end_idx > start_idx) {
        // This diagram session already has a block in the note — grow it in
        // place instead of appending a new copy every time a node is captured.
        int block_end = end_idx + end_marker.length();
        content.replace(start_idx, block_end - start_idx, wrapped_block);

        if (file.open(QIODevice::WriteOnly | QIODevice::Truncate | QIODevice::Text)) {
            QTextStream out(&file);
            out << content;
            file.close();
            return true;
        }
        return false;
    }

    // First node captured for this session: insert a fresh block, following
    // the same placement rules as insertDiagramToNote (under the selected
    // heading if one is set, otherwise appended at the end of the file).
    if (!selectedSlug.isEmpty()) {
        int start_pos = -1;
        int end_pos = -1;
        bool is_html = false;
        int insert_pos = -1;

        if (MarkdownUtils::get_heading_bounds(content, selectedSlug, start_pos, end_pos, is_html)) {
            insert_pos = end_pos;
        } else if (!MarkdownUtils::get_subheading_insert_pos(content, selectedSlug, insert_pos)) {
            insert_pos = content.length();
        }

        QString to_insert = wrapped_block + "\n";
        if (insert_pos > 0 && content[insert_pos - 1] != '\n') {
            to_insert.prepend("\n");
        }
        content.insert(insert_pos, to_insert);

        if (file.open(QIODevice::WriteOnly | QIODevice::Truncate | QIODevice::Text)) {
            QTextStream out(&file);
            out << content;
            file.close();
            return true;
        }
        return false;
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

        outfile << "\n" << wrapped_block.toStdString() << "\n";
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
        std::string slug = formatter_->generateSlug(title).toStdString();
        
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

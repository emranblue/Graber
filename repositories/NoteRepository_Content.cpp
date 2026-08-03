#include "NoteRepository.h"
#include "MarkdownUtils.h"

#include <QFile>
#include <QTextStream>
#include <QDateTime>
#include <fstream>

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

QList<NoteItem> NoteRepository::parseNoteStructure(const QString &filePath,
                                                   const QList<SectionItem> &sections,
                                                   QSet<QString> &customAddedSections,
                                                   const QString &subjectName) {
    Q_UNUSED(subjectName);
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

bool NoteRepository::writeImageToNote(const QString &targetFile, const QString &imageFilename,
                                      QString &lastDate) {
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

bool NoteRepository::injectHeadingToNote(const QString &targetFile, const QString &simplifiedText,
                                         const QString &section, QString &lastDate) {
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

        outfile << "\n<h2 id=\"" << slug << "\" data-section=\"" << section.toStdString()
                << "\" style=\"color: #e74c3c; font-weight: bold; font-style: italic; margin-bottom: 5px;\">"
                << title.toStdString() << "</h2>\n";

        outfile.close();
        return true;
    }
    return false;
}

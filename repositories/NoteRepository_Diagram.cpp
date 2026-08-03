#include "NoteRepository.h"
#include "MarkdownUtils.h"

#include <QFile>
#include <QTextStream>
#include <QDateTime>
#include <fstream>

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


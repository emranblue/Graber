#ifndef MARKDOWNUTILS_H
#define MARKDOWNUTILS_H

#include <QString>
#include <QStringList>
#include <QList>
#include <string>
#include "Types.h"

namespace MarkdownUtils {
    std::string generate_slug(const QString &text);
    QString detect_section_from_title(const QString &title);
    QList<SectionItem> get_default_sections();
    void save_tree_file(const QString &file_path, const QList<NoteItem> &items);
    QString restore_state_from_file(const QString &file_path);
    bool get_heading_bounds(const QString &content, const QString &slug, int &start_pos, int &end_pos, bool &is_html);
    bool get_subheading_insert_pos(const QString &content, const QString &slug, int &insert_pos);
    bool get_subheading_bounds(const QString &content, const QString &slug, int &start_pos, int &end_pos);

    // Computes the same numeric display ids used in the TOC ("1", "2", "1.1", "2.1"...)
    // for a list of NoteItems in document order. Index i of the returned list is the
    // display id for all_headings[i]. Keep this in sync with the numbering scheme in
    // MarkdownDocumentFormatter::updateTocInContent so the UI never shows a heading
    // "id" that disagrees with what's written into the note's Table of Contents.
    QStringList compute_display_ids(const QList<NoteItem> &all_headings);
}

#endif // MARKDOWNUTILS_H

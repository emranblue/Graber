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
}

#endif // MARKDOWNUTILS_H

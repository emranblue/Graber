#ifndef TOC_GENERATOR_H
#define TOC_GENERATOR_H

#include <QString>
#include <QList>
#include "../../core/application_state.h"

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

class TOCGenerator {
public:
    static void updateTOC(const QString &file_path);
    static void parseNoteStructure(const QString &file_path, QList<NoteItem> &items);
    
private:
    static QString generateTOCBlock(const QList<HeadingInfo> &headings);
    static bool getHeadingBounds(const QString &content, const QString &slug, int &start_pos, int &end_pos, bool &is_html);
    static bool getSubheadingBounds(const QString &content, const QString &slug, int &start_pos, int &end_pos);
};

#endif // TOC_GENERATOR_H

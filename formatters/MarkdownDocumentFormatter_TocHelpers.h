#ifndef MARKDOWNDOCUMENTFORMATTER_TOCHELPERS_H
#define MARKDOWNDOCUMENTFORMATTER_TOCHELPERS_H

#include <QString>
#include <QStringList>
#include <QList>
#include "Types.h"

namespace MarkdownTocHelpers {

struct HeadingInfo {
    int index = 0;
    int sub_index = 0;
    QString title;
    QString slug;
    QString date;
    bool is_html = true;
    QString style;
    int level = 2;
    QString section;
    QString parent_slug;
    QString excerpt;
    int line_index = -1;
};

QString plainExcerpt(const QString &raw, int maxChars = 140);
QString extractPreview(const QStringList &lines, int startLine);
QString stripExistingToc(const QString &content);
QString buildTocBlock(const QList<HeadingInfo> &headings,
                      const QList<SectionItem> &sections);

} // namespace MarkdownTocHelpers

#endif

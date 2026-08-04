#ifndef IDOCUMENTFORMATTER_H
#define IDOCUMENTFORMATTER_H

#include <QString>
#include <QList>
#include <QSet>
#include "Types.h"

class IDocumentFormatter {
public:
    virtual ~IDocumentFormatter() = default;

    virtual QString generateSlug(const QString &text) const = 0;
    virtual QString normalizeContent(const QString &content) const = 0;
    virtual QString generateToc(const QString &content, const QList<SectionItem> &sections) const = 0;
    virtual QString updateTocInContent(const QString &content, const QList<SectionItem> &sections) const = 0;
    virtual QList<NoteItem> parseNoteStructure(const QString &content, const QList<SectionItem> &availableSections, QSet<QString> &outCustomSections) const = 0;
    virtual void saveStructureTree(const QString &treeFilePath, const QList<NoteItem> &items) const = 0;
    virtual QString restoreStateFromContent(const QString &content) const = 0;
};

#endif // IDOCUMENTFORMATTER_H

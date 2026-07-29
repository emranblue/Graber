#ifndef MARKDOWNDOCUMENTFORMATTER_H
#define MARKDOWNDOCUMENTFORMATTER_H

#include "interfaces/IDocumentFormatter.h"

class MarkdownDocumentFormatter : public IDocumentFormatter {
public:
    MarkdownDocumentFormatter() = default;
    ~MarkdownDocumentFormatter() override = default;

    QString generateSlug(const QString &text) const override;
    QString detectSectionFromTitle(const QString &title) const override;
    QString normalizeContent(const QString &content) const override;
    QString generateToc(const QString &content, const QList<SectionItem> &sections) const override;
    QString updateTocInContent(const QString &content, const QList<SectionItem> &sections) const override;
    QList<NoteItem> parseNoteStructure(const QString &content, const QList<SectionItem> &availableSections, QSet<QString> &outCustomSections) const override;
    void saveStructureTree(const QString &treeFilePath, const QList<NoteItem> &items) const override;
    QString restoreStateFromContent(const QString &content) const override;
};

#endif // MARKDOWNDOCUMENTFORMATTER_H

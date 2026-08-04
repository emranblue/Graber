#ifndef MARKDOWNDOCUMENTFORMATTER_H
#define MARKDOWNDOCUMENTFORMATTER_H

#include "interfaces/IDocumentFormatter.h"

/**
 * Markdown / HTML note formatter.
 *
 * Implementation is split for smaller, focused edits:
 *   MarkdownDocumentFormatter.cpp          — slug, diagram, tree, restore
 *   MarkdownDocumentFormatter_Normalize.cpp — heading normalize / cleanup
 *   MarkdownDocumentFormatter_Toc.cpp       — TOC build & inject
 *   MarkdownDocumentFormatter_Parse.cpp     — structure parse → NoteItem list
 */
class MarkdownDocumentFormatter : public IDocumentFormatter {
public:
    MarkdownDocumentFormatter() = default;
    ~MarkdownDocumentFormatter() override = default;

    QString generateSlug(const QString &text) const override;
    QString normalizeContent(const QString &content) const override;
    QString generateToc(const QString &content, const QList<SectionItem> &sections) const override;
    QString updateTocInContent(const QString &content, const QList<SectionItem> &sections) const override;
    QList<NoteItem> parseNoteStructure(const QString &content,
                                       const QList<SectionItem> &availableSections,
                                       QSet<QString> &outCustomSections) const override;
    void saveStructureTree(const QString &treeFilePath, const QList<NoteItem> &items) const override;
    QString restoreStateFromContent(const QString &content) const override;

    /** Fill a Mermaid template with clipboard text (single-slot skeletons). */
    QString formatDiagram(const QString &content, const QString &diagramType) const;
};

#endif // MARKDOWNDOCUMENTFORMATTER_H

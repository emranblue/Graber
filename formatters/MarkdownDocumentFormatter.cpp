#include "MarkdownDocumentFormatter.h"
#include "MarkdownUtils.h"
#include "DiagramTemplates.h"

#include <QRegularExpression>
#include <QTextStream>
#include <QStringList>

// ============================================================================
// Core / small helpers (slug, diagram, tree, restore)
// Heavier work lives in:
//   MarkdownDocumentFormatter_Normalize.cpp
//   MarkdownDocumentFormatter_Toc.cpp
//   MarkdownDocumentFormatter_Parse.cpp
// ============================================================================

QString MarkdownDocumentFormatter::generateSlug(const QString &text) const {
    return QString::fromStdString(MarkdownUtils::generate_slug(text));
}

QString MarkdownDocumentFormatter::formatDiagram(const QString &content,
                                                 const QString &diagramType) const {
    if (content.trimmed().isEmpty())
        return {};

    return DiagramTemplates::buildFromNodes(diagramType, QStringList{content});
}

void MarkdownDocumentFormatter::saveStructureTree(const QString &treeFilePath,
                                                  const QList<NoteItem> &items) const {
    MarkdownUtils::save_tree_file(treeFilePath, items);
}

QString MarkdownDocumentFormatter::restoreStateFromContent(const QString &content) const {
    static const QRegularExpression date_regex(
        QStringLiteral(
            "^###\\s*(?:\\*\\*\\*)?\\s*"
            "([0-9০-৯]{1,2}\\s+"
            "(?:January|February|March|April|May|June|July|August|September|"
            "October|November|December|"
            "জানুয়ারি|ফেব্রুয়ারি|মার্চ|এপ্রিল|মে|জুন|জুলাই|আগস্ট|"
            "সেপ্টেম্বর|অক্টোবর|নভেম্বর|ডিসেম্বর)"
            "[,\\s]+[0-9০-৯]{4})\\s*(?:\\*\\*\\*)?$"),
        QRegularExpression::CaseInsensitiveOption);

    QString last_found_date;
    const QStringList lines = content.split(QLatin1Char('\n'));
    for (const QString &raw : lines) {
        const QRegularExpressionMatch match = date_regex.match(raw.trimmed());
        if (match.hasMatch())
            last_found_date = match.captured(1).trimmed();
    }
    return last_found_date;
}

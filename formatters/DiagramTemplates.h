#ifndef DIAGRAMTEMPLATES_H
#define DIAGRAMTEMPLATES_H

#include <QString>
#include <QStringList>
#include <QList>
#include <QPair>

/**
 * DiagramTemplates – builds live multi-node diagrams from clipboard captures.
 *
 * Contract (used by ClipboardGrabber_Capture_Diagram):
 *   nodes.first()  → Root node
 *   nodes[1..]     → Sub-nodes of that root
 *
 * Scalability:
 *   - Any entry in templates.json with "is_diagram": true is supported.
 *   - Mermaid templates use the placeholder {{CONTENT}}.
 *   - Custom HTML templates may use {{ROOT}} and/or {{SUBNODES}}.
 *   - Adding a new diagram only requires editing templates.json; no C++ change.
 */
class DiagramTemplates {
public:
    static QList<QPair<QString, QString>> list();
    static QString getTemplate(const QString& type);
    static QString buildMarkdown(const QString& type);

    // Builds a diagram from an ordered list of clipboard-captured nodes:
    // nodes.first() is the root, everything after it is a flat child of that
    // root. Used for the live "diagram mode" capture flow.
    static QString buildFromNodes(const QString& type, const QStringList& nodes);
};

#endif // DIAGRAMTEMPLATES_H

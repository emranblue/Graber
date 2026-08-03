#ifndef DIAGRAMTEMPLATES_H
#define DIAGRAMTEMPLATES_H

#include <QString>
#include <QStringList>
#include <QList>
#include <QPair>

class DiagramTemplates {
public:
    static QList<QPair<QString, QString>> list();
    static QString getTemplate(const QString& type);
    static QString buildMarkdown(const QString& type);

    // Builds a diagram from an ordered list of clipboard-captured nodes:
    // nodes.first() is the root, everything after it is a flat child of that
    // root. Used for the live "diagram mode" capture flow, as opposed to
    // getTemplate()/buildMarkdown() which just fill a single-slot skeleton.
    static QString buildFromNodes(const QString& type, const QStringList& nodes);
};

#endif // DIAGRAMTEMPLATES_H

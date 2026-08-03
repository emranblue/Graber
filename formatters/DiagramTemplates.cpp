#include "DiagramTemplates.h"

namespace {
// Mermaid uses (), [], {} as shape delimiters and treats raw newlines/quotes
// as syntax breaks, so clipboard text has to be defanged before it goes
// inside a node label. Swapping to full-width punctuation keeps the text
// readable while it can no longer be mistaken for diagram syntax.
QString sanitizeForMermaid(const QString &text) {
    QString t = text.trimmed();
    t.replace('\n', "<br/>");
    t.replace('"', '\'');
    t.replace('(', QChar(0xFF08)); // （
    t.replace(')', QChar(0xFF09)); // ）
    t.replace('[', QChar(0xFF3B)); // ［
    t.replace(']', QChar(0xFF3D)); // ］
    t.replace('{', QChar(0xFF5B)); // ｛
    t.replace('}', QChar(0xFF5D)); // ｝
    return t;
}
}

QList<QPair<QString, QString>> DiagramTemplates::list() {
    return {
        {"flowchart", "Flowchart"},
        {"sequence",  "Sequence Diagram"},
        {"mindmap",   "Mind Map"},
        {"class",     "Class Diagram"}
    };
}

QString DiagramTemplates::getTemplate(const QString& type) {
    QString t = type.toLower().trimmed();

    if (t == "flowchart" || t == "flow") {
        return "```mermaid\n"
               "flowchart TD\n"
               "    Start([Start]) --> Action[\"{{CONTENT}}\"]\n"
               "    Action --> End([End])\n"
               "```\n";
    }
    else if (t == "sequence") {
        return "```mermaid\n"
               "sequenceDiagram\n"
               "    autonumber\n"
               "    User->>System: {{CONTENT}}\n"
               "    System-->>User: Response\n"
               "```\n";
    }
    else if (t == "mindmap") {
        return "```mermaid\n"
               "mindmap\n"
               "  root((Main Topic))\n"
               "    SubTopic1\n"
               "      {{CONTENT}}\n"
               "```\n";
    }
    else if (t == "class") {
        return "```mermaid\n"
               "classDiagram\n"
               "    class NoteNode {\n"
               "        +String content\n"
               "    }\n"
               "    NoteNode : {{CONTENT}}\n"
               "```\n";
    }

    // Default Fallback Diagram
    return "```mermaid\n"
           "flowchart TD\n"
           "    Node1[\"{{CONTENT}}\"]\n"
           "```\n";
}

QString DiagramTemplates::buildMarkdown(const QString& type) {
    return getTemplate(type);
}

QString DiagramTemplates::buildFromNodes(const QString& type, const QStringList& nodes) {
    if (nodes.isEmpty()) return QString();

    const QString rootText = sanitizeForMermaid(nodes.first());
    QStringList children;
    for (int i = 1; i < nodes.size(); ++i) {
        children << sanitizeForMermaid(nodes.at(i));
    }

    const QString t = type.toLower().trimmed();

    if (t == "mindmap") {
        // Root as a filled circle bubble, every capture after it hangs off
        // the root as its own branch — a flat star, exactly one level deep.
        QString out = "```mermaid\nmindmap\n";
        out += QString("  root((%1))\n").arg(rootText);
        for (const QString &c : children) {
            out += QString("    %1\n").arg(c);
        }
        out += "```\n";
        return out;
    }

    if (t == "class") {
        QString out = "```mermaid\nclassDiagram\n";
        out += "    class RootNode {\n";
        for (int i = 0; i < children.size(); ++i) {
            out += QString("        +%1\n").arg(children.at(i));
        }
        out += "    }\n";
        out += QString("    note for RootNode \"%1\"\n").arg(rootText);
        out += "```\n";
        return out;
    }

    if (t == "sequence") {
        QString out = "```mermaid\nsequenceDiagram\n    autonumber\n";
        out += QString("    participant Root as %1\n").arg(rootText);
        for (int i = 0; i < children.size(); ++i) {
            const QString actorId = QString("N%1").arg(i + 1);
            out += QString("    participant %1 as %2\n").arg(actorId, children.at(i));
        }
        for (int i = 0; i < children.size(); ++i) {
            out += QString("    Root->>N%1: \n").arg(i + 1);
        }
        out += "```\n";
        return out;
    }

    // Default / "flowchart": a star with the root in the middle and every
    // captured item branching straight off it, color-coded so the root
    // stands out from its children at a glance and stays readable as the
    // diagram grows.
    QString out = "```mermaid\nflowchart TD\n";
    out += QString("    root([\"%1\"])\n").arg(rootText);
    QStringList child_ids;
    for (int i = 0; i < children.size(); ++i) {
        const QString nodeId = QString("n%1").arg(i + 1);
        child_ids << nodeId;
        out += QString("    root --> %1[\"%2\"]\n").arg(nodeId, children.at(i));
    }
    out += "    classDef rootStyle fill:#4834d4,stroke:#30336b,stroke-width:2px,color:#ffffff,font-weight:bold;\n";
    out += "    classDef childStyle fill:#eaf6ff,stroke:#22a6b3,stroke-width:1px,color:#130f40;\n";
    out += "    class root rootStyle;\n";
    if (!child_ids.isEmpty()) {
        out += QString("    class %1 childStyle;\n").arg(child_ids.join(","));
    }
    out += "```\n";
    return out;
}

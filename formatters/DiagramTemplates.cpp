#include "DiagramTemplates.h"
#include "MarkdownTemplateManager.h"

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
    QList<QPair<QString, QString>> result;
    const auto diagList = MarkdownTemplateManager::instance().getDiagramFormatList();
    for (const auto &info : diagList) {
        result.append({info.key, info.displayName});
    }
    if (result.isEmpty()) {
        result = {
            {"flowchart", "Flowchart"},
            {"sequence",  "Sequence Diagram"},
            {"mindmap",   "Mind Map"},
            {"class",     "Class Diagram"}
        };
    }
    return result;
}

QString DiagramTemplates::getTemplate(const QString& type) {
    const QString t = type.toLower().trimmed();
    return MarkdownTemplateManager::instance().getTemplate(t);
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
    QString tmpl = MarkdownTemplateManager::instance().getTemplate(t);

    // Clean up legacy inline skeleton patterns if present in user's templates.json on disk
    if (tmpl.contains(QStringLiteral("Start([Start]) --> Action[\"{{CONTENT}}\"]"))) {
        tmpl.replace(QStringLiteral("Start([Start]) --> Action[\"{{CONTENT}}\"]\n    Action --> End([End])"), QStringLiteral("{{CONTENT}}"));
        tmpl.replace(QStringLiteral("Start([Start]) --> Action[\"{{CONTENT}}\"]"), QStringLiteral("{{CONTENT}}"));
    }
    if (tmpl.contains(QStringLiteral("User->>System: {{CONTENT}}"))) {
        tmpl.replace(QStringLiteral("User->>System: {{CONTENT}}\n    System-->>User: Response"), QStringLiteral("{{CONTENT}}"));
        tmpl.replace(QStringLiteral("User->>System: {{CONTENT}}"), QStringLiteral("{{CONTENT}}"));
    }
    if (tmpl.contains(QStringLiteral("NoteNode : {{CONTENT}}"))) {
        tmpl.replace(QStringLiteral("NoteNode : {{CONTENT}}"), QStringLiteral("{{CONTENT}}"));
    }

    if (nodes.size() == 1 && !tmpl.isEmpty()) {
        if (tmpl.contains(QStringLiteral("{content}")) || tmpl.contains(QStringLiteral("{raw_content}"))) {
            QString out = tmpl;
            out.replace(QStringLiteral("{content}"), rootText);
            out.replace(QStringLiteral("{raw_content}"), nodes.first().trimmed());
            out.replace(QStringLiteral("{{CONTENT}}"), rootText);
            return out;
        }
    }

    QString generatedContent;

    if (t == "mindmap" || tmpl.contains(QStringLiteral("mindmap"))) {
        QString mm;
        mm += QString("  root((%1))\n").arg(rootText);
        for (const QString &c : children) {
            mm += QString("    %1\n").arg(c);
        }
        generatedContent = mm;
        if (!tmpl.isEmpty() && tmpl.contains(QStringLiteral("{{CONTENT}}"))) {
            QString out = tmpl;
            out.replace(QStringLiteral("{{CONTENT}}"), generatedContent);
            if (out.contains(QStringLiteral("Main Topic"))) {
                out.replace(QStringLiteral("Main Topic"), rootText);
            }
            return out;
        }
        return QString("```mermaid\nmindmap\n%1```\n").arg(generatedContent);
    }

    if (t == "class" || tmpl.contains(QStringLiteral("classDiagram"))) {
        QString cls;
        cls += "    class RootNode {\n";
        for (const QString &c : children) {
            cls += QString("        +%1\n").arg(c);
        }
        cls += "    }\n";
        cls += QString("    note for RootNode \"%1\"\n").arg(rootText);
        generatedContent = cls;
        if (!tmpl.isEmpty() && tmpl.contains(QStringLiteral("{{CONTENT}}"))) {
            QString out = tmpl;
            out.replace(QStringLiteral("{{CONTENT}}"), generatedContent);
            return out;
        }
        return QString("```mermaid\nclassDiagram\n%1```\n").arg(generatedContent);
    }

    if (t == "sequence" || tmpl.contains(QStringLiteral("sequenceDiagram"))) {
        QString seq;
        seq += QString("    participant Root as %1\n").arg(rootText);
        for (int i = 0; i < children.size(); ++i) {
            seq += QString("    participant N%1 as %2\n").arg(i + 1).arg(children.at(i));
        }
        for (int i = 0; i < children.size(); ++i) {
            seq += QString("    Root->>N%1: %2\n").arg(i + 1).arg(children.at(i));
        }
        generatedContent = seq;
        if (!tmpl.isEmpty() && tmpl.contains(QStringLiteral("{{CONTENT}}"))) {
            QString out = tmpl;
            out.replace(QStringLiteral("{{CONTENT}}"), generatedContent);
            return out;
        }
        return QString("```mermaid\nsequenceDiagram\n    autonumber\n%1```\n").arg(generatedContent);
    }

    // Default: Flowchart / Custom diagram
    QString header = QStringLiteral("flowchart TD");
    if (tmpl.contains(QStringLiteral("flowchart LR"))) header = QStringLiteral("flowchart LR");
    else if (tmpl.contains(QStringLiteral("flowchart RL"))) header = QStringLiteral("flowchart RL");
    else if (tmpl.contains(QStringLiteral("flowchart BT"))) header = QStringLiteral("flowchart BT");
    else if (tmpl.contains(QStringLiteral("graph LR"))) header = QStringLiteral("graph LR");
    else if (tmpl.contains(QStringLiteral("graph TD"))) header = QStringLiteral("graph TD");

    QString node_lines;
    node_lines += QString("    root([\"%1\"])\n").arg(rootText);
    QStringList child_ids;
    for (int i = 0; i < children.size(); ++i) {
        const QString nodeId = QString("n%1").arg(i + 1);
        child_ids << nodeId;
        node_lines += QString("    root --> %1[\"%2\"]\n").arg(nodeId, children.at(i));
    }
    node_lines += "    classDef rootStyle fill:#4834d4,stroke:#30336b,stroke-width:2px,color:#ffffff,font-weight:bold;\n";
    node_lines += "    classDef childStyle fill:#eaf6ff,stroke:#22a6b3,stroke-width:1px,color:#130f40;\n";
    node_lines += "    class root rootStyle;\n";
    if (!child_ids.isEmpty()) {
        node_lines += QString("    class %1 childStyle;\n").arg(child_ids.join(","));
    }
    generatedContent = node_lines;

    if (!tmpl.isEmpty() && tmpl.contains(QStringLiteral("{{CONTENT}}"))) {
        QString out = tmpl;
        if (out.contains(QStringLiteral("<br/>{{CONTENT}}")) || out.contains(QStringLiteral(" : {{CONTENT}}")) ||
            out.contains(QStringLiteral("\"{{CONTENT}}\""))) {
            QString inlineStr = rootText;
            if (!children.isEmpty()) {
                if (out.contains(QStringLiteral("<br/>{{CONTENT}}")))
                    inlineStr += QStringLiteral("<br/>• ") + children.join(QStringLiteral("<br/>• "));
                else if (out.contains(QStringLiteral(" : {{CONTENT}}")))
                    inlineStr += QStringLiteral(" : ") + children.join(QStringLiteral(" : "));
                else
                    inlineStr += QStringLiteral(" - ") + children.join(QStringLiteral(" - "));
            }
            out.replace(QStringLiteral("{{CONTENT}}"), inlineStr);
            return out;
        }
        out.replace(QStringLiteral("{{CONTENT}}"), generatedContent);
        return out;
    }

    if (!tmpl.isEmpty() && (tmpl.contains(QStringLiteral("{content}")) || tmpl.contains(QStringLiteral("{raw_content}")))) {
        QString out = tmpl;
        QString contentStr = rootText;
        for (const QString &c : children) {
            contentStr += "\n" + c;
        }
        out.replace(QStringLiteral("{content}"), contentStr);
        out.replace(QStringLiteral("{raw_content}"), contentStr);
        return out;
    }

    QString out = QString("```mermaid\n%1\n").arg(header);
    out += generatedContent;
    out += "```\n";
    return out;
}

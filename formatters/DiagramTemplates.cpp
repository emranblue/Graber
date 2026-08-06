#include "DiagramTemplates.h"
#include "MarkdownTemplateManager.h"

#include <QRegularExpression>

namespace {

// Mermaid treats (), [], {}, newlines and quotes as syntax.
// Convert them to full-width / safe equivalents so clipboard text stays readable.
QString sanitizeForMermaid(const QString &text) {
    QString t = text.trimmed();
    t.replace('\n', QStringLiteral("<br/>"));
    t.replace('"',  QChar(0x2019)); // ’
    t.replace('(',  QChar(0xFF08)); // （
    t.replace(')',  QChar(0xFF09)); // ）
    t.replace('[',  QChar(0xFF3B)); // ［
    t.replace(']',  QChar(0xFF3D)); // ］
    t.replace('{',  QChar(0xFF5B)); // ｛
    t.replace('}',  QChar(0xFF5D)); // ｝
    return t;
}

// Lightweight sanitization for HTML attribute / text content.
QString sanitizeForHtml(const QString &text) {
    QString t = text.trimmed();
    t.replace('&',  QStringLiteral("&amp;"));
    t.replace('<',  QStringLiteral("&lt;"));
    t.replace('>',  QStringLiteral("&gt;"));
    t.replace('"',  QStringLiteral("&quot;"));
    t.replace('\'', QStringLiteral("&#39;"));
    t.replace('\n', QStringLiteral("<br/>"));
    return t;
}

// Default visual style for auto-generated HTML sub-nodes
// (used when a template declares {{SUBNODES}}).
QString makeHtmlSubNode(const QString &label, int index) {
    // Soft, reading-friendly palette that cycles
    static const char *colors[] = {
        "#5d6d7e", "#566573", "#2c3e50", "#1a5276", "#34495e",
        "#5d6d7e", "#1a5276", "#2c3e50"
    };
    const char *bg = colors[index % 8];

    return QStringLiteral(
        "<div style=\"display:inline-block;background:%1;color:#fff;"
        "padding:8px 16px;border-radius:22px;font-weight:600;font-size:13.5px;"
        "box-shadow:0 2px 6px rgba(0,0,0,0.12);margin:4px 6px;"
        "font-family:'Times New Roman',Times,'Kalpurush',serif;\">%2</div>\n")
        .arg(QString::fromLatin1(bg), label);
}

} // namespace

QList<QPair<QString, QString>> DiagramTemplates::list() {
    QList<QPair<QString, QString>> result;
    const auto diagList = MarkdownTemplateManager::instance().getDiagramFormatList();
    for (const auto &info : diagList) {
        result.append({info.key, info.displayName});
    }
    if (result.isEmpty()) {
        result = {
            {QStringLiteral("flowchart"), QStringLiteral("Flowchart")},
            {QStringLiteral("sequence"),  QStringLiteral("Sequence Diagram")},
            {QStringLiteral("mindmap"),   QStringLiteral("Mind Map")},
            {QStringLiteral("class"),     QStringLiteral("Class Diagram")}
        };
    }
    return result;
}

QString DiagramTemplates::getTemplate(const QString& type) {
    return MarkdownTemplateManager::instance().getTemplate(type.toLower().trimmed());
}

QString DiagramTemplates::buildMarkdown(const QString& type) {
    return getTemplate(type);
}

QString DiagramTemplates::buildFromNodes(const QString& type, const QStringList& nodes) {
    if (nodes.isEmpty())
        return QString();

    // ---------------------------------------------------------------
    // 1. Prepare root + children (first copy = root, rest = sub-nodes)
    // ---------------------------------------------------------------
    const QString rawRoot     = nodes.first().trimmed();
    const QString rootMermaid = sanitizeForMermaid(rawRoot);
    const QString rootHtml    = sanitizeForHtml(rawRoot);

    QStringList childrenMermaid;
    QStringList childrenHtml;
    for (int i = 1; i < nodes.size(); ++i) {
        const QString raw = nodes.at(i).trimmed();
        if (raw.isEmpty()) continue;
        childrenMermaid << sanitizeForMermaid(raw);
        childrenHtml    << sanitizeForHtml(raw);
    }

    const QString t    = type.toLower().trimmed();
    QString       tmpl = MarkdownTemplateManager::instance().getTemplate(t);

    // ---------------------------------------------------------------
    // 2. Custom HTML templates that use {{ROOT}} / {{SUBNODES}}
    //    (fully scalable – any future HTML diagram can use these)
    // ---------------------------------------------------------------
    if (tmpl.contains(QStringLiteral("{{ROOT}}")) ||
        tmpl.contains(QStringLiteral("{{SUBNODES}}"))) {

        QString subNodesHtml;
        for (int i = 0; i < childrenHtml.size(); ++i) {
            subNodesHtml += makeHtmlSubNode(childrenHtml.at(i), i);
        }

        QString out = tmpl;
        out.replace(QStringLiteral("{{ROOT}}"),     rootHtml);
        out.replace(QStringLiteral("{{SUBNODES}}"), subNodesHtml);

        // Also honour classic single-slot placeholders for completeness
        out.replace(QStringLiteral("{{CONTENT}}"),  rootHtml);
        out.replace(QStringLiteral("{content}"),    rootHtml);
        out.replace(QStringLiteral("{raw_content}"), rawRoot);
        return out;
    }

    // ---------------------------------------------------------------
    // 3. Single-node fast path (only root captured so far)
    // ---------------------------------------------------------------
    if (nodes.size() == 1 && !tmpl.isEmpty()) {
        if (tmpl.contains(QStringLiteral("{content}")) ||
            tmpl.contains(QStringLiteral("{raw_content}")) ||
            tmpl.contains(QStringLiteral("{{CONTENT}}"))) {
            QString out = tmpl;
            out.replace(QStringLiteral("{content}"),     rootMermaid);
            out.replace(QStringLiteral("{raw_content}"), rawRoot);
            out.replace(QStringLiteral("{{CONTENT}}"),   rootMermaid);
            return out;
        }
    }

    // ---------------------------------------------------------------
    // 4. Type-specific Mermaid generators
    //    (mindmap / class / sequence / flowchart / timeline)
    // ---------------------------------------------------------------
    QString generated;

    // ---- mindmap --------------------------------------------------
    if (t == QStringLiteral("mindmap") || tmpl.contains(QStringLiteral("mindmap"))) {
        generated  = QStringLiteral("  root((%1))\n").arg(rootMermaid);
        for (const QString &c : childrenMermaid)
            generated += QStringLiteral("    %1\n").arg(c);
    }
    // ---- class diagram --------------------------------------------
    else if (t == QStringLiteral("class") || tmpl.contains(QStringLiteral("classDiagram"))) {
        generated  = QStringLiteral("    class RootNode {\n");
        for (const QString &c : childrenMermaid)
            generated += QStringLiteral("        +%1\n").arg(c);
        generated += QStringLiteral("    }\n");
        generated += QStringLiteral("    note for RootNode \"%1\"\n").arg(rootMermaid);
    }
    // ---- sequence diagram -----------------------------------------
    else if (t == QStringLiteral("sequence") || tmpl.contains(QStringLiteral("sequenceDiagram"))) {
        generated  = QStringLiteral("    participant Root as %1\n").arg(rootMermaid);
        for (int i = 0; i < childrenMermaid.size(); ++i)
            generated += QStringLiteral("    participant N%1 as %2\n")
                             .arg(i + 1).arg(childrenMermaid.at(i));
        for (int i = 0; i < childrenMermaid.size(); ++i)
            generated += QStringLiteral("    Root->>N%1: %2\n")
                             .arg(i + 1).arg(childrenMermaid.at(i));
    }
    // ---- timeline -------------------------------------------------
    else if (t == QStringLiteral("timeline_diagram") ||
             t == QStringLiteral("timeline") ||
             tmpl.contains(QStringLiteral("timeline"))) {
        generated  = QStringLiteral("    title %1\n").arg(rootMermaid);
        for (int i = 0; i < childrenMermaid.size(); ++i)
            generated += QStringLiteral("    %1 : %2\n")
                             .arg(2000 + i * 2)          // synthetic years
                             .arg(childrenMermaid.at(i));
    }
    // ---- default / flowchart / any unknown Mermaid type -----------
    else {
        QString header = QStringLiteral("flowchart TD");
        if (tmpl.contains(QStringLiteral("flowchart LR"))) header = QStringLiteral("flowchart LR");
        else if (tmpl.contains(QStringLiteral("flowchart RL"))) header = QStringLiteral("flowchart RL");
        else if (tmpl.contains(QStringLiteral("flowchart BT"))) header = QStringLiteral("flowchart BT");
        else if (tmpl.contains(QStringLiteral("graph LR")))     header = QStringLiteral("graph LR");
        else if (tmpl.contains(QStringLiteral("graph TD")))     header = QStringLiteral("graph TD");

        generated  = QStringLiteral("    root([\"%1\"])\n").arg(rootMermaid);
        QStringList childIds;
        for (int i = 0; i < childrenMermaid.size(); ++i) {
            const QString id = QStringLiteral("n%1").arg(i + 1);
            childIds << id;
            generated += QStringLiteral("    root --> %1[\"%2\"]\n")
                             .arg(id, childrenMermaid.at(i));
        }
        // Soft, readable styles (no eye-strain colours)
        generated += QStringLiteral(
            "    classDef rootStyle fill:#2c3e50,stroke:#1a252f,stroke-width:2px,color:#f4f6f7,font-weight:bold;\n"
            "    classDef childStyle fill:#eaf2f8,stroke:#5dade2,stroke-width:1px,color:#1a5276;\n"
            "    class root rootStyle;\n");
        if (!childIds.isEmpty())
            generated += QStringLiteral("    class %1 childStyle;\n").arg(childIds.join(QLatin1Char(',')));

        // If the template already contains a header we only inject the body
        if (!tmpl.isEmpty() && tmpl.contains(QStringLiteral("{{CONTENT}}"))) {
            QString out = tmpl;
            out.replace(QStringLiteral("{{CONTENT}}"), generated);
            return out;
        }
        // Otherwise wrap ourselves
        return QStringLiteral("```mermaid\n%1\n%2```\n").arg(header, generated);
    }

    // ---------------------------------------------------------------
    // 5. Inject generated body into the template (or wrap ourselves)
    // ---------------------------------------------------------------
    if (!tmpl.isEmpty() && tmpl.contains(QStringLiteral("{{CONTENT}}"))) {
        QString out = tmpl;
        out.replace(QStringLiteral("{{CONTENT}}"), generated);
        // Clean any leftover single-slot placeholders
        out.replace(QStringLiteral("{content}"),     rootMermaid);
        out.replace(QStringLiteral("{raw_content}"), rawRoot);
        return out;
    }

    // Fallback – produce a complete mermaid block
    if (t == QStringLiteral("mindmap") || tmpl.contains(QStringLiteral("mindmap")))
        return QStringLiteral("```mermaid\nmindmap\n%1```\n").arg(generated);
    if (t == QStringLiteral("class") || tmpl.contains(QStringLiteral("classDiagram")))
        return QStringLiteral("```mermaid\nclassDiagram\n%1```\n").arg(generated);
    if (t == QStringLiteral("sequence") || tmpl.contains(QStringLiteral("sequenceDiagram")))
        return QStringLiteral("```mermaid\nsequenceDiagram\n    autonumber\n%1```\n").arg(generated);
    if (t.contains(QStringLiteral("timeline")))
        return QStringLiteral("```mermaid\ntimeline\n%1```\n").arg(generated);

    return QStringLiteral("```mermaid\nflowchart TD\n%1```\n").arg(generated);
}

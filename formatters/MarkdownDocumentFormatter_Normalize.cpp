#include "MarkdownDocumentFormatter.h"
#include "MarkdownUtils.h"

#include <QRegularExpression>
#include <QStringList>

// ============================================================================
// normalizeContent — strip TOC, rewrite headings to structured HTML
// ============================================================================

QString MarkdownDocumentFormatter::normalizeContent(const QString &content) const {
    QString clean_content = content;
    const int toc_start = content.indexOf(QStringLiteral("<!-- TOC_START -->"));
    const int toc_end = content.indexOf(QStringLiteral("<!-- TOC_END -->"));
    if (toc_start != -1 && toc_end != -1) {
        const QString pre_toc = content.left(toc_start);
        const QString post_toc =
            content.mid(toc_end + QStringLiteral("<!-- TOC_END -->").length());
        clean_content = pre_toc + post_toc;
    }

    const QStringList lines = clean_content.split(QLatin1Char('\n'));
    QStringList output_lines;

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
    static const QRegularExpression h2_regex(
        QStringLiteral("<h2([^>]*)>(.*?)</h2>"),
        QRegularExpression::CaseInsensitiveOption);
    static const QRegularExpression md_regex(
        QStringLiteral("^(#{1,3})\\s+(?!\\*\\*\\*)(.*?)$"));
    static const QRegularExpression md_section_regex(
        QStringLiteral("<!--\\s*section:([\\w-]+)\\s*-->"));
    static const QRegularExpression section_attr_regex(
        QStringLiteral("data-section=\"([^\"]*)\""),
        QRegularExpression::CaseInsensitiveOption);
    static const QRegularExpression style_regex(
        QStringLiteral("style=\"([^\"]*)\""),
        QRegularExpression::CaseInsensitiveOption);

    for (const QString &line : lines) {
        const QString trimmed_line = line.trimmed();

        const QRegularExpressionMatch date_match = date_regex.match(trimmed_line);
        const QRegularExpressionMatch h2_match = h2_regex.match(trimmed_line);
        const QRegularExpressionMatch md_match = md_regex.match(trimmed_line);

        // Drop legacy decorative container divs (keep timeline / bullet markup).
        if (trimmed_line.contains(QLatin1String("<div"))
            && (trimmed_line.contains(QLatin1String("border"))
                || trimmed_line.contains(QLatin1String("background-color")))
            && !trimmed_line.contains(QLatin1String("timeline"))
            && !trimmed_line.contains(QLatin1String("bullet"))) {
            continue;
        }
        if (trimmed_line == QLatin1String("</div>"))
            continue;

        if (date_match.hasMatch()) {
            output_lines.append(line);
            continue;
        }

        if (h2_match.hasMatch()) {
            const QString attributes = h2_match.captured(1);
            const QString title = h2_match.captured(2).trimmed();
            const QString slug = generateSlug(title);

            QString style = QStringLiteral(
                "color: #e74c3c; font-weight: bold; font-style: italic; margin-bottom: 5px;");
            const QRegularExpressionMatch style_match = style_regex.match(attributes);
            if (style_match.hasMatch())
                style = style_match.captured(1);

            // No keyword-guessing: section only if explicitly assigned.
            QString section = QStringLiteral("others");
            const QRegularExpressionMatch section_match = section_attr_regex.match(attributes);
            if (section_match.hasMatch())
                section = section_match.captured(1);

            output_lines.append(
                QStringLiteral("<h2 id=\"%1\" data-section=\"%2\" style=\"%3\">%4</h2>")
                    .arg(slug, section, style, title));
            continue;
        }

        if (md_match.hasMatch()) {
            const int level = md_match.captured(1).length();
            if (level == 3 && trimmed_line.contains(QLatin1String("***"))) {
                output_lines.append(line);
                continue;
            }

            const QString rest = md_match.captured(2).trimmed();
            QString section = QStringLiteral("others");
            QString title = rest;
            const QRegularExpressionMatch section_match = md_section_regex.match(rest);
            if (section_match.hasMatch()) {
                section = section_match.captured(1);
                title = rest.left(section_match.capturedStart()).trimmed();
            }

            const QString slug = generateSlug(title);

            if (level == 2) {
                output_lines.append(
                    QStringLiteral(
                        "<h2 id=\"%1\" data-section=\"%2\" style=\"%3\">%4</h2>")
                        .arg(slug, section,
                             QStringLiteral(
                                 "color: #e74c3c; font-weight: bold; font-style: italic; "
                                 "margin-bottom: 5px;"),
                             title));
            } else if (level == 3) {
                output_lines.append(
                    QStringLiteral("<h3 id=\"%1\" style=\"%2\">%3</h3>")
                        .arg(slug,
                             QStringLiteral(
                                 "color: #2980b9; font-weight: bold; font-style: italic; "
                                 "margin-top: 10px; margin-bottom: 5px;"),
                             title));
            }
            continue;
        }

        output_lines.append(line);
    }

    return output_lines.join(QLatin1Char('\n'));
}

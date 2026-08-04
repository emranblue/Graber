#include "MarkdownDocumentFormatter.h"
#include "MarkdownUtils.h"

#include <QRegularExpression>
#include <QStringList>

// ============================================================================
// TOC generation — hierarchical Markdown TOC with section grouping
// ============================================================================

namespace {

QString plainExcerpt(const QString &raw, int maxChars = 140) {
    QString t = raw;
    t.replace(QRegularExpression(QStringLiteral("<[^>]+>")), QStringLiteral(" "));
    t.replace(QRegularExpression(QStringLiteral("`{1,3}[^`]*`{1,3}")), QStringLiteral(" "));
    t.replace(QRegularExpression(QStringLiteral("\\[([^\\]]+)\\]\\([^)]*\\)")),
              QStringLiteral("\\1"));
    t.replace(QRegularExpression(QStringLiteral("[*_#>`|\\-]{1,}")), QStringLiteral(" "));
    t.replace(QRegularExpression(QStringLiteral("\\s+")), QStringLiteral(" "));
    t = t.trimmed();
    if (t.length() > maxChars)
        t = t.left(maxChars).trimmed() + QStringLiteral("…");
    t.replace(QLatin1Char('\\'), QStringLiteral("\\\\"));
    t.replace(QLatin1Char('"'), QLatin1Char('\''));
    t.replace(QLatin1Char('['), QLatin1Char('('));
    t.replace(QLatin1Char(']'), QLatin1Char(')'));
    return t;
}

QString extractPreview(const QStringList &lines, int startLine) {
    QStringList bits;
    static const QRegularExpression heading_rx(
        QStringLiteral("^(#{1,3}\\s+|<h[23]\\b)"),
        QRegularExpression::CaseInsensitiveOption);
    for (int i = startLine + 1; i < lines.size() && bits.size() < 4; ++i) {
        const QString trimmed = lines[i].trimmed();
        if (trimmed.isEmpty())
            continue;
        if (heading_rx.match(trimmed).hasMatch())
            break;
        if (trimmed.startsWith(QLatin1String("<!--")))
            continue;
        if (trimmed.startsWith(QLatin1String("```")))
            continue;
        bits << trimmed;
    }
    return plainExcerpt(bits.join(QLatin1Char(' ')));
}

} // namespace

QString MarkdownDocumentFormatter::generateToc(const QString &content,
                                               const QList<SectionItem> &sections) const {
    const QString body = updateTocInContent(content, sections);
    const int toc_start = body.indexOf(QStringLiteral("<!-- TOC_START -->"));
    const int toc_end = body.indexOf(QStringLiteral("<!-- TOC_END -->"));
    if (toc_start != -1 && toc_end != -1 && toc_end > toc_start) {
        return body.mid(toc_start,
                        toc_end + QStringLiteral("<!-- TOC_END -->").length() - toc_start);
    }
    return {};
}

QString MarkdownDocumentFormatter::updateTocInContent(const QString &content,
                                                      const QList<SectionItem> &sections) const {
    // Strip any existing TOC blocks.
    QString clean_content = content;
    while (true) {
        const int toc_start = clean_content.indexOf(QStringLiteral("<!-- TOC_START -->"));
        const int toc_end = clean_content.indexOf(QStringLiteral("<!-- TOC_END -->"));
        if (toc_start != -1 && toc_end != -1 && toc_end > toc_start) {
            const QString pre_toc = clean_content.left(toc_start);
            const QString post_toc =
                clean_content.mid(toc_end + QStringLiteral("<!-- TOC_END -->").length());
            clean_content = pre_toc + post_toc;
        } else {
            break;
        }
    }

    const QStringList lines = clean_content.split(QLatin1Char('\n'));

    struct HeadingInfo {
        int index = 0;
        int sub_index = 0;
        QString title;
        QString slug;
        QString date;
        bool is_html = true;
        QString style;
        int level = 2;
        QString section;
        QString parent_slug;
        QString excerpt;
        int line_index = -1;
    };

    QList<HeadingInfo> headings;
    QString current_date;
    int heading_counter = 0;
    int sub_counter = 0;
    QString current_parent_slug;
    int current_parent_index = 0;

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
    static const QRegularExpression h3_regex(
        QStringLiteral("<h3([^>]*)>(.*?)</h3>"),
        QRegularExpression::CaseInsensitiveOption);
    static const QRegularExpression style_regex(
        QStringLiteral("style=\"([^\"]*)\""),
        QRegularExpression::CaseInsensitiveOption);
    static const QRegularExpression section_attr_regex(
        QStringLiteral("data-section=\"([^\"]*)\""),
        QRegularExpression::CaseInsensitiveOption);
    static const QRegularExpression md_regex(QStringLiteral("^(#{2})\\s+(.*?)$"));
    static const QRegularExpression md_sub_regex(
        QStringLiteral("^(#{3})\\s+(?!\\*\\*\\*)(.*?)$"));
    static const QRegularExpression md_section_regex(
        QStringLiteral("<!--\\s*section:([\\w-]+)\\s*-->"));

    QStringList processed_lines;

    auto pushMain = [&](const QString &title, const QString &slug, const QString &section,
                        bool is_html, const QString &style, int lineIdx) {
        heading_counter++;
        sub_counter = 0;
        current_parent_slug = slug;
        current_parent_index = heading_counter;

        HeadingInfo info;
        info.index = heading_counter;
        info.sub_index = 0;
        info.title = title;
        info.slug = slug;
        info.date = current_date;
        info.is_html = is_html;
        info.style = style;
        info.level = 2;
        info.section = section.isEmpty() ? QStringLiteral("others") : section;
        info.parent_slug = QString();
        info.line_index = lineIdx;
        info.excerpt = extractPreview(lines, lineIdx);
        headings.append(info);
    };

    auto pushSub = [&](const QString &title, const QString &slug, bool is_html,
                       const QString &style, int lineIdx) {
        sub_counter++;
        HeadingInfo info;
        info.index = current_parent_index > 0 ? current_parent_index : heading_counter;
        info.sub_index = sub_counter;
        info.title = title;
        info.slug = slug;
        info.date = current_date;
        info.is_html = is_html;
        info.style = style;
        info.level = 3;
        info.section = QString();
        info.parent_slug = current_parent_slug;
        info.line_index = lineIdx;
        info.excerpt = extractPreview(lines, lineIdx);
        headings.append(info);
    };

    for (int i = 0; i < lines.size(); ++i) {
        const QString &line = lines[i];
        const QString trimmed_line = line.trimmed();

        const auto date_match = date_regex.match(trimmed_line);
        const auto h2_match = h2_regex.match(trimmed_line);
        const auto h3_match = h3_regex.match(trimmed_line);
        const auto md_match = md_regex.match(trimmed_line);
        const auto md_sub_match = md_sub_regex.match(trimmed_line);

        if (date_match.hasMatch()) {
            current_date = date_match.captured(1).trimmed();
            processed_lines.append(line);
        } else if (h2_match.hasMatch()) {
            const QString attributes = h2_match.captured(1);
            const QString title = h2_match.captured(2).trimmed();
            const QString slug = generateSlug(title);

            QString style = QStringLiteral(
                "color: #e74c3c; font-weight: bold; font-style: italic; margin-bottom: 5px;");
            const auto style_match = style_regex.match(attributes);
            if (style_match.hasMatch())
                style = style_match.captured(1);

            QString section = QStringLiteral("others");
            const auto section_match = section_attr_regex.match(attributes);
            if (section_match.hasMatch())
                section = section_match.captured(1);
            if (section.trimmed().isEmpty())
                section = QStringLiteral("others");

            pushMain(title, slug, section, true, style, i);
            processed_lines.append(
                QStringLiteral("<h2 id=\"%1\" data-section=\"%2\" style=\"%3\">%4</h2>")
                    .arg(slug, section, style, title));
        } else if (h3_match.hasMatch()) {
            const QString attributes = h3_match.captured(1);
            const QString title = h3_match.captured(2).trimmed();
            const QString slug = generateSlug(title);
            QString style = QStringLiteral(
                "color: #2980b9; font-weight: bold; font-style: italic; "
                "margin-top: 10px; margin-bottom: 5px;");
            const auto style_match = style_regex.match(attributes);
            if (style_match.hasMatch())
                style = style_match.captured(1);

            pushSub(title, slug, true, style, i);
            processed_lines.append(
                QStringLiteral("<h3 id=\"%1\" style=\"%2\">%3</h3>")
                    .arg(slug, style, title));
        } else if (md_match.hasMatch()) {
            const QString rest = md_match.captured(2).trimmed();
            QString section = QStringLiteral("others");
            QString title = rest;
            const auto section_match = md_section_regex.match(rest);
            if (section_match.hasMatch()) {
                section = section_match.captured(1);
                title = rest.left(section_match.capturedStart()).trimmed();
            }
            if (section.trimmed().isEmpty())
                section = QStringLiteral("others");

            const QString slug = generateSlug(title);
            pushMain(title, slug, section, false, QString(), i);
            processed_lines.append(line);
        } else if (md_sub_match.hasMatch()) {
            const QString title = md_sub_match.captured(2).trimmed();
            const QString slug = generateSlug(title);
            pushSub(title, slug, false, QString(), i);
            processed_lines.append(line);
        } else {
            processed_lines.append(line);
        }
    }

    // Plain hierarchical Markdown TOC (no HTML/CSS).
    QString toc_block;
    if (!headings.isEmpty()) {
        toc_block += QStringLiteral("<!-- TOC_START -->\n");
        toc_block += QStringLiteral("## সূচিপত্র (Table of Contents)\n\n");

        QList<SectionItem> effective_sections = sections;
        bool has_others = false;
        for (const auto &sec : effective_sections) {
            if (sec.slug == QLatin1String("others")) {
                has_others = true;
                break;
            }
        }
        if (!has_others)
            effective_sections.append(
                SectionItem{QStringLiteral("অন্যান্য (Others)"), QStringLiteral("others")});

        for (const auto &sec : effective_sections) {
            QList<int> mainIndices;
            for (int i = 0; i < headings.size(); ++i) {
                if (headings[i].level == 2 && headings[i].section == sec.slug)
                    mainIndices.append(i);
            }
            if (mainIndices.isEmpty())
                continue;

            toc_block += QStringLiteral("### %1\n\n").arg(sec.displayName);

            for (int mi : mainIndices) {
                const HeadingInfo &h = headings[mi];
                const QString datePart =
                    h.date.isEmpty() ? QString() : QStringLiteral(" · %1").arg(h.date);
                const QString preview = h.excerpt.isEmpty() ? h.title : h.excerpt;
                toc_block += QStringLiteral("%1. [%2](#%3 \"%4\") —%5 · id: %1\n")
                                 .arg(QString::number(h.index), h.title, h.slug, preview,
                                      datePart);

                for (int j = mi + 1; j < headings.size(); ++j) {
                    const HeadingInfo &s = headings[j];
                    if (s.level == 2)
                        break;
                    if (s.level != 3 || s.parent_slug != h.slug)
                        continue;

                    const QString subNum =
                        QStringLiteral("%1.%2").arg(h.index).arg(s.sub_index);
                    const QString subDate =
                        s.date.isEmpty() ? QString() : QStringLiteral(" · %1").arg(s.date);
                    const QString subPreview = s.excerpt.isEmpty() ? s.title : s.excerpt;
                    toc_block += QStringLiteral("    %1 [%2](#%3 \"%4\") —%5 · id: %1\n")
                                     .arg(subNum, s.title, s.slug, subPreview, subDate);
                }
            }
            toc_block += QLatin1Char('\n');
        }

        toc_block += QStringLiteral("<!-- TOC_END -->\n\n");
    }

    return toc_block + processed_lines.join(QLatin1Char('\n'));
}

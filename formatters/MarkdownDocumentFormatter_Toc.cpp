#include "MarkdownDocumentFormatter.h"
#include "MarkdownUtils.h"
#include "MarkdownDocumentFormatter_TocHelpers.h"
#include "MarkdownTemplateManager.h"

#include <QRegularExpression>
#include <QStringList>

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

static QString cleanHeadingTitle(const QString &raw) {
    QString t = raw;
    t.replace(QRegularExpression(QStringLiteral("<[^>]+>")), QString());
    t.replace(QRegularExpression(QStringLiteral("^[*_]{1,3}|[*_]{1,3}$")), QString());
    return t.trimmed();
}

QString MarkdownDocumentFormatter::updateTocInContent(const QString &content,
                                                      const QList<SectionItem> &sections) const {
    QString clean_content = MarkdownTocHelpers::stripExistingToc(content);

    const QStringList lines = clean_content.split(QLatin1Char('\n'));

    using MarkdownTocHelpers::HeadingInfo;
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
        QStringLiteral("^(#{3})\\s+(.*?)$"));
    static const QRegularExpression md_section_regex(
        QStringLiteral("<!--\\s*section:([\\w-]+)\\s*-->"));

    QStringList processed_lines;

    auto pushMain = [&](const QString &rawTitle, const QString &slug, const QString &section,
                        bool is_html, const QString &style, int lineIdx) {
        heading_counter++;
        sub_counter = 0;
        current_parent_slug = slug;
        current_parent_index = heading_counter;

        const QString displayTitle = cleanHeadingTitle(rawTitle);

        HeadingInfo info;
        info.index = heading_counter;
        info.sub_index = 0;
        info.title = displayTitle;
        info.slug = slug.isEmpty() ? generateSlug(displayTitle) : slug;
        info.date = current_date;
        info.is_html = is_html;
        info.style = style;
        info.level = 2;
        info.section = section.isEmpty() ? QStringLiteral("others") : section;
        info.parent_slug = QString();
        info.line_index = lineIdx;
        info.excerpt = MarkdownTocHelpers::extractPreview(lines, lineIdx);
        headings.append(info);
    };

    auto pushSub = [&](const QString &rawTitle, const QString &slug, bool is_html,
                       const QString &style, int lineIdx) {
        sub_counter++;
        const QString displayTitle = cleanHeadingTitle(rawTitle);

        HeadingInfo info;
        info.index = current_parent_index > 0 ? current_parent_index : heading_counter;
        info.sub_index = sub_counter;
        info.title = displayTitle;
        info.slug = slug.isEmpty() ? generateSlug(displayTitle) : slug;
        info.date = current_date;
        info.is_html = is_html;
        info.style = style;
        info.level = 3;
        info.section = QString();
        info.parent_slug = current_parent_slug;
        info.line_index = lineIdx;
        info.excerpt = MarkdownTocHelpers::extractPreview(lines, lineIdx);
        headings.append(info);
    };

    static const QRegularExpression div_h_regex(
        QStringLiteral("<div\\b[^>]*\\bid=[\"']([^\"']+)[\"'][^>]*>(.*?)</div>"),
        QRegularExpression::CaseInsensitiveOption);

    for (int i = 0; i < lines.size(); ++i) {
        const QString &line = lines[i];
        const QString trimmed_line = line.trimmed();

        const auto date_match = date_regex.match(trimmed_line);
        const auto h2_match = h2_regex.match(trimmed_line);
        const auto h3_match = h3_regex.match(trimmed_line);
        const auto md_match = md_regex.match(trimmed_line);
        const auto md_sub_match = md_sub_regex.match(trimmed_line);
        const auto div_h_match = (!line.contains(QStringLiteral("timeline-item")) && !line.contains(QStringLiteral("paragraph-item")))
            ? div_h_regex.match(trimmed_line) : QRegularExpressionMatch();

        if (date_match.hasMatch()) {
            current_date = date_match.captured(1).trimmed();
            processed_lines.append(line);
        } else if (h2_match.hasMatch()) {
            const QString attributes = h2_match.captured(1);
            const QString rawTitle = h2_match.captured(2).trimmed();
            const QString cleanTitle = cleanHeadingTitle(rawTitle);
            const QString slug = generateSlug(cleanTitle);

            QString section = QStringLiteral("others");
            const auto section_match = section_attr_regex.match(attributes);
            if (section_match.hasMatch())
                section = section_match.captured(1);
            if (section.trimmed().isEmpty())
                section = QStringLiteral("others");

            pushMain(cleanTitle, slug, section, true, QString(), i);
            const QString formatted = MarkdownTemplateManager::instance().formatByKey(QStringLiteral("heading"), cleanTitle, section);
            processed_lines.append(formatted.trimmed());
        } else if (h3_match.hasMatch()) {
            const QString rawTitle = h3_match.captured(2).trimmed();
            const QString cleanTitle = cleanHeadingTitle(rawTitle);
            const QString slug = generateSlug(cleanTitle);

            pushSub(cleanTitle, slug, true, QString(), i);
            const QString formatted = MarkdownTemplateManager::instance().formatByKey(QStringLiteral("subheading"), cleanTitle);
            processed_lines.append(formatted.trimmed());
        } else if (div_h_match.hasMatch()) {
            const QString tag_attributes = div_h_match.captured(1);
            const QString rawTitle = div_h_match.captured(2).trimmed();
            const QString cleanTitle = cleanHeadingTitle(rawTitle);
            if (cleanTitle.isEmpty()) {
                processed_lines.append(line);
                continue;
            }

            QString section = QStringLiteral("others");
            const auto section_match = section_attr_regex.match(tag_attributes);
            const bool is_main = section_match.hasMatch();

            if (is_main) {
                section = section_match.captured(1);
                if (section.trimmed().isEmpty())
                    section = QStringLiteral("others");

                const QString slug = generateSlug(cleanTitle);
                pushMain(cleanTitle, slug, section, true, QString(), i);
                const QString formatted = MarkdownTemplateManager::instance().formatByKey(QStringLiteral("heading"), cleanTitle, section);
                processed_lines.append(formatted.trimmed());
            } else {
                const QString slug = generateSlug(cleanTitle);
                pushSub(cleanTitle, slug, true, QString(), i);
                const QString formatted = MarkdownTemplateManager::instance().formatByKey(QStringLiteral("subheading"), cleanTitle);
                processed_lines.append(formatted.trimmed());
            }
        } else if (md_match.hasMatch()) {
            const QString rest = md_match.captured(2).trimmed();
            QString section = QStringLiteral("others");
            QString rawTitle = rest;
            const auto section_match = md_section_regex.match(rest);
            if (section_match.hasMatch()) {
                section = section_match.captured(1);
                rawTitle = rest.left(section_match.capturedStart()).trimmed();
            }
            if (section.trimmed().isEmpty())
                section = QStringLiteral("others");

            const QString cleanTitle = cleanHeadingTitle(rawTitle);
            const QString slug = generateSlug(cleanTitle);
            pushMain(cleanTitle, slug, section, false, QString(), i);
            const QString formatted = MarkdownTemplateManager::instance().formatByKey(QStringLiteral("heading"), cleanTitle, section);
            processed_lines.append(formatted.trimmed());
        } else if (md_sub_match.hasMatch()) {
            const QString rawTitle = md_sub_match.captured(2).trimmed();
            const QString cleanTitle = cleanHeadingTitle(rawTitle);
            if (!cleanTitle.isEmpty()) {
                const QString slug = generateSlug(cleanTitle);
                pushSub(cleanTitle, slug, false, QString(), i);
                const QString formatted = MarkdownTemplateManager::instance().formatByKey(QStringLiteral("subheading"), cleanTitle);
                processed_lines.append(formatted.trimmed());
            } else {
                processed_lines.append(line);
            }
        } else if (trimmed_line.startsWith(QLatin1String("<div style=\"text-align: center; margin: 18px 0 14px 0;\">")) ||
                   (trimmed_line == QLatin1String("</div>") && !processed_lines.isEmpty() && processed_lines.last().contains(QLatin1String("text-align: center")))) {
            // Skip redundant outer wrapper container divs
            continue;
        } else {
            processed_lines.append(line);
        }
    }

    const QString toc_block = MarkdownTocHelpers::buildTocBlock(headings, sections);
    return toc_block + processed_lines.join(QLatin1Char('\n'));
}


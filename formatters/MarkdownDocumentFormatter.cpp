#include "MarkdownDocumentFormatter.h"
#include "MarkdownUtils.h"
#include "DiagramTemplates.h"
#include <QRegularExpression>
#include <QTextStream>
#include <QFile>

// ============================================================================
// DIAGRAM FORMATTING & INJECTION
// ============================================================================

QString MarkdownDocumentFormatter::formatDiagram(const QString &content, const QString &diagramType) const {
    if (content.trimmed().isEmpty()) {
        return QString();
    }

    // 1. Clean and sanitize clipboard text so line breaks and quotes don't break Mermaid diagram syntax
    QString sanitizedContent = content.trimmed();
    sanitizedContent.replace("\"", "'");       // Replace double quotes with single quotes
    sanitizedContent.replace("\n", "<br/>"); // Convert line breaks into HTML breaks for diagram nodes

    // 2. Fetch selected diagram template
    QString tmpl = DiagramTemplates::getTemplate(diagramType);

    // 3. Inject the clipboard text directly inside {{CONTENT}}
    if (tmpl.contains("{{CONTENT}}")) {
        return tmpl.replace("{{CONTENT}}", sanitizedContent);
    }

    // Backup fallback if placeholder is missing
    return QString("```mermaid\nflowchart TD\n    Node1[\"%1\"]\n```\n").arg(sanitizedContent);
}

// ============================================================================
// SLUG & SECTION UTILITIES
// ============================================================================

QString MarkdownDocumentFormatter::generateSlug(const QString &text) const {
    return QString::fromStdString(MarkdownUtils::generate_slug(text));
}

QString MarkdownDocumentFormatter::detectSectionFromTitle(const QString &title) const {
    return MarkdownUtils::detect_section_from_title(title);
}

// ============================================================================
// MARKDOWN NORMALIZATION & HEADING PROCESSING
// ============================================================================

QString MarkdownDocumentFormatter::normalizeContent(const QString &content) const {
    int toc_start = content.indexOf("<!-- TOC_START -->");
    int toc_end = content.indexOf("<!-- TOC_END -->");
    QString clean_content = content;
    if (toc_start != -1 && toc_end != -1) {
        QString pre_toc = content.left(toc_start);
        QString post_toc = content.mid(toc_end + QString("<!-- TOC_END -->").length());
        clean_content = pre_toc + post_toc;
    }

    QStringList lines = clean_content.split('\n');
    QStringList output_lines;

    QRegularExpression date_regex("^###\\s*(?:\\*\\*\\*)?\\s*([0-9০-৯]{1,2}\\s+(?:January|February|March|April|May|June|July|August|September|October|November|December|জানুয়ারি|ফেব্রুয়ারি|মার্চ|এপ্রিল|মে|জুন|জুলাই|আগস্ট|সেপ্টেম্বর|অক্টোবর|নভেম্বর|ডিসেম্বর)[,\\s]+[0-9০-৯]{4})\\s*(?:\\*\\*\\*)?$", QRegularExpression::CaseInsensitiveOption);
    QRegularExpression h2_regex("<h2([^>]*)>(.*?)</h2>", QRegularExpression::CaseInsensitiveOption);
    QRegularExpression md_regex("^(#{1,3})\\s+(?!\\*\\*\\*)(.*?)$");
    QRegularExpression md_section_regex("<!--\\s*section:([\\w-]+)\\s*-->");
    QRegularExpression section_attr_regex("data-section=\"([^\"]*)\"", QRegularExpression::CaseInsensitiveOption);
    QRegularExpression style_regex("style=\"([^\"]*)\"", QRegularExpression::CaseInsensitiveOption);

    for (int i = 0; i < lines.size(); ++i) {
        QString line = lines[i];
        QString trimmed_line = line.trimmed();

        QRegularExpressionMatch date_match = date_regex.match(trimmed_line);
        QRegularExpressionMatch h2_match = h2_regex.match(trimmed_line);
        QRegularExpressionMatch md_match = md_regex.match(trimmed_line);

        if (trimmed_line.contains("<div") && (trimmed_line.contains("border") || trimmed_line.contains("background-color")) && !trimmed_line.contains("timeline") && !trimmed_line.contains("bullet")) {
            continue;
        } else if (trimmed_line == "</div>") {
            continue;
        } else if (date_match.hasMatch()) {
            output_lines.append(line);
        } else if (h2_match.hasMatch()) {
            QString attributes = h2_match.captured(1);
            QString title = h2_match.captured(2).trimmed();
            QString slug = generateSlug(title);

            QString style = "color: #e74c3c; font-weight: bold; font-style: italic; margin-bottom: 5px;";
            QRegularExpressionMatch style_match = style_regex.match(attributes);
            if (style_match.hasMatch()) {
                style = style_match.captured(1);
            }

            QString section = detectSectionFromTitle(title);
            QRegularExpressionMatch section_match = section_attr_regex.match(attributes);
            if (section_match.hasMatch()) {
                section = section_match.captured(1);
            }

            QString rewritten_line = QString("<h2 id=\"%1\" data-section=\"%2\" style=\"%3\">%4</h2>")
                                     .arg(slug, section, style, title);
            output_lines.append(rewritten_line);
        } else if (md_match.hasMatch()) {
            int level = md_match.captured(1).length();
            if (level == 3 && trimmed_line.contains("***")) {
                output_lines.append(line);
                continue;
            }

            QString rest = md_match.captured(2).trimmed();
            QString section = detectSectionFromTitle(rest);
            QRegularExpressionMatch section_match = md_section_regex.match(rest);
            QString title = rest;
            if (section_match.hasMatch()) {
                section = section_match.captured(1);
                title = rest.left(section_match.capturedStart()).trimmed();
            }

            QString slug = generateSlug(title);

            if (level == 2) {
                QString style = "color: #e74c3c; font-weight: bold; font-style: italic; margin-bottom: 5px;";
                QString html_heading = QString("<h2 id=\"%1\" data-section=\"%2\" style=\"%3\">%4</h2>")
                                       .arg(slug, section, style, title);
                output_lines.append(html_heading);
            } else if (level == 3) {
                QString style = "color: #2980b9; font-weight: bold; font-style: italic; margin-top: 10px; margin-bottom: 5px;";
                QString html_subheading = QString("<h3 id=\"%1\" style=\"%2\">%3</h3>")
                                          .arg(slug, style, title);
                output_lines.append(html_subheading);
            }
        } else {
            output_lines.append(line);
        }
    }

    return output_lines.join('\n');
}

// ============================================================================
// TOC GENERATION & UPDATES
// ============================================================================

QString MarkdownDocumentFormatter::generateToc(const QString &content, const QList<SectionItem> &sections) const {
    // generateToc is a thin wrapper; the real work lives in updateTocInContent.
    QString body = updateTocInContent(content, sections);
    int toc_start = body.indexOf("<!-- TOC_START -->");
    int toc_end = body.indexOf("<!-- TOC_END -->");
    if (toc_start != -1 && toc_end != -1 && toc_end > toc_start) {
        return body.mid(toc_start, toc_end + QString("<!-- TOC_END -->").length() - toc_start);
    }
    return QString();
}

namespace {

// Strip HTML tags / markdown noise and collapse whitespace for hover previews.
QString plainExcerpt(const QString &raw, int maxChars = 140) {
    QString t = raw;
    t.replace(QRegularExpression("<[^>]+>"), " ");
    t.replace(QRegularExpression("`{1,3}[^`]*`{1,3}"), " ");
    t.replace(QRegularExpression("\\[([^\\]]+)\\]\\([^)]*\\)"), "\\1");
    t.replace(QRegularExpression("[*_#>`|\\-]{1,}"), " ");
    t.replace(QRegularExpression("\\s+"), " ");
    t = t.trimmed();
    if (t.length() > maxChars) {
        t = t.left(maxChars).trimmed() + "…";
    }
    // Escape characters that would break a Markdown link title ("...") or HTML title.
    t.replace('\\', "\\\\");
    t.replace('"', "'");
    t.replace('[', "(");
    t.replace(']', ")");
    return t;
}

// Collect a short preview from lines after `startLine` until the next heading.
QString extractPreview(const QStringList &lines, int startLine) {
    QStringList bits;
    static const QRegularExpression heading_rx(
        "^(#{1,3}\\s+|<h[23]\\b)", QRegularExpression::CaseInsensitiveOption);
    for (int i = startLine + 1; i < lines.size() && bits.size() < 4; ++i) {
        QString trimmed = lines[i].trimmed();
        if (trimmed.isEmpty()) continue;
        if (heading_rx.match(trimmed).hasMatch()) break;
        if (trimmed.startsWith("<!--")) continue;
        if (trimmed.startsWith("```")) continue;
        bits << trimmed;
    }
    return plainExcerpt(bits.join(" "));
}

} // namespace

QString MarkdownDocumentFormatter::updateTocInContent(const QString &content, const QList<SectionItem> &sections) const {
    QString clean_content = content;
    while (true) {
        int toc_start = clean_content.indexOf("<!-- TOC_START -->");
        int toc_end = clean_content.indexOf("<!-- TOC_END -->");
        if (toc_start != -1 && toc_end != -1 && toc_end > toc_start) {
            QString pre_toc = clean_content.left(toc_start);
            QString post_toc = clean_content.mid(toc_end + QString("<!-- TOC_END -->").length());
            clean_content = pre_toc + post_toc;
        } else {
            break;
        }
    }

    QStringList lines = clean_content.split('\n');

    // Hierarchical heading node used for thesis-style TOC.
    struct HeadingInfo {
        int index = 0;          // main heading number (1, 2, 3…)
        int sub_index = 0;      // 0 = main heading; 1+ = subheading under parent
        QString title;
        QString slug;
        QString date;
        bool is_html = true;
        QString style;
        int level = 2;          // 2 = h2, 3 = h3
        QString section;
        QString parent_slug;
        QString excerpt;        // short body preview for hover title
        int line_index = -1;
    };

    QList<HeadingInfo> headings;
    QString current_date = "";
    int heading_counter = 0;
    int sub_counter = 0;
    QString current_parent_slug;
    int current_parent_index = 0;

    QRegularExpression date_regex("^###\\s*(?:\\*\\*\\*)?\\s*([0-9০-৯]{1,2}\\s+(?:January|February|March|April|May|June|July|August|September|October|November|December|জানুয়ারি|ফেব্রুয়ারি|মার্চ|এপ্রিল|মে|জুন|জুলাই|আগস্ট|সেপ্টেম্বর|অক্টোবর|নভেম্বর|ডিসেম্বর)[,\\s]+[0-9০-৯]{4})\\s*(?:\\*\\*\\*)?$", QRegularExpression::CaseInsensitiveOption);
    QRegularExpression h2_regex("<h2([^>]*)>(.*?)</h2>", QRegularExpression::CaseInsensitiveOption);
    QRegularExpression h3_regex("<h3([^>]*)>(.*?)</h3>", QRegularExpression::CaseInsensitiveOption);
    QRegularExpression style_regex("style=\"([^\"]*)\"", QRegularExpression::CaseInsensitiveOption);
    QRegularExpression section_attr_regex("data-section=\"([^\"]*)\"", QRegularExpression::CaseInsensitiveOption);
    QRegularExpression md_regex("^(#{2})\\s+(.*?)$");
    QRegularExpression md_sub_regex("^(#{3})\\s+(?!\\*\\*\\*)(.*?)$");
    QRegularExpression md_section_regex("<!--\\s*section:([\\w-]+)\\s*-->");

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
        info.section = QString(); // inherits parent section for TOC grouping
        info.parent_slug = current_parent_slug;
        info.line_index = lineIdx;
        info.excerpt = extractPreview(lines, lineIdx);
        headings.append(info);
    };

    for (int i = 0; i < lines.size(); ++i) {
        QString line = lines[i];
        QString trimmed_line = line.trimmed();

        QRegularExpressionMatch date_match = date_regex.match(trimmed_line);
        QRegularExpressionMatch h2_match = h2_regex.match(trimmed_line);
        QRegularExpressionMatch h3_match = h3_regex.match(trimmed_line);
        QRegularExpressionMatch md_match = md_regex.match(trimmed_line);
        QRegularExpressionMatch md_sub_match = md_sub_regex.match(trimmed_line);

        if (date_match.hasMatch()) {
            current_date = date_match.captured(1).trimmed();
            processed_lines.append(line);
        } else if (h2_match.hasMatch()) {
            QString attributes = h2_match.captured(1);
            QString title = h2_match.captured(2).trimmed();
            QString slug = generateSlug(title);

            QString style = "color: #e74c3c; font-weight: bold; font-style: italic; margin-bottom: 5px;";
            QRegularExpressionMatch style_match = style_regex.match(attributes);
            if (style_match.hasMatch()) {
                style = style_match.captured(1);
            }

            QString section = "others";
            QRegularExpressionMatch section_match = section_attr_regex.match(attributes);
            if (section_match.hasMatch()) {
                section = section_match.captured(1);
            }
            if (section.trimmed().isEmpty()) {
                section = "others";
            }

            pushMain(title, slug, section, true, style, i);

            QString rewritten_line = QString("<h2 id=\"%1\" data-section=\"%2\" style=\"%3\">%4</h2>")
                                     .arg(slug, section, style, title);
            processed_lines.append(rewritten_line);
        } else if (h3_match.hasMatch()) {
            QString attributes = h3_match.captured(1);
            QString title = h3_match.captured(2).trimmed();
            QString slug = generateSlug(title);
            QString style = "color: #2980b9; font-weight: bold; font-style: italic; margin-top: 10px; margin-bottom: 5px;";
            QRegularExpressionMatch style_match = style_regex.match(attributes);
            if (style_match.hasMatch()) {
                style = style_match.captured(1);
            }

            pushSub(title, slug, true, style, i);

            QString rewritten_line = QString("<h3 id=\"%1\" style=\"%2\">%3</h3>")
                                     .arg(slug, style, title);
            processed_lines.append(rewritten_line);
        } else if (md_match.hasMatch()) {
            QString rest = md_match.captured(2).trimmed();

            QString section = "others";
            QRegularExpressionMatch section_match = md_section_regex.match(rest);
            QString title = rest;
            if (section_match.hasMatch()) {
                section = section_match.captured(1);
                title = rest.left(section_match.capturedStart()).trimmed();
            }
            if (section.trimmed().isEmpty()) {
                section = "others";
            }

            QString slug = generateSlug(title);
            pushMain(title, slug, section, false, QString(), i);
            processed_lines.append(line);
        } else if (md_sub_match.hasMatch()) {
            QString title = md_sub_match.captured(2).trimmed();
            QString slug = generateSlug(title);
            pushSub(title, slug, false, QString(), i);
            processed_lines.append(line);
        } else {
            processed_lines.append(line);
        }
    }

    // --- Plain hierarchical Markdown TOC (no HTML, no CSS, no colors) ---
    QString toc_block;
    if (!headings.isEmpty()) {
        toc_block += "<!-- TOC_START -->\n";
        toc_block += "## সূচিপত্র (Table of Contents)\n\n";

        QList<SectionItem> effective_sections = sections;
        if (effective_sections.isEmpty()) {
            effective_sections = MarkdownUtils::get_default_sections();
        }
        bool has_others = false;
        for (const auto &sec : effective_sections) {
            if (sec.slug == "others") { has_others = true; break; }
        }
        if (!has_others) {
            effective_sections.append(SectionItem{"অন্যান্য (Others)", "others"});
        }

        for (const auto &sec : effective_sections) {
            QList<int> mainIndices;
            for (int i = 0; i < headings.size(); ++i) {
                if (headings[i].level == 2 && headings[i].section == sec.slug) {
                    mainIndices.append(i);
                }
            }
            if (mainIndices.isEmpty()) continue;

            toc_block += QString("### %1\n\n").arg(sec.displayName);

            for (int mi : mainIndices) {
                const HeadingInfo &h = headings[mi];

                // 1. [Title](#slug "preview") — date · id: 1
                // Anchor stays text slug; displayed id is numeric.
                QString datePart = h.date.isEmpty() ? QString() : QString(" · %1").arg(h.date);
                QString preview = h.excerpt.isEmpty() ? h.title : h.excerpt;
                toc_block += QString("%1. [%2](#%3 \"%4\") —%5 · id: %1\n")
                                 .arg(QString::number(h.index), h.title, h.slug, preview, datePart);

                // Nested:    1.1 [Sub](#slug "preview") — date · id: 1.1
                for (int j = mi + 1; j < headings.size(); ++j) {
                    const HeadingInfo &s = headings[j];
                    if (s.level == 2) break;
                    if (s.level != 3 || s.parent_slug != h.slug) continue;

                    QString subNum = QString("%1.%2").arg(h.index).arg(s.sub_index);
                    QString subDate = s.date.isEmpty() ? QString() : QString(" · %1").arg(s.date);
                    QString subPreview = s.excerpt.isEmpty() ? s.title : s.excerpt;
                    toc_block += QString("    %1 [%2](#%3 \"%4\") —%5 · id: %1\n")
                                     .arg(subNum, s.title, s.slug, subPreview, subDate);
                }
            }
            toc_block += "\n";
        }

        toc_block += "<!-- TOC_END -->\n\n";
    }

    return toc_block + processed_lines.join('\n');
}

// ============================================================================
// NOTE STRUCTURE PARSING & TREE STORAGE
// ============================================================================

QList<NoteItem> MarkdownDocumentFormatter::parseNoteStructure(const QString &content, const QList<SectionItem> &availableSections, QSet<QString> &outCustomSections) const {
    QList<NoteItem> items;
    QStringList lines = content.split('\n');

    QRegularExpression h2_regex("<h2([^>]*)>(.*?)</h2>", QRegularExpression::CaseInsensitiveOption);
    QRegularExpression h3_regex("<h3([^>]*)>(.*?)</h3>", QRegularExpression::CaseInsensitiveOption);
    QRegularExpression section_attr_regex("data-section=\"([^\"]*)\"", QRegularExpression::CaseInsensitiveOption);
    QRegularExpression md_regex("^(#{2})\\s+(.*?)$");
    QRegularExpression md_sub_regex("^(#{3})\\s+(?!\\*\\*\\*)(.*?)$");
    QRegularExpression md_section_regex("<!--\\s*section:([\\w-]+)\\s*-->");

    QString current_parent_slug = "";

    QSet<QString> known_slugs;
    for (const auto &sec : availableSections) {
        known_slugs.insert(sec.slug);
    }

    for (int i = 0; i < lines.size(); ++i) {
        QString line = lines[i].trimmed();
        QRegularExpressionMatch h2_match = h2_regex.match(line);
        QRegularExpressionMatch h3_match = h3_regex.match(line);
        QRegularExpressionMatch md_match = md_regex.match(line);
        QRegularExpressionMatch md_sub_match = md_sub_regex.match(line);

        if (h2_match.hasMatch()) {
            QString attributes = h2_match.captured(1);
            QString title = h2_match.captured(2).trimmed();
            QString slug = generateSlug(title);

            QString section = "others";
            QRegularExpressionMatch section_match = section_attr_regex.match(attributes);
            if (section_match.hasMatch()) {
                section = section_match.captured(1);
            }

            if (!known_slugs.contains(section) && section != "others") {
                outCustomSections.insert(section);
            }

            NoteItem item;
            item.title = title;
            item.slug = slug;
            item.type = "heading";
            item.section = section;
            item.parent_slug = "";

            items.append(item);
            current_parent_slug = slug;
        } else if (h3_match.hasMatch()) {
            QString title = h3_match.captured(2).trimmed();
            QString slug = generateSlug(title);

            NoteItem item;
            item.title = title;
            item.slug = slug;
            item.type = "subheading";
            item.section = "";
            item.parent_slug = current_parent_slug;

            items.append(item);
        } else if (md_match.hasMatch()) {
            QString rest = md_match.captured(2).trimmed();
            QString section = "others";
            QRegularExpressionMatch section_match = md_section_regex.match(rest);
            QString title = rest;
            if (section_match.hasMatch()) {
                section = section_match.captured(1);
                title = rest.left(section_match.capturedStart()).trimmed();
            }

            if (!known_slugs.contains(section) && section != "others") {
                outCustomSections.insert(section);
            }

            QString slug = generateSlug(title);

            NoteItem item;
            item.title = title;
            item.slug = slug;
            item.type = "heading";
            item.section = section;
            item.parent_slug = "";

            items.append(item);
            current_parent_slug = slug;
        } else if (md_sub_match.hasMatch()) {
            QString title = md_sub_match.captured(2).trimmed();
            QString slug = generateSlug(title);

            NoteItem item;
            item.title = title;
            item.slug = slug;
            item.type = "subheading";
            item.section = "";
            item.parent_slug = current_parent_slug;

            items.append(item);
        }
    }

    return items;
}

void MarkdownDocumentFormatter::saveStructureTree(const QString &treeFilePath, const QList<NoteItem> &items) const {
    MarkdownUtils::save_tree_file(treeFilePath, items);
}

QString MarkdownDocumentFormatter::restoreStateFromContent(const QString &content) const {
    QTextStream in(const_cast<QString*>(&content));
    QRegularExpression date_regex("^###\\s*(?:\\*\\*\\*)?\\s*([0-9০-৯]{1,2}\\s+(?:January|February|March|April|May|June|July|August|September|October|November|December|জানুয়ারি|ফেব্রুয়ারি|মার্চ|এপ্রিল|মে|জুন|জুলাই|আগস্ট|সেপ্টেম্বর|অক্টোবর|নভেম্বর|ডিসেম্বর)[,\\s]+[0-9০-৯]{4})\\s*(?:\\*\\*\\*)?$", QRegularExpression::CaseInsensitiveOption);

    QString last_found_date = "";
    while (!in.atEnd()) {
        QString line = in.readLine().trimmed();
        QRegularExpressionMatch match = date_regex.match(line);
        if (match.hasMatch()) {
            last_found_date = match.captured(1).trimmed();
        }
    }
    return last_found_date;
}

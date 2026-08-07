#include "MarkdownDocumentFormatter_TocHelpers.h"
#include <QRegularExpression>

namespace MarkdownTocHelpers {

QString stripExistingToc(const QString &content) {
    QString clean = content;
    while (true) {
        const int toc_start = clean.indexOf(QStringLiteral("<!-- TOC_START -->"));
        const int toc_end = clean.indexOf(QStringLiteral("<!-- TOC_END -->"));
        if (toc_start != -1 && toc_end != -1 && toc_end > toc_start) {
            const int end_len = QStringLiteral("<!-- TOC_END -->").length();
            int end_pos = toc_end + end_len;
            while (end_pos < clean.length() && (clean[end_pos] == QLatin1Char('\r') || clean[end_pos] == QLatin1Char('\n'))) {
                end_pos++;
            }
            clean = clean.left(toc_start) + clean.mid(end_pos);
        } else {
            break;
        }
    }
    return clean;
}


QString plainExcerpt(const QString &raw, int maxChars) {
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

QString buildTocBlock(const QList<HeadingInfo> &headings,
                      const QList<SectionItem> &sections) {
    if (headings.isEmpty())
        return {};

    QString toc_block;
    toc_block += QStringLiteral("<!-- TOC_START -->\n");
    toc_block += QStringLiteral(
        "<div class=\"toc-container\" style=\"background: linear-gradient(135deg, #ffffff 0%, #f4f7f6 100%); "
        "border: 1px solid #dcdde1; border-left: 5px solid #3498db; border-radius: 10px; padding: 18px 22px; "
        "margin: 15px 0 25px 0; box-shadow: 0 4px 15px rgba(0,0,0,0.05); font-family: 'Segoe UI', 'Kalpurush', sans-serif;\">\n");

    toc_block += QStringLiteral(
        "  <div style=\"display: flex; align-items: center; justify-content: space-between; border-bottom: 2px solid #eb4d4b; "
        "padding-bottom: 8px; margin-bottom: 16px;\">\n"
        "    <div style=\"font-size: 18px; font-weight: bold; color: #2c3e50; display: flex; align-items: center; gap: 8px;\">\n"
        "      <span style=\"background: #eb4d4b; color: white; border-radius: 6px; padding: 2px 8px; font-size: 13px;\">📌</span>\n"
        "      <span>সূচিপত্র (Table of Contents)</span>\n"
        "    </div>\n"
        "    <span style=\"font-size: 11px; background: #e1b12c; color: white; padding: 3px 8px; border-radius: 12px; font-weight: bold; text-transform: uppercase;\">Graber TOC</span>\n"
        "  </div>\n\n");

    // 1. Build list of effective sections (including any custom sections found in headings)
    QList<SectionItem> effective_sections = sections;
    QSet<QString> known_slugs;
    for (const auto &sec : effective_sections) {
        known_slugs.insert(sec.slug);
    }
    if (!known_slugs.contains(QStringLiteral("others"))) {
        effective_sections.append(SectionItem{QStringLiteral("অন্যান্য (Others)"), QStringLiteral("others")});
        known_slugs.insert(QStringLiteral("others"));
    }

    for (const auto &h : headings) {
        QString sec_slug = h.section;
        if (sec_slug.isEmpty()) sec_slug = QStringLiteral("others");
        if (!known_slugs.contains(sec_slug)) {
            QString displayName = sec_slug;
            if (!displayName.isEmpty())
                displayName[0] = displayName[0].toUpper();
            effective_sections.append(SectionItem{displayName, sec_slug});
            known_slugs.insert(sec_slug);
        }
    }

    // 2. Assign target section to each heading
    QVector<QString> item_sections(headings.size());
    QString current_sec = QStringLiteral("others");
    for (int i = 0; i < headings.size(); ++i) {
        if (headings[i].level == 2) {
            current_sec = headings[i].section.isEmpty() ? QStringLiteral("others") : headings[i].section;
            item_sections[i] = current_sec;
        } else {
            bool found_parent = false;
            if (!headings[i].parent_slug.isEmpty()) {
                for (int p = i - 1; p >= 0; --p) {
                    if (headings[p].level == 2 && headings[p].slug == headings[i].parent_slug) {
                        item_sections[i] = item_sections[p];
                        found_parent = true;
                        break;
                    }
                }
            }
            if (!found_parent) {
                item_sections[i] = current_sec;
            }
        }
    }

    QSet<int> rendered_headings;

    for (const auto &sec : effective_sections) {
        QList<int> sec_main_indices;
        for (int i = 0; i < headings.size(); ++i) {
            if (headings[i].level == 2 && item_sections[i] == sec.slug)
                sec_main_indices.append(i);
        }

        QList<int> sec_orphan_indices;
        for (int i = 0; i < headings.size(); ++i) {
            if (headings[i].level == 3 && item_sections[i] == sec.slug && !rendered_headings.contains(i)) {
                bool will_render = false;
                for (int mi : sec_main_indices) {
                    const HeadingInfo &h = headings[mi];
                    if (headings[i].parent_slug == h.slug || (i > mi && (mi == sec_main_indices.last() || i < sec_main_indices.value(sec_main_indices.indexOf(mi) + 1, headings.size())))) {
                        will_render = true;
                        break;
                    }
                }
                if (!will_render) {
                    sec_orphan_indices.append(i);
                }
            }
        }

        if (sec_main_indices.isEmpty() && sec_orphan_indices.isEmpty())
            continue;

        toc_block += QStringLiteral("  <div style=\"margin-top: 14px; margin-bottom: 12px;\">\n");
        toc_block += QStringLiteral(
            "    <div style=\"font-size: 14px; font-weight: bold; color: #2980b9; background: #eaf2f8; "
            "padding: 5px 12px; border-radius: 6px; border-left: 4px solid #3498db; margin-bottom: 8px;\">%1</div>\n")
            .arg(sec.displayName);

        toc_block += QStringLiteral("    <ul style=\"list-style: none; padding-left: 5px; margin: 0;\">\n");

        for (int mi : sec_main_indices) {
            const HeadingInfo &h = headings[mi];
            rendered_headings.insert(mi);

            const QString datePart = h.date.isEmpty() ? QString()
                : QStringLiteral("<span style=\"background: #e1f5fe; color: #0288d1; font-size: 11px; padding: 2px 6px; border-radius: 4px; font-weight: 500; margin-left: 8px;\">📅 %1</span>").arg(h.date);

            toc_block += QStringLiteral(
                "      <li style=\"margin-bottom: 8px; line-height: 1.5;\">\n"
                "        <div style=\"display: flex; align-items: baseline; gap: 6px;\">\n"
                "          <span style=\"background: #e74c3c; color: white; font-size: 11px; font-weight: bold; padding: 1px 6px; border-radius: 10px;\">%1</span>\n"
                "          <a href=\"#%2\" style=\"color: #c0392b; font-weight: bold; text-decoration: none; font-size: 14px;\">%3</a>\n"
                "          %4\n"
                "        </div>\n")
                .arg(QString::number(h.index), h.slug, h.title, datePart);

            if (!h.excerpt.isEmpty() && h.excerpt != h.title) {
                toc_block += QStringLiteral(
                    "        <div style=\"font-size: 12px; color: #7f8c8d; font-style: italic; margin-left: 26px; margin-top: 2px;\">%1</div>\n")
                    .arg(h.excerpt);
            }

            QList<int> sub_indices;
            for (int j = mi + 1; j < headings.size(); ++j) {
                if (headings[j].level == 2)
                    break;
                if (headings[j].level == 3) {
                    sub_indices.append(j);
                }
            }

            if (!sub_indices.isEmpty()) {
                toc_block += QStringLiteral("        <ul style=\"list-style: none; padding-left: 22px; margin-top: 4px; margin-bottom: 4px;\">\n");
                for (int sj : sub_indices) {
                    const HeadingInfo &s = headings[sj];
                    rendered_headings.insert(sj);

                    const QString subNum = QStringLiteral("%1.%2").arg(h.index).arg(s.sub_index);
                    const QString subDate = s.date.isEmpty() ? QString()
                        : QStringLiteral("<span style=\"background: #f0f4f8; color: #57606f; font-size: 10px; padding: 1px 5px; border-radius: 3px; margin-left: 6px;\">📅 %1</span>").arg(s.date);

                    toc_block += QStringLiteral(
                        "          <li style=\"margin-bottom: 4px; line-height: 1.4;\">\n"
                        "            <div style=\"display: flex; align-items: baseline; gap: 6px;\">\n"
                        "              <span style=\"color: #2980b9; font-weight: bold; font-size: 12px;\">↳ %1</span>\n"
                        "              <a href=\"#%2\" style=\"color: #2980b9; font-weight: 600; text-decoration: none; font-size: 13px;\">%3</a>\n"
                        "              %4\n"
                        "            </div>\n")
                        .arg(subNum, s.slug, s.title, subDate);

                    if (!s.excerpt.isEmpty() && s.excerpt != s.title) {
                        toc_block += QStringLiteral(
                            "            <div style=\"font-size: 11px; color: #95a5a6; font-style: italic; margin-left: 20px;\">%1</div>\n")
                            .arg(s.excerpt);
                    }
                    toc_block += QStringLiteral("          </li>\n");
                }
                toc_block += QStringLiteral("        </ul>\n");
            }

            toc_block += QStringLiteral("      </li>\n");
        }

        for (int sj : sec_orphan_indices) {
            if (rendered_headings.contains(sj))
                continue;
            const HeadingInfo &s = headings[sj];
            rendered_headings.insert(sj);

            const QString subNum = QStringLiteral("•");
            const QString subDate = s.date.isEmpty() ? QString()
                : QStringLiteral("<span style=\"background: #f0f4f8; color: #57606f; font-size: 10px; padding: 1px 5px; border-radius: 3px; margin-left: 6px;\">📅 %1</span>").arg(s.date);

            toc_block += QStringLiteral(
                "      <li style=\"margin-bottom: 6px; line-height: 1.4; padding-left: 10px;\">\n"
                "        <div style=\"display: flex; align-items: baseline; gap: 6px;\">\n"
                "          <span style=\"color: #2980b9; font-weight: bold; font-size: 12px;\">%1</span>\n"
                "          <a href=\"#%2\" style=\"color: #2980b9; font-weight: 600; text-decoration: none; font-size: 13px;\">%3</a>\n"
                "          %4\n"
                "        </div>\n")
                .arg(subNum, s.slug, s.title, subDate);

            if (!s.excerpt.isEmpty() && s.excerpt != s.title) {
                toc_block += QStringLiteral(
                    "        <div style=\"font-size: 11px; color: #95a5a6; font-style: italic; margin-left: 18px;\">%1</div>\n")
                    .arg(s.excerpt);
            }
            toc_block += QStringLiteral("      </li>\n");
        }

        toc_block += QStringLiteral("    </ul>\n");
        toc_block += QStringLiteral("  </div>\n");
    }

    for (int i = 0; i < headings.size(); ++i) {
        if (!rendered_headings.contains(i)) {
            const HeadingInfo &h = headings[i];
            toc_block += QStringLiteral(
                "  <div style=\"margin-left: 10px; margin-bottom: 4px;\"><a href=\"#%1\" style=\"color: #2c3e50; text-decoration: none; font-size: 13px;\">• %2</a></div>\n")
                .arg(h.slug, h.title);
        }
    }

    toc_block += QStringLiteral("</div>\n");
    toc_block += QStringLiteral("<!-- TOC_END -->\n\n");

    return toc_block;
}

} // namespace MarkdownTocHelpers

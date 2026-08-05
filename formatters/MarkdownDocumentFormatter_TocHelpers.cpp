#include "MarkdownDocumentFormatter_TocHelpers.h"
#include <QRegularExpression>

namespace MarkdownTocHelpers {

QString stripExistingToc(const QString &content) {
    QString clean = content;
    while (true) {
        const int toc_start = clean.indexOf(QStringLiteral("<!-- TOC_START -->"));
        const int toc_end = clean.indexOf(QStringLiteral("<!-- TOC_END -->"));
        if (toc_start != -1 && toc_end != -1 && toc_end > toc_start) {
            clean = clean.left(toc_start)
                  + clean.mid(toc_end + QStringLiteral("<!-- TOC_END -->").length());
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
    QString toc_block;
    if (headings.isEmpty())
        return toc_block;

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
    return toc_block;
}

} // namespace MarkdownTocHelpers

#include "MarkdownDocumentFormatter.h"
#include "MarkdownUtils.h"

#include <QRegularExpression>
#include <QStringList>
#include <QSet>

// ============================================================================
// parseNoteStructure — walk note body → ordered NoteItem list
// ============================================================================

QList<NoteItem> MarkdownDocumentFormatter::parseNoteStructure(
    const QString &content,
    const QList<SectionItem> &availableSections,
    QSet<QString> &outCustomSections) const {

    QList<NoteItem> items;
    const QStringList lines = content.split(QLatin1Char('\n'));

    static const QRegularExpression h2_regex(
        QStringLiteral("<h2([^>]*)>(.*?)</h2>"),
        QRegularExpression::CaseInsensitiveOption);
    static const QRegularExpression h3_regex(
        QStringLiteral("<h3([^>]*)>(.*?)</h3>"),
        QRegularExpression::CaseInsensitiveOption);
    static const QRegularExpression section_attr_regex(
        QStringLiteral("data-section=\"([^\"]*)\""),
        QRegularExpression::CaseInsensitiveOption);
    static const QRegularExpression md_regex(QStringLiteral("^(#{2})\\s+(.*?)$"));
    static const QRegularExpression md_sub_regex(
        QStringLiteral("^(#{3})\\s+(?!\\*\\*\\*)(.*?)$"));
    static const QRegularExpression md_section_regex(
        QStringLiteral("<!--\\s*section:([\\w-]+)\\s*-->"));

    QString current_parent_slug;

    QSet<QString> known_slugs;
    for (const auto &sec : availableSections)
        known_slugs.insert(sec.slug);

    for (const QString &raw : lines) {
        const QString line = raw.trimmed();
        const auto h2_match = h2_regex.match(line);
        const auto h3_match = h3_regex.match(line);
        const auto md_match = md_regex.match(line);
        const auto md_sub_match = md_sub_regex.match(line);

        if (h2_match.hasMatch()) {
            const QString attributes = h2_match.captured(1);
            const QString title = h2_match.captured(2).trimmed();
            const QString slug = generateSlug(title);

            QString section = QStringLiteral("others");
            const auto section_match = section_attr_regex.match(attributes);
            if (section_match.hasMatch())
                section = section_match.captured(1);

            if (!known_slugs.contains(section) && section != QLatin1String("others"))
                outCustomSections.insert(section);

            NoteItem item;
            item.title = title;
            item.slug = slug;
            item.type = QStringLiteral("heading");
            item.section = section;
            item.parent_slug = QString();
            items.append(item);
            current_parent_slug = slug;
        } else if (h3_match.hasMatch()) {
            const QString title = h3_match.captured(2).trimmed();
            const QString slug = generateSlug(title);

            NoteItem item;
            item.title = title;
            item.slug = slug;
            item.type = QStringLiteral("subheading");
            item.section = QString();
            item.parent_slug = current_parent_slug;
            items.append(item);
        } else if (md_match.hasMatch()) {
            const QString rest = md_match.captured(2).trimmed();
            QString section = QStringLiteral("others");
            QString title = rest;
            const auto section_match = md_section_regex.match(rest);
            if (section_match.hasMatch()) {
                section = section_match.captured(1);
                title = rest.left(section_match.capturedStart()).trimmed();
            }

            if (!known_slugs.contains(section) && section != QLatin1String("others"))
                outCustomSections.insert(section);

            const QString slug = generateSlug(title);

            NoteItem item;
            item.title = title;
            item.slug = slug;
            item.type = QStringLiteral("heading");
            item.section = section;
            item.parent_slug = QString();
            items.append(item);
            current_parent_slug = slug;
        } else if (md_sub_match.hasMatch()) {
            const QString title = md_sub_match.captured(2).trimmed();
            const QString slug = generateSlug(title);

            NoteItem item;
            item.title = title;
            item.slug = slug;
            item.type = QStringLiteral("subheading");
            item.section = QString();
            item.parent_slug = current_parent_slug;
            items.append(item);
        }
    }

    return items;
}

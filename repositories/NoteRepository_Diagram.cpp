#include "NoteRepository.h"
#include "MarkdownUtils.h"
#include "../utils/FileIO.h"
#include "../utils/CrashGuard.h"

#include <QDateTime>

namespace {

bool appendDiagramUnderDate(const QString &targetFile, const QString &block, QString &lastDate) {
    lastDate = MarkdownUtils::restore_state_from_file(targetFile);

    auto read = FileIO::readText(targetFile);
    if (read.isFail())
        return false;

    QString content = read.takeValue();
    const QString current_date = QDateTime::currentDateTime().toString(QStringLiteral("dd MMMM, yyyy"));

    if (current_date != lastDate) {
        if (!content.isEmpty() && !content.endsWith(QLatin1Char('\n')))
            content += QLatin1Char('\n');
        content += QStringLiteral("\n### ***%1***\n").arg(current_date);
        lastDate = current_date;
    }

    if (!content.isEmpty() && !content.endsWith(QLatin1Char('\n')))
        content += QLatin1Char('\n');
    content += block;
    if (!content.endsWith(QLatin1Char('\n')))
        content += QLatin1Char('\n');

    return FileIO::writeTextAtomic(targetFile, content).isOk();
}

} // namespace

bool NoteRepository::insertDiagramToNote(const QString &targetFile, const QString &diagramMarkdown,
                                         const QString &selectedSlug, QString &lastDate) {
    if (isUnselectedSubject(targetFile))
        return false;
    if (diagramMarkdown.isEmpty())
        return false;

    return CrashGuard::safeCallValue<bool>([&]() -> bool {
        if (!selectedSlug.isEmpty()) {
            auto read = FileIO::readText(targetFile);
            if (read.isFail())
                return false;

            QString content = read.takeValue();

            int start_pos = -1;
            int end_pos = -1;
            bool is_html = false;
            int insert_pos = -1;

            if (MarkdownUtils::get_heading_bounds(content, selectedSlug, start_pos, end_pos, is_html)) {
                insert_pos = end_pos;
            } else if (!MarkdownUtils::get_subheading_insert_pos(content, selectedSlug, insert_pos)) {
                return false;
            }

            if (insert_pos < 0 || insert_pos > content.size())
                return false;

            QString to_insert = diagramMarkdown;
            if (insert_pos > 0 && content[insert_pos - 1] != QLatin1Char('\n'))
                to_insert.prepend(QLatin1Char('\n'));
            content.insert(insert_pos, to_insert);

            return FileIO::writeTextAtomic(targetFile, content).isOk();
        }

        return appendDiagramUnderDate(targetFile, diagramMarkdown, lastDate);
    }, false, QStringLiteral("insertDiagramToNote"));
}

bool NoteRepository::upsertLiveDiagram(const QString &targetFile, const QString &sessionId,
                                       const QString &diagramMarkdown, const QString &selectedSlug,
                                       QString &lastDate) {
    if (isUnselectedSubject(targetFile))
        return false;
    if (diagramMarkdown.isEmpty() || sessionId.isEmpty())
        return false;

    // Session id is app-generated (epoch ms); still reject path-like noise.
    if (sessionId.contains(QLatin1Char('/')) || sessionId.contains(QLatin1Char('\\'))
        || sessionId.contains(QLatin1String("..")))
        return false;

    return CrashGuard::safeCallValue<bool>([&]() -> bool {
        const QString start_marker = QStringLiteral("<!-- DIAGRAM:%1 START -->").arg(sessionId);
        const QString end_marker = QStringLiteral("<!-- DIAGRAM:%1 END -->").arg(sessionId);
        const QString wrapped_block =
            start_marker + QLatin1Char('\n') + diagramMarkdown.trimmed() + QLatin1Char('\n') + end_marker;

        auto read = FileIO::readText(targetFile);
        if (read.isFail())
            return false;

        QString content = read.takeValue();

        const int start_idx = content.indexOf(start_marker);
        const int end_idx = content.indexOf(end_marker);

        if (start_idx != -1 && end_idx != -1 && end_idx > start_idx) {
            // Grow existing session block in place.
            const int block_end = end_idx + end_marker.length();
            content.replace(start_idx, block_end - start_idx, wrapped_block);
            return FileIO::writeTextAtomic(targetFile, content).isOk();
        }

        // First capture for this session.
        if (!selectedSlug.isEmpty()) {
            int start_pos = -1;
            int end_pos = -1;
            bool is_html = false;
            int insert_pos = -1;

            if (MarkdownUtils::get_heading_bounds(content, selectedSlug, start_pos, end_pos, is_html)) {
                insert_pos = end_pos;
            } else if (!MarkdownUtils::get_subheading_insert_pos(content, selectedSlug, insert_pos)) {
                insert_pos = content.length();
            }

            if (insert_pos < 0 || insert_pos > content.size())
                insert_pos = content.length();

            QString to_insert = wrapped_block + QLatin1Char('\n');
            if (insert_pos > 0 && content[insert_pos - 1] != QLatin1Char('\n'))
                to_insert.prepend(QLatin1Char('\n'));
            content.insert(insert_pos, to_insert);
            return FileIO::writeTextAtomic(targetFile, content).isOk();
        }

        return appendDiagramUnderDate(targetFile, wrapped_block + QLatin1Char('\n'), lastDate);
    }, false, QStringLiteral("upsertLiveDiagram"));
}

#include "NoteRepository.h"
#include "MarkdownUtils.h"
#include "CaptureContentFormatter.h"
#include "../utils/Utils.h"
#include "../utils/FileIO.h"
#include "../utils/CrashGuard.h"

#include <QDateTime>

namespace {

bool appendUnderDateSection(const QString &targetFile, const QString &block, QString &lastDate) {
    lastDate = MarkdownUtils::restore_state_from_file(targetFile);

    auto existing = FileIO::readText(targetFile);
    if (existing.isFail())
        return false;

    QString out = existing.value();

    const QString currentDate = QDateTime::currentDateTime().toString(QStringLiteral("dd MMMM, yyyy"));
    if (currentDate != lastDate) {
        if (!out.isEmpty() && !out.endsWith(QLatin1Char('\n')))
            out += QLatin1Char('\n');
        out += QStringLiteral("\n### ***%1***\n").arg(currentDate);
        lastDate = currentDate;
    }

    out += block;
    return FileIO::writeTextAtomic(targetFile, out).isOk();
}

} // namespace

bool NoteRepository::appendContentToHeading(const QString &filePath, const QString &slug,
                                            const QString &processedText, int formatIndex,
                                            const QString &section) {
    if (filePath.isEmpty() || slug.isEmpty())
        return false;

    return CrashGuard::safeCallValue<bool>([&]() -> bool {
        auto read = FileIO::readText(filePath);
        if (read.isFail())
            return false;

        QString content = read.takeValue();

        int start_pos = -1;
        int end_pos = -1;
        bool is_html = false;
        int insert_pos = -1;

        if (MarkdownUtils::get_heading_bounds(content, slug, start_pos, end_pos, is_html)) {
            insert_pos = end_pos;
        } else if (!MarkdownUtils::get_subheading_insert_pos(content, slug, insert_pos)) {
            return false;
        }

        // Bounds sanity — never insert with a wild index
        if (insert_pos < 0 || insert_pos > content.size())
            return false;

        QString to_append = CaptureContentFormatter::format(processedText, formatIndex, section);
        if (to_append.isEmpty())
            return false;

        if (insert_pos > 0 && content[insert_pos - 1] != QLatin1Char('\n'))
            to_append.prepend(QLatin1Char('\n'));

        content.insert(insert_pos, to_append);
        return FileIO::writeTextAtomic(filePath, content).isOk();
    }, false, QStringLiteral("appendContentToHeading"));
}

bool NoteRepository::writeToNote(const QString &targetFile, const QString &processedText,
                                 int formatIndex, const QString &section,
                                 const QString &selectedSlug, QString &lastDate,
                                 QString &outCapturedLabelText) {
    if (isUnselectedSubject(targetFile)) {
        outCapturedLabelText = QStringLiteral("ত্রুটি: ফাইলে লেখা যায়নি!");
        return false;
    }

    return CrashGuard::safeCallValue<bool>([&]() -> bool {
        if (!selectedSlug.isEmpty()) {
            if (appendContentToHeading(targetFile, selectedSlug, processedText, formatIndex, section)) {
                outCapturedLabelText = QStringLiteral("শেষ ক্যাপচার (নির্বাচিত শিরোনামে যুক্ত করা হয়েছে): ")
                                       + processedText;
                return true;
            }
            outCapturedLabelText = QStringLiteral("ত্রুটি: নির্বাচিত শিরোনামে যুক্ত করা যায়নি!");
            return false;
        }

        const QString block = CaptureContentFormatter::format(processedText, formatIndex, section);
        if (block.isEmpty() && !processedText.trimmed().isEmpty()) {
            outCapturedLabelText = QStringLiteral("ত্রুটি: ফাইলে লেখা যায়নি!");
            return false;
        }

        if (!appendUnderDateSection(targetFile, block, lastDate)) {
            outCapturedLabelText = QStringLiteral("ত্রুটি: ফাইলে লেখা যায়নি!");
            return false;
        }

        outCapturedLabelText = QStringLiteral("শেষ ক্যাপচার: ") + processedText;
        return true;
    }, false, QStringLiteral("writeToNote"));
}

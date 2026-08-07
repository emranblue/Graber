#include "NoteRepository.h"
#include "MarkdownUtils.h"
#include "MarkdownTemplateManager.h"
#include "../utils/FileIO.h"
#include "../utils/CrashGuard.h"
#include "../utils/Utils.h"

#include <QDateTime>
#include <QDir>
#include <QFileInfo>
#include <QRegularExpression>
#include <QSet>

bool NoteRepository::deleteHeadingSection(const QString &targetFile, const QString &slug,
                                          const QString &subjectName, QString &outCapturedLabelText) {
    if (targetFile.isEmpty() || slug.isEmpty()) {
        outCapturedLabelText = QStringLiteral("ত্রুটি: অবৈধ পথ বা স্লাগ!");
        return false;
    }

    return CrashGuard::safeCallValue<bool>([&]() -> bool {
        auto read = FileIO::readText(targetFile);
        if (read.isFail()) {
            outCapturedLabelText = QStringLiteral("ত্রুটি: ফাইলটি খোলা যায়নি!");
            return false;
        }

        QString content = read.takeValue();

        int start_pos = -1;
        int end_pos = -1;
        bool is_html = false;
        bool found = false;

        if (MarkdownUtils::get_heading_bounds(content, slug, start_pos, end_pos, is_html)) {
            found = true;
        } else if (MarkdownUtils::get_subheading_bounds(content, slug, start_pos, end_pos)) {
            found = true;
        }

        if (!found || start_pos < 0 || end_pos < start_pos || end_pos > content.size()) {
            outCapturedLabelText = QStringLiteral("ত্রুটি: শিরোনাম বা উপ-শিরোনামটি খুঁজে পাওয়া যায়নি!");
            return false;
        }

        const QString deleted_chunk = content.mid(start_pos, end_pos - start_pos);
        content.remove(start_pos, end_pos - start_pos);

        if (!FileIO::writeTextAtomic(targetFile, content).isOk()) {
            outCapturedLabelText = QStringLiteral("ত্রুটি: ফাইলে লেখা যায়নি!");
            return false;
        }

        // Safe backup filename: sanitize subject path segments
        QString safeSubject = sanitizeRelativePath(subjectName);
        if (safeSubject.isEmpty())
            safeSubject = QStringLiteral("unknown");
        safeSubject.replace(QLatin1Char('/'), QLatin1Char('_'));

        const QString del_filename = QStringLiteral("%1_%2_%3.txt")
                                         .arg(safeSubject, slug,
                                              QDateTime::currentDateTime().toString(QStringLiteral("yyyyMMdd_hhmmss")));

        const QString deleted_dir_path = notes_dir_path_ + QDir::separator() + QStringLiteral("deleted");
        const QString del_filepath = deleted_dir_path + QDir::separator() + del_filename;

        if (FileIO::writeTextAtomic(del_filepath, deleted_chunk).isOk()) {
            outCapturedLabelText = QStringLiteral("মুছে ফেলা হয়েছে এবং ব্যাকআপ রাখা হয়েছে: %1").arg(del_filename);
        } else {
            outCapturedLabelText = QStringLiteral("মুছে ফেলা হয়েছে কিন্তু ব্যাকআপ রাখা যায়নি!");
        }
        return true;
    }, false, QStringLiteral("deleteHeadingSection"));
}

bool NoteRepository::injectSubheadingFromNote(const QString &targetFile, const QString &insertAfterSlug,
                                              const QString &sourceFile, const QString &sourceSlug,
                                              const QList<SectionItem> &sections,
                                              QString &outCapturedLabelText) {
    if (targetFile.isEmpty() || sourceFile.isEmpty() || sourceSlug.isEmpty()) {
        outCapturedLabelText = QStringLiteral("ত্রুটি: অবৈধ পথ বা স্লাগ!");
        return false;
    }
    if (targetFile == sourceFile) {
        outCapturedLabelText = QStringLiteral("ত্রুটি: একই ফাইল থেকে ইনজেক্ট করা যাবে না — শাফল ব্যবহার করুন।");
        return false;
    }

    return CrashGuard::safeCallValue<bool>([&]() -> bool {
        auto srcRead = FileIO::readText(sourceFile);
        if (srcRead.isFail()) {
            outCapturedLabelText = QStringLiteral("ত্রুটি: সোর্স ফাইল খোলা যায়নি!");
            return false;
        }
        QString srcContent = srcRead.takeValue();

        int src_start = -1, src_end = -1;
        bool is_html = false;
        bool is_main = false;
        bool found = false;
        if (MarkdownUtils::get_subheading_bounds(srcContent, sourceSlug, src_start, src_end)) {
            found = true;
            is_main = false;
        } else if (MarkdownUtils::get_heading_bounds(srcContent, sourceSlug, src_start, src_end, is_html)) {
            found = true;
            is_main = true;
        }
        if (!found || src_start < 0 || src_end <= src_start) {
            outCapturedLabelText = QStringLiteral("ত্রুটি: সোর্স উপ-শিরোনাম খুঁজে পাওয়া যায়নি!");
            return false;
        }

        QString rawBlock = srcContent.mid(src_start, src_end - src_start);

        // Pull display title out of the extracted block (HTML or MD)
        QString title;
        {
            static const QRegularExpression htmlTitleRx(
                QStringLiteral("<h[23][^>]*>(.*?)</h[23]>"),
                QRegularExpression::CaseInsensitiveOption | QRegularExpression::DotMatchesEverythingOption);
            static const QRegularExpression divTitleRx(
                QStringLiteral("<div[^>]*\\bid=[\"'][^\"']+[\"'][^>]*>(.*?)</div>"),
                QRegularExpression::CaseInsensitiveOption | QRegularExpression::DotMatchesEverythingOption);
            static const QRegularExpression mdTitleRx(
                QStringLiteral("^#{2,3}\\s+(?:\\*\\*\\*)?(.*?)(?:\\*\\*\\*)?\\s*$"),
                QRegularExpression::MultilineOption);

            auto m = htmlTitleRx.match(rawBlock);
            if (m.hasMatch()) {
                title = m.captured(1);
            } else {
                auto d = divTitleRx.match(rawBlock);
                if (d.hasMatch())
                    title = d.captured(1);
                else {
                    auto md = mdTitleRx.match(rawBlock);
                    if (md.hasMatch())
                        title = md.captured(1);
                }
            }
            // strip inner tags / emphasis
            title.replace(QRegularExpression(QStringLiteral("<[^>]+>")), QString());
            title.replace(QRegularExpression(QStringLiteral("^[*_]{1,3}|[*_]{1,3}$")), QString());
            title = title.trimmed();
        }
        if (title.isEmpty())
            title = sourceSlug;

        // Body after the heading line(s) — keep user content under the heading
        QString bodyOnly = rawBlock;
        {
            static const QRegularExpression stripHeadRx(
                QStringLiteral("^(?:\\s*<div[^>]*>\\s*)?<h[23][^>]*>.*?</h[23]>\\s*(?:</div>)?\\s*"),
                QRegularExpression::CaseInsensitiveOption | QRegularExpression::DotMatchesEverythingOption);
            static const QRegularExpression stripDivHeadRx(
                QStringLiteral("^\\s*<div[^>]*\\bid=[\"'][^\"']+[\"'][^>]*>.*?</div>\\s*"),
                QRegularExpression::CaseInsensitiveOption | QRegularExpression::DotMatchesEverythingOption);
            static const QRegularExpression stripMdRx(
                QStringLiteral("^\\s*#{2,3}\\s+.*?\\n"),
                QRegularExpression::MultilineOption);
            if (stripHeadRx.match(bodyOnly).hasMatch())
                bodyOnly.remove(stripHeadRx);
            else if (stripDivHeadRx.match(bodyOnly).hasMatch())
                bodyOnly.remove(stripDivHeadRx);
            else if (stripMdRx.match(bodyOnly).hasMatch())
                bodyOnly.remove(stripMdRx);
        }

        // Rebuild heading with CURRENT templates so TOC parser always sees a canonical <h2>/<h3>
        QString headingHtml;
        if (is_main) {
            headingHtml = MarkdownTemplateManager::instance().formatByKey(
                QStringLiteral("heading"), title, QStringLiteral("others"));
        } else {
            headingHtml = MarkdownTemplateManager::instance().formatByKey(
                QStringLiteral("subheading"), title);
        }
        if (headingHtml.trimmed().isEmpty()) {
            // Fallback minimal tags — still TOC-detectable
            const QString slug = QString::fromStdString(MarkdownUtils::generate_slug(title));
            headingHtml = is_main
                ? QStringLiteral("\n<h2 id=\"%1\" data-section=\"others\">%2</h2>\n").arg(slug, title)
                : QStringLiteral("\n<h3 id=\"%1\">%2</h3>\n").arg(slug, title);
        }

        QString block = headingHtml;
        if (!block.endsWith(QLatin1Char('\n')))
            block += QLatin1Char('\n');
        if (!bodyOnly.trimmed().isEmpty()) {
            if (!bodyOnly.startsWith(QLatin1Char('\n')))
                block += QLatin1Char('\n');
            block += bodyOnly;
            if (!block.endsWith(QLatin1Char('\n')))
                block += QLatin1Char('\n');
        }

        auto tgtRead = FileIO::readText(targetFile);
        if (tgtRead.isFail()) {
            outCapturedLabelText = QStringLiteral("ত্রুটি: টার্গেট ফাইল খোলা যায়নি!");
            return false;
        }
        QString content = tgtRead.takeValue();

        int insert_pos = content.length();
        if (!insertAfterSlug.isEmpty()) {
            int t_start = -1, t_end = -1;
            bool t_html = false;
            if (MarkdownUtils::get_heading_bounds(content, insertAfterSlug, t_start, t_end, t_html)) {
                insert_pos = t_end;
            } else if (!MarkdownUtils::get_subheading_insert_pos(content, insertAfterSlug, insert_pos)) {
                outCapturedLabelText = QStringLiteral("ত্রুটি: টার্গেট স্থান খুঁজে পাওয়া যায়নি!");
                return false;
            }
        }

        if (insert_pos < 0 || insert_pos > content.size()) {
            outCapturedLabelText = QStringLiteral("ত্রুটি: অবৈধ ইনসার্ট পজিশন!");
            return false;
        }
        if (insert_pos > 0 && content[insert_pos - 1] != QLatin1Char('\n'))
            block.prepend(QLatin1Char('\n'));

        content.insert(insert_pos, block);

        // Rebuild TOC in the same write so injected heading is always listed
        if (formatter_) {
            content = formatter_->updateTocInContent(content, sections);
            if (!content.endsWith(QLatin1Char('\n')))
                content += QLatin1Char('\n');
        }

        if (!FileIO::writeTextAtomic(targetFile, content).isOk()) {
            outCapturedLabelText = QStringLiteral("ত্রুটি: টার্গেট ফাইলে লেখা যায়নি!");
            return false;
        }

        // Keep .tree sidecar in sync
        if (formatter_) {
            QSet<QString> custom;
            const auto items = formatter_->parseNoteStructure(content, sections, custom);
            formatter_->saveStructureTree(targetFile, items);
        }

        outCapturedLabelText = QStringLiteral("ইনজেক্ট সফল + TOC আপডেট: %1").arg(title);
        return true;
    }, false, QStringLiteral("injectSubheadingFromNote"));
}

bool NoteRepository::shiftHeadingSection(const QString &targetFile, const QString &sourceSlug,
                                         const QString &targetSlug, const QList<NoteItem> &allHeadings,
                                         QString &outCapturedLabelText) {
    Q_UNUSED(allHeadings);
    if (targetFile.isEmpty() || sourceSlug.isEmpty()) {
        outCapturedLabelText = QStringLiteral("ত্রুটি: অবৈধ পথ বা স্লাগ!");
        return false;
    }

    return CrashGuard::safeCallValue<bool>([&]() -> bool {
        auto read = FileIO::readText(targetFile);
        if (read.isFail()) {
            outCapturedLabelText = QStringLiteral("ত্রুটি: ফাইলটি খোলা যায়নি!");
            return false;
        }

        QString content = read.takeValue();

        int src_start = -1;
        int src_end = -1;
        bool is_html = false;
        bool found = false;

        if (MarkdownUtils::get_heading_bounds(content, sourceSlug, src_start, src_end, is_html)) {
            found = true;
        } else if (MarkdownUtils::get_subheading_bounds(content, sourceSlug, src_start, src_end)) {
            found = true;
        }

        if (!found || src_start < 0 || src_end < src_start || src_end > content.size()) {
            outCapturedLabelText = QStringLiteral("ত্রুটি: স্থানান্তর করার জন্য উৎস সেকশন পাওয়া যায়নি!");
            return false;
        }

        QString source_chunk = content.mid(src_start, src_end - src_start);
        content.remove(src_start, src_end - src_start);

        int insert_pos = -1;
        if (targetSlug.isEmpty()) {
            insert_pos = content.length();
        } else {
            int tgt_start = -1;
            int tgt_end = -1;
            bool tgt_is_html = false;

            if (MarkdownUtils::get_heading_bounds(content, targetSlug, tgt_start, tgt_end, tgt_is_html)) {
                insert_pos = tgt_end;
            } else if (!MarkdownUtils::get_subheading_insert_pos(content, targetSlug, insert_pos)) {
                outCapturedLabelText = QStringLiteral("ত্রুটি: গন্তব্য সেকশন খুঁজে পাওয়া যায়নি!");
                return false;
            }
        }

        if (insert_pos < 0 || insert_pos > content.size()) {
            outCapturedLabelText = QStringLiteral("ত্রুটি: অবৈধ ইনসার্ট পজিশন!");
            return false;
        }

        if (insert_pos > 0 && content[insert_pos - 1] != QLatin1Char('\n')) {
            source_chunk.prepend(QLatin1Char('\n'));
        }
        content.insert(insert_pos, source_chunk);

        if (!FileIO::writeTextAtomic(targetFile, content).isOk()) {
            outCapturedLabelText = QStringLiteral("ত্রুটি: ফাইলে স্থানান্তর সম্পন্ন করা যায়নি!");
            return false;
        }

        outCapturedLabelText = QStringLiteral("স্থানান্তর সফল হয়েছে: %1").arg(sourceSlug);
        return true;
    }, false, QStringLiteral("shiftHeadingSection"));
}

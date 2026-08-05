#include "CaptureContentFormatter.h"
#include "MarkdownTemplateManager.h"

namespace CaptureContentFormatter {

QString format(const QString &rawText, int formatIndex, const QString &sectionSlug) {
    return MarkdownTemplateManager::instance().formatContent(formatIndex, rawText, sectionSlug);
}

} // namespace CaptureContentFormatter

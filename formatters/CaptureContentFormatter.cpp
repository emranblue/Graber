#include "CaptureContentFormatter.h"
#include "MarkdownUtils.h"
#include "../utils/Utils.h"

namespace CaptureContentFormatter {

QString format(const QString &rawText, int formatIndex, const QString &sectionSlug) {
    const QString text = escapeHtml(rawText.trimmed());
    if (text.isEmpty())
        return {};

    if (formatIndex == 1) {
        const QString slug = QString::fromStdString(MarkdownUtils::generate_slug(rawText.trimmed()));
        const QString sec = sectionSlug.isEmpty() ? QStringLiteral("others") : escapeHtml(sectionSlug);
        return QStringLiteral(
            "\n<h2 id=\"%1\" data-section=\"%2\" "
            "style=\"color: #e74c3c; font-weight: bold; font-style: italic; margin-bottom: 5px;\">"
            "%3</h2>\n")
            .arg(escapeHtml(slug), sec, text);
    }

    if (formatIndex == 2) {
        const QString slug = QString::fromStdString(MarkdownUtils::generate_slug(rawText.trimmed()));
        return QStringLiteral(
            "\n<h3 id=\"%1\" "
            "style=\"color: #2980b9; font-weight: bold; font-style: italic; "
            "margin-top: 10px; margin-bottom: 5px;\">%2</h3>\n")
            .arg(escapeHtml(slug), text);
    }

    if (formatIndex == 3) {
        const QString color = get_random_beautiful_color();
        const QString colorAlpha = color + QStringLiteral("0f");
        return QStringLiteral(
            "<div class=\"timeline-item\" style=\"border-left: 2px dashed %1; margin-left: 20px; "
            "padding-left: 20px; padding-bottom: 12px; position: relative;\">"
            "<span style=\"position: absolute; left: -2px; top: 18px; width: 12px; height: 2px; "
            "background-color: %1;\"></span>"
            "<span style=\"position: absolute; left: 8px; top: 13px; color: %1; font-size: 10px; "
            "line-height: 1;\">➤</span>"
            "<div style=\"background-color: %2; border: 1px solid %1; border-radius: 6px; "
            "padding: 8px 12px; display: inline-block; box-shadow: 1px 1px 3px rgba(0,0,0,0.05); "
            "margin-left: 10px;\">"
            "<span style=\"color: %1; font-weight: 600; font-family: 'Segoe UI', 'Kalpurush', "
            "sans-serif; font-size: 16px;\">%3</span></div></div>\n")
            .arg(color, colorAlpha, text);
    }

    if (formatIndex == 4) {
        return QStringLiteral(
            "<p class=\"paragraph-item\" style=\"color: #2f3640; line-height: 1.6; "
            "font-family: 'Segoe UI', 'Kalpurush', sans-serif; margin-bottom: 10px; "
            "text-align: justify;\">%1</p>\n")
            .arg(text);
    }

    // Default: bullet
    return QStringLiteral("\n ▣ %1\n\n").arg(text);
}

} // namespace CaptureContentFormatter

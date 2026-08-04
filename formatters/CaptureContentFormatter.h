#ifndef CAPTURECONTENTFORMATTER_H
#define CAPTURECONTENTFORMATTER_H

#include <QString>

/**
 * Single place that turns clipboard text + format index into the markup
 * written into notes. Used by append-to-heading and end-of-file write paths
 * so HTML structure and escaping stay consistent.
 *
 * formatIndex:
 *   0 = bullet, 1 = h2 heading, 2 = h3 subheading,
 *   3 = timeline, 4 = paragraph
 */
namespace CaptureContentFormatter {
    QString format(const QString &rawText, int formatIndex, const QString &sectionSlug = QString());
}

#endif // CAPTURECONTENTFORMATTER_H

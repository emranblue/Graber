#ifndef UTILS_H
#define UTILS_H

#include <QString>
#include <QIcon>
#include <QColor>
#include <QChar>

void set_feather_font_family(const QString &familyName);
QString get_feather_font_family();

void debugLog(const QString &msg);

QString get_random_beautiful_color();

QIcon get_feather_icon(const QChar &code, const QColor &color = QColor("#ffffff"), int size = 16);

/** Multi-resolution app icon (taskbar, title bar, About). */
QIcon get_app_icon();
QIcon get_folder_icon();
QIcon get_file_icon();

/** Escape &, <, >, " for safe insertion into HTML attribute/text nodes. */
QString escapeHtml(const QString &text);

/**
 * Normalize a user-supplied relative path under the notes root.
 * Rejects absolute paths, drive letters, ".." segments, and empty results.
 * Accepts forward or back slashes; returns forward-slash form on success,
 * or empty QString if unsafe.
 */
QString sanitizeRelativePath(const QString &userPath);

/** True if path is non-empty and sanitizeRelativePath would accept it. */
bool isSafeRelativePath(const QString &userPath);

/**
 * True for empty subject / the UI "unselected" sentinel.
 * Covers both common Unicode spellings of "নয়" (YA+NUKTA vs YYA) so a
 * single helper replaces the duplicated dual-literal checks.
 */
bool isUnselectedSubject(const QString &nameOrPath);

/** Canonical display sentinel used when no subject is selected. */
inline QString unselectedSubjectLabel() {
    return QStringLiteral("নির্বাচিত নয়");
}

#endif // UTILS_H

#ifndef UTILS_H
#define UTILS_H

#include <QString>
#include <QIcon>
#include <QColor>
#include <QChar>

// Set and get the active icon font family
void set_feather_font_family(const QString &familyName);
QString get_feather_font_family();

// Write debug logs to ~/GraberNotes/debug.log
void debugLog(const QString &msg);

// Return a random hex color string
QString get_random_beautiful_color();

// Render an icon glyph into a QIcon
QIcon get_feather_icon(const QChar &code, const QColor &color = QColor("#ffffff"), int size = 16);

#endif // UTILS_H

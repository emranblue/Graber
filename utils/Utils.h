#ifndef UTILS_H
#define UTILS_H

#include <QString>
#include <QIcon>
#include <QColor>

void debugLog(const QString &msg);
QString get_random_beautiful_color();
QIcon get_feather_icon(const QChar &code, const QColor &color = QColor("#ffffff"), int size = 18);

#endif // UTILS_H

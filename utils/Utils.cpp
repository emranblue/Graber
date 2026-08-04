#include "Utils.h"
#include <QFile>
#include <QDir>
#include <QTextStream>
#include <QDateTime>
#include <QRandomGenerator>
#include <QPainter>
#include <QFont>
#include <QPixmap>
#include <QStringList>
#include <QStandardPaths>

static QString s_featherFontFamily = "icomoon";

void set_feather_font_family(const QString &familyName) {
    if (!familyName.isEmpty()) {
        s_featherFontFamily = familyName;
    }
}

QString get_feather_font_family() {
    return s_featherFontFamily;
}

void debugLog(const QString &msg) {
    QString log_dir = QStandardPaths::writableLocation(QStandardPaths::HomeLocation) + QDir::separator() + "GraberNotes";
    QDir dir(log_dir);
    if (!dir.exists()) {
        dir.mkpath(".");
    }
    QFile file(log_dir + QDir::separator() + "debug.log");
    if (file.open(QIODevice::WriteOnly | QIODevice::Append | QIODevice::Text)) {
        QTextStream out(&file);
        out << QDateTime::currentDateTime().toString("yyyy-MM-dd hh:mm:ss") << " - " << msg << "\n";
        file.close();
    }
}

QString get_random_beautiful_color() {
    static const QStringList colors = {
        "#1abc9c", "#2ecc71", "#3498db", "#9b59b6", "#e67e22",
        "#e74c3c", "#16a085", "#27ae60", "#2980b9", "#8e44ad",
        "#d35400", "#c0392b", "#d81b60", "#c2185b", "#3f51b5",
        "#1a5276", "#7d3c98", "#196f3d", "#b03a2e", "#0984e3",
        "#d63031", "#e84393", "#6c5ce7", "#00b894", "#fdb827"
    };
    static int last_idx = -1;
    int idx = last_idx;
    if (colors.size() > 1) {
        while (idx == last_idx) {
            idx = QRandomGenerator::global()->bounded(colors.size());
        }
        last_idx = idx;
    } else {
        idx = 0;
    }
    return colors.at(idx);
}

QIcon get_feather_icon(const QChar &code, const QColor &color, int size) {
    QPixmap pixmap(size, size);
    pixmap.fill(Qt::transparent);
    QPainter painter(&pixmap);
    painter.setRenderHint(QPainter::Antialiasing);
    painter.setRenderHint(QPainter::TextAntialiasing);

    QFont font(get_feather_font_family());
    font.setPixelSize(size - 2);
    painter.setFont(font);
    painter.setPen(color);

    painter.drawText(pixmap.rect(), Qt::AlignCenter, QString(code));
    painter.end();

    return QIcon(pixmap);
}


QString escapeHtml(const QString &text) {
    QString out;
    out.reserve(text.size() + 8);
    for (const QChar c : text) {
        switch (c.unicode()) {
        case '&':  out += QLatin1String("&amp;");  break;
        case '<':  out += QLatin1String("&lt;");   break;
        case '>':  out += QLatin1String("&gt;");   break;
        case '"':  out += QLatin1String("&quot;"); break;
        default:   out += c; break;
        }
    }
    return out;
}

QString sanitizeRelativePath(const QString &userPath) {
    QString p = userPath.trimmed();
    p.replace(QLatin1Char('\\'), QLatin1Char('/'));

    if (p.isEmpty())
        return {};

    // Absolute / UNC / drive-letter
    if (p.startsWith(QLatin1Char('/')) || p.startsWith(QLatin1String("//")))
        return {};
    if (p.size() >= 2 && p[1] == QLatin1Char(':'))
        return {};

    const QStringList parts = p.split(QLatin1Char('/'), Qt::SkipEmptyParts);
    QStringList clean;
    for (const QString &seg : parts) {
        if (seg == QLatin1String(".") || seg.isEmpty())
            continue;
        if (seg == QLatin1String(".."))
            return {}; // path traversal
        // Disallow control chars / null
        for (const QChar c : seg) {
            if (c.unicode() < 0x20)
                return {};
        }
        clean.append(seg);
    }
    if (clean.isEmpty())
        return {};
    return clean.join(QLatin1Char('/'));
}

bool isSafeRelativePath(const QString &userPath) {
    return !sanitizeRelativePath(userPath).isEmpty();
}

bool isUnselectedSubject(const QString &nameOrPath) {
    if (nameOrPath.isEmpty())
        return true;
    // YA + NUKTA (U+09AF U+09BC) vs single YYA (U+09DF) — both appear in UI strings.
    static const QString a = QStringLiteral("নির্বাচিত নয়");
    static const QString b = QStringLiteral("নির্বাচিত নয়");
    return nameOrPath == a || nameOrPath == b;
}

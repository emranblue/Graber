#include "ConfigPaths.h"
#include "Utils.h"

#include <QDir>
#include <QFile>
#include <QFileInfo>
#include <QStandardPaths>
#include <QDesktopServices>
#include <QUrl>

namespace ConfigPaths {

QString notesRoot() {
    return QStandardPaths::writableLocation(QStandardPaths::HomeLocation)
         + QDir::separator() + QStringLiteral("GraberNotes");
}

QString configDir() {
    return notesRoot() + QDir::separator() + QStringLiteral("config");
}

QString templatesJsonPath() {
    return configDir() + QDir::separator() + QStringLiteral("templates.json");
}

QString shortcutTemplatesJsonPath() {
    return configDir() + QDir::separator() + QStringLiteral("shortcut_templates.json");
}

static bool copyResourceIfMissing(const QString &resourcePath, const QString &destPath) {
    QFileInfo destInfo(destPath);
    if (destInfo.exists() && destInfo.size() > 0)
        return true;

    QDir dir(destInfo.absolutePath());
    if (!dir.exists() && !dir.mkpath(QStringLiteral("."))) {
        debugLog(QStringLiteral("ConfigPaths: cannot create dir %1").arg(destInfo.absolutePath()));
        return false;
    }

    QFile src(resourcePath);
    if (!src.open(QIODevice::ReadOnly)) {
        debugLog(QStringLiteral("ConfigPaths: missing resource %1").arg(resourcePath));
        return false;
    }
    const QByteArray data = src.readAll();
    src.close();

    QFile out(destPath);
    if (!out.open(QIODevice::WriteOnly | QIODevice::Truncate)) {
        debugLog(QStringLiteral("ConfigPaths: cannot write %1").arg(destPath));
        return false;
    }
    out.write(data);
    out.close();
    debugLog(QStringLiteral("ConfigPaths: seeded %1 from %2").arg(destPath, resourcePath));
    return true;
}

bool ensureUserConfigFiles() {
    QDir dir(configDir());
    if (!dir.exists())
        dir.mkpath(QStringLiteral("."));

    const bool okTpl = copyResourceIfMissing(
        QStringLiteral(":/config/templates.json"), templatesJsonPath());
    const bool okSc = copyResourceIfMissing(
        QStringLiteral(":/config/shortcut_templates.json"), shortcutTemplatesJsonPath());
    return okTpl && okSc;
}

bool openPathInSystem(const QString &path) {
    if (path.isEmpty())
        return false;
    return QDesktopServices::openUrl(QUrl::fromLocalFile(path));
}

} // namespace ConfigPaths

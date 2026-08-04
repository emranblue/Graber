#include "FileIO.h"
#include "Utils.h"

#include <QFile>
#include <QFileInfo>
#include <QDir>
#include <QSaveFile>
#include <QTextStream>

namespace FileIO {

bool ensureParentDir(const QString &filePath) {
    if (filePath.isEmpty())
        return false;
    QFileInfo fi(filePath);
    const QString dirPath = fi.absolutePath();
    if (dirPath.isEmpty())
        return false;
    QDir dir(dirPath);
    if (dir.exists())
        return true;
    return dir.mkpath(QStringLiteral("."));
}

bool isReadableFile(const QString &filePath) {
    if (filePath.isEmpty())
        return false;
    QFileInfo fi(filePath);
    return fi.exists() && fi.isFile() && fi.isReadable();
}

Result<QString> readText(const QString &filePath) {
    if (filePath.isEmpty())
        return Result<QString>::fail(QStringLiteral("Empty file path"));

    // Missing file is a soft failure (new note) — return empty content, not error.
    if (!QFileInfo::exists(filePath))
        return Result<QString>::ok(QString());

    QFile file(filePath);
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        const QString msg = QStringLiteral("Cannot open for read: %1 (%2)")
                                .arg(filePath, file.errorString());
        debugLog(msg);
        return Result<QString>::fail(msg);
    }

    QTextStream in(&file);
#if QT_VERSION < QT_VERSION_CHECK(6, 0, 0)
    in.setCodec("UTF-8");
#endif
    const QString content = in.readAll();
    file.close();
    return Result<QString>::ok(content);
}

VoidResult writeTextAtomic(const QString &filePath, const QString &content) {
    if (filePath.isEmpty())
        return VoidResult::fail(QStringLiteral("Empty file path"));

    if (!ensureParentDir(filePath)) {
        const QString msg = QStringLiteral("Cannot create parent dir for: %1").arg(filePath);
        debugLog(msg);
        return VoidResult::fail(msg);
    }

    // QSaveFile writes to a temp name then renames — crash-safe.
    QSaveFile file(filePath);
    if (!file.open(QIODevice::WriteOnly | QIODevice::Text)) {
        const QString msg = QStringLiteral("Cannot open for atomic write: %1 (%2)")
                                .arg(filePath, file.errorString());
        debugLog(msg);
        return VoidResult::fail(msg);
    }

    QTextStream out(&file);
#if QT_VERSION < QT_VERSION_CHECK(6, 0, 0)
    out.setCodec("UTF-8");
#endif
    out << content;
    out.flush();

    if (out.status() != QTextStream::Ok) {
        file.cancelWriting();
        const QString msg = QStringLiteral("Stream write failed: %1").arg(filePath);
        debugLog(msg);
        return VoidResult::fail(msg);
    }

    if (!file.commit()) {
        const QString msg = QStringLiteral("Atomic commit failed: %1 (%2)")
                                .arg(filePath, file.errorString());
        debugLog(msg);
        return VoidResult::fail(msg);
    }

    return VoidResult::ok();
}

VoidResult appendTextAtomic(const QString &filePath, const QString &chunk) {
    auto existing = readText(filePath);
    if (existing.isFail())
        return VoidResult::fail(existing.message());

    QString out = existing.value();
    out += chunk;
    return writeTextAtomic(filePath, out);
}

} // namespace FileIO

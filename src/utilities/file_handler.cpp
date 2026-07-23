#include "file_handler.h"
#include "logger.h"
#include <QFile>
#include <QTextStream>
#include <QDir>
#include <QDirIterator>
#include <QFileInfo>

bool FileHandler::ensureDirectoryExists(const QString &path) {
    QDir dir(path);
    if (!dir.exists()) {
        return dir.mkpath(".");
    }
    return true;
}

bool FileHandler::fileExists(const QString &path) {
    return QFile::exists(path);
}

QString FileHandler::readFile(const QString &path) {
    QFile file(path);
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        Logger::error(QString("Failed to open file: %1").arg(path));
        return "";
    }
    QTextStream in(&file);
    QString content = in.readAll();
    file.close();
    return content;
}

bool FileHandler::writeFile(const QString &path, const QString &content) {
    QFile file(path);
    if (!file.open(QIODevice::WriteOnly | QIODevice::Truncate | QIODevice::Text)) {
        Logger::error(QString("Failed to write file: %1").arg(path));
        return false;
    }
    QTextStream out(&file);
    out << content << "\n";
    file.close();
    return true;
}

bool FileHandler::appendFile(const QString &path, const QString &content) {
    QFile file(path);
    if (!file.open(QIODevice::WriteOnly | QIODevice::Append | QIODevice::Text)) {
        Logger::error(QString("Failed to append to file: %1").arg(path));
        return false;
    }
    QTextStream out(&file);
    out << content;
    file.close();
    return true;
}

QStringList FileHandler::getAllMarkdownFiles(const QString &dirPath) {
    QStringList results;
    QDirIterator it(dirPath, QStringList() << "*.md", QDir::Files, QDirIterator::Subdirectories);
    QDir baseDir(dirPath);
    
    while (it.hasNext()) {
        QString filepath = it.next();
        QString relativePath = baseDir.relativeFilePath(filepath);
        
        if (relativePath.startsWith(QString("deleted") + QDir::separator()) || relativePath == "deleted") {
            continue;
        }
        relativePath.chop(3);
        results << relativePath;
    }
    
    results.sort(Qt::CaseInsensitive);
    return results;
}

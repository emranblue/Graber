#ifndef FILE_HANDLER_H
#define FILE_HANDLER_H

#include <QString>
#include <QStringList>

class FileHandler {
public:
    static bool ensureDirectoryExists(const QString &path);
    static bool fileExists(const QString &path);
    static QString readFile(const QString &path);
    static bool writeFile(const QString &path, const QString &content);
    static bool appendFile(const QString &path, const QString &content);
    static QStringList getAllMarkdownFiles(const QString &dirPath);
};

#endif // FILE_HANDLER_H

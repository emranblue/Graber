#ifndef LOGGER_H
#define LOGGER_H

#include <QString>

class Logger {
public:
    static void debug(const QString &msg);
    static void error(const QString &msg);
    static void info(const QString &msg);
    
private:
    static QString getLogFilePath();
};

#endif // LOGGER_H

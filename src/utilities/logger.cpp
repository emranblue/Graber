#include "logger.h"
#include <QFile>
#include <QTextStream>
#include <QDateTime>
#include <QDir>
#include <iostream>

void Logger::debug(const QString &msg) {
    QFile file(getLogFilePath());
    if (file.open(QIODevice::WriteOnly | QIODevice::Append | QIODevice::Text)) {
        QTextStream out(&file);
        out << QDateTime::currentDateTime().toString("yyyy-MM-dd hh:mm:ss")
            << " [DEBUG] " << msg << "\n";
        file.close();
    }
}

void Logger::error(const QString &msg) {
    std::cerr << "[ERROR] " << msg.toStdString() << std::endl;
    debug(msg);
}

void Logger::info(const QString &msg) {
    debug(msg);
}

QString Logger::getLogFilePath() {
    return QDir::homePath() + "/GraberNotes/debug.log";
}

#include "CrashGuard.h"

#include <QCoreApplication>
#include <QDateTime>
#include <QDir>
#include <QFile>
#include <QStandardPaths>
#include <QTextStream>
#include <exception>
#include <iostream>

namespace {

void writeCrashLine(const QString &line) {
    // Always try debug.log; also stderr for packagers / terminal users.
    debugLog(line);
    const QByteArray utf8 = line.toUtf8();
    std::cerr.write(utf8.constData(), utf8.size());
    std::cerr << '\n';
}

void qtMessageHandler(QtMsgType type, const QMessageLogContext &ctx, const QString &msg) {
    const char *level = "INFO";
    switch (type) {
    case QtDebugMsg:    level = "DEBUG"; break;
    case QtInfoMsg:     level = "INFO";  break;
    case QtWarningMsg:  level = "WARN";  break;
    case QtCriticalMsg: level = "CRIT";  break;
    case QtFatalMsg:    level = "FATAL"; break;
    }
    QString line = QStringLiteral("[%1] %2").arg(QLatin1String(level), msg);
    if (ctx.file && ctx.file[0]) {
        line += QStringLiteral(" (%1:%2)").arg(QLatin1String(ctx.file)).arg(ctx.line);
    }
    writeCrashLine(line);
    if (type == QtFatalMsg) {
        // Let Qt abort after we logged; avoid silent death.
        std::abort();
    }
}

[[noreturn]] void terminateHandler() {
    writeCrashLine(QStringLiteral("FATAL: std::terminate called"));
    try {
        if (auto ep = std::current_exception()) {
            try {
                std::rethrow_exception(ep);
            } catch (const std::exception &ex) {
                writeCrashLine(QStringLiteral("  nested: %1").arg(QString::fromUtf8(ex.what())));
            } catch (...) {
                writeCrashLine(QStringLiteral("  nested: unknown exception type"));
            }
        }
    } catch (...) {
        // never throw from terminate handler
    }
    std::abort();
}

} // namespace

namespace CrashGuard {

void installGlobalHandlers() {
    static bool installed = false;
    if (installed)
        return;
    installed = true;

    qInstallMessageHandler(qtMessageHandler);
    std::set_terminate(terminateHandler);

    debugLog(QStringLiteral("CrashGuard handlers installed"));
}

void reportToUser(QWidget *parent, const QString &title, const QString &detail) {
    debugLog(QStringLiteral("USER_REPORT: %1 — %2").arg(title, detail));
    if (!parent)
        return;
    // Non-modal would be nicer, but keep simple and safe.
    QMessageBox::warning(parent, title, detail);
}

} // namespace CrashGuard

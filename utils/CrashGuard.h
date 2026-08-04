#ifndef GRABER_CRASHGUARD_H
#define GRABER_CRASHGUARD_H

#include <QString>
#include <QMessageBox>
#include <QWidget>
#include <functional>
#include <exception>
#include <cstdlib>
#include "Utils.h"

/**
 * Crash / exception hardening helpers.
 *
 * Qt slots and most UI paths cannot safely throw across the event loop.
 * Prefer SafeCall / SafeCallVoid around any code that touches disk, regex
 * on untrusted content, or third-party-ish operations.
 */
namespace CrashGuard {

/** Install process-wide terminate + Qt message handlers (call once from main). */
void installGlobalHandlers();

/**
 * Run callable; swallow std::exception and unknown exceptions.
 * Returns true on success, false if an exception was caught (logged).
 */
template <typename Fn>
bool safeCall(Fn &&fn, const QString &context = QString()) {
    try {
        fn();
        return true;
    } catch (const std::exception &ex) {
        const QString msg = QStringLiteral("[%1] exception: %2")
                                .arg(context.isEmpty() ? QStringLiteral("SafeCall") : context,
                                     QString::fromUtf8(ex.what()));
        debugLog(msg);
        return false;
    } catch (...) {
        const QString msg = QStringLiteral("[%1] unknown exception")
                                .arg(context.isEmpty() ? QStringLiteral("SafeCall") : context);
        debugLog(msg);
        return false;
    }
}

/**
 * Same as safeCall but returns a default value of type T on failure.
 */
template <typename T, typename Fn>
T safeCallValue(Fn &&fn, T fallback, const QString &context = QString()) {
    try {
        return fn();
    } catch (const std::exception &ex) {
        debugLog(QStringLiteral("[%1] exception: %2")
                     .arg(context.isEmpty() ? QStringLiteral("SafeCallValue") : context,
                          QString::fromUtf8(ex.what())));
        return fallback;
    } catch (...) {
        debugLog(QStringLiteral("[%1] unknown exception")
                     .arg(context.isEmpty() ? QStringLiteral("SafeCallValue") : context));
        return fallback;
    }
}

/** Optional user-visible error (non-blocking status preferred; dialog only for severe). */
void reportToUser(QWidget *parent, const QString &title, const QString &detail);

} // namespace CrashGuard

#endif // GRABER_CRASHGUARD_H

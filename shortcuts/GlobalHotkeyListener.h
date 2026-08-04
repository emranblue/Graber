#ifndef GLOBALHOTKEYLISTENER_H
#define GLOBALHOTKEYLISTENER_H

#include "core/QtFixes.h"
#include <QObject>
#include <QKeySequence>
#include <QString>
#include <QtGlobal>
#include <QAbstractNativeEventFilter>
#include <QMap>
#include <QList>

class QSocketNotifier;

/**
 * Platform-specific system-wide hotkey grabber (Windows RegisterHotKey /
 * Linux X11 XGrabKey). Kept in its own translation unit so Xlib / windows.h
 * macros never pollute ShortcutManager or the rest of the app.
 */
class GlobalHotkeyListener : public QObject {
    Q_OBJECT
public:
    explicit GlobalHotkeyListener(QObject *parent = nullptr);
    ~GlobalHotkeyListener() override;

    void registerGlobalHotkey(const QKeySequence &keySeq, const QString &actionId);
    void unregisterAllHotkeys();
    bool isSupported() const { return is_supported_; }

signals:
    void globalHotkeyPressed(const QString &actionId);

private:
    bool is_supported_;

#ifdef Q_OS_WIN
    class NativeEventFilter : public QAbstractNativeEventFilter {
    public:
        explicit NativeEventFilter(GlobalHotkeyListener *parent);
#if QT_VERSION >= QT_VERSION_CHECK(6, 0, 0)
        bool nativeEventFilter(const QByteArray &eventType, void *message, qintptr *result) override;
#else
        bool nativeEventFilter(const QByteArray &eventType, void *message, long *result) override;
#endif
    private:
        GlobalHotkeyListener *parent_;
    };

    struct HotkeyData {
        int id;
        QString actionId;
    };

    QMap<int, HotkeyData> registered_hotkeys_;
    int next_hotkey_id_;
    NativeEventFilter *native_filter_;
#endif

#ifdef Q_OS_LINUX
private slots:
    void on_x11_activity();

private:
    struct X11HotkeyData {
        unsigned int keycode;
        unsigned int modifiers;
        QString actionId;
    };

    // Opaque handles — real Display*/Window only live in the .cpp.
    void *x11_display_;
    unsigned long x11_root_;
    QSocketNotifier *x11_notifier_;
    QList<X11HotkeyData> x11_hotkeys_;
#endif
};

#endif // GLOBALHOTKEYLISTENER_H

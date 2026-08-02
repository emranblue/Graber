#ifndef SHORTCUTMANAGER_H
#define SHORTCUTMANAGER_H

#include "core/QtFixes.h"
#include <QObject>
#include <QList>
#include <QSettings>
#include <QWidget>
#include <QShortcut>
#include <QAbstractNativeEventFilter>
#include <QMap>
#include "Types.h"

// Platform-specific includes were removed from the header to avoid polluting
// the global macro namespace (X11/windows.h defines macros like "None" that
// conflict with Qt headers). Includes live in the .cpp file instead.

class QSocketNotifier;

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
        bool nativeEventFilter(const QByteArray &eventType, void *message, long *result) override;
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

    // Kept as opaque handles here so Xlib's macro-heavy headers
    // (which redefine names like "None", "Bool", "True") never have to be
    // included in this header. The real types (Display*, Window) are used
    // only inside ShortcutManager.cpp.
    void *x11_display_;
    unsigned long x11_root_;
    QSocketNotifier *x11_notifier_;
    QList<X11HotkeyData> x11_hotkeys_;
#endif
};

class ShortcutManager : public QObject {
    Q_OBJECT

public:
    explicit ShortcutManager(QObject *parent = nullptr);
    ~ShortcutManager();

    void initDefaultConfigs();
    void loadSettings(const QString &settingsFilePath);
    void saveSettings(const QString &settingsFilePath);
    void setupShortcuts(QWidget *parentWidget);
    void enableGlobalHotkeys(bool enable = true);

    // Whether global (system-wide) hotkeys are currently turned on, and
    // whether the current platform/session actually supports them.
    bool globalHotkeysEnabled() const { return global_hotkeys_enabled_; }
    bool globalHotkeysSupported() const;

    QList<ShortcutConfig>& configs();
    const QList<ShortcutConfig>& configs() const;

signals:
    void actionTriggered(const QString &actionId);

private slots:
    void on_global_hotkey_pressed(const QString &actionId);

private:
    QList<ShortcutConfig> configs_;
    GlobalHotkeyListener *global_hotkey_listener_;
    bool global_hotkeys_enabled_;
};

#endif // SHORTCUTMANAGER_H

#ifndef SHORTCUTMANAGER_H
#define SHORTCUTMANAGER_H

#include <QObject>
#include <QList>
#include <QSettings>
#include <QWidget>
#include <QShortcut>
#include <QAbstractNativeEventFilter>
#include "Types.h"

#ifdef Q_OS_LINUX
#include <X11/Xlib.h>
#endif

#ifdef Q_OS_WIN
#include <windows.h>
#endif

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

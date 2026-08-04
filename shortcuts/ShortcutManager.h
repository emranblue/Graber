#ifndef SHORTCUTMANAGER_H
#define SHORTCUTMANAGER_H

#include "core/QtFixes.h"
#include <QObject>
#include <QList>
#include <QWidget>
#include <QMap>
#include <QElapsedTimer>
#include "Types.h"

class GlobalHotkeyListener;

/**
 * Owns per-action ShortcutConfig list, local QShortcut wiring, and optional
 * system-wide GlobalHotkeyListener. Per-config `enabled` flag controls
 * whether that shortcut is active anywhere (local + global).
 */
class ShortcutManager : public QObject {
    Q_OBJECT

public:
    explicit ShortcutManager(QObject *parent = nullptr);
    ~ShortcutManager() override;

    void initDefaultConfigs();
    void loadSettings(const QString &settingsFilePath);
    void saveSettings(const QString &settingsFilePath);
    void setupShortcuts(QWidget *parentWidget);
    void enableGlobalHotkeys(bool enable = true);

    bool globalHotkeysEnabled() const { return global_hotkeys_enabled_; }
    bool globalHotkeysSupported() const;

    QList<ShortcutConfig>& configs();
    const QList<ShortcutConfig>& configs() const;

    /** True if this action's shortcut is allowed to fire (user toggle). */
    bool isShortcutEnabled(const QString &actionId) const;

signals:
    void actionTriggered(const QString &actionId);

private slots:
    void on_global_hotkey_pressed(const QString &actionId);

private:
    bool shouldFireAction(const QString &actionId);

    QList<ShortcutConfig> configs_;
    GlobalHotkeyListener *global_hotkey_listener_;
    bool global_hotkeys_enabled_;
    QElapsedTimer dedup_timer_;
    QMap<QString, qint64> last_fired_ms_;
};

#endif // SHORTCUTMANAGER_H

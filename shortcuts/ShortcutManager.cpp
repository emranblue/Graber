#include "ShortcutManager.h"
#include "GlobalHotkeyListener.h"
#include "ActionRegistry.h"

#include <QSettings>
#include <QShortcut>

ShortcutManager::ShortcutManager(QObject *parent)
    : QObject(parent), global_hotkeys_enabled_(false) {
    global_hotkey_listener_ = new GlobalHotkeyListener(this);
    connect(global_hotkey_listener_, &GlobalHotkeyListener::globalHotkeyPressed,
            this, &ShortcutManager::on_global_hotkey_pressed);
    initDefaultConfigs();
    dedup_timer_.start();
}

ShortcutManager::~ShortcutManager() {
    for (auto &cfg : configs_) {
        if (cfg.shortcut_obj) {
            delete cfg.shortcut_obj;
            cfg.shortcut_obj = nullptr;
        }
    }
}

bool ShortcutManager::shouldFireAction(const QString &actionId) {
    if (!isShortcutEnabled(actionId))
        return false;

    const qint64 now = dedup_timer_.elapsed();
    const qint64 kDedupWindowMs = 200;
    auto it = last_fired_ms_.find(actionId);
    if (it != last_fired_ms_.end() && (now - it.value()) < kDedupWindowMs)
        return false;
    last_fired_ms_[actionId] = now;
    return true;
}

bool ShortcutManager::isShortcutEnabled(const QString &actionId) const {
    for (const auto &cfg : configs_) {
        if (cfg.action_id == actionId)
            return cfg.enabled;
    }
    return true; // unknown ids default to on
}

void ShortcutManager::initDefaultConfigs() {
    // Preserve user key + enabled state across re-sync with ActionRegistry.
    QMap<QString, QKeySequence> prev_keys;
    QMap<QString, bool> prev_enabled;
    for (const auto &cfg : configs_) {
        prev_keys[cfg.action_id] = cfg.current_key;
        prev_enabled[cfg.action_id] = cfg.enabled;
    }

    configs_.clear();
    auto registered_actions = ActionRegistry::instance().getAllActions();

    for (const auto &action : registered_actions) {
        ShortcutConfig cfg;
        cfg.action_id = action->id();
        cfg.name_bangla = action->displayName();
        cfg.name_english = action->description();
        cfg.default_key = action->defaultShortcut().toString();
        cfg.current_key = prev_keys.contains(cfg.action_id)
            ? prev_keys[cfg.action_id]
            : action->defaultShortcut();
        cfg.enabled = prev_enabled.contains(cfg.action_id)
            ? prev_enabled[cfg.action_id]
            : true;
        cfg.shortcut_obj = nullptr;
        configs_.append(cfg);
    }
}

void ShortcutManager::loadSettings(const QString &settingsFilePath) {
    initDefaultConfigs();

    QSettings settings(settingsFilePath, QSettings::IniFormat);

    settings.beginGroup("Shortcuts");
    for (auto &cfg : configs_) {
        QString key_str = settings.value(cfg.action_id, cfg.default_key).toString();
        cfg.current_key = QKeySequence(key_str);
    }
    settings.endGroup();

    settings.beginGroup("ShortcutEnabled");
    for (auto &cfg : configs_) {
        cfg.enabled = settings.value(cfg.action_id, true).toBool();
    }
    settings.endGroup();

    global_hotkeys_enabled_ = settings.value("General/GlobalHotkeys", true).toBool();
}

void ShortcutManager::saveSettings(const QString &settingsFilePath) {
    QSettings settings(settingsFilePath, QSettings::IniFormat);

    settings.beginGroup("Shortcuts");
    for (const auto &cfg : configs_) {
        settings.setValue(cfg.action_id, cfg.current_key.toString());
    }
    settings.endGroup();

    settings.beginGroup("ShortcutEnabled");
    for (const auto &cfg : configs_) {
        settings.setValue(cfg.action_id, cfg.enabled);
    }
    settings.endGroup();

    settings.setValue("General/GlobalHotkeys", global_hotkeys_enabled_);
}

void ShortcutManager::setupShortcuts(QWidget *parentWidget) {
    for (auto &cfg : configs_) {
        if (cfg.shortcut_obj) {
            delete cfg.shortcut_obj;
            cfg.shortcut_obj = nullptr;
        }
    }

    for (auto &cfg : configs_) {
        if (!cfg.enabled || cfg.current_key.isEmpty())
            continue;

        cfg.shortcut_obj = new QShortcut(cfg.current_key, parentWidget);
        cfg.shortcut_obj->setContext(Qt::ApplicationShortcut);
        cfg.shortcut_obj->setAutoRepeat(false);
        const QString action_id = cfg.action_id;
        connect(cfg.shortcut_obj, &QShortcut::activated, this, [this, action_id]() {
            if (!shouldFireAction(action_id)) return;
            if (ActionRegistry::instance().isActionEnabled(action_id)) {
                ActionRegistry::instance().executeAction(action_id);
                emit actionTriggered(action_id);
            }
        });
    }

    global_hotkey_listener_->unregisterAllHotkeys();
    if (global_hotkeys_enabled_ && global_hotkey_listener_->isSupported()) {
        for (const auto &cfg : configs_) {
            if (cfg.enabled && !cfg.current_key.isEmpty())
                global_hotkey_listener_->registerGlobalHotkey(cfg.current_key, cfg.action_id);
        }
    }
}

void ShortcutManager::enableGlobalHotkeys(bool enable) {
    global_hotkeys_enabled_ = enable;
    global_hotkey_listener_->unregisterAllHotkeys();
    if (enable && global_hotkey_listener_->isSupported()) {
        for (const auto &cfg : configs_) {
            if (cfg.enabled && !cfg.current_key.isEmpty())
                global_hotkey_listener_->registerGlobalHotkey(cfg.current_key, cfg.action_id);
        }
    }
}

bool ShortcutManager::globalHotkeysSupported() const {
    return global_hotkey_listener_->isSupported();
}

void ShortcutManager::on_global_hotkey_pressed(const QString &actionId) {
    if (!shouldFireAction(actionId)) return;
    if (ActionRegistry::instance().isActionEnabled(actionId)) {
        ActionRegistry::instance().executeAction(actionId);
        emit actionTriggered(actionId);
    }
}

QList<ShortcutConfig>& ShortcutManager::configs() {
    return configs_;
}

const QList<ShortcutConfig>& ShortcutManager::configs() const {
    return configs_;
}

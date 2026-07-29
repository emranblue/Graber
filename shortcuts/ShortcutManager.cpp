#include "ShortcutManager.h"
#include "ActionRegistry.h"
#include <QDir>

ShortcutManager::ShortcutManager(QObject *parent) : QObject(parent) {
    initDefaultConfigs();
}

ShortcutManager::~ShortcutManager() {
    for (auto &cfg : configs_) {
        if (cfg.shortcut_obj) {
            delete cfg.shortcut_obj;
            cfg.shortcut_obj = nullptr;
        }
    }
}

void ShortcutManager::initDefaultConfigs() {
    configs_.clear();
    auto registered_actions = ActionRegistry::instance().getAllActions();
    
    for (const auto &action : registered_actions) {
        ShortcutConfig cfg;
        cfg.action_id = action->id();
        cfg.name_bangla = action->displayName();
        cfg.name_english = action->description();
        cfg.default_key = action->defaultShortcut().toString();
        cfg.current_key = action->defaultShortcut();
        cfg.shortcut_obj = nullptr;
        configs_.append(cfg);
    }
}

void ShortcutManager::loadSettings(const QString &settingsFilePath) {
    // Sync with ActionRegistry first to pick up any new actions/wizards
    initDefaultConfigs();

    QSettings settings(settingsFilePath, QSettings::IniFormat);
    settings.beginGroup("Shortcuts");
    for (auto &cfg : configs_) {
        QString key_str = settings.value(cfg.action_id, cfg.default_key).toString();
        cfg.current_key = QKeySequence(key_str);
    }
    settings.endGroup();
}

void ShortcutManager::saveSettings(const QString &settingsFilePath) {
    QSettings settings(settingsFilePath, QSettings::IniFormat);
    settings.beginGroup("Shortcuts");
    for (const auto &cfg : configs_) {
        settings.setValue(cfg.action_id, cfg.current_key.toString());
    }
    settings.endGroup();
}

void ShortcutManager::setupShortcuts(QWidget *parentWidget) {
    for (auto &cfg : configs_) {
        if (cfg.shortcut_obj) {
            delete cfg.shortcut_obj;
            cfg.shortcut_obj = nullptr;
        }
    }
    
    for (auto &cfg : configs_) {
        if (!cfg.current_key.isEmpty()) {
            cfg.shortcut_obj = new QShortcut(cfg.current_key, parentWidget);
            cfg.shortcut_obj->setContext(Qt::ApplicationShortcut);
            cfg.shortcut_obj->setAutoRepeat(false);
            QString action_id = cfg.action_id;
            connect(cfg.shortcut_obj, &QShortcut::activated, this, [this, action_id]() {
                if (ActionRegistry::instance().isActionEnabled(action_id)) {
                    ActionRegistry::instance().executeAction(action_id);
                    emit actionTriggered(action_id);
                }
            });
        }
    }
}

QList<ShortcutConfig>& ShortcutManager::configs() {
    return configs_;
}

const QList<ShortcutConfig>& ShortcutManager::configs() const {
    return configs_;
}

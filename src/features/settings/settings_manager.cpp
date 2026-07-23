#include "settings_manager.h"
#include "../../utilities/logger.h"
#include <QSettings>
#include <QDir>

SettingsManager::SettingsManager(const QString &notesDir)
    : notes_dir_path_(notesDir) {
}

QString SettingsManager::getSettingsFilePath() const {
    return notes_dir_path_ + QDir::separator() + "settings.ini";
}

void SettingsManager::saveShortcutSettings(const QList<ShortcutConfig> &configs) {
    QSettings settings(getSettingsFilePath(), QSettings::IniFormat);
    settings.beginGroup("Shortcuts");
    settings.remove(""); // Clear old settings
    
    for (const auto &cfg : configs) {
        settings.setValue(cfg.action_id, cfg.current_key.toString());
    }
    settings.endGroup();
    Logger::info("Shortcut settings saved");
}

void SettingsManager::loadShortcutSettings(QList<ShortcutConfig> &configs) {
    QSettings settings(getSettingsFilePath(), QSettings::IniFormat);
    settings.beginGroup("Shortcuts");
    
    for (auto &cfg : configs) {
        QString key_str = settings.value(cfg.action_id, cfg.default_key).toString();
        cfg.current_key = QKeySequence(key_str);
    }
    settings.endGroup();
    Logger::info("Shortcut settings loaded");
}

#include "shortcut_manager.h"
#include "../../utilities/logger.h"
#include <QSettings>
#include <QDir>

ShortcutManager::ShortcutManager(const QString &notesDir, QObject *parent)
    : QObject(parent), notes_dir_path_(notesDir), target_widget_(nullptr) {
    initializeShortcuts();
}

ShortcutManager::~ShortcutManager() {
    for (auto &cfg : shortcut_configs_) {
        if (cfg.shortcut_obj) {
            delete cfg.shortcut_obj;
            cfg.shortcut_obj = nullptr;
        }
    }
}

void ShortcutManager::initializeShortcuts() {
    shortcut_configs_ = {
        {"start", "শুরু করুন (Start)", "Start Monitoring", "Ctrl+Shift+S", QKeySequence("Ctrl+Shift+S"), nullptr},
        {"stop", "থামুন (Stop)", "Stop Monitoring", "Ctrl+Shift+T", QKeySequence("Ctrl+Shift+T"), nullptr},
        {"add_image", "ছবি যুক্ত করুন (Add Image)", "Add Clipboard Image", "Ctrl+Shift+I", QKeySequence("Ctrl+Shift+I"), nullptr},
        {"new_subject", "নতুন বিষয় (New Subject)", "Create New Subject", "Ctrl+Shift+N", QKeySequence("Ctrl+Shift+N"), nullptr},
        {"open_note", "নোট খুলুন (Open Note)", "Open Active Note", "Ctrl+Shift+O", QKeySequence("Ctrl+Shift+O"), nullptr},
        {"append", "যুক্ত করুন (Append)", "Append to Heading", "Ctrl+Shift+A", QKeySequence("Ctrl+Shift+A"), nullptr},
        {"inject", "ইনজেক্ট করুন (Inject)", "Inject Heading", "Ctrl+Shift+J", QKeySequence("Ctrl+Shift+J"), nullptr},
        {"shift", "স্থানান্তর (Shift)", "Shift Heading Section", "Ctrl+Shift+H", QKeySequence("Ctrl+Shift+H"), nullptr},
        {"delete", "মুছে ফেলুন (Delete)", "Delete Heading Section", "Ctrl+Shift+D", QKeySequence("Ctrl+Shift+D"), nullptr},
        {"new_section", "নতুন বিভাগ (New Section)", "Add Custom Section", "Ctrl+Shift+K", QKeySequence("Ctrl+Shift+K"), nullptr},
        {"toggle_format", "ফরম্যাট পরিবর্তন (Toggle Format)", "Toggle Capture Format", "Ctrl+Shift+F", QKeySequence("Ctrl+Shift+F"), nullptr},
        {"toggle_section", "বিভাগ পরিবর্তন (Toggle Section)", "Toggle Section Category", "Ctrl+Shift+G", QKeySequence("Ctrl+Shift+G"), nullptr},
        {"toggle_subject", "বিষয় পরিবর্তন (Toggle Subject)", "Toggle Subject Selection", "Ctrl+Shift+E", QKeySequence("Ctrl+Shift+E"), nullptr}
    };
}

void ShortcutManager::setupShortcuts(QObject *target) {
    target_widget_ = target;
    
    // Delete existing shortcut objects
    for (auto &cfg : shortcut_configs_) {
        if (cfg.shortcut_obj) {
            delete cfg.shortcut_obj;
            cfg.shortcut_obj = nullptr;
        }
    }
    
    // Create new ones
    for (auto &cfg : shortcut_configs_) {
        if (!cfg.current_key.isEmpty() && target) {
            cfg.shortcut_obj = new QShortcut(cfg.current_key, qobject_cast<QWidget*>(target));
            QString action_id = cfg.action_id;
            connect(cfg.shortcut_obj, &QShortcut::activated, this, [this, action_id]() {
                emit shortcutTriggered(action_id);
            });
        }
    }
    
    Logger::info("Shortcuts setup complete");
}

void ShortcutManager::loadSettings() {
    QSettings settings(notes_dir_path_ + QDir::separator() + "settings.ini", QSettings::IniFormat);
    settings.beginGroup("Shortcuts");
    for (auto &cfg : shortcut_configs_) {
        QString key_str = settings.value(cfg.action_id, cfg.default_key).toString();
        cfg.current_key = QKeySequence(key_str);
    }
    settings.endGroup();
    Logger::info("Shortcuts loaded from settings");
}

void ShortcutManager::saveSettings() {
    QSettings settings(notes_dir_path_ + QDir::separator() + "settings.ini", QSettings::IniFormat);
    settings.beginGroup("Shortcuts");
    for (const auto &cfg : shortcut_configs_) {
        settings.setValue(cfg.action_id, cfg.current_key.toString());
    }
    settings.endGroup();
    Logger::info("Shortcuts saved to settings");
}

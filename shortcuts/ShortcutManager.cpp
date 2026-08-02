#include "ShortcutManager.h"
#include "ActionRegistry.h"
#include <QDir>
#include <QApplication>
#include <QGuiApplication>

#ifdef Q_OS_WIN
#include <windows.h>
#endif

// ============================================================================
// GlobalHotkeyListener Implementation
// ============================================================================

#ifdef Q_OS_WIN
GlobalHotkeyListener::NativeEventFilter::NativeEventFilter(GlobalHotkeyListener *parent)
    : parent_(parent) {}

bool GlobalHotkeyListener::NativeEventFilter::nativeEventFilter(const QByteArray &eventType, void *message, long *result) {
    Q_UNUSED(result);
    if (eventType == "windows_generic_MSG") {
        MSG *msg = static_cast<MSG *>(message);
        if (msg->message == WM_HOTKEY) {
            int hotkeyId = static_cast<int>(msg->wParam);
            if (parent_->registered_hotkeys_.contains(hotkeyId)) {
                QString actionId = parent_->registered_hotkeys_[hotkeyId].actionId;
                emit parent_->globalHotkeyPressed(actionId);
                return true;
            }
        }
    }
    return false;
}
#endif

GlobalHotkeyListener::GlobalHotkeyListener(QObject *parent)
    : QObject(parent), is_supported_(true) {
#ifdef Q_OS_WIN
    next_hotkey_id_ = 1;
    native_filter_ = new NativeEventFilter(this);
    qApp->installNativeEventFilter(native_filter_);
#endif

#ifdef Q_OS_LINUX
    is_supported_ = false; // Fallback for Linux (would need X11/Wayland support)
#endif

#ifdef Q_OS_MAC
    is_supported_ = false; // Fallback for macOS (would need Cocoa support)
#endif
}

GlobalHotkeyListener::~GlobalHotkeyListener() {
    unregisterAllHotkeys();
#ifdef Q_OS_WIN
    if (native_filter_) {
        qApp->removeNativeEventFilter(native_filter_);
        delete native_filter_;
    }
#endif
}

void GlobalHotkeyListener::registerGlobalHotkey(const QKeySequence &keySeq, const QString &actionId) {
    if (keySeq.isEmpty()) return;

#ifdef Q_OS_WIN
    // Convert QKeySequence to Windows VK codes
    int key = keySeq[0];
    UINT modifiers = 0;
    UINT vk = 0;

    // Extract modifiers
    if (key & Qt::CTRL) modifiers |= MOD_CONTROL;
    if (key & Qt::ALT) modifiers |= MOD_ALT;
    if (key & Qt::SHIFT) modifiers |= MOD_SHIFT;
    if (key & Qt::META) modifiers |= MOD_WIN;

    // Extract actual key
    int actualKey = key & 0xFFFF;

    // Map Qt key codes to Windows VK codes
    switch (actualKey) {
        case Qt::Key_A: vk = 'A'; break;
        case Qt::Key_B: vk = 'B'; break;
        case Qt::Key_C: vk = 'C'; break;
        case Qt::Key_D: vk = 'D'; break;
        case Qt::Key_E: vk = 'E'; break;
        case Qt::Key_F: vk = 'F'; break;
        case Qt::Key_G: vk = 'G'; break;
        case Qt::Key_H: vk = 'H'; break;
        case Qt::Key_I: vk = 'I'; break;
        case Qt::Key_J: vk = 'J'; break;
        case Qt::Key_K: vk = 'K'; break;
        case Qt::Key_N: vk = 'N'; break;
        case Qt::Key_O: vk = 'O'; break;
        case Qt::Key_P: vk = 'P'; break;
        case Qt::Key_S: vk = 'S'; break;
        case Qt::Key_T: vk = 'T'; break;
        default: return; // Unsupported key
    }

    int hotkeyId = next_hotkey_id_++;
    if (RegisterHotKey(NULL, hotkeyId, modifiers, vk)) {
        HotkeyData data;
        data.id = hotkeyId;
        data.actionId = actionId;
        registered_hotkeys_[hotkeyId] = data;
    }
#endif
}

void GlobalHotkeyListener::unregisterAllHotkeys() {
#ifdef Q_OS_WIN
    for (const auto &data : registered_hotkeys_) {
        UnregisterHotKey(NULL, data.id);
    }
    registered_hotkeys_.clear();
#endif
}

// ============================================================================
// ShortcutManager Implementation
// ============================================================================

ShortcutManager::ShortcutManager(QObject *parent) 
    : QObject(parent), global_hotkeys_enabled_(false) {
    global_hotkey_listener_ = new GlobalHotkeyListener(this);
    connect(global_hotkey_listener_, &GlobalHotkeyListener::globalHotkeyPressed,
            this, &ShortcutManager::on_global_hotkey_pressed);
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
    // Clean up old shortcuts
    for (auto &cfg : configs_) {
        if (cfg.shortcut_obj) {
            delete cfg.shortcut_obj;
            cfg.shortcut_obj = nullptr;
        }
    }
    
    // Setup application-level shortcuts (work in focused window)
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
    
    // Setup global hotkeys if enabled
    if (global_hotkeys_enabled_ && global_hotkey_listener_->isSupported()) {
        global_hotkey_listener_->unregisterAllHotkeys();
        for (const auto &cfg : configs_) {
            if (!cfg.current_key.isEmpty()) {
                global_hotkey_listener_->registerGlobalHotkey(cfg.current_key, cfg.action_id);
            }
        }
    }
}

void ShortcutManager::enableGlobalHotkeys(bool enable) {
    global_hotkeys_enabled_ = enable;
    
    if (enable && global_hotkey_listener_->isSupported()) {
        // Register global hotkeys
        for (const auto &cfg : configs_) {
            if (!cfg.current_key.isEmpty()) {
                global_hotkey_listener_->registerGlobalHotkey(cfg.current_key, cfg.action_id);
            }
        }
    } else {
        // Unregister global hotkeys
        global_hotkey_listener_->unregisterAllHotkeys();
    }
}

void ShortcutManager::on_global_hotkey_pressed(const QString &actionId) {
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

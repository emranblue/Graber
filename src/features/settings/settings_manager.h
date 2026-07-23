#ifndef SETTINGS_MANAGER_H
#define SETTINGS_MANAGER_H

#include <QString>
#include "../keyboard_shortcuts/shortcut_manager.h"

class SettingsManager {
public:
    explicit SettingsManager(const QString &notesDir);
    
    void saveShortcutSettings(const QList<ShortcutConfig> &configs);
    void loadShortcutSettings(QList<ShortcutConfig> &configs);
    
private:
    QString notes_dir_path_;
    QString getSettingsFilePath() const;
};

#endif // SETTINGS_MANAGER_H

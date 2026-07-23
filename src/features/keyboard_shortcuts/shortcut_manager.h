#ifndef SHORTCUT_MANAGER_H
#define SHORTCUT_MANAGER_H

#include <QString>
#include <QList>
#include <QKeySequence>
#include <QShortcut>
#include <QObject>

struct ShortcutConfig {
    QString action_id;
    QString name_bangla;
    QString name_english;
    QString default_key;
    QKeySequence current_key;
    QShortcut* shortcut_obj = nullptr;
};

class ShortcutManager : public QObject {
    Q_OBJECT

public:
    explicit ShortcutManager(const QString &notesDir, QObject *parent = nullptr);
    ~ShortcutManager();
    
    void initializeShortcuts();
    void setupShortcuts(QObject *target);
    void loadSettings();
    void saveSettings();
    QList<ShortcutConfig>& getConfigs() { return shortcut_configs_; }
    
signals:
    void shortcutTriggered(const QString &action_id);
    
private slots:
    void onShortcutActivated(const QString &action_id);
    
private:
    QList<ShortcutConfig> shortcut_configs_;
    QString notes_dir_path_;
    QObject *target_widget_;
};

#endif // SHORTCUT_MANAGER_H

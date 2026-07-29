#ifndef SHORTCUTMANAGER_H
#define SHORTCUTMANAGER_H

#include <QObject>
#include <QList>
#include <QSettings>
#include <QWidget>
#include "Types.h"

class ShortcutManager : public QObject {
    Q_OBJECT

public:
    explicit ShortcutManager(QObject *parent = nullptr);
    ~ShortcutManager();

    void initDefaultConfigs();
    void loadSettings(const QString &settingsFilePath);
    void saveSettings(const QString &settingsFilePath);
    void setupShortcuts(QWidget *parentWidget);

    QList<ShortcutConfig>& configs();
    const QList<ShortcutConfig>& configs() const;

signals:
    void actionTriggered(const QString &actionId);

private:
    QList<ShortcutConfig> configs_;
};

#endif // SHORTCUTMANAGER_H

#ifndef ACTIONREGISTRY_H
#define ACTIONREGISTRY_H

#include <QObject>
#include <QMap>
#include <QList>
#include <QPushButton>
#include <QVariantMap>
#include <memory>
#include "interfaces/IAction.h"

class ActionRegistry : public QObject {
    Q_OBJECT

public:
    static ActionRegistry& instance();

    void registerAction(std::shared_ptr<IAction> action);
    void registerFunctionalAction(const QString &id,
                                  const QString &displayName,
                                  const QString &description,
                                  const QString &category,
                                  const QKeySequence &defaultShortcut,
                                  FunctionalAction::Handler handler,
                                  FunctionalAction::EnabledPredicate enabledPredicate = nullptr);

    bool hasAction(const QString &id) const;
    std::shared_ptr<IAction> getAction(const QString &id) const;
    QList<std::shared_ptr<IAction>> getAllActions() const;

    bool executeAction(const QString &id, const QVariantMap &args = QVariantMap());
    bool isActionEnabled(const QString &id) const;

    void bindButton(QPushButton *button, const QString &actionId);
    void updateBoundButtons();

signals:
    void actionRegistered(const QString &actionId);
    void actionTriggered(const QString &actionId);
    void actionStateChanged();

private:
    explicit ActionRegistry(QObject *parent = nullptr);
    ~ActionRegistry() override = default;

    QMap<QString, std::shared_ptr<IAction>> actions_;
    QList<QPair<QPushButton*, QString>> bound_buttons_;
};

#endif // ACTIONREGISTRY_H

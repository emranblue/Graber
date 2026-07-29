#include "ActionRegistry.h"

ActionRegistry& ActionRegistry::instance() {
    static ActionRegistry instance;
    return instance;
}

ActionRegistry::ActionRegistry(QObject *parent) : QObject(parent) {}

void ActionRegistry::registerAction(std::shared_ptr<IAction> action) {
    if (!action) return;
    actions_[action->id()] = action;
    emit actionRegistered(action->id());
    updateBoundButtons();
}

void ActionRegistry::registerFunctionalAction(const QString &id,
                                            const QString &displayName,
                                            const QString &description,
                                            const QString &category,
                                            const QKeySequence &defaultShortcut,
                                            FunctionalAction::Handler handler,
                                            FunctionalAction::EnabledPredicate enabledPredicate) {
    auto action = std::make_shared<FunctionalAction>(
        id, displayName, description, category, defaultShortcut, std::move(handler), std::move(enabledPredicate)
    );
    registerAction(action);
}

bool ActionRegistry::hasAction(const QString &id) const {
    return actions_.contains(id);
}

std::shared_ptr<IAction> ActionRegistry::getAction(const QString &id) const {
    return actions_.value(id, nullptr);
}

QList<std::shared_ptr<IAction>> ActionRegistry::getAllActions() const {
    return actions_.values();
}

bool ActionRegistry::executeAction(const QString &id, const QVariantMap &args) {
    auto action = getAction(id);
    if (action && action->isEnabled()) {
        action->execute(args);
        emit actionTriggered(id);
        updateBoundButtons();
        return true;
    }
    return false;
}

bool ActionRegistry::isActionEnabled(const QString &id) const {
    auto action = getAction(id);
    return action ? action->isEnabled() : false;
}

void ActionRegistry::bindButton(QPushButton *button, const QString &actionId) {
    if (!button) return;
    
    // Check if already bound
    for (const auto &pair : bound_buttons_) {
        if (pair.first == button && pair.second == actionId) {
            return;
        }
    }

    bound_buttons_.append({button, actionId});

    connect(button, &QPushButton::clicked, this, [this, actionId]() {
        this->executeAction(actionId);
    });

    updateBoundButtons();
}

void ActionRegistry::updateBoundButtons() {
    for (auto it = bound_buttons_.begin(); it != bound_buttons_.end(); ) {
        QPushButton *btn = it->first;
        QString actionId = it->second;

        if (!btn) {
            it = bound_buttons_.erase(it);
            continue;
        }

        auto action = getAction(actionId);
        if (action) {
            btn->setEnabled(action->isEnabled());
        }
        ++it;
    }
    emit actionStateChanged();
}

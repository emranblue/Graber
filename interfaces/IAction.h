#ifndef IACTION_H
#define IACTION_H

#include <QString>
#include <QKeySequence>
#include <QVariantMap>
#include <functional>
#include <memory>

class IAction {
public:
    virtual ~IAction() = default;

    virtual QString id() const = 0;
    virtual QString displayName() const = 0;
    virtual QString description() const = 0;
    virtual QString category() const = 0;
    virtual QKeySequence defaultShortcut() const = 0;

    virtual void execute(const QVariantMap &args = QVariantMap()) = 0;
    virtual bool isEnabled() const = 0;
};

// Convenient functional action class
class FunctionalAction : public IAction {
public:
    using Handler = std::function<void(const QVariantMap&)>;
    using EnabledPredicate = std::function<bool()>;

    FunctionalAction(const QString &id,
                     const QString &displayName,
                     const QString &description,
                     const QString &category,
                     const QKeySequence &defaultShortcut,
                     Handler handler,
                     EnabledPredicate enabledPredicate = nullptr)
        : id_(id), displayName_(displayName), description_(description),
          category_(category), defaultShortcut_(defaultShortcut),
          handler_(std::move(handler)), enabledPredicate_(std::move(enabledPredicate)) {}

    QString id() const override { return id_; }
    QString displayName() const override { return displayName_; }
    QString description() const override { return description_; }
    QString category() const override { return category_; }
    QKeySequence defaultShortcut() const override { return defaultShortcut_; }

    void execute(const QVariantMap &args = QVariantMap()) override {
        if (handler_ && isEnabled()) {
            handler_(args);
        }
    }

    bool isEnabled() const override {
        if (enabledPredicate_) {
            return enabledPredicate_();
        }
        return true;
    }

private:
    QString id_;
    QString displayName_;
    QString description_;
    QString category_;
    QKeySequence defaultShortcut_;
    Handler handler_;
    EnabledPredicate enabledPredicate_;
};

#endif // IACTION_H

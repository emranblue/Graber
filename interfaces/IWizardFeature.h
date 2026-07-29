#ifndef IWIZARDFEATURE_H
#define IWIZARDFEATURE_H

#include <QString>
#include <QWidget>
#include <memory>
#include "IServiceRegistry.h"

class IWizardFeature {
public:
    virtual ~IWizardFeature() = default;

    virtual QString id() const = 0;
    virtual QString displayName() const = 0;
    virtual QString description() const = 0;
    virtual QString iconName() const = 0;
    virtual QString category() const = 0;

    virtual void executeWizard(QWidget *parentWidget, IServiceRegistry *services) = 0;
    virtual QWidget* createFeatureWidget(QWidget *parentWidget, IServiceRegistry *services) {
        Q_UNUSED(parentWidget);
        Q_UNUSED(services);
        return nullptr;
    }
};

#endif // IWIZARDFEATURE_H

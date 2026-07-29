#ifndef FEATUREMANAGER_H
#define FEATUREMANAGER_H

#include <QObject>
#include <QList>
#include <QMap>
#include <memory>
#include "interfaces/IWizardFeature.h"
#include "interfaces/IServiceRegistry.h"

class FeatureManager : public QObject {
    Q_OBJECT

public:
    static FeatureManager& instance();

    void registerFeature(std::shared_ptr<IWizardFeature> feature);
    bool hasFeature(const QString &id) const;
    std::shared_ptr<IWizardFeature> getFeature(const QString &id) const;
    QList<std::shared_ptr<IWizardFeature>> getAllFeatures() const;

    bool executeWizard(const QString &id, QWidget *parentWidget, IServiceRegistry *services);

signals:
    void featureRegistered(const QString &featureId);

private:
    explicit FeatureManager(QObject *parent = nullptr);
    ~FeatureManager() override = default;

    QMap<QString, std::shared_ptr<IWizardFeature>> features_;
};

#endif // FEATUREMANAGER_H

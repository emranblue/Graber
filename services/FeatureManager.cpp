#include "FeatureManager.h"

FeatureManager& FeatureManager::instance() {
    static FeatureManager instance;
    return instance;
}

FeatureManager::FeatureManager(QObject *parent) : QObject(parent) {}

void FeatureManager::registerFeature(std::shared_ptr<IWizardFeature> feature) {
    if (!feature) return;
    features_[feature->id()] = feature;
    emit featureRegistered(feature->id());
}

bool FeatureManager::hasFeature(const QString &id) const {
    return features_.contains(id);
}

std::shared_ptr<IWizardFeature> FeatureManager::getFeature(const QString &id) const {
    return features_.value(id, nullptr);
}

QList<std::shared_ptr<IWizardFeature>> FeatureManager::getAllFeatures() const {
    return features_.values();
}

bool FeatureManager::executeWizard(const QString &id, QWidget *parentWidget, IServiceRegistry *services) {
    auto feature = getFeature(id);
    if (feature) {
        feature->executeWizard(parentWidget, services);
        return true;
    }
    return false;
}

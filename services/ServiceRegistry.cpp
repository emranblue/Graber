#include "ServiceRegistry.h"

ServiceRegistry& ServiceRegistry::instance() {
    static ServiceRegistry instance;
    return instance;
}

ServiceRegistry::ServiceRegistry(QObject *parent) : QObject(parent) {}

void ServiceRegistry::registerServiceByType(std::type_index typeIdx, std::shared_ptr<void> service) {
    type_services_[typeIdx] = service;
}

std::shared_ptr<void> ServiceRegistry::getServiceByType(std::type_index typeIdx) const {
    auto it = type_services_.find(typeIdx);
    if (it != type_services_.end()) {
        return it->second;
    }
    return nullptr;
}

void ServiceRegistry::registerServiceByName(const QString &name, std::shared_ptr<void> service) {
    name_services_[name] = service;
}

std::shared_ptr<void> ServiceRegistry::getServiceByName(const QString &name) const {
    return name_services_.value(name, nullptr);
}

void ServiceRegistry::clearAllServices() {
    type_services_.clear();
    name_services_.clear();
}

#ifndef SERVICEREGISTRY_H
#define SERVICEREGISTRY_H

#include "IServiceRegistry.h"
#include <QObject>
#include <unordered_map>
#include <typeindex>
#include <memory>
#include <QString>
#include <QMap>

class ServiceRegistry : public QObject, public IServiceRegistry {
    Q_OBJECT

public:
    static ServiceRegistry& instance();

    void registerServiceByName(const QString &name, std::shared_ptr<void> service) override;
    std::shared_ptr<void> getServiceByName(const QString &name) const override;

    void clearAllServices();

protected:
    void registerServiceByType(std::type_index typeIdx, std::shared_ptr<void> service) override;
    std::shared_ptr<void> getServiceByType(std::type_index typeIdx) const override;

private:
    explicit ServiceRegistry(QObject *parent = nullptr);
    ~ServiceRegistry() override = default;

    std::unordered_map<std::type_index, std::shared_ptr<void>> type_services_;
    QMap<QString, std::shared_ptr<void>> name_services_;
};

#endif // SERVICEREGISTRY_H

#ifndef ISERVICEREGISTRY_H
#define ISERVICEREGISTRY_H

#include <memory>
#include <typeindex>
#include <unordered_map>
#include <QString>
#include <stdexcept>

class IServiceRegistry {
public:
    virtual ~IServiceRegistry() = default;

    template <typename Interface>
    void registerService(std::shared_ptr<Interface> service) {
        registerServiceByType(typeid(Interface), std::static_pointer_cast<void>(service));
    }

    template <typename Interface>
    std::shared_ptr<Interface> getService() const {
        auto raw = getServiceByType(typeid(Interface));
        if (!raw) {
            return nullptr;
        }
        return std::static_pointer_cast<Interface>(raw);
    }

    virtual void registerServiceByName(const QString &name, std::shared_ptr<void> service) = 0;
    virtual std::shared_ptr<void> getServiceByName(const QString &name) const = 0;

protected:
    virtual void registerServiceByType(std::type_index typeIdx, std::shared_ptr<void> service) = 0;
    virtual std::shared_ptr<void> getServiceByType(std::type_index typeIdx) const = 0;
};

#endif // ISERVICEREGISTRY_H

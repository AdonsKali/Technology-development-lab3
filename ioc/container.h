#ifndef IOC_CONTAINER_H
#define IOC_CONTAINER_H

#include <functional>
#include <memory>
#include <map>

class IOCContainer
{
private:
    inline static int s_nextTypeId = 0;

    template<typename T>
    static int GetTypeID() {
        static int typeId = s_nextTypeId++;
        return typeId;
    }

    class FactoryRoot
    {
    public:
        virtual ~FactoryRoot() {}
    };

    template<typename T>
    class CFactory : public FactoryRoot
    {
        std::function<std::shared_ptr<T>()> m_functor;

    public:
        CFactory(std::function<std::shared_ptr<T>()> functor)
            : m_functor(functor)
        {}

        std::shared_ptr<T> GetObject() {
            return m_functor();
        }
    };

    std::map<int, std::shared_ptr<FactoryRoot>> m_factories;

public:
    template<typename T>
    std::shared_ptr<T> GetObject() {
        auto typeId = GetTypeID<T>();
        if (m_factories.find(typeId) == m_factories.end()) {
            return nullptr;
        }
        auto factoryBase = m_factories[typeId];
        auto factory = std::static_pointer_cast<CFactory<T>>(factoryBase);
        return factory->GetObject();
    }

    template<typename TInterface, typename TConcrete, typename... TArguments>
    void RegisterFactory() {
        auto functor = [this]() -> std::shared_ptr<TInterface> {
            return std::make_shared<TConcrete>(GetObject<TArguments>()...);
        };
        m_factories[GetTypeID<TInterface>()] =
            std::make_shared<CFactory<TInterface>>(functor);
    }

    template<typename TInterface>
    void RegisterInstance(std::shared_ptr<TInterface> instance) {
        m_factories[GetTypeID<TInterface>()] =
            std::make_shared<CFactory<TInterface>>([instance]() { return instance; });
    }

    template<typename TInterface, typename TConcrete>
    void RegisterInstance() {
        auto instance = std::make_shared<TConcrete>();
        RegisterInstance<TInterface>(instance);
    }

    template<typename TInterface, typename TConcrete, typename... TArgs>
    void RegisterInstanceWithArgs(TArgs&&... args) {
        auto instance = std::make_shared<TConcrete>(std::forward<TArgs>(args)...);
        RegisterInstance<TInterface>(instance);
    }
};

#endif

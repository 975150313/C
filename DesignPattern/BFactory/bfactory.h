#ifndef BFACTORY_H
#define BFACTORY_H

#include <map>
#include <string>
#include <functional>
#include <memory>

#include "bfactory_global.h"
#include "ibclass.h"
class BFACTORYSHARED_EXPORT BFactory
{

public:
    template<typename T>
    class register_t
    {
    public:
        register_t(const std::string& key)
        {
            BFactory::Instance().map_.emplace(key, &register_t<T>::create);
        }

        template<typename... Args>
        register_t(const std::string& key, Args... args)
        {
            BFactory::Instance().map_.emplace(key, [=] { return new T(args...); });
        }

        inline static IBClass* create() { return new T; }
    };

    std::unique_ptr<IBClass> produce_unique(const std::string& key)
    {
        return std::unique_ptr<IBClass>(produce(key));
    }

    std::shared_ptr<IBClass> produce_shared(const std::string& key)
    {
        return std::shared_ptr<IBClass>(produce(key));
    }
    typedef IBClass*(*FunPtr)();

    inline static BFactory& Instance()
    {
        static BFactory instance;
        return instance;
    }
private:
    BFactory();
    BFactory(const BFactory&) = delete;
    BFactory(BFactory&&) = delete;
    std::map<std::string,std::function<IBClass*()>> map_;

    inline IBClass* produce(const std::string& key)
    {
        if (map_.find(key) == map_.end())
            throw std::invalid_argument("the message key is not exist!");

        return map_[key]();
    }
};
#define REGISTER_Class(T, key) static BFactory::register_t<T> ##T(key);
#endif // BFACTORY_H



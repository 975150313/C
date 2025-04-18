#ifndef BASESINGLETON_H
#define BASESINGLETON_H

#include <typeindex>
#include <map>
#include <functional>
#include <iostream>

// Singleton mode for cpp
//
// Features --
//   1. Works for both dynamical library and executable.
//   2. Multithread safe
//   3. Lazy consturction

void getSharedInstance(const std::type_index &typeIndex,
                                     void *(*getStaticInstance)(),
                                     void *&instance);


template<typename T>
class BaseSingleton
{
    friend T;
public:
    static T* Instance()
    {
        static void* instance = nullptr;
        if (instance == nullptr)
        {
            getSharedInstance(typeid(T), &getStaticInstance, instance);
        }
        return reinterpret_cast<T *>(instance);
    }
private:
    static void* getStaticInstance()
    {
        static T t;
//        return reinterpret_cast<void *>(&reinterpret_cast<char &>(t));
        return reinterpret_cast<void *>(&t);
    }
};
#define Singleton(T) BaseSingleton<T>::Instance()


#endif // BASESINGLETON_H

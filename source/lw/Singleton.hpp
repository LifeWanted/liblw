#pragma once

namespace lw {

#define LW_DEFINE_SINGLETON_INSTANCE( _Type ) \
    template<> _Type* Singleton< _Type >::s_instance = nullptr;

/// @brief Base class for singletons.
///
/// @tparam T The type to have only a single instance.
template< class T >
class Singleton {
public:
    static bool exists( void ){
        return s_instance != nullptr;
    }

    static T* instance_ptr( void ){
        return s_instance;
    }

    static T& instance( void ){
        return *instance_ptr();
    }

    Singleton( void ){
        s_instance = (T*)this;
    }

    Singleton( const Singleton& ) = delete;

    Singleton( Singleton&& other ){
        s_instance = (T*)this;
    }

    ~Singleton( void ){
        s_instance = nullptr;
    }

private:
    static T* s_instance;
};

}

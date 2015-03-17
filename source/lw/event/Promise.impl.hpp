#pragma once

#include "lw/event/Promise.hpp"
#include "lw/event/Promise.void.hpp"

namespace lw {
namespace event {

template< typename T >
inline Future< T > Promise< T >::future( void ){
    return Future< T >( m_state );
}

// -------------------------------------------------------------------------- //

inline Future< void > Promise< void >::future( void ){
    return Future< void >( m_state );
}

// -------------------------------------------------------------------------- //

template< typename T >
template< typename Result, typename Func, typename >
Future< Result > Future< T >::then( Func&& func ){
    auto next = std::make_shared< Promise< Result > >();
    auto prev = m_state;
    m_state->resolve = [ func, prev, next ]( T&& value ) mutable {
        func( std::move( value ), std::move( *next ) );
        prev->reject = nullptr;
        prev.reset();
    };
    m_state->reject = [ prev, next ]() mutable {
        next->reject();
        prev->resolve = nullptr;
        prev.reset();
    };

    return next->future();
}

// -------------------------------------------------------------------------- //

template< typename T >
template< typename Func, typename >
Future<> Future< T >::then( Func&& func ){
    return then< void >( std::move( func ) );
}

// -------------------------------------------------------------------------- //

template< typename T >
template<
    typename Func,
    typename FuncResult,
    typename std::enable_if< IsFuture< FuncResult >::value >::type*
>
Future< typename FuncResult::result_type > Future< T >::then( Func&& func ){
    typedef typename FuncResult::result_type Result;
    return then< Result >(
        [ func ]( T&& value, Promise< Result >&& promise ) mutable {
            func( std::move( value ) ).then( std::move( promise ) );
        }
    );
}

// -------------------------------------------------------------------------- //

template< typename T >
template<
    typename Func,
    typename FuncResult,
    typename std::enable_if<
        !IsFuture< FuncResult >::value &&
        !std::is_void< FuncResult >::value
    >::type*
>
Future< FuncResult > Future< T >::then( Func&& func ){
    return then< FuncResult >(
        [ func ]( T&& value, Promise< FuncResult >&& promise ) mutable {
            promise.resolve( func( std::move( value ) ) );
        }
    );
}

// -------------------------------------------------------------------------- //

template< typename T >
template<
    typename Func,
    typename FuncResult,
    typename std::enable_if< std::is_void< FuncResult >::value >::type*
>
Future<> Future< T >::then( Func&& func ){
    return then([ func ]( T&& value, Promise<>&& promise ){
        func( std::move( value ) );
        promise.resolve();
    });
}

// -------------------------------------------------------------------------- //

template< typename T >
void Future< T >::then( promise_type&& promise ){
    auto next = std::make_shared< promise_type >( std::move( promise ) );
    auto prev = m_state;
    m_state->resolve = [ prev, next ]( T&& value ) mutable {
        next->resolve( std::move( value ) );
        prev->reject = nullptr;
        prev.reset();
    };
    m_state->reject = [ prev, next ]() mutable {
        next->reject();
        prev->resolve = nullptr;
        prev.reset();
    };
}


}
}

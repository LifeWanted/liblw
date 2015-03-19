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
template< typename Result, typename Resolve, typename Reject, typename >
Future< Result > Future< T >::_then( Resolve&& resolve, Reject&& reject ){
    auto next = std::make_shared< Promise< Result > >();
    auto prev = m_state;
    m_state->resolve = [ resolve, prev, next ]( T&& value ) mutable {
        resolve( std::move( value ), std::move( *next ) );
        prev->reject = nullptr;
        prev.reset();
    };
    m_state->reject = [ reject, prev, next ]() mutable {
        reject();
        prev->resolve = nullptr;
        prev.reset();
    };

    return next->future();
}

// -------------------------------------------------------------------------- //

template< typename T >
template< typename Resolve, typename Reject, typename >
inline Future<> Future< T >::_then( Resolve&& resolve, Reject&& reject ){
    return then< void >(
        std::forward< Resolve   >( resolve  ),
        std::forward< Reject    >( reject   )
    );
}

// -------------------------------------------------------------------------- //

template< typename T >
template<
    typename Resolve,
    typename Reject,
    typename ResolveResult,
    typename std::enable_if< IsFuture< ResolveResult >::value >::type*
>
Future< typename ResolveResult::result_type > Future< T >::_then( Resolve&& resolve, Reject&& reject ){
    typedef typename ResolveResult::result_type Result;
    return then< Result >(
        [ resolve ]( T&& value, Promise< Result >&& promise ) mutable {
            resolve( std::move( value ) ).then( std::move( promise ) );
        },
        std::forward< Reject >( reject )
    );
}

// -------------------------------------------------------------------------- //

template< typename T >
template<
    typename Resolve,
    typename Reject,
    typename ResolveResult,
    typename std::enable_if<
        !IsFuture< ResolveResult >::value &&
        !std::is_void< ResolveResult >::value
    >::type*
>
Future< ResolveResult > Future< T >::_then( Resolve&& resolve, Reject&& reject ){
    return then< ResolveResult >(
        [ resolve ]( T&& value, Promise< ResolveResult >&& promise ) mutable {
            promise.resolve( resolve( std::move( value ) ) );
        },
        std::forward< Reject >( reject )
    );
}

// -------------------------------------------------------------------------- //

template< typename T >
template<
    typename Resolve,
    typename Reject,
    typename ResolveResult,
    typename std::enable_if< std::is_void< ResolveResult >::value >::type*
>
Future<> Future< T >::_then( Resolve&& resolve, Reject&& reject ){
    return then(
        [ resolve ]( T&& value, Promise<>&& promise ){
            resolve( std::move( value ) );
            promise.resolve();
        },
        std::forward< Reject >( reject )
    );
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

// -------------------------------------------------------------------------- //

template< typename Result, typename Func, typename >
Future< Result > Future< void >::_then( Func&& func ){
    auto next = std::make_shared< Promise< Result > >();
    auto prev = m_state;
    m_state->resolve = [ func, prev, next ]() mutable {
        func( std::move( *next ) );
        prev.reset();
    };
    return next->future();
}

// -------------------------------------------------------------------------- //

template< typename Func, typename >
Future<> Future< void >::_then( Func&& func ){
    return then< void >( std::move( func ) );
}

// -------------------------------------------------------------------------- //

template<
    typename Func,
    typename FuncResult,
    typename std::enable_if< IsFuture< FuncResult >::value >::type*
>
Future< typename FuncResult::result_type > Future< void >::_then( Func&& func ){
    typedef typename FuncResult::result_type Result;
    return then< Result >([ func ]( Promise< Result >&& promise ){
        func().then( std::move( promise ) );
    });
}

// -------------------------------------------------------------------------- //

template<
    typename Func,
    typename FuncResult,
    typename std::enable_if<
        !IsFuture< FuncResult >::value &&
        !std::is_void< FuncResult >::value
    >::type*
>
Future< FuncResult > Future< void >::_then( Func&& func ){
    return then< FuncResult >([ func ]( Promise< FuncResult >&& promise ){
        promise.resolve( func() );
    });
}

// -------------------------------------------------------------------------- //

template<
    typename Func,
    typename FuncResult,
    typename std::enable_if< std::is_void< FuncResult >::value >::type*
>
Future< void > Future< void >::_then( Func&& func ){
    return then< void >([ func ]( Promise< void >&& promise ){
        func();
        promise.resolve();
    });
}

// -------------------------------------------------------------------------- //

inline void Future< void >::then( promise_type&& promise ){
    auto next = std::make_shared< promise_type >( std::move( promise ) );
    auto prev = m_state;
    m_state->resolve = [ prev, next ]() mutable {
        next->resolve();
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

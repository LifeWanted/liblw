#pragma once

#include <atomic>
#include <functional>
#include <memory>

namespace lw {
namespace event {

template< typename T >
class Future;

/// @brief `Promise`s and `Future`s allow for chaining callbacks without nesting.
///
/// A `Promise` is the active side of the pair. Asynchronous functions create a
/// promise and later fulfill it by either resolving or rejecting the promise.
template< typename T >
class Promise {
public:
    /// @brief Default construction.
    Promise( void ):
        m_state( new _SharedState() )
    {
        m_state->resolved   = false;
        m_state->rejected   = false;
        m_state->resolve    = nullptr;
    }

    // ---------------------------------------------------------------------- //

    /// @brief No copying!
    Promise( const Promise& ) = delete;

    // ---------------------------------------------------------------------- //

    /// @brief Moves the promise from `other` to `this`.
    Promise( Promise&& other ):
        m_state( std::move( other.m_state ) )
    {
        other.m_state = nullptr;
    }

    // ---------------------------------------------------------------------- //

    /// @brief Returns a future associated with this promise.
    Future< T > future( void ){
        return Future< T >( m_state );
    }

    // ---------------------------------------------------------------------- //

    /// @brief Resolves the promise as a success.
    void resolve( T&& value ){
        m_state->resolved = true;
        if( m_state->resolve ){
            m_state->resolve( std::move( value ) );
        }
    }

    // ---------------------------------------------------------------------- //

    /// @brief Rejects the promise as a failure.
    void reject( void ){
        m_state->rejected = true;
        if( m_state->reject ){
            m_state->reject();
        }
    }

    // ---------------------------------------------------------------------- //

    /// @brief No copying!
    Promise& operator=( const Promise& ) = delete;

    // ---------------------------------------------------------------------- //

    /// @brief Moves the promise from `other` into `this`.
    Promise& operator=( Promise&& other ){
        m_state = std::move( other.m_state );
        other.m_state = nullptr;
        return *this;
    }

private:
    template< typename Type >
    friend class ::lw::event::Future;

    // ---------------------------------------------------------------------- //

    /// @brief The container for the shared state between promises and futures.
    struct _SharedState {
        std::atomic_bool resolved;
        std::atomic_bool rejected;
        std::function< void( T&& ) > resolve;
        std::function< void( void ) > reject;
    };
    typedef std::shared_ptr< _SharedState > _SharedStatePtr;

    // ---------------------------------------------------------------------- //

    /// @brief The state of the promise.
    _SharedStatePtr m_state;
};

// -------------------------------------------------------------------------- //

/// @brief The passive half of the `Promise`-`Future` pair.
///
/// `Future`s are the requester's handle on an asynchronous event. They allow
/// callbacks to be registered for after it has been started.
template< typename T >
class Future {
public:
    /// @brief Chaining for generic functors.
    ///
    /// @tparam Result  The type given functor promises.
    /// @tparam Func    A functor type that can take a `Promise&&` as its parameter.
    ///
    /// @param func The functor to call when this one is resolved.
    ///
    /// @return A new future, for when the provided `func` completes its action.
    template<
        typename Result,
        typename Func,
        typename = typename std::result_of< Func( T&&, Promise< Result >&& ) >::type
    >
    Future< Result > then( Func&& func ){
        auto next = std::make_shared< Promise< Result > >();
        auto prev = m_state;
        m_state->resolve = [ func, prev, next ]( T&& value ) mutable {
            func( std::move( value ), std::move( *next ) );
            prev.reset();
        };
        return next->future();
    }

    // ---------------------------------------------------------------------- //

    /// @brief Chaining for `Future`-returning functors.
    ///
    /// @tparam Func A functor which returns a `Future`.
    ///
    /// @param func The functor to call when this one is ready.
    ///
    /// @return A `Future` which will be resolved by `func`.
    template<
        typename Result,
        typename Func,
        typename FuncResult = typename std::result_of< Func( T&& ) >::type,
        typename std::enable_if<
            std::is_base_of< Future< Result >, FuncResult >::value
        >::type* = nullptr
    >
    Future< Result > then( Func&& func ){
        return then< Result >([ func ]( T&& value, Promise< Result >&& promise ){
            func( std::move( value ) ).then( std::move( promise ) );
        });
    }

    // ---------------------------------------------------------------------- //

    /// @brief Connects this promise to the one provided.
    ///
    /// @param promise The promise to resolve/reject with this one.
    void then( Promise< T >&& promise ){
        auto next = std::make_shared< Promise< T > >( std::move( promise ) );
        auto prev = m_state;
        m_state->resolve = [ prev, next ]( T&& value ) mutable {
            if( next ){
                next->resolve( std::move( value ) );
            }
            prev.reset();
        };
    }


    // ---------------------------------------------------------------------- //

private:
    template< typename Type >
    friend class ::lw::event::Promise;

    // ---------------------------------------------------------------------- //

    /// @brief Only `Promise`s can construct us.
    ///
    /// @param state The shared state to associate with.
    Future( const typename Promise< T >::_SharedStatePtr& state ):
        m_state( state )
    {}

    // ---------------------------------------------------------------------- //

    /// @brief Returns true if this future has been resolved.
    bool is_resolved( void ) const {
        return m_state->resolved;
    }

    // ---------------------------------------------------------------------- //

    /// @brief Returns true if this future has been rejected.
    bool is_rejected( void ) const {
        return m_state->rejected;
    }

    // ---------------------------------------------------------------------- //

    /// @brief Our internal shared state.
    typename Promise< T >::_SharedStatePtr m_state;
};

}
}

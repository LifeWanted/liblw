#pragma once

#include <atomic>
#include <functional>
#include <memory>

#include "lw/event/Promise.hpp"

namespace lw {
namespace event {

template<>
class Promise< void >{
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
    Future< void > future( void );

    // ---------------------------------------------------------------------- //

    /// @brief Resolves the promise as a success.
    void resolve( void ){
        m_state->resolved = true;
        if( m_state->resolve ){
            m_state->resolve();
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
        std::function< void( void ) > resolve;
        std::function< void( void ) > reject;
    };
    typedef std::shared_ptr< _SharedState > _SharedStatePtr;

    // ---------------------------------------------------------------------- //

    /// @brief The state of the promise.
    _SharedStatePtr m_state;
};

// -------------------------------------------------------------------------- //

template<>
class Future< void >{
public:
    /// @brief The type promised by this future.
    typedef void ResultType;

    /// @brief The type of Promise that made this future.
    typedef Promise< void > PromiseType;

    // ---------------------------------------------------------------------- //

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
        typename = typename std::result_of< Func( Promise< Result >&& ) >::type
    >
    Future< Result > then( Func&& func ){
        auto next = std::make_shared< Promise< Result > >();
        auto prev = m_state;
        m_state->resolve = [ func, prev, next ]() mutable {
            func( std::move( *next ) );
            prev.reset();
        };
        return next->future();
    }

    // ---------------------------------------------------------------------- //

    /// @brief Chaining for generic functors promising nothing.
    ///
    /// @tparam Func A functor type that can take a `Promise&&` as its parameter.
    ///
    /// @param func The functor to call when this one is resolved.
    ///
    /// @return A new future, for when the provided `func` completes its action.
    template<
        typename Func,
        typename = typename std::result_of< Func( Promise<>&& ) >::type
    >
    Future<> then( Func&& func ){
        return then< void >( std::move( func ) );
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
        typename Func,
        typename FuncResult = typename std::result_of< Func() >::type,
        typename std::enable_if<
            std::is_base_of<
                Future< typename FuncResult::ResultType >,
                FuncResult
            >::value
        >::type* = nullptr
    >
    Future< typename FuncResult::ResultType > then( Func&& func ){
        typedef typename FuncResult::ResultType Result;
        return then< Result >([ func ]( Promise< Result >&& promise ){
            func().then( std::move( promise ) );
        });
    }

    // ---------------------------------------------------------------------- //

    /// @brief Connects this promise to the one provided.
    ///
    /// @param promise The promise to resolve/reject with this one.
    void then( PromiseType&& promise ){
        auto next = std::make_shared< PromiseType >( std::move( promise ) );
        auto prev = m_state;
        m_state->resolve = [ prev, next ]() mutable {
            if( next ){
                next->resolve();
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
    Future( const typename PromiseType::_SharedStatePtr& state ):
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
    typename PromiseType::_SharedStatePtr m_state;
};

}
}

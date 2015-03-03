#pragma once

#include <atomic>
#include <functional>
#include <memory>

namespace lw {
namespace event {

class Future;

/// @brief `Promise`s and `Future`s allow for chaining callbacks without nesting.
///
/// A `Promise` is the active side of the pair. Asynchronous functions create a
/// promise and later fulfill it by either resolving or rejecting the promise.
class Promise {
public:
    /// @brief Default construction.
    Promise( void );

    // ---------------------------------------------------------------------- //

    /// @brief No copying!
    Promise( const Promise& ) = delete;

    // ---------------------------------------------------------------------- //

    /// @brief Moves the promise from `other` to `this`.
    Promise( Promise&& other );

    // ---------------------------------------------------------------------- //

    /// @brief Returns a future associated with this promise.
    Future future( void );

    // ---------------------------------------------------------------------- //

    /// @brief Resolves the promise as a success.
    void resolve( void );

    // ---------------------------------------------------------------------- //

    /// @brief Rejects the promise as a failure.
    void reject( void );

    // ---------------------------------------------------------------------- //

    /// @brief No copying!
    Promise& operator=( const Promise& ) = delete;

    // ---------------------------------------------------------------------- //

    /// @brief Moves the promise from `other` into `this`.
    Promise& operator=( Promise&& other );

private:
    friend class ::lw::event::Future;

    // ---------------------------------------------------------------------- //

    /// @brief The container for the shared state between promises and futures.
    struct _SharedState {
        std::atomic_bool resolved;
        std::atomic_bool rejected;
        std::function< void( void ) > handler;
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
class Future {
public:
    /// @brief Chaining for generic functors.
    ///
    /// @tparam Func A functor type that can take a `Promise&&` as its parameter.
    ///
    /// @param func The functor to call when this one is resolved.
    ///
    /// @return A new future, for when the provided `func` completes its action.
    template<
        typename Func,
        typename = typename std::result_of< Func( Promise&& ) >::type
    >
    Future then( Func&& func ){
        auto next = std::make_shared< Promise >();
        auto prev = m_state;
        m_state->handler = [ func, prev, next ]() mutable {
            func( std::move( *next ) );
            prev.reset();
        };
        return next->future();
    }

    // ---------------------------------------------------------------------- //

    /// @brief Connects this promise to the one provided.
    ///
    /// @param promise The promise to resolve/reject with this one.
    void then( Promise&& promise );

    // ---------------------------------------------------------------------- //

private:
    friend class ::lw::event::Promise;

    // ---------------------------------------------------------------------- //

    /// @brief Only `Promise`s can construct us.
    ///
    /// @param state The shared state to associate with.
    Future( const Promise::_SharedStatePtr& state ):
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
    Promise::_SharedStatePtr m_state;
};

}
}

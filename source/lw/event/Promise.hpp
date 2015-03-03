#pragma once

#include <atomic>
#include <functional>
#include <memory>

namespace lw {
namespace event {

class Future;

class Promise {
public:
    Promise( void );
    Promise( const Promise& ) = delete;
    Promise( Promise&& other );

    Future future( void );

    void resolve( void );
    void reject( void );

    Promise& operator=( const Promise& ) = delete;
    Promise& operator=( Promise&& other );

private:
    friend class ::lw::event::Future;

    struct _SharedState {
        std::atomic_bool resolved;
        std::atomic_bool rejected;
        std::function< void( void ) > handler;
    };
    typedef std::shared_ptr< _SharedState > _SharedStatePtr;

    _SharedStatePtr m_state;
};

class Future {
public:
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

    void then( Promise&& promise );

private:
    friend class ::lw::event::Promise;

    Future( const Promise::_SharedStatePtr& state ):
        m_state( state )
    {}

    bool is_resolved( void ) const {
        return m_state->resolved;
    }

    bool is_rejected( void ) const {
        return m_state->rejected;
    }

    Promise::_SharedStatePtr m_state;
};

}
}

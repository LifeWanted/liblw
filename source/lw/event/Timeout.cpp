
#include <cstdlib>
#include <uv.h>

#include "lw/event/Timeout.hpp"
#include "lw/event/Timeout.impl.hpp"

namespace lw {
namespace event {

struct Timeout::_State : public std::enable_shared_from_this< _State >{
    _State( Loop& loop );
    ~_State( void );

    event::Loop& loop;
    std::atomic_bool triggered;
    uv_timer_s* handle;
    std::shared_ptr< Promise<> > promise;
    std::function< void( bool ) > task;
};

// -------------------------------------------------------------------------- //

Timeout::Timeout( Loop& loop ):
    m_state( std::make_shared< _State >( loop ) )
{}

// -------------------------------------------------------------------------- //

Timeout::_State::_State( Loop& _loop ):
    loop( _loop ),
    triggered( false ),
    handle( (uv_timer_t*)std::malloc( sizeof( uv_timer_t ) ) ),
    promise( std::make_shared< Promise<> >() )
{
    uv_timer_init( loop.lowest_layer(), handle );
    handle->data = (void*)this;
}

// -------------------------------------------------------------------------- //

Timeout::_State::~_State( void ){
    if( handle ){
        uv_timer_stop( handle );
        std::free( handle );
        handle = nullptr;
    }
}

// -------------------------------------------------------------------------- //

Future<> Timeout::start( const resolution& delay ){
    auto state = m_state;
    m_state->task = [ state ]( bool cancel ) mutable {
        if( cancel ){
            state->promise->reject( TimeoutError( 1, "Timeout cancelled." ) );
        }
        else {
            state->promise->resolve();
        }
        state.reset();
    };
    uv_timer_start( m_state->handle, &Timeout::_timer_cb, delay.count(), 0 );
    return m_state->promise->future();
}

// -------------------------------------------------------------------------- //

Future<> Timeout::repeat( const resolution& interval, const repeat_callback& cb ){
    auto state = m_state;
    m_state->task = [ state, cb ]( bool cancel ) mutable {
        if( cancel ){
            state->promise->resolve();
            state.reset();
        }
        else {
            Timeout timeout( state );
            cb( timeout );
        }
    };
    uv_timer_start(
        m_state->handle,
        &Timeout::_timer_cb,
        interval.count(),
        interval.count()
    );
    return m_state->promise->future();
}

// -------------------------------------------------------------------------- //

void Timeout::stop( void ){
    uv_timer_stop( m_state->handle );
    if( m_state->task ){
        m_state->task( true ); // true == cancelled
        m_state->task = nullptr;
    }
}

// -------------------------------------------------------------------------- //

void Timeout::_timer_cb( uv_timer_t* handle ){
    _State* state = (_State*)handle->data;
    state->triggered = true;
    state->task( false ); // false == not cancelled
}

}
}


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
    repeat_callback task;
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
    m_state->task = []( Timeout& timeout ){
        timeout.m_state->promise->resolve();
    };
    uv_timer_start( m_state->handle, &Timeout::_timer_cb, delay.count(), 0 );
    return m_state->promise->future();
}

// -------------------------------------------------------------------------- //

void Timeout::stop( void ){
    uv_timer_stop( m_state->handle );
    m_state->task = nullptr;
    if( !m_state->promise->is_finished() ){
        m_state->promise->resolve();
    }
}

// -------------------------------------------------------------------------- //

void Timeout::_timer_cb( uv_timer_t* handle ){
    _State* state = (_State*)handle->data;
    state->triggered = true;
    Timeout timeout( state->shared_from_this() );
    state->task( timeout );
}

// -------------------------------------------------------------------------- //

Future<> Timeout::repeat( const resolution& interval, const repeat_callback& cb ){
    auto state = m_state;
    m_state->task = [ state, cb ]( Timeout& timeout){ cb( timeout ); };
    uv_timer_start(
        m_state->handle,
        &Timeout::_timer_cb,
        interval.count(),
        interval.count()
    );
    return m_state->promise->future();
}

}
}

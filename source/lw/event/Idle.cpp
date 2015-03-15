
#include <cstdlib>
#include <uv.h>

#include "lw/event/Idle.hpp"

namespace lw {
namespace event {

Idle::Idle( Loop& loop ):
    m_handle( (uv_idle_s*)std::malloc( sizeof( uv_idle_s ) ) ),
    m_callback( nullptr ),
    m_started( false )
{
    uv_idle_init( loop.lowest_layer(), m_handle );
    m_handle->data = (void*)this;
}

Idle::~Idle( void ){
    stop();
    std::free( m_handle );
}

void Idle::start( void ){
    if( !m_started.exchange( true ) ){
        uv_idle_start( m_handle, &Idle::_handle_cb );
    }
}

void Idle::stop( void ){
    if( m_started.exchange( false ) ){
        uv_idle_stop( m_handle );
    }
}

void Idle::_handle_cb( uv_idle_s* handle ){
    ((Idle*)handle->data)->m_callback();
}

}
}

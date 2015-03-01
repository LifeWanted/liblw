
#include <cstdlib>
#include <uv.h>

#include "lw/event/Loop.hpp"

namespace lw {
namespace event {

Loop::Loop( void ):
    m_loop( (uv_loop_s*)malloc( sizeof( uv_loop_s ) ) )
{
    uv_loop_init( m_loop );
}

Loop::~Loop( void ){
    uv_loop_close( m_loop );
    free( m_loop );
}

void Loop::run( void ){
    uv_run( m_loop, UV_RUN_DEFAULT );
}

}
}

#pragma once

#include <atomic>
#include <functional>

#include "lw/event/Loop.hpp"

struct uv_idle_s;

namespace lw {
namespace event {

class Idle {
public:
    typedef std::function< void( void ) > Callback;

    Idle( Loop& loop );

    template< typename Func >
    Idle( Loop& loop, Func&& func ):
        Idle( loop )
    {
        start( std::forward< Func >( func ) );
    }

    ~Idle( void );

    void start( void );

    template< typename Func >
    void start( Func&& func ){
        m_callback = std::forward< Func >( func );
        start();
    }

    void stop( void );

private:
    static void _handle_cb( uv_idle_s* handle );

    uv_idle_s* m_handle;
    Idle::Callback m_callback;
    std::atomic_bool m_started;
};

}
}

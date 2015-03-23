#pragma once

#include "lw/event/Promise.impl.hpp"
#include "lw/event/Timeout.hpp"

namespace lw {
namespace event {

template< class Func >
Future<> Timeout::repeat( const resolution& interval, Func&& func ){
    auto state = m_state;
    m_state->task = [ state, func ](){ func( Timeout( state ) ); };
    return _repeat( interval );
}

// -------------------------------------------------------------------------- //

inline Future<> wait( Loop& loop, const Timeout::resolution& delay ){
    Timeout timeout( loop );
    return timeout.start( delay );
}

// -------------------------------------------------------------------------- //

template< class Clock >
Future<> wait_until(
    Loop& loop,
    const std::chrono::time_point< Clock, Timeout::resolution >& when
){
    return wait(
        loop,
        when - std::chrono::time_point_cast< Timeout::resolution >( Clock::now() )
    );
}

// -------------------------------------------------------------------------- //

template< typename Func >
Future<> repeat(
    Loop& loop,
    const Timeout::resolution& interval,
    Func&& func
){
    Timeout timeout( loop );
    return timeout.repeat( interval, std::forward< Func >( func ) );
}

}
}

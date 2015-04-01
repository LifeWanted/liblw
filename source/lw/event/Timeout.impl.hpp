#pragma once

#include "lw/event/Promise.impl.hpp"
#include "lw/event/Timeout.hpp"

namespace lw {
namespace event {

inline Future<> wait( Loop& loop, const Timeout::resolution& delay ){
    Timeout timeout( loop );
    return timeout.start( delay );
}

// -------------------------------------------------------------------------- //

template< class Clock, class Duration >
Future<> wait_until(
    Loop& loop,
    const std::chrono::time_point< Clock, Duration >& when
){
    return wait(
        loop,
        std::chrono::duration_cast< Timeout::resolution >( when - Clock::now() )
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

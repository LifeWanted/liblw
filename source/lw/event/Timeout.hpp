#pragma once

#include <atomic>
#include <chrono>
#include <memory>

#include "lw/event/Loop.hpp"
#include "lw/event/Promise.hpp"

struct uv_timer_s;

namespace lw {
namespace event {

class Timeout {
public:
    typedef std::chrono::milliseconds resolution;

    // ---------------------------------------------------------------------- //

    Timeout( Loop& loop );

    // ---------------------------------------------------------------------- //

    ~Timeout( void );

    // ---------------------------------------------------------------------- //

    Future<> start( const resolution& delay );

    // ---------------------------------------------------------------------- //

    template< class Func >
    Future<> repeat( const resolution& interval, Func&& func );

    // ---------------------------------------------------------------------- //

    void stop( void );

    // ---------------------------------------------------------------------- //

private:
    struct _State {
        ~_State( void );

        event::Loop& loop;
        std::atomic_bool triggered;
        uv_timer_s handle;
        std::function< void() > task;
    };

    // ---------------------------------------------------------------------- //

    Timeout( const std::shared_ptr< _State >& state );

    // ---------------------------------------------------------------------- //

    Future<> _repeat( const resolution& interval );

    // ---------------------------------------------------------------------- //

    std::shared_ptr< _State > m_state;
};

// -------------------------------------------------------------------------- //

/// @brief Starts a timeout that will be resolved at some point in the future.
///
/// @param loop     The event loop to use for waiting.
/// @param delay    The amount of time to wait. Up to millisecond resolution.
///
/// @return A future that will be resolved after time has passed.
Future<> wait( Loop& loop, const Timeout::resolution& delay );

// -------------------------------------------------------------------------- //

/// @brief Waits until the provided point in time before resolving.
///
/// @param loop The event loop to use for waiting.
/// @param when The point in time to wait until.
///
/// @return A future that will be resolved after the given point in time.
template< class Clock >
Future<> wait_until(
    Loop& loop,
    const std::chrono::time_point< Clock, Timeout::resolution >& when
);

// -------------------------------------------------------------------------- //

template< typename Func >
Future<> repeat( Loop& loop, const Timeout::resolution& interval, Func&& func );

}
}

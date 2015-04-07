#pragma once

#include <atomic>
#include <chrono>
#include <memory>

#include "lw/event/Loop.hpp"
#include "lw/event/Promise.hpp"

struct uv_timer_s;

namespace lw {
namespace event {

LW_DEFINE_EXCEPTION( TimeoutError );

/// @brief Provides the ability to schedule tasks at a future time.
class Timeout {
public:
    /// @brief The maximum resolution supported for durations.
    typedef std::chrono::milliseconds resolution;

    /// @brief Callback type used for repeating timeouts.
    ///
    /// @param timeout A reference to the repeating `Timeout`.
    typedef std::function< void( Timeout& timeout ) > repeat_callback;

    // ---------------------------------------------------------------------- //

    /// @brief Constructs a timeout object.
    ///
    /// @param loop The event loop we'll be scheduling the timeout on.
    Timeout( Loop& loop );

    // ---------------------------------------------------------------------- //

    /// @brief Schedules the event to happen in the future.
    ///
    /// @param delay How long to wait before resolving.
    ///
    /// @return A future that will be resolved after the time has passed.
    Future<> start( const resolution& delay );

    // ---------------------------------------------------------------------- //

    /// @brief Schedules a callback to be called repeatedly.
    ///
    /// The callback will receive a reference to this `Timeout`, thus giving it
    /// a handle to stop the repetitions at any point.
    ///
    /// @param interval How long between calls to wait.
    /// @param cb       The callback to execute repeatedly.
    ///
    /// @return A future that will be resolved when the repeating is stopped.
    Future<> repeat( const resolution& interval, const repeat_callback& cb );

    // ---------------------------------------------------------------------- //

    /// @brief Stops the timeout from executing.
    ///
    /// If the timeout was only going to fire once (`Timeout::start` was used)
    /// then the promise will be rejected. Otherwise (`Timeout::repeat` was
    /// used) no more executions of the callback will occur and the promise will
    /// be resolved.
    void stop( void );

    // ---------------------------------------------------------------------- //

private:
    struct _State; ///< Type used for managing internal state.

    // ---------------------------------------------------------------------- //

    /// @brief Triggers the callback in the state associated with the handle.
    ///
    /// @param handle The timer handle that fired.
    static void _timer_cb( uv_timer_s* handle );

    // ---------------------------------------------------------------------- //

    /// @brief Constructs a timeout around existing state.
    ///
    /// @param state The existing timeout state to wrap.
    Timeout( const std::shared_ptr< _State >& state ):
        m_state( state )
    {}

    // ---------------------------------------------------------------------- //

    /// @brief The timeout state information.
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
template< class Clock, class Duration >
Future<> wait_until(
    Loop& loop,
    const std::chrono::time_point< Clock, Duration >& when
);

// -------------------------------------------------------------------------- //

/// @brief Sets up the repeating timout.
///
/// The provided function must accept a reference to a `Timeout` object. This
/// object can be used to stop the repetitions if desired.
///
/// @tparam Func A type where `func( timeout )` is well formed.
///
/// @param loop     The event loop to use for execution.
/// @param interval The amount of time between each call of `func`.
/// @param func     A function to repeatedly call.
///
/// @return A future that will be resolved when the repeating is stopped.
template< typename Func >
Future<> repeat( Loop& loop, const Timeout::resolution& interval, Func&& func );

}
}

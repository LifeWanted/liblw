#pragma once

#include <chrono>

#include "lw/Application.hpp"
#include "lw/event/Loop.hpp"
#include "lw/event/Promise.hpp"
#include "lw/event/Promise.void.hpp"
#include "lw/event/Timeout.hpp"

#include "lw/event/Promise.impl.hpp"
#include "lw/event/Timeout.impl.hpp"

namespace lw {
namespace event {

/// @brief Creates a promise that is immediately resolved with the given value.
///
/// In order to guarantee that the calling function returns before the promise is resolved, the
/// resolution happens asynchronously on the event loop provided.
///
/// @tparam T The type that we're resolving with.
///
/// @param loop The event loop used to schedule the resolution of the promise.
/// @param t    The value to resolve with.
///
/// @return A promise for the given value.
template<typename T>
Future<T> resolve(Loop& loop, T&& t){
    return wait(loop, std::chrono::seconds(0)).then([t = std::forward<T>(t)](){ return t; });
}

inline Future<> resolve(Loop& loop){
    return wait(loop, std::chrono::seconds(0));
}

// ---------------------------------------------------------------------------------------------- //

/// @brief Creates a promise that is resolved with the given value on the `Application` event loop.
///
/// In order to guarantee that the calling function returns before the promise is resolved, the
/// resolution happens asynchronously on the `Application` instance event loop.
///
/// @tparam T The type that we're resolving with.
///
/// @param t The value to resolve with.
///
/// @return A promise for the given value.
template<typename T>
Future<T> resolve(T&& t){
    return resolve(Application::instance(), std::forward<T>(t));
}

inline Future<> resolve(){
    return wait(Application::instance(), std::chrono::seconds(0));
}

// ---------------------------------------------------------------------------------------------- //

/// @brief Creates a promise that is immediately rejected with the given error.
///
/// In order to guarantee that the calling function returns before the promise is rejected, the
/// rejection happens asynchronously on the event loop provided.
///
/// @tparam T The type that should be promised.
///
/// @param loop The event loop used to schedule the rejection of the promise.
/// @param err  The error to reject with.
///
/// @return A promise for the given value that will be rejected.
template<typename T>
Future<T> reject(Loop& loop, const error::Exception& err){
    return wait(loop, std::chrono::seconds(0)).then([err]() -> T { throw err; });
}

inline Future<> reject(Loop& loop, const error::Exception& err){
    return wait(loop, std::chrono::seconds(0)).then([err](){ throw err; });
}

// ---------------------------------------------------------------------------------------------- //

/// @brief Creates a promise that is rejected with the given error on the `Application` event loop.
///
/// In order to guarantee that the calling function returns before the promise is rejected, the
/// rejection happens asynchronously on the `Application` instance event loop.
///
/// @tparam T The type that should be promised.
///
/// @param err The error to reject with.
///
/// @return A promise for the given value that will be rejected.
template<typename T>
Future<T> reject(const error::Exception& err){
    return reject<T>(Application::instance(), err);
}

inline Future<> reject(const error::Exception& err){
    return reject(Application::instance(), err);
}

}
}


#include <cstdint>
#include <functional>
#include <list>
#include <map>
#include <string>
#include <tuple>
#include <type_traits>
#include <utility>

#include "lw/pp.hpp"

namespace lw {
namespace event {

// Declares event types.
#define LW_DECLARE_EVENT(_event_name) struct LW_CONCAT(_event_name, _t) {};
#define LW_DECLARE_EVENTS(...) LW_FOR_EACH(LW_DECLARE_EVENT, __VA_ARGS__)

// Defines a single event type.
#define LW_DEFINE_EVENT(_event_name, ...) \
    typedef ::lw::event::Event<LW_CONCAT(_event_name, _t), __VA_ARGS__> LW_CONCAT(_event_name, _event_t);
#define LW_DEFINE_EVENTS(...) LW_FOR_EACH(LW_DEFINE_EVENT, __VA_ARGS__)

// Creates an event emitter class using the given events.
#define _LW_EVENT_TYPE(_event_name, ...) ::lw::event::Event<LW_CONCAT(_event_name, _t), __VA_ARGS__>,
#define _LW_IMPORT_EVENT(_event_name, ...) static constexpr LW_CONCAT(_event_name, _t) _event_name{};
#define _LW_EVENT_STRING_CALL(_event_name, ...)             \
    if (event == LW_STRINGIFY(_event_name)) {               \
        on(_event_name, std::forward<Listener>(listener));  \
        return;                                             \
    }
#define LW_DEFINE_EMITTER(_emitter_name, ...)                                           \
    class _emitter_name :                                                               \
        public ::lw::event::Emitter<LW_FOR_EACH(_LW_EVENT_TYPE, __VA_ARGS__) nullptr_t> \
    {                                                                                   \
    public:                                                                             \
        struct event { LW_FOR_EACH(_LW_IMPORT_EVENT, __VA_ARGS__) };                    \
                                                                                        \
        template<typename Listener>                                                     \
        void on(const std::string& event, Listener&& listener){                         \
            LW_FOR_EACH(_LW_EVENT_STRING_CALL, __VA_ARGS__)                             \
        }                                                                               \
    };

// ---------------------------------------------------------------------------------------------- //

template<typename EventId, typename... EventArgs>
class Event {
public:
    typedef EventId event_id_type;
    typedef std::tuple<EventArgs...> event_argument_types;
    typedef std::function<void(EventArgs&&...)> listener_type;

private:
    std::list<listener_type> m_listeners;
};

template<
    std::size_t I,
    typename EventId,
    typename Event,
    typename OtherEvents...,
    typename = typename std::enable_if<
        !std::is_same<EventId, typename Event::event_id_type>::value
    >::type
>
struct EventIndexImpl : public EventIndexImpl<I + 1, EventId, OtherEvents...> {};

template<
    std::size_t I,
    typename EventId,
    typename Event,
    typename OtherEvents...,
    typename = typename std::enable_if<
        std::is_same<EventId, typename Event::event_id_type>::value
    >::type
>
struct EventIndexImpl : public std::integral_constant<std::size_t, I> {};

template<typename EventId, typename... Events>
struct EventIndex : public EventIndexImpl<0, EventId, Events...> {};

template<typename EventId, typename... Events>
struct GetEvent {
    typedef EventIndex<EventId, Events...> event_idx_type;
    typedef std::tuple<Events...> event_tuple_type;
    typedef typename std::tuple_element<event_idx_type::value, event_tuple_type>::type event_type;

    static event_type& from(event_tuple_type& events){
        return std::get<event_idx_type::value>(events);
    }

    static const event_type& from(const event_tuple_type& events){
        return std::get<event_idx_type::value>(events);
    }
};

template<typename Func, typename Args>
struct IsCallable : public std::false_type {};

template<typename Func, typename... Args, typename = typename std::result_of<Func(Args&&...)>::type>
struct IsCallback<Func, std::tuple<Args...>> : public std::true_type {};

template<typename Listener, typename Event>
struct EventListenerCallable : public IsCallable<Listener, typename Event::event_argument_types> {};

template<typename... Events>
class Emitter {
public:
    typedef std::tuple<Events...> event_types;

    template<
        typename EventId,
        typename Listener,
        typename = typename std::enable_if<
            EventListenerCallable<Listener, typename GetEvent<EventId, Events...>::type>::value
        >::type
    >
    void on(const EventId&, Listener&& listener){
        GetEvent<EventId, Events...>::from(m_events).add_listener(std::forward<Listener>(listener));
    }

private:
    event_types m_events;
};

}
}

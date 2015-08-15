#pragma once

struct uv_loop_s;

namespace lw {
namespace event {

/// @brief The event loop which runs all tasks.
class Loop {
public:
    /// @brief Default constructor.
    Loop(void);

    /// @brief No copying.
    Loop(const Loop&) = delete;

    /// @brief Move constructor.
    Loop(Loop&& other):
        m_loop(other.m_loop)
    {}

    // ------------------------------------------------------------------------------------------ //

    /// @brief Extendable destructor.
    virtual ~Loop(void);

    // ------------------------------------------------------------------------------------------ //

    /// @brief Runs all tasks in the loop.
    ///
    /// As long as there are items scheduled on the event loop, this method will not return. Once
    /// all tasks complete, and there are no connections keeping the loop alive, this method will
    /// return.
    void run(void);

    // ------------------------------------------------------------------------------------------ //

    /// @brief Gives access to the native loop handle.
    uv_loop_s* lowest_layer(void){
        return m_loop;
    }

    // ------------------------------------------------------------------------------------------ //
private:
    uv_loop_s* m_loop;
};

}
}

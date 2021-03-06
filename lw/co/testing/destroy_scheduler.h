
#include <thread>

namespace lw::co::testing {

/**
 * Destroys all schedulers on all threads.
 */
void destroy_all_schedulers();

/**
 * Destroys the scheduler tied to the given thread.
 */
void destroy_scheduler(std::thread::id thread_id);

/**
 * Destroys the scheduler for the current thread.
 */
inline void destroy_this_scheduler() {
  destroy_scheduler(std::this_thread::get_id());
}

}

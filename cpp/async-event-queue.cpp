/**@file
 * @copyright
 * @brief Async event queue code sample
 */

#include <boost/format.hpp>
#include <csignal>
#include "log.hpp"
#include "exception.hpp"
#include "async_event_loop.hpp"

namespace cpp_samples {

/**
 * @brief constructor
 */
AsyncEventQueue::AsyncEventQueue(const State* initial_state,
                                 const Handlers* handlers)
    : state_(initial_state)
    , handlers_(handlers)
    , shutdown_(true) {
}

/**
 * Sets up handlers and starts event thread in background
 */
void AsyncEventQueue::Setup( {
    event_thread_ = std::thread(&AsyncEventQueue::EventMain, this);
}

/**
 * Posts a message to tell the background event thread to shutdown
 */
void AsyncEventQueue::Teardown(void) {
    PostEvent(EventPtr(new event::Shutdown()));
    event_thread_.join();

    // No longer interested in messages queued before shutdown was processed
    while (!queue_.empty()) {
        queue_.pop();
    }
}

/**
 * Posts event to start
 */
void AsyncEventQueue::PostStart() {
    PostEvent(EventPtr(new event::Start()));
}

/**
 * Posts event to stop
 */
void AsyncEventQueue::PostStop(void) {
    PostEvent(EventPtr(new event::Stop()));
}

/**
 */
void AsyncEventQueue::PostEvent(EventPtr event) {
    {
        std::lock_guard<std::mutex> queue_guard(queue_mutex_);
        queue_.emplace(std::move(event));
    }
    queue_cond_.notify_one();
}

/**
 * @brief Event thread's main loop
 *
 * Reads events and processes using state machine
 *
 * Events are processed on a local queue while the queue_mutex_ is unlocked thereby
 * minimally blocking producing threads
 */
void AsyncEventQueue::EventMain(void) {
    // @todo: Give thread a name
    shutdown_ = false;
    decltype(queue_) local_queue;
    std::unique_lock<std::mutex> queue_lock(queue_mutex_);

    try {
        for (;;) {
            // Wait for an event
            while (queue_.empty()) {
                queue_cond_.wait(queue_lock);
            }

            // Let local_queue take ownership of all elements so main queue may
            // be unlocked
            local_queue = DequeueEvents();
            queue_lock.unlock();

            try {
                while (!local_queue.empty() && running()) {
                    EventPtr event(std::move(local_queue.front()));
                    local_queue.pop();
                    // Process the event using current state
                    event->Run(GetState(), this);
                }
            } catch(const Exception::Base&) {
                // @todo - handle known exceptions
            }

            if (!running()) {
                break;
            }

            queue_lock.lock();
        }
    } catch(const std::exception& e) { // unexpected exceptions
        Log::error(boost::str(boost::format(
                   "async event thread exited unexpectedly: %s") % e.what()));
        // Either kill process
        // kill(getpid(), SIGTERM); // Inform process that it should shutdown
        // Or call the background thread exited handler
    }
}

}  // namespace cpp_samples

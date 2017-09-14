/**@file
 * @copyright
 * @brief Async event queue code samples
 */

#ifndef _ASYNC_EVENT_QUEUE_HPP_
#define _ASYNC_EVENT_QUEUE_HPP_

#include <string>
#include <condition_variable>
#include <mutex>
#include <queue>
#include <thread>
#include "handler.hpp"
#include "state.hpp"

namespace cpp_samples {

/** Provides an API that asyncronously posts events to a background
 * thread and receive callbacks when actions corresponding to the
 * events are completed. Uses state machine.
 */
class AsyncEventQueue {
 public:
    explicit AsyncEventQueue(const State* initial_state,
                             const Handlers* handlers)

    /**
     * @brief Sets up the instance
     *
     * Call this method before doing anything else in this class.
     *
     * @warning do not call this method more than once
     *
     * @throws Exception::InvalidArgument if phandler is NULL
     */
    void Setup(void);

    /**
     * @brief Teardown
     *
     * Once you call this method, do not explicity do anything else in this class
     *
     * @warning do not call this method more than once
     */
    void Teardown(void);

    /**
     * @brief Posts start event to background thread
     */
    void PostStart();

    /**
     * @brief Posts stop event to background thread
     */
    void PostStop(void);

 protected:
    /**
     * @brief Is server event loop running?
     *
     * @returns true if it is running
     */
    bool running(void) { return !shutdown_; }

    /**
     * @returns current server state
     */
    const State* GetState(void) { return pstate_; }

    /**
     * @brief Change server state
     *
     * @warning state change must happen in the context of the event thread only
     *
     * @param[in] pstate points to the new state
     */
    void SetState(const State* pstate) { pstate_ = pstate; }

    /**
     * Posts a message to the background event thread
     */
    void PostEvent(EventPtr event);

    /**
     * Sets background thread to shutdown
     *
     * @warning shutdown must happen in the context of the event thread only
     */
    void PerformShutdown(void) override { shutdown_ = true; }

    /**
     * Dequeues events in the internal queue
     *
     * @returns internally queued events
     */
    std::queue<EventPtr> DequeueEvents() {
        decltype(queue_) local_queue; // empty queue
        queue_.swap(local_queue); // Move contents of queue_ into local_queue
        return std::move(local_queue);
    }

    /* Handlers for callback */
    Handlers* phandler_;

 private:
    // Event thread main
    void EventMain(void);

    //! Background event thread
    std::thread event_thread_;

    //! Queue for event messages
    std::queue<EventPtr> queue_;

    //! Queue mutex
    std::mutex queue_mutex_;

    //! Signal for new message
    std::condition_variable queue_cond_;

    //! Current state
    const State* state_;

    //! Server shutdown flag
    bool shutdown_;
};

// Unique Pointer type
typedef std::unique_ptr<AsyncEventQueue> AsyncEventQueuePtr;

}  // namespace cpp_samples

#endif  //  _ASYNC_EVENT_QUEUE_HPP_

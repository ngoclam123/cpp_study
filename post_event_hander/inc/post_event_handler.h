#ifndef POST_EVENT_HANDLER_H
#define POST_EVENT_HANDLER_H

#include <cstdint>
#include <cstdio>
#include <memory>
#include <thread>
#include <mutex>
#include <condition_variable>
#include <atomic>
#include <map>
#include <list>
#include <utility> // For std::move
#include <ctime>   // For clock_gettime

/**
 * @brief An interface for a generic event handler.
 * Users of EventBox should implement this interface.
 */
class IEventHandler
{
public:
    virtual ~IEventHandler() = default;
    virtual void handleEvent(const uint16_t& event) = 0;
};

using eh_ptr = std::shared_ptr<IEventHandler>;

/**
 * @brief A class that manages a queue of timed events and processes them in a dedicated thread.
 */
class EventBox
{
public:
    /**
     * @brief Construct a new Event Box object and starts its processing thread.
     * @param _eh A shared pointer to an object that implements the IEventHandler interface.
     */
    explicit EventBox(eh_ptr _eh);

    /**
     * @brief Destroy the Event Box object, ensuring a graceful shutdown of the processing thread.
     */
    ~EventBox();

    void postEventDelay(const uint16_t& event, const uint64_t& duration);
    void postEvent(const uint16_t& event);

private:
    void processEventBox();
    uint64_t getCurrentTime();

    eh_ptr eh;
    std::map<uint64_t, std::list<uint16_t>> eventBox;
    std::mutex eventBoxMtx;
    std::condition_variable eventBoxCv;
    std::thread processThread;
    std::atomic<bool> stop_flag{false};
};

#endif // POST_EVENT_HANDLER_H
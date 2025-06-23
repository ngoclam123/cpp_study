#include "post_event_handler.h"
#include <chrono>

static void printMap(const std::map<uint64_t, std::list<uint16_t>>& eventBox)
{
    for(const auto& timeEvent: eventBox)
    {
        for(const auto& event: timeEvent.second)
        {
            // Use %lu for uint64_t and %u for uint16_t for type correctness.
            printf("time %lu event %u\n", timeEvent.first, event);
        }
    }
    printf("\n");
}

EventBox::EventBox(eh_ptr _eh) : eh(std::move(_eh))
{
    if (eh.get() == nullptr)
    {
        fprintf(stdout, "Invalid Event Handler, never start Event Box processer\n");
        return;
    }
    eventBox.clear();
    
    // Store the thread to be joined later for a graceful shutdown.
    // Detaching threads is generally discouraged as it can lead to lifetime issues.
    processThread = std::thread(&EventBox::processEventBox, this);
}

// A destructor is added to ensure the background thread is properly shut down.
EventBox::~EventBox()
{
    stop_flag = true;
    eventBoxCv.notify_all(); // Wake up the thread if it's waiting.
    if (processThread.joinable())
    {
        processThread.join();
    }
}

void EventBox::processEventBox()
{
    printf("%s started\n", __func__);
    while(!stop_flag)
    {
        std::list<uint16_t> eventsToExecute;
        {
            std::unique_lock<std::mutex> lk(eventBoxMtx);
            if (eventBox.empty()) {
                // If the event box is empty, wait until a new event is added or stop is requested.
                eventBoxCv.wait(lk, [this]{ return stop_flag || !eventBox.empty(); });
            } else {
                // Calculate time until the next event and wait for that duration.
                // Wait until the time of the next event. Using an absolute time point
                // with wait_until is more robust against spurious wakeups than using a relative duration.
                auto next_event_tp = std::chrono::steady_clock::time_point(std::chrono::milliseconds(eventBox.begin()->first));
                eventBoxCv.wait_until(lk, next_event_tp);
            }

            if (stop_flag) break; // Exit if shutdown was requested while waiting.

            // After waking up, collect all events that are due.
            const uint64_t monoNow = getCurrentTime();

            // Find the first event that is scheduled for the future.
            auto first_not_due = eventBox.upper_bound(monoNow);

            // All events from the beginning up to 'first_not_due' are ready.
            for (auto it = eventBox.begin(); it != first_not_due; ++it) {
                // Move events efficiently using splice, which avoids copying elements.
                eventsToExecute.splice(eventsToExecute.end(), it->second);
            }
            // Erase all the processed time entries from the map in a single, efficient operation.
            eventBox.erase(eventBox.begin(), first_not_due);
        }
        
        // Execute handlers outside the lock to prevent potential deadlocks.
        for (const auto& event: eventsToExecute)
        {
            eh->handleEvent(event);
        }
    }
    printf("%s stopped\n", __func__);
}

uint64_t EventBox::getCurrentTime()
{
    struct timespec ts;
    if (clock_gettime(CLOCK_MONOTONIC, &ts) == -1)
    {
        perror("clock_gettime"); // It's good practice to log the actual error.
        return 0;
    }
    return (uint64_t)ts.tv_sec * 1000 + ts.tv_nsec / 1000000;
}

void EventBox::postEventDelay(const uint16_t& event, const uint64_t& duration)
{
    const uint64_t executedTime = getCurrentTime() + duration;
    bool is_new_earliest = false;
    {
        std::lock_guard<std::mutex> lk(eventBoxMtx);

        // Optimization: check if the new event is the earliest one.
        if (eventBox.empty() || executedTime < eventBox.begin()->first) {
            is_new_earliest = true;
        }

        auto it = eventBox.find(executedTime);
        if (it != eventBox.end())
        {
            it->second.push_back(event);
        }
        else
        {
            // Corrected type to std::list<uint16_t> to fix the compilation error.
            std::list<uint16_t> eventList = {event};
            eventBox.insert({executedTime, std::move(eventList)});
        }
        // printMap(eventBox);
    }

    // Only notify the processing thread if the new event might need to be handled sooner
    // than what the thread is currently waiting for. This avoids unnecessary wakeups.
    if (is_new_earliest) {
        eventBoxCv.notify_all(); // notify_one() is also an option if only one thread consumes.
    }
}

void EventBox::postEvent(const uint16_t& event)
{
    postEventDelay(event, 0);
}

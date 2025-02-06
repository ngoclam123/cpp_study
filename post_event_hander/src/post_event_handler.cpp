#include "post_event_handler.h"

static void printMap(std::map<uint64_t, std::list<int>> eventBox)
{
    for(auto& timeEvent: eventBox)
    {
        for(auto& event: timeEvent.second)
        {
            printf("time %ld event %d\n", timeEvent.first, event);
        }
    }
    printf("\n");
}

EventBox::EventBox(eh_ptr _eh)
{
    eh = std::move(_eh);
    eventBox.clear();
    std::thread processThread([this]()
    {
        processEventBox();
    });
    processThread.detach();
}

void EventBox::processEventBox()
{
    printf("%s started\n", __func__);
    while(true)
    {
        std::list<int> executedEvent;
        {
            std::unique_lock<std::mutex> lk(eventBoxMtx);
            uint64_t sleepDur = 500U;
            if (eventBox.size() > 0) sleepDur = eventBox.begin()->first - getCurrentTime();
            eventBoxCv.wait_for(lk, std::chrono::milliseconds(sleepDur));
            uint64_t monoNow = getCurrentTime();
            std::list<uint64_t> eraseList;
            for( auto& timeEventsPair: eventBox)
            {
                if (timeEventsPair.first <= monoNow)
                {
                    eraseList.push_back(timeEventsPair.first);
                    for( auto& event: timeEventsPair.second)
                    {
                        executedEvent.push_back(event);
                    }
                }
                else { break;}
            }
            for (auto& time: eraseList)
            {
                eventBox.erase(time);
            }
        }
        for (auto& event: executedEvent)
        {
            std::thread handlerThread([this, event]{
                eh->handleEvent(event);
            });
            handlerThread.join();
        }
    }
}

uint64_t EventBox::getCurrentTime()
{
    struct timespec ts;
    if (clock_gettime(CLOCK_MONOTONIC, &ts) == -1)
    {
        // if fail, execute immediately
        return 0;
    }
    return (ts.tv_sec * 1000 + ts.tv_nsec / 1000000);
}

void EventBox::postEventDelay(const uint16_t& event, const uint64_t& duration)
{
    const uint64_t executedTime = getCurrentTime() + duration;
    // can execute more than 1 event at the same time
    {
        std::lock_guard<std::mutex> lk(eventBoxMtx);
        auto it = eventBox.find(executedTime);
        if (it != eventBox.end())
        {
            it->second.push_back(event);
        }
        else
        {
            std::list<int> eventList = {event};
            eventBox.insert({executedTime, eventList});
        }
    }
    // printMap(eventBox);
    eventBoxCv.notify_all();
}

void EventBox::postEvent(const uint16_t& event)
{
    postEventDelay(event, 0);
}

// bool EventBox::cancelEvent(const uint16_t& event)
// {
//     {
//         std::lock_guard<std::mutex> lk(eventBoxMtx);
//         auto it = eventBox.find(executedTime);
//         if (it != eventBox.end())
//         {
//             it->second.push_back(event);
//         }
//         else
//         {
//             std::list<int> eventList = {event};
//             eventBox.insert({executedTime, eventList});
//         }
        
//         for (auto& timeEvent: eventBox)
//         {
//             for (auto time: )
//         }
//     }
//     // printMap(eventBox);
//     eventBoxCv.notify_all();
// }

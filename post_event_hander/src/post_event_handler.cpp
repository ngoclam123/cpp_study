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
    if (_eh.get() == nullptr)
    {
        fprintf(stdout, "Invalid Event Handler, never start Event Box processer");
        return;
    }
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
        std::list<uint16_t> executedEvent;
        {
            std::unique_lock<std::mutex> lk(eventBoxMtx);
            int64_t sleepDur{500U};
            sleepDur = (eventBox.size() > 0)? (eventBox.begin()->first - getCurrentTime()) : sleepDur;
            sleepDur = sleepDur < 0 ? 0 : sleepDur; // incase getCurrentTime() jump to next miliseconds -> sleep for so long
            eventBoxCv.wait_for(lk, std::chrono::milliseconds(sleepDur));
            uint64_t monoNow = getCurrentTime();
            std::list<uint64_t> eraseList;
            for( auto& timeEventsPair: eventBox)
            {
                if (timeEventsPair.first <= monoNow)
                {
                    eraseList.push_back(timeEventsPair.first);
                    for(const auto& event: timeEventsPair.second)
                    {
                        executedEvent.push_back(event);
                    }
                }
                else break;
            }
            for (const auto& time: eraseList)
            {
                eventBox.erase(time);
            }
        }
        
        for (const auto& event: executedEvent)
        {
            eh->handleEvent(event);
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
        // printMap(eventBox);
    }
    eventBoxCv.notify_all();
}

void EventBox::postEvent(const uint16_t& event)
{
    postEventDelay(event, 0);
}
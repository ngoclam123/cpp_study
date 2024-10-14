#include "post_event_handler.h"


EventBox::EventBox()
{
    eventBox.clear();
}

void EventBox::processEventBox()
{

}

uint64_t EventBox::getShorter(const uint64_t& executedTime)
{
    std::lock_guard<std::mutex> lk(eventBoxMtx);
    for(auto& pair: eventBox)
    {
        if (pair.first <= executedTime)
        {
            return pair.first;
        }
    }
    return executedTime;
}

uint64_t EventBox::getCurrentTime()
{
    struct timespec ts;
    if (clock_gettime(CLOCK_MONOTONIC, &ts) == -1)
    {
        printf("%s fail %d", __func__, errno);
        // if fail, execute immediately
        return 0;
    }
    return ts.tv_nsec*1000000;
}

void EventBox::postEventDelay(const uint16_t& event, const uint64_t& duration)
{
    const uint64_t executedTime = getCurrentTime() + duration;

    // can execute more than 1 event at the same time
    auto it = eventBox.find(executedTime);
    if (it != eventBox.end())
    {
        it->second.push_back(event);
        return;
    }

    // the shorest executed time that shorter

}

void EventBox::postEvent(const uint16_t& event)
{
    printf("%s %d", __func__, event);
    postEventDelay(event, 0);
}
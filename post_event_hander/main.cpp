#include <cstdio>
#include <unistd.h>
#include <iostream>
#include "inc/post_event_handler.h"
#include <thread>
#include <functional>

std::unique_ptr<EventBox> eb;
class EventhandlerImpl: public EventHandler
{
public:
    enum event_t: uint16_t
    {
        event1 = 1,
        event2 = 2,
    };

    void handleEvent(const uint16_t& event) override
    {
        switch (event)
        {
        case event_t::event1:
            printf("receive event1 at %ld executed thread: %ld\n", EventBox::getCurrentTime(), std::hash<std::thread::id>{}(std::this_thread::get_id()));
            break;
        case event_t::event2:
            printf("receive event2 at %ld executed thread: %ld\n", EventBox::getCurrentTime(), std::hash<std::thread::id>{}(std::this_thread::get_id()));
            eb->postEventDelay(EventhandlerImpl::event_t::event2, 2000);
            break;
        default:
            printf("Invalid event\n");
            break;
        }
    }
};

int main()
{
    printf("Hello world!!\nProcess started at %ld\n", EventBox::getCurrentTime());
    // below objects must be retain till the end of the program
    std::unique_ptr<EventHandler> ehi = std::make_unique<EventhandlerImpl>();
    eb = std::make_unique<EventBox>(std::move(ehi));
    eb->postEventDelay(EventhandlerImpl::event_t::event1, 2000);
    sleep(3);
    eb->postEventDelay(EventhandlerImpl::event_t::event2, 2000);
    while (true)
    {
        sleep(2);
    }
}

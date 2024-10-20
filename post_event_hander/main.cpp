#include <cstdio>
#include <unistd.h>
#include <iostream>
#include "inc/post_event_handler.h"

class EventhandlerImpl: public EventHandler
{
public:
    enum class event_t
    {
        event1 = 1,
        event2 = 2,
    };

    void handleEvent(const uint16_t& event) override
    {
        std::cout << __func__ << " " << event << std::endl;
        switch (event)
        {
        case 1:
            std::cout << "receive event1 at " << EventBox::getCurrentTime() << std::endl;
            break;
        case 2:
            std::cout << "receive event2 at " << EventBox::getCurrentTime() << std::endl;
            break;
        default:
            break;
        }
    }
};

int main()
{
    // printf("Hello World!");
    std::cout << "Hello World!" << std::endl;
    std::unique_ptr<EventHandler> ehi = std::make_unique<EventhandlerImpl>();
    std::unique_ptr<EventBox> eb = std::make_unique<EventBox>(std::move(ehi));
    sleep(1);
    eb->postEventDelay(1, 5000);
    // sleep(2);
    // eb->postEventDelay(2, 2000);
    // sleep(3);
    // eb->postEventDelay(2, 2000);
    // sleep(4);
    // eb->postEventDelay(2, 2000);
    while (true)
    {
        sleep(2);
    }
}
#ifndef __POST_EVENT_HANDLER_HEADER_H__
#define __POST_EVENT_HANDLER_HEADER_H__

#include <list>
#include <map>
#include <chrono>
#include <thread>
#include <condition_variable>
#include <time.h>

// Should be replaced by loggin later
#include <cstdio>

//! This class must be singleton
class EventHandler
{
public:
    EventHandler() = default;
    ~EventHandler() = default;

    /**
     * @brief New thread is created by 
     * 
     * @param event 
     */
    virtual void handleEvent(const uint16_t& event) = 0;
};

class EventBox
{
private:
    //! @brief this container contain map executed time- event list from shortest to longest
    //! @note Executed time is calculated by duration + monil
    std::map<uint64_t, std::list<int>> eventBox;

    std::mutex eventBoxMtx;
    
    /**
     * @brief Start process event box
     * 
     */
    void processEventBox();
    
    /**
     * @brief Find the shortest executed time that shorter input
     * 
     * @param[in] executedTime 
     */
    uint64_t getShorter(const uint64_t& executedTime);

    /**
     * @brief Get the system uptime, MONOTONIC.
     * 
     * @return[out] uint64_t in miliseconds
     */
    uint64_t getCurrentTime();
public:

    /**
     * @brief Process input event after the duration milisec delay
     * 
     * @param[in] event 
     * @param[in] duration 
     */
    void postEventDelay(const uint16_t& event, const uint64_t& duration);
    
    /**
     * @brief Handle event immediately
     * 
     * @param[in] event 
     */
    void postEvent(const uint16_t& event);
    
public:

    EventBox();
    ~EventBox() = default;
};



#endif
#pragma once

#include <cstddef>
#include <vector>

namespace engine {

template <class Event>
class EventQueue
{
public:
    void push(const Event &event);
    std::vector<Event> take();
    const std::vector<Event> &events() const;
    void clear();
    std::size_t size() const;
    bool empty() const;

private:
    std::vector<Event> m_events;
};

} // namespace engine

#include "EventQueue.tpp"

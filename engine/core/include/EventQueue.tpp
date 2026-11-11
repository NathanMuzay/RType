#pragma once

#include <utility>

namespace engine {

template <class Event>
void EventQueue<Event>::push(const Event &event)
{
    m_events.push_back(event);
}

template <class Event>
std::vector<Event> EventQueue<Event>::take()
{
    std::vector<Event> events;
    events.swap(m_events);
    return events;
}

template <class Event>
const std::vector<Event> &EventQueue<Event>::events() const
{
    return m_events;
}

template <class Event>
void EventQueue<Event>::clear()
{
    m_events.clear();
}

template <class Event>
std::size_t EventQueue<Event>::size() const
{
    return m_events.size();
}

template <class Event>
bool EventQueue<Event>::empty() const
{
    return m_events.empty();
}

} // namespace engine

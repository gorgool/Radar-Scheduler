#pragma once

#include <functional>
#include <queue>
#include <vector>
#include <cinttypes>

class Scheduler;

/*
  Model event.
*/
struct Event
{
  // Activation time, ns.
  std::uint64_t trigger_time;

  // event handler function.
  // handler signature: 
  //     input - class Scheduler referance.
  //     return - true for success, false otherwise
  std::function<bool(Scheduler&)> exec;

  // Constructor
  Event(std::uint64_t _trigger_time, std::function<bool(Scheduler&)>&& callback) : trigger_time(_trigger_time), exec(callback) {}

  friend bool operator<(const Event& lhs, const Event& rhs)
  {
    return lhs.trigger_time > rhs.trigger_time;
  }
};

/*
  Model events queue wrapper.
*/
class EventQueue
{
  // Model events queue. Sorted by trigger time.
  std::priority_queue<Event> event_queue;
public:

  // Constructor
  EventQueue() {}

  // Add event
  void add_event(const Event& ev)
  {
    event_queue.push(ev);
  }

  // Return all events which triiger time equal to time. If there are no such events return empty vector.
  std::vector<Event> get_events(const std::uint64_t time)
  {
    std::vector<Event> ret;

    if (event_queue.empty())
      return ret;

    while (true)
    {
      if (event_queue.empty())
        break;

      auto ev = event_queue.top();
      if (ev.trigger_time == time)
      {
        ret.push_back(ev);
        event_queue.pop();
      }
      else
      {
        break;
      }
    }

    return ret;
  }

  std::size_t size() const
  {
      return event_queue.size();
  }

  void clear()
  {
      event_queue = std::priority_queue<Event>();
  }
};

#pragma once

#include <functional>
#include <queue>
#include <vector>
#include <cinttypes>

class Scheduler;

/*
  ������� ������������� ������������.
  trigger_time - ����� ����������� �������, ��
  exec - ������� ���������� �������
*/
struct Event
{
  // ����� ����������� �������, ��
  std::uint64_t trigger_time;

  // ������� ���������� �������.
  // ������� �������� - ������ �� ����������.
  // ���������� true � ������ ��������� ����������, false �����.
  std::function<bool(Scheduler&)> exec;

  Event(std::uint64_t _trigger_time, std::function<bool(Scheduler&)>&& callback) : trigger_time(_trigger_time), exec(callback) {}

  friend bool operator<(const Event& lhs, const Event& rhs)
  {
    return lhs.trigger_time > rhs.trigger_time;
  }
};

/*
  ����� ������� �������.
  ������� ����������� �� ������� �����������.
*/
class EventQueue
{
  std::priority_queue<Event> event_queue;
public:
  EventQueue() {}

  // �������� �������
  void add_event(const Event& ev)
  {
    event_queue.push(ev);
  }

  // ���������� ��� �������, ����� ����������� ������� ����� time
  // ���� ����� ������� ���, �� ���������� ������ ������
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

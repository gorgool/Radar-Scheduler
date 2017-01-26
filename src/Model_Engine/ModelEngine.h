#pragma once

#include "EventQueue.h"
#include "Scheduler.h"
#include "CommandProcessor.h"
#include "ModelState.h"
#include <cinttypes>
#include <chrono>
#include <iostream>

class ModelEngine
{
  // ���
  Scheduler shed;

  // ��������� ���������
  CommandProcessor command_processor;

  // ������� ������� �������������
  EventQueue ev_queue;

  // ������� ����� �������������, ��
  std::uint64_t model_time = 0;

  // �������� ������������, ��
  std::uint64_t time_step = 1000000;

  // ���� ���������� �������� �������������
  bool running = false;

public:

  // �����������. ��������� ������� ������� �������������.
  ModelEngine() : command_processor("log.txt") {}

  // ������ ����� �������������
  ModelState run(const std::uint32_t times = 1)
  {
    ModelState ret;

    try
    {
        shed.save_timilines(ret.dcu_timeline_before, ret.au_tr_timeline_before, ret.au_rs_timeline_before);

        for(std::uint32_t counter = 0; counter < times; ++counter)
        {
            // ��������� ������ ������� �� ������� ����� �������������
            auto events = ev_queue.get_events(model_time);

            if (!events.empty())
            {
              // ���������� �������
              for (auto& ev : events)
              {
                auto ret = ev.exec(shed);
                if (!ret)
                  throw ModelException("ERROR: Event execution error.");
              }
            }

            // ������ ������������
            auto command_list = shed.run(model_time);

            // ���������� ���� �������������� �� ������� ����� �������
            command_processor.append(command_list);

            if (counter == times - 1)
            {
                ret.time = model_time;
                ret.valid_state = command_processor.validate();
            }

            // ���������� ������ ���������� ��� �������� �����
            command_processor.run(model_time);
            command_processor.save_execed_commands(ret);
            // ���������� �������� ������� �������������
            model_time += time_step;
        }

        command_processor.get_statistics(ret);
        shed.get_statistics(ret);
        shed.save_timilines(ret.dcu_timeline_after, ret.au_tr_timeline_after, ret.au_rs_timeline_after);
    }
    catch (const ModelException& ex)
    {
        model_time = 0;
        shed.reset();
        command_processor.clear();
        ev_queue.clear();
        ret.error_string = ex.what();
    }

    return ret;
  }

  // �������� ������� �������������
  void add_event(const Event& ev)
  {
      ev_queue.add_event(ev);
  }

  std::size_t events_count() const
  {
      return ev_queue.size();
  }
};

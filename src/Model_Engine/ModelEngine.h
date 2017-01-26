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
  // КВД
  Scheduler shed;

  // Командный процессор
  CommandProcessor command_processor;

  // Очередь событий моделирования
  EventQueue ev_queue;

  // Текущее время моделирования, мс
  std::uint64_t model_time = 0;

  // Интервал планирования, нс
  std::uint64_t time_step = 1000000;

  // Флаг выполнения процесса моделирования
  bool running = false;

public:

  // Конструктор. Заполняет очередь событий моделирования.
  ModelEngine() : command_processor("log.txt") {}

  // Запуск цикла моделирования
  ModelState run(const std::uint32_t times = 1)
  {
    ModelState ret;

    try
    {
        shed.save_timilines(ret.dcu_timeline_before, ret.au_tr_timeline_before, ret.au_rs_timeline_before);

        for(std::uint32_t counter = 0; counter < times; ++counter)
        {
            // Получение списка событий на текущее время моделирования
            auto events = ev_queue.get_events(model_time);

            if (!events.empty())
            {
              // Исполнение событий
              for (auto& ev : events)
              {
                auto ret = ev.exec(shed);
                if (!ret)
                  throw ModelException("ERROR: Event execution error.");
              }
            }

            // Запуск планировщика
            auto command_list = shed.run(model_time);

            // Добавление всех спланированных на текущем такте комманд
            command_processor.append(command_list);

            if (counter == times - 1)
            {
                ret.time = model_time;
                ret.valid_state = command_processor.validate();
            }

            // Выполнение кщманд управления для текущего такта
            command_processor.run(model_time);
            command_processor.save_execed_commands(ret);
            // Увеличение текущего времени моделирования
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

  // Дабавить событие моделирования
  void add_event(const Event& ev)
  {
      ev_queue.add_event(ev);
  }

  std::size_t events_count() const
  {
      return ev_queue.size();
  }
};

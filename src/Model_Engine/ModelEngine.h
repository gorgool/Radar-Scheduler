#pragma once

#include "EventQueue.h"
#include "Scheduler.h"
#include "CommandProcessor.h"
#include "ModelState.h"
#include <cinttypes>
#include <chrono>
#include <iostream>

/*
  Model engine. Control execution of the model.
*/

class ModelEngine
{
  // Planner.
  Scheduler shed;

  // Commands processor.
  CommandProcessor command_processor;

  // Event queue.
  EventQueue ev_queue;

  // Current modelling time, ns.
  std::uint64_t model_time = 0;

  // Modelling step, ns.
  std::uint64_t time_step = 1000000;

public:

  // Constructor.
  ModelEngine() : command_processor("log.txt") {}

  // Execution of "times" modelling cycles. 
  ModelState run(const std::uint32_t times = 1)
  {
    ModelState ret;

    try
    {
        // For debug.
        shed.save_timilines(ret.dcu_timeline_before, ret.au_tr_timeline_before, ret.au_rs_timeline_before);

        for(std::uint32_t counter = 0; counter < times; ++counter)
        {
            // Get event for current time.
            auto events = ev_queue.get_events(model_time);

            if (!events.empty())
            {
              // Execute events.
              for (auto& ev : events)
              {
                auto ret = ev.exec(shed);
                if (!ret)
                  throw ModelException("ERROR: Event execution error.");
              }
            }

            // Execute planner.
            auto command_list = shed.run(model_time);

            // Add all planned command in the queue.
            command_processor.append(command_list);

            // For debug. If it is the last cycle in this run check validate result.
            if (counter == times - 1)
            {
                ret.time = model_time;
                ret.valid_state = command_processor.validate();
            }

            // Execute all command for current time.
            command_processor.run(model_time);

            // For debug.
            command_processor.save_execed_commands(ret);

            // Next modelling time.
            model_time += time_step;
        }

        // For debug.
        command_processor.get_statistics(ret);
        shed.get_statistics(ret);
        shed.save_timilines(ret.dcu_timeline_after, ret.au_tr_timeline_after, ret.au_rs_timeline_after);
    }
    catch (const ModelException& ex)
    {
        // Reset model engine
        model_time = 0;
        shed.reset();
        command_processor.clear();
        ev_queue.clear();
        ret.error_string = ex.what();
    }

    return ret;
  }

  // Add modelling event
  void add_event(const Event& ev)
  {
      ev_queue.add_event(ev);
  }

  // Return size of event queue
  std::size_t events_count() const
  {
      return ev_queue.size();
  }
};

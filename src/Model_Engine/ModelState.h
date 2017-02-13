#pragma once

#include <cinttypes>
#include <vector>
#include <string>
#include "Query.h"
#include "ControlCommand.h"
#include "Timeline.h"

/*
  State of the Planner Model
*/

struct ModelState
{
  // Model referance time
  std::uint64_t time;

  // Validation flag at referance time
  bool valid_state = false;

  // All active queries at referance time
  std::vector<Query> active_queries;

  // List of all executed queries at referance time
  std::vector<Query> processed_queries;

  // List of all planned commands at referance time
  std::vector<ControlCommand> planned_commands;

  // List of all executed commands at referance time
  std::vector<ControlCommand> execed_commands;

  // DCU Timeline state before scheduler activation at referance time
  Timeline dcu_timeline_before;

  // DCU Timeline state after scheduler activation at referance time
  Timeline dcu_timeline_after;

  // AU Transmitter Timeline state before scheduler activation at referance time
  Timeline au_tr_timeline_before;

  // AU Transmitter Timeline state after scheduler activation at referance time
  Timeline au_tr_timeline_after;

  // AU Receiver Timeline state before scheduler activation at referance time
  Timeline au_rs_timeline_before;

  // AU Receiver Timeline state after scheduler activation at referance time
  Timeline au_rs_timeline_after;

  // Command history
  std::vector<std::string> command_history;

  // Error string. If there are not errors the string is empty.
  std::string error_string;
}; 

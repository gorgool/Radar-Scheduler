#pragma once
#include <queue>
#include <cinttypes>
#include <fstream>
#include <string>
#include <iostream>
#include <algorithm>
#include <map>

#include "ModelException.h"
#include "Settings.h"
#include "ModelState.h"
#include "ControlCommand.h"

/*
  Class that saves all commands and allows to "execute" them for specific time range.
*/
class CommandProcessor
{
  // All planned control commands queue. Sorted by referance time.
  std::priority_queue<ControlCommand> command_queue;

  // List of all executed (in last run) commands. For debug.
  std::vector<ControlCommand> execed_commands;

  // Command history log file. For debug.
  std::ofstream log;

  // "Command type" -> "Description" translation table. For debug.
  std::map<CommandType, std::string> command_tbl;
public:
  // Constuctor. log_filename - command history file name
  CommandProcessor(const std::string& log_filename);

  // Add all commands from command_list, except "nop"" type.
  void append(const std::vector<ControlCommand>& command_list);

  // Clear command queue.
  void clear();

  // "Execute" commands in time range from time to time + 1 microsecond (us).
  // Executed commands are deleted from command queue.
  void run(const std::uint64_t time);

  // Validate command_queue for complience with test project. For debug.
  bool validate();

  // Save in model_state states of command_queue and execed_commands.
  void get_statistics(ModelState& model_state);

  // Save in model_state command history.
  void save_execed_commands(ModelState& model_state);
};

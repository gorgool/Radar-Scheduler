#include "CommandProcessor.h"

// Constuctor. log_filename - command history file name.
CommandProcessor::CommandProcessor(const std::string & log_filename) : log(log_filename)
{
  if (log.good() == false)
    throw ModelException("ERROR: Opening log file error.");

  command_tbl[CommandType::ct_nop] = "Empty command";
  command_tbl[CommandType::ct_protect_command] = "Protection turn off command";
  command_tbl[CommandType::ct_receive_command] = "Receive command";
  command_tbl[CommandType::ct_rs_rephase_command] = "Receiver rephase command";
  command_tbl[CommandType::ct_tech_command] = "Maintance query command";
  command_tbl[CommandType::ct_transmit_command] = "Transmit command";
  command_tbl[CommandType::ct_tr_rephase_command] = "Transmitter rephase command";
}

// Add all commands from command_list, except "nop"" type.
void CommandProcessor::append(const std::vector<ControlCommand>& command_list)
{
  for (const auto& command : command_list)
  {
    if (command.type != CommandType::ct_nop)
      command_queue.push(command);
  }
}

// Clear command queue.
void CommandProcessor::clear()
{
  command_queue = std::priority_queue<ControlCommand>();
}

// "Execute" commands in time range from time to time + 1 microsecond.
  // Executed commands are deleted from command queue.
void CommandProcessor::run(const std::uint64_t time)
{
  execed_commands.clear();

  if (command_queue.empty())
    return;

  while (true)
  {
    if (command_queue.empty())
      break;

    auto ev = command_queue.top();
    if (ev.execution_time < time +  1e6)
    {
      execed_commands.push_back(ev);
      // "Execution" and logging.
      log << ev.execution_time << " [" << ev.query_id << "] : ";
      switch (ev.type)
      {
      case CommandType::ct_receive_command:
        log << "Receive.\n";
        break;
      case CommandType::ct_transmit_command:
        log << "Transmit.\n";
        break;
      case CommandType::ct_tr_rephase_command:
        log << "Transmit Rephase.\n";
        break;
      case CommandType::ct_rs_rephase_command:
        log << "Receive Rephase.\n";
        break;
      case CommandType::ct_protect_command:
        log << "Turn off protection.\n";
        break;
      case CommandType::ct_tech_command:
        log << "Tech.\n";
        break;
      default:
        log << "ERROR: unknown command in command processor run.\n";
        break;
      }
      
      command_queue.pop();
    }
    else
    {
      break;
    }
  }
}

// Validate command_queue for complience with test project. For debug.

bool CommandProcessor::validate()
{

  std::vector<ControlCommand> commands(execed_commands.begin(), execed_commands.end());
  std::vector<ControlCommand> current_commands;
  {
    auto command_copy = command_queue;
    while (command_copy.empty() != true)
    {
      current_commands.push_back(command_copy.top());
      commands.push_back(command_copy.top());
      command_copy.pop();
    }
  }
  
  // Check rephase command period.
  // Requirement: rephase command must be => 500 us.

  {
    ControlCommand prev_tr_rephase;
    ControlCommand prev_rs_rephase;
    for (const auto& c : current_commands)
    {
      if (c.type == CommandType::ct_tr_rephase_command)
      {
        if (prev_tr_rephase.type == CommandType::ct_nop)
          prev_tr_rephase = c;
        else
        {
          if (c.execution_time - prev_tr_rephase.execution_time < settings::au_command_delay)
          {
            throw ModelException("VALIDATE ERROR: Transmit rephase command is not valid.");
          }
          else
          {
            prev_tr_rephase = c;
          }
        }
      }
      if (c.type == CommandType::ct_rs_rephase_command)
      {
        if (prev_rs_rephase.type == CommandType::ct_nop)
          prev_rs_rephase = c;
        else
        {
          if (c.execution_time - prev_rs_rephase.execution_time < settings::au_command_delay)
          {
            throw ModelException("VALIDATE ERROR: Receive rephase command is not valid.");
          }
          else
          {
            prev_rs_rephase = c;
          }
        }
      }
    }
  }

  // Check receiving protection command execution.
  // Requirement: Protection command must be executed no later than 3 us before any receive command.

  {
    for (const auto& c : current_commands)
    {
      if (c.type == CommandType::ct_receive_command)
      {
        auto it = std::find_if(commands.begin(), commands.end(), [&](const auto& val) { return val.type == CommandType::ct_protect_command && val.query_id == c.query_id; });
        if (it != commands.end())
        {
          if (c.execution_time - it->execution_time < settings::channel_switch_delay)
          {
            throw ModelException("VALIDATE ERROR: Receive command was executed before protection was turned off.");
          }
        }
        else
        {
          throw ModelException("VALIDATE ERROR: Receiver protection was not turned off.");
        }
      }
    }
  }

  // Check setting up antenna direction before any transmition or receiving   
  // Requirement: Rephase command must be executed no later than 270 us before any receive or transmit command.

  {
    for (const auto& c : current_commands)
    {
      if (c.type == CommandType::ct_receive_command)
      {
        auto it = std::find_if(commands.begin(), commands.end(), [&](const auto& val) { return val.type == CommandType::ct_rs_rephase_command && val.query_id == c.query_id; });
        if (it != commands.end())
        {
          if (c.execution_time - it->execution_time < settings::phase_delay)
          {
            throw ModelException("VALIDATE ERROR: Receive command was executed before receiver has completed rephase command.");
          }
        }
        else
        {
          throw ModelException("VALIDATE ERROR: Receiver rephase was not been executed after transmit command.");
        }
      }

      if (c.type == CommandType::ct_transmit_command)
      {
        auto it = std::find_if(commands.begin(), commands.end(), [&](const auto& val) { return val.type == CommandType::ct_tr_rephase_command && val.query_id == c.query_id; });
        if (it != commands.end())
        {
          if (c.execution_time - it->execution_time < settings::phase_delay)
          {
            throw ModelException(" VALIDATE ERROR: Transmit command was executed before transmiter has completed rephase command.");
          }
        }
        else
        {
          throw ModelException("VALIDATE ERROR: Transmit command was executed before transmiter has completed rephase command.");
        }
      }
    }
  }

  // Transmit command check
  // Requirement: Transmit command must be executed no earlier than complition of previous transmit command 

  {
    ControlCommand prev_transmit;
    for (const auto& c : current_commands)
    {
      if (c.type == CommandType::ct_transmit_command)
      {
        if (prev_transmit.type == CommandType::ct_nop)
          prev_transmit = c;
        else
        {
          if (prev_transmit.execution_time + prev_transmit.execution_length >  c.execution_time)
          {
            throw ModelException("VALIDATE ERROR: Transmit command was executed in energy restore state.");
          }
          else
          {
            prev_transmit = c;
          }
        }
      }
    }
  }

  // Common 
  // Requirement: Execution time and execution length must not overlap for any adjacent command. 
  {
    for (const auto& c : current_commands)
    {
      if (c.type == CommandType::ct_receive_command)
      {

        if (c.execution_time < c.referance_time)
        {
          throw ModelException("VALIDATE ERROR: Execution time is smaller than referance time.");
        }

        if (c.execution_time - c.referance_time < settings::reserve_time)
        {
          throw ModelException("VALIDATE ERROR: Distance between reference time and execution time of receive command is not valid.");
        }
      }
      if (c.type == CommandType::ct_transmit_command)
      {
        if (c.execution_time - c.referance_time < settings::reserve_time + settings::blank_delay + settings::frequency_delay)
        {
          throw ModelException("VALIDATE ERROR: Distance between reference time and execution of transmit command time is not valid.");
        }
      }
    }
  }

  return true;
}

// Save in model_state states of command_queue and execed_commands.
void CommandProcessor::get_statistics(ModelState & model_state)
{
  auto planned_commands_copy = command_queue;
  while (planned_commands_copy.empty() != true)
  {
    model_state.planned_commands.push_back(planned_commands_copy.top());
    planned_commands_copy.pop();
  }
  
  model_state.execed_commands.resize(execed_commands.size());
  std::copy(execed_commands.begin(), execed_commands.end(), model_state.execed_commands.begin());
}

// Save in model_state command history.
void CommandProcessor::save_execed_commands(ModelState & model_state)
{
  for (const auto& c : execed_commands)
  {
    model_state.command_history.emplace_back("[" + std::to_string(c.query_id) + "] " + std::to_string(c.execution_time) + ": " + command_tbl[c.type]);
  }
}

#pragma once

#include <cinttypes>

/*
  Control command types.
*/
enum CommandType
{
  ct_nop,                   // No command (empty).
  ct_transmit_command,      // Transmit command.
  ct_receive_command,       // Receive command.
  ct_tr_rephase_command,    // Rephase of transmitter command.
  ct_rs_rephase_command,    // Rephaes of receiver command.
  ct_protect_command,       // Turn off receiver protection command.
  ct_tech_command           // Status request command.
};


/*
  Control command.
*/
struct ControlCommand
{
  // Control command type
  CommandType type;

  // Control message referance time with respect to modelling time, ns
  std::uint64_t referance_time;

  // Time of command execution with respect to modelling time, ns
  std::uint64_t execution_time;

  // Command execution length, ns
  std::uint32_t execution_length;

  // Id of query, that generate this command
  std::uint32_t query_id;

  // Defautl constructor
  ControlCommand() : type(CommandType::ct_nop), referance_time(0), execution_time(0), execution_length(0), query_id(0) {}

  // Defautl constructor
  ControlCommand(const CommandType& _type, const std::uint64_t ref_time, const std::uint64_t exec_time, const std::uint32_t& exec_length, const std::uint32_t& _query_id) :
    type(_type), referance_time(ref_time), execution_time(exec_time), execution_length(exec_length), query_id(_query_id) {}

  // Less operator for sorting
  friend bool operator<(const ControlCommand& lhs, const ControlCommand& rhs)
  {
    return lhs.execution_time > rhs.execution_time;
  }
};
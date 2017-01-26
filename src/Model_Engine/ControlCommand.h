#pragma once

#include <cinttypes>

/*
��� ������ ����������
*/
enum CommandType
{
  ct_nop,                   // ������ �������
  ct_transmit_command,      // ���������� ����������
  ct_receive_command,       // ���������� �������
  ct_tr_rephase_command,    // ����������� ��� �������
  ct_rs_rephase_command,    // ����������� ��� �������
  ct_protect_command,       // ���. / ����. ������
  ct_tech_command           // ������ ������������ ���������
};


/*
������� ����������
*/
struct ControlCommand
{
  // ��� ������� ����������
  CommandType type;

  // ����� ��������� ������� �� ���������� � ������ �������������, ��
  std::uint64_t referance_time;

  // ����� ����������������� ���������� ������� � ������ �������������, ��
  std::uint64_t execution_time;

  // ����� ���������� �������
  std::uint32_t execution_length;

  // ������������� ������, ������� ������������� �������
  std::uint32_t query_id;

  ControlCommand() : type(CommandType::ct_nop), referance_time(0), execution_time(0), execution_length(0), query_id(0) {}

  ControlCommand(const CommandType& _type, const std::uint64_t ref_time, const std::uint64_t exec_time, const std::uint32_t& exec_length, const std::uint32_t& _query_id) :
    type(_type), referance_time(ref_time), execution_time(exec_time), execution_length(exec_length), query_id(_query_id) {}

  friend bool operator<(const ControlCommand& lhs, const ControlCommand& rhs)
  {
    return lhs.execution_time > rhs.execution_time;
  }
};
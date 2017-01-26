#pragma once

#include <cinttypes>

/*
Тип команд управления
*/
enum CommandType
{
  ct_nop,                   // Пустая команда
  ct_transmit_command,      // Управление Излучением
  ct_receive_command,       // Управление приемом
  ct_tr_rephase_command,    // Перестройка фаз антенны
  ct_rs_rephase_command,    // Перестройка фаз антенны
  ct_protect_command,       // Вкл. / Откл. защиты
  ct_tech_command           // Запрос технического состояния
};


/*
Команда управления
*/
struct ControlCommand
{
  // Тип команды управления
  CommandType type;

  // Время пересылки команды на исполнение с начала моделирования, нс
  std::uint64_t referance_time;

  // Время непосредственного выполнения команды с начала моделирования, нс
  std::uint64_t execution_time;

  // Время исполнения команды
  std::uint32_t execution_length;

  // Идентификатор заявки, которая сгенерировала команду
  std::uint32_t query_id;

  ControlCommand() : type(CommandType::ct_nop), referance_time(0), execution_time(0), execution_length(0), query_id(0) {}

  ControlCommand(const CommandType& _type, const std::uint64_t ref_time, const std::uint64_t exec_time, const std::uint32_t& exec_length, const std::uint32_t& _query_id) :
    type(_type), referance_time(ref_time), execution_time(exec_time), execution_length(exec_length), query_id(_query_id) {}

  friend bool operator<(const ControlCommand& lhs, const ControlCommand& rhs)
  {
    return lhs.execution_time > rhs.execution_time;
  }
};
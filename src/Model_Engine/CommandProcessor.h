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
  Класс сохраняющий все запланированные команду управления и аредоставляющий возможность.
  отбирать команды, чье время привязки находится в заданном интервале
*/
class CommandProcessor
{
  // Очередь всех запланированных команд управления. Сортируется по времени привязки
  std::priority_queue<ControlCommand> command_queue;

  // Список исполненных в последнем запуске фукнции run команд
  std::vector<ControlCommand> execed_commands;

  // Файл для вывода истории исполнения команд
  std::ofstream log;

  // Таблица трансляции "тип команды" -> "описание"
  std::map<CommandType, std::string> command_tbl;
public:
  // Конструктор. log_filename - имя файла истории выполнения команд
  CommandProcessor(const std::string& log_filename);

  // Добавить команды из списка command_list. Если тип добавляемой команды nop, то команда игнорируется
  void append(const std::vector<ControlCommand>& command_list);

  // Очистить очередь команд
  void clear();

  // Отбор команд управления, чье время привзяки находится в интервале от time нс до (time + 1000000) нс.
  // Отобранные команды удаляются из списка запланированных команд
  void run(const std::uint64_t time);

  // Проверка очереди запланированных команд на соответствие требованиям протокола обмена (проект "Моренос РЛК")
  bool validate();

  // Запись в model_state состояния command_queue и execed_commands
  void get_statistics(ModelState& model_state);

  // Запись в model_state всех исполненных команд в текстовом представлении
  void save_execed_commands(ModelState& model_state);
};

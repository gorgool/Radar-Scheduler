#pragma once

#include <array>
#include <functional>
#include <vector>
#include <cassert>
#include <iostream>
#include "Utility.h"
#include "ModelException.h"

// Метки дискретов КВД
enum TimelineLabel : std::uint8_t
{
  tll_empty,                // Свободно
  tll_dcu_receive,          // Прием на УЦП
  tll_dcu_transmit,         // Излучение на УЦП
  tll_au_rephase,           // Перефазировка на АУ
  tll_au_command_delay,     // Задержка исполнения команд на АУ
  tll_au_channel_switch,    // Переключение приемного/передающего канала на АУ
  tll_dcu_energy_restore,   // Накопление энергии после излучения на УЦП
  tll_dcu_receive_prevent   // Запрет промежуточного приема при приеме пачки в УЦП
};

/*
  Обобщенный конвейер временных дискретов
*/

class Timeline
{
  using pipiline_type = std::array<TimelineLabel, settings::timeline_depth>;

public:

  // Время привязки первого дискрета КВД (постоянно возрастает), нс
  std::uint64_t start_time;

  // Индекс первого дискрета КВД
  std::uint32_t start_idx;

  // Массив временных дискретов КВД УЦП
  pipiline_type timeline;

  // Проверка возможности размещения участка длительностью length по индексу idx согласно критерию pred.
  // Возвращает false в случае неудачи и true в случае успеха
  bool check_index(const std::uint32_t length, const std::uint32_t idx, std::function<bool(const pipiline_type::value_type&)> pred);

  // Разметить участок на КВД начиная с инндекса start_idx до stop_idx меткой label
  void label_sector(const std::uint32_t start_idx, const std::uint32_t stop_idx, const TimelineLabel label);

  // Индексация массива временых дискретов. Возвращает ссылку на значение дискрета по индексу idx.
  // Пример: индекс 0 возвращает самый ранний по времени временной дискрет.
  const TimelineLabel& get_value_at(const std::size_t idx) const;
  TimelineLabel& get_value_at(const std::size_t idx);

  // Возвращает индекс дискрета для времени time (нс) относительно начала КВД, т.е время 0 вернет индекс равный 0.
  // Возвращает -1 в случае если такого индекса не существует и положительное целое, равное индексу, в случае успеха.
  std::int32_t get_idx_for(const std::uint32_t time);

  // Сместить конвейер вперед до времени time и подготовить новый участок.
  // Примечание 
  void move_timeline(std::uint64_t time);

  // Расчет коэффициента занятости КВД, как отношение свободных дискретов (tll_empty) ко всем дискретам.
  double occupation();

  // Сброс всех параметров КВД
  void reset();

  Timeline() : start_time(0), start_idx(0) { timeline.fill(TimelineLabel::tll_empty); }

  ~Timeline() {};
};

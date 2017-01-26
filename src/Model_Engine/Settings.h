#pragma once
#include <cinttypes>

/*
  Исходные параметры и константы
*/

namespace settings
{
  // Задержка изменения фаз антенны (изменение направление зонда), нс
  static const std::uint32_t phase_delay = 270000;

  // Задержка выполнения команд на АУ, нс
  static const std::uint32_t au_command_delay = 500000;

  // Бланк импульсов (допуски в начале и конце импульсов), нс
  static const std::uint32_t blank_delay = 3100;

  // Время переключения служебных частот, нс
  static const std::uint32_t frequency_delay = 3100;

  // Время переключения каналов, в дискретах, нс
  static const std::uint32_t channel_switch_delay = 3000;

  // Коэффициент скважности
  static const std::uint32_t duty_factor = 8;

  // Коэффициент кратности времени привязки, нс
  static const std::uint32_t frequency_factor = 100;

  // Время задержки передачи. Время задает запас по временир на выполнение сопряженных команд (перефазировка + излучение и т.п)
  static const std::uint32_t reserve_time = 5000;

  // Коэффициент роста поиcковой заявки (с расчетом темпа обзора 50 Гц)
  static const double search_query_speed = 50.0;

  // Порог выполнения поиcковой заявки
  static const double search_query_threshold = 1.0;
  
  // Коэффициент роста заявки на сопровождение
  static const double tracking_query_speed = 10.0;

  // Коэффициент роста заявки на дообнаружение
  static const double confirm_query_speed = 100.0;

  // Коэффициент роста заявки на захват
  static const double capture_query_speed = 100.0;

  // Коэффициент роста заявки на запрос технического состояния
  static const double tech_control_speed = 1.0;

  // Порог выполнения заявки на сопровождение
  static const double tracking_query_threshold = 1.0;

  // Порог выполнения заявки на дообнаружение
  static const double confirm_query_threshold = 0.0;

  // Порог выполнения заявки на захват
  static const double capture_query_threshold = 0.0;

  // Порог выполнения заявки на запрос технического состояния
  static const double tech_control_threshold = 1.0;

  // Длительность временного дискрета, нс
  static const std::uint32_t time_chunk_length = 10000;

  // Интервал планирования, в дискретах (1 мс)
  static const std::uint32_t planning_step = 100;

  // Глубина конвейера, в дискретах (1 сек)
  static const std::uint32_t timeline_depth = 100000;

  // Глубина планирования одиночного импульса, в интервалах планирования
  static const std::uint32_t pulse_planning_depth = 3;

  // Глубина планирования пачки, в интервалах планирования
  static const std::uint32_t pulse_train_planning_depth = 35;
}

#pragma once

#include "ScanPair.h"
#include "Utility.h"
#include "Query.h"
#include <map>
#include <inttypes.h>
#include <cassert>
#include <algorithm>
#define _USE_MATH_DEFINES
#include <math.h>

/*
  Выбор типа зондирующего сигнала на основании дальности и ожидаемой ЭПР ОК.
  Примечание: в текущей версии сигналы типа пачки генерируются без наложения приемного и передающего участка
  range - прогнозируемая дальность до ОК, м.
  rcs - прогнозируемая эффективная поверхность рассеивания, кв. м.
  type - тип заявки
*/

ScanPair build_scan_pair(const double range, const double rcs, const QueryType type);

/*
  Коды типов сигнала
*/
enum SignalType
{
  // Сигналы обнаружения и сопровождения
  SOTS1, SOTS2, SOTS3, SOTS4, SOTS5,
  SOTS6, SOTS7, SOTS8, SOTS9, SOTS10,
  SOTS11, SOTS12, SOTS13, SOTS14, SOTS15,
  // Спецсигналы
  SAFM1, SAFM2, SAFM3, SAFM4, SSS,
  SDP1, SDP2, SDP3, SDP4, SDSPS
};

/*
  Параметры сигнала
*/
struct SignalParams
{
  // Длительность сигнала, наносекунды
  std::uint32_t duration;

  // Девиация ЛЧМ, кГц
  std::uint32_t deviation;

  // Нижняя граница периода повторения импульсов в пачке, нс
  std::uint32_t period_low;

  // Верхняя граница периода повторения импульсов в пачке, нс
  std::uint32_t period_high;

  // Нижняя граница количества импульсов в пачке
  std::uint16_t pulses_number_low;

  // Верхняя граница количества импульсов в пачке
  std::uint16_t pulse_number_high;

  // Максимальное время оцифровки, нс
  std::uint32_t max_process_time;
};
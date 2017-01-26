#pragma once

#include <cinttypes>
#include <cstddef>
#include <iostream>
#include <cassert>
#include <exception>
#include "Settings.h"
#include "Utility.h"
#include "ModelException.h"

/*
  Временная связка
*/

class ScanPair
{
public:

  ScanPair(const std::size_t _size = 1) : receive_time(0)
  {
    // Примечание: рассматриваются только пачки от 1 до 16 импульсов
    
    // Округление количества импульсов до ближайшего числа (степени двойки)
    std::size_t temp = 0;
    while (static_cast<std::size_t>(std::pow(2, temp)) < _size)
    {
      temp++;
    }

    size = static_cast<std::size_t>(std::pow(2, temp));
    receive = new receive_desc[size];
    transmit = new transmit_desc[size];
  }

  ScanPair(const ScanPair& obj) : size(obj.size), receive_time(obj.receive_time)
  {
    receive = new receive_desc[size];
    transmit = new transmit_desc[size];

    for (std::size_t idx = 0; idx < size; ++idx)
    {
      transmit[idx] = obj.transmit[idx];
      receive[idx] = obj.receive[idx];
    }    
  }

  ~ScanPair()
  {
    delete[] receive;
    delete[] transmit;
  }

  // Задание время начала приема.
  // range - прогнозируемая дальность до ОК, м.
  void set_receive_time(const double range)
  {
    assert(transmit[0].length != 0);

    // Задержка времени по дальности
    double range_delay_time = 2.0 * range / 299792458.0;
    std::uint32_t rdt_nanosec = static_cast<std::uint32_t>(std::ceil(range_delay_time * 1e9));
    
    // Задержка времени излучения для пачек
    if (size > 1)
    {
      std::uint32_t transmit_time_nanosec = 0;
      for (std::size_t idx = 0; idx < size; ++idx)
      {
        transmit_time_nanosec += transmit[idx].length * settings::duty_factor;
      }

      if (transmit_time_nanosec > rdt_nanosec)
      {
        throw ModelException("ERROR: Pulse train transmit time is larger than range delay time offset.");
        return;
      }
    }

    receive_time = multiples_of(rdt_nanosec);
    return;
  }

  // Формирование приемной диаграммы направленности (ДН).
  // Значения по умолчанию соответствуют одиночному импульсу.
  // length - Длительность приема одного импульса в пачке (или единичного импульса), нс
  // offset_time - Период повторения приемных участков в пачке (0 для одиночного импульса), нс
  void set_receive(std::uint32_t length, std::uint32_t offset_time = 0)
  {
    for (std::size_t idx = 0; idx < size; ++idx)
    {
      if (idx == 0)
        receive[idx].offset_time = 0;
      else
        receive[idx].offset_time = offset_time;
      receive[idx].length = length;
    }
   
  }

  // Формирование пачки передающих ДН.
  // Значения по умолчанию соответствуют одиночному импульсу.
  // length - Длительность излучения одного импульса в пачке (или единичного импульса), нс
  // offset_time - Период повторения излучения в пачке (0 для одиночного импульса), нс
  void set_transmit(std::uint32_t length, std::uint32_t offset_time = 0)
  {
    for (std::size_t idx = 0; idx < size; ++idx)
    {
      if (idx == 0)
        transmit[idx].offset_time = 0;
      else
        transmit[idx].offset_time = offset_time;
      transmit[idx].length = length;
    }
  }

  // Количесво импульсов в пачке
  std::uint32_t size;

  // Масссив описания приемных интервалов
  struct receive_desc
  {
    // Смещение приема текущего участка, относительно предыдущего, нс
    std::uint32_t offset_time = 0;
    // Длительность приема, нс
    std::uint32_t length = 0;
  } *receive;

  // Массив описания импульсов излучения в пачке
  struct transmit_desc
  {
    // Смещение текущего импульса излучения, относительно предыдущего, нс
    std::uint32_t offset_time = 0;
    // Длительность излучения, нс
    std::uint32_t length = 0;
  } *transmit;
 
  // Время начала приема, относительно начала излучения, нс
  std::uint32_t receive_time;
};

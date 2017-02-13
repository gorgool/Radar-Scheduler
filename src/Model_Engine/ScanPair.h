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
  Pair of signals for scanning one target (group of targets).
*/

class ScanPair
{
public:

  // Contructor.
  ScanPair(const std::size_t _size = 1) : receive_time(0)
  { 
    // Round pulse train size up to the power of 2.
    std::size_t temp = 0;
    while (static_cast<std::size_t>(std::pow(2, temp)) < _size)
    {
      temp++;
    }

    size = static_cast<std::size_t>(std::pow(2, temp));
    receive = new pulse_train_desc[size];
    transmit = new pulse_train_desc[size];
  }

  // Copy constructor.
  ScanPair(const ScanPair& obj) : size(obj.size), receive_time(obj.receive_time)
  {
    receive = new pulse_train_desc[size];
    transmit = new pulse_train_desc[size];

    for (std::size_t idx = 0; idx < size; ++idx)
    {
      transmit[idx] = obj.transmit[idx];
      receive[idx] = obj.receive[idx];
    }    
  }

  // Destructor.
  ~ScanPair()
  {
    delete[] receive;
    delete[] transmit;
  }

  // Cumpute and setup receivng time, depending of range.
  // range - estimate range to target, m.
  void set_receive_time(const double range)
  {
    assert(transmit[0].length != 0);

    // Range time delay
    double range_delay_time = 2.0 * range / 299792458.0;
    std::uint32_t rdt_nanosec = static_cast<std::uint32_t>(std::ceil(range_delay_time * 1e9));
    
    // Pulse train delay time
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
    else
    {
      if (transmit[0].length > rdt_nanosec)
      {
        throw ModelException("ERROR: Single pulse transmit time is larger than range delay time offset.");
        return;
      }
    }

    receive_time = multiples_of(rdt_nanosec);
    return;
  }

  // Setup receive time.
  // length - length of single pulse receive (or the only pulse, if size equal 1), ns
  // offset_time - pulse train period (0 for single pulse), ns
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

  // Setup transmit time.
  // length - length of single pulse transmit (or the only pulse, if size equal 1), ns
  // offset_time - pulse train period (0 for single pulse), ns
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

  // Pulse train size
  std::uint32_t size;

  // Pulse train description
  struct pulse_train_desc
  {
    // Offset time for current pulse with respect to the previous pulse, ns
    std::uint32_t offset_time = 0;
    // Длительность приема, нс
    std::uint32_t length = 0;
  };
  pulse_train_desc *receive;
  pulse_train_desc *transmit;
 
  // Receive time offset with respect to the transmit time, ns
  std::uint32_t receive_time;
};

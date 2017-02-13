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
  Scan pair creation function depending of range and RCS
  range - range estimation, m.
  rcs - radar cross-section, sq. m.
  type - query type
*/

ScanPair build_scan_pair(const double range, const double rcs, const QueryType type);

/*
  Codes of signal types
*/
enum SignalType
{
  // Search and tracking
  SOTS1, SOTS2, SOTS3, SOTS4, SOTS5,
  SOTS6, SOTS7, SOTS8, SOTS9, SOTS10,
  SOTS11, SOTS12, SOTS13, SOTS14, SOTS15,
  // Special
  SAFM1, SAFM2, SAFM3, SAFM4, SSS,
  SDP1, SDP2, SDP3, SDP4, SDSPS
};

/*
  Signal parameters
*/
struct SignalParams
{
  // Length, nanoseconds (ns)
  std::uint32_t duration;

  // LFM deviation, kGh
  std::uint32_t deviation;

  // Lower bound of pulse period in pulse train, ns
  std::uint32_t period_low;

  // Upper bound of pulse period in pulse train, ns
  std::uint32_t period_high;

  // Lower bound of pulse number in pulse train
  std::uint16_t pulses_number_low;

  // Upper bound of pulse number in pulse train
  std::uint16_t pulse_number_high;

  // Maximum of range processing time, ns
  std::uint32_t max_process_time;
};
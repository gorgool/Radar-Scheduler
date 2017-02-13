#pragma once

#include <cinttypes>
#include <vector>
#include <memory>
#include "Timeline.h"
#include "Query.h"
#include "ModelState.h"
#include "BuildScanPair.h"

/*
  Scheduler class.
  Parameters of timelines and scheduler are specified in Setting.h
*/

class Scheduler
{
  // DCU (Digital Converter Unit) Timeline
  Timeline dcu_timeline;

  // Transmitter AU (Antenna Unit) Timeline
  Timeline tr_au_timeline;

  // Receiver AU (Antenna Unit) Timeline
  Timeline rs_au_timeline;  

  // Return index for DCU Timeline which allow to allocate region equal to transmit_len (ns) 
  // and coupled to it another region equal to receive_len (ns) with offset range_offset (ns).
  // Returns -1 if there is not such index or positive integer in case of success.
  std::int32_t find_index(const std::uint32_t transmit_len, const std::uint32_t receive_len, const std::uint32_t range_offset, const std::uint32_t depth);

  // Allocate scan_pair in DCU and AU (both) Timelines for execution query_id.
  // If succeeded return commands list, otherwise empty list of "nop"" commands.
  std::vector<ControlCommand> place_scan(const ScanPair& scan_pair, const std::uint32_t query_id);

  // Remove query whose id equal to query_id.
  void remove_query(const std::uint32_t query_id);

public:
  Scheduler() {}

  using QueryListType = std::vector<std::shared_ptr<Query>>;

  // Active query list
  QueryListType queries;

  // The last executed queries (during run function call).
  QueryListType processed_queries;

  // Execute one planning step for time moment equal time
  std::vector<ControlCommand> run(std::uint64_t time);

  // Fill model_state with active and executed queries parameters.
  void get_statistics(ModelState& model_state);

  // Save timelines state.
  void save_timilines(Timeline& _dcu_timeline, Timeline& _au_tr_timeline, Timeline& _au_rs_timeline);

  // Clear all queries lists and reset timelines.
  void reset();
};

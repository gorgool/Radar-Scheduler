#include "Scheduler.h"

// Return index for DCU Timeline which allow to allocate region equal to transmit_len (ns) 
// and coupled to it another region equal to receive_len (ns) with offset range_offset (ns).
// Returns -1 if there is not such index or positive integer in case of success.
std::int32_t Scheduler::find_index(const std::uint32_t transmit_len, const std::uint32_t receive_len, const std::uint32_t range_offset, const std::uint32_t depth)
{
  // Tramsmit length in timeline chunks
  std::uint32_t tr_length = static_cast<std::uint32_t>(std::ceil(static_cast<double>(transmit_len) / settings::time_chunk_length));

  if (tr_length >= depth)
  {
    throw ModelException("ERROR: Transmit command length is larger than planning depth in find_index.");
  }

  // Receive length in timeline chunks
  std::uint32_t rs_length = static_cast<std::uint32_t>(std::ceil(static_cast<double>(receive_len) / settings::time_chunk_length));;

  // Rephase length in timeline chunks
  std::uint32_t rph_length = static_cast<std::uint32_t>(std::ceil(static_cast<double>(settings::phase_delay) / settings::time_chunk_length));;

  std::uint32_t dcu_sum = 0;
  std::uint32_t au_sum = 0;
  
  std::vector<std::uint16_t> au_indices(depth);

  // Compute prefix sum such that if pred(idx) == true than sum += 1 otherwise sum = 0
  for (std::uint32_t idx = 0; idx < depth; ++idx)
  {
    // DCU prefix sum
    if (dcu_timeline.get_value_at(idx) == TimelineLabel::tll_empty)
      dcu_sum++;
    else
      dcu_sum = 0;
    // AU prefix sum
    if (tr_au_timeline.get_value_at(idx) == TimelineLabel::tll_empty)
      au_sum++;
    else
      au_sum = 0;

    // DCU Allocate region threshold (1 - allocation possible)
    auto dcu_value = dcu_sum > tr_length ? 1 : 0;

    // AU Allocate region threshold (1 - allocation possible)
    auto au_value = au_sum > rph_length ? 1 : 0;

    au_indices[idx] = au_value;

    if (dcu_value == 1)
    {
      auto tr_idx = idx - tr_length;
      // Convolution of DCU and AU allocate regions
      if (dcu_value * au_indices[tr_idx] == 1)
      {
        // Was abled to allocate coupled transmit and rephase (for the transmit) regions.
        // Check if we can allocate receive and rephase (for the receive) regions.
        std::uint32_t rs_start_time = tr_idx * settings::time_chunk_length + range_offset;
        std::uint32_t rs_start_idx = dcu_timeline.get_idx_for(rs_start_time);
        std::uint32_t rph_start_time = rs_start_time - settings::phase_delay;
        std::uint32_t rph_start_idx = rs_au_timeline.get_idx_for(rph_start_time);

        if (
          dcu_timeline.check_index(rs_length, rs_start_idx,
            [](const auto& val) -> bool { return (val == TimelineLabel::tll_empty || val == TimelineLabel::tll_dcu_energy_restore); }) == true &&
          rs_au_timeline.check_index(rph_length, rph_start_idx,
            [](const auto& val) -> bool { return val == TimelineLabel::tll_empty; }) == true)
        {
          // If allocation has been succeeded return index.
          return idx - tr_length;
        }
      }

    }
  }
  return -1;
}

// Allocate scan_pair in DCU and AU (both) Timelines for execution query_id.
// If succeeded return commands list, otherwise empty list of "nop"" commands.
std::vector<ControlCommand> Scheduler::place_scan(const ScanPair & scan_pair, const std::uint32_t query_id)
{
  // Timeline planning depth
  std::uint32_t depth;
  // Transmit command execution length, ns
  std::uint32_t transmit_len;
  // Receive command execution length, ns
  std::uint32_t receive_len;
  // Time offset between transmit and receive commands due to target range, ns
  std::uint32_t range_offset_len = multiples_of(static_cast<std::uint32_t>(scan_pair.receive_time)
    - settings::phase_delay - settings::reserve_time, settings::frequency_factor);

  // Pulse train period
  std::uint32_t period_time = 0;

  // Single pulse
  if (scan_pair.size == 1)
  {
    transmit_len = multiples_of(static_cast<std::uint32_t>(scan_pair.transmit[0].length) * settings::duty_factor
      + 2 * settings::frequency_delay
      + settings::blank_delay
      + settings::reserve_time, settings::frequency_factor);
    receive_len = multiples_of(static_cast<std::uint32_t>(scan_pair.receive[0].length + settings::reserve_time), settings::frequency_factor);
    depth = settings::planning_depth * settings::planning_step + transmit_len / settings::time_chunk_length;
  }
  // Pulse train
  else
  {
    period_time = static_cast<std::uint32_t>(scan_pair.transmit[1].offset_time);
    transmit_len = multiples_of(period_time * scan_pair.size + settings::blank_delay 
      + 2 * settings::frequency_delay + settings::reserve_time, settings::frequency_factor);
    receive_len = multiples_of(period_time * scan_pair.size + settings::reserve_time, settings::frequency_factor);
    depth = settings::planning_depth * settings::planning_step + transmit_len / settings::time_chunk_length;
  }

  // Find index for allocating the scan pair
  auto idx = find_index(transmit_len, receive_len, range_offset_len, depth);
  if (idx != -1)
  {
    // If found

    // Labeling transmitter AU Timeline
    // Transmitter rephase start time offset (from the begining of timeline, here and hereinafter), ns
    std::uint32_t tr_rephase_start_rel = idx * settings::time_chunk_length - settings::phase_delay;
    // Transmitter rephase stop time offset, ns
    std::uint32_t tr_rephase_stop_rel = tr_rephase_start_rel + settings::phase_delay;

    tr_au_timeline.label_sector(
      tr_au_timeline.get_idx_for(tr_rephase_start_rel),
      tr_au_timeline.get_idx_for(tr_rephase_stop_rel),
      TimelineLabel::tll_au_rephase);

    // Transmitter AU command execution ban start time offset, ns
    std::uint32_t tr_nocommand_start_rel = tr_rephase_stop_rel;
    // Transmitter AU command execution ban stop time offset, ns
    std::uint32_t tr_nocommand_stop_rel = tr_nocommand_start_rel + (settings::au_command_delay - settings::phase_delay);

    tr_au_timeline.label_sector(
      tr_au_timeline.get_idx_for(tr_nocommand_start_rel),
      tr_au_timeline.get_idx_for(tr_nocommand_stop_rel),
      TimelineLabel::tll_au_command_delay);

    // Labeling DCU Timeline (transmit)
    dcu_timeline.label_sector(
      dcu_timeline.get_idx_for(tr_rephase_stop_rel),
      dcu_timeline.get_idx_for(tr_rephase_stop_rel + settings::blank_delay + settings::frequency_delay + settings::reserve_time),
      TimelineLabel::tll_dcu_transmit);

    for (std::size_t pulse_idx = 0; pulse_idx < scan_pair.size; ++pulse_idx)
    {
      // i-th pulse transmit start time offset, ns
      std::uint32_t transmit_start_rel = tr_rephase_stop_rel + settings::blank_delay + settings::reserve_time + settings::frequency_delay + pulse_idx * period_time;
      // i-th pulse transmit stop time offset, ns
      std::uint32_t transmit_stop_rel = transmit_start_rel + scan_pair.transmit[pulse_idx].length + settings::blank_delay;

      dcu_timeline.label_sector(
        dcu_timeline.get_idx_for(transmit_start_rel),
        dcu_timeline.get_idx_for(transmit_stop_rel),
        TimelineLabel::tll_dcu_transmit);

      // i-th pulse transmit execution ban start time offset, ns
      std::uint32_t restore_start_rel = transmit_stop_rel;
      // i-th pulse transmit execution ban stop time offset, ns
      std::uint32_t restore_stop_rel = restore_start_rel + (settings::duty_factor - 1) * scan_pair.transmit[pulse_idx].length;

      dcu_timeline.label_sector(
        dcu_timeline.get_idx_for(restore_start_rel),
        dcu_timeline.get_idx_for(restore_stop_rel),
        TimelineLabel::tll_dcu_energy_restore);
    }

    // Labeling receiver AU Timeline
    // Receiver rephase start time offset, ns
    std::uint32_t rs_rephase_start_rel = tr_rephase_stop_rel + range_offset_len;
    // Receiver rephase stop time offset, ns
    std::uint32_t rs_rephase_stop_rel = rs_rephase_start_rel + settings::phase_delay;

    rs_au_timeline.label_sector(
      rs_au_timeline.get_idx_for(rs_rephase_start_rel),
      rs_au_timeline.get_idx_for(rs_rephase_stop_rel),
      TimelineLabel::tll_au_rephase);

    // Receiver AU command execution ban start time offset, ns
    std::uint32_t rs_nocommand_start_rel = rs_rephase_stop_rel;
    // Receiver AU command execution ban stop time offset, ns
    std::uint32_t rs_nocommand_stop_rel = rs_nocommand_start_rel + (settings::au_command_delay - settings::phase_delay);

    rs_au_timeline.label_sector(
      rs_au_timeline.get_idx_for(rs_nocommand_start_rel),
      rs_au_timeline.get_idx_for(rs_nocommand_stop_rel),
      TimelineLabel::tll_au_command_delay);

    // Labeling DCU Timeline (Receive)
    dcu_timeline.label_sector(
      dcu_timeline.get_idx_for(rs_rephase_stop_rel),
      dcu_timeline.get_idx_for(rs_rephase_stop_rel + settings::reserve_time),
      TimelineLabel::tll_dcu_receive);

    // Labeling pulse train receive (pulses and period between pulses)
    if (scan_pair.size > 1)
    {
      dcu_timeline.label_sector(
        dcu_timeline.get_idx_for(rs_rephase_stop_rel + settings::reserve_time),
        dcu_timeline.get_idx_for(rs_rephase_stop_rel + period_time * (scan_pair.size - 1)),
        TimelineLabel::tll_dcu_receive_prevent);
    }

    // Labeling pulses
    for (std::size_t pulse_idx = 0; pulse_idx < scan_pair.size; ++pulse_idx)
    {
      // i-th pulse receive start time offset, ns
      std::uint32_t receive_start_rel = rs_rephase_stop_rel + settings::reserve_time + pulse_idx * period_time;
      // i-th pulse receive stop time offset, ns
      std::uint32_t receive_stop_rel = receive_start_rel + scan_pair.receive[pulse_idx].length;

      dcu_timeline.label_sector(
        dcu_timeline.get_idx_for(receive_start_rel),
        dcu_timeline.get_idx_for(receive_stop_rel),
        TimelineLabel::tll_dcu_receive);
    }

    // Generate control commands

    // Transmitter rephase command start time (absolute time), ns
    std::uint64_t transmit_rephase_time = static_cast<std::uint64_t>(dcu_timeline.start_time + idx * settings::time_chunk_length - settings::phase_delay);
    // Transmit command start time (absolute time), ns
    std::uint64_t transmit_exec_time = static_cast<std::uint64_t>(transmit_rephase_time + settings::phase_delay);
    // Receiver rephase command start time (absolute time), ns
    std::uint64_t receive_rephase_time = static_cast<std::uint64_t>(transmit_exec_time + range_offset_len + settings::reserve_time);
    // Receive command start time (absolute time), ns
    std::uint64_t receive_exec_time = static_cast<std::uint64_t>(receive_rephase_time + settings::phase_delay);
    // Receiver protection turn off command start time (absolute), ns
    std::uint64_t antenna_ptotection_time = static_cast<std::uint64_t>(receive_exec_time - settings::channel_switch_delay);

    // Check range. For debug.
    auto range_offset = receive_exec_time - transmit_exec_time;
    if (range_offset != multiples_of(static_cast<std::uint64_t>(scan_pair.receive_time)))
    {
      throw ModelException("ERROR: range offset is not correct in place_scan.");
    }

    return{
      ControlCommand(CommandType::ct_transmit_command, transmit_exec_time - settings::reserve_time - settings::blank_delay - settings::frequency_delay, transmit_exec_time, transmit_len, query_id),
      ControlCommand(CommandType::ct_tr_rephase_command, transmit_rephase_time, transmit_rephase_time, settings::phase_delay, query_id),
      ControlCommand(CommandType::ct_protect_command, antenna_ptotection_time, antenna_ptotection_time, settings::channel_switch_delay, query_id),
      ControlCommand(CommandType::ct_rs_rephase_command, receive_rephase_time, receive_rephase_time, settings::phase_delay, query_id),
      ControlCommand(CommandType::ct_receive_command, receive_exec_time - settings::reserve_time, receive_exec_time, receive_len, query_id)
    };
  }
  else
  {
    return{ ControlCommand() };
  }
}

// Remove query whose id equal to query_id.
void Scheduler::remove_query(const std::uint32_t query_id)
{
  queries.erase(std::remove_if(queries.begin(), queries.end(), [query_id](const auto& val) { return val->id == query_id; }), queries.end());
}

// Execute one planning step for time moment equal time
std::vector<ControlCommand> Scheduler::run(std::uint64_t time)
{
  // Move timelines to adjust to current time.
  dcu_timeline.move_timeline(time);
  tr_au_timeline.move_timeline(time);
  rs_au_timeline.move_timeline(time);

  std::vector<ControlCommand> ret;

  // List of queries to be deleted.
  std::vector<std::uint32_t> removed_id;

  // List of queries to be added.
  // Query can be added after confirmation or capture query (aims processing imitation).
  std::vector<std::shared_ptr<Query>> new_queries;

  // List that contain one element cant be sorted, so special case for one element list
  if (queries.size() == 1)
  {
    queries.front()->p_value = queries.front()->k * (time - queries.front()->t_prev) * 1.0e-9;
  }
  else
  {
    // Compute and sort queries by thier priority in descending order
    std::sort(queries.begin(), queries.end(), [&](auto& lhs, auto& rhs)
    {
      rhs->p_value = rhs->k * (time - rhs->t_prev) * 1.0e-9;
      lhs->p_value = lhs->k * (time - lhs->t_prev) * 1.0e-9;
      return rhs->p_value < lhs->p_value;
    });
  }

  processed_queries.clear();

  for (auto& q : queries)
  {
    if (q->active == true && q->p_value >= q->p_threshold)
    {
      switch (q->type)
      {
      case QueryType::search:
      {
        // Search query
        auto scan_pair = build_scan_pair(q->range, q->rcs, QueryType::search);

        // Try to allocate scan pair
        auto commands = place_scan(scan_pair, q->id);

        // If commands list not empty (have elements with no "nop" commands)
        for (const auto& command : commands)
        {
          if (command.type != CommandType::ct_nop)
          {
            // Allocation of scan pair for the query is succeeded. Save current time as last execution time for the query. 
            q->t_prev = time;
            processed_queries.push_back(q);
            break;
          }
        }

        // Add commands
        ret.insert(ret.end(), commands.begin(), commands.end());

        break;
      }
      case QueryType::confirm:
      {
        // Confirmation
        auto scan_pair = build_scan_pair(q->range, q->rcs, QueryType::confirm);

        // Try to allocate scan pair
        auto commands = place_scan(scan_pair, q->id);

        // If commands list not empty (have elements with no "nop" commands)
        for (const auto& command : commands)
        {
          if (command.type != CommandType::ct_nop)
          {
            q->t_prev = time;
            processed_queries.push_back(q);

            // Single execution time query, delete it
            removed_id.push_back(q->id);

            // Add capture query
            new_queries.push_back(std::make_shared<Query>(QueryType::capture, q->id + 1, settings::capture_query_speed, settings::capture_query_threshold, time, q->range, q->rcs, true));
            break;
          }
        }

        // Add commands
        ret.insert(ret.end(), commands.begin(), commands.end());

        break;
      }
      case QueryType::capture:
      {
        // Capture
        auto scan_pair = build_scan_pair(q->range, q->rcs, QueryType::capture);

        // Try to allocate scan pair
        auto commands = place_scan(scan_pair, q->id);

        // If commands list not empty (have elements with no "nop" commands)
        for (const auto& command : commands)
        {
          if (command.type != CommandType::ct_nop)
          {
            q->t_prev = time;
            processed_queries.push_back(q);

            // Single execution time query, delete it
            removed_id.push_back(q->id);

            // Add tracking query
            new_queries.push_back(std::make_shared<Query>(QueryType::tracking, q->id + 1, settings::tracking_query_speed, settings::tracking_query_threshold, time, q->range, q->rcs, true));
            break;
          }
        }

        // Add commands
        ret.insert(ret.end(), commands.begin(), commands.end());
        
        break;
      }
      case QueryType::tracking:
      {
        // Tracking
        auto scan_pair = build_scan_pair(q->range, q->rcs, QueryType::tracking);

        // Try to allocate scan pair
        auto commands = place_scan(scan_pair, q->id);

        // If commands list not empty (have elements with no "nop" commands)
        for (const auto& command : commands)
        {
          if (command.type != CommandType::ct_nop)
          {
            // Allocation of scan pair for the query is succeeded. Save current time as last execution time for the query. 
            q->t_prev = time;
            processed_queries.push_back(q);
            break;
          }
        }

        // Add commands
        ret.insert(ret.end(), commands.begin(), commands.end());

        break;
      }
      case QueryType::drop:
      {
        // Drop query
        removed_id.push_back(q->id);
        processed_queries.push_back(q);
        break;
      }
      case QueryType::tech_control:
      {
        // Devise state request
        auto command = ControlCommand(CommandType::ct_tech_command, time - settings::reserve_time, static_cast<std::uint64_t>(time), 0, q->id);

        // Update last execution time
        q->t_prev = time;

        processed_queries.push_back(q);

        // Add command
        ret.push_back(command);

        break;
      }
      default:
        break;
      }
    }
  }

  // Delete queries
  for (const auto& id : removed_id)
    remove_query(id);

  // Add new queries
  for (const auto& q : new_queries)
    queries.push_back(q);

  return ret;
}

// Fill model_state with active and executed queries parameters.
void Scheduler::get_statistics(ModelState & model_state)
{
  for (const auto& q : queries)
  {
    model_state.active_queries.push_back(*q);
  }

  for (const auto& q : processed_queries)
  {
    model_state.processed_queries.push_back(*q);
  }
}

// Save timelines state.
void Scheduler::save_timilines(Timeline &_dcu_timeline, Timeline &_au_tr_timeline, Timeline &_au_rs_timeline)
{
    _dcu_timeline = dcu_timeline;
    _au_tr_timeline = tr_au_timeline;
    _au_rs_timeline = rs_au_timeline;
}

// Clear all queries lists and reset timelines.
void Scheduler::reset()
{
    queries.clear();
    dcu_timeline.reset();
    tr_au_timeline.reset();
    rs_au_timeline.reset();
}

#include "Scheduler.h"


// ����� ������ �� ���, ���������� ��� ���������� ������� transmit_len (��) � ���������������� ��� receive_len (��) 
// �� ��������� range_offset(��), �� ������� ������������ depth.
// ���������� -1 � ������ ���� ������ ������� �� ���������� � ������������� �����, ������ �������, � ������ ������.

std::int32_t Scheduler::find_index(const std::uint32_t transmit_len, const std::uint32_t receive_len, const std::uint32_t range_offset, const std::uint32_t depth)
{
  // ����� ������� ���������, � ���������
  std::uint32_t tr_length = static_cast<std::uint32_t>(std::ceil(static_cast<double>(transmit_len) / settings::time_chunk_length));

  if (tr_length >= depth)
  {
    throw ModelException("ERROR: Transmit command length is larger than planning depth in find_index.");
  }

  // ����� ������� ������, � ���������
  std::uint32_t rs_length = static_cast<std::uint32_t>(std::ceil(static_cast<double>(receive_len) / settings::time_chunk_length));;

  // ����� ������� ������������� � ���������
  std::uint32_t rph_length = static_cast<std::uint32_t>(std::ceil(static_cast<double>(settings::phase_delay) / settings::time_chunk_length));;

  std::uint32_t dcu_sum = 0;
  std::uint32_t au_sum = 0;
  
  std::vector<std::uint16_t> au_indices(depth);

  // ������ ���������� ����� � ��������� ����������
  for (std::uint32_t idx = 0; idx < depth; ++idx)
  {
    // ���������� ����� ���
    if (dcu_timeline.get_value_at(idx) == TimelineLabel::tll_empty)
      dcu_sum++;
    else
      dcu_sum = 0;
    // ���������� ����� ��
    if (tr_au_timeline.get_value_at(idx) == TimelineLabel::tll_empty)
      au_sum++;
    else
      au_sum = 0;

    // ���������� ����� ���
    auto dcu_value = dcu_sum > tr_length ? 1 : 0;

    // ���������� ����� ��
    auto au_value = au_sum > rph_length ? 1 : 0;
    au_indices[idx] = au_value;

    if (dcu_value == 1)
    {
      auto tr_idx = idx - tr_length;

      // ���������
      if (dcu_value * au_indices[tr_idx] == 1)
      {
        // ���������� ���������� � ��������� � �������������
        // ��������� ����������� ���������� ������ � �������������
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
          // ���������� ������. ���������� ���������� ������.
          return idx - tr_length;
        }
      }

    }
  }
  return -1;
}

// ���������� ��������� ������ scan_pair �� ��� � ��������� ������������ ������ query_id.
// � ������ ������ ���������� ������ ������ ���������� ��� � ��. � ������ ������� ���������� �������� ���� nop (no operation).


std::vector<ControlCommand> Scheduler::place_scan(const ScanPair & scan_pair, const std::uint32_t query_id)
{
  // ������� ������������
  std::uint32_t depth;
  // ����� ���������� ������� ���������, � �� (��������)
  std::uint32_t transmit_len;
  // ����� ���������� ������� ������, � �� (������ � ���������� � ��������)
  std::uint32_t receive_len;
  // �������� ������� ������ ������������ ������� ���������, �� ������� �� �������� �������������, � ��
  std::uint32_t range_offset_len = multiples_of(static_cast<std::uint32_t>(scan_pair.receive_time)
    - settings::phase_delay - settings::reserve_time, settings::frequency_factor);

  // ������ ��������� � ����� (��� ���������� �������� ����� 0), � ��
  std::uint32_t period_time = 0;

  // ��������� �������
  if (scan_pair.size == 1)
  {
    depth = settings::pulse_planning_depth * settings::planning_step;

    transmit_len = multiples_of(static_cast<std::uint32_t>(scan_pair.transmit[0].length) * settings::duty_factor
      + 2 * settings::frequency_delay
      + settings::blank_delay
      + settings::reserve_time, settings::frequency_factor);

    receive_len = multiples_of(static_cast<std::uint32_t>(scan_pair.receive[0].length + settings::reserve_time), settings::frequency_factor);
  }
  // �����
  else
  {
    depth = settings::pulse_train_planning_depth * settings::planning_step;
    
    period_time = static_cast<std::uint32_t>(scan_pair.transmit[1].offset_time);
    
    transmit_len = multiples_of(period_time * scan_pair.size + settings::blank_delay 
      + 2 * settings::frequency_delay + settings::reserve_time, settings::frequency_factor);
    
    receive_len = multiples_of(period_time * scan_pair.size + settings::reserve_time, settings::frequency_factor);
  }

  // ����� ����������� ����� ��� ��������� ������ �� ���
  auto idx = find_index(transmit_len, receive_len, range_offset_len, depth);
  if (idx != -1)
  {
    // ����� �����
    
    // �������� ��������������� �����������
    // ������������� ����� ������ ����������� ���� �����������, ��
    std::uint32_t tr_rephase_start_rel = idx * settings::time_chunk_length - settings::phase_delay;
    // ������������� ����� ����� ����������� ���� �����������, ��
    std::uint32_t tr_rephase_stop_rel = tr_rephase_start_rel + settings::phase_delay;

    tr_au_timeline.label_sector(
      tr_au_timeline.get_idx_for(tr_rephase_start_rel),
      tr_au_timeline.get_idx_for(tr_rephase_stop_rel),
      TimelineLabel::tll_au_rephase);

    // ������������� ����� ������ ������� ���������� ������ �����������, ��
    std::uint32_t tr_nocommand_start_rel = tr_rephase_stop_rel;
    // ������������� ����� ����� ����������� ���� �����������, ��
    std::uint32_t tr_nocommand_stop_rel = tr_nocommand_start_rel + (settings::au_command_delay - settings::phase_delay);

    tr_au_timeline.label_sector(
      tr_au_timeline.get_idx_for(tr_nocommand_start_rel),
      tr_au_timeline.get_idx_for(tr_nocommand_stop_rel),
      TimelineLabel::tll_au_command_delay);

    // �������� ���������
    dcu_timeline.label_sector(
      dcu_timeline.get_idx_for(tr_rephase_stop_rel),
      dcu_timeline.get_idx_for(tr_rephase_stop_rel + settings::blank_delay + settings::frequency_delay + settings::reserve_time),
      TimelineLabel::tll_dcu_transmit);

    for (std::size_t pulse_idx = 0; pulse_idx < scan_pair.size; ++pulse_idx)
    {
      // ������������� ����� ������ ��������� i-�� ��������, ��
      std::uint32_t transmit_start_rel = tr_rephase_stop_rel + settings::blank_delay + settings::reserve_time + settings::frequency_delay + pulse_idx * period_time;
      // ������������� ����� ����� ��������� i-�� ��������, ��
      std::uint32_t transmit_stop_rel = transmit_start_rel + scan_pair.transmit[pulse_idx].length + settings::blank_delay;// +2 * settings::frequency_delay;

      dcu_timeline.label_sector(
        dcu_timeline.get_idx_for(transmit_start_rel),
        dcu_timeline.get_idx_for(transmit_stop_rel),
        TimelineLabel::tll_dcu_transmit);

      // ������������� ����� ������ �������������� ����� i-�� ��������, ��
      std::uint32_t restore_start_rel = transmit_stop_rel;
      // ������������� ����� ����� �������������� ����� i-�� ��������, ��
      std::uint32_t restore_stop_rel = restore_start_rel + (settings::duty_factor - 1) * scan_pair.transmit[pulse_idx].length;

      dcu_timeline.label_sector(
        dcu_timeline.get_idx_for(restore_start_rel),
        dcu_timeline.get_idx_for(restore_stop_rel),
        TimelineLabel::tll_dcu_energy_restore);
    }

    // �������� ��������������� ���������
    // ������������� ����� ������ ����������� ���� �����������, ��
    std::uint32_t rs_rephase_start_rel = tr_rephase_stop_rel + range_offset_len;
    // ������������� ����� ����� ����������� ���� �����������, ��
    std::uint32_t rs_rephase_stop_rel = rs_rephase_start_rel + settings::phase_delay;

    rs_au_timeline.label_sector(
      rs_au_timeline.get_idx_for(rs_rephase_start_rel),
      rs_au_timeline.get_idx_for(rs_rephase_stop_rel),
      TimelineLabel::tll_au_rephase);

    // ������������� ����� ������ ������� ���������� ������ ���������, ��
    std::uint32_t rs_nocommand_start_rel = rs_rephase_stop_rel;
    // ������������� ����� ����� ����������� ���� ���������, ��
    std::uint32_t rs_nocommand_stop_rel = rs_nocommand_start_rel + (settings::au_command_delay - settings::phase_delay);

    rs_au_timeline.label_sector(
      rs_au_timeline.get_idx_for(rs_nocommand_start_rel),
      rs_au_timeline.get_idx_for(rs_nocommand_stop_rel),
      TimelineLabel::tll_au_command_delay);

    // �������� ������
    dcu_timeline.label_sector(
      dcu_timeline.get_idx_for(rs_rephase_stop_rel),
      dcu_timeline.get_idx_for(rs_rephase_stop_rel + settings::reserve_time),
      TimelineLabel::tll_dcu_receive);

    // �������������� �������� �������� ����� �������� ��� ������� ���� �����
    if (scan_pair.size > 1)
    {
      dcu_timeline.label_sector(
        dcu_timeline.get_idx_for(rs_rephase_stop_rel + settings::reserve_time),
        dcu_timeline.get_idx_for(rs_rephase_stop_rel + period_time * (scan_pair.size - 1)),
        TimelineLabel::tll_dcu_receive_prevent);
    }

    for (std::size_t pulse_idx = 0; pulse_idx < scan_pair.size; ++pulse_idx)
    {
      // ������������� ����� ������ ������ i-�� ��������, ��
      std::uint32_t receive_start_rel = rs_rephase_stop_rel + settings::reserve_time + pulse_idx * period_time;
      // ������������� ����� ����� ������ i-�� ��������, ��
      std::uint32_t receive_stop_rel = receive_start_rel + scan_pair.receive[pulse_idx].length;

      dcu_timeline.label_sector(
        dcu_timeline.get_idx_for(receive_start_rel),
        dcu_timeline.get_idx_for(receive_stop_rel),
        TimelineLabel::tll_dcu_receive);
    }

    // ��������� ����� ���������� ������ ����������
    // ����� ���������� ������� �� ����������� ���� �����������, ��
    std::uint64_t transmit_rephase_time = static_cast<std::uint64_t>(dcu_timeline.start_time + idx * settings::time_chunk_length - settings::phase_delay);

    // ����� ���������� ������� �� ���������, ��
    std::uint64_t transmit_exec_time = static_cast<std::uint64_t>(transmit_rephase_time + settings::phase_delay);
    // ����� ���������� ������� �� ����������� ���� ���������, ��
    std::uint64_t receive_rephase_time = static_cast<std::uint64_t>(transmit_exec_time + range_offset_len + settings::reserve_time);
    // ����� ���������� ������� �� �����, ��
    std::uint64_t receive_exec_time = static_cast<std::uint64_t>(receive_rephase_time + settings::phase_delay);
    // ����� ���������� ������� �� ���������/���������� ������ �������, ��
    std::uint64_t antenna_ptotection_time = static_cast<std::uint64_t>(receive_exec_time - settings::channel_switch_delay);

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
    // �� �����
    return{ ControlCommand() };
  }
}

// �������� ������ � ��������������� query_id
void Scheduler::remove_query(const std::uint32_t query_id)
{
  queries.erase(std::remove_if(queries.begin(), queries.end(), [query_id](const auto& val) { return val->id == query_id; }), queries.end());
}

// ���������� ������ ����� ������������ �� ������ ������� time
std::vector<ControlCommand> Scheduler::run(std::uint64_t time)
{
  // ������� ��� �� �������� �������
  dcu_timeline.move_timeline(time);
  tr_au_timeline.move_timeline(time);
  rs_au_timeline.move_timeline(time);

  // ������� ������ ������ ��� �������� ��������������� ������� ����������

  // ������ ������� ����������
  std::vector<ControlCommand> ret;

  // ������ ��������������� ��������� ������
  std::vector<std::uint32_t> removed_id;

  // ������ ����������� ������
  // ������ �������������� ����� ������������� (������) � ����� ������� (�������������). �������� ������ ���
  std::vector<std::shared_ptr<Query>> new_queries;

  // ���� ������ ����
  if (queries.size() == 1)
  {
    queries.front()->p_value = queries.front()->k * (time - queries.front()->t_prev) * 1.0e-9;
  }
  else
  {
    // ���� ����� �� ��������� ������ �� ������������� ����������, � ������� ����������
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
        // ��������� ������
        auto scan_pair = build_scan_pair(q->range, q->rcs, QueryType::search);

        // ������� ���������� �� ��� (� ��� � ��)
        auto commands = place_scan(scan_pair, q->id);

        // ��������� ����� ���������� ���������� (���� �������� �� ������)
        for (const auto& command : commands)
        {
          if (command.type != CommandType::ct_nop)
          {
            q->t_prev = time;
            processed_queries.push_back(q);
            break;
          }
        }

        // ��������� ���������� ������� � ������ ������
        ret.insert(ret.end(), commands.begin(), commands.end());

        break;
      }
      case QueryType::confirm:
      {
        // �������������
        auto scan_pair = build_scan_pair(q->range, q->rcs, QueryType::confirm);

        // ������� ���������� �� ��� (� ��� � ��)
        auto commands = place_scan(scan_pair, q->id);

        // ��������� ����� ���������� ���������� (���� �������� �� ������)
        for (const auto& command : commands)
        {
          if (command.type != CommandType::ct_nop)
          {
            q->t_prev = time;
            processed_queries.push_back(q);

            // ������� ������, �������.
            removed_id.push_back(q->id);

            // ��������� ����� ������ �� ������
            new_queries.push_back(std::make_shared<Query>(QueryType::capture, q->id + 1, settings::capture_query_speed, settings::capture_query_threshold, time, q->range, q->rcs, true));
            break;
          }
        }

        // ��������� ���������� ������� � ������ ������
        ret.insert(ret.end(), commands.begin(), commands.end());

        break;
      }
      case QueryType::capture:
      {
        // ������
        auto scan_pair = build_scan_pair(q->range, q->rcs, QueryType::capture);

        // ������� ���������� �� ��� (� ��� � ��)
        auto commands = place_scan(scan_pair, q->id);

        // ��������� ����� ���������� ���������� (���� �������� �� ������)
        for (const auto& command : commands)
        {
          if (command.type != CommandType::ct_nop)
          {
            q->t_prev = time;
            processed_queries.push_back(q);

            // ������� ������, �������.
            removed_id.push_back(q->id);

            // ��������� ����� ������ �� �������������
            new_queries.push_back(std::make_shared<Query>(QueryType::tracking, q->id + 1, settings::tracking_query_speed, settings::tracking_query_threshold, time, q->range, q->rcs, true));
            break;
          }
        }

        // ��������� ���������� ������� � ������ ������
        ret.insert(ret.end(), commands.begin(), commands.end());
        
        break;
      }
      case QueryType::tracking:
      {
        // �������������
        auto scan_pair = build_scan_pair(q->range, q->rcs, QueryType::tracking);

        // ������� ���������� �� ��� (� ��� � ��)
        auto commands = place_scan(scan_pair, q->id);

        // ��������� ����� ���������� ���������� (���� �������� �� ������)
        for (const auto& command : commands)
        {
          if (command.type != CommandType::ct_nop)
          {
            q->t_prev = time;
            processed_queries.push_back(q);
            break;
          }
        }

        // ��������� ���������� ������� � ������ ������
        ret.insert(ret.end(), commands.begin(), commands.end());

        break;
      }
      case QueryType::drop:
      {
        // ����� ������
        removed_id.push_back(q->id);
        processed_queries.push_back(q);
        break;
      }
      case QueryType::tech_control:
      {
        // ����������� ��������
        auto command = ControlCommand(CommandType::ct_tech_command, time - settings::reserve_time, static_cast<std::uint64_t>(time), 0, q->id);

        // ��������� ����� ���������� ����������
        q->t_prev = time;

        processed_queries.push_back(q);

        // ��������� ������� � ������ ������
        ret.push_back(command);

        break;
      }
      default:
        break;
      }
    }
  }

  // ������� ���������� ������
  for (const auto& id : removed_id)
    remove_query(id);

  // ��������� ����� ������
  for (const auto& q : new_queries)
    queries.push_back(q);

  return ret;
}

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

void Scheduler::save_timilines(Timeline &_dcu_timeline, Timeline &_au_tr_timeline, Timeline &_au_rs_timeline)
{
    _dcu_timeline = dcu_timeline;
    _au_tr_timeline = tr_au_timeline;
    _au_rs_timeline = rs_au_timeline;
}

void Scheduler::reset()
{
    queries.clear();
    dcu_timeline.reset();
    tr_au_timeline.reset();
    rs_au_timeline.reset();
}

#include "Scheduler.h"


// Найти индекс на КВД, подходящий для размещения участка transmit_len (нс) и соответствующего ему receive_len (нс) 
// со смещением range_offset(нс), на глубину планирования depth.
// Возвращает -1 в случае если такого индекса не существует и положительное целое, равное индексу, в случае успеха.

std::int32_t Scheduler::find_index(const std::uint32_t transmit_len, const std::uint32_t receive_len, const std::uint32_t range_offset, const std::uint32_t depth)
{
  // Длина участка излучения, в дискретах
  std::uint32_t tr_length = static_cast<std::uint32_t>(std::ceil(static_cast<double>(transmit_len) / settings::time_chunk_length));

  if (tr_length >= depth)
  {
    throw ModelException("ERROR: Transmit command length is larger than planning depth in find_index.");
  }

  // Длина участка приема, в дискретах
  std::uint32_t rs_length = static_cast<std::uint32_t>(std::ceil(static_cast<double>(receive_len) / settings::time_chunk_length));;

  // Длина участка перефазировки в дискретах
  std::uint32_t rph_length = static_cast<std::uint32_t>(std::ceil(static_cast<double>(settings::phase_delay) / settings::time_chunk_length));;

  std::uint32_t dcu_sum = 0;
  std::uint32_t au_sum = 0;
  
  std::vector<std::uint16_t> au_indices(depth);

  // Расчет префиксной суммы с индексами размещения
  for (std::uint32_t idx = 0; idx < depth; ++idx)
  {
    // Префиксная сумма УЦП
    if (dcu_timeline.get_value_at(idx) == TimelineLabel::tll_empty)
      dcu_sum++;
    else
      dcu_sum = 0;
    // Префиксная сумма АУ
    if (tr_au_timeline.get_value_at(idx) == TimelineLabel::tll_empty)
      au_sum++;
    else
      au_sum = 0;

    // Определяем порог УЦП
    auto dcu_value = dcu_sum > tr_length ? 1 : 0;

    // Определяем порог АУ
    auto au_value = au_sum > rph_length ? 1 : 0;
    au_indices[idx] = au_value;

    if (dcu_value == 1)
    {
      auto tr_idx = idx - tr_length;

      // Наложение
      if (dcu_value * au_indices[tr_idx] == 1)
      {
        // Получилось разместить и излучение и перефазировку
        // Проверяем возможность размещения приема и перефазировки
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
          // Разместили удачно. Возвращаем полученный индекс.
          return idx - tr_length;
        }
      }

    }
  }
  return -1;
}

// Разместить временную связку scan_pair на КВД в интересах обслуживания заявки query_id.
// В случае успеха возвращает список команд управления УЦП и АУ. В случае неудачи возвращает комманды типа nop (no operation).


std::vector<ControlCommand> Scheduler::place_scan(const ScanPair & scan_pair, const std::uint32_t query_id)
{
  // Глубина планирования
  std::uint32_t depth;
  // Длина выполнения команды излучения, в нс (бланками)
  std::uint32_t transmit_len;
  // Длина выполнения команды приема, в нс (вместе с оцифровкой и бланками)
  std::uint32_t receive_len;
  // Смещение команды приема относительно команды излучения, со сдвигом на величину перефазировки, в нс
  std::uint32_t range_offset_len = multiples_of(static_cast<std::uint32_t>(scan_pair.receive_time)
    - settings::phase_delay - settings::reserve_time, settings::frequency_factor);

  // Период импульсов в пачке (для одиночного импульса равен 0), в нс
  std::uint32_t period_time = 0;

  // Одиночный импульс
  if (scan_pair.size == 1)
  {
    depth = settings::pulse_planning_depth * settings::planning_step;

    transmit_len = multiples_of(static_cast<std::uint32_t>(scan_pair.transmit[0].length) * settings::duty_factor
      + 2 * settings::frequency_delay
      + settings::blank_delay
      + settings::reserve_time, settings::frequency_factor);

    receive_len = multiples_of(static_cast<std::uint32_t>(scan_pair.receive[0].length + settings::reserve_time), settings::frequency_factor);
  }
  // Пачка
  else
  {
    depth = settings::pulse_train_planning_depth * settings::planning_step;
    
    period_time = static_cast<std::uint32_t>(scan_pair.transmit[1].offset_time);
    
    transmit_len = multiples_of(period_time * scan_pair.size + settings::blank_delay 
      + 2 * settings::frequency_delay + settings::reserve_time, settings::frequency_factor);
    
    receive_len = multiples_of(period_time * scan_pair.size + settings::reserve_time, settings::frequency_factor);
  }

  // Поиск подходящего места для временной связки на КВД
  auto idx = find_index(transmit_len, receive_len, range_offset_len, depth);
  if (idx != -1)
  {
    // Место нашли
    
    // Разметка перефазирования передатчика
    // Относительное время начала перестройки фазы передатчика, нс
    std::uint32_t tr_rephase_start_rel = idx * settings::time_chunk_length - settings::phase_delay;
    // Относительное время конца перестройки фазы передатчика, нс
    std::uint32_t tr_rephase_stop_rel = tr_rephase_start_rel + settings::phase_delay;

    tr_au_timeline.label_sector(
      tr_au_timeline.get_idx_for(tr_rephase_start_rel),
      tr_au_timeline.get_idx_for(tr_rephase_stop_rel),
      TimelineLabel::tll_au_rephase);

    // Относительное время начала запрета выполнения команд передатчика, нс
    std::uint32_t tr_nocommand_start_rel = tr_rephase_stop_rel;
    // Относительное время конца перестройки фазы передатчика, нс
    std::uint32_t tr_nocommand_stop_rel = tr_nocommand_start_rel + (settings::au_command_delay - settings::phase_delay);

    tr_au_timeline.label_sector(
      tr_au_timeline.get_idx_for(tr_nocommand_start_rel),
      tr_au_timeline.get_idx_for(tr_nocommand_stop_rel),
      TimelineLabel::tll_au_command_delay);

    // Разметка излучения
    dcu_timeline.label_sector(
      dcu_timeline.get_idx_for(tr_rephase_stop_rel),
      dcu_timeline.get_idx_for(tr_rephase_stop_rel + settings::blank_delay + settings::frequency_delay + settings::reserve_time),
      TimelineLabel::tll_dcu_transmit);

    for (std::size_t pulse_idx = 0; pulse_idx < scan_pair.size; ++pulse_idx)
    {
      // Относительное время начала излучения i-го импульса, нс
      std::uint32_t transmit_start_rel = tr_rephase_stop_rel + settings::blank_delay + settings::reserve_time + settings::frequency_delay + pulse_idx * period_time;
      // Относительное время конца излучения i-го импульса, нс
      std::uint32_t transmit_stop_rel = transmit_start_rel + scan_pair.transmit[pulse_idx].length + settings::blank_delay;// +2 * settings::frequency_delay;

      dcu_timeline.label_sector(
        dcu_timeline.get_idx_for(transmit_start_rel),
        dcu_timeline.get_idx_for(transmit_stop_rel),
        TimelineLabel::tll_dcu_transmit);

      // Относительное время начала восстановления после i-го импульса, нс
      std::uint32_t restore_start_rel = transmit_stop_rel;
      // Относительное время концы восстановления после i-го импульса, нс
      std::uint32_t restore_stop_rel = restore_start_rel + (settings::duty_factor - 1) * scan_pair.transmit[pulse_idx].length;

      dcu_timeline.label_sector(
        dcu_timeline.get_idx_for(restore_start_rel),
        dcu_timeline.get_idx_for(restore_stop_rel),
        TimelineLabel::tll_dcu_energy_restore);
    }

    // Разметка перефазирования приемника
    // Относительное время начала перестройки фазы передатчика, нс
    std::uint32_t rs_rephase_start_rel = tr_rephase_stop_rel + range_offset_len;
    // Относительное время конца перестройки фазы передатчика, нс
    std::uint32_t rs_rephase_stop_rel = rs_rephase_start_rel + settings::phase_delay;

    rs_au_timeline.label_sector(
      rs_au_timeline.get_idx_for(rs_rephase_start_rel),
      rs_au_timeline.get_idx_for(rs_rephase_stop_rel),
      TimelineLabel::tll_au_rephase);

    // Относительное время начала запрета выполнения команд приемника, нс
    std::uint32_t rs_nocommand_start_rel = rs_rephase_stop_rel;
    // Относительное время конца перестройки фазы приемника, нс
    std::uint32_t rs_nocommand_stop_rel = rs_nocommand_start_rel + (settings::au_command_delay - settings::phase_delay);

    rs_au_timeline.label_sector(
      rs_au_timeline.get_idx_for(rs_nocommand_start_rel),
      rs_au_timeline.get_idx_for(rs_nocommand_stop_rel),
      TimelineLabel::tll_au_command_delay);

    // Разметка приема
    dcu_timeline.label_sector(
      dcu_timeline.get_idx_for(rs_rephase_stop_rel),
      dcu_timeline.get_idx_for(rs_rephase_stop_rel + settings::reserve_time),
      TimelineLabel::tll_dcu_receive);

    // Дополнительная разметка участков между приемами для сигнала типа пачки
    if (scan_pair.size > 1)
    {
      dcu_timeline.label_sector(
        dcu_timeline.get_idx_for(rs_rephase_stop_rel + settings::reserve_time),
        dcu_timeline.get_idx_for(rs_rephase_stop_rel + period_time * (scan_pair.size - 1)),
        TimelineLabel::tll_dcu_receive_prevent);
    }

    for (std::size_t pulse_idx = 0; pulse_idx < scan_pair.size; ++pulse_idx)
    {
      // Относительное время начала приема i-го импульса, нс
      std::uint32_t receive_start_rel = rs_rephase_stop_rel + settings::reserve_time + pulse_idx * period_time;
      // Относительное время конца приема i-го импульса, нс
      std::uint32_t receive_stop_rel = receive_start_rel + scan_pair.receive[pulse_idx].length;

      dcu_timeline.label_sector(
        dcu_timeline.get_idx_for(receive_start_rel),
        dcu_timeline.get_idx_for(receive_stop_rel),
        TimelineLabel::tll_dcu_receive);
    }

    // Формируем время исполнения команд управления
    // Время исполнения команды на перестройку фазы передатчика, нс
    std::uint64_t transmit_rephase_time = static_cast<std::uint64_t>(dcu_timeline.start_time + idx * settings::time_chunk_length - settings::phase_delay);

    // Время исполнения команды на излучение, нс
    std::uint64_t transmit_exec_time = static_cast<std::uint64_t>(transmit_rephase_time + settings::phase_delay);
    // Время исполнения команды на перестройку фазы приемника, нс
    std::uint64_t receive_rephase_time = static_cast<std::uint64_t>(transmit_exec_time + range_offset_len + settings::reserve_time);
    // Время исполнения команды на прием, нс
    std::uint64_t receive_exec_time = static_cast<std::uint64_t>(receive_rephase_time + settings::phase_delay);
    // Время исполнения команды на включение/отключение защиты антенны, нс
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
    // Не нашли
    return{ ControlCommand() };
  }
}

// Удаление заявки с идентификатором query_id
void Scheduler::remove_query(const std::uint32_t query_id)
{
  queries.erase(std::remove_if(queries.begin(), queries.end(), [query_id](const auto& val) { return val->id == query_id; }), queries.end());
}

// Выполнение одного такта планирования на момент времени time
std::vector<ControlCommand> Scheduler::run(std::uint64_t time)
{
  // Смещаем КВД до текущего времени
  dcu_timeline.move_timeline(time);
  tr_au_timeline.move_timeline(time);
  rs_au_timeline.move_timeline(time);

  // Перебор списка заявок для создания соответствующих комманд управления

  // Список комманд управления
  std::vector<ControlCommand> ret;

  // Список идентификаторов удаляемых заявок
  std::vector<std::uint32_t> removed_id;

  // Массив добавляемых заявок
  // Заявки активизируются после дообнаружения (захват) и после захвата (сопровождение). Имитация работы ИОТ
  std::vector<std::shared_ptr<Query>> new_queries;

  // Если заявки одна
  if (queries.size() == 1)
  {
    queries.front()->p_value = queries.front()->k * (time - queries.front()->t_prev) * 1.0e-9;
  }
  else
  {
    // Если много то сортируем заявки по динамическому приоритету, в порядке уменьшения
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
        // Поисковая заявка
        auto scan_pair = build_scan_pair(q->range, q->rcs, QueryType::search);

        // Попытка разместить на КВД (и УЦП и АУ)
        auto commands = place_scan(scan_pair, q->id);

        // Обновляем время последнего исполнения (если комманды не пустые)
        for (const auto& command : commands)
        {
          if (command.type != CommandType::ct_nop)
          {
            q->t_prev = time;
            processed_queries.push_back(q);
            break;
          }
        }

        // Добавляем полученные команды в список команд
        ret.insert(ret.end(), commands.begin(), commands.end());

        break;
      }
      case QueryType::confirm:
      {
        // Дообнаружение
        auto scan_pair = build_scan_pair(q->range, q->rcs, QueryType::confirm);

        // Попытка разместить на КВД (и УЦП и АУ)
        auto commands = place_scan(scan_pair, q->id);

        // Обновляем время последнего исполнения (если комманды не пустые)
        for (const auto& command : commands)
        {
          if (command.type != CommandType::ct_nop)
          {
            q->t_prev = time;
            processed_queries.push_back(q);

            // Разовая заявка, удаляем.
            removed_id.push_back(q->id);

            // Добавляем новую заявку на захват
            new_queries.push_back(std::make_shared<Query>(QueryType::capture, q->id + 1, settings::capture_query_speed, settings::capture_query_threshold, time, q->range, q->rcs, true));
            break;
          }
        }

        // Добавляем полученные команды в список команд
        ret.insert(ret.end(), commands.begin(), commands.end());

        break;
      }
      case QueryType::capture:
      {
        // Захват
        auto scan_pair = build_scan_pair(q->range, q->rcs, QueryType::capture);

        // Попытка разместить на КВД (и УЦП и АУ)
        auto commands = place_scan(scan_pair, q->id);

        // Обновляем время последнего исполнения (если комманды не пустые)
        for (const auto& command : commands)
        {
          if (command.type != CommandType::ct_nop)
          {
            q->t_prev = time;
            processed_queries.push_back(q);

            // Разовая заявка, удаляем.
            removed_id.push_back(q->id);

            // Добавляем новую заявку на сопровождение
            new_queries.push_back(std::make_shared<Query>(QueryType::tracking, q->id + 1, settings::tracking_query_speed, settings::tracking_query_threshold, time, q->range, q->rcs, true));
            break;
          }
        }

        // Добавляем полученные команды в список команд
        ret.insert(ret.end(), commands.begin(), commands.end());
        
        break;
      }
      case QueryType::tracking:
      {
        // Сопровождение
        auto scan_pair = build_scan_pair(q->range, q->rcs, QueryType::tracking);

        // Попытка разместить на КВД (и УЦП и АУ)
        auto commands = place_scan(scan_pair, q->id);

        // Обновляем время последнего исполнения (если комманды не пустые)
        for (const auto& command : commands)
        {
          if (command.type != CommandType::ct_nop)
          {
            q->t_prev = time;
            processed_queries.push_back(q);
            break;
          }
        }

        // Добавляем полученные команды в список команд
        ret.insert(ret.end(), commands.begin(), commands.end());

        break;
      }
      case QueryType::drop:
      {
        // Сброс заявки
        removed_id.push_back(q->id);
        processed_queries.push_back(q);
        break;
      }
      case QueryType::tech_control:
      {
        // Технический контроль
        auto command = ControlCommand(CommandType::ct_tech_command, time - settings::reserve_time, static_cast<std::uint64_t>(time), 0, q->id);

        // Обновляем время последнего исполнения
        q->t_prev = time;

        processed_queries.push_back(q);

        // Добавляем команду в список команд
        ret.push_back(command);

        break;
      }
      default:
        break;
      }
    }
  }

  // Удаляем сброшенные заявки
  for (const auto& id : removed_id)
    remove_query(id);

  // Добавляем новые заявки
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

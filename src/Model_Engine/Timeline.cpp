#include "Timeline.h"


bool Timeline::check_index(const std::uint32_t length, const std::uint32_t idx, std::function<bool(const pipiline_type::value_type&)> pred)
{
  assert(length < settings::timeline_depth);
  assert(idx < settings::timeline_depth);

  for (std::uint32_t offset = 0; offset < length; ++offset)
  {
    // Выход за пределы КВД
    if (idx + offset >= settings::timeline_depth)
      return false;

    auto timeline_value = get_value_at(idx + offset);

    // Проверка КВД на метки свободно (free) или время восстановления после излучения (energy_restore)
    if (pred(timeline_value))
      continue;
    else
      return false;
  }

  return true;
}

void Timeline::label_sector(const std::uint32_t start_idx, const std::uint32_t stop_idx, const TimelineLabel label)
{
  if (start_idx > stop_idx)
  {
    throw ModelException("ERROR: Start index is larger than stop index in label_sector.");
  }

  for (std::size_t idx = start_idx; idx <= stop_idx; ++idx)
  {
    get_value_at(idx) = label;
  }
}

std::int32_t Timeline::get_idx_for(const std::uint32_t time)
{
  // По факту работаем со смещенным времем для устранения возможности
  // наложения дискретов на их границах, если время кратно длительности дискрета
  auto shifted_time = time;

  if (time > 0)
  {
      shifted_time--;
  }

  if (shifted_time > settings::timeline_depth * settings::time_chunk_length)
  {
    throw ModelException("ERROR: Time is out of range in get_idx_for.");
  }

  auto ret = shifted_time / settings::time_chunk_length;

  // Проверка попадания на границу дискрета
  if (shifted_time % settings::time_chunk_length == 0)
    // Если попали, то считает как следующий дискрет
    return ret + 1;
  else
    return ret;

}

void Timeline::move_timeline(std::uint64_t time)
{
  if (time < start_time)
  {
    throw ModelException("ERROR: can't move timeline backward.");
  }

  // Смещение по времени
  std::uint32_t time_offset = static_cast<std::uint32_t>(time - start_time);

  // Обновляем время привязки КВД, с учетов выравнивания по границе дискретов КВД
  start_time = multiples_of(time, static_cast<std::uint64_t>(settings::time_chunk_length));

  //if (start_time > settings::time_chunk_length)
  //  start_time -= settings::time_chunk_length;

  // Очищаем крайний "левый" (ближайщий по времени) участок конвейера
  for (std::size_t offset = 0; offset < time_offset / settings::time_chunk_length; ++offset)
    get_value_at(offset) = TimelineLabel::tll_empty;

  // Смещаем индекс первого дискрета КВД
  start_idx = (start_idx + time_offset / settings::time_chunk_length) % settings::timeline_depth;
}

double Timeline::occupation()
{
  std::uint32_t empty_counter = 0;
  auto occupation_size = settings::pulse_planning_depth * settings::planning_step;
  for (std::size_t idx = 0; idx < occupation_size; ++idx)
  {
    if (get_value_at(idx) == TimelineLabel::tll_empty)
      empty_counter++;
  }

  return 1.0 - static_cast<double>(empty_counter) / occupation_size;
}

void Timeline::reset()
{
    timeline.fill(TimelineLabel::tll_empty);
    start_idx = 0;
    start_time = 0;
}

TimelineLabel& Timeline::get_value_at(const std::size_t idx)
{
  return timeline[(start_idx + idx) % settings::timeline_depth];
}

const TimelineLabel& Timeline::get_value_at(const std::size_t idx) const
{
  return timeline[(start_idx + idx) % settings::timeline_depth];
}

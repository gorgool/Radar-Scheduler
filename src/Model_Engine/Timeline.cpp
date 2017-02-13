#include "Timeline.h"

// Check if it is posible to allocate region of length starting from idx with given pred (predicate).
// Return true in case of success. false otherwise.
bool Timeline::check_index(const std::uint32_t length, const std::uint32_t idx, std::function<bool(const pipiline_type::value_type&)> pred)
{
  assert(length < settings::timeline_depth);
  assert(idx < settings::timeline_depth);

  for (std::uint32_t offset = 0; offset < length; ++offset)
  {
    // Out of timeline bound
    if (idx + offset >= settings::timeline_depth)
      return false;

    auto timeline_value = get_value_at(idx + offset);

    if (pred(timeline_value))
      continue;
    else
      return false;
  }

  return true;
}

// Mark timeline region from start_idx to stop_idx with given label.
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

// Return index for timeline chunk that corresponds to given relative time, g.e. time=0 return index=0.
// Return -1 if there is not such index.
std::int32_t Timeline::get_idx_for(const std::uint32_t time)
{
  auto shifted_time = time;

  if (time > 0)
  {
    // Move time a little for eliminating possibility to overlap chunks if time 
    // is equal to a value multiple of timeline chunk length
    shifted_time--;
  }

  if (shifted_time > settings::timeline_depth * settings::time_chunk_length)
  {
    throw ModelException("ERROR: Time is out of range in get_idx_for.");
  }

  auto ret = shifted_time / settings::time_chunk_length;

  // If time at the border of chunks
  if (shifted_time % settings::time_chunk_length == 0)
    // Return the next one
    return ret + 1;
  else
    return ret;

}

// Move timetile till given time. Update start_time and start_idx and prepare new region.
void Timeline::move_timeline(std::uint64_t time)
{
  if (time < start_time)
  {
    throw ModelException("ERROR: can't move timeline backward.");
  }

  std::uint32_t time_offset = static_cast<std::uint32_t>(time - start_time);
  start_time = multiples_of(time, static_cast<std::uint64_t>(settings::time_chunk_length));

  // CLear the oldest region 
  for (std::size_t offset = 0; offset < time_offset / settings::time_chunk_length; ++offset)
    get_value_at(offset) = TimelineLabel::tll_empty;

  start_idx = (start_idx + time_offset / settings::time_chunk_length) % settings::timeline_depth;
}

// Compute occupation coefficient.
double Timeline::occupation()
{
  std::uint32_t empty_counter = 0;
  auto occupation_size = settings::planning_step;
  for (std::size_t idx = 0; idx < occupation_size; ++idx)
  {
    if (get_value_at(idx) == TimelineLabel::tll_empty)
      empty_counter++;
  }

  return 1.0 - static_cast<double>(empty_counter) / occupation_size;
}

// Reset all.
void Timeline::reset()
{
    timeline.fill(TimelineLabel::tll_empty);
    start_idx = 0;
    start_time = 0;
}

// Return refarance to timeline chunk by idx. Idx=0 correspond to most recent chunk.
TimelineLabel& Timeline::get_value_at(const std::size_t idx)
{
  return timeline[(start_idx + idx) % settings::timeline_depth];
}

const TimelineLabel& Timeline::get_value_at(const std::size_t idx) const
{
  return timeline[(start_idx + idx) % settings::timeline_depth];
}

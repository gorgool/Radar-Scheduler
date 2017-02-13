#pragma once

#include <array>
#include <functional>
#include <vector>
#include <cassert>
#include <iostream>
#include "Utility.h"
#include "ModelException.h"

// Timeline labels
enum TimelineLabel : std::uint8_t
{
  tll_empty,                // Free (Not occupied)
  tll_dcu_receive,          // DCU receive command
  tll_dcu_transmit,         // DCU transmit command
  tll_au_rephase,           // AU rephasing command
  tll_au_command_delay,     // AU command execution ban
  tll_au_channel_switch,    // AU recieve/transmit channel switch command
  tll_dcu_energy_restore,   // DCU transmit duty cycle delay
  tll_dcu_receive_prevent   // DCU coherent pulse train recive
};

/*
  Timeline
*/

class Timeline
{
  using pipiline_type = std::array<TimelineLabel, settings::timeline_depth>;

public:

  // First (idx = 0) timeline chunk referance time, ns
  std::uint64_t start_time;

  // Internal index of first timeline chunk
  std::uint32_t start_idx;

  // Timeline array
  pipiline_type timeline;

  // Check if it is posible to allocate region of length starting from idx with given pred (predicate).
  // Return true in case of success. false otherwise.
  bool check_index(const std::uint32_t length, const std::uint32_t idx, std::function<bool(const pipiline_type::value_type&)> pred);

  // Mark timeline region from start_idx to stop_idx with given label.
  void label_sector(const std::uint32_t start_idx, const std::uint32_t stop_idx, const TimelineLabel label);

  // Return refarance to timeline chunk by idx. Idx=0 correspond to most recent chunk.
  const TimelineLabel& get_value_at(const std::size_t idx) const;
  TimelineLabel& get_value_at(const std::size_t idx);

  // Return index for timeline chunk that corresponds to given relative time, g.e. time=0 return index=0.
  // Return -1 if there is not such index.
  std::int32_t get_idx_for(const std::uint32_t time);

  // Move timetile till given time. Update start_time and start_idx and prepare new region.
  void move_timeline(std::uint64_t time);

  // Compute occupation coefficient.
  double occupation();

  // Reset all.
  void reset();

  // Contructor.
  Timeline() : start_time(0), start_idx(0) { timeline.fill(TimelineLabel::tll_empty); }

  // Destructor.
  ~Timeline() {};
};

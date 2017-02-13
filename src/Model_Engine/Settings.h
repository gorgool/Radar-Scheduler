#pragma once
#include <cinttypes>

/*
  Settings
*/

namespace settings
{
  // Time delay of rephase command (change beam direction), ns
  static const std::uint32_t phase_delay = 270000;

  // AU Command execution period, ns
  static const std::uint32_t au_command_delay = 500000;

  // Reserve time before and after pulses, ns
  static const std::uint32_t blank_delay = 3100;

  // Frequency switch time, ns
  static const std::uint32_t frequency_delay = 3100;

  // Time delay of channel_switch command (change transmit and recieve channels), ns
  static const std::uint32_t channel_switch_delay = 3000;

  // Pulses duty cycle
  static const std::uint32_t duty_factor = 8;

  // Referance time frequency factor, ns
  static const std::uint32_t frequency_factor = 100;

  // Control execution estimated time delay.
  static const std::uint32_t reserve_time = 5000;

  // Search query growth rate (search with 50 Hz rate).
  static const double search_query_speed = 50.0;

  // Search query threshold.
  static const double search_query_threshold = 1.0;
  
  // Tracking query growth rate.
  static const double tracking_query_speed = 10.0;

  // Confirmation query growth rate.
  static const double confirm_query_speed = 100.0;

  // Capture query growth rate.
  static const double capture_query_speed = 100.0;

  // Device status request query growth rate.
  static const double tech_control_speed = 1.0;

  // Tracking query threshold.
  static const double tracking_query_threshold = 1.0;

  // Confirmation query threshold.
  static const double confirm_query_threshold = 0.0;

  // Capture query growth threshold.
  static const double capture_query_threshold = 0.0;

  // Device status request query threshold.
  static const double tech_control_threshold = 1.0;

  // Timeline chunk (the samllest part), ns
  static const std::uint32_t time_chunk_length = 10000;

  // Planning period (1 ms), in timeline chunks
  static const std::uint32_t planning_step = 100;

  // Timeline length (1 second), in timeline chunks
  static const std::uint32_t timeline_depth = 100000;

  // Planning depth (10 ms), in timeline chunks
  static const std::uint32_t planning_depth = 10;
}

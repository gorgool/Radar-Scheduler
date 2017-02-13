#pragma once
#include <cinttypes>

// Query type
enum QueryType
{
  search,        // Search
  confirm,       // Target confirmation
  capture,       // Target capture
  tracking,      // Target tracking
  signal_change, // Переход на другой тип сигнала
  tech_control,  // Device state request
  drop,          // Drop query
};

/*
  Query struct
*/
struct Query
{
  // Query type
  QueryType type;

  // Query ID
  std::uint32_t id;

  // Priority growth rate
  double k;

  // Execution priority threshold
  double p_threshold;

  // Last (Previous) execution time, ns
  std::uint64_t t_prev;

  // Current value of priority
  double p_value;

  // Estimate range to target, m
  double range;

  // Estimate RCS of target, sq.m
  double rcs;

  // Activity flag
  bool active;

  Query(const QueryType query_type,
        const uint32_t query_id,
        const double query_k,
        const double query_threshold,
        const std::uint64_t query_time,
        const double estimate_range,
        const double estimate_rcs,
        const bool active_state = false) : type(query_type), id(query_id), k(query_k), p_threshold(query_threshold),
                                           t_prev(query_time), p_value(0.0), range(estimate_range), rcs(estimate_rcs),
                                           active(active_state) {}
};

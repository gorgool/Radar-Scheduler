#pragma once

#include <cinttypes>
#include "Settings.h"
#include <cmath>

/*
  Utility function for round-up value to nearest value that is mulpiples of given number.
  value - unput number
  number - frequency factor
*/

template<typename RetT>
RetT multiples_of(const RetT value, const RetT number = settings::frequency_factor)
{
  if (value % number == 0)
    return value;

  RetT k = static_cast<RetT>(std::floor(value / number));
  return (k + 1) * number;
}

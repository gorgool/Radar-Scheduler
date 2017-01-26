#pragma once

#include <cinttypes>
#include "Settings.h"
#include <cmath>

/*
¬спомогательна€ функци€ округлени€ времени до кратных величин в большую сторону.
value - преобразуемое число
number - фактор кратности
*/

template<typename RetT>
RetT multiples_of(const RetT value, const RetT number = settings::frequency_factor)
{
  if (value % number == 0)
    return value;

  RetT k = static_cast<RetT>(std::floor(value / number));
  return (k + 1) * number;
}

#pragma once

#include <cinttypes>
#include "Settings.h"
#include <cmath>

/*
��������������� ������� ���������� ������� �� ������� ������� � ������� �������.
value - ������������� �����
number - ������ ���������
*/

template<typename RetT>
RetT multiples_of(const RetT value, const RetT number = settings::frequency_factor)
{
  if (value % number == 0)
    return value;

  RetT k = static_cast<RetT>(std::floor(value / number));
  return (k + 1) * number;
}

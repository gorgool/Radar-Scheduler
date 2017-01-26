#pragma once

#include "ScanPair.h"
#include "Utility.h"
#include "Query.h"
#include <map>
#include <inttypes.h>
#include <cassert>
#include <algorithm>
#define _USE_MATH_DEFINES
#include <math.h>

/*
  ����� ���� ������������ ������� �� ��������� ��������� � ��������� ��� ��.
  ����������: � ������� ������ ������� ���� ����� ������������ ��� ��������� ��������� � ����������� �������
  range - �������������� ��������� �� ��, �.
  rcs - �������������� ����������� ����������� �����������, ��. �.
  type - ��� ������
*/

ScanPair build_scan_pair(const double range, const double rcs, const QueryType type);

/*
  ���� ����� �������
*/
enum SignalType
{
  // ������� ����������� � �������������
  SOTS1, SOTS2, SOTS3, SOTS4, SOTS5,
  SOTS6, SOTS7, SOTS8, SOTS9, SOTS10,
  SOTS11, SOTS12, SOTS13, SOTS14, SOTS15,
  // �����������
  SAFM1, SAFM2, SAFM3, SAFM4, SSS,
  SDP1, SDP2, SDP3, SDP4, SDSPS
};

/*
  ��������� �������
*/
struct SignalParams
{
  // ������������ �������, �����������
  std::uint32_t duration;

  // �������� ���, ���
  std::uint32_t deviation;

  // ������ ������� ������� ���������� ��������� � �����, ��
  std::uint32_t period_low;

  // ������� ������� ������� ���������� ��������� � �����, ��
  std::uint32_t period_high;

  // ������ ������� ���������� ��������� � �����
  std::uint16_t pulses_number_low;

  // ������� ������� ���������� ��������� � �����
  std::uint16_t pulse_number_high;

  // ������������ ����� ���������, ��
  std::uint32_t max_process_time;
};
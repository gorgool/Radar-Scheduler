#pragma once

#include <cinttypes>
#include <vector>
#include <memory>
#include "Timeline.h"
#include "Query.h"
#include "ModelState.h"
#include "BuildScanPair.h"

class Scheduler
{
  // �������� ��������� ��������� ��� (Digital Converter Unit)
  Timeline dcu_timeline;

  // �������� ��������� ��������� �� ��������� ���������� (Antenna Unit)
  Timeline tr_au_timeline;

  // �������� ��������� ��������� �� ����������� ����������
  Timeline rs_au_timeline;

  // ����� ������ �� ���, ���������� ��� ���������� ������� transmit_len (��) � ���������������� ��� receive_len (��) 
  // �� ��������� range_offset(��), �� ������� ������������ depth.
  // ���������� -1 � ������ ���� ������ ������� �� ���������� � ������������� �����, ������ �������, � ������ ������.
  std::int32_t find_index(const std::uint32_t transmit_len, const std::uint32_t receive_len, const std::uint32_t range_offset, const std::uint32_t depth);

  // ���������� ��������� ������ scan_pair �� ��� ��� dcu_timeline � ��� �� au_timeline � ��������� ������������ ������ query_id.
  // � ������ ������ ���������� ������ ������ ���������� ��� � ��. � ������ ������� ���������� �������� ���� nop (no operation).
  std::vector<ControlCommand> place_scan(const ScanPair& scan_pair, const std::uint32_t query_id);

  // �������� ������ � ��������������� query_id
  void remove_query(const std::uint32_t query_id);

public:
  Scheduler() {}

  using QueryListType = std::vector<std::shared_ptr<Query>>;

  // ������ ������
  QueryListType queries;

  // ������ ����������� � ��������� ������� �-�� run ������
  QueryListType processed_queries;

  // ���������� ������ ����� ������������ �� ������ ������� time
  std::vector<ControlCommand> run(std::uint64_t time);

  // ��������� ���������� ��������� model_state ����������� �������� � ����������� � ��������� ������� �-�� run ������
  void get_statistics(ModelState& model_state);

  // ��������� ��������� ��� �� ���������� �������
  void save_timilines(Timeline& _dcu_timeline, Timeline& _au_tr_timeline, Timeline& _au_rs_timeline);

  // �������� ������ ������ � �������� ���
  void reset();
};

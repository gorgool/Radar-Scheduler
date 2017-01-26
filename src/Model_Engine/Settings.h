#pragma once
#include <cinttypes>

/*
  �������� ��������� � ���������
*/

namespace settings
{
  // �������� ��������� ��� ������� (��������� ����������� �����), ��
  static const std::uint32_t phase_delay = 270000;

  // �������� ���������� ������ �� ��, ��
  static const std::uint32_t au_command_delay = 500000;

  // ����� ��������� (������� � ������ � ����� ���������), ��
  static const std::uint32_t blank_delay = 3100;

  // ����� ������������ ��������� ������, ��
  static const std::uint32_t frequency_delay = 3100;

  // ����� ������������ �������, � ���������, ��
  static const std::uint32_t channel_switch_delay = 3000;

  // ����������� ����������
  static const std::uint32_t duty_factor = 8;

  // ����������� ��������� ������� ��������, ��
  static const std::uint32_t frequency_factor = 100;

  // ����� �������� ��������. ����� ������ ����� �� �������� �� ���������� ����������� ������ (������������� + ��������� � �.�)
  static const std::uint32_t reserve_time = 5000;

  // ����������� ����� ���c����� ������ (� �������� ����� ������ 50 ��)
  static const double search_query_speed = 50.0;

  // ����� ���������� ���c����� ������
  static const double search_query_threshold = 1.0;
  
  // ����������� ����� ������ �� �������������
  static const double tracking_query_speed = 10.0;

  // ����������� ����� ������ �� �������������
  static const double confirm_query_speed = 100.0;

  // ����������� ����� ������ �� ������
  static const double capture_query_speed = 100.0;

  // ����������� ����� ������ �� ������ ������������ ���������
  static const double tech_control_speed = 1.0;

  // ����� ���������� ������ �� �������������
  static const double tracking_query_threshold = 1.0;

  // ����� ���������� ������ �� �������������
  static const double confirm_query_threshold = 0.0;

  // ����� ���������� ������ �� ������
  static const double capture_query_threshold = 0.0;

  // ����� ���������� ������ �� ������ ������������ ���������
  static const double tech_control_threshold = 1.0;

  // ������������ ���������� ��������, ��
  static const std::uint32_t time_chunk_length = 10000;

  // �������� ������������, � ��������� (1 ��)
  static const std::uint32_t planning_step = 100;

  // ������� ���������, � ��������� (1 ���)
  static const std::uint32_t timeline_depth = 100000;

  // ������� ������������ ���������� ��������, � ���������� ������������
  static const std::uint32_t pulse_planning_depth = 3;

  // ������� ������������ �����, � ���������� ������������
  static const std::uint32_t pulse_train_planning_depth = 35;
}

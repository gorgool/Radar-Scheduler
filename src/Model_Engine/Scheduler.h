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
  // Конвейер временных дискретов УЦП (Digital Converter Unit)
  Timeline dcu_timeline;

  // Конвейер временных дискретов АУ приемного устройства (Antenna Unit)
  Timeline tr_au_timeline;

  // Конвейер временных дискретов АУ передающего устройства
  Timeline rs_au_timeline;

  // Найти индекс на КВД, подходящий для размещения участка transmit_len (нс) и соответствующего ему receive_len (нс) 
  // со смещением range_offset(нс), на глубину планирования depth.
  // Возвращает -1 в случае если такого индекса не существует и положительное целое, равное индексу, в случае успеха.
  std::int32_t find_index(const std::uint32_t transmit_len, const std::uint32_t receive_len, const std::uint32_t range_offset, const std::uint32_t depth);

  // Разместить временную связку scan_pair на КВД УЦП dcu_timeline и КВД АУ au_timeline в интересах обслуживания заявки query_id.
  // В случае успеха возвращает список команд управления УЦП и АУ. В случае неудачи возвращает комманды типа nop (no operation).
  std::vector<ControlCommand> place_scan(const ScanPair& scan_pair, const std::uint32_t query_id);

  // Удаление заявки с идентификатором query_id
  void remove_query(const std::uint32_t query_id);

public:
  Scheduler() {}

  using QueryListType = std::vector<std::shared_ptr<Query>>;

  // Массив заявок
  QueryListType queries;

  // Массив исполненных в последнем запуске ф-ии run заявок
  QueryListType processed_queries;

  // Выполнение одного такта планирования на момент времени time
  std::vector<ControlCommand> run(std::uint64_t time);

  // Заполняет переданную структуру model_state параметрами активных и исполненных в последнем запуске ф-ии run заявок
  void get_statistics(ModelState& model_state);

  // Сохраняет состояние КВД по переданным ссылкам
  void save_timilines(Timeline& _dcu_timeline, Timeline& _au_tr_timeline, Timeline& _au_rs_timeline);

  // Очистить список заявок с сбросить КВД
  void reset();
};

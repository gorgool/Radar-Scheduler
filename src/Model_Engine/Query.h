#pragma once
#include <cinttypes>


  // Тип заявки
  enum QueryType
  {
    search,         // Поиск
    confirm,        // Дообнаружение
    capture,        // Захват
    tracking,       // Сопровождение
    signal_change,  // Переход на другой тип сигнала
    tech_control,   // Запрос технического состояния
    drop,           // Сброс заявки
  };

  // Заявка
  struct Query
  {
    // Тип заявки
    QueryType type;

    // Идентификатор заявки
    std::uint32_t id;

    // Коэффициент роста приоритета заявки
    double k;

    // Порог исполнения заявки
    double p_threshold;

    // Время предыдущего исполнения заявки, нс
    std::uint64_t t_prev;

    // Значение динамического приоритета
    double p_value;

    // Прогнозируемая дальность, м
    double range;

    // Прогнозируемая ЭПР, кв.м
    double rcs;

    // Признак активности заявки
    bool active;

    Query(const QueryType query_type,
      const uint32_t query_id,
      const double query_k,
      const double query_threshold,
      const std::uint64_t query_time,
      const double estimate_range,
      const double estimate_rcs,
      const bool active_state = false) : 
        type(query_type), id(query_id), k(query_k), p_threshold(query_threshold), 
        t_prev(query_time), p_value(0.0), range(estimate_range), rcs(estimate_rcs),
        active(active_state) {}

  };

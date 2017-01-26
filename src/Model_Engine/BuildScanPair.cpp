#include "BuildScanPair.h"

/*
  Вспомогательная функция определения кода сигнала.
  tau - длительность зондирующей посылки, секунды
  type - тип заявки (вид обслуживания)
*/

static SignalType get_signal_code(const double tau, const QueryType type)
{
  assert(tau > 0.0);
  std::uint64_t tau_nsec = static_cast<std::uint64_t>(tau * 1e9);
  switch (type)
  {
  case QueryType::search:
  case QueryType::capture:
  case QueryType::confirm:
  {
    if (tau_nsec <= 100)
      return SignalType::SOTS15;
    else if (tau_nsec >= 100 && tau_nsec <= 1000)
      return SignalType::SOTS12;
    else if (tau_nsec > 1000 && tau_nsec <= 8000)
      return SignalType::SOTS11;
    else if (tau_nsec > 8000 && tau_nsec <= 15000)
      return SignalType::SOTS10;
    else if (tau_nsec > 15000 && tau_nsec <= 30000)
      return SignalType::SOTS9;
    else if (tau_nsec > 30000 && tau_nsec <= 60000)
      return SignalType::SOTS8;
    else if (tau_nsec > 60000 && tau_nsec <= 120000)
      return SignalType::SOTS7;
    else
      return SignalType::SOTS6;
    break;
  }
  case QueryType::tracking:
  {
    if (tau_nsec <= 100)
      return SignalType::SOTS15;
    else if (tau_nsec >= 100 && tau_nsec <= 1000)
      return SignalType::SOTS14;
    else if (tau_nsec > 1000 && tau_nsec <= 8000)
      return SignalType::SOTS13;
    else if (tau_nsec > 8000 && tau_nsec <= 15000)
      return SignalType::SOTS5;
    else if (tau_nsec > 15000 && tau_nsec <= 30000)
      return SignalType::SOTS4;
    else if (tau_nsec > 30000 && tau_nsec <= 60000)
      return SignalType::SOTS3;
    else if (tau_nsec > 60000 && tau_nsec <= 120000)
      return SignalType::SOTS2;
    else
      return SignalType::SOTS1;
    break;
  }

  // !!! Примечание: вид обслуживания АЧХ, ДП, ДСП, ССС не рассматриваются

  default:
  {
    throw ModelException("ERROR : Unknown query type.");
  }
  }
}

/*
  Таблица параметров кодов сигналов
*/
static std::map<SignalType, SignalParams> signal_tbl
{
  { SOTS1 , SignalParams{ 240000, 20000, 1920000, 2300000, 1, 16, 10007 } },
  { SOTS2 , SignalParams{ 120000, 20000, 960000, 2300000, 1, 16, 10007 } },
  { SOTS3 , SignalParams{ 60000, 20000, 480000, 2300000, 1, 16, 10007 } },
  { SOTS4 , SignalParams{ 30000, 20000, 240000, 2300000, 1, 16, 10007 } },
  { SOTS5 , SignalParams{ 15000, 20000, 120000, 2300000, 1, 16, 10007 } },
  { SOTS6 , SignalParams{ 240000, 250, 1920000, 2300000, 1, 16, 10007 } },
  { SOTS7 , SignalParams{ 120000, 250, 960000, 2300000, 1, 16, 10007 } },
  { SOTS8 , SignalParams{ 60000, 250, 480000, 2300000, 1, 16, 800553 } },
  { SOTS9 , SignalParams{ 30000, 250, 240000, 2300000, 1, 16, 800553 } },
  { SOTS10 , SignalParams{ 15000, 250, 120000, 2300000, 1, 16, 800553 } },
  { SOTS11 , SignalParams{ 8000, 0, 64000, 2300000, 1, 16, 800553 } },
  { SOTS12 , SignalParams{ 1000, 0, 8000, 2300000, 1, 16, 800553 } },
  { SOTS13 , SignalParams{ 8000, 20000, 64000, 2300000, 1, 16, 800553 } },
  { SOTS14 , SignalParams{ 1000, 20000, 8000, 2300000, 1, 16, 200138 } },
  { SOTS15 , SignalParams{ 100, 0, 1000, 2300000, 1, 1, 20013 } },
};

/*
  Вспомогательная функция формирования временной связки по параметрам сигнала.
  signal_params - параметры сигнала, SignalParams
  tua - длительность зондирующей посылки, секунды
  range - прогнозируемая дальность до ОК, м.
  type - тип заявки
*/

static ScanPair create_scan_pair(const SignalParams& signal_params, const double tau, const double range)
{
  assert(tau > 0.0);

  std::uint64_t tau_nsec = static_cast<std::uint64_t>(tau * 1e9);

  // Кол-во импульсов в пачке
  std::uint32_t pulse_train_length = static_cast<std::uint32_t>(std::trunc(tau_nsec / 240000 + 1));

  if (pulse_train_length > signal_params.pulse_number_high || pulse_train_length < signal_params.pulses_number_low)
  {
    // Если количество импульсов в пачке слишком велико
    throw ModelException("ERROR. Pulse train length is too high in create_scan_pair.");
  }

  ScanPair ret(pulse_train_length);

  // Минимальный период излучения
  std::uint32_t T_transmit = signal_params.duration * settings::duty_factor;
  if (T_transmit > signal_params.period_high || T_transmit < signal_params.period_low)
  {
    // Период повторения не поддерживается 
    throw ModelException("ERROR. Pulse train period is not valid in create_scan_pair.");
  }

  // Передающая ДН
  ret.set_transmit(multiples_of(signal_params.duration), multiples_of(T_transmit));
  // Приемная ДН
  ret.set_receive(multiples_of(signal_params.duration), multiples_of(T_transmit));
  // Время смещения приема
  ret.set_receive_time(range);

  return ret;
}

/*
  Выбор типа зондирующего сигнала на основании дальности и ожидаемой ЭПР ОК.
  Примечание: в текущей версии сигналы типа пачки генерируются без наложения приемного и передающего участка
  range - прогнозируемая дальность до ОК, м.
  rcs - прогнозируемая эффективная поверхность рассеивания, кв. м.
  type - тип заявки
*/

ScanPair build_scan_pair(const double range, const double rcs, const QueryType type)
{
  // ========== Расчетные параметры и коэффициенты =============
  const double elevation = 20.0;
  const double elevation_rad = elevation * M_PI / 180.0;
  // Отношение сигнал/шум
  double q = 20.0;
  // !!! Примечание : реальное отношение для сопровождения вычисляется на основании тренда изменения сигнал/шум
  // Мощьность ПУ
  double P = 1.2e5;

  // Эффективная длина трассы водяного пара
  double r_h2o = std::sqrt(std::pow(8500.0 * sin(elevation_rad), 2) + 2.0 * 8500.0 * 2.0) - 8500.0 * sin(elevation_rad);

  // Эффективная длина трассы кислорода
  double r_o = std::sqrt(std::pow(8500.0 * sin(elevation_rad), 2) + 2.0 * 8500.0 * 4.0) - 8500.0 * sin(elevation_rad);

  // Коэффициент потерь в атмосфере
  double n = 1.0 / (std::pow(10.0, 2.0 * (0.07 * r_h2o + 0.045 * r_o) / 10.0));

  // Шумовая температура
  double T_a = 79.0 - 17.0 * elevation / 10.0;
  double T = T_a *0.507 + 441.5;

  // Промежуточный коэффициент 1
  double coef1 = (8.83e-3 * 8.83e-3 * n) / (std::pow(4.0 * M_PI, 3) * 1.38e-23 * T);

  // Промежуточный коэффициент 2
  // !!!Примечание : для упрощения моделирования принято, что направление ГОА и направление зонда всегда совпадают
  double coef2 = std::pow(10, 5.37) * std::pow(10, 6.785) * 1.0 * 1.0;

  // Расчет времени зондирующего сигнала
  double tau = q * q * std::pow(range, 4) / (coef1 * coef2 * rcs * P);

  // Параметры сигнала
  auto signal_params = signal_tbl[get_signal_code(tau, type)];

  return create_scan_pair(signal_params, tau, range);
}

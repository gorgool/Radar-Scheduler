#include "BuildScanPair.h"

/*
  Signal Code translation helper function.
  tau - transmit signal length, seconds
  type - query type
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

  // !!! NOTE:  Only SOTS sevice types are implemented

  default:
  {
    throw ModelException("ERROR : Unknown query type.");
  }
  }
}

/*
  Signal's code parameters table
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
  Scanning pair (receive and transmit) creation hepler function.
  signal_params - signal params
  tua - transmit time estimation, seconds
  range - range estimation, m.
*/

static ScanPair create_scan_pair(const SignalParams& signal_params, const double tau, const double range)
{
  assert(tau > 0.0);

  std::uint64_t tau_nsec = static_cast<std::uint64_t>(tau * 1e9);

  // Number of pulses in pulse train
  std::uint32_t pulse_train_length = static_cast<std::uint32_t>(std::trunc(tau_nsec / 240000 + 1));

  if (pulse_train_length > signal_params.pulse_number_high || pulse_train_length < signal_params.pulses_number_low)
  {
    throw ModelException("ERROR. Pulse train length is too high in create_scan_pair.");
  }

  ScanPair ret(pulse_train_length);

  // Minimum stransmit time period
  std::uint32_t T_transmit = signal_params.duration * settings::duty_factor;
  if (T_transmit > signal_params.period_high || T_transmit < signal_params.period_low)
  { 
    throw ModelException("ERROR. Pulse train period is not valid in create_scan_pair.");
  }

  ret.set_transmit(multiples_of(signal_params.duration), multiples_of(T_transmit));
  ret.set_receive(multiples_of(signal_params.duration), multiples_of(T_transmit));
  ret.set_receive_time(range);

  return ret;
}

/*
  Scan pair creation function depending of range and RCS
  range - range estimation, m.
  rcs - radar cross-section, sq. m.
  type - query type
*/

ScanPair build_scan_pair(const double range, const double rcs, const QueryType type)
{
  // ======= Transmit time estimation =======
  const double elevation = 20.0;
  const double elevation_rad = elevation * M_PI / 180.0;
  // SN ration of detection
  double q = 20.0;
  // Transmit Power
  double P = 1.2e5;

  // Water vapor path length
  double r_h2o = std::sqrt(std::pow(8500.0 * sin(elevation_rad), 2) + 2.0 * 8500.0 * 2.0) - 8500.0 * sin(elevation_rad);

  // Oxygen path length
  double r_o = std::sqrt(std::pow(8500.0 * sin(elevation_rad), 2) + 2.0 * 8500.0 * 4.0) - 8500.0 * sin(elevation_rad);

  // Atmosphere attenuation
  double n = 1.0 / (std::pow(10.0, 2.0 * (0.07 * r_h2o + 0.045 * r_o) / 10.0));

  // Temperature noise
  double T_a = 79.0 - 17.0 * elevation / 10.0;
  double T = T_a *0.507 + 441.5;

  // Helper coefficient 1
  double coef1 = (8.83e-3 * 8.83e-3 * n) / (std::pow(4.0 * M_PI, 3) * 1.38e-23 * T);

  // Helper coefficient 2
  // !!!NOTE : Assume that pulse direction and normal from antenna center are always the same
  double coef2 = std::pow(10, 5.37) * std::pow(10, 6.785) * 1.0 * 1.0;

  // Time estimation
  double tau = q * q * std::pow(range, 4) / (coef1 * coef2 * rcs * P);

  // Choose signal
  auto signal_params = signal_tbl[get_signal_code(tau, type)];

  return create_scan_pair(signal_params, tau, range);
}

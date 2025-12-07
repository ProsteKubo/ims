#pragma once

#include "../config/config_reader.hpp"

#include <iostream>

struct ModelParameters {
    double ka{};
    double Vd{};
    double Vp{};
    double kcp{};
    double kpc{};
    double Vmax{};
    double Km{};
    double keo{};
    double tau_e{};
    double Emax{};
    double EC50_base{};
    double n_Hill{};
    double kin{};
    double kout{};
    double EC50_signal{};
    double C_toxic{};
    double C_critical{};
    double Effect_resp_critical{};
    double current_dose{};
    double dosing_interval{};
    double sim_duration{};
    double sim_step_min{};
    double sim_step_max{};
    double sim_accuracy{};
    double output_interval{};
};

ModelParameters LoadModelParameters(const ConfigReader& config);
void PrintModelParameters(const ModelParameters& params);

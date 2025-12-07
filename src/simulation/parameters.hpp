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
    
    // Behavioral parameters (Petri net / discrete subsystem)
    bool petri_net_enabled{};
    double assessment_interval{};
    double relief_threshold{};
    double effect_relief_threshold{};
    double motivation_threshold{};
    double motivation_pain_rate{};
    double motivation_dose_reduction{};
    double motivation_decay_rate{};
    double min_dosing_interval{};
    double base_escalation_factor{};
    double tolerance_escalation_factor{};
    
    // Naloxone rescue parameters
    bool naloxone_available{};
    double naloxone_effective_window{};
    double naloxone_blockade_strength{};
};

ModelParameters LoadModelParameters(const ConfigReader& config);
void PrintModelParameters(const ModelParameters& params);

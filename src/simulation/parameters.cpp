#include "parameters.hpp"

#include <iostream>

using std::cout;
using std::endl;

ModelParameters LoadModelParameters(const ConfigReader& config) {
    ModelParameters params{};
    params.ka = config.get("ka", 2.0);
    params.Vd = config.get("Vd", 28.0);
    params.Vp = config.get("Vp", 105.0);
    params.kcp = config.get("kcp", 0.3);
    params.kpc = config.get("kpc", 0.4);
    params.Vmax = config.get("Vmax", 10.0);
    params.Km = config.get("Km", 2.0);
    params.keo = config.get("keo", 0.5);
    params.tau_e = config.get("tau_e", 2.0);
    params.Emax = config.get("Emax", 95.0);
    params.EC50_base = config.get("EC50_base", 3.0);
    params.n_Hill = config.get("n_Hill", 1.2);
    params.kin = config.get("kin", 0.10);
    params.kout = config.get("kout", 0.005);
    params.EC50_signal = config.get("EC50_signal", 2.0);
    params.C_toxic = config.get("C_toxic", 15.0);
    params.C_critical = config.get("C_critical", 50.0);
    params.Effect_resp_critical = config.get("Effect_resp_critical", 90.0);
    params.current_dose = config.get("initial_dose", 10.0);
    params.dosing_interval = config.get("dosing_interval", 12.0);
    params.sim_duration = config.get("duration", 720.0);
    params.sim_step_min = config.get("step_min", 0.01);
    params.sim_step_max = config.get("step_max", 0.1);
    params.sim_accuracy = config.get("accuracy", 1e-6);
    params.output_interval = config.get("output_interval", 1.0);
    
    // Behavioral parameters
    params.petri_net_enabled = config.get("petri_net_enabled", true);
    params.assessment_interval = config.get("assessment_interval", 12.0);
    params.relief_threshold = config.get("relief_threshold", 0.60);
    params.effect_relief_threshold = config.get("effect_relief_threshold", 60.0);
    params.motivation_threshold = config.get("motivation_threshold", 1.5);
    params.motivation_pain_rate = config.get("motivation_pain_rate", 0.1);
    params.motivation_dose_reduction = config.get("motivation_dose_reduction", 2.0);
    params.motivation_decay_rate = config.get("motivation_decay_rate", 0.05);
    params.min_dosing_interval = config.get("min_dosing_interval", 6.0);
    params.base_escalation_factor = config.get("base_escalation_factor", 0.10);
    params.tolerance_escalation_factor = config.get("tolerance_escalation_factor", 0.15);
    
    // Naloxone rescue parameters
    params.naloxone_available = config.get("naloxone_available", false);
    params.naloxone_effective_window = config.get("naloxone_effective_window", 5.0);
    params.naloxone_blockade_strength = config.get("naloxone_blockade_strength", 0.4);
    params.naloxone_response_delay = config.get("naloxone_response_delay", 0.083);  // 5 min default
    
    return params;
}

void PrintModelParameters(const ModelParameters& params) {
    cout << "Model Parameters:" << endl;
    cout << "  Absorption: ka = " << params.ka << " /h" << endl;
    cout << "  Distribution: Vd = " << params.Vd << " L, Vp = " << params.Vp << " L" << endl;
    cout << "  Transfer: kcp = " << params.kcp << " /h, kpc = " << params.kpc << " /h" << endl;
    cout << "  Elimination (M-M): Vmax = " << params.Vmax << " mg/h, Km = " << params.Km << " mg/L" << endl;
    cout << "  Effect-site: keo = " << params.keo << " /h, tau_e = " << params.tau_e << " h" << endl;
    cout << "  PD: Emax = " << params.Emax << "%, EC50 = " << params.EC50_base << " mg/L, n = " << params.n_Hill << endl;
    cout << "  Tolerance: kin = " << params.kin << " /h, kout = " << params.kout << " /h" << endl;
    cout << "  Dosing: " << params.current_dose << " mg every " << params.dosing_interval << " hours" << endl;
    cout << "  Toxicity: C_toxic = " << params.C_toxic << " mg/L, C_critical = " << params.C_critical << " mg/L" << endl;
    cout << "  Behavioral: assessment every " << params.assessment_interval << " h, relief threshold = " << params.effect_relief_threshold << "%" << endl;
    cout << "  Escalation: base = " << (params.base_escalation_factor * 100) << "%, tolerance factor = " << (params.tolerance_escalation_factor * 100) << "%" << endl;
    cout << "  Naloxone: " << (params.naloxone_available ? "AVAILABLE" : "NOT AVAILABLE") 
         << " (response delay: " << (params.naloxone_response_delay * 60) << " min, "
         << "window: " << (params.naloxone_effective_window * 60) << " min, blockade: " 
         << (params.naloxone_blockade_strength * 100) << "%)" << endl;
    cout << "  Simulation: " << params.sim_duration << " hours, output every " << params.output_interval << " hours" << endl;
    cout << endl;
}

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
    cout << "  Simulation: " << params.sim_duration << " hours, output every " << params.output_interval << " hours" << endl;
    cout << endl;
}

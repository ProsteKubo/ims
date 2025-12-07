
#include <simlib.h>

#include <iostream>
#include <string>

#include "config/config_reader.hpp"
#include "simulation/behavior.hpp"
#include "simulation/dynamics.hpp"
#include "simulation/monitoring.hpp"
#include "simulation/parameters.hpp"

int main(int argc, char* argv[]) {
    std::string config_file = "config.ini";
    if (argc > 1) {
        config_file = argv[1];
    }

    std::cout << "========================================================================" << std::endl;
    std::cout << "  THE DEADLY SPIRAL: Continuous PK/PD Simulation" << std::endl;
    std::cout << "  Pharmacokinetic-Pharmacodynamic Model with Metabolic Saturation" << std::endl;
    std::cout << "========================================================================" << std::endl;
    std::cout << std::endl;

    ConfigReader config;
    std::cout << "Loading configuration from: " << config_file << std::endl;
    if (!config.load(config_file)) {
        std::cerr << "Failed to load configuration. Exiting." << std::endl;
        return 1;
    }
    std::cout << std::endl;

    ModelParameters params = LoadModelParameters(config);
    PrintModelParameters(params);

    Init(0, params.sim_duration);
    SetStep(params.sim_step_min, params.sim_step_max);
    SetAccuracy(params.sim_accuracy);

    SimulationState state{};

    AbsorptionDynamics dA_dt(params, state);
    CentralDynamics dC_dt(params, state);
    PeripheralDynamics dP_dt(params, state);
    EffectSiteDynamics dCe_dt(params, state);
    ToleranceDynamics dTol_dt(params, state);

    Integrator A(dA_dt, params.current_dose);  // Start with initial dose in stomach
    Integrator C(dC_dt, 0.0);
    Integrator P(dP_dt, 0.0);
    Integrator Ce(dCe_dt, 0.0);
    Integrator Tol(dTol_dt, 0.0);

    state.A = &A;
    state.C = &C;
    state.P = &P;
    state.Ce = &Ce;
    state.Tol = &Tol;

    PetriNetState petri_state{};
    petri_state.pain_level = 2;  // Start with moderate pain
    petri_state.motivation = 1.0;
    petri_state.relief_state = false;
    petri_state.current_dose = params.current_dose;  // Initialize from config

    std::cout << "Initial Conditions:" << std::endl;
    std::cout << "  A(0) = " << A.Value() << " mg (first dose)" << std::endl;
    std::cout << "  C(0) = " << C.Value() << " mg/L" << std::endl;
    std::cout << "  P(0) = " << P.Value() << " mg/L" << std::endl;
    std::cout << "  Ce(0) = " << Ce.Value() << " mg/L" << std::endl;
    std::cout << "  Tol(0) = " << Tol.Value() << std::endl;
    std::cout << std::endl;
    std::cout << "========================================================================" << std::endl;
    std::cout << "                        SIMULATION OUTPUT" << std::endl;
    std::cout << "========================================================================" << std::endl;
    std::cout << std::endl;

    (new StatusMonitor(params, state, petri_state))->Activate(Time + params.output_interval);
    (new PatientAssessment(params, state, petri_state))->Activate(Time + params.assessment_interval);

    Run();

    std::cout << std::endl;
    std::cout << "========================================================================" << std::endl;
    std::cout << "                        SIMULATION SUMMARY" << std::endl;
    std::cout << "========================================================================" << std::endl;
    std::cout << std::endl;
    std::cout << "Final State (t=" << Time << " hours):" << std::endl;
    std::cout << "  A(t) = " << A.Value() << " mg" << std::endl;
    std::cout << "  C(t) = " << C.Value() << " mg/L" << std::endl;
    std::cout << "  P(t) = " << P.Value() << " mg/L" << std::endl;
    std::cout << "  Ce(t) = " << Ce.Value() << " mg/L" << std::endl;
    std::cout << "  Tol(t) = " << Tol.Value() << std::endl;
    std::cout << "  Effect = " << CalculateEffect(Ce.Value(), Tol.Value(), params) << "%" << std::endl;
    std::cout << std::endl;

    double saturation_ratio = C.Value() / params.Km;
    std::cout << "Pharmacokinetic Analysis:" << std::endl;
    std::cout << "  Saturation ratio (C/Km) = " << saturation_ratio << std::endl;
    if (saturation_ratio < 0.5) {
        std::cout << "  Status: LINEAR REGIME - First-order elimination dominates" << std::endl;
    } else if (saturation_ratio < 3.0) {
        std::cout << "  Status: SATURATION ZONE - Nonlinear kinetics active" << std::endl;
        std::cout << "  WARNING: Approaching dangerous territory!" << std::endl;
    } else {
        std::cout << "  Status: PLATEAU REGIME - Zero-order elimination (capacity exhausted)" << std::endl;
        std::cout << "  CRITICAL: System in deadly spiral zone!" << std::endl;
    }
    std::cout << std::endl;

    double EC50_current = params.EC50_base * (1.0 + Tol.Value());
    double tolerance_factor = EC50_current / params.EC50_base;
    std::cout << "Pharmacodynamic Analysis:" << std::endl;
    std::cout << "  Current EC50 = " << EC50_current << " mg/L (baseline: " << params.EC50_base << " mg/L)" << std::endl;
    std::cout << "  Tolerance multiplier = " << tolerance_factor << "x" << std::endl;
    std::cout << "  Required dose for same effect = " << tolerance_factor * params.current_dose << " mg" << std::endl;
    std::cout << std::endl;

    std::cout << "Behavioral Analysis (Petri Net):" << std::endl;
    std::cout << "  Patient Status: " << (petri_state.patient_alive ? "ALIVE" : "DECEASED") << std::endl;
    std::cout << "  Final Pain Level: " << petri_state.pain_level << " (0=None, 1=Mild, 2=Moderate, 3=Severe)" << std::endl;
    std::cout << "  Total Dose Escalations: " << petri_state.dose_history.size() << std::endl;
    if (!petri_state.dose_history.empty()) {
        auto first_dose = petri_state.dose_history.front().dose;
        auto last_dose = petri_state.dose_history.back().dose;
        std::cout << "  Dose Escalation: " << first_dose << " mg → " << last_dose << " mg (" 
                  << ((last_dose / first_dose - 1.0) * 100) << "% increase)" << std::endl;
    }
    std::cout << std::endl;

    std::cout << "========================================================================" << std::endl;

    return 0;
}

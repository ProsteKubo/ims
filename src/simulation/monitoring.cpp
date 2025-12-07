#include "monitoring.hpp"

#include <iomanip>
#include <iostream>

using std::cout;
using std::endl;
using std::fixed;
using std::setprecision;
using std::setw;

bool CheckToxicity(const SimulationState& state, const ModelParameters& params) {
    double effect = CalculateEffect(state.Ce->Value(), state.Tol->Value(), params);

    if (state.C->Value() > params.C_critical) {
        cout << "\n!!! CRITICAL OVERDOSE at t=" << Time << " hours !!!" << endl;
        cout << "    C(t) = " << state.C->Value() << " mg/L (critical threshold: "
             << params.C_critical << " mg/L)" << endl;
        return true;
    }

    if (effect > params.Effect_resp_critical) {
        cout << "\n!!! RESPIRATORY ARREST at t=" << Time << " hours !!!" << endl;
        cout << "    Respiratory depression = " << effect << "%" << endl;
        return true;
    }

    if (state.C->Value() > params.C_toxic) {
        cout << "\n>>> WARNING: Toxic concentration reached at t=" << Time
             << " hours <<<" << endl;
        cout << "    C(t) = " << state.C->Value() << " mg/L" << endl;
    }

    return false;
}

void StatusMonitor::Behavior() {
    double effect = CalculateEffect(state_.Ce->Value(), state_.Tol->Value(), params_);

    cout << fixed << setprecision(2);
    cout << "t=" << setw(6) << Time << "h | "
         << "A=" << setw(6) << state_.A->Value() << " mg | "
         << "C=" << setw(6) << state_.C->Value() << " mg/L | "
         << "P=" << setw(6) << state_.P->Value() << " mg/L | "
         << "Ce=" << setw(6) << state_.Ce->Value() << " mg/L | "
         << "Tol=" << setw(5) << state_.Tol->Value() << " | "
         << "Effect=" << setw(5) << effect << "%" << endl;

    if (CheckToxicity(state_, params_)) {
        petri_state_.patient_alive = false;
        petri_state_.time_overdose_detected = Time;
        Stop();
        return;
    }

    Activate(Time + params_.output_interval);
}

void DosingEvent::Behavior() {
    cout << "\n>>> DOSE ADMINISTERED at t=" << Time << " hours: "
         << params_.current_dose << " mg <<<" << endl;

    *state_.A = state_.A->Value() + params_.current_dose;

    Activate(Time + params_.dosing_interval);
}

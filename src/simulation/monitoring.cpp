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
        
        // Attempt naloxone rescue if available
        if (params_.naloxone_available) {
            double response_time = params_.naloxone_response_delay;
            cout << "\n>>> EMERGENCY RESPONSE DISPATCHED (ETA: " 
                 << (response_time * 60) << " minutes) <<<" << endl;
            
            // Schedule naloxone administration after response delay
            NaloxoneRescue* rescue_event = new NaloxoneRescue(params_, state_, petri_state_);
            rescue_event->Activate(Time + response_time);
            
            // Continue monitoring to see if rescue arrives in time
            Activate(Time + params_.output_interval);
            return;
        }
        
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

void NaloxoneRescue::Behavior() {
    double time_since_OD = Time - petri_state_.time_overdose_detected;
    
    cout << "\n>>> NALOXONE RESCUE TEAM ARRIVED at t=" << Time << " hours <<<" << endl;
    cout << "Time since overdose: " << fixed << setprecision(2) 
         << (time_since_OD * 60) << " minutes" << endl;
    
    // Check if still within effective window
    if (time_since_OD > params_.naloxone_effective_window) {
        cout << "\n!!! NALOXONE WINDOW EXPIRED (>" 
             << (params_.naloxone_effective_window * 60)
             << " min) - RESCUE FAILED !!!" << endl;
        cout << "Patient Status: DECEASED" << endl;
        cout << "Cause: Response time (" << (time_since_OD * 60) 
             << " min) exceeded therapeutic window" << endl;
        Stop();
        return;
    }
    
    // Apply naloxone - create PatientAssessment to use existing logic
    PatientAssessment* assessment = new PatientAssessment(params_, state_, petri_state_);
    assessment->CheckAndApplyNaloxonePublic();
    delete assessment;
    
    if (petri_state_.patient_alive) {
        cout << "\n>>> RESCUE SUCCESSFUL - Patient REVIVED <<<" << endl;
    } else {
        cout << "\n!!! RESCUE FAILED - Patient DECEASED !!!" << endl;
        Stop();
    }
}

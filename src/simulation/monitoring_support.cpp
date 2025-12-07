#include "monitoring_support.hpp"

#include <simlib.h>

#include <iomanip>
#include <iostream>

using std::cout;
using std::endl;

void MonitorSaturation(const ModelParameters& params, SimulationState& cont_state) {
    double C_val = cont_state.C->Value();
    double saturation_ratio = C_val / params.Km;
    
    static bool phase2_flagged = false;
    static bool phase3_flagged = false;
    
    if (!phase2_flagged && saturation_ratio > 1.0) {
        phase2_flagged = true;
        cout << "\n╔═══════════════════════════════════════════════════════════╗" << endl;
        cout << "║ PHASE TRANSITION: SATURATION ZONE ENTERED (Phase 2)      ║" << endl;
        cout << "║ Time: " << Time << " hours  |  C/Km ratio: " << saturation_ratio << endl;
        cout << "║ Concentration: " << C_val << " mg/L                              ║" << endl;
        cout << "║ Status: NONLINEAR KINETICS ACTIVE                        ║" << endl;
        cout << "╚═══════════════════════════════════════════════════════════╝\n" << endl;
    }
    
    if (!phase3_flagged && saturation_ratio > 3.0) {
        phase3_flagged = true;
        cout << "\n╔═══════════════════════════════════════════════════════════╗" << endl;
        cout << "║ PHASE TRANSITION: CATASTROPHIC ZONE (Phase 3)            ║" << endl;
        cout << "║ Time: " << Time << " hours  |  C/Km ratio: " << saturation_ratio << endl;
        cout << "║ PATIENT IN CRITICAL DANGER                               ║" << endl;
        cout << "╚═══════════════════════════════════════════════════════════╝\n" << endl;
    }
}

void CheckAndApplyNaloxone(const ModelParameters& params, SimulationState& cont_state, PetriNetState& petri_state) {
    if (petri_state.patient_alive) {
        return;
    }
    
    if (!params.naloxone_available) {
        cout << "\n!!! Naloxone NOT AVAILABLE - Patient cannot be rescued !!!" << endl;
        return;
    }
    
    cout << "\n╔═══════════════════════════════════════════════════════════╗" << endl;
    cout << "║ T6: NALOXONE RESCUE ACTIVATED                            ║" << endl;
    cout << "╚═══════════════════════════════════════════════════════════╝" << endl;
    
    double C_before = cont_state.C->Value();
    double Ce_before = cont_state.Ce->Value();
    double Tol_before = cont_state.Tol->Value();
    
    double blockade_factor = params.naloxone_blockade_strength;
    *cont_state.C = C_before * (1.0 - blockade_factor);
    
    *cont_state.Ce = Ce_before * 0.1;
    
    *cont_state.Tol = Tol_before * 0.7;
    
    cout << "C(t): " << C_before << " → " << cont_state.C->Value() << " mg/L" << endl;
    cout << "Ce(t): " << Ce_before << " → " << cont_state.Ce->Value() << " mg/L" << endl;
    cout << "Tol(t): " << Tol_before << " → " << cont_state.Tol->Value() << endl;
    
    petri_state.patient_alive = true;
    
    petri_state.pain_level = 3;
    petri_state.relief_state = false;
    petri_state.motivation = 3.0;
    
    cout << "\nPatient REVIVED but experiencing ACUTE WITHDRAWAL" << endl;
    cout << "Status: ALIVE but in severe distress" << endl;
    cout << "Requires: ICU monitoring, serial naloxone dosing" << endl;
    cout << "Risk: Re-overdose in 1-2 hours if opioid still circulating" << endl;
}

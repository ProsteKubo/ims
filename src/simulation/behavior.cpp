#include "behavior.hpp"

#include <iomanip>
#include <iostream>

#include "decision_logic.hpp"
#include "dose_management.hpp"
#include "monitoring.hpp"
#include "monitoring_support.hpp"
#include "pain_assessment.hpp"

using std::cout;
using std::endl;
using std::fixed;
using std::setprecision;
using std::setw;

void PatientAssessment::Behavior() {
    if (!petri_state_.patient_alive) {
        return;
    }
    
    double Ce_val = cont_state_.Ce->Value();
    double Tol_val = cont_state_.Tol->Value();
    double effect = CalculateEffect(Ce_val, Tol_val, params_);
    
    UpdatePainLevel(effect, petri_state_);
    UpdateMotivation(params_.assessment_interval, params_, petri_state_);
    
    MonitorSaturation(params_, cont_state_);

    cout << "\n========== PATIENT ASSESSMENT at t=" << Time << " hours ==========" << endl;
    cout << "Current Effect: " << fixed << setprecision(2) << effect << "%" << endl;
    cout << "Pain Level: " << petri_state_.pain_level
         << " (0=None, 1=Mild, 2=Moderate, 3=Severe)" << endl;
    cout << "Relief State: " << (petri_state_.relief_state ? "YES" : "NO") << endl;
    cout << "Motivation: " << setprecision(2) << petri_state_.motivation << endl;
    cout << "Current Dose: " << petri_state_.current_dose << " mg" << endl;
    
    if (CheckToxicity(cont_state_, params_)) {
        petri_state_.patient_alive = false;
        petri_state_.time_overdose_detected = Time;
        cout << "\n!!! SIMULATION TERMINATED - PATIENT DECEASED !!!" << endl;
        Stop();
        return;
    }
    
    if (params_.petri_net_enabled) {
        if (ShouldIncreaseDose(effect, params_, petri_state_)) {
            ExecuteDoseIncrease(params_, cont_state_, petri_state_);
        } else if (petri_state_.relief_state && effect >= params_.effect_relief_threshold) {
            MaintainDose(params_, cont_state_, petri_state_);
        } else {
            cout << "Decision: STABLE - No dose adjustment needed" << endl;
        }
    }
    
    cout << "================================================" << endl;
    petri_state_.time_since_last_dose += params_.assessment_interval;
    
    Activate(Time + params_.assessment_interval);
}

void PatientAssessment::CheckAndApplyNaloxonePublic() {
    CheckAndApplyNaloxone(params_, cont_state_, petri_state_);
}

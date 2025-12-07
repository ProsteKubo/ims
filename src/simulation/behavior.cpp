#include "behavior.hpp"

#include <cmath>
#include <iomanip>
#include <iostream>

#include "monitoring.hpp"

using std::cout;
using std::endl;

void PatientAssessment::Behavior() {
    if (!petri_state_.patient_alive) {
        return;  // Terminal state reached
    }

    // Calculate current pharmacodynamic effect
    double Ce_val = cont_state_.Ce->Value();
    double Tol_val = cont_state_.Tol->Value();
    double effect = CalculateEffect(Ce_val, Tol_val, params_);

    // Update Petri net state based on current effect
    UpdatePainLevel(effect);
    UpdateMotivation(params_.assessment_interval);

    cout << "\n========== PATIENT ASSESSMENT at t=" << Time << " hours ==========" << endl;
    cout << "Current Effect: " << effect << "%" << endl;
    cout << "Pain Level: " << petri_state_.pain_level
         << " (0=None, 1=Mild, 2=Moderate, 3=Severe)" << endl;
    cout << "Relief State: " << (petri_state_.relief_state ? "YES" : "NO") << endl;
    cout << "Motivation: " << petri_state_.motivation << endl;
    cout << "Current Dose: " << params_.current_dose << " mg" << endl;

    // Check for overdose (Transition T5)
    if (CheckToxicity(cont_state_, params_)) {
        petri_state_.patient_alive = false;
        cout << "\n!!! SIMULATION TERMINATED - PATIENT DECEASED !!!" << endl;
        Stop();
        return;
    }

    // Decision logic (Petri net transitions)
    if (ShouldIncreaseDose(effect)) {
        // Transition T2: INCREASE_DOSE
        ExecuteDoseIncrease();
    } else if (petri_state_.relief_state && effect >= params_.effect_relief_threshold) {
        // Transition T3: MAINTAIN_DOSE
        MaintainDose();
    } else {
        // Transition T4: ASSESS_STABLE (no intervention)
        cout << "Decision: STABLE - No dose adjustment needed" << endl;
    }

    cout << "================================================" << endl;

    petri_state_.time_since_last_dose += params_.assessment_interval;

    // Schedule next assessment
    Activate(Time + params_.assessment_interval);
}

void PatientAssessment::UpdatePainLevel(double effect) {
    // Update pain level based on effect (continuous → discrete mapping)
    if (effect > 80.0) {
        petri_state_.pain_level = 0;  // No pain
        petri_state_.relief_state = true;
    } else if (effect > 60.0) {
        petri_state_.pain_level = 1;  // Mild pain
        petri_state_.relief_state = true;
    } else if (effect > 40.0) {
        petri_state_.pain_level = 2;  // Moderate pain
        petri_state_.relief_state = false;
    } else {
        petri_state_.pain_level = 3;  // Severe pain
        petri_state_.relief_state = false;
    }
}

void PatientAssessment::UpdateMotivation(double dt) {
    // Motivation accumulates based on pain level
    // λ_pain * Pain_Level - λ_dose * δ(dose_event)
    double lambda_pain = params_.motivation_pain_rate;
    petri_state_.motivation += lambda_pain * petri_state_.pain_level * dt;

    // Natural decay if in relief
    if (petri_state_.relief_state) {
        petri_state_.motivation *= 0.95;  // 5% decay per assessment
    }

    // Cap motivation
    if (petri_state_.motivation > 5.0) {
        petri_state_.motivation = 5.0;
    }
}

bool PatientAssessment::ShouldIncreaseDose(double effect) {
    // Preconditions for Transition T2 (INCREASE_DOSE):
    // 1. Pain level >= 2 (Moderate or Severe)
    // 2. NOT in relief state
    // 3. Motivation above threshold
    // 4. Minimum time since last dose elapsed
    // 5. Effect below relief threshold

    bool pain_sufficient = petri_state_.pain_level >= 2;
    bool no_relief = !petri_state_.relief_state;
    bool motivated = petri_state_.motivation > params_.motivation_threshold;
    bool time_elapsed = petri_state_.time_since_last_dose >= params_.min_dosing_interval;
    bool effect_insufficient = effect < params_.effect_relief_threshold;

    return pain_sufficient && no_relief && motivated && time_elapsed && effect_insufficient;
}

void PatientAssessment::ExecuteDoseIncrease() {
    cout << "\n>>> DECISION: INCREASE DOSE (Transition T2) <<<" << endl;

    // Calculate escalation factor based on tolerance
    // f_escalation = 0.10 + 0.15 * Tol(t)
    double Tol_val = cont_state_.Tol->Value();
    double escalation_factor = params_.base_escalation_factor +
                               params_.tolerance_escalation_factor * Tol_val;

    // Calculate new dose
    double old_dose = params_.current_dose;
    double new_dose = old_dose * (1.0 + escalation_factor);

    cout << "Tolerance Level: " << Tol_val << endl;
    cout << "Escalation Factor: " << (escalation_factor * 100) << "%" << endl;
    cout << "Old Dose: " << old_dose << " mg" << endl;
    cout << "New Dose: " << new_dose << " mg" << endl;
    cout << "Dose Increase: +" << (new_dose - old_dose) << " mg (+"
         << ((new_dose / old_dose - 1.0) * 100) << "%)" << endl;

    // Apply dose to absorption compartment
    *cont_state_.A = cont_state_.A->Value() + new_dose;

    // Update state
    const_cast<ModelParameters&>(params_).current_dose = new_dose;
    petri_state_.motivation -= params_.motivation_dose_reduction;
    petri_state_.time_since_last_dose = 0.0;
    petri_state_.relief_state = true;  // Patient expects relief

    // Record dose event
    RecordDoseEvent(new_dose);
}

void PatientAssessment::MaintainDose() {
    cout << "\n>>> DECISION: MAINTAIN CURRENT DOSE (Transition T3) <<<" << endl;
    cout << "Current dose: " << params_.current_dose << " mg" << endl;

    // Apply current dose
    *cont_state_.A = cont_state_.A->Value() + params_.current_dose;

    // Update state
    petri_state_.time_since_last_dose = 0.0;

    // Record dose event
    RecordDoseEvent(params_.current_dose);
}

void PatientAssessment::RecordDoseEvent(double dose) {
    PetriNetState::DoseRecord record;
    record.time = Time;
    record.dose = dose;
    record.C = cont_state_.C->Value();
    record.Ce = cont_state_.Ce->Value();
    record.Tol = cont_state_.Tol->Value();
    record.effect = CalculateEffect(cont_state_.Ce->Value(), cont_state_.Tol->Value(), params_);

    petri_state_.dose_history.push_back(record);

    cout << "\n--- DOSE ADMINISTERED ---" << endl;
    cout << "Time: " << record.time << " h" << endl;
    cout << "Dose: " << record.dose << " mg" << endl;
    cout << "C(t): " << record.C << " mg/L" << endl;
    cout << "Ce(t): " << record.Ce << " mg/L" << endl;
    cout << "Tol(t): " << record.Tol << endl;
    cout << "Effect: " << record.effect << "%" << endl;
    cout << "Total doses given: " << petri_state_.dose_history.size() << endl;
}

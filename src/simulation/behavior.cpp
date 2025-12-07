#include "behavior.hpp"

#include <cmath>
#include <iomanip>
#include <iostream>

#include "monitoring.hpp"

using std::cout;
using std::endl;
using std::fixed;
using std::setprecision;
using std::setw;

void PatientAssessment::Behavior() {
    if (!petri_state_.patient_alive) {
        return; // Terminal state reached
    }
    
    // Calculate current pharmacodynamic effect
    double Ce_val = cont_state_.Ce->Value();
    double Tol_val = cont_state_.Tol->Value();
    double effect = CalculateEffect(Ce_val, Tol_val, params_);
    
    // Update Petri net state based on current effect
    UpdatePainLevel(effect);
    UpdateMotivation(params_.assessment_interval);
    
    cout << "\n========== PATIENT ASSESSMENT at t=" << Time << " hours ==========" << endl;
    cout << "Current Effect: " << fixed << setprecision(2) << effect << "%" << endl;
    cout << "Pain Level: " << petri_state_.pain_level
         << " (0=None, 1=Mild, 2=Moderate, 3=Severe)" << endl;
    cout << "Relief State: " << (petri_state_.relief_state ? "YES" : "NO") << endl;
    cout << "Motivation: " << setprecision(2) << petri_state_.motivation << endl;
    cout << "Current Dose: " << petri_state_.current_dose << " mg" << endl;
    
    // Check for overdose (Transition T5 - OVERDOSE_DETECTED)
    if (CheckToxicity(cont_state_, params_)) {
        petri_state_.patient_alive = false;
        petri_state_.time_overdose_detected = Time;
        cout << "\n!!! SIMULATION TERMINATED - PATIENT DECEASED !!!" << endl;
        Stop();
        return;
    }
    
    // Decision logic (Petri net transitions)
    if (params_.petri_net_enabled) {
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
    }
    
    cout << "================================================" << endl;
    petri_state_.time_since_last_dose += params_.assessment_interval;
    
    // Schedule next assessment
    Activate(Time + params_.assessment_interval);
}

// ============================================================================
// FIX #1: UpdatePainLevel() - Corrected thresholds
// ============================================================================

void PatientAssessment::UpdatePainLevel(double effect) {
    // With corrected PK params (Vd=14, Vmax=4, initial_dose variable),
    // Effect should be in realistic range [10-90%]
    
    if (effect > 80.0) {
        // Effect > 80%: excellent pain control
        petri_state_.pain_level = 0;  // No pain
        petri_state_.relief_state = true;
    } else if (effect > 60.0) {
        // Effect 60-80%: mild residual pain (acceptable)
        petri_state_.pain_level = 1;  // Mild pain
        petri_state_.relief_state = true;
    } else if (effect > 40.0) {
        // Effect 40-60%: moderate pain (distressing)
        petri_state_.pain_level = 2;  // Moderate pain
        petri_state_.relief_state = false;
    } else {
        // Effect < 40%: severe pain (intolerable)
        petri_state_.pain_level = 3;  // Severe pain
        petri_state_.relief_state = false;
    }
    
    // Debug output
    cout << "  [Pain Update] Effect=" << fixed << setprecision(1) << effect 
         << "% → PainLevel=" << petri_state_.pain_level 
         << " Relief=" << (petri_state_.relief_state ? "YES" : "NO") << endl;
}

// ============================================================================
// FIX #2: UpdateMotivation() - Dynamic cap, natural decay
// ============================================================================

void PatientAssessment::UpdateMotivation(double dt) {
    // Motivation accumulates based on pain level
    // Higher pain = more urgency to dose
    
    // Pain-driven accumulation
    double pain_severity = static_cast<double>(petri_state_.pain_level) / 3.0;
    double pain_contribution = params_.motivation_pain_rate * pain_severity * dt;
    
    petri_state_.motivation += pain_contribution;
    
    // Natural decay when patient feels relief
    // (simulates decreased urgency after successful dosing)
    if (petri_state_.relief_state && petri_state_.pain_level <= 1) {
        double decay_factor = 1.0 - params_.motivation_decay_rate;
        petri_state_.motivation *= decay_factor;
    }
    
    // Dynamic cap: motivation can increase with severe pain
    // Base cap = 5.0, additional 2.0 for each pain level (0-3)
    double pain_weight = static_cast<double>(petri_state_.pain_level) / 3.0;
    double max_motivation = 5.0 + (2.0 * pain_weight);
    
    if (petri_state_.motivation > max_motivation) {
        petri_state_.motivation = max_motivation;
    }
    
    // Floor: always some baseline urgency (addiction model)
    if (petri_state_.motivation < 0.5) {
        petri_state_.motivation = 0.5;
    }
    
    // Debug output
    cout << "  [Motivation] Value=" << fixed << setprecision(2) << petri_state_.motivation
         << " (cap: " << max_motivation << ")" << endl;
}

// ============================================================================
// PAIN LEVEL UPDATE - Maps effect to discrete pain level
// ============================================================================

void PatientAssessment::UpdatePainLevelContinuous(double effect) {
    // Pain naturally increases over time if not adequately treated
    // This models tolerance creating a pain escalation even at fixed dose
    
    if (effect < 40.0) {
        // Pain increases when relief inadequate
        petri_state_.pain_level = 3;
    } else if (effect < 60.0) {
        petri_state_.pain_level = 2;
    } else if (effect < 80.0) {
        petri_state_.pain_level = 1;
    } else {
        petri_state_.pain_level = 0;
    }
}

// ============================================================================
// DECISION LOGIC: Should we increase dose?
// ============================================================================

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
    
    bool should_dose = pain_sufficient && no_relief && motivated && time_elapsed && effect_insufficient;
    
    if (should_dose) {
        cout << "  [Decision Logic] Pain=" << petri_state_.pain_level
             << " Relief=" << no_relief << " Mot=" << setprecision(1) << petri_state_.motivation
             << " Effect=" << effect << "% → ESCALATE" << endl;
    }
    
    return should_dose;
}

// ============================================================================
// FIX #3: ExecuteDoseIncrease() - Safety checks, correct formula
// ============================================================================

void PatientAssessment::ExecuteDoseIncrease() {
    cout << "\n>>> DECISION: INCREASE DOSE (Transition T2) <<<" << endl;
    
    // Check preconditions
    if (!cont_state_.A || !cont_state_.C || !cont_state_.Ce || !cont_state_.Tol) {
        std::cerr << "ERROR: State not initialized!" << endl;
        petri_state_.patient_alive = false;
        Stop();
        return;
    }
    
    double Tol_val = cont_state_.Tol->Value();
    if (Tol_val < 0.0) Tol_val = 0.0;  // Clamp to zero
    
    // Calculate escalation per THESIS formula (Section VI):
    // f_escalation = 0.10 + 0.15 * Tol(t)
    double escalation_factor = params_.base_escalation_factor +
                               params_.tolerance_escalation_factor * Tol_val;
    
    // Clamp escalation factor to reasonable range
    if (escalation_factor < 0.01) escalation_factor = 0.01;    // Min 1%
    if (escalation_factor > 0.50) escalation_factor = 0.50;    // Max 50%
    
    double old_dose = petri_state_.current_dose;
    double new_dose = old_dose * (1.0 + escalation_factor);
    
    cout << "Tolerance Level: " << fixed << setprecision(4) << Tol_val << endl;
    cout << "Escalation Factor: " << (escalation_factor * 100.0) << "%" << endl;
    cout << "Old Dose: " << setprecision(2) << old_dose << " mg" << endl;
    cout << "New Dose: " << new_dose << " mg" << endl;
    cout << "Dose Increase: +" << (new_dose - old_dose) << " mg (+";
    cout << setprecision(2) << ((new_dose / old_dose - 1.0) * 100.0) << "%)" << endl;
    
    // Safety check: ensure previous dose has mostly absorbed
    // (A compartment should be <0.1 mg before next dose)
    double current_A = cont_state_.A->Value();
    if (current_A > 0.1) {
        cout << "WARNING: Previous dose still absorbing (" << current_A 
             << " mg in stomach). Dose stacking!" << endl;
    }
    
    // Add dose to absorption compartment
    // A(t) → A(t) + new_dose
    *cont_state_.A = current_A + new_dose;
    
    // Update parameters
    petri_state_.current_dose = new_dose;
    
    // Update Petri net state
    petri_state_.motivation -= params_.motivation_dose_reduction;
    if (petri_state_.motivation < 0.0) petri_state_.motivation = 0.0;
    
    petri_state_.time_since_last_dose = 0.0;
    petri_state_.relief_state = true;  // Patient expects relief after dosing
    
    // Record dose event for analysis
    RecordDoseEvent(new_dose);
    
    cout << "================================================" << endl;
}

// ============================================================================
// MAINTAIN DOSE: Continue current regimen
// ============================================================================

void PatientAssessment::MaintainDose() {
    cout << "\n>>> DECISION: MAINTAIN CURRENT DOSE (Transition T3) <<<" << endl;
    cout << "Current dose: " << petri_state_.current_dose << " mg" << endl;
    
    // Apply current dose
    double current_A = cont_state_.A->Value();
    *cont_state_.A = current_A + petri_state_.current_dose;
    
    // Update state
    petri_state_.time_since_last_dose = 0.0;
    
    // Record dose event
    RecordDoseEvent(petri_state_.current_dose);
    
    cout << "================================================" << endl;
}

// ============================================================================
// RECORD DOSE EVENT: Log to dose history
// ============================================================================

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

// ============================================================================
// OPTIONAL: Saturation Zone Monitoring (for diagnostics)
// ============================================================================

void PatientAssessment::MonitorSaturation() {
    double C_val = cont_state_.C->Value();
    double saturation_ratio = C_val / params_.Km;
    
    // Detect phase transitions
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

// ============================================================================
// OPTIONAL: Naloxone Rescue (T6 transition - for Scenario C)
// ============================================================================

void PatientAssessment::CheckAndApplyNaloxone() {
    // Preconditions for T6 (NALOXONE_RESCUE):
    if (petri_state_.patient_alive) {
        return;  // Not overdosed yet
    }
    
    if (!params_.naloxone_available) {
        return;  // No rescue medication available
    }
    
    // Check time window (naloxone effective within 5 minutes)
    double time_since_OD = Time - petri_state_.time_overdose_detected;
    if (time_since_OD > params_.naloxone_effective_window) {
        cout << "\n!!! NALOXONE WINDOW EXPIRED (>" << params_.naloxone_effective_window 
             << " min) - RESCUE FAILED !!!" << endl;
        return;  // Too late, window closed
    }
    
    // Apply naloxone (competitive antagonism)
    cout << "\n╔═══════════════════════════════════════════════════════════╗" << endl;
    cout << "║ T6: NALOXONE RESCUE ACTIVATED                            ║" << endl;
    cout << "╚═══════════════════════════════════════════════════════════╝" << endl;
    
    double C_before = cont_state_.C->Value();
    double Ce_before = cont_state_.Ce->Value();
    double Tol_before = cont_state_.Tol->Value();
    
    cout << "Time since OD: " << fixed << setprecision(1) << time_since_OD << " minutes" << endl;
    
    // Naloxone mechanism: competitive blockade
    // Effective concentration reduced (naloxone occupies receptors)
    double blockade_factor = params_.naloxone_blockade_strength;  // ~0.4 (40%)
    *cont_state_.C = C_before * (1.0 - blockade_factor);
    
    // Effect site suppression (naloxone has high affinity)
    *cont_state_.Ce = Ce_before * 0.1;  // Reduce to 10%
    
    // Partial tolerance reset (mu-receptor upregulation reversal)
    *cont_state_.Tol = Tol_before * 0.7;  // Reduce to 70%
    
    cout << "C(t): " << C_before << " → " << cont_state_.C->Value() << " mg/L" << endl;
    cout << "Ce(t): " << Ce_before << " → " << cont_state_.Ce->Value() << " mg/L" << endl;
    cout << "Tol(t): " << Tol_before << " → " << cont_state_.Tol->Value() << endl;
    
    // Revive patient
    petri_state_.patient_alive = true;
    
    // Acute withdrawal begins (opposite of relief)
    petri_state_.pain_level = 3;      // Severe pain from withdrawal
    petri_state_.relief_state = false;
    petri_state_.motivation = 3.0;    // High urgency to re-dose
    
    cout << "\nPatient REVIVED but experiencing ACUTE WITHDRAWAL" << endl;
    cout << "Status: ALIVE but in severe distress" << endl;
    cout << "Requires: ICU monitoring, serial naloxone dosing" << endl;
    cout << "Risk: Re-overdose in 1-2 hours if opioid still circulating" << endl;
}
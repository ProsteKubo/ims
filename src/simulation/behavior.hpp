#pragma once

#include <simlib.h>

#include <vector>

#include "dynamics.hpp"
#include "parameters.hpp"

// Petri Net State - Represents patient behavioral state
struct PetriNetState {
    // Places (state variables)
    int pain_level{2};           // 0=None, 1=Mild, 2=Moderate, 3=Severe
    bool relief_state{false};    // In pain relief state?
    double motivation{1.0};      // Accumulated motivation to dose
    double time_since_last_dose{0.0};
    bool patient_alive{true};    // Absorbing state for overdose
    double current_dose{10.0};   // Current dose amount in mg
    double time_overdose_detected{0.0};  // Time when overdose occurred

    // Dose history tracking
    struct DoseRecord {
        double time;
        double dose;
        double C;
        double Ce;
        double Tol;
        double effect;
    };
    std::vector<DoseRecord> dose_history;
};

// Patient Assessment Event - Discrete Decision Process
// Implements Petri net transitions based on current PK/PD state
class PatientAssessment : public Event {
public:
    PatientAssessment(const ModelParameters& params, SimulationState& cont_state,
                      PetriNetState& petri_state)
        : params_(params), cont_state_(cont_state), petri_state_(petri_state) {}

    void Behavior() override;

private:
    const ModelParameters& params_;
    SimulationState& cont_state_;
    PetriNetState& petri_state_;

    // Decision logic methods
    void UpdatePainLevel(double effect);
    void UpdatePainLevelContinuous(double effect);
    void UpdateMotivation(double dt);
    bool ShouldIncreaseDose(double effect);
    void ExecuteDoseIncrease();
    void MaintainDose();
    void RecordDoseEvent(double dose);
    void MonitorSaturation();
    void CheckAndApplyNaloxone();
};


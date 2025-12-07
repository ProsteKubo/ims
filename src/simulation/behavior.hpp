#pragma once

#include <simlib.h>

#include <vector>

#include "dynamics.hpp"
#include "parameters.hpp"

struct PetriNetState {
    int pain_level{2};
    bool relief_state{false};
    double motivation{1.0};
    double time_since_last_dose{0.0};
    bool patient_alive{true};
    double current_dose{10.0};
    double time_overdose_detected{0.0};

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

class PatientAssessment : public Event {
public:
    PatientAssessment(const ModelParameters& params, SimulationState& cont_state,
                      PetriNetState& petri_state)
        : params_(params), cont_state_(cont_state), petri_state_(petri_state) {}

    void Behavior() override;
    
    void CheckAndApplyNaloxonePublic();

private:
    const ModelParameters& params_;
    SimulationState& cont_state_;
    PetriNetState& petri_state_;
};


#include "pain_assessment.hpp"

#include <iomanip>
#include <iostream>

using std::cout;
using std::endl;
using std::fixed;
using std::setprecision;

void UpdatePainLevel(double effect, PetriNetState& petri_state) {
    if (effect > 80.0) {
        petri_state.pain_level = 0;
        petri_state.relief_state = true;
    } else if (effect > 60.0) {
        petri_state.pain_level = 1;
        petri_state.relief_state = true;
    } else if (effect > 40.0) {
        petri_state.pain_level = 2;
        petri_state.relief_state = false;
    } else {
        petri_state.pain_level = 3;
        petri_state.relief_state = false;
    }
    
    cout << "  [Pain Update] Effect=" << fixed << setprecision(1) << effect 
         << "% → PainLevel=" << petri_state.pain_level 
         << " Relief=" << (petri_state.relief_state ? "YES" : "NO") << endl;
}

void UpdatePainLevelContinuous(double effect, PetriNetState& petri_state) {
    if (effect < 40.0) {
        petri_state.pain_level = 3;
    } else if (effect < 60.0) {
        petri_state.pain_level = 2;
    } else if (effect < 80.0) {
        petri_state.pain_level = 1;
    } else {
        petri_state.pain_level = 0;
    }
}

void UpdateMotivation(double dt, const ModelParameters& params, PetriNetState& petri_state) {
    double pain_severity = static_cast<double>(petri_state.pain_level) / 3.0;
    double pain_contribution = params.motivation_pain_rate * pain_severity * dt;
    
    petri_state.motivation += pain_contribution;
    
    if (petri_state.relief_state && petri_state.pain_level <= 1) {
        double decay_factor = 1.0 - params.motivation_decay_rate;
        petri_state.motivation *= decay_factor;
    }
    
    double pain_weight = static_cast<double>(petri_state.pain_level) / 3.0;
    double max_motivation = 5.0 + (2.0 * pain_weight);
    
    if (petri_state.motivation > max_motivation) {
        petri_state.motivation = max_motivation;
    }
    
    if (petri_state.motivation < 0.5) {
        petri_state.motivation = 0.5;
    }
    
    cout << "  [Motivation] Value=" << fixed << setprecision(2) << petri_state.motivation
         << " (cap: " << max_motivation << ")" << endl;
}

#include "decision_logic.hpp"

#include <iomanip>
#include <iostream>

using std::cout;
using std::endl;
using std::fixed;
using std::setprecision;

bool ShouldIncreaseDose(double effect, const ModelParameters& params, PetriNetState& petri_state) {
    bool pain_sufficient = petri_state.pain_level >= 2;
    bool no_relief = !petri_state.relief_state;
    bool motivated = petri_state.motivation > params.motivation_threshold;
    bool time_elapsed = petri_state.time_since_last_dose >= params.min_dosing_interval;
    bool effect_insufficient = effect < params.effect_relief_threshold;
    
    bool should_dose = pain_sufficient && no_relief && motivated && time_elapsed && effect_insufficient;
    
    if (should_dose) {
        cout << "  [Decision Logic] Pain=" << petri_state.pain_level
             << " Relief=" << no_relief << " Mot=" << setprecision(1) << petri_state.motivation
             << " Effect=" << effect << "% → ESCALATE" << endl;
    }
    
    return should_dose;
}

#include "dose_management.hpp"

#include <simlib.h>

#include <iomanip>
#include <iostream>

using std::cout;
using std::endl;
using std::fixed;
using std::setprecision;

void ExecuteDoseIncrease(const ModelParameters& params, SimulationState& cont_state, PetriNetState& petri_state) {
    cout << "\n>>> DECISION: INCREASE DOSE (Transition T2) <<<" << endl;
    
    if (!cont_state.A || !cont_state.C || !cont_state.Ce || !cont_state.Tol) {
        std::cerr << "ERROR: State not initialized!" << endl;
        petri_state.patient_alive = false;
        Stop();
        return;
    }
    
    double Tol_val = cont_state.Tol->Value();
    if (Tol_val < 0.0) Tol_val = 0.0;
    
    double escalation_factor = params.base_escalation_factor +
                               params.tolerance_escalation_factor * Tol_val;
    
    if (escalation_factor < 0.01) escalation_factor = 0.01;
    if (escalation_factor > 0.50) escalation_factor = 0.50;
    
    double old_dose = petri_state.current_dose;
    double new_dose = old_dose * (1.0 + escalation_factor);
    
    cout << "Tolerance Level: " << fixed << setprecision(4) << Tol_val << endl;
    cout << "Escalation Factor: " << (escalation_factor * 100.0) << "%" << endl;
    cout << "Old Dose: " << setprecision(2) << old_dose << " mg" << endl;
    cout << "New Dose: " << new_dose << " mg" << endl;
    cout << "Dose Increase: +" << (new_dose - old_dose) << " mg (+";
    cout << setprecision(2) << ((new_dose / old_dose - 1.0) * 100.0) << "%" << endl;
    
    double current_A = cont_state.A->Value();
    if (current_A > 0.1) {
        cout << "WARNING: Previous dose still absorbing (" << current_A 
             << " mg in stomach). Dose stacking!" << endl;
    }
    
    *cont_state.A = current_A + new_dose;
    
    petri_state.current_dose = new_dose;
    
    petri_state.motivation -= params.motivation_dose_reduction;
    if (petri_state.motivation < 0.0) petri_state.motivation = 0.0;
    
    petri_state.time_since_last_dose = 0.0;
    petri_state.relief_state = true;
    
    RecordDoseEvent(new_dose, params, cont_state, petri_state);
    
    cout << "================================================" << endl;
}

void MaintainDose(const ModelParameters& params, SimulationState& cont_state, PetriNetState& petri_state) {
    cout << "\n>>> DECISION: MAINTAIN CURRENT DOSE (Transition T3) <<<" << endl;
    cout << "Current dose: " << petri_state.current_dose << " mg" << endl;
    
    double current_A = cont_state.A->Value();
    *cont_state.A = current_A + petri_state.current_dose;
    
    petri_state.time_since_last_dose = 0.0;
    
    RecordDoseEvent(petri_state.current_dose, params, cont_state, petri_state);
    
    cout << "================================================" << endl;
}

void RecordDoseEvent(double dose, const ModelParameters& params, SimulationState& cont_state, PetriNetState& petri_state) {
    PetriNetState::DoseRecord record;
    record.time = Time;
    record.dose = dose;
    record.C = cont_state.C->Value();
    record.Ce = cont_state.Ce->Value();
    record.Tol = cont_state.Tol->Value();
    record.effect = CalculateEffect(cont_state.Ce->Value(), cont_state.Tol->Value(), params);
    
    petri_state.dose_history.push_back(record);
    
    cout << "\n--- DOSE ADMINISTERED ---" << endl;
    cout << "Time: " << record.time << " h" << endl;
    cout << "Dose: " << record.dose << " mg" << endl;
    cout << "C(t): " << record.C << " mg/L" << endl;
    cout << "Ce(t): " << record.Ce << " mg/L" << endl;
    cout << "Tol(t): " << record.Tol << endl;
    cout << "Effect: " << record.effect << "%" << endl;
    cout << "Total doses given: " << petri_state.dose_history.size() << endl;
}

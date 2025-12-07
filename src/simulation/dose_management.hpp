#pragma once

#include "behavior.hpp"
#include "dynamics.hpp"
#include "parameters.hpp"

void ExecuteDoseIncrease(const ModelParameters& params, SimulationState& cont_state, PetriNetState& petri_state);
void MaintainDose(const ModelParameters& params, SimulationState& cont_state, PetriNetState& petri_state);
void RecordDoseEvent(double dose, const ModelParameters& params, SimulationState& cont_state, PetriNetState& petri_state);

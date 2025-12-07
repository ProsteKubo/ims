#pragma once

#include "behavior.hpp"
#include "dynamics.hpp"
#include "parameters.hpp"

void MonitorSaturation(const ModelParameters& params, SimulationState& cont_state);
void CheckAndApplyNaloxone(const ModelParameters& params, SimulationState& cont_state, PetriNetState& petri_state);

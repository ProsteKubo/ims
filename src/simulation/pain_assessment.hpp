#pragma once

#include "behavior.hpp"
#include "parameters.hpp"

void UpdatePainLevel(double effect, PetriNetState& petri_state);
void UpdatePainLevelContinuous(double effect, PetriNetState& petri_state);
void UpdateMotivation(double dt, const ModelParameters& params, PetriNetState& petri_state);

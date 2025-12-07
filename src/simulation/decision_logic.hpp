#pragma once

#include "behavior.hpp"
#include "parameters.hpp"

bool ShouldIncreaseDose(double effect, const ModelParameters& params, PetriNetState& petri_state);

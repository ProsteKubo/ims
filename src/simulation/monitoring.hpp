#pragma once

#include <simlib.h>

#include <iostream>

#include "dynamics.hpp"
#include "behavior.hpp"

bool CheckToxicity(const SimulationState& state, const ModelParameters& params);

class StatusMonitor : public Event {
public:
    StatusMonitor(const ModelParameters& params, SimulationState& state, PetriNetState& petri_state)
        : params_(params), state_(state), petri_state_(petri_state) {}
    void Behavior() override;

private:
    const ModelParameters& params_;
    SimulationState& state_;
    PetriNetState& petri_state_;
};

class DosingEvent : public Event {
public:
    DosingEvent(const ModelParameters& params, SimulationState& state)
        : params_(params), state_(state) {}
    void Behavior() override;

private:
    const ModelParameters& params_;
    SimulationState& state_;
};

class NaloxoneRescue : public Event {
public:
    NaloxoneRescue(const ModelParameters& params, SimulationState& state, PetriNetState& petri_state)
        : params_(params), state_(state), petri_state_(petri_state) {}
    void Behavior() override;

private:
    const ModelParameters& params_;
    SimulationState& state_;
    PetriNetState& petri_state_;
};

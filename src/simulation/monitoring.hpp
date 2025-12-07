#pragma once

#include <simlib.h>

#include <iostream>

#include "dynamics.hpp"

bool CheckToxicity(const SimulationState& state, const ModelParameters& params);

class StatusMonitor : public Event {
public:
    StatusMonitor(const ModelParameters& params, SimulationState& state)
        : params_(params), state_(state) {}
    void Behavior() override;

private:
    const ModelParameters& params_;
    SimulationState& state_;
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

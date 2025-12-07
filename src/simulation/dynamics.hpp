#pragma once

#include <simlib.h>

#include "parameters.hpp"

struct SimulationState {
    Integrator* A{};
    Integrator* C{};
    Integrator* P{};
    Integrator* Ce{};
    Integrator* Tol{};
};

double MichaelisMentenElimination(double concentration, const ModelParameters& params);
double CalculateEffect(double Ce_val, double Tol_val, const ModelParameters& params);
double ToleranceSignal(double Ce_val, const ModelParameters& params);

class AbsorptionDynamics : public aContiBlock {
public:
    AbsorptionDynamics(const ModelParameters& params, SimulationState& state)
        : params_(params), state_(state) {}
    double Value() override;

private:
    const ModelParameters& params_;
    SimulationState& state_;
};

class CentralDynamics : public aContiBlock {
public:
    CentralDynamics(const ModelParameters& params, SimulationState& state)
        : params_(params), state_(state) {}
    double Value() override;

private:
    const ModelParameters& params_;
    SimulationState& state_;
};

class PeripheralDynamics : public aContiBlock {
public:
    PeripheralDynamics(const ModelParameters& params, SimulationState& state)
        : params_(params), state_(state) {}
    double Value() override;

private:
    const ModelParameters& params_;
    SimulationState& state_;
};

class EffectSiteDynamics : public aContiBlock {
public:
    EffectSiteDynamics(const ModelParameters& params, SimulationState& state)
        : params_(params), state_(state) {}
    double Value() override;

private:
    const ModelParameters& params_;
    SimulationState& state_;
};

class ToleranceDynamics : public aContiBlock {
public:
    ToleranceDynamics(const ModelParameters& params, SimulationState& state)
        : params_(params), state_(state) {}
    double Value() override;

private:
    const ModelParameters& params_;
    SimulationState& state_;
};

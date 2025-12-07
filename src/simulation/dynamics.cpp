#include "dynamics.hpp"

double AbsorptionDynamics::Value() {
    if (!state_.A) return 0.0;
    return -params_.ka * state_.A->Value();
}

double CentralDynamics::Value() {
    if (!state_.A || !state_.C || !state_.P) return 0.0;
    double absorption_flux = (params_.ka * state_.A->Value()) / params_.Vd;
    double elimination_flux = MichaelisMentenElimination(state_.C->Value(), params_) / params_.Vd;
    double peripheral_out = params_.kcp * state_.C->Value();
    double peripheral_in = params_.kpc * state_.P->Value();
    return absorption_flux - elimination_flux - peripheral_out + peripheral_in;
}

double PeripheralDynamics::Value() {
    if (!state_.C || !state_.P) return 0.0;
    double influx = params_.kcp * state_.C->Value();
    double efflux = params_.kpc * state_.P->Value();
    return influx - efflux;
}

double EffectSiteDynamics::Value() {
    if (!state_.C || !state_.Ce) return 0.0;
    return (params_.keo / params_.tau_e) * (state_.C->Value() - state_.Ce->Value());
}

double ToleranceDynamics::Value() {
    if (!state_.Ce || !state_.Tol) return 0.0;
    double signal = ToleranceSignal(state_.Ce->Value(), params_);
    return params_.kin * signal - params_.kout * state_.Tol->Value();
}

#include "dynamics.hpp"

#include <cmath>

using std::pow;

double MichaelisMentenElimination(double concentration, const ModelParameters& params) {
    if (concentration < 0) return 0.0;
    return (params.Vmax * concentration) / (params.Km + concentration);
}

double CalculateEffect(double Ce_val, double Tol_val, const ModelParameters& params) {
    if (Ce_val < 0) Ce_val = 0;
    if (Tol_val < 0) Tol_val = 0;

    double EC50_current = params.EC50_base * (1.0 + Tol_val);
    double Ce_n = pow(Ce_val, params.n_Hill);
    double EC50_n = pow(EC50_current, params.n_Hill);

    return params.Emax * Ce_n / (EC50_n + Ce_n);
}

double ToleranceSignal(double Ce_val, const ModelParameters& params) {
    if (Ce_val < 0) Ce_val = 0;
    return Ce_val / (params.EC50_signal + Ce_val);
}

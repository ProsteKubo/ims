/**
 * The Deadly Spiral: Interaction of Tolerance and Metabolic Saturation
 * 
 * Hybrid discrete-continuous simulation of tolerance-driven escalation and
 * metabolic saturation in long-term opioid usage.
 * 
 * This file implements the continuous pharmacokinetic/pharmacodynamic model
 * with 5 differential equations governing physiological dynamics.
 */

#include <simlib.h>
#include <iostream>
#include <iomanip>
#include <fstream>
#include <sstream>
#include <string>
#include <map>
#include <cmath>

using namespace std;

//=============================================================================
// CONFIGURATION READER
//=============================================================================

class ConfigReader {
private:
    map<string, double> params;
    
    string trim(const string& str) {
        size_t first = str.find_first_not_of(" \t\r\n");
        if (first == string::npos) return "";
        size_t last = str.find_last_not_of(" \t\r\n");
        return str.substr(first, last - first + 1);
    }
    
public:
    bool load(const string& filename) {
        ifstream file(filename);
        if (!file.is_open()) {
            cerr << "Error: Cannot open config file: " << filename << endl;
            return false;
        }
        
        string line;
        while (getline(file, line)) {
            line = trim(line);
            
            // Skip empty lines and comments
            if (line.empty() || line[0] == '#' || line[0] == '[') continue;
            
            // Parse key = value
            size_t eq_pos = line.find('=');
            if (eq_pos != string::npos) {
                string key = trim(line.substr(0, eq_pos));
                string value_str = trim(line.substr(eq_pos + 1));
                
                // Remove inline comments
                size_t comment_pos = value_str.find('#');
                if (comment_pos != string::npos) {
                    value_str = trim(value_str.substr(0, comment_pos));
                }
                
                // Convert to double
                try {
                    double value = stod(value_str);
                    params[key] = value;
                } catch (...) {
                    cerr << "Warning: Invalid value for " << key << ": " << value_str << endl;
                }
            }
        }
        
        file.close();
        return true;
    }
    
    double get(const string& key, double default_value = 0.0) const {
        auto it = params.find(key);
        if (it != params.end()) {
            return it->second;
        }
        return default_value;
    }
    
    void print() const {
        cout << "Loaded Configuration Parameters:" << endl;
        for (const auto& p : params) {
            cout << "  " << p.first << " = " << p.second << endl;
        }
    }
};

//=============================================================================
// PHARMACOKINETIC/PHARMACODYNAMIC PARAMETERS
//=============================================================================

// Global configuration
ConfigReader config;

// Absorption parameters
double ka;

// Distribution volumes
double Vd;
double Vp;

// Intercompartmental transfer rates
double kcp;
double kpc;

// Michaelis-Menten elimination parameters (CRITICAL FOR DEADLY SPIRAL)
double Vmax;
double Km;

// Effect-site equilibration
double keo;
double tau_e;

// Pharmacodynamic parameters
double Emax;
double EC50_base;
double n_Hill;

// Tolerance dynamics
double kin;
double kout;
double EC50_signal;

// Toxicity thresholds
double C_toxic;
double C_critical;
double Effect_resp_critical;

// Dosing parameters
double current_dose;
double dosing_interval;

// Simulation parameters
double sim_duration;
double sim_step_min;
double sim_step_max;
double sim_accuracy;
double output_interval;

//=============================================================================
// CONTINUOUS STATE VARIABLES (Integrators) - Forward declarations
//=============================================================================

class AbsorptionDynamics;
class CentralDynamics;
class PeripheralDynamics;
class EffectSiteDynamics;
class ToleranceDynamics;

// Global state variables - will be initialized in main()
Integrator *A_ptr = nullptr;
Integrator *C_ptr = nullptr;
Integrator *P_ptr = nullptr;
Integrator *Ce_ptr = nullptr;
Integrator *Tol_ptr = nullptr;

//=============================================================================
// NONLINEAR ELIMINATION FUNCTION (Michaelis-Menten)
//=============================================================================

/**
 * Michaelis-Menten elimination kinetics
 * 
 * Three regimes:
 * 1. Linear (C << Km): Cl ≈ (Vmax/Km) * C  [first-order]
 * 2. Saturation (C ≈ Km): Cl ≈ Vmax/2      [mixed-order]
 * 3. Plateau (C >> Km): Cl ≈ Vmax          [zero-order]
 * 
 * This nonlinearity is the mathematical essence of the "deadly spiral"
 */
double MichaelisMenten_Elimination(double concentration) {
    if (concentration < 0) return 0.0;
    return (Vmax * concentration) / (Km + concentration);
}

//=============================================================================
// EFFECT CALCULATION (Sigmoid Emax with Tolerance)
//=============================================================================

/**
 * Pharmacodynamic effect model
 * 
 * Effect = Emax * Ce^n / (EC50(Tol)^n + Ce^n)
 * 
 * where EC50(Tol) = EC50_base * (1 + Tol)
 * 
 * As tolerance increases, higher concentration is needed for same effect.
 */
double Calculate_Effect(double Ce_val, double Tol_val) {
    if (Ce_val < 0) Ce_val = 0;
    if (Tol_val < 0) Tol_val = 0;
    
    double EC50_current = EC50_base * (1.0 + Tol_val);
    double Ce_n = pow(Ce_val, n_Hill);
    double EC50_n = pow(EC50_current, n_Hill);
    
    return Emax * Ce_n / (EC50_n + Ce_n);
}

/**
 * Tolerance signal function
 * 
 * Signal = Ce / (EC50_signal + Ce)
 * 
 * Drives tolerance accumulation proportional to effect-site concentration
 */
double Tolerance_Signal(double Ce_val) {
    if (Ce_val < 0) Ce_val = 0;
    return Ce_val / (EC50_signal + Ce_val);
}

//=============================================================================
// DIFFERENTIAL EQUATIONS (5 Equations)
//=============================================================================

/**
 * Equation 1: Absorption Compartment
 * dA/dt = -ka * A
 * 
 * Drug enters from GI tract and is absorbed into blood
 */
class AbsorptionDynamics : public aContiBlock {
public:
    double Value() {
        return -ka * A_ptr->Value();
    }
};

/**
 * Equation 2: Central Compartment (Blood)
 * dC/dt = (ka*A)/Vd - Vmax*C/(Km+C)/Vd - kcp*C + kpc*P
 * 
 * NONLINEAR elimination is the key feature here
 */
class CentralDynamics : public aContiBlock {
public:
    double Value() {
        double absorption_flux = (ka * A_ptr->Value()) / Vd;
        double elimination_flux = MichaelisMenten_Elimination(C_ptr->Value()) / Vd;
        double peripheral_out = kcp * C_ptr->Value();
        double peripheral_in = kpc * P_ptr->Value();
        return absorption_flux - elimination_flux - peripheral_out + peripheral_in;
    }
};

/**
 * Equation 3: Peripheral Compartment
 * dP/dt = kcp*C - kpc*P
 * 
 * Reversible distribution between central and peripheral tissues
 */
class PeripheralDynamics : public aContiBlock {
public:
    double Value() {
        double influx = kcp * C_ptr->Value();
        double efflux = kpc * P_ptr->Value();
        return influx - efflux;
    }
};

/**
 * Equation 4: Effect-Site Link (Delay System)
 * dCe/dt = keo * (C - Ce) / tau_e
 * 
 * Provides smoothing delay between blood and effect site
 */
class EffectSiteDynamics : public aContiBlock {
public:
    double Value() {
        return (keo / tau_e) * (C_ptr->Value() - Ce_ptr->Value());
    }
};

/**
 * Equation 5: Tolerance Dynamics
 * dTol/dt = kin * Signal(Ce) - kout * Tol
 * 
 * Tolerance increases with drug exposure and decays slowly
 * Key asymmetry: kin >> kout (builds fast, recovers slow)
 */
class ToleranceDynamics : public aContiBlock {
public:
    double Value() {
        double signal = Tolerance_Signal(Ce_ptr->Value());
        return kin * signal - kout * Tol_ptr->Value();
    }
};

//=============================================================================
// MONITORING AND OUTPUT
//=============================================================================

/**
 * Check toxicity conditions
 */
bool Check_Toxicity() {
    double effect = Calculate_Effect(Ce_ptr->Value(), Tol_ptr->Value());
    
    if (C_ptr->Value() > C_critical) {
        cout << "\n!!! CRITICAL OVERDOSE at t=" << Time << " hours !!!" << endl;
        cout << "    C(t) = " << C_ptr->Value() << " mg/L (critical threshold: " 
             << C_critical << " mg/L)" << endl;
        return true;
    }
    
    if (effect > Effect_resp_critical) {
        cout << "\n!!! RESPIRATORY ARREST at t=" << Time << " hours !!!" << endl;
        cout << "    Respiratory depression = " << effect << "%" << endl;
        return true;
    }
    
    if (C_ptr->Value() > C_toxic) {
        cout << "\n>>> WARNING: Toxic concentration reached at t=" << Time 
             << " hours <<<" << endl;
        cout << "    C(t) = " << C_ptr->Value() << " mg/L" << endl;
    }
    
    return false;
}

/**
 * Periodic status output
 */
class StatusMonitor : public Event {
    void Behavior() {
        double effect = Calculate_Effect(Ce_ptr->Value(), Tol_ptr->Value());
        
        cout << fixed << setprecision(2);
        cout << "t=" << setw(6) << Time << "h | "
             << "A=" << setw(6) << A_ptr->Value() << " mg | "
             << "C=" << setw(6) << C_ptr->Value() << " mg/L | "
             << "P=" << setw(6) << P_ptr->Value() << " mg/L | "
             << "Ce=" << setw(6) << Ce_ptr->Value() << " mg/L | "
             << "Tol=" << setw(5) << Tol_ptr->Value() << " | "
             << "Effect=" << setw(5) << effect << "%" << endl;
        
        // Check for toxicity
        if (Check_Toxicity()) {
            Stop();
            return;
        }
        
        // Schedule next monitoring event
        Activate(Time + output_interval);
    }
};

/**
 * Dosing event
 */
class DosingEvent : public Event {
    void Behavior() {
        cout << "\n>>> DOSE ADMINISTERED at t=" << Time << " hours: " 
             << current_dose << " mg <<<" << endl;
        
        // Add dose to absorption compartment
        *A_ptr = A_ptr->Value() + current_dose;
        
        // Schedule next dose
        Activate(Time + dosing_interval);
    }
};

//=============================================================================
// MAIN SIMULATION
//=============================================================================

int main(int argc, char* argv[]) {
    // Load configuration
    string config_file = "config.ini";
    if (argc > 1) {
        config_file = argv[1];
    }
    
    cout << "========================================================================" << endl;
    cout << "  THE DEADLY SPIRAL: Continuous PK/PD Simulation" << endl;
    cout << "  Pharmacokinetic-Pharmacodynamic Model with Metabolic Saturation" << endl;
    cout << "========================================================================" << endl;
    cout << endl;
    
    cout << "Loading configuration from: " << config_file << endl;
    if (!config.load(config_file)) {
        cerr << "Failed to load configuration. Exiting." << endl;
        return 1;
    }
    cout << endl;
    
    // Load all parameters from config
    ka = config.get("ka", 2.0);
    Vd = config.get("Vd", 28.0);
    Vp = config.get("Vp", 105.0);
    kcp = config.get("kcp", 0.3);
    kpc = config.get("kpc", 0.4);
    Vmax = config.get("Vmax", 10.0);
    Km = config.get("Km", 2.0);
    keo = config.get("keo", 0.5);
    tau_e = config.get("tau_e", 2.0);
    Emax = config.get("Emax", 95.0);
    EC50_base = config.get("EC50_base", 3.0);
    n_Hill = config.get("n_Hill", 1.2);
    kin = config.get("kin", 0.10);
    kout = config.get("kout", 0.005);
    EC50_signal = config.get("EC50_signal", 2.0);
    C_toxic = config.get("C_toxic", 15.0);
    C_critical = config.get("C_critical", 50.0);
    Effect_resp_critical = config.get("Effect_resp_critical", 90.0);
    current_dose = config.get("initial_dose", 10.0);
    dosing_interval = config.get("dosing_interval", 12.0);
    sim_duration = config.get("duration", 720.0);
    sim_step_min = config.get("step_min", 0.01);
    sim_step_max = config.get("step_max", 0.1);
    sim_accuracy = config.get("accuracy", 1e-6);
    output_interval = config.get("output_interval", 1.0);
    
    // Display model parameters
    cout << "Model Parameters:" << endl;
    cout << "  Absorption: ka = " << ka << " /h" << endl;
    cout << "  Distribution: Vd = " << Vd << " L, Vp = " << Vp << " L" << endl;
    cout << "  Transfer: kcp = " << kcp << " /h, kpc = " << kpc << " /h" << endl;
    cout << "  Elimination (M-M): Vmax = " << Vmax << " mg/h, Km = " << Km << " mg/L" << endl;
    cout << "  Effect-site: keo = " << keo << " /h, tau_e = " << tau_e << " h" << endl;
    cout << "  PD: Emax = " << Emax << "%, EC50 = " << EC50_base << " mg/L, n = " << n_Hill << endl;
    cout << "  Tolerance: kin = " << kin << " /h, kout = " << kout << " /h" << endl;
    cout << "  Dosing: " << current_dose << " mg every " << dosing_interval << " hours" << endl;
    cout << "  Toxicity: C_toxic = " << C_toxic << " mg/L, C_critical = " << C_critical << " mg/L" << endl;
    cout << "  Simulation: " << sim_duration << " hours, output every " << output_interval << " hours" << endl;
    cout << endl;
    
    // Initialize state variables
    Init(0, sim_duration);
    SetStep(sim_step_min, sim_step_max);
    SetAccuracy(sim_accuracy);
    
    // Create differential equation blocks
    AbsorptionDynamics dA_dt;
    CentralDynamics dC_dt;
    PeripheralDynamics dP_dt;
    EffectSiteDynamics dCe_dt;
    ToleranceDynamics dTol_dt;
    
    // Create integrators connected to differential equations
    Integrator A(dA_dt, current_dose);  // Initial dose in absorption compartment
    Integrator C(dC_dt, 0.0);           // No initial blood concentration
    Integrator P(dP_dt, 0.0);           // No initial peripheral concentration
    Integrator Ce(dCe_dt, 0.0);         // No initial effect-site concentration
    Integrator Tol(dTol_dt, 0.0);       // No initial tolerance
    
    // Set global pointers
    A_ptr = &A;
    C_ptr = &C;
    P_ptr = &P;
    Ce_ptr = &Ce;
    Tol_ptr = &Tol;
    
    cout << "Initial Conditions:" << endl;
    cout << "  A(0) = " << A.Value() << " mg (first dose)" << endl;
    cout << "  C(0) = " << C.Value() << " mg/L" << endl;
    cout << "  P(0) = " << P.Value() << " mg/L" << endl;
    cout << "  Ce(0) = " << Ce.Value() << " mg/L" << endl;
    cout << "  Tol(0) = " << Tol.Value() << endl;
    cout << endl;
    cout << "========================================================================" << endl;
    cout << "                        SIMULATION OUTPUT" << endl;
    cout << "========================================================================" << endl;
    cout << endl;
    
    // Schedule events
    (new StatusMonitor)->Activate(Time + output_interval);
    (new DosingEvent)->Activate(Time + dosing_interval);
    
    // Run simulation
    Run();
    
    // Final summary
    cout << endl;
    cout << "========================================================================" << endl;
    cout << "                        SIMULATION SUMMARY" << endl;
    cout << "========================================================================" << endl;
    cout << endl;
    cout << "Final State (t=" << Time << " hours):" << endl;
    cout << "  A(t) = " << A.Value() << " mg" << endl;
    cout << "  C(t) = " << C.Value() << " mg/L" << endl;
    cout << "  P(t) = " << P.Value() << " mg/L" << endl;
    cout << "  Ce(t) = " << Ce.Value() << " mg/L" << endl;
    cout << "  Tol(t) = " << Tol.Value() << endl;
    cout << "  Effect = " << Calculate_Effect(Ce.Value(), Tol.Value()) << "%" << endl;
    cout << endl;
    
    // Analysis
    double saturation_ratio = C.Value() / Km;
    cout << "Pharmacokinetic Analysis:" << endl;
    cout << "  Saturation ratio (C/Km) = " << saturation_ratio << endl;
    if (saturation_ratio < 0.5) {
        cout << "  Status: LINEAR REGIME - First-order elimination dominates" << endl;
    } else if (saturation_ratio < 3.0) {
        cout << "  Status: SATURATION ZONE - Nonlinear kinetics active" << endl;
        cout << "  WARNING: Approaching dangerous territory!" << endl;
    } else {
        cout << "  Status: PLATEAU REGIME - Zero-order elimination (capacity exhausted)" << endl;
        cout << "  CRITICAL: System in deadly spiral zone!" << endl;
    }
    cout << endl;
    
    double EC50_current = EC50_base * (1.0 + Tol.Value());
    double tolerance_factor = EC50_current / EC50_base;
    cout << "Pharmacodynamic Analysis:" << endl;
    cout << "  Current EC50 = " << EC50_current << " mg/L (baseline: " << EC50_base << " mg/L)" << endl;
    cout << "  Tolerance multiplier = " << tolerance_factor << "x" << endl;
    cout << "  Required dose for same effect = " << tolerance_factor * current_dose << " mg" << endl;
    cout << endl;
    
    cout << "========================================================================" << endl;
    
    return 0;
}

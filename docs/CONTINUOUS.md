# Continuous Subsystem – Pharmacokinetic and Pharmacodynamic Model

This file defines the **five continuous differential equations** governing
the physiological dynamics of the system.

---

## 1. Variables and Parameters

| Symbol | Meaning | Typical Range |
|----------|----------|----------------|
| `A(t)` | Amount of drug in absorption compartment | mg |
| `C(t)` | Concentration in central (blood) compartment | mg/L |
| `P(t)` | Concentration in peripheral tissue | mg/L |
| `Ce(t)` | Effect-site concentration | mg/L |
| `Tol(t)` | Tolerance factor | dimensionless |
| `ka` | Absorption rate constant | 0.1–2 /h |
| `Vd` | Apparent distribution volume | L |
| `Vmax` | Max elimination rate | mg/h |
| `Km` | Michaelis constant (saturation threshold) | mg/L |
| `kcp`, `kpc` | Intercompartmental transfer rates | /h |
| `keo` | Effect-site equilibration constant | /h |
| `Emax` | Maximal achievable effect | dimensionless |
| `EC50` | Concentration for 50% effect | mg/L |
| `kin`, `kout` | Tolerance kinetics constants | /h |

---

## 2. System of Differential Equations

### 2.1 Absorption compartment
\[
\frac{dA}{dt} = Dosing(t) - k_a \cdot A
\]

### 2.2 Central compartment (blood)
\[
\frac{dC}{dt} = \frac{k_a}{V_d} \cdot A
               - \frac{V_{max} \cdot C}{K_m + C}
               - k_{cp} \cdot C
               + k_{pc} \cdot P
\]

The middle term represents **nonlinear metabolic elimination** according to
Michaelis–Menten kinetics.

### 2.3 Peripheral compartment
\[
\frac{dP}{dt} = k_{cp} \cdot C - k_{pc} \cdot P
\]

Captures reversible distribution between central and peripheral compartments.

### 2.4 Effect-site link (delay system)
\[
\frac{dC_e}{dt} = k_{eo} \cdot (C - C_e)
\]

Provides smoothing delay for observed effect to prevent immediate reactions.

### 2.5 Tolerance dynamics
\[
\frac{dTol}{dt} = k_{in} \cdot E - k_{out} \cdot Tol
\]

Tolerance increases with the presence of effect and decays slowly otherwise.

---

## 3. Effect Equation (Algebraic Relation)
\[
E = E_{max} \cdot
    \frac{C_e}{EC_{50} \cdot Tol + C_e}
\]

- As tolerance rises, more drug is required to reach the same effect.
- When `Tol` → 0, this reduces to a classical Emax relationship.

---

## 4. Toxicity Condition

Define a threshold `Ctoxic`.  
When `C(t) > Ctoxic`, the discrete Petri system triggers transition to **Toxic**
state and optionally calls the **Antidote** process.

---

## 5. Simulation Notes

- Integrator step should be small relative to `ka` and `keo`.
- Saturation of enzyme elimination can cause stiffness — adaptive solvers
recommended.
- All continuous variables remain non-negative (clip at 0).

---

## 6. Outputs to Discrete Layer

| Output | Used For |
|----------|----------|
| `E(t)` | Determines behaviour state (`Pain`, `Relief`) |
| `C(t)` | Determines toxicity level |
| `Tol(t)` | Optional feedback display |

---

## 7. Key Dynamics Summary

| Regime | Characteristic |
|----------|----------------|
| **Sublinear** (`C << Km`) | Elimination ≈ first-order; steady oscillations |
| **Near-saturation** (`C ≈ Km`) | Small dosing changes amplify `C` growth |
| **Super-saturation** (`C >> Km`) | Elimination ~ constant rate; runaway concentration |

Transition from sublinear to supersaturation is the mathematical essence of
the “deadly spiral”.

---
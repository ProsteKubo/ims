# MODEL Overview

This document provides a high-level description of the entire hybrid model
architecture combining:

- **Continuous subsystem:** pharmacokinetic (PK) & pharmacodynamic (PD)
  processes — governed by differential equations.
- **Discrete subsystem:** behavioural regulation of dosing, represented as a
  Petri net and process logic.

Together, these form a **closed adaptive feedback loop** that can self‑escalate
towards toxicity.

---

## 1. Conceptual Layers

| Layer | Nature | Purpose | Implemented in |
|--------|--------|----------|----------------|
| **Continuous (PK–PD)** | Differential equations | Simulate drug absorption, distribution, elimination, tolerance | `CONTINUOUS.md` |
| **Discrete (Behaviour / Petri net)** | Event-based transitions | Model decision-making and dosing schedule | `DISCRETE.md` |

---

## 2. Signal Flow

[Behavioural Petri Net]

│     ↑

Dose│     │Feedback (E)

▼     │

+-----------------------------+

| Continuous Integrators: A,C,P,Ce,Tol |

+-----------------------------+

│

▼

Observable Effects

(Pain ↔ Relief ↔ Toxicity)

- The **discrete layer** injects new doses based on observed `E` (effect).
- The **continuous layer** transforms the dose into changing concentration,
  effect, and tolerance values.
- The **feedback** of low `E` increases dose → nonlinear PK regime →
  overdose.

---

## 3. Model Philosophy

This model serves both research and educational purposes:

- Demonstrates **how linear human decision loops can destabilize nonlinear
  physiological systems**.
- Shows emergent phenomena: bifurcation, delayed feedback, runaway escalation.
- Modular structure allows independent testing of each layer or full coupling.

---

## 4. Module Dependencies

| Module | Inputs | Outputs | Interaction |
|---------|--------|----------|-------------|
| `CONTINUOUS` | Dosing events from DISCRETE | Concentration `C`, Effect `E`, Tolerance `Tol` | Sends real values back to DISCRETE |
| `DISCRETE` | `E` (effect) from CONTINUOUS | Next dosing magnitude | Controls dosing frequency and escalation |

---

## 5. Terminology

| Symbol | Meaning | Domain |
|---------|----------|--------|
| **A** | Drug in absorption compartment (gut) | PK |
| **C** | Central (blood) concentration | PK |
| **P** | Peripheral tissue compartment | PK |
| **Ce** | Effect-site concentration | PD |
| **Tol** | Tolerance factor | PD |
| **E** | Analgesic effect | PD |
| **Dosing(t)** | Dose injection function triggered by Petri transitions | Behaviour |
| **State** | Petri net state (`Pain`, `Relief`, `Toxic`, etc.) | Discrete |

---

## 6. Time Scales

| Process | Typical Interval | Representation |
|----------|------------------|----------------|
| Absorption | Minutes–hours | Continuous |
| Elimination | Hours–days | Continuous, saturable |
| Tolerance growth | Hours–days | Continuous |
| Dosing behaviour | 12 hours (or configurable) | Discrete wake-up period |
| Transition detection | Instantaneous events | Petri triggers |

---

## 7. Integration Strategy

- Continuous equations solved by numerical integrator (e.g., RK4, adaptive Euler).
- Discrete process awakened periodically (each dosing cycle).
- Synchronization handled by SIMLIB scheduler — ensures events align with
integration steps.

---

## 8. Overall Behavioural Summary

LOW E → Increase Dose

↑ Dose → ↑ C → ↑ E → ↑ Tol → ↓ E  (feedback)

↓

Once C ~ Km → system nonlinear → overdose

This coupling forms the **“smrtiaca špirála”** (deadly spiral) — a self-reinforcing system collapse.
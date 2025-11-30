# Thesis – The Deadly Spiral: Interaction of Tolerance and Metabolic Saturation

## Abstract

This work models a paradoxical situation that arises in long-term users of analgesics and opioids — **the so-called deadly spiral**.
It involves a nonlinear interaction between two processes:

1. **Pharmacodynamic Tolerance (PD)** – the organism becomes accustomed to the effect, causing the patient to unknowingly increase the dose.
2. **Pharmacokinetic Saturation (PK)** – the liver and enzymes can only break down the drug up to a certain limit (Michaelis-Menten limit).

The combination of these phenomena leads to a phenomenon where **a small increase in dose** causes a **catastrophic increase in blood concentration**, which can lead to overdose.

---

## Research Question

**Why does accidental overdose occur in long-term users, even when they only slightly increase their usual dose?**

**Hypothesis:**
The rate of tolerance development forces the patient to increase the dose before pharmacokinetics stabilizes. Upon crossing the metabolic saturation limit, a sudden nonlinear increase in concentration occurs – the system "falls over the threshold" and eliminates the body's ability to maintain homeostasis.

---

## Project Goals

1. Develop a **mathematical model (5 differential equations)** connecting PK and PD processes.
2. Implement a **discrete-continuous hybrid model** in the **SIMLIB** environment.
3. Extend the model with a **Petri net** describing the patient's external behavior (decision-making, motivation, addiction).
4. Investigate **three experimental scenarios**:
   - A: Stable dose – patient lives, but with pain.
   - B: Dose escalation – typical path to overdose.
   - C: Antidote – reactive intervention after crossing the toxic zone.

---

## Methodology

The model consists of two layers:

### 1. **Internal Physiological Dynamics (Differential Equations)**
- `A` – amount of drug in the stomach (absorption)
- `C` – concentration in blood (central compartment)
- `P` – distribution to periphery
- `Ce` – effective concentration – delayed effect transfer
- `Tol` – tolerance parameter

The PK part contains the Michaelis-Menten term, introducing metabolic saturation.
The PD part models tolerance growth as dynamic resistance against the drug's effect.

### 2. **External Behavioral Layer (Petri net)**
The Petri net represents patient decision-making:
- node *Pain* → leads to action *Increase Dose*
- node *Relief* → leads to action *Maintain Dose*
- transition *Patient Check* activates every 12 hours (behavior cycle)

This layer triggers discrete dosing events and creates a closed feedback loop.

---

## Expected Contribution

The model will allow understanding the **counterintuitive dynamics between dose increase and nonlinear metabolism**.
The simulation will point out the risk point where the system transitions from a regulated to an uncontrolled regime.

In practice, it can serve as an **educational and research tool** for pharmacologists, doctors, and behavioral scientists studying adaptive processes in addiction.

---

## Keywords

pharmacokinetics, pharmacodynamics, Michaelis-Menten, tolerance, nonlinearity, hybrid model, SIMLIB, Petri net, overdose

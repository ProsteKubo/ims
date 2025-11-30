# Expected Behaviour of the Model

## 1. General Overview
The hybrid PK–PD–behavior model simulates the long-term interaction between
drug dynamics and patient adaptation. Each simulation step evolves through five
continuous states (A, C, P, Ce, Tol) while discrete events (drug administration)
are triggered at regular intervals by the behavioural Petri net.

The model should exhibit predictable macroscopic behaviours under controlled
conditions and then deviate sharply once nonlinearities dominate.

---

## 2. Baseline Behaviours (Expected)

### **Stage A – Stable Use (Linear Domain)**
- Dose: constant.
- Blood concentration (`C`): stable oscillations between dosing events.
- Tolerance (`Tol`): slowly increases; effect (`E`) decreases gradually.
- Effector concentration (`Ce`): tracks `C` with a small lag.
- Outcome: loss of effect, no toxicity.
- Behavioural response: patient *tends* to increase dose during future cycles.

### **Stage B – Escalation Phase (Approaching Saturation)**
- Patient increases dose according to behaviour logic:
  - If `E < threshold`, increase next dose by 10%.
- `C` begins to drift upward.
- Elimination still approximately linear, but approaching metabolic limit
  (`Vmax`).
- Tolerance rising faster, creating sustained feedback pressure to increase dose.

**Expected signature:**  
A quasi-linear growth in `Dose` and `C` before the "break point".

---

## 3. Nonlinear Transition (The Critical Point)
- Once dosing enters the range where `C > Km`, the elimination term
  `Vmax * C / (Km + C)` becomes saturated.
- Small further increases in dose yield **disproportionately larger rises in plasma concentration**.
- Within few dosing cycles, curve `C(t)` shows vertical surge → *metabolic collapse*.

**Expected observable effect:**  
`C(t)` grows exponentially while `E` initially rises sharply and then destabilises
due to both high `Tol` and approaching toxicity.

---

## 4. Stage C – Toxic Collapse
- `C` surpasses toxic threshold → defines event *overdose*.
- Patient's Petri net behaviour stops responding to feedback in time (delay).
- Model results in catastrophic overshoot and eventual equilibrium at toxic `C`
values (slow washout due to saturation).

**Clinically:** analogous to somnolence or coma.

---

## 5. Stage D – Intervention (Rescue Simulation)
Upon triggering the antidote event:
- Instantaneous reduction in either:
  - effective concentration (`Ce`) – blocking receptor binding, or
  - plasma concentration (`C`) – introducing neutralising agent.
- Due to nonlinear elimination, recovery is **asymmetric**:
  concentrations fall slowly compared to their earlier rise.

**Expected observation:**  
The system exhibits long “tail” kinetics even after antidote — consistent with
enzyme saturation and drug redistribution from tissues.

---

## 6. Petri Net Behaviour Summary
| Petri Node | Meaning | Expected Transition Condition |
|-------------|----------|-------------------------------|
| **Pain** | Patient feels pain | `E < Ethreshold` |
| **Relief** | Satisfied state | `E ≥ Ethreshold` |
| **Increase Dose** | Decision to self-escalate | From Pain → triggers `A += Dose * 1.1` |
| **Maintain Dose** | Regular schedule | From Relief → triggers `A += Dose` |
| **Toxic** | Reached toxic blood level | `C > Ctoxic` |
| **Rescue** | Antidote administration | Upon entering *Toxic* |
| **Recovery** | Returning phase | `C` decaying below safe threshold |

---

## 7. Graphical Expectations

- **Dose vs Time:** linear ramp with conditional steps.
- **C(t):** linear-growth region, then sharp knee → near-vertical rise.
- **Tol(t):** monotonic increase until saturation; continues to high values even
during toxicity.
- **E(t):** bell-shaped or collapsing pattern – initially stabilised by adaptive
dose, then collapses during nonlinear phase.

---

## 8. Simulation Termination Criteria
Simulation should stop if any of the following are true:
- `C` stays above `Ctoxic` for longer than threshold time.
- Integrator produces NaN or instability (numerical blow-up).
- Petri net enters terminal state *Death* or *Rescue Completed*.

---

## 9. Verification Checklist
- Verify that Michaelis–Menten term behaves linearly for `C << Km`.
- Check tolerance equation stability (avoid negative values).
- Confirm discrete cycle period = 12 hours for `PatientBehavior` process.
- Ensure synchronization between Petri transitions and continuous integration
steps.

---

## 10. Summary

Expected simulation pattern:

Linear phase → Slow drift → Saturation → Explosion → Slow detoxification

This progression captures the essence of the **deadly spiral** driven by
biochemical saturation and behavioural adaptation.
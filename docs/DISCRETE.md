# Discrete Subsystem – Behavioural & Petri Net Model

This file describes the **external behavioural control** that interacts with the
continuous PK–PD system.

It defines when and how the patient decides to **take, increase, or skip doses**
based on perceived effect.

---

## 1. Overview

The discrete component is modelled as a **Petri net** and an associated process
(`PatientBehavior`) that runs periodically (e.g. every 12 hours).

It reads variable `E(t)` from the continuous simulator and decides the next
action.

---

## 2. Petri Net Structure (Conceptual)

### **Places (States)**
| Name | Meaning |
|------|----------|
| `Pain` | Current effect below threshold, symptom returns |
| `Relief` | Effect sufficient, steady dosing |
| `Toxic` | Concentration above `Ctoxic`, risk zone |
| `Recovered` | After antidote, system stabilizing |

### **Transitions (Actions)**
| Name | Trigger | Action |
|------|----------|--------|
| `IncreaseDose` | `E < Ethreshold` | Next dose = last × 1.1 |
| `MaintainDose` | `E ≥ Ethreshold` | Next dose unchanged |
| `Antidote` | `C > Ctoxic` | Call antidote event |
| `CycleCheck` | Time trigger (every 12 h) | Evaluate Petri state |

### **Arcs**

Pain → IncreaseDose → Relief

Relief → MaintainDose → Pain

Relief → Toxic → Antidote → Recovered

Token movement represents the current behavioural state of the “patient.”

---

## 3. Process Logic (Pseudocode)

```text
loop every 12 hours:
    measure current effect E
    if C > Ctoxic:
        state = Toxic
        trigger Antidote()
    else if E < Ethreshold:
        Dose = Dose * 1.1
        state = Pain
    else:
        state = Relief
    apply_dose(Dose)  # A.Set(A.Value() + Dose)
end loop
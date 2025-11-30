# Visualization Plan

This document outlines how to visualize and interpret the hybrid simulation
results for the model  
**“Smrtiaca špirála: Interakcia tolerancie a metabolickej saturácie.”**

The purpose of visualization is *not only* to present numerical results, but to
**show the interplay between drug concentration, tolerance, and behaviour** in
an intuitive and dynamic way.

---

## 1. Goals of Visualization
1. Display both **internal physiological dynamics (PK–PD)** and **external
   behavioural transitions (Petri net)** within the same conceptual space.
2. Make the model comprehensible for education and presentation.
3. Allow “replay” of simulation runs with clear indications of transition points
   (e.g. break of linearity, toxic events, rescue).

---

## 2. Core Graphs (Time-Series Visualization)

Each experiment (A, B, C) should generate these standard outputs:

### **2.1 Concentration vs Time – Central Compartment (C)**
- **X-axis:** Time (hours or days)  
- **Y-axis:** Plasma concentration \( C(t) \)
- **Plotted series:**
  - `C(t)` (blue line)
  - `Ce(t)` (green dashed, delayed response)
  - Horizontal lines for `Km` and `Ctoxic`.
- **Purpose:** To show the transition from linear increase to exponential surge
  (metabolic saturation).

### **2.2 Tolerance vs Time (Tol)**
- **X-axis:** Time  
- **Y-axis:** \( Tol(t) \)
- Expected behaviour: monotonic increase; slope accelerates under frequent
  dosing.
- Overlay `E(t)` inverted, to show how tolerance suppresses effect.

### **2.3 Effect vs Dose**
- Plot measured analgesic effect \( E \) against current dose size.
- Early region: near-linear relationship.  
  After saturation: curve flattens, then collapses.
- Demonstrates decreasing returns of drug efficacy.

### **2.4 Phase Plot: C vs Dose**
- **X-axis:** Dose  
- **Y-axis:** Concentration \( C(t) \)
- Should visualize “break point” region clearly as a knee-shaped bifurcation.

### **2.5 Combined Dashboard (Optional)**
- Arrange mini-panels for A, B, and C on one page:
  - Experiment A – steady-state
  - Experiment B – escalation/overdose
  - Experiment C – rescue/recovery

---

## 3. Petri Net Visualization (Behavioural Layer)

If supported by the environment (e.g. Geogebra, yEd, or custom ReactFlow-like
visual), the Petri net should be drawn as a **simple directed bipartite graph:**

### **Nodes**
- **Places (circles):**
  - `Pain`
  - `Relief`
  - `Toxic`
  - `Recovered`
- **Transitions (rectangles):**
  - `Increase Dose`
  - `Maintain Dose`
  - `Antidote`
  - `Check Cycle`

### **Arcs**
- `Pain → Increase Dose → Relief`
- `Relief → Maintain Dose → Pain`
- `Relief → Toxic` (if `C > Ctoxic`)
- `Toxic → Antidote → Recovered`

### **Color Coding**
| State | Color | Meaning |
|--------|--------|---------|
| Pain | orange | Active pain; subthreshold effect |
| Relief | green | Therapeutic state |
| Toxic | red | Overdose region |
| Recovered | blue | Post-antidote phase |
| Transitions | gray | Behavioural logic steps |

#### Dynamic Visualization
- Token (dot) moves through states in real-time.
- Colour of `C(t)` graph background synchronised with Petri state (e.g. red tint
  during toxic state).
- Could be implemented in **Geogebra** sliders or **Python animation** using
  time-linked variable updates.

---

## 4. Combined Dynamic Scene (Concept)

For a conceptual, educational demo (e.g., in Geogebra or Processing):
- Slider for **time (t)** progresses automatically.
- Dynamic text boxes showing:
  - `Current dose`
  - `C`
  - `E`
  - `Tol`
  - `State` (Petri label)
- Graph of `C(t)` updates in sync with Petri transition animation.
- Optional indicator: warning symbol when `C > Ctoxic`.

This combination allows the viewer to *see*:
> “The patient increases dose → concentration spikes → Petri token enters Toxic.”

---

## 5. Implementation Suggestions

You can use any of these lightweight tools:

| Tool | Capability | Comment |
|-------|-------------|----------|
| **Geogebra** | Sliders, dynamic equations, event-based transitions | Ideal for educational live demos |
| **Python (Matplotlib + NetworkX)** | Time-series & Petri graph animation; reproducible scripts | Recommended for research documentation |
| **SIMLIB built-in visualization** | Real-time simulation plots | Integrates directly with model classes |
| **SVG animation** | Post-processed visualization of simulation logs | Simplest for static reports |

---

## 6. Visual Events Table

| Event | Visual Effect | Purpose |
|--------|----------------|----------|
| Dosing event | small pulse on A plot | Highlights discrete input |
| Petri transition | token moves to next state | Shows behavioural logic |
| `C > Km` | background color changes to yellow | Metabolic saturation |
| `C > Ctoxic` | background changes to red | Overdose |
| Antidote applied | flash blue | Rescue |
| Recovery | fade back to green | End phase |

---

## 7. Verification of Visual Consistency

During visualization testing, verify that:
- Colour changes correspond to correct Petri transitions.
- Graph scales remain consistent across experiments.
- Animation time matches simulation time units.
- No discontinuities appear in continuous variables due to discrete events.

---

## 8. Example Layout Sketch

+-----------------------------------------------------------+

| Time [h] →                                                |

|                                                           |

|   C(t)        /''''''''''''''''''''’’’’’’_  |

|   Ce(t)       ---- delayed ---                      _   |

|   |             |           BIFURCATION              |    |

|   v  Km--------------------------------------------->    |

|      Ctoxic------------------------------------------->  |

|                                                           |

|  Tol(t) ↑                  • escalating slope             |

|  E(t)   ↓ collapsing →__________________________________ |

+-----------------------------------------------------------+

[Pain]  →  [Increase Dose]  →  [Relief]
                  ↑                           ↓
                 [Maintain Dose] ←------------+
                           ↓
                        [Toxic] → [Antidote] → [Recovered]

---

## 9. Export & Presentation

- Export graphs in **.png** or **.svg** with consistent title style and axis
labels.
- Include visible horizontal guide lines (`Km`, `Ctoxic`).
- For dynamic presentation, export **animated GIF** or short video demonstrating
Petri net activity over time.

---

## 10. Summary

The visualization should make the system’s behaviour visually self-explanatory:

Linear → Nonlinear → Breakdown → Rescue

Viewers should *see* how a rational behavioural adaptation can drive the system
past a biochemical point of no return.
# The Deadly Spiral: Interaction of Tolerance and Metabolic Saturation

## Overview

This project models the "deadly spiral" phenomenon in long-term opioid usage, where the interaction between pharmacodynamic tolerance and pharmacokinetic metabolic saturation leads to sudden, catastrophic overdose.

The simulation combines:
- A **Continuous System** (Differential Equations) for physiological dynamics (PK/PD).
- A **Discrete System** (Petri Net) for patient behavior and decision making.

## Documentation

The project documentation is organized in the `docs/` directory:

- **[Thesis Overview](docs/THESIS.md)**: The core research question, hypothesis, and goals of the project.
- **[Model Architecture](docs/MODEL.md)**: High-level overview of the hybrid model structure.
- **[Continuous Subsystem](docs/CONTINUOUS.md)**: Details on the 5 differential equations governing the physiological dynamics.
- **[Discrete Subsystem](docs/DISCRETE.md)**: Description of the Petri net modeling patient behavior.
- **[Expected Behaviour](docs/EXPECTED_BEHAVIOUR.md)**: Analysis of the expected simulation stages (Stable, Escalation, Toxic Collapse).
- **[Research References](docs/RESEARCH.md)**: Key scientific papers and foundational research.
- **[Visualization Plan](docs/VISUALIZATION.md)**: Strategy for visualizing the simulation results.
- **[Build Instructions](docs/BUILD.md)**: How to compile and run the simulation using SIMLIB.

## Getting Started

To build and run the simulation, please refer to the **[Build Instructions](docs/BUILD.md)**.

### Quick Build
```bash
# Ensure SIMLIB is installed
make
./simulation
```

# Configuration Files Guide

## Overview

The simulation supports external configuration files, allowing for easy experiments with different parameter sets without recompiling.

## Usage

### Default Configuration
```bash
./simulation                    # Uses config.ini by default
```

### Custom Configuration
```bash
./simulation config_file.ini    # Use a specific config file
```

## Available Configurations

### 1. `config.ini` (Default)
**Standard baseline scenario** - 30 days with normal parameters
- Initial dose: 10 mg
- Tolerance: kin=0.10, kout=0.005 (moderate development)
- Metabolism: Vmax=10 mg/h, Km=2 mg/L
- Expected: Gradual tolerance buildup over 30 days

### 2. `config_aggressive.ini`
**Fast tolerance escalation** - Observe deadly spiral in 10 days
- Initial dose: 15 mg (higher starting dose)
- Tolerance: kin=0.20 (DOUBLED - builds very fast)
- Metabolism: Vmax=8 mg/h, Km=1.5 mg/L (reduced capacity)
- Lower toxicity thresholds
- Expected: Rapid tolerance → saturation → potential crisis

### 3. `config_stable.ini`
**Equilibrium without escalation** - Control scenario
- Initial dose: 8 mg (lower)
- Tolerance: kin=0.02, kout=0.01 (very slow, fast decay)
- Metabolism: Vmax=15 mg/h (higher capacity)
- Expected: Steady state, minimal tolerance accumulation

### 4. `config_poormetabolizer.ini`
**Genetic polymorphism simulation** - CYP2D6 poor metabolizer
- Metabolism: Vmax=5 mg/h (50% REDUCED capacity!)
- More sensitive to drug (EC50_base=2.5)
- Much lower toxicity thresholds
- Expected: Dangerous accumulation even with normal dosing

## Creating Your Own Configurations

Copy any existing config file and modify parameters:

```bash
cp config.ini my_experiment.ini
nano my_experiment.ini          # Edit parameters
./simulation my_experiment.ini  # Run your experiment
```

## Predicted Parameter Ranges (Safe Bounds)

| Parameter | Realistic Range | Notes |
|-----------|----------------|-------|
| ka | 0.5 - 3.0 /h | Drug absorption rate |
| Vmax | 3.0 - 20.0 mg/h | Metabolic capacity |
| Km | 0.5 - 5.0 mg/L | Saturation threshold |
| kin | 0.01 - 0.30 /h | Tolerance development |
| kout | 0.001 - 0.02 /h | Tolerance decay |
| initial_dose | 5.0 - 30.0 mg | Starting dose |

Going outside these ranges may produce unrealistic or unstable results.

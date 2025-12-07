# Naloxone Rescue Feature

## Overview

The simulation includes optional naloxone (opioid antagonist) rescue functionality. This implements **Transition T6** from the Petri net model, allowing the patient to be revived after an overdose event if naloxone is available and administered within the therapeutic window.

## Configuration

Add these parameters to your `.ini` config file under the `[TOXICITY]` section:

```ini
[TOXICITY]
C_toxic=15.0
C_critical=40.0
Effect_resp_critical=99.5

# Naloxone rescue parameters
naloxone_available=1              # 0=disabled, 1=enabled
naloxone_response_delay=0.05      # Emergency response time in hours (0.05 = 3 min)
naloxone_effective_window=0.5     # Therapeutic window in hours (0.5 = 30 min)
naloxone_blockade_strength=0.4    # Receptor blockade factor (0.4 = 40%)
```

## Parameters Explained

### `naloxone_available` (0 or 1)
- **0**: Naloxone NOT available (default) - patient dies if overdose occurs
- **1**: Naloxone available - rescue attempt made automatically on overdose detection

### `naloxone_response_delay` (hours)
- Time between overdose detection and naloxone administration (emergency response time)
- Typical values:
  - `0.05` = 3 minutes (fast EMS response)
  - `0.083` = 5 minutes (standard urban response)
  - `0.15` = 9 minutes (rural response)
  - `0.6` = 36 minutes (delayed/no bystander intervention)
- Simulates realistic emergency medical response times

### `naloxone_effective_window` (hours)
- Maximum time after overdose when naloxone can still be effective
- Typical values: 
  - `0.25` = 15 minutes (narrow window)
  - `0.5` = 30 minutes (standard clinical window)
  - `1.0` = 1 hour (extended window, rare)
- Based on clinical data: rescue must occur before irreversible brain damage
- **Critical**: `response_delay` must be < `effective_window` for rescue to succeed

### `naloxone_blockade_strength` (0.0 to 1.0)
- Fraction of opioid receptors blocked by naloxone
- Typical values:
  - `0.3` = Partial blockade (30%)
  - `0.4` = Standard dose (40%)
  - `0.6` = High dose (60%)
- Higher values = more aggressive reversal but also more severe withdrawal

## Pharmacology of Naloxone Rescue

When naloxone is administered, the following effects occur:

### 1. **Competitive Receptor Antagonism**
```
C_after = C_before × (1 - blockade_strength)
```
Central compartment concentration effectively reduced as naloxone displaces opioid from μ-receptors.

### 2. **Effect Site Suppression**
```
Ce_after = Ce_before × 0.1
```
Effect site concentration drops rapidly (90% reduction) due to high naloxone affinity at the receptor site.

### 3. **Partial Tolerance Reset**
```
Tol_after = Tol_before × 0.7
```
Some tolerance reversal occurs (30% reduction) as μ-receptor upregulation is partially reversed.

### 4. **Acute Withdrawal Syndrome**
After rescue:
- Pain level → 3 (Severe)
- Relief state → False
- Motivation → 3.0 (high urgency to re-dose)

The patient is **alive but in severe distress**, requiring ICU monitoring to prevent immediate re-dosing and subsequent re-overdose.

## Clinical Scenarios

### Scenario A: No Naloxone (Default Behavior)
```ini
naloxone_available=0
```
- Patient overdoses → simulation terminates immediately
- Status: DECEASED
- This is the default "deadly spiral" outcome

### Scenario B: Successful Rescue (Fast Response)
```ini
naloxone_available=1
naloxone_response_delay=0.05     # 3 minutes (fast EMS)
naloxone_effective_window=0.5    # 30 minutes window
naloxone_blockade_strength=0.4
```
- Patient overdoses → emergency response dispatched
- Naloxone arrives in 3 minutes (within 30 min window)
- Status: ALIVE (revived)
- Withdrawal symptoms begin
- Risk of re-overdose if patient re-doses

### Scenario C: Failed Rescue (Delayed Response)
```ini
naloxone_available=1
naloxone_response_delay=0.6      # 36 minutes (too slow!)
naloxone_effective_window=0.5    # 30 minutes window
```
- Patient overdoses → emergency response dispatched
- Naloxone arrives in 36 minutes (exceeds 30 min window)
- Window expired → rescue fails
- Status: DECEASED
- Demonstrates critical importance of rapid response

## Example Output

When naloxone rescue succeeds:

```
!!! RESPIRATORY ARREST at t=49.00 hours !!!
    Respiratory depression = 99.76%

>>> EMERGENCY RESPONSE DISPATCHED (ETA: 3.00 minutes) <<<

>>> NALOXONE RESCUE TEAM ARRIVED at t=49.05 hours <<<
Time since overdose: 3.00 minutes

╔═══════════════════════════════════════════════════════════╗
║ T6: NALOXONE RESCUE ACTIVATED                            ║
╚═══════════════════════════════════════════════════════════╝
C(t): 15.80 → 9.48 mg/L
Ce(t): 5.08 → 0.51 mg/L
Tol(t): 0.34 → 0.24

Patient REVIVED but experiencing ACUTE WITHDRAWAL
Status: ALIVE but in severe distress
Requires: ICU monitoring, serial naloxone dosing
Risk: Re-overdose in 1-2 hours if opioid still circulating

>>> RESCUE SUCCESSFUL - Patient REVIVED <<<
```

When naloxone rescue fails (delayed response):

```
!!! RESPIRATORY ARREST at t=38.00 hours !!!
    Respiratory depression = 99.81%

>>> EMERGENCY RESPONSE DISPATCHED (ETA: 36.00 minutes) <<<

>>> NALOXONE RESCUE TEAM ARRIVED at t=38.60 hours <<<
Time since overdose: 36.00 minutes

!!! NALOXONE WINDOW EXPIRED (>30.00 min) - RESCUE FAILED !!!
Patient Status: DECEASED
Cause: Response time (36.00 min) exceeded therapeutic window
```

## Testing

A pre-configured test file is available:

```bash
./sim models/config_naloxone_test.ini
```

This configuration has naloxone enabled with realistic parameters for testing the rescue mechanism.

## Implementation Notes

- Naloxone rescue is triggered automatically in `StatusMonitor` when overdose is detected
- The `CheckAndApplyNaloxone()` method is called from the monitoring loop
- If rescue is successful, the simulation continues (patient alive)
- If rescue fails or naloxone is not available, simulation terminates
- All existing configs without naloxone parameters work unchanged (backwards compatible)

## References

From RESEARCH.md Section II.C:
- **Transition T6: NALOXONE_RESCUE** implements competitive antagonism
- Naloxone half-life: ~30-90 minutes (shorter than most opioids)
- Risk of "re-narcotization" when naloxone wears off before the opioid
- Serial dosing often required in clinical practice

## Future Extensions

Potential enhancements:
- Multiple naloxone doses (serial administration)
- Naloxone pharmacokinetics (instead of instant effect)
- Different naloxone formulations (IV vs intranasal)
- Automated redosing if patient re-overdoses

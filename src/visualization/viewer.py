#!/usr/bin/env python3
import re
from pathlib import Path

import matplotlib.pyplot as plt
import pandas as pd
import numpy as np

ROOT = Path(__file__).resolve().parents[1]  # repo root
OUT_PATH = ROOT / "../out.txt"


# ---------- Parsing ----------

line_re = re.compile(
    r"t=\s*(?P<t>[\d\.]+)h\s*\|\s*A=\s*(?P<A>[-\d\.]+)\s*mg\s*\|\s*C=\s*(?P<C>[-\d\.]+)\s*mg/L\s*\|\s*P=\s*(?P<P>[-\d\.]+)\s*mg/L\s*\|\s*Ce=\s*(?P<Ce>[-\d\.]+)\s*mg/L\s*\|\s*Tol=\s*(?P<Tol>[-\d\.]+)\s*\|\s*Effect=\s*(?P<Effect>[-\d\.]+)%"
)

assessment_start_re = re.compile(r"^=+ PATIENT ASSESSMENT at t=(?P<t>[\d\.]+) hours =+")
current_effect_re = re.compile(r"^Current Effect:\s*([\d\.]+)%")
pain_level_re = re.compile(r"^Pain Level:\s*(\d+)")
relief_state_re = re.compile(r"^Relief State:\s*(YES|NO)")
motivation_re = re.compile(r"^Motivation:\s*([\d\.]+)")
current_dose_re = re.compile(r"^Current Dose:\s*([\d\.]+)\s*mg")
decision_increase_re = re.compile(r"^>>> DECISION: INCREASE DOSE")
decision_maintain_re = re.compile(r"^>>> DECISION: MAINTAIN CURRENT DOSE")
decision_stable_re = re.compile(r"^Decision:\s*STABLE")
dose_admin_re = re.compile(r"^--- DOSE ADMINISTERED ---")
dose_time_re = re.compile(r"^Time:\s*([\d\.]+)\s*h")
dose_amt_re = re.compile(r"^Dose:\s*([\d\.]+)\s*mg")
naloxone_rescue_re = re.compile(r"^>>> (ATTEMPTING NALOXONE RESCUE|RESCUE SUCCESSFUL|RESCUE FAILED)")
phase_transition_re = re.compile(r"PHASE TRANSITION: (SATURATION ZONE ENTERED|CATASTROPHIC ZONE)")
critical_overdose_re = re.compile(r"^!!! CRITICAL OVERDOSE at t=([\d\.]+) hours !!!")
respiratory_arrest_re = re.compile(r"^!!! RESPIRATORY ARREST at t=([\d\.]+) hours !!!")
toxic_warning_re = re.compile(r"^>>> WARNING: Toxic concentration reached at t=([\d\.]+) hours <<<")


def parse_out(path: Path):
    """Parse out.txt into comprehensive data structures."""
    times = []
    rows = []

    assessments = []
    current_assessment = None

    doses = []
    current_dose_block = None

    # New: track critical events
    naloxone_events = []
    phase_transitions = []
    critical_events = []

    with path.open() as f:
        for raw in f:
            line = raw.rstrip("\n")

            # Continuous line: t= ... | A= ... | C= ... | ...
            m = line_re.search(line)
            if m:
                d = m.groupdict()
                t = float(d["t"])
                times.append(t)
                rows.append(
                    {
                        "t": t,
                        "A": float(d["A"]),
                        "C": float(d["C"]),
                        "P": float(d["P"]),
                        "Ce": float(d["Ce"]),
                        "Tol": float(d["Tol"]),
                        "Effect": float(d["Effect"]),
                    }
                )
                continue

            # Assessment blocks
            m = assessment_start_re.match(line)
            if m:
                # close previous
                if current_assessment is not None:
                    assessments.append(current_assessment)
                t = float(m.group("t"))
                current_assessment = {
                    "t": t,
                    "effect": None,
                    "pain_level": None,
                    "relief_state": None,
                    "motivation": None,
                    "current_dose": None,
                    "decision": None,
                }
                continue

            if current_assessment is not None:
                m = current_effect_re.match(line)
                if m:
                    current_assessment["effect"] = float(m.group(1))
                    continue
                m = pain_level_re.match(line)
                if m:
                    current_assessment["pain_level"] = int(m.group(1))
                    continue
                m = relief_state_re.match(line)
                if m:
                    current_assessment["relief_state"] = 1 if m.group(1) == "YES" else 0
                    continue
                m = motivation_re.match(line)
                if m:
                    current_assessment["motivation"] = float(m.group(1))
                    continue
                m = current_dose_re.match(line)
                if m:
                    current_assessment["current_dose"] = float(m.group(1))
                    continue
                if decision_increase_re.match(line):
                    current_assessment["decision"] = "INCREASE"
                    continue
                if decision_maintain_re.match(line):
                    current_assessment["decision"] = "MAINTAIN"
                    continue
                if decision_stable_re.match(line):
                    current_assessment["decision"] = "STABLE"
                    continue

            # Dose administered blocks
            if dose_admin_re.match(line):
                if current_dose_block is not None:
                    doses.append(current_dose_block)
                current_dose_block = {
                    "time": None,
                    "dose": None,
                }
                continue

            if current_dose_block is not None:
                m = dose_time_re.match(line)
                if m:
                    current_dose_block["time"] = float(m.group(1))
                    continue
                m = dose_amt_re.match(line)
                if m:
                    current_dose_block["dose"] = float(m.group(1))
                    continue

                # heuristic: if we hit an empty separator, close dose block
                if not line.strip() and current_dose_block["time"] is not None:
                    doses.append(current_dose_block)
                    current_dose_block = None

            # Naloxone rescue events
            m = naloxone_rescue_re.match(line)
            if m:
                # Estimate time from last continuous data point
                t_approx = times[-1] if times else 0.0
                naloxone_events.append({
                    "time": t_approx,
                    "event": m.group(1)
                })
                continue

            # Phase transitions
            m = phase_transition_re.search(line)
            if m:
                t_approx = times[-1] if times else 0.0
                phase_transitions.append({
                    "time": t_approx,
                    "phase": m.group(1)
                })
                continue

            # Critical events
            m = critical_overdose_re.match(line)
            if m:
                critical_events.append({
                    "time": float(m.group(1)),
                    "type": "CRITICAL_OVERDOSE"
                })
                continue

            m = respiratory_arrest_re.match(line)
            if m:
                critical_events.append({
                    "time": float(m.group(1)),
                    "type": "RESPIRATORY_ARREST"
                })
                continue

            m = toxic_warning_re.match(line)
            if m:
                critical_events.append({
                    "time": float(m.group(1)),
                    "type": "TOXIC_WARNING"
                })
                continue

    # close last blocks
    if current_assessment is not None:
        assessments.append(current_assessment)
    if current_dose_block is not None:
        doses.append(current_dose_block)

    continuous_df = pd.DataFrame(rows).sort_values("t") if rows else pd.DataFrame()
    assessments_df = pd.DataFrame(assessments).sort_values("t") if assessments else pd.DataFrame()
    doses_df = pd.DataFrame(doses).sort_values("time") if doses else pd.DataFrame()
    naloxone_df = pd.DataFrame(naloxone_events)
    phase_df = pd.DataFrame(phase_transitions)
    critical_df = pd.DataFrame(critical_events)

    return continuous_df, assessments_df, doses_df, naloxone_df, phase_df, critical_df


# ---------- Plotting ----------

def plot_overview(continuous_df, assessments_df, doses_df, naloxone_df, phase_df, critical_df):
    """Create comprehensive 6-panel visualization of simulation data."""
    fig, axes = plt.subplots(6, 1, figsize=(18, 16), sharex=True)
    
    if continuous_df.empty:
        print("WARNING: No continuous data to plot")
        return fig

    # 1) Absorption Compartment (A) - shows dose stacking
    ax = axes[0]
    ax.plot(continuous_df["t"], continuous_df["A"], label="A (absorption)", color="tab:cyan", linewidth=2)
    ax.set_ylabel("Amount in GI [mg]", fontsize=11, fontweight='bold')
    ax.legend(loc="upper right")
    ax.set_title("Gastrointestinal Absorption Compartment", fontsize=12, fontweight='bold')
    ax.grid(True, alpha=0.3)
    
    # Mark doses on absorption plot
    if not doses_df.empty:
        ax.scatter(doses_df["time"], [0.5]*len(doses_df), marker='v', s=80, 
                  color='red', alpha=0.7, label='Dose administered', zorder=5)

    # 2) PK: Central (C), Peripheral (P), and Effect-site (Ce)
    ax = axes[1]
    ax.plot(continuous_df["t"], continuous_df["C"], label="C (blood)", color="tab:blue", linewidth=2)
    ax.plot(continuous_df["t"], continuous_df["P"], label="P (peripheral)", color="tab:purple", linewidth=2, alpha=0.7)
    ax.plot(continuous_df["t"], continuous_df["Ce"], label="Ce (effect-site)", color="tab:orange", linewidth=2)
    ax.set_ylabel("Concentration [mg/L]", fontsize=11, fontweight='bold')
    ax.legend(loc="upper right")
    ax.set_title("Pharmacokinetics: Multi-Compartment Distribution", fontsize=12, fontweight='bold')
    ax.grid(True, alpha=0.3)
    
    # Mark phase transitions
    if not phase_df.empty:
        for _, event in phase_df.iterrows():
            color = 'orange' if 'SATURATION' in event['phase'] else 'red'
            ax.axvline(event['time'], color=color, linestyle='--', alpha=0.5, linewidth=2)
            ax.text(event['time'], ax.get_ylim()[1]*0.9, event['phase'].split()[0], 
                   rotation=90, fontsize=8, alpha=0.7)

    # 3) Tolerance Development
    ax = axes[2]
    ax.plot(continuous_df["t"], continuous_df["Tol"], label="Tolerance", color="tab:red", linewidth=2)
    ax.set_ylabel("Tolerance [a.u.]", fontsize=11, fontweight='bold')
    ax.legend(loc="upper left")
    ax.set_title("Tolerance Development (Receptor Desensitization)", fontsize=12, fontweight='bold')
    ax.grid(True, alpha=0.3)
    
    # Highlight tolerance growth regions
    if len(continuous_df) > 1:
        tol_gradient = np.gradient(continuous_df["Tol"].values)
        high_growth = tol_gradient > np.percentile(tol_gradient, 75)
        ax.fill_between(continuous_df["t"], 0, continuous_df["Tol"], 
                        where=high_growth, alpha=0.2, color='red', 
                        label='Rapid tolerance growth')

    # 4) Effect and Pain with Motivation overlay
    ax = axes[3]
    ax.plot(continuous_df["t"], continuous_df["Effect"], label="Effect (%)", 
           color="tab:green", linewidth=2.5)
    
    # Add therapeutic window shading
    ax.axhspan(60, 80, alpha=0.15, color='green', label='Therapeutic window')
    ax.axhspan(40, 60, alpha=0.1, color='yellow', label='Subtherapeutic')
    ax.axhspan(0, 40, alpha=0.15, color='red', label='Severe pain')

    if not assessments_df.empty:
        # Effect at assessments
        ax.scatter(assessments_df["t"], assessments_df["effect"], 
                  color="darkgreen", s=50, marker='o', 
                  label="Assessment effect", zorder=5, edgecolors='black', linewidth=1)
        
        # Pain level on secondary axis
        ax2 = ax.twinx()
        pain_colors = {0: 'green', 1: 'yellow', 2: 'orange', 3: 'red'}
        for pain_val in range(4):
            pain_subset = assessments_df[assessments_df['pain_level'] == pain_val]
            if not pain_subset.empty:
                ax2.scatter(pain_subset["t"], pain_subset["pain_level"],
                           color=pain_colors[pain_val], s=120, marker='s',
                           alpha=0.7, label=f'Pain level {pain_val}', zorder=4)
        
        ax2.set_ylabel("Pain Level (0-3)", fontsize=11, fontweight='bold')
        ax2.set_ylim(-0.3, 3.3)
        ax2.legend(loc="upper right", fontsize=8)
        
        # Motivation on third axis
        ax3 = ax.twinx()
        ax3.spines['right'].set_position(('outward', 60))
        ax3.plot(assessments_df["t"], assessments_df["motivation"], 
                color='purple', marker='d', linestyle='--', linewidth=2, 
                markersize=6, label='Motivation', alpha=0.7)
        ax3.set_ylabel("Motivation", fontsize=10, color='purple')
        ax3.tick_params(axis='y', labelcolor='purple')
        ax3.set_ylim(0, max(assessments_df["motivation"].max() * 1.1, 5))

    ax.set_ylabel("Effect [%]", fontsize=11, fontweight='bold')
    ax.set_title("Pharmacodynamics: Effect, Pain, & Motivation", fontsize=12, fontweight='bold')
    ax.legend(loc="upper left", fontsize=8)
    ax.grid(True, alpha=0.3)

    # 5) Dose Escalation History
    ax = axes[4]
    if not doses_df.empty:
        # Step plot of dose history
        ax.step(doses_df["time"], doses_df["dose"], where="post", 
               label="Dose [mg]", color="tab:brown", linewidth=2.5)
        
        # Mark each dose with value annotation
        for idx, row in doses_df.iterrows():
            if idx % 2 == 0:  # Annotate every other dose to avoid clutter
                ax.annotate(f'{row["dose"]:.1f}', 
                           xy=(row["time"], row["dose"]),
                           xytext=(5, 5), textcoords='offset points',
                           fontsize=8, alpha=0.7)
        
        # Calculate escalation rate
        if len(doses_df) > 1:
            initial_dose = doses_df.iloc[0]["dose"]
            final_dose = doses_df.iloc[-1]["dose"]
            escalation_pct = ((final_dose / initial_dose) - 1) * 100
            ax.text(0.02, 0.98, f'Total escalation: {escalation_pct:.1f}%',
                   transform=ax.transAxes, fontsize=10, verticalalignment='top',
                   bbox=dict(boxstyle='round', facecolor='wheat', alpha=0.5))
    
    if not assessments_df.empty:
        # Mark decision types
        increase_decisions = assessments_df[assessments_df["decision"] == "INCREASE"]
        maintain_decisions = assessments_df[assessments_df["decision"] == "MAINTAIN"]
        
        if not increase_decisions.empty:
            y_pos = ax.get_ylim()[1] * 0.95
            ax.scatter(increase_decisions["t"], [y_pos]*len(increase_decisions),
                      marker="^", s=150, color="red", label="INCREASE decision",
                      zorder=5, edgecolors='darkred', linewidth=1.5)
        
        if not maintain_decisions.empty:
            y_pos = ax.get_ylim()[1] * 0.85
            ax.scatter(maintain_decisions["t"], [y_pos]*len(maintain_decisions),
                      marker="s", s=100, color="green", label="MAINTAIN decision",
                      zorder=5, alpha=0.7)

    ax.set_ylabel("Dose [mg]", fontsize=11, fontweight='bold')
    ax.set_title("Dose Escalation & Decision Events", fontsize=12, fontweight='bold')
    ax.legend(loc="upper left")
    ax.grid(True, alpha=0.3)

    # 6) Critical Events Timeline
    ax = axes[5]
    
    # Create event timeline visualization
    event_types = []
    event_times = []
    event_colors = []
    event_labels = []
    
    # Naloxone rescues
    if not naloxone_df.empty:
        for _, event in naloxone_df.iterrows():
            event_times.append(event['time'])
            if 'SUCCESSFUL' in event['event']:
                event_types.append(2)
                event_colors.append('green')
                event_labels.append('Naloxone rescue')
            elif 'FAILED' in event['event']:
                event_types.append(2)
                event_colors.append('darkred')
                event_labels.append('Rescue failed')
            else:
                event_types.append(2)
                event_colors.append('orange')
                event_labels.append('Naloxone attempt')
    
    # Critical events
    if not critical_df.empty:
        for _, event in critical_df.iterrows():
            event_times.append(event['time'])
            if event['type'] == 'CRITICAL_OVERDOSE':
                event_types.append(3)
                event_colors.append('red')
                event_labels.append('Critical overdose')
            elif event['type'] == 'RESPIRATORY_ARREST':
                event_types.append(3)
                event_colors.append('darkred')
                event_labels.append('Respiratory arrest')
            else:
                event_types.append(1)
                event_colors.append('orange')
                event_labels.append('Toxic warning')
    
    if event_times:
        scatter = ax.scatter(event_times, event_types, c=event_colors, 
                           s=200, marker='X', edgecolors='black', linewidth=1.5,
                           zorder=5, alpha=0.8)
        
        # Annotate events
        for t, y, label in zip(event_times, event_types, event_labels):
            ax.annotate(label, xy=(t, y), xytext=(0, 10), 
                       textcoords='offset points', fontsize=8,
                       ha='center', alpha=0.8, rotation=45)
    
    ax.set_ylim(0, 4)
    ax.set_yticks([1, 2, 3])
    ax.set_yticklabels(['Warnings', 'Rescues', 'Critical'])
    ax.set_ylabel("Event Type", fontsize=11, fontweight='bold')
    ax.set_xlabel("Time [h]", fontsize=12, fontweight='bold')
    ax.set_title("Critical Events Timeline", fontsize=12, fontweight='bold')
    ax.grid(True, alpha=0.3, axis='x')
    
    # Add summary text
    if event_times:
        summary_text = f'Total critical events: {len(event_times)}'
        if not naloxone_df.empty:
            rescues = len(naloxone_df[naloxone_df['event'].str.contains('SUCCESSFUL')])
            summary_text += f'\nSuccessful rescues: {rescues}'
        ax.text(0.02, 0.98, summary_text,
               transform=ax.transAxes, fontsize=9, verticalalignment='top',
               bbox=dict(boxstyle='round', facecolor='lightcoral', alpha=0.7))

    plt.tight_layout()
    return fig


def main():
    if not OUT_PATH.exists():
        raise SystemExit(f"out.txt not found at {OUT_PATH}")
    
    print("Parsing simulation output...")
    continuous_df, assessments_df, doses_df, naloxone_df, phase_df, critical_df = parse_out(OUT_PATH)

    print(f"📊 Data Summary:")
    print(f"  • {len(continuous_df)} continuous time points")
    print(f"  • {len(assessments_df)} patient assessments")
    print(f"  • {len(doses_df)} dose administrations")
    print(f"  • {len(naloxone_df)} naloxone events")
    print(f"  • {len(phase_df)} phase transitions")
    print(f"  • {len(critical_df)} critical events")
    print()

    if not continuous_df.empty:
        print(f"📈 Simulation Range: {continuous_df['t'].min():.1f} - {continuous_df['t'].max():.1f} hours")
        print(f"   Peak C: {continuous_df['C'].max():.2f} mg/L at t={continuous_df.loc[continuous_df['C'].idxmax(), 't']:.1f}h")
        print(f"   Peak Effect: {continuous_df['Effect'].max():.1f}% at t={continuous_df.loc[continuous_df['Effect'].idxmax(), 't']:.1f}h")
        print(f"   Final Tolerance: {continuous_df['Tol'].iloc[-1]:.4f}")
        print()
    
    if not doses_df.empty:
        initial_dose = doses_df.iloc[0]["dose"]
        final_dose = doses_df.iloc[-1]["dose"]
        escalation = ((final_dose / initial_dose) - 1) * 100
        print(f"💊 Dose Escalation: {initial_dose:.2f} mg → {final_dose:.2f} mg (+{escalation:.1f}%)")
        print()

    print("Creating comprehensive visualization...")
    fig = plot_overview(continuous_df, assessments_df, doses_df, naloxone_df, phase_df, critical_df)

    # Save outputs
    fig.savefig("ims_overview.png", dpi=200, bbox_inches="tight")
    fig.savefig("ims_overview.pdf", bbox_inches="tight")
    print("✅ Saved visualization to:")
    print("   • ims_overview.png")
    print("   • ims_overview.pdf")
    
    # Optional: save parsed data to CSV for further analysis
    if not continuous_df.empty:
        continuous_df.to_csv("ims_continuous_data.csv", index=False)
        print("   • ims_continuous_data.csv")
    if not assessments_df.empty:
        assessments_df.to_csv("ims_assessments.csv", index=False)
        print("   • ims_assessments.csv")
    if not doses_df.empty:
        doses_df.to_csv("ims_doses.csv", index=False)
        print("   • ims_doses.csv")


if __name__ == "__main__":
    main()
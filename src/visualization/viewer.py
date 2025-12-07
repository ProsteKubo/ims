#!/usr/bin/env python3
import re
from pathlib import Path

import matplotlib.pyplot as plt
import pandas as pd

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


def parse_out(path: Path):
    """Parse out.txt into:
    - continuous_df: rows for each t=... line
    - assessments_df: one row per assessment block
    - doses_df: one row per 'DOSE ADMINISTERED' block
    """
    times = []
    rows = []

    assessments = []
    current_assessment = None

    doses = []
    current_dose_block = None

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
                    # keep generic "STABLE"
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

    # close last blocks
    if current_assessment is not None:
        assessments.append(current_assessment)
    if current_dose_block is not None:
        doses.append(current_dose_block)

    continuous_df = pd.DataFrame(rows).sort_values("t")
    assessments_df = pd.DataFrame(assessments).sort_values("t") if assessments else pd.DataFrame()
    doses_df = pd.DataFrame(doses).sort_values("time") if doses else pd.DataFrame()

    return continuous_df, assessments_df, doses_df


# ---------- Plotting ----------

def plot_overview(continuous_df, assessments_df, doses_df):
    fig, axes = plt.subplots(4, 1, figsize=(16, 12), sharex=True)

    # 1) PK: C and Ce
    ax = axes[0]
    ax.plot(continuous_df["t"], continuous_df["C"], label="C (blood)", color="tab:blue")
    ax.plot(continuous_df["t"], continuous_df["Ce"], label="Ce (effect-site)", color="tab:orange")
    ax.set_ylabel("Concentration [mg/L]")
    ax.legend(loc="upper right")
    ax.set_title("PK / Effect-site")

    # 2) Tolerance
    ax = axes[1]
    ax.plot(continuous_df["t"], continuous_df["Tol"], label="Tol", color="tab:red")
    ax.set_ylabel("Tolerance [a.u.]")
    ax.legend(loc="upper left")
    ax.set_title("Tolerance")

    # 3) Effect and pain level at assessments
    ax = axes[2]
    ax.plot(continuous_df["t"], continuous_df["Effect"], label="Effect (%)", color="tab:green")

    if not assessments_df.empty:
        ax.scatter(
            assessments_df["t"],
            assessments_df["effect"],
            color="black",
            s=20,
            label="Effect at assessment",
        )
        # Plot pain level on secondary axis (0–3 discrete)
        ax2 = ax.twinx()
        ax2.step(
            assessments_df["t"],
            assessments_df["pain_level"],
            where="post",
            color="tab:purple",
            label="Pain level (0–3)",
        )
        ax2.set_ylabel("Pain level")
        ax2.set_ylim(-0.2, 3.2)

        # Combine legends
        lines1, labels1 = ax.get_legend_handles_labels()
        lines2, labels2 = ax2.get_legend_handles_labels()
        ax2.legend(lines1 + lines2, labels1 + labels2, loc="upper right")
    else:
        ax.legend(loc="upper right")
    ax.set_ylabel("Effect [%]")
    ax.set_title("Effect & Pain")

    # 4) Dose history
    ax = axes[3]
    if not doses_df.empty:
        ax.step(
            doses_df["time"],
            doses_df["dose"],
            where="post",
            label="Dose [mg]",
            color="tab:brown",
        )
    if not assessments_df.empty:
        # Mark INCREASE decisions
        inc = assessments_df[assessments_df["decision"] == "INCREASE"]
        ax.scatter(
            inc["t"],
            [ax.get_ylim()[1] * 0.9] * len(inc),
            marker="^",
            color="red",
            label="INCREASE decisions",
        )

    ax.set_ylabel("Dose [mg]")
    ax.set_xlabel("Time [h]")
    ax.set_title("Dosing & Decisions")
    ax.legend(loc="upper left")

    plt.tight_layout()
    return fig


def main():
    if not OUT_PATH.exists():
        raise SystemExit(f"out.txt not found at {OUT_PATH}")
    continuous_df, assessments_df, doses_df = parse_out(OUT_PATH)

    print(f"Parsed {len(continuous_df)} time points,"
          f" {len(assessments_df)} assessments,"
          f" {len(doses_df)} doses.")

    fig = plot_overview(continuous_df, assessments_df, doses_df)

    fig.savefig("ims_overview.png", dpi=200, bbox_inches="tight")
    fig.savefig("ims_overview.pdf", bbox_inches="tight")
    print("Saved visualization to ims_overview.png and ims_overview.pdf")


if __name__ == "__main__":
    main()
#!/usr/bin/env python3
import argparse
import csv
import math
from pathlib import Path
import matplotlib.pyplot as plt

def to_float(x):
    try:
        v = float(x)
        return None if math.isnan(v) else v
    except:
        return None

def prettify(col):
    return (col.replace("metric_", "")
               .replace("cfg_", "")
               .replace("_", " ")
               .title())

def load_rows(path):
    with open(path, newline="", encoding="utf-8") as f:
        return list(csv.DictReader(f))

def main():
    parser = argparse.ArgumentParser(
        description="Scatter + Pareto frontier plot for any two metrics"
    )
    parser.add_argument("--csv",    default="results/trials.csv",
                        help="CSV with trial metrics")
    parser.add_argument("--out",    default="results/pareto.png",
                        help="Output image file")
    parser.add_argument("--x",      default="metric_energy_total_J",
                        help="Column name for X axis (to minimize)")
    parser.add_argument("--y",      default="metric_rougeL",
                        help="Column name for Y axis (to maximize)")
    parser.add_argument("--label",  default="cfg_brevity",
                        help="Column to use for point labels")
    parser.add_argument("--title",  default="Accuracy vs Energy (Pareto)",
                        help="Plot title")
    args = parser.parse_args()

    rows = load_rows(args.csv)
    # Filter valid
    pts = []
    for r in rows:
        x = to_float(r.get(args.x)); y = to_float(r.get(args.y))
        lab = r.get(args.label, "")
        if x is None or y is None: continue
        pts.append((x, y, lab))
    if not pts:
        print("No valid data points.")
        return

    # Compute Pareto: best Y for each unique X
    best_for_x = {}
    for x, y, lab in pts:
        if x not in best_for_x or y > best_for_x[x][0]:
            best_for_x[x] = (y, lab)
    # Sort by X
    candidate = sorted(((x, ylab[0], ylab[1]) for x, ylab in best_for_x.items()), key=lambda t: t[0])
    # Envelope: monotonic non-decreasing Y
    envelope = []
    max_y = -math.inf
    for x, y, lab in candidate:
        if y >= max_y:
            envelope.append((x, y, lab))
            max_y = y

    # Plot all
    xs_all, ys_all, labs_all = zip(*pts)
    plt.figure(figsize=(10, 7))
    plt.scatter(xs_all, ys_all, s=60, alpha=0.6, label="All Trials")
    for x, y, lab in pts:
        plt.annotate(lab, (x, y), fontsize=8, xytext=(3,3), textcoords='offset points')

    # Plot frontier
    fx, fy, fl = zip(*envelope)
    plt.plot(fx, fy, color='C1', lw=2, label='Pareto Frontier')
    plt.scatter(fx, fy, s=120, color='C1', edgecolor='k', linewidth=1.2)
    for x, y, lab in envelope:
        plt.annotate(lab, (x, y), fontsize=9, fontweight='bold',
                     xytext=(3,6), textcoords='offset points', color='C1')

    plt.title(args.title)
    plt.xlabel(prettify(args.x))
    plt.ylabel(prettify(args.y))
    plt.grid(True, linestyle='--', alpha=0.3)
    plt.legend()

    outp = Path(args.out)
    outp.parent.mkdir(parents=True, exist_ok=True)
    plt.tight_layout()
    plt.savefig(outp, dpi=180)
    print(f"Saved Pareto plot to {outp.resolve()}")

if __name__ == "__main__":
    main()
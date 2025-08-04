#!/usr/bin/env bash
set -euo pipefail

# Paths (adjust as needed)
CONFIG="config.json"
PROMPT='{"style":"role","reasoning":"bounded","format":"bullets","brevity":"word50"}'

# 1) Run Optuna search (20 trials) and collect metrics
echo "Running prompt search..."
./eapo_cpp \
  --config examples/$CONFIG \
  --mode search \
  --trials 20

# 2) Evaluate the best prompt on the full dataset
echo "Evaluating best prompt..."
./eapo_cpp \
  --config examples/$CONFIG \
  --mode evaluate \
  --prompt "$PROMPT"

# 3) Generate Pareto plot (accuracy vs energy)
echo "Plotting Pareto curve..."
python3 scripts/plot_pareto.py \
  --csv results/trials.csv \
  --out results/pareto_acc_vs_energy.png \
  --x metric_energy_total_J \
  --y metric_rougeL \
  --label cfg_brevity \
  --show_frontier \
  --eps_y 0.001

echo "All done! Results in the 'results/' folder."

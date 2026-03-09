#!/usr/bin/env python3
import csv
import os
import sys
import matplotlib.pyplot as plt

report_dir = sys.argv[1] if len(sys.argv) > 1 else "reports/latest"

def read_csv(path, key_col, val_col):
    rows = []
    with open(path, newline="") as f:
        reader = csv.DictReader(f)
        for row in reader:
            rows.append((row[key_col], int(row[val_col])))
    return rows

util = read_csv(os.path.join(report_dir, "utilization.csv"), "stage", "busy_cycles")
stalls = read_csv(os.path.join(report_dir, "stalls.csv"), "stall_reason", "count")

if util:
    plt.figure(figsize=(8, 4))
    plt.bar([k for k, _ in util], [v for _, v in util])
    plt.title("Pipeline Utilization")
    plt.ylabel("Busy Cycles")
    plt.tight_layout()
    plt.savefig(os.path.join(report_dir, "pipeline_utilization.png"))
    plt.close()

if stalls:
    plt.figure(figsize=(8, 4))
    plt.bar([k for k, _ in stalls], [v for _, v in stalls])
    plt.title("Stall Breakdown")
    plt.ylabel("Count")
    plt.tight_layout()
    plt.savefig(os.path.join(report_dir, "stall_breakdown.png"))
    plt.close()

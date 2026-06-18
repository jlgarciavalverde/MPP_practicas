#!/usr/bin/env python3
"""
Parse cudaTemplate run output files and compute mean processing time per configuration.

This script searches a directory for files matching:
 - run*.txt      (regular runs)
 - run*-3d.txt   (3D runs)

For each group it parses lines to find the command line (starting with './cudaTemplate')
and the following 'Processing Time: X (ms)'. It groups by the command line (config)
and computes mean, stddev and count across the runs found.

Outputs CSV files in the same directory:
 - summary_cudaTemplate_run_mean.csv     (for run*.txt)
 - summary_cudaTemplate_run3d_mean.csv   (for run*-3d.txt)

Usage:
  python parse_cuda_template.py <dir>

Example:
  python parse_cuda_template.py Codigo/cudaTemplate

"""
import sys
import os
import glob
import csv
from statistics import mean, stdev
import re

CMD_RE = re.compile(r"^\s*\.?/?\.?/.*cudaTemplate\s+(.*)$")
TIME_RE = re.compile(r"Processing Time:\s*([0-9]+(?:\.[0-9]+)?)\s*\(ms\)")


def parse_file(path):
    """Return list of (config_string, time_ms) entries found in file."""
    entries = []
    current_cmd = None
    try:
        with open(path, 'r', encoding='utf-8', errors='ignore') as f:
            for line in f:
                line = line.strip()
                if not line:
                    continue
                m = CMD_RE.match(line)
                if m:
                    # capture the part after the program name, use as config key
                    current_cmd = m.group(1).strip()
                    continue
                t = TIME_RE.search(line)
                if t and current_cmd is not None:
                    time_ms = float(t.group(1))
                    entries.append((current_cmd, time_ms))
                    current_cmd = None
    except FileNotFoundError:
        print(f"Warning: file not found: {path}")
    return entries


def aggregate_from_files(file_list):
    # map config -> list of times
    agg = {}
    for p in sorted(file_list):
        entries = parse_file(p)
        for cfg, t in entries:
            agg.setdefault(cfg, []).append(t)
    return agg


def write_summary(agg, out_path):
    # write CSV: config, count, mean_ms, std_ms
    with open(out_path, 'w', newline='', encoding='utf-8') as csvf:
        writer = csv.writer(csvf)
        writer.writerow(['config', 'count', 'mean_ms', 'std_ms'])
        for cfg, vals in sorted(agg.items()):
            cnt = len(vals)
            m = mean(vals) if cnt else ''
            s = stdev(vals) if cnt > 1 else 0.0
            writer.writerow([cfg, cnt, f"{m:.6f}" if m != '' else '', f"{s:.6f}"])
    print(f"Wrote {out_path}")


def main(argv):
    if len(argv) < 2:
        print("Usage: python parse_cuda_template.py <dir-containing-run-files>")
        return 1
    base_dir = argv[1]
    pattern_a = os.path.join(base_dir, 'run*.txt')
    pattern_b = os.path.join(base_dir, 'run*-3d.txt')
    files_a = [f for f in glob.glob(pattern_a) if not f.endswith('-3d.txt')]
    files_b = glob.glob(pattern_b)

    agg_a = aggregate_from_files(files_a)
    agg_b = aggregate_from_files(files_b)

    out_a = os.path.join(base_dir, 'summary_cudaTemplate_run_mean.csv')
    out_b = os.path.join(base_dir, 'summary_cudaTemplate_run3d_mean.csv')

    write_summary(agg_a, out_a)
    write_summary(agg_b, out_b)
    return 0

if __name__ == '__main__':
    raise SystemExit(main(sys.argv))

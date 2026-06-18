#!/usr/bin/env python3
"""Parse bandwidthTest outputs grouped by patterns and compute mean across runs.

By default the script looks for files in the given directory matching
`run*.txt` and `run*-p.txt` and produces one CSV per group containing the
mean MB/s per transfer size for each bandwidth mode.

Usage examples:
  # process files in Codigo/bandwidthTest and write summaries there
  python parse_bandwidth.py Codigo/bandwidthTest

  # process a specific directory and write outputs to out.csv prefix
  python parse_bandwidth.py Codigo/bandwidthTest --out-prefix summary
"""

import sys
import re
import csv
import glob
import os
from statistics import mean

MODE_KEYS = {
    'Host to Device Bandwidth': 'h2d',
    'Device to Device Bandwidth': 'dtod',
    'Device to Host Bandwidth': 'dtoh',
}


def parse_file(path):
    """Parse a single bandwidthTest output file.

    Returns dict: {mode: {size: value}}
    """
    mode = None
    data = {'h2d': {}, 'dtod': {}, 'dtoh': {}}
    size_val_re = re.compile(r"^\s*(\d+)\s+([0-9]+(?:\.[0-9]+)?)\s*$")
    try:
        with open(path, 'r', encoding='utf-8', errors='ignore') as f:
            for line in f:
                line = line.rstrip('\n')
                for key in MODE_KEYS:
                    if key in line:
                        mode = MODE_KEYS[key]
                        break
                m = size_val_re.match(line)
                if m and mode is not None:
                    size = int(m.group(1))
                    val = float(m.group(2))
                    data[mode][size] = val
    except FileNotFoundError:
        print(f"Warning: file not found: {path}")
    return data


def aggregate_means(list_of_data_dicts):
    """Given a list of data dicts (one per run), compute mean per mode and size.

    Returns: dict mode -> {size: mean_value}
    """
    modes = ['h2d', 'dtod', 'dtoh']
    agg = {m: {} for m in modes}
    # collect values per mode and size
    collectors = {m: {} for m in modes}
    for d in list_of_data_dicts:
        for m in modes:
            for size, val in d.get(m, {}).items():
                collectors[m].setdefault(size, []).append(val)
    # compute means
    for m in modes:
        for size, vals in collectors[m].items():
            agg[m][size] = mean(vals)
    return agg


def write_summary_csv(agg, out_path):
    """Write aggregated means to CSV. Columns: bytes,h2d,dtod,dtoh,mean,count"""
    all_sizes = sorted(set().union(*[set(d.keys()) for d in agg.values()]))
    header = ['bytes', 'h2d_mb_s', 'dtod_mb_s', 'dtoh_mb_s', 'mean_mb_s']
    with open(out_path, 'w', newline='', encoding='utf-8') as csvf:
        writer = csv.writer(csvf)
        writer.writerow(header)
        for s in all_sizes:
            h2d = agg['h2d'].get(s, '')
            dtod = agg['dtod'].get(s, '')
            dtoh = agg['dtoh'].get(s, '')
            vals = [v for v in (h2d, dtod, dtoh) if v != '']
            meanv = mean(vals) if vals else ''
            writer.writerow([s, h2d, dtod, dtoh, meanv])


def process_group(files, out_csv):
    if not files:
        print(f"No files to process for group, skipping: {out_csv}")
        return
    print(f"Processing {len(files)} files -> {out_csv}")
    parsed = [parse_file(p) for p in files]
    agg = aggregate_means(parsed)
    write_summary_csv(agg, out_csv)


def main(argv):
    if len(argv) < 2:
        print("Usage: python parse_bandwidth.py <dir-containing-runs> [--out-prefix PREFIX]")
        return 1
    base_dir = argv[1]
    out_prefix = 'summary'
    if '--out-prefix' in argv:
        try:
            out_prefix = argv[argv.index('--out-prefix') + 1]
        except IndexError:
            pass

    # patterns for the two groups
    pattern_a = os.path.join(base_dir, 'run*.txt')
    pattern_b = os.path.join(base_dir, 'run*-p.txt')

    files_a = sorted(glob.glob(pattern_a))
    files_b = sorted(glob.glob(pattern_b))

    # write outputs into the same directory by default
    out_a = os.path.join(base_dir, f"{out_prefix}_run_mean.csv")
    out_b = os.path.join(base_dir, f"{out_prefix}_runp_mean.csv")

    process_group(files_a, out_a)
    process_group(files_b, out_b)
    print("Done.")
    return 0


if __name__ == '__main__':
    raise SystemExit(main(sys.argv))

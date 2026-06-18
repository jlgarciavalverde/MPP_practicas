#!/usr/bin/env python3
"""
Parse vectorAdd run output files and compute mean elapsed time per configuration.

Searches for files matching `run*.txt` in the directory and extracts command
lines like `./vectorAdd --nelem=1000000 --tpb=32` and the following
`Elapsed Time: X (ms)` entries. Aggregates by configuration and writes a CSV
with columns: config, nelem, tpb, count, mean_ms, std_ms

Usage:
  python parse_vectorAdd.py <dir>

"""
import sys
import os
import glob
import csv
import re
from statistics import mean, stdev

CMD_RE = re.compile(r"^\s*\.?/?\.?/.*vectorAdd\s+(.*)$")
TIME_RE = re.compile(r"Elapsed Time:\s*([0-9]+(?:\.[0-9]+)?)\s*\(ms\)")
NELEM_RE = re.compile(r"--nelem=(\d+)")
TPB_RE = re.compile(r"--tpb=(\d+)")


def parse_file(path):
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
                    current_cmd = m.group(1).strip()
                    continue
                t = TIME_RE.search(line)
                if t and current_cmd is not None:
                    time_ms = float(t.group(1))
                    # extract nelem and tpb if present
                    nelem = None
                    tpb = None
                    ne = NELEM_RE.search(current_cmd)
                    if ne:
                        nelem = int(ne.group(1))
                    tp = TPB_RE.search(current_cmd)
                    if tp:
                        tpb = int(tp.group(1))
                    entries.append((current_cmd, nelem, tpb, time_ms))
                    current_cmd = None
    except FileNotFoundError:
        print(f"Warning: file not found: {path}")
    return entries


def aggregate_from_files(file_list):
    agg = {}
    for p in sorted(file_list):
        entries = parse_file(p)
        for cfg, nelem, tpb, t in entries:
            key = cfg
            agg.setdefault(key, {'nelem': nelem, 'tpb': tpb, 'times': []})
            agg[key]['times'].append(t)
    return agg


def write_summary(agg, out_path):
    with open(out_path, 'w', newline='', encoding='utf-8') as csvf:
        writer = csv.writer(csvf)
        writer.writerow(['config', 'nelem', 'tpb', 'count', 'mean_ms', 'std_ms'])
        for cfg, data in sorted(agg.items()):
            vals = data['times']
            cnt = len(vals)
            m = mean(vals) if cnt else ''
            s = stdev(vals) if cnt > 1 else 0.0
            writer.writerow([cfg, data['nelem'] or '', data['tpb'] or '', cnt,
                             f"{m:.6f}" if m != '' else '', f"{s:.6f}"])
    print(f"Wrote {out_path}")


def main(argv):
    if len(argv) < 2:
        print("Usage: python parse_vectorAdd.py <dir-containing-run-files>")
        return 1
    base = argv[1]
    pattern = os.path.join(base, 'run*.txt')
    files = glob.glob(pattern)
    if not files:
        print('No run*.txt files found in', base)
        return 1
    agg = aggregate_from_files(files)
    out = os.path.join(base, 'summary_vectorAdd_run_mean.csv')
    write_summary(agg, out)
    return 0


if __name__ == '__main__':
    raise SystemExit(main(sys.argv))

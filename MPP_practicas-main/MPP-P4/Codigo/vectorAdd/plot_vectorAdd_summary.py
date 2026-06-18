#!/usr/bin/env python3
"""
Plot summary graph for vectorAdd runs.

Reads CSV produced by `parse_vectorAdd.py` (`summary_vectorAdd_run_mean.csv`) with
columns: config, nelem, tpb, count, mean_ms, std_ms

Generates `summary_vectorAdd.png` with mean processing time vs `tpb` for each
`nelem` value (different curves).

Usage:
  python plot_vectorAdd_summary.py <dir>
"""
import os
import sys
import csv
from collections import defaultdict
try:
    import matplotlib.pyplot as plt
except Exception:
    print('matplotlib required: python -m pip install matplotlib')
    raise


def read_summary(path):
    rows = []
    if not os.path.exists(path):
        print(f'Warning: {path} not found')
        return rows
    with open(path, 'r', encoding='utf-8', errors='ignore') as f:
        reader = csv.DictReader(f)
        for r in reader:
            rows.append(r)
    return rows


def main(argv):
    if len(argv) < 2:
        print('Usage: python plot_vectorAdd_summary.py <dir>')
        return 1
    base = argv[1]
    path = os.path.join(base, 'summary_vectorAdd_run_mean.csv')
    rows = read_summary(path)
    if not rows:
        print('No data to plot')
        return 1

    # group by nelem -> list of (tpb, mean_ms, std_ms)
    data = defaultdict(list)
    for r in rows:
        try:
            nelem = int(r.get('nelem') or 0)
            tpb = int(r.get('tpb') or 0)
            mean_ms = float(r.get('mean_ms') or 0.0)
            std_ms = float(r.get('std_ms') or 0.0)
        except Exception:
            continue
        data[nelem].append((tpb, mean_ms, std_ms))

    plt.figure(figsize=(10,6))
    for nelem, lst in sorted(data.items()):
        lst_sorted = sorted(lst, key=lambda x: x[0])
        tpb_vals = [x[0] for x in lst_sorted]
        mean_vals = [x[1] for x in lst_sorted]
        std_vals = [x[2] for x in lst_sorted]
        plt.errorbar(tpb_vals, mean_vals, yerr=std_vals, fmt='-o', capsize=3, label=f'nelem={nelem}')

    plt.xlabel('Threads per block (tpb)')
    plt.ylabel('Processing time (ms)')
    plt.title('vectorAdd: Processing time vs threads per block')
    plt.grid(True, ls='--', lw=0.5)
    plt.legend()
    plt.tight_layout()
    out = os.path.join(base, 'summary_vectorAdd.png')
    plt.savefig(out)
    plt.close()
    print('Wrote', out)
    return 0


if __name__ == '__main__':
    raise SystemExit(main(sys.argv))

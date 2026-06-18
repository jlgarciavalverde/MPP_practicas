#!/usr/bin/env python3
"""
Plot summary graphs for cudaTemplate 2D and 3D runs.

Reads CSV files produced by `parse_cuda_template.py`:
 - summary_cudaTemplate_run_mean.csv  (2D)
 - summary_cudaTemplate_run3d_mean.csv (3D)

Generates PNGs:
 - summary_cudaTemplate_2d.png
 - summary_cudaTemplate_3d.png

Usage:
  python plot_cudaTemplate_summary.py <dir-containing-summary-csvs>
"""
import os
import sys
import csv
from math import isnan
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


def parse_config_2d(cfg):
    # cfg like: '--gsx=16 --gsy=16 --bsx=2 --bsy=2'
    parts = cfg.split()
    d = {}
    for p in parts:
        if '=' in p:
            k, v = p.lstrip('-').split('=', 1)
            try:
                d[k] = int(v)
            except ValueError:
                d[k] = None
    # compute totals
    gsx = d.get('gsx', 1) or 1
    gsy = d.get('gsy', 1) or 1
    bsx = d.get('bsx', 1) or 1
    bsy = d.get('bsy', 1) or 1
    total_threads = gsx * gsy * bsx * bsy
    threads_per_block = bsx * bsy
    return {'gsx': gsx, 'gsy': gsy, 'bsx': bsx, 'bsy': bsy, 'total_threads': total_threads, 'tpb': threads_per_block}


def parse_config_3d(cfg):
    # cfg like: '--gsx=16 --gsy=16 --gsz=2 --bsx=2 --bsy=2 --bsz=2'
    parts = cfg.split()
    d = {}
    for p in parts:
        if '=' in p:
            k, v = p.lstrip('-').split('=', 1)
            try:
                d[k] = int(v)
            except ValueError:
                d[k] = None
    gsx = d.get('gsx', 1) or 1
    gsy = d.get('gsy', 1) or 1
    gsz = d.get('gsz', 1) or 1
    bsx = d.get('bsx', 1) or 1
    bsy = d.get('bsy', 1) or 1
    bsz = d.get('bsz', 1) or 1
    total_threads = gsx * gsy * gsz * bsx * bsy * bsz
    threads_per_block = bsx * bsy * bsz
    return {'gsx': gsx, 'gsy': gsy, 'gsz': gsz, 'bsx': bsx, 'bsy': bsy, 'bsz': bsz, 'total_threads': total_threads, 'tpb': threads_per_block}


def plot_rows(rows, parse_fn, out_path, title):
    if not rows:
        print('No rows to plot for', out_path)
        return
    configs = []
    totals = []
    means = []
    stds = []
    for r in rows:
        cfg = r.get('config')
        try:
            mean_ms = float(r.get('mean_ms', r.get('mean', 0)))
        except Exception:
            mean_ms = float(r.get('mean_ms', 0) or 0)
        try:
            std_ms = float(r.get('std_ms', 0))
        except Exception:
            std_ms = 0.0
        parsed = parse_fn(cfg)
        configs.append(cfg)
        totals.append(parsed['total_threads'])
        means.append(mean_ms)
        stds.append(std_ms)

    # sort by total threads
    zipped = sorted(zip(totals, means, stds, configs))
    totals_s, means_s, stds_s, cfgs_s = zip(*zipped)

    plt.figure(figsize=(10,6))
    plt.errorbar(totals_s, means_s, yerr=stds_s, fmt='-o', capsize=3)
    plt.xscale('log')
    plt.xlabel('Total threads (log scale)')
    plt.ylabel('Processing time (ms)')
    plt.title(title)
    plt.grid(True, which='both', ls='--', lw=0.5)
    plt.tight_layout()
    plt.savefig(out_path)
    plt.close()
    print('Wrote', out_path)


def main(argv):
    if len(argv) < 2:
        print('Usage: python plot_cudaTemplate_summary.py <dir>')
        return 1
    base = argv[1]
    path2d = os.path.join(base, 'summary_cudaTemplate_run_mean.csv')
    path3d = os.path.join(base, 'summary_cudaTemplate_run3d_mean.csv')

    rows2d = read_summary(path2d)
    rows3d = read_summary(path3d)

    out2d = os.path.join(base, 'summary_cudaTemplate_2d.png')
    out3d = os.path.join(base, 'summary_cudaTemplate_3d.png')

    plot_rows(rows2d, parse_config_2d, out2d, 'cudaTemplate 2D: Processing time vs total threads')
    plot_rows(rows3d, parse_config_3d, out3d, 'cudaTemplate 3D: Processing time vs total threads')
    return 0


if __name__ == '__main__':
    raise SystemExit(main(sys.argv))

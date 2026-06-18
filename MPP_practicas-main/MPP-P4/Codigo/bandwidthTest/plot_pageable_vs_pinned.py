#!/usr/bin/env python3
"""
Plot comparison between pageable and pinned mean bandwidths per transfer size.

Reads two CSV files produced by `parse_bandwidth.py`:
 - summary_run_mean.csv     (pageable)
 - summary_runp_mean.csv    (pinned)

Generates PNGs in the same directory (one per mode) and a combined figure.

Usage:
  python plot_pageable_vs_pinned.py <dir-containing-summary-csvs>

Example:
  python plot_pageable_vs_pinned.py Codigo/bandwidthTest

The script requires matplotlib. If not installed, install with:
  python -m pip install matplotlib
"""
import os
import sys
import csv
import math
from statistics import mean

try:
    import matplotlib.pyplot as plt
except Exception as e:
    print("matplotlib is required. Install with: python -m pip install matplotlib")
    raise


DEFAULT_PAGEABLE = 'summary_run_mean.csv'
DEFAULT_PINNED = 'summary_runp_mean.csv'

MODES = [('h2d_mb_s', 'Host -> Device (H2D)'),
         ('dtod_mb_s', 'Device -> Device (D2D)'),
         ('dtoh_mb_s', 'Device -> Host (D2H)')]


def read_summary_csv(path):
    """Return dict: bytes -> {colname: float or None} for each row in CSV"""
    data = {}
    if not os.path.exists(path):
        print(f"Warning: file not found: {path}")
        return data
    with open(path, 'r', encoding='utf-8', errors='ignore') as f:
        reader = csv.DictReader(f)
        for row in reader:
            try:
                b = int(row.get('bytes') or row.get('Bytes') or 0)
            except ValueError:
                continue
            entry = {}
            for key in ['h2d_mb_s', 'dtod_mb_s', 'dtoh_mb_s', 'mean_mb_s']:
                v = row.get(key, '')
                if v is None or v == '':
                    entry[key] = None
                else:
                    try:
                        entry[key] = float(v)
                    except ValueError:
                        entry[key] = None
            data[b] = entry
    return data


def merge_sizes(pageable, pinned):
    sizes = sorted(set(list(pageable.keys()) + list(pinned.keys())))
    return sizes


def plot_mode(sizes, pageable, pinned, mode_key, title, out_path):
    xs = []
    y_page = []
    y_pin = []
    for s in sizes:
        p = pageable.get(s, {}).get(mode_key)
        q = pinned.get(s, {}).get(mode_key)
        # require at least one to be present
        if p is None and q is None:
            continue
        xs.append(s)
        y_page.append(p if p is not None else float('nan'))
        y_pin.append(q if q is not None else float('nan'))

    if not xs:
        print(f"No data for mode {mode_key}, skipping plot")
        return

    # Convert bytes to KB for x axis
    x_kb = [v/1024.0 for v in xs]

    plt.figure(figsize=(10,5))
    plt.plot(x_kb, y_page, label='Pageable', marker='.', linewidth=1)
    plt.plot(x_kb, y_pin, label='Pinned', marker='.', linewidth=1)
    plt.xscale('log')
    plt.xlabel('Transfer size (KB, log scale)')
    plt.ylabel('Bandwidth (MB/s)')
    plt.title(title)
    plt.grid(True, which='both', ls='--', lw=0.5)
    plt.legend()
    plt.tight_layout()
    plt.savefig(out_path)
    plt.close()
    print(f"Wrote {out_path}")


def plot_combined(sizes, pageable, pinned, out_path):
    # create 2x2 subplots (3 modes + mean across modes)
    fig, axes = plt.subplots(2, 2, figsize=(18,10))
    axes = axes.flatten()
    xs = sizes
    x_kb = [v/1024.0 for v in xs]

    # first three: the individual modes
    for ax, (mode_key, title) in zip(axes[:3], MODES):
        y_page = [pageable.get(s, {}).get(mode_key, float('nan')) for s in xs]
        y_pin = [pinned.get(s, {}).get(mode_key, float('nan')) for s in xs]
        ax.plot(x_kb, y_page, label='Pageable', marker='.', linewidth=1)
        ax.plot(x_kb, y_pin, label='Pinned', marker='.', linewidth=1)
        ax.set_xscale('log')
        ax.set_xlabel('Size (KB)')
        ax.set_ylabel('MB/s')
        ax.set_title(title)
        ax.grid(True, which='both', ls='--', lw=0.5)
        ax.legend()

    # fourth subplot: mean across the three modes for pageable and pinned
    ax = axes[3]
    y_page_mean = []
    y_pin_mean = []
    for s in xs:
        p_entry = pageable.get(s, {})
        q_entry = pinned.get(s, {})
        p_vals = [v for v in (p_entry.get('h2d_mb_s'), p_entry.get('dtod_mb_s'), p_entry.get('dtoh_mb_s')) if v is not None]
        q_vals = [v for v in (q_entry.get('h2d_mb_s'), q_entry.get('dtod_mb_s'), q_entry.get('dtoh_mb_s')) if v is not None]
        # if both missing, mark as nan
        if not p_vals and not q_vals:
            y_page_mean.append(float('nan'))
            y_pin_mean.append(float('nan'))
        else:
            y_page_mean.append(mean(p_vals) if p_vals else float('nan'))
            y_pin_mean.append(mean(q_vals) if q_vals else float('nan'))

    ax.plot(x_kb, y_page_mean, label='Pageable (mean)', marker='.', linewidth=1)
    ax.plot(x_kb, y_pin_mean, label='Pinned (mean)', marker='.', linewidth=1)
    ax.set_xscale('log')
    ax.set_xlabel('Size (KB)')
    ax.set_ylabel('MB/s')
    ax.set_title('Mean across H2D/D2D/D2H')
    ax.grid(True, which='both', ls='--', lw=0.5)
    ax.legend()

    plt.suptitle('Pageable vs Pinned: Bandwidth comparison (modes + mean)')
    plt.tight_layout(rect=[0,0,1,0.96])
    plt.savefig(out_path)
    plt.close()
    print(f"Wrote {out_path}")


def plot_mean_all_types(sizes, pageable, pinned, out_path):
    """Plot the mean across H2D, D2D, D2H for pageable vs pinned."""
    xs = []
    y_page = []
    y_pin = []
    for s in sizes:
        p_entry = pageable.get(s, {})
        q_entry = pinned.get(s, {})
        p_vals = [v for v in (p_entry.get('h2d_mb_s'), p_entry.get('dtod_mb_s'), p_entry.get('dtoh_mb_s')) if v is not None]
        q_vals = [v for v in (q_entry.get('h2d_mb_s'), q_entry.get('dtod_mb_s'), q_entry.get('dtoh_mb_s')) if v is not None]
        if not p_vals and not q_vals:
            continue
        xs.append(s)
        y_page.append(mean(p_vals) if p_vals else float('nan'))
        y_pin.append(mean(q_vals) if q_vals else float('nan'))

    if not xs:
        print("No data for mean-all-types plot, skipping")
        return

    x_kb = [v/1024.0 for v in xs]
    plt.figure(figsize=(10,5))
    plt.plot(x_kb, y_page, label='Pageable (mean of modes)', marker='.', linewidth=1)
    plt.plot(x_kb, y_pin, label='Pinned (mean of modes)', marker='.', linewidth=1)
    plt.xscale('log')
    plt.xlabel('Transfer size (KB, log scale)')
    plt.ylabel('Bandwidth (MB/s)')
    plt.title('Mean bandwidth across H2D/D2D/D2H: Pageable vs Pinned')
    plt.grid(True, which='both', ls='--', lw=0.5)
    plt.legend()
    plt.tight_layout()
    plt.savefig(out_path)
    plt.close()
    print(f"Wrote {out_path}")


def main(argv):
    if len(argv) < 2:
        print('Usage: python plot_pageable_vs_pinned.py <dir-containing-summary-csvs>')
        return 1
    base_dir = argv[1]
    page_path = os.path.join(base_dir, DEFAULT_PAGEABLE)
    pin_path = os.path.join(base_dir, DEFAULT_PINNED)

    pageable = read_summary_csv(page_path)
    pinned = read_summary_csv(pin_path)

    sizes = merge_sizes(pageable, pinned)
    if not sizes:
        print('No sizes found in inputs; make sure summary CSVs exist and have data')
        return 1

    # individual mode plots
    for mode_key, title in MODES:
        out_file = os.path.join(base_dir, f'grafica_{mode_key}.png')
        plot_mode(sizes, pageable, pinned, mode_key, title, out_file)

    # combined
    out_comb = os.path.join(base_dir, 'grafica_pageable_vs_pinned_combined.png')
    plot_combined(sizes, pageable, pinned, out_comb)
    # mean across all three modes per size (pageable vs pinned)
    out_mean_all = os.path.join(base_dir, 'grafica_mean_all_types.png')
    plot_mean_all_types(sizes, pageable, pinned, out_mean_all)
    return 0


if __name__ == '__main__':
    raise SystemExit(main(sys.argv))

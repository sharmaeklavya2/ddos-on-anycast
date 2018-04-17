#!/usr/bin/env python3

import sys
import os
from os.path import join as pjoin
from os.path import dirname


FIELDS = ('r_mean', 'r_max', 'r_90p', 'r_75p', 'r_50p')

C90 = 1.6448536269514722
C95 = 1.959963984540054
C99 = 2.5758293035489004


def smart_sortkey(item):
    s, path = item
    try:
        f = float(s)
        v = ('f', f)
    except ValueError:
        v = ('s', s)
    return v


def plot_family(ax, dir, reps, field, plot_type, default_label='default', for_sns=False):
    fpaths = {default_label: pjoin(dirname(dir), 'default.csv')}

    for fname in os.listdir(dir):
        name, ext = os.path.splitext(fname)
        if ext == '.csv':
            fpaths[name] = pjoin(dir, fname)

    import pandas as pd

    for name, fpath in sorted(fpaths.items(), key=smart_sortkey):
        try:
            df = pd.read_csv(fpath)
        except pd.errors.EmptyDataError:
            print('File {} was empty'.format(repr(fpath)), file=sys.stderr)
            continue
        x = df['n_victims']
        if plot_type == 'r':
            y = df[field]
            ax.plot(x, y, linewidth=1, label=name)
        elif plot_type == 'm':
            y = df[field] * x
            yerr = C90 * df['r_stddev'] * x / reps
            ax.errorbar(x, y, yerr=yerr, linewidth=1, label=name)
        else:
            raise NotImplementedError('plot_type {} is not implemented'.format(repr(plot_type)))


    ax.set_xscale('log')
    ax.set_xlabel('n_victims')
    if plot_type == 'r':
        ax.set_yscale('log')
        ax.set_ylabel('relative catchment')
    elif plot_type == 'm':
        ax.set_ylabel('misdistribution factor')
    ax.legend()

    if for_sns:
        ax.grid(True, which='minor', color='w', linewidth=0.5)
        ax.grid(True, which='major', color='k', linewidth=0.5)
    else:
        ax.grid(True, which='minor', color='grey', linewidth=0.5)
        ax.grid(True, which='major', color='k', linewidth=1)


def main():
    import argparse
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('dir')
    parser.add_argument('reps', type=int)
    parser.add_argument('--field', default='r_mean', choices=FIELDS)
    parser.add_argument('--plot-type', default='r', choices=('r', 'm'))
    parser.add_argument('--default-label', default='default')
    parser.add_argument('--figsize', help='Comma-sep sizes')
    parser.add_argument('--sns', action='store_true', default=False, help='Beautify plots using seaborn')
    parser.add_argument('-o', '--output')
    args = parser.parse_args()

    if args.figsize is None:
        figsize = None
    else:
        figsize = tuple([float(x) for x in args.figsize.split(',')])

    if args.sns:
        import seaborn as sns
        sns.set()

    import matplotlib.pyplot as plt

    fig, ax = plt.subplots(figsize=figsize)
    plot_family(ax, args.dir, args.reps, args.field, args.plot_type, args.default_label, args.sns)

    if args.output is None:
        plt.show()
    else:
        fig.tight_layout()
        plt.savefig(args.output)


if __name__ == '__main__':
    main()

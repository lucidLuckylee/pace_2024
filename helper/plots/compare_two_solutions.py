import argparse

import matplotlib.pyplot as plt
import numpy as np
import pandas as pd


def main():
    parser = argparse.ArgumentParser(description="Create a scatter plot")
    parser.add_argument("solution1", type=str,
                        help="The path to the first csv files.")
    parser.add_argument("solution2", type=str,
                        help="The path to the first csv files.")

    parser.add_argument("--label_solution1", type=str, default="Solution 1", help="The label for the x-axis")
    parser.add_argument("--label_solution2", type=str, default="Solution 2", help="The label for the x-axis")

    parser.add_argument("--limit", type=int, default=300, help="The line drawn as the limit")
    parser.add_argument("--limit_name", type=str, default="timelimit", help="The name of the limit line")

    parser.add_argument("--limit2", type=int, default=400, help="The line drawn as the second limit")
    parser.add_argument("--limit2_name", type=str, default="RAM Limit", help="The name of the second limit line")

    parser.add_argument("--output", type=str, default="out.png", help="Output file name")
    parser.add_argument("--replace_process_error", type=int, default=600,
                        help="Replace the process error with this value")
    parser.add_argument("--replace_timeout", type=int, default=300,
                        help="Replace the timeout with this value")

    plt.rcParams['text.usetex'] = True
    plt.rcParams['font.family'] = 'serif'
    plt.figure(figsize=(2.8, 2.5))

    args = parser.parse_args()
    df1 = pd.read_csv(args.solution1)
    df2 = pd.read_csv(args.solution2)

    for df in [df1, df2]:
        df.loc[df['exit_code'] == -15, 'time'] = args.replace_timeout
        df.loc[(df['exit_code'] != 0) & (df['exit_code'] != -15), 'time'] = args.replace_process_error

    plt.scatter(df1["time"], df2["time"], marker='x', s=10, linewidths=0.75)

    lims = np.array([
        0,
        1000,
    ])

    plt.axvline(x=args.replace_timeout, color='orange', linestyle='--', zorder=0, linewidth=0.5)
    plt.axvline(x=args.replace_process_error, color='red', linestyle='--', zorder=0, linewidth=0.5)

    plt.axhline(y=args.replace_timeout, color='orange', linestyle='--', zorder=0, linewidth=0.5)
    plt.axhline(y=args.replace_process_error, color='red', linestyle='--', zorder=0, linewidth=0.5)

    lim1 = plt.plot(lims, lims, 'k-', zorder=0, linewidth=0.5)
    lim2 = plt.plot(lims, lims * 5, 'k--', zorder=0, alpha=0.8, linewidth=0.5)
    lim3 = plt.plot(lims, lims * 20, 'k:', zorder=0, alpha=0.6, linewidth=0.5)
    lim4 = plt.plot(lims, lims * 200, 'k:', zorder=0, alpha=0.4, linewidth=0.5)

    plt.plot(lims, lims / 5, 'k--', zorder=0, alpha=0.8, linewidth=0.5)
    plt.plot(lims, lims / 20, 'k:', zorder=0, alpha=0.6, linewidth=0.5)
    plt.plot(lims, lims / 200, 'k:', zorder=0, alpha=0.4, linewidth=0.5)

    plt.legend((lim1[0], lim2[0], lim3[0], lim4[0]), ("Factor 1", "Factor 5", "Factor 20", "Factor 200"),
               loc='lower right', framealpha=1.0, handlelength=0.75)
    plt.xlim([0.1, 1000])
    plt.ylim([0.1, 1000])

    plt.xlabel(args.label_solution1)
    plt.ylabel(args.label_solution2)

    plt.yscale('log')
    plt.xscale('log')
    plt.savefig(args.output, bbox_inches='tight', dpi=600)
    plt.show()


if __name__ == '__main__':
    main()

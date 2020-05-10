#!/bin/env python3

import csv
import os
import statistics  # requires Python 3.4+
import sys

# define the scaling factor for charts
SCALING_FACTOR = 1000000  # output in MOps/sec

# defining the factors for "perfect scaling"
CORES = 32
THREADS = 64
SMT_FACTOR = 0.25  # hyperthreads count as this much of a core


def usage():
    sys.stderr.write('Usage: pivot-parhash.py eval-parhash-output [measure] >stats.csv\n')
    sys.stderr.write("  'measure' may be 'median' (default), 'mean', or 'max'\n")
    return

def make_int(val,scale=None):
    try:
        return int(val)/scale if scale else int(val)
    except:
        return val

def metric(vals, measure):
    if measure == 'mean':
        return statistics.mean(vals)
    elif measure == 'max':
        return max(vals)
    else:
        return statistics.median(vals)

def perfect_scaling(threads, base):
    if threads == 1:
        return base
    elif threads == CORES:
        return int(CORES * base)
    elif threads >= THREADS:
        return int((CORES + (THREADS-CORES)*SMT_FACTOR) * base)
    else:
        return ''
    
def load_results(f, measure):
    csvreader = csv.reader(f)
    op_ht = {}
    for row in csvreader:
        hsh = row[0]
        thr = make_int(row[1])
        op = row[2]
        speed = make_int(row[3],SCALING_FACTOR)
        if not speed:
            continue
        if op not in op_ht:
            op_ht[op] = {}
        hsh_ht = op_ht[op]
        if hsh not in hsh_ht:
            hsh_ht[hsh] = {}
        thr_ht = hsh_ht[hsh]
        if thr not in thr_ht:
            thr_ht[thr] = []
        thr_ht[thr].append(speed)
    # compute the requested representative value among all runs
    for op,hsh_ht in op_ht.items():
        overall = {}
        for hsh,thr_ht in hsh_ht.items():
            overall[hsh] = sorted([ (thr, metric(vals,measure)) for thr,vals in thr_ht.items() ])
        op_ht[op] = overall
    return op_ht

def main():
    if len(sys.argv) < 2:
        usage()
        return False
    measure = sys.argv[2] if len(sys.argv) > 2 else 'median'
    op_ht = {}
    with open(sys.argv[1],'r') as f:
        op_ht = load_results(f,measure)
    for op,hsh_ht in op_ht.items():
        print(op)
        row = 1
        total = len(hsh_ht)
        base = 0.1
        for hsh,stats in sorted(hsh_ht.items()):
            if row == 1:
                print(',',end=',')
                for thr,speed in stats[:-1]:
                    print(thr,end=',')
                print(stats[-1][0])
            print(','+hsh,end=',')
            for thr,speed in stats[:-1]:
                print(speed,end=',')
                if thr == 1 and speed > base:
                    base = speed
            print(stats[-1][1])
            if row == total:
                print(',perfect',end=',')
                for thr,speed in stats[:-1]:
                    print(perfect_scaling(thr,base),end=',')
                print(perfect_scaling(stats[-1][0],base))
            row += 1
        print('')
    return True

if __name__ == '__main__':
    main()


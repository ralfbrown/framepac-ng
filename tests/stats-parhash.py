#!/bin/env python3

import csv
import os
import statistics  # requires Python 3.4+
import sys

def usage():
    sys.stderr.write('Usage: stats-parhash.py eval-parhash-output >stats.csv\n')
    return

def make_int(val):
    try:
        return int(val)
    except:
        return val

def load_results(f, ht):
    csvreader = csv.reader(f)
    for row in csvreader:
        key = (row[0],make_int(row[1]),row[2])
        if key in ht:
            ht[key].append(make_int(row[3]))
        else:
            ht[key] = [make_int(row[3])]
    return

def main():
    if len(sys.argv) < 2:
        usage()
        return False
    ht = {}
    with open(sys.argv[1],'r') as f:
        load_results(f,ht)
    if ht:
        print('Hash,Threads,Operations,Min,Median,Max,Ops/sec 1,Ops/sec 2,Ops/sec 3,Ops/sec 4,Ops/sec 5')
        stats = []
        for key in ht:
            values = ht[key]
            try:
                minval = min(values)
            except:
                print('values =',values)
                exit
            medval = statistics.median(values)
            maxval = max(values)
            row = [key[0],key[1],key[2],minval,medval,maxval] + values
            stats += [row]
        stats = sorted(stats)
        for s in stats:
            for v in s[:-1]:
                print(v,end=',')
            print(s[-1])
    else:
        print('No data')
    return True

if __name__ == '__main__':
    main()


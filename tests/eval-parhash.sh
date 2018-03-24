#!/bin/bash

# the program to be run
hashtest=bin/parhash

# which hash tables to run
#   -i = FramepaC (integer)
#   -H = Hopscotch (integer)
#   -I = STL (integer)
#   '' = FramepaC (object)
#tables="-i -H -I ''"
tables="-i -H -I"

# the number of threads to use
threads="1 2 3 4 6 9 12 24 48 96 192 384"	# hex-core w/ HT
#threads="1 2 4 8 14 28 42 56 112 224 448"	# dual 14-core w/ HT

# the concurrency for Hopscotch
concurrency="16 32 64 128 256"

# the number of repetitions of each of the above combinations
reps=5

# how long to run each throughput test
seconds=5

# maximum seconds to allow the test program to run (auto-kill on hang)
timelimit=600

# the generation of key values
keys="-k 1"	# default (pseudo-random sequence)
#keys="-k 0"	# sequential (maximizes cache hits and possible fill factor)
#keys="-k 4"	# apply Fasthash64 to sequential integers

# the hash array size
size=67100000  # ~2^26
size=16720000  # ~2^24

# the number of items to put in the hash table
count=12400000

run_hash()
{
    algo=$1
    concur=$2
    [ -n "$concur" ] && concur=-C$concur
    for thr in $threads; do
	timeout -k5 $timelimit $hashtest -T100 --time $seconds -j-$thr -s$size -g$count $concur $keys $algo
    done
    return
}

for rep in $(seq $reps); do
    for algo in $tables; do
	if [ "x$algo" = "x-H" ]; then
	    for concur in $concurrency; do
		run_hash $algo $concur
	    done
	else
	    run_hash $algo
	fi
    done
done

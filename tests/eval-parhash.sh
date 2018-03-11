#!/bin/bash

# the program to be run
hashtest=bin/parhash

# which hash tables to run
#   '' = FramepaC (object)
#   -i = FramepaC (integer)
#   -I = STL (integer)
#   -H = Hopscotch (integer)
tables="'' -i -I -H"

# the number of threads to use
threads="1 2 3 4 6 9 12 24 48 96 192 384"

# the concurrency for Hopscotch
concurrency="16 32 64 128 256"

# the number of repetitions of each of the above combinations
reps=5

# how long to run each throughput test
seconds=4

# the hash array size
size=16700000

# the number of items to put in the hash table
count=12000000

run_hash()
{
    algo=$1
    concur=$2
    [ -n "$concur" ] && concur=-C$concur
    for thr in $threads; do
	for rep in $(seq $reps); do
	    $hashtest -T100 --time $seconds -j-$thr -s$size -g$count $concur $algo
	done
    done
    return
}

for algo in $tables; do
    if [ "x$algo" = "x-H" ]; then
	for concur in $concurrency; do
	    run_hash $algo $concur
	done
    else
	run_hash $algo
    fi
done

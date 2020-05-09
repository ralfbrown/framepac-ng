#!/bin/bash

sz=small
if [ "x$1" == x-med ]; then
    sz=med
elif [ "x$1" == x-big ]; then
    sz=big
elif [ "x$1" == x-huge ]; then
    sz=huge
fi

# the program to be run
hashtest=bin/parhash

# which hash tables to run
#   -i = FramepaC (integer)
#   -H = Hopscotch (integer)
#   -I = STL (integer)
#   '' = FramepaC (object)
#tables="-i -H -I ''"
tables="-i -H -I"
tables="-i -H"

# the number of threads to use
#threads="1 2 3 4 6 9 12 24 48 96 192 384"	# hex-core w/ HT
#threads="1 2 4 8 14 20 28 42 56 112 224 448"	# dual 14-core w/ HT
threads="512 256 128 64 56 48 40 32 28 24 20 16 14 12 10 8 7 6 5 4 3 2 1"  # 32C/64T

# the concurrency for Hopscotch
concurrency="16 32 64 128 256"

# the number of repetitions of each of the above combinations
reps=5

# how long to run each throughput test
seconds=5

# maximum seconds to allow the test program to run (auto-kill on hang)
timelimit=600

# the generation of key values
#keys="-k 0"	# sequential (maximizes cache hits and possible fill factor)
keys="-k 1"	# default (pseudo-random sequence)
#keys="-k 2"	# fixed pseudo-random sequence
#keys="-k 4"	# apply Fasthash64 to sequential integers (may cause inadvertent duplication)

# the hash array size and number of items to put in the hash table, as a function of the requested test size
# number of items is selected to yield a fill factor just under 75% after the initial fill
if [ "$sz" == "huge" ]; then
  size=1073600000 # ~2^30
  count=788000000
  let "seconds=2*seconds"
elif [ "$sz" == "big" ]; then
  size=268400000 # ~2^28
  count=197000000
  let "seconds=2*seconds"
elif [ "$sz" == "med" ]; then
  size=67100000  # ~2^26
  count=49500000
else # sze == small
  size=16720000  # ~2^24
  count=12400000
fi

invoke_hash()
{
    algo=$1
    thr=$2
    concur=$3
    timeout -k5 $timelimit $hashtest -T100 --time $seconds -j-$thr -s$size -g$count $concur $keys $algo
    return
}

run_hash()
{
    algo=$1
    thr=$2
    if [ "x$algo" = "x-H" ]; then
	for concur in $concurrency; do
	    invoke_hash $algo $thr -C$concur
	done
    else
	invoke_hash $algo $thr ""
    fi
    return
}

for rep in $(seq $reps); do
    for thr in $threads; do
	for algo in $tables; do
	    run_hash $algo $thr
	done
    done
done

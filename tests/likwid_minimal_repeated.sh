#!/bin/bash

# used to demonstrate unreasonably high values sometimes produced by likwid.
# This is non-deterministic behavior and is documented at the following
# locations:

# https://groups.google.com/forum/?utm_medium=email&utm_source=footer#!msg/likwid-users/m1ElsBTerfk/rHczVoFkBQAJ

# https://github.com/RRZE-HPC/likwid/issues/292 

# Counts the number of failures in num_iter iterations. Expects likwid_minimal
# to only produce output when unreasonably high values are detected.

# For some reason, the output sent to file is all on one line. To fix this,
# after this command is finished running, I use the following command to add
# newlines retroactively:

# FILES='./*.txt' bash -c 'sed -i "s/ thread /\nthread /g" $FILES; sed -i "s/ DEBUG -/\nDEBUG -/g" $FILES; sed -i "s/ WARNING:/\nWARNING:/g" $FILES'

num_iter=100
num_failures=0

DATE=$(date +%F_%H%M)
DIR=data/unreasonably-high-values
FILENAME_BASE=$DIR/likwid_minimal_repeated

mkdir -p $DIR

for i in $(seq $num_iter); do
  output=$(./likwid_minimal) 
  if [[ $output == *"WARNING: unreasonably high"* ]]; then
      FILENAME=$(echo $FILENAME_BASE)_$(echo $DATE)_iteration_$i.txt
      echo "$i Warning! Something went wrong. Sending output to $FILENAME"
      echo $output > $FILENAME
      ((num_failures++))
  else
      echo "$i No problems detected..."
  fi
done

echo
echo
echo "number of iterations : $num_iter"
echo "number of failures   : $num_failures"

#!/bin/bash

# used to demonstrate unreasonably high values sometimes produced by likwid.
# This is non-deterministic behavior and is documented at the following
# locations:

# https://groups.google.com/forum/?utm_medium=email&utm_source=footer#!msg/likwid-users/m1ElsBTerfk/rHczVoFkBQAJ

# https://github.com/RRZE-HPC/likwid/issues/292 

# Counts the number of failures in num_iter iterations. Expects likwid_minimal
# to only produce output when unreasonably high values are detected.

num_iter=100
num_failures=0

for i in $(seq $num_iter); do
  output=$(./likwid_minimal) 
  if [[ $output ]]; then
      echo "$i Warning! Something went wrong"
      echo $output
      ((num_failures++))
  else
      echo "$i No output, things went as usual..."
  fi
done

echo
echo
echo "number of iterations : $num_iter"
echo "number of failures   : $num_failures"


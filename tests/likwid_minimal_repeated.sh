#!/bin/bash

# used to demonstrate non-deterministic behavior in likwid. This is documented
# here:
# https://groups.google.com/forum/?utm_medium=email&utm_source=footer#!msg/likwid-users/m1ElsBTerfk/rHczVoFkBQAJ

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


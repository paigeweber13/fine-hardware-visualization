#!/bin/sh
#PBS -q mamba
#PBS -l walltime=24:00:00
#PBS -l nodes=1:ppn=16

make clean 1>&2
make polynomial_block_likwid 1>&2

for degree in `seq 1 9` \
		  `seq 10 10 99` \
		  `seq 100 100 999` 
do
    for n in $(echo 1024 | bc) \
		 $(echo 16 \*1024 | bc) \
		 $(echo 64 \*1024 | bc) \
		 $(echo 256 \*1024 | bc) \
		 $(echo 1024 \*1024 | bc) \
		 $(echo 16 \*1024 \*1024 | bc) \
		 $(echo 64 \*1024 \*1024 | bc) # \
#		 $(echo 256 \*1024 \*1024 | bc) 
    do
	echo "$n,$degree,"
	OMP_SCHEDULE="dynamic,8" ./polynomial_block_likwid $n $degree 10
    done
done


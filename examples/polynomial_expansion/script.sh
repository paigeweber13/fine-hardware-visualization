#!/bin/sh
#PBS -q mamba
#PBS -l walltime=24:00:00
#PBS -l nodes=1:ppn=16

make clean
make polynomial

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
	echo $n $degree 1>&2
	OMP_SCHEDULE="dynamic,128" ./polynomial $n $degree 10
    done
done


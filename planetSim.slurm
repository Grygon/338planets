#!/bin/sh
#SBATCH --nodes=1
#SBATCH --cpus-per-task=10  # 10 cores, one for each major body in the system
#SBATCH --time=0-00:01:00  # 01 minute, for the sake of testing
#SBATCH --output=my_output.txt 

cp * $PFSDIR/.
cd $PFSDIR
gcc -o sim sim.c -lrt
 
./sim

#!/bin/sh
#SBATCH --nodes=1
#SBATCH --cpus-per-task=10  # 10 cores, one for each major body in the system
#SBATCH --time=0-00:01:00  # 01 minute, for the sake of testing
#SBATCH --output=my_output.txt 


cp * $PFSDIR/.
cd $PFSDIR
make
./sim
mkdir images
./sim --images 10

mkdir -p ../planetSimResults

cp -ru ./images $SLURM_SUBMIT_DIR/images

./sim --threads 31557600 --finalState 3

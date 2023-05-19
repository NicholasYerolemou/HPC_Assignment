#!/bin/sh
#BATCH --account=icts
#SBATCH --partition=curie
#SBATCH --time=00:10
#SBATCH --nodes=1 --ntasks=8 --ntasks-per-node=8
#SBATCH --job-name=parallel_sort
#SBATCH --mail-user=yrlnic001@myuct.ac.za
module load mpi/openmpi-4.0.1
make clean
make bench
./bench >> output.txt

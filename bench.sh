#!/bin/sh
#BATCH --account=icts
#SBATCH --partition=curie
#SBATCH --time=06:00:00
#SBATCH --nodes=1 --ntasks=32 --ntasks-per-node=32
#SBATCH --job-name=parallel_sort
#SBATCH --mail-user=yrlnic001@myuct.ac.za,ndhcar002@myuct.ac.za
#SBATCH --mail-type=ALL
module load mpi/openmpi-4.0.1
make clean
make bench
./bench >> output.txt

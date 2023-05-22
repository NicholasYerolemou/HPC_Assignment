# !/bin/bash
# !/bin/sh
# BATCH --account=icts
# SBATCH --partition=curie
# SBATCH --time=06:00:00
# SBATCH --nodes=4 --ntasks=32 --ntasks-per-node=32
# SBATCH --job-name=parallel_sort
# SBATCH --mail-user=yrlnic001@myuct.ac.za,ndhcar002@myuct.ac.za
# SBATCH --mail-type=ALL


make clean
make mpi
make hybrid

# MPI program executable
MPI_PROGRAM="./ParallelSort_MPI"
HYBRID_PROGRAM="./hybrid"

# Number of runs
NUM_RUNS=1

# Varying number of nodes
# NODES=(1 2 3 4)
NODES=(3)

# Varying number of nodes
# THREADS=(1 2 4 8 16 32)
THREADS=(32)

# Varying seeds for random number generator
# SEEDS=(42, 23, 143)
SEEDS=(42)

# Varying input sizes
# SIZES=(100000 1000000 10000000)
# SIZES=(12000 120000 1200000)
SIZES=(1000)

NUM_SEEDS=${#SEEDS[@]}

# # Run the MPI program for each combination of parameters
# for ((run=1; run<=$NUM_RUNS; run++))
# do
#     # echo "Run $run"
#     for nodes in "${NODES[@]}"
#     do
#         for ((i=0; i<NUM_SEEDS; i++))
#         do
#                 seed=${SEEDS[i]}
#                 size=${SIZES[i]}
#                 echo "Running MPI program with - Seed: $seed, Size $size, Nodes: $nodes"
#                 mpirun -np $nodes $MPI_PROGRAM $seed $size
#                 echo "-----------------------------"
#         done
#     done
# done

#Run the Hybrid program for each combination of parameters
for ((run=1; run<=$NUM_RUNS; run++))
do
    # echo "Run $run"
    for nodes in "${NODES[@]}"
    do
        for ((i=0; i<NUM_SEEDS; i++))
        do
            for thread in "${THREADS[@]}"
            do
                    seed=${SEEDS[i]}
                    size=${SIZES[i]}
                    echo "Running MPI program with - Seed: $seed, Size $size, Nodes: $nodes, Threads: $thread"
                    mpirun -np $nodes $HYBRID_PROGRAM $seed $size $thread
                    echo "-----------------------------"
            done
        done
    done
done


#!/bin/bash
#!/bin/sh
#BATCH --account=icts
#SBATCH --partition=curie
#SBATCH --time=06:00:00
#SBATCH --nodes=4 --ntasks=32 --ntasks-per-node=3
#SBATCH --job-name=parallel_sort
#SBATCH --mail-user=yrlnic001@myuct.ac.za,ndhcar002@myuct.ac.za
#SBATCH --mail-type=ALL


# MPI program executable
MPI_PROGRAM="./ParallelSort_MPI"
HYBRID_PROGRAM="./ParallelSort_Hybrid"

# Number of runs
NUM_RUNS=1

# Varying number of nodes
NODES=(1 2 3 4)

# Varying number of nodes
THREADS=(1 2 4 8 16 32)

# Varying seeds for random number generator
SEEDS=(123 456 789)

# Varying input sizes
SIZES=(1000 10000 100000 1000000 10000000 100000000 1000000000)

# Run the MPI program for each combination of parameters
for ((run=1; run<=$NUM_RUNS; run++))
do
    # echo "Run $run"
    for nodes in "${NODES[@]}"
    do
        for seed in "${SEEDS[@]}"
        do
            for size in "${SIZES[@]}"
            do
                echo "Running MPI program with - Seed: $seed, Size $size, Nodes: $nodes"
                mpirun -np $nodes $MPI_PROGRAM $seed $size
                echo "-----------------------------"
            done
        done
    done
done

# Run the Hybrid program for each combination of parameters
for ((run=1; run<=$NUM_RUNS; run++))
do
    # echo "Run $run"
    for nodes in "${NODES[@]}"
    do
        for seed in "${SEEDS[@]}"
        do
            for thread in "${THREADS[@]}"
            do
                for size in "${SIZES[@]}"
                do
                echo "Running MPI program with - Seed: $seed, Size $size, Nodes: $nodes, Threads: $thread"
                    mpirun -np $nodes $HYBRID_PROGRAM $seed $size $thread
                    echo "-----------------------------"
                done
            done
        done
    done
done

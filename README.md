# HPC_Assignment

CSC4028Z High performance computing main assignment

This assignment deals with benchmarking the parallel sorting regular sampling algortihm implemented in 4 different ways. First, implements the algorithm in serial (`bash ParallelSort_Serial.c`) being the basic solution to the problem. Second, implements parallelism using OpenMP (`bash ParallelSort_OpenMP.c`) exploiting the use of threads. Third, explores alternative parallelism through the use of multiple computing nodes instead of threads (`bash ParallelSort_MPI.c`). Finally, the most parallel solution utlising both multiple computing nodes along with multiple threads on each node (`bash ParallelSort_Hybrid.c`).

## Running

As this is mainly a benchmarking exercise, the programs themselves are not run individually, but instead are executed by benchmark programs. The serial and OpenMP programs are executed by the `powershell benchMarks.c` file while the MPI and hybrid versions are executed using the `bash bench_mpi.sh` bash script file. Both these files run the programs for multiple iterations with each iteration having different parameter values that are used by the programs such as the number of threads, size of the input array to be sorted and the number of nodes.

To run the file, simply run the following commands:

```bash
make bench
./bench_mpi.sh
```

If the `bash bench_mpi.sh` errors out, it may be due to it not being recognised as an executable file. To resolve this, simply run:

```bash
chmod +x bench_mpi.sh
```

Then rerun the file as an excutable.

## Outputs

The outputs the benchmark programs (`bash benchMarks.c` and `bash bench_mpi.sh`) are slightly different due to the different parameters each of the 4 programs takes.

### BenchMarks.c

The output printed by this benchmark is below:

```bash
Seed 143, Size 1200000, Threads 1
Serial: 0.460000
OpenMP: 0.210000
```

Here, `bash Seed` relates to the seed value used for generating the values placed into the input array. The `bash Size` is the length of the input array. `bash Threads` which is the number of threads the programs are run with. The OpenMP version uses actual threads while the serial version simulates them using for loops. The execution times for both `bash Serial` and `bash OpenMP` are then printed.

### Bench_mpi.sh

The output printed by this benchmark is below with slight differences between the MPI and hybrid versions:

```bash
Running MPI program with - Seed: 42, Size 12000, Nodes: 1
MPI: Rank 0: 0.140000
-----------------------------
```

```bash
Running Hybrid program with - Seed: 42, Size 12000, Nodes: 2, Threads: 1
Hybrid: Rank 0: 0.150000
-----------------------------
```

Here, `bash Seed` relates to the seed value used for generating the values placed into the input array. The `bash Size` is the length of the input array. `bash Threads` which is the number of threads the programs are run with. The OpenMP version uses actual threads while the serial version simulates them using for loops. The execution times for both `bash Serial` and `bash OpenMP` are then printed. `bash Nodes` is the number of computing nodes used to run the program on.

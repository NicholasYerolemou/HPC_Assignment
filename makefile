CC=gcc
CFLAGS=-lm -fopenmp -std=c99 -Wall

all: openMP serial mpi bench

openMP: ParallelSort_OpenMP.c
	$(CC) -o ParallelSort_OpenMP ParallelSort_OpenMP.c -fopenmp $(CFLAGS)

mpi: ParallelSort_MPI.c
	mpicc -o ParallelSort_MPI ParallelSort_MPI.c $(CFLAGS)

serial: ParallelSort_Serial.c
	$(CC) -o ParallelSort_Serial ParallelSort_Serial.c $(CFLAGS)

bench: benchMarks.c ParallelSort_Serial.c ParallelSort_OpenMP.c
	$(CC) -o bench benchMarks.c  $(CFLAGS)

hybrid: ParallelSort_Hybrid.c
	mpicc -o hybrid ParallelSort_Hybrid.c -g $(CFLAGS)

run_openMP: openMP
	./ParallelSort_OpenMP

run_mpi: mpi
	mpirun -n 2 ./ParallelSort_MPI 100

run_serial: serial
	./ParallelSort_Serial

run_bench: bench
	./bench

run_hybrid:
	mpirun -n 4 ./hybrid

clean:
	rm -f ParallelSort_OpenMP ParallelSort_Serial bench ParallelSort_MPI hybrid

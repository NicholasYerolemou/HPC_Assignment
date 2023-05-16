CC=gcc
CFLAGS=-lm -fopenmp

all: openMP serial mpi bench

openMP: ParallelSort_OpenMP.c
	$(CC) -o ParallelSort_OpenMP ParallelSort_OpenMP.c -fopenmp $(CFLAGS)

mpi: ParallelSort_MPI.c
	mpicc -o ParallelSort_MPI ParallelSort_MPI.c $(CFLAGS)

serial: ParallelSort_Serial.c
	$(CC) -o ParallelSort_Serial ParallelSort_Serial.c $(CFLAGS)

bench: benchMarks.c
	$(CC) -o bench benchMarks.c  $(CFLAGS)

run_openMP: openMP
	./ParallelSort_OpenMP

run_mpi: mpi
	mpirun -n 2 ./ParallelSort_MPI 100

run_serial: serial
	./ParallelSort_Serial

run_bench: bench
	./bench

clean:
	rm -f ParallelSort_OpenMP ParallelSort_Serial bench ParallelSort_MPI

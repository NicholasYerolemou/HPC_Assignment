CC=gcc
CFLAGS=-fopenmp -lm

all: openMP

openMP: ParallelSort_OpenMP.c
	$(CC) -o ParallelSort_OpenMP ParallelSort_OpenMP.c $(CFLAGS)

run: openMP
	./ParallelSort_OpenMP

clean:
	rm -f ParallelSort_OpenMP

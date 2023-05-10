CC=gcc
CFLAGS=-lm

all: openMP serial

openMP: ParallelSort_OpenMP.c
	$(CC) -o ParallelSort_OpenMP ParallelSort_OpenMP.c -fopenmp $(CFLAGS)

serial: ParallelSort_Serial.c
	$(CC) -o ParallelSort_Serial ParallelSort_Serial.c $(CFLAGS)

run_openMP: openMP
	./ParallelSort_OpenMP

run_serial: serial
	./ParallelSort_Serial

clean:
	rm -f ParallelSort_OpenMP ParallelSort_Serial

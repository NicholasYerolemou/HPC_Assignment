#define NDEBUG
#include <assert.h>
#include <mpi.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include "math.h"
#include "time.h"

int main(int argc, char **argv)
{
}

double hybrid_psrs_sort(long long *arr, int n, int p)
{
    // spilt data amogst nodes
    // in each node run in parallel sample selection
    // in openMP collect pivots
    // in MPI collect pivots from all nodes

    int size = 4;                               // number of nodes
    int n_per = n / size;                       // amount of arr per node
    int *a = (int *)calloc(n_per, sizeof(int)); // part of arr for this node
    assert(a != NULL);

    MPI_Scatter(a_all, n_per, MPI_INT, a, n_per, MPI_INT, 0, MPI_COMM_WORLD); // dristribute part of arr amongst nodes
    
    
    
    qsort(a, n_per, sizeof(int), cmp);//sorting of threads on this node

    //done in parallel using openMP
    int *samples = (int *)calloc(size, sizeof(int));
    assert(samples != NULL);
    for (i = 0; i < size; i++)
        samples[i] = a[i * size];
    //


    
}
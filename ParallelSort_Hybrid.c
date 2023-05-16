#define NDEBUG
#include "common.c"
#include "math.h"
#include "time.h"
#include <assert.h>
#include <mpi.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

double hybrid_psrs_sort(long long *arr, int n, int p);
// int *hybrid_merge_sort(int *arr, int size);
// int *hybrid_merge(int *left, int *right, int l_end, int r_end);
// void hybrid_sortll(int *a, int len);
// int hybrid_lcompare(const void *ptr2num1, const void *ptr2num2);
int main(int argc, char **argv)
{
    int size = 1000;
    long long numbers[size];

    // Seed the random number generator
    srand(time(NULL));

    // Generate random long long numbers and store them in the array
    for (int i = 0; i < size; i++)
    {
        numbers[i] = rand();
        // printf("%lld\n", numbers[i]);
    }

    hybrid_psrs_sort(numbers, size, 8);
}

double hybrid_psrs_sort(long long *arr, int n, int p)
{
    // spilt data amogst nodes
    // in each node run in parallel sample selection
    // in openMP collect pivots
    // in MPI collect pivots from all nodes
    int sizeMPI, rank;
    MPI_Init(NULL, NULL);
    MPI_Comm_size(MPI_COMM_WORLD, &sizeMPI);
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);

    // int size = 4;
    int numNodes = 4;                           // number of nodes
    int n_per = n / numNodes;                   // amount of arr per node
    int *a = (int *)calloc(n_per, sizeof(int)); // part of arr for this node
    assert(a != NULL);

    MPI_Scatter(arr, n_per, MPI_INT, a, n_per, MPI_INT, 0,
                MPI_COMM_WORLD); // dristribute part of arr amongst nodes

    omp_set_num_threads(p);
    long long size, rsize, sample_size;
    long long *sample, *pivots;
    long long *partition_borders, *bucket_sizes, *result_positions;
    long long **loc_a_ptrs;
    size = (n_per + p - 1) / p; // size of each threads sub-array
    rsize = (size + p - 1) / p;
    sample_size = p * (p - 1); // the number of samples to be taken by this node

    loc_a_ptrs = malloc(p * sizeof(int *));
    sample = malloc(sample_size * sizeof(int)); // this holds the samples from this node
    // printf("Rank:%d sample size%d\n", rank, sizeof(sample) / sizeof(int));
    partition_borders = malloc(p * (p + 1) * sizeof(int));
    bucket_sizes = malloc(p * sizeof(int));
    result_positions = malloc(p * sizeof(int));
    pivots = malloc((p - 1) * sizeof(int));

    // printf("size:%lld\n", size);
    // printf("n per node:%lld\n", n_per);

    // arr is the total array used by all nodes
    // a is the array that has been given to a single node

#pragma omp parallel // distribute this node's part of the arr amongst threads
    {
        int i, j, max, thread_num, start, end, loc_size, offset, this_result_size;
        int *loc_a, *this_result, *current_a;

        thread_num = omp_get_thread_num();
        start = thread_num * size;
        end = start + size - 1;

        if (end >= n_per)
            end = n_per - 1;
        loc_size = (end - start + 1); // size of each threads subarray
        end = end % size;

        // if (thread_num == 2)
        // {
        //     printf("Rank:%d loc_size%d\n", rank, loc_size);
        //     printf("Rank:%d start:%d\n", rank, start);
        // }

        // loc_a is per thread
        loc_a = malloc(loc_size * sizeof(int));           // we are taking samples per thread
        memcpy(loc_a, a + start, loc_size * sizeof(int)); // puttig them in loc_a
        loc_a_ptrs[thread_num] = loc_a;

        sortll(loc_a, loc_size);

        offset = thread_num * (p - 1) - 1;

        if (thread_num == 0)
        {
            printf("Rank:%d sample size%d\n", rank, loc_size);
            // printf("Rank:%d start:%d\n", rank, start);
        }

        for (int i = 1; i < p; i++) // moving the samples we collected into the main sample array, it holds the samples from all threads
        {

            // if (i * rsize <= end)
            // {
            //     sample[offset + i] = loc_a[i * rsize - 1];
            // }
            // else
            // {
            //     sample[offset + i] = loc_a[end];
            // }
        }
    }

    // #pragma omp barrier
    //     // int *samples = (int *)calloc(size, sizeof(int));

    //     int *samples_all;
    //     if (rank == 0)
    //     {
    //         samples_all = (int *)calloc((size * size), sizeof(int));
    //         assert(samples_all != NULL);
    //     }

    // this needs to hold the samples from each node
    // we are actually taking p samples per node. if nodes = 2 we are taking twice as many samples as we should

    // #pragma omp single
    //     {

    //         MPI_Barrier(MPI_COMM_WORLD);
    //         // gather the samples data from all processes to the root process
    //         MPI_Gather(sample, sizeMPI, MPI_INT, samples_all, sizeMPI, MPI_INT, 0,
    //                    MPI_COMM_WORLD);

    //         if (rank == 0)
    //         {
    //             printf("There are %li sampels", sizeof(samples_all) / sizeof(int));
    //             // merge_sort(samples_all, sample_size); // we need to work out sample size, because we are actually taking more samples than we should
    //             // for (int i = 0; i < p - 1; i++)
    //             // {
    //             //     pivots[i] = sample[i * p + p / 2]; // select the pivots from the sample array
    //             // }
    //         }
    //     }

    // #pragma omp barrier

    //     for (int i = 0; i < p; ++i)
    //     {
    //         printf("Pivots: %lld %d", pivots[i], rank);
    //     }

    // qsort(a, n_per, sizeof(int), cmp);//sorting of threads on this node

    // //done in parallel using openMP
    // int *samples = (int *)calloc(size, sizeof(int));
    // assert(samples != NULL);
    // for (i = 0; i < size; i++)
    //     samples[i] = a[i * size];
    //

    MPI_Finalize();

    return 0;
}

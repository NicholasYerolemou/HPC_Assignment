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
#include <stdbool.h>

double hybrid_psrs_sort(int *arr, int n, int p);
// int *hybrid_merge_sort(int *arr, int size);
// int *hybrid_merge(int *left, int *right, int l_end, int r_end);
// void hybrid_sortll(int *a, int len);
// int hybrid_lcompare(const void *ptr2num1, const void *ptr2num2);
int main(int argc, char **argv)
{
    int size = 1000;
    int numbers[size];

    // Seed the random number generator
    srand(time(NULL));

    // Generate random int numbers and store them in the array
    for (int i = 0; i < size; i++)
    {
        numbers[i] = rand();
        // printf("%lld\n", numbers[i]);
    }

    hybrid_psrs_sort(numbers, size, 8);
}

double hybrid_psrs_sort(int *arr, int n, int p)
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
    int size, rsize, sample_size;
    int *sample, *pivots;
    int *partition_borders, *bucket_sizes, *result_positions;
    int **loc_a_ptrs;
    size = (n_per + p - 1) / p; // size of each threads sub-array
    // printf("size %lld\n", size);
    rsize = (size + p - 1) / p;
    sample_size = (p * (p - 1)) / numNodes; // the number of samples to be taken by this node
    if (sample_size == 0)
    {
        sample_size = 1;
    }
    // printf("num samples per node %d\n", sample_size);

    loc_a_ptrs = malloc(p * sizeof(int *));
    sample = malloc(sample_size * sizeof(int)); // this holds the samples from this node is of size sample_size

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
        int i, j, thread_num, start, end, loc_size, offset, this_result_size;
        int *per_thread_subarray, *this_result, *current_a;
        thread_num = omp_get_thread_num();
        start = thread_num * size; // multiples of 32, starting index of where this threads values are placed
        end = start + size - 1;

        if (end >= n_per)
            end = n_per - 1;
        loc_size = (end - start + 1); // size of each threads subarray = 32 // this changes to 26 for thread 7 which is correct as that is how many elements are left of the 250
        end = end % size;             // now end is just telling us how much we need to add to start to get to the next start value = 124

        // per_thread_subarray is per thread
        per_thread_subarray = malloc(loc_size * sizeof(int));           // we are taking samples per thread
        memcpy(per_thread_subarray, a + start, loc_size * sizeof(int)); // puttig them in per_thread_subarray
        loc_a_ptrs[thread_num] = per_thread_subarray;

        sortll(per_thread_subarray, loc_size);

        offset = fmax(((thread_num * (p - 1)) - 1) / numNodes, thread_num);
        // if (rank == 0)
        // {
        //     printf("thread num %d offset %d\n", thread_num, offset);
        // }
        int range = (sample_size + p - 1) / p;

        // printf("%d\n", range);
        // if (thread_num == 31)
        // {
        //     printf("%d", loc_size);
        // }
        for (int i = 0; i < range; i++) // moving the samples we collected into the main sample array, it holds the samples from all threads
        {
            // rsize is size of each threads subarray
            // if (i * rsize <= end)
            // {
            rsize = loc_size / range;

            if ((((i + 1) * rsize) - 1) >= 0 && (((i + 1) * rsize) - 1) <= (loc_size - 1))
            {
                sample[offset + i] = per_thread_subarray[((i + 1) * rsize) - 1];
            }
            else
            {

                // printf("Thread:%d val:%d\n", thread_num, per_thread_subarray[loc_size - 1]);

                sample[offset + i] = per_thread_subarray[loc_size - 1]; // come back to this !!!!!!!!!!!!!!
            }
        }
    }

#pragma omp barrier

    // if (rank == 0)
    // {
    //     // printf("Thread:%d/n", omp_get_thread_num());
    //     for (int i = 0; i < sample_size; ++i) // check the sample values
    //     {
    //         printf("i %d: Rank:%d: %d\n", i, rank, sample[i]);
    //     }
    // }

    // int *samples = (int *)calloc(size, sizeof(int));

    int *samples_all; // holds the samples from all the nodes
    if (rank == 0)
    {
        samples_all = (int *)calloc((numNodes * sample_size), sizeof(int));
        assert(samples_all != NULL);
    }

    // we are actually taking p samples per node. if nodes = 4 we are taking 4 times as many samples as we should

#pragma omp single
    {

        MPI_Barrier(MPI_COMM_WORLD);
        // gather the samples data from all processes to the root process
        MPI_Gather(sample, sample_size, MPI_INT, samples_all, sample_size, MPI_INT, 0,
                   MPI_COMM_WORLD);

        // if (rank == 0)
        // {
        //     for (int i = 0; i < numNodes * sample_size; ++i)
        //     {
        //         printf("%d\n", samples_all[i]);
        //     }
        // }

        if (rank == 0) // sort the samples and select pivots
        {
            // printf("There are %li sampels", sizeof(samples_all) / sizeof(int));
            merge_sort(samples_all, numNodes * sample_size); // we need to work out sample size, because we are actually taking more samples than we should
            for (int i = 0; i < p - 1; i++)
            {
                printf("%d\n", samples_all[i * p + p / 2]);
                pivots[i] = samples_all[i * p + p / 2]; // select the pivots from the sample array
            }
        }
    }

#pragma omp barrier

    // if (rank == 0)
    // {
    //     for (int i = 0; i < p - 1; ++i)
    //     {
    //         printf("Pivots: %d\n", pivots[i]);
    //     }
    // }

    MPI_Bcast(pivots, (p - 1), MPI_INT, 0, MPI_COMM_WORLD);
    // redistribute pivots

#pragma omp parallel
    {

        int *partition_borders, *bucket_sizes;
        int thread_num, end, loc_size, offset;
        int *loc_a;
        thread_num = omp_get_thread_num();

        offset = thread_num * (p + 1);
        partition_borders[offset] = 0;
        partition_borders[offset + p] = end + 1;
        calc_partition_borders(loc_a, 0, loc_size - 1, partition_borders, offset, pivots, 1, p - 1);
    }

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

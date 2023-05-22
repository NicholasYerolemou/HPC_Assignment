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

double hybrid_psrs_sort(int *arr, long n, int p);

double hybrid_psrs_sort(int *arr, long n, int p)
{
    clock_t start_time = clock(); // end timer
    // spilt data amogst nodes
    // in each node run in parallel sample selection
    // in openMP collect pivots
    // in MPI collect pivots from all nodes
    int sizeMPI, rank;
    MPI_Init(NULL, NULL);
    MPI_Comm_size(MPI_COMM_WORLD, &sizeMPI);
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);

    // int size = 4;
    int numNodes = sizeMPI;                              // number of nodes
    long n_per = n / numNodes;                           // amount of arr per node
    int *node_array = (int *)calloc(n_per, sizeof(int)); // part of arr for this node
    assert(node_array != NULL);

    MPI_Scatter(arr, n_per, MPI_INT, node_array, n_per, MPI_INT, 0,
                MPI_COMM_WORLD); // dristribute part of arr amongst nodes

    int size, rsize, sample_size;
    int *sample, *pivots;
    int *partition_borders, *bucket_sizes, *result_positions;
    int **ptrs_to_each_threads_subarray;
    size = (n_per + p - 1) / p; // size of each threads sub-array

    if (size * (p - 1) >= n_per) // the eay the arr is divided the last thread would not have any data to operate on.
    {
        p = p - 1;
        printf("reducing num threads");
    }

    omp_set_num_threads(p);

    rsize = (size + p - 1) / p;
    sample_size = (p * (p - 1)) / numNodes; // the number of samples to be taken by this node
    if (sample_size == 0)
    {
        sample_size = 1;
    }
    // printf("num samples per node %d\n", sample_size);

    ptrs_to_each_threads_subarray = malloc(p * sizeof(int *));
    sample = malloc(sample_size * sizeof(int)); // this holds the samples from this node is of size sample_size

    partition_borders = calloc(p * (p + 1), sizeof(int));
    bucket_sizes = malloc(p * sizeof(int));
    result_positions = malloc(p * sizeof(int));
    pivots = malloc((p - 1) * sizeof(int));

    int *samples_all;

    // arr is the total array used by all nodes
    //     node_array is the array that has been given to node_array single node

#pragma omp parallel // distribute this node's part of the arr amongst threads
    {
        int i, j, thread_num, max, start, end, per_thread_subarray_size, offset, this_result_size;
        int *per_thread_subarray, *this_result;
        thread_num = omp_get_thread_num();
        start = thread_num * size; // multiples of 32, starting index of where this threads values are placed
        end = start + size - 1;

        if (end >= n_per)
            end = n_per - 1;

        // if (start >= end)
        // {
        //     per_thread_subarray_size = 0;
        //     start = end;
        // }
        // else
        // {
        //     per_thread_subarray_size = (end - start + 1); // size of each threads subarray = 32 // this changes to 26 for thread 7 which is correct as that is how many elements are left of the 250
        // }
        per_thread_subarray_size = (end - start + 1);
        end = end % size; // now end is just telling us how much we need to add to start to get to the next start value = 124

        // per_thread_subarray is per thread
        per_thread_subarray = malloc(per_thread_subarray_size * sizeof(int));                    // we are taking samples per thread
        memcpy(per_thread_subarray, node_array + start, per_thread_subarray_size * sizeof(int)); // puttig them in per_thread_subarray
        ptrs_to_each_threads_subarray[thread_num] = per_thread_subarray;

        sortll(per_thread_subarray, per_thread_subarray_size);

        offset = fmax(((thread_num * (p - 1)) - 1) / numNodes, thread_num);

        int range = (sample_size + p - 1) / p;

        for (int i = 0; i < range; i++) // moving the samples we collected into the main sample array, it holds the samples from all threads
        {

            rsize = per_thread_subarray_size / range;

            if ((((i + 1) * rsize) - 1) >= 0 && (((i + 1) * rsize) - 1) <= (per_thread_subarray_size - 1))
            {
                sample[offset + i] = per_thread_subarray[((i + 1) * rsize) - 1];
            }
            else
            {

                sample[offset + i] = per_thread_subarray[per_thread_subarray_size - 1]; // come back to this !!!!!!!!!!!!!!
            }
        }

#pragma omp barrier

        // we are actually taking p samples per node. if nodes = 4 we are taking 4 times as many samples as we should

#pragma omp single
        {
            // holds the samples from all the nodes
            if (rank == 0)
            {
                samples_all = (int *)calloc((numNodes * sample_size), sizeof(int));
                assert(samples_all != NULL);
            }

            MPI_Barrier(MPI_COMM_WORLD);
            // gather the samples data from all processes to the root process
            MPI_Gather(sample, sample_size, MPI_INT, samples_all, sample_size, MPI_INT, 0,
                       MPI_COMM_WORLD);

            if (rank == 0) // sort the samples and select pivots
            {
                // printf("There are %li sampels", sizeof(samples_all) / sizeof(int));
                merge_sort(samples_all, numNodes * sample_size); // we need to work out sample size, because we are actually taking more samples than we should
                for (int i = 0; i < p - 1; i++)
                {
                    // printf("%d\n", samples_all[i * p + p / 2]);
                    pivots[i] = samples_all[i * p + p / 2]; // select the pivots from the sample array
                }
            }

            MPI_Bcast(pivots, (p - 1), MPI_INT, 0, MPI_COMM_WORLD);
            // redistribute pivots
        }

#pragma omp barrier

        // partition local data
        offset = thread_num * (p + 1);
        partition_borders[offset] = 0;
        partition_borders[offset + p] = end + 1;

        calc_partition_borders(per_thread_subarray, 0, per_thread_subarray_size - 1, partition_borders, offset, pivots, 1, p - 1);

#pragma omp barrier

        max = p * (p + 1);
        bucket_sizes[thread_num] = 0;

        for (i = thread_num; i < max; i += p + 1)
        {
            bucket_sizes[thread_num] += partition_borders[i + 1] - partition_borders[i];
            if (rank == 3)
            {
                // printf("Thread %d  in %d\n", thread_num, partition_borders[45]);
            }
        }

#pragma omp barrier

#pragma omp single
        {

            {
                result_positions[0] = 0;
                for (i = 1; i < p; i++)
                {
                    result_positions[i] = bucket_sizes[i - 1] + result_positions[i - 1];
                }
            }
        }

#pragma omp barrier

        this_result = node_array + result_positions[thread_num]; // this is the starting position of this threads sorted sub array in the final array

        if (thread_num == p - 1)
        {
            this_result_size = n_per - result_positions[thread_num];
        }
        else
        {
            this_result_size = result_positions[thread_num + 1] - result_positions[thread_num];
        }

        this_result = node_array + result_positions[thread_num];

        for (i = 0, j = 0; i < p; i++)
        {
            int low, high, partition_size;
            offset = i * (p + 1) + thread_num;
            low = partition_borders[offset];
            high = partition_borders[offset + 1];
            partition_size = (high - low);
            if (partition_size > 0)
            {

                memcpy(this_result + j, &(ptrs_to_each_threads_subarray[i][low]), partition_size * sizeof(int));

                j += partition_size;
            }
        }

        sortll(this_result, this_result_size); // this_result holds the sorted subarray for each thread
                                               // print_array(this_result, this_result_size, "hello", 4);

        // if (rank == 0)
        // {
        //     // print_array(this_result, this_result_size, "this result", thread_num);
        //     printf("Thread %d: %d\n", thread_num, checkSorted(this_result, this_result_size));
        // }

#pragma omp barrier
        free(per_thread_subarray);

    } // end parallel - at this point each nodes array is sorted and help in node_array

    free(sample);
    free(partition_borders);
    free(bucket_sizes);
    free(result_positions);
    free(pivots);
    free(samples_all);

    // when n is not divisable by num nodes we might be getting different n_per per node
    MPI_Gather(node_array, n_per, MPI_INT, arr, n_per, MPI_INT, 0, MPI_COMM_WORLD); // Gather all the sorted nodes from all processes into the results_from_processes array

    // if (rank == 0)
    // {
    // }
    // printf("Rank: %d\n", rank);
    merge_sort(arr, n); // sort the arrays retuned from each node

    free(node_array);
    free(ptrs_to_each_threads_subarray);
    MPI_Finalize();

    clock_t end_time = clock(); // end timer
    double elapsed_time = (double)(end_time - start_time) / CLOCKS_PER_SEC;

    return elapsed_time;
}

int main(int argc, char **argv)
{

    if (argc < 4)
    {
        printf("Insufficient arguments. Please provide two values.\nUsage: ./ParallelSort_MPI <seed> <size> <threads>\n");
        return 1;
    }

    int seed = atoi(argv[1]);    // Seed
    long n = atoi(argv[2]);      // Size of input
    int threads = atoi(argv[3]); // Size of input
    int *arr = (int *)calloc(n, sizeof(int));

    generate_input_values(arr, n, seed);

    // hybrid_psrs_sort(arr, n, threads);

    // printf("Seed %i, Size %li, Processors %i\n", seed, n, threads);
    printf("Hybrid: %f\n", hybrid_psrs_sort(arr, n, threads));

    free(arr);
    return 0;
}

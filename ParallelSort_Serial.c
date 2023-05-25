//*********************************************************************************
// Filename : 'psrs.c'
//
// Function : Parallel sorting by regular sampling (using quick sort for local
// sorting) implemented in serial
//
// Author : Benjin Zhu
//
// Date : 2017/08
// Url: https://github.com/poodarchu/parallel-sorting-by-regular-sampling
//
//*********************************************************************************

#include <stdio.h>
#include <stdlib.h>
#include "math.h"
#include "time.h"
#include "common.c"

// parallel sort regular sampling

// partition the data into equal subsets. The pivot used to partition the data is based on a regular sample from sorted sublists of the data.
// PSRS has three phases

// Phase 1 - Pivot Selection
//   Randomly sample the input array to obtain a set of elements to use as samples
//   Sort the sampled array in parallel.
//   Select pivots by dividing the sorted sample array into p-1 equally sized segments, and selecting the element in the middle of each segment as a pivot.

// Phase 2 - Partition the input array
//    Partition the input array using the pivots by counting the number of elements in each partition.
//    Calculate displacements by summing the counts to determine the starting index of each partition.
//    Move elements to their corresponding partitions by iterating over the input array and placing each element in its designated partition.

// Phase 3 - Sorting and Merge
//    Recursively sort the partitions using the same parallel sort algorithm until each partition is sorted.
//    Merge the sorted partitions back into a single sorted array.

// The swap, compare and parallel_sort methods were adapted from a ChatGPT version for parallel openMP into a seriel version

double serial_psrs_sort(int *a, long n, int p)
{
    clock_t start_time = clock(); // start timer

    int size, rsize, sample_size;
    int *sample, *pivots;
    int *partition_borders, *bucket_sizes, *result_positions;
    int **loc_a_ptrs;

    size = (n + p - 1) / p;
    rsize = (size + p - 1) / p;
    sample_size = p * (p - 1);

    loc_a_ptrs = malloc(p * sizeof(int *));
    sample = malloc(sample_size * sizeof(int));
    partition_borders = malloc(p * (p + 1) * sizeof(int));
    bucket_sizes = malloc(p * sizeof(int));
    result_positions = malloc(p * sizeof(int));
    pivots = malloc((p - 1) * sizeof(int));

    int i, j, max, start, end, loc_size, offset, this_result_size;
    int *loc_a, *this_result;

    int loc_sizes[p]; // holds the sizes of each loc_a array
    for (int thread_num = 0; thread_num < p; thread_num++)
    {
        start = thread_num * size;
        end = start + size - 1;
        if (end >= n)
            end = n - 1;
        loc_sizes[thread_num] = (end - start + 1);
        end = end % size;

        loc_a = malloc(loc_sizes[thread_num] * sizeof(int));
        memcpy(loc_a, a + start, loc_sizes[thread_num] * sizeof(int));
        loc_a_ptrs[thread_num] = loc_a;

        sortll(loc_a_ptrs[thread_num], loc_sizes[thread_num]);

        offset = thread_num * (p - 1) - 1;

        // Pick the samples for this simulatd thread
        for (i = 1; i < p; i++)
        {
            if (i * rsize <= end)
            {
                sample[offset + i] = loc_a_ptrs[thread_num][i * rsize - 1];
            }
            else
            {
                sample[offset + i] = loc_a_ptrs[thread_num][end];
            }
        }
    }

    merge_sort(sample, sample_size);

    //   Sort the sampled array in parallel.
    for (i = 0; i < p - 1; i++)
    {
        pivots[i] = sample[i * p + p / 2];
    }

    for (int thread_num = 0; thread_num < p; thread_num++)
    {

        offset = thread_num * (p + 1);
        partition_borders[offset] = 0;
        partition_borders[offset + p] = end + 1;
        calc_partition_borders(loc_a_ptrs[thread_num], 0, loc_sizes[thread_num] - 1, partition_borders, offset, pivots, 1, p - 1);
    }

    for (int thread_num = 0; thread_num < p; thread_num++)
    {

        max = p * (p + 1);
        bucket_sizes[thread_num] = 0;
        for (i = thread_num; i < max; i += p + 1)

        {
            bucket_sizes[thread_num] += partition_borders[i + 1] - partition_borders[i];
        }
    }

    result_positions[0] = 0;
    for (i = 1; i < p; i++)
    {
        result_positions[i] = bucket_sizes[i - 1] + result_positions[i - 1];
    }

    for (int thread_num = 0; thread_num < p; thread_num++)
    {
        this_result = a + result_positions[thread_num];

        if (thread_num == p - 1)
        {
            this_result_size = n - result_positions[thread_num];
        }
        else
        {
            this_result_size = result_positions[thread_num + 1] - result_positions[thread_num];
        }

        this_result = a + result_positions[thread_num];

        for (i = 0, j = 0; i < p; i++)
        {
            int low, high, partition_size;
            offset = i * (p + 1) + thread_num;
            low = partition_borders[offset];
            high = partition_borders[offset + 1];
            partition_size = (high - low);
            if (partition_size > 0)
            {
                memcpy(this_result + j, &(loc_a_ptrs[i][low]), partition_size * sizeof(int));
                j += partition_size;
            }
        }

        sortll(this_result, this_result_size);
    }

    free(loc_a);

    free(loc_a_ptrs);
    free(sample);
    free(partition_borders);
    free(bucket_sizes);
    free(result_positions);
    free(pivots);

    clock_t end_time = clock();
    double elapsed_time = (double)(end_time - start_time) / CLOCKS_PER_SEC;

    if (!(checkSorted(a, n)))
    {
        printf("Error: The array has not been sorted\n");
    }

    return elapsed_time;
}

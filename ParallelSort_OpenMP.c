//*********************************************************************************
// Filename : 'psrs_omp.c'
//
// Function : Parallel sorting by regular sampling (using quick sort for local
// sorting) using OpenMP
//
// Author : Benjin Zhu
//
// Date : 2017/08
// Url: https://github.com/poodarchu/parallel-sorting-by-regular-sampling
//
//*********************************************************************************
#include "math.h"
#include "time.h"
#include <omp.h>

double openMP_psrs_sort(int *a, long n, int p) // issue when p = 0 and we commment out the n<=10000 part
{
    clock_t start_time = clock(); // start timer

    int size, rsize, sample_size;
    int *sample, *pivots;
    int *partition_borders, *bucket_sizes, *result_positions;
    int **loc_a_ptrs;

    omp_set_num_threads(p);

    size = (n + p - 1) / p;
    rsize = (size + p - 1) / p;
    sample_size = p * (p - 1);

    loc_a_ptrs = malloc(p * sizeof(int *));
    sample = malloc(sample_size * sizeof(int));
    partition_borders = malloc(p * (p + 1) * sizeof(int));
    bucket_sizes = malloc(p * sizeof(int));
    result_positions = malloc(p * sizeof(int));
    pivots = malloc((p - 1) * sizeof(int));

#pragma omp parallel
    {
        int i, j, max, thread_num, start, end, loc_size, offset, this_result_size;
        int *loc_a, *this_result;

        thread_num = omp_get_thread_num();
        start = thread_num * size;
        end = start + size - 1;
        if (end >= n)
            end = n - 1;
        loc_size = (end - start + 1);
        end = end % size;

        loc_a = malloc(loc_size * sizeof(int));
        memcpy(loc_a, a + start, loc_size * sizeof(int));
        loc_a_ptrs[thread_num] = loc_a;

        sortll(loc_a, loc_size);

        offset = thread_num * (p - 1) - 1;

        for (i = 1; i < p; i++)
        {
            if (i * rsize <= end)
            {
                sample[offset + i] = loc_a[i * rsize - 1];
            }
            else
            {
                sample[offset + i] = loc_a[end];
            }
        }

#pragma omp barrier
#pragma omp single
        {
            merge_sort(sample, sample_size);
            for (i = 0; i < p - 1; i++)
            {
                pivots[i] = sample[i * p + p / 2];
            }
        }

#pragma omp barrier

        offset = thread_num * (p + 1);
        partition_borders[offset] = 0;
        partition_borders[offset + p] = end + 1;

        calc_partition_borders(loc_a, 0, loc_size - 1, partition_borders, offset, pivots, 1, p - 1);

#pragma omp barrier

        max = p * (p + 1);
        bucket_sizes[thread_num] = 0;
        for (i = thread_num; i < max; i += p + 1)
        {
            bucket_sizes[thread_num] += partition_borders[i + 1] - partition_borders[i];
        }

#pragma omp barrier

#pragma omp single
        {
            result_positions[0] = 0;
            for (i = 1; i < p; i++)
            {
                result_positions[i] = bucket_sizes[i - 1] + result_positions[i - 1];
            }
        }

#pragma omp barrier

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

#pragma omp barrier
        free(loc_a);
    } // close parallel

    free(loc_a_ptrs);
    free(sample);
    free(partition_borders);
    free(bucket_sizes);
    free(result_positions);
    free(pivots);

    clock_t end_time = clock(); // start timer
    double elapsed_time = (double)(end_time - start_time) / CLOCKS_PER_SEC;

    if (!(checkSorted(a, n)))
    {
        printf("Error: The array has not been sorted\n");
    }

    return elapsed_time;
} // end method

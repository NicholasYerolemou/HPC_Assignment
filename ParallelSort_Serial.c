#include <stdio.h>
// #include "ParallelSort_Serial.h"
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

// https://github.com/poodarchu/parallel-sorting-by-regular-sampling/blob/master/omp/test.c

// int main(int argc, char **argv)
// {
// }

double serial_psrs_sort(int *a, long n, int p);
double serial_psrs_sort(int *a, long n, int p)
{
    clock_t start_time = clock(); // start timer
    if (n > 1)
    {
        if (n <= 55)
        {
            insertion_sort(a, n);
        }
        else if (n <= 10000)
        {
            merge_sort(a, n);
        }
        else
        {

            int size, rsize, sample_size;
            int *sample, *pivots;
            int *partition_borders, *bucket_sizes, *result_positions;
            int **loc_a_ptrs;

            // p = omp_get_max_threads();

            // not sure what this is for
            //  p = p * p * p;
            //  if (p > n)
            //  {
            //      p = floor(pow(n, 0.33));
            //      p -= p % 2;
            //  }
            //  else
            //  {
            //      p = omp_get_max_threads();
            //      p -= p % 2;
            //  }
            //  omp_set_num_threads(p);

            size = (n + p - 1) / p;
            rsize = (size + p - 1) / p;
            sample_size = p * (p - 1);

            loc_a_ptrs = malloc(p * sizeof(int *));
            sample = malloc(sample_size * sizeof(int));
            partition_borders = malloc(p * (p + 1) * sizeof(int));
            bucket_sizes = malloc(p * sizeof(int));
            result_positions = malloc(p * sizeof(int));
            pivots = malloc((p - 1) * sizeof(int));

            // #pragma omp parallel
            // printf("P:%i\n", p);
            int i, j, max, start, end, loc_size, offset, this_result_size;
            int *loc_a, *this_result;
            for (int thread_num = 0; thread_num < p; thread_num++)
            {
                // thread_num = omp_get_thread_num();
                start = thread_num * size;
                end = start + size - 1;
                if (end >= n)
                    end = n - 1;
                loc_size = (end - start + 1);
                // printf("Loc Size: %i\n", loc_size);
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
            }

            merge_sort(sample, sample_size);
            for (i = 0; i < p - 1; i++)
            {
                pivots[i] = sample[i * p + p / 2];
            }

            for (int thread_num = 0; thread_num < p; thread_num++)
            {

                offset = thread_num * (p + 1);
                partition_borders[offset] = 0;
                partition_borders[offset + p] = end + 1;
                calc_partition_borders(loc_a, 0, loc_size - 1, partition_borders, offset, pivots, 1, p - 1);
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
            // end for

            free(loc_a_ptrs);
            free(sample);
            free(partition_borders);
            free(bucket_sizes);
            free(result_positions);
            free(pivots);
        }
    }

    clock_t end_time = clock();
    double elapsed_time = (double)(end_time - start_time) / CLOCKS_PER_SEC;

    return elapsed_time;
}

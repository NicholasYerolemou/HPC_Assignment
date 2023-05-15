#include <stdio.h>
#include "ParallelSort_Serial.h"
#include <stdlib.h>
#include "math.h"
#include "time.h"

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

int lcompare(const void *ptr2num1, const void *ptr2num2)
{
    int num1 = *((int *)ptr2num1);
    int num2 = *((int *)ptr2num2);

    if (num1 > num2)
        return 1;
    else if (num1 < num2)
        return -1;
    else
        return 0;
}

int *merge_sort(int *arr, int size)
{
    if (size > 1)
    {
        int middle = size / 2, i;
        int *left, *right;
        left = arr;
        right = arr + middle;

        left = merge_sort(left, middle);
        right = merge_sort(right, size - middle);
        return merge(left, right, middle, size - middle);
    }
    else
    {
        return arr;
    }
}

int *merge(int *left, int *right, int l_end, int r_end)
{
    int temp_off, l_off, r_off, size = l_end + r_end;
    int *temp = malloc(sizeof(int) * l_end);

    for (l_off = 0, temp_off = 0; left + l_off != right; l_off++, temp_off++)
    {
        *(temp + temp_off) = *(left + l_off);
    }

    temp_off = 0;
    l_off = 0;
    r_off = 0;

    while (l_off < size)
    {
        if (temp_off < l_end)
        {
            if (r_off < r_end)
            {
                if (*(temp + temp_off) < *(right + r_off))
                {
                    *(left + l_off) = *(temp + temp_off);
                    temp_off++;
                }
                else
                {
                    *(left + l_off) = *(right + r_off);
                    r_off++;
                }
            }
            else
            {
                *(left + l_off) = *(temp + temp_off);
                temp_off++;
            }
        }
        else
        {
            if (r_off < r_end)
            {
                *(left + l_off) = *(right + r_off);
                r_off++;
            }
            else
            {
                printf("\nERROR - merging loop going too far\n");
            }
        }
        l_off++;
    }
    free(temp);
    return left;
}

void insertion_sort(int *arr, int n)
{
    int i, j, k, temp;

    for (i = 1; i <= n; i++)
    {
        for (j = 0; j < i; j++)
        {
            if (arr[j] > arr[i])
            {
                temp = arr[j];
                arr[j] = arr[i];

                for (k = i; k > j; k--)
                    arr[k] = arr[k - 1];

                arr[k + 1] = temp;
            }
        }
    }
}

// determine the boundaries for the sublists of an local array
void calc_partition_borders(int array[], // array being sorted
                            int start,
                            int end, // separate the array into current process range
                            int result[],
                            int at,       // this process start point in result
                            int pivots[], // the pivot values
                            int first_pv, // first pivot
                            int last_pv)  // last pivot
{
    int mid, lowerbound, upperbound, center;
    int pv;

    mid = (first_pv + last_pv) / 2;
    pv = pivots[mid - 1];
    lowerbound = start;
    upperbound = end;
    while (lowerbound <= upperbound)
    {
        center = (lowerbound + upperbound) / 2;
        if (array[center] > pv)
        {
            upperbound = center - 1;
        }
        else
        {
            lowerbound = center + 1;
        }
    }
    result[at + mid] = lowerbound;

    if (first_pv < mid)
    {
        calc_partition_borders(array, start, lowerbound - 1, result, at, pivots, first_pv, mid - 1);
    }
    if (mid < last_pv)
    {
        calc_partition_borders(array, lowerbound, end, result, at, pivots, mid + 1, last_pv);
    }
}

double serial_psrs_sort(int *a, int n, int p)
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
            printf("P:%i\n", p);
            int i, j, max, thread_num, start, end, loc_size, offset, this_result_size;
            int *loc_a, *this_result, *current_a;
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

            // #pragma omp barrier

            // #pragma omp single
            // {
            merge_sort(sample, sample_size);
            for (i = 0; i < p - 1; i++)
            {
                pivots[i] = sample[i * p + p / 2];
            }
            // }

            for (int thread_num = 0; thread_num < p; thread_num++)
            {

                // #pragma omp barrier
                offset = thread_num * (p + 1);
                partition_borders[offset] = 0;
                partition_borders[offset + p] = end + 1;
                calc_partition_borders(loc_a, 0, loc_size - 1, partition_borders, offset, pivots, 1, p - 1);
            }

            for (int thread_num = 0; thread_num < p; thread_num++)
            {

                // #pragma omp barrier
                max = p * (p + 1);
                bucket_sizes[thread_num] = 0;
                for (i = thread_num; i < max; i += p + 1)

                {
                    bucket_sizes[thread_num] += partition_borders[i + 1] - partition_borders[i];
                }
            }

            // #pragma omp barrier

            // #pragma omp single
            // {
            result_positions[0] = 0;
            for (i = 1; i < p; i++)
            {
                result_positions[i] = bucket_sizes[i - 1] + result_positions[i - 1];
            }
            // }

            // #pragma omp barrier

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

            // #pragma omp barrier
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

void sortll(int *a, int len)
{
    qsort(a, len, sizeof(int), lcompare);
}
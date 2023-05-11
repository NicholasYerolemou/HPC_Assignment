#include "ParallelSort_OpenMP.h"
#include "math.h"
#include "time.h"

// https://github.com/poodarchu/parallel-sorting-by-regular-sampling/blob/master/omp/test.c

// int main(int argc, char **argv)
// {

//     int n = 100;
//     long long arr[n];
//     srand(123);

//     for (int i = 0; i < n; ++i) // fill the array with random values
//     {
//         arr[i] = rand() % 1000 + 1;
//         // printf("%i\n", arr[i]);
//     }

//     psrs_sort(arr, n);
//     for (int i = 0; i < n; ++i)
//     {
//         printf("%lli\n", arr[i]);
//     }
//     return 0;
// }

int lcompare(const void *ptr2num1, const void *ptr2num2)
{
    long long num1 = *((long long *)ptr2num1);
    long long num2 = *((long long *)ptr2num2);

    if (num1 > num2)
        return 1;
    else if (num1 < num2)
        return -1;
    else
        return 0;
}

long long *merge_sort(long long *arr, int size)
{
    if (size > 1)
    {
        int middle = size / 2, i;
        long long *left, *right;
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

long long *merge(long long *left, long long *right, int l_end, int r_end)
{
    int temp_off, l_off, r_off, size = l_end + r_end;
    long long *temp = malloc(sizeof(long long) * l_end);

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

void insertion_sort(long long *arr, int n)
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
void calc_partition_borders(long long array[], // array being sorted
                            int start,
                            int end, // separate the array into current process range
                            int result[],
                            int at,             // this process start point in result
                            long long pivots[], // the pivot values
                            int first_pv,       // first pivot
                            int last_pv)        // last pivot
{
    int mid, lowerbound, upperbound, center;
    long long pv;

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

double psrs_sort(long long *a, int n)
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
            int p, size, rsize, sample_size;
            long long *sample, *pivots;
            int *partition_borders, *bucket_sizes, *result_positions;
            long long **loc_a_ptrs;

            p = omp_get_max_threads();
            p = p * p * p;
            if (p > n)
            {
                p = floor(pow(n, 0.33));
                p -= p % 2;
            }
            else
            {
                p = omp_get_max_threads();
                p -= p % 2;
            }
            omp_set_num_threads(p);

            size = (n + p - 1) / p;
            rsize = (size + p - 1) / p;
            sample_size = p * (p - 1);

            loc_a_ptrs = malloc(p * sizeof(long long *));
            sample = malloc(sample_size * sizeof(long long));
            partition_borders = malloc(p * (p + 1) * sizeof(int));
            bucket_sizes = malloc(p * sizeof(int));
            result_positions = malloc(p * sizeof(int));
            pivots = malloc((p - 1) * sizeof(long long));

#pragma omp parallel
            {
                int i, j, max, thread_num, start, end, loc_size, offset, this_result_size;
                long long *loc_a, *this_result, *current_a;

                thread_num = omp_get_thread_num();
                start = thread_num * size;
                end = start + size - 1;
                if (end >= n)
                    end = n - 1;
                loc_size = (end - start + 1);
                end = end % size;

                loc_a = malloc(loc_size * sizeof(long long));
                memcpy(loc_a, a + start, loc_size * sizeof(long long));
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
                        memcpy(this_result + j, &(loc_a_ptrs[i][low]), partition_size * sizeof(long long));
                        j += partition_size;
                    }
                }

                sortll(this_result, this_result_size);

#pragma omp barrier
                free(loc_a);
            }

            free(loc_a_ptrs);
            free(sample);
            free(partition_borders);
            free(bucket_sizes);
            free(result_positions);
            free(pivots);
        }
    }

    clock_t end_time = clock(); // start timer
    double elapsed_time = (double)(end_time - start_time) / CLOCKS_PER_SEC;
    return elapsed_time;
}

void sortll(long long *a, int len)
{
    qsort(a, len, sizeof(long long), lcompare);
}
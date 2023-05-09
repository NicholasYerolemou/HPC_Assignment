#include <stdio.h>
#include <stdlib.h>
#include <omp.h>
#include "ParallelSort_OpenMP.h"

#define SIZE 1000000
// #define NUM_THREADS 4

void regularSamplingSort(int *array, int size)
{
    int i, j, k, numBuckets;
    int **buckets;

    numBuckets = 4;
    buckets = (int **)calloc(numBuckets, sizeof(int *));

#pragma omp parallel for private(j)
    for (i = 0; i < numBuckets; i++)
    {
        buckets[i] = (int *)calloc(size, sizeof(int));
        for (j = 0; j < size; j++)
        {
            buckets[i][j] = 0;
        }
    }

#pragma omp parallel for
    for (i = 0; i < numBuckets; i++)
    {
        int start = i * (size / numBuckets);
        int end = (i + 1) * (size / numBuckets);
        for (j = start; j < end; j++)
        {
            int bucketIndex = omp_get_thread_num();
            buckets[bucketIndex][j] = array[j];
        }
    }

#pragma omp parallel for private(j)
    for (i = 0; i < numBuckets; i++)
    {
        int start = i * (size / numBuckets);
        int end = (i + 1) * (size / numBuckets);
        for (j = start; j < end; j++)
        {
            int temp = 0;
            for (k = 0; k < size; k++)
            {
                if (buckets[i][k] < buckets[i][j])
                {
                    temp++;
                }
            }
            array[start + temp] = buckets[i][j];
        }
    }

    for (i = 0; i < numBuckets; i++)
    {
        free(buckets[i]);
    }
    free(buckets);
}

int main()
{
    int i, *array;

    array = (int *)calloc(SIZE, sizeof(int));
    for (i = 0; i < SIZE; i++)
    {
        array[i] = rand() % 1000;
    }

    // omp_set_num_threads(NUM_THREADS);
    regularSamplingSort(array, SIZE);

    for (i = 0; i < SIZE; i++)
    {
        printf("%d ", array[i]);
    }
    printf("\n");

    free(array);
    return 0;
}

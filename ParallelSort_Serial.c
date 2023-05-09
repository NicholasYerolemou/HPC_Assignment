#include <stdio.h>
#include "ParallelSort_Serial.h"
#include <stdlib.h>

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

int main(int argc, char **argv)
{
    int n = 100; // size of array
    int p = 10;  // number of threads
    // int chunk_size = 1000; // chunk size for qsort

    // allocate memory for array
    int *arr = (int *)malloc(n * sizeof(int));

    srand(123);

    // Generate random number between 0 and 100

    for (int i = 0; i < n; ++i)
    {
        int random_number = rand() % 101;
        arr[i] = random_number;
        // printf("The number is %d\n", i);
    }

    parallel_sort(arr, n, p);

    free(arr);
    return 0;
}

void swap(int *a, int *b) // ChatGPT
{                         // this swaps int's a and b, used to move elements into the correct partition.
    int temp = *a;
    *a = *b;
    *b = temp;
}

int compare(const void *a, const void *b)
{ // used by quicksort - it is used to determine their relative order
    return (*(int *)a - *(int *)b);
}

void parallel_sort(int *arr, int n, int p)
{ // arr - input array, n - num elements in input array, p - num processors
    int i, j, k, num_samples, chunk_size, *samples, *pivots, *counts, *displs;

    // Allocate memory for local samples, pivots, counts and displacements
    num_samples = p - 1;                                // how many samples were taken
    chunk_size = n / p;                                 // number of elements that are assigned to each processor for processing during each iteration of the algorithm.
    samples = (int *)malloc(sizeof(int) * num_samples); // the samples selected from input arrray in order to find the pivots
    pivots = (int *)malloc(sizeof(int) * (p - 1));      // pivots selected

    counts = (int *)calloc(p, sizeof(int)); // This array is used to store the number of elements in each partition after partitioning
    displs = (int *)calloc(p, sizeof(int)); // This array is used to store the starting index of each partition in the output array.
    int *temp = (int *)malloc(n * sizeof(int));

    // Phase 1
    for (i = 0; i < num_samples; i++)
    { // sample the initial array to find the pivots
        samples[i] = arr[rand() % n];
    }

    qsort(samples, num_samples, sizeof(int), compare); // sort the sampled array

    for (i = 0; i < p - 1; i++)
    { // select pivots
        pivots[i] = samples[(i + 1) * (num_samples / p)];
        // printf("The number is %d\n", pivots[i]);
    }

    // Phase 2
    int num_pivots = p - 1;
    for (i = 0; i < n; i++)
    {
        j = 0;
        while (j < num_pivots && arr[i] > pivots[j])
        {
            j++;
        }
        counts[j]++;
    }

    displs[0] = 0;
    for (i = 1; i < p; i++)
    {
        displs[i] = displs[i - 1] + counts[i - 1];
        printf("The number is %d\n", displs[i]);
    }

    // Partition the array
    for (i = 0; i < n; i++)
    {
        j = 0;
        while (j < num_pivots && arr[i] > pivots[j])
        {
            j++;
        }
        temp[displs[j]] = arr[i];
        displs[j]++;
    }

    // Copy the temporary array back to the original array
    for (i = 0; i < n; i++)
    {
        arr[i] = temp[i];
    }

    free(counts);
    free(displs);
    free(temp);

    // Phase 3
    if (p > 1)
    { // there is no need to sort an array of size 1
        int *partition_offsets = (int *)malloc(sizeof(int) * (p + 1));
        partition_offsets[0] = 0;

        for (i = 0; i < p; i++)
        {
            partition_offsets[i + 1] = partition_offsets[i] + chunk_size;
        }
        partition_offsets[p] = n;

        for (i = 0; i < p; i++)
        {
            parallel_sort(arr + partition_offsets[i], partition_offsets[i + 1] - partition_offsets[i], 1);
        }
        free(partition_offsets);
        // Merge the partitions
        for (i = 1; i < p; i++)
        {
            k = 0;
            for (j = 0; j < n; j++)
            {
                if (j >= displs[i] && j < displs[i + 1])
                {
                    continue;
                }
                temp[k] = arr[j];
                k++;
            }

            for (j = 0; j < (displs[i + 1] - displs[i]); j++)
            {
                temp[k] = arr[j + displs[i]];
                k++;
            }

            for (j = 0; j < k; j++)
            {
                arr[j] = temp[j];
            }
        }
    }

    // Clean up
    free(samples);
    free(pivots);
}

//     //     // Phase 3

//     // Recursively sort the partitions
//     for (i = 0; i < p; i++)
//     {
//         int start = (i == 0) ? 0 : displs[i - 1];
//         int end = displs[i];
//         int size = end - start;
//         if (size > 1)
//         {

//             parallel_sort(&arr[start], size, p);
//         }
//     }

//     // Merge the sorted partitions
//     int *sorted = (int *)malloc(sizeof(int) * n);
//     if (sorted == NULL)
//     {
//         printf("Error: memory allocation failed.\n");
//         exit(EXIT_FAILURE);
//     }

//     int *indices = (int *)calloc(p, sizeof(int));

//     if (indices == NULL)
//     {
//         printf("Error: memory allocation failed.\n");
//         exit(EXIT_FAILURE);
//     }
//     for (i = 0; i < n; i++)
//     {
//         int min_index = -1;
//         int min_value = 0;
//         for (j = 0; j < p; j++)
//         {
//             int index = indices[j];
//             if (index < (j == p - 1 ? n : (j + 1) * chunk_size))
//             {
//                 int value = arr[index];
//                 if (min_index == -1 || value < min_value)
//                 {
//                     min_index = j;
//                     min_value = value;
//                 }
//             }
//         }
//         sorted[i] = min_value;
//         indices[min_index]++;
//     }

//     for (int i = 0; i < n; ++i)
//     {
//         printf("%d ", sorted[i]);
//     }

//     // Copy the sorted array back into the original array
//     // memcpy(arr, sorted, sizeof(int) * n);
//     free(sorted);
//     free(indices);
// }
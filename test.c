#include <stdio.h>
#include <stdlib.h>
#include "test.h"

const int p = 10;

void printArray(int arr[], int n)
{
    for (int i = 0; i < n; ++i)
    {
        printf("%d ", arr[i]);
    }
    printf("\n");
}

int main(int argc, char **argv)
{

    int n = 9991;
    // int p = 10;
    int arr[n];
    srand(123);

    for (int i = 0; i < n; ++i) // fill the array with random values
    {
        arr[i] = rand() % 1000 + 1;
        // printf("%i\n", arr[i]);
    }

    parallelSort(arr, n, p);
    // for (int i = 0; i < n; ++i)
    // {
    //     printf("%i", arr[i]);
    // }
    return 0;
}

void parallelSort(int arr[], int n, int p)
{

    // divide the array into p contiguous sublists and sort each in parallel
    // from each sublist we select p-1 elements, that are evenly spaced.

    // To do this we need to create the subarrays, select the pivots from each of them and then sort the pivots

    int numSamples = p * (p - 1);

    int samples[numSamples]; // holds all the samples from all the subarrays.
    int pivots[p - 1];       // check how many this is meant to be

    // select p-1 evenly spaced elements for the pivots
    int size = (n + p - 1) / p;     // size of each subarray - 1 subarray per thread
    int rsize = (size + p - 1) / p; // how much space between each regular sample in the subarrays. i.e. sublist of size 20 with p=5 we need a sample every 5

    // printf("%i\n", size);
    // printf("%i\n", rsize);

    int start, end;
    // Phase 1

    int pos = 0;
    for (int i = 0; i < p; i++) // // loop through all the sublists
    {

        // we create the partition start-end of this sublist
        start = i * size;
        end = start + size - 1;

        if (end >= n)
        {
            end = n - 1;
        }

        qsort(&arr[start], end - start, sizeof(int), cmpfunc); // sort this sublist in ascending order. Not sure if i should add +1 to second parameter(size)

        // select p-1 samples from this sublist
        for (int j = 1; j < p; ++j)
        {

            if (j * rsize <= end)
            {

                samples[pos] = arr[j * rsize]; // this array holds all the samples from every sublist
                pos++;
            }
            else
            {

                samples[pos] = arr[end];
                pos++;
            }
        }
    }

    qsort(&samples[0], numSamples, sizeof(int), cmpfunc); // we sort all the samples we took from all the sublists to get one main list of in order samples that we can select regular pivots from

    for (int i = 0; i < p - 1; ++i)
    {
        pivots[i] = samples[i * p + p / 2];
        printf("%i\n", pivots[i]);
    }

    //     int subsize[p]; // check this
    //     for (int i = 0; i < p; ++i)
    //     {
    //         start = i * size;
    //         end = (i + 1) * size - 1;
    //         if (end >= n)
    //         {
    //             end = n - 1;
    //         }
    //         // int subsize[i * ]; // check this, i dont know where to make it or how big
    //         subsize[i * (p + 1)] = start;
    //         subsize[i * (p + 1) + p] = end + 1;
    //         Sublists(arr, start, end, subsize, 1 * (p + 1), pivots, 1, p - 1);
    //     }

    //     int bucksize[p];
    //     for (int i = 0; i < p; ++i)
    //     {
    //         bucksize[i] = 0;

    //         for (int j = i; j < p * (p + 1); i += p + 1) // check this
    //         {
    //             bucksize[i] = bucksize[i] + subsize[j + 1] - subsize[j];
    //         }
    //     }

    //     for (int i = 1; i < p; ++i)
    //     {
    //         bucksize[i] = bucksize[i] + bucksize[i - 1];
    //     }
    //     bucksize[0] = 0;

    //     for (int i = 0; i < p; ++i) // check this
    //     {
    //         int j = 1;
    //         // 0 <= j <= p
    //         mergeSort(arr, subsize[i + j * (p + 1)], subsize[i + j * (p + 1) + 1]);
    //     }
    // }

    // void Sublists(int arr[], int start, int end, int subsize[], int at, int pivots[], int fp, int lp) // check these
    // {
    //     int mid = (fp + lp) / 2;
    //     int pv = pivots[mid];
    //     int lb = start;
    //     int ub = end;
    //     while (lb <= ub)
    //     {
    //         int center = (lb + ub) / 2;
    //         if (arr[center] > pv)
    //         {
    //             ub = center - 1;
    //         }
    //         else
    //         {
    //             lb = center + 1;
    //         }
    //     }

    //     subsize[at + mid] = lb;

    //     if (fp < mid)
    //     {
    //         Sublists(arr, start, lb - 1, subsize, at, pivots, fp, mid - 1);
    //     }
    //     if (mid < lp)
    //     {
    //         Sublists(arr, lb, end, subsize, at, pivots, mid + 1, lp);
    //     }
}

int cmpfunc(const void *a, const void *b)
{
    return (*(int *)a - *(int *)b);
}

// geeksForGeeks
void mergeSort(int arr[], int l, int r)
{
    if (l < r)
    {
        // Same as (l+r)/2, but avoids
        // overflow for large l and h
        int m = l + (r - l) / 2;

        // Sort first and second halves
        mergeSort(arr, l, m);
        mergeSort(arr, m + 1, r);

        merge(arr, l, m, r);
    }
}

void merge(int arr[], int l,
           int m, int r)
{
    int i, j, k;
    int n1 = m - l + 1;
    int n2 = r - m;

    // Create temp arrays
    int L[n1], R[n2];

    // Copy data to temp arrays
    // L[] and R[]
    for (i = 0; i < n1; i++)
        L[i] = arr[l + i];
    for (j = 0; j < n2; j++)
        R[j] = arr[m + 1 + j];

    // Merge the temp arrays back
    // into arr[l..r]
    // Initial index of first subarray
    i = 0;

    // Initial index of second subarray
    j = 0;

    // Initial index of merged subarray
    k = l;
    while (i < n1 && j < n2)
    {
        if (L[i] <= R[j])
        {
            arr[k] = L[i];
            i++;
        }
        else
        {
            arr[k] = R[j];
            j++;
        }
        k++;
    }

    // Copy the remaining elements
    // of L[], if there are any
    while (i < n1)
    {
        arr[k] = L[i];
        i++;
        k++;
    }

    // Copy the remaining elements of
    // R[], if there are any
    while (j < n2)
    {
        arr[k] = R[j];
        j++;
        k++;
    }
}
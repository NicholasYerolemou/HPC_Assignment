#include <stdio.h>
#include <stdlib.h>

int main(int argc, char **argv)
{

    int n = 1000;
    int p = 10;
    int arr[n];
    srand(time(NULL));

    for (int i = 0; i < n; ++i) // fill the array with random values
    {
        arr[i] = rand() % 1000 + 1;
    }

    return 0;
}

void parallelSort(int *arr, int n, int p)
{

    int samples[p - 1];
    int pivots[p]; // chech how many this is meant to be
    // select p-1 evenly spaced elements for the pivots

    int size = (n + p - 1) / p;
    int rsize = (size + p - 1) / p;

    int start, end;
    // Phase 1

    for (int i = 0; i < p; ++i)
    {
        start = i * size;
        end = (i + 1) * size - 1;

        if (end >= n)
        {
            end = n - 1;
        }

        qsort(arr, start, end, compare);
        for (int j = 1; j < p; ++j)
        {
            if (j * rsize <= end)
            {
                samples[i * p + j] = arr[j * rsize];
            }
            else
            {
                samples[i * p + j] = arr[end];
            }
        }
    }

    qsort(samples, 0, p * (p - 1) - 1, compare);

    for (int i = 0; i < p - 1; ++i)
    {
        pivots[i] = samples[i * p + p / 2];
    }

    int subsize[p]; // check this
    for (int i = 0; i < p; ++i)
    {
        start = i * size;
        end = (i + 1) * size - 1;
        if (end >= n)
        {
            end = n - 1;
        }
        // int subsize[i * ]; // check this, i dont know where to make it or how big
        subsize[i * (p + 1)] = start;
        subsize[i * (p + 1) + p] = end + 1;
        Sublists(arr, start, end, subsize, 1 * (p + 1), pivots, 1, p - 1);
    }

    int bucksize[p];
    for (int i = 0; i < p; ++i)
    {
        bucksize[i] = 0;

        for (int j = i; j < p * (p + 1); i += p + 1) // check this
        {
            bucksize[i] = bucksize[i] + subsize[j + 1] - subsize[j];
        }
    }

    for (int i = 1; i < p; ++i)
    {
        bucksize[i] = bucksize[i] + bucksize[i - 1];
    }
    bucksize[0] = 0;

    for (int i = 0; i < p; ++i) // check this
    {
        mergesort(subsize[i + j * (p + 1)], subsize[i + j * (p + 1) + 1], 0 <= j <= p);
        }
}

void sublists(int *arr, int start, int end, int *subsize, int at, int *pivots, int fp, int lp) // check these
{
    int mid = (fp + lp) / 2;
    int pv = pivots[mid];
    int lb = start;
    int ub = end;
    while (lb <= ub)
    {
        int center = (lb + ub) / 2;
        if (arr[center] > pv)
        {
            ub = center - 1;
        }
        else
        {
            lb = center + 1;
        }
    }

    subsize[at + mid] = lb;

    if (fp < mid)
    {
        Sublists(arr, start, lb - 1, subsize, at, pivots, fp, mid - 1);
    }
    if (mid < lp)
    {
        Sublists(arr, lb, end, subsize, at, pivots, mid + 1, lp);
    }
}

int compare(const void *a, const void *b)
{ // used by quicksort - it is used to determine their relative order
    return (*(int *)a - *(int *)b);
}
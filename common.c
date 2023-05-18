#include <stdio.h>
#include <stdlib.h>
#include <limits.h>

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

void sortll(int *a, int len) { qsort(a, len, sizeof(int), lcompare); }

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

// combines the arrays in arrays into result in numerical order
void merge_lists(int *arrays[], int sizes[], int arraysCount, int result[], int resultSize)
{
  int *counters = (int *)calloc(arraysCount, sizeof(int));
  // Merge the arrays
  int mergedIndex = 0;
  while (mergedIndex < resultSize)
  {
    int minValue = INT_MAX;
    int minIndex = -1;

    // Find the minimum value among the current indices
    for (int i = 0; i < arraysCount; i++)
    {
      if (counters[i] < sizes[i] && arrays[i][counters[i]] < minValue)
      {
        minValue = arrays[i][counters[i]];
        minIndex = i;
      }
    }

    // If no minimum value found, break the loop
    if (minIndex == -1)
    {
      break;
    }

    // Add the minimum value to the result array
    result[mergedIndex] = minValue;
    mergedIndex++;

    // Move the counter for the array with the minimum value
    counters[minIndex]++;
  }

  free(counters);
}
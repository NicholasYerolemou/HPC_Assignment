#include <stdio.h>
#include <omp.h>
#include <stdlib.h>
#include <limits.h>
#include <string.h>
#include <stdbool.h>
#include "pcg-c-basic-0.9/pcg_basic.h"
#include "pcg-c-basic-0.9/pcg_basic.c"

/**
 * Inserts random numbers into the array
 * @param arr the array to insert into
 * @param size the size of the array
 * @param seed the seed to use for the random number generator
 */
void generate_input_values(int *arr, long size, int seed)
{
  pcg32_random_t rng;
  pcg32_srandom_r(&rng, seed, 0);
  for (int i = 0; i < size; i++)
  {
    arr[i] = abs(pcg32_random_r(&rng)) % size;
  }
}

/***
 * Prints an array
 * @param list the array to print
 * @param len the length of the array
 * @param initial_msg the message to print before the array
 * @param proc the processor number
 */
void print_array(int *list, int len, char *initial_msg, int proc)
{
  char msg[strlen(initial_msg) + 12];
  strcpy(msg, initial_msg);
  char proc_str[11];
  sprintf(proc_str, " %d", proc);
  strcat(msg, proc_str);
  printf("%s: ", msg);
  for (int i = 0; i < len; i++)
  {
    printf("%d ", list[i]);
  }
  printf("\n");
  printf("\n");
}

/**
 * Checks if an array is sorted
 * @param arr the array to check
 * @param size the size of the array
 */
bool checkSorted(int *arr, long size)
{
  for (int i = 1; i < size; ++i)
  {
    if (arr[i] < arr[i - 1])
    {
      printf("%d  %d\n", arr[i], arr[i - 1]);
      return false;
    }
  }
  return true;
}

/**
 * Compares two integers
 * @param a the first integer
 * @param b the second integer
 * @return the difference between the two integers
 */
int cmp(const void *a, const void *b) { return (*(int *)a - *(int *)b); }

/**
 * Compares two integers
 * @param ptr2num1 the first integer
 * @param ptr2num2 the second integer
 */
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

/**
 * Sorts an array using quicksort
 * @param a the array to sort
 * @param len the length of the array
 */
void sortll(int *a, int len) { qsort(a, len, sizeof(int), lcompare); }

/**
 * Merges two sorted arrays into a single one
 * @param left the left array
 * @param right the right array
 * @param l_end the length of the left array
 * @param r_end the length of the right array
 * @return the merged array
 */
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

/**
 * Sorts an array using mergesort
 * @param arr the array to sort
 * @param size the size of the array
 */
int *merge_sort(int *arr, int size)
{
  if (size > 1)
  {
    int middle = size / 2;
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

/**
 * Calculates the partition borders for a parallel quicksort algorithm
 * @param array the array being sorted
 * @param start the start of the array
 * @param end the end of the array
 * @param result the result array
 * @param at the start of the result array
 * @param pivots the pivot values
 * @param first_pv the first pivot
 * @param last_pv the last pivot
 */
void calc_partition_borders(int array[], int start, int end, int result[], int at, int pivots[], int first_pv, int last_pv)
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

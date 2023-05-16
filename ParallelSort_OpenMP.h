#include "omp.h"
#include "stdio.h"
#include "string.h"
#include "stdlib.h"

int OpenMP_lcompare(const void *ptr2num1, const void *ptr2num2);
int *OpenMP_merge(int *left, int *right, int l_end, int r_end);
int *OpenMP_merge_sort(int *arr, int size);
void OpenMP_insertion_sort(int *arr, int n);
void OpenMP_calc_partition_borders(int array[], int start, int end, int sublist_sizes[], int at, int pivots[], int first_p, int last_p);
double OpenMP_psrs_sort(int *a, int n, int p);
void OpenMP_sortll(int *a, int len);
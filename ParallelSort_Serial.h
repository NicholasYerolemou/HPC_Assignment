#include "stdio.h"
#include "string.h"
#include "stdlib.h"

int lcompare(const void *ptr2num1, const void *ptr2num2);
int *merge(int *left, int *right, int l_end, int r_end);
int *merge_sort(int *arr, int size);
void insertion_sort(int *arr, int n);
void calc_partition_borders(int array[], int start, int end, int sublist_sizes[], int at, int pivots[], int first_p, int last_p);
double serial_psrs_sort(int *a, int n, int p);
void sortll(int *a, int len);
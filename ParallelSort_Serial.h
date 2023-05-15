#include "stdio.h"
#include "string.h"
#include "stdlib.h"

int lcompare(const void *ptr2num1, const void *ptr2num2);
long *merge(long *left, long *right, int l_end, int r_end);
long *merge_sort(long *arr, int size);
void insertion_sort(long *arr, int n);
void calc_partition_borders(long array[], int start, int end, int sublist_sizes[], int at, long pivots[], int first_p, int last_p);
double psrs_sort(long *a, int n, int p);
void sortll(long *a, int len);
#ifndef PARALLEL_SORT_H
#define PARALLEL_SORT_H

#include <stdio.h>
#include <stdlib.h>

void swap(int *a, int *b);
int compare(const void *a, const void *b);
void parallel_sort(int *arr, int n, int p);

#endif /* PARALLEL_SORT_H */

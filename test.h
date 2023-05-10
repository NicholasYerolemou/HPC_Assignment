#ifndef PARALLELSORT_H
#define PARALLELSORT_H

#include <stdio.h>
#include <stdlib.h>

void parallelSort(int *arr, int n, int p);
void Sublists(int *arr, int start, int end, int *subsize, int at, int *pivots, int fp, int lp);
int cmpfunc(const void *a, const void *b);
void mergeSort(int arr[], int l, int r);
void merge(int arr[], int l, int m, int r);
void printArray();

#endif // PARALLELSORT_H

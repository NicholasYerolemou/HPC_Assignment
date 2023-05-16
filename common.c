#include <stdio.h>
#include <stdlib.h>

int lcompare(const void *ptr2num1, const void *ptr2num2) {
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

int *merge(int *left, int *right, int l_end, int r_end) {
  int temp_off, l_off, r_off, size = l_end + r_end;
  int *temp = malloc(sizeof(int) * l_end);

  for (l_off = 0, temp_off = 0; left + l_off != right; l_off++, temp_off++) {
    *(temp + temp_off) = *(left + l_off);
  }

  temp_off = 0;
  l_off = 0;
  r_off = 0;

  while (l_off < size) {
    if (temp_off < l_end) {
      if (r_off < r_end) {
        if (*(temp + temp_off) < *(right + r_off)) {
          *(left + l_off) = *(temp + temp_off);
          temp_off++;
        } else {
          *(left + l_off) = *(right + r_off);
          r_off++;
        }
      } else {
        *(left + l_off) = *(temp + temp_off);
        temp_off++;
      }
    } else {
      if (r_off < r_end) {
        *(left + l_off) = *(right + r_off);
        r_off++;
      } else {
        printf("\nERROR - merging loop going too far\n");
      }
    }
    l_off++;
  }
  free(temp);
  return left;
}

int *merge_sort(int *arr, int size) {
  if (size > 1) {
    int middle = size / 2, i;
    int *left, *right;
    left = arr;
    right = arr + middle;

    left = merge_sort(left, middle);
    right = merge_sort(right, size - middle);
    return merge(left, right, middle, size - middle);
  } else {
    return arr;
  }
}

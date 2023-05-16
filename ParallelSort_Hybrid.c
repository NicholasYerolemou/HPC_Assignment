#define NDEBUG
#include "common.c"
#include "math.h"
#include "time.h"
#include <assert.h>
#include <mpi.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

double hybrid_psrs_sort(long long *arr, int n, int p);
// int *hybrid_merge_sort(int *arr, int size);
// int *hybrid_merge(int *left, int *right, int l_end, int r_end);
// void hybrid_sortll(int *a, int len);
// int hybrid_lcompare(const void *ptr2num1, const void *ptr2num2);
int main(int argc, char **argv) {
  int size = 100;
  long long numbers[size];

  // Seed the random number generator
  srand(time(NULL));

  // Generate random long long numbers and store them in the array
  for (int i = 0; i < size; i++) {
    numbers[i] = rand();
    // printf("%lld\n", numbers[i]);
  }

  hybrid_psrs_sort(numbers, size, 8);
}

double hybrid_psrs_sort(long long *arr, int n, int p) {
  // spilt data amogst nodes
  // in each node run in parallel sample selection
  // in openMP collect pivots
  // in MPI collect pivots from all nodes
  int sizeMPI, rank;
  MPI_Init(NULL, NULL);
  MPI_Comm_size(MPI_COMM_WORLD, &sizeMPI);
  MPI_Comm_rank(MPI_COMM_WORLD, &rank);

  int size = 4;                               // number of nodes
  int n_per = n / size;                       // amount of arr per node
  int *a = (int *)calloc(n_per, sizeof(int)); // part of arr for this node
  assert(a != NULL);

  MPI_Scatter(arr, n_per, MPI_INT, a, n_per, MPI_INT, 0,
              MPI_COMM_WORLD); // dristribute part of arr amongst nodes

  omp_set_num_threads(p);
  long long rsize, sample_size;
  long long *sample, *pivots;
  long long *partition_borders, *bucket_sizes, *result_positions;
  long long **loc_a_ptrs;
  size = (n + p - 1) / p;
  rsize = (size + p - 1) / p;
  sample_size = p * (p - 1);

#pragma omp parallel // distribute this nodes part of the arr amongst threads
  {
    int i, j, max, thread_num, start, end, loc_size, offset, this_result_size;
    int *loc_a, *this_result, *current_a;

    thread_num = omp_get_thread_num();
    start = thread_num * size;
    end = start + size - 1;

    if (end >= n)
      end = n - 1;
    loc_size = (end - start + 1);
    end = end % size;

    loc_a = malloc(loc_size * sizeof(int));
    memcpy(loc_a, a + start, loc_size * sizeof(int));
    loc_a_ptrs[thread_num] = loc_a;

    sortll(loc_a, loc_size);

    offset = thread_num * (p - 1) - 1;

    for (int i = 1; i < p; i++) {
      if (i * rsize <= end) {
        sample[offset + i] = loc_a[i * rsize - 1];
      } else {
        sample[offset + i] = loc_a[end];
      }
    }
  }

#pragma omp barrier

#pragma omp single
  {
    merge_sort(sample, sample_size);
    for (int i = 0; i < p - 1; i++) {
      pivots[i] = sample[i * p + p / 2];
    }
  }

#pragma omp barrier

  for (int i = 0; i < p; ++i) {
    printf("Pivots: %lld %d", pivots[i], rank);
  }

  // qsort(a, n_per, sizeof(int), cmp);//sorting of threads on this node

  // //done in parallel using openMP
  // int *samples = (int *)calloc(size, sizeof(int));
  // assert(samples != NULL);
  // for (i = 0; i < size; i++)
  //     samples[i] = a[i * size];
  // //
  MPI_Finalize();
  return 0;
}

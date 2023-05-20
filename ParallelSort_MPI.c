//*********************************************************************************
//
// Filename : 'PSRS.c'
//
// Function : Parallel sorting by regular sampling (using quick sort for local
// sorting)
//
// Author : Xingzhong Li
//
// Date : 2018/05
//
//*********************************************************************************

#include <assert.h>
#include <mpi.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include "common.c"

int cmp(const void *a, const void *b) { return (*(int *)a - *(int *)b); }

double mpi_psrs_sort(int *arr, long len)
{
  clock_t start_time = clock(); // end timer
  // initialize MPI environment :
  int size, rank;
  MPI_Init(NULL, NULL);
  MPI_Comm_size(MPI_COMM_WORLD, &size);
  MPI_Comm_rank(MPI_COMM_WORLD, &rank);

  // phase 2 : Scatter data, local sort and regular samples collecte
  long i, j, k;
  double begin, end, t;
  int n_per = len / size;
  int *a = (int *)calloc(n_per, sizeof(int));
  assert(a != NULL);

  // scatter the data from the root process to all processes
  MPI_Scatter(arr, n_per, MPI_INT, a, n_per, MPI_INT, 0, MPI_COMM_WORLD);
  qsort(a, n_per, sizeof(int), cmp);

  int *samples = (int *)calloc(size, sizeof(int));
  assert(samples != NULL);
  for (i = 0; i < size; i++)
    samples[i] = a[i * size];

  // phase 3 : Gather and merge samples, choose and broadcast (size - 1) pivots
  int *samples_all;
  if (rank == 0)
  {
    samples_all = (int *)calloc((size * size), sizeof(int));
    assert(samples_all != NULL);
  }

  MPI_Barrier(MPI_COMM_WORLD);
  // gather the samples data from all processes to the root process
  MPI_Gather(samples, size, MPI_INT, samples_all, size, MPI_INT, 0,
             MPI_COMM_WORLD);

  // Calculate the pivots in the root process

  int *pivots = (int *)calloc((size - 1), sizeof(int));
  assert(pivots != NULL);
  if (rank == 0)
  {
    qsort(samples_all, (size * size), sizeof(int), cmp);
    for (i = 0; i < (size - 1); i++)
      pivots[i] = samples_all[(i + 1) * size];
  }

  MPI_Barrier(MPI_COMM_WORLD);
  // Send pivot values to all processes
  MPI_Bcast(pivots, (size - 1), MPI_INT, 0, MPI_COMM_WORLD);

  // phase 4 : Local data is partitioned

  int index = 0;
  if (size == 1)
  {
    index = -1;
  }
  int *partition_size = (int *)calloc(size, sizeof(int));
  assert(partition_size != NULL);
  for (i = 0; i < n_per; i++)
  {
    if (a[i] > pivots[index])
    {
      index += 1;
      // printf("pivots[Index]: %d\n", pivots[index]);
    }
    if (index == (size - 1))
    {
      // printf("Index: %d", index);
      partition_size[index] = n_per - i;
      break;
    }

    partition_size[index]++;
  }

  // phase 5 : All ith classes are gathered and merged
  int *new_partition_size = (int *)calloc(size, sizeof(int));
  assert(new_partition_size != NULL);

  MPI_Barrier(MPI_COMM_WORLD);
  // Redistribute all amongst all processes
  MPI_Alltoall(partition_size, 1, MPI_INT, new_partition_size, 1, MPI_INT,
               MPI_COMM_WORLD);

  int totalsize = 0;
  for (i = 0; i < size; i++)
    totalsize += new_partition_size[i];
  int *new_partitions = (int *)calloc(totalsize, sizeof(int));
  assert(new_partitions != NULL);

  int *send_dis = (int *)calloc(size, sizeof(int));
  assert(send_dis != NULL);
  int *recv_dis = (int *)calloc(size, sizeof(int));
  assert(recv_dis != NULL);
  send_dis[0] = 0;
  recv_dis[0] = 0;
  for (i = 1; i < size; i++)
  {
    send_dis[i] = send_dis[i - 1] + partition_size[i - 1];
    recv_dis[i] = recv_dis[i - 1] + new_partition_size[i - 1];
  }
  MPI_Barrier(MPI_COMM_WORLD);
  // Redistribute the displacement values amongst all processes
  MPI_Alltoallv(a, partition_size, send_dis, MPI_INT, new_partitions,
                new_partition_size, recv_dis, MPI_INT, MPI_COMM_WORLD);
  qsort(new_partitions, totalsize, sizeof(int), cmp);

  // phase 6 : Root processor collects all the data
  int *recv_count;
  if (rank == 0)
  {
    recv_count = (int *)calloc(size, sizeof(int));
    assert(recv_count != NULL);
  }

  MPI_Barrier(MPI_COMM_WORLD);
  // Gather the displacement values in the root process
  MPI_Gather(&totalsize, 1, MPI_INT, recv_count, 1, MPI_INT, 0, MPI_COMM_WORLD);

  if (rank == 0)
  {
    recv_dis[0] = 0;
    for (i = 1; i < size; i++)
      recv_dis[i] = recv_dis[i - 1] + recv_count[i - 1];
  }

  int *result = NULL;
  if (rank == 0)
    result = (int *)calloc(len, sizeof(int));

  MPI_Barrier(MPI_COMM_WORLD);
  // Gathers the data from all partitions on all processes to the root process
  MPI_Gatherv(new_partitions, totalsize, MPI_INT, result, recv_count, recv_dis,
              MPI_INT, 0, MPI_COMM_WORLD);

  // output sorting result :

  // // free memory :
  // if (rank == 0)
  // {
  // }

  free(a);
  free(samples);
  free(pivots);
  free(partition_size);
  free(new_partition_size);
  free(new_partitions);
  free(send_dis);
  free(recv_dis);
  MPI_Finalize();

  clock_t end_time = clock(); // end timer
  double elapsed_time = (double)(end_time - start_time) / CLOCKS_PER_SEC;
  if (rank == 0)
  {
    if (!(checkSorted(result, len)))
    {
      printf("Error: The array has not been sorted\n");

      free(samples_all);
      free(result);
      free(recv_count);
      return 0;
    }

    free(samples_all);
    free(result);
    free(recv_count);
  }

  return elapsed_time;
}

int main(int argc, char **argv)
{
  if (argc < 3)
  {
    printf("Insufficient arguments. Please provide two values.\nUsage: ./ParallelSort_MPI <seed> <size>\n");
    return 1;
  }

  int seed = atoi(argv[1]); // Seed
  long n = atoi(argv[2]);   // Size of input
  int *arr = (int *)calloc(n, sizeof(int));

  generate_input_values(arr, n, seed);
  // print_array(arr, n, "Input data", 0);

  // printf("Seed %i, Size %li, Processors %i\n", seed, n, 1);
  printf("MPI: %f\n", mpi_psrs_sort(arr, n));

  return 0;
}
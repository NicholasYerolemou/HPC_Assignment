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

#define NDEBUG
#include <assert.h>
#include <mpi.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

//================================================================================
//
// Function name : cmp
//
// Function : input parameter for qsort (sort integer)
//
//================================================================================

// Print the contents of a list
void print_array(int *list, int len, char *initial_msg, int proc) {
  char msg[strlen(initial_msg) + 12];
  strcpy(msg, initial_msg);
  char proc_str[11];
  sprintf(proc_str, " %d", proc);
  strcat(msg, proc_str);
  printf("%s: ", msg);
  for (int i = 0; i < len; i++) {
    printf("%d ", list[i]);
  }
  printf("\n");
  printf("\n");
}

int cmp(const void *a, const void *b) { return (*(int *)a - *(int *)b); }

int main(int argc, char *argv[]) {
  if (argc != 2) {
    printf("\n Error : the number of input parameters is wrong ! \n");
    exit(EXIT_FAILURE);
  }

  int n = atol(argv[1]); // size of array
  int *a_all = NULL;     // array a

  long i, j, k;
  int size, rank;
  double begin, end, t;

  // initialize MPI environment :
  MPI_Init(&argc, &argv);
  MPI_Comm_size(MPI_COMM_WORLD, &size);
  MPI_Comm_rank(MPI_COMM_WORLD, &rank);

  // phase 1 : Initialization
  if (rank == 0) {
    a_all = (int *)calloc(n, sizeof(int));
    assert(a_all != NULL);

    srand(time(NULL));
    for (i = 0; i < n; i++)
      a_all[i] = rand() % n;

    print_array(a_all, n, "Input data", rank);
  }

  // phase 2 : Scatter data, local sort and regular samples collecte
  int n_per = n / size;
  int *a = (int *)calloc(n_per, sizeof(int));
  assert(a != NULL);

  MPI_Scatter(a_all, n_per, MPI_INT, a, n_per, MPI_INT, 0, MPI_COMM_WORLD);
  qsort(a, n_per, sizeof(int), cmp);

  int *samples = (int *)calloc(size, sizeof(int));
  assert(samples != NULL);
  for (i = 0; i < size; i++)
    samples[i] = a[i * size];

  // phase 3 : Gather and merge samples, choose and broadcast (size - 1) pivots
  int *samples_all;
  if (rank == 0) {
    samples_all = (int *)calloc((size * size), sizeof(int));
    assert(samples_all != NULL);
  }

  MPI_Gather(samples, size, MPI_INT, samples_all, size, MPI_INT, 0,
             MPI_COMM_WORLD);
  // MPI_Barrier(MPI_COMM_WORLD);

  int *pivots = (int *)calloc((size - 1), sizeof(int));
  assert(pivots != NULL);
  if (rank == 0) {
    qsort(samples_all, (size * size), sizeof(int), cmp);
    for (i = 0; i < (size - 1); i++)
      pivots[i] = samples_all[(i + 1) * size];
  }
  // MPI_Barrier(MPI_COMM_WORLD);
  MPI_Bcast(pivots, (size - 1), MPI_INT, 0, MPI_COMM_WORLD);
  // MPI_Barrier(MPI_COMM_WORLD);

  // phase 4 : Local data is partitioned
  int index = 0;
  int *partition_size = (int *)calloc(size, sizeof(int));
  assert(partition_size != NULL);
  for (i = 0; i < n_per; i++) {
    if (a[i] > pivots[index]) {
      index += 1;
    }
    if (index == (size - 1)) {
      partition_size[index] = n_per - i;
      break;
    }
    partition_size[index]++;
  }

  // phase 5 : All ith classes are gathered and merged
  int *new_partition_size = (int *)calloc(size, sizeof(int));
  assert(new_partition_size != NULL);
  // MPI_Barrier(MPI_COMM_WORLD);
  MPI_Alltoall(partition_size, 1, MPI_INT, new_partition_size, 1, MPI_INT,
               MPI_COMM_WORLD);
  // MPI_Barrier(MPI_COMM_WORLD);

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
  for (i = 1; i < size; i++) {
    send_dis[i] = send_dis[i - 1] + partition_size[i - 1];
    recv_dis[i] = recv_dis[i - 1] + new_partition_size[i - 1];
  }
  // MPI_Barrier(MPI_COMM_WORLD);
  MPI_Alltoallv(a, partition_size, send_dis, MPI_INT, new_partitions,
                new_partition_size, recv_dis, MPI_INT, MPI_COMM_WORLD);
  qsort(new_partitions, totalsize, sizeof(int), cmp);
  // MPI_Barrier(MPI_COMM_WORLD);

  // phase 6 : Root processor collects all the data
  int *recv_count;
  if (rank == 0) {
    recv_count = (int *)calloc(size, sizeof(int));
    assert(recv_count != NULL);
  }
  // MPI_Barrier(MPI_COMM_WORLD);
  MPI_Gather(&totalsize, 1, MPI_INT, recv_count, 1, MPI_INT, 0, MPI_COMM_WORLD);
  // MPI_Barrier(MPI_COMM_WORLD);
  if (rank == 0) {
    recv_dis[0] = 0;
    for (i = 1; i < size; i++)
      recv_dis[i] = recv_dis[i - 1] + recv_count[i - 1];
  }
  // MPI_Barrier(MPI_COMM_WORLD);
  int *result = NULL;
  if (rank == 0)
    result = (int *)calloc(n, sizeof(int));
  // MPI_Barrier(MPI_COMM_WORLD);
  MPI_Gatherv(new_partitions, totalsize, MPI_INT, result, recv_count, recv_dis,
              MPI_INT, 0, MPI_COMM_WORLD);
  // MPI_Barrier(MPI_COMM_WORLD);

  // output sorting result :
  if (rank == 0) {
    print_array(result, n, "Sorted data", rank);
  }
  // free memory :
  // MPI_Barrier(MPI_COMM_WORLD);
  if (rank == 0) {
    free(a_all);
    free(samples_all);
    free(result);
    free(recv_count);
  }

  // MPI_Barrier(MPI_COMM_WORLD);
  free(a);
  free(samples);
  free(pivots);
  free(partition_size);
  free(new_partition_size);
  free(new_partitions);
  free(send_dis);
  free(recv_dis);
  // MPI_Barrier(MPI_COMM_WORLD);
  MPI_Finalize();
  printf("Finished processing..........................\n");

  return 0;
}

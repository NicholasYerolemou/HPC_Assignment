#include "benchMarks.h"
#include "pcg-c-basic-0.9/pcg_basic.h"
#include "pcg-c-basic-0.9/pcg_basic.c"
#include "ParallelSort_Serial.c"
#include "ParallelSort_OpenMP.c"

int main(int argc, char **argv)
{
    // array size from 100 thousand to 1 billion

    // int seeds[] = {42, 23, 143, 7, 199};
    int numThreads[] = {1, 2, 4, 8, 16, 32};
    int seeds[] = {42, 23, 143};
    // int processors = 8; // num processorss

    for (int p = 0; p < sizeof(numThreads) / sizeof(int); ++p) // loop through num processors
    {
        long size = 100000;                                   // maybe make this a multiple of p(p-1)// 100 thousand - size of the array
        for (int s = 0; s < sizeof(seeds) / sizeof(int); ++s) // loop through each sead and run each sort for the same input array
        {
            // generate array
            pcg32_random_t rng;
            pcg32_srandom_r(&rng, seeds[s], 0);
            uint64_t initstate = 0;
            uint64_t initseq = 0;
            // pcg32_srandom(uint64_t initstate, uint64_t initseq);
            pcg32_srandom_r(&rng, initstate,
                            initseq);

            int *arr = malloc(size * sizeof(int));

            for (int i = 0; i < size; i++)
            {
                arr[i] = pcg32_random_r(&rng);
                // printf(" %i \n", arr[i]);
            }

            printf("Seed %i, Size %li, Processors %i\n", seeds[s], size, numThreads[p]);
            printf("Serial: %f\n", serial_psrs_sort(arr, size, numThreads[p]));
            printf("OpenMP: %f\n", openMP_psrs_sort(arr, size, numThreads[p]));
            free(arr);

            size *= 10; // increase array size by factor of 10
        }
    }

    return 0;
}

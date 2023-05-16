#include "benchMarks.h"
#include "pcg-c-basic-0.9/pcg_basic.h"
#include "pcg-c-basic-0.9/pcg_basic.c"
#include "ParallelSort_Serial.c"
#include "ParallelSort_OpenMP.c"

int main(int argc, char **argv)
{
    // array size from 100 thousand to 1 billion

    // struct rlimit rl; // extend the timelimit of the program so it isnt killed if it runs too long
    // getrlimit(RLIMIT_CPU, &rl);
    // rl.rlim_cur = 10;
    // setrlimit(RLIMIT_CPU, &rl);

    int seeds[] = {42, 23, 143, 7, 199};
    int processors = 8;                                   // num processorss
    int size = 100000;                                    // 100 thousand - size of the array
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

        printf("Seed %i, Size %i, Processors %i\n", seeds[s], size, processors);
        printf("Serial: %f\n", serial_psrs_sort(arr, size, processors));
        printf("OpenMP: %f\n", openMP_psrs_sort(arr, size, processors));
        free(arr);

        size *= 10; // increase array size by factor of 10
    }

    return 0;
}

#include "benchMarks.h"
#include "pcg-c-basic-0.9/pcg_basic.h"
#include "pcg-c-basic-0.9/pcg_basic.c"
#include "ParallelSort_Serial.h"
#include "ParallelSort_Serial.c"

// #include "ParallelSort_OpenMP.h"

// this will contain a suite of tests that must be conducted to test a specific version of PSRS

int main(int argc, char **argv)
{

    pcg32_random_t rng;           // object of the random number generator
    pcg32_srandom_r(&rng, 42, 0); // the first value is a seed value

    uint64_t initstate = 42;
    uint64_t initseq = 0;
    // pcg32_srandom(uint64_t initstate, uint64_t initseq);
    pcg32_srandom_r(&rng, initstate,
                    initseq);

    int size = 100000; // size of the array // get segmentation fault when size>10 000
    int p = 1;         // num processors

    long long arr[size];
    for (int i = 0; i < size; i++)
    {
        arr[i] = pcg32_random_r(&rng);
    }

    printf("Time: %f\n", psrs_sort(arr, size, p));

    // conduct tests with varying array sizes and processor numbers
    // write output to a file

    // what size are the arrays
    // how many cores

    // cores 1,2,4,8,16,32

    return 0;
}
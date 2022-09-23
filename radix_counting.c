#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <string.h>
#include <x86intrin.h>
#include <mm_malloc.h>

unsigned int state = 123;
unsigned int xorshift32()
{
    state ^= state << 13;
    state ^= state >> 17;
    state ^= state << 5;
    return state;
}

void GenerateRandomData(unsigned int *arr, int count, int seed)
{
    state = seed;
    for (int i = 0; i < count; i++)
    {
        arr[i] = xorshift32();
    }
}

// Code:
// Radix sort based on Geeks for Geeks:
// https://www.geeksforgeeks.org/radix-s...
static void RadixSort256(unsigned int *arr, int n)
{
    if (n <= 1)
        return; // Added base case

    unsigned int *output = malloc(n * sizeof(unsigned int)); // new unsigned int[n]; // output array
    int *count = malloc(256 * sizeof(int));                  // new int[256];
    unsigned int *originalArr = arr;                         // So we know which was input

    for (int shift = 0, s = 0; shift < 4; shift++, s += 8)
    {
        // Zero the counts
        for (int i = 0; i < 256; i++)
            count[i] = 0;

        // Store count of occurrences in count[]
        for (int i = 0; i < n; i++)
            count[(arr[i] >> s) & 0xff]++;

        // Change count[i] so that count[i] now contains
        // actual position of this digit in output[]
        for (int i = 1; i < 256; i++)
            count[i] += count[i - 1];

        // Build the output array
        for (int i = n - 1; i >= 0; i--)
        {
            // precalculate the offset as it's a few instructions
            int idx = (arr[i] >> s) & 0xff;

            // Subtract from the count and store the value
            output[--count[idx]] = arr[i];
        }

        // Copy the output array to input[], so that input[]
        // is sorted according to current digit

        // We can just swap the pointers
        unsigned int *tmp = arr;
        arr = output;
        output = tmp;
    }

    // If we switched pointers an odd number of times,
    // make sure we copy before returning
    if (originalArr == output)
    {
        unsigned int *tmp = arr;
        arr = output;
        output = tmp;

        for (int i = 0; i < n; i++)
            arr[i] = output[i];
    }

    free(output);
    free(count);
}

// RadixSort256SIMD: refactoring to add avx512

// void r_count_occurrences(unsigned int *arr, int n, int *count, int shift)
// {
//     // Store count of occurrences in count[]
//     for (int i = 0; i < n; i++)
//         count[(arr[i] >> shift) & 0xff]++;
// }

// typedef double* __restrict__  __attribute__((align_value (64))) Real_ptr;

// typedef unsigned int arr_elem_ptr __attribute__((aligned(64)));
// typedef int count_elem_ptr __attribute__((aligned(32)));
typedef unsigned int *__restrict__ __attribute__((aligned(64))) arr_ptr;
typedef int  *__restrict__ __attribute__((aligned(32))) count_ptr;

// typedef unsigned int *__restrict__ __attribute__((vector_size(64), aligned(64))) arr_ptr;
// typedef int *__restrict__ __attribute__((vector_size(32), aligned(32))) count_ptr;

// typedef unsigned int *arr_ptr;
// typedef int *count_ptr;

inline void r_count_occurrences_middle(
    arr_ptr arr,
    int n,
    count_ptr count,
    int shift)
{
    // __assume_aligned__(arr, 64);
    // __assume_aligned__(count, 32);

    for (int i = 0; i < n; i++)
        count[(arr[i] >> shift) & 0xff]++;

    // for (int i = 0; i < n; i += 16)
    // {
    //     count[(arr[i] >> shift) & 0xff]++;
    //     count[(arr[i + 1] >> shift) & 0xff]++;
    //     count[(arr[i + 2] >> shift) & 0xff]++;
    //     count[(arr[i + 3] >> shift) & 0xff]++;
    //     count[(arr[i + 4] >> shift) & 0xff]++;
    //     count[(arr[i + 5] >> shift) & 0xff]++;
    //     count[(arr[i + 6] >> shift) & 0xff]++;
    //     count[(arr[i + 7] >> shift) & 0xff]++;
    //     count[(arr[i + 8] >> shift) & 0xff]++;
    //     count[(arr[i + 9] >> shift) & 0xff]++;
    //     count[(arr[i + 10] >> shift) & 0xff]++;
    //     count[(arr[i + 11] >> shift) & 0xff]++;
    //     count[(arr[i + 12] >> shift) & 0xff]++;
    //     count[(arr[i + 13] >> shift) & 0xff]++;
    //     count[(arr[i + 14] >> shift) & 0xff]++;
    //     count[(arr[i + 15] >> shift) & 0xff]++;
    // }
}

/**
 * arr - is n long
 * n - the number of elements we are sorting.
 * count - is 256 * 4 bytes
 * shift -
 */
inline void r_count_occurrences(unsigned int *__restrict arr, int n, int *__restrict count, int shift)
{
    // Store count of occurrences in count[]

    /* Ideas...

        count is 256 * 4 bytes == 1024 bytes == 8192 bits == 16 512bit vectors
        Store counts in 16 512bit vectors.
        16 Offsets into count.

        Maybe can do several masks to somehow count all at once.
        Mask off the parts we can do.
        How to find which vector to put it in?
        XXXXXXXXXXXXXXXX


    */
    // for (int i = 0; i < n; i++)
    //     count[(arr[i] >> shift) & 0xff]++;

    // // Handle the case where there's less than 16.
    // if (n <= 16)
    // {
    //     for (int i = 0; i < n; i++)
    //         count[(arr[i] >> shift) & 0xff]++;
    //     return;
    // }

    // // AVX512f can do 512bit vectors. 512/32==16.
    // // So loop on 16 offsets at once.
    // for (int i = 0; i < (n / 16) * 16; i += 16)
    // {
    //     count[(arr[i] >> shift) & 0xff]++;
    //     count[(arr[i + 1] >> shift) & 0xff]++;
    //     count[(arr[i + 2] >> shift) & 0xff]++;
    //     count[(arr[i + 3] >> shift) & 0xff]++;
    //     count[(arr[i + 4] >> shift) & 0xff]++;
    //     count[(arr[i + 5] >> shift) & 0xff]++;
    //     count[(arr[i + 6] >> shift) & 0xff]++;
    //     count[(arr[i + 7] >> shift) & 0xff]++;
    //     count[(arr[i + 8] >> shift) & 0xff]++;
    //     count[(arr[i + 9] >> shift) & 0xff]++;
    //     count[(arr[i + 10] >> shift) & 0xff]++;
    //     count[(arr[i + 11] >> shift) & 0xff]++;
    //     count[(arr[i + 12] >> shift) & 0xff]++;
    //     count[(arr[i + 13] >> shift) & 0xff]++;
    //     count[(arr[i + 14] >> shift) & 0xff]++;
    //     count[(arr[i + 15] >> shift) & 0xff]++;
    // }

    r_count_occurrences_middle(arr, (n / 16) * 16, count, shift);

    // count remainder of elements
    for (int i = (n / 16) * 16; i < n; i++)
    {
        int offset = (arr[i] >> shift) & 0xff;
        count[offset]++;
    }

}

inline void r_update_count_positions(int *count)
{
    // Change count[i] so that count[i] now contains
    // actual position of this digit in output[]
    for (int i = 1; i < 256; i++)
        count[i] += count[i - 1];
}

inline void r_build_output_array(unsigned int *arr, int n, int *count, unsigned int *output, int s)
{
    // Build the output array
    for (int i = n - 1; i >= 0; i--)
    {
        // precalculate the offset as it's a few instructions
        int idx = (arr[i] >> s) & 0xff;

        // Subtract from the count and store the value
        output[--count[idx]] = arr[i];
    }
}

static void RadixSort256SIMD(unsigned int *arr, int n)
{
    if (n <= 1)
        return; // Added base case

    // unsigned int *output = malloc(n * sizeof(unsigned int)); // new unsigned int[n]; // output array
    // int *count = malloc(256 * sizeof(int));                  // new int[256];

    unsigned int *output;
    int *count;

    void *somememory = NULL;
    posix_memalign(&somememory, 64, n * sizeof(unsigned int));
    output = somememory;
    posix_memalign(&somememory, 64, 256 * sizeof(int));
    count = somememory;

    unsigned int *originalArr = arr; // So we know which was input

    for (int shift = 0, s = 0; shift < 4; shift++, s += 8)
    {
        // Zero the counts
        // for (int i = 0; i < 256; i++)
        //     count[i] = 0;
        memset(count, 0, sizeof(int) * 256);

        // Store count of occurrences in count[]
        // for (int i = 0; i < n; i++)
        //     count[(arr[i] >> s) & 0xff]++;
        r_count_occurrences(arr, n, count, s);

        // Change count[i] so that count[i] now contains
        // actual position of this digit in output[]
        // for (int i = 1; i < 256; i++)
        //     count[i] += count[i - 1];
        r_update_count_positions(count);

        // Build the output array
        // for (int i = n - 1; i >= 0; i--)
        // {
        //     // precalculate the offset as it's a few instructions
        //     int idx = (arr[i] >> s) & 0xff;

        //     // Subtract from the count and store the value
        //     output[--count[idx]] = arr[i];
        // }
        r_build_output_array(arr, n, count, output, s);

        // Copy the output array to input[], so that input[]
        // is sorted according to current digit

        // We can just swap the pointers
        unsigned int *tmp = arr;
        arr = output;
        output = tmp;
    }

    // If we switched pointers an odd number of times,
    // make sure we copy before returning
    if (originalArr == output)
    {
        unsigned int *tmp = arr;
        arr = output;
        output = tmp;

        // for (int i = 0; i < n; i++)
        //     arr[i] = output[i];
        memcpy(arr, output, n * sizeof(unsigned int));
    }

    free(output);
    free(count);
}

// Quicksort:

int Partition(unsigned int *data, int lo, int hi)
{
    unsigned int pivot = data[lo + (hi - lo) / 2];
    int i = lo - 1;
    int j = hi + 1;

    for (;;)
    {
        do
        {
        } while (data[++i] < pivot);
        do
        {
        } while (data[--j] > pivot);

        if (i >= j)
            return j;

        // Swap [i] and [j]
        unsigned int tmp = data[i];
        data[i] = data[j];
        data[j] = tmp;
    }
}

void QuickSortLoHi(unsigned int *data, int lo, int hi)
{
    if (lo < hi)
    {
        int p = Partition(data, lo, hi);
        QuickSortLoHi(data, lo, p);
        QuickSortLoHi(data, p + 1, hi);
    }
}

void QuickSort(unsigned int *data, int count)
{
    if (count <= 1)
        return; // Added base case

    QuickSortLoHi(data, 0, count - 1);
}

/**
 * comparison function for qsort.
 */
static int
cmpuint(const void *p1, const void *p2)
{

    if ((unsigned int *const *)p1 > (unsigned int *const *)p2)
    {
        return -1;
    }
    if ((unsigned int *const *)p1 < (unsigned int *const *)p2)
    {
        return 1;
    }
    return 0;
}

/**
 * Sort using stdlib qsort.
 */
void QuickSortStd(unsigned int *data, int count)
{
    qsort(data, count, sizeof(unsigned int), cmpuint);
}

int r_check_results()
{
    int COUNT = 100;
    // unsigned int *arr_original = malloc(sizeof(unsigned int) * COUNT);
    // unsigned int *arr1 = malloc(sizeof(unsigned int) * COUNT);
    // unsigned int *arr2 = malloc(sizeof(unsigned int) * COUNT);
    // unsigned int *arr3 = malloc(sizeof(unsigned int) * COUNT);
    // unsigned int *arr4 = malloc(sizeof(unsigned int) * COUNT);


    unsigned int *arr_original;
    unsigned int *arr1;
    unsigned int *arr2;
    unsigned int *arr3;
    unsigned int *arr4;

    void *somememory = NULL;

    posix_memalign(&somememory, 64, sizeof(unsigned int) * COUNT);
    arr_original = somememory;
    posix_memalign(&somememory, 64, sizeof(unsigned int) * COUNT);
    arr1 = somememory;
    posix_memalign(&somememory, 64, sizeof(unsigned int) * COUNT);
    arr2 = somememory;
    posix_memalign(&somememory, 64, sizeof(unsigned int) * COUNT);
    arr3 = somememory;
    posix_memalign(&somememory, 64, sizeof(unsigned int) * COUNT);
    arr4 = somememory;


    GenerateRandomData(arr_original, COUNT, 123);
    memcpy(arr1, arr_original, sizeof(unsigned int) * COUNT);
    memcpy(arr2, arr_original, sizeof(unsigned int) * COUNT);
    memcpy(arr3, arr_original, sizeof(unsigned int) * COUNT);
    memcpy(arr4, arr_original, sizeof(unsigned int) * COUNT);

    // Run the sorts, and then compare their results to each other.
    QuickSort(arr1, COUNT);
    RadixSort256(arr2, COUNT);
    RadixSort256SIMD(arr3, COUNT);
    QuickSortStd(arr4, COUNT);

    if (memcmp(arr1, arr2, sizeof(unsigned int) * COUNT) < 0)
    {
        return 1;
    }
    if (memcmp(arr1, arr3, sizeof(unsigned int) * COUNT) < 0)
    {
        return 2;
    }
    if (memcmp(arr1, arr4, sizeof(unsigned int) * COUNT) < 0)
    {
        return 3;
    }

    return 0;
}

int main()
{
    int COUNT = 10000;
    // unsigned int *arr = malloc(sizeof(unsigned int) * COUNT); // new unsigned int[COUNT];

    unsigned int *arr;
    void *somememory = NULL;
    posix_memalign(&somememory, 64, sizeof(unsigned int) * COUNT);
    arr = somememory;

    for (int r = 0; r < 10; r++)
    {
        GenerateRandomData(arr, COUNT, 123);
        long startTime = clock();
        for (int i = 0; i < 1; i++)
        {
            // QuickSort(arr, COUNT);
            // QuickSortStd(arr, COUNT);
            RadixSort256(arr, COUNT);
            // RadixSort256SIMD(arr, COUNT);
        }
        long finishTime = clock();
        printf("TIME: %ld\n", finishTime - startTime);
    }
    int sort_failed = r_check_results();
    if (sort_failed)
    {
        printf("SORT TESTS FAILED: %d\n", sort_failed);
    }
    else
    {
        printf("SORT TESTS PASSED\n");
    }
    return sort_failed;
}

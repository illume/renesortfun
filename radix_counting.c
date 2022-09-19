#include <stdio.h>
#include <stdlib.h>
#include <time.h>

unsigned int state = 123;
unsigned int xorshift32()
{
    state ^= state << 13;
    state ^= state >> 17;
    state ^= state << 5;
    return state;
}

void GenerateRandomDataMax(unsigned int *arr, int count, int seed, int max)
{
    state = seed;
    for (int i = 0; i < count; i++)
    {
        arr[i] = xorshift32() % max;
    }
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

int main()
{
    int COUNT = 100;
    unsigned int *arr = malloc(sizeof(unsigned int) * COUNT); // new unsigned int[COUNT];
    for (int r = 0; r < 10; r++)
    {
        long startTime = clock();
        for (int i = 0; i < 1; i++)
        {
            GenerateRandomData(arr, COUNT, 123);
            // QuickSort(arr, COUNT);
            RadixSort256(arr, COUNT);
        }
        long finishTime = clock();
        printf("TIME: %ld\n", finishTime - startTime);
    }
}

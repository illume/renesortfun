#include <immintrin.h>
#include <math.h>
#include <stdio.h>
#include <string.h>
#include <stdint.h>

#define clock __rdtsc
// #define DATA_LENGTH 8192
#define DATA_LENGTH 524288

typedef float *__restrict__ __attribute__((aligned(64))) data_ptr;

/**
 *
 * ```
 * [_mm512_setzero_ps](https://www.intel.com/content/www/us/en/docs/intrinsics-guide/index.html#text=_mm512_setzero_ps&ig_expand=6252)
 * > Return vector of type __m512 with all elements set to zero.
 *
 * [_mm512_add_ps](https://www.intel.com/content/www/us/en/docs/intrinsics-guide/index.html#text=_mm512_add_ps&ig_expand=6252,159)
 * > Add packed single-precision (32-bit) floating-point elements in a and b, and store the results in dst.
 *
 * [_mm512_loadu_ps](https://www.intel.com/content/www/us/en/docs/intrinsics-guide/index.html#text=_mm512_loadu_ps&ig_expand=6252,159,4369)
 * > Load 512-bits (composed of 16 packed single-precision (32-bit) floating-point elements) from memory into dst. mem_addr does not need to be aligned on any particular boundary.
 *
 * [_mm512_load_ps](https://www.intel.com/content/www/us/en/docs/intrinsics-guide/index.html#text=_mm512_load_ps&ig_expand=6252,159,4369,4282)
 * Load 512-bits (composed of 16 packed single-precision (32-bit) floating-point elements) from memory into dst. mem_addr must be aligned on a 64-byte boundary or a general-protection exception may be generated.
 *
 * [_mm512_reduce_add_ps](https://www.intel.com/content/www/us/en/docs/intrinsics-guide/index.html#text=_mm512_reduce_add_ps&ig_expand=6252,159,4369,5660)
 * > Reduce the packed single-precision (32-bit) floating-point elements in a by addition. Returns the sum of all elements in a.
 * ```
 *
 * Have a register of 16 floats for summing up.
 * Loop over 16 elements at a time.
 * Load 16 floats into a register.
 * Add the 16 floats onto the sum register.
 * Reduce the sum register (by adding the 16 values together)
 *
 * Note, the aligned data loading goes faster (when the data is aligned).
 *
 *   TODO: this only handles input array that is 16 byte multiple.
 *       aka, it's broken for general usage.
 *   Probably should:
 *     - handle the first elements up to the 16 byte boundary
 *     - handle the remaining elements at the end.
 */
static float average_avx512(data_ptr data, int length)
{
    __m512 sum_16 = _mm512_setzero_ps();
    for (int i = 0; i < length; i += 16)
    {
        // sum_16 = _mm512_add_ps(sum_16, _mm512_loadu_ps(&(data[i])));
        sum_16 = _mm512_add_ps(sum_16, _mm512_load_ps(&(data[i])));
    }
    float sum = _mm512_reduce_add_ps(sum_16);
    return sum / length;
}

static float average_serial(data_ptr data, int length)
{
    float sum = 0.0;
    for (int i = 0; i < length; i++)
    {
        sum += data[i];
    }
    return sum / length;
}

static int average_test(data_ptr data, int length)
{
    float serial = average_serial(data, length);
    float avx512 = average_avx512(data, length);

    return fabs(avx512 - serial) < 0.0008;
}

static void data_init(data_ptr data, int length)
{
    for (int i = 0; i < length; ++i)
    {
        data[i] = 0.2f;
    }
}

int main(void)
{
    float result = 0.0f;
    float *data;
    // data = malloc(DATA_LENGTH * sizeof(float));

    void *aligned_memory = NULL;
    size_t alignment = 64;
    if (posix_memalign(&aligned_memory, alignment, DATA_LENGTH * sizeof(float)))
    {
        printf("posix_memalign FAILED.\n");
        return 1;
    }
    data = aligned_memory;

    data_init(data, DATA_LENGTH);

    if (!average_test(data, DATA_LENGTH))
    {
        printf("TESTS FAILED.\n");
        return 2;
    }

    result = 0.0f;
    for (int r = 0; r < 10; r++)
    {
        long start_time = clock();
        for (int i = 0; i < 1; i++)
        {
            result += average_serial(data, DATA_LENGTH);
        }
        long finish_time = clock();
        printf("average_serial TIME: %ld\n", finish_time - start_time);
    }

    for (int r = 0; r < 10; r++)
    {
        long start_time = clock();
        for (int i = 0; i < 1; i++)
        {
            result -= average_avx512(data, DATA_LENGTH);
        }
        long finish_time = clock();
        printf("average_avx512 TIME: %ld\n", finish_time - start_time);
    }

    if (fabs(result) > 0.03)
    {
        printf("FAILED. Difference: %f\n", result);
        return 3;
    }
    return 0;
}

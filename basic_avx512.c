/* Basic example showing how to use intrinsics and printing the result.

gcc -O2 -fno-trapping-math -fno-math-errno -fno-signed-zeros -mavx512f basic_avx512.c -o basic_avx512
*/
#include <x86intrin.h>
#include <stdio.h>

int main() {
    __m512d A = {1.0, 2.0, 3.0, 4.0, 5.0, 6.0, 7.0, 8.0 };
    __m512d B = {1.0, 2.0, 3.0, 4.0, 5.0, 6.0, 7.0, 8.0 };
    __m512d C = {0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0 };

    C = _mm512_add_pd(A, B);

    for(int i = 0; i < 8; i++) {
        printf("%f\n", C[i]);
    }

    return 0;
}
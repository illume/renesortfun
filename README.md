# Rene sort fun(ction)

Note: this is just experimental code, please don't use.

Some time ago, MS enabled avx512f in HyperV/WSL so now it would be convenient to work on. As well, AMD are releasing avx512 support in their next generation CPUs so it seems the instruction set isn't dead. I haven't really done any avx512 stuff, so am looking forward to learning that. Another thing I want to try is SIMD everywhere(SIMDe), which allows porting Intrinsics to different architectures. I've used SSE2neon, but SIMDe is much more further along (but still missing many parts of the massive avx512 instruction set). I'm interested to know what sort of performance it can give on apple silicon for a straight port using avx512... if SIMDe supports the instrinsics needed. Not super optimistic I'll finish, but will make a start for sure and will hopefully be able to answer the novel question about the suitability of SIMDe for avx512 and sorting.

## day 0

Just playing around with avx512 and sorting.

- Got a basic sort in C done with meson for building. 
- Also did a basic example of using avx512 intrinsics.
- The sort is a radix counting sort, so should be easier to implement branchless.


### Some resources and notes.

- [SIMDe implementation status avc512f](https://github.com/simd-everywhere/implementation-status/blob/)
- [AVX512 (1 of 3): Introduction and Overview](https://www.youtube.com/watch?v=D-mM6X5xnTY)
- [AVX512 (2 of 3): Programming AVX512 in 3 Different Ways](https://www.youtube.com/watch?v=I3efQKLgsjM)
- [AVX512 (3 of 3): Deep Dive into AVX512 Mechanisms](https://www.youtube.com/watch?v=543a1b-cPmU)

main/x86.md#avx512f)
- [vscode c/C++ intellisense, debugging, code browsing](https://marketplace.visualstudio.com/items?itemName=ms-vscode.cpptools)

sudo apt-get install build-essential gdb gdb-doc

```bash
11:21:05-rene~/dev/renesortfun$ gcc -march=native basic_avx512.c -o basic_avx512
11:21:14-rene~/dev/renesortfun$ ./basic_avx512
2.000000
4.000000
6.000000
8.000000
10.000000
12.000000
14.000000
16.000000
```


meson

sudo apt-get install python3 python3-pip python3-setuptools \
                       python3-wheel ninja-build


Radix counting sort?
[Why is Radix Sort so Fast? Part 2 Radix Sort](https://www.youtube.com/watch?v=ujb2CIWE8zY)


How to print register values in GDB?
https://stackoverflow.com/questions/5429137/how-to-print-register-values-in-gdb

i r a: print all register, include floating point & vector register (xmm, ymm, zmm).
i r <register_name>: print a single register, e.g i r rax, i r eax
i r <register_name_1> <register_name_2> ...: print multiple registers, e.g i r rdi rsi,

p $zmm0 


## day 1

Added some test cases, which compare the results of the different sort functions.
Broke the radix sort down into separate functions.
Started setting up loops which work on 16 elements at a time. Maybe it could do more.

Without knowing the instruction set, it's a "bit" hard to know what's optimal. So the 16 elements at a time is a start. I went offline for a bit, so need to download some manuals or even books next time I do that.

The vscode environment isn't that great for assembly/intrinsics programming. There's not much/any intellisense or docs. I can't get the debugger working at all. Not even sure the debugger supports displaying registers nicely. Gave up on it. This is with gcc and Ubuntu, which doesn't really have good support for avx512f intrinsics either (no man pages for example).

Seems clang and gcc don't auto vectorize anything in there much. To be fair, they are old versions and I haven't hinted the compiler at all. Except I've got an ancient version on WSL ubuntu... clang v10, where current is v15 and gcc is v9.4 and current is v12.2.

Reading through the assembly with:
```bash
cc -Wall -march=native -O3 -ftree-vectorize radix_counting.c -S
cat radix_counting.s
```



opmask registers

The AVX-512 instruction set adds eight new 64-bit registers, named k0 through k7 (512 bits total).

https://www.physicsforums.com/insights/avx-512-assembly-programming-opmask-registers-for-conditional-arithmetic/


- Good talk on SIMD, has some avx512 info in there. Shows different speedups comparing widths... often due to running out of cache when processing more data. [Angus Hewlett - SIMD, vector classes and branchless algorithms for audio synthesis (ADC'17)](https://www.youtube.com/watch?v=cn-5k8fm_u0)



- tool for analyzing code. "Intel?? Architecture Code Analyzer helps you statically analyze the data dependency, throughput and latency of code snippets on Intel?? microarchitectures. The term kernel is used throughout the rest of this document instead of code snippet." [IACA Intel?? Architecture Code Analyzer](https://www.intel.com/content/www/us/en/developer/articles/tool/architecture-code-analyzer.html)

[A Novel Hybrid Quicksort Algorithm Vectorized using AVX-512 on Intel Skylake](https://arxiv.org/pdf/1704.08579.pdf)


[Code Optimization for AVX-512](https://www.youtube.com/watch?v=wWFH-Z01Xxg)

The data of course has a big effect on how the algorithm works. A good test suite probably test for these (and more!) data.

- random
- partially sorted
- first half sorted, second random
- increasing
- decreasing
- rand()%10
- i%(N/10)


There's very little information available on AVX-512 programming.

There's a book "Modern Parallel Programming with C++ and Assembly Language: X86 SIMD Development Using AVX, AVX2, and AVX-512" which I started skimming through.


- [openmp SIMD directives](https://www.openmp.org/spec-html/5.0/openmpsu42.html)
- Some openmp SIMD stuff is available in visual studio. [SIMD Extension to C++ OpenMP in Visual Studio](https://devblogs.microsoft.com/cppblog/simd-extension-to-c-openmp-in-visual-studio/)


- This is [Lecture Parallel Quicksort algorithms](https://www.uio.no/studier/emner/matnat/ifi/INF3380/v10/undervisningsmateriale/inf3380-week12.pdf)
- https://stackoverflow.com/questions/16007640/openmp-parallel-quicksort

- This optimization guide has some good sections on AVX-512. Especially the section "12.2 AVX512 instruction set and ZMM registers". Also has good instructions on making code autovectorize. [Agner Fog "Optimizing software in C++"](https://www.agner.org/optimize/optimizing_cpp.pdf)




"If the Intel compiler is used, then use #pragma vector aligned or the __assume_aligned directive to tell the compiler that the arrays are aligned, and make sure that they are."

https://stackoverflow.com/questions/41639654/how-to-help-gcc-vectorize-c-code


## day 3

For 512bit vectors, it's suggested (by Optimizing software in C++ book and elsewhere) that AVX-512 should work on 64 byte aligned pointers. Malloc doesn't do this, and we can't control the alignment on input arrays always.

Rounding up to 64 byte alignment is suggested as a solution. We can do this instead from malloc, we process the first number of elements in a serial way. Usually malloc gives back data aligned at 16, if our elements are 4 (or power of 2), then this should be possible.


64 byte alignment looks like this: 
```
0, 64, 128, ...
```

Assume we get back a pointer at 16, then it looks like this:
```
16, 20, 24, 28, 32, 36, 40, 44, 48, 52, 56, 60, 64
```

The pseudo code of the loop will then be like this:
```
aligned_data, remaining_data = process_data_upto_64_alignment()
process_aligned_data(aligned_data)
process_remaining_data(remaining_data)
```

The process_aligned_data function can use __assume_aligned, and only process in 512bit aligned chunks. Then we can process the left over data at the end. It should also use __restrict to tell it this data is not aliased. This should give the compiler the best opportunity to work.

Note, the processing at the start, and at the end can use the same serial function.

This explains __assume and __assume_aligned: [Data Alignment to Assist Vectorization](https://www.intel.com/content/www/us/en/developer/articles/technical/data-alignment-to-assist-vectorization.html)




https://stackoverflow.com/questions/41639654/how-to-help-gcc-vectorize-c-code

```
typedef  float  vec2f __attribute__((vector_size (8), aligned (8)));  /* 64 bits; MMX, 3DNow! */
typedef  float  vec4f __attribute__((vector_size (16), aligned (16))); /* 128 bits; SSE */
typedef  float  vec8f __attribute__((vector_size (32), aligned (32))); /* 256 bits; AVX, AVX2 */
typedef  float  vec16f __attribute__((vector_size (64), aligned (64))); /* 512 bits; AVX512F */
```


This flag tells gcc to print out vectorizing debug info: `-fopt-info-vec-all`

How to build in release mode with meson: `--buildtype=release`.
```bash
meson buildfolder --reconfigure --buildtype=release
```


## day 4

Yesterday I played around with autovectorization a lot. Whilst I learnt some, it didn't help with autovectorizing the function I was trying to with it. It was too complex for the autovectorizer in gcc. It would be interesting to know if Intel's icc would be able to do it.

Additionally, I spent some time updating to the latest Windows 11, and Ubuntu 22.04 LTS. Was hoping to perhaps see some difference with gcc or clang and autovectorization with the newer versions. But for this complex function, the answer is no.

I did find out about the posix memory alignment functions. It also made me realize that different compilers implement all the vector size and alignment stuff in slightly different ways. Things like `__assume` and `__assume_aligned__` aren't really things on gcc for example (but are on icc). Doing stuff portably in C is always a challenge, and reminded me how wonderful portable C libraries (like SDL) are. It's very much how the situation was with web browsers and early JavaScript APIs. The MS compiler typically did things in it's own way, but at least clang and gcc collaborate to try and have some sort of standard APIs for things. Intel made there own aligned malloc function because they didn't want to use the posix one on windows. That's an example of non-portability in a way inside a single compiler!

- Another article on using masks. Using the array sum example(strangely popular example in the vectorization literature). It shows how a lot of the algorithms can be similar with AVX-2 and AVX-512, but that AVX-512 does have some extra functionality, not just wider vectors. Made me also think that perhaps prototyping an algorithm with an array language (like numpy) might be a good idea to help understand it better. [Accelerating Compute-Intensive Workloads with Intel?? AVX-512](https://devblogs.microsoft.com/cppblog/accelerating-compute-intensive-workloads-with-intel-avx-512/)
- The benchmark source code from the ms/intel blog above has an interesting feature in that it uses a hack to prevent the compiler from optimizing a function (incrementing a global variable). https://github.com/intel/Developer-Tools-Runtimes-Blogs/blob/master/AVX512_Blog/AverageFloat.cpp#L66
- It was interesting to find out that the mask registers in avx-512 are reusing mmx registers. [AVX-512 Mask Registers, Again](https://travisdowns.github.io/blog/2020/05/26/kreg2.html)

Another interesting thing is that at 03 optimization, I noticed where the algorithm was broken up into multiple functions didn't perform as well as the single function one. This is not supposed to happen these days. Something like a 10% difference, even when using inline. I've seen this before, and is really unfortunate... because it encourages big functions or macro abuse rather than writing nice small functions.

https://learn.microsoft.com/lv-lv/cpp/c-runtime-library/reference/countof-macro?view=msvc-170
```c
#define _countof(array) (sizeof(array) / sizeof(array[0]))
```


- [vectorizing a loop](https://travisdowns.github.io/blog/2019/08/26/vector-inc.html)

> Auto-vectorization explains all the very fast results: gcc only autovectorizes at -O3 but clang does it at -O2 and -O3 by default. The -O3 result for gcc is somewhat slower than clang because it does not unroll the autovectorized loop.

meson defaults to -O2 with gcc... which probably explains lack of vectorization.

- How to use -O3 with meson? (it's enabled with `--buildtype=release` also see for custom flags like -g -03 https://stackoverflow.com/questions/64794068/how-to-set-debug-and-optimization-flag-with-meson)
- How to use clang with meson? (Set up the relevant C/C++ environmental variables. i.e. CC=clang and CXX=clang++. Then run the build using meson build-clang.)

This compiler tool is wonderful: https://godbolt.org/z/JpwcRN You can quickly see how different compilers compile something. It shows which part of the C/C++/Rust code compiles to what assembly. It's got Intel ICC and different versions of compilers, and different architectures.

- Intel icc is available for free as part of oneAPI. [Install Intel oneAPI C++ and Fortran compiler
](https://www.scivision.dev/intel-oneapi-fortran-install/)
- Oh, the [AVX-512](https://en.wikipedia.org/wiki/AVX-512) is actually pretty good.
- The "vectorizor advizor" [Intel_Advisor](https://en.wikipedia.org/wiki/Intel_Advisor)

- Set of resources from Intel on vectorization... [Vectorization Essentials](https://www.intel.com/content/www/us/en/developer/articles/technical/vectorization-essential.html)

- __rdtsc for different timing [__rdtsc](https://learn.microsoft.com/en-us/cpp/intrinsics/rdtsc?view=msvc-170)


I tested a few things in the average_float.c

- _mm512_load_ps was a bit faster than _mm512_loadu_ps as advertised, because it handled aligned data more quickly.
- the serial function was vectorized... but not very well. I didn't get around to changing that into a 16 accumulators and check if it is also vectorized. But I expect it would.

### other implementations and reading

- avx-512 sort, original patch was in C, based on papers. compress store instruction useful,  https://github.com/numpy/numpy/blob/d66ca35de0cae4e41471dc44793c18511eb45109/numpy/core/src/npysort/x86-qsort.dispatch.cpp
    - https://github.com/numpy/numpy/pull/21099#issuecomment-1063422784
    - https://github.com/numpy/numpy/pull/20133
    - matlab sort is faster than numpy sort because it uses multiple cores (prior to AVX-512 patch)
    - does not port well to less than 512 bits because no compress store instruction
    - numpy has a pretty good infrastructure now for sort, including testing and benchmarking.
    - numpy has no current multi core parallel support for sort.
    - apparently better than IPP radix sort
- https://github.com/jmakino/simdsort
- https://github.com/WojciechMula/simd-sort
- https://devmesh.intel.com/projects/parallel-stable-sort-performance-optimization-using-intel-parallel-studio-xe-and-intel-oneapi-hpc-toolkit#project-docs
- https://github.com/numpy/numpy/pull/21099#issuecomment-1063422784
- parallel sort is quite useful was well https://s3-us-west-2.amazonaws.com/near-me-oregon/instances/132/uploads/attachments/custom_attachment/file/17195/Accelerating_Big_Data_Sorting_Performance_To_Speed_Up_The_Time_For_Insights_And_Solutions_ArthurRatz_SC2020.pdf
- https://github.com/arthurratz/intel_parallel_stable_sort_oneapi
- Duplicate values is something some sorts are not robust against. Many algorithms don't seem to have any fast paths for this. [Fast and Robust Vectorized In-Place Sorting of Primitive Types](https://drops.dagstuhl.de/opus/volltexte/2021/13775/pdf/LIPIcs-SEA-2021-3.pdf)
- https://github.com/simd-sorting/fast-and-robust
- https://github.com/xtensor-stack/xsimd
- [In-place Parallel Super Scalar Samplesort (IPS???o)](https://github.com/SaschaWitt/ips4o)
- Has int, double, and key/value int pairs. Also uses openmp. [AVX-512 sort functions](https://github.com/berenger-eu/avx-512-sort)
    - https://arxiv.org/abs/1704.08579
    - ARM SVE here: https://gitlab.inria.fr/bramas/arm-sve-sort
- https://github.com/Tabasz/quick-AVX-512-sort
- bitonic in register sort, plus merge sort using multi core [Efficient Parallel Sort on AVX-512-Based Multi-Core and Many-Core Architectures](https://ieeexplore.ieee.org/document/8855628)
- radix sort of fix length strings [LSD string sort](https://www.informit.com/articles/article.aspx?p=2180073&seqNum=2)
- multiple length string sorting [MSD string sort](https://www.informit.com/articles/article.aspx?p=2180073&seqNum=3)
- [Sorting_network](https://en.wikipedia.org/wiki/Sorting_network)
- [Bitonic_sorter](https://en.wikipedia.org/wiki/Bitonic_sorter)


# Conclusion of day 4

I didn't get around to completing a sort function that uses AVX-512. But did test a few things out on the way there. I think I know how a radix counting sort could be implemented, but it would be quite complex to vectorize. I see others have made vectorized quick sort implementations, and that it can be done.

- books, I did find two good books on SIMD and AVX-512 optimization, but even with these the existing literature is very limited compared to other instruction sets (like SSE). However a lot of the SSE and AVX techniques can be reused with higher widths... but there are new techniques with AVX-512.
- tools, I discovered how to use icc compiler and other intel tools (vectorizor advizor) without monetary cost, how to quickly check the auto-vectorizors in different compilers, learnt some things about the meson build tool.
- techniques, how to help the compiler with alignment and vector size annotations, how to handle the start and end of blocks, timing, how to check auto vectorization logs for why loops/functions aren't vectorizing, how to convert some serial algorithms to vector ones pretty easily, learnt about portability problems and how to solve them, and basic stuff like what flags are required to compile.

## What's next?

Probably taking an existing quick sort implementation and production-izing it would be the quickest path to getting something useful done. To do this, getting the testing infrastructure working well is important. Having different types of data to sort is an important test. Things like ascending, descending, presorted, and random are some of the data sets needed to test with. But also unaligned data, small data, and bigger than cache size data. Having benchmarks which can be easily run on different CPUs is important. CPU dispatch is another tricky thing to do, that can probably be tested most easily with qemu. Of course implementing all the standard testing infrastructure like *san and fuzzing needs to be done.

I still feel like the counting radix sort is worth pursuing, because theoretically there is enough register space. Also, more research could be done to look into existing SIMD sort implementations.


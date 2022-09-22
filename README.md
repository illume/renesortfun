# Rene sort fun(ction)

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



- tool for analyzing code. "Intel® Architecture Code Analyzer helps you statically analyze the data dependency, throughput and latency of code snippets on Intel® microarchitectures. The term kernel is used throughout the rest of this document instead of code snippet." [IACA Intel® Architecture Code Analyzer](https://www.intel.com/content/www/us/en/developer/articles/tool/architecture-code-analyzer.html)

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






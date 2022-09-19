# Rene sort fun(ction)

Some time ago, MS enabled avx512f in HyperV/WSL so now it would be convenient to work on. As well, AMD are releasing avx512 support in their next generation CPUs so it seems the instruction set isn't dead. I haven't really done any avx512 stuff, so am looking forward to learning that. Another thing I want to try is SIMD everywhere(SIMDe), which allows porting Intrinsics to different architectures. I've used SSE2neon, but SIMDe is much more further along (but still missing many parts of the massive avx512 instruction set). I'm interested to know what sort of performance it can give on apple silicon for a straight port using avx512... if SIMDe supports the instrinsics needed. Not super optimistic I'll finish, but will make a start for sure and will hopefully be able to answer the novel question about the suitability of SIMDe for avx512 and sorting.

## day 1

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





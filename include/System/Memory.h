#pragma once

// All the functions here are vectorized versions of the libc memory write
// functions (writing 16-bit or 32-bit values where possible). The assembly is
// extremely weird, with a bunch of conditional returns along the lines of:
//     cmp r2, 0
//     beq YES
//     b NO
//   YES:
//     bx lr
//   NO:
//     [function continues as usual]
// but no settings on the compiler seems to produce such output. (Indeed, unless
// I'm missing something this is strictly worse than using popeq/bxeq as the
// compiler typically does).
// Combined with the low-level nature of these functions, I suspect they may
// have originally been written in assembly / have come from an external
// library (if so, then also compiled with another compiler).

extern "C" {

// func_020ca594 in USA, func_020cc060 in JPN.
void VectorizedMemset(void* dst, int value, unsigned int length);

// func_020ca650 in USA, func_020cc11c in JPN.
// Note the break from convention: unlike libc memcpy, the first parameter
// is the source and second is the destination.
void VectorizedInvertedMemcpy(const void* src, void* dst, unsigned int length);

}
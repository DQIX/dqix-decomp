#pragma once

#ifdef __MWERKS__

#define INTERNAL_ASM_NOP(what) __asm("b " what "\n" what ":")
#define INTERNAL_ASM_NOP_2(num) INTERNAL_ASM_NOP("nop_label__" #num)
#define INTERNAL_ASM_NOP_3(num) INTERNAL_ASM_NOP_2(num)
// Inserts some inline assembly of the form
// ```     b right_here          ```
// ```     right_here:           ```
// with a uniquely generated label to prevent clashes.
// Such an instruction does nothing and is completely optimized out, but it
// prevents some quirks of inline assembly spilling over into later code
// (such as constants being loaded from the end of the function, even if they're
// small enough to be hardcoded into a command). It also disables some
// optimizations that move instructions to more efficient (but not matching)
// places - two C++ instructions on opposite sides of this boundary will 
// (usually?) have their order respected.
#define DECLARE_ASM_NOP() INTERNAL_ASM_NOP_3(__LINE__)

#define ASM_GOTO(label) __asm("b " #label)
#define ASM_LABEL(label) __asm(#label ":")

#else
#define DECLARE_ASM_NOP() ((void)0)

#define ASM_GOTO(label) goto label 
#define ASM_LABEL(label) label: ((void)0)
#endif
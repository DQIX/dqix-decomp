#pragma once

#include "Common.h"

struct Nitro3DBoneMatrix
{
    // bit 0: set if *no* translation data
    // bit 1: set if *no* matrix-based rotation
    // bit 2: set if *no* scaling
    // bit 3: set if there *is* a pivot matrix
    // If there is a pivot matrix, then we also use:
    // bits 4-7: form
    // bit 8: negate 1
    // bit 9: negate c 
    // bit 10: negate d
    // bits 11-15: index of bone matrix on stack?
    uint16_t flags;

    fix16_t m_11;

    struct Translation // present if flag bit 1 is zero. might be a Vector3i
    {
        fix32_t x;
        fix32_t y;
        fix32_t z;
    };

    // present if flag bit 3 is set.
    // The data (form, negate_1, negate_c, negate_d, a, b)
    // encode a PivotMatrix as follows: form is between 0 and 8
    // and encodes the position in a 3x3 matrix (column major, so e.g.
    // 3 means middle entry of top row) of an entry that is +1 or -1.
    // Which one it is depends on negate_1 (set: -1, clear: +1). The other
    // entries in its row and column are zero. The other four entries are
    // a, b, c and d in memory order, where a and b are specified here, and
    // c is either +b or -b (depending on negate_c) and d is either +a or -a
    // (depending on negate_d). Note the nsbmd docs have c paired with a and
    // d paired with b, but (at least in this codebase, and probably more
    // broadly considering the shape of rotation matrices) this is a typo.
    // Pictures of these matrices are available in the nsbmd docs.
    struct PivotMatrixData
    {
        fix16_t a;
        fix16_t b;
    };

    struct RotationMatrixData // present if flag 3 is clear and flag 1 is clear
    {
        fix16_t entries[8];
    };

    struct Scaling // present if flag bit 2 is zero
    {
        fix32_t x;
        fix32_t y;
        fix32_t z;

        fix32_t x_v2;
        fix32_t y_v2;
        fix32_t z_v2;
    };
};
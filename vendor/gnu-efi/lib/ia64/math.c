#include "lib.h"

#ifdef RUNTIME_CODE
#ifndef __GNUC__
#pragma RUNTIME_CODE(LShiftU64)
#pragma RUNTIME_CODE(RShiftU64)
#pragma RUNTIME_CODE(MultU64x32)
#pragma RUNTIME_CODE(DivU64x32)
#endif
#endif

UINT64
LShiftU64 (
    IN UINT64   Operand,
    IN UINTN    Count
    )

{
    return Operand << Count;
}

UINT64
RShiftU64 (
    IN UINT64   Operand,
    IN UINTN    Count
    )

{
    return Operand >> Count;
}

UINT64
MultU64x32 (
    IN UINT64   Multiplicand,
    IN UINTN    Multiplier
    )

{
    return Multiplicand * Multiplier;
}

UINT64
DivU64x32 (
    IN UINT64   Dividend,
    IN UINTN    Divisor,
    OUT UINTN   *Remainder OPTIONAL
    )

{
    ASSERT (Divisor != 0);

    if (Remainder) {
        *Remainder = Dividend % Divisor;
    }

    return Dividend / Divisor;
}

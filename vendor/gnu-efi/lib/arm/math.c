#include "lib.h"

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

    if (Remainder)
        *Remainder = Dividend % Divisor;
    return Dividend / Divisor;
}

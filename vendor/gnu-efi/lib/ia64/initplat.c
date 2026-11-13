#include "lib.h"

VOID
InitializeLibPlatform (
    IN EFI_HANDLE           ImageHandle EFI_UNUSED,
    IN EFI_SYSTEM_TABLE     *SystemTable EFI_UNUSED
    )
{
    PLABEL  SalPlabel;
    UINT64  PalEntry;

    LibInitSalAndPalProc (&SalPlabel, &PalEntry);
}

#include "lib.h"

VOID
InitializeLibPlatform (
    IN EFI_HANDLE           ImageHandle EFI_UNUSED,
    IN EFI_SYSTEM_TABLE     *SystemTable EFI_UNUSED
    )
{
}

#ifdef __GNUC__
void __div0(void)
{

    while (1);
}
#endif

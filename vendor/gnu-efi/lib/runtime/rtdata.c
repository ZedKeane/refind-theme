#include "lib.h"

#ifndef __GNUC__
#pragma BEGIN_RUNTIME_DATA()
#endif

EFI_RUNTIME_SERVICES    *RT;

BOOLEAN  LibFwInstance;

UINTN    EFIDebug    = EFI_DBUG_MASK;

SIMPLE_TEXT_OUTPUT_INTERFACE    *LibRuntimeDebugOut;

EFI_RAISE_TPL   LibRuntimeRaiseTPL   = NULL;
EFI_RESTORE_TPL LibRuntimeRestoreTPL = NULL;

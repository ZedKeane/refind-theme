#include "lib.h"

#ifndef __GNUC__
#pragma RUNTIME_CODE(RtLibEnableVirtualMappings)
#endif
VOID
RUNTIMEFUNCTION
RtLibEnableVirtualMappings (
    VOID
    )
{
    EFI_CONVERT_POINTER     ConvertPointer;

    if (!LibFwInstance) {

        ConvertPointer = RT->ConvertPointer;

        ConvertPointer (EFI_INTERNAL_PTR, (VOID **)&RT);
        ConvertPointer (EFI_OPTIONAL_PTR, (VOID **)&LibRuntimeDebugOut);

        ConvertPointer (EFI_INTERNAL_PTR, (VOID **)&LibRuntimeRaiseTPL);
        ConvertPointer (EFI_INTERNAL_PTR, (VOID **)&LibRuntimeRestoreTPL);

    }
}

#ifndef __GNUC__
#pragma RUNTIME_CODE(RtConvertList)
#endif
VOID
RUNTIMEFUNCTION
RtConvertList (
    IN UINTN                DebugDisposition,
    IN OUT LIST_ENTRY       *ListHead
    )
{
    LIST_ENTRY              *Link;
    LIST_ENTRY              *NextLink;
    EFI_CONVERT_POINTER     ConvertPointer;

    ConvertPointer = RT->ConvertPointer;

    Link = ListHead;
    do {
        NextLink = Link->Flink;

        ConvertPointer (
            Link->Flink == ListHead ? DebugDisposition : 0,
            (VOID **)&Link->Flink
            );

        ConvertPointer (
            Link->Blink == ListHead ? DebugDisposition : 0,
            (VOID **)&Link->Blink
            );

        Link = NextLink;
    } while (Link != ListHead);
}

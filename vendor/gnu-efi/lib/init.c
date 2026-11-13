#include "lib.h"

VOID
EFIDebugVariable (
    VOID
    );

VOID
InitializeLib (
    IN EFI_HANDLE           ImageHandle,
    IN EFI_SYSTEM_TABLE     *SystemTable
    )

{
    EFI_LOADED_IMAGE        *LoadedImage;
    EFI_STATUS              Status;
    CHAR8                   *LangCode;

    if (LibInitialized)
	return;

    LibInitialized = TRUE;
    LibFwInstance = FALSE;
    LibImageHandle = ImageHandle;

    ST = SystemTable;
    BS = SystemTable->BootServices;
    RT = SystemTable->RuntimeServices;

    if (ImageHandle) {
	Status = uefi_call_wrapper(
	    BS->HandleProtocol,
	    3,
	    ImageHandle,
	    &LoadedImageProtocol,
	    (VOID*)&LoadedImage
	);

	if (!EFI_ERROR(Status)) {
	    PoolAllocationType = LoadedImage->ImageDataType;
	}
	EFIDebugVariable ();
    }

    InitializeGuid();

    InitializeLibPlatform(ImageHandle,SystemTable);

    if (ImageHandle && UnicodeInterface == &LibStubUnicodeInterface) {
        LangCode = LibGetVariable (VarLanguage, &EfiGlobalVariable);
        InitializeUnicodeSupport (LangCode);
        if (LangCode) {
            FreePool (LangCode);
        }
    }
}

VOID
InitializeUnicodeSupport (
    CHAR8 *LangCode
    )
{
    EFI_UNICODE_COLLATION_INTERFACE *Ui;
    EFI_STATUS                      Status;
    CHAR8                           *Languages;
    UINTN                           Index, Position, Length;
    UINTN                           NoHandles;
    EFI_HANDLE                      *Handles;

    LibLocateHandle (ByProtocol, &UnicodeCollationProtocol, NULL, &NoHandles, &Handles);
    if (!LangCode || !NoHandles) {
        goto Done;
    }

    for (Index=0; Index < NoHandles; Index++) {
        Status = uefi_call_wrapper(BS->HandleProtocol, 3, Handles[Index], &UnicodeCollationProtocol, (VOID*)&Ui);
        if (EFI_ERROR(Status)) {
            continue;
        }

        Languages = Ui->SupportedLanguages;
        Length = strlena(Languages);
        for (Position=0; Position < Length; Position += ISO_639_2_ENTRY_SIZE) {

            if (CompareMem (Languages+Position, LangCode, ISO_639_2_ENTRY_SIZE) == 0) {
                UnicodeInterface = Ui;
                goto Done;
            }
        }
    }

Done:

    if (Handles) {
        FreePool (Handles);
    }
}

VOID
EFIDebugVariable (
    VOID
    )
{
    EFI_STATUS      Status;
    UINT32          Attributes;
    UINTN           DataSize;
    UINTN           NewEFIDebug;

    DataSize = sizeof(EFIDebug);
    Status = uefi_call_wrapper(RT->GetVariable, 5, L"EFIDebug", &EfiGlobalVariable, &Attributes, &DataSize, &NewEFIDebug);
    if (!EFI_ERROR(Status)) {
        EFIDebug = NewEFIDebug;
    }
}

#ifndef __SIZE_TYPE__
#define __SIZE_TYPE__ UINTN
#endif

void *memset(void *s, int c, __SIZE_TYPE__ n)
{
    unsigned char *p = s;

    while (n--)
        *p++ = c;

    return s;
}

void *memcpy(void *dest, const void *src, __SIZE_TYPE__ n)
{
    const unsigned char *q = src;
    unsigned char *p = dest;

    while (n--)
        *p++ = *q++;

    return dest;
}

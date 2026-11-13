#if !defined(_MSC_VER) && (!defined(__STDC_VERSION__) || (__STDC_VERSION__ < 199901L )) && !defined(__cplusplus)

typedef unsigned long long  uint64_t;
typedef long long           int64_t;
typedef unsigned int        uint32_t;
typedef int                 int32_t;
typedef unsigned short      uint16_t;
typedef short               int16_t;
typedef unsigned char       uint8_t;
typedef signed char         int8_t;
typedef uint32_t            uintptr_t;
typedef int32_t             intptr_t;

#else
#include <stdint.h>
#endif

#if defined(__GNUC__) && !__STDC_HOSTED__
#pragma GCC visibility push (hidden)
#endif

#include <stddef.h>

typedef wchar_t CHAR16;
#define WCHAR CHAR16

typedef uint64_t   UINT64;
typedef int64_t    INT64;

typedef uint32_t   UINT32;
typedef int32_t    INT32;

typedef uint16_t   UINT16;
typedef int16_t    INT16;

typedef uint8_t    UINT8;
typedef char       CHAR8;
typedef int8_t     INT8;

#undef VOID
typedef void       VOID;

typedef int32_t    INTN;
typedef uint32_t   UINTN;

#define EFIERR(a)           (0x80000000 | a)
#define EFI_ERROR_MASK      0x80000000
#define EFIERR_OEM(a)       (0xc0000000 | a)

#define BAD_POINTER         0xFBFBFBFB
#define MAX_ADDRESS         0xFFFFFFFF

#define BREAKPOINT()        while (TRUE);

#define MIN_ALIGNMENT_SIZE  4

#define ALIGN_VARIABLE(Value ,Adjustment) \
            (UINTN)Adjustment = 0; \
            if((UINTN)Value % MIN_ALIGNMENT_SIZE) \
                (UINTN)Adjustment = MIN_ALIGNMENT_SIZE - ((UINTN)Value % MIN_ALIGNMENT_SIZE); \
            Value = (UINTN)Value + (UINTN)Adjustment

#define EFI_SIGNATURE_16(A,B)             ((A) | (B<<8))
#define EFI_SIGNATURE_32(A,B,C,D)         (EFI_SIGNATURE_16(A,B)     | (EFI_SIGNATURE_16(C,D)     << 16))
#define EFI_SIGNATURE_64(A,B,C,D,E,F,G,H) (EFI_SIGNATURE_32(A,B,C,D) | ((UINT64)(EFI_SIGNATURE_32(E,F,G,H)) << 32))

#ifndef EFIAPI
#define EFIAPI
#endif

#define BOOTSERVICE
#define RUNTIMESERVICE
#define RUNTIMEFUNCTION

#define RUNTIME_CODE(a)         alloc_text("rtcode", a)
#define BEGIN_RUNTIME_DATA()    data_seg("rtdata")
#define END_RUNTIME_DATA()      data_seg("")

#define VOLATILE                volatile

#define MEMORY_FENCE            __sync_synchronize

#if defined(_MSC_EXTENSIONS)
#define EFI_DRIVER_ENTRY_POINT(InitFunction) \
    __pragma(comment(linker, "/ENTRY:" # InitFunction))
#else
#define EFI_DRIVER_ENTRY_POINT(InitFunction)    \
    UINTN                                       \
    InitializeDriver (                          \
        VOID    *ImageHandle,                   \
        VOID    *SystemTable                    \
        )                                       \
    {                                           \
        return InitFunction(ImageHandle,        \
                SystemTable);                   \
    }                                           \
                                                \
    EFI_STATUS efi_main(                        \
        EFI_HANDLE image,                       \
        EFI_SYSTEM_TABLE *systab                \
        ) __attribute__((weak,                  \
                alias ("InitializeDriver")));
#endif

#define LOAD_INTERNAL_DRIVER(_if, type, name, entry)    \
        (_if)->LoadInternal(type, name, entry)

#define INTERFACE_DECL(x) struct x

#define uefi_call_wrapper(func, va_num, ...) func(__VA_ARGS__)
#define EFI_FUNCTION

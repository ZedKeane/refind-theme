#ifndef _EFI_DEF_H
#define _EFI_DEF_H

#if !defined(__cplusplus)
#if defined(__STDC_VERSION__) && __STDC_VERSION__ >= 199901L
typedef _Bool BOOLEAN;
#else
typedef unsigned char BOOLEAN;
#endif
#else
typedef bool BOOLEAN;
#endif

#ifndef CONST
   #define CONST const
#endif
#ifndef TRUE
#if defined(__cplusplus) || (defined(__STDC_VERSION__) && __STDC_VERSION__ >= 202311L)
    #define TRUE    true
    #define FALSE   false
#else
    #define TRUE    ((BOOLEAN) 1)
    #define FALSE   ((BOOLEAN) 0)
#endif
#endif

#ifndef NULL
#if (defined(__cplusplus) && __cplusplus >= 201103L) || (defined(__STDC_VERSION__) && __STDC_VERSION__ >= 202311L)
    #define NULL    nullptr
#else
#if !defined(__cplusplus)
    #define NULL    ((VOID *) 0)
#else
    #define NULL    0
#endif
#endif
#endif

typedef UINTN           EFI_STATUS;
typedef UINT64          EFI_LBA;
typedef UINTN           EFI_TPL;
typedef VOID            *EFI_HANDLE;
typedef VOID            *EFI_EVENT;

#ifndef IN
    #define IN
    #define OUT
    #define OPTIONAL
#endif

typedef struct {
    UINT32  Data1;
    UINT16  Data2;
    UINT16  Data3;
    UINT8   Data4[8];
} EFI_GUID;

typedef struct {
    UINT16      Year;
    UINT8       Month;
    UINT8       Day;
    UINT8       Hour;
    UINT8       Minute;
    UINT8       Second;
    UINT8       Pad1;
    UINT32      Nanosecond;
    INT16       TimeZone;
    UINT8       Daylight;
    UINT8       Pad2;
} EFI_TIME;

#define EFI_TIME_ADJUST_DAYLIGHT    0x01
#define EFI_TIME_IN_DAYLIGHT        0x02

#define EFI_UNSPECIFIED_TIMEZONE    0x07FF

typedef struct {
    UINT8                   Addr[4];
} EFI_IPv4_ADDRESS;

typedef struct {
    UINT8                   Addr[16];
} EFI_IPv6_ADDRESS;

typedef struct {
    UINT8                   Addr[32];
} EFI_MAC_ADDRESS;

typedef struct {
    UINT32 ReceivedQueueTimeoutValue;
    UINT32 TransmitQueueTimeoutValue;
    UINT16 ProtocolTypeFilter;
    BOOLEAN EnableUnicastReceive;
    BOOLEAN EnableMulticastReceive;
    BOOLEAN EnableBroadcastReceive;
    BOOLEAN EnablePromiscuousReceive;
    BOOLEAN FlushQueuesOnReset;
    BOOLEAN EnableReceiveTimestamps;
    BOOLEAN DisableBackgroundPolling;
} EFI_MANAGED_NETWORK_CONFIG_DATA;

typedef UINT64          EFI_PHYSICAL_ADDRESS;
typedef UINT64          EFI_VIRTUAL_ADDRESS;

typedef enum {
    AllocateAnyPages,
    AllocateMaxAddress,
    AllocateAddress,
    MaxAllocateType
} EFI_ALLOCATE_TYPE;

typedef enum {
    EfiReservedMemoryType,
    EfiLoaderCode,
    EfiLoaderData,
    EfiBootServicesCode,
    EfiBootServicesData,
    EfiRuntimeServicesCode,
    EfiRuntimeServicesData,
    EfiConventionalMemory,
    EfiUnusableMemory,
    EfiACPIReclaimMemory,
    EfiACPIMemoryNVS,
    EfiMemoryMappedIO,
    EfiMemoryMappedIOPortSpace,
    EfiPalCode,
    EfiPersistentMemory,
    EfiUnacceptedMemoryType,
    EfiMaxMemoryType
} EFI_MEMORY_TYPE;

#define EFI_MEMORY_UC            0x0000000000000001
#define EFI_MEMORY_WC            0x0000000000000002
#define EFI_MEMORY_WT            0x0000000000000004
#define EFI_MEMORY_WB            0x0000000000000008
#define EFI_MEMORY_UCE           0x0000000000000010

#define EFI_MEMORY_WP            0x0000000000001000
#define EFI_MEMORY_RP            0x0000000000002000
#define EFI_MEMORY_XP            0x0000000000004000
#define EFI_MEMORY_RO            0x0000000000020000

#define EFI_MEMORY_NV            0x0000000000008000
#define EFI_MEMORY_RUNTIME       0x8000000000000000

#define EFI_MEMORY_MORE_RELIABLE 0x0000000000010000
#define EFI_MEMORY_SP            0x0000000000040000
#define EFI_MEMORY_CPU_CRYPTO    0x0000000000080000
#define EFI_MEMORY_ISA_VALID     0x4000000000000000
#define EFI_MEMORY_ISA_MASK      0x0FFFF00000000000

#define EFI_MEMORY_DESCRIPTOR_VERSION  1
typedef struct {
    UINT32                          Type;
    UINT32                          Pad;
    EFI_PHYSICAL_ADDRESS            PhysicalStart;
    EFI_VIRTUAL_ADDRESS             VirtualStart;
    UINT64                          NumberOfPages;
    UINT64                          Attribute;
} EFI_MEMORY_DESCRIPTOR;

typedef CHAR8 ISO_639_2;
#define ISO_639_2_ENTRY_SIZE    3

#define EFI_PAGE_SIZE   4096
#define EFI_PAGE_MASK   0xFFF
#define EFI_PAGE_SHIFT  12

#define EFI_SIZE_TO_PAGES(a)  \
    ( ((a) >> EFI_PAGE_SHIFT) + ((a) & EFI_PAGE_MASK ? 1 : 0) )

#define EFI_OS_INDICATIONS_BOOT_TO_FW_UI        0x0000000000000001
#define EFI_OS_INDICATIONS_TIMESTAMP_REVOCATION 0x0000000000000002
#define EFI_OS_INDICATIONS_FILE_CAPSULE_DELIVERY_SUPPORTED \
                                                0x0000000000000004
#define EFI_OS_INDICATIONS_FMP_CAPSULE_SUPPORTED \
                                                0x0000000000000008
#define EFI_OS_INDICATIONS_CAPSULE_RESULT_VAR_SUPPORTED \
                                                0x0000000000000010
#define EFI_OS_INDICATIONS_START_OS_RECOVERY    0x0000000000000020
#define EFI_OS_INDICATIONS_START_PLATFORM_RECOVERY \
                                                0x0000000000000040
#define EFI_OS_INDICATIONS_JSON_CONFIG_DATA_REFRESH \
                                                0x0000000000000080

#endif

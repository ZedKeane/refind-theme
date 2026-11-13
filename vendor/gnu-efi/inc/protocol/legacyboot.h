#ifndef _LEGACY_BOOT_INCLUDE_
#define _LEGACY_BOOT_INCLUDE_

#define LEGACY_BOOT_PROTOCOL \
    { 0x376e5eb2, 0x30e4, 0x11d3, { 0xba, 0xe5, 0x0, 0x80, 0xc7, 0x3c, 0x88, 0x81 } }

#pragma pack(1)

typedef struct {
    UINT16  DeviceType;
    UINT16  StatusFlag;
    UINT32  Reserved;
    VOID    *BootHandler;
    CHAR8   *DescString;
} BBS_TABLE_ENTRY;
#pragma pack()

typedef
EFI_STATUS
(EFIAPI *LEGACY_BOOT_CALL) (
    IN EFI_DEVICE_PATH      *DevicePath
    );

typedef enum {
    IplRelative,
    BcvRelative
} BBS_TYPE;

INTERFACE_DECL(_LEGACY_BOOT_INTERFACE);

typedef
EFI_STATUS
(EFIAPI *GET_DEVICE_COUNT) (
    IN  struct _LEGACY_BOOT_INTERFACE   *This,
    IN  BBS_TYPE        *TableType,
    OUT UINTN           *DeviceCount,
    OUT UINTN           *MaxCount
    );

typedef
EFI_STATUS
(EFIAPI *GET_PRIORITY_AND_TABLE) (
    IN  struct _LEGACY_BOOT_INTERFACE   *This,
    IN  BBS_TYPE        *TableType,
    IN OUT  UINTN       *PrioritySize,
    OUT     UINTN       *Priority,
    IN OUT  UINTN       *TableSize,
    OUT BBS_TABLE_ENTRY *TableEntrySize
    );

typedef
EFI_STATUS
(EFIAPI *SET_PRIORITY) (
    IN  struct _LEGACY_BOOT_INTERFACE   *This,
    IN  BBS_TYPE        *TableType,
    IN OUT  UINTN       *PrioritySize,
    OUT     UINTN       *Priority
    );

typedef struct _LEGACY_BOOT_INTERFACE {
    LEGACY_BOOT_CALL    BootIt;

    UINTN                   BbsVersion;
    GET_DEVICE_COUNT        GetDeviceCount;
    GET_PRIORITY_AND_TABLE  GetPriorityAndTable;
    SET_PRIORITY            SetPriority;
} LEGACY_BOOT_INTERFACE;

EFI_STATUS
PlInitializeLegacyBoot (
    VOID
    );

#endif

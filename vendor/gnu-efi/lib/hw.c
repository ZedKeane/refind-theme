#include "lib.h"

EFI_STATUS
InitializeGlobalIoDevice (
        IN  EFI_DEVICE_PATH             *DevicePath,
        IN  EFI_GUID                    *Protocol,
        IN  CHAR8                       *ErrorStr EFI_UNUSED,
        OUT EFI_DEVICE_IO_INTERFACE     **GlobalIoFncs
        )

{
    EFI_STATUS      Status;
    EFI_HANDLE      Handle;

    Status = uefi_call_wrapper(BS->LocateDevicePath, 3, Protocol, &DevicePath, &Handle);
    if (!EFI_ERROR(Status)) {
        DEBUG ((D_INIT, "Device Already Loaded for %a device\n", ErrorStr));
        return EFI_LOAD_ERROR;
    }

    Status = uefi_call_wrapper(BS->LocateDevicePath, 3, &DeviceIoProtocol, &DevicePath, &Handle);
    if (!EFI_ERROR(Status)) {
        Status = uefi_call_wrapper(BS->HandleProtocol, 3, Handle, &DeviceIoProtocol, (VOID*)GlobalIoFncs);
    }

    ASSERT (!EFI_ERROR(Status));
    return Status;
}

UINT32
ReadPort (
        IN  EFI_DEVICE_IO_INTERFACE     *GlobalIoFncs,
        IN  EFI_IO_WIDTH                Width,
        IN  UINTN                       Port
        )
{
    UINT32       Data;
    EFI_STATUS  Status EFI_UNUSED;

    Status = uefi_call_wrapper(GlobalIoFncs->Io.Read, 5, GlobalIoFncs, Width, (UINT64)Port, 1, &Data);
    ASSERT(!EFI_ERROR(Status));
    return Data;
}

UINT32
WritePort (
        IN  EFI_DEVICE_IO_INTERFACE     *GlobalIoFncs,
        IN  EFI_IO_WIDTH                Width,
        IN  UINTN                       Port,
        IN  UINTN                       Data
        )
{
    EFI_STATUS  Status EFI_UNUSED;

    Status = uefi_call_wrapper(GlobalIoFncs->Io.Write, 5, GlobalIoFncs, Width, (UINT64)Port, 1, &Data);
    ASSERT(!EFI_ERROR(Status));
    return (UINT32)Data;
}

UINT32
ReadPciConfig (
        IN  EFI_DEVICE_IO_INTERFACE     *GlobalIoFncs,
        IN  EFI_IO_WIDTH                Width,
        IN  UINTN                       Address
        )
{
    UINT32       Data;
    EFI_STATUS  Status EFI_UNUSED;

    Status = uefi_call_wrapper(GlobalIoFncs->Pci.Read, 5, GlobalIoFncs, Width, (UINT64)Address, 1, &Data);
    ASSERT(!EFI_ERROR(Status));
    return Data;
}

UINT32
WritePciConfig (
        IN  EFI_DEVICE_IO_INTERFACE     *GlobalIoFncs,
        IN  EFI_IO_WIDTH                Width,
        IN  UINTN                       Address,
        IN  UINTN                       Data
        )
{
    EFI_STATUS  Status EFI_UNUSED;

    Status = uefi_call_wrapper(GlobalIoFncs->Pci.Write, 5, GlobalIoFncs, Width, (UINT64)Address, 1, &Data);
    ASSERT(!EFI_ERROR(Status));
    return (UINT32)Data;
}

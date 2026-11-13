#include "../../../src/runtime/ui_language.h"
#include "global.h"
#include "lib.h"
#include "screen.h"
#include "install.h"
#include "log.h"
#include "menu.h"
#include "mystrings.h"
#include "../include/refit_call_wrapper.h"
#include "../include/Handle.h"

typedef struct _esp_list {
    REFIT_VOLUME     *Volume;
    struct _esp_list *NextESP;
} ESP_LIST;

static VOID DeleteESPList(ESP_LIST *AllESPs) {
    ESP_LIST *Temp;

    LOG(3, LOG_LINE_NORMAL, L"Deleting list of ESPs");
    while (AllESPs != NULL) {
        Temp = AllESPs;
        AllESPs = AllESPs->NextESP;
        MyFreePool(Temp);
    }
}

static ESP_LIST * FindAllESPs(VOID) {
    ESP_LIST *AllESPs = NULL;
    ESP_LIST *NewESP;
    UINTN VolumeIndex;
    EFI_GUID ESPGuid = ESP_GUID_VALUE;

    LOG(2, LOG_LINE_NORMAL, L"Checking %ld volumes to find ESP(s)", VolumesCount);
    for (VolumeIndex = 0; VolumeIndex < VolumesCount; VolumeIndex++) {
        if (Volumes[VolumeIndex]->DiskKind == DISK_KIND_INTERNAL &&
            GuidsAreEqual(&(Volumes[VolumeIndex]->PartTypeGuid), &ESPGuid) &&
            (Volumes[VolumeIndex]->FSType == FS_TYPE_FAT) &&
            ! GuidsAreEqual(&(Volumes[VolumeIndex]->PartGuid), &SelfVolume->PartGuid)) {
            NewESP = AllocateZeroPool(sizeof(ESP_LIST));
            if (NewESP != NULL) {
                NewESP->Volume = Volumes[VolumeIndex];
                NewESP->NextESP = AllESPs;
                AllESPs = NewESP;
            }
        }
    }
    return AllESPs;
}

static REFIT_VOLUME *PickOneESP(ESP_LIST *AllESPs) {
    ESP_LIST            *CurrentESP;
    REFIT_VOLUME        *ChosenVolume = NULL;
    CHAR16              *Temp = NULL, *GuidStr, *PartName, *FsName, *VolName;
    INTN                DefaultEntry = 0, MenuExit = MENU_EXIT_ESCAPE, i = 1;
    MENU_STYLE_FUNC     Style = TextMenuStyle;
    REFIT_MENU_ENTRY    *ChosenOption, *MenuEntryItem = NULL;
    REFIT_MENU_SCREEN   InstallMenu = { UI_INSTALL_REFIND, NULL, 0, NULL, 0, NULL, 0, NULL,
                                        UI_SELECT_A_DESTINATION_AND_PRESS_ENTER_OR,
                                        UI_PRESS_ESC_TO_RETURN_TO_MAIN_MENU_WITHOUT_CHANGES };

    LOG(2, LOG_LINE_NORMAL, L"Prompting user to select an ESP for installation");
    if (AllowGraphicsMode)
        Style = GraphicsMenuStyle;

    if (AllESPs) {
        CurrentESP = AllESPs;
        AddMenuInfoLine(&InstallMenu, UI_SELECT_A_PARTITION_AND_PRESS_ENTER_TO_INSTALL_REFIND);
        while (CurrentESP != NULL) {
            MenuEntryItem = AllocateZeroPool(sizeof(REFIT_MENU_ENTRY));
            GuidStr = GuidAsString(&(CurrentESP->Volume->PartGuid));
            PartName = CurrentESP->Volume->PartName;
            FsName = CurrentESP->Volume->FsName;
            VolName = CurrentESP->Volume->VolName;
            if (PartName && (StrLen(PartName) > 0) && FsName && (StrLen(FsName) > 0) &&
                !MyStriCmp(FsName, PartName)) {
                Temp = PoolPrint(L"%s - '%s', aka '%s'", GuidStr, PartName, FsName);
            } else if (FsName && (StrLen(FsName) > 0)) {
                Temp = PoolPrint(L"%s - '%s'", GuidStr, FsName);
            } else if (PartName && (StrLen(PartName) > 0)) {
                Temp = PoolPrint(L"%s - '%s'", GuidStr, PartName);
            } else if (VolName && (StrLen(VolName) > 0)) {
                Temp = PoolPrint(L"%s - '%s'", GuidStr, VolName);
            } else {
                Temp = PoolPrint(L"%s - no name", GuidStr);
            }
            LOG(3, LOG_LINE_NORMAL, L"Adding '%s' to UI list of ESPs", Temp);
            MyFreePool(GuidStr);
            MenuEntryItem->Title = Temp;
            MenuEntryItem->Tag = TAG_RETURN;
            MenuEntryItem->Row = i++;
            AddMenuEntry(&InstallMenu, MenuEntryItem);
            CurrentESP = CurrentESP->NextESP;
        }
        MenuExit = RunGenericMenu(&InstallMenu, Style, &DefaultEntry, &ChosenOption);
        if (MenuExit == MENU_EXIT_ENTER) {
            CurrentESP = AllESPs;
            while (CurrentESP != NULL) {
                Temp = GuidAsString(&(CurrentESP->Volume->PartGuid));
                if (MyStrStr(ChosenOption->Title, Temp)) {
                    ChosenVolume = CurrentESP->Volume;
                }
                CurrentESP = CurrentESP->NextESP;
                MyFreePool(Temp);
                Temp = NULL;
            }
        }
    } else {
        DisplaySimpleMessage(UI_INFORMATION, UI_NO_ELIGIBLE_ESPS_FOUND);
        LOG(2, LOG_LINE_NORMAL, L"No ESPs found");
    }
    MyFreePool(Temp);
    return ChosenVolume;
}

static EFI_STATUS RenameFile(IN EFI_FILE_PROTOCOL *BaseDir, CHAR16 *OldName, CHAR16 *NewName) {
    EFI_STATUS        Status;
    EFI_FILE_PROTOCOL *FilePtr = NULL;
    EFI_FILE_INFO     *NewInfo, *Buffer = NULL;
    UINTN             NewInfoSize;

    LOG(3, LOG_LINE_NORMAL, L"Trying to rename '%s' to '%s'", OldName, NewName);
    Status = refit_call5_wrapper(BaseDir->Open, BaseDir, &FilePtr, OldName,
                                 EFI_FILE_MODE_READ | EFI_FILE_MODE_WRITE, 0);
    if (Status == EFI_SUCCESS) {
        Buffer = LibFileInfo(FilePtr);
        if (Buffer == NULL) {
            refit_call1_wrapper(FilePtr->Close, FilePtr);
            return FALSE;
        }
    }
    if (Status == EFI_SUCCESS) {
        NewInfoSize = sizeof(EFI_FILE_INFO) + StrSize(NewName);
        NewInfo = (EFI_FILE_INFO *) AllocateZeroPool(NewInfoSize);
        if (NewInfo != NULL) {
            MyCopyMem(NewInfo, Buffer, sizeof(EFI_FILE_INFO));
            NewInfo->FileName[0] = 0;
            StrCat(NewInfo->FileName, NewName);

            Status = refit_call4_wrapper(BaseDir->SetInfo,
                                         FilePtr,
                                         &gEfiFileInfoGuid,
                                         NewInfoSize,
                                         (VOID *) NewInfo);
            refit_call1_wrapper(FilePtr->Close, FilePtr);
        } else {
            Status = EFI_BUFFER_TOO_SMALL;
        }
        MyFreePool(NewInfo);
    }
    MyFreePool(Buffer);
    return Status;
}

EFI_STATUS BackupOldFile(IN EFI_FILE_PROTOCOL *BaseDir, CHAR16 *FileName) {
    EFI_STATUS          Status = EFI_SUCCESS;
    CHAR16              *NewName;

    LOG(3, LOG_LINE_NORMAL, L"Backing up '%s'", FileName);
    if ((BaseDir == NULL) || (FileName == NULL))
       return EFI_INVALID_PARAMETER;

    NewName = PoolPrint(L"%s-old", FileName);
    if (!FileExists(BaseDir, NewName)) {
        Status = RenameFile(BaseDir, FileName, NewName);
    }
    MyFreePool(NewName);

    return (Status);
}

static EFI_STATUS CreateDirectories(IN EFI_FILE_PROTOCOL *BaseDir) {
    CHAR16            *FileName = NULL;
    UINTN             i = 0, Status = EFI_SUCCESS;
    EFI_FILE_PROTOCOL *TheDir = NULL;

    while ((FileName = FindCommaDelimited(INST_DIRECTORIES, i++)) != NULL && Status == EFI_SUCCESS) {
        Status = refit_call5_wrapper(BaseDir->Open, BaseDir, &TheDir, FileName,
                                 EFI_FILE_MODE_READ | EFI_FILE_MODE_WRITE | EFI_FILE_MODE_CREATE, EFI_FILE_DIRECTORY);
        Status = refit_call1_wrapper(TheDir->Close, TheDir);
        MyFreePool(FileName);
    }
    return (Status);
}

static EFI_STATUS CopyOneFile(IN EFI_FILE_PROTOCOL *SourceDir,
                              IN CHAR16 *SourceName,
                              IN EFI_FILE_PROTOCOL *DestDir,
                              IN CHAR16 *DestName) {
    EFI_FILE_PROTOCOL  *SourceFile = NULL, *DestFile = NULL;
    UINTN              FileSize = 0, Status;
    EFI_FILE_INFO      *FileInfo = NULL;
    UINTN              *Buffer = NULL;

    Status = refit_call5_wrapper(SourceDir->Open, SourceDir, &SourceFile, SourceName,
                                 EFI_FILE_MODE_READ, 0);
    if (Status == EFI_SUCCESS) {
        FileInfo = LibFileInfo(SourceFile);
        if (FileInfo == NULL) {
            refit_call1_wrapper(SourceFile->Close, SourceFile);
            return EFI_NO_RESPONSE;
        }
        FileSize = FileInfo->FileSize;
        MyFreePool(FileInfo);
    }
    if (Status == EFI_SUCCESS) {
        Buffer = AllocateZeroPool(FileSize);
        if (Buffer == NULL)
            Status = EFI_OUT_OF_RESOURCES;
    }
    if (Status == EFI_SUCCESS)
        Status = refit_call3_wrapper(SourceFile->Read, SourceFile, &FileSize, Buffer);
    if (Status == EFI_SUCCESS)
        refit_call1_wrapper(SourceFile->Close, SourceFile);

    if (Status == EFI_SUCCESS) {
        Status = refit_call5_wrapper(DestDir->Open, DestDir, &DestFile, DestName,
                                     EFI_FILE_MODE_READ | EFI_FILE_MODE_WRITE | EFI_FILE_MODE_CREATE, 0);
    }
    if (Status == EFI_SUCCESS)
        Status = refit_call3_wrapper(DestFile->Write, DestFile, &FileSize, Buffer);
    if (Status == EFI_SUCCESS)
        Status = refit_call1_wrapper(DestFile->Close, DestFile);

    MyFreePool(Buffer);

    if (EFI_ERROR(Status)) {
        LOG(1, LOG_LINE_NORMAL, L"Error %d when copying '%s' to '%s'", Status, SourceName, DestName);
    }
    return (Status);
}

static EFI_STATUS CopyDirectory(IN EFI_FILE_PROTOCOL *SourceDirPtr,
                                IN CHAR16 *SourceDirName,
                                IN EFI_FILE_PROTOCOL *DestDirPtr,
                                IN CHAR16 *DestDirName) {
    REFIT_DIR_ITER  DirIter;
    EFI_FILE_INFO   *DirEntry;
    CHAR16          *DestFileName = NULL, *SourceFileName = NULL;
    EFI_STATUS      Status = EFI_SUCCESS;

    DirIterOpen(SourceDirPtr, SourceDirName, &DirIter);
    while (DirIterNext(&DirIter, 2, NULL, &DirEntry) && (Status == EFI_SUCCESS)) {
        SourceFileName = PoolPrint(L"%s\\%s", SourceDirName, DirEntry->FileName);
        DestFileName = PoolPrint(L"%s\\%s", DestDirName, DirEntry->FileName);
        Status = CopyOneFile(SourceDirPtr, SourceFileName, DestDirPtr, DestFileName);
        MyFreePool(DestFileName);
        MyFreePool(SourceFileName);
        MyFreePool(DirEntry);
    }
    return (Status);
}

static EFI_STATUS CopyDrivers(IN EFI_FILE_PROTOCOL *SourceDirPtr,
                              IN CHAR16 *SourceDirName,
                              IN EFI_FILE_PROTOCOL *DestDirPtr,
                              IN CHAR16 *DestDirName) {
    CHAR16          *DestFileName = NULL, *SourceFileName = NULL;
    CHAR16          *DriverName = NULL;
    EFI_STATUS      Status = EFI_SUCCESS;
    EFI_STATUS      WorstStatus = EFI_SUCCESS;
    BOOLEAN         DriverCopied[NUM_FS_TYPES];
    UINTN           i;

    for (i = 0; i < NUM_FS_TYPES; i++)
        DriverCopied[i] = FALSE;

    LOG(1, LOG_LINE_NORMAL, L"Scanning %d volumes for identifiable filesystems", VolumesCount);
    for (i = 0; i < VolumesCount; i++) {
        LOG(3, LOG_LINE_NORMAL, L"Looking for driver for volume # %d, '%s'", i, Volumes[i]->VolName);
        DriverName = NULL;
        switch (Volumes[i]->FSType) {

            case FS_TYPE_BTRFS:
                if (DriverCopied[FS_TYPE_BTRFS] == FALSE) {
                    DriverName = L"btrfs";
                    DriverCopied[FS_TYPE_BTRFS] = TRUE;
                }
                break;

            case FS_TYPE_EXT2:
                if (DriverCopied[FS_TYPE_EXT2] == FALSE) {
                    DriverName = L"ext2";
                    DriverCopied[FS_TYPE_EXT2] = TRUE;
                    DriverCopied[FS_TYPE_EXT3] = TRUE;
                }
                break;

            case FS_TYPE_EXT3:
                if (DriverCopied[FS_TYPE_EXT3] == FALSE) {
                    DriverName = L"ext2";
                    DriverCopied[FS_TYPE_EXT2] = TRUE;
                    DriverCopied[FS_TYPE_EXT3] = TRUE;
                }
                break;

            case FS_TYPE_EXT4:
                if (DriverCopied[FS_TYPE_EXT4] == FALSE) {
                    DriverName = L"ext4";
                    DriverCopied[FS_TYPE_EXT4] = TRUE;
                }
                break;

            case FS_TYPE_REISERFS:
                if (DriverCopied[FS_TYPE_REISERFS] == FALSE) {
                    DriverName = L"reiserfs";
                    DriverCopied[FS_TYPE_REISERFS] = TRUE;
                }
                break;

            case FS_TYPE_HFSPLUS:
                if ((DriverCopied[FS_TYPE_HFSPLUS] == FALSE) && (!MyStriCmp(L"Apple", ST->FirmwareVendor))) {
                    DriverName = L"hfs";
                    DriverCopied[FS_TYPE_HFSPLUS] = TRUE;
                }
                break;

        }
        if (DriverName) {
            SourceFileName = PoolPrint(L"%s\\%s%s", SourceDirName, DriverName, INST_PLATFORM_EXTENSION);
            DestFileName = PoolPrint(L"%s\\%s%s", DestDirName, DriverName, INST_PLATFORM_EXTENSION);
            LOG(1, LOG_LINE_NORMAL, L"Trying to copy driver for %s", DriverName);
            Status = CopyOneFile(SourceDirPtr, SourceFileName, DestDirPtr, DestFileName);
            if (EFI_ERROR(Status))
                WorstStatus = Status;
            MyFreePool(SourceFileName);
            MyFreePool(DestFileName);
        }
    }

    return (WorstStatus);
}

static EFI_STATUS CopyFiles(IN EFI_FILE_PROTOCOL *TargetDir) {
    REFIT_VOLUME    *SourceVolume = NULL;
    CHAR16          *SourceFile = NULL, *SourceDir, *ConfFile;
    CHAR16          *SourceDriversDir, *TargetDriversDir, *RefindName;
    EFI_STATUS      Status;
    EFI_STATUS      WorstStatus = EFI_SUCCESS;

    FindVolumeAndFilename(GlobalConfig.SelfDevicePath, &SourceVolume, &SourceFile);
    SourceDir = FindPath(SourceFile);

    RefindName = PoolPrint(L"EFI\\refind\\%s", INST_REFIND_NAME);
    Status = CopyOneFile(SourceVolume->RootDir, SourceFile, TargetDir, RefindName);
    MyFreePool(SourceFile);
    MyFreePool(RefindName);
    if (EFI_ERROR(Status)) {
        LOG(1, LOG_LINE_NORMAL, L"Error copying rEFInd binary; installation has failed");
        Status = WorstStatus = EFI_ABORTED;
    }

    if (Status == EFI_SUCCESS) {

        ConfFile = PoolPrint(L"%s\\refind.conf-sample", SourceDir);
        if (FileExists(SourceVolume->RootDir, ConfFile)) {

            SourceFile = PoolPrint(L"%s", ConfFile);
        } else {
            SourceFile = PoolPrint(L"%s\\refind.conf", SourceDir);
        }
        MyFreePool(ConfFile);

        if (FileExists(TargetDir, L"\\EFI\\refind\\refind.conf")) {
            Status = CopyOneFile(SourceVolume->RootDir, SourceFile, TargetDir, L"EFI\\refind\\refind.conf-sample");
        } else {
            Status = CopyOneFile(SourceVolume->RootDir, SourceFile, TargetDir, L"EFI\\refind\\refind.conf");
        }
        if (EFI_ERROR(Status))
            WorstStatus = Status;
        MyFreePool(SourceFile);

        SourceFile = PoolPrint(L"%s\\icons", SourceDir);
        Status = CopyDirectory(SourceVolume->RootDir, SourceFile, TargetDir, L"EFI\\refind\\icons");
        if (EFI_ERROR(Status)) {
            LOG(1, LOG_LINE_NORMAL, L"Error %d copying icons directory", Status);
            WorstStatus = Status;
        }
        MyFreePool(SourceFile);

        SourceDriversDir = PoolPrint(L"%s\\%s", SourceDir, INST_DRIVERS_SUBDIR);
        TargetDriversDir = PoolPrint(L"EFI\\refind\\%s", INST_DRIVERS_SUBDIR);
        Status = CopyDrivers(SourceVolume->RootDir, SourceDriversDir, TargetDir, TargetDriversDir);
        if (EFI_ERROR(Status)) {
            LOG(1, LOG_LINE_NORMAL, L"Error %d copying drivers", Status);
            WorstStatus = Status;
        }
        MyFreePool(SourceDriversDir);
        MyFreePool(TargetDriversDir);
    }
    MyFreePool(SourceDir);
    return (WorstStatus);
}

static VOID CreateFallbackCSV(IN EFI_FILE_PROTOCOL *TargetDir) {
    CHAR16            *Contents = NULL;
    UINTN             FileSize, Status;
    EFI_FILE_PROTOCOL *FilePtr;

    Status = refit_call5_wrapper(TargetDir->Open, TargetDir, &FilePtr, L"\\EFI\\refind\\BOOT.CSV",
                                 EFI_FILE_MODE_READ | EFI_FILE_MODE_WRITE | EFI_FILE_MODE_CREATE, 0);
    if (Status == EFI_SUCCESS) {
        Contents = PoolPrint(L"%s,rEFInd Boot Manager,,This is the boot entry for rEFInd\n",
                             INST_REFIND_NAME);
        if (Contents) {
            FileSize = StrSize(Contents);
            Status = refit_call3_wrapper(FilePtr->Write, FilePtr, &FileSize, Contents);
            if (Status == EFI_SUCCESS)
                refit_call1_wrapper(FilePtr->Close, FilePtr);
        }
        MyFreePool(Contents);
    }
    if (EFI_ERROR(Status)) {
        LOG(1, LOG_LINE_NORMAL, L"Error %d when writing BOOT.CSV file", Status);
    }
}

static BOOLEAN CopyRefindFiles(IN EFI_FILE_PROTOCOL *TargetDir) {
    EFI_STATUS Status = EFI_SUCCESS, Status2;

    if (FileExists(TargetDir, L"\\EFI\\refind\\icons")) {
        Status = BackupOldFile(TargetDir, L"\\EFI\\refind\\icons");
        if (EFI_ERROR(Status)) {
            LOG(1, LOG_LINE_NORMAL, L"Error when backing up icons");
        }
    }
    if (Status == EFI_SUCCESS) {
        Status = CreateDirectories(TargetDir);
        if (EFI_ERROR(Status)) {
            LOG(1, LOG_LINE_NORMAL, L"Error when creating target directory");
        }
    }
    if (Status == EFI_SUCCESS) {

        Status2 = CopyFiles(TargetDir);
        if (EFI_ERROR(Status2)) {
            if (Status2 == EFI_ABORTED) {
                Status = EFI_ABORTED;
            } else {
                DisplaySimpleMessage(UI_WARNING, UI_ERROR_COPYING_SOME_FILES);
            }
        }
    }
    CreateFallbackCSV(TargetDir);

    return Status;
}

static UINTN FindBootNum(EFI_DEVICE_PATH *Entry, UINTN Size, BOOLEAN *AlreadyExists) {
    UINTN   i = 0, VarSize, Status;
    CHAR16  *VarName, *Contents = NULL;

    *AlreadyExists = FALSE;
    do {
        VarName = PoolPrint(L"Boot%04x", i++);
        Status = EfivarGetRaw(&GlobalGuid, VarName, (CHAR8**) &Contents, &VarSize);
        if ((Status == EFI_SUCCESS) && (VarSize == Size) && (CompareMem(Contents, Entry, VarSize) == 0)) {
            *AlreadyExists = TRUE;
        }
        MyFreePool(VarName);
    } while ((Status == EFI_SUCCESS) && (*AlreadyExists == FALSE));
    if (i > 0x10000)
        i = 0x10000;
    return (i - 1);
}

static EFI_STATUS ConstructBootEntry(EFI_HANDLE *TargetVolume,
                                     CHAR16 *Loader,
                                     CHAR16 *Label,
                                     CHAR8 **Entry,
                                     UINTN *Size) {
    EFI_DEVICE_PATH *DevicePath;
    UINTN           Status = EFI_SUCCESS, DevPathSize;
    CHAR8           *Working;

    DevicePath = FileDevicePath(TargetVolume, Loader);
    DevPathSize = DevicePathSize(DevicePath);
    *Size = sizeof(UINT32) + sizeof (UINT16) + StrSize(Label) + DevPathSize + 2;
    *Entry = Working = AllocateZeroPool(*Size);
    if (DevicePath && *Entry) {
        *(UINT32 *)Working = LOAD_OPTION_ACTIVE;
        Working += sizeof (UINT32);
        *(UINT16 *)Working = DevPathSize;
        Working += sizeof (UINT16);
        StrCpy((CHAR16 *)Working, Label);
        Working += StrSize(Label);
        MyCopyMem(Working, DevicePath, DevPathSize);

    } else {
        Status = EFI_OUT_OF_RESOURCES;
    }
    MyFreePool(DevicePath);
    return Status;
}

static EFI_STATUS SetBootDefault(UINTN BootNum) {
    UINTN    Status, VarSize, ListSize, i, j;
    UINT16   *BootOrder, *NewBootOrder;
    BOOLEAN  IsAlreadyFirst = FALSE;

    Status = EfivarGetRaw(&GlobalGuid, L"BootOrder", (CHAR8**) &BootOrder, &VarSize);
    if (Status == EFI_SUCCESS) {
        ListSize = VarSize / sizeof(UINT16);
        for (i = 0; i < ListSize; i++) {
            if (BootOrder[i] == BootNum) {
                if (i == 0)
                    IsAlreadyFirst = TRUE;
            }
        }
        if (!IsAlreadyFirst) {
            NewBootOrder = AllocateZeroPool((ListSize + 1) * sizeof(UINT16));
            NewBootOrder[0] = BootNum;
            j = 1;
            for (i = 0; i < ListSize; i++) {
                if (BootOrder[i] != BootNum) {
                    NewBootOrder[j++] = BootOrder[i];
                }
            }
            Status = EfivarSetRaw(&GlobalGuid, L"BootOrder", (CHAR8*) NewBootOrder,
                                  j * sizeof(UINT16), TRUE);
            MyFreePool(NewBootOrder);
        }
        MyFreePool(BootOrder);
    }

    return Status;
}

static EFI_STATUS CreateNvramEntry(EFI_HANDLE DeviceHandle) {
    CHAR16           *VarName = NULL, *ProgName;
    UINTN            Status, Size, BootNum = 0;
    EFI_DEVICE_PATH  *Entry;
    BOOLEAN          AlreadyExists = FALSE;

    ProgName = PoolPrint(L"\\EFI\\refind\\%s", INST_REFIND_NAME);
    Status = ConstructBootEntry(DeviceHandle, ProgName,
                                L"rEFInd Boot Manager", (CHAR8**) &Entry, &Size);
    MyFreePool(ProgName);
    if (Status == EFI_SUCCESS)
        BootNum = FindBootNum(Entry, Size, &AlreadyExists);

    if ((Status == EFI_SUCCESS) && (AlreadyExists == FALSE)) {
        VarName = PoolPrint(L"Boot%04x", BootNum);
        Status = EfivarSetRaw(&GlobalGuid, VarName, (CHAR8*) Entry, Size, TRUE);
        MyFreePool(VarName);
    }
    MyFreePool(Entry);

    if (Status == EFI_SUCCESS) {
        Status = SetBootDefault(BootNum);
    }
    return (Status);
}

VOID InstallRefind(VOID) {
    ESP_LIST      *AllESPs;
    REFIT_VOLUME  *SelectedESP;
    UINTN         Status;

    LOG(1, LOG_LINE_THIN_SEP, L"Installing rEFInd to an ESP");
    AllESPs = FindAllESPs();
    SelectedESP = PickOneESP(AllESPs);
    if (SelectedESP) {
        Status = CopyRefindFiles(SelectedESP->RootDir);
        if (Status == EFI_SUCCESS)
            Status = CreateNvramEntry(SelectedESP->DeviceHandle);
        DeleteESPList(AllESPs);
        if (Status == EFI_SUCCESS) {
            DisplaySimpleMessage(UI_INFORMATION, UI_REFIND_SUCCESSFULLY_INSTALLED);
        } else {
            DisplaySimpleMessage(UI_WARNING, UI_PROBLEMS_ENCOUNTERED_DURING_INSTALLATION);
        }
    }
    LOG(1, LOG_LINE_THIN_SEP, L"Done installing rEFInd to an ESP");
}

BOOT_ENTRY_LIST * FindBootOrderEntries(VOID) {
    UINTN            Status = EFI_SUCCESS, i;
    UINT16           *BootOrder = NULL;
    UINTN            VarSize, ListSize;
    CHAR16           *VarName = NULL;
    CHAR16           *Contents = NULL;
    BOOT_ENTRY_LIST  *L, *ListStart = NULL, *ListEnd = NULL;

    LOG(1, LOG_LINE_NORMAL, L"Finding boot order entries");
    Status = EfivarGetRaw(&GlobalGuid, L"BootOrder", (CHAR8**) &BootOrder, &VarSize);
    if (Status != EFI_SUCCESS)
        return NULL;

    ListSize = VarSize / sizeof(UINT16);
    for (i = 0; i < ListSize; i++) {
        VarName = PoolPrint(L"Boot%04x", BootOrder[i]);
        Status = EfivarGetRaw(&GlobalGuid, VarName, (CHAR8**) &Contents, &VarSize);
        if (Status == EFI_SUCCESS) {
            L = AllocateZeroPool(sizeof(BOOT_ENTRY_LIST));
            if (L) {
                L->BootEntry.BootNum = BootOrder[i];
                L->BootEntry.Options = (UINT32) Contents[0];
                L->BootEntry.Size = (UINT16) Contents[2];
                L->BootEntry.Label = StrDuplicate((CHAR16*) &(Contents[3]));
                L->BootEntry.DevPath = AllocatePool(L->BootEntry.Size);
                MyCopyMem(L->BootEntry.DevPath,
                        (EFI_DEVICE_PATH*) &Contents[3 + StrSize(L->BootEntry.Label)/2],
                        L->BootEntry.Size);
                L->NextBootEntry = NULL;
                if (ListStart == NULL) {
                    ListStart = L;
                } else {
                    ListEnd->NextBootEntry = L;
                }
                ListEnd = L;
            } else {
                Status = EFI_OUT_OF_RESOURCES;
            }
        }
        MyFreePool(VarName);
        MyFreePool(Contents);
    }
    MyFreePool(BootOrder);

    return ListStart;
}

VOID DeleteBootOrderEntries(BOOT_ENTRY_LIST *Entries) {
    BOOT_ENTRY_LIST *Current;

    while (Entries != NULL) {
        Current = Entries;
        MyFreePool(Current->BootEntry.Label);
        MyFreePool(Current->BootEntry.DevPath);
        Entries = Entries->NextBootEntry;
        MyFreePool(Current);
    }
}

static UINTN PickOneBootOption(IN BOOT_ENTRY_LIST *Entries, IN OUT UINTN *BootOrderNum) {
    CHAR16              *Temp = NULL, *Filename = NULL;
    REFIT_VOLUME        *Volume = NULL;
    INTN                DefaultEntry = 0, MenuExit = MENU_EXIT_ESCAPE;
    UINTN               Operation = EFI_BOOT_OPTION_DO_NOTHING;
    MENU_STYLE_FUNC     Style = TextMenuStyle;
    REFIT_MENU_ENTRY    *ChosenOption, *MenuEntryItem = NULL;
    REFIT_MENU_SCREEN   Menu = { UI_MANAGE_EFI_BOOT_ORDER, NULL, 0, NULL, 0, NULL, 0, NULL,
                                 UI_SELECT_AN_OPTION_AND_PRESS_ENTER_TO_MAKE_IT_THE_DEFAULT_PRESS_OR,
                                 UI_DELETE_TO_DELETE_IT_OR_ESC_TO_RETURN_TO_MAIN_MENU_WITHOUT_CHANGES };
    if (AllowGraphicsMode)
        Style = GraphicsMenuStyle;

    if (Entries) {
        AddMenuInfoLine(&Menu, UI_SELECT_AN_OPTION_AND_PRESS_ENTER_TO_MAKE_IT_THE_DEFAULT_OR_TO_DELETE_I);
        while (Entries != NULL) {
            MenuEntryItem = AllocateZeroPool(sizeof(REFIT_MENU_ENTRY));
            FindVolumeAndFilename(Entries->BootEntry.DevPath, &Volume, &Filename);
            if ((Filename != NULL) && (StrLen(Filename) > 0)) {
                if ((Volume != NULL) && (Volume->VolName != NULL)) {
                    Temp = PoolPrint(L"Boot%04x - %s - %s on %s",
                                    Entries->BootEntry.BootNum,
                                    Entries->BootEntry.Label,
                                    Filename, Volume->VolName);
                } else {
                    Temp = PoolPrint(L"Boot%04x - %s - %s",
                                    Entries->BootEntry.BootNum,
                                    Entries->BootEntry.Label,
                                    Filename);
                }
            } else {
                Temp = PoolPrint(L"Boot%04x - %s",
                                 Entries->BootEntry.BootNum,
                                 Entries->BootEntry.Label);
            }

            MyFreePool(Filename);
            Filename = NULL;
            Volume = NULL;
            MenuEntryItem->Title = StrDuplicate(Temp);
            MenuEntryItem->Row = Entries->BootEntry.BootNum;
            AddMenuEntry(&Menu, MenuEntryItem);
            Entries = Entries->NextBootEntry;
            MyFreePool(Temp);
        }
        MenuExit = RunGenericMenu(&Menu, Style, &DefaultEntry, &ChosenOption);
        if (MenuExit == MENU_EXIT_ENTER) {
            Operation = EFI_BOOT_OPTION_MAKE_DEFAULT;
            *BootOrderNum = ChosenOption->Row;
        }
        if (MenuExit == MENU_EXIT_HIDE) {
            Operation = EFI_BOOT_OPTION_DELETE;
            *BootOrderNum = ChosenOption->Row;
        }
        MyFreePool(MenuEntryItem);
    } else {
        DisplaySimpleMessage(UI_INFORMATION, UI_EFI_BOOT_ORDER_LIST_IS_UNAVAILABLE);
    }
    return Operation;
}

static EFI_STATUS DeleteInvalidBootEntries(VOID) {
    UINTN    Status, VarSize, ListSize, i, j = 0;
    UINT16   *BootOrder, *NewBootOrder;
    CHAR8    *Contents;
    CHAR16   *VarName;

    LOG(1, LOG_LINE_NORMAL, L"Deleting invalid boot entries from internal BootOrder list");
    Status = EfivarGetRaw(&GlobalGuid, L"BootOrder", (CHAR8**) &BootOrder, &VarSize);
    if (Status == EFI_SUCCESS) {
        ListSize = VarSize / sizeof(UINT16);
        NewBootOrder = AllocateZeroPool(VarSize);
        for (i = 0; i < ListSize; i++) {
            VarName = PoolPrint(L"Boot%04x", BootOrder[i]);
            Status = EfivarGetRaw(&GlobalGuid, VarName, &Contents, &VarSize);
            MyFreePool(VarName);
            if (Status == EFI_SUCCESS) {
                NewBootOrder[j++] = BootOrder[i];
                MyFreePool(Contents);
            }
        }
        Status = EfivarSetRaw(&GlobalGuid, L"BootOrder", (CHAR8*) NewBootOrder,
                              j * sizeof(UINT16), TRUE);
        MyFreePool(NewBootOrder);
        MyFreePool(BootOrder);
    }

    return Status;
}

VOID ManageBootorder(VOID) {
    BOOT_ENTRY_LIST *Entries;
    UINTN           BootNum = 0, Operation;
    CHAR16          *Name, *Message;

    LOG(1, LOG_LINE_NORMAL, L"Managing boot order list");
    Entries = FindBootOrderEntries();
    Operation = PickOneBootOption(Entries, &BootNum);
    if (Operation == EFI_BOOT_OPTION_DELETE) {
        Name = PoolPrint(L"Boot%04x", BootNum);
        EfivarSetRaw(&GlobalGuid, Name, NULL, 0, TRUE);
        DeleteInvalidBootEntries();
        Message = PoolPrint(UI_BOOT_04X_HAS_BEEN_DELETED, BootNum);
        DisplaySimpleMessage(UI_INFORMATION, Message);
        MyFreePool(Name);
        MyFreePool(Message);
    }
    if (Operation == EFI_BOOT_OPTION_MAKE_DEFAULT) {
        SetBootDefault(BootNum);
        Message = PoolPrint(UI_BOOT_04X_IS_NOW_THE_DEFAULT_EFI_BOOT_OPTION, BootNum);
        DisplaySimpleMessage(UI_INFORMATION, Message);
        MyFreePool(Message);
    }
    DeleteBootOrderEntries(Entries);
}

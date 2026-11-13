#include "../../../src/runtime/ui_language.h"
#include "../../../src/runtime/os_registry.h"

#include "global.h"
#include "lib.h"
#include "icns.h"
#include "log.h"
#include "menu.h"
#include "config.h"
#include "screen.h"
#include "apple.h"
#include "mystrings.h"
#include "scan.h"
#include "../include/refit_call_wrapper.h"
#include "../mok/mok.h"

typedef struct { REFIT_VOLUME *Volume; UINT8 Uuid[16]; CHAR16 *Label; } AUTO_FS;
static AUTO_FS AutoFs[256];
static UINTN AutoFsCount;
static VOID ClearDiskNames(VOID);
static VOID ClearSystemLocations(VOID) {
    ClearDiskNames();
    UINTN n;
    for (n=0;n<AutoFsCount;n++) MyFreePool(AutoFs[n].Label);
    AutoFsCount=0;
}
static UINTN PhysicalPartNumber(REFIT_VOLUME *v) {
    EFI_DEVICE_PATH *p=v ? v->DevicePath : NULL;
    while (p && !IsDevicePathEndType(p)) {
        if (DevicePathType(p)==MEDIA_DEVICE_PATH && DevicePathSubType(p)==MEDIA_HARDDRIVE_DP)
            return ((HARDDRIVE_DEVICE_PATH *)p)->PartitionNumber;
        p=NextDevicePathNode(p);
    }
    return 0;
}
static EFI_BLOCK_IO *PhysicalDisk(REFIT_VOLUME *v) {
    if (!v) return NULL;
    if (v->WholeDiskBlockIO) return v->WholeDiskBlockIO;
    if (v->BlockIO && !v->BlockIO->Media->LogicalPartition) return v->BlockIO;
    return NULL;
}
static BOOLEAN SamePhysicalDisk(REFIT_VOLUME *a, REFIT_VOLUME *b) {
    EFI_BLOCK_IO *disk=PhysicalDisk(a);
    return disk && disk==PhysicalDisk(b);
}
static UINT8 *ReadVolumeStart(REFIT_VOLUME *v, UINTN bytes) {
    UINT8 *buf;
    UINTN size,block;
    EFI_STATUS status;
    if (!v || !v->BlockIO || !v->BlockIO->Media->MediaPresent) return NULL;
    block=v->BlockIO->Media->BlockSize;
    if (!block || block>65536 || bytes>131072) return NULL;
    size=((bytes+block-1)/block)*block;
    buf=AllocatePool(size);
    if (!buf) return NULL;
    status=refit_call5_wrapper(v->BlockIO->ReadBlocks,v->BlockIO,v->BlockIO->Media->MediaId,v->BlockIOOffset,size,buf);
    if (EFI_ERROR(status)) { MyFreePool(buf); return NULL; }
    return buf;
}
static CHAR16 *AsciiLabel(UINT8 *p, UINTN len) {
    CHAR16 *s=AllocateZeroPool((len+1)*sizeof(CHAR16));
    UINTN n=0,out=0,k,need;
    UINT32 cp;
    if(!s) return NULL;
    while(n<len && p[n]) {
        UINT8 c=p[n++];need=0;cp=c;
        if(c>=0xC2 && c<=0xDF) { cp=c&31;need=1; }
        else if(c>=0xE0 && c<=0xEF) { cp=c&15;need=2; }
        else if(c>=128) cp='?';
        for(k=0;k<need && n<len;k++) { if((p[n]&0xC0)!=0x80) break;cp=(cp<<6)|(p[n++]&63); }
        if(k<need || cp<32 || (cp>=0xD800 && cp<=0xDFFF)) cp='?';
        s[out++]=(CHAR16)cp;
    }
    while(out && s[out-1]==L' ') s[--out]=0;
    return s;
}
static AUTO_FS *FilesystemMeta(REFIT_VOLUME *v) {
    UINTN n;
    UINT8 *p;
    AUTO_FS *m;
    for(n=0;n<AutoFsCount;n++) if(AutoFs[n].Volume==v) return &AutoFs[n];
    if(AutoFsCount>=256) return NULL;
    m=&AutoFs[AutoFsCount++]; SetMem(m,sizeof(*m),0); m->Volume=v;
    p=ReadVolumeStart(v,4096);
    if(p) {
        if(CompareMem(p,"XFSB",4)==0) { MyCopyMem(m->Uuid,p+32,16); m->Label=AsciiLabel(p+108,12); }
        else if(p[1080]==0x53 && p[1081]==0xEF) { MyCopyMem(m->Uuid,p+1128,16); m->Label=AsciiLabel(p+1144,16); }
        MyFreePool(p);
    }
    if(v->FsName && v->FsName[0]) { MyFreePool(m->Label); m->Label=StrDuplicate(v->FsName); }
    return m;
}
static UINT8 *ReadDisplayFile(REFIT_VOLUME *v, CHAR16 *path, UINTN *size) {
    EFI_FILE_HANDLE handle;
    EFI_FILE_INFO *info;
    UINT8 *data=NULL;
    EFI_STATUS status;
    if(!v || !v->RootDir || !FileExists(v->RootDir,path)) return NULL;
    status=refit_call5_wrapper(v->RootDir->Open,v->RootDir,&handle,path,EFI_FILE_MODE_READ,0);
    if(EFI_ERROR(status)) return NULL;
    info=LibFileInfo(handle);
    if(info && info->FileSize>0 && info->FileSize<=4*1024*1024 && !(info->Attribute&EFI_FILE_DIRECTORY)) {
        *size=(UINTN)info->FileSize;
        data=AllocatePool(*size);
        if(data) {
            status=refit_call3_wrapper(handle->Read,handle,size,data);
            if(EFI_ERROR(status)) { MyFreePool(data);data=NULL; }
        }
    }
    MyFreePool(info);refit_call1_wrapper(handle->Close,handle);return data;
}
static BOOLEAN BufferContains(UINT8 *data, UINTN len, UINT8 *needle, UINTN n) {
    UINTN i;
    if(!data || !needle || !n || len<n) return FALSE;
    for(i=0;i<=len-n;i++) if(CompareMem(data+i,needle,n)==0) return TRUE;
    return FALSE;
}
static BOOLEAN IsApfs(REFIT_VOLUME *v) {
    EFI_GUID type={0x7c3457ef,0x0000,0x11aa,{0xaa,0x11,0x00,0x30,0x65,0x43,0xec,0xac}};
    return GuidsAreEqual(&v->PartTypeGuid,&type);
}
static BOOLEAN LinuxFilesystem(REFIT_VOLUME *v) {
    return v->FSType==FS_TYPE_XFS || v->FSType==FS_TYPE_BTRFS ||
           v->FSType==FS_TYPE_EXT2 || v->FSType==FS_TYPE_EXT3 || v->FSType==FS_TYPE_EXT4;
}
static BOOLEAN RootTokenMatches(UINT8 *data, UINTN size, REFIT_VOLUME *v) {
    AUTO_FS *m=FilesystemMeta(v);
    CHAR16 *part=GuidAsString(&v->PartGuid);
    CHAR8 uuid[37],guid[37];
    UINTN i,j,k;
    BOOLEAN match=FALSE,nonzero=FALSE;
    const CHAR8 *hex="0123456789abcdef";
    if(!m || !part) { MyFreePool(part); return FALSE; }
    for(i=0,j=0;i<16;i++) {
        if(i==4 || i==6 || i==8 || i==10) uuid[j++]='-';
        uuid[j++]=hex[m->Uuid[i]>>4]; uuid[j++]=hex[m->Uuid[i]&15];
        if(m->Uuid[i]) nonzero=TRUE;
    }
    uuid[36]=0;
    for(i=0;i<36;i++) { CHAR16 c=part[i]; guid[i]=(c>='A'&&c<='Z') ? c+32 : c; } guid[36]=0;
    for(i=0;i+5<size;i++) {
        if(CompareMem(data+i,"root=",5)) continue;
        j=i+5;
        if(j+5<size && CompareMem(data+j,"UUID=",5)==0 && nonzero) {
            j+=5;
            if(j+36<=size) {
                for(k=0;k<36;k++) { UINT8 c=data[j+k]; if(c>='A'&&c<='Z') c+=32; if(c!=uuid[k]) break; }
                if(k==36) match=TRUE;
            }
        } else if(j+9<size && CompareMem(data+j,"PARTUUID=",9)==0) {
            j+=9;
            if(j+36<=size) {
                for(k=0;k<36;k++) { UINT8 c=data[j+k]; if(c>='A'&&c<='Z') c+=32; if(c!=guid[k]) break; }
                if(k==36) match=TRUE;
            }
        } else if(j+6<size && CompareMem(data+j,"LABEL=",6)==0 && m->Label && m->Label[0]) {
            j+=6;
            for(k=0;m->Label[k] && j+k<size && (CHAR16)data[j+k]==m->Label[k];k++);
            if(!m->Label[k] && (j+k==size || data[j+k]<=32 || data[j+k]=='\'' || data[j+k]=='"')) match=TRUE;
        }
    }
    MyFreePool(part); return match;
}
static BOOLEAN RootInBootConfig(REFIT_VOLUME *boot, REFIT_VOLUME *candidate) {
    CHAR16 *paths[]={L"\\grub2\\grub.cfg",L"\\grub2\\grubenv",L"\\boot\\grub2\\grub.cfg",L"\\boot\\grub2\\grubenv"};
    UINTN n,size,count=0;
    UINT8 *data;
    REFIT_DIR_ITER iter;
    EFI_FILE_INFO *entry;
    CHAR16 *path;
    BOOLEAN found=FALSE;
    if(!boot->RootDir) return FALSE;
    for(n=0;n<4;n++) {
        data=ReadDisplayFile(boot,paths[n],&size);
        if(data) { found|=RootTokenMatches(data,size,candidate); MyFreePool(data); }
    }
    if(FileExists(boot->RootDir,L"\\loader\\entries")) {
        DirIterOpen(boot->RootDir,L"\\loader\\entries",&iter);
        while(count++<128 && DirIterNext(&iter,2,L"*.conf",&entry)) {
            path=PoolPrint(L"\\loader\\entries\\%s",entry->FileName);
            data=ReadDisplayFile(boot,path,&size);
            if(data) { found|=RootTokenMatches(data,size,candidate); MyFreePool(data); }
            MyFreePool(path);
        }
        DirIterClose(&iter);
    }
    return found;
}
static BOOLEAN IsSeparateBoot(REFIT_VOLUME *esp, CHAR16 *loader, REFIT_VOLUME *v) {
    EFI_GUID type={0xbc13c2ff,0x59e6,0x4262,{0xa3,0x52,0xb2,0x75,0xfd,0x6f,0x71,0x72}};
    AUTO_FS *m;
    CHAR16 *path;
    UINT8 *data;
    UINTN size=0,i,j;
    CHAR8 uuid[36];
    const CHAR8 *hex="0123456789abcdef";
    BOOLEAN result=FALSE;
    if(GuidsAreEqual(&v->PartTypeGuid,&type)) return TRUE;
    m=FilesystemMeta(v);if(!m) return FALSE;
    path=FindPath(loader);MergeStrings(&path,L"grub.cfg",L'\\');data=ReadDisplayFile(esp,path,&size);MyFreePool(path);
    for(i=0,j=0;i<16;i++) {
        if(i==4 || i==6 || i==8 || i==10) uuid[j++]='-';
        uuid[j++]=hex[m->Uuid[i]>>4];uuid[j++]=hex[m->Uuid[i]&15];
    }
    if(BufferContains(data,size,(UINT8 *)"--fs-uuid",9) && BufferContains(data,size,(UINT8 *)")/grub2",7) &&
       BufferContains(data,size,(UINT8 *)uuid,36)) result=TRUE;
    MyFreePool(data);return result;
}
static REFIT_VOLUME *DiscoverSystemPartitionKind(REFIT_VOLUME *esp, CHAR16 *loader, unsigned family) {
    REFIT_VOLUME *v,*found=NULL,*fallback=NULL;
    UINTN i,j,count=0,fallbackCount=0,size=0;
    BOOLEAN windows=family==OSR_WINDOWS,mac=family==OSR_MACOS;
    UINT8 *bcd=NULL;
    CHAR16 *path;
    if(!windows && !mac && family!=OSR_LINUX) return NULL;
    if(windows) {
        path=FindPath(loader); MergeStrings(&path,L"BCD",L'\\'); bcd=ReadDisplayFile(esp,path,&size); MyFreePool(path);
    }
    for(i=0;i<VolumesCount;i++) {
        v=Volumes[i];
        if(!PhysicalPartNumber(v) || v==esp) continue;
        if(windows && v->FSType==FS_TYPE_NTFS) {
            if(v->RootDir && FileExists(v->RootDir,L"\\Windows\\System32\\winload.efi")) { fallback=v;fallbackCount++; }
            if(BufferContains(bcd,size,(UINT8 *)&v->PartGuid,16)) { found=v;count++; }
        } else if(mac && IsApfs(v)) { found=v;count++; }
        else if(!windows && !mac && LinuxFilesystem(v)) {
            BOOLEAN matched=FALSE;
            for(j=0;j<VolumesCount;j++) if(SamePhysicalDisk(esp,Volumes[j]) && RootInBootConfig(Volumes[j],v)) matched=TRUE;
            if(matched || (v->RootDir && FileExists(v->RootDir,L"\\etc\\os-release"))) { found=v;count++; }

            if(SamePhysicalDisk(esp,v) && !IsSeparateBoot(esp,loader,v) && (!v->RootDir || (!FileExists(v->RootDir,L"\\grub2") && !FileExists(v->RootDir,L"\\loader\\entries")))) {
                fallback=v;fallbackCount++;
            }
        }
    }
    MyFreePool(bcd);
    if(count==1) return found;
    if(!mac && count==0 && fallbackCount==1) return fallback;
    return NULL;
}
static UINTN DiskOrdinal(REFIT_VOLUME *v) {
    UINTN i,j,ordinal=0;
    for(i=0;i<VolumesCount;i++) {
        BOOLEAN seen=FALSE;
        REFIT_VOLUME *a=Volumes[i];
        if(a->DiskKind!=v->DiskKind || !PhysicalDisk(a)) continue;
        for(j=0;j<i;j++) if(Volumes[j]->DiskKind==v->DiskKind && SamePhysicalDisk(a,Volumes[j])) { seen=TRUE;break; }
        if(!seen) { ordinal++; if(SamePhysicalDisk(a,v)) return ordinal; }
    }
    return 1;
}
static REFIT_VOLUME *DiscoverSystemPartition(REFIT_VOLUME *esp, CHAR16 *loader) {
    OSR_MATCH match=osr_identify(loader,NULL,NULL,NULL,NULL,NULL);
    return DiscoverSystemPartitionKind(esp,loader,match.system->family);
}
static CHAR16 *AutomaticSystemName(CHAR16 *loader) {
    return (CHAR16 *)osr_identify(loader,NULL,NULL,NULL,NULL,NULL).system->title;
}
#include "../../../src/runtime/refind_disk_name.inc"
static CHAR16 *AutomaticLocationForTarget(REFIT_VOLUME *esp, REFIT_VOLUME *target) {
    CHAR16 *name=PhysicalDiskName(target ? target : esp),*result;
    if(target) result=PoolPrint(UI_FROM_S_PARTITION_D,name,PhysicalPartNumber(target));
    else result=PoolPrint(UI_FROM_S_SYSTEM_PARTITION_IS_AMBIGUOUS,name);
    MyFreePool(name);return result;
}
static CHAR16 *AutomaticLocation(REFIT_VOLUME *esp, CHAR16 *loader) {
    return AutomaticLocationForTarget(esp,DiscoverSystemPartition(esp,loader));
}
#include "../../../src/runtime/refind_registry.inc"
CHAR16 *SystemLocationLabel(CHAR16 *tag) {
    CHAR16 *path=StrDuplicate(tag),*guid=NULL,*description=NULL,*result;
    REFIT_VOLUME *esp=NULL;
    if(path && SplitVolumeAndFilename(&path,&guid) && FindVolume(&esp,guid) && esp) {
        description=AutomaticLocation(esp,path);
        result=PoolPrint(L"%s：%s",AutomaticSystemName(path),description);
    } else result=StrDuplicate(tag);
    MyFreePool(path);MyFreePool(guid);MyFreePool(description);return result;
}

#define LINUX_OPTIONS_FILENAMES  L"refind_linux.conf,refind-linux.conf"
#define MAXCONFIGFILESIZE        (128*1024)

#define ENCODING_ISO8859_1  (0)
#define ENCODING_UTF8       (1)
#define ENCODING_UTF16_LE   (2)

#define GetTime ST->RuntimeServices->GetTime
#define LAST_MINUTE 1439

EFI_STATUS ReadFile(IN EFI_FILE_HANDLE BaseDir, IN CHAR16 *FileName, IN OUT REFIT_FILE *File, OUT UINTN *size)
{
    EFI_STATUS      Status;
    EFI_FILE_HANDLE FileHandle;
    EFI_FILE_INFO   *FileInfo;
    UINT64          ReadSize;
    CHAR16          *Message;

    File->Buffer = NULL;
    File->BufferSize = 0;

    Status = refit_call5_wrapper(BaseDir->Open, BaseDir, &FileHandle, FileName, EFI_FILE_MODE_READ, 0);
    Message = PoolPrint(L"while loading the file '%s'", FileName);
    if (CheckError(Status, Message)) {
        MyFreePool(Message);
        return Status;
    }

    FileInfo = LibFileInfo(FileHandle);
    if (FileInfo == NULL) {

        refit_call1_wrapper(FileHandle->Close, FileHandle);
        return EFI_LOAD_ERROR;
    }
    ReadSize = FileInfo->FileSize;
    MyFreePool(FileInfo);

    File->BufferSize = (UINTN)ReadSize;
    File->Buffer = AllocatePool(File->BufferSize);
    if (File->Buffer == NULL) {
       size = 0;
       return EFI_OUT_OF_RESOURCES;
    } else {
       *size = File->BufferSize;
    }
    Status = refit_call3_wrapper(FileHandle->Read, FileHandle, &File->BufferSize, File->Buffer);
    if (CheckError(Status, Message)) {
        MyFreePool(File->Buffer);
        MyFreePool(Message);
        File->Buffer = NULL;
        refit_call1_wrapper(FileHandle->Close, FileHandle);
        return Status;
    }
    Status = refit_call1_wrapper(FileHandle->Close, FileHandle);

    File->Current8Ptr  = (CHAR8 *)File->Buffer;
    File->End8Ptr      = File->Current8Ptr + File->BufferSize;
    File->Current16Ptr = (CHAR16 *)File->Buffer;
    File->End16Ptr     = File->Current16Ptr + (File->BufferSize >> 1);

    File->Encoding = ENCODING_ISO8859_1;
    if (File->BufferSize >= 4) {
        if (File->Buffer[0] == 0xFF && File->Buffer[1] == 0xFE) {

            File->Encoding = ENCODING_UTF16_LE;
            File->Current16Ptr++;
        } else if (File->Buffer[0] == 0xEF && File->Buffer[1] == 0xBB && File->Buffer[2] == 0xBF) {

            File->Encoding = ENCODING_UTF8;
            File->Current8Ptr += 3;
        } else if (File->Buffer[1] == 0 && File->Buffer[3] == 0) {
            File->Encoding = ENCODING_UTF16_LE;
        }

    }

    return EFI_SUCCESS;
}

static CHAR16 *ReadLine(REFIT_FILE *File)
{
    CHAR16  *Line, *q;
    UINTN   LineLength;

    if (File->Buffer == NULL)
        return NULL;

    if (File->Encoding == ENCODING_ISO8859_1 || File->Encoding == ENCODING_UTF8) {

        CHAR8 *p, *LineStart, *LineEnd;

        p = File->Current8Ptr;
        if (p >= File->End8Ptr)
            return NULL;

        LineStart = p;
        for (; p < File->End8Ptr; p++)
            if (*p == 13 || *p == 10)
                break;
        LineEnd = p;
        for (; p < File->End8Ptr; p++)
            if (*p != 13 && *p != 10)
                break;
        File->Current8Ptr = p;

        LineLength = (UINTN)(LineEnd - LineStart) + 1;
        Line = AllocatePool(LineLength * sizeof(CHAR16));
        if (Line == NULL)
            return NULL;

        q = Line;
        if (File->Encoding == ENCODING_ISO8859_1) {
            for (p = LineStart; p < LineEnd; )
                *q++ = *p++;
        } else if (File->Encoding == ENCODING_UTF8) {
            for (p = LineStart; p < LineEnd; ) {
                UINT32 cp;
                UINTN n, needed = 0;
                UINT8 first = (UINT8)*p++;
                cp = first;
                if (first >= 0xC2 && first <= 0xDF) { cp = first & 0x1F; needed = 1; }
                else if (first >= 0xE0 && first <= 0xEF) { cp = first & 0x0F; needed = 2; }
                else if (first >= 0xF0 && first <= 0xF4) { cp = first & 7; needed = 3; }
                else if (first >= 0x80) cp = 0xFFFD;
                if (needed) {
                    if ((UINTN)(LineEnd - p) < needed) { *q++ = 0xFFFD; break; }
                    for (n = 0; n < needed; n++) {
                        if (((UINT8)p[n] & 0xC0) != 0x80) break;
                        cp = (cp << 6) | ((UINT8)p[n] & 0x3F);
                    }
                    if (n != needed) { *q++ = 0xFFFD; continue; }
                    p += needed;
                    if ((needed == 1 && cp < 0x80) || (needed == 2 && cp < 0x800) ||
                        (needed == 3 && cp < 0x10000) || cp > 0x10FFFF ||
                        (cp >= 0xD800 && cp <= 0xDFFF)) cp = 0xFFFD;
                }
                if (cp > 0xFFFF) { cp -= 0x10000; *q++ = 0xD800 + (cp >> 10); *q++ = 0xDC00 + (cp & 0x3FF); }
                else *q++ = (CHAR16)cp;
            }
        }
        *q = 0;

    } else if (File->Encoding == ENCODING_UTF16_LE) {

        CHAR16 *p, *LineStart, *LineEnd;

        p = File->Current16Ptr;
        if (p >= File->End16Ptr)
            return NULL;

        LineStart = p;
        for (; p < File->End16Ptr; p++)
            if (*p == 13 || *p == 10)
                break;
        LineEnd = p;
        for (; p < File->End16Ptr; p++)
            if (*p != 13 && *p != 10)
                break;
        File->Current16Ptr = p;

        LineLength = (UINTN)(LineEnd - LineStart) + 1;
        Line = AllocatePool(LineLength * sizeof(CHAR16));
        if (Line == NULL)
            return NULL;

        for (p = LineStart, q = Line; p < LineEnd; )
            *q++ = *p++;
        *q = 0;

    } else
        return NULL;

    return Line;
}

static BOOLEAN KeepReading(IN OUT CHAR16 *p, IN OUT BOOLEAN *IsQuoted) {
    BOOLEAN MoreToRead = FALSE;
    CHAR16  *Temp = NULL;

    if ((p == NULL) || (IsQuoted == NULL))
        return FALSE;

    if (*p == L'\0')
        return FALSE;

    if ((*p != ' ' && *p != '\t' && *p != '=' && *p != '#' && *p != ',') || *IsQuoted) {
        MoreToRead = TRUE;
    }
    if (*p == L'"') {
        if (p[1] == L'"') {
            Temp = StrDuplicate(&p[1]);
            if (Temp != NULL) {
                StrCpy(p, Temp);
                MyFreePool(Temp);
            }
            MoreToRead = TRUE;
        } else {
            *IsQuoted = !(*IsQuoted);
            MoreToRead = FALSE;
        }
    }

    return MoreToRead;
}

UINTN ReadTokenLine(IN REFIT_FILE *File, OUT CHAR16 ***TokenList)
{
    BOOLEAN         LineFinished, IsQuoted = FALSE;
    CHAR16          *Line, *Token, *p;
    UINTN           TokenCount = 0;

    *TokenList = NULL;

    while (TokenCount == 0) {
        Line = ReadLine(File);
        if (Line == NULL)
            return(0);

        p = Line;
        LineFinished = FALSE;
        while (!LineFinished) {

            while ((*p == ' ' || *p == '\t' || *p == '=' || *p == ',') && !IsQuoted)
                p++;
            if (*p == 0 || *p == '#')
                break;

            if (*p == '"') {
               IsQuoted = !IsQuoted;
               p++;
            }
            Token = p;

            while (KeepReading(p, &IsQuoted)) {
               if ((*p == L'/') && !IsQuoted)
                  *p = L'\\';
               p++;
            }
            if (*p == L'\0' || *p == L'#')
                LineFinished = TRUE;
            *p++ = 0;

            AddListElement((VOID ***)TokenList, &TokenCount, (VOID *)StrDuplicate(Token));
        }

        MyFreePool(Line);
    }
    return (TokenCount);
}

VOID FreeTokenLine(IN OUT CHAR16 ***TokenList, IN OUT UINTN *TokenCount)
{

    FreeList((VOID ***)TokenList, TokenCount);
}

static VOID HandleInt(IN CHAR16 **TokenList, IN UINTN TokenCount, OUT UINTN *Value)
{
    if (TokenCount == 2) {
       if (StrCmp(TokenList[1], L"-1") == 0)
          *Value = -1;
       else
          *Value = Atoi(TokenList[1]);
    }
}

static VOID HandleString(IN CHAR16 **TokenList, IN UINTN TokenCount, OUT CHAR16 **Target) {
    if ((TokenCount == 2) && Target) {
        MyFreePool(*Target);
        *Target = StrDuplicate(TokenList[1]);
    }
}

static VOID HandleStrings(IN CHAR16 **TokenList, IN UINTN TokenCount, OUT CHAR16 **Target) {
    UINTN i;
    BOOLEAN AddMode = FALSE;

    if ((TokenCount > 2) && (StrCmp(TokenList[1], L"+") == 0)) {
        AddMode = TRUE;
    }

    if ((*Target != NULL) && !AddMode) {
        MyFreePool(*Target);
        *Target = NULL;
    }
    for (i = 1; i < TokenCount; i++) {
        if ((i != 1) || !AddMode) {
            CleanUpPathNameSlashes(TokenList[i]);
            MergeStrings(Target, TokenList[i], L',');
        }
    }
}

static VOID HandleHexes(IN CHAR16 **TokenList,
                        IN UINTN TokenCount,
                        IN UINTN MaxValue,
                        OUT UINT32_LIST **Target) {
    UINTN       InputIndex = 1, i;
    UINT32      Value;
    UINT32_LIST *EndOfList = NULL;
    UINT32_LIST *NewEntry;

    if ((TokenCount > 2) && (StrCmp(TokenList[1], L"+") == 0)) {
        InputIndex = 2;
        EndOfList = *Target;
        while (EndOfList && (EndOfList->Next != NULL)) {
            EndOfList = EndOfList->Next;
        }
    } else {
        EraseUint32List(Target);
    }

    for (i = InputIndex; i < TokenCount; i++) {
        if (IsValidHex(TokenList[i])) {
            Value = (UINT32) StrToHex(TokenList[i], 0, 8);
            if (Value <= MaxValue) {
                NewEntry = AllocatePool(sizeof(UINT32_LIST));
                if (NewEntry) {
                    NewEntry->Value = Value;
                    NewEntry->Next = NULL;
                    if (EndOfList == NULL) {
                        EndOfList = NewEntry;
                        *Target = NewEntry;
                    } else {
                        EndOfList->Next = NewEntry;
                        EndOfList = NewEntry;
                    }
                }
            }
        }
    }
}

static UINTN HandleTime(IN CHAR16 *TimeString) {
    UINTN Hour = 0, Minute = 0, TimeLength, i = 0;

    TimeLength = StrLen(TimeString);
    while (i < TimeLength) {
        if (TimeString[i] == L':') {
            Hour = Minute;
            Minute = 0;
        }
        if ((TimeString[i] >= L'0') && (TimeString[i] <= '9')) {
            Minute *= 10;
            Minute += (TimeString[i] - L'0');
        }
        i++;
    }
    return (Hour * 60 + Minute);
}

static BOOLEAN HandleBoolean(IN CHAR16 **TokenList, IN UINTN TokenCount) {
    BOOLEAN TruthValue = TRUE;

    if ((TokenCount >= 2) && ((StrCmp(TokenList[1], L"0") == 0) ||
                              MyStriCmp(TokenList[1], L"false") ||
                              MyStriCmp(TokenList[1], L"off"))) {
        TruthValue = FALSE;
    }

    return TruthValue;
}

static VOID SetDefaultByTime(IN CHAR16 **TokenList, OUT CHAR16 **Default) {
    EFI_STATUS            Status;
    EFI_TIME              CurrentTime;
    UINTN                 StartTime, EndTime, Now;
    BOOLEAN               SetIt = FALSE;

    StartTime = HandleTime(TokenList[2]);
    EndTime = HandleTime(TokenList[3]);

    if ((StartTime <= LAST_MINUTE) && (EndTime <= LAST_MINUTE)) {
        Status = refit_call2_wrapper(GetTime, &CurrentTime, NULL);
        if (Status != EFI_SUCCESS)
            return;
        Now = CurrentTime.Hour * 60 + CurrentTime.Minute;

        if (Now > LAST_MINUTE) {
            Print(L"Warning: Impossible system time: %d:%d\n", CurrentTime.Hour, CurrentTime.Minute);
            return;
        }

        if (StartTime < EndTime) {
            if ((Now >= StartTime) && (Now <= EndTime))
                SetIt = TRUE;
        } else {
            if ((Now >= StartTime) && (Now <= EndTime))
                SetIt = TRUE;
        }

        if (SetIt) {
            MyFreePool(*Default);
            *Default = StrDuplicate(TokenList[1]);
        }
    }
}

static LOADER_ENTRY * AddPreparedLoaderEntry(LOADER_ENTRY *Entry) {
    AddMenuEntry(&MainMenu, (REFIT_MENU_ENTRY *)Entry);

    return(Entry);
}

static VOID SetLinuxMatchPatterns(CHAR16 *Prefixes) {
    CHAR16 *Pattern, *PatternSet = NULL;
    UINTN i = 0;

    while ((Pattern = FindCommaDelimited(Prefixes, i++)) != NULL) {
        MergeStrings(&Pattern, L"*", 0);
        MergeStrings(&PatternSet, Pattern, L',');
        MyFreePool(Pattern);
    }
    if (GlobalConfig.LinuxMatchPatterns)
        MyFreePool(GlobalConfig.LinuxMatchPatterns);
    GlobalConfig.LinuxMatchPatterns = PatternSet;
}

static VOID ReadLanguageConfig(CHAR16 *FileName, BOOLEAN AllowInclude)
{
    REFIT_FILE File;
    CHAR16 **Tokens;
    UINTN Count, Size, i, Depth = 0;
    if (!FileExists(SelfDir, FileName) || EFI_ERROR(ReadFile(SelfDir, FileName, &File, &Size)))
        return;
    for (;;) {
        Count = ReadTokenLine(&File, &Tokens);
        if (!Count)
            break;
        if (!Depth && Count == 2) {
            if (MyStriCmp(Tokens[0], L"language")) {
                if (MyStriCmp(Tokens[1], L"en"))
                    ui_set_language(1);
                else if (MyStriCmp(Tokens[1], L"zh") || MyStriCmp(Tokens[1], L"zh-CN"))
                    ui_set_language(0);
            } else if (AllowInclude && MyStriCmp(Tokens[0], L"include") && !MyStriCmp(Tokens[1], FileName)) {
                ReadLanguageConfig(Tokens[1], FALSE);
            }
        }
        for (i = 0; i < Count; i++) {
            if (MyStriCmp(Tokens[i], L"{"))
                Depth++;
            else if (Depth && MyStriCmp(Tokens[i], L"}"))
                Depth--;
        }
        FreeTokenLine(&Tokens, &Count);
    }
    MyFreePool(File.Buffer);
}

VOID ReadConfig(CHAR16 *FileName)
{
    EFI_STATUS      Status;
    REFIT_FILE      File;
    CHAR16          **TokenList;
    CHAR16          *FlagName;
    CHAR16          *TempStr = NULL;
    UINTN           TokenCount, i;

    if (MyStriCmp(FileName, GlobalConfig.ConfigFilename)) {
        ui_set_language(0);
        ReadLanguageConfig(FileName, TRUE);
        ClearSystemLocations();
        MyFreePool(GlobalConfig.AlsoScan);
        GlobalConfig.AlsoScan = StrDuplicate(ALSO_SCAN_DIRS);
        MyFreePool(GlobalConfig.DontScanDirs);
        if (SelfVolume)
            TempStr = GuidAsString(&(SelfVolume->PartGuid));
        MergeStrings(&TempStr, SelfDirPath, L':');
        if (GlobalConfig.ToolLocations == NULL)
            GlobalConfig.ToolLocations = StrDuplicate(TOOL_LOCATIONS);
        MergeStrings(&TempStr, TOOL_LOCATIONS, L',');
        GlobalConfig.DontScanDirs = TempStr;
        MyFreePool(GlobalConfig.DontScanFiles);
        GlobalConfig.DontScanFiles = StrDuplicate(DONT_SCAN_FILES);
        MyFreePool(GlobalConfig.DontScanTools);
        GlobalConfig.DontScanTools = NULL;
        MyFreePool(GlobalConfig.DontScanFirmware);
        GlobalConfig.DontScanFirmware = NULL;
        MergeStrings(&(GlobalConfig.DontScanFiles), MOK_NAMES, L',');
        MergeStrings(&(GlobalConfig.DontScanFiles), FWUPDATE_NAMES, L',');
        MergeStrings(&(GlobalConfig.DontScanFiles), MEMTEST_NAMES, L',');
        MergeStrings(&(GlobalConfig.DontScanFiles), SHELL_NAMES, L',');
        MyFreePool(GlobalConfig.DontScanVolumes);
        GlobalConfig.DontScanVolumes = StrDuplicate(DONT_SCAN_VOLUMES);
        GlobalConfig.WindowsRecoveryFiles = StrDuplicate(WINDOWS_RECOVERY_FILES);
        GlobalConfig.MacOSRecoveryFiles = StrDuplicate(MACOS_RECOVERY_FILES);
        MyFreePool(GlobalConfig.DefaultSelection);
        GlobalConfig.DefaultSelection = StrDuplicate(L"+");
        GlobalConfig.LinuxPrefixes = StrDuplicate(LINUX_PREFIXES);
    }

    if (!FileExists(SelfDir, FileName)) {
        Print(L"Configuration file '%s' missing!\n", FileName);
        if (!FileExists(SelfDir, L"icons")) {
            Print(L"Icons directory doesn't exist; setting textonly = TRUE!\n");
            GlobalConfig.TextOnly = TRUE;
        }
        return;
    }

    Status = ReadFile(SelfDir, FileName, &File, &i);
    if (EFI_ERROR(Status))
        return;

    for (;;) {
        TokenCount = ReadTokenLine(&File, &TokenList);
        if (TokenCount == 0)
            break;

        if (MyStriCmp(TokenList[0], L"timeout")) {
            HandleInt(TokenList, TokenCount, &(GlobalConfig.Timeout));

        } else if (MyStriCmp(TokenList[0], L"shutdown_after_timeout")) {
           GlobalConfig.ShutdownAfterTimeout = HandleBoolean(TokenList, TokenCount);

        } else if (MyStriCmp(TokenList[0], L"hideui")) {
            for (i = 1; i < TokenCount; i++) {
                FlagName = TokenList[i];
                if (MyStriCmp(FlagName, L"banner")) {
                    GlobalConfig.HideUIFlags |= HIDEUI_FLAG_BANNER;
                } else if (MyStriCmp(FlagName, L"label")) {
                    GlobalConfig.HideUIFlags |= HIDEUI_FLAG_LABEL;
                } else if (MyStriCmp(FlagName, L"singleuser")) {
                    GlobalConfig.HideUIFlags |= HIDEUI_FLAG_SINGLEUSER;
                } else if (MyStriCmp(FlagName, L"hwtest")) {
                    GlobalConfig.HideUIFlags |= HIDEUI_FLAG_HWTEST;
                } else if (MyStriCmp(FlagName, L"arrows")) {
                    GlobalConfig.HideUIFlags |= HIDEUI_FLAG_ARROWS;
                } else if (MyStriCmp(FlagName, L"hints")) {
                    GlobalConfig.HideUIFlags |= HIDEUI_FLAG_HINTS;
                } else if (MyStriCmp(FlagName, L"editor")) {
                    GlobalConfig.HideUIFlags |= HIDEUI_FLAG_EDITOR;
                } else if (MyStriCmp(FlagName, L"safemode")) {
                    GlobalConfig.HideUIFlags |= HIDEUI_FLAG_SAFEMODE;
                } else if (MyStriCmp(FlagName, L"badges")) {
                    GlobalConfig.HideUIFlags |= HIDEUI_FLAG_BADGES;
                } else if (MyStriCmp(FlagName, L"all")) {
                    GlobalConfig.HideUIFlags = HIDEUI_FLAG_ALL;
                } else {
                    Print(L" unknown hideui flag: '%s'\n", FlagName);
                }
            }

        } else if (MyStriCmp(TokenList[0], L"icons_dir")) {
            HandleString(TokenList, TokenCount, &(GlobalConfig.IconsDir));

        } else if (MyStriCmp(TokenList[0], L"scanfor")) {
            for (i = 0; i < NUM_SCAN_OPTIONS; i++) {
                if (i < TokenCount)
                    GlobalConfig.ScanFor[i] = TokenList[i][0];
                else
                    GlobalConfig.ScanFor[i] = ' ';
            }

        } else if (MyStriCmp(TokenList[0], L"follow_symlinks")) {
            GlobalConfig.FollowSymlinks = HandleBoolean(TokenList, TokenCount);

        } else if (MyStriCmp(TokenList[0], L"use_nvram")) {
            GlobalConfig.UseNvram = HandleBoolean(TokenList, TokenCount);

        } else if (MyStriCmp(TokenList[0], L"uefi_deep_legacy_scan")) {
            GlobalConfig.DeepLegacyScan = HandleBoolean(TokenList, TokenCount);

        } else if (MyStriCmp(TokenList[0], L"scan_delay") && (TokenCount == 2)) {
            HandleInt(TokenList, TokenCount, &(GlobalConfig.ScanDelay));

        } else if (MyStriCmp(TokenList[0], L"log_level") && (TokenCount == 2)) {
            HandleInt(TokenList, TokenCount, &(GlobalConfig.LogLevel));

        } else if (MyStriCmp(TokenList[0], L"also_scan_dirs")) {
             HandleStrings(TokenList, TokenCount, &(GlobalConfig.AlsoScan));

        } else if (MyStriCmp(TokenList[0], L"don't_scan_volumes") ||
                   MyStriCmp(TokenList[0], L"dont_scan_volumes")) {

            MyFreePool(GlobalConfig.DontScanVolumes);
            GlobalConfig.DontScanVolumes = NULL;
            for (i = 1; i < TokenCount; i++) {
                MergeStrings(&GlobalConfig.DontScanVolumes, TokenList[i], L',');
            }

        } else if (MyStriCmp(TokenList[0], L"don't_scan_dirs") ||
                   MyStriCmp(TokenList[0], L"dont_scan_dirs")) {
            HandleStrings(TokenList, TokenCount, &(GlobalConfig.DontScanDirs));

        } else if (MyStriCmp(TokenList[0], L"don't_scan_files") ||
                   MyStriCmp(TokenList[0], L"dont_scan_files")) {
            HandleStrings(TokenList, TokenCount, &(GlobalConfig.DontScanFiles));

        } else if (MyStriCmp(TokenList[0], L"don't_scan_firmware") ||
                   MyStriCmp(TokenList[0], L"dont_scan_firmware")) {
            HandleStrings(TokenList, TokenCount, &(GlobalConfig.DontScanFirmware));

        } else if (MyStriCmp(TokenList[0], L"also_scan_tool_dirs")) {
            HandleStrings(TokenList, TokenCount, &(GlobalConfig.ExtraToolLocations));

        } else if (MyStriCmp(TokenList[0], L"don't_scan_tools") ||
                   MyStriCmp(TokenList[0], L"dont_scan_tools")) {
            HandleStrings(TokenList, TokenCount, &(GlobalConfig.DontScanTools));

        } else if (MyStriCmp(TokenList[0], L"windows_recovery_files")) {
            HandleStrings(TokenList, TokenCount, &(GlobalConfig.WindowsRecoveryFiles));

        } else if (MyStriCmp(TokenList[0], L"scan_driver_dirs")) {
            HandleStrings(TokenList, TokenCount, &(GlobalConfig.DriverDirs));

        } else if (MyStriCmp(TokenList[0], L"showtools")) {
            SetMem(GlobalConfig.ShowTools, NUM_TOOLS * sizeof(UINTN), 0);
            GlobalConfig.HiddenTags = FALSE;
            for (i = 1; (i < TokenCount) && (i < NUM_TOOLS); i++) {
                FlagName = TokenList[i];
                if (MyStriCmp(FlagName, L"shell")) {
                    GlobalConfig.ShowTools[i - 1] = TAG_SHELL;
                } else if (MyStriCmp(FlagName, L"gptsync")) {
                    GlobalConfig.ShowTools[i - 1] = TAG_GPTSYNC;
                } else if (MyStriCmp(FlagName, L"gdisk")) {
                   GlobalConfig.ShowTools[i - 1] = TAG_GDISK;
                } else if (MyStriCmp(FlagName, L"about")) {
                   GlobalConfig.ShowTools[i - 1] = TAG_ABOUT;
                } else if (MyStriCmp(FlagName, L"exit")) {
                   GlobalConfig.ShowTools[i - 1] = TAG_EXIT;
                } else if (MyStriCmp(FlagName, L"reboot")) {
                   GlobalConfig.ShowTools[i - 1] = TAG_REBOOT;
                } else if (MyStriCmp(FlagName, L"shutdown")) {
                   GlobalConfig.ShowTools[i - 1] = TAG_SHUTDOWN;
                } else if (MyStriCmp(FlagName, L"install")) {
                   GlobalConfig.ShowTools[i - 1] = TAG_INSTALL;
                } else if (MyStriCmp(FlagName, L"bootorder")) {
                   GlobalConfig.ShowTools[i - 1] = TAG_BOOTORDER;
                } else if (MyStriCmp(FlagName, L"apple_recovery")) {
                   GlobalConfig.ShowTools[i - 1] = TAG_APPLE_RECOVERY;
                } else if (MyStriCmp(FlagName, L"windows_recovery")) {
                   GlobalConfig.ShowTools[i - 1] = TAG_WINDOWS_RECOVERY;
                } else if (MyStriCmp(FlagName, L"mok_tool")) {
                   GlobalConfig.ShowTools[i - 1] = TAG_MOK_TOOL;
                } else if (MyStriCmp(FlagName, L"fwupdate")) {
                   GlobalConfig.ShowTools[i - 1] = TAG_FWUPDATE_TOOL;
                } else if (MyStriCmp(FlagName, L"csr_rotate")) {
                   GlobalConfig.ShowTools[i - 1] = TAG_CSR_ROTATE;
                } else if (MyStriCmp(FlagName, L"firmware")) {
                   GlobalConfig.ShowTools[i - 1] = TAG_FIRMWARE;
                } else if (MyStriCmp(FlagName, L"memtest86") || MyStriCmp(FlagName, L"memtest")) {
                   GlobalConfig.ShowTools[i - 1] = TAG_MEMTEST;
                } else if (MyStriCmp(FlagName, L"netboot")) {
                   GlobalConfig.ShowTools[i - 1] = TAG_NETBOOT;
                } else if (MyStriCmp(FlagName, L"hidden_tags")) {
                    GlobalConfig.ShowTools[i - 1] = TAG_HIDDEN;
                    GlobalConfig.HiddenTags = TRUE;
                } else {
                   Print(L" unknown showtools flag: '%s'\n", FlagName);
                }
            }

        } else if (MyStriCmp(TokenList[0], L"support_gzipped_loaders")) {
           GlobalConfig.GzippedLoaders = HandleBoolean(TokenList, TokenCount);

        } else if (MyStriCmp(TokenList[0], L"banner")) {
            HandleString(TokenList, TokenCount, &(GlobalConfig.BannerFileName));

        } else if (MyStriCmp(TokenList[0], L"banner_scale") && (TokenCount == 2)) {
            if (MyStriCmp(TokenList[1], L"noscale")) {
                GlobalConfig.BannerScale = BANNER_NOSCALE;
            } else if (MyStriCmp(TokenList[1], L"fillscreen") || MyStriCmp(TokenList[1], L"fullscreen")) {
                GlobalConfig.BannerScale = BANNER_FILLSCREEN;
            } else {
                Print(L" unknown banner_type flag: '%s'\n", TokenList[1]);
            }

        } else if (MyStriCmp(TokenList[0], L"arrow_icon_center") && (TokenCount == 2)) {
            HandleInt(TokenList, TokenCount, &i);
            if(i <= 256) GlobalConfig.ArrowIconCenter = i;

        } else if (MyStriCmp(TokenList[0], L"small_icon_size") && (TokenCount == 2)) {
            HandleInt(TokenList, TokenCount, &i);
            if (i >= 32) {
                GlobalConfig.IconSizes[ICON_SIZE_SMALL] = i;
                HaveResized = TRUE;
            }

        } else if (MyStriCmp(TokenList[0], L"big_icon_size") && (TokenCount == 2)) {
            HandleInt(TokenList, TokenCount, &i);
            if (i >= 32) {
                GlobalConfig.IconSizes[ICON_SIZE_BIG] = i;
                GlobalConfig.IconSizes[ICON_SIZE_BADGE] = i / 4;
                HaveResized = TRUE;
            }

        } else if (MyStriCmp(TokenList[0], L"mouse_size") && (TokenCount == 2)) {
            HandleInt(TokenList, TokenCount, &i);
            if (i >= DEFAULT_MOUSE_SIZE) {
                GlobalConfig.IconSizes[ICON_SIZE_MOUSE] = i;
            }

        } else if (MyStriCmp(TokenList[0], L"selection_small")) {
            HandleString(TokenList, TokenCount, &(GlobalConfig.SelectionSmallFileName));

        } else if (MyStriCmp(TokenList[0], L"selection_big")) {
            HandleString(TokenList, TokenCount, &(GlobalConfig.SelectionBigFileName));

        } else if (MyStriCmp(TokenList[0], L"default_selection")) {
            if (TokenCount == 4) {
                SetDefaultByTime(TokenList, &(GlobalConfig.DefaultSelection));
            } else {
                HandleString(TokenList, TokenCount, &(GlobalConfig.DefaultSelection));
            }

        } else if (MyStriCmp(TokenList[0], L"textonly")) {
            GlobalConfig.TextOnly = HandleBoolean(TokenList, TokenCount);

        } else if (MyStriCmp(TokenList[0], L"textmode")) {
            HandleInt(TokenList, TokenCount, &(GlobalConfig.RequestedTextMode));

        } else if (MyStriCmp(TokenList[0], L"resolution") && ((TokenCount == 2) || (TokenCount == 3))) {
            if (MyStriCmp(TokenList[1], L"max")) {
                GlobalConfig.RequestedScreenWidth = MAX_RES_CODE;
                GlobalConfig.RequestedScreenHeight = MAX_RES_CODE;
            } else {
                GlobalConfig.RequestedScreenWidth = Atoi(TokenList[1]);
                if (TokenCount == 3)
                    GlobalConfig.RequestedScreenHeight = Atoi(TokenList[2]);
                else
                    GlobalConfig.RequestedScreenHeight = 0;
            }

        } else if (MyStriCmp(TokenList[0], L"screensaver")) {
            HandleInt(TokenList, TokenCount, &(GlobalConfig.ScreensaverTime));

        } else if (MyStriCmp(TokenList[0], L"use_graphics_for")) {
            if ((TokenCount == 2) || ((TokenCount > 2) && (!MyStriCmp(TokenList[1], L"+"))))
                GlobalConfig.GraphicsFor = 0;
            for (i = 1; i < TokenCount; i++) {
                if (MyStriCmp(TokenList[i], L"osx")) {
                    GlobalConfig.GraphicsFor |= GRAPHICS_FOR_OSX;
                } else if (MyStriCmp(TokenList[i], L"linux")) {
                    GlobalConfig.GraphicsFor |= GRAPHICS_FOR_LINUX;
                } else if (MyStriCmp(TokenList[i], L"elilo")) {
                    GlobalConfig.GraphicsFor |= GRAPHICS_FOR_ELILO;
                } else if (MyStriCmp(TokenList[i], L"grub")) {
                    GlobalConfig.GraphicsFor |= GRAPHICS_FOR_GRUB;
                } else if (MyStriCmp(TokenList[i], L"windows")) {
                    GlobalConfig.GraphicsFor |= GRAPHICS_FOR_WINDOWS;
                }
            }

        } else if (MyStriCmp(TokenList[0], L"font") && (TokenCount == 2)) {
            egLoadFont(TokenList[1]);

        } else if (MyStriCmp(TokenList[0], L"scan_all_linux_kernels")) {
            GlobalConfig.ScanAllLinux = HandleBoolean(TokenList, TokenCount);

        } else if (MyStriCmp(TokenList[0], L"fold_linux_kernels")) {
            GlobalConfig.FoldLinuxKernels = HandleBoolean(TokenList, TokenCount);

        } else if (MyStriCmp(TokenList[0], L"linux_prefixes")) {
            HandleStrings(TokenList, TokenCount, &(GlobalConfig.LinuxPrefixes));

        } else if (MyStriCmp(TokenList[0], L"extra_kernel_version_strings")) {
            HandleStrings(TokenList, TokenCount, &(GlobalConfig.ExtraKernelVersionStrings));

        } else if (MyStriCmp(TokenList[0], L"max_tags")) {
            HandleInt(TokenList, TokenCount, &(GlobalConfig.MaxTags));

        } else if (MyStriCmp(TokenList[0], L"enable_and_lock_vmx")) {
            GlobalConfig.EnableAndLockVMX = HandleBoolean(TokenList, TokenCount);

        } else if (MyStriCmp(TokenList[0], L"spoof_osx_version")) {
            HandleString(TokenList, TokenCount, &(GlobalConfig.SpoofOSXVersion));

        } else if (MyStriCmp(TokenList[0], L"csr_values")) {
            HandleHexes(TokenList, TokenCount, CSR_MAX_LEGAL_VALUE, &(GlobalConfig.CsrValues));

        } else if (MyStriCmp(TokenList[0], L"write_systemd_vars")) {
            GlobalConfig.WriteSystemdVars = HandleBoolean(TokenList, TokenCount);

        } else if (MyStriCmp(TokenList[0], L"include") && (TokenCount == 2) &&
                   MyStriCmp(FileName, GlobalConfig.ConfigFilename)) {
            if (!MyStriCmp(TokenList[1], FileName)) {
                ReadConfig(TokenList[1]);
            }

        } else if (MyStriCmp(TokenList[0], L"enable_mouse")) {
            GlobalConfig.EnableMouse = HandleBoolean(TokenList, TokenCount);
            if (GlobalConfig.EnableMouse) {
                GlobalConfig.EnableTouch = FALSE;
            }

        } else if (MyStriCmp(TokenList[0], L"enable_touch")) {
            GlobalConfig.EnableTouch = HandleBoolean(TokenList, TokenCount);
            if (GlobalConfig.EnableTouch) {
                GlobalConfig.EnableMouse = FALSE;
            }

        } else if (MyStriCmp(TokenList[0], L"mouse_speed") && (TokenCount == 2)) {
            HandleInt(TokenList, TokenCount, &i);
            if (i < 1)
                i = 1;
            if (i > 32)
                i = 32;
            GlobalConfig.MouseSpeed = i;
        }

        FreeTokenLine(&TokenList, &TokenCount);
    }
    if ((GlobalConfig.DontScanFiles) && (GlobalConfig.WindowsRecoveryFiles))
        MergeStrings(&(GlobalConfig.DontScanFiles), GlobalConfig.WindowsRecoveryFiles, L',');
    MyFreePool(File.Buffer);

    SetLinuxMatchPatterns(GlobalConfig.LinuxPrefixes);

    if (!FileExists(SelfDir, L"icons") && !FileExists(SelfDir, GlobalConfig.IconsDir)) {
        Print(L"Icons directory doesn't exist; setting textonly = TRUE!\n");
        GlobalConfig.TextOnly = TRUE;
    }
}

static VOID AddSubmenu(LOADER_ENTRY *Entry, REFIT_FILE *File, REFIT_VOLUME *Volume, CHAR16 *Title) {
    REFIT_MENU_SCREEN  *SubScreen;
    LOADER_ENTRY       *SubEntry;
    UINTN              TokenCount;
    CHAR16             **TokenList;

    SubScreen = InitializeSubScreen(Entry);

    SubEntry = InitializeLoaderEntry(Entry);

    if ((SubEntry == NULL) || (SubScreen == NULL))
        return;
    SubEntry->me.Title        = StrDuplicate(Title);

    while (((TokenCount = ReadTokenLine(File, &TokenList)) > 0) && (StrCmp(TokenList[0], L"}") != 0)) {

        if (MyStriCmp(TokenList[0], L"loader") && (TokenCount > 1)) {
            MyFreePool(SubEntry->LoaderPath);
            SubEntry->LoaderPath = StrDuplicate(TokenList[1]);
            SubEntry->Volume = Volume;

        } else if (MyStriCmp(TokenList[0], L"volume") && (TokenCount > 1)) {
            if (FindVolume(&Volume, TokenList[1])) {
                if ((Volume != NULL) && (Volume->IsReadable) && (Volume->RootDir)) {
                    MyFreePool(SubEntry->me.Title);
                    SubEntry->me.Title      = PoolPrint(UI_BOOT_S_FROM_S,
                                                        (Title != NULL) ? Title : UI_UNKNOWN,
                                                        Volume->VolName);
                    SubEntry->me.BadgeImage = Volume->VolBadgeImage;
                    SubEntry->Volume        = Volume;
                }
            }

        } else if (MyStriCmp(TokenList[0], L"initrd")) {
            MyFreePool(SubEntry->InitrdPath);
            SubEntry->InitrdPath = NULL;
            if (TokenCount > 1) {
                SubEntry->InitrdPath = StrDuplicate(TokenList[1]);
            }

        } else if (MyStriCmp(TokenList[0], L"options")) {
            MyFreePool(SubEntry->LoadOptions);
            SubEntry->LoadOptions = NULL;
            if (TokenCount > 1) {
                SubEntry->LoadOptions = StrDuplicate(TokenList[1]);
            }

        } else if (MyStriCmp(TokenList[0], L"add_options") && (TokenCount > 1)) {
            MergeStrings(&SubEntry->LoadOptions, TokenList[1], L' ');

        } else if (MyStriCmp(TokenList[0], L"graphics") && (TokenCount > 1)) {
            SubEntry->UseGraphicsMode = MyStriCmp(TokenList[1], L"on");

        } else if (MyStriCmp(TokenList[0], L"disabled")) {
            SubEntry->Enabled = FALSE;
        }

        FreeTokenLine(&TokenList, &TokenCount);
    }

    if (SubEntry->InitrdPath != NULL) {
        MergeStrings(&SubEntry->LoadOptions, L"initrd=", L' ');
        MergeStrings(&SubEntry->LoadOptions, SubEntry->InitrdPath, 0);
        MyFreePool(SubEntry->InitrdPath);
        SubEntry->InitrdPath = NULL;
    }
    if (SubEntry->Enabled == TRUE) {
        AddMenuEntry(SubScreen, (REFIT_MENU_ENTRY *)SubEntry);
    }
    Entry->me.SubScreen = SubScreen;
}

static LOADER_ENTRY * AddStanzaEntries(REFIT_FILE *File, REFIT_VOLUME *Volume, CHAR16 *Title) {
    CHAR16       **TokenList;
    UINTN        TokenCount;
    LOADER_ENTRY *Entry;
    BOOLEAN      DefaultsSet = FALSE, AddedSubmenu = FALSE;
    REFIT_VOLUME *CurrentVolume = Volume, *PreviousVolume;

    Entry = InitializeLoaderEntry(NULL);
    if (Entry == NULL)
        return NULL;

    Entry->Title           = StrDuplicate(Title);
    Entry->me.Title        = AllocateZeroPool(256 * sizeof(CHAR16));
    Entry->me.Title        = PoolPrint(UI_BOOT_S_FROM_S,
                                       (Title != NULL) ? Title : UI_UNKNOWN,
                                       CurrentVolume->VolName);
    Entry->me.Row          = 0;
    Entry->me.BadgeImage   = CurrentVolume->VolBadgeImage;
    Entry->Volume          = CurrentVolume;
    Entry->DiscoveryType   = DISCOVERY_TYPE_MANUAL;

    while (((TokenCount = ReadTokenLine(File, &TokenList)) > 0) && (StrCmp(TokenList[0], L"}") != 0)) {
        if (MyStriCmp(TokenList[0], L"loader") && (TokenCount > 1)) {
            Entry->LoaderPath = StrDuplicate(TokenList[1]);
            LOG(1, LOG_LINE_NORMAL, L"Adding manual loader for '%s'", Entry->LoaderPath);
            SetLoaderDefaults(Entry, TokenList[1], CurrentVolume);
            MyFreePool(Entry->LoadOptions);
            Entry->LoadOptions = NULL;
            DefaultsSet = TRUE;

        } else if (MyStriCmp(TokenList[0], L"volume") && (TokenCount > 1)) {
            PreviousVolume = CurrentVolume;
            if (FindVolume(&CurrentVolume, TokenList[1])) {
                if ((CurrentVolume != NULL) && (CurrentVolume->IsReadable) && (CurrentVolume->RootDir)) {
                    MyFreePool(Entry->me.Title);
                    Entry->me.Title        = PoolPrint(UI_BOOT_S_FROM_S,
                                                       (Title != NULL) ? Title : UI_UNKNOWN,
                                                       CurrentVolume->VolName);
                    Entry->me.BadgeImage   = CurrentVolume->VolBadgeImage;
                    Entry->Volume          = CurrentVolume;
                } else {
                    CurrentVolume = PreviousVolume;
                }
            }

        } else if (MyStriCmp(TokenList[0], L"icon") && (TokenCount > 1)) {
            Entry->IconExplicit = TRUE;
            MyFreePool(Entry->me.Image);
            Entry->me.Image = egLoadIcon(CurrentVolume->RootDir, TokenList[1], GlobalConfig.IconSizes[ICON_SIZE_BIG]);
            if (Entry->me.Image == NULL) {
                Entry->me.Image = DummyImage(GlobalConfig.IconSizes[ICON_SIZE_BIG]);
            }

        } else if (MyStriCmp(TokenList[0], L"initrd") && (TokenCount > 1)) {
            MyFreePool(Entry->InitrdPath);
            Entry->InitrdPath = StrDuplicate(TokenList[1]);

        } else if (MyStriCmp(TokenList[0], L"options") && (TokenCount > 1)) {
            MyFreePool(Entry->LoadOptions);
            Entry->LoadOptions = StrDuplicate(TokenList[1]);

        } else if (MyStriCmp(TokenList[0], L"ostype") && (TokenCount > 1)) {
            if (TokenCount > 1) {
                Entry->OSType = TokenList[1][0];
            }

        } else if (MyStriCmp(TokenList[0], L"graphics") && (TokenCount > 1)) {
            Entry->UseGraphicsMode = MyStriCmp(TokenList[1], L"on");

        } else if (MyStriCmp(TokenList[0], L"disabled")) {
            LOG(1, LOG_LINE_NORMAL, L"Entry is disabled");
            Entry->Enabled = FALSE;

        } else if (MyStriCmp(TokenList[0], L"firmware_bootnum") && (TokenCount > 1)) {
            Entry->EfiBootNum = StrToHex(TokenList[1], 0, 16);
            Entry->EfiLoaderPath = NULL;
            Entry->LoaderPath = NULL;
            MyFreePool(Entry->me.Title);
            Entry->me.Title = StrDuplicate(Title);
            Entry->me.BadgeImage = BuiltinIcon(BUILTIN_ICON_VOL_EFI);
            Entry->me.Tag = TAG_FIRMWARE_LOADER;

        } else if (MyStriCmp(TokenList[0], L"submenuentry") && (TokenCount > 1)) {
            AddSubmenu(Entry, File, CurrentVolume, TokenList[1]);
            AddedSubmenu = TRUE;

        }
        FreeTokenLine(&TokenList, &TokenCount);
    }

    if (AddedSubmenu)
         AddMenuEntry(Entry->me.SubScreen, &MenuEntryReturn);

    if (Entry->InitrdPath) {
        MergeStrings(&Entry->LoadOptions, L"initrd=", L' ');
        MergeStrings(&Entry->LoadOptions, Entry->InitrdPath, 0);
        MyFreePool(Entry->InitrdPath);
        Entry->InitrdPath = NULL;
    }

    if (!DefaultsSet) {

        SetLoaderDefaults(Entry, L"\\EFI\\BOOT\\nemo.efi", CurrentVolume);
    }

    return(Entry);
}

VOID ScanUserConfigured(CHAR16 *FileName)
{
    EFI_STATUS        Status;
    REFIT_FILE        File;
    REFIT_VOLUME      *Volume;
    CHAR16            **TokenList;
    UINTN             TokenCount, size;
    LOADER_ENTRY      *Entry;

    LOG(1, LOG_LINE_THIN_SEP, L"Scanning for user-configured boot stanzas");
    if (FileExists(SelfDir, FileName)) {
        Status = ReadFile(SelfDir, FileName, &File, &size);
        if (EFI_ERROR(Status))
            return;

        Volume = SelfVolume;

        while ((TokenCount = ReadTokenLine(&File, &TokenList)) > 0) {
            if (MyStriCmp(TokenList[0], L"menuentry") && (TokenCount > 1)) {
                Entry = AddStanzaEntries(&File, Volume, TokenList[1]);
                if ((Entry) && (Entry->Enabled)) {
                    ApplySystemLocation(Entry);
                    if (Entry->me.SubScreen == NULL)
                        GenerateSubScreen(Entry, Volume, TRUE);
                    AddPreparedLoaderEntry(Entry);
                } else {
                    MyFreePool(Entry);
                }

            } else if (MyStriCmp(TokenList[0], L"include") && (TokenCount == 2) &&
                       MyStriCmp(FileName, GlobalConfig.ConfigFilename)) {
                if (!MyStriCmp(TokenList[1], FileName)) {
                    ScanUserConfigured(TokenList[1]);
                }

            }
            FreeTokenLine(&TokenList, &TokenCount);
        }
    }
}

static REFIT_FILE * GenerateOptionsFromEtcFstab(REFIT_VOLUME *Volume) {
    UINTN        TokenCount, i;
    REFIT_FILE   *Options = NULL, *Fstab = NULL;
    EFI_STATUS   Status;
    CHAR16       **TokenList, *Line, *Root = NULL;

    if (FileExists(Volume->RootDir, L"\\etc\\fstab")) {
        Options = AllocateZeroPool(sizeof(REFIT_FILE));
        Fstab = AllocateZeroPool(sizeof(REFIT_FILE));
        Status = ReadFile(Volume->RootDir, L"\\etc\\fstab", Fstab, &i);
        if (CheckError(Status, L"while reading /etc/fstab")) {
            if (Options != NULL)
                MyFreePool(Options);
            if (Fstab != NULL)
                MyFreePool(Fstab);
            Options = NULL;
            Fstab = NULL;
        } else {
            Options->Encoding = ENCODING_UTF16_LE;
            while ((TokenCount = ReadTokenLine(Fstab, &TokenList)) > 0) {
                LOG(3, LOG_LINE_NORMAL, L"Read line from /etc/fstab holding %d tokens", TokenCount);
                if (TokenCount > 2) {
                    if (StrCmp(TokenList[1], L"\\") == 0) {
                        Root = PoolPrint(L"%s", TokenList[0]);
                    } else if (StrCmp(TokenList[2], L"\\") == 0) {
                        Root = PoolPrint(L"%s=%s", TokenList[0], TokenList[1]);
                    }
                    if (Root && (Root[0] != L'\0')) {
                        for (i = 0; i < StrLen(Root); i++)
                            if (Root[i] == '\\')
                                Root[i] = '/';
                        Line = PoolPrint(L"\"Boot with normal options\"    \"ro root=%s\"\n", Root);
                        MergeStrings((CHAR16 **) &(Options->Buffer), Line, 0);
                        MyFreePool(Line);
                        Line = PoolPrint(L"\"Boot into single-user mode\"  \"ro root=%s single\"\n", Root);
                        MergeStrings((CHAR16**) &(Options->Buffer), Line, 0);
                        MyFreePool(Line);
                        Options->BufferSize = StrLen((CHAR16*) Options->Buffer) * sizeof(CHAR16);
                    }
                    MyFreePool(Root);
                    Root = NULL;
                 }
                 FreeTokenLine(&TokenList, &TokenCount);
            }

            if (Options->Buffer) {
                Options->Current8Ptr  = (CHAR8 *)Options->Buffer;
                Options->End8Ptr      = Options->Current8Ptr + Options->BufferSize;
                Options->Current16Ptr = (CHAR16 *)Options->Buffer;
                Options->End16Ptr     = Options->Current16Ptr + (Options->BufferSize >> 1);
            } else {
                MyFreePool(Options);
                Options = NULL;
            }

            MyFreePool(Fstab->Buffer);
            MyFreePool(Fstab);
        }
    }
    return Options;
}

static REFIT_FILE * GenerateOptionsFromPartTypes(VOID) {
    REFIT_FILE   *Options = NULL;
    CHAR16       *Line, *GuidString, *WriteStatus;

    if (GlobalConfig.DiscoveredRoot) {
        Options = AllocateZeroPool(sizeof(REFIT_FILE));
        if (Options) {
            Options->Encoding = ENCODING_UTF16_LE;
            GuidString = GuidAsString(&(GlobalConfig.DiscoveredRoot->PartGuid));
            WriteStatus = GlobalConfig.DiscoveredRoot->IsMarkedReadOnly ? L"ro" : L"rw";
            ToLower(GuidString);
            if (GuidString) {
                Line = PoolPrint(L"\"Boot with normal options\"    \"%s root=/dev/disk/by-partuuid/%s\"\n",
                                 WriteStatus, GuidString);
                MergeStrings((CHAR16 **) &(Options->Buffer), Line, 0);
                MyFreePool(Line);
                Line = PoolPrint(L"\"Boot into single-user mode\"  \"%s root=/dev/disk/by-partuuid/%s single\"\n",
                                 WriteStatus, GuidString);
                MergeStrings((CHAR16**) &(Options->Buffer), Line, 0);
                MyFreePool(Line);
                MyFreePool(GuidString);
            }
            Options->BufferSize = StrLen((CHAR16*) Options->Buffer) * sizeof(CHAR16);

            Options->Current8Ptr  = (CHAR8 *)Options->Buffer;
            Options->End8Ptr      = Options->Current8Ptr + Options->BufferSize;
            Options->Current16Ptr = (CHAR16 *)Options->Buffer;
            Options->End16Ptr     = Options->Current16Ptr + (Options->BufferSize >> 1);
        }
    }
    return Options;
}

REFIT_FILE * ReadLinuxOptionsFile(IN CHAR16 *LoaderPath, IN REFIT_VOLUME *Volume) {
    CHAR16       *OptionsFilename, *FullFilename;
    BOOLEAN      GoOn = TRUE, FileFound = FALSE;
    UINTN        i = 0, size;
    REFIT_FILE   *File = NULL;
    EFI_STATUS   Status;

    do {
        OptionsFilename = FindCommaDelimited(LINUX_OPTIONS_FILENAMES, i++);
        FullFilename = FindPath(LoaderPath);
        if ((OptionsFilename != NULL) && (FullFilename != NULL)) {
            MergeStrings(&FullFilename, OptionsFilename, '\\');
            if (FileExists(Volume->RootDir, FullFilename)) {
                File = AllocateZeroPool(sizeof(REFIT_FILE));
                Status = ReadFile(Volume->RootDir, FullFilename, File, &size);
                if (CheckError(Status, L"while loading the Linux options file")) {
                    MyFreePool(File);
                    File = NULL;
                } else {
                    GoOn = FALSE;
                    FileFound = TRUE;
                }
            }
        } else {
            GoOn = FALSE;
        }
        MyFreePool(OptionsFilename);
        MyFreePool(FullFilename);
        OptionsFilename = FullFilename = NULL;
    } while (GoOn);
    if (!FileFound) {

        File = GenerateOptionsFromEtcFstab(Volume);

        if (!File)
            File = GenerateOptionsFromPartTypes();
    }
    return (File);
}

CHAR16 * GetFirstOptionsFromFile(IN CHAR16 *LoaderPath, IN REFIT_VOLUME *Volume) {
    UINTN        TokenCount;
    CHAR16       *Options = NULL;
    CHAR16       **TokenList;
    REFIT_FILE   *File;

    File = ReadLinuxOptionsFile(LoaderPath, Volume);
    if (File != NULL) {
        TokenCount = ReadTokenLine(File, &TokenList);
        if (TokenCount > 1)
            Options = StrDuplicate(TokenList[1]);
        FreeTokenLine(&TokenList, &TokenCount);
        MyFreePool(File);
    }
    return Options;
}

#include "disk_name.h"
typedef struct _AUTO_DISK_INFO AUTO_DISK_INFO;
typedef EFI_STATUS (EFIAPI *AUTO_DISK_QUERY)(AUTO_DISK_INFO *,VOID *,UINT32 *);
struct _AUTO_DISK_INFO {
    EFI_GUID Interface;
    AUTO_DISK_QUERY Inquiry;
    AUTO_DISK_QUERY Identify;
    VOID *SenseData;
    VOID *WhichIde;
};
typedef struct { EFI_BLOCK_IO *Disk; CHAR16 *Name; } AUTO_DISK_NAME;
static AUTO_DISK_NAME AutoDiskNames[64];
static UINTN AutoDiskNameCount;
static VOID ClearDiskNames(VOID) {
    UINTN i;
    for(i=0;i<AutoDiskNameCount;i++) MyFreePool(AutoDiskNames[i].Name);
    AutoDiskNameCount=0;
}
static EFI_HANDLE PhysicalDiskHandle(REFIT_VOLUME *v) {
    EFI_BLOCK_IO *disk=PhysicalDisk(v),*io;
    EFI_DEVICE_PATH *path=v->WholeDiskDevicePath;
    EFI_HANDLE handle=NULL,*handles=NULL;
    UINTN i,count=0;
    if(!disk)return NULL;
    if(path && !EFI_ERROR(refit_call3_wrapper(BS->LocateDevicePath,&BlockIoProtocol,&path,&handle)) &&
       !EFI_ERROR(refit_call3_wrapper(BS->HandleProtocol,handle,&BlockIoProtocol,(VOID **)&io)) && io==disk) return handle;
    if(!EFI_ERROR(refit_call5_wrapper(BS->LocateHandleBuffer,ByProtocol,&BlockIoProtocol,NULL,&count,&handles))) {
        for(i=0;i<count;i++) {
            if(!EFI_ERROR(refit_call3_wrapper(BS->HandleProtocol,handles[i],&BlockIoProtocol,(VOID **)&io)) && io==disk) {
                handle=handles[i];MyFreePool(handles);return handle;
            }
        }
    }
    MyFreePool(handles);return NULL;
}
static CHAR16 *DiskInfoName(EFI_HANDLE handle) {
    EFI_GUID protocol={0xd432a67f,0x14dc,0x484b,{0xb3,0xbb,0x3f,0x02,0x91,0x84,0x93,0x27}};
    EFI_GUID ide={0x5e948fe3,0x26d3,0x42b5,{0xaf,0x17,0x61,0x02,0x87,0x18,0x8d,0xec}};
    EFI_GUID ahci={0x9e498932,0x4abc,0x45af,{0xa3,0x4d,0x02,0x47,0x78,0x7b,0xe7,0xc6}};
    EFI_GUID usb={0xcb871572,0xc11a,0x47b5,{0xb4,0x92,0x67,0x5e,0xaf,0xa7,0x77,0x27}};
    EFI_GUID scsi={0x08f74baa,0xea36,0x41d9,{0x95,0x21,0x21,0xa7,0x0f,0x87,0x80,0xbc}};
    AUTO_DISK_INFO *info=NULL;
    UINT8 data[512];
    CHAR8 name[80];
    UINT32 size;
    unsigned kind=0;
    EFI_STATUS status;
    if(EFI_ERROR(refit_call3_wrapper(BS->HandleProtocol,handle,&protocol,(VOID **)&info)) || !info)return NULL;

    size=sizeof(data);SetMem(data,sizeof(data),0);
    if((GuidsAreEqual(&info->Interface,&ide)||GuidsAreEqual(&info->Interface,&ahci)) && info->Identify) {
        status=refit_call3_wrapper(info->Identify,info,data,&size);kind=DISK_NAME_ATA;
    } else if((GuidsAreEqual(&info->Interface,&usb)||GuidsAreEqual(&info->Interface,&scsi)) && info->Inquiry) {
        status=refit_call3_wrapper(info->Inquiry,info,data,&size);kind=DISK_NAME_SCSI;
    } else return NULL;
    if(EFI_ERROR(status) || size>sizeof(data) || !disk_name_decode(kind,data,size,name,sizeof(name)))return NULL;
    return AsciiLabel((UINT8 *)name,AsciiStrLen(name));
}
static BOOLEAN IsNvmeDiskHandle(EFI_HANDLE handle) {
    EFI_DEVICE_PATH *path=DevicePathFromHandle(handle);
    while(path && !IsDevicePathEndType(path)) {

        if(DevicePathType(path)==MESSAGING_DEVICE_PATH && DevicePathSubType(path)==0x17)return TRUE;
        path=NextDevicePathNode(path);
    }
    return FALSE;
}
static CHAR16 *CleanControllerName(CHAR16 *name, BOOLEAN nvme) {
    UINTN i,n=0,space=0;
    CHAR16 *out;
    if(!name)return NULL;
    out=AllocateZeroPool(129*sizeof(CHAR16));if(!out)return NULL;
    for(i=0;i<128 && name[i];i++) {}
    if(i==128) { MyFreePool(out);return NULL; }
    if(nvme && disk_name_nvme_model(name,(unsigned)i,out,129))return out;
    for(i=0;i<128 && name[i];i++) {
        CHAR16 c=name[i];
        if(c<32 || (c>=0xD800 && c<=0xDFFF)) { MyFreePool(out);return NULL; }
        if(c==L' ') { if(n)space=1;continue; }
        if(space)out[n++]=L' ';
        out[n++]=c;space=0;
    }
    if(!n || i==128) { MyFreePool(out);return NULL; }
    return out;
}
static CHAR16 *ComponentDiskName(EFI_HANDLE diskHandle) {
    EFI_GUID *protocols[]={&gEfiComponentName2ProtocolGuid,&gEfiComponentNameProtocolGuid};
    EFI_HANDLE *drivers=NULL,*parents=NULL;
    EFI_DEVICE_PATH *diskPath=DevicePathFromHandle(diskHandle),*parentPath;
    UINTN p,d,c,driverCount,parentCount=0,diskSize,parentSize,l;
    EFI_COMPONENT_NAME2_PROTOCOL *component;
    EFI_STATUS status;
    CHAR8 language[16];
    CHAR16 *name,*result=NULL;
    BOOLEAN nvme=IsNvmeDiskHandle(diskHandle);
    if(!diskPath)return NULL;
    diskSize=DevicePathSize(diskPath);
    if(EFI_ERROR(refit_call5_wrapper(BS->LocateHandleBuffer,ByProtocol,&DevicePathProtocol,NULL,&parentCount,&parents)))parentCount=0;
    for(p=0;p<2 && !result;p++) {
        driverCount=0;drivers=NULL;
        status=refit_call5_wrapper(BS->LocateHandleBuffer,ByProtocol,protocols[p],NULL,&driverCount,&drivers);
        if(EFI_ERROR(status))continue;
        for(d=0;d<driverCount && !result;d++) {
            if(EFI_ERROR(refit_call3_wrapper(BS->HandleProtocol,drivers[d],protocols[p],(VOID **)&component)) || !component->SupportedLanguages)continue;
            l=0;
            while(l<sizeof(language)-1 && component->SupportedLanguages[l] && component->SupportedLanguages[l]!=';') {
                language[l]=component->SupportedLanguages[l];l++;
            }
            language[l]=0;
            if(!l || (!p && component->SupportedLanguages[l] && component->SupportedLanguages[l]!=';'))continue;

            if(p) { if(l<3)continue;language[3]=0; }
            name=NULL;
            status=refit_call5_wrapper(component->GetControllerName,component,diskHandle,NULL,language,&name);
            if(!EFI_ERROR(status))result=CleanControllerName(name,nvme);

            for(c=0;c<parentCount && !result;c++) {
                if(parents[c]==diskHandle)continue;
                parentPath=DevicePathFromHandle(parents[c]);if(!parentPath)continue;
                parentSize=DevicePathSize(parentPath);
                if(parentSize<END_DEVICE_PATH_LENGTH || parentSize>=diskSize ||
                   CompareMem(parentPath,diskPath,parentSize-END_DEVICE_PATH_LENGTH))continue;
                name=NULL;
                status=refit_call5_wrapper(component->GetControllerName,component,parents[c],diskHandle,language,&name);
                if(!EFI_ERROR(status))result=CleanControllerName(name,nvme);
            }
        }
        MyFreePool(drivers);
    }
    MyFreePool(parents);return result;
}
static CHAR16 *PhysicalDiskName(REFIT_VOLUME *v) {
    EFI_BLOCK_IO *disk=PhysicalDisk(v);
    EFI_HANDLE handle;
    CHAR16 *name=NULL;
    UINTN i;
    for(i=0;i<AutoDiskNameCount;i++) if(AutoDiskNames[i].Disk==disk)return StrDuplicate(AutoDiskNames[i].Name);
    handle=PhysicalDiskHandle(v);
    if(handle) {
        name=DiskInfoName(handle);
        if(!name)name=ComponentDiskName(handle);
    }
    if(!name)name=PoolPrint(UI_S_DISK_D,v->DiskKind==DISK_KIND_EXTERNAL ? UI_EXTERNAL : UI_INTERNAL,DiskOrdinal(v));
    if(AutoDiskNameCount<64) {
        AutoDiskNames[AutoDiskNameCount].Disk=disk;AutoDiskNames[AutoDiskNameCount++].Name=StrDuplicate(name);
    }
    return name;
}

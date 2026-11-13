VOID ApplySystemLocation(LOADER_ENTRY *entry) {
    OSR_MATCH match;
    REFIT_VOLUME *target;
    AUTO_FS *meta;
    osr_char id[128],variant[128];
    UINT8 *release=NULL;
    UINTN size=0;
    EG_IMAGE *icon;
    CHAR16 *label=NULL;
    if(!entry || !entry->Volume || !entry->LoaderPath) return;
    match=osr_identify(entry->LoaderPath,entry->Title,entry->Volume->FsName,entry->Volume->PartName,NULL,NULL);
    target=DiscoverSystemPartitionKind(entry->Volume,entry->LoaderPath,match.system->family);
    id[0]=variant[0]=0;
    if(target && match.system->family==OSR_LINUX) {
        meta=FilesystemMeta(target);
        if(meta) label=meta->Label;

        release=ReadDisplayFile(target,L"\\etc\\os-release",&size);
        if(!release && target->RootDir && !FileExists(target->RootDir,L"\\etc\\os-release"))
            release=ReadDisplayFile(target,L"\\usr\\lib\\os-release",&size);
        if(release) osr_parse_release(release,(unsigned)size,id,128,variant,128);
        MyFreePool(release);
    }
    match=osr_identify(entry->LoaderPath,entry->Title,label?label:entry->Volume->FsName,entry->Volume->PartName,id,variant);
    if(match.system->family!=OSR_LINUX && match.system->family!=OSR_WINDOWS && match.system->family!=OSR_MACOS) target=NULL;
    MyFreePool(entry->me.Title);
    entry->me.Title=AutomaticLocationForTarget(entry->Volume,target);

    if(entry->DiscoveryType!=DISCOVERY_TYPE_MANUAL) {
        MyFreePool(entry->Title);entry->Title=StrDuplicate((CHAR16 *)match.system->title);
    }
    if(!entry->IconExplicit) {
        icon=LoadOSIcon((CHAR16 *)match.system->icon_key,
                        match.system->family==OSR_LINUX ? L"linux" : L"unknown",FALSE);
        if(icon) { egFreeImage(entry->me.Image);entry->me.Image=icon; }
    }
}

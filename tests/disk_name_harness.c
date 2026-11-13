#include <assert.h>
#include <string.h>
#include "../src/runtime/disk_name.h"
static unsigned nvme_name(unsigned short *raw,unsigned serial_width,unsigned model_width,const char *model,const char *suffix) {
    unsigned i,offset=serial_width+1,tail=offset+model_width;
    const char *serial="SERIAL-TEST-123";
    for(i=0;i<serial_width;i++)raw[i]=i<strlen(serial)?(unsigned char)serial[i]:' ';
    raw[serial_width]='-';
    for(i=0;i<model_width;i++)raw[offset+i]=i<strlen(model)?(unsigned char)model[i]:' ';
    raw[tail++]='-';for(i=0;suffix[i];i++)raw[tail+i]=(unsigned char)suffix[i];
    return tail+i;
}
static void same_model(const unsigned short *wide,const char *expected) {
    unsigned i;for(i=0;expected[i];i++)assert(wide[i]==(unsigned char)expected[i]);assert(!wide[i]);
}
static void test_nvme_models(void) {
    unsigned short raw[128]={0},out[80];unsigned units;
    units=nvme_name(raw,20,40,"Example NVMe 1TB SSD","1234ABCDEF");
    assert(disk_name_nvme_model(raw,units,out,80));same_model(out,"Example NVMe 1TB SSD");
    units=nvme_name(raw,20,40,"ACME-980-PRO 2TB","0");
    assert(disk_name_nvme_model(raw,units,out,80));same_model(out,"ACME-980-PRO 2TB");
    units=nvme_name(raw,19,39,"Legacy-Model 512GB","ABCDEF");
    assert(disk_name_nvme_model(raw,units,out,80));same_model(out,"Legacy-Model 512GB");
    assert(!disk_name_nvme_model(raw,units,out,4) && !out[0]);
    assert(!disk_name_nvme_model(raw,40,out,80) && !out[0]);
    units=nvme_name(raw,20,40,"  ","1234");
    assert(!disk_name_nvme_model(raw,units,out,80) && !out[0]);
    units=nvme_name(raw,20,40,"Model-980 1TB","NOT-HEX");
    assert(!disk_name_nvme_model(raw,units,out,80) && !out[0]);
    units=nvme_name(raw,20,40,"Model-980 1TB","12345678901234567");
    assert(!disk_name_nvme_model(raw,units,out,80) && !out[0]);
    units=nvme_name(raw,20,40,"Model-980 1TB","0");raw[24]=1;
    assert(!disk_name_nvme_model(raw,units,out,80) && !out[0]);
    assert(!disk_name_nvme_model(0,0,out,80));assert(!disk_name_nvme_model(raw,units,0,0));
}
int main(void) {
    test_nvme_models();
    unsigned char ata[512]={0},scsi[36]={0};char out[80];unsigned i;
    const char *model="  Model X 2000 GB                        ";
    for(i=0;i<40;i++)ata[54+(i^1u)]=(unsigned char)model[i];
    assert(disk_name_decode(DISK_NAME_ATA,ata,sizeof(ata),out,sizeof(out)));
    assert(!strcmp(out,"Model X 2000 GB"));
    scsi[4]=31;memcpy(scsi+8,"Vendor  Product-2000    ",24);
    assert(disk_name_decode(DISK_NAME_SCSI,scsi,sizeof(scsi),out,sizeof(out)));
    assert(!strcmp(out,"Vendor Product-2000"));

    memcpy(scsi+32,"1234",4);
    assert(disk_name_decode(DISK_NAME_SCSI,scsi,sizeof(scsi),out,sizeof(out)));
    assert(!strcmp(out,"Vendor Product-2000"));
    assert(!disk_name_decode(DISK_NAME_ATA,ata,93,out,sizeof(out)) && !out[0]);
    assert(!disk_name_decode(DISK_NAME_SCSI,scsi,31,out,sizeof(out)) && !out[0]);
    scsi[4]=0;assert(!disk_name_decode(DISK_NAME_SCSI,scsi,sizeof(scsi),out,sizeof(out)));
    scsi[4]=31;scsi[9]=1;assert(!disk_name_decode(DISK_NAME_SCSI,scsi,sizeof(scsi),out,sizeof(out)) && !out[0]);
    assert(!disk_name_decode(DISK_NAME_ATA,ata,sizeof(ata),out,8) && !out[0]);
    memset(ata+54,0xff,40);assert(!disk_name_decode(DISK_NAME_ATA,ata,sizeof(ata),out,sizeof(out)));
    assert(!disk_name_decode(999,ata,sizeof(ata),out,sizeof(out)));
    assert(!disk_name_decode(DISK_NAME_ATA,0,0,out,sizeof(out)));
    assert(!disk_name_decode(DISK_NAME_ATA,ata,sizeof(ata),0,0));
    return 0;
}

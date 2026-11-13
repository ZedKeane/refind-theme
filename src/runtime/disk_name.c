#include "disk_name.h"
unsigned disk_name_decode(unsigned kind,const unsigned char *data,unsigned size,char *out,unsigned cap) {
    unsigned i,n=0,space=0,start,len,c;
    if(!cap) return 0;
    out[0]=0;
    if(!data) return 0;
    if(kind==DISK_NAME_ATA) { start=54;len=40;if(size<94)return 0; }
    else if(kind==DISK_NAME_SCSI) { start=8;len=24;if(size<32 || (data[4]+5u)<32)return 0; }
    else return 0;
    for(i=0;i<len;i++) {
        c=data[start+(kind==DISK_NAME_ATA ? (i^1u) : i)];
        if(c==0 || c==255) c=' ';
        if(c<32 || c>126) { out[0]=0;return 0; }
        if(c==' ') { if(n)space=1;continue; }

        if(n+space+1>=cap) { out[0]=0;return 0; }
        if(space)out[n++]=' ';
        out[n++]=(char)c;space=0;
    }
    out[n]=0;return n;
}

unsigned disk_name_nvme_model(const unsigned short *raw,unsigned units,unsigned short *out,unsigned cap) {
    unsigned format,sn,mn,end,i,n,space,c,hexlen;
    if(!cap)return 0;
    out[0]=0;if(!raw)return 0;
    for(format=0;format<2;format++) {
        sn=format?19:20;mn=format?39:40;end=sn+1+mn;
        if(units<=end+1 || raw[sn]!='-' || raw[end]!='-')continue;
        hexlen=units-end-1;if(hexlen>16)continue;
        for(i=end+1;i<units;i++) {
            c=raw[i];if(!((c>='0'&&c<='9')||(c>='a'&&c<='f')||(c>='A'&&c<='F')))break;
        }
        if(i!=units)continue;
        for(i=0;i<sn;i++)if(raw[i]<32 || raw[i]>126)break;
        if(i!=sn)continue;
        n=0;space=0;
        for(i=sn+1;i<end;i++) {
            c=raw[i];if(c<32 || c>126)break;
            if(c==' ') { if(n)space=1;continue; }
            if(n+space+1>=cap)break;
            if(space)out[n++]=' ';
            out[n++]=(unsigned short)c;space=0;
        }
        if(i==end && n) { out[n]=0;return n; }
    }
    out[0]=0;return 0;
}

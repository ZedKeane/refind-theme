#include <stdio.h>
#include <string.h>
#include "../src/runtime/os_registry.h"
static void from_utf8(const char *p,osr_char *out,unsigned cap) {
 unsigned n=0,c,k,need;
 while(*p&&n+1<cap) {
  c=(unsigned char)*p++;need=0;
  if(c>=0xC2&&c<=0xDF){c&=31;need=1;}else if(c>=0xE0&&c<=0xEF){c&=15;need=2;}else if(c>=128)c='?';
  for(k=0;k<need&&*p;k++)c=(c<<6)|((unsigned char)*p++&63);out[n++]=(osr_char)c;
 }out[n]=0;
}
int main(int argc,char **argv) {
 osr_char v[6][1024];OSR_MATCH m;unsigned i;
 if(argc>1&&!strcmp(argv[1],"--release")) {
  osr_parse_release((const unsigned char *)argv[2],(unsigned)strlen(argv[2]),v[0],1024,v[1],1024);
  for(i=0;v[0][i];i++)putchar(v[0][i]);putchar('|');for(i=0;v[1][i];i++)putchar(v[1][i]);putchar('\n');return 0;
 }
 for(i=0;i<6;i++)from_utf8(argc>(int)i+1?argv[i+1]:"",v[i],1024);
 m=osr_identify(v[0],v[1],v[2],v[3],v[4],v[5]);printf("%s|%u|%s\n",m.system->id,m.score,m.evidence);return 0;
}

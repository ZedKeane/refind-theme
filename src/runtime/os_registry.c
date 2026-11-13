#include "os_registry.h"
#include "../generated/os_registry_data.h"
static unsigned fold(unsigned c) { return c>='A'&&c<='Z' ? c+32 : c; }
static int word(unsigned c) { c=fold(c);return (c>='a'&&c<='z')||(c>='0'&&c<='9')||(c>=128&&!(c>=0x2000&&c<=0x206f)&&!(c>=0x3000&&c<=0x303f)); }
static unsigned length(const osr_char *s) { unsigned n=0;if(s)while(s[n])n++;return n; }
static int equal(const osr_char *a,const osr_char *b) {
 if(!a||!b)return 0;
 while(*a&&*b) { if(fold(*a++)!=fold(*b++))return 0; }
 return !*a&&!*b;
}
static int ascii_equal(const char *a,const char *b) {
 if (!a || !b) { return 0; }
 while (*a && *b) {
  if (*a++ != *b++) { return 0; }
 }
 return !*a && !*b;
}
static int separator(unsigned c) { return c=='/'||c=='\\'; }
static int component(const osr_char *path,const osr_char *alias) {
 unsigned i=0,j,n,depth=0,found=0;if(!path||!alias)return 0;n=length(alias);
 while(path[i]) {
  while(separator(path[i]))i++;
  j=0;while(path[i+j]&&!separator(path[i+j]))j++;
  if(j==n) { unsigned k;for(k=0;k<n&&fold(path[i+k])==fold(alias[k]);k++) {}if(k==n)found=++depth; }
  i+=j;depth++;
 }
 return found;
}
static const osr_char *basename(const osr_char *path) {
 const osr_char *p=path;if(!p)return 0;while(*path) { if(separator(*path))p=path+1;path++; }return p;
}
static int suffix(const osr_char *path,const osr_char *alias) {
 unsigned a=length(path),b=length(alias),i;if(!path||a<b)return 0;
 for(i=0;i<b;i++) {
  unsigned x=path[a-b+i],y=alias[i];if(separator(x)&&separator(y))continue;if(fold(x)!=fold(y))return 0;
 }return 1;
}

static int phrase(const osr_char *text,const osr_char *alias) {
 unsigned i=0,j,k;if(!text||!alias||!*alias)return 0;
 for(i=0;text[i];i++) {
  if(i&&word(text[i-1]))continue;
  j=i;k=0;
  while(alias[k]) {
   if(!word(alias[k])) { while(alias[k]&&!word(alias[k]))k++;while(text[j]&&!word(text[j]))j++; }
   else { if(fold(alias[k])!=fold(text[j]))break;j++;k++; }
  }
  if(!alias[k]&&!word(text[j]))return 1;
 }
 return 0;
}
unsigned osr_count(void) { return OSR_SYSTEM_COUNT; }
const OSR_SYSTEM *osr_find(const char *id) {
 unsigned i;for(i=0;i<OSR_SYSTEM_COUNT;i++)if(ascii_equal(OSR_SYSTEMS[i].id,id))return &OSR_SYSTEMS[i];return 0;
}
static int rule_matches(const OSR_RULE *r,const osr_char *path,const osr_char *title,const osr_char *fs,const osr_char *part,const osr_char *id,const osr_char *variant) {
 unsigned n;
 switch(r->kind) {
 case 1:return component(path,r->value);
 case 2:return equal(basename(path),r->value);
 case 3:return phrase(title,r->value)||phrase(fs,r->value)||phrase(part,r->value);
 case 4:return equal(id,r->value);
 case 5:return suffix(path,r->value);
 case 6:
  n=length(r->value);path=basename(path);
  if(!path)return 0;
  { unsigned i;for(i=0;i<n;i++)if(fold(path[i])!=fold(r->value[i]))return 0;return !word(path[n]); }
 case 7:
  if(!id||!variant)return 0;
  n=0;while(r->value[n]&&r->value[n]!=':')n++;
  if(length(id)!=n)return 0;
  { unsigned i;for(i=0;i<n;i++)if(fold(id[i])!=fold(r->value[i]))return 0;return equal(variant,r->value+n+1); }
 }
 return 0;
}
OSR_MATCH osr_identify(const osr_char *path,const osr_char *title,const osr_char *fs,const osr_char *part,const osr_char *id,const osr_char *variant) {
 OSR_MATCH best={0,0,"none"};unsigned i,score;const OSR_RULE *r;
 const char *reason[]={"none","directory","filename","label","os-release","path","kernel-prefix","os-release-variant"};
 for(i=0;i<OSR_RULE_COUNT;i++) {
  r=&OSR_RULES[i];if(!rule_matches(r,path,title,fs,part,id,variant))continue;
  score=r->score;
  if(r->kind==3)score+=length(r->value)<60 ? length(r->value) : 60;
  if(r->kind==1)score+=component(path,r->value);
  if((r->kind==2||r->kind==5)&&(OSR_SYSTEMS[r->system].family==OSR_WINDOWS||OSR_SYSTEMS[r->system].family==OSR_MACOS))score=1400;
  if(score>best.score) { best.system=&OSR_SYSTEMS[r->system];best.score=score;best.evidence=reason[r->kind]; }
 }

 if(id&&*id&&best.system&&best.system->family==OSR_LINUX&&best.score<1200) {
  best.system=osr_find("linux");best.score=1100;best.evidence="unknown-release";
 }

 if(best.system && best.score!=1100) {
  const OSR_SYSTEM *winner=best.system,*chosen=0;unsigned specificity=0;
  for(i=0;i<OSR_RULE_COUNT;i++) {
   const OSR_SYSTEM *candidate,*parent;unsigned steps=0,n;
   r=&OSR_RULES[i];if(r->kind!=1&&r->kind!=3)continue;
   candidate=&OSR_SYSTEMS[r->system];parent=candidate;
   while(parent&&parent->parent&&steps++<OSR_SYSTEM_COUNT) {
    parent=osr_find(parent->parent);if(parent==winner)break;
   }
   if(parent!=winner||candidate==winner||!rule_matches(r,path,title,fs,part,id,variant))continue;
   n=length(r->value)+steps*64;
   if(n>specificity) { chosen=candidate;specificity=n; }
  }
  if(chosen) { best.system=chosen;best.score++;best.evidence="edition-hint"; }
 }
 if(!best.system) { best.system=osr_find("uefi");best.score=1;best.evidence="fallback"; }
 return best;
}
static int key_is(const unsigned char *p,unsigned n,const char *key) {
 unsigned i;for(i=0;i<n&&key[i];i++)if(p[i]!=(unsigned char)key[i])return 0;return i==n&&!key[i];
}
void osr_parse_release(const unsigned char *data,unsigned size,osr_char *id,unsigned id_cap,osr_char *variant,unsigned variant_cap) {
 unsigned at=0,start,end,p,k,n,cap,quote;osr_char *out;
 if (id_cap) { id[0] = 0; }
 if (variant_cap) { variant[0] = 0; }
 while(at<size) {
  start=at;while(at<size&&data[at]!='\n'&&data[at]!='\r')at++;end=at;while(at<size&&(data[at]=='\n'||data[at]=='\r'))at++;
  p=start;while(p<end&&(data[p]==' '||data[p]=='\t'))p++;
  if(p==end||data[p]=='#')continue;
  k=p;while(p<end&&data[p]!='='&&data[p]!=' '&&data[p]!='\t')p++;
  if(key_is(data+k,p-k,"ID")){out=id;cap=id_cap;}
  else if(key_is(data+k,p-k,"VARIANT_ID")){out=variant;cap=variant_cap;}
  else continue;
  while(p<end&&(data[p]==' '||data[p]=='\t'))p++;
  if(p==end||data[p++]!='=')continue;
  while(p<end&&(data[p]==' '||data[p]=='\t'))p++;
  quote=(p<end&&(data[p]=='\''||data[p]=='"')) ? data[p++] : 0;
  k=p;while(p<end&&((data[p]>='a'&&data[p]<='z')||(data[p]>='A'&&data[p]<='Z')||(data[p]>='0'&&data[p]<='9')||data[p]=='_'||data[p]=='-'||data[p]=='.'))p++;
  n=p-k;if(!n||n>=cap)continue;
  if(quote&&(p==end||data[p++]!=quote))continue;
  while(p<end&&(data[p]==' '||data[p]=='\t'))p++;
  if(p<end&&data[p]!='#')continue;
  for (p = 0; p < n; p++) { out[p] = data[k + p]; }
  out[n] = 0;
 }
}

#ifndef OS_REGISTRY_H
#define OS_REGISTRY_H
typedef unsigned short osr_char;
typedef struct {
 const char *id;
 const osr_char *title;
 const osr_char *icon_key;
 const char *parent;
 unsigned family;
} OSR_SYSTEM;
enum { OSR_UNKNOWN=0,OSR_WINDOWS,OSR_MACOS,OSR_LINUX,OSR_BSD,OSR_ANDROID,OSR_CHROMEOS,OSR_BOOT,OSR_OTHER };
typedef struct { const OSR_SYSTEM *system; unsigned score; const char *evidence; } OSR_MATCH;
OSR_MATCH osr_identify(const osr_char *loader,const osr_char *title,const osr_char *fs_label,const osr_char *part_label,const osr_char *release_id,const osr_char *variant_id);
const OSR_SYSTEM *osr_find(const char *id);
void osr_parse_release(const unsigned char *data,unsigned size,osr_char *id,unsigned id_cap,osr_char *variant,unsigned variant_cap);
unsigned osr_count(void);
#endif

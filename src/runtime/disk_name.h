#ifndef DISK_NAME_H
#define DISK_NAME_H

enum { DISK_NAME_ATA = 1, DISK_NAME_SCSI = 2 };
unsigned disk_name_decode(unsigned kind, const unsigned char *data, unsigned size,
                          char *output, unsigned capacity);

unsigned disk_name_nvme_model(const unsigned short *raw, unsigned units,
                              unsigned short *output, unsigned capacity);
#endif

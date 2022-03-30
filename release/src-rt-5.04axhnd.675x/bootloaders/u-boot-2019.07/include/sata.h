#ifndef __SATA_H__
#define __SATA_H__
#include <part.h>

#if !defined(CONFIG_DM_SCSI) && !defined(CONFIG_AHCI)
int init_sata(int dev);
int reset_sata(int dev);
int scan_sata(int dev);
ulong sata_read(int dev, ulong blknr, lbaint_t blkcnt, void *buffer);
ulong sata_write(int dev, ulong blknr, lbaint_t blkcnt, const void *buffer);

int sata_initialize(void);
int __sata_initialize(void);
int sata_stop(void);
int __sata_stop(void);
int sata_port_status(int dev, int port);

extern struct blk_desc sata_dev_desc[];
#endif

int sata_probe(int devnum);
int sata_remove(int devnum);

#endif

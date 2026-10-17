#ifndef __SATA_H__
#define __SATA_H__
#include <part.h>

int sata_probe(int devnum);
int sata_remove(int devnum);

/*
 * Remove existing AHCI SATA device uclass and all of its children,
 * if any, and probe it again.
 */
int sata_rescan(bool verbose);

#endif

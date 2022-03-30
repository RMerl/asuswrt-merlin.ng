/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * BTRFS filesystem implementation for U-Boot
 *
 * 2017 Marek Behun, CZ.NIC, marek.behun@nic.cz
 */

#ifndef __U_BOOT_BTRFS_H__
#define __U_BOOT_BTRFS_H__

int btrfs_probe(struct blk_desc *, disk_partition_t *);
int btrfs_ls(const char *);
int btrfs_exists(const char *);
int btrfs_size(const char *, loff_t *);
int btrfs_read(const char *, void *, loff_t, loff_t, loff_t *);
void btrfs_close(void);
int btrfs_uuid(char *);
void btrfs_list_subvols(void);

#endif /* __U_BOOT_BTRFS_H__ */

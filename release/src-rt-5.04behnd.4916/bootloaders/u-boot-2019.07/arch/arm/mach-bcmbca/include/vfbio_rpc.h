/* SPDX-License-Identifier: GPL-2.0+
 *
 *  Copyright 2019 Broadcom Ltd.
 */

#ifndef __VFBIO_RPC_H__
#define __VFBIO_RPC_H__

#define VFBIO_LUN_INFO_NAME_MAX		16
#define VFBIO_LUN_MAX			0xff

struct vfbio_lun_info {
	char name[VFBIO_LUN_INFO_NAME_MAX];
	uint32_t blk_sz;
	uint32_t n_blks;
	int read_only;
	int dynamic;
};

/* lun id-name pair */
struct vfbio_lun_id_name
{
    int id;
    char name[VFBIO_LUN_INFO_NAME_MAX];
};

int vfbio_rpc_lun_info(int lun, struct vfbio_lun_info *info);
int vfbio_rpc_read(int lun, ulong addr, uint32_t blk, uint32_t blk_sz, uint32_t *n_blks);
int vfbio_rpc_write(int lun, ulong addr, uint32_t blk, uint32_t blk_sz, uint32_t *n_blks);
int vfbio_rpc_finish_first_boot(void);
int vfbio_rpc_lun_create( int lun, const char *name, uint32_t size);
int vfbio_rpc_lun_delete(int lun);
int vfbio_rpc_lun_resize(int lun, uint32_t size);
int vfbio_rpc_lun_rename(uint8_t num_luns, struct vfbio_lun_id_name id_name[]);
int vfbio_rpc_device_get_info(uint64_t *total_size, uint64_t *free_size);

#endif
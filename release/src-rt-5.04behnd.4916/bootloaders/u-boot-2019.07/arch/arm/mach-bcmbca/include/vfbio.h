/* SPDX-License-Identifier: GPL-2.0+
 *
 *  Copyright 2019 Broadcom Ltd.
 */
#ifndef _VFBIO_H_
#define _VFBIO_H_

#include "vfbio_rpc.h"

/************************************************************************
 * vfbio_init - call to initialize vfbio module
 ***********************************************************************/
void vfbio_init(void);

/************************************************************************
 * vfbio_create_lun - call to create LUN by name, by ID, or by both
 * 	id - if -1 will be generated automatically and returned here
 * 	name - if NULL will be unnamed
 * 	size - LUN size in bytes (alligned to block size if needed)
 * 	Returns 0 if succeeded or -1 if error
 ***********************************************************************/
int vfbio_lun_create(char *name, unsigned int size, int *id);

/************************************************************************
 * vfbio_delete_lun - call to create LUN by name, by ID, or by both
 * 	id - LUN ID to delete
 * 	Returns 0 if succeeded or -1 if error
 ***********************************************************************/
int vfbio_lun_delete(int id);

/************************************************************************
 * vfbio_get_next_lun - enumerates LUNs
 * 	prev_id - use -1 to get first LUN, and previous LUN ID to get next
 * 	id(OUT) - next LUN ID
 * 	Returns 0 if succeeded or negative if error
 ***********************************************************************/
int vfbio_lun_get_next(int prev_id, int *id);

/************************************************************************
 * vfbio_get_lun_id - get LUN ID by name
 * 	name - LUN name
 * 	id(OUT) - LUN ID
 * 	Returns 0 if succeeded or negative if error
 ***********************************************************************/
int vfbio_lun_get_id(char *name, int *id);

/************************************************************************
 * vfbio_get_lun_name - get LUN name by ID
 * 	id - LUN ID
 * 	Returns LUN name or NULL if error
 ***********************************************************************/
const char *vfbio_lun_get_name(int id);

/************************************************************************
 * vfbio_get_lun_size - get LUN size by ID
 * 	id - LUN ID
 * 	size(OUT) - LUN size in bytes
 * 	Returns 0 if succeeded or -1 if error
 ***********************************************************************/
int vfbio_lun_get_size(int id, unsigned int *size);

/************************************************************************
 * vfbio_get_lun_blk_size - get LUN block size by ID
 * 	id - LUN ID
 * 	size(OUT) - LUN block size in bytes
 * 	Returns 0 if succeeded or -1 if error
 ***********************************************************************/
int vfbio_lun_get_blk_size(int id, unsigned int *size);

/************************************************************************
 * vfbio_get_lun_blk_num - get LUN blocks number by ID
 * 	id - LUN ID
 * 	num(OUT) - LUN blocks number
 * 	Returns 0 if succeeded or -1 if error
 ***********************************************************************/
int vfbio_lun_get_blk_num(int id, unsigned int *num);

/************************************************************************
 * vfbio_resize_lun - resize LUN by ID
 * 	id - LUN ID
 * 	size - new LUN size in bytes (alligned to block size if needed) 
 * 	Returns 0 if succeeded or -1 if error,
 * 	 -2 if inconsistent (could be corrected by reset)
 ***********************************************************************/
int vfbio_lun_resize(int id, unsigned int size);

/************************************************************************
 * vfbio_rename_lun - atomically renames a number of LUNS
 * 	num_luns - number of LUNS to rename
 * 	id_name - LUN ID and its new name pairs array of num_s size
 * 	Returns 0 if succeeded or -1 if error,
 * 	 -2 if inconsistent (could be corrected by reset)
 ***********************************************************************/
int vfbio_lun_rename(uint8_t num_luns, struct vfbio_lun_id_name id_name[]);

/************************************************************************
 * vfbio_read_lun - read LUN by ID
 * 	id - LUN ID
 * 	blk - block to start from
 * 	cnt - number of blocks to read, if UINT_MAX then read up to 
 * 	      the end of LUN
 * 	buffer - points to target buffer
 * 	Returns 0 if succeeded or -1 if error
 ***********************************************************************/
int vfbio_lun_read(int id, unsigned int blk, unsigned int cnt, void *buffer);

/************************************************************************
 * vfbio_write_lun - write LUN by ID
 * 	id - LUN ID
 * 	blk - block to start from
 * 	cnt - number of blocks to wri
 * 	buffer - points to source buffer
 * 	Returns 0 if succeeded or -1 if error
 ***********************************************************************/
int vfbio_lun_write(int id, unsigned int blk, unsigned int cnt, void *buffer);

/************************************************************************
 * vfbio_device_get_info - get flash total and free sizes
 * 	total_size(OUT) - flash total size in bytes
 * 	free_size(OUT) - flash free size in bytes
 * 	Returns 0 if succeeded or -1 if error
 ***********************************************************************/
int vfbio_device_get_info(uint64_t *total_size, uint64_t *free_size);

/************************************************************************
 * vfbio_finish_first_boot - reinit vfbio and turn on LVM support
 * 	Returns 0 if succeeded or -1 if error
 ***********************************************************************/
int vfbio_finish_first_boot(void);

#endif
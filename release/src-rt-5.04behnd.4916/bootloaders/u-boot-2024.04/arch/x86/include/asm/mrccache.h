/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * Copyright (C) 2014 Google, Inc
 * Copyright (C) 2015 Bin Meng <bmeng.cn@gmail.com>
 */

#ifndef _ASM_MRCCACHE_H
#define _ASM_MRCCACHE_H

#include <compiler.h>

#define MRC_DATA_ALIGN		0x100
#define MRC_DATA_SIGNATURE	(('M' << 0) | ('R' << 8) | \
				 ('C' << 16) | ('D'<<24))

#define MRC_DATA_HEADER_SIZE	32

struct __packed mrc_data_container {
	u32	signature;	/* "MRCD" */
	u32	data_size;	/* Size of the 'data' field */
	u32	checksum;	/* IP style checksum */
	u32	reserved;	/* For header alignment */
	u8	data[0];	/* Variable size, platform/run time dependent */
};

struct mrc_region {
	u32	base;
	u32	offset;
	u32	length;
};

/* Types of MRC data */
enum mrc_type_t {
	MRC_TYPE_NORMAL,
	MRC_TYPE_VAR,

	MRC_TYPE_COUNT,
};

struct udevice;

/**
 * mrccache_find_current() - find the latest MRC cache record
 *
 * This searches the MRC cache region looking for the latest record to use
 * for setting up SDRAM
 *
 * @entry:	Position and size of MRC cache in SPI flash
 * Return: pointer to latest record, or NULL if none
 */
struct mrc_data_container *mrccache_find_current(struct mrc_region *entry);

/**
 * mrccache_reserve() - reserve MRC data on the stack
 *
 * This copies MRC data pointed by gd->arch.mrc_output to a new place on the
 * stack with length gd->arch.mrc_output_len, and updates gd->arch.mrc_output
 * to point to the new place once the migration is done.
 *
 * This routine should be called by reserve_arch() before U-Boot is relocated
 * when MRC cache is enabled.
 *
 * Return: 0 always
 */
int mrccache_reserve(void);

/**
 * mrccache_get_region() - get MRC region on the SPI flash
 *
 * This gets MRC region whose offset and size are described in the device tree
 * as a subnode to the SPI flash. This tries to find the SPI flash device
 * (without probing it), falling back to looking for the devicetree node if
 * driver model is not inited or the SPI flash is not found.
 *
 * @type:	Type of MRC data to use
 * @devp:	Returns pointer to the SPI flash device, if found
 * @entry:	Position and size of MRC cache in SPI flash
 * Return: 0 if success, -ENOENT if SPI flash node does not exist in the
 * device tree, -EPERM if MRC region subnode does not exist in the device
 * tree, -EINVAL if MRC region properties format is incorrect, other error
 * if SPI flash probe failed.
 */
int mrccache_get_region(enum mrc_type_t type, struct udevice **devp,
			struct mrc_region *entry);

/**
 * mrccache_save() - save MRC data to the SPI flash
 *
 * This saves MRC data stored previously by gd->arch.mrc_output to a proper
 * place within the MRC region on the SPI flash.
 *
 * Return: 0 if saved to SPI flash successfully, other error if failed
 */
int mrccache_save(void);

/**
 * mrccache_spl_save() - Save to the MRC region from SPL
 *
 * When SPL is used to set up the memory controller we want to save the MRC
 * data in SPL to avoid needing to pass it up to U-Boot proper to save. This
 * function handles that.
 *
 * Return: 0 if saved to SPI flash successfully, other error if failed
 */
int mrccache_spl_save(void);

#endif /* _ASM_MRCCACHE_H */

/*
 *  linux/include/linux/mtd/onenand.h
 *
 *  Copyright (C) 2005-2007 Samsung Electronics
 *  Kyungmin Park <kyungmin.park@samsung.com>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 */

#ifndef __LINUX_MTD_ONENAND_H
#define __LINUX_MTD_ONENAND_H

#include <linux/mtd/onenand_regs.h>

/* Note: The header order is impoertant */
#include <onenand_uboot.h>

#include <linux/compat.h>
#include <linux/mtd/bbm.h>

#define MAX_DIES		2
#define MAX_BUFFERRAM		2
#define MAX_ONENAND_PAGESIZE	(4096 + 128)

/* Scan and identify a OneNAND device */
extern int onenand_scan (struct mtd_info *mtd, int max_chips);
/* Free resources held by the OneNAND device */
extern void onenand_release (struct mtd_info *mtd);

/**
 * struct onenand_bufferram - OneNAND BufferRAM Data
 * @param blockpage	block & page address in BufferRAM
 */
struct onenand_bufferram {
	int blockpage;
};

/**
 * struct onenand_chip - OneNAND Private Flash Chip Data
 * @param base		[BOARDSPECIFIC] address to access OneNAND
 * @dies:               [INTERN][FLEXONENAND] number of dies on chip
 * @boundary:           [INTERN][FLEXONENAND] Boundary of the dies
 * @diesize:            [INTERN][FLEXONENAND] Size of the dies
 * @param chipsize	[INTERN] the size of one chip for multichip arrays
 * @param device_id	[INTERN] device ID
 * @param verstion_id	[INTERN] version ID
 * @technology		[INTERN] describes the internal NAND array technology such as SLC or MLC.
 * @density_mask:	[INTERN] chip density, used for DDP devices
 * @param options	[BOARDSPECIFIC] various chip options. They can partly be set to inform onenand_scan about
 * @param erase_shift	[INTERN] number of address bits in a block
 * @param page_shift	[INTERN] number of address bits in a page
 * @param ppb_shift	[INTERN] number of address bits in a pages per block
 * @param page_mask	[INTERN] a page per block mask
 * @param writesize	[INTERN] a real page size
 * @param bufferam_index	[INTERN] BufferRAM index
 * @param bufferam	[INTERN] BufferRAM info
 * @param readw		[REPLACEABLE] hardware specific function for read short
 * @param writew	[REPLACEABLE] hardware specific function for write short
 * @param command	[REPLACEABLE] hardware specific function for writing commands to the chip
 * @param wait		[REPLACEABLE] hardware specific function for wait on ready
 * @param read_bufferram	[REPLACEABLE] hardware specific function for BufferRAM Area
 * @param write_bufferram	[REPLACEABLE] hardware specific function for BufferRAM Area
 * @param chip_lock	[INTERN] spinlock used to protect access to this structure and the chip
 * @param wq		[INTERN] wait queue to sleep on if a OneNAND operation is in progress
 * @param state		[INTERN] the current state of the OneNAND device
 * @param autooob	[REPLACEABLE] the default (auto)placement scheme
 * @param priv		[OPTIONAL] pointer to private chip date
 */
struct onenand_chip {
	void __iomem *base;
	unsigned int dies;
	unsigned int boundary[MAX_DIES];
	unsigned int diesize[MAX_DIES];
	unsigned int chipsize;
	unsigned int device_id;
	unsigned int version_id;
	unsigned int technology;
	unsigned int density_mask;
	unsigned int options;

	unsigned int erase_shift;
	unsigned int page_shift;
	unsigned int ppb_shift;	/* Pages per block shift */
	unsigned int page_mask;
	unsigned int writesize;

	unsigned int bufferram_index;
	struct onenand_bufferram bufferram[MAX_BUFFERRAM];

	int (*command) (struct mtd_info *mtd, int cmd, loff_t address,
			size_t len);
	int (*wait) (struct mtd_info *mtd, int state);
	int (*bbt_wait) (struct mtd_info *mtd, int state);
	void (*unlock_all)(struct mtd_info *mtd);
	int (*read_bufferram) (struct mtd_info *mtd, loff_t addr, int area,
			       unsigned char *buffer, int offset, size_t count);
	int (*write_bufferram) (struct mtd_info *mtd, loff_t addr, int area,
				const unsigned char *buffer, int offset,
				size_t count);
	unsigned short (*read_word) (void __iomem *addr);
	void (*write_word) (unsigned short value, void __iomem *addr);
	int (*chip_probe)(struct mtd_info *mtd);
	void (*mmcontrol) (struct mtd_info *mtd, int sync_read);
	int (*block_markbad)(struct mtd_info *mtd, loff_t ofs);
	int (*scan_bbt)(struct mtd_info *mtd);

	unsigned char		*main_buf;
	unsigned char		*spare_buf;
#ifdef DONT_USE_UBOOT
	spinlock_t chip_lock;
	wait_queue_head_t wq;
#endif
	int state;
	unsigned char		*page_buf;
	unsigned char		*oob_buf;

	struct nand_oobinfo *autooob;
	int			subpagesize;
	struct nand_ecclayout	*ecclayout;

	void *bbm;

	void *priv;
};

/*
 * Helper macros
 */
#define ONENAND_CURRENT_BUFFERRAM(this)		(this->bufferram_index)
#define ONENAND_NEXT_BUFFERRAM(this)		(this->bufferram_index ^ 1)
#define ONENAND_SET_NEXT_BUFFERRAM(this)	(this->bufferram_index ^= 1)
#define ONENAND_SET_PREV_BUFFERRAM(this)	(this->bufferram_index ^= 1)
#define ONENAND_SET_BUFFERRAM0(this)		(this->bufferram_index = 0)
#define ONENAND_SET_BUFFERRAM1(this)		(this->bufferram_index = 1)

#define FLEXONENAND(this)	(this->device_id & DEVICE_IS_FLEXONENAND)
#define ONENAND_IS_MLC(this)	(this->technology & ONENAND_TECHNOLOGY_IS_MLC)
#define ONENAND_IS_DDP(this)						\
	(this->device_id & ONENAND_DEVICE_IS_DDP)

#define ONENAND_IS_4KB_PAGE(this)					\
	(this->options & ONENAND_HAS_4KB_PAGE)

#define ONENAND_IS_2PLANE(this)			(0)

/*
 * Options bits
 */
#define ONENAND_HAS_CONT_LOCK		(0x0001)
#define ONENAND_HAS_UNLOCK_ALL		(0x0002)
#define ONENAND_HAS_2PLANE		(0x0004)
#define ONENAND_HAS_4KB_PAGE            (0x0008)
#define ONENAND_RUNTIME_BADBLOCK_CHECK	(0x0200)
#define ONENAND_PAGEBUF_ALLOC		(0x1000)
#define ONENAND_OOBBUF_ALLOC		(0x2000)

/*
 * OneNAND Flash Manufacturer ID Codes
 */
#define ONENAND_MFR_NUMONYX	0x20
#define ONENAND_MFR_SAMSUNG	0xec

/**
 * struct nand_manufacturers - NAND Flash Manufacturer ID Structure
 * @param name:		Manufacturer name
 * @param id:		manufacturer ID code of device.
*/
struct onenand_manufacturers {
	int id;
	char *name;
};

int onenand_bbt_read_oob(struct mtd_info *mtd, loff_t from,
			struct mtd_oob_ops *ops);

unsigned int onenand_block(struct onenand_chip *this, loff_t addr);
int flexonenand_region(struct mtd_info *mtd, loff_t addr);
#endif				/* __LINUX_MTD_ONENAND_H */

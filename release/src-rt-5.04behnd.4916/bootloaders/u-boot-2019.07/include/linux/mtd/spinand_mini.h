/* SPDX-License-Identifier: GPL-2.0+
 *
 *  Copyright 2020 Broadcom Ltd.
 */

#ifndef _SPINAND_MINI_H
#define _SPINAND_MINI_H

#include <spi.h>
#include <spi-mem.h>
#include <linux/mtd/spinand.h>

#define SPINAND_MFR_WINBOND		0xEF
#define SPINAND_MFR_GIGADEVICE_ESMT	0xC8
#define SPINAND_MFR_MACRONIX		0xC2
#define SPINAND_MFR_TOSHIBA		0x98
#define SPINAND_MFR_MICRON		0x2C
#define SPINAND_MFR_PARAGON		0xA1
#define SPINAND_MFR_ETRON		0xD5


#define SPINAND_READID_OP_EXT(naddr, ndummy, buf, len)		\
	SPI_MEM_OP(SPI_MEM_OP_CMD(0x9f, 1),			\
		   SPI_MEM_OP_ADDR(naddr, 0, 1),		\
		   SPI_MEM_OP_DUMMY(ndummy, 1),			\
		   SPI_MEM_OP_DATA_IN(len, buf, 1))


#define SPINAND_READID_OP_ONLY(buf, len)			\
	SPINAND_READID_OP_EXT(0, 0, buf, len)

#define SPINAND_READID_OP_ADDR(buf, len)			\
	SPINAND_READID_OP_EXT(1, 0, buf, len)

#define SPINAND_READID_OP_DUMMY(buf, len)			\
	SPINAND_READID_OP_EXT(0, 1, buf, len)

#pragma pack(push,1)
struct spinandmini_mem_org {
	u8 bits_per_cell;
	u16 pagesize;
	u16 pages_per_eraseblock;
	u16 eraseblocks_per_lun;
	u8 planes_per_lun;
	u8 luns_per_target;
	u8 ntargets;
};

#define SPINANDMINI_MEMORG(bpc, ps, os, ppe, epl, ppl, lpt, nt)	\
	{							\
		.bits_per_cell = (bpc),				\
		.pagesize = (ps),				\
		.pages_per_eraseblock = (ppe),			\
		.eraseblocks_per_lun = (epl),			\
		.planes_per_lun = (ppl),			\
		.luns_per_target = (lpt),			\
		.ntargets = (nt),				\
	}

struct spinandmini_info {
	u8 id[SPINAND_MAX_ID_LEN];
	u8 id_len;
	struct spinandmini_mem_org memorg;
};
#pragma pack(pop)

struct spinandmini_device {
	struct spi_slave *slave;  
	struct spinand_id id;
	const struct spinandmini_mem_org* memorg;
	u32 page_shift;
	u32 block_shift;
  	u64 target_shift; /* die shift */
	u32 block_size;
	u32 page_size;
	u64 total_size;
	int (*select_target)(struct spinandmini_device *spinand,
			     unsigned int target);
	unsigned int cur_target;
	u8 *cfg_cache;
};

void spinandmini_init(void);
u32 spinandmini_get_block_size(void);
u32 spinandmini_get_page_size(void);
u64 spinandmini_get_total_size(void);
int spinandmini_read_buf(int block, int offset, u8 *dst, u32 len);
int spinandmini_is_bad_block(int block);

#endif

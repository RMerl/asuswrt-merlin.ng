/*
 *  linux/drivers/mtd/onenand/onenand_base.c
 *
 *  Copyright (C) 2005-2007 Samsung Electronics
 *  Kyungmin Park <kyungmin.park@samsung.com>
 *
 *  Credits:
 *      Adrian Hunter <ext-adrian.hunter@nokia.com>:
 *      auto-placement support, read-while load support, various fixes
 *      Copyright (C) Nokia Corporation, 2007
 *
 *      Rohit Hagargundgi <h.rohit at samsung.com>,
 *      Amul Kumar Saha <amul.saha@samsung.com>:
 *      Flex-OneNAND support
 *      Copyright (C) Samsung Electronics, 2009
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 */

#include <common.h>
#include <watchdog.h>
#include <linux/compat.h>
#include <linux/mtd/mtd.h>
#include "linux/mtd/flashchip.h"
#include <linux/mtd/onenand.h>

#include <asm/io.h>
#include <linux/errno.h>
#include <malloc.h>

/* It should access 16-bit instead of 8-bit */
static void *memcpy_16(void *dst, const void *src, unsigned int len)
{
	void *ret = dst;
	short *d = dst;
	const short *s = src;

	len >>= 1;
	while (len-- > 0)
		*d++ = *s++;
	return ret;
}

/**
 *  onenand_oob_128 - oob info for Flex-Onenand with 4KB page
 *  For now, we expose only 64 out of 80 ecc bytes
 */
static struct nand_ecclayout onenand_oob_128 = {
	.eccbytes	= 64,
	.eccpos		= {
		6, 7, 8, 9, 10, 11, 12, 13, 14, 15,
		22, 23, 24, 25, 26, 27, 28, 29, 30, 31,
		38, 39, 40, 41, 42, 43, 44, 45, 46, 47,
		54, 55, 56, 57, 58, 59, 60, 61, 62, 63,
		70, 71, 72, 73, 74, 75, 76, 77, 78, 79,
		86, 87, 88, 89, 90, 91, 92, 93, 94, 95,
		102, 103, 104, 105
		},
	.oobfree	= {
		{2, 4}, {18, 4}, {34, 4}, {50, 4},
		{66, 4}, {82, 4}, {98, 4}, {114, 4}
	}
};

/**
 * onenand_oob_64 - oob info for large (2KB) page
 */
static struct nand_ecclayout onenand_oob_64 = {
	.eccbytes	= 20,
	.eccpos		= {
		8, 9, 10, 11, 12,
		24, 25, 26, 27, 28,
		40, 41, 42, 43, 44,
		56, 57, 58, 59, 60,
		},
	.oobfree	= {
		{2, 3}, {14, 2}, {18, 3}, {30, 2},
		{34, 3}, {46, 2}, {50, 3}, {62, 2}
	}
};

/**
 * onenand_oob_32 - oob info for middle (1KB) page
 */
static struct nand_ecclayout onenand_oob_32 = {
	.eccbytes	= 10,
	.eccpos		= {
		8, 9, 10, 11, 12,
		24, 25, 26, 27, 28,
		},
	.oobfree	= { {2, 3}, {14, 2}, {18, 3}, {30, 2} }
};

/*
 * Warning! This array is used with the memcpy_16() function, thus
 * it must be aligned to 2 bytes. GCC can make this array unaligned
 * as the array is made of unsigned char, which memcpy16() doesn't
 * like and will cause unaligned access.
 */
static const unsigned char __aligned(2) ffchars[] = {
	0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
	0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,	/* 16 */
	0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
	0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,	/* 32 */
	0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
	0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,	/* 48 */
	0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
	0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,	/* 64 */
	0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
	0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,	/* 80 */
	0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
	0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,	/* 96 */
	0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
	0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,	/* 112 */
	0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
	0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,	/* 128 */
};

/**
 * onenand_readw - [OneNAND Interface] Read OneNAND register
 * @param addr		address to read
 *
 * Read OneNAND register
 */
static unsigned short onenand_readw(void __iomem * addr)
{
	return readw(addr);
}

/**
 * onenand_writew - [OneNAND Interface] Write OneNAND register with value
 * @param value		value to write
 * @param addr		address to write
 *
 * Write OneNAND register with value
 */
static void onenand_writew(unsigned short value, void __iomem * addr)
{
	writew(value, addr);
}

/**
 * onenand_block_address - [DEFAULT] Get block address
 * @param device	the device id
 * @param block		the block
 * @return		translated block address if DDP, otherwise same
 *
 * Setup Start Address 1 Register (F100h)
 */
static int onenand_block_address(struct onenand_chip *this, int block)
{
	/* Device Flash Core select, NAND Flash Block Address */
	if (block & this->density_mask)
		return ONENAND_DDP_CHIP1 | (block ^ this->density_mask);

	return block;
}

/**
 * onenand_bufferram_address - [DEFAULT] Get bufferram address
 * @param device	the device id
 * @param block		the block
 * @return		set DBS value if DDP, otherwise 0
 *
 * Setup Start Address 2 Register (F101h) for DDP
 */
static int onenand_bufferram_address(struct onenand_chip *this, int block)
{
	/* Device BufferRAM Select */
	if (block & this->density_mask)
		return ONENAND_DDP_CHIP1;

	return ONENAND_DDP_CHIP0;
}

/**
 * onenand_page_address - [DEFAULT] Get page address
 * @param page		the page address
 * @param sector	the sector address
 * @return		combined page and sector address
 *
 * Setup Start Address 8 Register (F107h)
 */
static int onenand_page_address(int page, int sector)
{
	/* Flash Page Address, Flash Sector Address */
	int fpa, fsa;

	fpa = page & ONENAND_FPA_MASK;
	fsa = sector & ONENAND_FSA_MASK;

	return ((fpa << ONENAND_FPA_SHIFT) | fsa);
}

/**
 * onenand_buffer_address - [DEFAULT] Get buffer address
 * @param dataram1	DataRAM index
 * @param sectors	the sector address
 * @param count		the number of sectors
 * @return		the start buffer value
 *
 * Setup Start Buffer Register (F200h)
 */
static int onenand_buffer_address(int dataram1, int sectors, int count)
{
	int bsa, bsc;

	/* BufferRAM Sector Address */
	bsa = sectors & ONENAND_BSA_MASK;

	if (dataram1)
		bsa |= ONENAND_BSA_DATARAM1;	/* DataRAM1 */
	else
		bsa |= ONENAND_BSA_DATARAM0;	/* DataRAM0 */

	/* BufferRAM Sector Count */
	bsc = count & ONENAND_BSC_MASK;

	return ((bsa << ONENAND_BSA_SHIFT) | bsc);
}

/**
 * flexonenand_block - Return block number for flash address
 * @param this		- OneNAND device structure
 * @param addr		- Address for which block number is needed
 */
static unsigned int flexonenand_block(struct onenand_chip *this, loff_t addr)
{
	unsigned int boundary, blk, die = 0;

	if (ONENAND_IS_DDP(this) && addr >= this->diesize[0]) {
		die = 1;
		addr -= this->diesize[0];
	}

	boundary = this->boundary[die];

	blk = addr >> (this->erase_shift - 1);
	if (blk > boundary)
		blk = (blk + boundary + 1) >> 1;

	blk += die ? this->density_mask : 0;
	return blk;
}

unsigned int onenand_block(struct onenand_chip *this, loff_t addr)
{
	if (!FLEXONENAND(this))
		return addr >> this->erase_shift;
	return flexonenand_block(this, addr);
}

/**
 * flexonenand_addr - Return address of the block
 * @this:		OneNAND device structure
 * @block:		Block number on Flex-OneNAND
 *
 * Return address of the block
 */
static loff_t flexonenand_addr(struct onenand_chip *this, int block)
{
	loff_t ofs = 0;
	int die = 0, boundary;

	if (ONENAND_IS_DDP(this) && block >= this->density_mask) {
		block -= this->density_mask;
		die = 1;
		ofs = this->diesize[0];
	}

	boundary = this->boundary[die];
	ofs += (loff_t) block << (this->erase_shift - 1);
	if (block > (boundary + 1))
		ofs += (loff_t) (block - boundary - 1)
			<< (this->erase_shift - 1);
	return ofs;
}

loff_t onenand_addr(struct onenand_chip *this, int block)
{
	if (!FLEXONENAND(this))
		return (loff_t) block << this->erase_shift;
	return flexonenand_addr(this, block);
}

/**
 * flexonenand_region - [Flex-OneNAND] Return erase region of addr
 * @param mtd		MTD device structure
 * @param addr		address whose erase region needs to be identified
 */
int flexonenand_region(struct mtd_info *mtd, loff_t addr)
{
	int i;

	for (i = 0; i < mtd->numeraseregions; i++)
		if (addr < mtd->eraseregions[i].offset)
			break;
	return i - 1;
}

/**
 * onenand_get_density - [DEFAULT] Get OneNAND density
 * @param dev_id        OneNAND device ID
 *
 * Get OneNAND density from device ID
 */
static inline int onenand_get_density(int dev_id)
{
	int density = dev_id >> ONENAND_DEVICE_DENSITY_SHIFT;
	return (density & ONENAND_DEVICE_DENSITY_MASK);
}

/**
 * onenand_command - [DEFAULT] Send command to OneNAND device
 * @param mtd		MTD device structure
 * @param cmd		the command to be sent
 * @param addr		offset to read from or write to
 * @param len		number of bytes to read or write
 *
 * Send command to OneNAND device. This function is used for middle/large page
 * devices (1KB/2KB Bytes per page)
 */
static int onenand_command(struct mtd_info *mtd, int cmd, loff_t addr,
			   size_t len)
{
	struct onenand_chip *this = mtd->priv;
	int value;
	int block, page;

	/* Now we use page size operation */
	int sectors = 0, count = 0;

	/* Address translation */
	switch (cmd) {
	case ONENAND_CMD_UNLOCK:
	case ONENAND_CMD_LOCK:
	case ONENAND_CMD_LOCK_TIGHT:
	case ONENAND_CMD_UNLOCK_ALL:
		block = -1;
		page = -1;
		break;

	case FLEXONENAND_CMD_PI_ACCESS:
		/* addr contains die index */
		block = addr * this->density_mask;
		page = -1;
		break;

	case ONENAND_CMD_ERASE:
	case ONENAND_CMD_BUFFERRAM:
		block = onenand_block(this, addr);
		page = -1;
		break;

	case FLEXONENAND_CMD_READ_PI:
		cmd = ONENAND_CMD_READ;
		block = addr * this->density_mask;
		page = 0;
		break;

	default:
		block = onenand_block(this, addr);
		page = (int) (addr
			- onenand_addr(this, block)) >> this->page_shift;
		page &= this->page_mask;
		break;
	}

	/* NOTE: The setting order of the registers is very important! */
	if (cmd == ONENAND_CMD_BUFFERRAM) {
		/* Select DataRAM for DDP */
		value = onenand_bufferram_address(this, block);
		this->write_word(value,
				 this->base + ONENAND_REG_START_ADDRESS2);

		if (ONENAND_IS_4KB_PAGE(this))
			ONENAND_SET_BUFFERRAM0(this);
		else
			/* Switch to the next data buffer */
			ONENAND_SET_NEXT_BUFFERRAM(this);

		return 0;
	}

	if (block != -1) {
		/* Write 'DFS, FBA' of Flash */
		value = onenand_block_address(this, block);
		this->write_word(value,
				 this->base + ONENAND_REG_START_ADDRESS1);

		/* Select DataRAM for DDP */
		value = onenand_bufferram_address(this, block);
		this->write_word(value,
				 this->base + ONENAND_REG_START_ADDRESS2);
	}

	if (page != -1) {
		int dataram;

		switch (cmd) {
		case FLEXONENAND_CMD_RECOVER_LSB:
		case ONENAND_CMD_READ:
		case ONENAND_CMD_READOOB:
			if (ONENAND_IS_4KB_PAGE(this))
				dataram = ONENAND_SET_BUFFERRAM0(this);
			else
				dataram = ONENAND_SET_NEXT_BUFFERRAM(this);

			break;

		default:
			dataram = ONENAND_CURRENT_BUFFERRAM(this);
			break;
		}

		/* Write 'FPA, FSA' of Flash */
		value = onenand_page_address(page, sectors);
		this->write_word(value,
				 this->base + ONENAND_REG_START_ADDRESS8);

		/* Write 'BSA, BSC' of DataRAM */
		value = onenand_buffer_address(dataram, sectors, count);
		this->write_word(value, this->base + ONENAND_REG_START_BUFFER);
	}

	/* Interrupt clear */
	this->write_word(ONENAND_INT_CLEAR, this->base + ONENAND_REG_INTERRUPT);
	/* Write command */
	this->write_word(cmd, this->base + ONENAND_REG_COMMAND);

	return 0;
}

/**
 * onenand_read_ecc - return ecc status
 * @param this		onenand chip structure
 */
static int onenand_read_ecc(struct onenand_chip *this)
{
	int ecc, i;

	if (!FLEXONENAND(this))
		return this->read_word(this->base + ONENAND_REG_ECC_STATUS);

	for (i = 0; i < 4; i++) {
		ecc = this->read_word(this->base
				+ ((ONENAND_REG_ECC_STATUS + i) << 1));
		if (likely(!ecc))
			continue;
		if (ecc & FLEXONENAND_UNCORRECTABLE_ERROR)
			return ONENAND_ECC_2BIT_ALL;
	}

	return 0;
}

/**
 * onenand_wait - [DEFAULT] wait until the command is done
 * @param mtd		MTD device structure
 * @param state		state to select the max. timeout value
 *
 * Wait for command done. This applies to all OneNAND command
 * Read can take up to 30us, erase up to 2ms and program up to 350us
 * according to general OneNAND specs
 */
static int onenand_wait(struct mtd_info *mtd, int state)
{
	struct onenand_chip *this = mtd->priv;
	unsigned int interrupt = 0;
	unsigned int ctrl;

	/* Wait at most 20ms ... */
	u32 timeo = (CONFIG_SYS_HZ * 20) / 1000;
	u32 time_start = get_timer(0);
	do {
		WATCHDOG_RESET();
		if (get_timer(time_start) > timeo)
			return -EIO;
		interrupt = this->read_word(this->base + ONENAND_REG_INTERRUPT);
	} while ((interrupt & ONENAND_INT_MASTER) == 0);

	ctrl = this->read_word(this->base + ONENAND_REG_CTRL_STATUS);

	if (interrupt & ONENAND_INT_READ) {
		int ecc = onenand_read_ecc(this);
		if (ecc & ONENAND_ECC_2BIT_ALL) {
			printk("onenand_wait: ECC error = 0x%04x\n", ecc);
			return -EBADMSG;
		}
	}

	if (ctrl & ONENAND_CTRL_ERROR) {
		printk("onenand_wait: controller error = 0x%04x\n", ctrl);
		if (ctrl & ONENAND_CTRL_LOCK)
			printk("onenand_wait: it's locked error = 0x%04x\n",
				ctrl);

		return -EIO;
	}


	return 0;
}

/**
 * onenand_bufferram_offset - [DEFAULT] BufferRAM offset
 * @param mtd		MTD data structure
 * @param area		BufferRAM area
 * @return		offset given area
 *
 * Return BufferRAM offset given area
 */
static inline int onenand_bufferram_offset(struct mtd_info *mtd, int area)
{
	struct onenand_chip *this = mtd->priv;

	if (ONENAND_CURRENT_BUFFERRAM(this)) {
		if (area == ONENAND_DATARAM)
			return mtd->writesize;
		if (area == ONENAND_SPARERAM)
			return mtd->oobsize;
	}

	return 0;
}

/**
 * onenand_read_bufferram - [OneNAND Interface] Read the bufferram area
 * @param mtd		MTD data structure
 * @param area		BufferRAM area
 * @param buffer	the databuffer to put/get data
 * @param offset	offset to read from or write to
 * @param count		number of bytes to read/write
 *
 * Read the BufferRAM area
 */
static int onenand_read_bufferram(struct mtd_info *mtd, loff_t addr, int area,
				  unsigned char *buffer, int offset,
				  size_t count)
{
	struct onenand_chip *this = mtd->priv;
	void __iomem *bufferram;

	bufferram = this->base + area;
	bufferram += onenand_bufferram_offset(mtd, area);

	memcpy_16(buffer, bufferram + offset, count);

	return 0;
}

/**
 * onenand_sync_read_bufferram - [OneNAND Interface] Read the bufferram area with Sync. Burst mode
 * @param mtd		MTD data structure
 * @param area		BufferRAM area
 * @param buffer	the databuffer to put/get data
 * @param offset	offset to read from or write to
 * @param count		number of bytes to read/write
 *
 * Read the BufferRAM area with Sync. Burst Mode
 */
static int onenand_sync_read_bufferram(struct mtd_info *mtd, loff_t addr, int area,
				       unsigned char *buffer, int offset,
				       size_t count)
{
	struct onenand_chip *this = mtd->priv;
	void __iomem *bufferram;

	bufferram = this->base + area;
	bufferram += onenand_bufferram_offset(mtd, area);

	this->mmcontrol(mtd, ONENAND_SYS_CFG1_SYNC_READ);

	memcpy_16(buffer, bufferram + offset, count);

	this->mmcontrol(mtd, 0);

	return 0;
}

/**
 * onenand_write_bufferram - [OneNAND Interface] Write the bufferram area
 * @param mtd		MTD data structure
 * @param area		BufferRAM area
 * @param buffer	the databuffer to put/get data
 * @param offset	offset to read from or write to
 * @param count		number of bytes to read/write
 *
 * Write the BufferRAM area
 */
static int onenand_write_bufferram(struct mtd_info *mtd, loff_t addr, int area,
				   const unsigned char *buffer, int offset,
				   size_t count)
{
	struct onenand_chip *this = mtd->priv;
	void __iomem *bufferram;

	bufferram = this->base + area;
	bufferram += onenand_bufferram_offset(mtd, area);

	memcpy_16(bufferram + offset, buffer, count);

	return 0;
}

/**
 * onenand_get_2x_blockpage - [GENERIC] Get blockpage at 2x program mode
 * @param mtd		MTD data structure
 * @param addr		address to check
 * @return		blockpage address
 *
 * Get blockpage address at 2x program mode
 */
static int onenand_get_2x_blockpage(struct mtd_info *mtd, loff_t addr)
{
	struct onenand_chip *this = mtd->priv;
	int blockpage, block, page;

	/* Calculate the even block number */
	block = (int) (addr >> this->erase_shift) & ~1;
	/* Is it the odd plane? */
	if (addr & this->writesize)
		block++;
	page = (int) (addr >> (this->page_shift + 1)) & this->page_mask;
	blockpage = (block << 7) | page;

	return blockpage;
}

/**
 * onenand_check_bufferram - [GENERIC] Check BufferRAM information
 * @param mtd		MTD data structure
 * @param addr		address to check
 * @return		1 if there are valid data, otherwise 0
 *
 * Check bufferram if there is data we required
 */
static int onenand_check_bufferram(struct mtd_info *mtd, loff_t addr)
{
	struct onenand_chip *this = mtd->priv;
	int blockpage, found = 0;
	unsigned int i;

	if (ONENAND_IS_2PLANE(this))
		blockpage = onenand_get_2x_blockpage(mtd, addr);
	else
		blockpage = (int) (addr >> this->page_shift);

	/* Is there valid data? */
	i = ONENAND_CURRENT_BUFFERRAM(this);
	if (this->bufferram[i].blockpage == blockpage)
		found = 1;
	else {
		/* Check another BufferRAM */
		i = ONENAND_NEXT_BUFFERRAM(this);
		if (this->bufferram[i].blockpage == blockpage) {
			ONENAND_SET_NEXT_BUFFERRAM(this);
			found = 1;
		}
	}

	if (found && ONENAND_IS_DDP(this)) {
		/* Select DataRAM for DDP */
		int block = onenand_block(this, addr);
		int value = onenand_bufferram_address(this, block);
		this->write_word(value, this->base + ONENAND_REG_START_ADDRESS2);
	}

	return found;
}

/**
 * onenand_update_bufferram - [GENERIC] Update BufferRAM information
 * @param mtd		MTD data structure
 * @param addr		address to update
 * @param valid		valid flag
 *
 * Update BufferRAM information
 */
static int onenand_update_bufferram(struct mtd_info *mtd, loff_t addr,
				    int valid)
{
	struct onenand_chip *this = mtd->priv;
	int blockpage;
	unsigned int i;

	if (ONENAND_IS_2PLANE(this))
		blockpage = onenand_get_2x_blockpage(mtd, addr);
	else
		blockpage = (int)(addr >> this->page_shift);

	/* Invalidate another BufferRAM */
	i = ONENAND_NEXT_BUFFERRAM(this);
	if (this->bufferram[i].blockpage == blockpage)
		this->bufferram[i].blockpage = -1;

	/* Update BufferRAM */
	i = ONENAND_CURRENT_BUFFERRAM(this);
	if (valid)
		this->bufferram[i].blockpage = blockpage;
	else
		this->bufferram[i].blockpage = -1;

	return 0;
}

/**
 * onenand_invalidate_bufferram - [GENERIC] Invalidate BufferRAM information
 * @param mtd           MTD data structure
 * @param addr          start address to invalidate
 * @param len           length to invalidate
 *
 * Invalidate BufferRAM information
 */
static void onenand_invalidate_bufferram(struct mtd_info *mtd, loff_t addr,
					 unsigned int len)
{
	struct onenand_chip *this = mtd->priv;
	int i;
	loff_t end_addr = addr + len;

	/* Invalidate BufferRAM */
	for (i = 0; i < MAX_BUFFERRAM; i++) {
		loff_t buf_addr = this->bufferram[i].blockpage << this->page_shift;

		if (buf_addr >= addr && buf_addr < end_addr)
			this->bufferram[i].blockpage = -1;
	}
}

/**
 * onenand_get_device - [GENERIC] Get chip for selected access
 * @param mtd		MTD device structure
 * @param new_state	the state which is requested
 *
 * Get the device and lock it for exclusive access
 */
static void onenand_get_device(struct mtd_info *mtd, int new_state)
{
	/* Do nothing */
}

/**
 * onenand_release_device - [GENERIC] release chip
 * @param mtd		MTD device structure
 *
 * Deselect, release chip lock and wake up anyone waiting on the device
 */
static void onenand_release_device(struct mtd_info *mtd)
{
	/* Do nothing */
}

/**
 * onenand_transfer_auto_oob - [INTERN] oob auto-placement transfer
 * @param mtd		MTD device structure
 * @param buf		destination address
 * @param column	oob offset to read from
 * @param thislen	oob length to read
 */
static int onenand_transfer_auto_oob(struct mtd_info *mtd, uint8_t *buf,
					int column, int thislen)
{
	struct onenand_chip *this = mtd->priv;
	struct nand_oobfree *free;
	int readcol = column;
	int readend = column + thislen;
	int lastgap = 0;
	unsigned int i;
	uint8_t *oob_buf = this->oob_buf;

	free = this->ecclayout->oobfree;
	for (i = 0; i < MTD_MAX_OOBFREE_ENTRIES_LARGE && free->length;
	     i++, free++) {
		if (readcol >= lastgap)
			readcol += free->offset - lastgap;
		if (readend >= lastgap)
			readend += free->offset - lastgap;
		lastgap = free->offset + free->length;
	}
	this->read_bufferram(mtd, 0, ONENAND_SPARERAM, oob_buf, 0, mtd->oobsize);
	free = this->ecclayout->oobfree;
	for (i = 0; i < MTD_MAX_OOBFREE_ENTRIES_LARGE && free->length;
	     i++, free++) {
		int free_end = free->offset + free->length;
		if (free->offset < readend && free_end > readcol) {
			int st = max_t(int,free->offset,readcol);
			int ed = min_t(int,free_end,readend);
			int n = ed - st;
			memcpy(buf, oob_buf + st, n);
			buf += n;
		} else if (column == 0)
			break;
	}
	return 0;
}

/**
 * onenand_recover_lsb - [Flex-OneNAND] Recover LSB page data
 * @param mtd		MTD device structure
 * @param addr		address to recover
 * @param status	return value from onenand_wait
 *
 * MLC NAND Flash cell has paired pages - LSB page and MSB page. LSB page has
 * lower page address and MSB page has higher page address in paired pages.
 * If power off occurs during MSB page program, the paired LSB page data can
 * become corrupt. LSB page recovery read is a way to read LSB page though page
 * data are corrupted. When uncorrectable error occurs as a result of LSB page
 * read after power up, issue LSB page recovery read.
 */
static int onenand_recover_lsb(struct mtd_info *mtd, loff_t addr, int status)
{
	struct onenand_chip *this = mtd->priv;
	int i;

	/* Recovery is only for Flex-OneNAND */
	if (!FLEXONENAND(this))
		return status;

	/* check if we failed due to uncorrectable error */
	if (!mtd_is_eccerr(status) && status != ONENAND_BBT_READ_ECC_ERROR)
		return status;

	/* check if address lies in MLC region */
	i = flexonenand_region(mtd, addr);
	if (mtd->eraseregions[i].erasesize < (1 << this->erase_shift))
		return status;

	printk("onenand_recover_lsb:"
		"Attempting to recover from uncorrectable read\n");

	/* Issue the LSB page recovery command */
	this->command(mtd, FLEXONENAND_CMD_RECOVER_LSB, addr, this->writesize);
	return this->wait(mtd, FL_READING);
}

/**
 * onenand_read_ops_nolock - [OneNAND Interface] OneNAND read main and/or out-of-band
 * @param mtd		MTD device structure
 * @param from		offset to read from
 * @param ops		oob operation description structure
 *
 * OneNAND read main and/or out-of-band data
 */
static int onenand_read_ops_nolock(struct mtd_info *mtd, loff_t from,
		struct mtd_oob_ops *ops)
{
	struct onenand_chip *this = mtd->priv;
	struct mtd_ecc_stats stats;
	size_t len = ops->len;
	size_t ooblen = ops->ooblen;
	u_char *buf = ops->datbuf;
	u_char *oobbuf = ops->oobbuf;
	int read = 0, column, thislen;
	int oobread = 0, oobcolumn, thisooblen, oobsize;
	int ret = 0, boundary = 0;
	int writesize = this->writesize;

	pr_debug("onenand_read_ops_nolock: from = 0x%08x, len = %i\n",
		 (unsigned int) from, (int) len);

	if (ops->mode == MTD_OPS_AUTO_OOB)
		oobsize = this->ecclayout->oobavail;
	else
		oobsize = mtd->oobsize;

	oobcolumn = from & (mtd->oobsize - 1);

	/* Do not allow reads past end of device */
	if ((from + len) > mtd->size) {
		printk(KERN_ERR "onenand_read_ops_nolock: Attempt read beyond end of device\n");
		ops->retlen = 0;
		ops->oobretlen = 0;
		return -EINVAL;
	}

	stats = mtd->ecc_stats;

	/* Read-while-load method */
	/* Note: We can't use this feature in MLC */

	/* Do first load to bufferRAM */
	if (read < len) {
		if (!onenand_check_bufferram(mtd, from)) {
			this->main_buf = buf;
			this->command(mtd, ONENAND_CMD_READ, from, writesize);
			ret = this->wait(mtd, FL_READING);
			if (unlikely(ret))
				ret = onenand_recover_lsb(mtd, from, ret);
			onenand_update_bufferram(mtd, from, !ret);
			if (ret == -EBADMSG)
				ret = 0;
		}
	}

	thislen = min_t(int, writesize, len - read);
	column = from & (writesize - 1);
	if (column + thislen > writesize)
		thislen = writesize - column;

	while (!ret) {
		/* If there is more to load then start next load */
		from += thislen;
		if (!ONENAND_IS_4KB_PAGE(this) && read + thislen < len) {
			this->main_buf = buf + thislen;
			this->command(mtd, ONENAND_CMD_READ, from, writesize);
			/*
			 * Chip boundary handling in DDP
			 * Now we issued chip 1 read and pointed chip 1
			 * bufferam so we have to point chip 0 bufferam.
			 */
			if (ONENAND_IS_DDP(this) &&
					unlikely(from == (this->chipsize >> 1))) {
				this->write_word(ONENAND_DDP_CHIP0, this->base + ONENAND_REG_START_ADDRESS2);
				boundary = 1;
			} else
				boundary = 0;
			ONENAND_SET_PREV_BUFFERRAM(this);
		}

		/* While load is going, read from last bufferRAM */
		this->read_bufferram(mtd, from - thislen, ONENAND_DATARAM, buf, column, thislen);

		/* Read oob area if needed */
		if (oobbuf) {
			thisooblen = oobsize - oobcolumn;
			thisooblen = min_t(int, thisooblen, ooblen - oobread);

			if (ops->mode == MTD_OPS_AUTO_OOB)
				onenand_transfer_auto_oob(mtd, oobbuf, oobcolumn, thisooblen);
			else
				this->read_bufferram(mtd, 0, ONENAND_SPARERAM, oobbuf, oobcolumn, thisooblen);
			oobread += thisooblen;
			oobbuf += thisooblen;
			oobcolumn = 0;
		}

		if (ONENAND_IS_4KB_PAGE(this) && (read + thislen < len)) {
			this->command(mtd, ONENAND_CMD_READ, from, writesize);
			ret = this->wait(mtd, FL_READING);
			if (unlikely(ret))
				ret = onenand_recover_lsb(mtd, from, ret);
			onenand_update_bufferram(mtd, from, !ret);
			if (mtd_is_eccerr(ret))
				ret = 0;
		}

		/* See if we are done */
		read += thislen;
		if (read == len)
			break;
		/* Set up for next read from bufferRAM */
		if (unlikely(boundary))
			this->write_word(ONENAND_DDP_CHIP1, this->base + ONENAND_REG_START_ADDRESS2);
		if (!ONENAND_IS_4KB_PAGE(this))
			ONENAND_SET_NEXT_BUFFERRAM(this);
		buf += thislen;
		thislen = min_t(int, writesize, len - read);
		column = 0;

		if (!ONENAND_IS_4KB_PAGE(this)) {
			/* Now wait for load */
			ret = this->wait(mtd, FL_READING);
			onenand_update_bufferram(mtd, from, !ret);
			if (mtd_is_eccerr(ret))
				ret = 0;
		}
	}

	/*
	 * Return success, if no ECC failures, else -EBADMSG
	 * fs driver will take care of that, because
	 * retlen == desired len and result == -EBADMSG
	 */
	ops->retlen = read;
	ops->oobretlen = oobread;

	if (ret)
		return ret;

	if (mtd->ecc_stats.failed - stats.failed)
		return -EBADMSG;

	/* return max bitflips per ecc step; ONENANDs correct 1 bit only */
	return mtd->ecc_stats.corrected != stats.corrected ? 1 : 0;
}

/**
 * onenand_read_oob_nolock - [MTD Interface] OneNAND read out-of-band
 * @param mtd		MTD device structure
 * @param from		offset to read from
 * @param ops		oob operation description structure
 *
 * OneNAND read out-of-band data from the spare area
 */
static int onenand_read_oob_nolock(struct mtd_info *mtd, loff_t from,
		struct mtd_oob_ops *ops)
{
	struct onenand_chip *this = mtd->priv;
	struct mtd_ecc_stats stats;
	int read = 0, thislen, column, oobsize;
	size_t len = ops->ooblen;
	unsigned int mode = ops->mode;
	u_char *buf = ops->oobbuf;
	int ret = 0, readcmd;

	from += ops->ooboffs;

	pr_debug("onenand_read_oob_nolock: from = 0x%08x, len = %i\n",
		 (unsigned int) from, (int) len);

	/* Initialize return length value */
	ops->oobretlen = 0;

	if (mode == MTD_OPS_AUTO_OOB)
		oobsize = this->ecclayout->oobavail;
	else
		oobsize = mtd->oobsize;

	column = from & (mtd->oobsize - 1);

	if (unlikely(column >= oobsize)) {
		printk(KERN_ERR "onenand_read_oob_nolock: Attempted to start read outside oob\n");
		return -EINVAL;
	}

	/* Do not allow reads past end of device */
	if (unlikely(from >= mtd->size ||
		column + len > ((mtd->size >> this->page_shift) -
				(from >> this->page_shift)) * oobsize)) {
		printk(KERN_ERR "onenand_read_oob_nolock: Attempted to read beyond end of device\n");
		return -EINVAL;
	}

	stats = mtd->ecc_stats;

	readcmd = ONENAND_IS_4KB_PAGE(this) ?
		ONENAND_CMD_READ : ONENAND_CMD_READOOB;

	while (read < len) {
		thislen = oobsize - column;
		thislen = min_t(int, thislen, len);

		this->spare_buf = buf;
		this->command(mtd, readcmd, from, mtd->oobsize);

		onenand_update_bufferram(mtd, from, 0);

		ret = this->wait(mtd, FL_READING);
		if (unlikely(ret))
			ret = onenand_recover_lsb(mtd, from, ret);

		if (ret && ret != -EBADMSG) {
			printk(KERN_ERR "onenand_read_oob_nolock: read failed = 0x%x\n", ret);
			break;
		}

		if (mode == MTD_OPS_AUTO_OOB)
			onenand_transfer_auto_oob(mtd, buf, column, thislen);
		else
			this->read_bufferram(mtd, 0, ONENAND_SPARERAM, buf, column, thislen);

		read += thislen;

		if (read == len)
			break;

		buf += thislen;

		/* Read more? */
		if (read < len) {
			/* Page size */
			from += mtd->writesize;
			column = 0;
		}
	}

	ops->oobretlen = read;

	if (ret)
		return ret;

	if (mtd->ecc_stats.failed - stats.failed)
		return -EBADMSG;

	return 0;
}

/**
 * onenand_read - [MTD Interface] MTD compability function for onenand_read_ecc
 * @param mtd		MTD device structure
 * @param from		offset to read from
 * @param len		number of bytes to read
 * @param retlen	pointer to variable to store the number of read bytes
 * @param buf		the databuffer to put data
 *
 * This function simply calls onenand_read_ecc with oob buffer and oobsel = NULL
*/
int onenand_read(struct mtd_info *mtd, loff_t from, size_t len,
		 size_t * retlen, u_char * buf)
{
	struct mtd_oob_ops ops = {
		.len    = len,
		.ooblen = 0,
		.datbuf = buf,
		.oobbuf = NULL,
	};
	int ret;

	onenand_get_device(mtd, FL_READING);
	ret = onenand_read_ops_nolock(mtd, from, &ops);
	onenand_release_device(mtd);

	*retlen = ops.retlen;
	return ret;
}

/**
 * onenand_read_oob - [MTD Interface] OneNAND read out-of-band
 * @param mtd		MTD device structure
 * @param from		offset to read from
 * @param ops		oob operations description structure
 *
 * OneNAND main and/or out-of-band
 */
int onenand_read_oob(struct mtd_info *mtd, loff_t from,
			struct mtd_oob_ops *ops)
{
	int ret;

	switch (ops->mode) {
	case MTD_OPS_PLACE_OOB:
	case MTD_OPS_AUTO_OOB:
		break;
	case MTD_OPS_RAW:
		/* Not implemented yet */
	default:
		return -EINVAL;
	}

	onenand_get_device(mtd, FL_READING);
	if (ops->datbuf)
		ret = onenand_read_ops_nolock(mtd, from, ops);
	else
		ret = onenand_read_oob_nolock(mtd, from, ops);
	onenand_release_device(mtd);

	return ret;
}

/**
 * onenand_bbt_wait - [DEFAULT] wait until the command is done
 * @param mtd		MTD device structure
 * @param state		state to select the max. timeout value
 *
 * Wait for command done.
 */
static int onenand_bbt_wait(struct mtd_info *mtd, int state)
{
	struct onenand_chip *this = mtd->priv;
	unsigned int interrupt;
	unsigned int ctrl;

	/* Wait at most 20ms ... */
	u32 timeo = (CONFIG_SYS_HZ * 20) / 1000;
	u32 time_start = get_timer(0);
	do {
		WATCHDOG_RESET();
		if (get_timer(time_start) > timeo)
			return ONENAND_BBT_READ_FATAL_ERROR;
		interrupt = this->read_word(this->base + ONENAND_REG_INTERRUPT);
	} while ((interrupt & ONENAND_INT_MASTER) == 0);

	/* To get correct interrupt status in timeout case */
	interrupt = this->read_word(this->base + ONENAND_REG_INTERRUPT);
	ctrl = this->read_word(this->base + ONENAND_REG_CTRL_STATUS);

	if (interrupt & ONENAND_INT_READ) {
		int ecc = onenand_read_ecc(this);
		if (ecc & ONENAND_ECC_2BIT_ALL) {
			printk(KERN_INFO "onenand_bbt_wait: ecc error = 0x%04x"
				", controller = 0x%04x\n", ecc, ctrl);
			return ONENAND_BBT_READ_ERROR;
		}
	} else {
		printk(KERN_ERR "onenand_bbt_wait: read timeout!"
				"ctrl=0x%04x intr=0x%04x\n", ctrl, interrupt);
		return ONENAND_BBT_READ_FATAL_ERROR;
	}

	/* Initial bad block case: 0x2400 or 0x0400 */
	if (ctrl & ONENAND_CTRL_ERROR) {
		printk(KERN_DEBUG "onenand_bbt_wait: controller error = 0x%04x\n", ctrl);
		return ONENAND_BBT_READ_ERROR;
	}

	return 0;
}

/**
 * onenand_bbt_read_oob - [MTD Interface] OneNAND read out-of-band for bbt scan
 * @param mtd		MTD device structure
 * @param from		offset to read from
 * @param ops		oob operation description structure
 *
 * OneNAND read out-of-band data from the spare area for bbt scan
 */
int onenand_bbt_read_oob(struct mtd_info *mtd, loff_t from,
		struct mtd_oob_ops *ops)
{
	struct onenand_chip *this = mtd->priv;
	int read = 0, thislen, column;
	int ret = 0, readcmd;
	size_t len = ops->ooblen;
	u_char *buf = ops->oobbuf;

	pr_debug("onenand_bbt_read_oob: from = 0x%08x, len = %zi\n",
		 (unsigned int) from, len);

	readcmd = ONENAND_IS_4KB_PAGE(this) ?
		ONENAND_CMD_READ : ONENAND_CMD_READOOB;

	/* Initialize return value */
	ops->oobretlen = 0;

	/* Do not allow reads past end of device */
	if (unlikely((from + len) > mtd->size)) {
		printk(KERN_ERR "onenand_bbt_read_oob: Attempt read beyond end of device\n");
		return ONENAND_BBT_READ_FATAL_ERROR;
	}

	/* Grab the lock and see if the device is available */
	onenand_get_device(mtd, FL_READING);

	column = from & (mtd->oobsize - 1);

	while (read < len) {

		thislen = mtd->oobsize - column;
		thislen = min_t(int, thislen, len);

		this->spare_buf = buf;
		this->command(mtd, readcmd, from, mtd->oobsize);

		onenand_update_bufferram(mtd, from, 0);

		ret = this->bbt_wait(mtd, FL_READING);
		if (unlikely(ret))
			ret = onenand_recover_lsb(mtd, from, ret);

		if (ret)
			break;

		this->read_bufferram(mtd, 0, ONENAND_SPARERAM, buf, column, thislen);
		read += thislen;
		if (read == len)
			break;

		buf += thislen;

		/* Read more? */
		if (read < len) {
			/* Update Page size */
			from += this->writesize;
			column = 0;
		}
	}

	/* Deselect and wake up anyone waiting on the device */
	onenand_release_device(mtd);

	ops->oobretlen = read;
	return ret;
}


#ifdef CONFIG_MTD_ONENAND_VERIFY_WRITE
/**
 * onenand_verify_oob - [GENERIC] verify the oob contents after a write
 * @param mtd           MTD device structure
 * @param buf           the databuffer to verify
 * @param to            offset to read from
 */
static int onenand_verify_oob(struct mtd_info *mtd, const u_char *buf, loff_t to)
{
	struct onenand_chip *this = mtd->priv;
	u_char *oob_buf = this->oob_buf;
	int status, i, readcmd;

	readcmd = ONENAND_IS_4KB_PAGE(this) ?
		ONENAND_CMD_READ : ONENAND_CMD_READOOB;

	this->command(mtd, readcmd, to, mtd->oobsize);
	onenand_update_bufferram(mtd, to, 0);
	status = this->wait(mtd, FL_READING);
	if (status)
		return status;

	this->read_bufferram(mtd, 0, ONENAND_SPARERAM, oob_buf, 0, mtd->oobsize);
	for (i = 0; i < mtd->oobsize; i++)
		if (buf[i] != 0xFF && buf[i] != oob_buf[i])
			return -EBADMSG;

	return 0;
}

/**
 * onenand_verify - [GENERIC] verify the chip contents after a write
 * @param mtd          MTD device structure
 * @param buf          the databuffer to verify
 * @param addr         offset to read from
 * @param len          number of bytes to read and compare
 */
static int onenand_verify(struct mtd_info *mtd, const u_char *buf, loff_t addr, size_t len)
{
	struct onenand_chip *this = mtd->priv;
	void __iomem *dataram;
	int ret = 0;
	int thislen, column;

	while (len != 0) {
		thislen = min_t(int, this->writesize, len);
		column = addr & (this->writesize - 1);
		if (column + thislen > this->writesize)
			thislen = this->writesize - column;

		this->command(mtd, ONENAND_CMD_READ, addr, this->writesize);

		onenand_update_bufferram(mtd, addr, 0);

		ret = this->wait(mtd, FL_READING);
		if (ret)
			return ret;

		onenand_update_bufferram(mtd, addr, 1);

		dataram = this->base + ONENAND_DATARAM;
		dataram += onenand_bufferram_offset(mtd, ONENAND_DATARAM);

		if (memcmp(buf, dataram + column, thislen))
			return -EBADMSG;

		len -= thislen;
		buf += thislen;
		addr += thislen;
	}

	return 0;
}
#else
#define onenand_verify(...)             (0)
#define onenand_verify_oob(...)         (0)
#endif

#define NOTALIGNED(x)	((x & (this->subpagesize - 1)) != 0)

/**
 * onenand_fill_auto_oob - [INTERN] oob auto-placement transfer
 * @param mtd           MTD device structure
 * @param oob_buf       oob buffer
 * @param buf           source address
 * @param column        oob offset to write to
 * @param thislen       oob length to write
 */
static int onenand_fill_auto_oob(struct mtd_info *mtd, u_char *oob_buf,
		const u_char *buf, int column, int thislen)
{
	struct onenand_chip *this = mtd->priv;
	struct nand_oobfree *free;
	int writecol = column;
	int writeend = column + thislen;
	int lastgap = 0;
	unsigned int i;

	free = this->ecclayout->oobfree;
	for (i = 0; i < MTD_MAX_OOBFREE_ENTRIES_LARGE && free->length;
	     i++, free++) {
		if (writecol >= lastgap)
			writecol += free->offset - lastgap;
		if (writeend >= lastgap)
			writeend += free->offset - lastgap;
		lastgap = free->offset + free->length;
	}
	free = this->ecclayout->oobfree;
	for (i = 0; i < MTD_MAX_OOBFREE_ENTRIES_LARGE && free->length;
	     i++, free++) {
		int free_end = free->offset + free->length;
		if (free->offset < writeend && free_end > writecol) {
			int st = max_t(int,free->offset,writecol);
			int ed = min_t(int,free_end,writeend);
			int n = ed - st;
			memcpy(oob_buf + st, buf, n);
			buf += n;
		} else if (column == 0)
			break;
	}
	return 0;
}

/**
 * onenand_write_ops_nolock - [OneNAND Interface] write main and/or out-of-band
 * @param mtd           MTD device structure
 * @param to            offset to write to
 * @param ops           oob operation description structure
 *
 * Write main and/or oob with ECC
 */
static int onenand_write_ops_nolock(struct mtd_info *mtd, loff_t to,
		struct mtd_oob_ops *ops)
{
	struct onenand_chip *this = mtd->priv;
	int written = 0, column, thislen, subpage;
	int oobwritten = 0, oobcolumn, thisooblen, oobsize;
	size_t len = ops->len;
	size_t ooblen = ops->ooblen;
	const u_char *buf = ops->datbuf;
	const u_char *oob = ops->oobbuf;
	u_char *oobbuf;
	int ret = 0;

	pr_debug("onenand_write_ops_nolock: to = 0x%08x, len = %i\n",
		 (unsigned int) to, (int) len);

	/* Initialize retlen, in case of early exit */
	ops->retlen = 0;
	ops->oobretlen = 0;

	/* Reject writes, which are not page aligned */
	if (unlikely(NOTALIGNED(to) || NOTALIGNED(len))) {
		printk(KERN_ERR "onenand_write_ops_nolock: Attempt to write not page aligned data\n");
		return -EINVAL;
	}

	if (ops->mode == MTD_OPS_AUTO_OOB)
		oobsize = this->ecclayout->oobavail;
	else
		oobsize = mtd->oobsize;

	oobcolumn = to & (mtd->oobsize - 1);

	column = to & (mtd->writesize - 1);

	/* Loop until all data write */
	while (written < len) {
		u_char *wbuf = (u_char *) buf;

		thislen = min_t(int, mtd->writesize - column, len - written);
		thisooblen = min_t(int, oobsize - oobcolumn, ooblen - oobwritten);

		this->command(mtd, ONENAND_CMD_BUFFERRAM, to, thislen);

		/* Partial page write */
		subpage = thislen < mtd->writesize;
		if (subpage) {
			memset(this->page_buf, 0xff, mtd->writesize);
			memcpy(this->page_buf + column, buf, thislen);
			wbuf = this->page_buf;
		}

		this->write_bufferram(mtd, to, ONENAND_DATARAM, wbuf, 0, mtd->writesize);

		if (oob) {
			oobbuf = this->oob_buf;

			/* We send data to spare ram with oobsize
			 *                          * to prevent byte access */
			memset(oobbuf, 0xff, mtd->oobsize);
			if (ops->mode == MTD_OPS_AUTO_OOB)
				onenand_fill_auto_oob(mtd, oobbuf, oob, oobcolumn, thisooblen);
			else
				memcpy(oobbuf + oobcolumn, oob, thisooblen);

			oobwritten += thisooblen;
			oob += thisooblen;
			oobcolumn = 0;
		} else
			oobbuf = (u_char *) ffchars;

		this->write_bufferram(mtd, 0, ONENAND_SPARERAM, oobbuf, 0, mtd->oobsize);

		this->command(mtd, ONENAND_CMD_PROG, to, mtd->writesize);

		ret = this->wait(mtd, FL_WRITING);

		/* In partial page write we don't update bufferram */
		onenand_update_bufferram(mtd, to, !ret && !subpage);
		if (ONENAND_IS_2PLANE(this)) {
			ONENAND_SET_BUFFERRAM1(this);
			onenand_update_bufferram(mtd, to + this->writesize, !ret && !subpage);
		}

		if (ret) {
			printk(KERN_ERR "onenand_write_ops_nolock: write filaed %d\n", ret);
			break;
		}

		/* Only check verify write turn on */
		ret = onenand_verify(mtd, buf, to, thislen);
		if (ret) {
			printk(KERN_ERR "onenand_write_ops_nolock: verify failed %d\n", ret);
			break;
		}

		written += thislen;

		if (written == len)
			break;

		column = 0;
		to += thislen;
		buf += thislen;
	}

	ops->retlen = written;

	return ret;
}

/**
 * onenand_write_oob_nolock - [INTERN] OneNAND write out-of-band
 * @param mtd           MTD device structure
 * @param to            offset to write to
 * @param len           number of bytes to write
 * @param retlen        pointer to variable to store the number of written bytes
 * @param buf           the data to write
 * @param mode          operation mode
 *
 * OneNAND write out-of-band
 */
static int onenand_write_oob_nolock(struct mtd_info *mtd, loff_t to,
		struct mtd_oob_ops *ops)
{
	struct onenand_chip *this = mtd->priv;
	int column, ret = 0, oobsize;
	int written = 0, oobcmd;
	u_char *oobbuf;
	size_t len = ops->ooblen;
	const u_char *buf = ops->oobbuf;
	unsigned int mode = ops->mode;

	to += ops->ooboffs;

	pr_debug("onenand_write_oob_nolock: to = 0x%08x, len = %i\n",
		 (unsigned int) to, (int) len);

	/* Initialize retlen, in case of early exit */
	ops->oobretlen = 0;

	if (mode == MTD_OPS_AUTO_OOB)
		oobsize = this->ecclayout->oobavail;
	else
		oobsize = mtd->oobsize;

	column = to & (mtd->oobsize - 1);

	if (unlikely(column >= oobsize)) {
		printk(KERN_ERR "onenand_write_oob_nolock: Attempted to start write outside oob\n");
		return -EINVAL;
	}

	/* For compatibility with NAND: Do not allow write past end of page */
	if (unlikely(column + len > oobsize)) {
		printk(KERN_ERR "onenand_write_oob_nolock: "
				"Attempt to write past end of page\n");
		return -EINVAL;
	}

	/* Do not allow reads past end of device */
	if (unlikely(to >= mtd->size ||
				column + len > ((mtd->size >> this->page_shift) -
					(to >> this->page_shift)) * oobsize)) {
		printk(KERN_ERR "onenand_write_oob_nolock: Attempted to write past end of device\n");
		return -EINVAL;
	}

	oobbuf = this->oob_buf;

	oobcmd = ONENAND_IS_4KB_PAGE(this) ?
		ONENAND_CMD_PROG : ONENAND_CMD_PROGOOB;

	/* Loop until all data write */
	while (written < len) {
		int thislen = min_t(int, oobsize, len - written);

		this->command(mtd, ONENAND_CMD_BUFFERRAM, to, mtd->oobsize);

		/* We send data to spare ram with oobsize
		 * to prevent byte access */
		memset(oobbuf, 0xff, mtd->oobsize);
		if (mode == MTD_OPS_AUTO_OOB)
			onenand_fill_auto_oob(mtd, oobbuf, buf, column, thislen);
		else
			memcpy(oobbuf + column, buf, thislen);
		this->write_bufferram(mtd, 0, ONENAND_SPARERAM, oobbuf, 0, mtd->oobsize);

		if (ONENAND_IS_4KB_PAGE(this)) {
			/* Set main area of DataRAM to 0xff*/
			memset(this->page_buf, 0xff, mtd->writesize);
			this->write_bufferram(mtd, 0, ONENAND_DATARAM,
				this->page_buf,	0, mtd->writesize);
		}

		this->command(mtd, oobcmd, to, mtd->oobsize);

		onenand_update_bufferram(mtd, to, 0);
		if (ONENAND_IS_2PLANE(this)) {
			ONENAND_SET_BUFFERRAM1(this);
			onenand_update_bufferram(mtd, to + this->writesize, 0);
		}

		ret = this->wait(mtd, FL_WRITING);
		if (ret) {
			printk(KERN_ERR "onenand_write_oob_nolock: write failed %d\n", ret);
			break;
		}

		ret = onenand_verify_oob(mtd, oobbuf, to);
		if (ret) {
			printk(KERN_ERR "onenand_write_oob_nolock: verify failed %d\n", ret);
			break;
		}

		written += thislen;
		if (written == len)
			break;

		to += mtd->writesize;
		buf += thislen;
		column = 0;
	}

	ops->oobretlen = written;

	return ret;
}

/**
 * onenand_write - [MTD Interface] compability function for onenand_write_ecc
 * @param mtd		MTD device structure
 * @param to		offset to write to
 * @param len		number of bytes to write
 * @param retlen	pointer to variable to store the number of written bytes
 * @param buf		the data to write
 *
 * Write with ECC
 */
int onenand_write(struct mtd_info *mtd, loff_t to, size_t len,
		  size_t * retlen, const u_char * buf)
{
	struct mtd_oob_ops ops = {
		.len    = len,
		.ooblen = 0,
		.datbuf = (u_char *) buf,
		.oobbuf = NULL,
	};
	int ret;

	onenand_get_device(mtd, FL_WRITING);
	ret = onenand_write_ops_nolock(mtd, to, &ops);
	onenand_release_device(mtd);

	*retlen = ops.retlen;
	return ret;
}

/**
 * onenand_write_oob - [MTD Interface] OneNAND write out-of-band
 * @param mtd		MTD device structure
 * @param to		offset to write to
 * @param ops		oob operation description structure
 *
 * OneNAND write main and/or out-of-band
 */
int onenand_write_oob(struct mtd_info *mtd, loff_t to,
			struct mtd_oob_ops *ops)
{
	int ret;

	switch (ops->mode) {
	case MTD_OPS_PLACE_OOB:
	case MTD_OPS_AUTO_OOB:
		break;
	case MTD_OPS_RAW:
		/* Not implemented yet */
	default:
		return -EINVAL;
	}

	onenand_get_device(mtd, FL_WRITING);
	if (ops->datbuf)
		ret = onenand_write_ops_nolock(mtd, to, ops);
	else
		ret = onenand_write_oob_nolock(mtd, to, ops);
	onenand_release_device(mtd);

	return ret;

}

/**
 * onenand_block_isbad_nolock - [GENERIC] Check if a block is marked bad
 * @param mtd		MTD device structure
 * @param ofs		offset from device start
 * @param allowbbt	1, if its allowed to access the bbt area
 *
 * Check, if the block is bad, Either by reading the bad block table or
 * calling of the scan function.
 */
static int onenand_block_isbad_nolock(struct mtd_info *mtd, loff_t ofs, int allowbbt)
{
	struct onenand_chip *this = mtd->priv;
	struct bbm_info *bbm = this->bbm;

	/* Return info from the table */
	return bbm->isbad_bbt(mtd, ofs, allowbbt);
}


/**
 * onenand_erase - [MTD Interface] erase block(s)
 * @param mtd		MTD device structure
 * @param instr		erase instruction
 *
 * Erase one ore more blocks
 */
int onenand_erase(struct mtd_info *mtd, struct erase_info *instr)
{
	struct onenand_chip *this = mtd->priv;
	unsigned int block_size;
	loff_t addr = instr->addr;
	unsigned int len = instr->len;
	int ret = 0, i;
	struct mtd_erase_region_info *region = NULL;
	unsigned int region_end = 0;

	pr_debug("onenand_erase: start = 0x%08x, len = %i\n",
			(unsigned int) addr, len);

	if (FLEXONENAND(this)) {
		/* Find the eraseregion of this address */
		i = flexonenand_region(mtd, addr);
		region = &mtd->eraseregions[i];

		block_size = region->erasesize;
		region_end = region->offset
			+ region->erasesize * region->numblocks;

		/* Start address within region must align on block boundary.
		 * Erase region's start offset is always block start address.
		 */
		if (unlikely((addr - region->offset) & (block_size - 1))) {
			pr_debug("onenand_erase:" " Unaligned address\n");
			return -EINVAL;
		}
	} else {
		block_size = 1 << this->erase_shift;

		/* Start address must align on block boundary */
		if (unlikely(addr & (block_size - 1))) {
			pr_debug("onenand_erase:" "Unaligned address\n");
			return -EINVAL;
		}
	}

	/* Length must align on block boundary */
	if (unlikely(len & (block_size - 1))) {
		pr_debug("onenand_erase: Length not block aligned\n");
		return -EINVAL;
	}

	/* Grab the lock and see if the device is available */
	onenand_get_device(mtd, FL_ERASING);

	/* Loop throught the pages */
	instr->state = MTD_ERASING;

	while (len) {

		/* Check if we have a bad block, we do not erase bad blocks */
		if (instr->priv == 0 && onenand_block_isbad_nolock(mtd, addr, 0)) {
			printk(KERN_WARNING "onenand_erase: attempt to erase"
				" a bad block at addr 0x%08x\n",
				(unsigned int) addr);
			instr->state = MTD_ERASE_FAILED;
			goto erase_exit;
		}

		this->command(mtd, ONENAND_CMD_ERASE, addr, block_size);

		onenand_invalidate_bufferram(mtd, addr, block_size);

		ret = this->wait(mtd, FL_ERASING);
		/* Check, if it is write protected */
		if (ret) {
			if (ret == -EPERM)
				pr_debug("onenand_erase: "
					 "Device is write protected!!!\n");
			else
				pr_debug("onenand_erase: "
					 "Failed erase, block %d\n",
					 onenand_block(this, addr));
			instr->state = MTD_ERASE_FAILED;
			instr->fail_addr = addr;

			goto erase_exit;
		}

		len -= block_size;
		addr += block_size;

		if (addr == region_end) {
			if (!len)
				break;
			region++;

			block_size = region->erasesize;
			region_end = region->offset
				+ region->erasesize * region->numblocks;

			if (len & (block_size - 1)) {
				/* This has been checked at MTD
				 * partitioning level. */
				printk("onenand_erase: Unaligned address\n");
				goto erase_exit;
			}
		}
	}

	instr->state = MTD_ERASE_DONE;

erase_exit:

	ret = instr->state == MTD_ERASE_DONE ? 0 : -EIO;
	/* Do call back function */
	if (!ret)
		mtd_erase_callback(instr);

	/* Deselect and wake up anyone waiting on the device */
	onenand_release_device(mtd);

	return ret;
}

/**
 * onenand_sync - [MTD Interface] sync
 * @param mtd		MTD device structure
 *
 * Sync is actually a wait for chip ready function
 */
void onenand_sync(struct mtd_info *mtd)
{
	pr_debug("onenand_sync: called\n");

	/* Grab the lock and see if the device is available */
	onenand_get_device(mtd, FL_SYNCING);

	/* Release it and go back */
	onenand_release_device(mtd);
}

/**
 * onenand_block_isbad - [MTD Interface] Check whether the block at the given offset is bad
 * @param mtd		MTD device structure
 * @param ofs		offset relative to mtd start
 *
 * Check whether the block is bad
 */
int onenand_block_isbad(struct mtd_info *mtd, loff_t ofs)
{
	int ret;

	/* Check for invalid offset */
	if (ofs > mtd->size)
		return -EINVAL;

	onenand_get_device(mtd, FL_READING);
	ret = onenand_block_isbad_nolock(mtd,ofs, 0);
	onenand_release_device(mtd);
	return ret;
}

/**
 * onenand_default_block_markbad - [DEFAULT] mark a block bad
 * @param mtd           MTD device structure
 * @param ofs           offset from device start
 *
 * This is the default implementation, which can be overridden by
 * a hardware specific driver.
 */
static int onenand_default_block_markbad(struct mtd_info *mtd, loff_t ofs)
{
	struct onenand_chip *this = mtd->priv;
	struct bbm_info *bbm = this->bbm;
	u_char buf[2] = {0, 0};
	struct mtd_oob_ops ops = {
		.mode = MTD_OPS_PLACE_OOB,
		.ooblen = 2,
		.oobbuf = buf,
		.ooboffs = 0,
	};
	int block;

	/* Get block number */
	block = onenand_block(this, ofs);
	if (bbm->bbt)
		bbm->bbt[block >> 2] |= 0x01 << ((block & 0x03) << 1);

	/* We write two bytes, so we dont have to mess with 16 bit access */
	ofs += mtd->oobsize + (bbm->badblockpos & ~0x01);
	return onenand_write_oob_nolock(mtd, ofs, &ops);
}

/**
 * onenand_block_markbad - [MTD Interface] Mark the block at the given offset as bad
 * @param mtd		MTD device structure
 * @param ofs		offset relative to mtd start
 *
 * Mark the block as bad
 */
int onenand_block_markbad(struct mtd_info *mtd, loff_t ofs)
{
	struct onenand_chip *this = mtd->priv;
	int ret;

	ret = onenand_block_isbad(mtd, ofs);
	if (ret) {
		/* If it was bad already, return success and do nothing */
		if (ret > 0)
			return 0;
		return ret;
	}

	onenand_get_device(mtd, FL_WRITING);
	ret = this->block_markbad(mtd, ofs);
	onenand_release_device(mtd);

	return ret;
}

/**
 * onenand_do_lock_cmd - [OneNAND Interface] Lock or unlock block(s)
 * @param mtd           MTD device structure
 * @param ofs           offset relative to mtd start
 * @param len           number of bytes to lock or unlock
 * @param cmd           lock or unlock command
 *
 * Lock or unlock one or more blocks
 */
static int onenand_do_lock_cmd(struct mtd_info *mtd, loff_t ofs, size_t len, int cmd)
{
	struct onenand_chip *this = mtd->priv;
	int start, end, block, value, status;

	start = onenand_block(this, ofs);
	end = onenand_block(this, ofs + len);

	/* Continuous lock scheme */
	if (this->options & ONENAND_HAS_CONT_LOCK) {
		/* Set start block address */
		this->write_word(start,
				 this->base + ONENAND_REG_START_BLOCK_ADDRESS);
		/* Set end block address */
		this->write_word(end - 1,
				 this->base + ONENAND_REG_END_BLOCK_ADDRESS);
		/* Write unlock command */
		this->command(mtd, cmd, 0, 0);

		/* There's no return value */
		this->wait(mtd, FL_UNLOCKING);

		/* Sanity check */
		while (this->read_word(this->base + ONENAND_REG_CTRL_STATUS)
		       & ONENAND_CTRL_ONGO)
			continue;

		/* Check lock status */
		status = this->read_word(this->base + ONENAND_REG_WP_STATUS);
		if (!(status & ONENAND_WP_US))
			printk(KERN_ERR "wp status = 0x%x\n", status);

		return 0;
	}

	/* Block lock scheme */
	for (block = start; block < end; block++) {
		/* Set block address */
		value = onenand_block_address(this, block);
		this->write_word(value, this->base + ONENAND_REG_START_ADDRESS1);
		/* Select DataRAM for DDP */
		value = onenand_bufferram_address(this, block);
		this->write_word(value, this->base + ONENAND_REG_START_ADDRESS2);

		/* Set start block address */
		this->write_word(block,
				 this->base + ONENAND_REG_START_BLOCK_ADDRESS);
		/* Write unlock command */
		this->command(mtd, ONENAND_CMD_UNLOCK, 0, 0);

		/* There's no return value */
		this->wait(mtd, FL_UNLOCKING);

		/* Sanity check */
		while (this->read_word(this->base + ONENAND_REG_CTRL_STATUS)
		       & ONENAND_CTRL_ONGO)
			continue;

		/* Check lock status */
		status = this->read_word(this->base + ONENAND_REG_WP_STATUS);
		if (!(status & ONENAND_WP_US))
			printk(KERN_ERR "block = %d, wp status = 0x%x\n",
			       block, status);
	}

	return 0;
}

#ifdef ONENAND_LINUX
/**
 * onenand_lock - [MTD Interface] Lock block(s)
 * @param mtd           MTD device structure
 * @param ofs           offset relative to mtd start
 * @param len           number of bytes to unlock
 *
 * Lock one or more blocks
 */
static int onenand_lock(struct mtd_info *mtd, loff_t ofs, size_t len)
{
	int ret;

	onenand_get_device(mtd, FL_LOCKING);
	ret = onenand_do_lock_cmd(mtd, ofs, len, ONENAND_CMD_LOCK);
	onenand_release_device(mtd);
	return ret;
}

/**
 * onenand_unlock - [MTD Interface] Unlock block(s)
 * @param mtd           MTD device structure
 * @param ofs           offset relative to mtd start
 * @param len           number of bytes to unlock
 *
 * Unlock one or more blocks
 */
static int onenand_unlock(struct mtd_info *mtd, loff_t ofs, size_t len)
{
	int ret;

	onenand_get_device(mtd, FL_LOCKING);
	ret = onenand_do_lock_cmd(mtd, ofs, len, ONENAND_CMD_UNLOCK);
	onenand_release_device(mtd);
	return ret;
}
#endif

/**
 * onenand_check_lock_status - [OneNAND Interface] Check lock status
 * @param this          onenand chip data structure
 *
 * Check lock status
 */
static int onenand_check_lock_status(struct onenand_chip *this)
{
	unsigned int value, block, status;
	unsigned int end;

	end = this->chipsize >> this->erase_shift;
	for (block = 0; block < end; block++) {
		/* Set block address */
		value = onenand_block_address(this, block);
		this->write_word(value, this->base + ONENAND_REG_START_ADDRESS1);
		/* Select DataRAM for DDP */
		value = onenand_bufferram_address(this, block);
		this->write_word(value, this->base + ONENAND_REG_START_ADDRESS2);
		/* Set start block address */
		this->write_word(block, this->base + ONENAND_REG_START_BLOCK_ADDRESS);

		/* Check lock status */
		status = this->read_word(this->base + ONENAND_REG_WP_STATUS);
		if (!(status & ONENAND_WP_US)) {
			printk(KERN_ERR "block = %d, wp status = 0x%x\n", block, status);
			return 0;
		}
	}

	return 1;
}

/**
 * onenand_unlock_all - [OneNAND Interface] unlock all blocks
 * @param mtd           MTD device structure
 *
 * Unlock all blocks
 */
static void onenand_unlock_all(struct mtd_info *mtd)
{
	struct onenand_chip *this = mtd->priv;
	loff_t ofs = 0;
	size_t len = mtd->size;

	if (this->options & ONENAND_HAS_UNLOCK_ALL) {
		/* Set start block address */
		this->write_word(0, this->base + ONENAND_REG_START_BLOCK_ADDRESS);
		/* Write unlock command */
		this->command(mtd, ONENAND_CMD_UNLOCK_ALL, 0, 0);

		/* There's no return value */
		this->wait(mtd, FL_LOCKING);

		/* Sanity check */
		while (this->read_word(this->base + ONENAND_REG_CTRL_STATUS)
				& ONENAND_CTRL_ONGO)
			continue;

		/* Check lock status */
		if (onenand_check_lock_status(this))
			return;

		/* Workaround for all block unlock in DDP */
		if (ONENAND_IS_DDP(this) && !FLEXONENAND(this)) {
			/* All blocks on another chip */
			ofs = this->chipsize >> 1;
			len = this->chipsize >> 1;
		}
	}

	onenand_do_lock_cmd(mtd, ofs, len, ONENAND_CMD_UNLOCK);
}


/**
 * onenand_check_features - Check and set OneNAND features
 * @param mtd           MTD data structure
 *
 * Check and set OneNAND features
 * - lock scheme
 * - two plane
 */
static void onenand_check_features(struct mtd_info *mtd)
{
	struct onenand_chip *this = mtd->priv;
	unsigned int density, process;

	/* Lock scheme depends on density and process */
	density = onenand_get_density(this->device_id);
	process = this->version_id >> ONENAND_VERSION_PROCESS_SHIFT;

	/* Lock scheme */
	switch (density) {
	case ONENAND_DEVICE_DENSITY_4Gb:
		if (ONENAND_IS_DDP(this))
			this->options |= ONENAND_HAS_2PLANE;
		else
			this->options |= ONENAND_HAS_4KB_PAGE;

	case ONENAND_DEVICE_DENSITY_2Gb:
		/* 2Gb DDP don't have 2 plane */
		if (!ONENAND_IS_DDP(this))
			this->options |= ONENAND_HAS_2PLANE;
		this->options |= ONENAND_HAS_UNLOCK_ALL;

	case ONENAND_DEVICE_DENSITY_1Gb:
		/* A-Die has all block unlock */
		if (process)
			this->options |= ONENAND_HAS_UNLOCK_ALL;
		break;

	default:
		/* Some OneNAND has continuous lock scheme */
		if (!process)
			this->options |= ONENAND_HAS_CONT_LOCK;
		break;
	}

	if (ONENAND_IS_MLC(this))
		this->options |= ONENAND_HAS_4KB_PAGE;

	if (ONENAND_IS_4KB_PAGE(this))
		this->options &= ~ONENAND_HAS_2PLANE;

	if (FLEXONENAND(this)) {
		this->options &= ~ONENAND_HAS_CONT_LOCK;
		this->options |= ONENAND_HAS_UNLOCK_ALL;
	}

	if (this->options & ONENAND_HAS_CONT_LOCK)
		printk(KERN_DEBUG "Lock scheme is Continuous Lock\n");
	if (this->options & ONENAND_HAS_UNLOCK_ALL)
		printk(KERN_DEBUG "Chip support all block unlock\n");
	if (this->options & ONENAND_HAS_2PLANE)
		printk(KERN_DEBUG "Chip has 2 plane\n");
	if (this->options & ONENAND_HAS_4KB_PAGE)
		printk(KERN_DEBUG "Chip has 4KiB pagesize\n");

}

/**
 * onenand_print_device_info - Print device ID
 * @param device        device ID
 *
 * Print device ID
 */
char *onenand_print_device_info(int device, int version)
{
	int vcc, demuxed, ddp, density, flexonenand;
	char *dev_info = malloc(80);
	char *p = dev_info;

	vcc = device & ONENAND_DEVICE_VCC_MASK;
	demuxed = device & ONENAND_DEVICE_IS_DEMUX;
	ddp = device & ONENAND_DEVICE_IS_DDP;
	density = onenand_get_density(device);
	flexonenand = device & DEVICE_IS_FLEXONENAND;
	p += sprintf(dev_info, "%s%sOneNAND%s %dMB %sV 16-bit (0x%02x)",
	       demuxed ? "" : "Muxed ",
	       flexonenand ? "Flex-" : "",
	       ddp ? "(DDP)" : "",
	       (16 << density), vcc ? "2.65/3.3" : "1.8", device);

	sprintf(p, "\nOneNAND version = 0x%04x", version);
	printk("%s\n", dev_info);

	return dev_info;
}

static const struct onenand_manufacturers onenand_manuf_ids[] = {
	{ONENAND_MFR_NUMONYX, "Numonyx"},
	{ONENAND_MFR_SAMSUNG, "Samsung"},
};

/**
 * onenand_check_maf - Check manufacturer ID
 * @param manuf         manufacturer ID
 *
 * Check manufacturer ID
 */
static int onenand_check_maf(int manuf)
{
	int size = ARRAY_SIZE(onenand_manuf_ids);
	int i;
#ifdef ONENAND_DEBUG
	char *name;
#endif

	for (i = 0; i < size; i++)
		if (manuf == onenand_manuf_ids[i].id)
			break;

#ifdef ONENAND_DEBUG
	if (i < size)
		name = onenand_manuf_ids[i].name;
	else
		name = "Unknown";

	printk(KERN_DEBUG "OneNAND Manufacturer: %s (0x%0x)\n", name, manuf);
#endif

	return i == size;
}

/**
* flexonenand_get_boundary	- Reads the SLC boundary
* @param onenand_info		- onenand info structure
*
* Fill up boundary[] field in onenand_chip
**/
static int flexonenand_get_boundary(struct mtd_info *mtd)
{
	struct onenand_chip *this = mtd->priv;
	unsigned int die, bdry;
	int syscfg, locked;

	/* Disable ECC */
	syscfg = this->read_word(this->base + ONENAND_REG_SYS_CFG1);
	this->write_word((syscfg | 0x0100), this->base + ONENAND_REG_SYS_CFG1);

	for (die = 0; die < this->dies; die++) {
		this->command(mtd, FLEXONENAND_CMD_PI_ACCESS, die, 0);
		this->wait(mtd, FL_SYNCING);

		this->command(mtd, FLEXONENAND_CMD_READ_PI, die, 0);
		this->wait(mtd, FL_READING);

		bdry = this->read_word(this->base + ONENAND_DATARAM);
		if ((bdry >> FLEXONENAND_PI_UNLOCK_SHIFT) == 3)
			locked = 0;
		else
			locked = 1;
		this->boundary[die] = bdry & FLEXONENAND_PI_MASK;

		this->command(mtd, ONENAND_CMD_RESET, 0, 0);
		this->wait(mtd, FL_RESETING);

		printk(KERN_INFO "Die %d boundary: %d%s\n", die,
		       this->boundary[die], locked ? "(Locked)" : "(Unlocked)");
	}

	/* Enable ECC */
	this->write_word(syscfg, this->base + ONENAND_REG_SYS_CFG1);
	return 0;
}

/**
 * flexonenand_get_size - Fill up fields in onenand_chip and mtd_info
 * 			  boundary[], diesize[], mtd->size, mtd->erasesize,
 * 			  mtd->eraseregions
 * @param mtd		- MTD device structure
 */
static void flexonenand_get_size(struct mtd_info *mtd)
{
	struct onenand_chip *this = mtd->priv;
	int die, i, eraseshift, density;
	int blksperdie, maxbdry;
	loff_t ofs;

	density = onenand_get_density(this->device_id);
	blksperdie = ((loff_t)(16 << density) << 20) >> (this->erase_shift);
	blksperdie >>= ONENAND_IS_DDP(this) ? 1 : 0;
	maxbdry = blksperdie - 1;
	eraseshift = this->erase_shift - 1;

	mtd->numeraseregions = this->dies << 1;

	/* This fills up the device boundary */
	flexonenand_get_boundary(mtd);
	die = 0;
	ofs = 0;
	i = -1;
	for (; die < this->dies; die++) {
		if (!die || this->boundary[die-1] != maxbdry) {
			i++;
			mtd->eraseregions[i].offset = ofs;
			mtd->eraseregions[i].erasesize = 1 << eraseshift;
			mtd->eraseregions[i].numblocks =
							this->boundary[die] + 1;
			ofs += mtd->eraseregions[i].numblocks << eraseshift;
			eraseshift++;
		} else {
			mtd->numeraseregions -= 1;
			mtd->eraseregions[i].numblocks +=
							this->boundary[die] + 1;
			ofs += (this->boundary[die] + 1) << (eraseshift - 1);
		}
		if (this->boundary[die] != maxbdry) {
			i++;
			mtd->eraseregions[i].offset = ofs;
			mtd->eraseregions[i].erasesize = 1 << eraseshift;
			mtd->eraseregions[i].numblocks = maxbdry ^
							 this->boundary[die];
			ofs += mtd->eraseregions[i].numblocks << eraseshift;
			eraseshift--;
		} else
			mtd->numeraseregions -= 1;
	}

	/* Expose MLC erase size except when all blocks are SLC */
	mtd->erasesize = 1 << this->erase_shift;
	if (mtd->numeraseregions == 1)
		mtd->erasesize >>= 1;

	printk(KERN_INFO "Device has %d eraseregions\n", mtd->numeraseregions);
	for (i = 0; i < mtd->numeraseregions; i++)
		printk(KERN_INFO "[offset: 0x%08llx, erasesize: 0x%05x,"
			" numblocks: %04u]\n", mtd->eraseregions[i].offset,
			mtd->eraseregions[i].erasesize,
			mtd->eraseregions[i].numblocks);

	for (die = 0, mtd->size = 0; die < this->dies; die++) {
		this->diesize[die] = (loff_t) (blksperdie << this->erase_shift);
		this->diesize[die] -= (loff_t) (this->boundary[die] + 1)
						 << (this->erase_shift - 1);
		mtd->size += this->diesize[die];
	}
}

/**
 * flexonenand_check_blocks_erased - Check if blocks are erased
 * @param mtd_info	- mtd info structure
 * @param start		- first erase block to check
 * @param end		- last erase block to check
 *
 * Converting an unerased block from MLC to SLC
 * causes byte values to change. Since both data and its ECC
 * have changed, reads on the block give uncorrectable error.
 * This might lead to the block being detected as bad.
 *
 * Avoid this by ensuring that the block to be converted is
 * erased.
 */
static int flexonenand_check_blocks_erased(struct mtd_info *mtd,
					int start, int end)
{
	struct onenand_chip *this = mtd->priv;
	int i, ret;
	int block;
	struct mtd_oob_ops ops = {
		.mode = MTD_OPS_PLACE_OOB,
		.ooboffs = 0,
		.ooblen	= mtd->oobsize,
		.datbuf	= NULL,
		.oobbuf	= this->oob_buf,
	};
	loff_t addr;

	printk(KERN_DEBUG "Check blocks from %d to %d\n", start, end);

	for (block = start; block <= end; block++) {
		addr = flexonenand_addr(this, block);
		if (onenand_block_isbad_nolock(mtd, addr, 0))
			continue;

		/*
		 * Since main area write results in ECC write to spare,
		 * it is sufficient to check only ECC bytes for change.
		 */
		ret = onenand_read_oob_nolock(mtd, addr, &ops);
		if (ret)
			return ret;

		for (i = 0; i < mtd->oobsize; i++)
			if (this->oob_buf[i] != 0xff)
				break;

		if (i != mtd->oobsize) {
			printk(KERN_WARNING "Block %d not erased.\n", block);
			return 1;
		}
	}

	return 0;
}

/**
 * flexonenand_set_boundary	- Writes the SLC boundary
 * @param mtd			- mtd info structure
 */
int flexonenand_set_boundary(struct mtd_info *mtd, int die,
				    int boundary, int lock)
{
	struct onenand_chip *this = mtd->priv;
	int ret, density, blksperdie, old, new, thisboundary;
	loff_t addr;

	if (die >= this->dies)
		return -EINVAL;

	if (boundary == this->boundary[die])
		return 0;

	density = onenand_get_density(this->device_id);
	blksperdie = ((16 << density) << 20) >> this->erase_shift;
	blksperdie >>= ONENAND_IS_DDP(this) ? 1 : 0;

	if (boundary >= blksperdie) {
		printk("flexonenand_set_boundary:"
			"Invalid boundary value. "
			"Boundary not changed.\n");
		return -EINVAL;
	}

	/* Check if converting blocks are erased */
	old = this->boundary[die] + (die * this->density_mask);
	new = boundary + (die * this->density_mask);
	ret = flexonenand_check_blocks_erased(mtd, min(old, new)
						+ 1, max(old, new));
	if (ret) {
		printk(KERN_ERR "flexonenand_set_boundary: Please erase blocks before boundary change\n");
		return ret;
	}

	this->command(mtd, FLEXONENAND_CMD_PI_ACCESS, die, 0);
	this->wait(mtd, FL_SYNCING);

	/* Check is boundary is locked */
	this->command(mtd, FLEXONENAND_CMD_READ_PI, die, 0);
	ret = this->wait(mtd, FL_READING);

	thisboundary = this->read_word(this->base + ONENAND_DATARAM);
	if ((thisboundary >> FLEXONENAND_PI_UNLOCK_SHIFT) != 3) {
		printk(KERN_ERR "flexonenand_set_boundary: boundary locked\n");
		goto out;
	}

	printk(KERN_INFO "flexonenand_set_boundary: Changing die %d boundary: %d%s\n",
			die, boundary, lock ? "(Locked)" : "(Unlocked)");

	boundary &= FLEXONENAND_PI_MASK;
	boundary |= lock ? 0 : (3 << FLEXONENAND_PI_UNLOCK_SHIFT);

	addr = die ? this->diesize[0] : 0;
	this->command(mtd, ONENAND_CMD_ERASE, addr, 0);
	ret = this->wait(mtd, FL_ERASING);
	if (ret) {
		printk("flexonenand_set_boundary:"
			"Failed PI erase for Die %d\n", die);
		goto out;
	}

	this->write_word(boundary, this->base + ONENAND_DATARAM);
	this->command(mtd, ONENAND_CMD_PROG, addr, 0);
	ret = this->wait(mtd, FL_WRITING);
	if (ret) {
		printk("flexonenand_set_boundary:"
			"Failed PI write for Die %d\n", die);
		goto out;
	}

	this->command(mtd, FLEXONENAND_CMD_PI_UPDATE, die, 0);
	ret = this->wait(mtd, FL_WRITING);
out:
	this->write_word(ONENAND_CMD_RESET, this->base + ONENAND_REG_COMMAND);
	this->wait(mtd, FL_RESETING);
	if (!ret)
		/* Recalculate device size on boundary change*/
		flexonenand_get_size(mtd);

	return ret;
}

/**
 * onenand_chip_probe - [OneNAND Interface] Probe the OneNAND chip
 * @param mtd		MTD device structure
 *
 * OneNAND detection method:
 *   Compare the the values from command with ones from register
 */
static int onenand_chip_probe(struct mtd_info *mtd)
{
	struct onenand_chip *this = mtd->priv;
	int bram_maf_id, bram_dev_id, maf_id, dev_id;
	int syscfg;

	/* Save system configuration 1 */
	syscfg = this->read_word(this->base + ONENAND_REG_SYS_CFG1);

	/* Clear Sync. Burst Read mode to read BootRAM */
	this->write_word((syscfg & ~ONENAND_SYS_CFG1_SYNC_READ),
			 this->base + ONENAND_REG_SYS_CFG1);

	/* Send the command for reading device ID from BootRAM */
	this->write_word(ONENAND_CMD_READID, this->base + ONENAND_BOOTRAM);

	/* Read manufacturer and device IDs from BootRAM */
	bram_maf_id = this->read_word(this->base + ONENAND_BOOTRAM + 0x0);
	bram_dev_id = this->read_word(this->base + ONENAND_BOOTRAM + 0x2);

	/* Reset OneNAND to read default register values */
	this->write_word(ONENAND_CMD_RESET, this->base + ONENAND_BOOTRAM);

	/* Wait reset */
	if (this->wait(mtd, FL_RESETING))
		return -ENXIO;

	/* Restore system configuration 1 */
	this->write_word(syscfg, this->base + ONENAND_REG_SYS_CFG1);

	/* Check manufacturer ID */
	if (onenand_check_maf(bram_maf_id))
		return -ENXIO;

	/* Read manufacturer and device IDs from Register */
	maf_id = this->read_word(this->base + ONENAND_REG_MANUFACTURER_ID);
	dev_id = this->read_word(this->base + ONENAND_REG_DEVICE_ID);

	/* Check OneNAND device */
	if (maf_id != bram_maf_id || dev_id != bram_dev_id)
		return -ENXIO;

	return 0;
}

/**
 * onenand_probe - [OneNAND Interface] Probe the OneNAND device
 * @param mtd		MTD device structure
 *
 * OneNAND detection method:
 *   Compare the the values from command with ones from register
 */
int onenand_probe(struct mtd_info *mtd)
{
	struct onenand_chip *this = mtd->priv;
	int dev_id, ver_id;
	int density;
	int ret;

	ret = this->chip_probe(mtd);
	if (ret)
		return ret;

	/* Read device IDs from Register */
	dev_id = this->read_word(this->base + ONENAND_REG_DEVICE_ID);
	ver_id = this->read_word(this->base + ONENAND_REG_VERSION_ID);
	this->technology = this->read_word(this->base + ONENAND_REG_TECHNOLOGY);

	/* Flash device information */
	mtd->name = onenand_print_device_info(dev_id, ver_id);
	this->device_id = dev_id;
	this->version_id = ver_id;

	/* Check OneNAND features */
	onenand_check_features(mtd);

	density = onenand_get_density(dev_id);
	if (FLEXONENAND(this)) {
		this->dies = ONENAND_IS_DDP(this) ? 2 : 1;
		/* Maximum possible erase regions */
		mtd->numeraseregions = this->dies << 1;
		mtd->eraseregions = malloc(sizeof(struct mtd_erase_region_info)
					* (this->dies << 1));
		if (!mtd->eraseregions)
			return -ENOMEM;
	}

	/*
	 * For Flex-OneNAND, chipsize represents maximum possible device size.
	 * mtd->size represents the actual device size.
	 */
	this->chipsize = (16 << density) << 20;

	/* OneNAND page size & block size */
	/* The data buffer size is equal to page size */
	mtd->writesize =
	    this->read_word(this->base + ONENAND_REG_DATA_BUFFER_SIZE);
	/* We use the full BufferRAM */
	if (ONENAND_IS_4KB_PAGE(this))
		mtd->writesize <<= 1;

	mtd->oobsize = mtd->writesize >> 5;
	/* Pagers per block is always 64 in OneNAND */
	mtd->erasesize = mtd->writesize << 6;
	/*
	 * Flex-OneNAND SLC area has 64 pages per block.
	 * Flex-OneNAND MLC area has 128 pages per block.
	 * Expose MLC erase size to find erase_shift and page_mask.
	 */
	if (FLEXONENAND(this))
		mtd->erasesize <<= 1;

	this->erase_shift = ffs(mtd->erasesize) - 1;
	this->page_shift = ffs(mtd->writesize) - 1;
	this->ppb_shift = (this->erase_shift - this->page_shift);
	this->page_mask = (mtd->erasesize / mtd->writesize) - 1;
	/* Set density mask. it is used for DDP */
	if (ONENAND_IS_DDP(this))
		this->density_mask = this->chipsize >> (this->erase_shift + 1);
	/* It's real page size */
	this->writesize = mtd->writesize;

	/* REVIST: Multichip handling */

	if (FLEXONENAND(this))
		flexonenand_get_size(mtd);
	else
		mtd->size = this->chipsize;

	mtd->flags = MTD_CAP_NANDFLASH;
	mtd->_erase = onenand_erase;
	mtd->_read_oob = onenand_read_oob;
	mtd->_write_oob = onenand_write_oob;
	mtd->_sync = onenand_sync;
	mtd->_block_isbad = onenand_block_isbad;
	mtd->_block_markbad = onenand_block_markbad;
	mtd->writebufsize = mtd->writesize;

	return 0;
}

/**
 * onenand_scan - [OneNAND Interface] Scan for the OneNAND device
 * @param mtd		MTD device structure
 * @param maxchips	Number of chips to scan for
 *
 * This fills out all the not initialized function pointers
 * with the defaults.
 * The flash ID is read and the mtd/chip structures are
 * filled with the appropriate values.
 */
int onenand_scan(struct mtd_info *mtd, int maxchips)
{
	int i;
	struct onenand_chip *this = mtd->priv;

	if (!this->read_word)
		this->read_word = onenand_readw;
	if (!this->write_word)
		this->write_word = onenand_writew;

	if (!this->command)
		this->command = onenand_command;
	if (!this->wait)
		this->wait = onenand_wait;
	if (!this->bbt_wait)
		this->bbt_wait = onenand_bbt_wait;

	if (!this->read_bufferram)
		this->read_bufferram = onenand_read_bufferram;
	if (!this->write_bufferram)
		this->write_bufferram = onenand_write_bufferram;

	if (!this->chip_probe)
		this->chip_probe = onenand_chip_probe;

	if (!this->block_markbad)
		this->block_markbad = onenand_default_block_markbad;
	if (!this->scan_bbt)
		this->scan_bbt = onenand_default_bbt;

	if (onenand_probe(mtd))
		return -ENXIO;

	/* Set Sync. Burst Read after probing */
	if (this->mmcontrol) {
		printk(KERN_INFO "OneNAND Sync. Burst Read support\n");
		this->read_bufferram = onenand_sync_read_bufferram;
	}

	/* Allocate buffers, if necessary */
	if (!this->page_buf) {
		this->page_buf = kzalloc(mtd->writesize, GFP_KERNEL);
		if (!this->page_buf) {
			printk(KERN_ERR "onenand_scan(): Can't allocate page_buf\n");
			return -ENOMEM;
		}
		this->options |= ONENAND_PAGEBUF_ALLOC;
	}
	if (!this->oob_buf) {
		this->oob_buf = kzalloc(mtd->oobsize, GFP_KERNEL);
		if (!this->oob_buf) {
			printk(KERN_ERR "onenand_scan: Can't allocate oob_buf\n");
			if (this->options & ONENAND_PAGEBUF_ALLOC) {
				this->options &= ~ONENAND_PAGEBUF_ALLOC;
				kfree(this->page_buf);
			}
			return -ENOMEM;
		}
		this->options |= ONENAND_OOBBUF_ALLOC;
	}

	this->state = FL_READY;

	/*
	 * Allow subpage writes up to oobsize.
	 */
	switch (mtd->oobsize) {
	case 128:
		this->ecclayout = &onenand_oob_128;
		mtd->subpage_sft = 0;
		break;

	case 64:
		this->ecclayout = &onenand_oob_64;
		mtd->subpage_sft = 2;
		break;

	case 32:
		this->ecclayout = &onenand_oob_32;
		mtd->subpage_sft = 1;
		break;

	default:
		printk(KERN_WARNING "No OOB scheme defined for oobsize %d\n",
			mtd->oobsize);
		mtd->subpage_sft = 0;
		/* To prevent kernel oops */
		this->ecclayout = &onenand_oob_32;
		break;
	}

	this->subpagesize = mtd->writesize >> mtd->subpage_sft;

	/*
	 * The number of bytes available for a client to place data into
	 * the out of band area
	 */
	this->ecclayout->oobavail = 0;

	for (i = 0; i < MTD_MAX_OOBFREE_ENTRIES_LARGE &&
	    this->ecclayout->oobfree[i].length; i++)
		this->ecclayout->oobavail +=
			this->ecclayout->oobfree[i].length;
	mtd->oobavail = this->ecclayout->oobavail;

	mtd->ecclayout = this->ecclayout;

	/* Unlock whole block */
	onenand_unlock_all(mtd);

	return this->scan_bbt(mtd);
}

/**
 * onenand_release - [OneNAND Interface] Free resources held by the OneNAND device
 * @param mtd		MTD device structure
 */
void onenand_release(struct mtd_info *mtd)
{
}

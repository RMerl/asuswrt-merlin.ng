// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright (C) 2011 Marek Vasut <marek.vasut@gmail.com>
 *
 * Based on code:
 *	Copyright (C) 2005-2009 Samsung Electronics
 *	Kyungmin Park <kyungmin.park@samsung.com>
 */

#include <common.h>
#include <asm/io.h>
#include <linux/mtd/onenand_regs.h>
#include <onenand_uboot.h>

/*
 * Device geometry:
 * - 2048b page, 128k erase block.
 * - 4096b page, 256k erase block.
 */
enum onenand_spl_pagesize {
	PAGE_2K = 2048,
	PAGE_4K = 4096,
};

static unsigned int density_mask;

#define ONENAND_PAGES_PER_BLOCK			64
#define onenand_sector_address(page)		(page << 2)
#define onenand_buffer_address()		((1 << 3) << 8)

static inline int onenand_block_address(int block)
{
	/* Device Flash Core select, NAND Flash Block Address */
	if (block & density_mask)
		return ONENAND_DDP_CHIP1 | (block ^ density_mask);

	return block;
}

static inline int onenand_bufferram_address(int block)
{
	/* Device BufferRAM Select */
	if (block & density_mask)
		return ONENAND_DDP_CHIP1;

	return ONENAND_DDP_CHIP0;
}

static inline uint16_t onenand_readw(uint32_t addr)
{
	return readw(CONFIG_SYS_ONENAND_BASE + addr);
}

static inline void onenand_writew(uint16_t value, uint32_t addr)
{
	writew(value, CONFIG_SYS_ONENAND_BASE + addr);
}

static enum onenand_spl_pagesize onenand_spl_get_geometry(void)
{
	unsigned int dev_id, density, size;

	if (!onenand_readw(ONENAND_REG_TECHNOLOGY)) {
		dev_id = onenand_readw(ONENAND_REG_DEVICE_ID);
		density = dev_id >> ONENAND_DEVICE_DENSITY_SHIFT;
		density &= ONENAND_DEVICE_DENSITY_MASK;

		if (density < ONENAND_DEVICE_DENSITY_4Gb)
			return PAGE_2K;

		if (dev_id & ONENAND_DEVICE_IS_DDP) {
			size = onenand_readw(ONENAND_REG_DATA_BUFFER_SIZE);
			density_mask = 1 << (18 + density - ffs(size));
			return PAGE_2K;
		}
	}

	return PAGE_4K;
}

static int onenand_spl_read_page(uint32_t block, uint32_t page, uint32_t *buf,
					enum onenand_spl_pagesize pagesize)
{
	const uint32_t addr = CONFIG_SYS_ONENAND_BASE + ONENAND_DATARAM;
	uint32_t offset;

	onenand_writew(onenand_block_address(block),
			ONENAND_REG_START_ADDRESS1);

	onenand_writew(onenand_bufferram_address(block),
			ONENAND_REG_START_ADDRESS2);

	onenand_writew(onenand_sector_address(page),
			ONENAND_REG_START_ADDRESS8);

	onenand_writew(onenand_buffer_address(),
			ONENAND_REG_START_BUFFER);

	onenand_writew(ONENAND_INT_CLEAR, ONENAND_REG_INTERRUPT);

	onenand_writew(ONENAND_CMD_READ, ONENAND_REG_COMMAND);

	while (!(onenand_readw(ONENAND_REG_INTERRUPT) & ONENAND_INT_READ))
		continue;

	/* Check for invalid block mark */
	if (page < 2 && (onenand_readw(ONENAND_SPARERAM) != 0xffff))
		return 1;

	for (offset = 0; offset < pagesize; offset += 4)
		buf[offset / 4] = readl(addr + offset);

	return 0;
}

#ifdef CONFIG_SPL_UBI
/* Temporary storage for non page aligned and non page sized reads. */
static u8 scratch_buf[PAGE_4K];

/**
 * onenand_spl_read_block - Read data from physical eraseblock into a buffer
 * @block:	Number of the physical eraseblock
 * @offset:	Data offset from the start of @peb
 * @len:	Data size to read
 * @dst:	Address of the destination buffer
 *
 * Notes:
 *	@offset + @len are not allowed to be larger than a physical
 *	erase block. No sanity check done for simplicity reasons.
 */
int onenand_spl_read_block(int block, int offset, int len, void *dst)
{
	int page, read;
	static int psize;

	if (!psize)
		psize = onenand_spl_get_geometry();

	/* Calculate the page number */
	page = offset / psize;
	/* Offset to the start of a flash page */
	offset = offset % psize;

	while (len) {
		/*
		 * Non page aligned reads go to the scratch buffer.
		 * Page aligned reads go directly to the destination.
		 */
		if (offset || len < psize) {
			onenand_spl_read_page(block, page,
			                      (uint32_t *)scratch_buf, psize);
			read = min(len, psize - offset);
			memcpy(dst, scratch_buf + offset, read);
			offset = 0;
		} else {
			onenand_spl_read_page(block, page, dst, psize);
			read = psize;
		}
		page++;
		len -= read;
		dst += read;
	}
	return 0;
}
#endif

void onenand_spl_load_image(uint32_t offs, uint32_t size, void *dst)
{
	uint32_t *addr = (uint32_t *)dst;
	uint32_t to_page;
	uint32_t block;
	uint32_t page, rpage;
	enum onenand_spl_pagesize pagesize;
	int ret;

	pagesize = onenand_spl_get_geometry();

	/*
	 * The page can be either 2k or 4k, avoid using DIV_ROUND_UP to avoid
	 * pulling further unwanted functions into the SPL.
	 */
	if (pagesize == 2048) {
		page = offs / 2048;
		to_page = page + DIV_ROUND_UP(size, 2048);
	} else {
		page = offs / 4096;
		to_page = page + DIV_ROUND_UP(size, 4096);
	}

	for (; page <= to_page; page++) {
		block = page / ONENAND_PAGES_PER_BLOCK;
		rpage = page & (ONENAND_PAGES_PER_BLOCK - 1);
		ret = onenand_spl_read_page(block, rpage, addr, pagesize);
		if (ret)
			page += ONENAND_PAGES_PER_BLOCK - 1;
		else
			addr += pagesize / 4;
	}
}

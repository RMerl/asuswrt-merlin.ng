// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright (C) 2015 Thomas Chou <thomas@wytron.com.tw>
 */

#include <common.h>
#include <console.h>
#include <dm.h>
#include <errno.h>
#include <fdt_support.h>
#include <flash.h>
#include <log.h>
#include <mtd.h>
#include <asm/global_data.h>
#include <asm/io.h>
#include <linux/bitops.h>

DECLARE_GLOBAL_DATA_PTR;

/* The STATUS register */
#define QUADSPI_SR_BP0				BIT(2)
#define QUADSPI_SR_BP1				BIT(3)
#define QUADSPI_SR_BP2				BIT(4)
#define QUADSPI_SR_BP2_0			GENMASK(4, 2)
#define QUADSPI_SR_BP3				BIT(6)
#define QUADSPI_SR_TB				BIT(5)

/*
 * The QUADSPI_MEM_OP register is used to do memory protect and erase operations
 */
#define QUADSPI_MEM_OP_BULK_ERASE		0x00000001
#define QUADSPI_MEM_OP_SECTOR_ERASE		0x00000002
#define QUADSPI_MEM_OP_SECTOR_PROTECT		0x00000003

/*
 * The QUADSPI_ISR register is used to determine whether an invalid write or
 * erase operation trigerred an interrupt
 */
#define QUADSPI_ISR_ILLEGAL_ERASE		BIT(0)
#define QUADSPI_ISR_ILLEGAL_WRITE		BIT(1)

struct altera_qspi_regs {
	u32	rd_status;
	u32	rd_sid;
	u32	rd_rdid;
	u32	mem_op;
	u32	isr;
	u32	imr;
	u32	chip_select;
};

struct altera_qspi_plat {
	struct altera_qspi_regs *regs;
	void *base;
	unsigned long size;
};

static uint flash_verbose;
flash_info_t flash_info[CONFIG_SYS_MAX_FLASH_BANKS];	/* FLASH chips info */

static void altera_qspi_get_locked_range(struct mtd_info *mtd, loff_t *ofs,
					 uint64_t *len);

void flash_print_info(flash_info_t *info)
{
	struct mtd_info *mtd = info->mtd;
	loff_t ofs;
	u64 len;

	printf("Altera QSPI flash  Size: %ld MB in %d Sectors\n",
	       info->size >> 20, info->sector_count);
	altera_qspi_get_locked_range(mtd, &ofs, &len);
	printf("  %08lX +%lX", info->start[0], info->size);
	if (len) {
		printf(", protected %08llX +%llX",
		       info->start[0] + ofs, len);
	}
	putc('\n');
}

void flash_set_verbose(uint v)
{
	flash_verbose = v;
}

int flash_erase(flash_info_t *info, int s_first, int s_last)
{
	struct mtd_info *mtd = info->mtd;
	struct erase_info instr;
	int ret;

	memset(&instr, 0, sizeof(instr));
	instr.mtd = mtd;
	instr.addr = mtd->erasesize * s_first;
	instr.len = mtd->erasesize * (s_last + 1 - s_first);
	flash_set_verbose(1);
	ret = mtd_erase(mtd, &instr);
	flash_set_verbose(0);
	if (ret)
		return ERR_PROTECTED;

	puts(" done\n");
	return 0;
}

int write_buff(flash_info_t *info, uchar *src, ulong addr, ulong cnt)
{
	struct mtd_info *mtd = info->mtd;
	struct udevice *dev = mtd->dev;
	struct altera_qspi_plat *pdata = dev_get_plat(dev);
	ulong base = (ulong)pdata->base;
	loff_t to = addr - base;
	size_t retlen;
	int ret;

	ret = mtd_write(mtd, to, cnt, &retlen, src);
	if (ret)
		return ERR_PROTECTED;

	return 0;
}

unsigned long flash_init(void)
{
	struct udevice *dev;

	/* probe every MTD device */
	for (uclass_first_device(UCLASS_MTD, &dev);
	     dev;
	     uclass_next_device(&dev)) {
	}

	return flash_info[0].size;
}

static int altera_qspi_erase(struct mtd_info *mtd, struct erase_info *instr)
{
	struct udevice *dev = mtd->dev;
	struct altera_qspi_plat *pdata = dev_get_plat(dev);
	struct altera_qspi_regs *regs = pdata->regs;
	size_t addr = instr->addr;
	size_t len = instr->len;
	size_t end = addr + len;
	u32 sect;
	u32 stat;
	u32 *flash, *last;

	instr->state = MTD_ERASING;
	addr &= ~(mtd->erasesize - 1); /* get lower aligned address */
	while (addr < end) {
		if (ctrlc()) {
			if (flash_verbose)
				putc('\n');
			instr->fail_addr = MTD_FAIL_ADDR_UNKNOWN;
			instr->state = MTD_ERASE_FAILED;
			return -EIO;
		}
		flash = pdata->base + addr;
		last = pdata->base + addr + mtd->erasesize;
		/* skip erase if sector is blank */
		while (flash < last) {
			if (readl(flash) != 0xffffffff)
				break;
			flash++;
		}
		if (flash < last) {
			sect = addr / mtd->erasesize;
			sect <<= 8;
			sect |= QUADSPI_MEM_OP_SECTOR_ERASE;
			debug("erase %08x\n", sect);
			writel(sect, &regs->mem_op);
			stat = readl(&regs->isr);
			if (stat & QUADSPI_ISR_ILLEGAL_ERASE) {
				/* erase failed, sector might be protected */
				debug("erase %08x fail %x\n", sect, stat);
				writel(stat, &regs->isr); /* clear isr */
				instr->fail_addr = addr;
				instr->state = MTD_ERASE_FAILED;
				return -EIO;
			}
			if (flash_verbose)
				putc('.');
		} else {
			if (flash_verbose)
				putc(',');
		}
		addr += mtd->erasesize;
	}
	instr->state = MTD_ERASE_DONE;

	return 0;
}

static int altera_qspi_read(struct mtd_info *mtd, loff_t from, size_t len,
			    size_t *retlen, u_char *buf)
{
	struct udevice *dev = mtd->dev;
	struct altera_qspi_plat *pdata = dev_get_plat(dev);

	memcpy_fromio(buf, pdata->base + from, len);
	*retlen = len;

	return 0;
}

static int altera_qspi_write(struct mtd_info *mtd, loff_t to, size_t len,
			     size_t *retlen, const u_char *buf)
{
	struct udevice *dev = mtd->dev;
	struct altera_qspi_plat *pdata = dev_get_plat(dev);
	struct altera_qspi_regs *regs = pdata->regs;
	u32 stat;

	memcpy_toio(pdata->base + to, buf, len);
	/* check whether write triggered a illegal write interrupt */
	stat = readl(&regs->isr);
	if (stat & QUADSPI_ISR_ILLEGAL_WRITE) {
		/* write failed, sector might be protected */
		debug("write fail %x\n", stat);
		writel(stat, &regs->isr); /* clear isr */
		return -EIO;
	}
	*retlen = len;

	return 0;
}

static void altera_qspi_sync(struct mtd_info *mtd)
{
}

static void altera_qspi_get_locked_range(struct mtd_info *mtd, loff_t *ofs,
					 uint64_t *len)
{
	struct udevice *dev = mtd->dev;
	struct altera_qspi_plat *pdata = dev_get_plat(dev);
	struct altera_qspi_regs *regs = pdata->regs;
	int shift0 = ffs(QUADSPI_SR_BP2_0) - 1;
	int shift3 = ffs(QUADSPI_SR_BP3) - 1 - 3;
	u32 stat = readl(&regs->rd_status);
	unsigned pow = ((stat & QUADSPI_SR_BP2_0) >> shift0) |
		((stat & QUADSPI_SR_BP3) >> shift3);

	*ofs = 0;
	*len = 0;
	if (pow) {
		*len = mtd->erasesize << (pow - 1);
		if (*len > mtd->size)
			*len = mtd->size;
		if (!(stat & QUADSPI_SR_TB))
			*ofs = mtd->size - *len;
	}
}

static int altera_qspi_lock(struct mtd_info *mtd, loff_t ofs, uint64_t len)
{
	struct udevice *dev = mtd->dev;
	struct altera_qspi_plat *pdata = dev_get_plat(dev);
	struct altera_qspi_regs *regs = pdata->regs;
	u32 sector_start, sector_end;
	u32 num_sectors;
	u32 mem_op;
	u32 sr_bp;
	u32 sr_tb;

	num_sectors = mtd->size / mtd->erasesize;
	sector_start = ofs / mtd->erasesize;
	sector_end = (ofs + len) / mtd->erasesize;

	if (sector_start >= num_sectors / 2) {
		sr_bp = fls(num_sectors - 1 - sector_start) + 1;
		sr_tb = 0;
	} else if (sector_end < num_sectors / 2) {
		sr_bp = fls(sector_end) + 1;
		sr_tb = 1;
	} else {
		sr_bp = 15;
		sr_tb = 0;
	}

	mem_op = (sr_tb << 12) | (sr_bp << 8);
	mem_op |= QUADSPI_MEM_OP_SECTOR_PROTECT;
	debug("lock %08x\n", mem_op);
	writel(mem_op, &regs->mem_op);

	return 0;
}

static int altera_qspi_unlock(struct mtd_info *mtd, loff_t ofs, uint64_t len)
{
	struct udevice *dev = mtd->dev;
	struct altera_qspi_plat *pdata = dev_get_plat(dev);
	struct altera_qspi_regs *regs = pdata->regs;
	u32 mem_op;

	mem_op = QUADSPI_MEM_OP_SECTOR_PROTECT;
	debug("unlock %08x\n", mem_op);
	writel(mem_op, &regs->mem_op);

	return 0;
}

static int altera_qspi_probe(struct udevice *dev)
{
	struct altera_qspi_plat *pdata = dev_get_plat(dev);
	struct altera_qspi_regs *regs = pdata->regs;
	unsigned long base = (unsigned long)pdata->base;
	struct mtd_info *mtd;
	flash_info_t *flash = &flash_info[0];
	u32 rdid;
	int i;

	rdid = readl(&regs->rd_rdid);
	debug("rdid %x\n", rdid);

	mtd = dev_get_uclass_priv(dev);
	mtd->dev = dev;
	mtd->name		= "nor0";
	mtd->type		= MTD_NORFLASH;
	mtd->flags		= MTD_CAP_NORFLASH;
	mtd->size		= 1 << ((rdid & 0xff) - 6);
	mtd->writesize		= 1;
	mtd->writebufsize	= mtd->writesize;
	mtd->_erase		= altera_qspi_erase;
	mtd->_read		= altera_qspi_read;
	mtd->_write		= altera_qspi_write;
	mtd->_sync		= altera_qspi_sync;
	mtd->_lock		= altera_qspi_lock;
	mtd->_unlock		= altera_qspi_unlock;
	mtd->numeraseregions = 0;
	mtd->erasesize = 0x10000;
	if (add_mtd_device(mtd))
		return -ENOMEM;

	flash->mtd = mtd;
	flash->size = mtd->size;
	flash->sector_count = mtd->size / mtd->erasesize;
	flash->flash_id = rdid;
	flash->start[0] = base;
	for (i = 1; i < flash->sector_count; i++)
		flash->start[i] = flash->start[i - 1] + mtd->erasesize;
	gd->bd->bi_flashstart = base;

	return 0;
}

static int altera_qspi_of_to_plat(struct udevice *dev)
{
	struct altera_qspi_plat *pdata = dev_get_plat(dev);
	void *blob = (void *)gd->fdt_blob;
	int node = dev_of_offset(dev);
	const char *list, *end;
	const fdt32_t *cell;
	void *base;
	unsigned long addr, size;
	int parent, addrc, sizec;
	int len, idx;

	/*
	 * decode regs. there are multiple reg tuples, and they need to
	 * match with reg-names.
	 */
	parent = fdt_parent_offset(blob, node);
	fdt_support_default_count_cells(blob, parent, &addrc, &sizec);
	list = fdt_getprop(blob, node, "reg-names", &len);
	if (!list)
		return -ENOENT;
	end = list + len;
	cell = fdt_getprop(blob, node, "reg", &len);
	if (!cell)
		return -ENOENT;
	idx = 0;
	while (list < end) {
		addr = fdt_translate_address((void *)blob,
					     node, cell + idx);
		size = fdt_addr_to_cpu(cell[idx + addrc]);
		base = map_physmem(addr, size, MAP_NOCACHE);
		len = strlen(list);
		if (strcmp(list, "avl_csr") == 0) {
			pdata->regs = base;
		} else if (strcmp(list, "avl_mem") == 0) {
			pdata->base = base;
			pdata->size = size;
		}
		idx += addrc + sizec;
		list += (len + 1);
	}

	return 0;
}

static const struct udevice_id altera_qspi_ids[] = {
	{ .compatible = "altr,quadspi-1.0" },
	{}
};

U_BOOT_DRIVER(altera_qspi) = {
	.name	= "altera_qspi",
	.id	= UCLASS_MTD,
	.of_match = altera_qspi_ids,
	.of_to_plat = altera_qspi_of_to_plat,
	.plat_auto	= sizeof(struct altera_qspi_plat),
	.probe	= altera_qspi_probe,
};

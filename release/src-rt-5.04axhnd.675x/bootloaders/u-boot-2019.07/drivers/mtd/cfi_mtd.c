// SPDX-License-Identifier: GPL-2.0+
/*
 * (C) Copyright 2008 Semihalf
 *
 * Written by: Piotr Ziecik <kosmo@semihalf.com>
 */

#include <common.h>
#include <flash.h>
#include <malloc.h>

#include <linux/errno.h>
#include <linux/mtd/mtd.h>
#include <linux/mtd/concat.h>
#include <mtd/cfi_flash.h>

static struct mtd_info cfi_mtd_info[CFI_MAX_FLASH_BANKS];
static char cfi_mtd_names[CFI_MAX_FLASH_BANKS][16];
#ifdef CONFIG_MTD_CONCAT
static char c_mtd_name[16];
#endif

static int cfi_mtd_erase(struct mtd_info *mtd, struct erase_info *instr)
{
	flash_info_t *fi = mtd->priv;
	size_t a_start = fi->start[0] + instr->addr;
	size_t a_end = a_start + instr->len;
	int s_first = -1;
	int s_last = -1;
	int error, sect;

	for (sect = 0; sect < fi->sector_count; sect++) {
		if (a_start == fi->start[sect])
			s_first = sect;

		if (sect < fi->sector_count - 1) {
			if (a_end == fi->start[sect + 1]) {
				s_last = sect;
				break;
			}
		} else {
			s_last = sect;
			break;
		}
	}

	if (s_first >= 0 && s_first <= s_last) {
		instr->state = MTD_ERASING;

		flash_set_verbose(0);
		error = flash_erase(fi, s_first, s_last);
		flash_set_verbose(1);

		if (error) {
			instr->state = MTD_ERASE_FAILED;
			return -EIO;
		}

		instr->state = MTD_ERASE_DONE;
		mtd_erase_callback(instr);
		return 0;
	}

	return -EINVAL;
}

static int cfi_mtd_read(struct mtd_info *mtd, loff_t from, size_t len,
	size_t *retlen, u_char *buf)
{
	flash_info_t *fi = mtd->priv;
	u_char *f = (u_char*)(fi->start[0]) + from;

	memcpy(buf, f, len);
	*retlen = len;

	return 0;
}

static int cfi_mtd_write(struct mtd_info *mtd, loff_t to, size_t len,
	size_t *retlen, const u_char *buf)
{
	flash_info_t *fi = mtd->priv;
	u_long t = fi->start[0] + to;
	int error;

	flash_set_verbose(0);
	error = write_buff(fi, (u_char*)buf, t, len);
	flash_set_verbose(1);

	if (!error) {
		*retlen = len;
		return 0;
	}

	return -EIO;
}

static void cfi_mtd_sync(struct mtd_info *mtd)
{
	/*
	 * This function should wait until all pending operations
	 * finish. However this driver is fully synchronous, so
	 * this function returns immediately
	 */
}

static int cfi_mtd_lock(struct mtd_info *mtd, loff_t ofs, uint64_t len)
{
	flash_info_t *fi = mtd->priv;

	flash_set_verbose(0);
	flash_protect(FLAG_PROTECT_SET, fi->start[0] + ofs,
					fi->start[0] + ofs + len - 1, fi);
	flash_set_verbose(1);

	return 0;
}

static int cfi_mtd_unlock(struct mtd_info *mtd, loff_t ofs, uint64_t len)
{
	flash_info_t *fi = mtd->priv;

	flash_set_verbose(0);
	flash_protect(FLAG_PROTECT_CLEAR, fi->start[0] + ofs,
					fi->start[0] + ofs + len - 1, fi);
	flash_set_verbose(1);

	return 0;
}

static int cfi_mtd_set_erasesize(struct mtd_info *mtd, flash_info_t *fi)
{
	int sect_size = 0;
	int sect_size_old = 0;
	int sect;
	int regions = 0;
	int numblocks = 0;
	ulong offset;
	ulong base_addr;

	/*
	 * First detect the number of eraseregions so that we can allocate
	 * the array of eraseregions correctly
	 */
	for (sect = 0; sect < fi->sector_count; sect++) {
		if (sect_size_old != flash_sector_size(fi, sect))
			regions++;
		sect_size_old = flash_sector_size(fi, sect);
	}

	switch (regions) {
	case 0:
		return 1;
	case 1:	/* flash has uniform erase size */
		mtd->numeraseregions = 0;
		mtd->erasesize = sect_size_old;
		return 0;
	}

	mtd->numeraseregions = regions;
	mtd->eraseregions = malloc(sizeof(struct mtd_erase_region_info) * regions);

	/*
	 * Now detect the largest sector and fill the eraseregions
	 */
	regions = 0;
	base_addr = offset = fi->start[0];
	sect_size_old = flash_sector_size(fi, 0);
	for (sect = 0; sect < fi->sector_count; sect++) {
		if (sect_size_old != flash_sector_size(fi, sect)) {
			mtd->eraseregions[regions].offset = offset - base_addr;
			mtd->eraseregions[regions].erasesize = sect_size_old;
			mtd->eraseregions[regions].numblocks = numblocks;
			/* Now start counting the next eraseregions */
			numblocks = 0;
			regions++;
			offset = fi->start[sect];
		}
		numblocks++;

		/*
		 * Select the largest sector size as erasesize (e.g. for UBI)
		 */
		if (flash_sector_size(fi, sect) > sect_size)
			sect_size = flash_sector_size(fi, sect);

		sect_size_old = flash_sector_size(fi, sect);
	}

	/*
	 * Set the last region
	 */
	mtd->eraseregions[regions].offset = offset - base_addr;
	mtd->eraseregions[regions].erasesize = sect_size_old;
	mtd->eraseregions[regions].numblocks = numblocks;

	mtd->erasesize = sect_size;

	return 0;
}

int cfi_mtd_init(void)
{
	struct mtd_info *mtd;
	flash_info_t *fi;
	int error, i;
#ifdef CONFIG_MTD_CONCAT
	int devices_found = 0;
	struct mtd_info *mtd_list[CONFIG_SYS_MAX_FLASH_BANKS];
#endif

	for (i = 0; i < CONFIG_SYS_MAX_FLASH_BANKS; i++) {
		fi = &flash_info[i];
		mtd = &cfi_mtd_info[i];

		memset(mtd, 0, sizeof(struct mtd_info));

		error = cfi_mtd_set_erasesize(mtd, fi);
		if (error)
			continue;

		sprintf(cfi_mtd_names[i], "nor%d", i);
		mtd->name		= cfi_mtd_names[i];
		mtd->type		= MTD_NORFLASH;
		mtd->flags		= MTD_CAP_NORFLASH;
		mtd->size		= fi->size;
		mtd->writesize		= 1;
		mtd->writebufsize	= mtd->writesize;

		mtd->_erase		= cfi_mtd_erase;
		mtd->_read		= cfi_mtd_read;
		mtd->_write		= cfi_mtd_write;
		mtd->_sync		= cfi_mtd_sync;
		mtd->_lock		= cfi_mtd_lock;
		mtd->_unlock		= cfi_mtd_unlock;
		mtd->priv		= fi;

		if (add_mtd_device(mtd))
			return -ENOMEM;

#ifdef CONFIG_MTD_CONCAT
		mtd_list[devices_found++] = mtd;
#endif
	}

#ifdef CONFIG_MTD_CONCAT
	if (devices_found > 1) {
		/*
		 * We detected multiple devices. Concatenate them together.
		 */
		sprintf(c_mtd_name, "nor%d", devices_found);
		mtd = mtd_concat_create(mtd_list, devices_found, c_mtd_name);

		if (mtd == NULL)
			return -ENXIO;

		if (add_mtd_device(mtd))
			return -ENOMEM;
	}
#endif /* CONFIG_MTD_CONCAT */

	return 0;
}

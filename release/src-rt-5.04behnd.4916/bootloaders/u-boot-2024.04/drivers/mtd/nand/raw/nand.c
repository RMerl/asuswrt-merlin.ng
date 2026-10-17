// SPDX-License-Identifier: GPL-2.0
/*
 * (C) Copyright 2005
 * 2N Telekomunikace, a.s. <www.2n.cz>
 * Ladislav Michl <michl@2n.cz>
 */

#include <common.h>
#include <nand.h>
#include <errno.h>
#include <linux/mtd/concat.h>
#include <linux/mtd/rawnand.h>

#ifndef CFG_SYS_NAND_BASE_LIST
#define CFG_SYS_NAND_BASE_LIST { CFG_SYS_NAND_BASE }
#endif

int nand_curr_device = -1;

static struct mtd_info *nand_info[CONFIG_SYS_MAX_NAND_DEVICE];

#if !CONFIG_IS_ENABLED(SYS_NAND_SELF_INIT)
static struct nand_chip nand_chip[CONFIG_SYS_MAX_NAND_DEVICE];
static ulong base_address[CONFIG_SYS_MAX_NAND_DEVICE] = CFG_SYS_NAND_BASE_LIST;
#endif

static char dev_name[CONFIG_SYS_MAX_NAND_DEVICE][8];

static unsigned long total_nand_size; /* in kiB */

struct mtd_info *get_nand_dev_by_index(int dev)
{
	if (dev < 0 || dev >= CONFIG_SYS_MAX_NAND_DEVICE || !nand_info[dev] ||
	    !nand_info[dev]->name)
		return NULL;

	return nand_info[dev];
}

int nand_mtd_to_devnum(struct mtd_info *mtd)
{
	int i;

	if (!mtd)
		return -ENODEV;

	for (i = 0; i < CONFIG_SYS_MAX_NAND_DEVICE; i++) {
		if (get_nand_dev_by_index(i) == mtd)
			return i;
	}

	return -ENODEV;
}

/* Register an initialized NAND mtd device with the U-Boot NAND command. */
int nand_register(int devnum, struct mtd_info *mtd)
{
	if (!mtd || devnum >= CONFIG_SYS_MAX_NAND_DEVICE)
		return -EINVAL;

	nand_info[devnum] = mtd;

	sprintf(dev_name[devnum], "nand%d", devnum);
	mtd->name = dev_name[devnum];

	/*
	 * Add MTD device so that we can reference it later
	 * via the mtdcore infrastructure (e.g. ubi).
	 */
	add_mtd_device(mtd);

	total_nand_size += mtd->size / 1024;

	if (nand_curr_device == -1)
		nand_curr_device = devnum;

	return 0;
}

void nand_unregister(struct mtd_info *mtd)
{
	int devnum = nand_mtd_to_devnum(mtd);

	if (devnum < 0)
		return;

	if (nand_curr_device == devnum)
		nand_curr_device = -1;

	total_nand_size -= mtd->size / 1024;

	del_mtd_device(nand_info[devnum]);

	nand_info[devnum] = NULL;
}

#if !CONFIG_IS_ENABLED(SYS_NAND_SELF_INIT)
static void nand_init_chip(int i)
{
	struct nand_chip *nand = &nand_chip[i];
	struct mtd_info *mtd = nand_to_mtd(nand);
	ulong base_addr = base_address[i];
	int maxchips = CONFIG_SYS_NAND_MAX_CHIPS;

	if (maxchips < 1)
		maxchips = 1;

	nand->IO_ADDR_R = nand->IO_ADDR_W = (void  __iomem *)base_addr;

	if (board_nand_init(nand))
		return;

	if (nand_scan(mtd, maxchips))
		return;

	nand_register(i, mtd);
}
#endif

#ifdef CONFIG_MTD_CONCAT
struct mtd_info *concat_mtd;

static void create_mtd_concat(void)
{
	struct mtd_info *nand_info_list[CONFIG_SYS_MAX_NAND_DEVICE];
	int nand_devices_found = 0;
	int i;

	for (i = 0; i < CONFIG_SYS_MAX_NAND_DEVICE; i++) {
		struct mtd_info *mtd = get_nand_dev_by_index(i);
		if (mtd != NULL) {
			nand_info_list[nand_devices_found] = mtd;
			nand_devices_found++;
		}
	}
	if (nand_devices_found > 1) {
		char c_mtd_name[16];

		/*
		 * We detected multiple devices. Concatenate them together.
		 */
		sprintf(c_mtd_name, "nand%d", nand_devices_found);
		concat_mtd = mtd_concat_create(nand_info_list,
					       nand_devices_found, c_mtd_name);

		if (!concat_mtd)
			return;

		nand_register(nand_devices_found, concat_mtd);
	}

	return;
}

static void destroy_mtd_concat(void)
{
	if (!concat_mtd)
		return;

	mtd_concat_destroy(concat_mtd);
	concat_mtd = NULL;
}
#else
static void create_mtd_concat(void)
{
}

static void destroy_mtd_concat(void)
{
}
#endif

unsigned long nand_size(void)
{
	return total_nand_size;
}

static int initialized;

void nand_init(void)
{
	/*
	 * Avoid initializing NAND Flash multiple times,
	 * otherwise it will calculate a wrong total size.
	 */
	if (initialized)
		return;
	initialized = 1;

#if CONFIG_IS_ENABLED(SYS_NAND_SELF_INIT)
	board_nand_init();
#else
	int i;

	for (i = 0; i < CONFIG_SYS_MAX_NAND_DEVICE; i++)
		nand_init_chip(i);
#endif

#ifdef CONFIG_SYS_NAND_SELECT_DEVICE
	/*
	 * Select the chip in the board/cpu specific driver
	 */
	board_nand_select_device(mtd_to_nand(get_nand_dev_by_index(nand_curr_device)),
				 nand_curr_device);
#endif

	create_mtd_concat();
}

void nand_reinit(void)
{
	int i;

	destroy_mtd_concat();
	for (i = 0; i < CONFIG_SYS_MAX_NAND_DEVICE; i++)
		assert(!nand_info[i]);

	initialized = 0;
	nand_init();
}

unsigned int nand_page_size(void)
{
	struct mtd_info *mtd = get_nand_dev_by_index(nand_curr_device);

	return mtd ? mtd->writesize : 1;
}

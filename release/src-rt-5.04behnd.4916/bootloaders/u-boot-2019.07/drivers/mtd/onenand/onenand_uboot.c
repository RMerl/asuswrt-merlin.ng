/*
 *  drivers/mtd/onenand/onenand_uboot.c
 *
 *  Copyright (C) 2005-2008 Samsung Electronics
 *  Kyungmin Park <kyungmin.park@samsung.com>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 */

/*
 * OneNAND initialization at U-Boot
 */

#include <common.h>
#include <linux/compat.h>
#include <linux/mtd/mtd.h>
#include <linux/mtd/onenand.h>

struct mtd_info onenand_mtd;
struct onenand_chip onenand_chip;
static __attribute__((unused)) char dev_name[] = "onenand0";

void onenand_init(void)
{
	int err = 0;
	memset(&onenand_mtd, 0, sizeof(struct mtd_info));
	memset(&onenand_chip, 0, sizeof(struct onenand_chip));

	onenand_mtd.priv = &onenand_chip;

#ifdef CONFIG_USE_ONENAND_BOARD_INIT
	/* It's used for some board init required */
	err = onenand_board_init(&onenand_mtd);
#else
	onenand_chip.base = (void *) CONFIG_SYS_ONENAND_BASE;
#endif

	if (!err && !(onenand_scan(&onenand_mtd, 1))) {

		if (onenand_chip.device_id & DEVICE_IS_FLEXONENAND)
			puts("Flex-");
		puts("OneNAND: ");

#ifdef CONFIG_MTD_DEVICE
		/*
		 * Add MTD device so that we can reference it later
		 * via the mtdcore infrastructure (e.g. ubi).
		 */
		onenand_mtd.name = dev_name;
		add_mtd_device(&onenand_mtd);
#endif
	}
	print_size(onenand_chip.chipsize, "\n");
}

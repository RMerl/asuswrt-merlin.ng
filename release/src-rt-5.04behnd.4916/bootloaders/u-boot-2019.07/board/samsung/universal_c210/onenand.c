// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright (C) 2010 Samsung Electronics
 * Kyungmin Park <kyungmin.park@samsung.com>
 */

#include <common.h>
#include <linux/mtd/mtd.h>
#include <linux/mtd/onenand.h>
#include <linux/mtd/samsung_onenand.h>

int onenand_board_init(struct mtd_info *mtd)
{
	struct onenand_chip *this = mtd->priv;

	this->base = (void *)CONFIG_SYS_ONENAND_BASE;
	this->options |= ONENAND_RUNTIME_BADBLOCK_CHECK;
	this->chip_probe = s5pc210_chip_probe;

	return 0;
}

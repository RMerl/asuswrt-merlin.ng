/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * Copyright (C) 2015 Thomas Chou <thomas@wytron.com.tw>
 */

#ifndef _MTD_H_
#define _MTD_H_

#include <dm/device.h>
#include <jffs2/load_kernel.h>
#include <linux/mtd/mtd.h>

int mtd_probe_devices(void);

void board_mtdparts_default(const char **mtdids, const char **mtdparts);

/* compute the max size for the string associated to a dev type */
#define MTD_NAME_SIZE(type) (sizeof(MTD_DEV_TYPE(type)) +  DM_MAX_SEQ_STR)

#endif	/* _MTD_H_ */

/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * Copyright (C) 2015 Thomas Chou <thomas@wytron.com.tw>
 */

#ifndef _MTD_H_
#define _MTD_H_

#include <linux/mtd/mtd.h>

int mtd_probe(struct udevice *dev);
int mtd_probe_devices(void);

#endif	/* _MTD_H_ */

/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * Copyright 2020 Broadcom.
 *
 */

#ifndef BL33_INFO_H
#define BL33_INFO_H
#include <asm/io.h>

/* Increase version number each time this file is modified */
#define BL33_INFO_VERSION	1

struct chip_info {
	unsigned int chip_id;
	unsigned int rev_id;
};

struct bl33_info {
	unsigned int version;
	struct chip_info chip;
};

extern struct bl33_info *bl33_info;

#endif

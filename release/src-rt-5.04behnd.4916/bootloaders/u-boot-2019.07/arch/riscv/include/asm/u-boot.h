/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * (C) Copyright 2002
 * Sysgo Real-Time Solutions, GmbH <www.elinos.com>
 * Marius Groeger <mgroeger@sysgo.de>
 *
 * Copyright (C) 2017 Andes Technology Corporation
 * Rick Chen, Andes Technology Corporation <rick@andestech.com>
 *
 ********************************************************************
 * NOTE: This header file defines an interface to U-Boot. Including
 * this (unmodified) header file in another file is considered normal
 * use of U-Boot, and does *not* fall under the heading of "derived
 * work".
 ********************************************************************
 */

#ifndef _U_BOOT_H_
#define _U_BOOT_H_	1

#include <asm/u-boot-riscv.h>

#include <environment.h>

typedef struct bd_info {
	unsigned long	bi_boot_params;	/* where this board expects params */
	unsigned long	bi_memstart;	/* start of DRAM memory */
	unsigned long	bi_memsize;	/* size	 of DRAM memory in bytes */
	unsigned long	bi_flashstart;	/* start of FLASH memory */
	unsigned long	bi_flashsize;	/* size	 of FLASH memory */
	unsigned long	bi_flashoffset; /* reserved area for startup monitor */
	unsigned char	bi_enetaddr[6];

	struct				/* RAM configuration */
	{
		unsigned long start;
		unsigned long size;
	} bi_dram[CONFIG_NR_DRAM_BANKS];
} bd_t;

/* For image.h:image_check_target_arch() */
#define IH_ARCH_DEFAULT IH_ARCH_RISCV

#endif	/* _U_BOOT_H_ */

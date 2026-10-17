/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * Copyright (c) 2011 The Chromium OS Authors.
 *
 * (C) Copyright 2002-2010
 * Wolfgang Denk, DENX Software Engineering, wd@denx.de.
 */

#ifndef	__ASM_GBL_DATA_H
#define __ASM_GBL_DATA_H

#include <linux/types.h>

/* Architecture-specific global data */
struct arch_global_data {
	uint8_t		*ram_buf;	/* emulated RAM buffer */
	void		*text_base;	/* pointer to base of text region */
	ulong table_start;		/* Start address of x86 tables */
	ulong table_end;		/* End address of x86 tables */
	ulong table_start_high;		/* Start address of high x86 tables */
	ulong table_end_high;		/* End address of high x86 tables */
	ulong smbios_start;		/* Start address of SMBIOS table */
};

#include <asm-generic/global_data.h>

#define DECLARE_GLOBAL_DATA_PTR     extern gd_t *gd

#endif /* __ASM_GBL_DATA_H */

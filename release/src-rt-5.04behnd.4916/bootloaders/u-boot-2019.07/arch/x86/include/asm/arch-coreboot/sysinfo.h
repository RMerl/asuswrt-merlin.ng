/* SPDX-License-Identifier: BSD-3-Clause */
/*
 * This file is part of the libpayload project.
 *
 * Copyright (C) 2008 Advanced Micro Devices, Inc.
 */

#ifndef _COREBOOT_SYSINFO_H
#define _COREBOOT_SYSINFO_H

#include <asm/coreboot_tables.h>

/* Maximum number of memory range definitions */
#define SYSINFO_MAX_MEM_RANGES	32
/* Allow a maximum of 8 GPIOs */
#define SYSINFO_MAX_GPIOS	8

struct sysinfo_t {
	int n_memranges;
	struct memrange {
		unsigned long long base;
		unsigned long long size;
		unsigned int type;
	} memrange[SYSINFO_MAX_MEM_RANGES];

	u32 cmos_range_start;
	u32 cmos_range_end;
	u32 cmos_checksum_location;
	u32 vbnv_start;
	u32 vbnv_size;

	char *version;
	char *extra_version;
	char *build;
	char *compile_time;
	char *compile_by;
	char *compile_host;
	char *compile_domain;
	char *compiler;
	char *linker;
	char *assembler;

	struct cb_framebuffer *framebuffer;

	int num_gpios;
	struct cb_gpio gpios[SYSINFO_MAX_GPIOS];

	void	*vdat_addr;
	u32	vdat_size;
	void	*tstamp_table;
	void	*cbmem_cons;

	struct cb_serial *serial;
};

extern struct sysinfo_t lib_sysinfo;

int get_coreboot_info(struct sysinfo_t *info);

#endif

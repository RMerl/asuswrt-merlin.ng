/* SPDX-License-Identifier: GPL-2.0 */
/*
 * ifdtool - Manage Intel Firmware Descriptor information
 *
 * Copyright (C) 2011 The ChromiumOS Authors.
 *
 * From Coreboot project
 */

#include <stdint.h>

#define __packed	__attribute__((packed))

#define IFDTOOL_VERSION "1.1-U-Boot"

#define WRITE_MAX	16

enum spi_frequency {
	SPI_FREQUENCY_20MHZ = 0,
	SPI_FREQUENCY_33MHZ = 1,
	SPI_FREQUENCY_50MHZ = 4,
};

enum component_density {
	COMPONENT_DENSITY_512KB = 0,
	COMPONENT_DENSITY_1MB   = 1,
	COMPONENT_DENSITY_2MB   = 2,
	COMPONENT_DENSITY_4MB   = 3,
	COMPONENT_DENSITY_8MB   = 4,
	COMPONENT_DENSITY_16MB  = 5,
};

/* flash descriptor */
struct __packed fdbar_t {
	uint32_t flvalsig;
	uint32_t flmap0;
	uint32_t flmap1;
	uint32_t flmap2;
	uint8_t  reserved[0xefc - 0x20];
	uint32_t flumap1;
};

#define MAX_REGIONS	5

/* regions */
struct __packed frba_t {
	uint32_t flreg[MAX_REGIONS];
};

/* component section */
struct __packed fcba_t {
	uint32_t flcomp;
	uint32_t flill;
	uint32_t flpb;
};

#define MAX_STRAPS	18

/* pch strap */
struct __packed fpsba_t {
	uint32_t pchstrp[MAX_STRAPS];
};

/* master */
struct __packed fmba_t {
	uint32_t flmstr1;
	uint32_t flmstr2;
	uint32_t flmstr3;
};

/* processor strap */
struct __packed fmsba_t {
	uint32_t data[8];
};

/* ME VSCC */
struct vscc_t {
	uint32_t jid;
	uint32_t vscc;
};

struct vtba_t {
	/* Actual number of entries specified in vtl */
	struct vscc_t entry[8];
};

struct region_t {
	int base, limit, size;
};

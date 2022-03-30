/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * Copyright (C) 2012 Boundary Devices Inc.
 */

#ifndef _ASM_BOOT_MODE_H
#define _ASM_BOOT_MODE_H
#define MAKE_CFGVAL(cfg1, cfg2, cfg3, cfg4) \
	((cfg4) << 24) | ((cfg3) << 16) | ((cfg2) << 8) | (cfg1)

enum boot_device {
	WEIM_NOR_BOOT,
	ONE_NAND_BOOT,
	PATA_BOOT,
	SATA_BOOT,
	I2C_BOOT,
	SPI_NOR_BOOT,
	SD1_BOOT,
	SD2_BOOT,
	SD3_BOOT,
	SD4_BOOT,
	MMC1_BOOT,
	MMC2_BOOT,
	MMC3_BOOT,
	MMC4_BOOT,
	NAND_BOOT,
	QSPI_BOOT,
	FLEXSPI_BOOT,
	USB_BOOT,
	UNKNOWN_BOOT,
	BOOT_DEV_NUM = UNKNOWN_BOOT,
};

/* Boot device type */
#define BOOT_TYPE_SD		0x1
#define BOOT_TYPE_MMC		0x2
#define BOOT_TYPE_NAND		0x3
#define BOOT_TYPE_QSPI		0x4
#define BOOT_TYPE_WEIM		0x5
#define BOOT_TYPE_SPINOR	0x6
#define BOOT_TYPE_USB		0xF

struct boot_mode {
	const char *name;
	unsigned cfg_val;
};

void add_board_boot_modes(const struct boot_mode *p);
void boot_mode_apply(unsigned cfg_val);
extern const struct boot_mode soc_boot_modes[];
#endif

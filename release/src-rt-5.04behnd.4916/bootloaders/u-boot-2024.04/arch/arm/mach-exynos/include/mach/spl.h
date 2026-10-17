/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * Copyright (c) 2012 The Chromium OS Authors.
 */

#ifndef __ASM_ARCH_EXYNOS_SPL_H__
#define __ASM_ARCH_EXYNOS_SPL_H__

#include <asm/arch/dmc.h>
#include <asm/arch/power.h>

#ifndef __ASSEMBLY__
/* Parameters of early board initialization in SPL */
struct spl_machine_param {
	/* Add fields as and when required */
	u32		signature;
	u32		version;	/* Version number */
	u32		size;		/* Size of block */
	/**
	 * Parameters we expect, in order, terminated with \0. Each parameter
	 * is a single character representing one 32-bit word in this
	 * structure.
	 *
	 * Valid characters in this string are:
	 *
	 * Code		Name
	 * v		mem_iv_size
	 * m		mem_type
	 * u		uboot_size
	 * b		boot_source
	 * f		frequency_mhz (memory frequency in MHz)
	 * a		ARM clock frequency in MHz
	 * s		serial base address
	 * i		i2c base address for early access (meant for PMIC)
	 * r		board rev GPIO numbers used to read board revision
	 *			(lower halfword=bit 0, upper=bit 1)
	 * M		Memory Manufacturer name
	 * \0		termination
	 */
	char		params[12];	/* Length must be word-aligned */
	u32		mem_iv_size;	/* Memory channel interleaving size */
	enum ddr_mode	mem_type;	/* Type of on-board memory */
	/*
	 * U-Boot size - The iROM mmc copy function used by the SPL takes a
	 * block count paramter to describe the U-Boot size unlike the spi
	 * boot copy function which just uses the U-Boot size directly. Align
	 * the U-Boot size to block size (512 bytes) when populating the SPL
	 * table only for mmc boot.
	 */
	u32		uboot_size;
	unsigned	boot_source;	/* Boot device */
	unsigned	frequency_mhz;	/* Frequency of memory in MHz */
	unsigned	arm_freq_mhz;	/* ARM Frequency in MHz */
	u32		serial_base;	/* Serial base address */
	u32		i2c_base;	/* i2c base address */
	u32		board_rev_gpios;	/* Board revision GPIOs */
	enum mem_manuf	mem_manuf;	/* Memory Manufacturer */
} __attribute__((__packed__));
#endif

/**
 * Validate signature and return a pointer to the parameter table.  If the
 * signature is invalid, call panic() and never return.
 *
 * Return: pointer to the parameter table if signature matched or never return.
 */
struct spl_machine_param *spl_get_machine_params(void);

#endif /* __ASM_ARCH_EXYNOS_SPL_H__ */

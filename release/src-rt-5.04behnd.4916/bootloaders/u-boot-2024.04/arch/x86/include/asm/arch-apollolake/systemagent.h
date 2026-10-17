/* SPDX-License-Identifier: GPL-2.0 */
/*
 * Copyright (C) 2017 Intel Corporation.
 * Take from coreboot project file of the same name
 */

#ifndef _ASM_ARCH_SYSTEMAGENT_H
#define _ASM_ARCH_SYSTEMAGENT_H

/* Device 0:0.0 PCI configuration space */
#include <linux/bitops.h>

struct udevice;

#define MCHBAR		0x48

/* RAPL Package Power Limit register under MCHBAR */
#define PUNIT_THERMAL_DEVICE_IRQ		0x700C
#define PUINT_THERMAL_DEVICE_IRQ_VEC_NUMBER	0x18
#define PUINT_THERMAL_DEVICE_IRQ_LOCK		0x80000000
#define BIOS_RESET_CPL		0x7078
#define   PCODE_INIT_DONE	BIT(8)
#define MCHBAR_RAPL_PPL		0x70A8
#define CORE_DISABLE_MASK	0x7168
#define CAPID0_A		0xE4
#define   VTD_DISABLE		BIT(23)
#define DEFVTBAR		0x6c80
#define GFXVTBAR		0x6c88
#define   VTBAR_ENABLED		0x01
#define VTBAR_MASK		GENMASK_ULL(39, 12)
#define VTBAR_SIZE		0x1000

/**
 * enable_bios_reset_cpl() - Tell the system agent that memory/power are ready
 *
 * This should be called when U-Boot has set up the memory and power
 * management.
 */
void enable_bios_reset_cpl(void);

/**
 * sa_get_tolud_base() - Get the TOLUD base address
 *
 * This returns the Top Of Low Useable DRAM, marking the top of usable DRAM
 * below 4GB
 *
 * @dev: hostbridge device
 * Return: TOLUD address
 */
ulong sa_get_tolud_base(struct udevice *dev);

/**
 * sa_get_gsm_base() - Get the GSM base address
 *
 * This returns the base of GTT Stolen Memory, marking the start of memory used
 * for Graphics Translation Tables.
 *
 * @dev: hostbridge device
 * Return: GSM address
 */
ulong sa_get_gsm_base(struct udevice *dev);

/**
 * sa_get_tseg_base() - Get the TSEG base address
 *
 * This returns the top address of DRAM available below 4GB
 *
 * Return: TSEG base
 */
ulong sa_get_tseg_base(struct udevice *dev);

#endif

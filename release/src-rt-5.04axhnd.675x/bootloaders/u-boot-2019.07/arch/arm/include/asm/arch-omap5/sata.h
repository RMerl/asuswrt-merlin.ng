/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * SATA Wrapper Register map
 *
 * (C) Copyright 2013
 * Texas Instruments, <www.ti.com>
 */

#ifndef _TI_SATA_H
#define _TI_SATA_H

/* SATA Wrapper module */
#define TI_SATA_WRAPPER_BASE		(OMAP54XX_L4_CORE_BASE + 0x141100)
/* SATA PHY Module */
#define TI_SATA_PLLCTRL_BASE		(OMAP54XX_L4_CORE_BASE + 0x96800)

/* SATA Wrapper register offsets */
#define TI_SATA_SYSCONFIG			0x00
#define TI_SATA_CDRLOCK				0x04

/* Register Set */
#define TI_SATA_SYSCONFIG_OVERRIDE0		(1 << 16)
#define TI_SATA_SYSCONFIG_STANDBY_MASK		(0x3 << 4)
#define TI_SATA_SYSCONFIG_IDLE_MASK		(0x3 << 2)

/* Standby modes */
#define TI_SATA_STANDBY_FORCE			0x0
#define TI_SATA_STANDBY_NO			(0x1 << 4)
#define TI_SATA_STANDBY_SMART_WAKE		(0x3 << 4)
#define TI_SATA_STANDBY_SMART			(0x2 << 4)

/* Idle modes */
#define TI_SATA_IDLE_FORCE			0x0
#define TI_SATA_IDLE_NO				(0x1 << 2)
#define TI_SATA_IDLE_SMART_WAKE			(0x3 << 2)
#define TI_SATA_IDLE_SMART			(0x2 << 2)

#endif /* _TI_SATA_H */

/*
 * hardware_ti816x.h
 *
 * TI816x hardware specific header
 *
 * Copyright (C) 2013, Adeneo Embedded <www.adeneo-embedded.com>
 * Antoine Tenart, <atenart@adeneo-embedded.com>
 * Based on TI-PSP-04.00.02.14
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation; either version 2 of
 * the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR /PURPOSE.  See the
 * GNU General Public License for more details.
 */

#ifndef __AM33XX_HARDWARE_TI816X_H
#define __AM33XX_HARDWARE_TI816X_H

/* UART */
#define UART0_BASE		0x48020000
#define UART1_BASE		0x48022000
#define UART2_BASE		0x48024000

/* Watchdog Timer */
#define WDT_BASE		0x480C2000

/* Control Module Base Address */
#define CTRL_BASE		0x48140000
#define CTRL_DEVICE_BASE	0x48140600

/* PRCM Base Address */
#define PRCM_BASE		0x48180000

#define PRM_RSTCTRL		(PRCM_BASE + 0x00A0)
#define PRM_RSTST		(PRM_RSTCTRL + 8)

/* VTP Base address */
#define VTP0_CTRL_ADDR		0x48198358
#define VTP1_CTRL_ADDR		0x4819A358

/* DDR Base address */
#define DDR_PHY_CMD_ADDR	0x48198000
#define DDR_PHY_DATA_ADDR	0x481980C8
#define DDR_PHY_CMD_ADDR2	0x4819A000
#define DDR_PHY_DATA_ADDR2	0x4819A0C8
#define DDR_DATA_REGS_NR	4


#define DDRPHY_0_CONFIG_BASE	0x48198000
#define DDRPHY_1_CONFIG_BASE	0x4819A000
#define DDRPHY_CONFIG_BASE	((emif == 0) ? \
	DDRPHY_0_CONFIG_BASE : DDRPHY_1_CONFIG_BASE)

/* RTC base address */
#define RTC_BASE		0x480C0000

#endif /* __AM33XX_HARDWARE_TI816X_H */

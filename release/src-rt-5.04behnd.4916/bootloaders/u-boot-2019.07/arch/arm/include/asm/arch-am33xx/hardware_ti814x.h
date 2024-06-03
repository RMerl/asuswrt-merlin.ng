/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * hardware_ti814x.h
 *
 * TI814x hardware specific header
 *
 * Copyright (C) 2013, Texas Instruments, Incorporated - http://www.ti.com/
 */

#ifndef __AM33XX_HARDWARE_TI814X_H
#define __AM33XX_HARDWARE_TI814X_H

/* Module base addresses */

/* UART Base Address */
#define UART0_BASE			0x48020000

/* Watchdog Timer */
#define WDT_BASE			0x481C7000

/* Control Module Base Address */
#define CTRL_BASE			0x48140000
#define CTRL_DEVICE_BASE		0x48140600

/* PRCM Base Address */
#define PRCM_BASE			0x48180000
#define CM_PER				0x44E00000
#define CM_WKUP				0x44E00400

#define PRM_RSTCTRL			(PRCM_BASE + 0x00A0)
#define PRM_RSTST			(PRM_RSTCTRL + 8)

/* PLL Subsystem Base Address */
#define PLL_SUBSYS_BASE			0x481C5000

/* VTP Base address */
#define VTP0_CTRL_ADDR			0x48140E0C
#define VTP1_CTRL_ADDR			0x48140E10

/* DDR Base address */
#define DDR_PHY_CMD_ADDR		0x47C0C400
#define DDR_PHY_DATA_ADDR		0x47C0C4C8
#define DDR_PHY_CMD_ADDR2		0x47C0C800
#define DDR_PHY_DATA_ADDR2		0x47C0C8C8
#define DDR_DATA_REGS_NR		4

#define DDRPHY_0_CONFIG_BASE		(CTRL_BASE + 0x1400)
#define DDRPHY_CONFIG_BASE		DDRPHY_0_CONFIG_BASE

/* CPSW Config space */
#define CPSW_MDIO_BASE			0x4A100800

/* RTC base address */
#define RTC_BASE			0x480C0000

/* OTG */
#define USB0_OTG_BASE			0x47401000
#define USB1_OTG_BASE			0x47401800

#endif /* __AM33XX_HARDWARE_TI814X_H */

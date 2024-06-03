/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * hardware_am33xx.h
 *
 * AM33xx hardware specific header
 *
 * Copyright (C) 2013, Texas Instruments, Incorporated - http://www.ti.com/
 */

#ifndef __AM33XX_HARDWARE_AM33XX_H
#define __AM33XX_HARDWARE_AM33XX_H

/* Module base addresses */

/* UART Base Address */
#define UART0_BASE			0x44E09000
#define UART1_BASE			0x48022000
#define UART2_BASE			0x48024000
#define UART3_BASE			0x481A6000
#define UART4_BASE			0x481A8000
#define UART5_BASE			0x481AA000

/* GPIO Base address */
#define GPIO2_BASE			0x481AC000

/* Watchdog Timer */
#define WDT_BASE			0x44E35000

/* Control Module Base Address */
#define CTRL_BASE			0x44E10000
#define CTRL_DEVICE_BASE		0x44E10600

/* PRCM Base Address */
#define PRCM_BASE			0x44E00000
#define CM_PER				0x44E00000
#define CM_WKUP				0x44E00400
#define CM_DPLL				0x44E00500
#define CM_RTC				0x44E00800

#define PRM_RSTCTRL			(PRCM_BASE + 0x0F00)
#define PRM_RSTST			(PRM_RSTCTRL + 8)

/* VTP Base address */
#define VTP0_CTRL_ADDR			0x44E10E0C
#define VTP1_CTRL_ADDR			0x48140E10
#define PRM_DEVICE_INST			0x44E00F00

/* DDR Base address */
#define DDR_PHY_CMD_ADDR		0x44E12000
#define DDR_PHY_DATA_ADDR		0x44E120C8
#define DDR_PHY_CMD_ADDR2		0x47C0C800
#define DDR_PHY_DATA_ADDR2		0x47C0C8C8
#define DDR_DATA_REGS_NR		2

#define DDRPHY_0_CONFIG_BASE		(CTRL_BASE + 0x1400)
#define DDRPHY_CONFIG_BASE		DDRPHY_0_CONFIG_BASE

/* CPSW Config space */
#define CPSW_MDIO_BASE			0x4A101000

/* RTC base address */
#define RTC_BASE			0x44E3E000

/* OTG */
#define USB0_OTG_BASE			0x47401000
#define USB1_OTG_BASE			0x47401800

/* LCD Controller */
#define LCD_CNTL_BASE			0x4830E000

/* PWMSS */
#define PWMSS0_BASE			0x48300000
#define AM33XX_ECAP0_BASE		0x48300100
#define AM33XX_EPWM_BASE		0x48300200

#endif /* __AM33XX_HARDWARE_AM33XX_H */

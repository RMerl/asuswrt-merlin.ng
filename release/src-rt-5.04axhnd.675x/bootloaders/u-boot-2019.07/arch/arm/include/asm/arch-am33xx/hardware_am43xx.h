/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * hardware_am43xx.h
 *
 * AM43xx hardware specific header
 *
 * Copyright (C) 2013, Texas Instruments, Incorporated - http://www.ti.com/
 */

#ifndef __AM43XX_HARDWARE_AM43XX_H
#define __AM43XX_HARDWARE_AM43XX_H

/* Module base addresses */

/* L3 Fast Configuration Bandwidth Limiter Base Address */
#define L3F_CFG_BWLIMITER		0x44005200

/* UART Base Address */
#define UART0_BASE			0x44E09000

/* GPIO Base address */
#define GPIO2_BASE			0x481AC000

/* Watchdog Timer */
#define WDT_BASE			0x44E35000

/* Control Module Base Address */
#define CTRL_BASE			0x44E10000
#define CTRL_DEVICE_BASE		0x44E10600

/* PRCM Base Address */
#define PRCM_BASE			0x44DF0000
#define	CM_WKUP				0x44DF2800
#define	CM_PER				0x44DF8800
#define CM_DPLL				0x44DF4200
#define CM_RTC				0x44DF8500

#define PRM_RSTCTRL			(PRCM_BASE + 0x4000)
#define PRM_RSTST			(PRM_RSTCTRL + 4)

/* VTP Base address */
#define VTP0_CTRL_ADDR			0x44E10E0C
#define VTP1_CTRL_ADDR			0x48140E10

/* USB CTRL Base Address */
#define USB1_CTRL			0x44e10628
#define USB1_CTRL_CM_PWRDN		BIT(0)
#define USB1_CTRL_OTG_PWRDN		BIT(1)

/* DDR Base address */
#define DDR_PHY_CMD_ADDR		0x44E12000
#define DDR_PHY_DATA_ADDR		0x44E120C8
#define DDR_PHY_CMD_ADDR2		0x47C0C800
#define DDR_PHY_DATA_ADDR2		0x47C0C8C8
#define DDR_DATA_REGS_NR		2

/* CPSW Config space */
#define CPSW_MDIO_BASE			0x4A101000

/* RTC base address */
#define RTC_BASE			0x44E3E000

/* USB OTG */
#define USB_OTG_SS1_BASE		0x48390000
#define USB_OTG_SS1_GLUE_BASE		0x48380000
#define USB2_PHY1_POWER			0x44E10620

#define USB_OTG_SS2_BASE		0x483D0000
#define USB_OTG_SS2_GLUE_BASE		0x483C0000
#define USB2_PHY2_POWER			0x44E10628

/* USB Clock Control */
#define PRM_PER_USB_OTG_SS0_CLKCTRL (CM_PER + 0x260)
#define PRM_PER_USB_OTG_SS1_CLKCTRL (CM_PER + 0x268)
#define USBOTGSSX_CLKCTRL_MODULE_EN	(1 << 1)
#define USBOTGSSX_CLKCTRL_OPTFCLKEN_REFCLK960 (1 << 8)

#define PRM_PER_USBPHYOCP2SCP0_CLKCTRL (CM_PER + 0x5b8)
#define PRM_PER_USBPHYOCP2SCP1_CLKCTRL (CM_PER + 0x5c0)
#define USBPHYOCPSCP_MODULE_EN	(1 << 1)
#define CM_DEVICE_INST			0x44df4100
#define PRM_DEVICE_INST			0x44df4000

#define	USBOTGSSX_CLKCTRL_OPTFCLKEN_REFCLK960	(1 << 8)
#define	USBPHY0_CLKCTRL_OPTFCLKEN_CLK32K	(1 << 8)

/* EDMA3 Base Address */
#define EDMA3_BASE				0x49000000

#endif /* __AM43XX_HARDWARE_AM43XX_H */

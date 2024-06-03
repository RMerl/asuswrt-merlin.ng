/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * (C) Copyright 2017 Linaro
 * Jorge Ramirez-Ortiz <jorge.ramirez-ortiz@linaro.org>
 */

#ifndef __HI3798cv200_H__
#define __HI3798cv200_H__

#define REG_BASE_PERI_CTRL		0xF8A20000
#define REG_BASE_CRG			0xF8A22000

/* DEVICES */
#define REG_BASE_UART0			0xF8B00000
#define HIOTG_BASE_ADDR			0xF98C0000

/* PERI control registers (4KB) */
	/* USB2 PHY01 configuration register */
#define PERI_CTRL_USB0			(REG_BASE_PERI_CTRL + 0x120)

	/* USB2 controller configuration register */
#define PERI_CTRL_USB3			(REG_BASE_PERI_CTRL + 0x12c)
#define USB2_2P_CHIPID			(1 << 28)

/* PERI CRG registers (4KB) */
	/* USB2 CTRL0 clock and soft reset */
#define PERI_CRG46			(REG_BASE_CRG + 0xb8)
#define USB2_BUS_CKEN			(1<<0)
#define USB2_OHCI48M_CKEN		(1<<1)
#define USB2_OHCI12M_CKEN		(1<<2)
#define USB2_OTG_UTMI_CKEN		(1<<3)
#define USB2_HST_PHY_CKEN		(1<<4)
#define USB2_UTMI0_CKEN			(1<<5)
#define USB2_BUS_SRST_REQ		(1<<12)
#define USB2_UTMI0_SRST_REQ		(1<<13)
#define USB2_HST_PHY_SYST_REQ		(1<<16)
#define USB2_OTG_PHY_SYST_REQ		(1<<17)
#define USB2_CLK48_SEL			(1<<20)

	/* USB2 PHY clock and soft reset */
#define PERI_CRG47			(REG_BASE_CRG + 0xbc)
#define USB2_PHY01_REF_CKEN		(1 << 0)
#define USB2_PHY2_REF_CKEN		(1 << 2)
#define USB2_PHY01_SRST_REQ		(1 << 4)
#define USB2_PHY2_SRST_REQ		(1 << 6)
#define USB2_PHY01_SRST_TREQ0		(1 << 8)
#define USB2_PHY01_SRST_TREQ1		(1 << 9)
#define USB2_PHY2_SRST_TREQ		(1 << 10)
#define USB2_PHY01_REFCLK_SEL		(1 << 12)
#define USB2_PHY2_REFCLK_SEL		(1 << 14)


#endif

/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * (c) 2015 Paul Thacker <paul.thacker@microchip.com>
 *
 */

#ifndef __PIC32_REGS_H__
#define __PIC32_REGS_H__

#include <asm/io.h>

/* System Configuration */
#define PIC32_CFG_BASE		0x1f800000

/* System config register offsets */
#define CFGCON		0x0000
#define DEVID		0x0020
#define SYSKEY		0x0030
#define PMD1		0x0040
#define PMD7		0x00a0
#define CFGEBIA		0x00c0
#define CFGEBIC		0x00d0
#define CFGPG		0x00e0
#define CFGMPLL		0x0100

/* Non Volatile Memory (NOR flash) */
#define PIC32_NVM_BASE		(PIC32_CFG_BASE + 0x0600)
/* Oscillator Configuration */
#define PIC32_OSC_BASE		(PIC32_CFG_BASE + 0x1200)
/* Peripheral Pin Select Input */
#define PPS_IN_BASE		0x1f801400
/* Peripheral Pin Select Output */
#define PPS_OUT_BASE		0x1f801500
/* Pin Config */
#define PINCTRL_BASE		0x1f860000

/* USB Core */
#define PIC32_USB_CORE_BASE	0x1f8e3000
#define PIC32_USB_CTRL_BASE	0x1f884000

/* SPI1-SPI6 */
#define PIC32_SPI1_BASE		0x1f821000

/* Prefetch Module */
#define PREFETCH_BASE		0x1f8e0000

/* DDR2 Controller */
#define PIC32_DDR2C_BASE	0x1f8e8000

/* DDR2 PHY */
#define PIC32_DDR2P_BASE	0x1f8e9100

/* EBI */
#define PIC32_EBI_BASE		0x1f8e1000

/* SQI */
#define PIC32_SQI_BASE		0x1f8e2000

struct pic32_reg_atomic {
	u32 raw;
	u32 clr;
	u32 set;
	u32 inv;
};

#define _CLR_OFFSET	0x04
#define _SET_OFFSET	0x08
#define _INV_OFFSET	0x0c

static inline void __iomem *pic32_get_syscfg_base(void)
{
	return (void __iomem *)CKSEG1ADDR(PIC32_CFG_BASE);
}

/* Core */
const char *get_core_name(void);

#endif	/* __PIC32_REGS_H__ */

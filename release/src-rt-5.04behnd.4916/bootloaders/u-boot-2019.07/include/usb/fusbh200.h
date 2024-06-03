/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * Faraday USB 2.0 EHCI Controller
 *
 * (C) Copyright 2010 Faraday Technology
 * Dante Su <dantesu@faraday-tech.com>
 */

#ifndef _FUSBH200_H
#define _FUSBH200_H

struct fusbh200_regs {
	struct {
		uint32_t data[4];
	} hccr;			/* 0x00 - 0x0f: hccr */
	struct {
		uint32_t data[9];
	} hcor;			/* 0x10 - 0x33: hcor */
	uint32_t easstr;/* 0x34: EOF&Async. Sched. Sleep Timer Register */
	uint32_t rsvd[2];
	uint32_t bmcsr;	/* 0x40: Bus Monitor Control Status Register */
	uint32_t bmisr;	/* 0x44: Bus Monitor Interrupt Status Register */
	uint32_t bmier; /* 0x48: Bus Monitor Interrupt Enable Register */
};

/* EOF & Async. Schedule Sleep Timer Register */
#define EASSTR_RUNNING  (1 << 6) /* Put transceiver in running/resume mode */
#define EASSTR_SUSPEND  (0 << 6) /* Put transceiver in suspend mode */
#define EASSTR_EOF2(x)  (((x) & 0x3) << 4) /* EOF 2 Timing */
#define EASSTR_EOF1(x)  (((x) & 0x3) << 2) /* EOF 1 Timing */
#define EASSTR_ASST(x)  (((x) & 0x3) << 0) /* Async. Sched. Sleep Timer */

/* Bus Monitor Control Status Register */
#define BMCSR_SPD_HIGH  (2 << 9) /* Speed of the attached device */
#define BMCSR_SPD_LOW   (1 << 9)
#define BMCSR_SPD_FULL  (0 << 9)
#define BMCSR_SPD_MASK  (3 << 9)
#define BMCSR_SPD_SHIFT 9
#define BMCSR_SPD(x)    ((x >> 9) & 0x03)
#define BMCSR_VBUS      (1 << 8) /* VBUS Valid */
#define BMCSR_VBUS_OFF  (1 << 4) /* VBUS Off */
#define BMCSR_VBUS_ON   (0 << 4) /* VBUS On */
#define BMCSR_IRQLH     (1 << 3) /* IRQ triggered at level-high */
#define BMCSR_IRQLL     (0 << 3) /* IRQ triggered at level-low */
#define BMCSR_HALFSPD   (1 << 2) /* Half speed mode for FPGA test */
#define BMCSR_HFT_LONG  (1 << 1) /* HDISCON noise filter = 270 us*/
#define BMCSR_HFT       (0 << 1) /* HDISCON noise filter = 135 us*/
#define BMCSR_VFT_LONG  (1 << 1) /* VBUS noise filter = 472 us*/
#define BMCSR_VFT       (0 << 1) /* VBUS noise filter = 135 us*/

/* Bus Monitor Interrupt Status Register */
/* Bus Monitor Interrupt Enable Register */
#define BMISR_DMAERR    (1 << 4) /* DMA error */
#define BMISR_DMA       (1 << 3) /* DMA complete */
#define BMISR_DEVRM     (1 << 2) /* device removed */
#define BMISR_OVD       (1 << 1) /* over-current detected */
#define BMISR_VBUSERR   (1 << 0) /* VBUS error */

#endif

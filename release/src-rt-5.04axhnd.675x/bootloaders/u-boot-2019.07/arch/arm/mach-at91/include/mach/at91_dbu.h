/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * Copyright (C) 2010
 * Reinhard Meyer, reinhard.meyer@emk-elektronik.de
 *
 * Debug Unit
 * Based on AT91SAM9XE datasheet
 */

#ifndef AT91_DBU_H
#define AT91_DBU_H

#ifndef __ASSEMBLY__

typedef struct at91_dbu {
	u32	cr;	/* Control Register WO */
	u32	mr;	/* Mode Register  RW */
	u32	ier;	/* Interrupt Enable Register WO */
	u32	idr;	/* Interrupt Disable Register WO */
	u32	imr;	/* Interrupt Mask Register RO */
	u32	sr;	/* Status Register RO */
	u32	rhr;	/* Receive Holding Register RO */
	u32	thr;	/* Transmit Holding Register WO */
	u32	brgr;	/* Baud Rate Generator Register RW */
	u32	res1[7];/* 0x0024 - 0x003C Reserved */
	u32	cidr;	/* Chip ID Register RO */
	u32	exid;	/* Chip ID Extension Register RO */
	u32	fnr;	/* Force NTRST Register RW */
} at91_dbu_t;

#endif /* __ASSEMBLY__ */

#define AT91_DBU_CID_ARCH_MASK		0x0ff00000
#define AT91_DBU_CID_ARCH_9xx		0x01900000
#define AT91_DBU_CID_ARCH_9XExx	0x02900000

#endif

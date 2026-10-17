/* SPDX-License-Identifier: GPL-2.0 */
/*
 * Copyright (c) 2016 Google, Inc
 */

#ifndef __asm_pch_common_h
#define __asm_pch_common_h

/* Common Intel SATA registers */
#define SATA_SIRI		0xa0 /* SATA Indexed Register Index */
#define SATA_SIRD		0xa4 /* SATA Indexed Register Data */
#define SATA_SP			0xd0 /* Scratchpad */

#define INTR_LN			0x3c
#define IDE_TIM_PRI		0x40	/* IDE timings, primary */
#define   IDE_DECODE_ENABLE	(1 << 15)
#define   IDE_SITRE		(1 << 14)
#define   IDE_ISP_5_CLOCKS	(0 << 12)
#define   IDE_ISP_4_CLOCKS	(1 << 12)
#define   IDE_ISP_3_CLOCKS	(2 << 12)
#define   IDE_RCT_4_CLOCKS	(0 <<  8)
#define   IDE_RCT_3_CLOCKS	(1 <<  8)
#define   IDE_RCT_2_CLOCKS	(2 <<  8)
#define   IDE_RCT_1_CLOCKS	(3 <<  8)
#define   IDE_DTE1		(1 <<  7)
#define   IDE_PPE1		(1 <<  6)
#define   IDE_IE1		(1 <<  5)
#define   IDE_TIME1		(1 <<  4)
#define   IDE_DTE0		(1 <<  3)
#define   IDE_PPE0		(1 <<  2)
#define   IDE_IE0		(1 <<  1)
#define   IDE_TIME0		(1 <<  0)
#define IDE_TIM_SEC		0x42	/* IDE timings, secondary */

#define SERIRQ_CNTL		0x64

/**
 * pch_common_sir_read() - Read from a SATA indexed register
 *
 * @dev:	SATA device
 * @idx:	Register index to read
 * Return: value read from register
 */
u32 pch_common_sir_read(struct udevice *dev, int idx);

/**
 * pch_common_sir_write() - Write to a SATA indexed register
 *
 * @dev:	SATA device
 * @idx:	Register index to write
 * @value:	Value to write
 */
void pch_common_sir_write(struct udevice *dev, int idx, u32 value);

#endif

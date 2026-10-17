/* SPDX-License-Identifier: GPL-2.0 */
/*
 * From coreboot file of same name
 *
 * Copyright (C) 2014 Google, Inc
 */

#ifndef _ARCH_ASM_LAPIC_H
#define _ARCH_ASM_LAPIC_H

#define LAPIC_DEFAULT_BASE		0xfee00000

#define LAPIC_ID			0x020
#define LAPIC_LVR			0x030

#define LAPIC_TASKPRI			0x080
#define LAPIC_TPRI_MASK			0xff

#define LAPIC_RRR			0x0c0

#define LAPIC_SPIV			0x0f0
#define LAPIC_SPIV_ENABLE		0x100

#define LAPIC_ICR			0x300
#define LAPIC_DEST_SELF			0x40000
#define LAPIC_DEST_ALLINC		0x80000
#define LAPIC_DEST_ALLBUT		0xc0000
#define LAPIC_ICR_RR_MASK		0x30000
#define LAPIC_ICR_RR_INVALID		0x00000
#define LAPIC_ICR_RR_INPROG		0x10000
#define LAPIC_ICR_RR_VALID		0x20000
#define LAPIC_INT_LEVELTRIG		0x08000
#define LAPIC_INT_ASSERT		0x04000
#define LAPIC_ICR_BUSY			0x01000
#define LAPIC_DEST_LOGICAL		0x00800
#define LAPIC_DM_FIXED			0x00000
#define LAPIC_DM_LOWEST			0x00100
#define LAPIC_DM_SMI			0x00200
#define LAPIC_DM_REMRD			0x00300
#define LAPIC_DM_NMI			0x00400
#define LAPIC_DM_INIT			0x00500
#define LAPIC_DM_STARTUP		0x00600
#define LAPIC_DM_EXTINT			0x00700
#define LAPIC_VECTOR_MASK		0x000ff

#define LAPIC_ICR2			0x310
#define GET_LAPIC_DEST_FIELD(x)		(((x) >> 24) & 0xff)
#define SET_LAPIC_DEST_FIELD(x)		((x) << 24)

#define LAPIC_LVT0			0x350
#define LAPIC_LVT1			0x360
#define LAPIC_LVT_MASKED		(1 << 16)
#define LAPIC_LVT_LEVEL_TRIGGER		(1 << 15)
#define LAPIC_LVT_REMOTE_IRR		(1 << 14)
#define LAPIC_INPUT_POLARITY		(1 << 13)
#define LAPIC_SEND_PENDING		(1 << 12)
#define LAPIC_LVT_RESERVED_1		(1 << 11)
#define LAPIC_DELIVERY_MODE_MASK	(7 << 8)
#define LAPIC_DELIVERY_MODE_FIXED	(0 << 8)
#define LAPIC_DELIVERY_MODE_NMI		(4 << 8)
#define LAPIC_DELIVERY_MODE_EXTINT	(7 << 8)

unsigned long lapic_read(unsigned long reg);

void lapic_write(unsigned long reg, unsigned long v);

void enable_lapic(void);

void disable_lapic(void);

unsigned long lapicid(void);

int lapic_remote_read(int apicid, int reg, unsigned long *pvalue);

void lapic_setup(void);

#endif

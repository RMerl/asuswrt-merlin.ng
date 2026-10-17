/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * Copyright 2014 Freescale Semiconductor, Inc.
 */

#ifndef __FSL_EPU_H
#define __FSL_EPU_H

#include <asm/types.h>

#define FSL_STRIDE_4B	4
#define FSL_STRIDE_8B	8

/* Block offsets */
#define EPU_BLOCK_OFFSET	0x00000000

/* EPGCR (Event Processor Global Control Register) */
#define EPGCR		0x000

/* EPEVTCR0-9 (Event Processor EVT Pin Control Registers) */
#define EPEVTCR0	0x050
#define EPEVTCR9	0x074
#define EPEVTCR_STRIDE	FSL_STRIDE_4B

/* EPXTRIGCR (Event Processor Crosstrigger Control Register) */
#define EPXTRIGCR	0x090

/* EPIMCR0-31 (Event Processor Input Mux Control Registers) */
#define EPIMCR0		0x100
#define EPIMCR31	0x17C
#define EPIMCR_STRIDE	FSL_STRIDE_4B

/* EPSMCR0-15 (Event Processor SCU Mux Control Registers) */
#define EPSMCR0		0x200
#define EPSMCR15	0x278
#define EPSMCR_STRIDE	FSL_STRIDE_8B

/* EPECR0-15 (Event Processor Event Control Registers) */
#define EPECR0		0x300
#define EPECR15		0x33C
#define EPECR_STRIDE	FSL_STRIDE_4B

/* EPACR0-15 (Event Processor Action Control Registers) */
#define EPACR0		0x400
#define EPACR15		0x43C
#define EPACR_STRIDE	FSL_STRIDE_4B

/* EPCCRi0-15 (Event Processor Counter Control Registers) */
#define EPCCR0		0x800
#define EPCCR15		0x83C
#define EPCCR31		0x87C
#define EPCCR_STRIDE	FSL_STRIDE_4B

/* EPCMPR0-15 (Event Processor Counter Compare Registers) */
#define EPCMPR0		0x900
#define EPCMPR15	0x93C
#define EPCMPR31	0x97C
#define EPCMPR_STRIDE	FSL_STRIDE_4B

/* EPCTR0-31 (Event Processor Counter Register) */
#define EPCTR0		0xA00
#define EPCTR31		0xA7C
#define EPCTR_STRIDE	FSL_STRIDE_4B

#define FSM_END_FLAG	0xFFFFFFFFUL

struct fsm_reg_vals {
	u32 offset;
	u32 value;
};

void fsl_epu_setup(void *epu_base);
void fsl_epu_clean(void *epu_base);

#endif

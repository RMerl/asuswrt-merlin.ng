// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright 2014 Freescale Semiconductor, Inc.
 */

#include <common.h>
#include <asm/io.h>

#include "fsl_epu.h"

struct fsm_reg_vals epu_default_val[] = {
	/* EPGCR (Event Processor Global Control Register) */
	{EPGCR, 0},
	/* EPECR (Event Processor Event Control Registers) */
	{EPECR0 + EPECR_STRIDE * 0, 0},
	{EPECR0 + EPECR_STRIDE * 1, 0},
	{EPECR0 + EPECR_STRIDE * 2, 0xF0004004},
	{EPECR0 + EPECR_STRIDE * 3, 0x80000084},
	{EPECR0 + EPECR_STRIDE * 4, 0x20000084},
	{EPECR0 + EPECR_STRIDE * 5, 0x08000004},
	{EPECR0 + EPECR_STRIDE * 6, 0x80000084},
	{EPECR0 + EPECR_STRIDE * 7, 0x80000084},
	{EPECR0 + EPECR_STRIDE * 8, 0x60000084},
	{EPECR0 + EPECR_STRIDE * 9, 0x08000084},
	{EPECR0 + EPECR_STRIDE * 10, 0x42000084},
	{EPECR0 + EPECR_STRIDE * 11, 0x90000084},
	{EPECR0 + EPECR_STRIDE * 12, 0x80000084},
	{EPECR0 + EPECR_STRIDE * 13, 0x08000084},
	{EPECR0 + EPECR_STRIDE * 14, 0x02000084},
	{EPECR0 + EPECR_STRIDE * 15, 0x00000004},
	/*
	 * EPEVTCR (Event Processor EVT Pin Control Registers)
	 * SCU8 triger EVT2, and SCU11 triger EVT9
	 */
	{EPEVTCR0 + EPEVTCR_STRIDE * 0, 0},
	{EPEVTCR0 + EPEVTCR_STRIDE * 1, 0},
	{EPEVTCR0 + EPEVTCR_STRIDE * 2, 0x80000001},
	{EPEVTCR0 + EPEVTCR_STRIDE * 3, 0},
	{EPEVTCR0 + EPEVTCR_STRIDE * 4, 0},
	{EPEVTCR0 + EPEVTCR_STRIDE * 5, 0},
	{EPEVTCR0 + EPEVTCR_STRIDE * 6, 0},
	{EPEVTCR0 + EPEVTCR_STRIDE * 7, 0},
	{EPEVTCR0 + EPEVTCR_STRIDE * 8, 0},
	{EPEVTCR0 + EPEVTCR_STRIDE * 9, 0xB0000001},
	/* EPCMPR (Event Processor Counter Compare Registers) */
	{EPCMPR0 + EPCMPR_STRIDE * 0, 0},
	{EPCMPR0 + EPCMPR_STRIDE * 1, 0},
	{EPCMPR0 + EPCMPR_STRIDE * 2, 0x000000FF},
	{EPCMPR0 + EPCMPR_STRIDE * 3, 0},
	{EPCMPR0 + EPCMPR_STRIDE * 4, 0x000000FF},
	{EPCMPR0 + EPCMPR_STRIDE * 5, 0x00000020},
	{EPCMPR0 + EPCMPR_STRIDE * 6, 0},
	{EPCMPR0 + EPCMPR_STRIDE * 7, 0},
	{EPCMPR0 + EPCMPR_STRIDE * 8, 0x000000FF},
	{EPCMPR0 + EPCMPR_STRIDE * 9, 0x000000FF},
	{EPCMPR0 + EPCMPR_STRIDE * 10, 0x000000FF},
	{EPCMPR0 + EPCMPR_STRIDE * 11, 0x000000FF},
	{EPCMPR0 + EPCMPR_STRIDE * 12, 0x000000FF},
	{EPCMPR0 + EPCMPR_STRIDE * 13, 0},
	{EPCMPR0 + EPCMPR_STRIDE * 14, 0x000000FF},
	{EPCMPR0 + EPCMPR_STRIDE * 15, 0x000000FF},
	/* EPCCR (Event Processor Counter Control Registers) */
	{EPCCR0 + EPCCR_STRIDE * 0, 0},
	{EPCCR0 + EPCCR_STRIDE * 1, 0},
	{EPCCR0 + EPCCR_STRIDE * 2, 0x92840000},
	{EPCCR0 + EPCCR_STRIDE * 3, 0},
	{EPCCR0 + EPCCR_STRIDE * 4, 0x92840000},
	{EPCCR0 + EPCCR_STRIDE * 5, 0x92840000},
	{EPCCR0 + EPCCR_STRIDE * 6, 0},
	{EPCCR0 + EPCCR_STRIDE * 7, 0},
	{EPCCR0 + EPCCR_STRIDE * 8, 0x92840000},
	{EPCCR0 + EPCCR_STRIDE * 9, 0x92840000},
	{EPCCR0 + EPCCR_STRIDE * 10, 0x92840000},
	{EPCCR0 + EPCCR_STRIDE * 11, 0x92840000},
	{EPCCR0 + EPCCR_STRIDE * 12, 0x92840000},
	{EPCCR0 + EPCCR_STRIDE * 13, 0},
	{EPCCR0 + EPCCR_STRIDE * 14, 0x92840000},
	{EPCCR0 + EPCCR_STRIDE * 15, 0x92840000},
	/* EPSMCR (Event Processor SCU Mux Control Registers) */
	{EPSMCR0 + EPSMCR_STRIDE * 0, 0},
	{EPSMCR0 + EPSMCR_STRIDE * 1, 0},
	{EPSMCR0 + EPSMCR_STRIDE * 2, 0x6C700000},
	{EPSMCR0 + EPSMCR_STRIDE * 3, 0x2F000000},
	{EPSMCR0 + EPSMCR_STRIDE * 4, 0x002F0000},
	{EPSMCR0 + EPSMCR_STRIDE * 5, 0x00002E00},
	{EPSMCR0 + EPSMCR_STRIDE * 6, 0x7C000000},
	{EPSMCR0 + EPSMCR_STRIDE * 7, 0x30000000},
	{EPSMCR0 + EPSMCR_STRIDE * 8, 0x64300000},
	{EPSMCR0 + EPSMCR_STRIDE * 9, 0x00003000},
	{EPSMCR0 + EPSMCR_STRIDE * 10, 0x65000030},
	{EPSMCR0 + EPSMCR_STRIDE * 11, 0x31740000},
	{EPSMCR0 + EPSMCR_STRIDE * 12, 0x7F000000},
	{EPSMCR0 + EPSMCR_STRIDE * 13, 0x00003100},
	{EPSMCR0 + EPSMCR_STRIDE * 14, 0x00000031},
	{EPSMCR0 + EPSMCR_STRIDE * 15, 0x76000000},
	/* EPACR (Event Processor Action Control Registers) */
	{EPACR0 + EPACR_STRIDE * 0, 0},
	{EPACR0 + EPACR_STRIDE * 1, 0},
	{EPACR0 + EPACR_STRIDE * 2, 0},
	{EPACR0 + EPACR_STRIDE * 3, 0x00000080},
	{EPACR0 + EPACR_STRIDE * 4, 0},
	{EPACR0 + EPACR_STRIDE * 5, 0x00000040},
	{EPACR0 + EPACR_STRIDE * 6, 0},
	{EPACR0 + EPACR_STRIDE * 7, 0},
	{EPACR0 + EPACR_STRIDE * 8, 0},
	{EPACR0 + EPACR_STRIDE * 9, 0x0000001C},
	{EPACR0 + EPACR_STRIDE * 10, 0x00000020},
	{EPACR0 + EPACR_STRIDE * 11, 0},
	{EPACR0 + EPACR_STRIDE * 12, 0x00000003},
	{EPACR0 + EPACR_STRIDE * 13, 0x06000000},
	{EPACR0 + EPACR_STRIDE * 14, 0x04000000},
	{EPACR0 + EPACR_STRIDE * 15, 0x02000000},
	/* EPIMCR (Event Processor Input Mux Control Registers) */
	{EPIMCR0 + EPIMCR_STRIDE * 0, 0},
	{EPIMCR0 + EPIMCR_STRIDE * 1, 0},
	{EPIMCR0 + EPIMCR_STRIDE * 2, 0},
	{EPIMCR0 + EPIMCR_STRIDE * 3, 0},
	{EPIMCR0 + EPIMCR_STRIDE * 4, 0x44000000},
	{EPIMCR0 + EPIMCR_STRIDE * 5, 0x40000000},
	{EPIMCR0 + EPIMCR_STRIDE * 6, 0},
	{EPIMCR0 + EPIMCR_STRIDE * 7, 0},
	{EPIMCR0 + EPIMCR_STRIDE * 8, 0},
	{EPIMCR0 + EPIMCR_STRIDE * 9, 0},
	{EPIMCR0 + EPIMCR_STRIDE * 10, 0},
	{EPIMCR0 + EPIMCR_STRIDE * 11, 0},
	{EPIMCR0 + EPIMCR_STRIDE * 12, 0x44000000},
	{EPIMCR0 + EPIMCR_STRIDE * 13, 0},
	{EPIMCR0 + EPIMCR_STRIDE * 14, 0},
	{EPIMCR0 + EPIMCR_STRIDE * 15, 0},
	{EPIMCR0 + EPIMCR_STRIDE * 16, 0x6A000000},
	{EPIMCR0 + EPIMCR_STRIDE * 17, 0},
	{EPIMCR0 + EPIMCR_STRIDE * 18, 0},
	{EPIMCR0 + EPIMCR_STRIDE * 19, 0},
	{EPIMCR0 + EPIMCR_STRIDE * 20, 0x48000000},
	{EPIMCR0 + EPIMCR_STRIDE * 21, 0},
	{EPIMCR0 + EPIMCR_STRIDE * 22, 0x6C000000},
	{EPIMCR0 + EPIMCR_STRIDE * 23, 0},
	{EPIMCR0 + EPIMCR_STRIDE * 24, 0},
	{EPIMCR0 + EPIMCR_STRIDE * 25, 0},
	{EPIMCR0 + EPIMCR_STRIDE * 26, 0},
	{EPIMCR0 + EPIMCR_STRIDE * 27, 0},
	{EPIMCR0 + EPIMCR_STRIDE * 28, 0x76000000},
	{EPIMCR0 + EPIMCR_STRIDE * 29, 0},
	{EPIMCR0 + EPIMCR_STRIDE * 30, 0},
	{EPIMCR0 + EPIMCR_STRIDE * 31, 0x76000000},
	/* EPXTRIGCR (Event Processor Crosstrigger Control Register) */
	{EPXTRIGCR, 0x0000FFDF},
	/* end */
	{FSM_END_FLAG, 0},
};

/**
 * fsl_epu_setup - Setup EPU registers to default values
 */
void fsl_epu_setup(void *epu_base)
{
	struct fsm_reg_vals *data = epu_default_val;

	if (!epu_base || !data)
		return;

	while (data->offset != FSM_END_FLAG) {
		out_be32(epu_base + data->offset, data->value);
		data++;
	}
}

/**
 * fsl_epu_clean - Clear EPU registers
 */
void fsl_epu_clean(void *epu_base)
{
	u32 offset;

	/* follow the exact sequence to clear the registers */
	/* Clear EPACRn */
	for (offset = EPACR0; offset <= EPACR15; offset += EPACR_STRIDE)
		out_be32(epu_base + offset, 0);

	/* Clear EPEVTCRn */
	for (offset = EPEVTCR0; offset <= EPEVTCR9; offset += EPEVTCR_STRIDE)
		out_be32(epu_base + offset, 0);

	/* Clear EPGCR */
	out_be32(epu_base + EPGCR, 0);

	/* Clear EPSMCRn */
	for (offset = EPSMCR0; offset <= EPSMCR15; offset += EPSMCR_STRIDE)
		out_be32(epu_base + offset, 0);

	/* Clear EPCCRn */
	for (offset = EPCCR0; offset <= EPCCR31; offset += EPCCR_STRIDE)
		out_be32(epu_base + offset, 0);

	/* Clear EPCMPRn */
	for (offset = EPCMPR0; offset <= EPCMPR31; offset += EPCMPR_STRIDE)
		out_be32(epu_base + offset, 0);

	/* Clear EPCTRn */
	for (offset = EPCTR0; offset <= EPCTR31; offset += EPCTR_STRIDE)
		out_be32(epu_base + offset, 0);

	/* Clear EPIMCRn */
	for (offset = EPIMCR0; offset <= EPIMCR31; offset += EPIMCR_STRIDE)
		out_be32(epu_base + offset, 0);

	/* Clear EPXTRIGCRn */
	out_be32(epu_base + EPXTRIGCR, 0);

	/* Clear EPECRn */
	for (offset = EPECR0; offset <= EPECR15; offset += EPECR_STRIDE)
		out_be32(epu_base + offset, 0);
}

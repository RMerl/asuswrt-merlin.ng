/* SPDX-License-Identifier: GPL-2.0+ */
/*
 *  (C) Copyright 2010,2011
 *  NVIDIA Corporation <www.nvidia.com>
 */

#ifndef _UART_H_
#define _UART_H_

/* UART registers */
struct uart_ctlr {
	uint uart_thr_dlab_0;		/* UART_THR_DLAB_0_0, offset 00 */
	uint uart_ier_dlab_0;		/* UART_IER_DLAB_0_0, offset 04 */
	uint uart_iir_fcr;		/* UART_IIR_FCR_0, offset 08 */
	uint uart_lcr;			/* UART_LCR_0, offset 0C */
	uint uart_mcr;			/* UART_MCR_0, offset 10 */
	uint uart_lsr;			/* UART_LSR_0, offset 14 */
	uint uart_msr;			/* UART_MSR_0, offset 18 */
	uint uart_spr;			/* UART_SPR_0, offset 1C */
	uint uart_irda_csr;		/* UART_IRDA_CSR_0, offset 20 */
	uint uart_reserved[6];		/* Reserved, unused, offset 24-38*/
	uint uart_asr;			/* UART_ASR_0, offset 3C */
};

#define NVRM_PLLP_FIXED_FREQ_KHZ	216000
#define NV_DEFAULT_DEBUG_BAUD		115200

#define UART_FCR_TRIGGER_3	0x30	/* Mask for trigger set at 3 */

#endif	/* UART_H */

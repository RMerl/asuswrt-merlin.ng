/*
 * UART h/w and s/w communication low level interface
 *
 * Copyright 2020 Broadcom
 *
 * This program is the proprietary software of Broadcom and/or
 * its licensors, and may only be used, duplicated, modified or distributed
 * pursuant to the terms and conditions of a separate, written license
 * agreement executed between you and Broadcom (an "Authorized License").
 * Except as set forth in an Authorized License, Broadcom grants no license
 * (express or implied), right to use, or waiver of any kind with respect to
 * the Software, and Broadcom expressly reserves all rights in and to the
 * Software and all intellectual property rights therein.  IF YOU HAVE NO
 * AUTHORIZED LICENSE, THEN YOU HAVE NO RIGHT TO USE THIS SOFTWARE IN ANY
 * WAY, AND SHOULD IMMEDIATELY NOTIFY BROADCOM AND DISCONTINUE ALL USE OF
 * THE SOFTWARE.
 *
 * Except as expressly set forth in the Authorized License,
 *
 * 1. This program, including its structure, sequence and organization,
 * constitutes the valuable trade secrets of Broadcom, and you shall use
 * all reasonable efforts to protect the confidentiality thereof, and to
 * use this information only in connection with your use of Broadcom
 * integrated circuit products.
 *
 * 2. TO THE MAXIMUM EXTENT PERMITTED BY LAW, THE SOFTWARE IS PROVIDED
 * "AS IS" AND WITH ALL FAULTS AND BROADCOM MAKES NO PROMISES,
 * REPRESENTATIONS OR WARRANTIES, EITHER EXPRESS, IMPLIED, STATUTORY, OR
 * OTHERWISE, WITH RESPECT TO THE SOFTWARE.  BROADCOM SPECIFICALLY
 * DISCLAIMS ANY AND ALL IMPLIED WARRANTIES OF TITLE, MERCHANTABILITY,
 * NONINFRINGEMENT, FITNESS FOR A PARTICULAR PURPOSE, LACK OF VIRUSES,
 * ACCURACY OR COMPLETENESS, QUIET ENJOYMENT, QUIET POSSESSION OR
 * CORRESPONDENCE TO DESCRIPTION. YOU ASSUME THE ENTIRE RISK ARISING
 * OUT OF USE OR PERFORMANCE OF THE SOFTWARE.
 *
 * 3. TO THE MAXIMUM EXTENT PERMITTED BY LAW, IN NO EVENT SHALL
 * BROADCOM OR ITS LICENSORS BE LIABLE FOR (i) CONSEQUENTIAL, INCIDENTAL,
 * SPECIAL, INDIRECT, OR EXEMPLARY DAMAGES WHATSOEVER ARISING OUT OF OR
 * IN ANY WAY RELATING TO YOUR USE OF OR INABILITY TO USE THE SOFTWARE EVEN
 * IF BROADCOM HAS BEEN ADVISED OF THE POSSIBILITY OF SUCH DAMAGES; OR (ii)
 * ANY AMOUNT IN EXCESS OF THE AMOUNT ACTUALLY PAID FOR THE SOFTWARE ITSELF
 * OR U.S. $1, WHICHEVER IS GREATER. THESE LIMITATIONS SHALL APPLY
 * NOTWITHSTANDING ANY FAILURE OF ESSENTIAL PURPOSE OF ANY LIMITED REMEDY.
 *
 * $Id: rte_uart.h 474540 2014-05-01 18:40:15Z $
 */

#ifndef _rte_uart_h_
#define _rte_uart_h_

#include <typedefs.h>
#include <osl.h>
#include <siutils.h>
#include <sbchipc.h>
#include <rte_chipc.h>

/* uart control block */
typedef struct serial_dev {
	osl_t	*osh;
	int	baud_base;
	int	irq;
	uint8	*reg_base;
	uint16	reg_shift;
} serial_dev_t;

/* uart id/idx */
#define RTE_UART_0	0

/* UART interrupts */
#define UART_IER_INTERRUPTS	(UART_IER_ERBFI|UART_IER_ETBEI|UART_IER_PTIME)

#ifdef RTE_UART

/* init/free */
int serial_init_devs(si_t *sih, osl_t *osh);
void serial_free_devs(si_t *sih, osl_t *osh);

/* bind the isr to the uart h/w */
serial_dev_t *serial_bind_dev(si_t *sih, uint id, uint32 ccintmask,
	cc_isr_fn isr, void *isr_ctx, cc_dpc_fn dpc, void *dpc_ctx);

/* ============= in/out ops ============== */

/* serial_in: read a uart register */
static INLINE int
serial_in(serial_dev_t *dev, int offset)
{
	return (int)R_REG(dev->osh, (uint8 *)(dev->reg_base + (offset << dev->reg_shift)));
}

/* serial_out: write a uart register */
static INLINE void
serial_out(serial_dev_t *dev, int offset, int value)
{
	W_REG(dev->osh, (uint8 *)(dev->reg_base + (offset << dev->reg_shift)), value);
}

/* serial_getc: non-blocking, return -1 when there is no input */
static INLINE int
serial_getc(serial_dev_t *dev)
{
	/* Input available? */
	if ((serial_in(dev, UART_LSR) & UART_LSR_RXRDY) == 0)
		return -1;

	/* Get the char */
	return serial_in(dev, UART_RX);
}

/* serial_putc: spinwait for room in UART output FIFO, then write character */
static INLINE void
serial_putc(serial_dev_t *dev, int c)
{
	while ((serial_in(dev, UART_LSR) & UART_LSR_THRE))
		;
	serial_out(dev, UART_TX, c);
}

#else

/* init/free */
#define serial_init_devs(sih, osh) (BCME_OK)
#define serial_free_devs(sih, osh) do {} while (0)

#define serial_in(dev, offset) (int)(-1)
#define serial_out(dev, offset, value) do {} while (0)
#define serial_getc(dev) (int)(-1)
#define serial_putc(dev, c) do {} while (0)

#endif /* RTE_UART */

#endif /* _rte_uart_h_ */

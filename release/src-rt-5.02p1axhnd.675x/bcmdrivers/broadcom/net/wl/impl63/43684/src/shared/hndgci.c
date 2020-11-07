/*
 * HND GCI core software interface
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
 * Implementation of Plain UART mode functionality of GCI
 *
 * $Id: hndgci.c 773199 2019-03-14 14:36:42Z $
 */

#include <bcm_cfg.h>
#include <typedefs.h>
#include <osl.h>
#include <bcmutils.h>
#include <bcmdevs.h>
#include <siutils.h>
#include <hndpmu.h>
#include <hndgci.h>
#include <hndsoc.h>
#include <sbchipc.h>
#include <sbgci.h>
#include <rte_timer.h>

#ifdef BCMDBG_ERR
#define	GCI_ERROR(args)	printf args
#else
#define	GCI_ERROR(args)
#endif	/* BCMDBG_ERR */

#ifdef BCMDBG
#define	GCI_MSG(args)	printf args
#else
#define	GCI_MSG(args)
#endif /* BCMDBG */

/* general macros */
#define	GCI_INVALID_DELIM	255

#define	GCI_BREAK_BAUDS		11

#define	CLK_ON	1
#define	CLK_OFF	0

/* Baudrate table */
struct _baudrate {
	uint8	divisor;
	uint8	adjustment;
	uint8	us_per_baud;
};

struct _baudrate g_baudrate[GCI_UART_MAX_BR_IDX] = {
	{ 0xEC, 0x22, 9}
};

static hndgci_cbs_t gcbs;

struct _uart {
#define	SLEEP		1
#define	IDLE		2
#define	DATA_XFER	3
	uint8	wake_state;
	bool	echo_mode;	/* if true, echos every received byte */
	uint8	br_idx;		/* index into baudrate table */
	hnd_timer_t *timer;	/* to track inactivity */
	uint32 timeout;		/* inactivity timeout */

	/* transmit */
	volatile bool	tx_flow_on;	/* to control tx flow */
	char	*txbuf;			/* ref to client tx buf */
	int	tx_count;
	int	tx_idx;
	int	tx_fifo_ae_level; /* tx fifo almost empty level */
	hnd_timer_t *tx_timer; /* zero timer */

	/* receive */
#define GCI_UART_RX_BUF_SIZE	1024
#define GCI_UART_TX_BUF_SIZE	1024
	char	*rxbuf; /* buffer rxdata */
	int	rx_idx;

	/* rx callback trigger conditions */
	char	rx_delim;
	int	rx_len;
	int	rx_timeout;	/* in ms */

	/* interrupt counters */
	uint32	rx_af;
	uint32	rx_ne;
	uint32	rx_of;
	uint32	rx_it;

	/* callback */
	hndgci_cbs_t *cbs;
};

typedef struct _gci {
	bool oper_mode;
	osl_t *osh;
	void *gci_i;

	struct _uart	uart;
} gci_t;
static gci_t *gci = NULL;

static void hndgci_uart_intr_handler(si_t *sih, uint32 intstatus);
static void hndgci_uart_timer_handler(hnd_timer_t *t);
static void hndgci_uart_tx_timer_handler(hnd_timer_t *t);
static void _gci_write_uart_data(si_t *sih, gci_t *gu);

#ifdef	GCI_DEBUG
typedef struct _gci_test {
	int rx_count;
	char rxbuf[GCI_UART_RX_BUF_SIZE];
} gci_test_t;

gci_test_t	gcitest;

static void _gci_test_rx_callback(void *sih, char *buf, int i);

static void
_gci_test_rx_callback(void *sih, char *buf, int size)
{
	gci_test_t *gu = &gcitest;

	if ((gu->rx_count + size) > GCI_UART_RX_BUF_SIZE)
		return;

	bcopy(buf, gu->rxbuf + gu->rx_count, size);
	gu->rx_count += size;
}

/* Test Rx function: */
int
hndgci_uart_rx(si_t *sih, char *buf, int count, int timeout)
{
	gci_test_t *gu = &gcitest;
	int bytes_to_copy = 0;

	/* copy data */
	if (gu->rx_count > 0) {
		bytes_to_copy = (gu->rx_count < count)? gu->rx_count : count;
		bcopy(gu->rxbuf, buf, bytes_to_copy);
	}

	gu->rx_count = 0;

	return bytes_to_copy;
}
#endif /* GCI_DEBUG */

static void
hndgci_send_break(si_t *sih)
{
	uint8 us_per_baud = g_baudrate[gci->uart.br_idx].us_per_baud;

	si_gci_direct(sih, GCI_OFFSETOF(sih, gci_secilcr),
		SECI_UART_LCR_LBRK_CTRL, SECI_UART_LCR_LBRK_CTRL);
	OSL_DELAY(us_per_baud * GCI_BREAK_BAUDS);
	si_gci_direct(sih, GCI_OFFSETOF(sih, gci_secilcr),
		SECI_UART_LCR_LBRK_CTRL, 0);
}

static void
hndgci_force_clk(si_t *sih, uint8 on)
{
	uint32 mask = CCS_ALPAREQ;

	if (on) {
		/* force gci clk */
		si_corereg(sih, SI_CC_IDX, OFFSETOF(chipcregs_t, clk_ctl_st),
			mask, mask);
	} else {
		/* disable force gci clk */
		si_corereg(sih, SI_CC_IDX, OFFSETOF(chipcregs_t, clk_ctl_st),
			mask, 0);
	}
}

static void
hndgci_wake(si_t *sih)
{
	/* force ht&alp clk */
	hndgci_force_clk(sih, CLK_ON);

	/* disable wake interrupt on GciInt */
	si_gci_indirect(sih, 0,
		GCI_OFFSETOF(sih, gci_intmask),
		(GCI_INTSTATUS_GPIOINT | GCI_INTSTATUS_GPIOWAKE),
		0);

	/* disable wake on GciWake */
	si_gci_indirect(sih, 0,
		GCI_OFFSETOF(sih, gci_wakemask),
		(GCI_INTSTATUS_GPIOINT | GCI_INTSTATUS_GPIOWAKE),
		0);

	hndgci_send_break(sih);
}

static void
hndgci_sleep(si_t *sih)
{
	uint32 reg = 0;

	/*
	 * when the chip is awake, gpiostatus value has no use here.
	 * Clear it, otherwise it causes wake up interrupt.
	 */
	reg = si_gci_gpio_status(sih, CC_GCI_GPIO_6, 0x0, 0x0);

	/* writing to clear gpio status */
	si_gci_gpio_status(sih, CC_GCI_GPIO_6, reg, reg);

	/* enable wake interrupt */
	si_gci_indirect(sih, 0,
		GCI_OFFSETOF(sih, gci_intmask),
		(GCI_INTSTATUS_GPIOWAKE),
		(GCI_INTSTATUS_GPIOWAKE));

	/* enable wake event */
	si_gci_indirect(sih, 0,
		GCI_OFFSETOF(sih, gci_wakemask),
		(GCI_INTSTATUS_GPIOWAKE),
		(GCI_INTSTATUS_GPIOWAKE));

	/* disable force gci clk */
	hndgci_force_clk(sih, CLK_OFF);

	GCI_MSG(("SLEEP \n"));
	gci->uart.wake_state = SLEEP;
}

/*
 * performs real tx by writting to the register
 */
static void
_gci_write_uart_data(si_t *sih, gci_t *gu)
{
	int bytes_to_tx = 0;
	int i = 0;
	uint8 *r = NULL;
	uint idx;

	if (!gu->uart.txbuf)
		return;

	idx = si_coreidx(sih);

	r = (uint8 *)si_setcoreidx(sih, GCI_CORE_IDX(sih)) +
		GCI_OFFSETOF(sih, gci_seciuartdata);

	/* stop timer */
	hnd_timer_stop(gu->uart.timer);

	/* calculate num bytes to transfer */
	bytes_to_tx = gu->uart.tx_count < (14 - gu->uart.tx_fifo_ae_level) ?
		gu->uart.tx_count : (14 - gu->uart.tx_fifo_ae_level);

	/* put the data into register */
	while (i < bytes_to_tx) {
		*r = gu->uart.txbuf[gu->uart.tx_idx + i];
		i++;
	}

	si_setcoreidx(sih, idx);

	/* store index for next iteration */
	gu->uart.tx_idx += i;

	/*
	 * fifo has enough data;
	 * stop tx until almost empty interrupt
	 */
	gu->uart.tx_flow_on = 0;

	if (gu->uart.tx_count > 0) {
		gu->uart.tx_count -= i;
	}

	/* check for end of transmission */
	if (!gu->uart.tx_count) {
		MFREE(gu->osh, gu->uart.txbuf, gu->uart.tx_idx);
		gu->uart.txbuf = NULL;
		gu->uart.tx_idx = 0;
		gu->uart.tx_flow_on = 0;

		/* disable tx fifo almost empty intr */
		si_gci_indirect(sih, 0,
			GCI_OFFSETOF(sih, gci_intmask), GCI_INTSTATUS_STFAE, 0);

		hnd_timer_stop(gu->uart.tx_timer);
	}

	/* start timer */
	hnd_timer_start(gu->uart.timer, gu->uart.timeout, FALSE);
}

/*
 *	zero length timer. triggered periodicaly;
 *	when there is something to transmit
 */
static void hndgci_uart_tx_timer_handler(hnd_timer_t *t)
{
	si_t *sih = (si_t *)hnd_timer_get_ctx(t);
	gci_t *gu = hnd_timer_get_data(t);

	/* enable tx fifo almost empty intr */
	si_gci_indirect(sih, 0,
		GCI_OFFSETOF(sih, gci_intmask), GCI_INTSTATUS_STFAE, GCI_INTSTATUS_STFAE);

	if (gu->uart.tx_flow_on)
		_gci_write_uart_data(sih, gu);
}

/*
 *	config the conditions for rx callback
 *
 */
int
hndgci_uart_config_rx_complete(char delim, int len, int timeout, rx_cb_t rx_cb, void *ctx)
{
	gci_t *gu = gci;

	GCI_MSG(("%s \n", __FUNCTION__));

	if (!gu) {
		return HND_GCI_UNINITIALIZED;
	}

	/* reset delim/len */
	gu->uart.rx_delim = GCI_INVALID_DELIM;
	gu->uart.rx_len = 0;

	/* look for delimitter or number of bytes to receive */
	if (delim != GCI_INVALID_DELIM) {
		gu->uart.rx_delim = delim;
	} else if (len > 0) {
		gu->uart.rx_len = len;
	} else if (timeout < 0) {
		/* called with all invalid params */
		return HND_GCI_INVALID_PARAM;
	}

	/* configure timeout */
	if (timeout >= 0) {
		gu->uart.rx_timeout = timeout;
	}

	/* init callback */
	gcbs.rx_cb = rx_cb;
	gcbs.context = ctx;

	return HND_GCI_SUCCESS;
}

/*
 * gci uart transmit function
 */
int
hndgci_uart_tx(si_t *sih, void *buf, int count)
{
	gci_t *gu;

	GCI_MSG(("%s \n", __FUNCTION__));

	if (!buf) {
		GCI_ERROR(("buffer not valid \n"));
		return HND_GCI_INVALID_BUFFER;
	}

	if (count <= 0) {
		GCI_ERROR(("count %d \n", count));
		return HND_GCI_SUCCESS;
	}

	gu = gci;

	/* prepare tx buffer */
	if (!gu->uart.tx_count) {
		gu->uart.tx_count =
			(count < GCI_UART_TX_BUF_SIZE)? count : GCI_UART_TX_BUF_SIZE;
		gu->uart.txbuf = buf;
	} else {
		return HND_GCI_TX_INPROGRESS;
	}

	/* wake up the other end */
	if (gci->uart.wake_state == SLEEP) {

		hndgci_wake(sih);

		gu->uart.wake_state = IDLE;

		/* start idle timer */
		hnd_timer_start(gci->uart.timer, gci->uart.timeout, FALSE);
	}

	/* start tx timer */
	if (gci->uart.wake_state == DATA_XFER)
		hnd_timer_start(gci->uart.tx_timer, 0, TRUE);

	return gu->uart.tx_count;
}

/*
 * idle timer
 */
static void hndgci_uart_timer_handler(hnd_timer_t *t)
{
	si_t *sih = (si_t *)hnd_timer_get_ctx(t);
	gci_t *gu = hnd_timer_get_data(t);

	/* stop timer */
	hnd_timer_stop(gu->uart.timer);

	/* if the gci uart is not sleep put it in sleep state */
	if (gu->uart.wake_state != SLEEP) {
		hndgci_sleep(sih);
	}
}

/*
 *	GCI interrupt handler.
 */
void
hndgci_handler_process(uint32 stat, si_t *sih)
{
	gci_t *gu = gci;

	if (gu->oper_mode == HND_GCI_PLAIN_UART_MODE) {
		hndgci_uart_intr_handler(sih, stat);
	}
}

static uint32
_gci_read_uart_data(si_t *sih)
{
	uint32 udata = 0;
#ifdef BCMDBG
	uint8 rptr = 0, wptr = 0;
	uint8 of = 0, f = 0, ne = 0;
#endif /* BCMDBG */

	udata = si_gci_direct(sih, GCI_OFFSETOF(sih, gci_seciuartdata), 0, 0);

#ifdef BCMDBG
	wptr	= (udata >> 24) & 0xFF;
	rptr	= (udata >> 16) & 0xFF;
	of	= (udata >> 14) & 0x1;
	f	= (udata >> 13) & 0x1;
	ne	= (udata >> 12) & 0x1;

	GCI_MSG(("wp %d rp %d of(%d) f(%d) ne(%d) %c\n",
		wptr, rptr, of, f, ne, (char)udata));
#endif /* BCMDBG */

	return udata;
}

static void
hndgci_gpiowake_handler(uint32 stat, void *sih_p)
{
	gci_t *gu = gci;
	uint32 reg = stat;
	si_t *sih = sih_p;

	GCI_MSG(("%s %x \n", __FUNCTION__, reg));

	if (reg & GCI_GPIO_STS_POS_EDGE_BIT) {
		if (gu->uart.wake_state == SLEEP) {

			/* keep it awake */
			hndgci_wake(sih);

			gu->uart.wake_state = DATA_XFER;
			if (gci->uart.timer)
				hnd_timer_start(gci->uart.timer, gci->uart.timeout, FALSE);
			else
				printf("Timer isn't setup \n");
		}
	}
}

/*
 *	GCI plain uart mode interrupt handler.
 */
static void
hndgci_uart_intr_handler(si_t *sih, uint32 intstatus)
{
	gci_t *gu = gci;
	uint32 udata = 0xdb;
	char ubyte;

	/* Parity error */
	if (intstatus & GCI_INTSTATUS_SPE) {
		si_gci_direct(sih, GCI_OFFSETOF(sih, gci_intstat),
			GCI_INTSTATUS_SPE, GCI_INTSTATUS_SPE);
	}

	/* Frame error */
	if (intstatus & GCI_INTSTATUS_SFE) {
		si_gci_direct(sih, GCI_OFFSETOF(sih, gci_intstat),
			GCI_INTSTATUS_SFE, GCI_INTSTATUS_SFE);
	}

	/* Rx Break interrupt */
	if (intstatus & GCI_INTSTATUS_RBI) {
		/* clear rx break interrupt */
		si_gci_direct(sih, GCI_OFFSETOF(sih, gci_intstat),
			GCI_INTSTATUS_RBI, GCI_INTSTATUS_RBI);

		if (gu->uart.wake_state == IDLE) {
			/* stop idle timer */
			hnd_timer_stop(gu->uart.timer);

			/* send brk and transition to ACTIVE */
			gu->uart.wake_state = DATA_XFER;
			hnd_timer_start(gci->uart.tx_timer, 0, TRUE);
		} else if (gu->uart.wake_state == SLEEP) {

			/* keep it awake */
			hndgci_wake(sih);

			gu->uart.wake_state = DATA_XFER;
		}
		/* start idle timer */
		hnd_timer_start(gci->uart.timer, gci->uart.timeout, FALSE);
	}

	if (intstatus & GCI_INTSTATUS_SRITI) {
		GCI_ERROR(("*** rx idle *** \n"));
	}

	if (intstatus & GCI_INTSTATUS_SRFOF) {
		GCI_ERROR(("*** rx overflow *** \n"));
		si_gci_direct(sih, GCI_OFFSETOF(sih, gci_intstat),
			GCI_INTSTATUS_SRFOF, GCI_INTSTATUS_SRFOF);
		gci->uart.rx_of++;
	}

	if (intstatus & (GCI_INTSTATUS_SRFAF |
		GCI_INTSTATUS_SRITI)) {

		bool call_cb = FALSE;
		uint32	read = 0;

		hnd_timer_stop(gci->uart.timer);

		udata = _gci_read_uart_data(sih);

		if (intstatus & GCI_INTSTATUS_SRITI) {
			gci->uart.rx_it++;
		}
		if (intstatus & GCI_INTSTATUS_SRFAF) {
			gci->uart.rx_af++;
		} else if (intstatus & GCI_INTSTATUS_SRFNE) {
			gci->uart.rx_ne++;
		}

		while (udata & SECI_UART_DATA_RF_NOT_EMPTY_BIT) {

			ubyte = (char)udata;

			if (gci->uart.echo_mode) {
				hndgci_uart_tx(sih, &ubyte, 1);
			}

			/* check for callback conditions */
			if (gci->uart.rx_delim != GCI_INVALID_DELIM) {
				if (gci->uart.rx_delim == ubyte) {
					call_cb = TRUE;
				}
			} else if (gci->uart.rx_len > 0) {
				if (gci->uart.rx_len == gci->uart.rx_idx) {
					call_cb = TRUE;
				}
			} else {
				call_cb = TRUE;
			}

			/* store in local buffer */
			gci->uart.rxbuf[gci->uart.rx_idx] = ubyte;
			gci->uart.rx_idx = MODINC(gci->uart.rx_idx, GCI_UART_RX_BUF_SIZE);
#ifdef GCI_DEBUG
			_gci_test_rx_callback(NULL, &ubyte, 1);
#endif /* GCI_DEBUG */

			/* per interrupt byte count for debugging */
			read++;
			if (call_cb)
				break;

			udata = _gci_read_uart_data(sih);
		}
		/* if callback registered; call it */
		if (call_cb && gci->uart.cbs->rx_cb) {
			gci->uart.cbs->rx_cb(gci->uart.cbs->context, gci->uart.rxbuf,
				gci->uart.rx_idx);
			bzero(gci->uart.rxbuf, GCI_UART_RX_BUF_SIZE);
			gci->uart.rx_idx = 0;

			call_cb = FALSE;
		}
		hnd_timer_start(gci->uart.timer, gci->uart.timeout, FALSE);
	}

	if (intstatus & GCI_INTSTATUS_STFF) {
		gu->uart.tx_flow_on = 0;
		GCI_MSG(("tx fifo full %d\n", gu->uart.tx_idx));

		/* enable tx fifo almost empty intr */
		si_gci_indirect(sih, 0,
			GCI_OFFSETOF(sih, gci_intmask), GCI_INTSTATUS_STFAE,
			GCI_INTSTATUS_STFAE);
	}
	else if ((!gu->uart.tx_flow_on) && (intstatus & GCI_INTSTATUS_STFAE)) {
		gu->uart.tx_flow_on = 1;

		hnd_timer_stop(gu->uart.timer);
		hnd_timer_stop(gu->uart.tx_timer);

		/* start timer */
		hnd_timer_start(gu->uart.timer, gu->uart.timeout, FALSE);
		if (gu->uart.tx_count)
			hnd_timer_start(gu->uart.tx_timer, 0, TRUE);
	}
}

static void
hndgci_config_pinmux(si_t *sih)
{
	switch (CHIPID(sih->chip)) {
		default:
			GCI_ERROR(("unsupported chip \n"));
		break;
	}
}

/*
 * gci uart init function
 */
static int
BCMINITFN(hndgci_uart_init)(si_t *sih, uint8 baudrate_idx)
{
	uint8 txf_ae_level = TXF_AE_LVL_DEFAULT;
	uint8 wake_mask = 0;
	uint8 cur_status = 0;
	uint8 gci_gpio = CC_GCI_GPIO_INVALID;
	uint8 gpio_status = (1 << GCI_GPIO_STS_POS_EDGE_BIT);
	uint32	regval = 0;
	uint32	intmask = 0;

	/* reset GCI block */
	si_gci_reset(sih);

	/* enable plain UART mode */
	si_gci_direct(sih, GCI_OFFSETOF(sih, gci_corectrl),
		(GCI_CORECTRL_ES_MASK | SECI_MODE_UART),
		GCI_CORECTRL_ES_MASK | SECI_MODE_UART);

	hndgci_config_pinmux(sih);

	/* hardcoded to 115k baudrate */
	si_gci_direct(sih, GCI_OFFSETOF(sih, gci_secibauddiv),
		0xFFFFFFFF, g_baudrate[baudrate_idx].divisor);
	si_gci_direct(sih, GCI_OFFSETOF(sih, gci_baudadj),
		0xFFFFFFFF, g_baudrate[baudrate_idx].adjustment);

	/* reset gci fifo */
	regval = (SECI_UART_FCR_RFR | SECI_UART_FCR_TFR);
	si_gci_direct(sih, GCI_OFFSETOF(sih, gci_secifcr), 0xFFFFFFFF, regval);
	OSL_DELAY(10);
	si_gci_direct(sih, GCI_OFFSETOF(sih, gci_secifcr), 0xFFFFFFFF, 0x00);

	/* enable tx and baudrate adjust */
	regval = (SECI_UART_MCR_TX_EN | SECI_UART_MCR_BAUD_ADJ_EN);
	si_gci_direct(sih, GCI_OFFSETOF(sih, gci_secimcr), 0xFFFFFFFF, regval);

	/* enable rx */
	regval = (SECI_UART_LCR_RX_EN | SECI_UART_LCR_TXO_EN);
	si_gci_direct(sih, GCI_OFFSETOF(sih, gci_secilcr), 0xFFFFFFFF, regval);

	/* rx fifo level to indicate almost full condition */
	si_gci_direct(sih, GCI_OFFSETOF(sih, gci_rxfifoctrl), 0xFFFFFFFF, 0x80E);

	/* tx fifo level to indicate almost empty condition */
	si_gci_direct(sih, GCI_OFFSETOF(sih, gci_secififolevel), 0xFFFFFFFF,
		(txf_ae_level << 8));

	gci_gpio = si_enable_device_wake(sih, &wake_mask, &cur_status);

	gci->gci_i = si_gci_gpioint_handler_register(sih, gci_gpio, gpio_status,
		hndgci_gpiowake_handler, (void *)sih);

	/* enable gci uart/seci interrupts:
	 * UART Break Interrupt
	 * SECI Rx Idle timer interrupt
	 * SECI Rx FIFO Almost Full
	 * SECI Rx FIFO Not Empty
	 */
	intmask = GCI_INTSTATUS_UB | GCI_INTSTATUS_SRFAF |
		GCI_INTSTATUS_STFF | GCI_INTSTATUS_RBI |
		GCI_INTSTATUS_SRFOF | GCI_INTSTATUS_SRITI;
	si_gci_indirect(sih, 0,
		GCI_OFFSETOF(sih, gci_intmask), ~0, intmask);

	/* initialize driver context */
	gci->uart.tx_flow_on = 1;
	gci->uart.echo_mode = 0;
	gci->uart.br_idx = baudrate_idx;

	/* initialize idle timer */
	gci->uart.timeout = 100;
	if (!gci->uart.timer)
		gci->uart.timer = hnd_timer_create((void *)sih, gci,
			hndgci_uart_timer_handler, NULL);

	hnd_timer_start(gci->uart.timer, gci->uart.timeout, FALSE);

	/* tx context */
	gci->uart.tx_count = 0;
	gci->uart.tx_idx = 0;
	gci->uart.tx_fifo_ae_level = txf_ae_level;
	if (!gci->uart.tx_timer)
		gci->uart.tx_timer = hnd_timer_create((void *)sih, gci,
			hndgci_uart_tx_timer_handler, NULL);

	/* rx context */
	gci->uart.rx_delim = GCI_INVALID_DELIM;
	gci->uart.rx_len = 0;
	gci->uart.rx_timeout = 0;

	/* reset rx interrupt counters */
	gci->uart.rx_af = 0;
	gci->uart.rx_ne = 0;
	gci->uart.rx_of = 0;
	gci->uart.rx_it = 0;

	hndgci_sleep(sih);

	return HND_GCI_SUCCESS;
}

/*
 * Setup the gci core.
 */
int
BCMINITFN(hndgci_init)(si_t *sih, osl_t *osh, uint8 mode, uint8 br_idx)
{
	int ret = HND_GCI_SUCCESS;

	if (!sih)
		printf("sih NULL\n");
	if (!osh)
		printf("osh NULL\n");

	if (!sih || !osh)
		return HND_GCI_INVALID_PARAM;

	/* check whether GCI is present */
	if (!si_gci(sih)) {
		GCI_ERROR(("GCI is not present \n"));
		return HND_GCI_NO_SUPPORT;
	}

	/* allocate memory for gci context */
	if (!gci) {
		gci = (gci_t *)MALLOC(osh, sizeof(gci_t));
		gci->osh = osh;
	} else {
		return HND_GCI_NO_MEMORY;
	}

	switch (mode) {
		case HND_GCI_PLAIN_UART_MODE:
			if (!gci->uart.rxbuf)
				gci->uart.rxbuf = (char *)MALLOC(osh, GCI_UART_RX_BUF_SIZE);
			ret = hndgci_uart_init(sih, br_idx);
			gci->uart.cbs = &gcbs;
#ifdef GCI_DEBUG
			hndgci_uart_config_rx_complete(-1, -1, 0, _gci_test_rx_callback, NULL);
#endif /* GCI_DEBUG */
			break;
		case HND_GCI_SECI_MODE:
			break;
		case HND_GCI_PURE_GPIO_MODE:
			break;
		default:
			break;
	}

	return ret;
}

/*
 * deinitialize gci
 */
void
hndgci_deinit(si_t *sih)
{

	/* disable gci interrupts */
	si_gci_indirect(sih, 0,
		GCI_OFFSETOF(sih, gci_intmask), ~0, 0);

	/* enable wake events same as gci interrupts */
	si_gci_indirect(sih, 0,
		GCI_OFFSETOF(sih, gci_wakemask), ~0, 0);

	si_gci_gpioint_handler_unregister(sih, gci->gci_i);

	/* free gci context */
	if (gci)
		MFREE(gci->osh, gci, sizeof(gci_t));
}

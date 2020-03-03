/*
 *  Freescale lpuart serial port driver
 *
 *  Copyright 2012-2014 Freescale Semiconductor, Inc.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 */

#if defined(CONFIG_SERIAL_FSL_LPUART_CONSOLE) && defined(CONFIG_MAGIC_SYSRQ)
#define SUPPORT_SYSRQ
#endif

#include <linux/clk.h>
#include <linux/console.h>
#include <linux/dma-mapping.h>
#include <linux/dmaengine.h>
#include <linux/dmapool.h>
#include <linux/io.h>
#include <linux/irq.h>
#include <linux/module.h>
#include <linux/of.h>
#include <linux/of_device.h>
#include <linux/of_dma.h>
#include <linux/serial_core.h>
#include <linux/slab.h>
#include <linux/tty_flip.h>

/* All registers are 8-bit width */
#define UARTBDH			0x00
#define UARTBDL			0x01
#define UARTCR1			0x02
#define UARTCR2			0x03
#define UARTSR1			0x04
#define UARTCR3			0x06
#define UARTDR			0x07
#define UARTCR4			0x0a
#define UARTCR5			0x0b
#define UARTMODEM		0x0d
#define UARTPFIFO		0x10
#define UARTCFIFO		0x11
#define UARTSFIFO		0x12
#define UARTTWFIFO		0x13
#define UARTTCFIFO		0x14
#define UARTRWFIFO		0x15

#define UARTBDH_LBKDIE		0x80
#define UARTBDH_RXEDGIE		0x40
#define UARTBDH_SBR_MASK	0x1f

#define UARTCR1_LOOPS		0x80
#define UARTCR1_RSRC		0x20
#define UARTCR1_M		0x10
#define UARTCR1_WAKE		0x08
#define UARTCR1_ILT		0x04
#define UARTCR1_PE		0x02
#define UARTCR1_PT		0x01

#define UARTCR2_TIE		0x80
#define UARTCR2_TCIE		0x40
#define UARTCR2_RIE		0x20
#define UARTCR2_ILIE		0x10
#define UARTCR2_TE		0x08
#define UARTCR2_RE		0x04
#define UARTCR2_RWU		0x02
#define UARTCR2_SBK		0x01

#define UARTSR1_TDRE		0x80
#define UARTSR1_TC		0x40
#define UARTSR1_RDRF		0x20
#define UARTSR1_IDLE		0x10
#define UARTSR1_OR		0x08
#define UARTSR1_NF		0x04
#define UARTSR1_FE		0x02
#define UARTSR1_PE		0x01

#define UARTCR3_R8		0x80
#define UARTCR3_T8		0x40
#define UARTCR3_TXDIR		0x20
#define UARTCR3_TXINV		0x10
#define UARTCR3_ORIE		0x08
#define UARTCR3_NEIE		0x04
#define UARTCR3_FEIE		0x02
#define UARTCR3_PEIE		0x01

#define UARTCR4_MAEN1		0x80
#define UARTCR4_MAEN2		0x40
#define UARTCR4_M10		0x20
#define UARTCR4_BRFA_MASK	0x1f
#define UARTCR4_BRFA_OFF	0

#define UARTCR5_TDMAS		0x80
#define UARTCR5_RDMAS		0x20

#define UARTMODEM_RXRTSE	0x08
#define UARTMODEM_TXRTSPOL	0x04
#define UARTMODEM_TXRTSE	0x02
#define UARTMODEM_TXCTSE	0x01

#define UARTPFIFO_TXFE		0x80
#define UARTPFIFO_FIFOSIZE_MASK	0x7
#define UARTPFIFO_TXSIZE_OFF	4
#define UARTPFIFO_RXFE		0x08
#define UARTPFIFO_RXSIZE_OFF	0

#define UARTCFIFO_TXFLUSH	0x80
#define UARTCFIFO_RXFLUSH	0x40
#define UARTCFIFO_RXOFE		0x04
#define UARTCFIFO_TXOFE		0x02
#define UARTCFIFO_RXUFE		0x01

#define UARTSFIFO_TXEMPT	0x80
#define UARTSFIFO_RXEMPT	0x40
#define UARTSFIFO_RXOF		0x04
#define UARTSFIFO_TXOF		0x02
#define UARTSFIFO_RXUF		0x01

/* 32-bit register defination */
#define UARTBAUD		0x00
#define UARTSTAT		0x04
#define UARTCTRL		0x08
#define UARTDATA		0x0C
#define UARTMATCH		0x10
#define UARTMODIR		0x14
#define UARTFIFO		0x18
#define UARTWATER		0x1c

#define UARTBAUD_MAEN1		0x80000000
#define UARTBAUD_MAEN2		0x40000000
#define UARTBAUD_M10		0x20000000
#define UARTBAUD_TDMAE		0x00800000
#define UARTBAUD_RDMAE		0x00200000
#define UARTBAUD_MATCFG		0x00400000
#define UARTBAUD_BOTHEDGE	0x00020000
#define UARTBAUD_RESYNCDIS	0x00010000
#define UARTBAUD_LBKDIE		0x00008000
#define UARTBAUD_RXEDGIE	0x00004000
#define UARTBAUD_SBNS		0x00002000
#define UARTBAUD_SBR		0x00000000
#define UARTBAUD_SBR_MASK	0x1fff

#define UARTSTAT_LBKDIF		0x80000000
#define UARTSTAT_RXEDGIF	0x40000000
#define UARTSTAT_MSBF		0x20000000
#define UARTSTAT_RXINV		0x10000000
#define UARTSTAT_RWUID		0x08000000
#define UARTSTAT_BRK13		0x04000000
#define UARTSTAT_LBKDE		0x02000000
#define UARTSTAT_RAF		0x01000000
#define UARTSTAT_TDRE		0x00800000
#define UARTSTAT_TC		0x00400000
#define UARTSTAT_RDRF		0x00200000
#define UARTSTAT_IDLE		0x00100000
#define UARTSTAT_OR		0x00080000
#define UARTSTAT_NF		0x00040000
#define UARTSTAT_FE		0x00020000
#define UARTSTAT_PE		0x00010000
#define UARTSTAT_MA1F		0x00008000
#define UARTSTAT_M21F		0x00004000

#define UARTCTRL_R8T9		0x80000000
#define UARTCTRL_R9T8		0x40000000
#define UARTCTRL_TXDIR		0x20000000
#define UARTCTRL_TXINV		0x10000000
#define UARTCTRL_ORIE		0x08000000
#define UARTCTRL_NEIE		0x04000000
#define UARTCTRL_FEIE		0x02000000
#define UARTCTRL_PEIE		0x01000000
#define UARTCTRL_TIE		0x00800000
#define UARTCTRL_TCIE		0x00400000
#define UARTCTRL_RIE		0x00200000
#define UARTCTRL_ILIE		0x00100000
#define UARTCTRL_TE		0x00080000
#define UARTCTRL_RE		0x00040000
#define UARTCTRL_RWU		0x00020000
#define UARTCTRL_SBK		0x00010000
#define UARTCTRL_MA1IE		0x00008000
#define UARTCTRL_MA2IE		0x00004000
#define UARTCTRL_IDLECFG	0x00000100
#define UARTCTRL_LOOPS		0x00000080
#define UARTCTRL_DOZEEN		0x00000040
#define UARTCTRL_RSRC		0x00000020
#define UARTCTRL_M		0x00000010
#define UARTCTRL_WAKE		0x00000008
#define UARTCTRL_ILT		0x00000004
#define UARTCTRL_PE		0x00000002
#define UARTCTRL_PT		0x00000001

#define UARTDATA_NOISY		0x00008000
#define UARTDATA_PARITYE	0x00004000
#define UARTDATA_FRETSC		0x00002000
#define UARTDATA_RXEMPT		0x00001000
#define UARTDATA_IDLINE		0x00000800
#define UARTDATA_MASK		0x3ff

#define UARTMODIR_IREN		0x00020000
#define UARTMODIR_TXCTSSRC	0x00000020
#define UARTMODIR_TXCTSC	0x00000010
#define UARTMODIR_RXRTSE	0x00000008
#define UARTMODIR_TXRTSPOL	0x00000004
#define UARTMODIR_TXRTSE	0x00000002
#define UARTMODIR_TXCTSE	0x00000001

#define UARTFIFO_TXEMPT		0x00800000
#define UARTFIFO_RXEMPT		0x00400000
#define UARTFIFO_TXOF		0x00020000
#define UARTFIFO_RXUF		0x00010000
#define UARTFIFO_TXFLUSH	0x00008000
#define UARTFIFO_RXFLUSH	0x00004000
#define UARTFIFO_TXOFE		0x00000200
#define UARTFIFO_RXUFE		0x00000100
#define UARTFIFO_TXFE		0x00000080
#define UARTFIFO_FIFOSIZE_MASK	0x7
#define UARTFIFO_TXSIZE_OFF	4
#define UARTFIFO_RXFE		0x00000008
#define UARTFIFO_RXSIZE_OFF	0

#define UARTWATER_COUNT_MASK	0xff
#define UARTWATER_TXCNT_OFF	8
#define UARTWATER_RXCNT_OFF	24
#define UARTWATER_WATER_MASK	0xff
#define UARTWATER_TXWATER_OFF	0
#define UARTWATER_RXWATER_OFF	16

#define FSL_UART_RX_DMA_BUFFER_SIZE	64

#define DRIVER_NAME	"fsl-lpuart"
#define DEV_NAME	"ttyLP"
#define UART_NR		6

struct lpuart_port {
	struct uart_port	port;
	struct clk		*clk;
	unsigned int		txfifo_size;
	unsigned int		rxfifo_size;
	bool			lpuart32;

	bool			lpuart_dma_tx_use;
	bool			lpuart_dma_rx_use;
	struct dma_chan		*dma_tx_chan;
	struct dma_chan		*dma_rx_chan;
	struct dma_async_tx_descriptor  *dma_tx_desc;
	struct dma_async_tx_descriptor  *dma_rx_desc;
	dma_addr_t		dma_tx_buf_bus;
	dma_addr_t		dma_rx_buf_bus;
	dma_cookie_t		dma_tx_cookie;
	dma_cookie_t		dma_rx_cookie;
	unsigned char		*dma_tx_buf_virt;
	unsigned char		*dma_rx_buf_virt;
	unsigned int		dma_tx_bytes;
	unsigned int		dma_rx_bytes;
	int			dma_tx_in_progress;
	int			dma_rx_in_progress;
	unsigned int		dma_rx_timeout;
	struct timer_list	lpuart_timer;
};

static const struct of_device_id lpuart_dt_ids[] = {
	{
		.compatible = "fsl,vf610-lpuart",
	},
	{
		.compatible = "fsl,ls1021a-lpuart",
	},
	{ /* sentinel */ }
};
MODULE_DEVICE_TABLE(of, lpuart_dt_ids);

/* Forward declare this for the dma callbacks*/
static void lpuart_dma_tx_complete(void *arg);
static void lpuart_dma_rx_complete(void *arg);

static u32 lpuart32_read(void __iomem *addr)
{
	return ioread32be(addr);
}

static void lpuart32_write(u32 val, void __iomem *addr)
{
	iowrite32be(val, addr);
}

static void lpuart_stop_tx(struct uart_port *port)
{
	unsigned char temp;

	temp = readb(port->membase + UARTCR2);
	temp &= ~(UARTCR2_TIE | UARTCR2_TCIE);
	writeb(temp, port->membase + UARTCR2);
}

static void lpuart32_stop_tx(struct uart_port *port)
{
	unsigned long temp;

	temp = lpuart32_read(port->membase + UARTCTRL);
	temp &= ~(UARTCTRL_TIE | UARTCTRL_TCIE);
	lpuart32_write(temp, port->membase + UARTCTRL);
}

static void lpuart_stop_rx(struct uart_port *port)
{
	unsigned char temp;

	temp = readb(port->membase + UARTCR2);
	writeb(temp & ~UARTCR2_RE, port->membase + UARTCR2);
}

static void lpuart32_stop_rx(struct uart_port *port)
{
	unsigned long temp;

	temp = lpuart32_read(port->membase + UARTCTRL);
	lpuart32_write(temp & ~UARTCTRL_RE, port->membase + UARTCTRL);
}

static void lpuart_copy_rx_to_tty(struct lpuart_port *sport,
		struct tty_port *tty, int count)
{
	int copied;

	sport->port.icount.rx += count;

	if (!tty) {
		dev_err(sport->port.dev, "No tty port\n");
		return;
	}

	dma_sync_single_for_cpu(sport->port.dev, sport->dma_rx_buf_bus,
			FSL_UART_RX_DMA_BUFFER_SIZE, DMA_FROM_DEVICE);
	copied = tty_insert_flip_string(tty,
			((unsigned char *)(sport->dma_rx_buf_virt)), count);

	if (copied != count) {
		WARN_ON(1);
		dev_err(sport->port.dev, "RxData copy to tty layer failed\n");
	}

	dma_sync_single_for_device(sport->port.dev, sport->dma_rx_buf_bus,
			FSL_UART_RX_DMA_BUFFER_SIZE, DMA_TO_DEVICE);
}

static void lpuart_pio_tx(struct lpuart_port *sport)
{
	struct circ_buf *xmit = &sport->port.state->xmit;
	unsigned long flags;

	spin_lock_irqsave(&sport->port.lock, flags);

	while (!uart_circ_empty(xmit) &&
		readb(sport->port.membase + UARTTCFIFO) < sport->txfifo_size) {
		writeb(xmit->buf[xmit->tail], sport->port.membase + UARTDR);
		xmit->tail = (xmit->tail + 1) & (UART_XMIT_SIZE - 1);
		sport->port.icount.tx++;
	}

	if (uart_circ_chars_pending(xmit) < WAKEUP_CHARS)
		uart_write_wakeup(&sport->port);

	if (uart_circ_empty(xmit))
		writeb(readb(sport->port.membase + UARTCR5) | UARTCR5_TDMAS,
			sport->port.membase + UARTCR5);

	spin_unlock_irqrestore(&sport->port.lock, flags);
}

static int lpuart_dma_tx(struct lpuart_port *sport, unsigned long count)
{
	struct circ_buf *xmit = &sport->port.state->xmit;
	dma_addr_t tx_bus_addr;

	dma_sync_single_for_device(sport->port.dev, sport->dma_tx_buf_bus,
				UART_XMIT_SIZE, DMA_TO_DEVICE);
	sport->dma_tx_bytes = count & ~(sport->txfifo_size - 1);
	tx_bus_addr = sport->dma_tx_buf_bus + xmit->tail;
	sport->dma_tx_desc = dmaengine_prep_slave_single(sport->dma_tx_chan,
					tx_bus_addr, sport->dma_tx_bytes,
					DMA_MEM_TO_DEV, DMA_PREP_INTERRUPT);

	if (!sport->dma_tx_desc) {
		dev_err(sport->port.dev, "Not able to get desc for tx\n");
		return -EIO;
	}

	sport->dma_tx_desc->callback = lpuart_dma_tx_complete;
	sport->dma_tx_desc->callback_param = sport;
	sport->dma_tx_in_progress = 1;
	sport->dma_tx_cookie = dmaengine_submit(sport->dma_tx_desc);
	dma_async_issue_pending(sport->dma_tx_chan);

	return 0;
}

static void lpuart_prepare_tx(struct lpuart_port *sport)
{
	struct circ_buf *xmit = &sport->port.state->xmit;
	unsigned long count =  CIRC_CNT_TO_END(xmit->head,
					xmit->tail, UART_XMIT_SIZE);

	if (!count)
		return;

	if (count < sport->txfifo_size)
		writeb(readb(sport->port.membase + UARTCR5) & ~UARTCR5_TDMAS,
				sport->port.membase + UARTCR5);
	else {
		writeb(readb(sport->port.membase + UARTCR5) | UARTCR5_TDMAS,
				sport->port.membase + UARTCR5);
		lpuart_dma_tx(sport, count);
	}
}

static void lpuart_dma_tx_complete(void *arg)
{
	struct lpuart_port *sport = arg;
	struct circ_buf *xmit = &sport->port.state->xmit;
	unsigned long flags;

	async_tx_ack(sport->dma_tx_desc);

	spin_lock_irqsave(&sport->port.lock, flags);

	xmit->tail = (xmit->tail + sport->dma_tx_bytes) & (UART_XMIT_SIZE - 1);
	sport->dma_tx_in_progress = 0;

	if (uart_circ_chars_pending(xmit) < WAKEUP_CHARS)
		uart_write_wakeup(&sport->port);

	lpuart_prepare_tx(sport);

	spin_unlock_irqrestore(&sport->port.lock, flags);
}

static int lpuart_dma_rx(struct lpuart_port *sport)
{
	dma_sync_single_for_device(sport->port.dev, sport->dma_rx_buf_bus,
			FSL_UART_RX_DMA_BUFFER_SIZE, DMA_TO_DEVICE);
	sport->dma_rx_desc = dmaengine_prep_slave_single(sport->dma_rx_chan,
			sport->dma_rx_buf_bus, FSL_UART_RX_DMA_BUFFER_SIZE,
			DMA_DEV_TO_MEM, DMA_PREP_INTERRUPT);

	if (!sport->dma_rx_desc) {
		dev_err(sport->port.dev, "Not able to get desc for rx\n");
		return -EIO;
	}

	sport->dma_rx_desc->callback = lpuart_dma_rx_complete;
	sport->dma_rx_desc->callback_param = sport;
	sport->dma_rx_in_progress = 1;
	sport->dma_rx_cookie = dmaengine_submit(sport->dma_rx_desc);
	dma_async_issue_pending(sport->dma_rx_chan);

	return 0;
}

static void lpuart_flush_buffer(struct uart_port *port)
{
	struct lpuart_port *sport = container_of(port, struct lpuart_port, port);
	if (sport->lpuart_dma_tx_use) {
		dmaengine_terminate_all(sport->dma_tx_chan);
		sport->dma_tx_in_progress = 0;
	}
}

static void lpuart_dma_rx_complete(void *arg)
{
	struct lpuart_port *sport = arg;
	struct tty_port *port = &sport->port.state->port;
	unsigned long flags;

	async_tx_ack(sport->dma_rx_desc);
	mod_timer(&sport->lpuart_timer, jiffies + sport->dma_rx_timeout);

	spin_lock_irqsave(&sport->port.lock, flags);

	sport->dma_rx_in_progress = 0;
	lpuart_copy_rx_to_tty(sport, port, FSL_UART_RX_DMA_BUFFER_SIZE);
	tty_flip_buffer_push(port);
	lpuart_dma_rx(sport);

	spin_unlock_irqrestore(&sport->port.lock, flags);
}

static void lpuart_timer_func(unsigned long data)
{
	struct lpuart_port *sport = (struct lpuart_port *)data;
	struct tty_port *port = &sport->port.state->port;
	struct dma_tx_state state;
	unsigned long flags;
	unsigned char temp;
	int count;

	del_timer(&sport->lpuart_timer);
	dmaengine_pause(sport->dma_rx_chan);
	dmaengine_tx_status(sport->dma_rx_chan, sport->dma_rx_cookie, &state);
	dmaengine_terminate_all(sport->dma_rx_chan);
	count = FSL_UART_RX_DMA_BUFFER_SIZE - state.residue;
	async_tx_ack(sport->dma_rx_desc);

	spin_lock_irqsave(&sport->port.lock, flags);

	sport->dma_rx_in_progress = 0;
	lpuart_copy_rx_to_tty(sport, port, count);
	tty_flip_buffer_push(port);
	temp = readb(sport->port.membase + UARTCR5);
	writeb(temp & ~UARTCR5_RDMAS, sport->port.membase + UARTCR5);

	spin_unlock_irqrestore(&sport->port.lock, flags);
}

static inline void lpuart_prepare_rx(struct lpuart_port *sport)
{
	unsigned long flags;
	unsigned char temp;

	spin_lock_irqsave(&sport->port.lock, flags);

	sport->lpuart_timer.expires = jiffies + sport->dma_rx_timeout;
	add_timer(&sport->lpuart_timer);

	lpuart_dma_rx(sport);
	temp = readb(sport->port.membase + UARTCR5);
	writeb(temp | UARTCR5_RDMAS, sport->port.membase + UARTCR5);

	spin_unlock_irqrestore(&sport->port.lock, flags);
}

static inline void lpuart_transmit_buffer(struct lpuart_port *sport)
{
	struct circ_buf *xmit = &sport->port.state->xmit;

	while (!uart_circ_empty(xmit) &&
		(readb(sport->port.membase + UARTTCFIFO) < sport->txfifo_size)) {
		writeb(xmit->buf[xmit->tail], sport->port.membase + UARTDR);
		xmit->tail = (xmit->tail + 1) & (UART_XMIT_SIZE - 1);
		sport->port.icount.tx++;
	}

	if (uart_circ_chars_pending(xmit) < WAKEUP_CHARS)
		uart_write_wakeup(&sport->port);

	if (uart_circ_empty(xmit))
		lpuart_stop_tx(&sport->port);
}

static inline void lpuart32_transmit_buffer(struct lpuart_port *sport)
{
	struct circ_buf *xmit = &sport->port.state->xmit;
	unsigned long txcnt;

	txcnt = lpuart32_read(sport->port.membase + UARTWATER);
	txcnt = txcnt >> UARTWATER_TXCNT_OFF;
	txcnt &= UARTWATER_COUNT_MASK;
	while (!uart_circ_empty(xmit) && (txcnt < sport->txfifo_size)) {
		lpuart32_write(xmit->buf[xmit->tail], sport->port.membase + UARTDATA);
		xmit->tail = (xmit->tail + 1) & (UART_XMIT_SIZE - 1);
		sport->port.icount.tx++;
		txcnt = lpuart32_read(sport->port.membase + UARTWATER);
		txcnt = txcnt >> UARTWATER_TXCNT_OFF;
		txcnt &= UARTWATER_COUNT_MASK;
	}

	if (uart_circ_chars_pending(xmit) < WAKEUP_CHARS)
		uart_write_wakeup(&sport->port);

	if (uart_circ_empty(xmit))
		lpuart32_stop_tx(&sport->port);
}

static void lpuart_start_tx(struct uart_port *port)
{
	struct lpuart_port *sport = container_of(port,
			struct lpuart_port, port);
	struct circ_buf *xmit = &sport->port.state->xmit;
	unsigned char temp;

	temp = readb(port->membase + UARTCR2);
	writeb(temp | UARTCR2_TIE, port->membase + UARTCR2);

	if (sport->lpuart_dma_tx_use) {
		if (!uart_circ_empty(xmit) && !sport->dma_tx_in_progress)
			lpuart_prepare_tx(sport);
	} else {
		if (readb(port->membase + UARTSR1) & UARTSR1_TDRE)
			lpuart_transmit_buffer(sport);
	}
}

static void lpuart32_start_tx(struct uart_port *port)
{
	struct lpuart_port *sport = container_of(port, struct lpuart_port, port);
	unsigned long temp;

	temp = lpuart32_read(port->membase + UARTCTRL);
	lpuart32_write(temp | UARTCTRL_TIE, port->membase + UARTCTRL);

	if (lpuart32_read(port->membase + UARTSTAT) & UARTSTAT_TDRE)
		lpuart32_transmit_buffer(sport);
}

static irqreturn_t lpuart_txint(int irq, void *dev_id)
{
	struct lpuart_port *sport = dev_id;
	struct circ_buf *xmit = &sport->port.state->xmit;
	unsigned long flags;

	spin_lock_irqsave(&sport->port.lock, flags);
	if (sport->port.x_char) {
		if (sport->lpuart32)
			lpuart32_write(sport->port.x_char, sport->port.membase + UARTDATA);
		else
			writeb(sport->port.x_char, sport->port.membase + UARTDR);
		goto out;
	}

	if (uart_circ_empty(xmit) || uart_tx_stopped(&sport->port)) {
		if (sport->lpuart32)
			lpuart32_stop_tx(&sport->port);
		else
			lpuart_stop_tx(&sport->port);
		goto out;
	}

	if (sport->lpuart32)
		lpuart32_transmit_buffer(sport);
	else
		lpuart_transmit_buffer(sport);

	if (uart_circ_chars_pending(xmit) < WAKEUP_CHARS)
		uart_write_wakeup(&sport->port);

out:
	spin_unlock_irqrestore(&sport->port.lock, flags);
	return IRQ_HANDLED;
}

static irqreturn_t lpuart_rxint(int irq, void *dev_id)
{
	struct lpuart_port *sport = dev_id;
	unsigned int flg, ignored = 0;
	struct tty_port *port = &sport->port.state->port;
	unsigned long flags;
	unsigned char rx, sr;

	spin_lock_irqsave(&sport->port.lock, flags);

	while (!(readb(sport->port.membase + UARTSFIFO) & UARTSFIFO_RXEMPT)) {
		flg = TTY_NORMAL;
		sport->port.icount.rx++;
		/*
		 * to clear the FE, OR, NF, FE, PE flags,
		 * read SR1 then read DR
		 */
		sr = readb(sport->port.membase + UARTSR1);
		rx = readb(sport->port.membase + UARTDR);

		if (uart_handle_sysrq_char(&sport->port, (unsigned char)rx))
			continue;

		if (sr & (UARTSR1_PE | UARTSR1_OR | UARTSR1_FE)) {
			if (sr & UARTSR1_PE)
				sport->port.icount.parity++;
			else if (sr & UARTSR1_FE)
				sport->port.icount.frame++;

			if (sr & UARTSR1_OR)
				sport->port.icount.overrun++;

			if (sr & sport->port.ignore_status_mask) {
				if (++ignored > 100)
					goto out;
				continue;
			}

			sr &= sport->port.read_status_mask;

			if (sr & UARTSR1_PE)
				flg = TTY_PARITY;
			else if (sr & UARTSR1_FE)
				flg = TTY_FRAME;

			if (sr & UARTSR1_OR)
				flg = TTY_OVERRUN;

#ifdef SUPPORT_SYSRQ
			sport->port.sysrq = 0;
#endif
		}

		tty_insert_flip_char(port, rx, flg);
	}

out:
	spin_unlock_irqrestore(&sport->port.lock, flags);

	tty_flip_buffer_push(port);
	return IRQ_HANDLED;
}

static irqreturn_t lpuart32_rxint(int irq, void *dev_id)
{
	struct lpuart_port *sport = dev_id;
	unsigned int flg, ignored = 0;
	struct tty_port *port = &sport->port.state->port;
	unsigned long flags;
	unsigned long rx, sr;

	spin_lock_irqsave(&sport->port.lock, flags);

	while (!(lpuart32_read(sport->port.membase + UARTFIFO) & UARTFIFO_RXEMPT)) {
		flg = TTY_NORMAL;
		sport->port.icount.rx++;
		/*
		 * to clear the FE, OR, NF, FE, PE flags,
		 * read STAT then read DATA reg
		 */
		sr = lpuart32_read(sport->port.membase + UARTSTAT);
		rx = lpuart32_read(sport->port.membase + UARTDATA);
		rx &= 0x3ff;

		if (uart_handle_sysrq_char(&sport->port, (unsigned char)rx))
			continue;

		if (sr & (UARTSTAT_PE | UARTSTAT_OR | UARTSTAT_FE)) {
			if (sr & UARTSTAT_PE)
				sport->port.icount.parity++;
			else if (sr & UARTSTAT_FE)
				sport->port.icount.frame++;

			if (sr & UARTSTAT_OR)
				sport->port.icount.overrun++;

			if (sr & sport->port.ignore_status_mask) {
				if (++ignored > 100)
					goto out;
				continue;
			}

			sr &= sport->port.read_status_mask;

			if (sr & UARTSTAT_PE)
				flg = TTY_PARITY;
			else if (sr & UARTSTAT_FE)
				flg = TTY_FRAME;

			if (sr & UARTSTAT_OR)
				flg = TTY_OVERRUN;

#ifdef SUPPORT_SYSRQ
			sport->port.sysrq = 0;
#endif
		}

		tty_insert_flip_char(port, rx, flg);
	}

out:
	spin_unlock_irqrestore(&sport->port.lock, flags);

	tty_flip_buffer_push(port);
	return IRQ_HANDLED;
}

static irqreturn_t lpuart_int(int irq, void *dev_id)
{
	struct lpuart_port *sport = dev_id;
	unsigned char sts, crdma;

	sts = readb(sport->port.membase + UARTSR1);
	crdma = readb(sport->port.membase + UARTCR5);

	if (sts & UARTSR1_RDRF && !(crdma & UARTCR5_RDMAS)) {
		if (sport->lpuart_dma_rx_use)
			lpuart_prepare_rx(sport);
		else
			lpuart_rxint(irq, dev_id);
	}
	if (sts & UARTSR1_TDRE && !(crdma & UARTCR5_TDMAS)) {
		if (sport->lpuart_dma_tx_use)
			lpuart_pio_tx(sport);
		else
			lpuart_txint(irq, dev_id);
	}

	return IRQ_HANDLED;
}

static irqreturn_t lpuart32_int(int irq, void *dev_id)
{
	struct lpuart_port *sport = dev_id;
	unsigned long sts, rxcount;

	sts = lpuart32_read(sport->port.membase + UARTSTAT);
	rxcount = lpuart32_read(sport->port.membase + UARTWATER);
	rxcount = rxcount >> UARTWATER_RXCNT_OFF;

	if (sts & UARTSTAT_RDRF || rxcount > 0)
		lpuart32_rxint(irq, dev_id);

	if ((sts & UARTSTAT_TDRE) &&
		!(lpuart32_read(sport->port.membase + UARTBAUD) & UARTBAUD_TDMAE))
		lpuart_txint(irq, dev_id);

	lpuart32_write(sts, sport->port.membase + UARTSTAT);
	return IRQ_HANDLED;
}

/* return TIOCSER_TEMT when transmitter is not busy */
static unsigned int lpuart_tx_empty(struct uart_port *port)
{
	return (readb(port->membase + UARTSR1) & UARTSR1_TC) ?
		TIOCSER_TEMT : 0;
}

static unsigned int lpuart32_tx_empty(struct uart_port *port)
{
	return (lpuart32_read(port->membase + UARTSTAT) & UARTSTAT_TC) ?
		TIOCSER_TEMT : 0;
}

static unsigned int lpuart_get_mctrl(struct uart_port *port)
{
	unsigned int temp = 0;
	unsigned char reg;

	reg = readb(port->membase + UARTMODEM);
	if (reg & UARTMODEM_TXCTSE)
		temp |= TIOCM_CTS;

	if (reg & UARTMODEM_RXRTSE)
		temp |= TIOCM_RTS;

	return temp;
}

static unsigned int lpuart32_get_mctrl(struct uart_port *port)
{
	unsigned int temp = 0;
	unsigned long reg;

	reg = lpuart32_read(port->membase + UARTMODIR);
	if (reg & UARTMODIR_TXCTSE)
		temp |= TIOCM_CTS;

	if (reg & UARTMODIR_RXRTSE)
		temp |= TIOCM_RTS;

	return temp;
}

static void lpuart_set_mctrl(struct uart_port *port, unsigned int mctrl)
{
	unsigned char temp;

	temp = readb(port->membase + UARTMODEM) &
			~(UARTMODEM_RXRTSE | UARTMODEM_TXCTSE);

	if (mctrl & TIOCM_RTS)
		temp |= UARTMODEM_RXRTSE;

	if (mctrl & TIOCM_CTS)
		temp |= UARTMODEM_TXCTSE;

	writeb(temp, port->membase + UARTMODEM);
}

static void lpuart32_set_mctrl(struct uart_port *port, unsigned int mctrl)
{
	unsigned long temp;

	temp = lpuart32_read(port->membase + UARTMODIR) &
			~(UARTMODIR_RXRTSE | UARTMODIR_TXCTSE);

	if (mctrl & TIOCM_RTS)
		temp |= UARTMODIR_RXRTSE;

	if (mctrl & TIOCM_CTS)
		temp |= UARTMODIR_TXCTSE;

	lpuart32_write(temp, port->membase + UARTMODIR);
}

static void lpuart_break_ctl(struct uart_port *port, int break_state)
{
	unsigned char temp;

	temp = readb(port->membase + UARTCR2) & ~UARTCR2_SBK;

	if (break_state != 0)
		temp |= UARTCR2_SBK;

	writeb(temp, port->membase + UARTCR2);
}

static void lpuart32_break_ctl(struct uart_port *port, int break_state)
{
	unsigned long temp;

	temp = lpuart32_read(port->membase + UARTCTRL) & ~UARTCTRL_SBK;

	if (break_state != 0)
		temp |= UARTCTRL_SBK;

	lpuart32_write(temp, port->membase + UARTCTRL);
}

static void lpuart_setup_watermark(struct lpuart_port *sport)
{
	unsigned char val, cr2;
	unsigned char cr2_saved;

	cr2 = readb(sport->port.membase + UARTCR2);
	cr2_saved = cr2;
	cr2 &= ~(UARTCR2_TIE | UARTCR2_TCIE | UARTCR2_TE |
			UARTCR2_RIE | UARTCR2_RE);
	writeb(cr2, sport->port.membase + UARTCR2);

	val = readb(sport->port.membase + UARTPFIFO);
	writeb(val | UARTPFIFO_TXFE | UARTPFIFO_RXFE,
			sport->port.membase + UARTPFIFO);

	/* explicitly clear RDRF */
	readb(sport->port.membase + UARTSR1);

	/* flush Tx and Rx FIFO */
	writeb(UARTCFIFO_TXFLUSH | UARTCFIFO_RXFLUSH,
			sport->port.membase + UARTCFIFO);

	writeb(0, sport->port.membase + UARTTWFIFO);
	writeb(1, sport->port.membase + UARTRWFIFO);

	/* Restore cr2 */
	writeb(cr2_saved, sport->port.membase + UARTCR2);
}

static void lpuart32_setup_watermark(struct lpuart_port *sport)
{
	unsigned long val, ctrl;
	unsigned long ctrl_saved;

	ctrl = lpuart32_read(sport->port.membase + UARTCTRL);
	ctrl_saved = ctrl;
	ctrl &= ~(UARTCTRL_TIE | UARTCTRL_TCIE | UARTCTRL_TE |
			UARTCTRL_RIE | UARTCTRL_RE);
	lpuart32_write(ctrl, sport->port.membase + UARTCTRL);

	/* enable FIFO mode */
	val = lpuart32_read(sport->port.membase + UARTFIFO);
	val |= UARTFIFO_TXFE | UARTFIFO_RXFE;
	val |= UARTFIFO_TXFLUSH | UARTFIFO_RXFLUSH;
	lpuart32_write(val, sport->port.membase + UARTFIFO);

	/* set the watermark */
	val = (0x1 << UARTWATER_RXWATER_OFF) | (0x0 << UARTWATER_TXWATER_OFF);
	lpuart32_write(val, sport->port.membase + UARTWATER);

	/* Restore cr2 */
	lpuart32_write(ctrl_saved, sport->port.membase + UARTCTRL);
}

static int lpuart_dma_tx_request(struct uart_port *port)
{
	struct lpuart_port *sport = container_of(port,
					struct lpuart_port, port);
	struct dma_slave_config dma_tx_sconfig;
	dma_addr_t dma_bus;
	unsigned char *dma_buf;
	int ret;

	dma_bus = dma_map_single(sport->dma_tx_chan->device->dev,
				sport->port.state->xmit.buf,
				UART_XMIT_SIZE, DMA_TO_DEVICE);

	if (dma_mapping_error(sport->dma_tx_chan->device->dev, dma_bus)) {
		dev_err(sport->port.dev, "dma_map_single tx failed\n");
		return -ENOMEM;
	}

	dma_buf = sport->port.state->xmit.buf;
	dma_tx_sconfig.dst_addr = sport->port.mapbase + UARTDR;
	dma_tx_sconfig.dst_addr_width = DMA_SLAVE_BUSWIDTH_1_BYTE;
	dma_tx_sconfig.dst_maxburst = sport->txfifo_size;
	dma_tx_sconfig.direction = DMA_MEM_TO_DEV;
	ret = dmaengine_slave_config(sport->dma_tx_chan, &dma_tx_sconfig);

	if (ret < 0) {
		dev_err(sport->port.dev,
				"Dma slave config failed, err = %d\n", ret);
		return ret;
	}

	sport->dma_tx_buf_virt = dma_buf;
	sport->dma_tx_buf_bus = dma_bus;
	sport->dma_tx_in_progress = 0;

	return 0;
}

static int lpuart_dma_rx_request(struct uart_port *port)
{
	struct lpuart_port *sport = container_of(port,
					struct lpuart_port, port);
	struct dma_slave_config dma_rx_sconfig;
	dma_addr_t dma_bus;
	unsigned char *dma_buf;
	int ret;

	dma_buf = devm_kzalloc(sport->port.dev,
				FSL_UART_RX_DMA_BUFFER_SIZE, GFP_KERNEL);

	if (!dma_buf) {
		dev_err(sport->port.dev, "Dma rx alloc failed\n");
		return -ENOMEM;
	}

	dma_bus = dma_map_single(sport->dma_rx_chan->device->dev, dma_buf,
				FSL_UART_RX_DMA_BUFFER_SIZE, DMA_FROM_DEVICE);

	if (dma_mapping_error(sport->dma_rx_chan->device->dev, dma_bus)) {
		dev_err(sport->port.dev, "dma_map_single rx failed\n");
		return -ENOMEM;
	}

	dma_rx_sconfig.src_addr = sport->port.mapbase + UARTDR;
	dma_rx_sconfig.src_addr_width = DMA_SLAVE_BUSWIDTH_1_BYTE;
	dma_rx_sconfig.src_maxburst = 1;
	dma_rx_sconfig.direction = DMA_DEV_TO_MEM;
	ret = dmaengine_slave_config(sport->dma_rx_chan, &dma_rx_sconfig);

	if (ret < 0) {
		dev_err(sport->port.dev,
				"Dma slave config failed, err = %d\n", ret);
		return ret;
	}

	sport->dma_rx_buf_virt = dma_buf;
	sport->dma_rx_buf_bus = dma_bus;
	sport->dma_rx_in_progress = 0;

	return 0;
}

static void lpuart_dma_tx_free(struct uart_port *port)
{
	struct lpuart_port *sport = container_of(port,
					struct lpuart_port, port);

	dma_unmap_single(sport->port.dev, sport->dma_tx_buf_bus,
			UART_XMIT_SIZE, DMA_TO_DEVICE);

	sport->dma_tx_buf_bus = 0;
	sport->dma_tx_buf_virt = NULL;
}

static void lpuart_dma_rx_free(struct uart_port *port)
{
	struct lpuart_port *sport = container_of(port,
					struct lpuart_port, port);

	dma_unmap_single(sport->port.dev, sport->dma_rx_buf_bus,
			FSL_UART_RX_DMA_BUFFER_SIZE, DMA_FROM_DEVICE);

	sport->dma_rx_buf_bus = 0;
	sport->dma_rx_buf_virt = NULL;
}

static int lpuart_startup(struct uart_port *port)
{
	struct lpuart_port *sport = container_of(port, struct lpuart_port, port);
	int ret;
	unsigned long flags;
	unsigned char temp;

	/* determine FIFO size and enable FIFO mode */
	temp = readb(sport->port.membase + UARTPFIFO);

	sport->txfifo_size = 0x1 << (((temp >> UARTPFIFO_TXSIZE_OFF) &
		UARTPFIFO_FIFOSIZE_MASK) + 1);

	sport->port.fifosize = sport->txfifo_size;

	sport->rxfifo_size = 0x1 << (((temp >> UARTPFIFO_RXSIZE_OFF) &
		UARTPFIFO_FIFOSIZE_MASK) + 1);

	if (sport->dma_rx_chan && !lpuart_dma_rx_request(port)) {
		sport->lpuart_dma_rx_use = true;
		setup_timer(&sport->lpuart_timer, lpuart_timer_func,
			    (unsigned long)sport);
	} else
		sport->lpuart_dma_rx_use = false;


	if (sport->dma_tx_chan && !lpuart_dma_tx_request(port)) {
		sport->lpuart_dma_tx_use = true;
		temp = readb(port->membase + UARTCR5);
		temp &= ~UARTCR5_RDMAS;
		writeb(temp | UARTCR5_TDMAS, port->membase + UARTCR5);
	} else
		sport->lpuart_dma_tx_use = false;

	ret = devm_request_irq(port->dev, port->irq, lpuart_int, 0,
				DRIVER_NAME, sport);
	if (ret)
		return ret;

	spin_lock_irqsave(&sport->port.lock, flags);

	lpuart_setup_watermark(sport);

	temp = readb(sport->port.membase + UARTCR2);
	temp |= (UARTCR2_RIE | UARTCR2_TIE | UARTCR2_RE | UARTCR2_TE);
	writeb(temp, sport->port.membase + UARTCR2);

	spin_unlock_irqrestore(&sport->port.lock, flags);
	return 0;
}

static int lpuart32_startup(struct uart_port *port)
{
	struct lpuart_port *sport = container_of(port, struct lpuart_port, port);
	int ret;
	unsigned long flags;
	unsigned long temp;

	/* determine FIFO size */
	temp = lpuart32_read(sport->port.membase + UARTFIFO);

	sport->txfifo_size = 0x1 << (((temp >> UARTFIFO_TXSIZE_OFF) &
		UARTFIFO_FIFOSIZE_MASK) - 1);

	sport->rxfifo_size = 0x1 << (((temp >> UARTFIFO_RXSIZE_OFF) &
		UARTFIFO_FIFOSIZE_MASK) - 1);

	ret = devm_request_irq(port->dev, port->irq, lpuart32_int, 0,
				DRIVER_NAME, sport);
	if (ret)
		return ret;

	spin_lock_irqsave(&sport->port.lock, flags);

	lpuart32_setup_watermark(sport);

	temp = lpuart32_read(sport->port.membase + UARTCTRL);
	temp |= (UARTCTRL_RIE | UARTCTRL_TIE | UARTCTRL_RE | UARTCTRL_TE);
	temp |= UARTCTRL_ILIE;
	lpuart32_write(temp, sport->port.membase + UARTCTRL);

	spin_unlock_irqrestore(&sport->port.lock, flags);
	return 0;
}

static void lpuart_shutdown(struct uart_port *port)
{
	struct lpuart_port *sport = container_of(port, struct lpuart_port, port);
	unsigned char temp;
	unsigned long flags;

	spin_lock_irqsave(&port->lock, flags);

	/* disable Rx/Tx and interrupts */
	temp = readb(port->membase + UARTCR2);
	temp &= ~(UARTCR2_TE | UARTCR2_RE |
			UARTCR2_TIE | UARTCR2_TCIE | UARTCR2_RIE);
	writeb(temp, port->membase + UARTCR2);

	spin_unlock_irqrestore(&port->lock, flags);

	devm_free_irq(port->dev, port->irq, sport);

	if (sport->lpuart_dma_rx_use) {
		lpuart_dma_rx_free(&sport->port);
		del_timer_sync(&sport->lpuart_timer);
	}

	if (sport->lpuart_dma_tx_use)
		lpuart_dma_tx_free(&sport->port);
}

static void lpuart32_shutdown(struct uart_port *port)
{
	struct lpuart_port *sport = container_of(port, struct lpuart_port, port);
	unsigned long temp;
	unsigned long flags;

	spin_lock_irqsave(&port->lock, flags);

	/* disable Rx/Tx and interrupts */
	temp = lpuart32_read(port->membase + UARTCTRL);
	temp &= ~(UARTCTRL_TE | UARTCTRL_RE |
			UARTCTRL_TIE | UARTCTRL_TCIE | UARTCTRL_RIE);
	lpuart32_write(temp, port->membase + UARTCTRL);

	spin_unlock_irqrestore(&port->lock, flags);

	devm_free_irq(port->dev, port->irq, sport);
}

static void
lpuart_set_termios(struct uart_port *port, struct ktermios *termios,
		   struct ktermios *old)
{
	struct lpuart_port *sport = container_of(port, struct lpuart_port, port);
	unsigned long flags;
	unsigned char cr1, old_cr1, old_cr2, cr4, bdh, modem;
	unsigned int  baud;
	unsigned int old_csize = old ? old->c_cflag & CSIZE : CS8;
	unsigned int sbr, brfa;

	cr1 = old_cr1 = readb(sport->port.membase + UARTCR1);
	old_cr2 = readb(sport->port.membase + UARTCR2);
	cr4 = readb(sport->port.membase + UARTCR4);
	bdh = readb(sport->port.membase + UARTBDH);
	modem = readb(sport->port.membase + UARTMODEM);
	/*
	 * only support CS8 and CS7, and for CS7 must enable PE.
	 * supported mode:
	 *  - (7,e/o,1)
	 *  - (8,n,1)
	 *  - (8,m/s,1)
	 *  - (8,e/o,1)
	 */
	while ((termios->c_cflag & CSIZE) != CS8 &&
		(termios->c_cflag & CSIZE) != CS7) {
		termios->c_cflag &= ~CSIZE;
		termios->c_cflag |= old_csize;
		old_csize = CS8;
	}

	if ((termios->c_cflag & CSIZE) == CS8 ||
		(termios->c_cflag & CSIZE) == CS7)
		cr1 = old_cr1 & ~UARTCR1_M;

	if (termios->c_cflag & CMSPAR) {
		if ((termios->c_cflag & CSIZE) != CS8) {
			termios->c_cflag &= ~CSIZE;
			termios->c_cflag |= CS8;
		}
		cr1 |= UARTCR1_M;
	}

	if (termios->c_cflag & CRTSCTS) {
		modem |= (UARTMODEM_RXRTSE | UARTMODEM_TXCTSE);
	} else {
		termios->c_cflag &= ~CRTSCTS;
		modem &= ~(UARTMODEM_RXRTSE | UARTMODEM_TXCTSE);
	}

	if (termios->c_cflag & CSTOPB)
		termios->c_cflag &= ~CSTOPB;

	/* parity must be enabled when CS7 to match 8-bits format */
	if ((termios->c_cflag & CSIZE) == CS7)
		termios->c_cflag |= PARENB;

	if ((termios->c_cflag & PARENB)) {
		if (termios->c_cflag & CMSPAR) {
			cr1 &= ~UARTCR1_PE;
			cr1 |= UARTCR1_M;
		} else {
			cr1 |= UARTCR1_PE;
			if ((termios->c_cflag & CSIZE) == CS8)
				cr1 |= UARTCR1_M;
			if (termios->c_cflag & PARODD)
				cr1 |= UARTCR1_PT;
			else
				cr1 &= ~UARTCR1_PT;
		}
	}

	/* ask the core to calculate the divisor */
	baud = uart_get_baud_rate(port, termios, old, 50, port->uartclk / 16);

	spin_lock_irqsave(&sport->port.lock, flags);

	sport->port.read_status_mask = 0;
	if (termios->c_iflag & INPCK)
		sport->port.read_status_mask |=	(UARTSR1_FE | UARTSR1_PE);
	if (termios->c_iflag & (IGNBRK | BRKINT | PARMRK))
		sport->port.read_status_mask |= UARTSR1_FE;

	/* characters to ignore */
	sport->port.ignore_status_mask = 0;
	if (termios->c_iflag & IGNPAR)
		sport->port.ignore_status_mask |= UARTSR1_PE;
	if (termios->c_iflag & IGNBRK) {
		sport->port.ignore_status_mask |= UARTSR1_FE;
		/*
		 * if we're ignoring parity and break indicators,
		 * ignore overruns too (for real raw support).
		 */
		if (termios->c_iflag & IGNPAR)
			sport->port.ignore_status_mask |= UARTSR1_OR;
	}

	/* update the per-port timeout */
	uart_update_timeout(port, termios->c_cflag, baud);

	if (sport->lpuart_dma_rx_use) {
		/* Calculate delay for 1.5 DMA buffers */
		sport->dma_rx_timeout = (sport->port.timeout - HZ / 50) *
					FSL_UART_RX_DMA_BUFFER_SIZE * 3 /
					sport->rxfifo_size / 2;
		dev_dbg(port->dev, "DMA Rx t-out %ums, tty t-out %u jiffies\n",
			sport->dma_rx_timeout * 1000 / HZ, sport->port.timeout);
		if (sport->dma_rx_timeout < msecs_to_jiffies(20))
			sport->dma_rx_timeout = msecs_to_jiffies(20);
	}

	/* wait transmit engin complete */
	while (!(readb(sport->port.membase + UARTSR1) & UARTSR1_TC))
		barrier();

	/* disable transmit and receive */
	writeb(old_cr2 & ~(UARTCR2_TE | UARTCR2_RE),
			sport->port.membase + UARTCR2);

	sbr = sport->port.uartclk / (16 * baud);
	brfa = ((sport->port.uartclk - (16 * sbr * baud)) * 2) / baud;
	bdh &= ~UARTBDH_SBR_MASK;
	bdh |= (sbr >> 8) & 0x1F;
	cr4 &= ~UARTCR4_BRFA_MASK;
	brfa &= UARTCR4_BRFA_MASK;
	writeb(cr4 | brfa, sport->port.membase + UARTCR4);
	writeb(bdh, sport->port.membase + UARTBDH);
	writeb(sbr & 0xFF, sport->port.membase + UARTBDL);
	writeb(cr1, sport->port.membase + UARTCR1);
	writeb(modem, sport->port.membase + UARTMODEM);

	/* restore control register */
	writeb(old_cr2, sport->port.membase + UARTCR2);

	spin_unlock_irqrestore(&sport->port.lock, flags);
}

static void
lpuart32_set_termios(struct uart_port *port, struct ktermios *termios,
		   struct ktermios *old)
{
	struct lpuart_port *sport = container_of(port, struct lpuart_port, port);
	unsigned long flags;
	unsigned long ctrl, old_ctrl, bd, modem;
	unsigned int  baud;
	unsigned int old_csize = old ? old->c_cflag & CSIZE : CS8;
	unsigned int sbr;

	ctrl = old_ctrl = lpuart32_read(sport->port.membase + UARTCTRL);
	bd = lpuart32_read(sport->port.membase + UARTBAUD);
	modem = lpuart32_read(sport->port.membase + UARTMODIR);
	/*
	 * only support CS8 and CS7, and for CS7 must enable PE.
	 * supported mode:
	 *  - (7,e/o,1)
	 *  - (8,n,1)
	 *  - (8,m/s,1)
	 *  - (8,e/o,1)
	 */
	while ((termios->c_cflag & CSIZE) != CS8 &&
		(termios->c_cflag & CSIZE) != CS7) {
		termios->c_cflag &= ~CSIZE;
		termios->c_cflag |= old_csize;
		old_csize = CS8;
	}

	if ((termios->c_cflag & CSIZE) == CS8 ||
		(termios->c_cflag & CSIZE) == CS7)
		ctrl = old_ctrl & ~UARTCTRL_M;

	if (termios->c_cflag & CMSPAR) {
		if ((termios->c_cflag & CSIZE) != CS8) {
			termios->c_cflag &= ~CSIZE;
			termios->c_cflag |= CS8;
		}
		ctrl |= UARTCTRL_M;
	}

	if (termios->c_cflag & CRTSCTS) {
		modem |= (UARTMODEM_RXRTSE | UARTMODEM_TXCTSE);
	} else {
		termios->c_cflag &= ~CRTSCTS;
		modem &= ~(UARTMODEM_RXRTSE | UARTMODEM_TXCTSE);
	}

	if (termios->c_cflag & CSTOPB)
		termios->c_cflag &= ~CSTOPB;

	/* parity must be enabled when CS7 to match 8-bits format */
	if ((termios->c_cflag & CSIZE) == CS7)
		termios->c_cflag |= PARENB;

	if ((termios->c_cflag & PARENB)) {
		if (termios->c_cflag & CMSPAR) {
			ctrl &= ~UARTCTRL_PE;
			ctrl |= UARTCTRL_M;
		} else {
			ctrl |= UARTCR1_PE;
			if ((termios->c_cflag & CSIZE) == CS8)
				ctrl |= UARTCTRL_M;
			if (termios->c_cflag & PARODD)
				ctrl |= UARTCTRL_PT;
			else
				ctrl &= ~UARTCTRL_PT;
		}
	}

	/* ask the core to calculate the divisor */
	baud = uart_get_baud_rate(port, termios, old, 50, port->uartclk / 16);

	spin_lock_irqsave(&sport->port.lock, flags);

	sport->port.read_status_mask = 0;
	if (termios->c_iflag & INPCK)
		sport->port.read_status_mask |=	(UARTSTAT_FE | UARTSTAT_PE);
	if (termios->c_iflag & (IGNBRK | BRKINT | PARMRK))
		sport->port.read_status_mask |= UARTSTAT_FE;

	/* characters to ignore */
	sport->port.ignore_status_mask = 0;
	if (termios->c_iflag & IGNPAR)
		sport->port.ignore_status_mask |= UARTSTAT_PE;
	if (termios->c_iflag & IGNBRK) {
		sport->port.ignore_status_mask |= UARTSTAT_FE;
		/*
		 * if we're ignoring parity and break indicators,
		 * ignore overruns too (for real raw support).
		 */
		if (termios->c_iflag & IGNPAR)
			sport->port.ignore_status_mask |= UARTSTAT_OR;
	}

	/* update the per-port timeout */
	uart_update_timeout(port, termios->c_cflag, baud);

	/* wait transmit engin complete */
	while (!(lpuart32_read(sport->port.membase + UARTSTAT) & UARTSTAT_TC))
		barrier();

	/* disable transmit and receive */
	lpuart32_write(old_ctrl & ~(UARTCTRL_TE | UARTCTRL_RE),
			sport->port.membase + UARTCTRL);

	sbr = sport->port.uartclk / (16 * baud);
	bd &= ~UARTBAUD_SBR_MASK;
	bd |= sbr & UARTBAUD_SBR_MASK;
	bd |= UARTBAUD_BOTHEDGE;
	bd &= ~(UARTBAUD_TDMAE | UARTBAUD_RDMAE);
	lpuart32_write(bd, sport->port.membase + UARTBAUD);
	lpuart32_write(modem, sport->port.membase + UARTMODIR);
	lpuart32_write(ctrl, sport->port.membase + UARTCTRL);
	/* restore control register */

	spin_unlock_irqrestore(&sport->port.lock, flags);
}

static const char *lpuart_type(struct uart_port *port)
{
	return "FSL_LPUART";
}

static void lpuart_release_port(struct uart_port *port)
{
	/* nothing to do */
}

static int lpuart_request_port(struct uart_port *port)
{
	return  0;
}

/* configure/autoconfigure the port */
static void lpuart_config_port(struct uart_port *port, int flags)
{
	if (flags & UART_CONFIG_TYPE)
		port->type = PORT_LPUART;
}

static int lpuart_verify_port(struct uart_port *port, struct serial_struct *ser)
{
	int ret = 0;

	if (ser->type != PORT_UNKNOWN && ser->type != PORT_LPUART)
		ret = -EINVAL;
	if (port->irq != ser->irq)
		ret = -EINVAL;
	if (ser->io_type != UPIO_MEM)
		ret = -EINVAL;
	if (port->uartclk / 16 != ser->baud_base)
		ret = -EINVAL;
	if (port->iobase != ser->port)
		ret = -EINVAL;
	if (ser->hub6 != 0)
		ret = -EINVAL;
	return ret;
}

static struct uart_ops lpuart_pops = {
	.tx_empty	= lpuart_tx_empty,
	.set_mctrl	= lpuart_set_mctrl,
	.get_mctrl	= lpuart_get_mctrl,
	.stop_tx	= lpuart_stop_tx,
	.start_tx	= lpuart_start_tx,
	.stop_rx	= lpuart_stop_rx,
	.break_ctl	= lpuart_break_ctl,
	.startup	= lpuart_startup,
	.shutdown	= lpuart_shutdown,
	.set_termios	= lpuart_set_termios,
	.type		= lpuart_type,
	.request_port	= lpuart_request_port,
	.release_port	= lpuart_release_port,
	.config_port	= lpuart_config_port,
	.verify_port	= lpuart_verify_port,
	.flush_buffer	= lpuart_flush_buffer,
};

static struct uart_ops lpuart32_pops = {
	.tx_empty	= lpuart32_tx_empty,
	.set_mctrl	= lpuart32_set_mctrl,
	.get_mctrl	= lpuart32_get_mctrl,
	.stop_tx	= lpuart32_stop_tx,
	.start_tx	= lpuart32_start_tx,
	.stop_rx	= lpuart32_stop_rx,
	.break_ctl	= lpuart32_break_ctl,
	.startup	= lpuart32_startup,
	.shutdown	= lpuart32_shutdown,
	.set_termios	= lpuart32_set_termios,
	.type		= lpuart_type,
	.request_port	= lpuart_request_port,
	.release_port	= lpuart_release_port,
	.config_port	= lpuart_config_port,
	.verify_port	= lpuart_verify_port,
	.flush_buffer	= lpuart_flush_buffer,
};

static struct lpuart_port *lpuart_ports[UART_NR];

#ifdef CONFIG_SERIAL_FSL_LPUART_CONSOLE
static void lpuart_console_putchar(struct uart_port *port, int ch)
{
	while (!(readb(port->membase + UARTSR1) & UARTSR1_TDRE))
		barrier();

	writeb(ch, port->membase + UARTDR);
}

static void lpuart32_console_putchar(struct uart_port *port, int ch)
{
	while (!(lpuart32_read(port->membase + UARTSTAT) & UARTSTAT_TDRE))
		barrier();

	lpuart32_write(ch, port->membase + UARTDATA);
}

static void
lpuart_console_write(struct console *co, const char *s, unsigned int count)
{
	struct lpuart_port *sport = lpuart_ports[co->index];
	unsigned char  old_cr2, cr2;

	/* first save CR2 and then disable interrupts */
	cr2 = old_cr2 = readb(sport->port.membase + UARTCR2);
	cr2 |= (UARTCR2_TE |  UARTCR2_RE);
	cr2 &= ~(UARTCR2_TIE | UARTCR2_TCIE | UARTCR2_RIE);
	writeb(cr2, sport->port.membase + UARTCR2);

	uart_console_write(&sport->port, s, count, lpuart_console_putchar);

	/* wait for transmitter finish complete and restore CR2 */
	while (!(readb(sport->port.membase + UARTSR1) & UARTSR1_TC))
		barrier();

	writeb(old_cr2, sport->port.membase + UARTCR2);
}

static void
lpuart32_console_write(struct console *co, const char *s, unsigned int count)
{
	struct lpuart_port *sport = lpuart_ports[co->index];
	unsigned long  old_cr, cr;

	/* first save CR2 and then disable interrupts */
	cr = old_cr = lpuart32_read(sport->port.membase + UARTCTRL);
	cr |= (UARTCTRL_TE |  UARTCTRL_RE);
	cr &= ~(UARTCTRL_TIE | UARTCTRL_TCIE | UARTCTRL_RIE);
	lpuart32_write(cr, sport->port.membase + UARTCTRL);

	uart_console_write(&sport->port, s, count, lpuart32_console_putchar);

	/* wait for transmitter finish complete and restore CR2 */
	while (!(lpuart32_read(sport->port.membase + UARTSTAT) & UARTSTAT_TC))
		barrier();

	lpuart32_write(old_cr, sport->port.membase + UARTCTRL);
}

/*
 * if the port was already initialised (eg, by a boot loader),
 * try to determine the current setup.
 */
static void __init
lpuart_console_get_options(struct lpuart_port *sport, int *baud,
			   int *parity, int *bits)
{
	unsigned char cr, bdh, bdl, brfa;
	unsigned int sbr, uartclk, baud_raw;

	cr = readb(sport->port.membase + UARTCR2);
	cr &= UARTCR2_TE | UARTCR2_RE;
	if (!cr)
		return;

	/* ok, the port was enabled */

	cr = readb(sport->port.membase + UARTCR1);

	*parity = 'n';
	if (cr & UARTCR1_PE) {
		if (cr & UARTCR1_PT)
			*parity = 'o';
		else
			*parity = 'e';
	}

	if (cr & UARTCR1_M)
		*bits = 9;
	else
		*bits = 8;

	bdh = readb(sport->port.membase + UARTBDH);
	bdh &= UARTBDH_SBR_MASK;
	bdl = readb(sport->port.membase + UARTBDL);
	sbr = bdh;
	sbr <<= 8;
	sbr |= bdl;
	brfa = readb(sport->port.membase + UARTCR4);
	brfa &= UARTCR4_BRFA_MASK;

	uartclk = clk_get_rate(sport->clk);
	/*
	 * baud = mod_clk/(16*(sbr[13]+(brfa)/32)
	 */
	baud_raw = uartclk / (16 * (sbr + brfa / 32));

	if (*baud != baud_raw)
		printk(KERN_INFO "Serial: Console lpuart rounded baud rate"
				"from %d to %d\n", baud_raw, *baud);
}

static void __init
lpuart32_console_get_options(struct lpuart_port *sport, int *baud,
			   int *parity, int *bits)
{
	unsigned long cr, bd;
	unsigned int sbr, uartclk, baud_raw;

	cr = lpuart32_read(sport->port.membase + UARTCTRL);
	cr &= UARTCTRL_TE | UARTCTRL_RE;
	if (!cr)
		return;

	/* ok, the port was enabled */

	cr = lpuart32_read(sport->port.membase + UARTCTRL);

	*parity = 'n';
	if (cr & UARTCTRL_PE) {
		if (cr & UARTCTRL_PT)
			*parity = 'o';
		else
			*parity = 'e';
	}

	if (cr & UARTCTRL_M)
		*bits = 9;
	else
		*bits = 8;

	bd = lpuart32_read(sport->port.membase + UARTBAUD);
	bd &= UARTBAUD_SBR_MASK;
	sbr = bd;
	uartclk = clk_get_rate(sport->clk);
	/*
	 * baud = mod_clk/(16*(sbr[13]+(brfa)/32)
	 */
	baud_raw = uartclk / (16 * sbr);

	if (*baud != baud_raw)
		printk(KERN_INFO "Serial: Console lpuart rounded baud rate"
				"from %d to %d\n", baud_raw, *baud);
}

static int __init lpuart_console_setup(struct console *co, char *options)
{
	struct lpuart_port *sport;
	int baud = 115200;
	int bits = 8;
	int parity = 'n';
	int flow = 'n';

	/*
	 * check whether an invalid uart number has been specified, and
	 * if so, search for the first available port that does have
	 * console support.
	 */
	if (co->index == -1 || co->index >= ARRAY_SIZE(lpuart_ports))
		co->index = 0;

	sport = lpuart_ports[co->index];
	if (sport == NULL)
		return -ENODEV;

	if (options)
		uart_parse_options(options, &baud, &parity, &bits, &flow);
	else
		if (sport->lpuart32)
			lpuart32_console_get_options(sport, &baud, &parity, &bits);
		else
			lpuart_console_get_options(sport, &baud, &parity, &bits);

	if (sport->lpuart32)
		lpuart32_setup_watermark(sport);
	else
		lpuart_setup_watermark(sport);

	return uart_set_options(&sport->port, co, baud, parity, bits, flow);
}

static struct uart_driver lpuart_reg;
static struct console lpuart_console = {
	.name		= DEV_NAME,
	.write		= lpuart_console_write,
	.device		= uart_console_device,
	.setup		= lpuart_console_setup,
	.flags		= CON_PRINTBUFFER,
	.index		= -1,
	.data		= &lpuart_reg,
};

static struct console lpuart32_console = {
	.name		= DEV_NAME,
	.write		= lpuart32_console_write,
	.device		= uart_console_device,
	.setup		= lpuart_console_setup,
	.flags		= CON_PRINTBUFFER,
	.index		= -1,
	.data		= &lpuart_reg,
};

#define LPUART_CONSOLE	(&lpuart_console)
#define LPUART32_CONSOLE	(&lpuart32_console)
#else
#define LPUART_CONSOLE	NULL
#define LPUART32_CONSOLE	NULL
#endif

static struct uart_driver lpuart_reg = {
	.owner		= THIS_MODULE,
	.driver_name	= DRIVER_NAME,
	.dev_name	= DEV_NAME,
	.nr		= ARRAY_SIZE(lpuart_ports),
	.cons		= LPUART_CONSOLE,
};

static int lpuart_probe(struct platform_device *pdev)
{
	struct device_node *np = pdev->dev.of_node;
	struct lpuart_port *sport;
	struct resource *res;
	int ret;

	sport = devm_kzalloc(&pdev->dev, sizeof(*sport), GFP_KERNEL);
	if (!sport)
		return -ENOMEM;

	pdev->dev.coherent_dma_mask = 0;

	ret = of_alias_get_id(np, "serial");
	if (ret < 0) {
		dev_err(&pdev->dev, "failed to get alias id, errno %d\n", ret);
		return ret;
	}
	sport->port.line = ret;
	sport->lpuart32 = of_device_is_compatible(np, "fsl,ls1021a-lpuart");

	res = platform_get_resource(pdev, IORESOURCE_MEM, 0);
	sport->port.membase = devm_ioremap_resource(&pdev->dev, res);
	if (IS_ERR(sport->port.membase))
		return PTR_ERR(sport->port.membase);

	sport->port.mapbase = res->start;
	sport->port.dev = &pdev->dev;
	sport->port.type = PORT_LPUART;
	sport->port.iotype = UPIO_MEM;
	sport->port.irq = platform_get_irq(pdev, 0);
	if (sport->lpuart32)
		sport->port.ops = &lpuart32_pops;
	else
		sport->port.ops = &lpuart_pops;
	sport->port.flags = UPF_BOOT_AUTOCONF;

	sport->clk = devm_clk_get(&pdev->dev, "ipg");
	if (IS_ERR(sport->clk)) {
		ret = PTR_ERR(sport->clk);
		dev_err(&pdev->dev, "failed to get uart clk: %d\n", ret);
		return ret;
	}

	ret = clk_prepare_enable(sport->clk);
	if (ret) {
		dev_err(&pdev->dev, "failed to enable uart clk: %d\n", ret);
		return ret;
	}

	sport->port.uartclk = clk_get_rate(sport->clk);

	lpuart_ports[sport->port.line] = sport;

	platform_set_drvdata(pdev, &sport->port);

	if (sport->lpuart32)
		lpuart_reg.cons = LPUART32_CONSOLE;
	else
		lpuart_reg.cons = LPUART_CONSOLE;

	ret = uart_add_one_port(&lpuart_reg, &sport->port);
	if (ret) {
		clk_disable_unprepare(sport->clk);
		return ret;
	}

	sport->dma_tx_chan = dma_request_slave_channel(sport->port.dev, "tx");
	if (!sport->dma_tx_chan)
		dev_info(sport->port.dev, "DMA tx channel request failed, "
				"operating without tx DMA\n");

	sport->dma_rx_chan = dma_request_slave_channel(sport->port.dev, "rx");
	if (!sport->dma_rx_chan)
		dev_info(sport->port.dev, "DMA rx channel request failed, "
				"operating without rx DMA\n");

	return 0;
}

static int lpuart_remove(struct platform_device *pdev)
{
	struct lpuart_port *sport = platform_get_drvdata(pdev);

	uart_remove_one_port(&lpuart_reg, &sport->port);

	clk_disable_unprepare(sport->clk);

	if (sport->dma_tx_chan)
		dma_release_channel(sport->dma_tx_chan);

	if (sport->dma_rx_chan)
		dma_release_channel(sport->dma_rx_chan);

	return 0;
}

#ifdef CONFIG_PM_SLEEP
static int lpuart_suspend(struct device *dev)
{
	struct lpuart_port *sport = dev_get_drvdata(dev);
	unsigned long temp;

	if (sport->lpuart32) {
		/* disable Rx/Tx and interrupts */
		temp = lpuart32_read(sport->port.membase + UARTCTRL);
		temp &= ~(UARTCTRL_TE | UARTCTRL_TIE | UARTCTRL_TCIE);
		lpuart32_write(temp, sport->port.membase + UARTCTRL);
	} else {
		/* disable Rx/Tx and interrupts */
		temp = readb(sport->port.membase + UARTCR2);
		temp &= ~(UARTCR2_TE | UARTCR2_TIE | UARTCR2_TCIE);
		writeb(temp, sport->port.membase + UARTCR2);
	}

	uart_suspend_port(&lpuart_reg, &sport->port);

	return 0;
}

static int lpuart_resume(struct device *dev)
{
	struct lpuart_port *sport = dev_get_drvdata(dev);
	unsigned long temp;

	if (sport->lpuart32) {
		lpuart32_setup_watermark(sport);
		temp = lpuart32_read(sport->port.membase + UARTCTRL);
		temp |= (UARTCTRL_RIE | UARTCTRL_TIE | UARTCTRL_RE |
			 UARTCTRL_TE | UARTCTRL_ILIE);
		lpuart32_write(temp, sport->port.membase + UARTCTRL);
	} else {
		lpuart_setup_watermark(sport);
		temp = readb(sport->port.membase + UARTCR2);
		temp |= (UARTCR2_RIE | UARTCR2_TIE | UARTCR2_RE | UARTCR2_TE);
		writeb(temp, sport->port.membase + UARTCR2);
	}

	uart_resume_port(&lpuart_reg, &sport->port);

	return 0;
}
#endif

static SIMPLE_DEV_PM_OPS(lpuart_pm_ops, lpuart_suspend, lpuart_resume);

static struct platform_driver lpuart_driver = {
	.probe		= lpuart_probe,
	.remove		= lpuart_remove,
	.driver		= {
		.name	= "fsl-lpuart",
		.of_match_table = lpuart_dt_ids,
		.pm	= &lpuart_pm_ops,
	},
};

static int __init lpuart_serial_init(void)
{
	int ret = uart_register_driver(&lpuart_reg);

	if (ret)
		return ret;

	ret = platform_driver_register(&lpuart_driver);
	if (ret)
		uart_unregister_driver(&lpuart_reg);

	return ret;
}

static void __exit lpuart_serial_exit(void)
{
	platform_driver_unregister(&lpuart_driver);
	uart_unregister_driver(&lpuart_reg);
}

module_init(lpuart_serial_init);
module_exit(lpuart_serial_exit);

MODULE_DESCRIPTION("Freescale lpuart serial port driver");
MODULE_LICENSE("GPL v2");

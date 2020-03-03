/*
 * 8250-core based driver for the OMAP internal UART
 *
 * based on omap-serial.c, Copyright (C) 2010 Texas Instruments.
 *
 * Copyright (C) 2014 Sebastian Andrzej Siewior
 *
 */

#include <linux/device.h>
#include <linux/io.h>
#include <linux/module.h>
#include <linux/serial_8250.h>
#include <linux/serial_reg.h>
#include <linux/tty_flip.h>
#include <linux/platform_device.h>
#include <linux/slab.h>
#include <linux/of.h>
#include <linux/of_gpio.h>
#include <linux/of_irq.h>
#include <linux/delay.h>
#include <linux/pm_runtime.h>
#include <linux/console.h>
#include <linux/pm_qos.h>
#include <linux/dma-mapping.h>

#include "8250.h"

#define DEFAULT_CLK_SPEED	48000000

#define UART_ERRATA_i202_MDR1_ACCESS	(1 << 0)
#define OMAP_UART_WER_HAS_TX_WAKEUP	(1 << 1)
#define OMAP_DMA_TX_KICK		(1 << 2)

#define OMAP_UART_FCR_RX_TRIG		6
#define OMAP_UART_FCR_TX_TRIG		4

/* SCR register bitmasks */
#define OMAP_UART_SCR_RX_TRIG_GRANU1_MASK	(1 << 7)
#define OMAP_UART_SCR_TX_TRIG_GRANU1_MASK	(1 << 6)
#define OMAP_UART_SCR_TX_EMPTY			(1 << 3)
#define OMAP_UART_SCR_DMAMODE_MASK		(3 << 1)
#define OMAP_UART_SCR_DMAMODE_1			(1 << 1)
#define OMAP_UART_SCR_DMAMODE_CTL		(1 << 0)

/* MVR register bitmasks */
#define OMAP_UART_MVR_SCHEME_SHIFT	30
#define OMAP_UART_LEGACY_MVR_MAJ_MASK	0xf0
#define OMAP_UART_LEGACY_MVR_MAJ_SHIFT	4
#define OMAP_UART_LEGACY_MVR_MIN_MASK	0x0f
#define OMAP_UART_MVR_MAJ_MASK		0x700
#define OMAP_UART_MVR_MAJ_SHIFT		8
#define OMAP_UART_MVR_MIN_MASK		0x3f

#define UART_TI752_TLR_TX	0
#define UART_TI752_TLR_RX	4

#define TRIGGER_TLR_MASK(x)	((x & 0x3c) >> 2)
#define TRIGGER_FCR_MASK(x)	(x & 3)

/* Enable XON/XOFF flow control on output */
#define OMAP_UART_SW_TX		0x08
/* Enable XON/XOFF flow control on input */
#define OMAP_UART_SW_RX		0x02

#define OMAP_UART_WER_MOD_WKUP	0x7f
#define OMAP_UART_TX_WAKEUP_EN	(1 << 7)

#define TX_TRIGGER	1
#define RX_TRIGGER	48

#define OMAP_UART_TCR_RESTORE(x)	((x / 4) << 4)
#define OMAP_UART_TCR_HALT(x)		((x / 4) << 0)

#define UART_BUILD_REVISION(x, y)	(((x) << 8) | (y))

#define OMAP_UART_REV_46 0x0406
#define OMAP_UART_REV_52 0x0502
#define OMAP_UART_REV_63 0x0603

struct omap8250_priv {
	int line;
	u8 habit;
	u8 mdr1;
	u8 efr;
	u8 scr;
	u8 wer;
	u8 xon;
	u8 xoff;
	u8 delayed_restore;
	u16 quot;

	bool is_suspending;
	int wakeirq;
	int wakeups_enabled;
	u32 latency;
	u32 calc_latency;
	struct pm_qos_request pm_qos_request;
	struct work_struct qos_work;
	struct uart_8250_dma omap8250_dma;
};

static u32 uart_read(struct uart_8250_port *up, u32 reg)
{
	return readl(up->port.membase + (reg << up->port.regshift));
}

static void omap8250_set_mctrl(struct uart_port *port, unsigned int mctrl)
{
	struct uart_8250_port *up = up_to_u8250p(port);
	struct omap8250_priv *priv = up->port.private_data;
	u8 lcr;

	serial8250_do_set_mctrl(port, mctrl);

	/*
	 * Turn off autoRTS if RTS is lowered and restore autoRTS setting
	 * if RTS is raised
	 */
	lcr = serial_in(up, UART_LCR);
	serial_out(up, UART_LCR, UART_LCR_CONF_MODE_B);
	if ((mctrl & TIOCM_RTS) && (port->status & UPSTAT_AUTORTS))
		priv->efr |= UART_EFR_RTS;
	else
		priv->efr &= ~UART_EFR_RTS;
	serial_out(up, UART_EFR, priv->efr);
	serial_out(up, UART_LCR, lcr);
}

/*
 * Work Around for Errata i202 (2430, 3430, 3630, 4430 and 4460)
 * The access to uart register after MDR1 Access
 * causes UART to corrupt data.
 *
 * Need a delay =
 * 5 L4 clock cycles + 5 UART functional clock cycle (@48MHz = ~0.2uS)
 * give 10 times as much
 */
static void omap_8250_mdr1_errataset(struct uart_8250_port *up,
				     struct omap8250_priv *priv)
{
	u8 timeout = 255;
	u8 old_mdr1;

	old_mdr1 = serial_in(up, UART_OMAP_MDR1);
	if (old_mdr1 == priv->mdr1)
		return;

	serial_out(up, UART_OMAP_MDR1, priv->mdr1);
	udelay(2);
	serial_out(up, UART_FCR, up->fcr | UART_FCR_CLEAR_XMIT |
			UART_FCR_CLEAR_RCVR);
	/*
	 * Wait for FIFO to empty: when empty, RX_FIFO_E bit is 0 and
	 * TX_FIFO_E bit is 1.
	 */
	while (UART_LSR_THRE != (serial_in(up, UART_LSR) &
				(UART_LSR_THRE | UART_LSR_DR))) {
		timeout--;
		if (!timeout) {
			/* Should *never* happen. we warn and carry on */
			dev_crit(up->port.dev, "Errata i202: timedout %x\n",
				 serial_in(up, UART_LSR));
			break;
		}
		udelay(1);
	}
}

static void omap_8250_get_divisor(struct uart_port *port, unsigned int baud,
				  struct omap8250_priv *priv)
{
	unsigned int uartclk = port->uartclk;
	unsigned int div_13, div_16;
	unsigned int abs_d13, abs_d16;

	/*
	 * Old custom speed handling.
	 */
	if (baud == 38400 && (port->flags & UPF_SPD_MASK) == UPF_SPD_CUST) {
		priv->quot = port->custom_divisor & 0xffff;
		/*
		 * I assume that nobody is using this. But hey, if somebody
		 * would like to specify the divisor _and_ the mode then the
		 * driver is ready and waiting for it.
		 */
		if (port->custom_divisor & (1 << 16))
			priv->mdr1 = UART_OMAP_MDR1_13X_MODE;
		else
			priv->mdr1 = UART_OMAP_MDR1_16X_MODE;
		return;
	}
	div_13 = DIV_ROUND_CLOSEST(uartclk, 13 * baud);
	div_16 = DIV_ROUND_CLOSEST(uartclk, 16 * baud);

	if (!div_13)
		div_13 = 1;
	if (!div_16)
		div_16 = 1;

	abs_d13 = abs(baud - uartclk / 13 / div_13);
	abs_d16 = abs(baud - uartclk / 16 / div_16);

	if (abs_d13 >= abs_d16) {
		priv->mdr1 = UART_OMAP_MDR1_16X_MODE;
		priv->quot = div_16;
	} else {
		priv->mdr1 = UART_OMAP_MDR1_13X_MODE;
		priv->quot = div_13;
	}
}

static void omap8250_update_scr(struct uart_8250_port *up,
				struct omap8250_priv *priv)
{
	u8 old_scr;

	old_scr = serial_in(up, UART_OMAP_SCR);
	if (old_scr == priv->scr)
		return;

	/*
	 * The manual recommends not to enable the DMA mode selector in the SCR
	 * (instead of the FCR) register _and_ selecting the DMA mode as one
	 * register write because this may lead to malfunction.
	 */
	if (priv->scr & OMAP_UART_SCR_DMAMODE_MASK)
		serial_out(up, UART_OMAP_SCR,
			   priv->scr & ~OMAP_UART_SCR_DMAMODE_MASK);
	serial_out(up, UART_OMAP_SCR, priv->scr);
}

static void omap8250_restore_regs(struct uart_8250_port *up)
{
	struct omap8250_priv *priv = up->port.private_data;
	struct uart_8250_dma	*dma = up->dma;

	if (dma && dma->tx_running) {
		/*
		 * TCSANOW requests the change to occur immediately however if
		 * we have a TX-DMA operation in progress then it has been
		 * observed that it might stall and never complete. Therefore we
		 * delay DMA completes to prevent this hang from happen.
		 */
		priv->delayed_restore = 1;
		return;
	}

	serial_out(up, UART_LCR, UART_LCR_CONF_MODE_B);
	serial_out(up, UART_EFR, UART_EFR_ECB);

	serial_out(up, UART_LCR, UART_LCR_CONF_MODE_A);
	serial_out(up, UART_MCR, UART_MCR_TCRTLR);
	serial_out(up, UART_FCR, up->fcr);

	omap8250_update_scr(up, priv);

	serial_out(up, UART_LCR, UART_LCR_CONF_MODE_B);

	serial_out(up, UART_TI752_TCR, OMAP_UART_TCR_RESTORE(16) |
			OMAP_UART_TCR_HALT(52));
	serial_out(up, UART_TI752_TLR,
		   TRIGGER_TLR_MASK(TX_TRIGGER) << UART_TI752_TLR_TX |
		   TRIGGER_TLR_MASK(RX_TRIGGER) << UART_TI752_TLR_RX);

	serial_out(up, UART_LCR, 0);

	/* drop TCR + TLR access, we setup XON/XOFF later */
	serial_out(up, UART_MCR, up->mcr);
	serial_out(up, UART_IER, up->ier);

	serial_out(up, UART_LCR, UART_LCR_CONF_MODE_B);
	serial_dl_write(up, priv->quot);

	serial_out(up, UART_EFR, priv->efr);

	/* Configure flow control */
	serial_out(up, UART_LCR, UART_LCR_CONF_MODE_B);
	serial_out(up, UART_XON1, priv->xon);
	serial_out(up, UART_XOFF1, priv->xoff);

	serial_out(up, UART_LCR, up->lcr);
	/* need mode A for FCR */
	if (priv->habit & UART_ERRATA_i202_MDR1_ACCESS)
		omap_8250_mdr1_errataset(up, priv);
	else
		serial_out(up, UART_OMAP_MDR1, priv->mdr1);
	up->port.ops->set_mctrl(&up->port, up->port.mctrl);
}

/*
 * OMAP can use "CLK / (16 or 13) / div" for baud rate. And then we have have
 * some differences in how we want to handle flow control.
 */
static void omap_8250_set_termios(struct uart_port *port,
				  struct ktermios *termios,
				  struct ktermios *old)
{
	struct uart_8250_port *up =
		container_of(port, struct uart_8250_port, port);
	struct omap8250_priv *priv = up->port.private_data;
	unsigned char cval = 0;
	unsigned int baud;

	switch (termios->c_cflag & CSIZE) {
	case CS5:
		cval = UART_LCR_WLEN5;
		break;
	case CS6:
		cval = UART_LCR_WLEN6;
		break;
	case CS7:
		cval = UART_LCR_WLEN7;
		break;
	default:
	case CS8:
		cval = UART_LCR_WLEN8;
		break;
	}

	if (termios->c_cflag & CSTOPB)
		cval |= UART_LCR_STOP;
	if (termios->c_cflag & PARENB)
		cval |= UART_LCR_PARITY;
	if (!(termios->c_cflag & PARODD))
		cval |= UART_LCR_EPAR;
	if (termios->c_cflag & CMSPAR)
		cval |= UART_LCR_SPAR;

	/*
	 * Ask the core to calculate the divisor for us.
	 */
	baud = uart_get_baud_rate(port, termios, old,
				  port->uartclk / 16 / 0xffff,
				  port->uartclk / 13);
	omap_8250_get_divisor(port, baud, priv);

	/*
	 * Ok, we're now changing the port state. Do it with
	 * interrupts disabled.
	 */
	pm_runtime_get_sync(port->dev);
	spin_lock_irq(&port->lock);

	/*
	 * Update the per-port timeout.
	 */
	uart_update_timeout(port, termios->c_cflag, baud);

	up->port.read_status_mask = UART_LSR_OE | UART_LSR_THRE | UART_LSR_DR;
	if (termios->c_iflag & INPCK)
		up->port.read_status_mask |= UART_LSR_FE | UART_LSR_PE;
	if (termios->c_iflag & (IGNBRK | PARMRK))
		up->port.read_status_mask |= UART_LSR_BI;

	/*
	 * Characters to ignore
	 */
	up->port.ignore_status_mask = 0;
	if (termios->c_iflag & IGNPAR)
		up->port.ignore_status_mask |= UART_LSR_PE | UART_LSR_FE;
	if (termios->c_iflag & IGNBRK) {
		up->port.ignore_status_mask |= UART_LSR_BI;
		/*
		 * If we're ignoring parity and break indicators,
		 * ignore overruns too (for real raw support).
		 */
		if (termios->c_iflag & IGNPAR)
			up->port.ignore_status_mask |= UART_LSR_OE;
	}

	/*
	 * ignore all characters if CREAD is not set
	 */
	if ((termios->c_cflag & CREAD) == 0)
		up->port.ignore_status_mask |= UART_LSR_DR;

	/*
	 * Modem status interrupts
	 */
	up->ier &= ~UART_IER_MSI;
	if (UART_ENABLE_MS(&up->port, termios->c_cflag))
		up->ier |= UART_IER_MSI;

	up->lcr = cval;
	/* Up to here it was mostly serial8250_do_set_termios() */

	/*
	 * We enable TRIG_GRANU for RX and TX and additionaly we set
	 * SCR_TX_EMPTY bit. The result is the following:
	 * - RX_TRIGGER amount of bytes in the FIFO will cause an interrupt.
	 * - less than RX_TRIGGER number of bytes will also cause an interrupt
	 *   once the UART decides that there no new bytes arriving.
	 * - Once THRE is enabled, the interrupt will be fired once the FIFO is
	 *   empty - the trigger level is ignored here.
	 *
	 * Once DMA is enabled:
	 * - UART will assert the TX DMA line once there is room for TX_TRIGGER
	 *   bytes in the TX FIFO. On each assert the DMA engine will move
	 *   TX_TRIGGER bytes into the FIFO.
	 * - UART will assert the RX DMA line once there are RX_TRIGGER bytes in
	 *   the FIFO and move RX_TRIGGER bytes.
	 * This is because threshold and trigger values are the same.
	 */
	up->fcr = UART_FCR_ENABLE_FIFO;
	up->fcr |= TRIGGER_FCR_MASK(TX_TRIGGER) << OMAP_UART_FCR_TX_TRIG;
	up->fcr |= TRIGGER_FCR_MASK(RX_TRIGGER) << OMAP_UART_FCR_RX_TRIG;

	priv->scr = OMAP_UART_SCR_RX_TRIG_GRANU1_MASK | OMAP_UART_SCR_TX_EMPTY |
		OMAP_UART_SCR_TX_TRIG_GRANU1_MASK;

	if (up->dma)
		priv->scr |= OMAP_UART_SCR_DMAMODE_1 |
			OMAP_UART_SCR_DMAMODE_CTL;

	priv->xon = termios->c_cc[VSTART];
	priv->xoff = termios->c_cc[VSTOP];

	priv->efr = 0;
	up->mcr &= ~(UART_MCR_RTS | UART_MCR_XONANY);
	up->port.status &= ~(UPSTAT_AUTOCTS | UPSTAT_AUTORTS | UPSTAT_AUTOXOFF);

	if (termios->c_cflag & CRTSCTS && up->port.flags & UPF_HARD_FLOW) {
		/* Enable AUTOCTS (autoRTS is enabled when RTS is raised) */
		up->port.status |= UPSTAT_AUTOCTS | UPSTAT_AUTORTS;
		priv->efr |= UART_EFR_CTS;
	} else	if (up->port.flags & UPF_SOFT_FLOW) {
		/*
		 * IXON Flag:
		 * Enable XON/XOFF flow control on input.
		 * Receiver compares XON1, XOFF1.
		 */
		if (termios->c_iflag & IXON)
			priv->efr |= OMAP_UART_SW_RX;

		/*
		 * IXOFF Flag:
		 * Enable XON/XOFF flow control on output.
		 * Transmit XON1, XOFF1
		 */
		if (termios->c_iflag & IXOFF) {
			up->port.status |= UPSTAT_AUTOXOFF;
			priv->efr |= OMAP_UART_SW_TX;
		}

		/*
		 * IXANY Flag:
		 * Enable any character to restart output.
		 * Operation resumes after receiving any
		 * character after recognition of the XOFF character
		 */
		if (termios->c_iflag & IXANY)
			up->mcr |= UART_MCR_XONANY;
	}
	omap8250_restore_regs(up);

	spin_unlock_irq(&up->port.lock);
	pm_runtime_mark_last_busy(port->dev);
	pm_runtime_put_autosuspend(port->dev);

	/* calculate wakeup latency constraint */
	priv->calc_latency = USEC_PER_SEC * 64 * 8 / baud;
	priv->latency = priv->calc_latency;

	schedule_work(&priv->qos_work);

	/* Don't rewrite B0 */
	if (tty_termios_baud_rate(termios))
		tty_termios_encode_baud_rate(termios, baud, baud);
}

/* same as 8250 except that we may have extra flow bits set in EFR */
static void omap_8250_pm(struct uart_port *port, unsigned int state,
			 unsigned int oldstate)
{
	struct uart_8250_port *up = up_to_u8250p(port);
	u8 efr;

	pm_runtime_get_sync(port->dev);
	serial_out(up, UART_LCR, UART_LCR_CONF_MODE_B);
	efr = serial_in(up, UART_EFR);
	serial_out(up, UART_EFR, efr | UART_EFR_ECB);
	serial_out(up, UART_LCR, 0);

	serial_out(up, UART_IER, (state != 0) ? UART_IERX_SLEEP : 0);
	serial_out(up, UART_LCR, UART_LCR_CONF_MODE_B);
	serial_out(up, UART_EFR, efr);
	serial_out(up, UART_LCR, 0);

	pm_runtime_mark_last_busy(port->dev);
	pm_runtime_put_autosuspend(port->dev);
}

static void omap_serial_fill_features_erratas(struct uart_8250_port *up,
					      struct omap8250_priv *priv)
{
	u32 mvr, scheme;
	u16 revision, major, minor;

	mvr = uart_read(up, UART_OMAP_MVER);

	/* Check revision register scheme */
	scheme = mvr >> OMAP_UART_MVR_SCHEME_SHIFT;

	switch (scheme) {
	case 0: /* Legacy Scheme: OMAP2/3 */
		/* MINOR_REV[0:4], MAJOR_REV[4:7] */
		major = (mvr & OMAP_UART_LEGACY_MVR_MAJ_MASK) >>
			OMAP_UART_LEGACY_MVR_MAJ_SHIFT;
		minor = (mvr & OMAP_UART_LEGACY_MVR_MIN_MASK);
		break;
	case 1:
		/* New Scheme: OMAP4+ */
		/* MINOR_REV[0:5], MAJOR_REV[8:10] */
		major = (mvr & OMAP_UART_MVR_MAJ_MASK) >>
			OMAP_UART_MVR_MAJ_SHIFT;
		minor = (mvr & OMAP_UART_MVR_MIN_MASK);
		break;
	default:
		dev_warn(up->port.dev,
			 "Unknown revision, defaulting to highest\n");
		/* highest possible revision */
		major = 0xff;
		minor = 0xff;
	}
	/* normalize revision for the driver */
	revision = UART_BUILD_REVISION(major, minor);

	switch (revision) {
	case OMAP_UART_REV_46:
		priv->habit = UART_ERRATA_i202_MDR1_ACCESS;
		break;
	case OMAP_UART_REV_52:
		priv->habit = UART_ERRATA_i202_MDR1_ACCESS |
				OMAP_UART_WER_HAS_TX_WAKEUP;
		break;
	case OMAP_UART_REV_63:
		priv->habit = UART_ERRATA_i202_MDR1_ACCESS |
			OMAP_UART_WER_HAS_TX_WAKEUP;
		break;
	default:
		break;
	}
}

static void omap8250_uart_qos_work(struct work_struct *work)
{
	struct omap8250_priv *priv;

	priv = container_of(work, struct omap8250_priv, qos_work);
	pm_qos_update_request(&priv->pm_qos_request, priv->latency);
}

static irqreturn_t omap_wake_irq(int irq, void *dev_id)
{
	struct uart_port *port = dev_id;
	int ret;

	ret = port->handle_irq(port);
	if (ret)
		return IRQ_HANDLED;
	return IRQ_NONE;
}

#ifdef CONFIG_SERIAL_8250_DMA
static int omap_8250_dma_handle_irq(struct uart_port *port);
#endif

static irqreturn_t omap8250_irq(int irq, void *dev_id)
{
	struct uart_port *port = dev_id;
	struct uart_8250_port *up = up_to_u8250p(port);
	unsigned int iir;
	int ret;

#ifdef CONFIG_SERIAL_8250_DMA
	if (up->dma) {
		ret = omap_8250_dma_handle_irq(port);
		return IRQ_RETVAL(ret);
	}
#endif

	serial8250_rpm_get(up);
	iir = serial_port_in(port, UART_IIR);
	ret = serial8250_handle_irq(port, iir);
	serial8250_rpm_put(up);

	return IRQ_RETVAL(ret);
}

static int omap_8250_startup(struct uart_port *port)
{
	struct uart_8250_port *up = up_to_u8250p(port);
	struct omap8250_priv *priv = port->private_data;
	int ret;

	if (priv->wakeirq) {
		ret = request_irq(priv->wakeirq, omap_wake_irq,
				  port->irqflags, "uart wakeup irq", port);
		if (ret)
			return ret;
		disable_irq(priv->wakeirq);
	}

	pm_runtime_get_sync(port->dev);

	up->mcr = 0;
	serial_out(up, UART_FCR, UART_FCR_CLEAR_RCVR | UART_FCR_CLEAR_XMIT);

	serial_out(up, UART_LCR, UART_LCR_WLEN8);

	up->lsr_saved_flags = 0;
	up->msr_saved_flags = 0;

	/* Disable DMA for console UART */
	if (uart_console(port))
		up->dma = NULL;

	if (up->dma) {
		ret = serial8250_request_dma(up);
		if (ret) {
			dev_warn_ratelimited(port->dev,
					     "failed to request DMA\n");
			up->dma = NULL;
		}
	}

	ret = request_irq(port->irq, omap8250_irq, IRQF_SHARED,
			  dev_name(port->dev), port);
	if (ret < 0)
		goto err;

	up->ier = UART_IER_RLSI | UART_IER_RDI;
	serial_out(up, UART_IER, up->ier);

#ifdef CONFIG_PM
	up->capabilities |= UART_CAP_RPM;
#endif

	/* Enable module level wake up */
	priv->wer = OMAP_UART_WER_MOD_WKUP;
	if (priv->habit & OMAP_UART_WER_HAS_TX_WAKEUP)
		priv->wer |= OMAP_UART_TX_WAKEUP_EN;
	serial_out(up, UART_OMAP_WER, priv->wer);

	if (up->dma)
		up->dma->rx_dma(up, 0);

	pm_runtime_mark_last_busy(port->dev);
	pm_runtime_put_autosuspend(port->dev);
	return 0;
err:
	pm_runtime_mark_last_busy(port->dev);
	pm_runtime_put_autosuspend(port->dev);
	if (priv->wakeirq)
		free_irq(priv->wakeirq, port);
	return ret;
}

static void omap_8250_shutdown(struct uart_port *port)
{
	struct uart_8250_port *up = up_to_u8250p(port);
	struct omap8250_priv *priv = port->private_data;

	flush_work(&priv->qos_work);
	if (up->dma)
		up->dma->rx_dma(up, UART_IIR_RX_TIMEOUT);

	pm_runtime_get_sync(port->dev);

	serial_out(up, UART_OMAP_WER, 0);

	up->ier = 0;
	serial_out(up, UART_IER, 0);

	if (up->dma)
		serial8250_release_dma(up);

	/*
	 * Disable break condition and FIFOs
	 */
	if (up->lcr & UART_LCR_SBC)
		serial_out(up, UART_LCR, up->lcr & ~UART_LCR_SBC);
	serial_out(up, UART_FCR, UART_FCR_CLEAR_RCVR | UART_FCR_CLEAR_XMIT);

	pm_runtime_mark_last_busy(port->dev);
	pm_runtime_put_autosuspend(port->dev);

	free_irq(port->irq, port);
	if (priv->wakeirq)
		free_irq(priv->wakeirq, port);
}

static void omap_8250_throttle(struct uart_port *port)
{
	unsigned long flags;
	struct uart_8250_port *up =
		container_of(port, struct uart_8250_port, port);

	pm_runtime_get_sync(port->dev);

	spin_lock_irqsave(&port->lock, flags);
	up->ier &= ~(UART_IER_RLSI | UART_IER_RDI);
	serial_out(up, UART_IER, up->ier);
	spin_unlock_irqrestore(&port->lock, flags);

	pm_runtime_mark_last_busy(port->dev);
	pm_runtime_put_autosuspend(port->dev);
}

static void omap_8250_unthrottle(struct uart_port *port)
{
	unsigned long flags;
	struct uart_8250_port *up =
		container_of(port, struct uart_8250_port, port);

	pm_runtime_get_sync(port->dev);

	spin_lock_irqsave(&port->lock, flags);
	up->ier |= UART_IER_RLSI | UART_IER_RDI;
	serial_out(up, UART_IER, up->ier);
	spin_unlock_irqrestore(&port->lock, flags);

	pm_runtime_mark_last_busy(port->dev);
	pm_runtime_put_autosuspend(port->dev);
}

#ifdef CONFIG_SERIAL_8250_DMA
static int omap_8250_rx_dma(struct uart_8250_port *p, unsigned int iir);

static void __dma_rx_do_complete(struct uart_8250_port *p, bool error)
{
	struct uart_8250_dma    *dma = p->dma;
	struct tty_port         *tty_port = &p->port.state->port;
	struct dma_tx_state     state;
	int                     count;

	dma_sync_single_for_cpu(dma->rxchan->device->dev, dma->rx_addr,
				dma->rx_size, DMA_FROM_DEVICE);

	dma->rx_running = 0;
	dmaengine_tx_status(dma->rxchan, dma->rx_cookie, &state);
	dmaengine_terminate_all(dma->rxchan);

	count = dma->rx_size - state.residue;

	tty_insert_flip_string(tty_port, dma->rx_buf, count);
	p->port.icount.rx += count;
	if (!error)
		omap_8250_rx_dma(p, 0);

	tty_flip_buffer_push(tty_port);
}

static void __dma_rx_complete(void *param)
{
	__dma_rx_do_complete(param, false);
}

static int omap_8250_rx_dma(struct uart_8250_port *p, unsigned int iir)
{
	struct uart_8250_dma            *dma = p->dma;
	struct dma_async_tx_descriptor  *desc;

	switch (iir & 0x3f) {
	case UART_IIR_RLSI:
		/* 8250_core handles errors and break interrupts */
		if (dma->rx_running) {
			dmaengine_pause(dma->rxchan);
			__dma_rx_do_complete(p, true);
		}
		return -EIO;
	case UART_IIR_RX_TIMEOUT:
		/*
		 * If RCVR FIFO trigger level was not reached, complete the
		 * transfer and let 8250_core copy the remaining data.
		 */
		if (dma->rx_running) {
			dmaengine_pause(dma->rxchan);
			__dma_rx_do_complete(p, true);
		}
		return -ETIMEDOUT;
	case UART_IIR_RDI:
		/*
		 * The OMAP UART is a special BEAST. If we receive RDI we _have_
		 * a DMA transfer programmed but it didn't work. One reason is
		 * that we were too slow and there were too many bytes in the
		 * FIFO, the UART counted wrong and never kicked the DMA engine
		 * to do anything. That means once we receive RDI on OMAP then
		 * the DMA won't do anything soon so we have to cancel the DMA
		 * transfer and purge the FIFO manually.
		 */
		if (dma->rx_running) {
			dmaengine_pause(dma->rxchan);
			__dma_rx_do_complete(p, true);
		}
		return -ETIMEDOUT;

	default:
		break;
	}

	if (dma->rx_running)
		return 0;

	desc = dmaengine_prep_slave_single(dma->rxchan, dma->rx_addr,
					   dma->rx_size, DMA_DEV_TO_MEM,
					   DMA_PREP_INTERRUPT | DMA_CTRL_ACK);
	if (!desc)
		return -EBUSY;

	dma->rx_running = 1;
	desc->callback = __dma_rx_complete;
	desc->callback_param = p;

	dma->rx_cookie = dmaengine_submit(desc);

	dma_sync_single_for_device(dma->rxchan->device->dev, dma->rx_addr,
				   dma->rx_size, DMA_FROM_DEVICE);

	dma_async_issue_pending(dma->rxchan);
	return 0;
}

static int omap_8250_tx_dma(struct uart_8250_port *p);

static void omap_8250_dma_tx_complete(void *param)
{
	struct uart_8250_port	*p = param;
	struct uart_8250_dma	*dma = p->dma;
	struct circ_buf		*xmit = &p->port.state->xmit;
	unsigned long		flags;
	bool			en_thri = false;
	struct omap8250_priv	*priv = p->port.private_data;

	dma_sync_single_for_cpu(dma->txchan->device->dev, dma->tx_addr,
				UART_XMIT_SIZE, DMA_TO_DEVICE);

	spin_lock_irqsave(&p->port.lock, flags);

	dma->tx_running = 0;

	xmit->tail += dma->tx_size;
	xmit->tail &= UART_XMIT_SIZE - 1;
	p->port.icount.tx += dma->tx_size;

	if (priv->delayed_restore) {
		priv->delayed_restore = 0;
		omap8250_restore_regs(p);
	}

	if (uart_circ_chars_pending(xmit) < WAKEUP_CHARS)
		uart_write_wakeup(&p->port);

	if (!uart_circ_empty(xmit) && !uart_tx_stopped(&p->port)) {
		int ret;

		ret = omap_8250_tx_dma(p);
		if (ret)
			en_thri = true;

	} else if (p->capabilities & UART_CAP_RPM) {
		en_thri = true;
	}

	if (en_thri) {
		dma->tx_err = 1;
		p->ier |= UART_IER_THRI;
		serial_port_out(&p->port, UART_IER, p->ier);
	}

	spin_unlock_irqrestore(&p->port.lock, flags);
}

static int omap_8250_tx_dma(struct uart_8250_port *p)
{
	struct uart_8250_dma		*dma = p->dma;
	struct omap8250_priv		*priv = p->port.private_data;
	struct circ_buf			*xmit = &p->port.state->xmit;
	struct dma_async_tx_descriptor	*desc;
	unsigned int	skip_byte = 0;
	int ret;

	if (dma->tx_running)
		return 0;
	if (uart_tx_stopped(&p->port) || uart_circ_empty(xmit)) {

		/*
		 * Even if no data, we need to return an error for the two cases
		 * below so serial8250_tx_chars() is invoked and properly clears
		 * THRI and/or runtime suspend.
		 */
		if (dma->tx_err || p->capabilities & UART_CAP_RPM) {
			ret = -EBUSY;
			goto err;
		}
		if (p->ier & UART_IER_THRI) {
			p->ier &= ~UART_IER_THRI;
			serial_out(p, UART_IER, p->ier);
		}
		return 0;
	}

	dma->tx_size = CIRC_CNT_TO_END(xmit->head, xmit->tail, UART_XMIT_SIZE);
	if (priv->habit & OMAP_DMA_TX_KICK) {
		u8 tx_lvl;

		/*
		 * We need to put the first byte into the FIFO in order to start
		 * the DMA transfer. For transfers smaller than four bytes we
		 * don't bother doing DMA at all. It seem not matter if there
		 * are still bytes in the FIFO from the last transfer (in case
		 * we got here directly from omap_8250_dma_tx_complete()). Bytes
		 * leaving the FIFO seem not to trigger the DMA transfer. It is
		 * really the byte that we put into the FIFO.
		 * If the FIFO is already full then we most likely got here from
		 * omap_8250_dma_tx_complete(). And this means the DMA engine
		 * just completed its work. We don't have to wait the complete
		 * 86us at 115200,8n1 but around 60us (not to mention lower
		 * baudrates). So in that case we take the interrupt and try
		 * again with an empty FIFO.
		 */
		tx_lvl = serial_in(p, UART_OMAP_TX_LVL);
		if (tx_lvl == p->tx_loadsz) {
			ret = -EBUSY;
			goto err;
		}
		if (dma->tx_size < 4) {
			ret = -EINVAL;
			goto err;
		}
		skip_byte = 1;
	}

	desc = dmaengine_prep_slave_single(dma->txchan,
			dma->tx_addr + xmit->tail + skip_byte,
			dma->tx_size - skip_byte, DMA_MEM_TO_DEV,
			DMA_PREP_INTERRUPT | DMA_CTRL_ACK);
	if (!desc) {
		ret = -EBUSY;
		goto err;
	}

	dma->tx_running = 1;

	desc->callback = omap_8250_dma_tx_complete;
	desc->callback_param = p;

	dma->tx_cookie = dmaengine_submit(desc);

	dma_sync_single_for_device(dma->txchan->device->dev, dma->tx_addr,
				   UART_XMIT_SIZE, DMA_TO_DEVICE);

	dma_async_issue_pending(dma->txchan);
	if (dma->tx_err)
		dma->tx_err = 0;

	if (p->ier & UART_IER_THRI) {
		p->ier &= ~UART_IER_THRI;
		serial_out(p, UART_IER, p->ier);
	}
	if (skip_byte)
		serial_out(p, UART_TX, xmit->buf[xmit->tail]);
	return 0;
err:
	dma->tx_err = 1;
	return ret;
}

/*
 * This is mostly serial8250_handle_irq(). We have a slightly different DMA
 * hoook for RX/TX and need different logic for them in the ISR. Therefore we
 * use the default routine in the non-DMA case and this one for with DMA.
 */
static int omap_8250_dma_handle_irq(struct uart_port *port)
{
	struct uart_8250_port *up = up_to_u8250p(port);
	unsigned char status;
	unsigned long flags;
	u8 iir;
	int dma_err = 0;

	serial8250_rpm_get(up);

	iir = serial_port_in(port, UART_IIR);
	if (iir & UART_IIR_NO_INT) {
		serial8250_rpm_put(up);
		return 0;
	}

	spin_lock_irqsave(&port->lock, flags);

	status = serial_port_in(port, UART_LSR);

	if (status & (UART_LSR_DR | UART_LSR_BI)) {

		dma_err = omap_8250_rx_dma(up, iir);
		if (dma_err) {
			status = serial8250_rx_chars(up, status);
			omap_8250_rx_dma(up, 0);
		}
	}
	serial8250_modem_status(up);
	if (status & UART_LSR_THRE && up->dma->tx_err) {
		if (uart_tx_stopped(&up->port) ||
		    uart_circ_empty(&up->port.state->xmit)) {
			up->dma->tx_err = 0;
			serial8250_tx_chars(up);
		} else  {
			/*
			 * try again due to an earlier failer which
			 * might have been resolved by now.
			 */
			dma_err = omap_8250_tx_dma(up);
			if (dma_err)
				serial8250_tx_chars(up);
		}
	}

	spin_unlock_irqrestore(&port->lock, flags);
	serial8250_rpm_put(up);
	return 1;
}

static bool the_no_dma_filter_fn(struct dma_chan *chan, void *param)
{
	return false;
}

#else

static inline int omap_8250_rx_dma(struct uart_8250_port *p, unsigned int iir)
{
	return -EINVAL;
}
#endif

static int omap8250_no_handle_irq(struct uart_port *port)
{
	/* IRQ has not been requested but handling irq? */
	WARN_ONCE(1, "Unexpected irq handling before port startup\n");
	return 0;
}

static int omap8250_probe(struct platform_device *pdev)
{
	struct resource *regs = platform_get_resource(pdev, IORESOURCE_MEM, 0);
	struct resource *irq = platform_get_resource(pdev, IORESOURCE_IRQ, 0);
	struct omap8250_priv *priv;
	struct uart_8250_port up;
	int ret;
	void __iomem *membase;

	if (!regs || !irq) {
		dev_err(&pdev->dev, "missing registers or irq\n");
		return -EINVAL;
	}

	priv = devm_kzalloc(&pdev->dev, sizeof(*priv), GFP_KERNEL);
	if (!priv)
		return -ENOMEM;

	membase = devm_ioremap_nocache(&pdev->dev, regs->start,
				       resource_size(regs));
	if (!membase)
		return -ENODEV;

	memset(&up, 0, sizeof(up));
	up.port.dev = &pdev->dev;
	up.port.mapbase = regs->start;
	up.port.membase = membase;
	up.port.irq = irq->start;
	/*
	 * It claims to be 16C750 compatible however it is a little different.
	 * It has EFR and has no FCR7_64byte bit. The AFE (which it claims to
	 * have) is enabled via EFR instead of MCR. The type is set here 8250
	 * just to get things going. UNKNOWN does not work for a few reasons and
	 * we don't need our own type since we don't use 8250's set_termios()
	 * or pm callback.
	 */
	up.port.type = PORT_8250;
	up.port.iotype = UPIO_MEM;
	up.port.flags = UPF_FIXED_PORT | UPF_FIXED_TYPE | UPF_SOFT_FLOW |
		UPF_HARD_FLOW;
	up.port.private_data = priv;

	up.port.regshift = 2;
	up.port.fifosize = 64;
	up.tx_loadsz = 64;
	up.capabilities = UART_CAP_FIFO;
#ifdef CONFIG_PM
	/*
	 * Runtime PM is mostly transparent. However to do it right we need to a
	 * TX empty interrupt before we can put the device to auto idle. So if
	 * PM is not enabled we don't add that flag and can spare that one extra
	 * interrupt in the TX path.
	 */
	up.capabilities |= UART_CAP_RPM;
#endif
	up.port.set_termios = omap_8250_set_termios;
	up.port.set_mctrl = omap8250_set_mctrl;
	up.port.pm = omap_8250_pm;
	up.port.startup = omap_8250_startup;
	up.port.shutdown = omap_8250_shutdown;
	up.port.throttle = omap_8250_throttle;
	up.port.unthrottle = omap_8250_unthrottle;

	if (pdev->dev.of_node) {
		ret = of_alias_get_id(pdev->dev.of_node, "serial");

		of_property_read_u32(pdev->dev.of_node, "clock-frequency",
				     &up.port.uartclk);
		priv->wakeirq = irq_of_parse_and_map(pdev->dev.of_node, 1);
	} else {
		ret = pdev->id;
	}
	if (ret < 0) {
		dev_err(&pdev->dev, "failed to get alias/pdev id\n");
		return ret;
	}
	up.port.line = ret;

	if (!up.port.uartclk) {
		up.port.uartclk = DEFAULT_CLK_SPEED;
		dev_warn(&pdev->dev,
			 "No clock speed specified: using default: %d\n",
			 DEFAULT_CLK_SPEED);
	}

	priv->latency = PM_QOS_CPU_DMA_LAT_DEFAULT_VALUE;
	priv->calc_latency = PM_QOS_CPU_DMA_LAT_DEFAULT_VALUE;
	pm_qos_add_request(&priv->pm_qos_request, PM_QOS_CPU_DMA_LATENCY,
			   priv->latency);
	INIT_WORK(&priv->qos_work, omap8250_uart_qos_work);

	device_init_wakeup(&pdev->dev, true);
	pm_runtime_use_autosuspend(&pdev->dev);
	pm_runtime_set_autosuspend_delay(&pdev->dev, -1);

	pm_runtime_irq_safe(&pdev->dev);
	pm_runtime_enable(&pdev->dev);

	pm_runtime_get_sync(&pdev->dev);

	omap_serial_fill_features_erratas(&up, priv);
	up.port.handle_irq = omap8250_no_handle_irq;
#ifdef CONFIG_SERIAL_8250_DMA
	if (pdev->dev.of_node) {
		/*
		 * Oh DMA support. If there are no DMA properties in the DT then
		 * we will fall back to a generic DMA channel which does not
		 * really work here. To ensure that we do not get a generic DMA
		 * channel assigned, we have the the_no_dma_filter_fn() here.
		 * To avoid "failed to request DMA" messages we check for DMA
		 * properties in DT.
		 */
		ret = of_property_count_strings(pdev->dev.of_node, "dma-names");
		if (ret == 2) {
			up.dma = &priv->omap8250_dma;
			priv->omap8250_dma.fn = the_no_dma_filter_fn;
			priv->omap8250_dma.tx_dma = omap_8250_tx_dma;
			priv->omap8250_dma.rx_dma = omap_8250_rx_dma;
			priv->omap8250_dma.rx_size = RX_TRIGGER;
			priv->omap8250_dma.rxconf.src_maxburst = RX_TRIGGER;
			priv->omap8250_dma.txconf.dst_maxburst = TX_TRIGGER;

			if (of_machine_is_compatible("ti,am33xx"))
				priv->habit |= OMAP_DMA_TX_KICK;
		}
	}
#endif
	ret = serial8250_register_8250_port(&up);
	if (ret < 0) {
		dev_err(&pdev->dev, "unable to register 8250 port\n");
		goto err;
	}
	priv->line = ret;
	platform_set_drvdata(pdev, priv);
	pm_runtime_mark_last_busy(&pdev->dev);
	pm_runtime_put_autosuspend(&pdev->dev);
	return 0;
err:
	pm_runtime_dont_use_autosuspend(&pdev->dev);
	pm_runtime_put_sync(&pdev->dev);
	pm_runtime_disable(&pdev->dev);
	return ret;
}

static int omap8250_remove(struct platform_device *pdev)
{
	struct omap8250_priv *priv = platform_get_drvdata(pdev);

	pm_runtime_dont_use_autosuspend(&pdev->dev);
	pm_runtime_put_sync(&pdev->dev);
	pm_runtime_disable(&pdev->dev);
	serial8250_unregister_port(priv->line);
	pm_qos_remove_request(&priv->pm_qos_request);
	device_init_wakeup(&pdev->dev, false);
	return 0;
}

#ifdef CONFIG_PM

static inline void omap8250_enable_wakeirq(struct omap8250_priv *priv,
					   bool enable)
{
	if (!priv->wakeirq)
		return;

	if (enable)
		enable_irq(priv->wakeirq);
	else
		disable_irq_nosync(priv->wakeirq);
}

static void omap8250_enable_wakeup(struct omap8250_priv *priv,
				   bool enable)
{
	if (enable == priv->wakeups_enabled)
		return;

	omap8250_enable_wakeirq(priv, enable);
	priv->wakeups_enabled = enable;
}
#endif

#ifdef CONFIG_PM_SLEEP
static int omap8250_prepare(struct device *dev)
{
	struct omap8250_priv *priv = dev_get_drvdata(dev);

	if (!priv)
		return 0;
	priv->is_suspending = true;
	return 0;
}

static void omap8250_complete(struct device *dev)
{
	struct omap8250_priv *priv = dev_get_drvdata(dev);

	if (!priv)
		return;
	priv->is_suspending = false;
}

static int omap8250_suspend(struct device *dev)
{
	struct omap8250_priv *priv = dev_get_drvdata(dev);

	serial8250_suspend_port(priv->line);
	flush_work(&priv->qos_work);

	if (device_may_wakeup(dev))
		omap8250_enable_wakeup(priv, true);
	else
		omap8250_enable_wakeup(priv, false);
	return 0;
}

static int omap8250_resume(struct device *dev)
{
	struct omap8250_priv *priv = dev_get_drvdata(dev);

	if (device_may_wakeup(dev))
		omap8250_enable_wakeup(priv, false);

	serial8250_resume_port(priv->line);
	return 0;
}
#else
#define omap8250_prepare NULL
#define omap8250_complete NULL
#endif

#ifdef CONFIG_PM
static int omap8250_lost_context(struct uart_8250_port *up)
{
	u32 val;

	val = serial_in(up, UART_OMAP_MDR1);
	/*
	 * If we lose context, then MDR1 is set to its reset value which is
	 * UART_OMAP_MDR1_DISABLE. After set_termios() we set it either to 13x
	 * or 16x but never to disable again.
	 */
	if (val == UART_OMAP_MDR1_DISABLE)
		return 1;
	return 0;
}

static int omap8250_runtime_suspend(struct device *dev)
{
	struct omap8250_priv *priv = dev_get_drvdata(dev);
	struct uart_8250_port *up;

	/* In case runtime-pm tries this before we are setup */
	if (!priv)
		return 0;

	up = serial8250_get_port(priv->line);
	/*
	 * When using 'no_console_suspend', the console UART must not be
	 * suspended. Since driver suspend is managed by runtime suspend,
	 * preventing runtime suspend (by returning error) will keep device
	 * active during suspend.
	 */
	if (priv->is_suspending && !console_suspend_enabled) {
		if (uart_console(&up->port))
			return -EBUSY;
	}

	omap8250_enable_wakeup(priv, true);
	if (up->dma)
		omap_8250_rx_dma(up, UART_IIR_RX_TIMEOUT);

	priv->latency = PM_QOS_CPU_DMA_LAT_DEFAULT_VALUE;
	schedule_work(&priv->qos_work);

	return 0;
}

static int omap8250_runtime_resume(struct device *dev)
{
	struct omap8250_priv *priv = dev_get_drvdata(dev);
	struct uart_8250_port *up;
	int loss_cntx;

	/* In case runtime-pm tries this before we are setup */
	if (!priv)
		return 0;

	up = serial8250_get_port(priv->line);
	omap8250_enable_wakeup(priv, false);
	loss_cntx = omap8250_lost_context(up);

	if (loss_cntx)
		omap8250_restore_regs(up);

	if (up->dma)
		omap_8250_rx_dma(up, 0);

	priv->latency = priv->calc_latency;
	schedule_work(&priv->qos_work);
	return 0;
}
#endif

#ifdef CONFIG_SERIAL_8250_OMAP_TTYO_FIXUP
static int __init omap8250_console_fixup(void)
{
	char *omap_str;
	char *options;
	u8 idx;

	if (strstr(boot_command_line, "console=ttyS"))
		/* user set a ttyS based name for the console */
		return 0;

	omap_str = strstr(boot_command_line, "console=ttyO");
	if (!omap_str)
		/* user did not set ttyO based console, so we don't care */
		return 0;

	omap_str += 12;
	if ('0' <= *omap_str && *omap_str <= '9')
		idx = *omap_str - '0';
	else
		return 0;

	omap_str++;
	if (omap_str[0] == ',') {
		omap_str++;
		options = omap_str;
	} else {
		options = NULL;
	}

	add_preferred_console("ttyS", idx, options);
	pr_err("WARNING: Your 'console=ttyO%d' has been replaced by 'ttyS%d'\n",
	       idx, idx);
	pr_err("This ensures that you still see kernel messages. Please\n");
	pr_err("update your kernel commandline.\n");
	return 0;
}
console_initcall(omap8250_console_fixup);
#endif

static const struct dev_pm_ops omap8250_dev_pm_ops = {
	SET_SYSTEM_SLEEP_PM_OPS(omap8250_suspend, omap8250_resume)
	SET_RUNTIME_PM_OPS(omap8250_runtime_suspend,
			   omap8250_runtime_resume, NULL)
	.prepare        = omap8250_prepare,
	.complete       = omap8250_complete,
};

static const struct of_device_id omap8250_dt_ids[] = {
	{ .compatible = "ti,omap2-uart" },
	{ .compatible = "ti,omap3-uart" },
	{ .compatible = "ti,omap4-uart" },
	{},
};
MODULE_DEVICE_TABLE(of, omap8250_dt_ids);

static struct platform_driver omap8250_platform_driver = {
	.driver = {
		.name		= "omap8250",
		.pm		= &omap8250_dev_pm_ops,
		.of_match_table = omap8250_dt_ids,
	},
	.probe			= omap8250_probe,
	.remove			= omap8250_remove,
};
module_platform_driver(omap8250_platform_driver);

MODULE_AUTHOR("Sebastian Andrzej Siewior");
MODULE_DESCRIPTION("OMAP 8250 Driver");
MODULE_LICENSE("GPL v2");

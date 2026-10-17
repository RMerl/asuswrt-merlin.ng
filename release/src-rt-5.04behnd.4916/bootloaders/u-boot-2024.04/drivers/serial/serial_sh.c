// SPDX-License-Identifier: GPL-2.0+
/*
 * SuperH SCIF device driver.
 * Copyright (C) 2013  Renesas Electronics Corporation
 * Copyright (C) 2007,2008,2010, 2014 Nobuhiro Iwamatsu
 * Copyright (C) 2002 - 2008  Paul Mundt
 */

#include <asm/global_data.h>
#include <asm/io.h>
#include <asm/processor.h>
#include <clk.h>
#include <dm.h>
#include <dm/device_compat.h>
#include <dm/platform_data/serial_sh.h>
#include <errno.h>
#include <linux/compiler.h>
#include <linux/delay.h>
#include <reset.h>
#include <serial.h>
#include "serial_sh.h"

DECLARE_GLOBAL_DATA_PTR;

#if defined(CONFIG_CPU_SH7780)
static int scif_rxfill(struct uart_port *port)
{
	return sci_in(port, SCRFDR) & 0xff;
}
#elif defined(CONFIG_CPU_SH7763)
static int scif_rxfill(struct uart_port *port)
{
	if ((port->mapbase == 0xffe00000) ||
	    (port->mapbase == 0xffe08000)) {
		/* SCIF0/1*/
		return sci_in(port, SCRFDR) & 0xff;
	} else {
		/* SCIF2 */
		return sci_in(port, SCFDR) & SCIF2_RFDC_MASK;
	}
}
#else
static int scif_rxfill(struct uart_port *port)
{
	return sci_in(port, SCFDR) & SCIF_RFDC_MASK;
}
#endif

static void sh_serial_init_generic(struct uart_port *port)
{
	sci_out(port, SCSCR , SCSCR_INIT(port));
	sci_out(port, SCSCR , SCSCR_INIT(port));
	sci_out(port, SCSMR, 0);
	sci_out(port, SCSMR, 0);
	sci_out(port, SCFCR, SCFCR_RFRST|SCFCR_TFRST);
	sci_in(port, SCFCR);
	sci_out(port, SCFCR, 0);
#if defined(CONFIG_RZA1)
	sci_out(port, SCSPTR, 0x0003);
#endif

#if IS_ENABLED(CONFIG_RCAR_GEN2) || IS_ENABLED(CONFIG_RCAR_GEN3) || IS_ENABLED(CONFIG_RCAR_GEN4)
	if (port->type == PORT_HSCIF)
		sci_out(port, HSSRR, HSSRR_SRE | HSSRR_SRCYC8);
#endif
}

static void
sh_serial_setbrg_generic(struct uart_port *port, int clk, int baudrate)
{
	if (port->clk_mode == EXT_CLK) {
		unsigned short dl = DL_VALUE(baudrate, clk);
		sci_out(port, DL, dl);
		/* Need wait: Clock * 1/dl * 1/16 */
		udelay((1000000 * dl * 16 / clk) * 1000 + 1);
	} else {
		sci_out(port, SCBRR, SCBRR_VALUE(baudrate, clk));
	}
}

static void handle_error(struct uart_port *port)
{
	/*
	 * Most errors are cleared by resetting the relevant error bits to zero
	 * in the FSR & LSR registers. For each register, a read followed by a
	 * write is needed according to the relevant datasheets.
	 */
	unsigned short status = sci_in(port, SCxSR);
	sci_out(port, SCxSR, status & ~SCxSR_ERRORS(port));
	sci_in(port, SCLSR);
	sci_out(port, SCLSR, 0x00);

	/*
	 * To clear framing errors, we also need to read and discard a
	 * character.
	 */
	if ((port->type != PORT_SCI) && (status & SCIF_FER))
		sci_in(port, SCxRDR);
}

static int serial_raw_putc(struct uart_port *port, const char c)
{
	/* Tx fifo is empty */
	if (!(sci_in(port, SCxSR) & SCxSR_TEND(port)))
		return -EAGAIN;

	sci_out(port, SCxTDR, c);
	sci_out(port, SCxSR, sci_in(port, SCxSR) & ~SCxSR_TEND(port));

	return 0;
}

static int serial_rx_fifo_level(struct uart_port *port)
{
	return scif_rxfill(port);
}

static int sh_serial_tstc_generic(struct uart_port *port)
{
	if (sci_in(port, SCxSR) & SCIF_ERRORS) {
		handle_error(port);
		return 0;
	}

	return serial_rx_fifo_level(port) ? 1 : 0;
}

static int serial_getc_check(struct uart_port *port)
{
	unsigned short status;

	status = sci_in(port, SCxSR);

	if (status & SCIF_ERRORS)
		handle_error(port);
	if (sci_in(port, SCLSR) & SCxSR_ORER(port))
		handle_error(port);
	status &= (SCIF_DR | SCxSR_RDxF(port));
	if (status)
		return status;
	return scif_rxfill(port);
}

static int sh_serial_getc_generic(struct uart_port *port)
{
	unsigned short status;
	char ch;

	if (!serial_getc_check(port))
		return -EAGAIN;

	ch = sci_in(port, SCxRDR);
	status = sci_in(port, SCxSR);

	sci_out(port, SCxSR, SCxSR_RDxF_CLEAR(port));

	if (status & SCIF_ERRORS)
		handle_error(port);

	if (sci_in(port, SCLSR) & SCxSR_ORER(port))
		handle_error(port);

	return ch;
}

#if CONFIG_IS_ENABLED(DM_SERIAL)

static int sh_serial_pending(struct udevice *dev, bool input)
{
	struct uart_port *priv = dev_get_priv(dev);

	return sh_serial_tstc_generic(priv);
}

static int sh_serial_putc(struct udevice *dev, const char ch)
{
	struct uart_port *priv = dev_get_priv(dev);

	return serial_raw_putc(priv, ch);
}

static int sh_serial_getc(struct udevice *dev)
{
	struct uart_port *priv = dev_get_priv(dev);

	return sh_serial_getc_generic(priv);
}

static int sh_serial_setbrg(struct udevice *dev, int baudrate)
{
	struct sh_serial_plat *plat = dev_get_plat(dev);
	struct uart_port *priv = dev_get_priv(dev);

	sh_serial_setbrg_generic(priv, plat->clk, baudrate);

	return 0;
}

static int sh_serial_probe(struct udevice *dev)
{
	struct sh_serial_plat *plat = dev_get_plat(dev);
	struct uart_port *priv = dev_get_priv(dev);
	struct reset_ctl rst;
	int ret;

	priv->membase	= (unsigned char *)plat->base;
	priv->mapbase	= plat->base;
	priv->type	= plat->type;
	priv->clk_mode	= plat->clk_mode;

	/* De-assert the module reset if it is defined. */
	ret = reset_get_by_index(dev, 0, &rst);
	if (!ret) {
		ret = reset_deassert(&rst);
		if (ret < 0) {
			dev_err(dev, "failed to de-assert reset line\n");
			return ret;
		}
	}

	sh_serial_init_generic(priv);

	return 0;
}

static const struct dm_serial_ops sh_serial_ops = {
	.putc = sh_serial_putc,
	.pending = sh_serial_pending,
	.getc = sh_serial_getc,
	.setbrg = sh_serial_setbrg,
};

#if CONFIG_IS_ENABLED(OF_CONTROL)
static const struct udevice_id sh_serial_id[] ={
	{.compatible = "renesas,sci", .data = PORT_SCI},
	{.compatible = "renesas,scif", .data = PORT_SCIF},
	{.compatible = "renesas,scif-r9a07g044", .data = PORT_SCIFA},
	{.compatible = "renesas,scifa", .data = PORT_SCIFA},
	{.compatible = "renesas,hscif", .data = PORT_HSCIF},
	{}
};

static int sh_serial_of_to_plat(struct udevice *dev)
{
	struct sh_serial_plat *plat = dev_get_plat(dev);
	struct clk sh_serial_clk;
	fdt_addr_t addr;
	int ret;

	addr = dev_read_addr(dev);
	if (!addr)
		return -EINVAL;

	plat->base = addr;

	ret = clk_get_by_name(dev, "fck", &sh_serial_clk);
	if (!ret) {
		ret = clk_enable(&sh_serial_clk);
		if (!ret)
			plat->clk = clk_get_rate(&sh_serial_clk);
	} else {
		plat->clk = fdtdec_get_int(gd->fdt_blob, dev_of_offset(dev),
					   "clock", 1);
	}

	plat->type = dev_get_driver_data(dev);
	return 0;
}
#endif

U_BOOT_DRIVER(serial_sh) = {
	.name	= "serial_sh",
	.id	= UCLASS_SERIAL,
	.of_match = of_match_ptr(sh_serial_id),
	.of_to_plat = of_match_ptr(sh_serial_of_to_plat),
	.plat_auto	= sizeof(struct sh_serial_plat),
	.probe	= sh_serial_probe,
	.ops	= &sh_serial_ops,
#if !CONFIG_IS_ENABLED(OF_CONTROL)
	.flags	= DM_FLAG_PRE_RELOC,
#endif
	.priv_auto	= sizeof(struct uart_port),
};
#endif

#if !CONFIG_IS_ENABLED(DM_SERIAL) || IS_ENABLED(CONFIG_DEBUG_UART_SCIF)

#if defined(CFG_SCIF_A)
	#define SCIF_BASE_PORT	PORT_SCIFA
#elif defined(CFG_SCI)
	#define SCIF_BASE_PORT  PORT_SCI
#elif defined(CFG_HSCIF)
	#define SCIF_BASE_PORT  PORT_HSCIF
#else
	#define SCIF_BASE_PORT	PORT_SCIF
#endif

static void sh_serial_init_nodm(struct uart_port *port)
{
	sh_serial_init_generic(port);
	serial_setbrg();
}

static void sh_serial_putc_nondm(struct uart_port *port, const char c)
{
	if (c == '\n') {
		while (1) {
			if  (serial_raw_putc(port, '\r') != -EAGAIN)
				break;
		}
	}
	while (1) {
		if  (serial_raw_putc(port, c) != -EAGAIN)
			break;
	}
}
#endif

#if !CONFIG_IS_ENABLED(DM_SERIAL)
#if defined(CONFIG_CONS_SCIF0)
# define SCIF_BASE	SCIF0_BASE
#elif defined(CONFIG_CONS_SCIF1)
# define SCIF_BASE	SCIF1_BASE
#elif defined(CONFIG_CONS_SCIF2)
# define SCIF_BASE	SCIF2_BASE
#elif defined(CONFIG_CONS_SCIF3)
# define SCIF_BASE	SCIF3_BASE
#elif defined(CONFIG_CONS_SCIF4)
# define SCIF_BASE	SCIF4_BASE
#elif defined(CONFIG_CONS_SCIF5)
# define SCIF_BASE	SCIF5_BASE
#elif defined(CONFIG_CONS_SCIF6)
# define SCIF_BASE	SCIF6_BASE
#elif defined(CONFIG_CONS_SCIF7)
# define SCIF_BASE	SCIF7_BASE
#elif defined(CONFIG_CONS_SCIFA0)
# define SCIF_BASE	SCIFA0_BASE
#else
# error "Default SCIF doesn't set....."
#endif

static struct uart_port sh_sci = {
	.membase	= (unsigned char *)SCIF_BASE,
	.mapbase	= SCIF_BASE,
	.type		= SCIF_BASE_PORT,
#ifdef CFG_SCIF_USE_EXT_CLK
	.clk_mode =	EXT_CLK,
#endif
};

static void sh_serial_setbrg(void)
{
	DECLARE_GLOBAL_DATA_PTR;
	struct uart_port *port = &sh_sci;

	sh_serial_setbrg_generic(port, CONFIG_SH_SCIF_CLK_FREQ, gd->baudrate);
}

static int sh_serial_init(void)
{
	sh_serial_init_nodm(&sh_sci);

	return 0;
}

static void sh_serial_putc(const char c)
{
	sh_serial_putc_nondm(&sh_sci, c);
}

static int sh_serial_tstc(void)
{
	struct uart_port *port = &sh_sci;

	return sh_serial_tstc_generic(port);
}

static int sh_serial_getc(void)
{
	struct uart_port *port = &sh_sci;
	int ch;

	while (1) {
		ch = sh_serial_getc_generic(port);
		if (ch != -EAGAIN)
			break;
	}

	return ch;
}

static struct serial_device sh_serial_drv = {
	.name	= "sh_serial",
	.start	= sh_serial_init,
	.stop	= NULL,
	.setbrg	= sh_serial_setbrg,
	.putc	= sh_serial_putc,
	.puts	= default_serial_puts,
	.getc	= sh_serial_getc,
	.tstc	= sh_serial_tstc,
};

void sh_serial_initialize(void)
{
	serial_register(&sh_serial_drv);
}

__weak struct serial_device *default_serial_console(void)
{
	return &sh_serial_drv;
}
#endif /* CONFIG_DM_SERIAL */

#ifdef CONFIG_DEBUG_UART_SCIF
#include <debug_uart.h>

static struct uart_port debug_uart_sci = {
	.membase	= (unsigned char *)CONFIG_DEBUG_UART_BASE,
	.mapbase	= CONFIG_DEBUG_UART_BASE,
	.type		= SCIF_BASE_PORT,
#ifdef CFG_SCIF_USE_EXT_CLK
	.clk_mode =	EXT_CLK,
#endif
};

static inline void _debug_uart_init(void)
{
	sh_serial_init_nodm(&debug_uart_sci);
}

static inline void _debug_uart_putc(int c)
{
	sh_serial_putc_nondm(&debug_uart_sci, c);
}

DEBUG_UART_FUNCS

#endif

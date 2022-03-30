// SPDX-License-Identifier: GPL-2.0+
/*
 * SuperH SCIF device driver.
 * Copyright (C) 2013  Renesas Electronics Corporation
 * Copyright (C) 2007,2008,2010, 2014 Nobuhiro Iwamatsu
 * Copyright (C) 2002 - 2008  Paul Mundt
 */

#include <common.h>
#include <errno.h>
#include <clk.h>
#include <dm.h>
#include <asm/io.h>
#include <asm/processor.h>
#include <serial.h>
#include <linux/compiler.h>
#include <dm/platform_data/serial_sh.h>
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
	sci_in(port, SCxSR);
	sci_out(port, SCxSR, SCxSR_ERROR_CLEAR(port));
	sci_in(port, SCLSR);
	sci_out(port, SCLSR, 0x00);
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
	return status & (SCIF_DR | SCxSR_RDxF(port));
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
	struct sh_serial_platdata *plat = dev_get_platdata(dev);
	struct uart_port *priv = dev_get_priv(dev);

	sh_serial_setbrg_generic(priv, plat->clk, baudrate);

	return 0;
}

static int sh_serial_probe(struct udevice *dev)
{
	struct sh_serial_platdata *plat = dev_get_platdata(dev);
	struct uart_port *priv = dev_get_priv(dev);

	priv->membase	= (unsigned char *)plat->base;
	priv->mapbase	= plat->base;
	priv->type	= plat->type;
	priv->clk_mode	= plat->clk_mode;

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
	{.compatible = "renesas,scifa", .data = PORT_SCIFA},
	{}
};

static int sh_serial_ofdata_to_platdata(struct udevice *dev)
{
	struct sh_serial_platdata *plat = dev_get_platdata(dev);
	struct clk sh_serial_clk;
	fdt_addr_t addr;
	int ret;

	addr = devfdt_get_addr(dev);
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
	.ofdata_to_platdata = of_match_ptr(sh_serial_ofdata_to_platdata),
	.platdata_auto_alloc_size = sizeof(struct sh_serial_platdata),
	.probe	= sh_serial_probe,
	.ops	= &sh_serial_ops,
#if !CONFIG_IS_ENABLED(OF_CONTROL)
	.flags	= DM_FLAG_PRE_RELOC,
#endif
	.priv_auto_alloc_size = sizeof(struct uart_port),
};

#else /* CONFIG_DM_SERIAL */

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

#if defined(CONFIG_SCIF_A)
	#define SCIF_BASE_PORT	PORT_SCIFA
#elif defined(CONFIG_SCI)
	#define SCIF_BASE_PORT  PORT_SCI
#else
	#define SCIF_BASE_PORT	PORT_SCIF
#endif

static struct uart_port sh_sci = {
	.membase	= (unsigned char *)SCIF_BASE,
	.mapbase	= SCIF_BASE,
	.type		= SCIF_BASE_PORT,
#ifdef CONFIG_SCIF_USE_EXT_CLK
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
	struct uart_port *port = &sh_sci;

	sh_serial_init_generic(port);
	serial_setbrg();

	return 0;
}

static void sh_serial_putc(const char c)
{
	struct uart_port *port = &sh_sci;

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

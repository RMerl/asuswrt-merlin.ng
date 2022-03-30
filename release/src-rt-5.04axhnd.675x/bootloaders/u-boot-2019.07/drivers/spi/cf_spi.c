// SPDX-License-Identifier: GPL-2.0+
/*
 *
 * (C) Copyright 2000-2003
 * Wolfgang Denk, DENX Software Engineering, wd@denx.de.
 *
 * Copyright (C) 2004-2009 Freescale Semiconductor, Inc.
 * TsiChung Liew (Tsi-Chung.Liew@freescale.com)
 *
 * Support for DM and DT, non-DM code removed.
 * Copyright (C) 2018 Angelo Dureghello <angelo@sysam.it>
 *
 * TODO: fsl_dspi.c should work as a driver for the DSPI module.
 */

#include <common.h>
#include <dm.h>
#include <dm/platform_data/spi_coldfire.h>
#include <spi.h>
#include <malloc.h>
#include <asm/coldfire/dspi.h>
#include <asm/io.h>

struct coldfire_spi_priv {
	struct dspi *regs;
	uint baudrate;
	int mode;
	int charbit;
};

DECLARE_GLOBAL_DATA_PTR;

#ifndef CONFIG_SPI_IDLE_VAL
#if defined(CONFIG_SPI_MMC)
#define CONFIG_SPI_IDLE_VAL	0xFFFF
#else
#define CONFIG_SPI_IDLE_VAL	0x0
#endif
#endif

/*
 * DSPI specific mode
 *
 * bit 31 - 28: Transfer size 3 to 16 bits
 *     27 - 26: PCS to SCK delay prescaler
 *     25 - 24: After SCK delay prescaler
 *     23 - 22: Delay after transfer prescaler
 *     21     : Allow overwrite for bit 31-22 and bit 20-8
 *     20     : Double baud rate
 *     19 - 16: PCS to SCK delay scaler
 *     15 - 12: After SCK delay scaler
 *     11 -  8: Delay after transfer scaler
 *      7 -  0: SPI_CPHA, SPI_CPOL, SPI_LSB_FIRST
 */
#define SPI_MODE_MOD			0x00200000
#define SPI_MODE_DBLRATE		0x00100000

#define SPI_MODE_XFER_SZ_MASK		0xf0000000
#define SPI_MODE_DLY_PRE_MASK		0x0fc00000
#define SPI_MODE_DLY_SCA_MASK		0x000fff00

#define MCF_FRM_SZ_16BIT		DSPI_CTAR_TRSZ(0xf)
#define MCF_DSPI_SPEED_BESTMATCH	0x7FFFFFFF
#define MCF_DSPI_MAX_CTAR_REGS		8

/* Default values */
#define MCF_DSPI_DEFAULT_SCK_FREQ	10000000
#define MCF_DSPI_DEFAULT_MAX_CS		4
#define MCF_DSPI_DEFAULT_MODE		0

#define MCF_DSPI_DEFAULT_CTAR		(DSPI_CTAR_TRSZ(7) | \
					DSPI_CTAR_PCSSCK_1CLK | \
					DSPI_CTAR_PASC(0) | \
					DSPI_CTAR_PDT(0) | \
					DSPI_CTAR_CSSCK(0) | \
					DSPI_CTAR_ASC(0) | \
					DSPI_CTAR_DT(1) | \
					DSPI_CTAR_BR(6))

#define MCF_CTAR_MODE_MASK		(MCF_FRM_SZ_16BIT | \
					DSPI_CTAR_PCSSCK(3) | \
					DSPI_CTAR_PASC_7CLK | \
					DSPI_CTAR_PDT(3) | \
					DSPI_CTAR_CSSCK(0x0f) | \
					DSPI_CTAR_ASC(0x0f) | \
					DSPI_CTAR_DT(0x0f))

#define setup_ctrl(ctrl, cs)	((ctrl & 0xFF000000) | ((1 << cs) << 16))

static inline void cfspi_tx(struct coldfire_spi_priv *cfspi,
			    u32 ctrl, u16 data)
{
	/*
	 * Need to check fifo level here
	 */
	while ((readl(&cfspi->regs->sr) & 0x0000F000) >= 0x4000)
		;

	writel(ctrl | data, &cfspi->regs->tfr);
}

static inline u16 cfspi_rx(struct coldfire_spi_priv *cfspi)
{

	while ((readl(&cfspi->regs->sr) & 0x000000F0) == 0)
		;

	return readw(&cfspi->regs->rfr);
}

static int coldfire_spi_claim_bus(struct udevice *dev)
{
	struct udevice *bus = dev->parent;
	struct coldfire_spi_priv *cfspi = dev_get_priv(bus);
	struct dspi *dspi = cfspi->regs;
	struct dm_spi_slave_platdata *slave_plat =
		dev_get_parent_platdata(dev);

	if ((in_be32(&dspi->sr) & DSPI_SR_TXRXS) != DSPI_SR_TXRXS)
		return -1;

	/* Clear FIFO and resume transfer */
	clrbits_be32(&dspi->mcr, DSPI_MCR_CTXF | DSPI_MCR_CRXF);

	dspi_chip_select(slave_plat->cs);

	return 0;
}

static int coldfire_spi_release_bus(struct udevice *dev)
{
	struct udevice *bus = dev->parent;
	struct coldfire_spi_priv *cfspi = dev_get_priv(bus);
	struct dspi *dspi = cfspi->regs;
	struct dm_spi_slave_platdata *slave_plat =
		dev_get_parent_platdata(dev);

	/* Clear FIFO */
	clrbits_be32(&dspi->mcr, DSPI_MCR_CTXF | DSPI_MCR_CRXF);

	dspi_chip_unselect(slave_plat->cs);

	return 0;
}

static int coldfire_spi_xfer(struct udevice *dev, unsigned int bitlen,
			     const void *dout, void *din,
			     unsigned long flags)
{
	struct udevice *bus = dev_get_parent(dev);
	struct coldfire_spi_priv *cfspi = dev_get_priv(bus);
	struct dm_spi_slave_platdata *slave_plat = dev_get_parent_platdata(dev);
	u16 *spi_rd16 = NULL, *spi_wr16 = NULL;
	u8 *spi_rd = NULL, *spi_wr = NULL;
	static u32 ctrl;
	uint len = bitlen >> 3;

	if (cfspi->charbit == 16) {
		bitlen >>= 1;
		spi_wr16 = (u16 *)dout;
		spi_rd16 = (u16 *)din;
	} else {
		spi_wr = (u8 *)dout;
		spi_rd = (u8 *)din;
	}

	if ((flags & SPI_XFER_BEGIN) == SPI_XFER_BEGIN)
		ctrl |= DSPI_TFR_CONT;

	ctrl = setup_ctrl(ctrl, slave_plat->cs);

	if (len > 1) {
		int tmp_len = len - 1;

		while (tmp_len--) {
			if (dout) {
				if (cfspi->charbit == 16)
					cfspi_tx(cfspi, ctrl, *spi_wr16++);
				else
					cfspi_tx(cfspi, ctrl, *spi_wr++);
				cfspi_rx(cfspi);
			}

			if (din) {
				cfspi_tx(cfspi, ctrl, CONFIG_SPI_IDLE_VAL);
				if (cfspi->charbit == 16)
					*spi_rd16++ = cfspi_rx(cfspi);
				else
					*spi_rd++ = cfspi_rx(cfspi);
			}
		}

		len = 1;	/* remaining byte */
	}

	if (flags & SPI_XFER_END)
		ctrl &= ~DSPI_TFR_CONT;

	if (len) {
		if (dout) {
			if (cfspi->charbit == 16)
				cfspi_tx(cfspi, ctrl, *spi_wr16);
			else
				cfspi_tx(cfspi, ctrl, *spi_wr);
			cfspi_rx(cfspi);
		}

		if (din) {
			cfspi_tx(cfspi, ctrl, CONFIG_SPI_IDLE_VAL);
			if (cfspi->charbit == 16)
				*spi_rd16 = cfspi_rx(cfspi);
			else
				*spi_rd = cfspi_rx(cfspi);
		}
	} else {
		/* dummy read */
		cfspi_tx(cfspi, ctrl, CONFIG_SPI_IDLE_VAL);
		cfspi_rx(cfspi);
	}

	return 0;
}

static int coldfire_spi_set_speed(struct udevice *bus, uint max_hz)
{
	struct coldfire_spi_priv *cfspi = dev_get_priv(bus);
	struct dspi *dspi = cfspi->regs;
	int prescaler[] = { 2, 3, 5, 7 };
	int scaler[] = {
		2, 4, 6, 8,
		16, 32, 64, 128,
		256, 512, 1024, 2048,
		4096, 8192, 16384, 32768
	};
	int i, j, pbrcnt, brcnt, diff, tmp, dbr = 0;
	int best_i, best_j, bestmatch = MCF_DSPI_SPEED_BESTMATCH, baud_speed;
	u32 bus_setup;

	cfspi->baudrate = max_hz;

	/* Read current setup */
	bus_setup = readl(&dspi->ctar[bus->seq]);

	tmp = (prescaler[3] * scaler[15]);
	/* Maximum and minimum baudrate it can handle */
	if ((cfspi->baudrate > (gd->bus_clk >> 1)) ||
	    (cfspi->baudrate < (gd->bus_clk / tmp))) {
		printf("Exceed baudrate limitation: Max %d - Min %d\n",
		       (int)(gd->bus_clk >> 1), (int)(gd->bus_clk / tmp));
		return -1;
	}

	/* Activate Double Baud when it exceed 1/4 the bus clk */
	if ((bus_setup & DSPI_CTAR_DBR) ||
	    (cfspi->baudrate > (gd->bus_clk / (prescaler[0] * scaler[0])))) {
		bus_setup |= DSPI_CTAR_DBR;
		dbr = 1;
	}

	/* Overwrite default value set in platform configuration file */
	if (cfspi->mode & SPI_MODE_MOD) {
		/*
		 * Check to see if it is enabled by default in platform
		 * config, or manual setting passed by mode parameter
		 */
		if (cfspi->mode & SPI_MODE_DBLRATE) {
			bus_setup |= DSPI_CTAR_DBR;
			dbr = 1;
		}
	}

	pbrcnt = sizeof(prescaler) / sizeof(int);
	brcnt = sizeof(scaler) / sizeof(int);

	/* baudrate calculation - to closer value, may not be exact match */
	for (best_i = 0, best_j = 0, i = 0; i < pbrcnt; i++) {
		baud_speed = gd->bus_clk / prescaler[i];
		for (j = 0; j < brcnt; j++) {
			tmp = (baud_speed / scaler[j]) * (1 + dbr);

			if (tmp > cfspi->baudrate)
				diff = tmp - cfspi->baudrate;
			else
				diff = cfspi->baudrate - tmp;

			if (diff < bestmatch) {
				bestmatch = diff;
				best_i = i;
				best_j = j;
			}
		}
	}

	bus_setup &= ~(DSPI_CTAR_PBR(0x03) | DSPI_CTAR_BR(0x0f));
	bus_setup |= (DSPI_CTAR_PBR(best_i) | DSPI_CTAR_BR(best_j));
	writel(bus_setup, &dspi->ctar[bus->seq]);

	return 0;
}

static int coldfire_spi_set_mode(struct udevice *bus, uint mode)
{
	struct coldfire_spi_priv *cfspi = dev_get_priv(bus);
	struct dspi *dspi = cfspi->regs;
	u32 bus_setup = 0;

	cfspi->mode = mode;

	if (cfspi->mode & SPI_CPOL)
		bus_setup |= DSPI_CTAR_CPOL;
	if (cfspi->mode & SPI_CPHA)
		bus_setup |= DSPI_CTAR_CPHA;
	if (cfspi->mode & SPI_LSB_FIRST)
		bus_setup |= DSPI_CTAR_LSBFE;

	/* Overwrite default value set in platform configuration file */
	if (cfspi->mode & SPI_MODE_MOD) {
		if ((cfspi->mode & SPI_MODE_XFER_SZ_MASK) == 0)
			bus_setup |=
			    readl(&dspi->ctar[bus->seq]) & MCF_FRM_SZ_16BIT;
		else
			bus_setup |=
			    ((cfspi->mode & SPI_MODE_XFER_SZ_MASK) >> 1);

		/* PSCSCK, PASC, PDT */
		bus_setup |= (cfspi->mode & SPI_MODE_DLY_PRE_MASK) >> 4;
		/* CSSCK, ASC, DT */
		bus_setup |= (cfspi->mode & SPI_MODE_DLY_SCA_MASK) >> 4;
	} else {
		bus_setup |=
			(readl(&dspi->ctar[bus->seq]) & MCF_CTAR_MODE_MASK);
	}

	cfspi->charbit =
		((readl(&dspi->ctar[bus->seq]) & MCF_FRM_SZ_16BIT) ==
			MCF_FRM_SZ_16BIT) ? 16 : 8;

	setbits_be32(&dspi->ctar[bus->seq], bus_setup);

	return 0;
}

static int coldfire_spi_probe(struct udevice *bus)
{
	struct coldfire_spi_platdata *plat = dev_get_platdata(bus);
	struct coldfire_spi_priv *cfspi = dev_get_priv(bus);
	struct dspi *dspi = cfspi->regs;
	int i;

	cfspi->regs = (struct dspi *)plat->regs_addr;

	cfspi->baudrate = plat->speed_hz;
	cfspi->mode = plat->mode;

	for (i = 0; i < MCF_DSPI_MAX_CTAR_REGS; i++) {
		unsigned int ctar = 0;

		if (plat->ctar[i][0] == 0)
			break;

		ctar = DSPI_CTAR_TRSZ(plat->ctar[i][0]) |
			DSPI_CTAR_PCSSCK(plat->ctar[i][1]) |
			DSPI_CTAR_PASC(plat->ctar[i][2]) |
			DSPI_CTAR_PDT(plat->ctar[i][3]) |
			DSPI_CTAR_CSSCK(plat->ctar[i][4]) |
			DSPI_CTAR_ASC(plat->ctar[i][5]) |
			DSPI_CTAR_DT(plat->ctar[i][6]) |
			DSPI_CTAR_BR(plat->ctar[i][7]);

		writel(ctar, &cfspi->regs->ctar[i]);
	}

	/* Default CTARs */
	for (i = 0; i < MCF_DSPI_MAX_CTAR_REGS; i++)
		writel(MCF_DSPI_DEFAULT_CTAR, &dspi->ctar[i]);

	dspi->mcr = DSPI_MCR_MSTR | DSPI_MCR_CSIS7 | DSPI_MCR_CSIS6 |
	    DSPI_MCR_CSIS5 | DSPI_MCR_CSIS4 | DSPI_MCR_CSIS3 |
	    DSPI_MCR_CSIS2 | DSPI_MCR_CSIS1 | DSPI_MCR_CSIS0 |
	    DSPI_MCR_CRXF | DSPI_MCR_CTXF;

	return 0;
}

void spi_init(void)
{
}

#if CONFIG_IS_ENABLED(OF_CONTROL) && !CONFIG_IS_ENABLED(OF_PLATDATA)
static int coldfire_dspi_ofdata_to_platdata(struct udevice *bus)
{
	fdt_addr_t addr;
	struct coldfire_spi_platdata *plat = bus->platdata;
	const void *blob = gd->fdt_blob;
	int node = dev_of_offset(bus);
	int *ctar, len;

	addr = devfdt_get_addr(bus);
	if (addr == FDT_ADDR_T_NONE)
		return -ENOMEM;

	plat->regs_addr = addr;

	plat->num_cs = fdtdec_get_int(blob, node, "num-cs",
				      MCF_DSPI_DEFAULT_MAX_CS);

	plat->speed_hz = fdtdec_get_int(blob, node, "spi-max-frequency",
					MCF_DSPI_DEFAULT_SCK_FREQ);

	plat->mode = fdtdec_get_int(blob, node, "spi-mode",
				    MCF_DSPI_DEFAULT_MODE);

	memset(plat->ctar, 0, sizeof(plat->ctar));

	ctar = (int *)fdt_getprop(blob, node, "ctar-params", &len);

	if (ctar && len) {
		int i, q, ctar_regs;

		ctar_regs = len / sizeof(unsigned int) / MAX_CTAR_FIELDS;

		if (ctar_regs > MAX_CTAR_REGS)
			ctar_regs = MAX_CTAR_REGS;

		for (i = 0; i < ctar_regs; i++) {
			for (q = 0; q < MAX_CTAR_FIELDS; q++)
				plat->ctar[i][q] = *ctar++;
		}
	}

	debug("DSPI: regs=%pa, max-frequency=%d, num-cs=%d, mode=%d\n",
	      (void *)plat->regs_addr,
	       plat->speed_hz, plat->num_cs, plat->mode);

	return 0;
}

static const struct udevice_id coldfire_spi_ids[] = {
	{ .compatible = "fsl,mcf-dspi" },
	{ }
};
#endif

static const struct dm_spi_ops coldfire_spi_ops = {
	.claim_bus	= coldfire_spi_claim_bus,
	.release_bus	= coldfire_spi_release_bus,
	.xfer		= coldfire_spi_xfer,
	.set_speed	= coldfire_spi_set_speed,
	.set_mode	= coldfire_spi_set_mode,
};

U_BOOT_DRIVER(coldfire_spi) = {
	.name = "spi_coldfire",
	.id = UCLASS_SPI,
#if CONFIG_IS_ENABLED(OF_CONTROL) && !CONFIG_IS_ENABLED(OF_PLATDATA)
	.of_match = coldfire_spi_ids,
	.ofdata_to_platdata = coldfire_dspi_ofdata_to_platdata,
	.platdata_auto_alloc_size = sizeof(struct coldfire_spi_platdata),
#endif
	.probe = coldfire_spi_probe,
	.ops = &coldfire_spi_ops,
	.priv_auto_alloc_size = sizeof(struct coldfire_spi_priv),
};

// SPDX-License-Identifier: GPL-2.0+
/*
 * (C) Copyright 2000-2003
 * Wolfgang Denk, DENX Software Engineering, wd@denx.de.
 *
 * Copyright (C) 2004-2009, 2015 Freescale Semiconductor, Inc.
 * TsiChung Liew (Tsi-Chung.Liew@freescale.com)
 * Chao Fu (B44548@freescale.com)
 * Haikun Wang (B53464@freescale.com)
 */

#include <asm/global_data.h>
#include <linux/math64.h>
#include <common.h>
#include <dm.h>
#include <errno.h>
#include <log.h>
#include <spi.h>
#include <malloc.h>
#include <asm/io.h>
#include <fdtdec.h>
#ifndef CONFIG_M68K
#include <asm/arch/clock.h>
#endif
#include <fsl_dspi.h>
#include <linux/bitops.h>
#include <linux/delay.h>
#include <linux/printk.h>
#include <linux/time.h>

DECLARE_GLOBAL_DATA_PTR;

/* fsl_dspi_plat flags */
#define DSPI_FLAG_REGMAP_ENDIAN_BIG	BIT(0)

/* idle data value */
#define DSPI_IDLE_VAL			0x0

/* max chipselect signals number */
#define FSL_DSPI_MAX_CHIPSELECT		6

/* default SCK frequency, unit: HZ */
#define FSL_DSPI_DEFAULT_SCK_FREQ	10000000

/* tx/rx data wait timeout value, unit: us */
#define DSPI_TXRX_WAIT_TIMEOUT		1000000

/* CTAR register pre-configure value */
#define DSPI_CTAR_DEFAULT_VALUE		(DSPI_CTAR_TRSZ(7) | \
					DSPI_CTAR_PCSSCK_1CLK | \
					DSPI_CTAR_PASC(0) | \
					DSPI_CTAR_PDT(0) | \
					DSPI_CTAR_CSSCK(0) | \
					DSPI_CTAR_ASC(0) | \
					DSPI_CTAR_DT(0))

/* CTAR register pre-configure mask */
#define DSPI_CTAR_SET_MODE_MASK		(DSPI_CTAR_TRSZ(15) | \
					DSPI_CTAR_PCSSCK(3) | \
					DSPI_CTAR_PASC(3) | \
					DSPI_CTAR_PDT(3) | \
					DSPI_CTAR_CSSCK(15) | \
					DSPI_CTAR_ASC(15) | \
					DSPI_CTAR_DT(15))

/**
 * struct fsl_dspi_plat - platform data for Freescale DSPI
 *
 * @flags: Flags for DSPI DSPI_FLAG_...
 * @speed_hz: Default SCK frequency
 * @num_chipselect: Number of DSPI chipselect signals
 * @regs_addr: Base address of DSPI registers
 */
struct fsl_dspi_plat {
	uint flags;
	uint speed_hz;
	uint num_chipselect;
	fdt_addr_t regs_addr;
};

/**
 * struct fsl_dspi_priv - private data for Freescale DSPI
 *
 * @flags: Flags for DSPI DSPI_FLAG_...
 * @mode: SPI mode to use for slave device (see SPI mode flags)
 * @mcr_val: MCR register configure value
 * @bus_clk: DSPI input clk frequency
 * @speed_hz: Default SCK frequency
 * @charbit: How many bits in every transfer
 * @num_chipselect: Number of DSPI chipselect signals
 * @ctar_val: CTAR register configure value of per chipselect slave device
 * @regs: Point to DSPI register structure for I/O access
 */
struct fsl_dspi_priv {
	uint flags;
	uint mode;
	uint mcr_val;
	uint bus_clk;
	uint speed_hz;
	uint charbit;
	uint num_chipselect;
	uint ctar_val[FSL_DSPI_MAX_CHIPSELECT];
	struct dspi *regs;
};

__weak void cpu_dspi_port_conf(void)
{
}

__weak int cpu_dspi_claim_bus(uint bus, uint cs)
{
	return 0;
}

__weak void cpu_dspi_release_bus(uint bus, uint cs)
{
}

static uint dspi_read32(uint flags, uint *addr)
{
	return flags & DSPI_FLAG_REGMAP_ENDIAN_BIG ?
		in_be32(addr) : in_le32(addr);
}

static void dspi_write32(uint flags, uint *addr, uint val)
{
	flags & DSPI_FLAG_REGMAP_ENDIAN_BIG ?
		out_be32(addr, val) : out_le32(addr, val);
}

static void dspi_halt(struct fsl_dspi_priv *priv, u8 halt)
{
	uint mcr_val;

	mcr_val = dspi_read32(priv->flags, &priv->regs->mcr);

	if (halt)
		mcr_val |= DSPI_MCR_HALT;
	else
		mcr_val &= ~DSPI_MCR_HALT;

	dspi_write32(priv->flags, &priv->regs->mcr, mcr_val);
}

static void fsl_dspi_init_mcr(struct fsl_dspi_priv *priv, uint cfg_val)
{
	/* halt DSPI module */
	dspi_halt(priv, 1);

	dspi_write32(priv->flags, &priv->regs->mcr, cfg_val);

	/* resume module */
	dspi_halt(priv, 0);

	priv->mcr_val = cfg_val;
}

static void fsl_dspi_cfg_cs_active_state(struct fsl_dspi_priv *priv,
		uint cs, uint state)
{
	uint mcr_val;

	dspi_halt(priv, 1);

	mcr_val = dspi_read32(priv->flags, &priv->regs->mcr);
	if (state & SPI_CS_HIGH)
		/* CSx inactive state is low */
		mcr_val &= ~DSPI_MCR_PCSIS(cs);
	else
		/* CSx inactive state is high */
		mcr_val |= DSPI_MCR_PCSIS(cs);
	dspi_write32(priv->flags, &priv->regs->mcr, mcr_val);

	dspi_halt(priv, 0);
}

static int fsl_dspi_cfg_ctar_mode(struct fsl_dspi_priv *priv,
		uint cs, uint mode)
{
	uint bus_setup;

	bus_setup = dspi_read32(priv->flags, &priv->regs->ctar[0]);

	bus_setup &= ~DSPI_CTAR_SET_MODE_MASK;
	bus_setup |= priv->ctar_val[cs];
	bus_setup &= ~(DSPI_CTAR_CPOL | DSPI_CTAR_CPHA | DSPI_CTAR_LSBFE);

	if (mode & SPI_CPOL)
		bus_setup |= DSPI_CTAR_CPOL;
	if (mode & SPI_CPHA)
		bus_setup |= DSPI_CTAR_CPHA;
	if (mode & SPI_LSB_FIRST)
		bus_setup |= DSPI_CTAR_LSBFE;

	dspi_write32(priv->flags, &priv->regs->ctar[0], bus_setup);

	priv->charbit =
		((dspi_read32(priv->flags, &priv->regs->ctar[0]) &
		  DSPI_CTAR_TRSZ(15)) == DSPI_CTAR_TRSZ(15)) ? 16 : 8;

	return 0;
}

static void fsl_dspi_clr_fifo(struct fsl_dspi_priv *priv)
{
	uint mcr_val;

	dspi_halt(priv, 1);
	mcr_val = dspi_read32(priv->flags, &priv->regs->mcr);
	/* flush RX and TX FIFO */
	mcr_val |= (DSPI_MCR_CTXF | DSPI_MCR_CRXF);
	dspi_write32(priv->flags, &priv->regs->mcr, mcr_val);
	dspi_halt(priv, 0);
}

static void dspi_tx(struct fsl_dspi_priv *priv, u32 ctrl, u16 data)
{
	int timeout = DSPI_TXRX_WAIT_TIMEOUT;

	/* wait for empty entries in TXFIFO or timeout */
	while (DSPI_SR_TXCTR(dspi_read32(priv->flags, &priv->regs->sr)) >= 4 &&
			timeout--)
		udelay(1);

	if (timeout >= 0)
		dspi_write32(priv->flags, &priv->regs->tfr, (ctrl | data));
	else
		debug("dspi_tx: waiting timeout!\n");
}

static u16 dspi_rx(struct fsl_dspi_priv *priv)
{
	int timeout = DSPI_TXRX_WAIT_TIMEOUT;

	/* wait for valid entries in RXFIFO or timeout */
	while (DSPI_SR_RXCTR(dspi_read32(priv->flags, &priv->regs->sr)) == 0 &&
			timeout--)
		udelay(1);

	if (timeout >= 0)
		return (u16)DSPI_RFR_RXDATA(
				dspi_read32(priv->flags, &priv->regs->rfr));
	else {
		debug("dspi_rx: waiting timeout!\n");
		return (u16)(~0);
	}
}

static int dspi_xfer(struct fsl_dspi_priv *priv, uint cs, unsigned int bitlen,
		const void *dout, void *din, unsigned long flags)
{
	u16 *spi_rd16 = NULL, *spi_wr16 = NULL;
	u8 *spi_rd = NULL, *spi_wr = NULL;
	static u32 ctrl;
	uint len = bitlen >> 3;

	if (priv->charbit == 16) {
		bitlen >>= 1;
		spi_wr16 = (u16 *)dout;
		spi_rd16 = (u16 *)din;
	} else {
		spi_wr = (u8 *)dout;
		spi_rd = (u8 *)din;
	}

	if ((flags & SPI_XFER_BEGIN) == SPI_XFER_BEGIN)
		ctrl |= DSPI_TFR_CONT;

	ctrl = ctrl & DSPI_TFR_CONT;
	ctrl = ctrl | DSPI_TFR_CTAS(0) | DSPI_TFR_PCS(cs);

	if (len > 1) {
		int tmp_len = len - 1;
		while (tmp_len--) {
			if ((dout != NULL) && (din != NULL)) {
				if (priv->charbit == 16) {
					dspi_tx(priv, ctrl, *spi_wr16++);
					*spi_rd16++ = dspi_rx(priv);
				}
				else {
					dspi_tx(priv, ctrl, *spi_wr++);
					*spi_rd++ = dspi_rx(priv);
				}
			}

			else if (dout != NULL) {
				if (priv->charbit == 16)
					dspi_tx(priv, ctrl, *spi_wr16++);
				else
					dspi_tx(priv, ctrl, *spi_wr++);
				dspi_rx(priv);
			}

			else if (din != NULL) {
				dspi_tx(priv, ctrl, DSPI_IDLE_VAL);
				if (priv->charbit == 16)
					*spi_rd16++ = dspi_rx(priv);
				else
					*spi_rd++ = dspi_rx(priv);
			}
		}

		len = 1;	/* remaining byte */
	}

	if ((flags & SPI_XFER_END) == SPI_XFER_END)
		ctrl &= ~DSPI_TFR_CONT;

	if (len) {
		if ((dout != NULL) && (din != NULL)) {
			if (priv->charbit == 16) {
				dspi_tx(priv, ctrl, *spi_wr16++);
				*spi_rd16++ = dspi_rx(priv);
			}
			else {
				dspi_tx(priv, ctrl, *spi_wr++);
				*spi_rd++ = dspi_rx(priv);
			}
		}

		else if (dout != NULL) {
			if (priv->charbit == 16)
				dspi_tx(priv, ctrl, *spi_wr16);
			else
				dspi_tx(priv, ctrl, *spi_wr);
			dspi_rx(priv);
		}

		else if (din != NULL) {
			dspi_tx(priv, ctrl, DSPI_IDLE_VAL);
			if (priv->charbit == 16)
				*spi_rd16 = dspi_rx(priv);
			else
				*spi_rd = dspi_rx(priv);
		}
	} else {
		/* dummy read */
		dspi_tx(priv, ctrl, DSPI_IDLE_VAL);
		dspi_rx(priv);
	}

	return 0;
}

/**
 * Calculate the divide value between input clk frequency and expected SCK frequency
 * Formula: SCK = (clkrate/pbr) x ((1+dbr)/br)
 * Dbr: use default value 0
 *
 * @pbr: return Baud Rate Prescaler value
 * @br: return Baud Rate Scaler value
 * @speed_hz: expected SCK frequency
 * @clkrate: input clk frequency
 */
static int fsl_dspi_hz_to_spi_baud(int *pbr, int *br,
		int speed_hz, uint clkrate)
{
	/* Valid baud rate pre-scaler values */
	int pbr_tbl[4] = {2, 3, 5, 7};
	int brs[16] = {2, 4, 6, 8,
		16, 32, 64, 128,
		256, 512, 1024, 2048,
		4096, 8192, 16384, 32768};
	int temp, i = 0, j = 0;

	temp = clkrate / speed_hz;

	for (i = 0; i < ARRAY_SIZE(pbr_tbl); i++)
		for (j = 0; j < ARRAY_SIZE(brs); j++) {
			if (pbr_tbl[i] * brs[j] >= temp) {
				*pbr = i;
				*br = j;
				return 0;
			}
		}

	debug("Can not find valid baud rate,speed_hz is %d, ", speed_hz);
	debug("clkrate is %d, we use the max prescaler value.\n", clkrate);

	*pbr = ARRAY_SIZE(pbr_tbl) - 1;
	*br =  ARRAY_SIZE(brs) - 1;
	return -EINVAL;
}

static void ns_delay_scale(unsigned char *psc, unsigned char *sc, int delay_ns,
			   unsigned long clkrate)
{
	int scale_needed, scale, minscale = INT_MAX;
	int pscale_tbl[4] = {1, 3, 5, 7};
	u32 remainder;
	int i, j;

	scale_needed = div_u64_rem((u64)delay_ns * clkrate, NSEC_PER_SEC,
				   &remainder);
	if (remainder)
		scale_needed++;

	for (i = 0; i < ARRAY_SIZE(pscale_tbl); i++)
		for (j = 0; j <= DSPI_CTAR_SCALE_BITS; j++) {
			scale = pscale_tbl[i] * (2 << j);
			if (scale >= scale_needed) {
				if (scale < minscale) {
					minscale = scale;
					*psc = i;
					*sc = j;
				}
				break;
			}
		}

	if (minscale == INT_MAX) {
		pr_warn("Cannot find correct scale values for %dns delay at clkrate %ld, using max prescaler value",
			delay_ns, clkrate);
		*psc = ARRAY_SIZE(pscale_tbl) - 1;
		*sc = DSPI_CTAR_SCALE_BITS;
	}
}

static int fsl_dspi_cfg_speed(struct fsl_dspi_priv *priv, uint speed)
{
	int ret;
	uint bus_setup;
	int best_i, best_j, bus_clk;

	bus_clk = priv->bus_clk;

	debug("DSPI set_speed: expected SCK speed %u, bus_clk %u.\n",
	      speed, bus_clk);

	bus_setup = dspi_read32(priv->flags, &priv->regs->ctar[0]);
	bus_setup &= ~(DSPI_CTAR_DBR | DSPI_CTAR_PBR(0x3) | DSPI_CTAR_BR(0xf));

	ret = fsl_dspi_hz_to_spi_baud(&best_i, &best_j, speed, bus_clk);
	if (ret) {
		speed = priv->speed_hz;
		debug("DSPI set_speed use default SCK rate %u.\n", speed);
		fsl_dspi_hz_to_spi_baud(&best_i, &best_j, speed, bus_clk);
	}

	bus_setup |= (DSPI_CTAR_PBR(best_i) | DSPI_CTAR_BR(best_j));
	dspi_write32(priv->flags, &priv->regs->ctar[0], bus_setup);

	priv->speed_hz = speed;

	return 0;
}

static int fsl_dspi_child_pre_probe(struct udevice *dev)
{
	struct dm_spi_slave_plat *slave_plat = dev_get_parent_plat(dev);
	struct fsl_dspi_priv *priv = dev_get_priv(dev->parent);
	u32 cs_sck_delay = 0, sck_cs_delay = 0;
	unsigned char pcssck = 0, cssck = 0;
	unsigned char pasc = 0, asc = 0;

	if (slave_plat->cs >= priv->num_chipselect) {
		debug("DSPI invalid chipselect number %d(max %d)!\n",
		      slave_plat->cs, priv->num_chipselect - 1);
		return -EINVAL;
	}

	ofnode_read_u32(dev_ofnode(dev), "fsl,spi-cs-sck-delay",
			&cs_sck_delay);
	ofnode_read_u32(dev_ofnode(dev), "fsl,spi-sck-cs-delay",
			&sck_cs_delay);

	/* Set PCS to SCK delay scale values */
	ns_delay_scale(&pcssck, &cssck, cs_sck_delay, priv->bus_clk);

	/* Set After SCK delay scale values */
	ns_delay_scale(&pasc, &asc, sck_cs_delay, priv->bus_clk);

	priv->ctar_val[slave_plat->cs] = DSPI_CTAR_DEFAULT_VALUE |
					 DSPI_CTAR_PCSSCK(pcssck) |
					 DSPI_CTAR_PASC(pasc);

	debug("DSPI pre_probe slave device on CS %u, max_hz %u, mode 0x%x.\n",
	      slave_plat->cs, slave_plat->max_hz, slave_plat->mode);

	return 0;
}

static int fsl_dspi_probe(struct udevice *bus)
{
	struct fsl_dspi_plat *plat = dev_get_plat(bus);
	struct fsl_dspi_priv *priv = dev_get_priv(bus);
	struct dm_spi_bus *dm_spi_bus;
	uint mcr_cfg_val;

	dm_spi_bus = dev_get_uclass_priv(bus);

	/* cpu special pin muxing configure */
	cpu_dspi_port_conf();

	/* get input clk frequency */
	priv->regs = (struct dspi *)plat->regs_addr;
	priv->flags = plat->flags;
#ifdef CONFIG_M68K
	priv->bus_clk = gd->bus_clk;
#else
	priv->bus_clk = mxc_get_clock(MXC_DSPI_CLK);
#endif
	priv->num_chipselect = plat->num_chipselect;
	priv->speed_hz = plat->speed_hz;
	/* frame data length in bits, default 8bits */
	priv->charbit = 8;

	dm_spi_bus->max_hz = plat->speed_hz;

	/* default: all CS signals inactive state is high */
	mcr_cfg_val = DSPI_MCR_MSTR | DSPI_MCR_PCSIS_MASK |
		DSPI_MCR_CRXF | DSPI_MCR_CTXF;
	fsl_dspi_init_mcr(priv, mcr_cfg_val);

	debug("%s probe done, bus-num %d.\n", bus->name, dev_seq(bus));

	return 0;
}

static int fsl_dspi_claim_bus(struct udevice *dev)
{
	uint sr_val;
	struct fsl_dspi_priv *priv;
	struct udevice *bus = dev->parent;
	struct dm_spi_slave_plat *slave_plat =
		dev_get_parent_plat(dev);

	priv = dev_get_priv(bus);

	/* processor special preparation work */
	cpu_dspi_claim_bus(dev_seq(bus), slave_plat->cs);

	/* configure transfer mode */
	fsl_dspi_cfg_ctar_mode(priv, slave_plat->cs, priv->mode);

	/* configure active state of CSX */
	fsl_dspi_cfg_cs_active_state(priv, slave_plat->cs,
				     priv->mode);

	fsl_dspi_clr_fifo(priv);

	/* check module TX and RX status */
	sr_val = dspi_read32(priv->flags, &priv->regs->sr);
	if ((sr_val & DSPI_SR_TXRXS) != DSPI_SR_TXRXS) {
		debug("DSPI RX/TX not ready!\n");
		return -EIO;
	}

	return 0;
}

static int fsl_dspi_release_bus(struct udevice *dev)
{
	struct udevice *bus = dev->parent;
	struct fsl_dspi_priv *priv = dev_get_priv(bus);
	struct dm_spi_slave_plat *slave_plat =
		dev_get_parent_plat(dev);

	/* halt module */
	dspi_halt(priv, 1);

	/* processor special release work */
	cpu_dspi_release_bus(dev_seq(bus), slave_plat->cs);

	return 0;
}

/**
 * This function doesn't do anything except help with debugging
 */
static int fsl_dspi_bind(struct udevice *bus)
{
	debug("%s assigned seq %d.\n", bus->name, dev_seq(bus));
	return 0;
}

static int fsl_dspi_of_to_plat(struct udevice *bus)
{
	fdt_addr_t addr;
	struct fsl_dspi_plat *plat = dev_get_plat(bus);
	const void *blob = gd->fdt_blob;
	int node = dev_of_offset(bus);

	if (fdtdec_get_bool(blob, node, "big-endian"))
		plat->flags |= DSPI_FLAG_REGMAP_ENDIAN_BIG;

	plat->num_chipselect = fdtdec_get_int(blob, node,
					      "spi-num-chipselects",
					      FSL_DSPI_MAX_CHIPSELECT);

	addr = dev_read_addr(bus);
	if (addr == FDT_ADDR_T_NONE) {
		debug("DSPI: Can't get base address or size\n");
		return -ENOMEM;
	}
	plat->regs_addr = addr;

	plat->speed_hz = fdtdec_get_int(blob,
			node, "spi-max-frequency", FSL_DSPI_DEFAULT_SCK_FREQ);

	debug("DSPI: regs=%pa, max-frequency=%d, endianness=%s, num-cs=%d\n",
	      &plat->regs_addr, plat->speed_hz,
	      plat->flags & DSPI_FLAG_REGMAP_ENDIAN_BIG ? "be" : "le",
	      plat->num_chipselect);

	return 0;
}

static int fsl_dspi_xfer(struct udevice *dev, unsigned int bitlen,
		const void *dout, void *din, unsigned long flags)
{
	struct fsl_dspi_priv *priv;
	struct dm_spi_slave_plat *slave_plat = dev_get_parent_plat(dev);
	struct udevice *bus;

	bus = dev->parent;
	priv = dev_get_priv(bus);

	return dspi_xfer(priv, slave_plat->cs, bitlen, dout, din, flags);
}

static int fsl_dspi_set_speed(struct udevice *bus, uint speed)
{
	struct fsl_dspi_priv *priv = dev_get_priv(bus);

	return fsl_dspi_cfg_speed(priv, speed);
}

static int fsl_dspi_set_mode(struct udevice *bus, uint mode)
{
	struct fsl_dspi_priv *priv = dev_get_priv(bus);

	debug("DSPI set_mode: mode 0x%x.\n", mode);

	/*
	 * We store some chipselect special configure value in priv->ctar_val,
	 * and we can't get the correct chipselect number here,
	 * so just store mode value.
	 * Do really configuration when claim_bus.
	 */
	priv->mode = mode;

	return 0;
}

static const struct dm_spi_ops fsl_dspi_ops = {
	.claim_bus	= fsl_dspi_claim_bus,
	.release_bus	= fsl_dspi_release_bus,
	.xfer		= fsl_dspi_xfer,
	.set_speed	= fsl_dspi_set_speed,
	.set_mode	= fsl_dspi_set_mode,
};

static const struct udevice_id fsl_dspi_ids[] = {
	{ .compatible = "fsl,vf610-dspi" },
	{ .compatible = "fsl,ls1021a-v1.0-dspi" },
	{ }
};

U_BOOT_DRIVER(fsl_dspi) = {
	.name	= "fsl_dspi",
	.id	= UCLASS_SPI,
	.of_match = fsl_dspi_ids,
	.ops	= &fsl_dspi_ops,
	.of_to_plat = fsl_dspi_of_to_plat,
	.plat_auto	= sizeof(struct fsl_dspi_plat),
	.priv_auto	= sizeof(struct fsl_dspi_priv),
	.probe	= fsl_dspi_probe,
	.child_pre_probe = fsl_dspi_child_pre_probe,
	.bind = fsl_dspi_bind,
};

// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright (C) 2016 Jagan Teki <jteki@openedev.com>
 *		      Christophe Ricard <christophe.ricard@gmail.com>
 *
 * Copyright (C) 2010 Dirk Behme <dirk.behme@googlemail.com>
 *
 * Driver for McSPI controller on OMAP3. Based on davinci_spi.c
 * Copyright (C) 2009 Texas Instruments Incorporated - https://www.ti.com/
 *
 * Copyright (C) 2007 Atmel Corporation
 *
 * Parts taken from linux/drivers/spi/omap2_mcspi.c
 * Copyright (C) 2005, 2006 Nokia Corporation
 *
 * Modified by Ruslan Araslanov <ruslan.araslanov@vitecmm.com>
 */

#include <common.h>
#include <dm.h>
#include <spi.h>
#include <malloc.h>
#include <asm/global_data.h>
#include <asm/io.h>
#include <linux/bitops.h>
#include <omap3_spi.h>

DECLARE_GLOBAL_DATA_PTR;

struct omap2_mcspi_platform_config {
	unsigned int regs_offset;
};

struct omap3_spi_priv {
	struct mcspi *regs;
	unsigned int cs;
	unsigned int freq;
	unsigned int mode;
	unsigned int wordlen;
	unsigned int pin_dir:1;

	bool bus_claimed;
};

static void omap3_spi_write_chconf(struct omap3_spi_priv *priv, int val)
{
	writel(val, &priv->regs->channel[priv->cs].chconf);
	/* Flash post writes to make immediate effect */
	readl(&priv->regs->channel[priv->cs].chconf);
}

static void omap3_spi_set_enable(struct omap3_spi_priv *priv, int enable)
{
	writel(enable, &priv->regs->channel[priv->cs].chctrl);
	/* Flash post writes to make immediate effect */
	readl(&priv->regs->channel[priv->cs].chctrl);
}

static int omap3_spi_write(struct omap3_spi_priv *priv, unsigned int len,
			   const void *txp, unsigned long flags)
{
	ulong start;
	int i, chconf;

	chconf = readl(&priv->regs->channel[priv->cs].chconf);

	/* Enable the channel */
	omap3_spi_set_enable(priv, OMAP3_MCSPI_CHCTRL_EN);

	chconf &= ~(OMAP3_MCSPI_CHCONF_TRM_MASK | OMAP3_MCSPI_CHCONF_WL_MASK);
	chconf |= (priv->wordlen - 1) << 7;
	chconf |= OMAP3_MCSPI_CHCONF_TRM_TX_ONLY;
	chconf |= OMAP3_MCSPI_CHCONF_FORCE;
	omap3_spi_write_chconf(priv, chconf);

	for (i = 0; i < len; i++) {
		/* wait till TX register is empty (TXS == 1) */
		start = get_timer(0);
		while (!(readl(&priv->regs->channel[priv->cs].chstat) &
			 OMAP3_MCSPI_CHSTAT_TXS)) {
			if (get_timer(start) > SPI_WAIT_TIMEOUT) {
				printf("SPI TXS timed out, status=0x%08x\n",
					readl(&priv->regs->channel[priv->cs].chstat));
				return -1;
			}
		}
		/* Write the data */
		unsigned int *tx = &priv->regs->channel[priv->cs].tx;
		if (priv->wordlen > 16)
			writel(((u32 *)txp)[i], tx);
		else if (priv->wordlen > 8)
			writel(((u16 *)txp)[i], tx);
		else
			writel(((u8 *)txp)[i], tx);
	}

	/* wait to finish of transfer */
	while ((readl(&priv->regs->channel[priv->cs].chstat) &
			(OMAP3_MCSPI_CHSTAT_EOT | OMAP3_MCSPI_CHSTAT_TXS)) !=
			(OMAP3_MCSPI_CHSTAT_EOT | OMAP3_MCSPI_CHSTAT_TXS))
		;

	/* Disable the channel otherwise the next immediate RX will get affected */
	omap3_spi_set_enable(priv, OMAP3_MCSPI_CHCTRL_DIS);

	if (flags & SPI_XFER_END) {

		chconf &= ~OMAP3_MCSPI_CHCONF_FORCE;
		omap3_spi_write_chconf(priv, chconf);
	}
	return 0;
}

static int omap3_spi_read(struct omap3_spi_priv *priv, unsigned int len,
			  void *rxp, unsigned long flags)
{
	int i, chconf;
	ulong start;

	chconf = readl(&priv->regs->channel[priv->cs].chconf);

	/* Enable the channel */
	omap3_spi_set_enable(priv, OMAP3_MCSPI_CHCTRL_EN);

	chconf &= ~(OMAP3_MCSPI_CHCONF_TRM_MASK | OMAP3_MCSPI_CHCONF_WL_MASK);
	chconf |= (priv->wordlen - 1) << 7;
	chconf |= OMAP3_MCSPI_CHCONF_TRM_RX_ONLY;
	chconf |= OMAP3_MCSPI_CHCONF_FORCE;
	omap3_spi_write_chconf(priv, chconf);

	writel(0, &priv->regs->channel[priv->cs].tx);

	for (i = 0; i < len; i++) {
		start = get_timer(0);
		/* Wait till RX register contains data (RXS == 1) */
		while (!(readl(&priv->regs->channel[priv->cs].chstat) &
			 OMAP3_MCSPI_CHSTAT_RXS)) {
			if (get_timer(start) > SPI_WAIT_TIMEOUT) {
				printf("SPI RXS timed out, status=0x%08x\n",
					readl(&priv->regs->channel[priv->cs].chstat));
				return -1;
			}
		}

		/* Disable the channel to prevent further receiving */
		if (i == (len - 1))
			omap3_spi_set_enable(priv, OMAP3_MCSPI_CHCTRL_DIS);

		/* Read the data */
		unsigned int *rx = &priv->regs->channel[priv->cs].rx;
		if (priv->wordlen > 16)
			((u32 *)rxp)[i] = readl(rx);
		else if (priv->wordlen > 8)
			((u16 *)rxp)[i] = (u16)readl(rx);
		else
			((u8 *)rxp)[i] = (u8)readl(rx);
	}

	if (flags & SPI_XFER_END) {
		chconf &= ~OMAP3_MCSPI_CHCONF_FORCE;
		omap3_spi_write_chconf(priv, chconf);
	}

	return 0;
}

/*McSPI Transmit Receive Mode*/
static int omap3_spi_txrx(struct omap3_spi_priv *priv, unsigned int len,
			  const void *txp, void *rxp, unsigned long flags)
{
	ulong start;
	int chconf, i = 0;

	chconf = readl(&priv->regs->channel[priv->cs].chconf);

	/*Enable SPI channel*/
	omap3_spi_set_enable(priv, OMAP3_MCSPI_CHCTRL_EN);

	/*set TRANSMIT-RECEIVE Mode*/
	chconf &= ~(OMAP3_MCSPI_CHCONF_TRM_MASK | OMAP3_MCSPI_CHCONF_WL_MASK);
	chconf |= (priv->wordlen - 1) << 7;
	chconf |= OMAP3_MCSPI_CHCONF_FORCE;
	omap3_spi_write_chconf(priv, chconf);

	/*Shift in and out 1 byte at time*/
	for (i=0; i < len; i++){
		/* Write: wait for TX empty (TXS == 1)*/
		start = get_timer(0);
		while (!(readl(&priv->regs->channel[priv->cs].chstat) &
			 OMAP3_MCSPI_CHSTAT_TXS)) {
			if (get_timer(start) > SPI_WAIT_TIMEOUT) {
				printf("SPI TXS timed out, status=0x%08x\n",
					readl(&priv->regs->channel[priv->cs].chstat));
				return -1;
			}
		}
		/* Write the data */
		unsigned int *tx = &priv->regs->channel[priv->cs].tx;
		if (priv->wordlen > 16)
			writel(((u32 *)txp)[i], tx);
		else if (priv->wordlen > 8)
			writel(((u16 *)txp)[i], tx);
		else
			writel(((u8 *)txp)[i], tx);

		/*Read: wait for RX containing data (RXS == 1)*/
		start = get_timer(0);
		while (!(readl(&priv->regs->channel[priv->cs].chstat) &
			 OMAP3_MCSPI_CHSTAT_RXS)) {
			if (get_timer(start) > SPI_WAIT_TIMEOUT) {
				printf("SPI RXS timed out, status=0x%08x\n",
					readl(&priv->regs->channel[priv->cs].chstat));
				return -1;
			}
		}
		/* Read the data */
		unsigned int *rx = &priv->regs->channel[priv->cs].rx;
		if (priv->wordlen > 16)
			((u32 *)rxp)[i] = readl(rx);
		else if (priv->wordlen > 8)
			((u16 *)rxp)[i] = (u16)readl(rx);
		else
			((u8 *)rxp)[i] = (u8)readl(rx);
	}
	/* Disable the channel */
	omap3_spi_set_enable(priv, OMAP3_MCSPI_CHCTRL_DIS);

	/*if transfer must be terminated disable the channel*/
	if (flags & SPI_XFER_END) {
		chconf &= ~OMAP3_MCSPI_CHCONF_FORCE;
		omap3_spi_write_chconf(priv, chconf);
	}

	return 0;
}

static int _spi_xfer(struct omap3_spi_priv *priv, unsigned int bitlen,
		     const void *dout, void *din, unsigned long flags)
{
	unsigned int	len;
	int ret = -1;

	if (priv->wordlen < 4 || priv->wordlen > 32) {
		printf("omap3_spi: invalid wordlen %d\n", priv->wordlen);
		return -1;
	}

	if (bitlen % priv->wordlen)
		return -1;

	len = bitlen / priv->wordlen;

	if (bitlen == 0) {	 /* only change CS */
		int chconf = readl(&priv->regs->channel[priv->cs].chconf);

		if (flags & SPI_XFER_BEGIN) {
			omap3_spi_set_enable(priv, OMAP3_MCSPI_CHCTRL_EN);
			chconf |= OMAP3_MCSPI_CHCONF_FORCE;
			omap3_spi_write_chconf(priv, chconf);
		}
		if (flags & SPI_XFER_END) {
			chconf &= ~OMAP3_MCSPI_CHCONF_FORCE;
			omap3_spi_write_chconf(priv, chconf);
			omap3_spi_set_enable(priv, OMAP3_MCSPI_CHCTRL_DIS);
		}
		ret = 0;
	} else {
		if (dout != NULL && din != NULL)
			ret = omap3_spi_txrx(priv, len, dout, din, flags);
		else if (dout != NULL)
			ret = omap3_spi_write(priv, len, dout, flags);
		else if (din != NULL)
			ret = omap3_spi_read(priv, len, din, flags);
	}
	return ret;
}

static void _omap3_spi_set_speed(struct omap3_spi_priv *priv)
{
	uint32_t confr, div = 0;

	confr = readl(&priv->regs->channel[priv->cs].chconf);

	/* Calculate clock divisor. Valid range: 0x0 - 0xC ( /1 - /4096 ) */
	if (priv->freq) {
		while (div <= 0xC && (OMAP3_MCSPI_MAX_FREQ / (1 << div))
					> priv->freq)
			div++;
	} else {
		 div = 0xC;
	}

	/* set clock divisor */
	confr &= ~OMAP3_MCSPI_CHCONF_CLKD_MASK;
	confr |= div << 2;

	omap3_spi_write_chconf(priv, confr);
}

static void _omap3_spi_set_mode(struct omap3_spi_priv *priv)
{
	uint32_t confr;

	confr = readl(&priv->regs->channel[priv->cs].chconf);

	/* standard 4-wire master mode:  SCK, MOSI/out, MISO/in, nCS
	 * REVISIT: this controller could support SPI_3WIRE mode.
	 */
	if (priv->pin_dir == MCSPI_PINDIR_D0_IN_D1_OUT) {
		confr &= ~(OMAP3_MCSPI_CHCONF_IS|OMAP3_MCSPI_CHCONF_DPE1);
		confr |= OMAP3_MCSPI_CHCONF_DPE0;
	} else {
		confr &= ~OMAP3_MCSPI_CHCONF_DPE0;
		confr |= OMAP3_MCSPI_CHCONF_IS|OMAP3_MCSPI_CHCONF_DPE1;
	}

	/* set SPI mode 0..3 */
	confr &= ~(OMAP3_MCSPI_CHCONF_POL | OMAP3_MCSPI_CHCONF_PHA);
	if (priv->mode & SPI_CPHA)
		confr |= OMAP3_MCSPI_CHCONF_PHA;
	if (priv->mode & SPI_CPOL)
		confr |= OMAP3_MCSPI_CHCONF_POL;

	/* set chipselect polarity; manage with FORCE */
	if (!(priv->mode & SPI_CS_HIGH))
		confr |= OMAP3_MCSPI_CHCONF_EPOL; /* active-low; normal */
	else
		confr &= ~OMAP3_MCSPI_CHCONF_EPOL;

	/* Transmit & receive mode */
	confr &= ~OMAP3_MCSPI_CHCONF_TRM_MASK;

	omap3_spi_write_chconf(priv, confr);
}

static void _omap3_spi_set_wordlen(struct omap3_spi_priv *priv)
{
	unsigned int confr;

	/* McSPI individual channel configuration */
	confr = readl(&priv->regs->channel[priv->cs].chconf);

	/* wordlength */
	confr &= ~OMAP3_MCSPI_CHCONF_WL_MASK;
	confr |= (priv->wordlen - 1) << 7;

	omap3_spi_write_chconf(priv, confr);
}

static void spi_reset(struct omap3_spi_priv *priv)
{
	unsigned int tmp;

	writel(OMAP3_MCSPI_SYSCONFIG_SOFTRESET, &priv->regs->sysconfig);
	do {
		tmp = readl(&priv->regs->sysstatus);
	} while (!(tmp & OMAP3_MCSPI_SYSSTATUS_RESETDONE));

	writel(OMAP3_MCSPI_SYSCONFIG_AUTOIDLE |
	       OMAP3_MCSPI_SYSCONFIG_ENAWAKEUP |
	       OMAP3_MCSPI_SYSCONFIG_SMARTIDLE, &priv->regs->sysconfig);

	writel(OMAP3_MCSPI_WAKEUPENABLE_WKEN, &priv->regs->wakeupenable);

	/*
	 * Set the same default mode for each channel, especially CS polarity
	 * which must be common for all SPI slaves before any transfer.
	 */
	for (priv->cs = 0 ; priv->cs < OMAP4_MCSPI_CHAN_NB ; priv->cs++)
		_omap3_spi_set_mode(priv);
	priv->cs = 0;
}

static void _omap3_spi_claim_bus(struct omap3_spi_priv *priv)
{
	unsigned int conf;
	/*
	 * setup when switching from (reset default) slave mode
	 * to single-channel master mode
	 */
	conf = readl(&priv->regs->modulctrl);
	conf &= ~(OMAP3_MCSPI_MODULCTRL_STEST | OMAP3_MCSPI_MODULCTRL_MS);
	conf |= OMAP3_MCSPI_MODULCTRL_SINGLE;

	writel(conf, &priv->regs->modulctrl);

	priv->bus_claimed = true;
}

static int omap3_spi_claim_bus(struct udevice *dev)
{
	struct udevice *bus = dev->parent;
	struct omap3_spi_priv *priv = dev_get_priv(bus);
	struct dm_spi_slave_plat *slave_plat = dev_get_parent_plat(dev);

	priv->cs = slave_plat->cs;
	if (!priv->freq)
		priv->freq = slave_plat->max_hz;

	_omap3_spi_claim_bus(priv);
	_omap3_spi_set_speed(priv);
	_omap3_spi_set_mode(priv);

	return 0;
}

static int omap3_spi_release_bus(struct udevice *dev)
{
	struct udevice *bus = dev->parent;
	struct omap3_spi_priv *priv = dev_get_priv(bus);

	writel(OMAP3_MCSPI_MODULCTRL_MS, &priv->regs->modulctrl);

	priv->bus_claimed = false;

	return 0;
}

static int omap3_spi_set_wordlen(struct udevice *dev, unsigned int wordlen)
{
	struct udevice *bus = dev->parent;
	struct omap3_spi_priv *priv = dev_get_priv(bus);
	struct dm_spi_slave_plat *slave_plat = dev_get_parent_plat(dev);

	priv->cs = slave_plat->cs;
	priv->wordlen = wordlen;
	_omap3_spi_set_wordlen(priv);

	return 0;
}

static int omap3_spi_probe(struct udevice *dev)
{
	struct omap3_spi_priv *priv = dev_get_priv(dev);
	struct omap3_spi_plat *plat = dev_get_plat(dev);

	priv->regs = plat->regs;
	priv->pin_dir = plat->pin_dir;
	priv->wordlen = SPI_DEFAULT_WORDLEN;

	spi_reset(priv);

	return 0;
}

static int omap3_spi_xfer(struct udevice *dev, unsigned int bitlen,
			    const void *dout, void *din, unsigned long flags)
{
	struct udevice *bus = dev->parent;
	struct omap3_spi_priv *priv = dev_get_priv(bus);

	return _spi_xfer(priv, bitlen, dout, din, flags);
}

static int omap3_spi_set_speed(struct udevice *dev, unsigned int speed)
{

	struct omap3_spi_priv *priv = dev_get_priv(dev);

	priv->freq = speed;
	if (priv->bus_claimed)
		_omap3_spi_set_speed(priv);

	return 0;
}

static int omap3_spi_set_mode(struct udevice *dev, uint mode)
{
	struct omap3_spi_priv *priv = dev_get_priv(dev);

	priv->mode = mode;

	if (priv->bus_claimed)
		_omap3_spi_set_mode(priv);

	return 0;
}

static const struct dm_spi_ops omap3_spi_ops = {
	.claim_bus      = omap3_spi_claim_bus,
	.release_bus    = omap3_spi_release_bus,
	.set_wordlen    = omap3_spi_set_wordlen,
	.xfer	    = omap3_spi_xfer,
	.set_speed      = omap3_spi_set_speed,
	.set_mode	= omap3_spi_set_mode,
	/*
	 * cs_info is not needed, since we require all chip selects to be
	 * in the device tree explicitly
	 */
};

#if CONFIG_IS_ENABLED(OF_REAL)
static struct omap2_mcspi_platform_config omap2_pdata = {
	.regs_offset = 0,
};

static struct omap2_mcspi_platform_config omap4_pdata = {
	.regs_offset = OMAP4_MCSPI_REG_OFFSET,
};

static int omap3_spi_of_to_plat(struct udevice *dev)
{
	struct omap2_mcspi_platform_config *data =
		(struct omap2_mcspi_platform_config *)dev_get_driver_data(dev);
	struct omap3_spi_plat *plat = dev_get_plat(dev);

	plat->regs = (struct mcspi *)(dev_read_addr(dev) + data->regs_offset);

	if (dev_read_bool(dev, "ti,pindir-d0-out-d1-in"))
		plat->pin_dir = MCSPI_PINDIR_D0_OUT_D1_IN;
	else
		plat->pin_dir = MCSPI_PINDIR_D0_IN_D1_OUT;

	return 0;
}

static const struct udevice_id omap3_spi_ids[] = {
	{ .compatible = "ti,omap2-mcspi", .data = (ulong)&omap2_pdata },
	{ .compatible = "ti,omap4-mcspi", .data = (ulong)&omap4_pdata },
	{ }
};
#endif
U_BOOT_DRIVER(omap3_spi) = {
	.name   = "omap3_spi",
	.id     = UCLASS_SPI,
	.flags	= DM_FLAG_PRE_RELOC,
#if CONFIG_IS_ENABLED(OF_REAL)
	.of_match = omap3_spi_ids,
	.of_to_plat = omap3_spi_of_to_plat,
	.plat_auto	= sizeof(struct omap3_spi_plat),
#endif
	.probe = omap3_spi_probe,
	.ops    = &omap3_spi_ops,
	.priv_auto	= sizeof(struct omap3_spi_priv),
};

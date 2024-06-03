// SPDX-License-Identifier: GPL-2.0+
/*
 * spi driver for rockchip
 *
 * (C) 2019 Theobroma Systems Design und Consulting GmbH
 *
 * (C) Copyright 2015 Google, Inc
 *
 * (C) Copyright 2008-2013 Rockchip Electronics
 * Peter, Software Engineering, <superpeter.cai@gmail.com>.
 */

#include <common.h>
#include <clk.h>
#include <dm.h>
#include <dt-structs.h>
#include <errno.h>
#include <spi.h>
#include <linux/errno.h>
#include <asm/io.h>
#include <asm/arch-rockchip/clock.h>
#include <asm/arch-rockchip/periph.h>
#include <dm/pinctrl.h>
#include "rk_spi.h"

/* Change to 1 to output registers at the start of each transaction */
#define DEBUG_RK_SPI	0

struct rockchip_spi_params {
	/* RXFIFO overruns and TXFIFO underruns stop the master clock */
	bool master_manages_fifo;
};

struct rockchip_spi_platdata {
#if CONFIG_IS_ENABLED(OF_PLATDATA)
	struct dtd_rockchip_rk3288_spi of_plat;
#endif
	s32 frequency;		/* Default clock frequency, -1 for none */
	fdt_addr_t base;
	uint deactivate_delay_us;	/* Delay to wait after deactivate */
	uint activate_delay_us;		/* Delay to wait after activate */
};

struct rockchip_spi_priv {
	struct rockchip_spi *regs;
	struct clk clk;
	unsigned int max_freq;
	unsigned int mode;
	ulong last_transaction_us;	/* Time of last transaction end */
	unsigned int speed_hz;
	unsigned int last_speed_hz;
	uint input_rate;
};

#define SPI_FIFO_DEPTH		32

static void rkspi_dump_regs(struct rockchip_spi *regs)
{
	debug("ctrl0: \t\t0x%08x\n", readl(&regs->ctrlr0));
	debug("ctrl1: \t\t0x%08x\n", readl(&regs->ctrlr1));
	debug("ssienr: \t\t0x%08x\n", readl(&regs->enr));
	debug("ser: \t\t0x%08x\n", readl(&regs->ser));
	debug("baudr: \t\t0x%08x\n", readl(&regs->baudr));
	debug("txftlr: \t\t0x%08x\n", readl(&regs->txftlr));
	debug("rxftlr: \t\t0x%08x\n", readl(&regs->rxftlr));
	debug("txflr: \t\t0x%08x\n", readl(&regs->txflr));
	debug("rxflr: \t\t0x%08x\n", readl(&regs->rxflr));
	debug("sr: \t\t0x%08x\n", readl(&regs->sr));
	debug("imr: \t\t0x%08x\n", readl(&regs->imr));
	debug("isr: \t\t0x%08x\n", readl(&regs->isr));
	debug("dmacr: \t\t0x%08x\n", readl(&regs->dmacr));
	debug("dmatdlr: \t0x%08x\n", readl(&regs->dmatdlr));
	debug("dmardlr: \t0x%08x\n", readl(&regs->dmardlr));
}

static void rkspi_enable_chip(struct rockchip_spi *regs, bool enable)
{
	writel(enable ? 1 : 0, &regs->enr);
}

static void rkspi_set_clk(struct rockchip_spi_priv *priv, uint speed)
{
	/*
	 * We should try not to exceed the speed requested by the caller:
	 * when selecting a divider, we need to make sure we round up.
	 */
	uint clk_div = DIV_ROUND_UP(priv->input_rate, speed);

	/* The baudrate register (BAUDR) is defined as a 32bit register where
	 * the upper 16bit are reserved and having 'Fsclk_out' in the lower
	 * 16bits with 'Fsclk_out' defined as follows:
	 *
	 *   Fsclk_out = Fspi_clk/ SCKDV
	 *   Where SCKDV is any even value between 2 and 65534.
	 */
	if (clk_div > 0xfffe) {
		clk_div = 0xfffe;
		debug("%s: can't divide down to %d Hz (actual will be %d Hz)\n",
		      __func__, speed, priv->input_rate / clk_div);
	}

	/* Round up to the next even 16bit number */
	clk_div = (clk_div + 1) & 0xfffe;

	debug("spi speed %u, div %u\n", speed, clk_div);

	clrsetbits_le32(&priv->regs->baudr, 0xffff, clk_div);
	priv->last_speed_hz = speed;
}

static int rkspi_wait_till_not_busy(struct rockchip_spi *regs)
{
	unsigned long start;

	start = get_timer(0);
	while (readl(&regs->sr) & SR_BUSY) {
		if (get_timer(start) > ROCKCHIP_SPI_TIMEOUT_MS) {
			debug("RK SPI: Status keeps busy for 1000us after a read/write!\n");
			return -ETIMEDOUT;
		}
	}

	return 0;
}

static void spi_cs_activate(struct udevice *dev, uint cs)
{
	struct udevice *bus = dev->parent;
	struct rockchip_spi_platdata *plat = bus->platdata;
	struct rockchip_spi_priv *priv = dev_get_priv(bus);
	struct rockchip_spi *regs = priv->regs;

	/* If it's too soon to do another transaction, wait */
	if (plat->deactivate_delay_us && priv->last_transaction_us) {
		ulong delay_us;		/* The delay completed so far */
		delay_us = timer_get_us() - priv->last_transaction_us;
		if (delay_us < plat->deactivate_delay_us) {
			ulong additional_delay_us =
				plat->deactivate_delay_us - delay_us;
			debug("%s: delaying by %ld us\n",
			      __func__, additional_delay_us);
			udelay(additional_delay_us);
		}
	}

	debug("activate cs%u\n", cs);
	writel(1 << cs, &regs->ser);
	if (plat->activate_delay_us)
		udelay(plat->activate_delay_us);
}

static void spi_cs_deactivate(struct udevice *dev, uint cs)
{
	struct udevice *bus = dev->parent;
	struct rockchip_spi_platdata *plat = bus->platdata;
	struct rockchip_spi_priv *priv = dev_get_priv(bus);
	struct rockchip_spi *regs = priv->regs;

	debug("deactivate cs%u\n", cs);
	writel(0, &regs->ser);

	/* Remember time of this transaction so we can honour the bus delay */
	if (plat->deactivate_delay_us)
		priv->last_transaction_us = timer_get_us();
}

#if CONFIG_IS_ENABLED(OF_PLATDATA)
static int conv_of_platdata(struct udevice *dev)
{
	struct rockchip_spi_platdata *plat = dev->platdata;
	struct dtd_rockchip_rk3288_spi *dtplat = &plat->of_plat;
	struct rockchip_spi_priv *priv = dev_get_priv(dev);
	int ret;

	plat->base = dtplat->reg[0];
	plat->frequency = 20000000;
	ret = clk_get_by_index_platdata(dev, 0, dtplat->clocks, &priv->clk);
	if (ret < 0)
		return ret;
	dev->req_seq = 0;

	return 0;
}
#endif

static int rockchip_spi_ofdata_to_platdata(struct udevice *bus)
{
#if !CONFIG_IS_ENABLED(OF_PLATDATA)
	struct rockchip_spi_platdata *plat = dev_get_platdata(bus);
	struct rockchip_spi_priv *priv = dev_get_priv(bus);
	int ret;

	plat->base = dev_read_addr(bus);

	ret = clk_get_by_index(bus, 0, &priv->clk);
	if (ret < 0) {
		debug("%s: Could not get clock for %s: %d\n", __func__,
		      bus->name, ret);
		return ret;
	}

	plat->frequency =
		dev_read_u32_default(bus, "spi-max-frequency", 50000000);
	plat->deactivate_delay_us =
		dev_read_u32_default(bus, "spi-deactivate-delay", 0);
	plat->activate_delay_us =
		dev_read_u32_default(bus, "spi-activate-delay", 0);

	debug("%s: base=%x, max-frequency=%d, deactivate_delay=%d\n",
	      __func__, (uint)plat->base, plat->frequency,
	      plat->deactivate_delay_us);
#endif

	return 0;
}

static int rockchip_spi_calc_modclk(ulong max_freq)
{
	/*
	 * While this is not strictly correct for the RK3368, as the
	 * GPLL will be 576MHz, things will still work, as the
	 * clk_set_rate(...) implementation in our clock-driver will
	 * chose the next closest rate not exceeding what we request
	 * based on the output of this function.
	 */

	unsigned div;
	const unsigned long gpll_hz = 594000000UL;

	/*
	 * We need to find an input clock that provides at least twice
	 * the maximum frequency and can be generated from the assumed
	 * speed of GPLL (594MHz) using an integer divider.
	 *
	 * To give us more achievable bitrates at higher speeds (these
	 * are generated by dividing by an even 16-bit integer from
	 * this frequency), we try to have an input frequency of at
	 * least 4x our max_freq.
	 */

	div = DIV_ROUND_UP(gpll_hz, max_freq * 4);
	return gpll_hz / div;
}

static int rockchip_spi_probe(struct udevice *bus)
{
	struct rockchip_spi_platdata *plat = dev_get_platdata(bus);
	struct rockchip_spi_priv *priv = dev_get_priv(bus);
	int ret;

	debug("%s: probe\n", __func__);
#if CONFIG_IS_ENABLED(OF_PLATDATA)
	ret = conv_of_platdata(bus);
	if (ret)
		return ret;
#endif
	priv->regs = (struct rockchip_spi *)plat->base;

	priv->last_transaction_us = timer_get_us();
	priv->max_freq = plat->frequency;

	/* Clamp the value from the DTS against any hardware limits */
	if (priv->max_freq > ROCKCHIP_SPI_MAX_RATE)
		priv->max_freq = ROCKCHIP_SPI_MAX_RATE;

	/* Find a module-input clock that fits with the max_freq setting */
	ret = clk_set_rate(&priv->clk,
			   rockchip_spi_calc_modclk(priv->max_freq));
	if (ret < 0) {
		debug("%s: Failed to set clock: %d\n", __func__, ret);
		return ret;
	}
	priv->input_rate = ret;
	debug("%s: rate = %u\n", __func__, priv->input_rate);

	return 0;
}

static int rockchip_spi_claim_bus(struct udevice *dev)
{
	struct udevice *bus = dev->parent;
	struct rockchip_spi_priv *priv = dev_get_priv(bus);
	struct rockchip_spi *regs = priv->regs;
	uint ctrlr0;

	/* Disable the SPI hardware */
	rkspi_enable_chip(regs, false);

	if (priv->speed_hz != priv->last_speed_hz)
		rkspi_set_clk(priv, priv->speed_hz);

	/* Operation Mode */
	ctrlr0 = OMOD_MASTER << OMOD_SHIFT;

	/* Data Frame Size */
	ctrlr0 |= DFS_8BIT << DFS_SHIFT;

	/* set SPI mode 0..3 */
	if (priv->mode & SPI_CPOL)
		ctrlr0 |= SCOL_HIGH << SCOL_SHIFT;
	if (priv->mode & SPI_CPHA)
		ctrlr0 |= SCPH_TOGSTA << SCPH_SHIFT;

	/* Chip Select Mode */
	ctrlr0 |= CSM_KEEP << CSM_SHIFT;

	/* SSN to Sclk_out delay */
	ctrlr0 |= SSN_DELAY_ONE << SSN_DELAY_SHIFT;

	/* Serial Endian Mode */
	ctrlr0 |= SEM_LITTLE << SEM_SHIFT;

	/* First Bit Mode */
	ctrlr0 |= FBM_MSB << FBM_SHIFT;

	/* Byte and Halfword Transform */
	ctrlr0 |= HALF_WORD_OFF << HALF_WORD_TX_SHIFT;

	/* Rxd Sample Delay */
	ctrlr0 |= 0 << RXDSD_SHIFT;

	/* Frame Format */
	ctrlr0 |= FRF_SPI << FRF_SHIFT;

	/* Tx and Rx mode */
	ctrlr0 |= TMOD_TR << TMOD_SHIFT;

	writel(ctrlr0, &regs->ctrlr0);

	return 0;
}

static int rockchip_spi_release_bus(struct udevice *dev)
{
	struct udevice *bus = dev->parent;
	struct rockchip_spi_priv *priv = dev_get_priv(bus);

	rkspi_enable_chip(priv->regs, false);

	return 0;
}

static inline int rockchip_spi_16bit_reader(struct udevice *dev,
					    u8 **din, int *len)
{
	struct udevice *bus = dev->parent;
	const struct rockchip_spi_params * const data =
		(void *)dev_get_driver_data(bus);
	struct rockchip_spi_priv *priv = dev_get_priv(bus);
	struct rockchip_spi *regs = priv->regs;
	const u32 saved_ctrlr0 = readl(&regs->ctrlr0);
#if defined(DEBUG)
	u32 statistics_rxlevels[33] = { };
#endif
	u32 frames = *len / 2;
	u8 *in = (u8 *)(*din);
	u32 max_chunk_size = SPI_FIFO_DEPTH;

	if (!frames)
		return 0;

	/*
	 * If we know that the hardware will manage RXFIFO overruns
	 * (i.e. stop the SPI clock until there's space in the FIFO),
	 * we the allow largest possible chunk size that can be
	 * represented in CTRLR1.
	 */
	if (data && data->master_manages_fifo)
		max_chunk_size = 0x10000;

	// rockchip_spi_configure(dev, mode, size)
	rkspi_enable_chip(regs, false);
	clrsetbits_le32(&regs->ctrlr0,
			TMOD_MASK << TMOD_SHIFT,
			TMOD_RO << TMOD_SHIFT);
	/* 16bit data frame size */
	clrsetbits_le32(&regs->ctrlr0, DFS_MASK, DFS_16BIT);

	/* Update caller's context */
	const u32 bytes_to_process = 2 * frames;
	*din += bytes_to_process;
	*len -= bytes_to_process;

	/* Process our frames */
	while (frames) {
		u32 chunk_size = min(frames, max_chunk_size);

		frames -= chunk_size;

		writew(chunk_size - 1, &regs->ctrlr1);
		rkspi_enable_chip(regs, true);

		do {
			u32 rx_level = readw(&regs->rxflr);
#if defined(DEBUG)
			statistics_rxlevels[rx_level]++;
#endif
			chunk_size -= rx_level;
			while (rx_level--) {
				u16 val = readw(regs->rxdr);
				*in++ = val & 0xff;
				*in++ = val >> 8;
			}
		} while (chunk_size);

		rkspi_enable_chip(regs, false);
	}

#if defined(DEBUG)
	debug("%s: observed rx_level during processing:\n", __func__);
	for (int i = 0; i <= 32; ++i)
		if (statistics_rxlevels[i])
			debug("\t%2d: %d\n", i, statistics_rxlevels[i]);
#endif
	/* Restore the original transfer setup and return error-free. */
	writel(saved_ctrlr0, &regs->ctrlr0);
	return 0;
}

static int rockchip_spi_xfer(struct udevice *dev, unsigned int bitlen,
			   const void *dout, void *din, unsigned long flags)
{
	struct udevice *bus = dev->parent;
	struct rockchip_spi_priv *priv = dev_get_priv(bus);
	struct rockchip_spi *regs = priv->regs;
	struct dm_spi_slave_platdata *slave_plat = dev_get_parent_platdata(dev);
	int len = bitlen >> 3;
	const u8 *out = dout;
	u8 *in = din;
	int toread, towrite;
	int ret = 0;

	debug("%s: dout=%p, din=%p, len=%x, flags=%lx\n", __func__, dout, din,
	      len, flags);
	if (DEBUG_RK_SPI)
		rkspi_dump_regs(regs);

	/* Assert CS before transfer */
	if (flags & SPI_XFER_BEGIN)
		spi_cs_activate(dev, slave_plat->cs);

	/*
	 * To ensure fast loading of firmware images (e.g. full U-Boot
	 * stage, ATF, Linux kernel) from SPI flash, we optimise the
	 * case of read-only transfers by using the full 16bits of each
	 * FIFO element.
	 */
	if (!out)
		ret = rockchip_spi_16bit_reader(dev, &in, &len);

	/* This is the original 8bit reader/writer code */
	while (len > 0) {
		int todo = min(len, 0x10000);

		rkspi_enable_chip(regs, false);
		writel(todo - 1, &regs->ctrlr1);
		rkspi_enable_chip(regs, true);

		toread = todo;
		towrite = todo;
		while (toread || towrite) {
			u32 status = readl(&regs->sr);

			if (towrite && !(status & SR_TF_FULL)) {
				writel(out ? *out++ : 0, regs->txdr);
				towrite--;
			}
			if (toread && !(status & SR_RF_EMPT)) {
				u32 byte = readl(regs->rxdr);

				if (in)
					*in++ = byte;
				toread--;
			}
		}

		/*
		 * In case that there's a transmit-component, we need to wait
		 * until the control goes idle before we can disable the SPI
		 * control logic (as this will implictly flush the FIFOs).
		 */
		if (out) {
			ret = rkspi_wait_till_not_busy(regs);
			if (ret)
				break;
		}

		len -= todo;
	}

	/* Deassert CS after transfer */
	if (flags & SPI_XFER_END)
		spi_cs_deactivate(dev, slave_plat->cs);

	rkspi_enable_chip(regs, false);

	return ret;
}

static int rockchip_spi_set_speed(struct udevice *bus, uint speed)
{
	struct rockchip_spi_priv *priv = dev_get_priv(bus);

	/* Clamp to the maximum frequency specified in the DTS */
	if (speed > priv->max_freq)
		speed = priv->max_freq;

	priv->speed_hz = speed;

	return 0;
}

static int rockchip_spi_set_mode(struct udevice *bus, uint mode)
{
	struct rockchip_spi_priv *priv = dev_get_priv(bus);

	priv->mode = mode;

	return 0;
}

static const struct dm_spi_ops rockchip_spi_ops = {
	.claim_bus	= rockchip_spi_claim_bus,
	.release_bus	= rockchip_spi_release_bus,
	.xfer		= rockchip_spi_xfer,
	.set_speed	= rockchip_spi_set_speed,
	.set_mode	= rockchip_spi_set_mode,
	/*
	 * cs_info is not needed, since we require all chip selects to be
	 * in the device tree explicitly
	 */
};

const  struct rockchip_spi_params rk3399_spi_params = {
	.master_manages_fifo = true,
};

static const struct udevice_id rockchip_spi_ids[] = {
	{ .compatible = "rockchip,rk3288-spi" },
	{ .compatible = "rockchip,rk3368-spi",
	  .data = (ulong)&rk3399_spi_params },
	{ .compatible = "rockchip,rk3399-spi",
	  .data = (ulong)&rk3399_spi_params },
	{ }
};

U_BOOT_DRIVER(rockchip_spi) = {
#if CONFIG_IS_ENABLED(OF_PLATDATA)
	.name	= "rockchip_rk3288_spi",
#else
	.name	= "rockchip_spi",
#endif
	.id	= UCLASS_SPI,
	.of_match = rockchip_spi_ids,
	.ops	= &rockchip_spi_ops,
	.ofdata_to_platdata = rockchip_spi_ofdata_to_platdata,
	.platdata_auto_alloc_size = sizeof(struct rockchip_spi_platdata),
	.priv_auto_alloc_size = sizeof(struct rockchip_spi_priv),
	.probe	= rockchip_spi_probe,
};

// SPDX-License-Identifier: GPL-2.0+
/*
 * (C) Copyright 2012
 * Armando Visconti, ST Microelectronics, armando.visconti@st.com.
 *
 * (C) Copyright 2018
 * Quentin Schulz, Bootlin, quentin.schulz@bootlin.com
 *
 * Driver for ARM PL022 SPI Controller.
 */

#include <clk.h>
#include <common.h>
#include <dm.h>
#include <dm/platform_data/spi_pl022.h>
#include <linux/io.h>
#include <spi.h>

#define SSP_CR0		0x000
#define SSP_CR1		0x004
#define SSP_DR		0x008
#define SSP_SR		0x00C
#define SSP_CPSR	0x010
#define SSP_IMSC	0x014
#define SSP_RIS		0x018
#define SSP_MIS		0x01C
#define SSP_ICR		0x020
#define SSP_DMACR	0x024
#define SSP_CSR		0x030 /* vendor extension */
#define SSP_ITCR	0x080
#define SSP_ITIP	0x084
#define SSP_ITOP	0x088
#define SSP_TDR		0x08C

#define SSP_PID0	0xFE0
#define SSP_PID1	0xFE4
#define SSP_PID2	0xFE8
#define SSP_PID3	0xFEC

#define SSP_CID0	0xFF0
#define SSP_CID1	0xFF4
#define SSP_CID2	0xFF8
#define SSP_CID3	0xFFC

/* SSP Control Register 0  - SSP_CR0 */
#define SSP_CR0_SPO		(0x1 << 6)
#define SSP_CR0_SPH		(0x1 << 7)
#define SSP_CR0_BIT_MODE(x)	((x) - 1)
#define SSP_SCR_MIN		(0x00)
#define SSP_SCR_MAX		(0xFF)
#define SSP_SCR_SHFT		8
#define DFLT_CLKRATE		2

/* SSP Control Register 1  - SSP_CR1 */
#define SSP_CR1_MASK_SSE	(0x1 << 1)

#define SSP_CPSR_MIN		(0x02)
#define SSP_CPSR_MAX		(0xFE)
#define DFLT_PRESCALE		(0x40)

/* SSP Status Register - SSP_SR */
#define SSP_SR_MASK_TFE		(0x1 << 0) /* Transmit FIFO empty */
#define SSP_SR_MASK_TNF		(0x1 << 1) /* Transmit FIFO not full */
#define SSP_SR_MASK_RNE		(0x1 << 2) /* Receive FIFO not empty */
#define SSP_SR_MASK_RFF		(0x1 << 3) /* Receive FIFO full */
#define SSP_SR_MASK_BSY		(0x1 << 4) /* Busy Flag */

struct pl022_spi_slave {
	void *base;
	unsigned int freq;
};

/*
 * ARM PL022 exists in different 'flavors'.
 * This drivers currently support the standard variant (0x00041022), that has a
 * 16bit wide and 8 locations deep TX/RX FIFO.
 */
static int pl022_is_supported(struct pl022_spi_slave *ps)
{
	/* PL022 version is 0x00041022 */
	if ((readw(ps->base + SSP_PID0) == 0x22) &&
	    (readw(ps->base + SSP_PID1) == 0x10) &&
	    ((readw(ps->base + SSP_PID2) & 0xf) == 0x04) &&
	    (readw(ps->base + SSP_PID3) == 0x00))
		return 1;

	return 0;
}

static int pl022_spi_probe(struct udevice *bus)
{
	struct pl022_spi_pdata *plat = dev_get_platdata(bus);
	struct pl022_spi_slave *ps = dev_get_priv(bus);

	ps->base = ioremap(plat->addr, plat->size);
	ps->freq = plat->freq;

	/* Check the PL022 version */
	if (!pl022_is_supported(ps))
		return -ENOTSUPP;

	/* 8 bits per word, high polarity and default clock rate */
	writew(SSP_CR0_BIT_MODE(8), ps->base + SSP_CR0);
	writew(DFLT_PRESCALE, ps->base + SSP_CPSR);

	return 0;
}

static void flush(struct pl022_spi_slave *ps)
{
	do {
		while (readw(ps->base + SSP_SR) & SSP_SR_MASK_RNE)
			readw(ps->base + SSP_DR);
	} while (readw(ps->base + SSP_SR) & SSP_SR_MASK_BSY);
}

static int pl022_spi_claim_bus(struct udevice *dev)
{
	struct udevice *bus = dev->parent;
	struct pl022_spi_slave *ps = dev_get_priv(bus);
	u16 reg;

	/* Enable the SPI hardware */
	reg = readw(ps->base + SSP_CR1);
	reg |= SSP_CR1_MASK_SSE;
	writew(reg, ps->base + SSP_CR1);

	flush(ps);

	return 0;
}

static int pl022_spi_release_bus(struct udevice *dev)
{
	struct udevice *bus = dev->parent;
	struct pl022_spi_slave *ps = dev_get_priv(bus);
	u16 reg;

	flush(ps);

	/* Disable the SPI hardware */
	reg = readw(ps->base + SSP_CR1);
	reg &= ~SSP_CR1_MASK_SSE;
	writew(reg, ps->base + SSP_CR1);

	return 0;
}

static int pl022_spi_xfer(struct udevice *dev, unsigned int bitlen,
			  const void *dout, void *din, unsigned long flags)
{
	struct udevice *bus = dev->parent;
	struct pl022_spi_slave *ps = dev_get_priv(bus);
	u32		len_tx = 0, len_rx = 0, len;
	u32		ret = 0;
	const u8	*txp = dout;
	u8		*rxp = din, value;

	if (bitlen == 0)
		/* Finish any previously submitted transfers */
		return 0;

	/*
	 * TODO: The controller can do non-multiple-of-8 bit
	 * transfers, but this driver currently doesn't support it.
	 *
	 * It's also not clear how such transfers are supposed to be
	 * represented as a stream of bytes...this is a limitation of
	 * the current SPI interface.
	 */
	if (bitlen % 8) {
		/* Errors always terminate an ongoing transfer */
		flags |= SPI_XFER_END;
		return -1;
	}

	len = bitlen / 8;

	while (len_tx < len) {
		if (readw(ps->base + SSP_SR) & SSP_SR_MASK_TNF) {
			value = txp ? *txp++ : 0;
			writew(value, ps->base + SSP_DR);
			len_tx++;
		}

		if (readw(ps->base + SSP_SR) & SSP_SR_MASK_RNE) {
			value = readw(ps->base + SSP_DR);
			if (rxp)
				*rxp++ = value;
			len_rx++;
		}
	}

	while (len_rx < len_tx) {
		if (readw(ps->base + SSP_SR) & SSP_SR_MASK_RNE) {
			value = readw(ps->base + SSP_DR);
			if (rxp)
				*rxp++ = value;
			len_rx++;
		}
	}

	return ret;
}

static inline u32 spi_rate(u32 rate, u16 cpsdvsr, u16 scr)
{
	return rate / (cpsdvsr * (1 + scr));
}

static int pl022_spi_set_speed(struct udevice *bus, uint speed)
{
	struct pl022_spi_slave *ps = dev_get_priv(bus);
	u16 scr = SSP_SCR_MIN, cr0 = 0, cpsr = SSP_CPSR_MIN, best_scr = scr,
	    best_cpsr = cpsr;
	u32 min, max, best_freq = 0, tmp;
	u32 rate = ps->freq;
	bool found = false;

	max = spi_rate(rate, SSP_CPSR_MIN, SSP_SCR_MIN);
	min = spi_rate(rate, SSP_CPSR_MAX, SSP_SCR_MAX);

	if (speed > max || speed < min) {
		pr_err("Tried to set speed to %dHz but min=%d and max=%d\n",
		       speed, min, max);
		return -EINVAL;
	}

	while (cpsr <= SSP_CPSR_MAX && !found) {
		while (scr <= SSP_SCR_MAX) {
			tmp = spi_rate(rate, cpsr, scr);

			if (abs(speed - tmp) < abs(speed - best_freq)) {
				best_freq = tmp;
				best_cpsr = cpsr;
				best_scr = scr;

				if (tmp == speed) {
					found = true;
					break;
				}
			}

			scr++;
		}
		cpsr += 2;
		scr = SSP_SCR_MIN;
	}

	writew(best_cpsr, ps->base + SSP_CPSR);
	cr0 = readw(ps->base + SSP_CR0);
	writew(cr0 | (best_scr << SSP_SCR_SHFT), ps->base + SSP_CR0);

	return 0;
}

static int pl022_spi_set_mode(struct udevice *bus, uint mode)
{
	struct pl022_spi_slave *ps = dev_get_priv(bus);
	u16 reg;

	reg = readw(ps->base + SSP_CR0);
	reg &= ~(SSP_CR0_SPH | SSP_CR0_SPO);
	if (mode & SPI_CPHA)
		reg |= SSP_CR0_SPH;
	if (mode & SPI_CPOL)
		reg |= SSP_CR0_SPO;
	writew(reg, ps->base + SSP_CR0);

	return 0;
}

static int pl022_cs_info(struct udevice *bus, uint cs,
			 struct spi_cs_info *info)
{
	return 0;
}

static const struct dm_spi_ops pl022_spi_ops = {
	.claim_bus      = pl022_spi_claim_bus,
	.release_bus    = pl022_spi_release_bus,
	.xfer           = pl022_spi_xfer,
	.set_speed      = pl022_spi_set_speed,
	.set_mode       = pl022_spi_set_mode,
	.cs_info        = pl022_cs_info,
};

#if !CONFIG_IS_ENABLED(OF_PLATDATA)
static int pl022_spi_ofdata_to_platdata(struct udevice *bus)
{
	struct pl022_spi_pdata *plat = bus->platdata;
	const void *fdt = gd->fdt_blob;
	int node = dev_of_offset(bus);
	struct clk clkdev;
	int ret;

	plat->addr = fdtdec_get_addr_size(fdt, node, "reg", &plat->size);

	ret = clk_get_by_index(bus, 0, &clkdev);
	if (ret)
		return ret;

	plat->freq = clk_get_rate(&clkdev);

	return 0;
}

static const struct udevice_id pl022_spi_ids[] = {
	{ .compatible = "arm,pl022-spi" },
	{ }
};
#endif

U_BOOT_DRIVER(pl022_spi) = {
	.name   = "pl022_spi",
	.id     = UCLASS_SPI,
#if !CONFIG_IS_ENABLED(OF_PLATDATA)
	.of_match = pl022_spi_ids,
	.ofdata_to_platdata = pl022_spi_ofdata_to_platdata,
#endif
	.ops    = &pl022_spi_ops,
	.platdata_auto_alloc_size = sizeof(struct pl022_spi_pdata),
	.priv_auto_alloc_size = sizeof(struct pl022_spi_slave),
	.probe  = pl022_spi_probe,
};

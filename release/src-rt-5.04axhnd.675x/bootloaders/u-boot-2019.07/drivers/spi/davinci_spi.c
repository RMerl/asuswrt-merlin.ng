// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright (C) 2009 Texas Instruments Incorporated - http://www.ti.com/
 *
 * Driver for SPI controller on DaVinci. Based on atmel_spi.c
 * by Atmel Corporation
 *
 * Copyright (C) 2007 Atmel Corporation
 */

#include <common.h>
#include <spi.h>
#include <malloc.h>
#include <asm/io.h>
#include <asm/arch/hardware.h>
#include <dm.h>
#include <dm/platform_data/spi_davinci.h>

/* SPIGCR0 */
#define SPIGCR0_SPIENA_MASK	0x1
#define SPIGCR0_SPIRST_MASK	0x0

/* SPIGCR0 */
#define SPIGCR1_CLKMOD_MASK	BIT(1)
#define SPIGCR1_MASTER_MASK	BIT(0)
#define SPIGCR1_SPIENA_MASK	BIT(24)

/* SPIPC0 */
#define SPIPC0_DIFUN_MASK	BIT(11)		/* SIMO */
#define SPIPC0_DOFUN_MASK	BIT(10)		/* SOMI */
#define SPIPC0_CLKFUN_MASK	BIT(9)		/* CLK */
#define SPIPC0_EN0FUN_MASK	BIT(0)

/* SPIFMT0 */
#define SPIFMT_SHIFTDIR_SHIFT	20
#define SPIFMT_POLARITY_SHIFT	17
#define SPIFMT_PHASE_SHIFT	16
#define SPIFMT_PRESCALE_SHIFT	8

/* SPIDAT1 */
#define SPIDAT1_CSHOLD_SHIFT	28
#define SPIDAT1_CSNR_SHIFT	16

/* SPIDELAY */
#define SPI_C2TDELAY_SHIFT	24
#define SPI_T2CDELAY_SHIFT	16

/* SPIBUF */
#define SPIBUF_RXEMPTY_MASK	BIT(31)
#define SPIBUF_TXFULL_MASK	BIT(29)

/* SPIDEF */
#define SPIDEF_CSDEF0_MASK	BIT(0)

#ifndef CONFIG_DM_SPI
#define SPI0_BUS		0
#define SPI0_BASE		CONFIG_SYS_SPI_BASE
/*
 * Define default SPI0_NUM_CS as 1 for existing platforms that uses this
 * driver. Platform can configure number of CS using CONFIG_SYS_SPI0_NUM_CS
 * if more than one CS is supported and by defining CONFIG_SYS_SPI0.
 */
#ifndef CONFIG_SYS_SPI0
#define SPI0_NUM_CS		1
#else
#define SPI0_NUM_CS		CONFIG_SYS_SPI0_NUM_CS
#endif

/*
 * define CONFIG_SYS_SPI1 when platform has spi-1 device (bus #1) and
 * CONFIG_SYS_SPI1_NUM_CS defines number of CS on this bus
 */
#ifdef CONFIG_SYS_SPI1
#define SPI1_BUS		1
#define SPI1_NUM_CS		CONFIG_SYS_SPI1_NUM_CS
#define SPI1_BASE		CONFIG_SYS_SPI1_BASE
#endif

/*
 * define CONFIG_SYS_SPI2 when platform has spi-2 device (bus #2) and
 * CONFIG_SYS_SPI2_NUM_CS defines number of CS on this bus
 */
#ifdef CONFIG_SYS_SPI2
#define SPI2_BUS		2
#define SPI2_NUM_CS		CONFIG_SYS_SPI2_NUM_CS
#define SPI2_BASE		CONFIG_SYS_SPI2_BASE
#endif
#endif

DECLARE_GLOBAL_DATA_PTR;

/* davinci spi register set */
struct davinci_spi_regs {
	dv_reg	gcr0;		/* 0x00 */
	dv_reg	gcr1;		/* 0x04 */
	dv_reg	int0;		/* 0x08 */
	dv_reg	lvl;		/* 0x0c */
	dv_reg	flg;		/* 0x10 */
	dv_reg	pc0;		/* 0x14 */
	dv_reg	pc1;		/* 0x18 */
	dv_reg	pc2;		/* 0x1c */
	dv_reg	pc3;		/* 0x20 */
	dv_reg	pc4;		/* 0x24 */
	dv_reg	pc5;		/* 0x28 */
	dv_reg	rsvd[3];
	dv_reg	dat0;		/* 0x38 */
	dv_reg	dat1;		/* 0x3c */
	dv_reg	buf;		/* 0x40 */
	dv_reg	emu;		/* 0x44 */
	dv_reg	delay;		/* 0x48 */
	dv_reg	def;		/* 0x4c */
	dv_reg	fmt0;		/* 0x50 */
	dv_reg	fmt1;		/* 0x54 */
	dv_reg	fmt2;		/* 0x58 */
	dv_reg	fmt3;		/* 0x5c */
	dv_reg	intvec0;	/* 0x60 */
	dv_reg	intvec1;	/* 0x64 */
};

/* davinci spi slave */
struct davinci_spi_slave {
#ifndef CONFIG_DM_SPI
	struct spi_slave slave;
#endif
	struct davinci_spi_regs *regs;
	unsigned int freq; /* current SPI bus frequency */
	unsigned int mode; /* current SPI mode used */
	u8 num_cs;	   /* total no. of CS available */
	u8 cur_cs;	   /* CS of current slave */
	bool half_duplex;  /* true, if master is half-duplex only */
};

/*
 * This functions needs to act like a macro to avoid pipeline reloads in the
 * loops below. Use always_inline. This gains us about 160KiB/s and the bloat
 * appears to be zero bytes (da830).
 */
__attribute__((always_inline))
static inline u32 davinci_spi_xfer_data(struct davinci_spi_slave *ds, u32 data)
{
	u32	buf_reg_val;

	/* send out data */
	writel(data, &ds->regs->dat1);

	/* wait for the data to clock in/out */
	while ((buf_reg_val = readl(&ds->regs->buf)) & SPIBUF_RXEMPTY_MASK)
		;

	return buf_reg_val;
}

static int davinci_spi_read(struct davinci_spi_slave *ds, unsigned int len,
			    u8 *rxp, unsigned long flags)
{
	unsigned int data1_reg_val;

	/* enable CS hold, CS[n] and clear the data bits */
	data1_reg_val = ((1 << SPIDAT1_CSHOLD_SHIFT) |
			 (ds->cur_cs << SPIDAT1_CSNR_SHIFT));

	/* wait till TXFULL is deasserted */
	while (readl(&ds->regs->buf) & SPIBUF_TXFULL_MASK)
		;

	/* preload the TX buffer to avoid clock starvation */
	writel(data1_reg_val, &ds->regs->dat1);

	/* keep reading 1 byte until only 1 byte left */
	while ((len--) > 1)
		*rxp++ = davinci_spi_xfer_data(ds, data1_reg_val);

	/* clear CS hold when we reach the end */
	if (flags & SPI_XFER_END)
		data1_reg_val &= ~(1 << SPIDAT1_CSHOLD_SHIFT);

	/* read the last byte */
	*rxp = davinci_spi_xfer_data(ds, data1_reg_val);

	return 0;
}

static int davinci_spi_write(struct davinci_spi_slave *ds, unsigned int len,
			     const u8 *txp, unsigned long flags)
{
	unsigned int data1_reg_val;

	/* enable CS hold and clear the data bits */
	data1_reg_val = ((1 << SPIDAT1_CSHOLD_SHIFT) |
			 (ds->cur_cs << SPIDAT1_CSNR_SHIFT));

	/* wait till TXFULL is deasserted */
	while (readl(&ds->regs->buf) & SPIBUF_TXFULL_MASK)
		;

	/* preload the TX buffer to avoid clock starvation */
	if (len > 2) {
		writel(data1_reg_val | *txp++, &ds->regs->dat1);
		len--;
	}

	/* keep writing 1 byte until only 1 byte left */
	while ((len--) > 1)
		davinci_spi_xfer_data(ds, data1_reg_val | *txp++);

	/* clear CS hold when we reach the end */
	if (flags & SPI_XFER_END)
		data1_reg_val &= ~(1 << SPIDAT1_CSHOLD_SHIFT);

	/* write the last byte */
	davinci_spi_xfer_data(ds, data1_reg_val | *txp);

	return 0;
}

static int davinci_spi_read_write(struct davinci_spi_slave *ds, unsigned
				  int len, u8 *rxp, const u8 *txp,
				  unsigned long flags)
{
	unsigned int data1_reg_val;

	/* enable CS hold and clear the data bits */
	data1_reg_val = ((1 << SPIDAT1_CSHOLD_SHIFT) |
			 (ds->cur_cs << SPIDAT1_CSNR_SHIFT));

	/* wait till TXFULL is deasserted */
	while (readl(&ds->regs->buf) & SPIBUF_TXFULL_MASK)
		;

	/* keep reading and writing 1 byte until only 1 byte left */
	while ((len--) > 1)
		*rxp++ = davinci_spi_xfer_data(ds, data1_reg_val | *txp++);

	/* clear CS hold when we reach the end */
	if (flags & SPI_XFER_END)
		data1_reg_val &= ~(1 << SPIDAT1_CSHOLD_SHIFT);

	/* read and write the last byte */
	*rxp = davinci_spi_xfer_data(ds, data1_reg_val | *txp);

	return 0;
}


static int __davinci_spi_claim_bus(struct davinci_spi_slave *ds, int cs)
{
	unsigned int mode = 0, scalar;

	/* Enable the SPI hardware */
	writel(SPIGCR0_SPIRST_MASK, &ds->regs->gcr0);
	udelay(1000);
	writel(SPIGCR0_SPIENA_MASK, &ds->regs->gcr0);

	/* Set master mode, powered up and not activated */
	writel(SPIGCR1_MASTER_MASK | SPIGCR1_CLKMOD_MASK, &ds->regs->gcr1);

	/* CS, CLK, SIMO and SOMI are functional pins */
	writel(((1 << cs) | SPIPC0_CLKFUN_MASK |
		SPIPC0_DOFUN_MASK | SPIPC0_DIFUN_MASK), &ds->regs->pc0);

	/* setup format */
	scalar = ((CONFIG_SYS_SPI_CLK / ds->freq) - 1) & 0xFF;

	/*
	 * Use following format:
	 *   character length = 8,
	 *   MSB shifted out first
	 */
	if (ds->mode & SPI_CPOL)
		mode |= SPI_CPOL;
	if (!(ds->mode & SPI_CPHA))
		mode |= SPI_CPHA;
	writel(8 | (scalar << SPIFMT_PRESCALE_SHIFT) |
		(mode << SPIFMT_PHASE_SHIFT), &ds->regs->fmt0);

	/*
	 * Including a minor delay. No science here. Should be good even with
	 * no delay
	 */
	writel((50 << SPI_C2TDELAY_SHIFT) |
		(50 << SPI_T2CDELAY_SHIFT), &ds->regs->delay);

	/* default chip select register */
	writel(SPIDEF_CSDEF0_MASK, &ds->regs->def);

	/* no interrupts */
	writel(0, &ds->regs->int0);
	writel(0, &ds->regs->lvl);

	/* enable SPI */
	writel((readl(&ds->regs->gcr1) | SPIGCR1_SPIENA_MASK), &ds->regs->gcr1);

	return 0;
}

static int __davinci_spi_release_bus(struct davinci_spi_slave *ds)
{
	/* Disable the SPI hardware */
	writel(SPIGCR0_SPIRST_MASK, &ds->regs->gcr0);

	return 0;
}

static int __davinci_spi_xfer(struct davinci_spi_slave *ds,
		unsigned int bitlen,  const void *dout, void *din,
		unsigned long flags)
{
	unsigned int len;

	if (bitlen == 0)
		/* Finish any previously submitted transfers */
		goto out;

	/*
	 * It's not clear how non-8-bit-aligned transfers are supposed to be
	 * represented as a stream of bytes...this is a limitation of
	 * the current SPI interface - here we terminate on receiving such a
	 * transfer request.
	 */
	if (bitlen % 8) {
		/* Errors always terminate an ongoing transfer */
		flags |= SPI_XFER_END;
		goto out;
	}

	len = bitlen / 8;

	if (!dout)
		return davinci_spi_read(ds, len, din, flags);
	if (!din)
		return davinci_spi_write(ds, len, dout, flags);
	if (!ds->half_duplex)
		return davinci_spi_read_write(ds, len, din, dout, flags);

	printf("SPI full duplex not supported\n");
	flags |= SPI_XFER_END;

out:
	if (flags & SPI_XFER_END) {
		u8 dummy = 0;
		davinci_spi_write(ds, 1, &dummy, flags);
	}
	return 0;
}

#ifndef CONFIG_DM_SPI

static inline struct davinci_spi_slave *to_davinci_spi(struct spi_slave *slave)
{
	return container_of(slave, struct davinci_spi_slave, slave);
}

int spi_cs_is_valid(unsigned int bus, unsigned int cs)
{
	int ret = 0;

	switch (bus) {
	case SPI0_BUS:
		if (cs < SPI0_NUM_CS)
			ret = 1;
		break;
#ifdef CONFIG_SYS_SPI1
	case SPI1_BUS:
		if (cs < SPI1_NUM_CS)
			ret = 1;
		break;
#endif
#ifdef CONFIG_SYS_SPI2
	case SPI2_BUS:
		if (cs < SPI2_NUM_CS)
			ret = 1;
		break;
#endif
	default:
		/* Invalid bus number. Do nothing */
		break;
	}
	return ret;
}

void spi_cs_activate(struct spi_slave *slave)
{
	/* do nothing */
}

void spi_cs_deactivate(struct spi_slave *slave)
{
	/* do nothing */
}

struct spi_slave *spi_setup_slave(unsigned int bus, unsigned int cs,
			unsigned int max_hz, unsigned int mode)
{
	struct davinci_spi_slave	*ds;

	if (!spi_cs_is_valid(bus, cs))
		return NULL;

	ds = spi_alloc_slave(struct davinci_spi_slave, bus, cs);
	if (!ds)
		return NULL;

	switch (bus) {
	case SPI0_BUS:
		ds->regs = (struct davinci_spi_regs *)SPI0_BASE;
		break;
#ifdef CONFIG_SYS_SPI1
	case SPI1_BUS:
		ds->regs = (struct davinci_spi_regs *)SPI1_BASE;
		break;
#endif
#ifdef CONFIG_SYS_SPI2
	case SPI2_BUS:
		ds->regs = (struct davinci_spi_regs *)SPI2_BASE;
		break;
#endif
	default: /* Invalid bus number */
		return NULL;
	}

	ds->freq = max_hz;
	ds->mode = mode;

	return &ds->slave;
}

void spi_free_slave(struct spi_slave *slave)
{
	struct davinci_spi_slave *ds = to_davinci_spi(slave);

	free(ds);
}

int spi_xfer(struct spi_slave *slave, unsigned int bitlen,
	     const void *dout, void *din, unsigned long flags)
{
	struct davinci_spi_slave *ds = to_davinci_spi(slave);

	ds->cur_cs = slave->cs;

	return __davinci_spi_xfer(ds, bitlen, dout, din, flags);
}

int spi_claim_bus(struct spi_slave *slave)
{
	struct davinci_spi_slave *ds = to_davinci_spi(slave);

#ifdef CONFIG_SPI_HALF_DUPLEX
	ds->half_duplex = true;
#else
	ds->half_duplex = false;
#endif
	return __davinci_spi_claim_bus(ds, ds->slave.cs);
}

void spi_release_bus(struct spi_slave *slave)
{
	struct davinci_spi_slave *ds = to_davinci_spi(slave);

	__davinci_spi_release_bus(ds);
}

#else
static int davinci_spi_set_speed(struct udevice *bus, uint max_hz)
{
	struct davinci_spi_slave *ds = dev_get_priv(bus);

	debug("%s speed %u\n", __func__, max_hz);
	if (max_hz > CONFIG_SYS_SPI_CLK / 2)
		return -EINVAL;

	ds->freq = max_hz;

	return 0;
}

static int davinci_spi_set_mode(struct udevice *bus, uint mode)
{
	struct davinci_spi_slave *ds = dev_get_priv(bus);

	debug("%s mode %u\n", __func__, mode);
	ds->mode = mode;

	return 0;
}

static int davinci_spi_claim_bus(struct udevice *dev)
{
	struct dm_spi_slave_platdata *slave_plat =
		dev_get_parent_platdata(dev);
	struct udevice *bus = dev->parent;
	struct davinci_spi_slave *ds = dev_get_priv(bus);

	if (slave_plat->cs >= ds->num_cs) {
		printf("Invalid SPI chipselect\n");
		return -EINVAL;
	}
	ds->half_duplex = slave_plat->mode & SPI_PREAMBLE;

	return __davinci_spi_claim_bus(ds, slave_plat->cs);
}

static int davinci_spi_release_bus(struct udevice *dev)
{
	struct davinci_spi_slave *ds = dev_get_priv(dev->parent);

	return __davinci_spi_release_bus(ds);
}

static int davinci_spi_xfer(struct udevice *dev, unsigned int bitlen,
			    const void *dout, void *din,
			    unsigned long flags)
{
	struct dm_spi_slave_platdata *slave =
		dev_get_parent_platdata(dev);
	struct udevice *bus = dev->parent;
	struct davinci_spi_slave *ds = dev_get_priv(bus);

	if (slave->cs >= ds->num_cs) {
		printf("Invalid SPI chipselect\n");
		return -EINVAL;
	}
	ds->cur_cs = slave->cs;

	return __davinci_spi_xfer(ds, bitlen, dout, din, flags);
}

static const struct dm_spi_ops davinci_spi_ops = {
	.claim_bus	= davinci_spi_claim_bus,
	.release_bus	= davinci_spi_release_bus,
	.xfer		= davinci_spi_xfer,
	.set_speed	= davinci_spi_set_speed,
	.set_mode	= davinci_spi_set_mode,
};

static int davinci_spi_probe(struct udevice *bus)
{
	struct davinci_spi_slave *ds = dev_get_priv(bus);
	struct davinci_spi_platdata *plat = bus->platdata;
	ds->regs = plat->regs;
	ds->num_cs = plat->num_cs;

	return 0;
}

#if CONFIG_IS_ENABLED(OF_CONTROL) && !CONFIG_IS_ENABLED(OF_PLATDATA)
static int davinci_ofdata_to_platadata(struct udevice *bus)
{
	struct davinci_spi_platdata *plat = bus->platdata;
	fdt_addr_t addr;

	addr = devfdt_get_addr(bus);
	if (addr == FDT_ADDR_T_NONE)
		return -EINVAL;

	plat->regs = (struct davinci_spi_regs *)addr;
	plat->num_cs = fdtdec_get_int(gd->fdt_blob, dev_of_offset(bus), "num-cs", 4);

	return 0;
}

static const struct udevice_id davinci_spi_ids[] = {
	{ .compatible = "ti,keystone-spi" },
	{ .compatible = "ti,dm6441-spi" },
	{ .compatible = "ti,da830-spi" },
	{ }
};
#endif

U_BOOT_DRIVER(davinci_spi) = {
	.name = "davinci_spi",
	.id = UCLASS_SPI,
#if CONFIG_IS_ENABLED(OF_CONTROL) && !CONFIG_IS_ENABLED(OF_PLATDATA)
	.of_match = davinci_spi_ids,
	.ofdata_to_platdata = davinci_ofdata_to_platadata,
        .platdata_auto_alloc_size = sizeof(struct davinci_spi_platdata),
#endif
	.probe = davinci_spi_probe,
	.ops = &davinci_spi_ops,
	.priv_auto_alloc_size = sizeof(struct davinci_spi_slave),
};
#endif

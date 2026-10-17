// SPDX-License-Identifier: GPL-2.0+
/*
 * Microchip PIC32 SPI controller driver.
 *
 * Copyright (c) 2015, Microchip Technology Inc.
 *      Purna Chandra Mandal <purna.mandal@microchip.com>
 */

#include <common.h>
#include <clk.h>
#include <dm.h>
#include <log.h>
#include <asm/global_data.h>
#include <linux/bitops.h>
#include <linux/compat.h>
#include <malloc.h>
#include <spi.h>

#include <asm/types.h>
#include <asm/io.h>
#include <asm/gpio.h>
#include <dt-bindings/clock/microchip,clock.h>
#include <mach/pic32.h>

DECLARE_GLOBAL_DATA_PTR;

/* PIC32 SPI controller registers */
struct pic32_reg_spi {
	struct pic32_reg_atomic ctrl;
	struct pic32_reg_atomic status;
	struct pic32_reg_atomic buf;
	struct pic32_reg_atomic baud;
	struct pic32_reg_atomic ctrl2;
};

/* Bit fields in SPI Control Register */
#define PIC32_SPI_CTRL_MSTEN	BIT(5) /* Enable SPI Master */
#define PIC32_SPI_CTRL_CKP	BIT(6) /* active low */
#define PIC32_SPI_CTRL_CKE	BIT(8) /* Tx on falling edge */
#define PIC32_SPI_CTRL_SMP	BIT(9) /* Rx at middle or end of tx */
#define PIC32_SPI_CTRL_BPW_MASK	0x03   /* Bits per word */
#define  PIC32_SPI_CTRL_BPW_8		0x0
#define  PIC32_SPI_CTRL_BPW_16		0x1
#define  PIC32_SPI_CTRL_BPW_32		0x2
#define PIC32_SPI_CTRL_BPW_SHIFT	10
#define PIC32_SPI_CTRL_ON	BIT(15) /* Macro enable */
#define PIC32_SPI_CTRL_ENHBUF	BIT(16) /* Enable enhanced buffering */
#define PIC32_SPI_CTRL_MCLKSEL	BIT(23) /* Select SPI Clock src */
#define PIC32_SPI_CTRL_MSSEN	BIT(28) /* SPI macro will drive SS */
#define PIC32_SPI_CTRL_FRMEN	BIT(31) /* Enable framing mode */

/* Bit fields in SPI Status Register */
#define PIC32_SPI_STAT_RX_OV		BIT(6) /* err, s/w needs to clear */
#define PIC32_SPI_STAT_TF_LVL_MASK	0x1f
#define PIC32_SPI_STAT_TF_LVL_SHIFT	16
#define PIC32_SPI_STAT_RF_LVL_MASK	0x1f
#define PIC32_SPI_STAT_RF_LVL_SHIFT	24

/* Bit fields in SPI Baud Register */
#define PIC32_SPI_BAUD_MASK	0x1ff

struct pic32_spi_priv {
	struct pic32_reg_spi	*regs;
	u32			fifo_depth; /* FIFO depth in bytes */
	u32			fifo_n_word; /* FIFO depth in words */
	struct gpio_desc	cs_gpio;

	/* Current SPI slave specific */
	ulong			clk_rate;
	u32			speed_hz; /* spi-clk rate */
	int			mode;

	/* Current message/transfer state */
	const void		*tx;
	const void		*tx_end;
	const void		*rx;
	const void		*rx_end;
	u32			len;

	/* SPI FiFo accessor */
	void (*rx_fifo)(struct pic32_spi_priv *);
	void (*tx_fifo)(struct pic32_spi_priv *);
};

static inline void pic32_spi_enable(struct pic32_spi_priv *priv)
{
	writel(PIC32_SPI_CTRL_ON, &priv->regs->ctrl.set);
}

static inline void pic32_spi_disable(struct pic32_spi_priv *priv)
{
	writel(PIC32_SPI_CTRL_ON, &priv->regs->ctrl.clr);
}

static inline u32 pic32_spi_rx_fifo_level(struct pic32_spi_priv *priv)
{
	u32 sr = readl(&priv->regs->status.raw);

	return (sr >> PIC32_SPI_STAT_RF_LVL_SHIFT) & PIC32_SPI_STAT_RF_LVL_MASK;
}

static inline u32 pic32_spi_tx_fifo_level(struct pic32_spi_priv *priv)
{
	u32 sr = readl(&priv->regs->status.raw);

	return (sr >> PIC32_SPI_STAT_TF_LVL_SHIFT) & PIC32_SPI_STAT_TF_LVL_MASK;
}

/* Return the max entries we can fill into tx fifo */
static u32 pic32_tx_max(struct pic32_spi_priv *priv, int n_bytes)
{
	u32 tx_left, tx_room, rxtx_gap;

	tx_left = (priv->tx_end - priv->tx) / n_bytes;
	tx_room = priv->fifo_n_word - pic32_spi_tx_fifo_level(priv);

	rxtx_gap = (priv->rx_end - priv->rx) - (priv->tx_end - priv->tx);
	rxtx_gap /= n_bytes;
	return min3(tx_left, tx_room, (u32)(priv->fifo_n_word - rxtx_gap));
}

/* Return the max entries we should read out of rx fifo */
static u32 pic32_rx_max(struct pic32_spi_priv *priv, int n_bytes)
{
	u32 rx_left = (priv->rx_end - priv->rx) / n_bytes;

	return min_t(u32, rx_left, pic32_spi_rx_fifo_level(priv));
}

#define BUILD_SPI_FIFO_RW(__name, __type, __bwl)		\
static void pic32_spi_rx_##__name(struct pic32_spi_priv *priv)	\
{								\
	__type val;						\
	u32 mx = pic32_rx_max(priv, sizeof(__type));		\
								\
	for (; mx; mx--) {					\
		val = read##__bwl(&priv->regs->buf.raw);	\
		if (priv->rx_end - priv->len)			\
			*(__type *)(priv->rx) = val;		\
		priv->rx += sizeof(__type);			\
	}							\
}								\
								\
static void pic32_spi_tx_##__name(struct pic32_spi_priv *priv)	\
{								\
	__type val;						\
	u32 mx = pic32_tx_max(priv, sizeof(__type));		\
								\
	for (; mx ; mx--) {					\
		val = (__type) ~0U;				\
		if (priv->tx_end - priv->len)			\
			val =  *(__type *)(priv->tx);		\
		write##__bwl(val, &priv->regs->buf.raw);	\
		priv->tx += sizeof(__type);			\
	}							\
}
BUILD_SPI_FIFO_RW(byte, u8, b);
BUILD_SPI_FIFO_RW(word, u16, w);
BUILD_SPI_FIFO_RW(dword, u32, l);

static int pic32_spi_set_word_size(struct pic32_spi_priv *priv,
				   unsigned int wordlen)
{
	u32 bits_per_word;
	u32 val;

	switch (wordlen) {
	case 8:
		priv->rx_fifo = pic32_spi_rx_byte;
		priv->tx_fifo = pic32_spi_tx_byte;
		bits_per_word = PIC32_SPI_CTRL_BPW_8;
		break;
	case 16:
		priv->rx_fifo = pic32_spi_rx_word;
		priv->tx_fifo = pic32_spi_tx_word;
		bits_per_word = PIC32_SPI_CTRL_BPW_16;
		break;
	case 32:
		priv->rx_fifo = pic32_spi_rx_dword;
		priv->tx_fifo = pic32_spi_tx_dword;
		bits_per_word = PIC32_SPI_CTRL_BPW_32;
		break;
	default:
		printf("pic32-spi: unsupported wordlen\n");
		return -EINVAL;
	}

	/* set bits-per-word */
	val = readl(&priv->regs->ctrl.raw);
	val &= ~(PIC32_SPI_CTRL_BPW_MASK << PIC32_SPI_CTRL_BPW_SHIFT);
	val |= bits_per_word << PIC32_SPI_CTRL_BPW_SHIFT;
	writel(val, &priv->regs->ctrl.raw);

	/* calculate maximum number of words fifo can hold */
	priv->fifo_n_word = DIV_ROUND_UP(priv->fifo_depth, wordlen / 8);

	return 0;
}

static int pic32_spi_claim_bus(struct udevice *slave)
{
	struct pic32_spi_priv *priv = dev_get_priv(slave->parent);

	/* enable chip */
	pic32_spi_enable(priv);

	return 0;
}

static int pic32_spi_release_bus(struct udevice *slave)
{
	struct pic32_spi_priv *priv = dev_get_priv(slave->parent);

	/* disable chip */
	pic32_spi_disable(priv);

	return 0;
}

static void spi_cs_activate(struct pic32_spi_priv *priv)
{
	if (!dm_gpio_is_valid(&priv->cs_gpio))
		return;

	dm_gpio_set_value(&priv->cs_gpio, 1);
}

static void spi_cs_deactivate(struct pic32_spi_priv *priv)
{
	if (!dm_gpio_is_valid(&priv->cs_gpio))
		return;

	dm_gpio_set_value(&priv->cs_gpio, 0);
}

static int pic32_spi_xfer(struct udevice *slave, unsigned int bitlen,
			  const void *tx_buf, void *rx_buf,
			  unsigned long flags)
{
	struct dm_spi_slave_plat *slave_plat;
	struct udevice *bus = slave->parent;
	struct pic32_spi_priv *priv;
	int len = bitlen / 8;
	int ret = 0;
	ulong tbase;

	priv = dev_get_priv(bus);
	slave_plat = dev_get_parent_plat(slave);

	debug("spi_xfer: bus:%i cs:%i flags:%lx\n",
	      dev_seq(bus), slave_plat->cs, flags);
	debug("msg tx %p, rx %p submitted of %d byte(s)\n",
	      tx_buf, rx_buf, len);

	/* assert cs */
	if (flags & SPI_XFER_BEGIN)
		spi_cs_activate(priv);

	/* set current transfer information */
	priv->tx = tx_buf;
	priv->rx = rx_buf;
	priv->tx_end = priv->tx + len;
	priv->rx_end = priv->rx + len;
	priv->len = len;

	/* transact by polling */
	tbase = get_timer(0);
	for (;;) {
		priv->tx_fifo(priv);
		priv->rx_fifo(priv);

		/* received sufficient data */
		if (priv->rx >= priv->rx_end) {
			ret = 0;
			break;
		}

		if (get_timer(tbase) > 5 * CONFIG_SYS_HZ) {
			printf("pic32_spi: error, xfer timedout.\n");
			flags |= SPI_XFER_END;
			ret = -ETIMEDOUT;
			break;
		}
	}

	/* deassert cs */
	if (flags & SPI_XFER_END)
		spi_cs_deactivate(priv);

	return ret;
}

static int pic32_spi_set_speed(struct udevice *bus, uint speed)
{
	struct pic32_spi_priv *priv = dev_get_priv(bus);
	u32 div;

	debug("%s: %s, speed %u\n", __func__, bus->name, speed);

	/* div = [clk_in / (2 * spi_clk)] - 1 */
	div = (priv->clk_rate / 2 / speed) - 1;
	div &= PIC32_SPI_BAUD_MASK;
	writel(div, &priv->regs->baud.raw);

	priv->speed_hz = speed;

	return 0;
}

static int pic32_spi_set_mode(struct udevice *bus, uint mode)
{
	struct pic32_spi_priv *priv = dev_get_priv(bus);
	u32 val;

	debug("%s: %s, mode %d\n", __func__, bus->name, mode);

	/* set spi-clk mode */
	val = readl(&priv->regs->ctrl.raw);
	/* HIGH when idle */
	if (mode & SPI_CPOL)
		val |= PIC32_SPI_CTRL_CKP;
	else
		val &= ~PIC32_SPI_CTRL_CKP;

	/* TX at idle-to-active clk transition */
	if (mode & SPI_CPHA)
		val &= ~PIC32_SPI_CTRL_CKE;
	else
		val |= PIC32_SPI_CTRL_CKE;

	/* RX at end of tx */
	val |= PIC32_SPI_CTRL_SMP;
	writel(val, &priv->regs->ctrl.raw);

	priv->mode = mode;

	return 0;
}

static int pic32_spi_set_wordlen(struct udevice *slave, unsigned int wordlen)
{
	struct pic32_spi_priv *priv = dev_get_priv(slave->parent);

	return pic32_spi_set_word_size(priv, wordlen);
}

static void pic32_spi_hw_init(struct pic32_spi_priv *priv)
{
	u32 val;

	/* disable module */
	pic32_spi_disable(priv);

	val = readl(&priv->regs->ctrl);

	/* enable enhanced fifo of 128bit deep */
	val |= PIC32_SPI_CTRL_ENHBUF;
	priv->fifo_depth = 16;

	/* disable framing mode */
	val &= ~PIC32_SPI_CTRL_FRMEN;

	/* enable master mode */
	val |= PIC32_SPI_CTRL_MSTEN;

	/* select clk source */
	val &= ~PIC32_SPI_CTRL_MCLKSEL;

	/* set manual /CS mode */
	val &= ~PIC32_SPI_CTRL_MSSEN;

	writel(val, &priv->regs->ctrl);

	/* clear rx overflow indicator */
	writel(PIC32_SPI_STAT_RX_OV, &priv->regs->status.clr);
}

static int pic32_spi_probe(struct udevice *bus)
{
	struct pic32_spi_priv *priv = dev_get_priv(bus);
	struct dm_spi_bus *dm_spi = dev_get_uclass_priv(bus);
	int node = dev_of_offset(bus);
	struct udevice *clkdev;
	fdt_addr_t addr;
	fdt_size_t size;
	int ret;

	debug("%s: %d, bus: %i\n", __func__, __LINE__, dev_seq(bus));
	addr = fdtdec_get_addr_size(gd->fdt_blob, node, "reg", &size);
	if (addr == FDT_ADDR_T_NONE)
		return -EINVAL;

	priv->regs = ioremap(addr, size);
	if (!priv->regs)
		return -EINVAL;

	dm_spi->max_hz = fdtdec_get_int(gd->fdt_blob, node, "spi-max-frequency",
					250000000);
	/* get clock rate */
	ret = clk_get_by_index(bus, 0, &clkdev);
	if (ret < 0) {
		printf("pic32-spi: error, clk not found\n");
		return ret;
	}
	priv->clk_rate = clk_get_periph_rate(clkdev, ret);

	/* initialize HW */
	pic32_spi_hw_init(priv);

	/* set word len */
	pic32_spi_set_word_size(priv, SPI_DEFAULT_WORDLEN);

	/* PIC32 SPI controller can automatically drive /CS during transfer
	 * depending on fifo fill-level. /CS will stay asserted as long as
	 * TX fifo is non-empty, else will be deasserted confirming completion
	 * of the ongoing transfer. To avoid this sort of error we will drive
	 * /CS manually by toggling cs-gpio pins.
	 */
	ret = gpio_request_by_name_nodev(offset_to_ofnode(node), "cs-gpios", 0,
					 &priv->cs_gpio, GPIOD_IS_OUT);
	if (ret) {
		printf("pic32-spi: error, cs-gpios not found\n");
		return ret;
	}

	return 0;
}

static const struct dm_spi_ops pic32_spi_ops = {
	.claim_bus	= pic32_spi_claim_bus,
	.release_bus	= pic32_spi_release_bus,
	.xfer		= pic32_spi_xfer,
	.set_speed	= pic32_spi_set_speed,
	.set_mode	= pic32_spi_set_mode,
	.set_wordlen	= pic32_spi_set_wordlen,
};

static const struct udevice_id pic32_spi_ids[] = {
	{ .compatible = "microchip,pic32mzda-spi" },
	{ }
};

U_BOOT_DRIVER(pic32_spi) = {
	.name		= "pic32_spi",
	.id		= UCLASS_SPI,
	.of_match	= pic32_spi_ids,
	.ops		= &pic32_spi_ops,
	.priv_auto	= sizeof(struct pic32_spi_priv),
	.probe		= pic32_spi_probe,
};

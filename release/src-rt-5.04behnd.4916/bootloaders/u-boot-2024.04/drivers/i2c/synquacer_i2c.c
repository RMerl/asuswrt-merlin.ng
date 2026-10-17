// SPDX-License-Identifier: GPL-2.0+
/*
 */

#include <dm/device_compat.h>
#include <linux/delay.h>
#include <linux/errno.h>
#include <linux/io.h>
#include <linux/sizes.h>
#include <linux/types.h>
#include <dm.h>
#include <fdtdec.h>
#include <i2c.h>
#include <clk.h>

#define REG_BSR		0x0
#define REG_BCR		0x4
#define REG_CCR		0x8
#define REG_ADR		0xc
#define REG_DAR		0x10
#define REG_CSR		0x14
#define REG_FSR		0x18
#define REG_BC2R	0x1c

/* I2C register bit definitions */
#define BSR_FBT		BIT(0)	// First Byte Transfer
#define BSR_GCA		BIT(1)	// General Call Address
#define BSR_AAS		BIT(2)	// Address as Slave
#define BSR_TRX		BIT(3)	// Transfer/Receive
#define BSR_LRB		BIT(4)	// Last Received Bit
#define BSR_AL		BIT(5)	// Arbitration Lost
#define BSR_RSC		BIT(6)	// Repeated Start Cond.
#define BSR_BB		BIT(7)	// Bus Busy

#define BCR_INT		BIT(0)	// Interrupt
#define BCR_INTE		BIT(1)	// Interrupt Enable
#define BCR_GCAA		BIT(2)	// Gen. Call Access Ack.
#define BCR_ACK		BIT(3)	// Acknowledge
#define BCR_MSS		BIT(4)	// Master Slave Select
#define BCR_SCC		BIT(5)	// Start Condition Cont.
#define BCR_BEIE		BIT(6)	// Bus Error Int Enable
#define BCR_BER		BIT(7)	// Bus Error

#define CCR_CS_MASK	(0x1f)	// CCR Clock Period Sel.
#define CCR_EN		BIT(5)	// Enable
#define CCR_FM		BIT(6)	// Speed Mode Select

#define CSR_CS_MASK	(0x3f)	// CSR Clock Period Sel.

#define BC2R_SCLL		BIT(0)	// SCL Low Drive
#define BC2R_SDAL		BIT(1)	// SDA Low Drive
#define BC2R_SCLS		BIT(4)	// SCL Status
#define BC2R_SDAS		BIT(5)	// SDA Status

/* PCLK frequency */
#define BUS_CLK_FR(rate)	(((rate) / 20000000) + 1)

#define I2C_CLK_DEF		62500000

/* STANDARD MODE frequency */
#define CLK_MASTER_STD(rate)			\
	DIV_ROUND_UP(DIV_ROUND_UP((rate), I2C_SPEED_STANDARD_RATE) - 2, 2)
/* FAST MODE frequency */
#define CLK_MASTER_FAST(rate)			\
	DIV_ROUND_UP((DIV_ROUND_UP((rate), I2C_SPEED_FAST_RATE) - 2) * 2, 3)

/* (clkrate <= 18000000) */
/* calculate the value of CS bits in CCR register on standard mode */
#define CCR_CS_STD_MAX_18M(rate)			\
	   ((CLK_MASTER_STD(rate) - 65)		\
					& CCR_CS_MASK)

/* calculate the value of CS bits in CSR register on standard mode */
#define CSR_CS_STD_MAX_18M(rate)		0x00

/* calculate the value of CS bits in CCR register on fast mode */
#define CCR_CS_FAST_MAX_18M(rate)			\
	   ((CLK_MASTER_FAST(rate) - 1)		\
					& CCR_CS_MASK)

/* calculate the value of CS bits in CSR register on fast mode */
#define CSR_CS_FAST_MAX_18M(rate)		0x00

/* (clkrate > 18000000) */
/* calculate the value of CS bits in CCR register on standard mode */
#define CCR_CS_STD_MIN_18M(rate)			\
	   ((CLK_MASTER_STD(rate) - 1)		\
					& CCR_CS_MASK)

/* calculate the value of CS bits in CSR register on standard mode */
#define CSR_CS_STD_MIN_18M(rate)			\
	   (((CLK_MASTER_STD(rate) - 1) >> 5)	\
					& CSR_CS_MASK)

/* calculate the value of CS bits in CCR register on fast mode */
#define CCR_CS_FAST_MIN_18M(rate)			\
	   ((CLK_MASTER_FAST(rate) - 1)		\
					& CCR_CS_MASK)

/* calculate the value of CS bits in CSR register on fast mode */
#define CSR_CS_FAST_MIN_18M(rate)			\
	   (((CLK_MASTER_FAST(rate) - 1) >> 5)	\
					& CSR_CS_MASK)

/* min I2C clock frequency 14M */
#define MIN_CLK_RATE	(14 * 1000000)
/* max I2C clock frequency 200M */
#define MAX_CLK_RATE	(200 * 1000000)
/* I2C clock frequency 18M */
#define CLK_RATE_18M	(18 * 1000000)

#define SPEED_FM		400	// Fast Mode
#define SPEED_SM		100	// Standard Mode

DECLARE_GLOBAL_DATA_PTR;

struct synquacer_i2c {
	void __iomem *base;
	unsigned long pclkrate;
	unsigned long speed_khz;
};

static int wait_irq(struct udevice *dev)
{
	struct synquacer_i2c *i2c = dev_get_priv(dev);
	int timeout = 500000;

	do {
		if (readb(i2c->base + REG_BCR) & BCR_INT)
			return 0;
	} while (timeout--);

	pr_err("%s: timeout\n", __func__);
	return -1;
}

static int synquacer_i2c_xfer_start(struct synquacer_i2c *i2c,
				    int addr, int read)
{
	u8 bsr, bcr;

	writeb((addr << 1) | (read ? 1 : 0), i2c->base + REG_DAR);

	bsr = readb(i2c->base + REG_BSR);
	bcr = readb(i2c->base + REG_BCR);

	if ((bsr & BSR_BB) && !(bcr & BCR_MSS))
		return -EBUSY;

	if (bsr & BSR_BB) {
		writeb(bcr | BCR_SCC, i2c->base + REG_BCR);
	} else {
		if (bcr & BCR_MSS)
			return -EAGAIN;
		/* Start Condition + Enable Interrupts */
		writeb(bcr | BCR_MSS | BCR_INTE | BCR_BEIE, i2c->base + REG_BCR);
	}

	udelay(100);
	return 0;
}

static int synquacer_i2c_xfer(struct udevice *bus,
			      struct i2c_msg *msg, int nmsgs)
{
	struct synquacer_i2c *i2c = dev_get_priv(bus);
	u8 bsr, bcr;
	int idx;

	for (; nmsgs > 0; nmsgs--, msg++) {
		synquacer_i2c_xfer_start(i2c, msg->addr, msg->flags & I2C_M_RD);
		if (wait_irq(bus))
			return -EREMOTEIO;

		bsr = readb(i2c->base + REG_BSR);
		if (bsr & BSR_LRB) {
			debug("%s: No ack received\n", __func__);
			return -EREMOTEIO;
		}

		idx = 0;
		do {
			bsr = readb(i2c->base + REG_BSR);
			bcr = readb(i2c->base + REG_BCR);
			if (bcr & BCR_BER) {
				debug("%s: Bus error detected\n", __func__);
				return -EREMOTEIO;
			}
			if ((bsr & BSR_AL) || !(bcr & BCR_MSS)) {
				debug("%s: Arbitration lost\n", __func__);
				return -EREMOTEIO;
			}

			if (msg->flags & I2C_M_RD) {
				bcr = BCR_MSS | BCR_INTE | BCR_BEIE;
				if (idx < msg->len - 1)
					bcr |= BCR_ACK;
				writeb(bcr, i2c->base + REG_BCR);
				if (wait_irq(bus))
					return -EREMOTEIO;
				bsr = readb(i2c->base + REG_BSR);
				if (!(bsr & BSR_FBT))
					msg->buf[idx++] = readb(i2c->base + REG_DAR);
			} else {
				writeb(msg->buf[idx++], i2c->base + REG_DAR);
				bcr = BCR_MSS | BCR_INTE | BCR_BEIE;
				writeb(bcr, i2c->base + REG_BCR);
				if (wait_irq(bus))
					return -EREMOTEIO;
				bsr = readb(i2c->base + REG_BSR);
				if (bsr & BSR_LRB) {
					debug("%s: no ack\n", __func__);
					return -EREMOTEIO;
				}
			}
		} while (idx < msg->len);
	}

	/* Force bus state to idle, terminating any ongoing transfer */
	writeb(0, i2c->base + REG_BCR);
	udelay(100);

	return 0;
}

static void synquacer_i2c_hw_reset(struct synquacer_i2c *i2c)
{
	/* Disable clock */
	writeb(0, i2c->base + REG_CCR);
	writeb(0, i2c->base + REG_CSR);

	/* Set own Address */
	writeb(0, i2c->base + REG_ADR);

	/* Set PCLK frequency */
	writeb(BUS_CLK_FR(i2c->pclkrate), i2c->base + REG_FSR);

	/* clear IRQ (INT=0, BER=0), Interrupt Disable */
	writeb(0, i2c->base + REG_BCR);
	writeb(0, i2c->base + REG_BC2R);
}

static int synquacer_i2c_get_bus_speed(struct udevice *bus)
{
	struct synquacer_i2c *i2c = dev_get_priv(bus);

	return i2c->speed_khz * 1000;
}

static int synquacer_i2c_set_bus_speed(struct udevice *bus, unsigned int speed)
{
	struct synquacer_i2c *i2c = dev_get_priv(bus);
	u32 rt = i2c->pclkrate;
	u8 ccr_cs, csr_cs;

	/* Set PCLK frequency */
	writeb(BUS_CLK_FR(i2c->pclkrate), i2c->base + REG_FSR);

	if (speed >= SPEED_FM * 1000) {
		i2c->speed_khz = SPEED_FM;
		if (i2c->pclkrate <= CLK_RATE_18M) {
			ccr_cs = CCR_CS_FAST_MAX_18M(rt);
			csr_cs = CSR_CS_FAST_MAX_18M(rt);
		} else {
			ccr_cs = CCR_CS_FAST_MIN_18M(rt);
			csr_cs = CSR_CS_FAST_MIN_18M(rt);
		}

		/* Set Clock and enable, Set fast mode */
		writeb(ccr_cs | CCR_FM | CCR_EN, i2c->base + REG_CCR);
		writeb(csr_cs, i2c->base + REG_CSR);
	} else {
		i2c->speed_khz = SPEED_SM;
		if (i2c->pclkrate <= CLK_RATE_18M) {
			ccr_cs = CCR_CS_STD_MAX_18M(rt);
			csr_cs = CSR_CS_STD_MAX_18M(rt);
		} else {
			ccr_cs = CCR_CS_STD_MIN_18M(rt);
			csr_cs = CSR_CS_STD_MIN_18M(rt);
		}

		/* Set Clock and enable, Set standard mode */
		writeb(ccr_cs | CCR_EN, i2c->base + REG_CCR);
		writeb(csr_cs, i2c->base + REG_CSR);
	}

	return 0;
}

static int synquacer_i2c_of_to_plat(struct udevice *bus)
{
	struct synquacer_i2c *priv = dev_get_priv(bus);
	struct clk ck;
	int ret;

	ret = clk_get_by_index(bus, 0, &ck);
	if (ret < 0) {
		priv->pclkrate = I2C_CLK_DEF;
	} else {
		clk_enable(&ck);
		priv->pclkrate = clk_get_rate(&ck);
	}

	return 0;
}

static int synquacer_i2c_probe(struct udevice *bus)
{
	struct synquacer_i2c *i2c = dev_get_priv(bus);

	i2c->base = dev_read_addr_ptr(bus);
	synquacer_i2c_hw_reset(i2c);
	synquacer_i2c_set_bus_speed(bus, 400000); /* set default speed */
	return 0;
}

static const struct dm_i2c_ops synquacer_i2c_ops = {
	.xfer = synquacer_i2c_xfer,
	.set_bus_speed = synquacer_i2c_set_bus_speed,
	.get_bus_speed = synquacer_i2c_get_bus_speed,
};

static const struct udevice_id synquacer_i2c_ids[] = {
	{
		.compatible = "socionext,synquacer-i2c",
	},
	{ }
};

U_BOOT_DRIVER(sni_synquacer_i2c) = {
	.name	= "sni_synquacer_i2c",
	.id	= UCLASS_I2C,
	.of_match = synquacer_i2c_ids,
	.of_to_plat = synquacer_i2c_of_to_plat,
	.probe	= synquacer_i2c_probe,
	.priv_auto	= sizeof(struct synquacer_i2c),
	.ops	= &synquacer_i2c_ops,
};

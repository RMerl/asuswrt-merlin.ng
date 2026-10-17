// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright (C) 2019 Stephan Gerhold
 *
 * Adapted from old U-Boot and Linux kernel implementation:
 * Copyright (C) STMicroelectronics 2009
 * Copyright (C) ST-Ericsson SA 2010
 */

#include <common.h>
#include <dm.h>
#include <regmap.h>
#include <syscon.h>
#include <linux/bitops.h>
#include <linux/err.h>
#include <power/ab8500.h>
#include <power/pmic.h>

/* CPU mailbox registers */
#define PRCM_MBOX_CPU_VAL		0x0fc
#define PRCM_MBOX_CPU_SET		0x100
#define PRCM_MBOX_CPU_CLR		0x104

#define PRCM_ARM_IT1_CLR		0x48C
#define PRCM_ARM_IT1_VAL		0x494

#define PRCM_TCDM_RANGE			2
#define PRCM_REQ_MB5			0xE44
#define PRCM_ACK_MB5			0xDF4
#define _PRCM_MBOX_HEADER		0xFE8
#define PRCM_MBOX_HEADER_REQ_MB5	(_PRCM_MBOX_HEADER + 0x5)
#define PRCMU_I2C_MBOX_BIT		BIT(5)

/* Mailbox 5 Requests */
#define PRCM_REQ_MB5_I2C_SLAVE_OP	(PRCM_REQ_MB5 + 0x0)
#define PRCM_REQ_MB5_I2C_HW_BITS	(PRCM_REQ_MB5 + 0x1)
#define PRCM_REQ_MB5_I2C_REG		(PRCM_REQ_MB5 + 0x2)
#define PRCM_REQ_MB5_I2C_VAL		(PRCM_REQ_MB5 + 0x3)
#define PRCMU_I2C(bank)			(((bank) << 1) | BIT(6))
#define PRCMU_I2C_WRITE			0
#define PRCMU_I2C_READ			1
#define PRCMU_I2C_STOP_EN		BIT(3)

/* Mailbox 5 ACKs */
#define PRCM_ACK_MB5_I2C_STATUS		(PRCM_ACK_MB5 + 0x1)
#define PRCM_ACK_MB5_I2C_VAL		(PRCM_ACK_MB5 + 0x3)
#define PRCMU_I2C_WR_OK			0x1
#define PRCMU_I2C_RD_OK			0x2

/* AB8500 version registers */
#define AB8500_MISC_REV_REG		AB8500_MISC(0x80)
#define AB8500_MISC_IC_NAME_REG		AB8500_MISC(0x82)

struct ab8500_priv {
	struct ab8500 ab8500;
	struct regmap *regmap;
};

static inline int prcmu_tcdm_readb(struct regmap *map, uint offset, u8 *valp)
{
	return regmap_raw_read_range(map, PRCM_TCDM_RANGE, offset,
				     valp, sizeof(*valp));
}

static inline int prcmu_tcdm_writeb(struct regmap *map, uint offset, u8 val)
{
	return regmap_raw_write_range(map, PRCM_TCDM_RANGE, offset,
				      &val, sizeof(val));
}

static int prcmu_wait_i2c_mbx_ready(struct ab8500_priv *priv)
{
	uint val;
	int ret;

	ret = regmap_read(priv->regmap, PRCM_ARM_IT1_VAL, &val);
	if (ret)
		return ret;

	if (val & PRCMU_I2C_MBOX_BIT) {
		printf("ab8500: warning: PRCMU i2c mailbox was not acked\n");
		/* clear mailbox 5 ack irq */
		ret = regmap_write(priv->regmap, PRCM_ARM_IT1_CLR,
				   PRCMU_I2C_MBOX_BIT);
		if (ret)
			return ret;
	}

	/* wait for on-going transaction, use 1s timeout */
	return regmap_read_poll_timeout(priv->regmap, PRCM_MBOX_CPU_VAL, val,
					!(val & PRCMU_I2C_MBOX_BIT), 0, 1000);
}

static int prcmu_wait_i2c_mbx_done(struct ab8500_priv *priv)
{
	uint val;
	int ret;

	/* set interrupt to XP70 */
	ret = regmap_write(priv->regmap, PRCM_MBOX_CPU_SET, PRCMU_I2C_MBOX_BIT);
	if (ret)
		return ret;

	/* wait for mailbox 5 (i2c) ack, use 1s timeout */
	return regmap_read_poll_timeout(priv->regmap, PRCM_ARM_IT1_VAL, val,
					(val & PRCMU_I2C_MBOX_BIT), 0, 1000);
}

static int ab8500_transfer(struct udevice *dev, uint bank_reg, u8 *val,
			   u8 op, u8 expected_status)
{
	struct ab8500_priv *priv = dev_get_priv(dev);
	u8 reg = bank_reg & 0xff;
	u8 bank = bank_reg >> 8;
	u8 status;
	int ret;

	ret = prcmu_wait_i2c_mbx_ready(priv);
	if (ret)
		return ret;

	ret = prcmu_tcdm_writeb(priv->regmap, PRCM_MBOX_HEADER_REQ_MB5, 0);
	if (ret)
		return ret;
	ret = prcmu_tcdm_writeb(priv->regmap, PRCM_REQ_MB5_I2C_SLAVE_OP,
				PRCMU_I2C(bank) | op);
	if (ret)
		return ret;
	ret = prcmu_tcdm_writeb(priv->regmap, PRCM_REQ_MB5_I2C_HW_BITS,
				PRCMU_I2C_STOP_EN);
	if (ret)
		return ret;
	ret = prcmu_tcdm_writeb(priv->regmap, PRCM_REQ_MB5_I2C_REG, reg);
	if (ret)
		return ret;
	ret = prcmu_tcdm_writeb(priv->regmap, PRCM_REQ_MB5_I2C_VAL, *val);
	if (ret)
		return ret;

	ret = prcmu_wait_i2c_mbx_done(priv);
	if (ret) {
		printf("%s: mailbox request timed out\n", __func__);
		return ret;
	}

	/* read transfer result */
	ret = prcmu_tcdm_readb(priv->regmap, PRCM_ACK_MB5_I2C_STATUS, &status);
	if (ret)
		return ret;
	ret = prcmu_tcdm_readb(priv->regmap, PRCM_ACK_MB5_I2C_VAL, val);
	if (ret)
		return ret;

	/*
	 * Clear mailbox 5 ack irq. Note that the transfer is already complete
	 * here so checking for errors does not make sense. Clearing the irq
	 * will be retried in prcmu_wait_i2c_mbx_ready() on the next transfer.
	 */
	regmap_write(priv->regmap, PRCM_ARM_IT1_CLR, PRCMU_I2C_MBOX_BIT);

	if (status != expected_status) {
		/*
		 * AB8500 does not have the AB8500_MISC_IC_NAME_REG register,
		 * but we need to try reading it to detect AB8505.
		 * In case of an error, assume that we have AB8500.
		 */
		if (op == PRCMU_I2C_READ && bank_reg == AB8500_MISC_IC_NAME_REG) {
			*val = AB8500_VERSION_AB8500;
			return 0;
		}

		printf("%s: return status %d\n", __func__, status);
		return -EIO;
	}

	return 0;
}

static int ab8500_reg_count(struct udevice *dev)
{
	return AB8500_NUM_REGISTERS;
}

static int ab8500_read(struct udevice *dev, uint reg, uint8_t *buf, int len)
{
	int ret;

	if (len != 1)
		return -EINVAL;

	*buf = 0;
	ret = ab8500_transfer(dev, reg, buf, PRCMU_I2C_READ, PRCMU_I2C_RD_OK);
	if (ret) {
		printf("%s failed: %d\n", __func__, ret);
		return ret;
	}

	return 0;
}

static int ab8500_write(struct udevice *dev, uint reg, const uint8_t *buf, int len)
{
	int ret;
	u8 val;

	if (len != 1)
		return -EINVAL;

	val = *buf;
	ret = ab8500_transfer(dev, reg, &val, PRCMU_I2C_WRITE, PRCMU_I2C_WR_OK);
	if (ret) {
		printf("%s failed: %d\n", __func__, ret);
		return ret;
	}

	return 0;
}

static struct dm_pmic_ops ab8500_ops = {
	.reg_count = ab8500_reg_count,
	.read = ab8500_read,
	.write = ab8500_write,
};

static int ab8500_probe(struct udevice *dev)
{
	struct ab8500_priv *priv = dev_get_priv(dev);
	int ret;

	/* get regmap from the PRCMU parent device (syscon in U-Boot) */
	priv->regmap = syscon_get_regmap(dev->parent);
	if (IS_ERR(priv->regmap))
		return PTR_ERR(priv->regmap);

	ret = pmic_reg_read(dev, AB8500_MISC_IC_NAME_REG);
	if (ret < 0) {
		printf("ab8500: failed to read chip version: %d\n", ret);
		return ret;
	}
	priv->ab8500.version = ret;

	ret = pmic_reg_read(dev, AB8500_MISC_REV_REG);
	if (ret < 0) {
		printf("ab8500: failed to read chip id: %d\n", ret);
		return ret;
	}
	priv->ab8500.chip_id = ret;

	debug("ab8500: version: %#x, chip id: %#x\n",
	      priv->ab8500.version, priv->ab8500.chip_id);

	return 0;
}

static const struct udevice_id ab8500_ids[] = {
	{ .compatible = "stericsson,ab8500" },
	{ }
};

U_BOOT_DRIVER(pmic_ab8500) = {
	.name		= "pmic_ab8500",
	.id		= UCLASS_PMIC,
	.of_match	= ab8500_ids,
	.bind		= dm_scan_fdt_dev,
	.probe		= ab8500_probe,
	.ops		= &ab8500_ops,
	.priv_auto	= sizeof(struct ab8500_priv),
};

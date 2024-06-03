// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * Copyright 2023, Farhan Ali <farhan.ali@broadcom.com>
 *
 * Driver for Broadcom bcmbca True random number generator
 */

#include <common.h>
#include <dm.h>
#include <malloc.h>
#include <linux/delay.h>
#include <rng.h>
#include <asm/io.h>
#include <linux/errno.h>
#include <linux/io.h>
#include <linux/ioport.h>

/**
 * @brief Supported command types.
 */
typedef enum
{
    TRNG_CMD_NONE = 0,          /**< No command currently submitted.            */
    TRNG_CMD_INSTANTIATE,   /**< Instantiate the block ready to generate.   */
    TRNG_CMD_RESEED,        /**< Force the generation of a new seed.        */
    TRNG_CMD_GENERATE,      /**< Generate random bytes.                     */
    TRNG_CMD_TEST,          /**< Run self tests of DRBG and entropy source. */
    TRNG_CMD_UNINSTANTIATE, /**< Erase internal state and flush TRNG fifo.  */
    TRNG_CMD_STANDBY,       /**< Toggle standby mode.                       */
    TRNG_CMD_RESET          /**< Reset the block and execute startup tests. */
} TRNG_COMMAND;

#define TRNG_CMD_OFFSET			0x00000010
#define TRNG_CMD_MASK			0x0000000F

#define TRNG_STATUS_OFFSET		0x00000020
#define TRNG_STATUS_CMD_MASK		0x0000000F
#define TRNG_STATUS_RDY			0x00000010
#define TRNG_STATUS_ERR			0x00000020
#define TRNG_STATUS_DATA_CNT		0xFF000000
#define TRNG_STATUS_DATA_CNT_SHIFT	24

#define TRNG_DOUT_OFFSET		0x00000040	/* Fifo depth is 32 * 32 bits */

#define TRNG_REQ_LEN_OFFSET		0x00000060
#define TRNG_REQ_LEN_MAX_VAL		4095		/* Generate 4095 X 128bit blocks */

#define TRNG_USER_IN_LEN_OFFSET		0x00000070

#define TRNG_CTRL_OFFSET		0x00000080
#define TRNG_CTRL_USE_RO		0x00000001
#define TRNG_CTRL_PRED_RES_ON		0x00000002

#define TRNG_MAX_RDY_WAIT_US		500

#define TRNG_MAX_RESETS_PER_READ	1

#define TRNG_DEBUG			0

struct bcmbca_trng_platdata {
	fdt_addr_t base;
};

static int bcmbca_trng_waitrdy(struct udevice *dev)
{
	struct bcmbca_trng_platdata *priv = dev_get_platdata(dev);
	int wait_us = 0;
	u32 val = 0;

	/* Check if TRNG is ready */
	val = readl(priv->base + TRNG_STATUS_OFFSET);
	while (!(val & TRNG_STATUS_RDY) && !(val & TRNG_STATUS_ERR)) {
		/* Check wait limit */
		if (wait_us > TRNG_MAX_RDY_WAIT_US)
		{
			dev_err(dev,"%s: Error TRNG not responding! status: 0x%08x\n", __FUNCTION__,
				val);
			return -EBUSY;
		}

		/* wait */
		udelay(10);
		wait_us += 10;

		/* read status */
		val = readl(priv->base + TRNG_STATUS_OFFSET);
	}

#if TRNG_DEBUG
	printf("%s: status: 0x%08x wait: %d\n", __FUNCTION__, val, wait_us);
#endif

	return (val & TRNG_STATUS_ERR);
}

static int bcmbca_trng_sendcmd(struct udevice *dev, TRNG_COMMAND cmd)
{
	int ret = 0;
	struct bcmbca_trng_platdata *priv = dev_get_platdata(dev);

	/* wait for ready status for certain commands */
	if( (cmd != TRNG_CMD_TEST) && (cmd != TRNG_CMD_RESET) )
		ret = bcmbca_trng_waitrdy(dev);

	if( ret == 0 ) {
		/* Send command */
		writel(cmd, priv->base + TRNG_CMD_OFFSET);
	}
	return ret;
}

static int bcmbca_trng_reset(struct udevice *dev, bool enable)
{
	struct bcmbca_trng_platdata *priv = dev_get_platdata(dev);
	u32 val;
	int ret = 0;

	/* Reset the TRNG */
	ret = bcmbca_trng_sendcmd(dev, TRNG_CMD_RESET);
	if (ret) {
		dev_err(dev, "%s: Error sending TRNG_CMD_RESET\n", __FUNCTION__);
		return -EFAULT;
	}

	if( enable ) {	
		/* Enable use of internal entropy source */
		val = readl(priv->base + TRNG_CTRL_OFFSET);
		val |= TRNG_CTRL_USE_RO;
		writel(val, priv->base + TRNG_CTRL_OFFSET);

		/* Disable user personalization string */
		writel(0, priv->base + TRNG_USER_IN_LEN_OFFSET);
		
		/* Instantiate initial seed */
		ret = bcmbca_trng_sendcmd(dev, TRNG_CMD_INSTANTIATE);
		if (ret) {
			dev_err(dev,"%s: Error sending TRNG_CMD_INSTANTIATE\n", __FUNCTION__);
			return -EFAULT;
		}
	}

	/* Return after waiting for entire reset process to complete */
	return bcmbca_trng_waitrdy(dev);
}

static int bcmbca_trng_read(struct udevice *dev, void *data, size_t len)
{
	struct bcmbca_trng_platdata *priv = dev_get_platdata(dev);
	u32 status;
	char *buf = (char *)data;
	u32 num_remaining = len;
	u32 ret = 0;
	u32 num_words_avail = 0;

	u32 num_resets = 0;

	while (num_remaining > 0) {
		/* Get TRNG status */
		status = readl(priv->base + TRNG_STATUS_OFFSET);

		/* Check TRNG status */
		if (status & TRNG_STATUS_ERR) {
			/* TRNG is in error condition, reset it */
			if (num_resets >= TRNG_MAX_RESETS_PER_READ)
				return len - num_remaining;

			ret = bcmbca_trng_reset(dev, true);
			if (ret) {
				dev_err(dev,"%s: Error resetting TRNG!\n", __FUNCTION__);
				return -EFAULT;
			}

			/* Log number of resets */
			num_resets++;
		} else if (status & TRNG_STATUS_DATA_CNT) {
			/* Are there any random numbers available? */
			num_words_avail = ((status & TRNG_STATUS_DATA_CNT) >> TRNG_STATUS_DATA_CNT_SHIFT);	

			/* Use the words available */
			while( num_words_avail && num_remaining )
			{
				if (num_remaining >= sizeof(u32)) {
					/* Buffer has room to store entire word */
					*(u32 *)buf = readl(priv->base +
								TRNG_DOUT_OFFSET);
					buf += sizeof(u32);
					num_remaining -= sizeof(u32);
				} else {
					/* Buffer can only store partial word */
					u32 rnd_number = readl(priv->base +
								TRNG_DOUT_OFFSET);
					memcpy(buf, &rnd_number, num_remaining);
					buf += num_remaining;
					num_remaining = 0;
				}
				num_words_avail--;
			}
		} else if (status & TRNG_STATUS_RDY) {
			/* No random number available AND num_remaining !=0 ---> generate new random numbers */
			writel(TRNG_REQ_LEN_MAX_VAL, priv->base + TRNG_REQ_LEN_OFFSET);
			ret = bcmbca_trng_sendcmd(dev, TRNG_CMD_GENERATE);
			if (ret) {
				dev_err(dev,"%s: Error sending TRNG_CMD_GENERATE\n", __FUNCTION__);
				return -EFAULT;
			}

			/* Wait for generation */
			udelay(10);
		} else {
			/* Should never come here */
			dev_err(dev,"%s: Unknown Error\n", __FUNCTION__);
			return -EFAULT;
		}

	}

	return 0;
}

static int bcmbca_trng_probe(struct udevice *dev)
{
	struct bcmbca_trng_platdata *priv = dev_get_platdata(dev);

	priv->base = (uintptr_t)dev_remap_addr(dev);
	if (!priv->base) {
		dev_err(dev, "can't get rng regs !\n");
		return -EINVAL;
	}

	return bcmbca_trng_reset(dev, true);
}

static int bcmbca_trng_remove(struct udevice *dev)
{
	return bcmbca_trng_reset(dev, false);
}

static const struct dm_rng_ops bcmbca_trng_ops = {
	.read = bcmbca_trng_read,
};

static const struct udevice_id bcmbca_trng_rng_match[] = {
	{ .compatible = "brcm,bcmbca-trng", },
	{},
};

U_BOOT_DRIVER(bcmbca_trng_rng) = {
	.name = "bcmbca-trng",
	.id = UCLASS_RNG,
	.of_match = bcmbca_trng_rng_match,
	.ops = &bcmbca_trng_ops,
	.probe = bcmbca_trng_probe,
	.remove = bcmbca_trng_remove,
	.platdata_auto_alloc_size = sizeof(struct bcmbca_trng_platdata),
};

// SPDX-License-Identifier: GPL-2.0
/*
 * drivers/char/hw_random/bcmbca-rng.c
 *
 * True Random Number Generator Driver for Broadcom's BCA chips
 *
 * Author: Farhan Ali <farhan.ali@broadcom.com>
 *
 * Copyright 2023 (c) Broadcom Corporation
 */

#include <linux/hw_random.h>
#include <linux/init.h>
#include <linux/io.h>
#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/of_address.h>
#include <linux/of_platform.h>
#include <linux/platform_device.h>
#include <linux/delay.h>
#include <linux/slab.h>

#define TRNG_CMD_OFFSET			0x00000010
enum trng_command {
	TRNG_CMD_NONE = 0,      /**< No command currently submitted.            */
	TRNG_CMD_INSTANTIATE,   /**< Instantiate the block ready to generate.   */
	TRNG_CMD_RESEED,        /**< Force the generation of a new seed.        */
	TRNG_CMD_GENERATE,      /**< Generate random bytes.                     */
	TRNG_CMD_TEST,          /**< Run self tests of DRBG and entropy source. */
	TRNG_CMD_UNINSTANTIATE, /**< Erase internal state and flush TRNG fifo.  */
	TRNG_CMD_STANDBY,       /**< Toggle standby mode.                       */
	TRNG_CMD_RESET          /**< Reset the block and execute startup tests. */
};
#define TRNG_CMD_MASK			0x0000000F

#define TRNG_STATUS_OFFSET		0x00000020
#define TRNG_STATUS_CMD_MASK		0x0000000F
#define TRNG_STATUS_RDY			0x00000010
#define TRNG_STATUS_ERR			0x00000020
#define TRNG_STATUS_DATA_CNT_MASK	0xFF000000
#define TRNG_STATUS_DATA_CNT_SHIFT	24

#define TRNG_DOUT_OFFSET		0x00000040	/* Fifo depth is 32 * 32 bits */

#define TRNG_REQ_LEN_OFFSET		0x00000060
#define TRNG_REQ_LEN_MAX_VAL		4095		/* Generate 4095 X 128bit blocks */

#define TRNG_USER_IN_LEN_OFFSET		0x00000070

#define TRNG_CTRL_OFFSET		0x00000080
#define TRNG_CTRL_USE_RO		0x00000001
#define TRNG_CTRL_PRED_RES_ON		0x00000002

#define TRNG_MIN_ENTR_OFFSET		0x00000090
#define TRNG_MIN_ENTR_DFLT		4
#define TRNG_MIN_ENTR_MAX_VAL		31

#define TRNG_MAX_RDY_WAIT_US		500

#define TRNG_MAX_RESETS_PER_READ	1

#define TRNG_HIGH_PERFORMANCE		1		/* Disabling allows better concurrency */

#define TRNG_MAX_IDLE_MSEC		1000

#define to_trng_priv(rng)		container_of(rng, struct bcmbca_trng_private, rng)

struct bcmbca_trng_private {
	struct hwrng rng;
	void __iomem *base;
};

static unsigned int num_blk_gen = TRNG_REQ_LEN_MAX_VAL;
static unsigned int min_entr = TRNG_MIN_ENTR_DFLT;
static bool reset_pending;

/* Handler for num_blk_gen module param */
static int num_blk_gen_set(const char *val, const struct kernel_param *kp)
{
	int ret;
	unsigned int old_num_blk_gen = num_blk_gen;
	/* sets num_blk_gen directly. no need to restore it in case of
	 * illegal value since we assume this will fail insmod
	 */
	ret = param_set_uint(val, kp);
	if (ret)
		return ret;

	if (num_blk_gen < 1 || num_blk_gen > TRNG_REQ_LEN_MAX_VAL) {
		num_blk_gen = old_num_blk_gen;
		ret = -EINVAL;
	}

	reset_pending = true;
	return ret;
}
static const struct kernel_param_ops num_blk_gen_ops = {
	.set = num_blk_gen_set,
	.get = param_get_uint,
};
module_param_cb(num_blk_gen, &num_blk_gen_ops, &num_blk_gen, 0644);
MODULE_PARM_DESC(num_blk_gen, "No. of 128-bit random blocks to generate");

/* Handler for min_entr module param */
static int min_entr_set(const char *val, const struct kernel_param *kp)
{
	int ret;
	unsigned int old_min_entr = min_entr;
	/* sets min_entr directly. no need to restore it in case of
	 * illegal value since we assume this will fail insmod
	 */
	ret = param_set_uint(val, kp);
	if (ret)
		return ret;

	if (min_entr > TRNG_MIN_ENTR_MAX_VAL) {
		min_entr = old_min_entr;
		ret = -EINVAL;
	}

	reset_pending = true;
	return ret;
}
static const struct kernel_param_ops min_entr_ops = {
	.set = min_entr_set,
	.get = param_get_uint,
};
module_param_cb(min_entr, &min_entr_ops, &min_entr, 0644);
MODULE_PARM_DESC(min_entr, "Assessed minimum entropy");

static int bcmbca_trng_waitrdy(struct hwrng *rng)
{
	struct bcmbca_trng_private *priv = to_trng_priv(rng);
	struct device *dev = (struct device *)priv->rng.priv;
	int wait_us = 0;
	u32 val = 0;

	/* Check if TRNG is ready */
	val = readl(priv->base + TRNG_STATUS_OFFSET);
	while (!(val & TRNG_STATUS_RDY) && !(val & TRNG_STATUS_ERR)) {
		/* Check wait limit */
		if (wait_us > TRNG_MAX_RDY_WAIT_US) {
			dev_err(dev, "%s: Error TRNG not responding! status: 0x%08x\n",
				__func__, val);
			return -EBUSY;
		}

		/* wait */
		usleep_range(10, 11);
		wait_us += 10;

		/* read status */
		val = readl(priv->base + TRNG_STATUS_OFFSET);
	}

	dev_dbg(dev, "%s: status: 0x%08x wait: %d\n", __func__, val, wait_us);

	return val & TRNG_STATUS_ERR;
}

static int bcmbca_trng_sendcmd(struct hwrng *rng, enum trng_command cmd)
{
	struct bcmbca_trng_private *priv = to_trng_priv(rng);
	int ret = 0;
	/* wait for ready status for certain commands */
	if ((cmd != TRNG_CMD_TEST) && (cmd != TRNG_CMD_RESET))
		ret = bcmbca_trng_waitrdy(rng);

	/* Send command */
	if (ret == 0)
		writel(cmd, priv->base + TRNG_CMD_OFFSET);

	return ret;
}

static int bcmbca_trng_reset(struct hwrng *rng, bool enable)
{
	struct bcmbca_trng_private *priv = to_trng_priv(rng);
	struct device *dev = (struct device *)priv->rng.priv;
	int ret = 0;
	u32 val;
	/* Reset the TRNG */
	ret = bcmbca_trng_sendcmd(rng, TRNG_CMD_RESET);
	if (ret) {
		dev_err(dev, "%s: Error sending TRNG_CMD_RESET, ret:%d\n", __func__, ret);
		return ret;
	}

	if (!enable)
		goto exit_trng_reset;

	/* Get control values */
	val = readl(priv->base + TRNG_CTRL_OFFSET);
	/* Enable use of internal entropy source */
	val |= TRNG_CTRL_USE_RO;
	/* Turn on prediction resistance mode - Reseed before every GENERATE cmd */
	val |= TRNG_CTRL_PRED_RES_ON;
	/* Set control values */
	writel(val, priv->base + TRNG_CTRL_OFFSET);
	/* Disable user personalization string */
	writel(0, priv->base + TRNG_USER_IN_LEN_OFFSET);
	/* Set minimum entropy */
	writel(min_entr, priv->base + TRNG_MIN_ENTR_OFFSET);
	/* Instantiate initial seed */
	ret = bcmbca_trng_sendcmd(rng, TRNG_CMD_INSTANTIATE);
	if (ret) {
		dev_err(dev, "%s: Error sending TRNG_CMD_INSTANTIATE, ret:%d\n", __func__, ret);
		return ret;
	}

exit_trng_reset:
	/* Return after waiting for entire reset process to complete */
	return bcmbca_trng_waitrdy(rng);
}

static int bcmbca_trng_read(struct hwrng *rng, void *buf, size_t max,
			     bool wait)
{
	struct bcmbca_trng_private *priv = to_trng_priv(rng);
	struct device *dev = (struct device *)priv->rng.priv;
	u32 num_remaining = max;
	u32 num_words_avail = 0;
	u32 status;
	u32 ret = 0;
	u32 num_resets = 0;
	unsigned long idle_endtime = jiffies + msecs_to_jiffies(TRNG_MAX_IDLE_MSEC);

	while ((num_remaining > 0) && time_before(jiffies, idle_endtime)) {
		/* Get TRNG status */
		status = readl(priv->base + TRNG_STATUS_OFFSET);
		/* Check TRNG status */
		if (reset_pending || status & TRNG_STATUS_ERR) {
			if (!reset_pending) {
				/* TRNG is in error condition, reset it */
				if ((num_resets >= TRNG_MAX_RESETS_PER_READ) || !wait)
					return max - num_remaining;
			}

			ret = bcmbca_trng_reset(rng, true);
			if (ret) {
				dev_err(dev, "%s: Error resetting TRNG! ret:%d\n", __func__, ret);
				return ret;
			}

			/* Log number of resets */
			if (!reset_pending)
				num_resets++;
			else
				reset_pending = false;

		} else if (status & TRNG_STATUS_DATA_CNT_MASK) {
			/* Are there any random numbers available? */
			num_words_avail = ((status & TRNG_STATUS_DATA_CNT_MASK) >>
						TRNG_STATUS_DATA_CNT_SHIFT);

#if TRNG_HIGH_PERFORMANCE
			/* Use all the random words available */
			while (num_words_avail && num_remaining)
#else
			/* Use one random word per iteration */
			if (num_words_avail && num_remaining)
#endif
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

				/* Reset the IDLE timeout */
				idle_endtime = jiffies + msecs_to_jiffies(TRNG_MAX_IDLE_MSEC);
			}
		} else if (status & TRNG_STATUS_RDY) {
			/* Is TRNG ready to generte new numbers? */
			/* If we cannot wait, return immediately */
			if (!wait)
				return max - num_remaining;
			/* No random number available AND num_remaining !=0 ---> generate
			 * new random numbers
			 */
			writel(num_blk_gen, priv->base + TRNG_REQ_LEN_OFFSET);
			ret = bcmbca_trng_sendcmd(rng, TRNG_CMD_GENERATE);
			if (ret) {
				dev_err(dev, "%s: Error sending TRNG_CMD_GENERATE, ret:%d\n",
					__func__, ret);
				return ret;
			}

			/* Can wait, give others chance to run */
			usleep_range(min(num_remaining * 20, 500U), 500);
		} else {
			/* Should never come here */
			WARN_ON(status);
			dev_err(dev, "%s: Unknown Error\n", __func__);
			return -EFAULT;
		}
	}
	return max - num_remaining;
}

static int bcmbca_trng_init(struct hwrng *rng)
{
	struct bcmbca_trng_private *priv = to_trng_priv(rng);
	struct device *dev = (struct device *)priv->rng.priv;

	dev_dbg(dev, "Using REQ_LEN:%d\n", num_blk_gen);
	reset_pending = false;
	return bcmbca_trng_reset(rng, true);
}

static void bcmbca_trng_cleanup(struct hwrng *rng)
{
	bcmbca_trng_reset(rng, false);
}

static int bcmbca_trng_remove(struct platform_device *pdev)
{
	struct device *dev = &pdev->dev;
	struct bcmbca_trng_private *priv = dev_get_drvdata(dev);

	bcmbca_trng_reset(&priv->rng, false);
	devm_hwrng_unregister(dev, &priv->rng);
	return 0;
}

static int bcmbca_trng_probe(struct platform_device *pdev)
{
	struct bcmbca_trng_private *priv;
	struct device *dev = &pdev->dev;
	struct resource *res;
	int ret = 0;

	priv = devm_kzalloc(dev, sizeof(*priv), GFP_KERNEL);
	if (!priv)
		return -ENOMEM;

	res = platform_get_resource(pdev, IORESOURCE_MEM, 0);
	priv->base = devm_ioremap_resource(dev, res);
	if (IS_ERR(priv->base)) {
		dev_err(dev, "failed to remap rng regs\n");
		ret = PTR_ERR(priv->base);
		goto err_exit;
	}

	dev_set_drvdata(dev, priv);
	priv->rng.name = "bcmbca-trng",
	priv->rng.read = bcmbca_trng_read,
	priv->rng.init = bcmbca_trng_init,
	priv->rng.cleanup = bcmbca_trng_cleanup,
	priv->rng.priv = (unsigned long)dev;

	ret = devm_hwrng_register(dev, &priv->rng);
	if (ret) {
		dev_err(dev, "bcmbca-trng registration failed!\n");
		goto err_exit;
	}

	dev_info(dev, "bcmbca-trng registered\n");
	return 0;

err_exit:
	return ret;
}

static const struct of_device_id bcmbca_trng_of_match[] = {
	{ .compatible = "brcm,bcmbca-trng", },
	{},
};
MODULE_DEVICE_TABLE(of, bcmbca_trng_of_match);

static struct platform_driver bcmbca_trng_driver = {
	.driver = {
		.name		= "bcmbca-trng",
		.of_match_table = bcmbca_trng_of_match,
	},
	.probe		= bcmbca_trng_probe,
	.remove		= bcmbca_trng_remove,
};
module_platform_driver(bcmbca_trng_driver);

MODULE_AUTHOR("Farhan Ali");
MODULE_DESCRIPTION("Broadcom BCA True Random Number Generator driver");
MODULE_LICENSE("GPL v2");

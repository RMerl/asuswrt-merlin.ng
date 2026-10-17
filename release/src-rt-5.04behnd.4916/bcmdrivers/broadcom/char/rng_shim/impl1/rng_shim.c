/*
<:copyright-BRCM:2023:DUAL/GPL:standard

   Copyright (c) 2023 Broadcom 
   All Rights Reserved

Unless you and Broadcom execute a separate written software license
agreement governing use of this software, this software is licensed
to you under the terms of the GNU General Public License version 2
(the "GPL"), available at http://www.broadcom.com/licenses/GPLv2.php,
with the following added to such license:

   As a special exception, the copyright holders of this software give
   you permission to link this software with independent modules, and
   to copy and distribute the resulting executable under terms of your
   choice, provided that you also meet, for each linked independent
   module, the terms and conditions of the license of that module.
   An independent module is a module which is not derived from this
   software.  The special exception does not apply to any modifications
   of the software.

Not withstanding the above, under no circumstances may you combine
this software in any way with any other Broadcom software provided
under a license other than the GPL, without Broadcom's express prior
written consent.

:>
*/
#include <linux/version.h>
#include <linux/hw_random.h>
#include <linux/init.h>
#include <linux/io.h>
#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/of_address.h>
#include <linux/of_platform.h>
#include <linux/platform_device.h>
#include <linux/delay.h>
#include <linux/arm-smccc.h>


#if(LINUX_VERSION_CODE < KERNEL_VERSION(5,15,0))

#define ARM_SMCCC_TRNG_MIN_VERSION			0x10000UL

#define ARM_SMCCC_TRNG_VERSION					\
	ARM_SMCCC_CALL_VAL(ARM_SMCCC_FAST_CALL,			\
			   ARM_SMCCC_SMC_32,			\
			   ARM_SMCCC_OWNER_STANDARD,		\
			   0x50)

#define ARM_SMCCC_TRNG_RND32					\
	ARM_SMCCC_CALL_VAL(ARM_SMCCC_FAST_CALL,			\
			   ARM_SMCCC_SMC_32,			\
			   ARM_SMCCC_OWNER_STANDARD,		\
			   0x53)

#define ARM_SMCCC_TRNG_RND64					\
	ARM_SMCCC_CALL_VAL(ARM_SMCCC_FAST_CALL,			\
			   ARM_SMCCC_SMC_64,			\
			   ARM_SMCCC_OWNER_STANDARD,		\
			   0x53)
#endif

#if defined(__aarch64__)
#define READ_RNG	ARM_SMCCC_TRNG_RND64
#define READ_SIZE	8
#else
#define READ_RNG	ARM_SMCCC_TRNG_RND32
#define READ_SIZE	4
#endif

struct rng_shim_dev {
	struct hwrng rng;
};

static bool smccc_rng_shim_available = false;

static int rng_shim_read(struct hwrng *rng, void *buf, size_t max, bool wait)
{
	uint32_t num_remaining = max;

	if (smccc_rng_shim_available) {
		struct arm_smccc_res smc_res;

		while (num_remaining > 0) {
			arm_smccc_1_1_smc(READ_RNG, READ_SIZE * 8, &smc_res);
			memcpy(buf, &smc_res.a3 , (num_remaining > READ_SIZE) ? READ_SIZE : num_remaining);
			num_remaining -= READ_SIZE;
			buf += READ_SIZE;
		}
	}
	return max;
}

static int  rng_shim_init(struct hwrng *rng) { return 0; }

static void rng_shim_cleanup(struct hwrng *rng) { }

static int  rng_shim_probe(struct platform_device *pdev)
{
	struct rng_shim_dev *priv;
	struct arm_smccc_res smc_res;
	struct device *dev = &pdev->dev;
	int ret = 0;

	arm_smccc_1_1_smc(ARM_SMCCC_TRNG_VERSION, &smc_res);
	if ((s32)smc_res.a0 < 0)
		smccc_rng_shim_available = false;
	else
		smccc_rng_shim_available = (smc_res.a0 >= ARM_SMCCC_TRNG_MIN_VERSION);

	if (smccc_rng_shim_available) {
	  	dev_info(dev, "Using hwrng shim\n");
	}
	else {
		dev_err(dev, "failed to get rng resources\n");
		return -EINVAL;
	}

	priv = devm_kzalloc(dev, sizeof(*priv), GFP_KERNEL);
	if (!priv)
		return -ENOMEM;

	priv->rng.name = "rng-shim",
	priv->rng.read = rng_shim_read,
	priv->rng.init = rng_shim_init,
	priv->rng.cleanup = rng_shim_cleanup,
	/* Register driver */
	ret = devm_hwrng_register(dev, &priv->rng);
	if (ret) {
		dev_err(dev, "hwrng shim registration failed\n");
		return ret;
	}

	dev_info(dev, "hwrng shim registered\n");

	return 0;
}

static const struct of_device_id rng_shim_of_match[] = {
 	{ .compatible = "brcm,rng-shim", },
	{},
};
MODULE_DEVICE_TABLE(of, rng_shim_of_match);

static struct platform_driver rng_shim_driver = {
	.driver = {
		.name		= "rng-shim",
		.of_match_table = rng_shim_of_match,
	},
	.probe	= rng_shim_probe,
};
module_platform_driver(rng_shim_driver);

MODULE_AUTHOR("Broadcom");
MODULE_DESCRIPTION("RNG SHIM Random Number Generator driver");
MODULE_LICENSE("GPL");

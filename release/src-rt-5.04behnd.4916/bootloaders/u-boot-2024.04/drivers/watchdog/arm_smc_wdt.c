// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * ARM Secure Monitor Call watchdog driver
 * Copyright (C) 2022, STMicroelectronics - All Rights Reserved
 * This file is based on Linux driver drivers/watchdog/arm_smc_wdt.c
 */

#define LOG_CATEGORY UCLASS_WDT

#include <dm.h>
#include <dm/device_compat.h>
#include <linux/arm-smccc.h>
#include <linux/psci.h>
#include <wdt.h>

#define DRV_NAME		"arm_smc_wdt"

#define WDT_TIMEOUT_SECS(TIMEOUT)	((TIMEOUT) / 1000)

enum smcwd_call {
	SMCWD_INIT		= 0,
	SMCWD_SET_TIMEOUT	= 1,
	SMCWD_ENABLE		= 2,
	SMCWD_PET		= 3,
	SMCWD_GET_TIMELEFT	= 4,
};

struct smcwd_priv_data {
	u32 smc_id;
	unsigned int min_timeout;
	unsigned int max_timeout;
};

static int smcwd_call(struct udevice *dev, enum smcwd_call call,
		      unsigned long arg, struct arm_smccc_res *res)
{
	struct smcwd_priv_data *priv = dev_get_priv(dev);
	struct arm_smccc_res local_res;

	if (!res)
		res = &local_res;

	arm_smccc_smc(priv->smc_id, call, arg, 0, 0, 0, 0, 0, res);

	if (res->a0 == PSCI_RET_NOT_SUPPORTED)
		return -ENODEV;
	if (res->a0 == PSCI_RET_INVALID_PARAMS)
		return -EINVAL;
	if (res->a0 != PSCI_RET_SUCCESS)
		return -EIO;

	return 0;
}

static int smcwd_reset(struct udevice *dev)
{
	return smcwd_call(dev, SMCWD_PET, 0, NULL);
}

static int smcwd_stop(struct udevice *dev)
{
	return smcwd_call(dev, SMCWD_ENABLE, 0, NULL);
}

static int smcwd_start(struct udevice *dev, u64 timeout_ms, ulong flags)
{
	struct smcwd_priv_data *priv = dev_get_priv(dev);
	u64 timeout_sec = WDT_TIMEOUT_SECS(timeout_ms);
	int err;

	if (timeout_sec < priv->min_timeout || timeout_sec > priv->max_timeout)	{
		dev_err(dev, "Timeout value not supported\n");
		return -EINVAL;
	}

	err = smcwd_call(dev, SMCWD_SET_TIMEOUT, timeout_sec, NULL);
	if (err) {
		dev_err(dev, "Timeout out configuration failed\n");
		return err;
	}

	return smcwd_call(dev, SMCWD_ENABLE, 1, NULL);
}

static int smcwd_probe(struct udevice *dev)
{
	struct smcwd_priv_data *priv = dev_get_priv(dev);
	struct arm_smccc_res res;
	int err;

	priv->smc_id = dev_read_u32_default(dev, "arm,smc-id", 0x82003D06);

	err = smcwd_call(dev, SMCWD_INIT, 0, &res);
	if (err < 0) {
		dev_err(dev, "Init failed %i\n", err);
		return err;
	}

	priv->min_timeout = res.a1;
	priv->max_timeout = res.a2;

	return 0;
}

static const struct wdt_ops smcwd_ops = {
	.start		= smcwd_start,
	.stop		= smcwd_stop,
	.reset		= smcwd_reset,
};

static const struct udevice_id smcwd_dt_ids[] = {
	{ .compatible = "arm,smc-wdt" },
	{}
};

U_BOOT_DRIVER(wdt_sandbox) = {
	.name = "smcwd",
	.id = UCLASS_WDT,
	.of_match = smcwd_dt_ids,
	.priv_auto = sizeof(struct smcwd_priv_data),
	.probe = smcwd_probe,
	.ops = &smcwd_ops,
};

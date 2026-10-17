// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright (c) 2015 Google, Inc
 */

#include <common.h>
#include <dm.h>
#include <errno.h>
#include <log.h>
#include <linux/delay.h>
#include <power/pmic.h>
#include <power/regulator.h>
#include <power/tps65090.h>

static int tps65090_fet_probe(struct udevice *dev)
{
	struct dm_regulator_uclass_plat *uc_pdata;

	uc_pdata = dev_get_uclass_plat(dev);

	uc_pdata->type = REGULATOR_TYPE_OTHER;
	uc_pdata->mode_count = 0;

	return 0;
}

static int tps65090_fet_get_enable(struct udevice *dev)
{
	struct udevice *pmic = dev_get_parent(dev);
	int ret, fet_id;

	fet_id = dev->driver_data;
	debug("%s: fet_id=%d\n", __func__, fet_id);

	ret = pmic_reg_read(pmic, REG_FET_BASE + fet_id);
	if (ret < 0)
		return ret;

	return ret & FET_CTRL_ENFET;
}

/**
 * Set the power state for a FET
 *
 * @param pmic		pmic structure for the tps65090
 * @param fet_id	FET number to set (1..MAX_FET_NUM)
 * @param set		1 to power on FET, 0 to power off
 * Return: -EIO if we got a comms error, -EAGAIN if the FET failed to
 * change state. If all is ok, returns 0.
 */
static int tps65090_fet_set(struct udevice *pmic, int fet_id, bool set)
{
	int retry;
	u32 value;
	int ret;

	value = FET_CTRL_ADENFET | FET_CTRL_WAIT;
	if (set)
		value |= FET_CTRL_ENFET;

	if (pmic_reg_write(pmic, REG_FET_BASE + fet_id, value))
		return -EIO;

	/* Try reading until we get a result */
	for (retry = 0; retry < MAX_CTRL_READ_TRIES; retry++) {
		ret = pmic_reg_read(pmic, REG_FET_BASE + fet_id);
		if (ret < 0)
			return ret;

		/* Check that the FET went into the expected state */
		debug("%s: flags=%x\n", __func__, ret);
		if (!!(ret & FET_CTRL_PGFET) == set)
			return 0;

		/* If we got a timeout, there is no point in waiting longer */
		if (ret & FET_CTRL_TOFET)
			break;

		mdelay(1);
	}

	debug("FET %d: Power good should have set to %d but reg=%#02x\n",
	      fet_id, set, ret);
	return -EAGAIN;
}

static int tps65090_fet_set_enable(struct udevice *dev, bool enable)
{
	struct udevice *pmic = dev_get_parent(dev);
	int ret, fet_id;
	ulong start;
	int loops;

	fet_id = dev->driver_data;
	debug("%s: fet_id=%d, enable=%d\n", __func__, fet_id, enable);

	start = get_timer(0);
	for (loops = 0;; loops++) {
		ret = tps65090_fet_set(pmic, fet_id, enable);
		if (!ret)
			break;

		if (get_timer(start) > 100)
			break;

		/* Turn it off and try again until we time out */
		tps65090_fet_set(pmic, fet_id, false);
	}

	if (ret)
		debug("%s: FET%d failed to power on: time=%lums, loops=%d\n",
		      __func__, fet_id, get_timer(start), loops);
	else if (loops)
		debug("%s: FET%d powered on after %lums, loops=%d\n",
		      __func__, fet_id, get_timer(start), loops);

	/*
	 * Unfortunately there are some conditions where the power-good bit
	 * will be 0, but the FET still comes up. One such case occurs with
	 * the LCD backlight on snow. We'll just return 0 here and assume
	 * that the FET will eventually come up.
	 */
	if (ret == -EAGAIN)
		ret = 0;

	return ret;
}

static const struct dm_regulator_ops tps65090_fet_ops = {
	.get_enable = tps65090_fet_get_enable,
	.set_enable = tps65090_fet_set_enable,
};

U_BOOT_DRIVER(tps65090_fet) = {
	.name = TPS65090_FET_DRIVER,
	.id = UCLASS_REGULATOR,
	.ops = &tps65090_fet_ops,
	.probe = tps65090_fet_probe,
};

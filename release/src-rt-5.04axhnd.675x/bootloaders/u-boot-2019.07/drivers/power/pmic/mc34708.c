// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright (C) 2018
 * Lukasz Majewski, DENX Software Engineering, lukma@denx.de
 *
 */

#include <common.h>
#include <dm.h>
#include <errno.h>
#include <fsl_pmic.h>
#include <i2c.h>
#include <power/pmic.h>

DECLARE_GLOBAL_DATA_PTR;

static int mc34708_reg_count(struct udevice *dev)
{
	return PMIC_NUM_OF_REGS;
}

static int mc34708_write(struct udevice *dev, uint reg, const u8 *buff,
			 int len)
{
	u8 buf[3] = { 0 };
	int ret;

	if (len != MC34708_TRANSFER_SIZE)
		return -EINVAL;

	/*
	 * The MC34708 sends data with big endian format, hence we need to
	 * perform manual byte swap.
	 */
	buf[0] = buff[2];
	buf[1] = buff[1];
	buf[2] = buff[0];

	ret = dm_i2c_write(dev, reg, buf, len);
	if (ret)
		printf("write error to device: %p register: %#x!\n", dev, reg);

	return ret;
}

static int mc34708_read(struct udevice *dev, uint reg, u8 *buff, int len)
{
	u8 buf[3] = { 0 };
	int ret;

	if (len != MC34708_TRANSFER_SIZE)
		return -EINVAL;

	ret = dm_i2c_read(dev, reg, buf, len);
	if (ret)
		printf("read error from device: %p register: %#x!\n", dev, reg);

	buff[0] = buf[2];
	buff[1] = buf[1];
	buff[2] = buf[0];

	return ret;
}

static int mc34708_probe(struct udevice *dev)
{
	struct uc_pmic_priv *priv = dev_get_uclass_priv(dev);

	priv->trans_len = MC34708_TRANSFER_SIZE;

	/*
	 * Handle PMIC Errata 37: APS mode not fully functional,
	 * use explicit PWM or PFM instead
	 */
	pmic_clrsetbits(dev, MC34708_REG_SW12_OPMODE,
			MC34708_SW1AMODE_MASK | MC34708_SW2MODE_MASK,
			SW_MODE_PWMPWM | (SW_MODE_PWMPWM << 14u));

	pmic_clrsetbits(dev, MC34708_REG_SW345_OPMODE,
			MC34708_SW3MODE_MASK | MC34708_SW4AMODE_MASK |
			MC34708_SW4BMODE_MASK | MC34708_SW5MODE_MASK,
			SW_MODE_PWMPWM | (SW_MODE_PWMPWM << 6u) |
			(SW_MODE_PWMPWM << 12u) | (SW_MODE_PWMPWM << 18u));

	return 0;
}

static struct dm_pmic_ops mc34708_ops = {
	.reg_count = mc34708_reg_count,
	.read	= mc34708_read,
	.write	= mc34708_write,
};

static const struct udevice_id mc34708_ids[] = {
	{ .compatible = "fsl,mc34708" },
	{ }
};

U_BOOT_DRIVER(pmic_mc34708) = {
	.name		= "mc34708_pmic",
	.id		= UCLASS_PMIC,
	.of_match	= mc34708_ids,
	.probe          = mc34708_probe,
	.ops		= &mc34708_ops,
};

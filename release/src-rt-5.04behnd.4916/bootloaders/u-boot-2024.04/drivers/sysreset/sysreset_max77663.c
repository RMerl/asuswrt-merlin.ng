// SPDX-License-Identifier: GPL-2.0+
/*
 *  Copyright(C) 2023 Svyatoslav Ryhel <clamor95@gmail.com>
 */

#include <dm.h>
#include <i2c.h>
#include <errno.h>
#include <sysreset.h>
#include <power/pmic.h>
#include <power/max77663.h>

static int max77663_sysreset_request(struct udevice *dev,
				     enum sysreset_t type)
{
	int val;

	val = pmic_reg_read(dev->parent, MAX77663_REG_ONOFF_CFG1);
	if (val < 0)
		return val;

	/* clear both bits */
	val &= ~ONOFF_SFT_RST;
	val &= ~ONOFF_PWR_OFF;

	switch (type) {
	case SYSRESET_POWER:
		/* MAX77663: SFT_RST > ONOFF_CFG1 */
		pmic_reg_write(dev->parent, MAX77663_REG_ONOFF_CFG1,
			       val | ONOFF_SFT_RST);
		break;
	case SYSRESET_POWER_OFF:
		/* MAX77663: PWR_OFF > ONOFF_CFG1 */
		pmic_reg_write(dev->parent, MAX77663_REG_ONOFF_CFG1,
			       val | ONOFF_PWR_OFF);
		break;
	default:
		return -EPROTONOSUPPORT;
	}

	return -EINPROGRESS;
}

static struct sysreset_ops max77663_sysreset = {
	.request = max77663_sysreset_request,
};

U_BOOT_DRIVER(sysreset_max77663) = {
	.id	= UCLASS_SYSRESET,
	.name	= MAX77663_RST_DRIVER,
	.ops	= &max77663_sysreset,
};

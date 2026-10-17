// SPDX-License-Identifier: GPL-2.0+
/*
 *  Copyright(C) 2023 Svyatoslav Ryhel <clamor95@gmail.com>
 */

#include <dm.h>
#include <i2c.h>
#include <errno.h>
#include <sysreset.h>
#include <power/pmic.h>
#include <power/tps80031.h>

static int tps80031_sysreset_request(struct udevice *dev,
				     enum sysreset_t type)
{
	switch (type) {
	case SYSRESET_POWER:
		/* TPS80031: SW_RESET > PHOENIX_DEV_ON */
		pmic_reg_write(dev->parent, TPS80031_PHOENIX_DEV_ON, SW_RESET);
		break;
	case SYSRESET_POWER_OFF:
		/* TPS80031: DEVOFF > PHOENIX_DEV_ON */
		pmic_reg_write(dev->parent, TPS80031_PHOENIX_DEV_ON, DEVOFF);
		break;
	default:
		return -EPROTONOSUPPORT;
	}

	return -EINPROGRESS;
}

static struct sysreset_ops tps80031_sysreset = {
	.request = tps80031_sysreset_request,
};

U_BOOT_DRIVER(sysreset_tps80031) = {
	.id	= UCLASS_SYSRESET,
	.name	= TPS80031_RST_DRIVER,
	.ops	= &tps80031_sysreset,
};

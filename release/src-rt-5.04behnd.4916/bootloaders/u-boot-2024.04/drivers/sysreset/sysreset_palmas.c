// SPDX-License-Identifier: GPL-2.0+
/*
 *  Copyright(C) 2023 Svyatoslav Ryhel <clamor95@gmail.com>
 */

#include <dm.h>
#include <i2c.h>
#include <errno.h>
#include <sysreset.h>
#include <power/pmic.h>
#include <power/palmas.h>

static int palmas_sysreset_request(struct udevice *dev,
				   enum sysreset_t type)
{
	struct palmas_priv *priv = dev_get_priv(dev->parent);
	int ret;

	/*
	 * Mask INT3 on second page which detects vbus
	 * or device will immediately turn on.
	 */
	ret = dm_i2c_reg_clrset(priv->chip2, PALMAS_INT3_MASK,
				MASK_VBUS, MASK_VBUS);
	if (ret < 0)
		return ret;

	switch (type) {
	case SYSRESET_POWER:
		/* PALMAS: SW_RST > DEV_CTRL */
		pmic_reg_write(dev->parent, PALMAS_DEV_CTRL, SW_RST);
		break;
	case SYSRESET_POWER_OFF:
		/* PALMAS: DEV_OFF > DEV_CTRL */
		pmic_reg_write(dev->parent, PALMAS_DEV_CTRL, DEV_OFF);
		break;
	default:
		return -EPROTONOSUPPORT;
	}

	return -EINPROGRESS;
}

static struct sysreset_ops palmas_sysreset = {
	.request = palmas_sysreset_request,
};

U_BOOT_DRIVER(sysreset_palmas) = {
	.id	= UCLASS_SYSRESET,
	.name	= PALMAS_RST_DRIVER,
	.ops	= &palmas_sysreset,
};

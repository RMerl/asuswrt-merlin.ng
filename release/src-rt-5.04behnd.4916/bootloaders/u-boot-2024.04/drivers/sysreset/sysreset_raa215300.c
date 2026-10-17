// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright (C) 2023 Renesas Electronics Corporation
 */

#include <dm.h>
#include <power/pmic.h>
#include <sysreset.h>

#define RAA215300_REG_SWRESET	0x6D
#define RAA215300_COLD_RESET	BIT(0)
#define RAA215300_WARM_RESET	BIT(1)

static int raa215300_sysreset_request(struct udevice *dev, enum sysreset_t type)
{
	struct udevice *pmic = dev_get_parent(dev);
	int ret;
	u8 val;

	/*
	 * The RAA215300 documentation names the available reset types
	 * differently to u-boot:
	 *
	 *   - A "warm" reset via the RAA215300 PMIC will fully reset the SoC
	 *     (CPU & GPIOs), so this corresponds to SYSRESET_COLD.
	 *
	 *   - A "cold" reset via the RAA215300 PMIC will cycle all power supply
	 *     rails, so this corresponds to SYSRESET_POWER.
	 */
	switch (type) {
	case SYSRESET_COLD:
		val = RAA215300_WARM_RESET;
		break;

	case SYSRESET_POWER:
		val = RAA215300_COLD_RESET;
		break;

	default:
		return -EPROTONOSUPPORT;
	}

	ret = pmic_reg_write(pmic, RAA215300_REG_SWRESET, val);
	if (ret)
		return ret;

	return -EINPROGRESS;
}

static struct sysreset_ops raa215300_sysreset_ops = {
	.request = raa215300_sysreset_request,
};

U_BOOT_DRIVER(raa215300_sysreset) = {
	.name = "raa215300_sysreset",
	.id = UCLASS_SYSRESET,
	.ops = &raa215300_sysreset_ops,
};

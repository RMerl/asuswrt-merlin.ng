// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright (c), Vaisala Oyj
 */

#include <common.h>
#include <dm.h>
#include <dm/device_compat.h>
#include <reboot-mode/reboot-mode-rtc.h>
#include <reboot-mode/reboot-mode.h>
#include <rtc.h>

DECLARE_GLOBAL_DATA_PTR;

static int reboot_mode_get(struct udevice *dev, u32 *buf)
{
	if (!buf)
		return -EINVAL;

	int ret;
	u8 *val = (u8 *)buf;
	struct reboot_mode_rtc_platdata *plat_data;

	plat_data = dev_get_plat(dev);
	if (!plat_data)
		return -EINVAL;

	for (int i = 0; i < plat_data->size; i++) {
		ret = rtc_read8(plat_data->rtc, plat_data->addr + i);
		if (ret < 0)
			return ret;

		val[i] = ret;
	}

	if (plat_data->is_big_endian)
		*buf = __be32_to_cpu(*buf);
	else
		*buf = __le32_to_cpu(*buf);

	return 0;
}

static int reboot_mode_set(struct udevice *dev, u32 buf)
{
	int ret;
	u8 *val;
	struct reboot_mode_rtc_platdata *plat_data;

	plat_data = dev_get_plat(dev);
	if (!plat_data)
		return -EINVAL;

	if (plat_data->is_big_endian)
		buf = __cpu_to_be32(buf);
	else
		buf = __cpu_to_le32(buf);

	val = (u8 *)&buf;

	for (int i = 0; i < plat_data->size; i++) {
		ret = rtc_write8(plat_data->rtc, (plat_data->addr + i), val[i]);
		if (ret < 0)
			return ret;
	}

	return 0;
}

#if CONFIG_IS_ENABLED(OF_CONTROL)
static int reboot_mode_ofdata_to_platdata(struct udevice *dev)
{
	struct ofnode_phandle_args phandle_args;
	struct reboot_mode_rtc_platdata *plat_data;

	plat_data = dev_get_plat(dev);
	if (!plat_data)
		return -EINVAL;

	if (dev_read_phandle_with_args(dev, "rtc", NULL, 0, 0, &phandle_args)) {
		dev_err(dev, "RTC device not specified\n");
		return -ENOENT;
	}

	if (uclass_get_device_by_ofnode(UCLASS_RTC, phandle_args.node,
					&plat_data->rtc)) {
		dev_err(dev, "could not get the RTC device\n");
		return -ENODEV;
	}

	plat_data->addr =
		dev_read_addr_size_index(dev, 0, (fdt_size_t *)&plat_data->size);
	if (plat_data->addr == FDT_ADDR_T_NONE) {
		dev_err(dev, "Invalid RTC address\n");
		return -EINVAL;
	}
	if (plat_data->size > sizeof(u32)) {
		dev_err(dev, "Invalid reg size\n");
		return -EINVAL;
	}

	plat_data->is_big_endian = ofnode_read_bool(dev_ofnode(dev), "big-endian");

	return 0;
}

static const struct udevice_id reboot_mode_ids[] = {
	{ .compatible = "reboot-mode-rtc", 0 },
	{}
};
#endif

static const struct reboot_mode_ops reboot_mode_rtc_ops = {
	.get = reboot_mode_get,
	.set = reboot_mode_set,
};

U_BOOT_DRIVER(reboot_mode_rtc) = {
	.name = "reboot-mode-rtc",
	.id = UCLASS_REBOOT_MODE,
#if CONFIG_IS_ENABLED(OF_CONTROL)
	.of_match = reboot_mode_ids,
	.of_to_plat = reboot_mode_ofdata_to_platdata,
#endif
	.plat_auto = sizeof(struct reboot_mode_rtc_platdata),
	.ops = &reboot_mode_rtc_ops,
};

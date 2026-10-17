// SPDX-License-Identifier: GPL-2.0+

#include <common.h>
#include <dm.h>
#include <init.h>
#include <sysinfo.h>
#include <asm/global_data.h>
#include <linux/libfdt.h>
#include <linux/compiler.h>

DECLARE_GLOBAL_DATA_PTR;

int __weak checkboard(void)
{
	return 0;
}

static const struct to_show {
	const char *name;
	enum sysinfo_id id;
} to_show[] = {
	{ "Manufacturer", SYSINFO_ID_BOARD_MANUFACTURER},
	{ "Prior-stage version", SYSINFO_ID_PRIOR_STAGE_VERSION },
	{ "Prior-stage date", SYSINFO_ID_PRIOR_STAGE_DATE },
	{ /* sentinel */ }
};

static int try_sysinfo(void)
{
	struct udevice *dev;
	char str[80];
	int ret;

	/* This might provide more detail */
	ret = sysinfo_get(&dev);
	if (ret)
		return ret;

	ret = sysinfo_detect(dev);
	if (ret)
		return ret;

	ret = sysinfo_get_str(dev, SYSINFO_ID_BOARD_MODEL, sizeof(str), str);
	if (ret)
		return ret;
	printf("Model: %s\n", str);

	if (IS_ENABLED(CONFIG_SYSINFO_EXTRA)) {
		const struct to_show *item;

		for (item = to_show; item->id; item++) {
			ret = sysinfo_get_str(dev, item->id, sizeof(str), str);
			if (!ret)
				printf("%s: %s\n", item->name, str);
		}
	}

	return 0;
}

int show_board_info(void)
{
	if (IS_ENABLED(CONFIG_OF_CONTROL)) {
		int ret = -ENOSYS;

		if (IS_ENABLED(CONFIG_SYSINFO))
			ret = try_sysinfo();

		/* Fail back to the main 'model' if available */
		if (ret) {
			const char *model;

			model = fdt_getprop(gd->fdt_blob, 0, "model", NULL);
			if (model)
				printf("Model: %s\n", model);
		}
	}

	return checkboard();
}

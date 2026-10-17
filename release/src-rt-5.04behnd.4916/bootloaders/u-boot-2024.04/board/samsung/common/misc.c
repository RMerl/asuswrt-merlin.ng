// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright (C) 2013 Samsung Electronics
 * Przemyslaw Marczak <p.marczak@samsung.com>
 */

#include <common.h>
#include <command.h>
#include <env.h>
#include <libtizen.h>
#include <asm/global_data.h>
#include <linux/delay.h>
#include <linux/printk.h>
#include <samsung/misc.h>
#include <errno.h>
#include <version.h>
#include <malloc.h>
#include <memalign.h>
#include <linux/sizes.h>
#include <asm/arch/cpu.h>
#include <asm/gpio.h>
#include <linux/input.h>
#include <dm.h>
/*
 * Use #ifdef to work around conflicting headers while we wait for this to be
 * converted to driver model.
 */
#ifdef CONFIG_DM_PMIC_MAX77686
#include <power/max77686_pmic.h>
#endif
#ifdef CONFIG_DM_PMIC_MAX8998
#include <power/max8998_pmic.h>
#endif
#ifdef CONFIG_PMIC_MAX8997
#include <power/max8997_pmic.h>
#endif
#include <power/pmic.h>
#include <mmc.h>

DECLARE_GLOBAL_DATA_PTR;

#ifdef CONFIG_SET_DFU_ALT_INFO
void set_dfu_alt_info(char *interface, char *devstr)
{
	size_t buf_size = CFG_SET_DFU_ALT_BUF_LEN;
	ALLOC_CACHE_ALIGN_BUFFER(char, buf, buf_size);
	char *alt_info = "Settings not found!";
	char *status = "error!\n";
	char *alt_setting;
	char *alt_sep;
	int offset = 0;

	puts("DFU alt info setting: ");

	alt_setting = get_dfu_alt_boot(interface, devstr);
	if (alt_setting) {
		env_set("dfu_alt_boot", alt_setting);
		offset = snprintf(buf, buf_size, "%s", alt_setting);
	}

	alt_setting = get_dfu_alt_system(interface, devstr);
	if (alt_setting) {
		if (offset)
			alt_sep = ";";
		else
			alt_sep = "";

		offset += snprintf(buf + offset, buf_size - offset,
				    "%s%s", alt_sep, alt_setting);
	}

	if (offset) {
		alt_info = buf;
		status = "done\n";
	}

	env_set("dfu_alt_info", alt_info);
	puts(status);
}
#endif

#ifdef CONFIG_ENV_VARS_UBOOT_RUNTIME_CONFIG
void set_board_info(void)
{
	char info[64];

	snprintf(info, ARRAY_SIZE(info), "%u.%u", (s5p_cpu_rev & 0xf0) >> 4,
		 s5p_cpu_rev & 0xf);
	env_set("soc_rev", info);

	snprintf(info, ARRAY_SIZE(info), "%x", s5p_cpu_id);
	env_set("soc_id", info);

#ifdef CONFIG_REVISION_TAG
	snprintf(info, ARRAY_SIZE(info), "%x", get_board_rev());
	env_set("board_rev", info);
#endif
#ifdef CONFIG_OF_LIBFDT
	const char *bdtype = "";
	const char *bdname = CONFIG_SYS_BOARD;

#ifdef CONFIG_BOARD_TYPES
	bdtype = get_board_type();
	if (!bdtype)
		bdtype = "";

	sprintf(info, "%s%s", bdname, bdtype);
	env_set("board_name", info);
#endif
	snprintf(info, ARRAY_SIZE(info),  "%s%x-%s%s.dtb",
		 CONFIG_SYS_SOC, s5p_cpu_id, bdname, bdtype);
	env_set("fdtfile", info);
#endif
}
#endif /* CONFIG_ENV_VARS_UBOOT_RUNTIME_CONFIG */

#ifdef CONFIG_CMD_BMP
void draw_logo(void)
{
	int x, y;
	ulong addr;

	addr = panel_info.logo_addr;
	if (!addr) {
		pr_err("There is no logo data.\n");
		return;
	}

	if (panel_info.vl_width >= panel_info.logo_width) {
		x = ((panel_info.vl_width - panel_info.logo_width) >> 1);
		x += panel_info.logo_x_offset; /* For X center align */
	} else {
		x = 0;
		printf("Warning: image width is bigger than display width\n");
	}

	if (panel_info.vl_height >= panel_info.logo_height) {
		y = ((panel_info.vl_height - panel_info.logo_height) >> 1);
		y += panel_info.logo_y_offset; /* For Y center align */
	} else {
		y = 0;
		printf("Warning: image height is bigger than display height\n");
	}

	bmp_display(addr, x, y);
}
#endif /* CONFIG_CMD_BMP */

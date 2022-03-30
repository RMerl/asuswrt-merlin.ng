// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright (C) 2015-2017 Socionext Inc.
 *   Author: Masahiro Yamada <yamada.masahiro@socionext.com>
 */

#include <common.h>
#include <spl.h>
#include <stdio.h>
#include <linux/log2.h>

#include "../init.h"
#include "../sbc/sbc-regs.h"
#include "../sg-regs.h"
#include "../soc-info.h"
#include "boot-device.h"

struct uniphier_boot_device_info {
	unsigned int soc_id;
	unsigned int boot_device_sel_shift;
	const struct uniphier_boot_device *boot_device_table;
	const unsigned int *boot_device_count;
	int (*boot_device_is_usb)(u32 pinmon);
	unsigned int (*boot_device_fixup)(unsigned int mode);
	int have_internal_stm;
};

static const struct uniphier_boot_device_info uniphier_boot_device_info[] = {
#if defined(CONFIG_ARCH_UNIPHIER_LD4)
	{
		.soc_id = UNIPHIER_LD4_ID,
		.boot_device_sel_shift = 1,
		.boot_device_table = uniphier_ld4_boot_device_table,
		.boot_device_count = &uniphier_ld4_boot_device_count,
		.have_internal_stm = 1,
	},
#endif
#if defined(CONFIG_ARCH_UNIPHIER_PRO4)
	{
		.soc_id = UNIPHIER_PRO4_ID,
		.boot_device_sel_shift = 1,
		.boot_device_table = uniphier_ld4_boot_device_table,
		.boot_device_count = &uniphier_ld4_boot_device_count,
		.have_internal_stm = 0,
	},
#endif
#if defined(CONFIG_ARCH_UNIPHIER_SLD8)
	{
		.soc_id = UNIPHIER_SLD8_ID,
		.boot_device_sel_shift = 1,
		.boot_device_table = uniphier_ld4_boot_device_table,
		.boot_device_count = &uniphier_ld4_boot_device_count,
		.have_internal_stm = 1,
	},
#endif
#if defined(CONFIG_ARCH_UNIPHIER_PRO5)
	{
		.soc_id = UNIPHIER_PRO5_ID,
		.boot_device_sel_shift = 1,
		.boot_device_table = uniphier_pro5_boot_device_table,
		.boot_device_count = &uniphier_pro5_boot_device_count,
		.have_internal_stm = 0,
	},
#endif
#if defined(CONFIG_ARCH_UNIPHIER_PXS2)
	{
		.soc_id = UNIPHIER_PXS2_ID,
		.boot_device_sel_shift = 1,
		.boot_device_table = uniphier_pxs2_boot_device_table,
		.boot_device_count = &uniphier_pxs2_boot_device_count,
		.boot_device_is_usb = uniphier_pxs2_boot_device_is_usb,
		.boot_device_fixup = uniphier_pxs2_boot_device_fixup,
		.have_internal_stm = 0,
	},
#endif
#if defined(CONFIG_ARCH_UNIPHIER_LD6B)
	{
		.soc_id = UNIPHIER_LD6B_ID,
		.boot_device_sel_shift = 1,
		.boot_device_table = uniphier_pxs2_boot_device_table,
		.boot_device_count = &uniphier_pxs2_boot_device_count,
		.boot_device_is_usb = uniphier_pxs2_boot_device_is_usb,
		.boot_device_fixup = uniphier_pxs2_boot_device_fixup,
		.have_internal_stm = 1,	/* STM on A-chip */
	},
#endif
#if defined(CONFIG_ARCH_UNIPHIER_LD11)
	{
		.soc_id = UNIPHIER_LD11_ID,
		.boot_device_sel_shift = 1,
		.boot_device_table = uniphier_ld11_boot_device_table,
		.boot_device_count = &uniphier_ld11_boot_device_count,
		.boot_device_is_usb = uniphier_ld11_boot_device_is_usb,
		.boot_device_fixup = uniphier_ld11_boot_device_fixup,
		.have_internal_stm = 1,
	},
#endif
#if defined(CONFIG_ARCH_UNIPHIER_LD20)
	{
		.soc_id = UNIPHIER_LD20_ID,
		.boot_device_sel_shift = 1,
		.boot_device_table = uniphier_ld11_boot_device_table,
		.boot_device_count = &uniphier_ld11_boot_device_count,
		.boot_device_is_usb = uniphier_ld20_boot_device_is_usb,
		.boot_device_fixup = uniphier_ld11_boot_device_fixup,
		.have_internal_stm = 1,
	},
#endif
#if defined(CONFIG_ARCH_UNIPHIER_PXS3)
	{
		.soc_id = UNIPHIER_PXS3_ID,
		.boot_device_sel_shift = 1,
		.boot_device_table = uniphier_pxs3_boot_device_table,
		.boot_device_count = &uniphier_pxs3_boot_device_count,
		.boot_device_is_usb = uniphier_pxs3_boot_device_is_usb,
		.have_internal_stm = 0,
	},
#endif
};
UNIPHIER_DEFINE_SOCDATA_FUNC(uniphier_get_boot_device_info,
			     uniphier_boot_device_info)

static unsigned int __uniphier_boot_device_raw(
				const struct uniphier_boot_device_info *info)
{
	u32 pinmon;
	unsigned int boot_sel;

	if (boot_is_swapped())
		return BOOT_DEVICE_NOR;

	pinmon = readl(SG_PINMON0);

	if (info->boot_device_is_usb && info->boot_device_is_usb(pinmon))
		return BOOT_DEVICE_USB;

	boot_sel = pinmon >> info->boot_device_sel_shift;

	BUG_ON(!is_power_of_2(*info->boot_device_count));
	boot_sel &= *info->boot_device_count - 1;

	return info->boot_device_table[boot_sel].boot_device;
}

unsigned int uniphier_boot_device_raw(void)
{
	const struct uniphier_boot_device_info *info;

	info = uniphier_get_boot_device_info();
	if (!info) {
		pr_err("unsupported SoC\n");
		return BOOT_DEVICE_NONE;
	}

	return __uniphier_boot_device_raw(info);
}

u32 spl_boot_device(void)
{
	const struct uniphier_boot_device_info *info;
	u32 raw_mode;

	info = uniphier_get_boot_device_info();
	if (!info) {
		pr_err("unsupported SoC\n");
		return BOOT_DEVICE_NONE;
	}

	raw_mode = __uniphier_boot_device_raw(info);

	return info->boot_device_fixup ?
				info->boot_device_fixup(raw_mode) : raw_mode;
}

int uniphier_have_internal_stm(void)
{
	const struct uniphier_boot_device_info *info;

	info = uniphier_get_boot_device_info();
	if (!info) {
		pr_err("unsupported SoC\n");
		return -ENOTSUPP;
	}

	return info->have_internal_stm;
}

int uniphier_boot_from_backend(void)
{
	return !!(readl(SG_PINMON0) & BIT(27));
}

#ifndef CONFIG_SPL_BUILD

static int do_pinmon(cmd_tbl_t *cmdtp, int flag, int argc, char * const argv[])
{
	const struct uniphier_boot_device_info *info;
	u32 pinmon;
	unsigned int boot_device_count, boot_sel;
	int i;

	info = uniphier_get_boot_device_info();
	if (!info) {
		pr_err("unsupported SoC\n");
		return CMD_RET_FAILURE;
	}

	if (uniphier_have_internal_stm())
		printf("STB Micon: %s\n",
		       uniphier_boot_from_backend() ? "OFF" : "ON");

	printf("Boot Swap: %s\n", boot_is_swapped() ? "ON" : "OFF");

	pinmon = readl(SG_PINMON0);

	if (info->boot_device_is_usb)
		printf("USB Boot:  %s\n",
		       info->boot_device_is_usb(pinmon) ? "ON" : "OFF");

	boot_device_count = *info->boot_device_count;

	boot_sel = pinmon >> info->boot_device_sel_shift;
	boot_sel &= boot_device_count - 1;

	printf("\nBoot Mode Sel:\n");
	for (i = 0; i < boot_device_count; i++)
		printf(" %c %02x %s\n", i == boot_sel ? '*' : ' ', i,
		       info->boot_device_table[i].desc);

	return CMD_RET_SUCCESS;
}

U_BOOT_CMD(
	pinmon,	1,	1,	do_pinmon,
	"pin monitor",
	""
);

#endif /* !CONFIG_SPL_BUILD */

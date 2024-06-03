// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright (C) 2016 Amarula Solutions B.V.
 * Copyright (C) 2016 Engicam S.r.l.
 * Author: Jagan Teki <jagan@amarulasolutions.com>
 */

#include <common.h>
#include <mmc.h>
#include <asm/arch/sys_proto.h>
#include <watchdog.h>

#include "board.h"

DECLARE_GLOBAL_DATA_PTR;

#ifdef CONFIG_ENV_IS_IN_MMC
static void mmc_late_init(void)
{
	char cmd[32];
	char mmcblk[32];
	u32 dev_no = mmc_get_env_dev();

	env_set_ulong("mmcdev", dev_no);

	/* Set mmcblk env */
	sprintf(mmcblk, "/dev/mmcblk%dp2 rootwait rw", dev_no);
	env_set("mmcroot", mmcblk);

	sprintf(cmd, "mmc dev %d", dev_no);
	run_command(cmd, 0);
}
#endif

static void setenv_fdt_file(void)
{
	const char *cmp_dtb = CONFIG_DEFAULT_DEVICE_TREE;

	if (!strcmp(cmp_dtb, "imx6q-icore")) {
		if (is_mx6dq())
			env_set("fdt_file", "imx6q-icore.dtb");
		else if (is_mx6dl() || is_mx6solo())
			env_set("fdt_file", "imx6dl-icore.dtb");
	} else if (!strcmp(cmp_dtb, "imx6q-icore-mipi")) {
		if (is_mx6dq())
			env_set("fdt_file", "imx6q-icore-mipi.dtb");
		else if (is_mx6dl() || is_mx6solo())
			env_set("fdt_file", "imx6dl-icore-mipi.dtb");
	} else if (!strcmp(cmp_dtb, "imx6q-icore-rqs")) {
		if (is_mx6dq())
			env_set("fdt_file", "imx6q-icore-rqs.dtb");
		else if (is_mx6dl() || is_mx6solo())
			env_set("fdt_file", "imx6dl-icore-rqs.dtb");
	} else if (!strcmp(cmp_dtb, "imx6ul-geam"))
		env_set("fdt_file", "imx6ul-geam.dtb");
	else if (!strcmp(cmp_dtb, "imx6ul-isiot-mmc"))
		env_set("fdt_file", "imx6ul-isiot-emmc.dtb");
	else if (!strcmp(cmp_dtb, "imx6ul-isiot-emmc"))
		env_set("fdt_file", "imx6ul-isiot-emmc.dtb");
	else if (!strcmp(cmp_dtb, "imx6ul-isiot-nand"))
		env_set("fdt_file", "imx6ul-isiot-nand.dtb");
}

int board_late_init(void)
{
	switch ((imx6_src_get_boot_mode() & IMX6_BMODE_MASK) >>
			IMX6_BMODE_SHIFT) {
	case IMX6_BMODE_SD:
	case IMX6_BMODE_ESD:
	case IMX6_BMODE_MMC:
	case IMX6_BMODE_EMMC:
#ifdef CONFIG_ENV_IS_IN_MMC
		mmc_late_init();
#endif
		env_set("modeboot", "mmcboot");
		break;
	case IMX6_BMODE_NAND_MIN ... IMX6_BMODE_NAND_MAX:
		env_set("modeboot", "nandboot");
		break;
	default:
		env_set("modeboot", "");
		break;
	}

	if (is_mx6ul())
		env_set("console", "ttymxc0");
	else
		env_set("console", "ttymxc3");

	setenv_fdt_file();

#ifdef CONFIG_HW_WATCHDOG
	hw_watchdog_init();
#endif

	return 0;
}

int board_init(void)
{
	/* Address of boot parameters */
	gd->bd->bi_boot_params = PHYS_SDRAM + 0x100;

#ifdef CONFIG_NAND_MXS
	setup_gpmi_nand();
#endif

#ifdef CONFIG_VIDEO_IPUV3
	setup_display();
#endif

	return 0;
}

int dram_init(void)
{
	gd->ram_size = imx_ddr_size();

	return 0;
}

// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright (C) 2016 Amarula Solutions B.V.
 * Copyright (C) 2016 Engicam S.r.l.
 * Author: Jagan Teki <jagan@amarulasolutions.com>
 */

#include <common.h>
#include <command.h>
#include <env.h>
#include <hang.h>
#include <init.h>
#include <log.h>
#include <mmc.h>
#include <asm/arch/clock.h>
#include <asm/arch/imx-regs.h>
#include <asm/arch/sys_proto.h>
#include <watchdog.h>
#include <asm/global_data.h>

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

enum engicam_boards {
	IMX6Q_ICORE,
	IMX6DL_ICORE,
	IMX6Q_ICORE_MIPI,
	IMX6DL_ICORE_MIPI,
	IMX6Q_ICORE_RQS,
	IMX6DL_ICORE_RQS,
	IMX6UL_GEAM,
	IMX6UL_ISIOT_EMMC,
	IMX6UL_ISIOT_NAND,
	ENGICAM_BOARDS,
};

static const char * const board_fdt_file[ENGICAM_BOARDS] = {
	[IMX6Q_ICORE] = "imx6q-icore.dtb",
	[IMX6DL_ICORE] = "imx6dl-icore.dtb",
	[IMX6Q_ICORE_MIPI] = "imx6q-icore-mipi.dtb",
	[IMX6DL_ICORE_MIPI] = "imx6dl-icore-mipi.dtb",
	[IMX6Q_ICORE_RQS] = "imx6q-icore-rqs.dtb",
	[IMX6DL_ICORE_RQS] = "imx6dl-icore-rqs.dtb",
	[IMX6UL_GEAM] = "imx6ul-geam.dtb",
	[IMX6UL_ISIOT_EMMC] = "imx6ul-isiot-emmc.dtb",
	[IMX6UL_ISIOT_NAND] = "imx6ul-isiot-nand.dtb",
};

static int setenv_fdt_file(int board_detected)
{
	if (board_detected < 0 || board_detected >= ENGICAM_BOARDS)
		return -EINVAL;

	if (!board_fdt_file[board_detected])
		return -ENODEV;

	env_set("fdt_file", board_fdt_file[board_detected]);
	return 0;
}

static enum engicam_boards engicam_board_detect(void)
{
	const char *cmp_dtb = CONFIG_DEFAULT_DEVICE_TREE;

	if (!strcmp(cmp_dtb, "imx6q-icore")) {
		if (is_mx6dq())
			return IMX6Q_ICORE;
		else if (is_mx6dl() || is_mx6solo())
			return IMX6DL_ICORE;
	} else if (!strcmp(cmp_dtb, "imx6q-icore-mipi")) {
		if (is_mx6dq())
			return IMX6Q_ICORE_MIPI;
		else if (is_mx6dl() || is_mx6solo())
			return IMX6DL_ICORE_MIPI;
	} else if (!strcmp(cmp_dtb, "imx6q-icore-rqs")) {
		if (is_mx6dq())
			return IMX6Q_ICORE_RQS;
		else if (is_mx6dl() || is_mx6solo())
			return IMX6DL_ICORE_RQS;
	} else if (!strcmp(cmp_dtb, "imx6ul-geam"))
			return IMX6UL_GEAM;
	else if (!strcmp(cmp_dtb, "imx6ul-isiot-emmc"))
			return IMX6UL_ISIOT_EMMC;
	else if (!strcmp(cmp_dtb, "imx6ul-isiot-nand"))
			return IMX6UL_ISIOT_NAND;

	return -EINVAL;
}

static int fixup_enet_clock(enum engicam_boards board_detected)
{
	struct iomuxc *iomuxc_regs = (struct iomuxc *)IOMUXC_BASE_ADDR;
	int clk_internal = 0;

	switch (board_detected) {
	case IMX6Q_ICORE_MIPI:
	case IMX6DL_ICORE_MIPI:
		clk_internal = 1;
		break;
	default:
		break;
	}

	/* set gpr1[21] to select anatop clock */
	debug("fixup_enet_clock %d\n", clk_internal);
	clrsetbits_le32(&iomuxc_regs->gpr[1], 0x1 << 21, clk_internal << 21);

	if (!clk_internal) {
		/* clock is external */
		return 0;
	}

	return enable_fec_anatop_clock(0, ENET_50MHZ);
}

int board_late_init(void)
{
	enum engicam_boards board_detected = IMX6Q_ICORE;

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

	board_detected = engicam_board_detect();
	if (board_detected < 0)
		hang();

	fixup_enet_clock(board_detected);
	setenv_fdt_file(board_detected);

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

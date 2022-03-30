/*
 * (C) Copyright 2015
 * Dirk Eibach,  Guntermann & Drunck GmbH, eibach@gdsys.de
 *
 * SPDX-License-Identifier:	GPL-2.0+
 */

#include <common.h>
#include <board.h>
#include <dm.h>
#include <fdt_support.h>
#include <fsl_esdhc.h>
#include <miiphy.h>
#include <misc.h>
#include <tpm-v1.h>
#include <video_osd.h>

#include "../common/ihs_mdio.h"
#include "../../../drivers/board/gazerbeam.h"

DECLARE_GLOBAL_DATA_PTR;

struct ihs_mdio_info ihs_mdio_info[] = {
	{ .fpga = NULL, .name = "ihs0", .base = 0x58 },
	{ .fpga = NULL, .name = "ihs1", .base = 0x58 },
};

static int get_tpm(struct udevice **devp)
{
	int rc;

	rc = uclass_first_device_err(UCLASS_TPM, devp);
	if (rc) {
		printf("Could not find TPM (ret=%d)\n", rc);
		return CMD_RET_FAILURE;
	}

	return 0;
}

int board_early_init_r(void)
{
	struct udevice *board;
	struct udevice *serdes;
	int mc = 0;
	int con = 0;

	if (board_get(&board))
		puts("Could not find board information device.\n");

	/* Initialize serdes */
	uclass_get_device_by_phandle(UCLASS_MISC, board, "serdes", &serdes);

	if (board_detect(board))
		puts("Device information detection failed.\n");

	board_get_int(board, BOARD_MULTICHANNEL, &mc);
	board_get_int(board, BOARD_VARIANT, &con);

	if (mc == 2 || mc == 1)
		dev_disable_by_path("/immr@e0000000/i2c@3100/pca9698@22");

	if (mc == 4) {
		dev_disable_by_path("/immr@e0000000/i2c@3100/pca9698@20");
		dev_enable_by_path("/localbus@e0005000/iocon_uart@2,0");
		dev_enable_by_path("/fpga1bus");
	}

	if (mc == 2 || con == VAR_CON) {
		dev_enable_by_path("/fpga0bus/fpga0_video1");
		dev_enable_by_path("/fpga0bus/fpga0_iic_video1");
		dev_enable_by_path("/fpga0bus/fpga0_axi_video1");
	}

	if (con == VAR_CON) {
		dev_enable_by_path("/fpga0bus/fpga0_video0");
		dev_enable_by_path("/fpga0bus/fpga0_iic_video0");
		dev_enable_by_path("/fpga0bus/fpga0_axi_video0");
	}

	return 0;
}

int checkboard(void)
{
	struct udevice *board;
	char *s = env_get("serial#");
	int mc = 0;
	int con = 0;

	if (board_get(&board))
		puts("Could not find board information device.\n");

	board_get_int(board, BOARD_MULTICHANNEL, &mc);
	board_get_int(board, BOARD_VARIANT, &con);

	puts("Board: Gazerbeam ");
	printf("%s ", mc == 4 ? "MC4" : mc == 2 ? "MC2" : "SC");
	printf("%s", con == VAR_CON ? "CON" : "CPU");

	if (s) {
		puts(", serial# ");
		puts(s);
	}

	puts("\n");

	return 0;
}

static void display_osd_info(struct udevice *osd,
			     struct video_osd_info *osd_info)
{
	printf("OSD-%s: Digital-OSD version %01d.%02d, %d x %d characters\n",
	       osd->name, osd_info->major_version, osd_info->minor_version,
	       osd_info->width, osd_info->height);
}

int last_stage_init(void)
{
	int fpga_hw_rev = 0;
	int i;
	struct udevice *board;
	struct udevice *osd;
	struct video_osd_info osd_info;
	struct udevice *tpm;
	int ret;

	if (board_get(&board))
		puts("Could not find board information device.\n");

	if (board) {
		int res = board_get_int(board, BOARD_HWVERSION, &fpga_hw_rev);

		if (res)
			printf("Could not determind FPGA HW revision (res = %d)\n", res);
	}

	env_set_ulong("fpga_hw_rev", fpga_hw_rev);

	ret = get_tpm(&tpm);
	if (ret || tpm_init(tpm) || tpm_startup(tpm, TPM_ST_CLEAR) ||
	    tpm_continue_self_test(tpm)) {
		printf("TPM init failed\n");
	}

	if (fpga_hw_rev >= 4) {
		for (i = 0; i < 4; i++) {
			struct udevice *rxaui;
			char name[8];

			snprintf(name, sizeof(name), "rxaui%d", i);
			/* Disable RXAUI polarity inversion */
			ret = uclass_get_device_by_phandle(UCLASS_MISC, board, name, &rxaui);
			if (!ret)
				misc_set_enabled(rxaui, false);
		}
	}

	for (uclass_first_device(UCLASS_VIDEO_OSD, &osd);
	     osd;
	     uclass_next_device(&osd)) {
		video_osd_get_info(osd, &osd_info);
		display_osd_info(osd, &osd_info);
	}

	return 0;
}

#if defined(CONFIG_OF_BOARD_SETUP)
int ft_board_setup(void *blob, bd_t *bd)
{
	ft_cpu_setup(blob, bd);
	fsl_fdt_fixup_dr_usb(blob, bd);
	fdt_fixup_esdhc(blob, bd);

	return 0;
}
#endif

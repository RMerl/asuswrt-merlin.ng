// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright 2021 NXP
 */

#include <common.h>
#include <init.h>
#include <spl.h>
#include <asm/io.h>
#include <errno.h>
#include <asm/arch/sys_proto.h>
#include <asm/arch/clock.h>
#include <asm/arch/imx8ulp-pins.h>
#include <dm/uclass.h>
#include <dm/device.h>
#include <dm/uclass-internal.h>
#include <dm/device-internal.h>
#include <dm/lists.h>
#include <asm/arch/ddr.h>
#include <asm/arch/rdc.h>
#include <asm/arch/upower.h>
#include <asm/mach-imx/ele_api.h>
#include <asm/sections.h>

DECLARE_GLOBAL_DATA_PTR;

void spl_dram_init(void)
{
	/* Reboot in dual boot setting no need to init ddr again */
	bool ddr_enable = pcc_clock_is_enable(5, LPDDR4_PCC5_SLOT);

	if (!ddr_enable) {
		init_clk_ddr();
		ddr_init(&dram_timing);
	} else {
		/* reinit pfd/pfddiv and lpavnic except pll4*/
		cgc2_pll4_init(false);
	}
}

u32 spl_boot_device(void)
{
	return BOOT_DEVICE_BOOTROM;
}

int power_init_board(void)
{
	if (IS_ENABLED(CONFIG_IMX8ULP_LD_MODE)) {
		/* Set buck3 to 0.9v LD */
		upower_pmic_i2c_write(0x22, 0x18);
	} else if (IS_ENABLED(CONFIG_IMX8ULP_ND_MODE)) {
		/* Set buck3 to 1.0v ND */
		upower_pmic_i2c_write(0x22, 0x20);
	} else {
		/* Set buck3 to 1.1v OD */
		upower_pmic_i2c_write(0x22, 0x28);
	}

	return 0;
}

void display_ele_fw_version(void)
{
	u32 fw_version, sha1, res;
	int ret;

	ret = ele_get_fw_version(&fw_version, &sha1, &res);
	if (ret) {
		printf("ele get firmware version failed %d, 0x%x\n", ret, res);
	} else {
		printf("ELE firmware version %u.%u.%u-%x",
		       (fw_version & (0x00ff0000)) >> 16,
		       (fw_version & (0x0000ff00)) >> 8,
		       (fw_version & (0x000000ff)), sha1);
		((fw_version & (0x80000000)) >> 31) == 1 ? puts("-dirty\n") : puts("\n");
	}
}

void spl_board_init(void)
{
	u32 res;
	int ret;

	ret = imx8ulp_dm_post_init();
	if (ret)
		return;

	board_early_init_f();

	preloader_console_init();

	puts("Normal Boot\n");

	display_ele_fw_version();

	/* After AP set iomuxc0, the i2c can't work, Need M33 to set it now */

	/* Load the lposc fuse to work around ROM issue. The fuse depends on S400 to read. */
	if (is_soc_rev(CHIP_REV_1_0))
		load_lposc_fuse();

	upower_init();

	power_init_board();

	clock_init_late();

	/* This must place after upower init, so access to MDA and MRC are valid */
	/* Init XRDC MDA  */
	xrdc_init_mda();

	/* Init XRDC MRC for VIDEO, DSP domains */
	xrdc_init_mrc();

	xrdc_init_pdac_msc();

	/* DDR initialization */
	spl_dram_init();

	/* Call it after PS16 power up */
	set_lpav_qos();

	/* Enable A35 access to the CAAM */
	ret = ele_release_caam(0x7, &res);
	if (ret)
		printf("ele release caam failed %d, 0x%x\n", ret, res);

	/*
	 * RNG start only available on the A1 soc revision.
	 * Check some JTAG register for the SoC revision.
	 */
	if (!is_soc_rev(CHIP_REV_1_0)) {
		ret = ele_start_rng();
		if (ret)
			printf("Fail to start RNG: %d\n", ret);
	}
}

void board_init_f(ulong dummy)
{
	/* Clear the BSS. */
	memset(__bss_start, 0, __bss_end - __bss_start);

	timer_init();

	arch_cpu_init();

	board_init_r(NULL, 0);
}

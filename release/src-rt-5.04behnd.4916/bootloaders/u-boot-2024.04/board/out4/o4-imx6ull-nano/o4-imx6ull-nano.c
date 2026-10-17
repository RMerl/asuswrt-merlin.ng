// SPDX-License-Identifier: GPL-2.0+
// Copyright (C) 2021 Oleh Kravchenko <oleg@kaa.org.ua>

#include <asm/arch-mx6/clock.h>
#include <asm/arch/sys_proto.h>
#include <asm/global_data.h>
#include <asm/mach-imx/boot_mode.h>
#include <common.h>
#include <env.h>

DECLARE_GLOBAL_DATA_PTR;

int dram_init(void)
{
	gd->ram_size = imx_ddr_size();

	return 0;
}

int board_early_init_f(void)
{
	return 0;
}

static int setup_fec_clock(void)
{
	if (IS_ENABLED(CONFIG_FEC_MXC) && !IS_ENABLED(CONFIG_CLK_IMX6Q)) {
		struct iomuxc *const iomuxc_regs = (struct iomuxc *)IOMUXC_BASE_ADDR;
		int ret;

		/*
		 * Use 50M anatop loopback REF_CLK1 for ENET1,
		 * clear gpr1[13], set gpr1[17].
		 */
		clrsetbits_le32(&iomuxc_regs->gpr[1], IOMUX_GPR1_FEC1_MASK,
				IOMUX_GPR1_FEC1_CLOCK_MUX1_SEL_MASK);

		ret = enable_fec_anatop_clock(0, ENET_50MHZ);
		if (ret)
			return ret;

		if (!IS_ENABLED(CONFIG_EV_IMX280_NANO_X_MB)) {
			/*
			 * Use 50M anatop loopback REF_CLK2 for ENET2,
			 * clear gpr1[14], set gpr1[18].
			 */
			clrsetbits_le32(&iomuxc_regs->gpr[1], IOMUX_GPR1_FEC2_MASK,
					IOMUX_GPR1_FEC2_CLOCK_MUX1_SEL_MASK);

			ret = enable_fec_anatop_clock(1, ENET_50MHZ);
			if (ret)
				return ret;
		}

		enable_enet_clk(1);
	}

	return 0;
}

int board_init(void)
{
	/* Address of boot parameters */
	gd->bd->bi_boot_params = PHYS_SDRAM + 0x100;

	return setup_fec_clock();
}

int board_late_init(void)
{
	if (IS_ENABLED(CONFIG_CMD_BMODE))
		add_board_boot_modes(NULL);

	if (IS_ENABLED(CONFIG_ENV_VARS_UBOOT_RUNTIME_CONFIG)) {
		const char *model;

		model = fdt_getprop(gd->fdt_blob, 0, "model", NULL);
		if (model)
			env_set("board_name", model);
	}

	if (is_boot_from_usb()) {
		env_set("bootcmd", "run bootcmd_mfg");
		env_set("bootdelay", "0");
	}

	return 0;
}

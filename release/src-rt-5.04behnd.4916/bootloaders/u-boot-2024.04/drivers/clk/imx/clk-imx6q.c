// SPDX-License-Identifier: GPL-2.0
/*
 * Copyright (C) 2019 DENX Software Engineering
 * Lukasz Majewski, DENX Software Engineering, lukma@denx.de
 */

#include <common.h>
#include <clk-uclass.h>
#include <dm.h>
#include <log.h>
#include <asm/arch/clock.h>
#include <asm/arch/imx-regs.h>
#include <dt-bindings/clock/imx6qdl-clock.h>

#include "clk.h"

static int imx6q_clk_request(struct clk *clk)
{
	if (clk->id < IMX6QDL_CLK_DUMMY || clk->id >= IMX6QDL_CLK_END) {
		printf("%s: Invalid clk ID #%lu\n", __func__, clk->id);
		return -EINVAL;
	}

	return 0;
}

static struct clk_ops imx6q_clk_ops = {
	.request = imx6q_clk_request,
	.set_rate = ccf_clk_set_rate,
	.get_rate = ccf_clk_get_rate,
	.enable = ccf_clk_enable,
	.disable = ccf_clk_disable,
};

static const char *const usdhc_sels[] = { "pll2_pfd2_396m", "pll2_pfd0_352m", };
static const char *const periph_sels[]	= { "periph_pre", "periph_clk2", };
static const char *const periph_pre_sels[] = { "pll2_bus", "pll2_pfd2_396m",
					       "pll2_pfd0_352m", "pll2_198m", };

static int imx6q_clk_probe(struct udevice *dev)
{
	void *base;

	/* Anatop clocks */
	base = (void *)ANATOP_BASE_ADDR;

	clk_dm(IMX6QDL_CLK_PLL2,
	       imx_clk_pllv3(IMX_PLLV3_GENERIC, "pll2_bus", "osc",
			     base + 0x30, 0x1));
	clk_dm(IMX6QDL_CLK_PLL3_USB_OTG,
	       imx_clk_pllv3(IMX_PLLV3_USB, "pll3_usb_otg", "osc",
			     base + 0x10, 0x3));
	clk_dm(IMX6QDL_CLK_PLL3_60M,
	       imx_clk_fixed_factor("pll3_60m",  "pll3_usb_otg",   1, 8));
	clk_dm(IMX6QDL_CLK_PLL2_PFD0_352M,
	       imx_clk_pfd("pll2_pfd0_352m", "pll2_bus", base + 0x100, 0));
	clk_dm(IMX6QDL_CLK_PLL2_PFD2_396M,
	       imx_clk_pfd("pll2_pfd2_396m", "pll2_bus", base + 0x100, 2));
	clk_dm(IMX6QDL_CLK_PLL6,
	       imx_clk_pllv3(IMX_PLLV3_ENET, "pll6", "osc", base + 0xe0, 0x3));
	clk_dm(IMX6QDL_CLK_PLL6_ENET,
	       imx_clk_gate("pll6_enet", "pll6", base + 0xe0, 13));

	/* CCM clocks */
	base = dev_read_addr_ptr(dev);
	if (!base)
		return -EINVAL;

	clk_dm(IMX6QDL_CLK_USDHC1_SEL,
	       imx_clk_mux("usdhc1_sel", base + 0x1c, 16, 1,
			   usdhc_sels, ARRAY_SIZE(usdhc_sels)));
	clk_dm(IMX6QDL_CLK_USDHC2_SEL,
	       imx_clk_mux("usdhc2_sel", base + 0x1c, 17, 1,
			   usdhc_sels, ARRAY_SIZE(usdhc_sels)));
	clk_dm(IMX6QDL_CLK_USDHC3_SEL,
	       imx_clk_mux("usdhc3_sel", base + 0x1c, 18, 1,
			   usdhc_sels, ARRAY_SIZE(usdhc_sels)));
	clk_dm(IMX6QDL_CLK_USDHC4_SEL,
	       imx_clk_mux("usdhc4_sel", base + 0x1c, 19, 1,
			   usdhc_sels, ARRAY_SIZE(usdhc_sels)));

	clk_dm(IMX6QDL_CLK_USDHC1_PODF,
	       imx_clk_divider("usdhc1_podf", "usdhc1_sel",
			       base + 0x24, 11, 3));
	clk_dm(IMX6QDL_CLK_USDHC2_PODF,
	       imx_clk_divider("usdhc2_podf", "usdhc2_sel",
			       base + 0x24, 16, 3));
	clk_dm(IMX6QDL_CLK_USDHC3_PODF,
	       imx_clk_divider("usdhc3_podf", "usdhc3_sel",
			       base + 0x24, 19, 3));
	clk_dm(IMX6QDL_CLK_USDHC4_PODF,
	       imx_clk_divider("usdhc4_podf", "usdhc4_sel",
			       base + 0x24, 22, 3));

	clk_dm(IMX6QDL_CLK_ECSPI_ROOT,
	       imx_clk_divider("ecspi_root", "pll3_60m", base + 0x38, 19, 6));

	clk_dm(IMX6QDL_CLK_ECSPI1,
	       imx_clk_gate2("ecspi1", "ecspi_root", base + 0x6c, 0));
	clk_dm(IMX6QDL_CLK_ECSPI2,
	       imx_clk_gate2("ecspi2", "ecspi_root", base + 0x6c, 2));
	clk_dm(IMX6QDL_CLK_ECSPI3,
	       imx_clk_gate2("ecspi3", "ecspi_root", base + 0x6c, 4));
	clk_dm(IMX6QDL_CLK_ECSPI4,
	       imx_clk_gate2("ecspi4", "ecspi_root", base + 0x6c, 6));
	clk_dm(IMX6QDL_CLK_USDHC1,
	       imx_clk_gate2("usdhc1", "usdhc1_podf", base + 0x80, 2));
	clk_dm(IMX6QDL_CLK_USDHC2,
	       imx_clk_gate2("usdhc2", "usdhc2_podf", base + 0x80, 4));
	clk_dm(IMX6QDL_CLK_USDHC3,
	       imx_clk_gate2("usdhc3", "usdhc3_podf", base + 0x80, 6));
	clk_dm(IMX6QDL_CLK_USDHC4,
	       imx_clk_gate2("usdhc4", "usdhc4_podf", base + 0x80, 8));

	clk_dm(IMX6QDL_CLK_PERIPH_PRE,
	       imx_clk_mux("periph_pre", base + 0x18, 18, 2, periph_pre_sels,
			   ARRAY_SIZE(periph_pre_sels)));
	clk_dm(IMX6QDL_CLK_PERIPH,
	       imx_clk_busy_mux("periph",  base + 0x14, 25, 1, base + 0x48,
				5, periph_sels,  ARRAY_SIZE(periph_sels)));
	clk_dm(IMX6QDL_CLK_AHB,
	       imx_clk_busy_divider("ahb", "periph", base + 0x14, 10, 3,
				    base + 0x48, 1));
	clk_dm(IMX6QDL_CLK_IPG,
	       imx_clk_divider("ipg", "ahb", base + 0x14, 8, 2));
	clk_dm(IMX6QDL_CLK_IPG_PER,
	       imx_clk_divider("ipg_per", "ipg", base + 0x1c, 0, 6));
	clk_dm(IMX6QDL_CLK_I2C1,
	       imx_clk_gate2("i2c1", "ipg_per", base + 0x70, 6));
	clk_dm(IMX6QDL_CLK_I2C2,
	       imx_clk_gate2("i2c2", "ipg_per", base + 0x70, 8));

	clk_dm(IMX6QDL_CLK_ENET, imx_clk_gate2("enet", "ipg", base + 0x6c, 10));
	clk_dm(IMX6QDL_CLK_ENET_REF,
	       imx_clk_fixed_factor("enet_ref", "pll6_enet", 1, 1));

	return 0;
}

static const struct udevice_id imx6q_clk_ids[] = {
	{ .compatible = "fsl,imx6q-ccm" },
	{ },
};

U_BOOT_DRIVER(imx6q_clk) = {
	.name = "clk_imx6q",
	.id = UCLASS_CLK,
	.of_match = imx6q_clk_ids,
	.ops = &imx6q_clk_ops,
	.probe = imx6q_clk_probe,
	.flags = DM_FLAG_PRE_RELOC,
};

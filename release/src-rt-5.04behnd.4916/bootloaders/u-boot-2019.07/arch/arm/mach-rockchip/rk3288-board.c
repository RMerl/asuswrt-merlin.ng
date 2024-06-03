// SPDX-License-Identifier: GPL-2.0+
/*
 * (C) Copyright 2015 Google, Inc
 */

#include <common.h>
#include <clk.h>
#include <dm.h>
#include <ram.h>
#include <syscon.h>
#include <asm/io.h>
#include <asm/arch-rockchip/clock.h>
#include <asm/arch-rockchip/cru_rk3288.h>
#include <asm/arch-rockchip/periph.h>
#include <asm/arch-rockchip/pmu_rk3288.h>
#include <asm/arch-rockchip/qos_rk3288.h>
#include <asm/arch-rockchip/boot_mode.h>
#include <asm/gpio.h>
#include <dm/pinctrl.h>
#include <dt-bindings/clock/rk3288-cru.h>
#include <power/regulator.h>

DECLARE_GLOBAL_DATA_PTR;

__weak int rk_board_late_init(void)
{
	return 0;
}

int rk3288_qos_init(void)
{
	int val = 2 << PRIORITY_HIGH_SHIFT | 2 << PRIORITY_LOW_SHIFT;
	/* set vop qos to higher priority */
	writel(val, CPU_AXI_QOS_PRIORITY + VIO0_VOP_QOS);
	writel(val, CPU_AXI_QOS_PRIORITY + VIO1_VOP_QOS);

	if (!fdt_node_check_compatible(gd->fdt_blob, 0,
				       "rockchip,rk3288-tinker"))
	{
		/* set isp qos to higher priority */
		writel(val, CPU_AXI_QOS_PRIORITY + VIO1_ISP_R_QOS);
		writel(val, CPU_AXI_QOS_PRIORITY + VIO1_ISP_W0_QOS);
		writel(val, CPU_AXI_QOS_PRIORITY + VIO1_ISP_W1_QOS);
	}
	return 0;
}

static void rk3288_detect_reset_reason(void)
{
	struct rk3288_cru *cru = rockchip_get_cru();
	const char *reason;

	if (IS_ERR(cru))
		return;

	switch (cru->cru_glb_rst_st) {
	case GLB_POR_RST:
		reason = "POR";
		break;
	case FST_GLB_RST_ST:
	case SND_GLB_RST_ST:
		reason = "RST";
		break;
	case FST_GLB_TSADC_RST_ST:
	case SND_GLB_TSADC_RST_ST:
		reason = "THERMAL";
		break;
	case FST_GLB_WDT_RST_ST:
	case SND_GLB_WDT_RST_ST:
		reason = "WDOG";
		break;
	default:
		reason = "unknown reset";
	}

	env_set("reset_reason", reason);

	/*
	 * Clear cru_glb_rst_st, so we can determine the last reset cause
	 * for following resets.
	 */
	rk_clrreg(&cru->cru_glb_rst_st, GLB_RST_ST_MASK);
}

int board_late_init(void)
{
	setup_boot_mode();
	rk3288_qos_init();
	rk3288_detect_reset_reason();

	return rk_board_late_init();
}

#if !CONFIG_IS_ENABLED(ROCKCHIP_BACK_TO_BROM)
static int veyron_init(void)
{
	struct udevice *dev;
	struct clk clk;
	int ret;

	ret = regulator_get_by_platname("vdd_arm", &dev);
	if (ret) {
		debug("Cannot set regulator name\n");
		return ret;
	}

	/* Slowly raise to max CPU voltage to prevent overshoot */
	ret = regulator_set_value(dev, 1200000);
	if (ret)
		return ret;
	udelay(175); /* Must wait for voltage to stabilize, 2mV/us */
	ret = regulator_set_value(dev, 1400000);
	if (ret)
		return ret;
	udelay(100); /* Must wait for voltage to stabilize, 2mV/us */

	ret = rockchip_get_clk(&clk.dev);
	if (ret)
		return ret;
	clk.id = PLL_APLL;
	ret = clk_set_rate(&clk, 1800000000);
	if (IS_ERR_VALUE(ret))
		return ret;

	ret = regulator_get_by_platname("vcc33_sd", &dev);
	if (ret) {
		debug("Cannot get regulator name\n");
		return ret;
	}

	ret = regulator_set_value(dev, 3300000);
	if (ret)
		return ret;

	ret = regulators_enable_boot_on(false);
	if (ret) {
		debug("%s: Cannot enable boot on regulators\n", __func__);
		return ret;
	}

	return 0;
}
#endif

int board_init(void)
{
#if CONFIG_IS_ENABLED(ROCKCHIP_BACK_TO_BROM)
	struct udevice *pinctrl;
	int ret;

	/*
	 * We need to implement sdcard iomux here for the further
	 * initlization, otherwise, it'll hit sdcard command sending
	 * timeout exception.
	 */
	ret = uclass_get_device(UCLASS_PINCTRL, 0, &pinctrl);
	if (ret) {
		debug("%s: Cannot find pinctrl device\n", __func__);
		goto err;
	}
	ret = pinctrl_request_noflags(pinctrl, PERIPH_ID_SDCARD);
	if (ret) {
		debug("%s: Failed to set up SD card\n", __func__);
		goto err;
	}

	return 0;
err:
	printf("board_init: Error %d\n", ret);

	/* No way to report error here */
	hang();

	return -1;
#else
	int ret;

	/* We do some SoC one time setting here */
	if (!fdt_node_check_compatible(gd->fdt_blob, 0, "google,veyron")) {
		ret = veyron_init();
		if (ret)
			return ret;
	}

	return 0;
#endif
}

#if !CONFIG_IS_ENABLED(SYS_DCACHE_OFF)
void enable_caches(void)
{
	/* Enable D-cache. I-cache is already enabled in start.S */
	dcache_enable();
}
#endif

#if defined(CONFIG_USB_GADGET) && defined(CONFIG_USB_GADGET_DWC2_OTG)
#include <usb.h>
#include <usb/dwc2_udc.h>

static struct dwc2_plat_otg_data rk3288_otg_data = {
	.rx_fifo_sz	= 512,
	.np_tx_fifo_sz	= 16,
	.tx_fifo_sz	= 128,
};

int board_usb_init(int index, enum usb_init_type init)
{
	int node, phy_node;
	const char *mode;
	bool matched = false;
	const void *blob = gd->fdt_blob;
	u32 grf_phy_offset;

	/* find the usb_otg node */
	node = fdt_node_offset_by_compatible(blob, -1,
					"rockchip,rk3288-usb");

	while (node > 0) {
		mode = fdt_getprop(blob, node, "dr_mode", NULL);
		if (mode && strcmp(mode, "otg") == 0) {
			matched = true;
			break;
		}

		node = fdt_node_offset_by_compatible(blob, node,
					"rockchip,rk3288-usb");
	}
	if (!matched) {
		debug("Not found usb_otg device\n");
		return -ENODEV;
	}
	rk3288_otg_data.regs_otg = fdtdec_get_addr(blob, node, "reg");

	node = fdtdec_lookup_phandle(blob, node, "phys");
	if (node <= 0) {
		debug("Not found usb phy device\n");
		return -ENODEV;
	}

	phy_node = fdt_parent_offset(blob, node);
	if (phy_node <= 0) {
		debug("Not found usb phy device\n");
		return -ENODEV;
	}

	rk3288_otg_data.phy_of_node = phy_node;
	grf_phy_offset = fdtdec_get_addr(blob, node, "reg");

	/* find the grf node */
	node = fdt_node_offset_by_compatible(blob, -1,
					"rockchip,rk3288-grf");
	if (node <= 0) {
		debug("Not found grf device\n");
		return -ENODEV;
	}
	rk3288_otg_data.regs_phy = grf_phy_offset +
				fdtdec_get_addr(blob, node, "reg");

	return dwc2_udc_probe(&rk3288_otg_data);
}

int board_usb_cleanup(int index, enum usb_init_type init)
{
	return 0;
}
#endif

static int do_clock(cmd_tbl_t *cmdtp, int flag, int argc,
		       char * const argv[])
{
	static const struct {
		char *name;
		int id;
	} clks[] = {
		{ "osc", CLK_OSC },
		{ "apll", CLK_ARM },
		{ "dpll", CLK_DDR },
		{ "cpll", CLK_CODEC },
		{ "gpll", CLK_GENERAL },
#ifdef CONFIG_ROCKCHIP_RK3036
		{ "mpll", CLK_NEW },
#else
		{ "npll", CLK_NEW },
#endif
	};
	int ret, i;
	struct udevice *dev;

	ret = rockchip_get_clk(&dev);
	if (ret) {
		printf("clk-uclass not found\n");
		return 0;
	}

	for (i = 0; i < ARRAY_SIZE(clks); i++) {
		struct clk clk;
		ulong rate;

		clk.id = clks[i].id;
		ret = clk_request(dev, &clk);
		if (ret < 0)
			continue;

		rate = clk_get_rate(&clk);
		printf("%s: %lu\n", clks[i].name, rate);

		clk_free(&clk);
	}

	return 0;
}

U_BOOT_CMD(
	clock, 2, 1, do_clock,
	"display information about clocks",
	""
);

int board_early_init_f(void)
{
	const uintptr_t GRF_SOC_CON0 = 0xff770244;
	const uintptr_t GRF_SOC_CON2 = 0xff77024c;
	struct udevice *dev;
	int ret;

	/*
	 * This init is done in SPL, but when chain-loading U-Boot SPL will
	 * have been skipped. Allow the clock driver to check if it needs
	 * setting up.
	 */
	ret = rockchip_get_clk(&dev);
	if (ret) {
		debug("CLK init failed: %d\n", ret);
		return ret;
	}

	rk_setreg(GRF_SOC_CON2, 1 << 0);

	/*
	 * Disable JTAG on sdmmc0 IO. The SDMMC won't work until this bit is
	 * cleared
	 */
	rk_clrreg(GRF_SOC_CON0, 1 << 12);

	return 0;
}

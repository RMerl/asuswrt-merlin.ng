// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright (c) 2016 Rockchip Electronics Co., Ltd
 */
#include <common.h>
#include <command.h>
#include <dm.h>
#include <env.h>
#include <clk.h>
#include <init.h>
#include <malloc.h>
#include <asm/armv7.h>
#include <asm/global_data.h>
#include <asm/io.h>
#include <asm/arch-rockchip/bootrom.h>
#include <asm/arch-rockchip/clock.h>
#include <asm/arch-rockchip/cpu_rk3288.h>
#include <asm/arch-rockchip/cru.h>
#include <asm/arch-rockchip/hardware.h>
#include <asm/arch-rockchip/grf_rk3288.h>
#include <asm/arch-rockchip/pmu_rk3288.h>
#include <asm/arch-rockchip/qos_rk3288.h>
#include <asm/arch-rockchip/sdram.h>
#include <linux/err.h>

DECLARE_GLOBAL_DATA_PTR;

#define GRF_BASE	0xff770000

const char * const boot_devices[BROM_LAST_BOOTSOURCE + 1] = {
	[BROM_BOOTSOURCE_EMMC] = "/mmc@ff0f0000",
	[BROM_BOOTSOURCE_SD] = "/mmc@ff0c0000",
};

#ifdef CONFIG_SPL_BUILD
static void configure_l2ctlr(void)
{
	u32 l2ctlr;

	l2ctlr = read_l2ctlr();
	l2ctlr &= 0xfffc0000; /* clear bit0~bit17 */

	/*
	 * Data RAM write latency: 2 cycles
	 * Data RAM read latency: 2 cycles
	 * Data RAM setup latency: 1 cycle
	 * Tag RAM write latency: 1 cycle
	 * Tag RAM read latency: 1 cycle
	 * Tag RAM setup latency: 1 cycle
	 */
	l2ctlr |= (1 << 3 | 1 << 0);
	write_l2ctlr(l2ctlr);
}
#endif

int rk3288_qos_init(void)
{
	int val = 2 << PRIORITY_HIGH_SHIFT | 2 << PRIORITY_LOW_SHIFT;
	/* set vop qos to higher priority */
	writel(val, CPU_AXI_QOS_PRIORITY + VIO0_VOP_QOS);
	writel(val, CPU_AXI_QOS_PRIORITY + VIO1_VOP_QOS);

	if (!fdt_node_check_compatible(gd->fdt_blob, 0,
				       "rockchip,rk3288-tinker")) {
		/* set isp qos to higher priority */
		writel(val, CPU_AXI_QOS_PRIORITY + VIO1_ISP_R_QOS);
		writel(val, CPU_AXI_QOS_PRIORITY + VIO1_ISP_W0_QOS);
		writel(val, CPU_AXI_QOS_PRIORITY + VIO1_ISP_W1_QOS);
	}

	return 0;
}

int arch_cpu_init(void)
{
#ifdef CONFIG_SPL_BUILD
	configure_l2ctlr();
#else
	/* We do some SoC one time setting here. */
	struct rk3288_grf * const grf = (void *)GRF_BASE;

	/* Use rkpwm by default */
	rk_setreg(&grf->soc_con2, 1 << 0);

	/*
	 * Disable JTAG on sdmmc0 IO. The SDMMC won't work until this bit is
	 * cleared
	 */
	rk_clrreg(&grf->soc_con0, 1 << 12);

	rk3288_qos_init();
#endif

	return 0;
}

#ifdef CONFIG_DEBUG_UART_BOARD_INIT
void board_debug_uart_init(void)
{
	/* Enable early UART on the RK3288 */
	struct rk3288_grf * const grf = (void *)GRF_BASE;

	rk_clrsetreg(&grf->gpio7ch_iomux, GPIO7C7_MASK << GPIO7C7_SHIFT |
		     GPIO7C6_MASK << GPIO7C6_SHIFT,
		     GPIO7C7_UART2DBG_SOUT << GPIO7C7_SHIFT |
		     GPIO7C6_UART2DBG_SIN << GPIO7C6_SHIFT);
}
#endif

__weak int rk3288_board_late_init(void)
{
	return 0;
}

int rk_board_late_init(void)
{
	return rk3288_board_late_init();
}

static int ft_rk3288w_setup(void *blob)
{
	const char *path;
	int offs, ret;

	path = "/clock-controller@ff760000";
	offs = fdt_path_offset(blob, path);
	if (offs < 0) {
		debug("failed to found fdt path %s\n", path);
		return offs;
	}

	ret = fdt_setprop_string(blob, offs, "compatible", "rockchip,rk3288w-cru");
	if (ret) {
		printf("failed to set rk3288w-cru compatible (ret=%d)\n", ret);
		return ret;
	}

	return ret;
}

int ft_system_setup(void *blob, struct bd_info *bd)
{
	if (soc_is_rk3288w())
		return ft_rk3288w_setup(blob);

	return 0;
}

static int do_clock(struct cmd_tbl *cmdtp, int flag, int argc,
		    char *const argv[])
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
	}

	return 0;
}

U_BOOT_CMD(
	clock, 2, 1, do_clock,
	"display information about clocks",
	""
);

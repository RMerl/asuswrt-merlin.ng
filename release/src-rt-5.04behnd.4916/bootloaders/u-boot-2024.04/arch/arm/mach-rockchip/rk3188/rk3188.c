// SPDX-License-Identifier: GPL-2.0+
/*
 * (C) Copyright 2019 Rockchip Electronics Co., Ltd
 */
#include <common.h>
#include <dm.h>
#include <hang.h>
#include <init.h>
#include <led.h>
#include <log.h>
#include <syscon.h>
#include <asm/global_data.h>
#include <asm/io.h>
#include <asm/arch-rockchip/bootrom.h>
#include <asm/arch-rockchip/clock.h>
#include <asm/arch-rockchip/grf_rk3188.h>
#include <asm/arch-rockchip/hardware.h>
#include <dm/ofnode.h>
#include <linux/err.h>
#include <linux/printk.h>

#define GRF_BASE	0x20008000

const char * const boot_devices[BROM_LAST_BOOTSOURCE + 1] = {
	[BROM_BOOTSOURCE_EMMC] = "/mmc@1021c000",
	[BROM_BOOTSOURCE_SD] = "/mmc@10214000",
};

#ifdef CONFIG_DEBUG_UART_BOARD_INIT
void board_debug_uart_init(void)
{
	/* Enable early UART on the RK3188 */
	struct rk3188_grf * const grf = (void *)GRF_BASE;
	enum {
		GPIO1B1_SHIFT		= 2,
		GPIO1B1_MASK		= 3,
		GPIO1B1_GPIO		= 0,
		GPIO1B1_UART2_SOUT,
		GPIO1B1_JTAG_TDO,

		GPIO1B0_SHIFT		= 0,
		GPIO1B0_MASK		= 3,
		GPIO1B0_GPIO		= 0,
		GPIO1B0_UART2_SIN,
		GPIO1B0_JTAG_TDI,
	};

	rk_clrsetreg(&grf->gpio1b_iomux,
		     GPIO1B1_MASK << GPIO1B1_SHIFT |
		     GPIO1B0_MASK << GPIO1B0_SHIFT,
		     GPIO1B1_UART2_SOUT << GPIO1B1_SHIFT |
		     GPIO1B0_UART2_SIN << GPIO1B0_SHIFT);
}
#endif

#ifdef CONFIG_SPL_BUILD
int arch_cpu_init(void)
{
	struct rk3188_grf *grf;

	grf = syscon_get_first_range(ROCKCHIP_SYSCON_GRF);
	if (IS_ERR(grf)) {
		pr_err("grf syscon returned %ld\n", PTR_ERR(grf));
		return 0;
	}
#ifdef CONFIG_ROCKCHIP_USB_UART
	rk_clrsetreg(&grf->uoc0_con[0],
		     SIDDQ_MASK | UOC_DISABLE_MASK | COMMON_ON_N_MASK,
		     1 << SIDDQ_SHIFT | 1 << UOC_DISABLE_SHIFT |
		     1 << COMMON_ON_N_SHIFT);
	rk_clrsetreg(&grf->uoc0_con[2],
		     SOFT_CON_SEL_MASK, 1 << SOFT_CON_SEL_SHIFT);
	rk_clrsetreg(&grf->uoc0_con[3],
		     OPMODE_MASK | XCVRSELECT_MASK |
		     TERMSEL_FULLSPEED_MASK | SUSPENDN_MASK,
		     OPMODE_NODRIVING << OPMODE_SHIFT |
		     XCVRSELECT_FSTRANSC << XCVRSELECT_SHIFT |
		     1 << TERMSEL_FULLSPEED_SHIFT |
		     1 << SUSPENDN_SHIFT);
	rk_clrsetreg(&grf->uoc0_con[0],
		     BYPASSSEL_MASK | BYPASSDMEN_MASK,
		     1 << BYPASSSEL_SHIFT | 1 << BYPASSDMEN_SHIFT);
#endif
	return 0;
}
#endif

__weak int rk3188_board_late_init(void)
{
	return 0;
}

int rk_board_late_init(void)
{
	struct rk3188_grf *grf;

	grf = syscon_get_first_range(ROCKCHIP_SYSCON_GRF);
	if (IS_ERR(grf)) {
		pr_err("grf syscon returned %ld\n", PTR_ERR(grf));
		return 0;
	}

	/* enable noc remap to mimic legacy loaders */
	rk_clrsetreg(&grf->soc_con0,
		     NOC_REMAP_MASK << NOC_REMAP_SHIFT,
		     NOC_REMAP_MASK << NOC_REMAP_SHIFT);

	return rk3188_board_late_init();
}

#ifdef CONFIG_SPL_BUILD
static int setup_led(void)
{
#ifdef CONFIG_SPL_LED
	struct udevice *dev;
	char *led_name;
	int ret;

	led_name = ofnode_conf_read_str("u-boot,boot-led");
	if (!led_name)
		return 0;
	ret = led_get_by_label(led_name, &dev);
	if (ret) {
		debug("%s: get=%d\n", __func__, ret);
		return ret;
	}
	ret = led_set_state(dev, LEDST_ON);
	if (ret)
		return ret;
#endif

	return 0;
}

void spl_board_init(void)
{
	int ret;

	ret = setup_led();
	if (ret) {
		debug("LED ret=%d\n", ret);
		hang();
	}
}
#endif

// SPDX-License-Identifier: GPL-2.0+
/*
 * (C) Copyright 2008
 * Grazvydas Ignotas <notasas@gmail.com>
 *
 * Derived from Beagle Board, 3430 SDP, and OMAP3EVM code by
 *	Richard Woodruff <r-woodruff2@ti.com>
 *	Syed Mohammed Khasim <khasim@ti.com>
 *	Sunil Kumar <sunilsaini05@gmail.com>
 *	Shashi Ranjan <shashiranjanmca05@gmail.com>
 *
 * (C) Copyright 2004-2008
 * Texas Instruments, <www.ti.com>
 */
#include <common.h>
#include <dm.h>
#include <ns16550.h>
#include <twl4030.h>
#include <asm/io.h>
#include <asm/gpio.h>
#include <asm/arch/mmc_host_def.h>
#include <asm/arch/mux.h>
#include <asm/arch/gpio.h>
#include <asm/arch/sys_proto.h>
#include <asm/mach-types.h>
#include "pandora.h"

DECLARE_GLOBAL_DATA_PTR;

#define TWL4030_BB_CFG_BBCHEN		(1 << 4)
#define TWL4030_BB_CFG_BBSEL_3200MV	(3 << 2)
#define TWL4030_BB_CFG_BBISEL_500UA	2

#define CONTROL_WKUP_CTRL		0x48002a5c
#define GPIO_IO_PWRDNZ			(1 << 6)
#define PBIASLITEVMODE1			(1 << 8)

static const struct ns16550_platdata pandora_serial = {
	.base = OMAP34XX_UART3,
	.reg_shift = 2,
	.clock = V_NS16550_CLK,
	.fcr = UART_FCR_DEFVAL,
};

U_BOOT_DEVICE(pandora_uart) = {
	"ns16550_serial",
	&pandora_serial
};

/*
 * Routine: board_init
 * Description: Early hardware init.
 */
int board_init(void)
{
	gpmc_init(); /* in SRAM or SDRAM, finish GPMC */
	/* board id for Linux */
	gd->bd->bi_arch_number = MACH_TYPE_OMAP3_PANDORA;
	/* boot param addr */
	gd->bd->bi_boot_params = (OMAP34XX_SDRC_CS0 + 0x100);

	return 0;
}

static void set_output_gpio(unsigned int gpio, int value)
{
	int ret;

	ret = gpio_request(gpio, "");
	if (ret != 0) {
		printf("could not request GPIO %u\n", gpio);
		return;
	}
	ret = gpio_direction_output(gpio, value);
	if (ret != 0)
		printf("could not set GPIO %u to %d\n", gpio, value);
}

/*
 * Routine: misc_init_r
 * Description: Configure board specific parts
 */
int misc_init_r(void)
{
	t2_t *t2_base = (t2_t *)T2_BASE;
	u32 pbias_lite;

	twl4030_led_init(TWL4030_LED_LEDEN_LEDBON);

	/* set up dual-voltage GPIOs to 1.8V */
	pbias_lite = readl(&t2_base->pbias_lite);
	pbias_lite &= ~PBIASLITEVMODE1;
	pbias_lite |= PBIASLITEPWRDNZ1;
	writel(pbias_lite, &t2_base->pbias_lite);
	if (get_cpu_family() == CPU_OMAP36XX)
		writel(readl(CONTROL_WKUP_CTRL) | GPIO_IO_PWRDNZ,
			CONTROL_WKUP_CTRL);

	/* make sure audio and BT chips are in powerdown state */
	set_output_gpio(14, 0);
	set_output_gpio(15, 0);
	set_output_gpio(118, 0);

	/* enable USB supply */
	set_output_gpio(164, 1);

	/* wifi needs a short pulse to enter powersave state */
	set_output_gpio(23, 1);
	udelay(5000);
	gpio_direction_output(23, 0);

	/* Enable battery backup capacitor (3.2V, 0.5mA charge current) */
	twl4030_i2c_write_u8(TWL4030_CHIP_PM_RECEIVER,
		TWL4030_PM_RECEIVER_BB_CFG,
		TWL4030_BB_CFG_BBCHEN | TWL4030_BB_CFG_BBSEL_3200MV |
		TWL4030_BB_CFG_BBISEL_500UA);

	omap_die_id_display();

	return 0;
}

/*
 * Routine: set_muxconf_regs
 * Description: Setting up the configuration Mux registers specific to the
 *		hardware. Many pins need to be moved from protect to primary
 *		mode.
 */
void set_muxconf_regs(void)
{
	MUX_PANDORA();
	if (get_cpu_family() == CPU_OMAP36XX) {
		MUX_PANDORA_3730();
	}
}

#ifdef CONFIG_MMC
int board_mmc_init(bd_t *bis)
{
	return omap_mmc_init(0, 0, 0, -1, -1);
}

void board_mmc_power_init(void)
{
	twl4030_power_mmc_init(0);
}
#endif

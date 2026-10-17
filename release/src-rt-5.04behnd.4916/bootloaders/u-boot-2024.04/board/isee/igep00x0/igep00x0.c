// SPDX-License-Identifier: GPL-2.0+
/*
 * (C) Copyright 2010
 * ISEE 2007 SL, <www.iseebcn.com>
 */
#include <common.h>
#include <env.h>
#include <init.h>
#include <malloc.h>
#include <net.h>
#include <status_led.h>
#include <dm.h>
#include <ns16550.h>
#include <twl4030.h>
#include <netdev.h>
#include <spl.h>
#include <asm/gpio.h>
#include <asm/io.h>
#include <asm/arch/mem.h>
#include <asm/arch/mmc_host_def.h>
#include <asm/arch/mux.h>
#include <asm/arch/sys_proto.h>
#include <linux/delay.h>
#include <linux/mtd/mtd.h>
#include <linux/mtd/rawnand.h>
#include <linux/mtd/onenand.h>
#include <jffs2/load_kernel.h>
#include <mtd_node.h>
#include <fdt_support.h>
#include "igep00x0.h"

static const struct ns16550_plat igep_serial = {
	.base = OMAP34XX_UART3,
	.reg_shift = 2,
	.clock = V_NS16550_CLK,
	.fcr = UART_FCR_DEFVAL,
};

U_BOOT_DRVINFO(igep_uart) = {
	"ns16550_serial",
	&igep_serial
};

/*
 * Routine: get_board_revision
 * Description: GPIO_28 and GPIO_129 are used to read board and revision from
 * IGEP00x0 boards. First of all, it is necessary to reset USB transceiver from
 * IGEP0030 in order to read GPIO_IGEP00X0_BOARD_DETECTION correctly, because
 * this functionality is shared by USB HOST.
 * Once USB reset is applied, U-Boot configures these pins as input pullup to
 * detect board and revision:
 * IGEP0020-RF = 0b00
 * IGEP0020-RC = 0b01
 * IGEP0030-RG = 0b10
 * IGEP0030-RE = 0b11
 */
static int get_board_revision(void)
{
	int revision;

	gpio_request(IGEP0030_USB_TRANSCEIVER_RESET,
				"igep0030_usb_transceiver_reset");
	gpio_direction_output(IGEP0030_USB_TRANSCEIVER_RESET, 0);

	gpio_request(GPIO_IGEP00X0_BOARD_DETECTION, "igep00x0_board_detection");
	gpio_direction_input(GPIO_IGEP00X0_BOARD_DETECTION);
	revision = 2 * gpio_get_value(GPIO_IGEP00X0_BOARD_DETECTION);
	gpio_free(GPIO_IGEP00X0_BOARD_DETECTION);

	gpio_request(GPIO_IGEP00X0_REVISION_DETECTION,
				"igep00x0_revision_detection");
	gpio_direction_input(GPIO_IGEP00X0_REVISION_DETECTION);
	revision = revision + gpio_get_value(GPIO_IGEP00X0_REVISION_DETECTION);
	gpio_free(GPIO_IGEP00X0_REVISION_DETECTION);

	gpio_free(IGEP0030_USB_TRANSCEIVER_RESET);

	return revision;
}

int onenand_board_init(struct mtd_info *mtd)
{
	if (gpmc_cs0_flash == MTD_DEV_TYPE_ONENAND) {
		struct onenand_chip *this = mtd->priv;
		this->base = (void *)CFG_SYS_ONENAND_BASE;
		return 0;
	}
	return 1;
}

#ifdef CONFIG_OF_BOARD_SETUP
static int ft_enable_by_compatible(void *blob, char *compat, int enable)
{
	int off = fdt_node_offset_by_compatible(blob, -1, compat);
	if (off < 0)
		return off;

	if (enable)
		fdt_status_okay(blob, off);
	else
		fdt_status_disabled(blob, off);

	return 0;
}

int ft_board_setup(void *blob, struct bd_info *bd)
{
#ifdef CONFIG_FDT_FIXUP_PARTITIONS
	static const struct node_info nodes[] = {
		{ "ti,omap2-nand", MTD_DEV_TYPE_NAND, },
		{ "ti,omap2-onenand", MTD_DEV_TYPE_ONENAND, },
	};

	fdt_fixup_mtdparts(blob, nodes, ARRAY_SIZE(nodes));
#endif
	ft_enable_by_compatible(blob, "ti,omap2-nand",
				gpmc_cs0_flash == MTD_DEV_TYPE_NAND);
	ft_enable_by_compatible(blob, "ti,omap2-onenand",
				gpmc_cs0_flash == MTD_DEV_TYPE_ONENAND);

	return 0;
}
#endif

void set_led(void)
{
	switch (get_board_revision()) {
	case 0:
	case 1:
		gpio_request(IGEP0020_GPIO_LED, "igep0020_gpio_led");
		gpio_direction_output(IGEP0020_GPIO_LED, 1);
		break;
	case 2:
	case 3:
		gpio_request(IGEP0030_GPIO_LED, "igep0030_gpio_led");
		gpio_direction_output(IGEP0030_GPIO_LED, 0);
		break;
	default:
		/* Should not happen... */
		break;
	}
}

void set_boardname(void)
{
	char rev[5] = { 'F','C','G','E', };
	int i = get_board_revision();

	rev[i+1] = 0;
	env_set("board_rev", rev + i);
	env_set("board_name", i < 2 ? "igep0020" : "igep0030");
}

/*
 * Routine: misc_init_r
 * Description: Configure board specific parts
 */
int misc_init_r(void)
{
	t2_t *t2_base = (t2_t *)T2_BASE;
	u32 pbias_lite;

	twl4030_power_init();

	/* set VSIM to 1.8V */
	twl4030_pmrecv_vsel_cfg(TWL4030_PM_RECEIVER_VSIM_DEDICATED,
				TWL4030_PM_RECEIVER_VSIM_VSEL_18,
				TWL4030_PM_RECEIVER_VSIM_DEV_GRP,
				TWL4030_PM_RECEIVER_DEV_GRP_P1);

	/* set up dual-voltage GPIOs to 1.8V */
	pbias_lite = readl(&t2_base->pbias_lite);
	pbias_lite &= ~PBIASLITEVMODE1;
	pbias_lite |= PBIASLITEPWRDNZ1;
	writel(pbias_lite, &t2_base->pbias_lite);
	if (get_cpu_family() == CPU_OMAP36XX)
		writel(readl(OMAP34XX_CTRL_WKUP_CTRL) |
					 OMAP34XX_CTRL_WKUP_CTRL_GPIO_IO_PWRDNZ,
					 OMAP34XX_CTRL_WKUP_CTRL);

	omap_die_id_display();

	set_led();

	set_boardname();

	return 0;
}

void board_mtdparts_default(const char **mtdids, const char **mtdparts)
{
	struct mtd_info *mtd = get_mtd_device(NULL, 0);
	if (mtd) {
		static char ids[24];
		static char parts[48];
		const char *linux_name = "omap2-nand";
		if (strncmp(mtd->name, "onenand0", 8) == 0)
			linux_name = "omap2-onenand";
		snprintf(ids, sizeof(ids), "%s=%s", mtd->name, linux_name);
		snprintf(parts, sizeof(parts), "mtdparts=%s:%dk(SPL),-(UBI)",
		         linux_name, 4 * mtd->erasesize >> 10);
		*mtdids = ids;
		*mtdparts = parts;
	}
}

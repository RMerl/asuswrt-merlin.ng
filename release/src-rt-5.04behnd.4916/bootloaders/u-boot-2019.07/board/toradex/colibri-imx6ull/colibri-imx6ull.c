// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright (C) 2018-2019 Toradex AG
 */
#include <common.h>

#include <asm/arch/clock.h>
#include <asm/arch/crm_regs.h>
#include <asm/arch/imx-regs.h>
#include <asm/arch-mx6/clock.h>
#include <asm/arch-mx6/imx-regs.h>
#include <asm/arch-mx6/mx6ull_pins.h>
#include <asm/arch/sys_proto.h>
#include <asm/gpio.h>
#include <asm/mach-imx/boot_mode.h>
#include <asm/mach-imx/iomux-v3.h>
#include <asm/io.h>
#include <dm.h>
#include <dm/platform_data/serial_mxc.h>
#include <fdt_support.h>
#include <imx_thermal.h>
#include <jffs2/load_kernel.h>
#include <linux/sizes.h>
#include <miiphy.h>
#include <mtd_node.h>
#include <netdev.h>

#include "../common/tdx-common.h"
#include "../common/tdx-cfg-block.h"

DECLARE_GLOBAL_DATA_PTR;

#define LCD_PAD_CTRL    (PAD_CTL_HYS | PAD_CTL_PUS_100K_UP | \
		PAD_CTL_DSE_48ohm)

#define MX6_PAD_SNVS_PMIC_STBY_REQ_ADDR 0x2290040

#define NAND_PAD_CTRL (PAD_CTL_DSE_48ohm | PAD_CTL_SRE_SLOW | PAD_CTL_HYS)

#define NAND_PAD_READY0_CTRL (PAD_CTL_DSE_48ohm | PAD_CTL_PUS_22K_UP)

int dram_init(void)
{
	gd->ram_size = get_ram_size((void *)PHYS_SDRAM, PHYS_SDRAM_SIZE);

	return 0;
}

#ifdef CONFIG_NAND_MXS
static void setup_gpmi_nand(void)
{
	setup_gpmi_io_clk((3 << MXC_CCM_CSCDR1_BCH_PODF_OFFSET) |
			  (3 << MXC_CCM_CSCDR1_GPMI_PODF_OFFSET));
}
#endif /* CONFIG_NAND_MXS */

#ifdef CONFIG_VIDEO_MXS
static iomux_v3_cfg_t const lcd_pads[] = {
	MX6_PAD_LCD_CLK__LCDIF_CLK		| MUX_PAD_CTRL(LCD_PAD_CTRL),
	MX6_PAD_LCD_ENABLE__LCDIF_ENABLE	| MUX_PAD_CTRL(LCD_PAD_CTRL),
	MX6_PAD_LCD_HSYNC__LCDIF_HSYNC		| MUX_PAD_CTRL(LCD_PAD_CTRL),
	MX6_PAD_LCD_CLK__LCDIF_CLK		| MUX_PAD_CTRL(LCD_PAD_CTRL),
	MX6_PAD_LCD_DATA00__LCDIF_DATA00	| MUX_PAD_CTRL(LCD_PAD_CTRL),
	MX6_PAD_LCD_DATA01__LCDIF_DATA01	| MUX_PAD_CTRL(LCD_PAD_CTRL),
	MX6_PAD_LCD_DATA02__LCDIF_DATA02	| MUX_PAD_CTRL(LCD_PAD_CTRL),
	MX6_PAD_LCD_DATA03__LCDIF_DATA03	| MUX_PAD_CTRL(LCD_PAD_CTRL),
	MX6_PAD_LCD_DATA04__LCDIF_DATA04	| MUX_PAD_CTRL(LCD_PAD_CTRL),
	MX6_PAD_LCD_DATA05__LCDIF_DATA05	| MUX_PAD_CTRL(LCD_PAD_CTRL),
	MX6_PAD_LCD_DATA06__LCDIF_DATA06	| MUX_PAD_CTRL(LCD_PAD_CTRL),
	MX6_PAD_LCD_DATA07__LCDIF_DATA07	| MUX_PAD_CTRL(LCD_PAD_CTRL),
	MX6_PAD_LCD_DATA08__LCDIF_DATA08	| MUX_PAD_CTRL(LCD_PAD_CTRL),
	MX6_PAD_LCD_DATA09__LCDIF_DATA09	| MUX_PAD_CTRL(LCD_PAD_CTRL),
	MX6_PAD_LCD_DATA10__LCDIF_DATA10	| MUX_PAD_CTRL(LCD_PAD_CTRL),
	MX6_PAD_LCD_DATA11__LCDIF_DATA11	| MUX_PAD_CTRL(LCD_PAD_CTRL),
	MX6_PAD_LCD_DATA12__LCDIF_DATA12	| MUX_PAD_CTRL(LCD_PAD_CTRL),
	MX6_PAD_LCD_DATA13__LCDIF_DATA13	| MUX_PAD_CTRL(LCD_PAD_CTRL),
	MX6_PAD_LCD_DATA14__LCDIF_DATA14	| MUX_PAD_CTRL(LCD_PAD_CTRL),
	MX6_PAD_LCD_DATA15__LCDIF_DATA15	| MUX_PAD_CTRL(LCD_PAD_CTRL),
	MX6_PAD_LCD_DATA16__LCDIF_DATA16	| MUX_PAD_CTRL(LCD_PAD_CTRL),
	MX6_PAD_LCD_DATA17__LCDIF_DATA17	| MUX_PAD_CTRL(LCD_PAD_CTRL),
};

static iomux_v3_cfg_t const backlight_pads[] = {
	/* Backlight On */
	MX6_PAD_JTAG_TMS__GPIO1_IO11		| MUX_PAD_CTRL(NO_PAD_CTRL),
	/* Backlight PWM<A> (multiplexed pin) */
	MX6_PAD_NAND_WP_B__GPIO4_IO11		| MUX_PAD_CTRL(NO_PAD_CTRL),
};

#define GPIO_BL_ON IMX_GPIO_NR(1, 11)
#define GPIO_PWM_A IMX_GPIO_NR(4, 11)

static int setup_lcd(void)
{
	imx_iomux_v3_setup_multiple_pads(lcd_pads, ARRAY_SIZE(lcd_pads));

	imx_iomux_v3_setup_multiple_pads(backlight_pads, ARRAY_SIZE(backlight_pads));

	/* Set BL_ON */
	gpio_request(GPIO_BL_ON, "BL_ON");
	gpio_direction_output(GPIO_BL_ON, 1);

	/* Set PWM<A> to full brightness (assuming inversed polarity) */
	gpio_request(GPIO_PWM_A, "PWM<A>");
	gpio_direction_output(GPIO_PWM_A, 0);

	return 0;
}
#endif

#ifdef CONFIG_FEC_MXC
static int setup_fec(void)
{
	struct iomuxc *iomuxc_regs = (struct iomuxc *)IOMUXC_BASE_ADDR;
	int ret;

	/* provide the PHY clock from the i.MX 6 */
	ret = enable_fec_anatop_clock(1, ENET_50MHZ);
	if (ret)
		return ret;

	/* Use 50M anatop REF_CLK and output it on ENET2_TX_CLK */
	clrsetbits_le32(&iomuxc_regs->gpr[1],
			IOMUX_GPR1_FEC2_CLOCK_MUX2_SEL_MASK,
			IOMUX_GPR1_FEC2_CLOCK_MUX1_SEL_MASK);

	/* give new Ethernet PHY power save mode circuitry time to settle */
	mdelay(300);

	return 0;
}

int board_phy_config(struct phy_device *phydev)
{
	if (phydev->drv->config)
		phydev->drv->config(phydev);
	return 0;
}
#endif /* CONFIG_FEC_MXC */

int board_init(void)
{
	/* address of boot parameters */
	gd->bd->bi_boot_params = PHYS_SDRAM + 0x100;

#ifdef CONFIG_FEC_MXC
	setup_fec();
#endif

#ifdef CONFIG_NAND_MXS
	setup_gpmi_nand();
#endif

#ifdef CONFIG_VIDEO_MXS
	setup_lcd();
#endif

	return 0;
}

#ifdef CONFIG_CMD_BMODE
/* TODO */
static const struct boot_mode board_boot_modes[] = {
	/* 4 bit bus width */
	{"nand", MAKE_CFGVAL(0x40, 0x34, 0x00, 0x00)},
	{"sd1", MAKE_CFGVAL(0x10, 0x10, 0x00, 0x00)},
	{NULL, 0},
};
#endif

int board_late_init(void)
{
#ifdef CONFIG_TDX_CFG_BLOCK
	/*
	 * If we have a valid config block and it says we are a module with
	 * Wi-Fi/Bluetooth make sure we use the -wifi device tree.
	 */
	if (tdx_hw_tag.prodid == COLIBRI_IMX6ULL_WIFI_BT_IT ||
	    tdx_hw_tag.prodid == COLIBRI_IMX6ULL_WIFI_BT)
		env_set("variant", "-wifi");
#endif

	/*
	 * Disable output driver of PAD CCM_PMIC_STBY_REQ. This prevents the
	 * SOC to request for a lower voltage during sleep. This is necessary
	 * because the voltage is changing too slow for the SOC to wake up
	 * properly.
	 */
	__raw_writel(0x8080, MX6_PAD_SNVS_PMIC_STBY_REQ_ADDR);

#ifdef CONFIG_CMD_BMODE
	add_board_boot_modes(board_boot_modes);
#endif

#ifdef CONFIG_CMD_USB_SDP
	if (is_boot_from_usb()) {
		printf("Serial Downloader recovery mode, using sdp command\n");
		env_set("bootdelay", "0");
		env_set("bootcmd", "sdp 0");
	}
#endif /* CONFIG_CMD_USB_SDP */

	return 0;
}

int checkboard(void)
{
	printf("Model: Toradex Colibri iMX6ULL\n");

	return 0;
}

#if defined(CONFIG_OF_LIBFDT) && defined(CONFIG_OF_BOARD_SETUP)
int ft_board_setup(void *blob, bd_t *bd)
{
#if defined(CONFIG_FDT_FIXUP_PARTITIONS)
	static struct node_info nodes[] = {
		{ "fsl,imx6ull-gpmi-nand", MTD_DEV_TYPE_NAND, },
		{ "fsl,imx6q-gpmi-nand", MTD_DEV_TYPE_NAND, },
	};

	/* Update partition nodes using info from mtdparts env var */
	puts("   Updating MTD partitions...\n");
	fdt_fixup_mtdparts(blob, nodes, ARRAY_SIZE(nodes));
#endif

	return ft_common_board_setup(blob, bd);
}
#endif

static struct mxc_serial_platdata mxc_serial_plat = {
	.reg = (struct mxc_uart *)UART1_BASE,
	.use_dte = 1,
};

U_BOOT_DEVICE(mxc_serial) = {
	.name = "serial_mxc",
	.platdata = &mxc_serial_plat,
};

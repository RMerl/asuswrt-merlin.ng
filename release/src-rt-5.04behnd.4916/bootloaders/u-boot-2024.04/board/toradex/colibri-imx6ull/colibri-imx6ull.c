// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright (C) 2018-2019 Toradex AG
 */
#include <common.h>
#include <init.h>
#include <asm/global_data.h>
#include <linux/delay.h>

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
#include <env.h>
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

#define FLASH_DETECTION_CTRL (PAD_CTL_HYS | PAD_CTL_PUE)
#define FLASH_DET_GPIO	IMX_GPIO_NR(4, 1)
static const iomux_v3_cfg_t flash_detection_pads[] = {
	MX6_PAD_NAND_WE_B__GPIO4_IO01 | MUX_PAD_CTRL(FLASH_DETECTION_CTRL),
};

static bool is_emmc;

int dram_init(void)
{
	gd->ram_size = get_ram_size((void *)PHYS_SDRAM, PHYS_SDRAM_SIZE);

	return 0;
}

#ifdef CONFIG_NAND_MXS
static void setup_gpmi_nand(void)
{
	setup_gpmi_io_clk((MXC_CCM_CS2CDR_ENFC_CLK_PODF(0) |
			   MXC_CCM_CS2CDR_ENFC_CLK_PRED(3) |
			   MXC_CCM_CS2CDR_ENFC_CLK_SEL(3)));
}
#endif /* CONFIG_NAND_MXS */

#ifdef CONFIG_VIDEO
static const iomux_v3_cfg_t backlight_pads[] = {
	/* Backlight On */
	MX6_PAD_JTAG_TMS__GPIO1_IO11		| MUX_PAD_CTRL(NO_PAD_CTRL),
	/* Backlight PWM<A> (multiplexed pin) */
	MX6_PAD_NAND_WP_B__GPIO4_IO11		| MUX_PAD_CTRL(NO_PAD_CTRL),
};

#define GPIO_BL_ON IMX_GPIO_NR(1, 11)
#define GPIO_PWM_A IMX_GPIO_NR(4, 11)

static int setup_lcd(void)
{
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

	/*
	 * Use 50MHz anatop loopback REF_CLK2 for ENET2,
	 * clear gpr1[14], set gpr1[18].
	 */
	clrsetbits_le32(&iomuxc_regs->gpr[1], IOMUX_GPR1_FEC2_MASK,
			IOMUX_GPR1_FEC2_CLOCK_MUX1_SEL_MASK);

	ret = enable_fec_anatop_clock(1, ENET_50MHZ);
	if (ret)
		return ret;

	enable_enet_clk(1);

	return 0;
}
#endif /* CONFIG_FEC_MXC */

int board_init(void)
{
	/* address of boot parameters */
	gd->bd->bi_boot_params = PHYS_SDRAM + 0x100;

	/*
	 * Enable GPIO on NAND_WE_B/eMMC_RST with 100k pull-down. eMMC_RST
	 * is pulled high with 4.7k for eMMC devices. This allows to reliably
	 * detect eMMC/NAND flash
	 */
	imx_iomux_v3_setup_multiple_pads(flash_detection_pads, ARRAY_SIZE(flash_detection_pads));
	gpio_request(FLASH_DET_GPIO, "flash-detection-gpio");
	is_emmc = gpio_get_value(FLASH_DET_GPIO);
	gpio_free(FLASH_DET_GPIO);

#ifdef CONFIG_FEC_MXC
	setup_fec();
#endif

#ifdef CONFIG_NAND_MXS
	setup_gpmi_nand();
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
	    tdx_hw_tag.prodid == COLIBRI_IMX6ULL_WIFI_BT) {
		env_set("variant", "-wifi");
	} else {
		if (is_emmc)
			env_set("variant", "-emmc");
		else
			env_set("variant", "");
	}
#else
	if (is_emmc)
		env_set("variant", "-emmc");
	else
		env_set("variant", "");
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

	if (IS_ENABLED(CONFIG_USB) && is_boot_from_usb()) {
		env_set("bootdelay", "0");
		if (IS_ENABLED(CONFIG_CMD_USB_SDP)) {
			printf("Serial Downloader recovery mode, using sdp command\n");
			env_set("bootcmd", "sdp 0");
		} else if (IS_ENABLED(CONFIG_CMD_FASTBOOT)) {
			printf("Fastboot recovery mode, using fastboot command\n");
			env_set("bootcmd", "fastboot usb 0");
		}
	}

#if defined(CONFIG_VIDEO)
	setup_lcd();
#endif

	return 0;
}

#if defined(CONFIG_OF_LIBFDT) && defined(CONFIG_OF_BOARD_SETUP)
int ft_board_setup(void *blob, struct bd_info *bd)
{
	return ft_common_board_setup(blob, bd);
}
#endif

static struct mxc_serial_plat mxc_serial_plat = {
	.reg = (struct mxc_uart *)UART1_BASE,
	.use_dte = 1,
};

U_BOOT_DRVINFO(mxc_serial) = {
	.name = "serial_mxc",
	.plat = &mxc_serial_plat,
};

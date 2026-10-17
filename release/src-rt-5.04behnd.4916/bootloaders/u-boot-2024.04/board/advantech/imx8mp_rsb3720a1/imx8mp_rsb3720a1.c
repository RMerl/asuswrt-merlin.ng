// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright 2019 NXP
 * Copyright 2022 Linaro
 */

#include <common.h>
#include <dwc3-uboot.h>
#include <efi.h>
#include <efi_loader.h>
#include <errno.h>
#include <miiphy.h>
#include <netdev.h>
#include <spl.h>
#include <usb.h>
#include <asm/io.h>
#include <asm/mach-imx/iomux-v3.h>
#include <asm-generic/gpio.h>
#include <asm/arch/imx8mp_pins.h>
#include <asm/arch/sys_proto.h>
#include <asm/mach-imx/gpio.h>
#include <asm/mach-imx/mxc_i2c.h>
#include <asm/arch/clock.h>
#include <asm/mach-imx/dma.h>
#include <linux/delay.h>
#include <linux/kernel.h>
#include <power/pmic.h>

DECLARE_GLOBAL_DATA_PTR;

#ifdef CONFIG_NAND_MXS
static void setup_gpmi_nand(void)
{
	init_nand_clk();
}
#endif

#if IS_ENABLED(CONFIG_EFI_HAVE_CAPSULE_SUPPORT)
struct efi_fw_image fw_images[] = {
#if defined(CONFIG_TARGET_IMX8MP_RSB3720A1_4G)
	{
		.image_type_id = IMX8MP_RSB3720A1_4G_FIT_IMAGE_GUID,
		.fw_name = u"IMX8MP-RSB3720-FIT",
		.image_index = 1,
	},
#elif defined(CONFIG_TARGET_IMX8MP_RSB3720A1_6G)
	{
		.image_type_id = IMX8MP_RSB3720A1_6G_FIT_IMAGE_GUID,
		.fw_name = u"IMX8MP-RSB3720-FIT",
		.image_index = 1,
	},
#endif
};

struct efi_capsule_update_info update_info = {
	.dfu_string = "mmc 2=flash-bin raw 0 0x1B00 mmcpart 1",
	.num_images = ARRAY_SIZE(fw_images),
	.images = fw_images,
};

#endif /* EFI_HAVE_CAPSULE_SUPPORT */


int board_early_init_f(void)
{
	init_uart_clk(2);

	return 0;
}

#ifdef CONFIG_OF_BOARD_SETUP
int ft_board_setup(void *blob, struct bd_info *bd)
{
	return 0;
}
#endif

#ifdef CONFIG_FEC_MXC
#define FEC_RST_PAD IMX_GPIO_NR(4, 2)
static const iomux_v3_cfg_t fec1_rst_pads[] = {
	MX8MP_PAD_SAI1_RXD0__GPIO4_IO02 | MUX_PAD_CTRL(NO_PAD_CTRL),
};

static void setup_iomux_fec(void)
{
	imx_iomux_v3_setup_multiple_pads(fec1_rst_pads,
					 ARRAY_SIZE(fec1_rst_pads));

	gpio_request(FEC_RST_PAD, "fec1_rst");
	gpio_direction_output(FEC_RST_PAD, 0);
	mdelay(15);
	gpio_direction_output(FEC_RST_PAD, 1);
	mdelay(100);
}

static int setup_fec(void)
{
	struct iomuxc_gpr_base_regs *gpr =
		(struct iomuxc_gpr_base_regs *)IOMUXC_GPR_BASE_ADDR;

	setup_iomux_fec();

	/* Enable RGMII TX clk output */
	setbits_le32(&gpr->gpr[1], BIT(22));

	return 0;
}
#endif /* CONFIG_FEC_MXC */

#ifdef CONFIG_DWC_ETH_QOS
#define EQOS_RST_PAD IMX_GPIO_NR(4, 22)
static const iomux_v3_cfg_t eqos_rst_pads[] = {
	MX8MP_PAD_SAI2_RXC__GPIO4_IO22 | MUX_PAD_CTRL(NO_PAD_CTRL),
};

static void setup_eqos(void)
{
	imx_iomux_v3_setup_multiple_pads(eqos_rst_pads,
					 ARRAY_SIZE(eqos_rst_pads));

	gpio_request(EQOS_RST_PAD, "eqos_rst");
	gpio_direction_output(EQOS_RST_PAD, 0);
	mdelay(15);
	gpio_direction_output(EQOS_RST_PAD, 1);
	mdelay(100);
}
#endif /* CONFIG_DWC_ETH_QOS */

int board_phy_config(struct phy_device *phydev)
{
	if (IS_ENABLED(CONFIG_FEC_MXC) || IS_ENABLED(CONFIG_DWC_ETH_QOS)) {
		/* enable rgmii rxc skew and phy mode select to RGMII copper */
		phy_write(phydev, MDIO_DEVAD_NONE, 0x1d, 0x1f);
		phy_write(phydev, MDIO_DEVAD_NONE, 0x1e, 0x8);

		phy_write(phydev, MDIO_DEVAD_NONE, 0x1d, 0x00);
		phy_write(phydev, MDIO_DEVAD_NONE, 0x1e, 0x82ee);
		phy_write(phydev, MDIO_DEVAD_NONE, 0x1d, 0x05);
		phy_write(phydev, MDIO_DEVAD_NONE, 0x1e, 0x100);

		if (phydev->drv->config)
			phydev->drv->config(phydev);
	}

	return 0;
}

#define DISPMIX				13
#define MIPI				15

#define WDOG_TRIG IMX_GPIO_NR(4, 20)

static iomux_v3_cfg_t wdt_trig[] = {
	MX8MP_PAD_SAI1_MCLK__GPIO4_IO20 | MUX_PAD_CTRL(NO_PAD_CTRL),
};

static void setup_iomux_wdt(void)
{
	imx_iomux_v3_setup_multiple_pads(wdt_trig, ARRAY_SIZE(wdt_trig));
	gpio_request(WDOG_TRIG, "wdt_trig");
	gpio_direction_output(WDOG_TRIG, 1);
}

int board_init(void)
{
#ifdef CONFIG_FEC_MXC
	setup_fec();
#endif

#ifdef CONFIG_DWC_ETH_QOS
	/* clock, pin, gpr */
	setup_eqos();
#endif

#ifdef CONFIG_NAND_MXS
	setup_gpmi_nand();
#endif

	setup_iomux_wdt();

	return 0;
}

int board_late_init(void)
{
	if (IS_ENABLED(CONFIG_ENV_VARS_UBOOT_RUNTIME_CONFIG)) {
		env_set("board_name", "RSB3720A1");
		env_set("board_rev", "iMX8MP");
	}

	return 0;
}

#ifdef CONFIG_SPL_MMC
#define UBOOT_RAW_SECTOR_OFFSET 0x40
unsigned long board_spl_mmc_get_uboot_raw_sector(struct mmc *mmc,
					   unsigned long raw_sector)
{
	u32 boot_dev = spl_boot_device();

	switch (boot_dev) {
	case BOOT_DEVICE_MMC2:
		return CONFIG_SYS_MMCSD_RAW_MODE_U_BOOT_SECTOR - UBOOT_RAW_SECTOR_OFFSET;
	default:
		return CONFIG_SYS_MMCSD_RAW_MODE_U_BOOT_SECTOR;
	}
}
#endif /* CONFIG_SPL_MMC */

// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright 2018 congatec AG
 * Copyright (C) 2019 Oliver Graute <oliver.graute@kococonnector.com>
 */
#include <common.h>
#include <errno.h>
#include <linux/libfdt.h>
#include <fsl_esdhc.h>
#include <init.h>

#include <asm/io.h>
#include <asm/gpio.h>
#include <asm/arch/clock.h>
#include <firmware/imx/sci/sci.h>
#include <asm/arch/imx8-pins.h>
#include <usb.h>
#include <asm/arch/iomux.h>
#include <asm/arch/sys_proto.h>
#include <linux/delay.h>
#include <power-domain.h>

DECLARE_GLOBAL_DATA_PTR;

#define ESDHC_PAD_CTRL	((SC_PAD_CONFIG_NORMAL << PADRING_CONFIG_SHIFT) | \
		(SC_PAD_ISO_OFF << PADRING_LPCONFIG_SHIFT) | \
		(SC_PAD_28FDSOI_DSE_DV_HIGH << PADRING_DSE_SHIFT) | \
		(SC_PAD_28FDSOI_PS_PU << PADRING_PULL_SHIFT))

#define ESDHC_CLK_PAD_CTRL	((SC_PAD_CONFIG_OUT_IN << PADRING_CONFIG_SHIFT) | \
			(SC_PAD_ISO_OFF << PADRING_LPCONFIG_SHIFT) | \
			(SC_PAD_28FDSOI_DSE_DV_HIGH << PADRING_DSE_SHIFT) | \
			(SC_PAD_28FDSOI_PS_PU << PADRING_PULL_SHIFT))

#define ENET_INPUT_PAD_CTRL	((SC_PAD_CONFIG_OD_IN << PADRING_CONFIG_SHIFT) | \
			(SC_PAD_ISO_OFF << PADRING_LPCONFIG_SHIFT) | \
			(SC_PAD_28FDSOI_DSE_18V_10MA << PADRING_DSE_SHIFT) | \
			(SC_PAD_28FDSOI_PS_PU << PADRING_PULL_SHIFT))

#define ENET_NORMAL_PAD_CTRL	((SC_PAD_CONFIG_NORMAL << PADRING_CONFIG_SHIFT) | \
			(SC_PAD_ISO_OFF << PADRING_LPCONFIG_SHIFT) | \
			(SC_PAD_28FDSOI_DSE_18V_10MA << PADRING_DSE_SHIFT) | \
			(SC_PAD_28FDSOI_PS_PU << PADRING_PULL_SHIFT))

#define FSPI_PAD_CTRL	((SC_PAD_CONFIG_NORMAL << PADRING_CONFIG_SHIFT) | \
			(SC_PAD_ISO_OFF << PADRING_LPCONFIG_SHIFT) | \
			(SC_PAD_28FDSOI_DSE_DV_HIGH << PADRING_DSE_SHIFT) | \
			(SC_PAD_28FDSOI_PS_PU << PADRING_PULL_SHIFT))

#define GPIO_PAD_CTRL	((SC_PAD_CONFIG_NORMAL << PADRING_CONFIG_SHIFT) | \
			(SC_PAD_ISO_OFF << PADRING_LPCONFIG_SHIFT) | \
			(SC_PAD_28FDSOI_DSE_DV_HIGH << PADRING_DSE_SHIFT) | \
			(SC_PAD_28FDSOI_PS_PU << PADRING_PULL_SHIFT))

#define I2C_PAD_CTRL	((SC_PAD_CONFIG_OUT_IN << PADRING_CONFIG_SHIFT) | \
			(SC_PAD_ISO_OFF << PADRING_LPCONFIG_SHIFT) | \
			(SC_PAD_28FDSOI_DSE_DV_LOW << PADRING_DSE_SHIFT) | \
			(SC_PAD_28FDSOI_PS_PU << PADRING_PULL_SHIFT))

#define UART_PAD_CTRL	((SC_PAD_CONFIG_OUT_IN << PADRING_CONFIG_SHIFT) | \
			(SC_PAD_ISO_OFF << PADRING_LPCONFIG_SHIFT) | \
			(SC_PAD_28FDSOI_DSE_DV_HIGH << PADRING_DSE_SHIFT) | \
			(SC_PAD_28FDSOI_PS_PU << PADRING_PULL_SHIFT))

static iomux_cfg_t uart0_pads[] = {
	SC_P_UART0_RX | MUX_PAD_CTRL(UART_PAD_CTRL),
	SC_P_UART0_TX | MUX_PAD_CTRL(UART_PAD_CTRL),
};

void board_late_mmc_env_init(void);
void init_clk_usdhc(u32 index);
int fsl_esdhc_initialize(struct bd_info *bis, struct fsl_esdhc_cfg *cfg);

static void setup_iomux_uart(void)
{
	imx8_iomux_setup_multiple_pads(uart0_pads, ARRAY_SIZE(uart0_pads));
}

int board_early_init_f(void)
{
	/* sc_ipc_t ipcHndl = 0; */
	int scierr;

	/* When start u-boot in XEN VM, directly return */
	/* if (IS_ENABLED(CONFIG_XEN)) */
		/* return 0; */

	/* ipcHndl = gd->arch.ipc_channel_handle; */

	/* Power up UART0, this is very early while power domain is not working */
	scierr = sc_pm_set_resource_power_mode(-1, SC_R_UART_0, SC_PM_PW_MODE_ON);
	if (scierr)
		return 0;

	/* Set UART0 clock root to 80 MHz */
	sc_pm_clock_rate_t rate = 80000000;

	scierr = sc_pm_set_clock_rate(-1, SC_R_UART_0, 2, &rate);
	if (scierr)
		return 0;

	/* Enable UART0 clock root */
	scierr = sc_pm_clock_enable(-1, SC_R_UART_0, 2, true, false);
	if (scierr)
		return 0;

	setup_iomux_uart();

	return 0;
}

#if IS_ENABLED(CONFIG_FSL_ESDHC_IMX)

#define USDHC1_CD_GPIO	IMX_GPIO_NR(5, 22)
#define USDHC2_CD_GPIO	IMX_GPIO_NR(4, 12)

static struct fsl_esdhc_cfg usdhc_cfg[CFG_SYS_FSL_USDHC_NUM] = {
	{USDHC1_BASE_ADDR, 0, 8},
	{USDHC2_BASE_ADDR, 0, 4},
	{USDHC3_BASE_ADDR, 0, 4},
};

static iomux_cfg_t emmc0[] = {
	SC_P_EMMC0_CLK | MUX_PAD_CTRL(ESDHC_CLK_PAD_CTRL),
	SC_P_EMMC0_CMD | MUX_PAD_CTRL(ESDHC_PAD_CTRL),
	SC_P_EMMC0_DATA0 | MUX_PAD_CTRL(ESDHC_PAD_CTRL),
	SC_P_EMMC0_DATA1 | MUX_PAD_CTRL(ESDHC_PAD_CTRL),
	SC_P_EMMC0_DATA2 | MUX_PAD_CTRL(ESDHC_PAD_CTRL),
	SC_P_EMMC0_DATA3 | MUX_PAD_CTRL(ESDHC_PAD_CTRL),
	SC_P_EMMC0_DATA4 | MUX_PAD_CTRL(ESDHC_PAD_CTRL),
	SC_P_EMMC0_DATA5 | MUX_PAD_CTRL(ESDHC_PAD_CTRL),
	SC_P_EMMC0_DATA6 | MUX_PAD_CTRL(ESDHC_PAD_CTRL),
	SC_P_EMMC0_DATA7 | MUX_PAD_CTRL(ESDHC_PAD_CTRL),
	SC_P_EMMC0_RESET_B | MUX_PAD_CTRL(ESDHC_PAD_CTRL),
	SC_P_EMMC0_STROBE | MUX_PAD_CTRL(ESDHC_PAD_CTRL),
};

static iomux_cfg_t usdhc1_sd[] = {
	SC_P_USDHC1_CLK | MUX_PAD_CTRL(ESDHC_CLK_PAD_CTRL),
	SC_P_USDHC1_CMD | MUX_PAD_CTRL(ESDHC_PAD_CTRL),
	SC_P_USDHC1_DATA0 | MUX_PAD_CTRL(ESDHC_PAD_CTRL),
	SC_P_USDHC1_DATA1 | MUX_PAD_CTRL(ESDHC_PAD_CTRL),
	SC_P_USDHC1_DATA2 | MUX_PAD_CTRL(ESDHC_PAD_CTRL),
	SC_P_USDHC1_DATA3 | MUX_PAD_CTRL(ESDHC_PAD_CTRL),
	SC_P_USDHC1_DATA6 | MUX_MODE_ALT(2) | MUX_PAD_CTRL(ESDHC_PAD_CTRL),
	SC_P_USDHC1_DATA7 | MUX_MODE_ALT(3) | MUX_PAD_CTRL(ESDHC_PAD_CTRL),
	SC_P_USDHC1_RESET_B | MUX_PAD_CTRL(ESDHC_PAD_CTRL),
	SC_P_USDHC1_VSELECT | MUX_PAD_CTRL(ESDHC_PAD_CTRL),
};

static iomux_cfg_t usdhc2_sd[] = {
	SC_P_USDHC2_CLK | MUX_PAD_CTRL(ESDHC_CLK_PAD_CTRL),
	SC_P_USDHC2_CMD | MUX_PAD_CTRL(ESDHC_PAD_CTRL),
	SC_P_USDHC2_DATA0 | MUX_PAD_CTRL(ESDHC_PAD_CTRL),
	SC_P_USDHC2_DATA1 | MUX_PAD_CTRL(ESDHC_PAD_CTRL),
	SC_P_USDHC2_DATA2 | MUX_PAD_CTRL(ESDHC_PAD_CTRL),
	SC_P_USDHC2_DATA3 | MUX_PAD_CTRL(ESDHC_PAD_CTRL),
	SC_P_USDHC2_RESET_B | MUX_PAD_CTRL(ESDHC_PAD_CTRL),
	SC_P_USDHC2_VSELECT | MUX_PAD_CTRL(ESDHC_PAD_CTRL),
	SC_P_USDHC2_WP | MUX_PAD_CTRL(ESDHC_PAD_CTRL),
	SC_P_USDHC2_CD_B | MUX_MODE_ALT(3) | MUX_PAD_CTRL(ESDHC_PAD_CTRL),
};

int board_mmc_init(struct bd_info *bis)
{
	int i, ret;
	struct power_domain pd;

	/*
	 * According to the board_mmc_init() the following map is done:
	 * (U-Boot device node)    (Physical Port)
	 * mmc0 (onboard eMMC)     USDHC1
	 * mmc1 (external SD card) USDHC2
	 * mmc2 (onboard µSD)      USDHC3
	 */
	for (i = 0; i < CFG_SYS_FSL_USDHC_NUM; i++) {
		switch (i) {
		case 0:
		  /* onboard eMMC */
			if (!imx8_power_domain_lookup_name("conn_sdhc0", &pd))
				power_domain_on(&pd);

			imx8_iomux_setup_multiple_pads(emmc0, ARRAY_SIZE(emmc0));
			init_clk_usdhc(0);
			usdhc_cfg[i].sdhc_clk = mxc_get_clock(MXC_ESDHC_CLK);
			break;
		case 1:
		  /* external SD card */
			if (!imx8_power_domain_lookup_name("conn_sdhc1", &pd))
				power_domain_on(&pd);

			imx8_iomux_setup_multiple_pads(usdhc1_sd, ARRAY_SIZE(usdhc1_sd));
			init_clk_usdhc(1);
			usdhc_cfg[i].sdhc_clk = mxc_get_clock(MXC_ESDHC2_CLK);
			gpio_request(USDHC1_CD_GPIO, "sd1_cd");
			gpio_direction_input(USDHC1_CD_GPIO);
			break;
		case 2:
		  /* onboard µSD */
			if (!imx8_power_domain_lookup_name("conn_sdhc2", &pd))
				power_domain_on(&pd);

			imx8_iomux_setup_multiple_pads(usdhc2_sd, ARRAY_SIZE(usdhc2_sd));
			init_clk_usdhc(2);
			usdhc_cfg[i].sdhc_clk = mxc_get_clock(MXC_ESDHC3_CLK);
			gpio_request(USDHC2_CD_GPIO, "sd2_cd");
			gpio_direction_input(USDHC2_CD_GPIO);
			break;
		default:
			printf("Warning: you configured more USDHC controllers"
				"(%d) than supported by the board\n", i + 1);
			return 0;
		}
		ret = fsl_esdhc_initialize(bis, &usdhc_cfg[i]);
		if (ret) {
			printf("Warning: failed to initialize mmc dev %d\n", i);
			return ret;
		}
	}

	return 0;
}

int board_mmc_getcd(struct mmc *mmc)
{
	struct fsl_esdhc_cfg *cfg = (struct fsl_esdhc_cfg *)mmc->priv;
	int ret = 0;

	switch (cfg->esdhc_base) {
	case USDHC1_BASE_ADDR:
		ret = 1;
		break;
	case USDHC2_BASE_ADDR:
		ret = !gpio_get_value(USDHC1_CD_GPIO);
		break;
	case USDHC3_BASE_ADDR:
		ret = !gpio_get_value(USDHC2_CD_GPIO);
		break;
	}

	return ret;
}

#endif /* CONFIG_FSL_ESDHC_IMX */

#if (IS_ENABLED(CONFIG_FEC_MXC))

#include <miiphy.h>

static iomux_cfg_t pad_enet0[] = {
	SC_P_ENET0_RGMII_RX_CTL | MUX_PAD_CTRL(ENET_INPUT_PAD_CTRL),
	SC_P_ENET0_RGMII_RXD0 | MUX_PAD_CTRL(ENET_INPUT_PAD_CTRL),
	SC_P_ENET0_RGMII_RXD1 | MUX_PAD_CTRL(ENET_INPUT_PAD_CTRL),
	SC_P_ENET0_RGMII_RXD2 | MUX_PAD_CTRL(ENET_INPUT_PAD_CTRL),
	SC_P_ENET0_RGMII_RXD3 | MUX_PAD_CTRL(ENET_INPUT_PAD_CTRL),
	SC_P_ENET0_RGMII_RXC | MUX_PAD_CTRL(ENET_INPUT_PAD_CTRL),
	SC_P_ENET0_RGMII_TX_CTL | MUX_PAD_CTRL(ENET_NORMAL_PAD_CTRL),
	SC_P_ENET0_RGMII_TXD0 | MUX_PAD_CTRL(ENET_NORMAL_PAD_CTRL),
	SC_P_ENET0_RGMII_TXD1 | MUX_PAD_CTRL(ENET_NORMAL_PAD_CTRL),
	SC_P_ENET0_RGMII_TXD2 | MUX_PAD_CTRL(ENET_NORMAL_PAD_CTRL),
	SC_P_ENET0_RGMII_TXD3 | MUX_PAD_CTRL(ENET_NORMAL_PAD_CTRL),
	SC_P_ENET0_RGMII_TXC | MUX_PAD_CTRL(ENET_NORMAL_PAD_CTRL),
	SC_P_ENET0_MDC | MUX_PAD_CTRL(ENET_NORMAL_PAD_CTRL),
	SC_P_ENET0_MDIO | MUX_PAD_CTRL(ENET_NORMAL_PAD_CTRL),
};

static void setup_iomux_fec(void)
{
	imx8_iomux_setup_multiple_pads(pad_enet0, ARRAY_SIZE(pad_enet0));
}

static void enet_device_phy_reset(void)
{
	gpio_set_value(FEC0_RESET, 0);
	udelay(50);
	gpio_set_value(FEC0_RESET, 1);

	/* The board has a long delay for this reset to become stable */
	mdelay(200);
}

int board_eth_init(struct bd_info *bis)
{
	setup_iomux_fec();

	return 0;
}

int board_phy_config(struct phy_device *phydev)
{
	phy_write(phydev, MDIO_DEVAD_NONE, 0x1d, 0x1f);
	phy_write(phydev, MDIO_DEVAD_NONE, 0x1e, 0x8);

	phy_write(phydev, MDIO_DEVAD_NONE, 0x1d, 0x00);
	phy_write(phydev, MDIO_DEVAD_NONE, 0x1e, 0x82ee);
	phy_write(phydev, MDIO_DEVAD_NONE, 0x1d, 0x05);
	phy_write(phydev, MDIO_DEVAD_NONE, 0x1e, 0x100);

	if (phydev->drv->config)
		phydev->drv->config(phydev);

	return 0;
}

static int setup_fec(void)
{
	/* Reset ENET PHY */
	enet_device_phy_reset();

	return 0;
}
#endif

#ifdef CONFIG_MXC_GPIO

#define LVDS_ENABLE IMX_GPIO_NR(1, 6)
#define BKL_ENABLE  IMX_GPIO_NR(1, 7)

static iomux_cfg_t board_gpios[] = {
	SC_P_LVDS0_I2C0_SCL | MUX_MODE_ALT(3) | MUX_PAD_CTRL(GPIO_PAD_CTRL),
	SC_P_LVDS0_I2C0_SDA | MUX_MODE_ALT(3) | MUX_PAD_CTRL(GPIO_PAD_CTRL),
	SC_P_ESAI1_FST | MUX_MODE_ALT(3) | MUX_PAD_CTRL(GPIO_PAD_CTRL),
};

static void board_gpio_init(void)
{
	imx8_iomux_setup_multiple_pads(board_gpios, ARRAY_SIZE(board_gpios));

	/* enable LVDS */
	gpio_request(LVDS_ENABLE, "lvds_enable");
	gpio_direction_output(LVDS_ENABLE, 1);

	/* enable backlight */
	gpio_request(BKL_ENABLE, "bkl_enable");
	gpio_direction_output(BKL_ENABLE, 1);

	/* ethernet reset */
	gpio_request(FEC0_RESET, "enet0_reset");
	gpio_direction_output(FEC0_RESET, 1);
}
#endif

int checkboard(void)
{
	puts("Board: conga-QMX8\n");

	build_info();
	print_bootinfo();

	return 0;
}

int board_init(void)
{
	if (IS_ENABLED(CONFIG_XEN))
		return 0;

#ifdef CONFIG_MXC_GPIO
	board_gpio_init();
#endif

#if (IS_ENABLED(CONFIG_FEC_MXC))
	setup_fec();
#endif

	return 0;
}

void detail_board_ddr_info(void)
{
	puts("\nDDR    ");
}

/*
 * Board specific reset that is system reset.
 */
void reset_cpu(void)
{
	/* TODO */
}

#ifdef CONFIG_OF_BOARD_SETUP
int ft_board_setup(void *blob, struct bd_info *bd)
{
	return 0;
}
#endif

int board_mmc_get_env_dev(int devno)
{
	/* Use EMMC */
	if (IS_ENABLED(CONFIG_XEN))
		return 0;

	return devno;
}

int mmc_map_to_kernel_blk(int dev_no)
{
	/* Use EMMC */
	if (IS_ENABLED(CONFIG_XEN))
		return 0;

	return dev_no;
}

extern u32 _end_ofs;
int board_late_init(void)
{
#ifdef CONFIG_ENV_VARS_UBOOT_RUNTIME_CONFIG
	env_set("board_name", "QMX8");
	env_set("board_rev", "iMX8QM");
#endif

	env_set("sec_boot", "no");
#ifdef CONFIG_AHAB_BOOT
	env_set("sec_boot", "yes");
#endif

#ifdef CONFIG_ENV_IS_IN_MMC
	board_late_mmc_env_init();
#endif

#ifdef IMX_LOAD_HDMI_FIMRWARE
	char *end_of_uboot;
	char command[256];

	end_of_uboot = (char *)(ulong)(CONFIG_TEXT_BASE + _end_ofs
			+ fdt_totalsize(gd->fdt_blob));
	end_of_uboot += 9;

	/* load hdmitxfw.bin and hdmirxfw.bin*/
	memcpy(IMX_HDMI_FIRMWARE_LOAD_ADDR, end_of_uboot,
	       IMX_HDMITX_FIRMWARE_SIZE + IMX_HDMIRX_FIRMWARE_SIZE);

	sprintf(command, "hdp load 0x%x", IMX_HDMI_FIRMWARE_LOAD_ADDR);
	run_command(command, 0);

	sprintf(command, "hdprx load 0x%x",
		IMX_HDMI_FIRMWARE_LOAD_ADDR + IMX_HDMITX_FIRMWARE_SIZE);
	run_command(command, 0);
#endif

	return 0;
}

#ifdef CONFIG_FSL_FASTBOOT
#ifdef CONFIG_ANDROID_RECOVERY
int is_recovery_key_pressing(void)
{
	return 0; /*TODO*/
}
#endif /*CONFIG_ANDROID_RECOVERY*/
#endif /*CONFIG_FSL_FASTBOOT*/

/* Only Enable USB3 resources currently */
int board_usb_init(int index, enum usb_init_type init)
{
	return 0;
}

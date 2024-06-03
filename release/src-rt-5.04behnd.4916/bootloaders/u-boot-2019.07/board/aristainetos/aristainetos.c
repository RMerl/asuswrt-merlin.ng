// SPDX-License-Identifier: GPL-2.0+
/*
 * (C) Copyright 2014
 * Heiko Schocher, DENX Software Engineering, hs@denx.de.
 *
 * Based on:
 * Copyright (C) 2012 Freescale Semiconductor, Inc.
 *
 * Author: Fabio Estevam <fabio.estevam@freescale.com>
 */

#include <asm/arch/clock.h>
#include <asm/arch/imx-regs.h>
#include <asm/arch/iomux.h>
#include <asm/arch/mx6-pins.h>
#include <linux/errno.h>
#include <asm/gpio.h>
#include <asm/mach-imx/iomux-v3.h>
#include <asm/mach-imx/boot_mode.h>
#include <asm/mach-imx/mxc_i2c.h>
#include <asm/mach-imx/video.h>
#include <mmc.h>
#include <fsl_esdhc.h>
#include <miiphy.h>
#include <netdev.h>
#include <asm/arch/mxc_hdmi.h>
#include <asm/arch/crm_regs.h>
#include <linux/fb.h>
#include <ipu_pixfmt.h>
#include <input.h>
#include <asm/io.h>
#include <asm/arch/sys_proto.h>
#include <pwm.h>

DECLARE_GLOBAL_DATA_PTR;

#define UART_PAD_CTRL  (PAD_CTL_PUS_100K_UP |			\
	PAD_CTL_SPEED_MED | PAD_CTL_DSE_40ohm |			\
	PAD_CTL_SRE_FAST  | PAD_CTL_HYS)

#define USDHC_PAD_CTRL (PAD_CTL_PUS_47K_UP |			\
	PAD_CTL_SPEED_LOW | PAD_CTL_DSE_80ohm |			\
	PAD_CTL_SRE_FAST  | PAD_CTL_HYS)

#define ENET_PAD_CTRL  (PAD_CTL_PUS_100K_UP |			\
	PAD_CTL_SPEED_MED | PAD_CTL_DSE_40ohm | PAD_CTL_HYS)

#define SPI_PAD_CTRL (PAD_CTL_HYS | PAD_CTL_SPEED_MED | \
		      PAD_CTL_DSE_40ohm | PAD_CTL_SRE_FAST)

#define I2C_PAD_CTRL	(PAD_CTL_PUS_100K_UP |			\
	PAD_CTL_SPEED_MED | PAD_CTL_DSE_40ohm | PAD_CTL_HYS |	\
	PAD_CTL_ODE | PAD_CTL_SRE_FAST)

#define PC MUX_PAD_CTRL(I2C_PAD_CTRL)

#define DISP_PAD_CTRL	(0x10)

#define ECSPI4_CS1		IMX_GPIO_NR(5, 2)

#if (CONFIG_SYS_BOARD_VERSION == 1)
#include "./aristainetos-v1.c"
#elif ((CONFIG_SYS_BOARD_VERSION == 2) || (CONFIG_SYS_BOARD_VERSION == 3))
#include "./aristainetos-v2.c"
#endif


struct i2c_pads_info i2c_pad_info1 = {
	.scl = {
		.i2c_mode = MX6_PAD_CSI0_DAT9__I2C1_SCL | PC,
		.gpio_mode = MX6_PAD_CSI0_DAT9__GPIO5_IO27 | PC,
		.gp = IMX_GPIO_NR(5, 27)
	},
	.sda = {
		.i2c_mode = MX6_PAD_CSI0_DAT8__I2C1_SDA | PC,
		.gpio_mode = MX6_PAD_CSI0_DAT8__GPIO5_IO26 | PC,
		.gp = IMX_GPIO_NR(5, 26)
	}
};

struct i2c_pads_info i2c_pad_info2 = {
	.scl = {
		.i2c_mode = MX6_PAD_KEY_COL3__I2C2_SCL | PC,
		.gpio_mode = MX6_PAD_KEY_COL3__GPIO4_IO12 | PC,
		.gp = IMX_GPIO_NR(4, 12)
	},
	.sda = {
		.i2c_mode = MX6_PAD_KEY_ROW3__I2C2_SDA | PC,
		.gpio_mode = MX6_PAD_KEY_ROW3__GPIO4_IO13 | PC,
		.gp = IMX_GPIO_NR(4, 13)
	}
};

iomux_v3_cfg_t const usdhc1_pads[] = {
	MX6_PAD_SD1_CLK__SD1_CLK	| MUX_PAD_CTRL(USDHC_PAD_CTRL),
	MX6_PAD_SD1_CMD__SD1_CMD	| MUX_PAD_CTRL(USDHC_PAD_CTRL),
	MX6_PAD_SD1_DAT0__SD1_DATA0	| MUX_PAD_CTRL(USDHC_PAD_CTRL),
	MX6_PAD_SD1_DAT1__SD1_DATA1	| MUX_PAD_CTRL(USDHC_PAD_CTRL),
	MX6_PAD_SD1_DAT2__SD1_DATA2	| MUX_PAD_CTRL(USDHC_PAD_CTRL),
	MX6_PAD_SD1_DAT3__SD1_DATA3	| MUX_PAD_CTRL(USDHC_PAD_CTRL),
};

int dram_init(void)
{
	gd->ram_size = imx_ddr_size();

	return 0;
}

#ifdef CONFIG_FSL_ESDHC
struct fsl_esdhc_cfg usdhc_cfg[2] = {
	{USDHC1_BASE_ADDR},
	{USDHC2_BASE_ADDR},
};

int board_mmc_getcd(struct mmc *mmc)
{
	return 1;
}

int board_mmc_init(bd_t *bis)
{
	usdhc_cfg[0].sdhc_clk = mxc_get_clock(MXC_ESDHC_CLK);
	imx_iomux_v3_setup_multiple_pads(usdhc1_pads, ARRAY_SIZE(usdhc1_pads));
#if (CONFIG_SYS_BOARD_VERSION == 2)
	/*
	 * usdhc2 has a levelshifter on the carrier board Rev. DV1,
	 * that will automatically detect the driving direction.
	 * During initialisation this isn't working correctly,
	 * which causes DAT3 to be driven low towards the SD-card.
	 * This causes a SD-card enetring the SPI-Mode
	 * and therefore getting inaccessible until next power cycle.
	 * As workaround we drive the DAT3 line as GPIO and set it high.
	 * This makes usdhc2 unusable in u-boot, but works for the
	 * initialisation in Linux
	 */
	imx_iomux_v3_setup_pad(MX6_PAD_SD2_DAT3__GPIO1_IO12 |
			       MUX_PAD_CTRL(NO_PAD_CTRL));
	gpio_direction_output(IMX_GPIO_NR(1, 12) , 1);
#endif
	return fsl_esdhc_initialize(bis, &usdhc_cfg[0]);
}
#endif

/*
 * Do not overwrite the console
 * Use always serial for U-Boot console
 */
int overwrite_console(void)
{
	return 1;
}

struct display_info_t const displays[] = {
	{
		.bus	= -1,
		.addr	= 0,
		.pixfmt	= IPU_PIX_FMT_RGB24,
		.detect	= NULL,
		.enable	= enable_lvds,
		.mode	= {
			.name           = "lb07wv8",
			.refresh        = 60,
			.xres           = 800,
			.yres           = 480,
			.pixclock       = 30066,
			.left_margin    = 88,
			.right_margin   = 88,
			.upper_margin   = 20,
			.lower_margin   = 20,
			.hsync_len      = 80,
			.vsync_len      = 5,
			.sync           = FB_SYNC_EXT,
			.vmode          = FB_VMODE_NONINTERLACED
		}
	}
#if ((CONFIG_SYS_BOARD_VERSION == 2) || (CONFIG_SYS_BOARD_VERSION == 3))
	, {
		.bus	= -1,
		.addr	= 0,
		.pixfmt	= IPU_PIX_FMT_RGB24,
		.detect	= NULL,
		.enable	= enable_spi_display,
		.mode	= {
			.name           = "lg4573",
			.refresh        = 57,
			.xres           = 480,
			.yres           = 800,
			.pixclock       = 37037,
			.left_margin    = 59,
			.right_margin   = 10,
			.upper_margin   = 15,
			.lower_margin   = 15,
			.hsync_len      = 10,
			.vsync_len      = 15,
			.sync           = FB_SYNC_EXT | FB_SYNC_HOR_HIGH_ACT |
					  FB_SYNC_VERT_HIGH_ACT,
			.vmode          = FB_VMODE_NONINTERLACED
		}
	}
#endif
};
size_t display_count = ARRAY_SIZE(displays);

/* no console on this board */
int board_cfb_skip(void)
{
	return 1;
}

iomux_v3_cfg_t nfc_pads[] = {
	MX6_PAD_NANDF_CLE__NAND_CLE		| MUX_PAD_CTRL(NO_PAD_CTRL),
	MX6_PAD_NANDF_ALE__NAND_ALE		| MUX_PAD_CTRL(NO_PAD_CTRL),
	MX6_PAD_NANDF_WP_B__NAND_WP_B	| MUX_PAD_CTRL(NO_PAD_CTRL),
	MX6_PAD_NANDF_RB0__NAND_READY_B	| MUX_PAD_CTRL(NO_PAD_CTRL),
	MX6_PAD_NANDF_CS0__NAND_CE0_B		| MUX_PAD_CTRL(NO_PAD_CTRL),
	MX6_PAD_SD4_CMD__NAND_RE_B		| MUX_PAD_CTRL(NO_PAD_CTRL),
	MX6_PAD_SD4_CLK__NAND_WE_B		| MUX_PAD_CTRL(NO_PAD_CTRL),
	MX6_PAD_NANDF_D0__NAND_DATA00		| MUX_PAD_CTRL(NO_PAD_CTRL),
	MX6_PAD_NANDF_D1__NAND_DATA01		| MUX_PAD_CTRL(NO_PAD_CTRL),
	MX6_PAD_NANDF_D2__NAND_DATA02		| MUX_PAD_CTRL(NO_PAD_CTRL),
	MX6_PAD_NANDF_D3__NAND_DATA03		| MUX_PAD_CTRL(NO_PAD_CTRL),
	MX6_PAD_NANDF_D4__NAND_DATA04		| MUX_PAD_CTRL(NO_PAD_CTRL),
	MX6_PAD_NANDF_D5__NAND_DATA05		| MUX_PAD_CTRL(NO_PAD_CTRL),
	MX6_PAD_NANDF_D6__NAND_DATA06		| MUX_PAD_CTRL(NO_PAD_CTRL),
	MX6_PAD_NANDF_D7__NAND_DATA07		| MUX_PAD_CTRL(NO_PAD_CTRL),
	MX6_PAD_SD4_DAT0__NAND_DQS		| MUX_PAD_CTRL(NO_PAD_CTRL),
};

static void setup_gpmi_nand(void)
{
	struct mxc_ccm_reg *mxc_ccm = (struct mxc_ccm_reg *)CCM_BASE_ADDR;

	/* config gpmi nand iomux */
	imx_iomux_v3_setup_multiple_pads(nfc_pads,
					 ARRAY_SIZE(nfc_pads));

	/* gate ENFC_CLK_ROOT clock first,before clk source switch */
	clrbits_le32(&mxc_ccm->CCGR2, MXC_CCM_CCGR2_IOMUX_IPT_CLK_IO_MASK);

	/* config gpmi and bch clock to 100 MHz */
	clrsetbits_le32(&mxc_ccm->cs2cdr,
			MXC_CCM_CS2CDR_ENFC_CLK_PODF_MASK |
			MXC_CCM_CS2CDR_ENFC_CLK_PRED_MASK |
			MXC_CCM_CS2CDR_ENFC_CLK_SEL_MASK,
			MXC_CCM_CS2CDR_ENFC_CLK_PODF(0) |
			MXC_CCM_CS2CDR_ENFC_CLK_PRED(3) |
			MXC_CCM_CS2CDR_ENFC_CLK_SEL(3));

	/* enable ENFC_CLK_ROOT clock */
	setbits_le32(&mxc_ccm->CCGR2, MXC_CCM_CCGR2_IOMUX_IPT_CLK_IO_MASK);

	/* enable gpmi and bch clock gating */
	setbits_le32(&mxc_ccm->CCGR4,
		     MXC_CCM_CCGR4_RAWNAND_U_BCH_INPUT_APB_MASK |
		     MXC_CCM_CCGR4_RAWNAND_U_GPMI_BCH_INPUT_BCH_MASK |
		     MXC_CCM_CCGR4_RAWNAND_U_GPMI_BCH_INPUT_GPMI_IO_MASK |
		     MXC_CCM_CCGR4_RAWNAND_U_GPMI_INPUT_APB_MASK |
		     MXC_CCM_CCGR4_PL301_MX6QPER1_BCH_OFFSET);

	/* enable apbh clock gating */
	setbits_le32(&mxc_ccm->CCGR0, MXC_CCM_CCGR0_APBHDMA_MASK);
}

int board_init(void)
{
	struct iomuxc *iomux = (struct iomuxc *)IOMUXC_BASE_ADDR;

	/* address of boot parameters */
	gd->bd->bi_boot_params = PHYS_SDRAM + 0x100;

	setup_spi();

	setup_i2c(0, CONFIG_SYS_I2C_SPEED, CONFIG_SYS_I2C_SLAVE,
		  &i2c_pad_info1);
	setup_i2c(1, CONFIG_SYS_I2C_SPEED, CONFIG_SYS_I2C_SLAVE,
		  &i2c_pad_info2);
	setup_i2c(2, CONFIG_SYS_I2C_SPEED, CONFIG_SYS_I2C_SLAVE,
		  &i2c_pad_info3);
	setup_i2c4();

	/* SPI NOR Flash read only */
	gpio_request(CONFIG_GPIO_ENABLE_SPI_FLASH, "ena_spi_nor");
	gpio_direction_output(CONFIG_GPIO_ENABLE_SPI_FLASH, 0);
	gpio_free(CONFIG_GPIO_ENABLE_SPI_FLASH);

	setup_board_gpio();
	setup_gpmi_nand();
	setup_board_spi();

	/* GPIO_1 for USB_OTG_ID */
	clrsetbits_le32(&iomux->gpr[1], IOMUXC_GPR1_USB_OTG_ID_SEL_MASK, 0);
	imx_iomux_v3_setup_multiple_pads(misc_pads, ARRAY_SIZE(misc_pads));
	return 0;
}

int checkboard(void)
{
	printf("Board: %s\n", CONFIG_BOARDNAME);
	return 0;
}

#ifdef CONFIG_USB_EHCI_MX6
int board_ehci_hcd_init(int port)
{
	int ret;

	ret = gpio_request(ARISTAINETOS_USB_H1_PWR, "usb-h1-pwr");
	if (!ret)
		gpio_direction_output(ARISTAINETOS_USB_H1_PWR, 1);
	ret = gpio_request(ARISTAINETOS_USB_OTG_PWR, "usb-OTG-pwr");
	if (!ret)
		gpio_direction_output(ARISTAINETOS_USB_OTG_PWR, 1);
	return 0;
}

int board_ehci_power(int port, int on)
{
	if (port)
		gpio_set_value(ARISTAINETOS_USB_OTG_PWR, on);
	else
		gpio_set_value(ARISTAINETOS_USB_H1_PWR, on);
	return 0;
}
#endif

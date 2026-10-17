// SPDX-License-Identifier: GPL-2.0+
/*
 * Novena SPL
 *
 * Copyright (C) 2014 Marek Vasut <marex@denx.de>
 */

#include <common.h>
#include <clock_legacy.h>
#include <init.h>
#include <asm/io.h>
#include <asm/arch/clock.h>
#include <asm/arch/iomux.h>
#include <asm/arch/mx6-ddr.h>
#include <asm/arch/mx6-pins.h>
#include <asm/arch/sys_proto.h>
#include <asm/gpio.h>
#include <asm/mach-imx/boot_mode.h>
#include <asm/mach-imx/iomux-v3.h>
#include <asm/mach-imx/mxc_i2c.h>
#include <asm/arch/crm_regs.h>
#include <i2c.h>
#include <mmc.h>
#include <fsl_esdhc_imx.h>
#include <spl.h>
#include <linux/delay.h>

#include <asm/arch/mx6-ddr.h>

#include "novena.h"

#define UART_PAD_CTRL						\
	(PAD_CTL_PKE | PAD_CTL_PUE |				\
	PAD_CTL_PUS_100K_UP | PAD_CTL_SPEED_MED |		\
	PAD_CTL_DSE_40ohm   | PAD_CTL_SRE_FAST  | PAD_CTL_HYS)

#define USDHC_PAD_CTRL						\
	(PAD_CTL_PKE | PAD_CTL_PUE |				\
	PAD_CTL_PUS_47K_UP  | PAD_CTL_SPEED_LOW |		\
	PAD_CTL_DSE_80ohm   | PAD_CTL_SRE_FAST  | PAD_CTL_HYS)

#define ENET_PAD_CTRL						\
	(PAD_CTL_PKE | PAD_CTL_PUE |				\
	PAD_CTL_PUS_100K_UP | PAD_CTL_SPEED_MED	  |		\
	PAD_CTL_DSE_40ohm   | PAD_CTL_HYS)

#define ENET_PHY_CFG_PAD_CTRL					\
	(PAD_CTL_PKE | PAD_CTL_PUE |				\
	PAD_CTL_PUS_22K_UP | PAD_CTL_HYS)

#define RGMII_PAD_CTRL						\
	(PAD_CTL_PKE | PAD_CTL_PUE |				\
	PAD_CTL_PUS_100K_UP | PAD_CTL_DSE_40ohm | PAD_CTL_HYS)

#define SPI_PAD_CTRL						\
	(PAD_CTL_HYS |						\
	PAD_CTL_PUS_100K_DOWN | PAD_CTL_SPEED_MED |		\
	PAD_CTL_DSE_40ohm     | PAD_CTL_SRE_FAST)

#define I2C_PAD_CTRL						\
	(PAD_CTL_PKE | PAD_CTL_PUE |				\
	PAD_CTL_PUS_100K_UP | PAD_CTL_SPEED_LOW |		\
	PAD_CTL_DSE_240ohm  | PAD_CTL_HYS |			\
	PAD_CTL_ODE)

#define BUTTON_PAD_CTRL						\
	(PAD_CTL_PKE | PAD_CTL_PUE |				\
	PAD_CTL_PUS_100K_UP | PAD_CTL_SPEED_MED   |		\
	PAD_CTL_DSE_40ohm   | PAD_CTL_HYS)

#define PC MUX_PAD_CTRL(I2C_PAD_CTRL)

/*
 * Audio
 */
static iomux_v3_cfg_t audio_pads[] = {
	/* AUD_PWRON */
	MX6_PAD_DISP0_DAT23__GPIO5_IO17 | MUX_PAD_CTRL(NO_PAD_CTRL),
};

static void novena_spl_setup_iomux_audio(void)
{
	imx_iomux_v3_setup_multiple_pads(audio_pads, ARRAY_SIZE(audio_pads));
	gpio_direction_output(NOVENA_AUDIO_PWRON, 1);
}

/*
 * ENET
 */
static iomux_v3_cfg_t enet_pads1[] = {
	MX6_PAD_ENET_MDIO__ENET_MDIO		| MUX_PAD_CTRL(ENET_PAD_CTRL),
	MX6_PAD_ENET_MDC__ENET_MDC		| MUX_PAD_CTRL(ENET_PAD_CTRL),
	MX6_PAD_RGMII_TXC__RGMII_TXC		| MUX_PAD_CTRL(RGMII_PAD_CTRL),
	MX6_PAD_RGMII_TD0__RGMII_TD0		| MUX_PAD_CTRL(RGMII_PAD_CTRL),
	MX6_PAD_RGMII_TD1__RGMII_TD1		| MUX_PAD_CTRL(RGMII_PAD_CTRL),
	MX6_PAD_RGMII_TD2__RGMII_TD2		| MUX_PAD_CTRL(RGMII_PAD_CTRL),
	MX6_PAD_RGMII_TD3__RGMII_TD3		| MUX_PAD_CTRL(RGMII_PAD_CTRL),
	MX6_PAD_RGMII_TX_CTL__RGMII_TX_CTL	| MUX_PAD_CTRL(RGMII_PAD_CTRL),
	MX6_PAD_ENET_REF_CLK__ENET_TX_CLK	| MUX_PAD_CTRL(ENET_PAD_CTRL),

	/* pin 35, PHY_AD2 */
	MX6_PAD_RGMII_RXC__GPIO6_IO30	| MUX_PAD_CTRL(ENET_PHY_CFG_PAD_CTRL),
	/* pin 32, MODE0 */
	MX6_PAD_RGMII_RD0__GPIO6_IO25	| MUX_PAD_CTRL(ENET_PHY_CFG_PAD_CTRL),
	/* pin 31, MODE1 */
	MX6_PAD_RGMII_RD1__GPIO6_IO27	| MUX_PAD_CTRL(ENET_PHY_CFG_PAD_CTRL),
	/* pin 28, MODE2 */
	MX6_PAD_RGMII_RD2__GPIO6_IO28	| MUX_PAD_CTRL(ENET_PHY_CFG_PAD_CTRL),
	/* pin 27, MODE3 */
	MX6_PAD_RGMII_RD3__GPIO6_IO29	| MUX_PAD_CTRL(ENET_PHY_CFG_PAD_CTRL),
	/* pin 33, CLK125_EN */
	MX6_PAD_RGMII_RX_CTL__GPIO6_IO24 | MUX_PAD_CTRL(ENET_PHY_CFG_PAD_CTRL),

	/* pin 42 PHY nRST */
	MX6_PAD_EIM_D23__GPIO3_IO23		| MUX_PAD_CTRL(NO_PAD_CTRL),
};

static iomux_v3_cfg_t enet_pads2[] = {
	MX6_PAD_RGMII_RXC__RGMII_RXC		| MUX_PAD_CTRL(RGMII_PAD_CTRL),
	MX6_PAD_RGMII_RD0__RGMII_RD0		| MUX_PAD_CTRL(RGMII_PAD_CTRL),
	MX6_PAD_RGMII_RD1__RGMII_RD1		| MUX_PAD_CTRL(RGMII_PAD_CTRL),
	MX6_PAD_RGMII_RD2__RGMII_RD2		| MUX_PAD_CTRL(RGMII_PAD_CTRL),
	MX6_PAD_RGMII_RD3__RGMII_RD3		| MUX_PAD_CTRL(RGMII_PAD_CTRL),
	MX6_PAD_RGMII_RX_CTL__RGMII_RX_CTL	| MUX_PAD_CTRL(RGMII_PAD_CTRL),
};

static void novena_spl_setup_iomux_enet(void)
{
	imx_iomux_v3_setup_multiple_pads(enet_pads1, ARRAY_SIZE(enet_pads1));

	/* Assert Ethernet PHY nRST */
	gpio_direction_output(IMX_GPIO_NR(3, 23), 0);

	/*
	 * Use imx6 internal pull-ups to drive PHY mode pins during PHY reset
	 * de-assertion. The intention is to use weak signal drivers (pull-ups)
	 * to prevent the conflict between PHY pins becoming outputs after
	 * reset and imx6 still driving the pins. The issue is described in PHY
	 * datasheet, p.14
	 */
	gpio_direction_input(IMX_GPIO_NR(6, 30)); /* PHY_AD2 = 1 */
	gpio_direction_input(IMX_GPIO_NR(6, 25)); /* MODE0 = 1 */
	gpio_direction_input(IMX_GPIO_NR(6, 27)); /* MODE1 = 1 */
	gpio_direction_input(IMX_GPIO_NR(6, 28)); /* MODE2 = 1 */
	gpio_direction_input(IMX_GPIO_NR(6, 29)); /* MODE3 = 1 */
	gpio_direction_input(IMX_GPIO_NR(6, 24)); /* CLK125_EN = 1 */

	/* Following reset timing (p.53, fig.8 from the PHY datasheet) */
	mdelay(10);

	/* De-assert Ethernet PHY nRST */
	gpio_set_value(IMX_GPIO_NR(3, 23), 1);

	/* PHY is now configured, connect FEC to the pads */
	imx_iomux_v3_setup_multiple_pads(enet_pads2, ARRAY_SIZE(enet_pads2));

	/*
	 * PHY datasheet recommends on p.53 to wait at least 100us after reset
	 * before using MII, so we enforce the delay here
	 */
	udelay(100);
}

/*
 * FPGA
 */
static iomux_v3_cfg_t fpga_pads[] = {
	/* FPGA_RESET_N */
	MX6_PAD_DISP0_DAT13__GPIO5_IO07 | MUX_PAD_CTRL(NO_PAD_CTRL),
};

static void novena_spl_setup_iomux_fpga(void)
{
	imx_iomux_v3_setup_multiple_pads(fpga_pads, ARRAY_SIZE(fpga_pads));
	gpio_direction_output(NOVENA_FPGA_RESET_N_GPIO, 0);
}

/*
 * GPIO Button
 */
static iomux_v3_cfg_t button_pads[] = {
	/* Debug */
	MX6_PAD_KEY_COL4__GPIO4_IO14 | MUX_PAD_CTRL(BUTTON_PAD_CTRL),
};

static void novena_spl_setup_iomux_buttons(void)
{
	imx_iomux_v3_setup_multiple_pads(button_pads, ARRAY_SIZE(button_pads));
}

/*
 * I2C
 */
/*
 * I2C1:
 *  0x1d ... MMA7455L
 *  0x30 ... SO-DIMM temp sensor
 *  0x44 ... STMPE610
 *  0x50 ... SO-DIMM ID
 */
struct i2c_pads_info i2c_pad_info0 = {
	.scl = {
		.i2c_mode	= MX6_PAD_EIM_D21__I2C1_SCL | PC,
		.gpio_mode	= MX6_PAD_EIM_D21__GPIO3_IO21 | PC,
		.gp		= IMX_GPIO_NR(3, 21)
	},
	.sda = {
		.i2c_mode	= MX6_PAD_EIM_D28__I2C1_SDA | PC,
		.gpio_mode	= MX6_PAD_EIM_D28__GPIO3_IO28 | PC,
		.gp		= IMX_GPIO_NR(3, 28)
	}
};

/*
 * I2C2:
 *  0x08 ... PMIC
 *  0x3a ... HDMI DCC
 *  0x50 ... HDMI DCC
 */
static struct i2c_pads_info i2c_pad_info1 = {
	.scl = {
		.i2c_mode	= MX6_PAD_EIM_EB2__I2C2_SCL | PC,
		.gpio_mode	= MX6_PAD_EIM_EB2__GPIO2_IO30 | PC,
		.gp		= IMX_GPIO_NR(2, 30)
	},
	.sda = {
		.i2c_mode	= MX6_PAD_EIM_D16__I2C2_SDA | PC,
		.gpio_mode	= MX6_PAD_EIM_D16__GPIO3_IO16 | PC,
		.gp		= IMX_GPIO_NR(3, 16)
	}
};

/*
 * I2C3:
 *  0x11 ... ES8283
 *  0x50 ... LCD EDID
 *  0x56 ... EEPROM
 */
static struct i2c_pads_info i2c_pad_info2 = {
	.scl = {
		.i2c_mode	= MX6_PAD_EIM_D17__I2C3_SCL | PC,
		.gpio_mode	= MX6_PAD_EIM_D17__GPIO3_IO17 | PC,
		.gp		= IMX_GPIO_NR(3, 17)
	},
	.sda = {
		.i2c_mode	= MX6_PAD_EIM_D18__I2C3_SDA | PC,
		.gpio_mode	= MX6_PAD_EIM_D18__GPIO3_IO18 | PC,
		.gp		= IMX_GPIO_NR(3, 18)
	}
};

static void novena_spl_setup_iomux_i2c(void)
{
	setup_i2c(0, CONFIG_SYS_I2C_SPEED, 0x7f, &i2c_pad_info0);
	setup_i2c(1, CONFIG_SYS_I2C_SPEED, 0x7f, &i2c_pad_info1);
	setup_i2c(2, CONFIG_SYS_I2C_SPEED, 0x7f, &i2c_pad_info2);
}

/*
 * PCI express
 */
#ifdef CONFIG_CMD_PCI
static iomux_v3_cfg_t pcie_pads[] = {
	/* "Reset" pin */
	MX6_PAD_EIM_D29__GPIO3_IO29 | MUX_PAD_CTRL(NO_PAD_CTRL),
	/* "Power on" pin */
	MX6_PAD_GPIO_17__GPIO7_IO12 | MUX_PAD_CTRL(NO_PAD_CTRL),
	/* "Wake up" pin (input) */
	MX6_PAD_EIM_D22__GPIO3_IO22 | MUX_PAD_CTRL(NO_PAD_CTRL),
	/* "Disable endpoint" (rfkill) pin */
	MX6_PAD_EIM_A22__GPIO2_IO16 | MUX_PAD_CTRL(NO_PAD_CTRL),
};

static void novena_spl_setup_iomux_pcie(void)
{
	imx_iomux_v3_setup_multiple_pads(pcie_pads, ARRAY_SIZE(pcie_pads));

	/* Ensure PCIe is powered down */
	gpio_direction_output(NOVENA_PCIE_POWER_ON_GPIO, 0);

	/* Put the card into reset */
	gpio_direction_output(NOVENA_PCIE_RESET_GPIO, 0);

	/* Input signal to wake system from mPCIe card */
	gpio_direction_input(NOVENA_PCIE_WAKE_UP_GPIO);

	/* Drive RFKILL high, to ensure the radio is turned on */
	gpio_direction_output(NOVENA_PCIE_DISABLE_GPIO, 1);
}
#else
static inline void novena_spl_setup_iomux_pcie(void) {}
#endif

/*
 * SDHC
 */
static iomux_v3_cfg_t usdhc2_pads[] = {
	MX6_PAD_SD2_CLK__SD2_CLK    | MUX_PAD_CTRL(USDHC_PAD_CTRL),
	MX6_PAD_SD2_CMD__SD2_CMD    | MUX_PAD_CTRL(USDHC_PAD_CTRL),
	MX6_PAD_SD2_DAT0__SD2_DATA0 | MUX_PAD_CTRL(USDHC_PAD_CTRL),
	MX6_PAD_SD2_DAT1__SD2_DATA1 | MUX_PAD_CTRL(USDHC_PAD_CTRL),
	MX6_PAD_SD2_DAT2__SD2_DATA2 | MUX_PAD_CTRL(USDHC_PAD_CTRL),
	MX6_PAD_SD2_DAT3__SD2_DATA3 | MUX_PAD_CTRL(USDHC_PAD_CTRL),
	MX6_PAD_GPIO_2__GPIO1_IO02  | MUX_PAD_CTRL(NO_PAD_CTRL), /* WP */
	MX6_PAD_GPIO_4__GPIO1_IO04  | MUX_PAD_CTRL(NO_PAD_CTRL), /* CD */
};

static iomux_v3_cfg_t usdhc3_pads[] = {
	MX6_PAD_SD3_CLK__SD3_CLK    | MUX_PAD_CTRL(USDHC_PAD_CTRL),
	MX6_PAD_SD3_CMD__SD3_CMD    | MUX_PAD_CTRL(USDHC_PAD_CTRL),
	MX6_PAD_SD3_DAT0__SD3_DATA0 | MUX_PAD_CTRL(USDHC_PAD_CTRL),
	MX6_PAD_SD3_DAT1__SD3_DATA1 | MUX_PAD_CTRL(USDHC_PAD_CTRL),
	MX6_PAD_SD3_DAT2__SD3_DATA2 | MUX_PAD_CTRL(USDHC_PAD_CTRL),
	MX6_PAD_SD3_DAT3__SD3_DATA3 | MUX_PAD_CTRL(USDHC_PAD_CTRL),
};

static void novena_spl_setup_iomux_sdhc(void)
{
	imx_iomux_v3_setup_multiple_pads(usdhc2_pads, ARRAY_SIZE(usdhc2_pads));
	imx_iomux_v3_setup_multiple_pads(usdhc3_pads, ARRAY_SIZE(usdhc3_pads));

	/* Big SD write-protect and card-detect */
	gpio_direction_input(IMX_GPIO_NR(1, 2));
	gpio_direction_input(IMX_GPIO_NR(1, 4));
}

/*
 * SPI
 */
#ifdef CONFIG_MXC_SPI
static iomux_v3_cfg_t ecspi3_pads[] = {
	/* SS1 */
	MX6_PAD_DISP0_DAT1__ECSPI3_MOSI | MUX_PAD_CTRL(SPI_PAD_CTRL),
	MX6_PAD_DISP0_DAT2__ECSPI3_MISO | MUX_PAD_CTRL(SPI_PAD_CTRL),
	MX6_PAD_DISP0_DAT0__ECSPI3_SCLK | MUX_PAD_CTRL(SPI_PAD_CTRL),
	MX6_PAD_DISP0_DAT3__GPIO4_IO24 | MUX_PAD_CTRL(SPI_PAD_CTRL),
	MX6_PAD_DISP0_DAT4__GPIO4_IO25 | MUX_PAD_CTRL(SPI_PAD_CTRL),
	MX6_PAD_DISP0_DAT5__GPIO4_IO26 | MUX_PAD_CTRL(SPI_PAD_CTRL),
	MX6_PAD_DISP0_DAT7__ECSPI3_RDY | MUX_PAD_CTRL(SPI_PAD_CTRL),
};

static void novena_spl_setup_iomux_spi(void)
{
	imx_iomux_v3_setup_multiple_pads(ecspi3_pads, ARRAY_SIZE(ecspi3_pads));
	/* De-assert the nCS */
	gpio_direction_output(MX6_PAD_DISP0_DAT3__GPIO4_IO24, 1);
	gpio_direction_output(MX6_PAD_DISP0_DAT4__GPIO4_IO25, 1);
	gpio_direction_output(MX6_PAD_DISP0_DAT5__GPIO4_IO26, 1);
}
#else
static void novena_spl_setup_iomux_spi(void) {}
#endif

/*
 * UART
 */
static iomux_v3_cfg_t const uart2_pads[] = {
	MX6_PAD_EIM_D26__UART2_TX_DATA | MUX_PAD_CTRL(UART_PAD_CTRL),
	MX6_PAD_EIM_D27__UART2_RX_DATA | MUX_PAD_CTRL(UART_PAD_CTRL),
};

static iomux_v3_cfg_t const uart3_pads[] = {
	MX6_PAD_EIM_D24__UART3_TX_DATA | MUX_PAD_CTRL(UART_PAD_CTRL),
	MX6_PAD_EIM_D25__UART3_RX_DATA | MUX_PAD_CTRL(UART_PAD_CTRL),
};

static iomux_v3_cfg_t const uart4_pads[] = {
	MX6_PAD_KEY_COL0__UART4_TX_DATA | MUX_PAD_CTRL(UART_PAD_CTRL),
	MX6_PAD_KEY_ROW0__UART4_RX_DATA | MUX_PAD_CTRL(UART_PAD_CTRL),
	MX6_PAD_CSI0_DAT16__UART4_CTS_B | MUX_PAD_CTRL(UART_PAD_CTRL),
	MX6_PAD_CSI0_DAT17__UART4_RTS_B | MUX_PAD_CTRL(UART_PAD_CTRL),

};

static void novena_spl_setup_iomux_uart(void)
{
	imx_iomux_v3_setup_multiple_pads(uart2_pads, ARRAY_SIZE(uart2_pads));
	imx_iomux_v3_setup_multiple_pads(uart3_pads, ARRAY_SIZE(uart3_pads));
	imx_iomux_v3_setup_multiple_pads(uart4_pads, ARRAY_SIZE(uart4_pads));
}

static inline void novena_spl_setup_iomux_video(void) {}

/*
 * SPL boots from uSDHC card
 */
#ifdef CONFIG_FSL_ESDHC_IMX
static struct fsl_esdhc_cfg usdhc_cfg = {
	USDHC3_BASE_ADDR, 0, 4
};

int board_mmc_getcd(struct mmc *mmc)
{
	/* There is no CD for a microSD card, assume always present. */
	return 1;
}

int board_mmc_init(struct bd_info *bis)
{
	usdhc_cfg.sdhc_clk = mxc_get_clock(MXC_ESDHC3_CLK);
	return fsl_esdhc_initialize(bis, &usdhc_cfg);
}
#endif

/* Configure MX6Q/DUAL mmdc DDR io registers */
static struct mx6dq_iomux_ddr_regs novena_ddr_ioregs = {
	/* SDCLK[0:1], CAS, RAS, Reset: Differential input, 40ohm */
	.dram_sdclk_0		= 0x00020038,
	.dram_sdclk_1		= 0x00020038,
	.dram_cas		= 0x00000038,
	.dram_ras		= 0x00000038,
	.dram_reset		= 0x00000038,
	/* SDCKE[0:1]: 100k pull-up */
	.dram_sdcke0		= 0x00000038,
	.dram_sdcke1		= 0x00000038,
	/* SDBA2: pull-up disabled */
	.dram_sdba2		= 0x00000000,
	/* SDODT[0:1]: 100k pull-up, 40 ohm */
	.dram_sdodt0		= 0x00000038,
	.dram_sdodt1		= 0x00000038,
	/* SDQS[0:7]: Differential input, 40 ohm */
	.dram_sdqs0		= 0x00000038,
	.dram_sdqs1		= 0x00000038,
	.dram_sdqs2		= 0x00000038,
	.dram_sdqs3		= 0x00000038,
	.dram_sdqs4		= 0x00000038,
	.dram_sdqs5		= 0x00000038,
	.dram_sdqs6		= 0x00000038,
	.dram_sdqs7		= 0x00000038,

	/* DQM[0:7]: Differential input, 40 ohm */
	.dram_dqm0		= 0x00000038,
	.dram_dqm1		= 0x00000038,
	.dram_dqm2		= 0x00000038,
	.dram_dqm3		= 0x00000038,
	.dram_dqm4		= 0x00000038,
	.dram_dqm5		= 0x00000038,
	.dram_dqm6		= 0x00000038,
	.dram_dqm7		= 0x00000038,
};

/* Configure MX6Q/DUAL mmdc GRP io registers */
static struct mx6dq_iomux_grp_regs novena_grp_ioregs = {
	/* DDR3 */
	.grp_ddr_type		= 0x000c0000,
	.grp_ddrmode_ctl	= 0x00020000,
	/* Disable DDR pullups */
	.grp_ddrpke		= 0x00000000,
	/* ADDR[00:16], SDBA[0:1]: 40 ohm */
	.grp_addds		= 0x00000038,
	/* CS0/CS1/SDBA2/CKE0/CKE1/SDWE: 40 ohm */
	.grp_ctlds		= 0x00000038,
	/* DATA[00:63]: Differential input, 40 ohm */
	.grp_ddrmode		= 0x00020000,
	.grp_b0ds		= 0x00000038,
	.grp_b1ds		= 0x00000038,
	.grp_b2ds		= 0x00000038,
	.grp_b3ds		= 0x00000038,
	.grp_b4ds		= 0x00000038,
	.grp_b5ds		= 0x00000038,
	.grp_b6ds		= 0x00000038,
	.grp_b7ds		= 0x00000038,
};

static struct mx6_mmdc_calibration novena_mmdc_calib = {
	/* write leveling calibration determine */
	.p0_mpwldectrl0		= 0x00420048,
	.p0_mpwldectrl1		= 0x006f0059,
	.p1_mpwldectrl0		= 0x005a0104,
	.p1_mpwldectrl1		= 0x01070113,
	/* Read DQS Gating calibration */
	.p0_mpdgctrl0		= 0x437c040b,
	.p0_mpdgctrl1		= 0x0413040e,
	.p1_mpdgctrl0		= 0x444f0446,
	.p1_mpdgctrl1		= 0x044d0422,
	/* Read Calibration: DQS delay relative to DQ read access */
	.p0_mprddlctl		= 0x4c424249,
	.p1_mprddlctl		= 0x4e48414f,
	/* Write Calibration: DQ/DM delay relative to DQS write access */
	.p0_mpwrdlctl		= 0x42414641,
	.p1_mpwrdlctl		= 0x46374b43,
};

static struct mx6_ddr_sysinfo novena_ddr_info = {
	/* Width of data bus: 0=16, 1=32, 2=64 */
	.dsize		= 2,
	/* Config for full 4GB range so that get_mem_size() works */
	.cs_density	= 32,	/* 32Gb per CS */
	/* Single chip select */
	.ncs		= 1,
	.cs1_mirror	= 0,
	.rtt_wr		= 0,	/* RTT_Wr = RZQ/4 */
	.rtt_nom	= 1,	/* RTT_Nom = RZQ/2 */
	.walat		= 0,	/* Write additional latency */
	.ralat		= 5,	/* Read additional latency */
	.mif3_mode	= 3,	/* Command prediction working mode */
	.bi_on		= 1,	/* Bank interleaving enabled */
	.sde_to_rst	= 0x10,	/* 14 cycles, 200us (JEDEC default) */
	.rst_to_cke	= 0x23,	/* 33 cycles, 500us (JEDEC default) */
	.refsel = 1,	/* Refresh cycles at 32KHz */
	.refr = 7,	/* 8 refresh commands per refresh cycle */
};

static struct mx6_ddr3_cfg elpida_4gib_1600 = {
	.mem_speed	= 1600,
	.density	= 4,
	.width		= 64,
	.banks		= 8,
	.rowaddr	= 16,
	.coladdr	= 10,
	.pagesz		= 2,
	.trcd		= 1375,
	.trcmin		= 4875,
	.trasmin	= 3500,
};

static void ccgr_init(void)
{
	struct mxc_ccm_reg *ccm = (struct mxc_ccm_reg *)CCM_BASE_ADDR;

	writel(0x00C03F3F, &ccm->CCGR0);
	writel(0x0030FC03, &ccm->CCGR1);
	writel(0x0FFFC000, &ccm->CCGR2);
	writel(0x3FF00000, &ccm->CCGR3);
	writel(0xFFFFF300, &ccm->CCGR4);
	writel(0x0F0000C3, &ccm->CCGR5);
	writel(0x000003FF, &ccm->CCGR6);
}

/*
 * called from C runtime startup code (arch/arm/lib/crt0.S:_main)
 * - we have a stack and a place to store GD, both in SRAM
 * - no variable global data is available
 */
void board_init_f(ulong dummy)
{
	/* setup AIPS and disable watchdog */
	arch_cpu_init();

	ccgr_init();
	gpr_init();

	/* setup GP timer */
	timer_init();

#ifdef CONFIG_BOARD_POSTCLK_INIT
	board_postclk_init();
#endif
#ifdef CONFIG_FSL_ESDHC_IMX
	get_clocks();
#endif

	/* Setup IOMUX and configure basics. */
	novena_spl_setup_iomux_audio();
	novena_spl_setup_iomux_buttons();
	novena_spl_setup_iomux_enet();
	novena_spl_setup_iomux_fpga();
	novena_spl_setup_iomux_i2c();
	novena_spl_setup_iomux_pcie();
	novena_spl_setup_iomux_sdhc();
	novena_spl_setup_iomux_spi();
	novena_spl_setup_iomux_uart();
	novena_spl_setup_iomux_video();

	/* UART clocks enabled and gd valid - init serial console */
	preloader_console_init();

	/* Start the DDR DRAM */
	mx6dq_dram_iocfg(64, &novena_ddr_ioregs, &novena_grp_ioregs);
	mx6_dram_cfg(&novena_ddr_info, &novena_mmdc_calib, &elpida_4gib_1600);

	/* Perform DDR DRAM calibration */
	udelay(100);
	mmdc_do_write_level_calibration(&novena_ddr_info);
	mmdc_do_dqs_calibration(&novena_ddr_info);
}

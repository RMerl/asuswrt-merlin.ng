// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright (C) 2012 Freescale Semiconductor, Inc.
 *
 * Author: Fabio Estevam <fabio.estevam@freescale.com>
 */

#include <common.h>
#include <asm/io.h>
#include <asm/arch/clock.h>
#include <asm/arch/imx-regs.h>
#include <asm/arch/iomux.h>
#include <asm/arch/mx6-pins.h>
#include <linux/errno.h>
#include <asm/gpio.h>
#include <asm/mach-imx/iomux-v3.h>
#include <asm/mach-imx/mxc_i2c.h>
#include <asm/mach-imx/boot_mode.h>
#include <asm/mach-imx/spi.h>
#include <mmc.h>
#include <fsl_esdhc.h>
#include <miiphy.h>
#include <netdev.h>
#include <asm/arch/sys_proto.h>
#include <i2c.h>
#include <input.h>
#include <asm/arch/mxc_hdmi.h>
#include <asm/mach-imx/video.h>
#include <asm/arch/crm_regs.h>
#include <pca953x.h>
#include <power/pmic.h>
#include <power/pfuze100_pmic.h>
#include "../common/pfuze.h"

DECLARE_GLOBAL_DATA_PTR;

#define UART_PAD_CTRL  (PAD_CTL_PUS_100K_UP |			\
	PAD_CTL_SPEED_MED | PAD_CTL_DSE_40ohm |			\
	PAD_CTL_SRE_FAST  | PAD_CTL_HYS)

#define USDHC_PAD_CTRL (PAD_CTL_PUS_47K_UP |			\
	PAD_CTL_SPEED_LOW | PAD_CTL_DSE_80ohm |			\
	PAD_CTL_SRE_FAST  | PAD_CTL_HYS)

#define ENET_PAD_CTRL  (PAD_CTL_PUS_100K_UP |			\
	PAD_CTL_SPEED_MED | PAD_CTL_DSE_40ohm | PAD_CTL_HYS)

#define I2C_PAD_CTRL	(PAD_CTL_PUS_100K_UP |			\
	PAD_CTL_SPEED_MED | PAD_CTL_DSE_40ohm | PAD_CTL_HYS |	\
	PAD_CTL_ODE | PAD_CTL_SRE_FAST)

#define GPMI_PAD_CTRL0 (PAD_CTL_PKE | PAD_CTL_PUE | PAD_CTL_PUS_100K_UP)
#define GPMI_PAD_CTRL1 (PAD_CTL_DSE_40ohm | PAD_CTL_SPEED_MED | \
			PAD_CTL_SRE_FAST)
#define GPMI_PAD_CTRL2 (GPMI_PAD_CTRL0 | GPMI_PAD_CTRL1)

#define PC MUX_PAD_CTRL(I2C_PAD_CTRL)

#define WEIM_NOR_PAD_CTRL (PAD_CTL_PKE | PAD_CTL_PUE |          \
	PAD_CTL_PUS_100K_UP | PAD_CTL_SPEED_MED |               \
	PAD_CTL_DSE_40ohm   | PAD_CTL_SRE_FAST)

#define I2C_PMIC	1

int dram_init(void)
{
	gd->ram_size = imx_ddr_size();

	return 0;
}

static iomux_v3_cfg_t const uart4_pads[] = {
	IOMUX_PADS(PAD_KEY_COL0__UART4_TX_DATA | MUX_PAD_CTRL(UART_PAD_CTRL)),
	IOMUX_PADS(PAD_KEY_ROW0__UART4_RX_DATA | MUX_PAD_CTRL(UART_PAD_CTRL)),
};

static iomux_v3_cfg_t const enet_pads[] = {
	IOMUX_PADS(PAD_KEY_COL1__ENET_MDIO		| MUX_PAD_CTRL(ENET_PAD_CTRL)),
	IOMUX_PADS(PAD_KEY_COL2__ENET_MDC		| MUX_PAD_CTRL(ENET_PAD_CTRL)),
	IOMUX_PADS(PAD_RGMII_TXC__RGMII_TXC		| MUX_PAD_CTRL(ENET_PAD_CTRL)),
	IOMUX_PADS(PAD_RGMII_TD0__RGMII_TD0		| MUX_PAD_CTRL(ENET_PAD_CTRL)),
	IOMUX_PADS(PAD_RGMII_TD1__RGMII_TD1		| MUX_PAD_CTRL(ENET_PAD_CTRL)),
	IOMUX_PADS(PAD_RGMII_TD2__RGMII_TD2		| MUX_PAD_CTRL(ENET_PAD_CTRL)),
	IOMUX_PADS(PAD_RGMII_TD3__RGMII_TD3		| MUX_PAD_CTRL(ENET_PAD_CTRL)),
	IOMUX_PADS(PAD_RGMII_TX_CTL__RGMII_TX_CTL	| MUX_PAD_CTRL(ENET_PAD_CTRL)),
	IOMUX_PADS(PAD_ENET_REF_CLK__ENET_TX_CLK	| MUX_PAD_CTRL(ENET_PAD_CTRL)),
	IOMUX_PADS(PAD_RGMII_RXC__RGMII_RXC		| MUX_PAD_CTRL(ENET_PAD_CTRL)),
	IOMUX_PADS(PAD_RGMII_RD0__RGMII_RD0		| MUX_PAD_CTRL(ENET_PAD_CTRL)),
	IOMUX_PADS(PAD_RGMII_RD1__RGMII_RD1		| MUX_PAD_CTRL(ENET_PAD_CTRL)),
	IOMUX_PADS(PAD_RGMII_RD2__RGMII_RD2		| MUX_PAD_CTRL(ENET_PAD_CTRL)),
	IOMUX_PADS(PAD_RGMII_RD3__RGMII_RD3		| MUX_PAD_CTRL(ENET_PAD_CTRL)),
	IOMUX_PADS(PAD_RGMII_RX_CTL__RGMII_RX_CTL	| MUX_PAD_CTRL(ENET_PAD_CTRL)),
};

/* I2C2 PMIC, iPod, Tuner, Codec, Touch, HDMI EDID, MIPI CSI2 card */
static struct i2c_pads_info mx6q_i2c_pad_info1 = {
	.scl = {
		.i2c_mode = MX6Q_PAD_EIM_EB2__I2C2_SCL | PC,
		.gpio_mode = MX6Q_PAD_EIM_EB2__GPIO2_IO30 | PC,
		.gp = IMX_GPIO_NR(2, 30)
	},
	.sda = {
		.i2c_mode = MX6Q_PAD_KEY_ROW3__I2C2_SDA | PC,
		.gpio_mode = MX6Q_PAD_KEY_ROW3__GPIO4_IO13 | PC,
		.gp = IMX_GPIO_NR(4, 13)
	}
};

static struct i2c_pads_info mx6dl_i2c_pad_info1 = {
	.scl = {
		.i2c_mode = MX6DL_PAD_EIM_EB2__I2C2_SCL | PC,
		.gpio_mode = MX6DL_PAD_EIM_EB2__GPIO2_IO30 | PC,
		.gp = IMX_GPIO_NR(2, 30)
	},
	.sda = {
		.i2c_mode = MX6DL_PAD_KEY_ROW3__I2C2_SDA | PC,
		.gpio_mode = MX6DL_PAD_KEY_ROW3__GPIO4_IO13 | PC,
		.gp = IMX_GPIO_NR(4, 13)
	}
};

#ifndef CONFIG_SYS_FLASH_CFI
/*
 * I2C3 MLB, Port Expanders (A, B, C), Video ADC, Light Sensor,
 * Compass Sensor, Accelerometer, Res Touch
 */
static struct i2c_pads_info mx6q_i2c_pad_info2 = {
	.scl = {
		.i2c_mode = MX6Q_PAD_GPIO_3__I2C3_SCL | PC,
		.gpio_mode = MX6Q_PAD_GPIO_3__GPIO1_IO03 | PC,
		.gp = IMX_GPIO_NR(1, 3)
	},
	.sda = {
		.i2c_mode = MX6Q_PAD_EIM_D18__I2C3_SDA | PC,
		.gpio_mode = MX6Q_PAD_EIM_D18__GPIO3_IO18 | PC,
		.gp = IMX_GPIO_NR(3, 18)
	}
};

static struct i2c_pads_info mx6dl_i2c_pad_info2 = {
	.scl = {
		.i2c_mode = MX6DL_PAD_GPIO_3__I2C3_SCL | PC,
		.gpio_mode = MX6DL_PAD_GPIO_3__GPIO1_IO03 | PC,
		.gp = IMX_GPIO_NR(1, 3)
	},
	.sda = {
		.i2c_mode = MX6DL_PAD_EIM_D18__I2C3_SDA | PC,
		.gpio_mode = MX6DL_PAD_EIM_D18__GPIO3_IO18 | PC,
		.gp = IMX_GPIO_NR(3, 18)
	}
};
#endif

static iomux_v3_cfg_t const i2c3_pads[] = {
	IOMUX_PADS(PAD_EIM_A24__GPIO5_IO04	| MUX_PAD_CTRL(NO_PAD_CTRL)),
};

static iomux_v3_cfg_t const port_exp[] = {
	IOMUX_PADS(PAD_SD2_DAT0__GPIO1_IO15	| MUX_PAD_CTRL(NO_PAD_CTRL)),
};

#ifdef CONFIG_MTD_NOR_FLASH
static iomux_v3_cfg_t const eimnor_pads[] = {
	IOMUX_PADS(PAD_EIM_D16__EIM_DATA16	| MUX_PAD_CTRL(WEIM_NOR_PAD_CTRL)),
	IOMUX_PADS(PAD_EIM_D17__EIM_DATA17	| MUX_PAD_CTRL(WEIM_NOR_PAD_CTRL)),
	IOMUX_PADS(PAD_EIM_D18__EIM_DATA18	| MUX_PAD_CTRL(WEIM_NOR_PAD_CTRL)),
	IOMUX_PADS(PAD_EIM_D19__EIM_DATA19	| MUX_PAD_CTRL(WEIM_NOR_PAD_CTRL)),
	IOMUX_PADS(PAD_EIM_D20__EIM_DATA20	| MUX_PAD_CTRL(WEIM_NOR_PAD_CTRL)),
	IOMUX_PADS(PAD_EIM_D21__EIM_DATA21	| MUX_PAD_CTRL(WEIM_NOR_PAD_CTRL)),
	IOMUX_PADS(PAD_EIM_D22__EIM_DATA22	| MUX_PAD_CTRL(WEIM_NOR_PAD_CTRL)),
	IOMUX_PADS(PAD_EIM_D23__EIM_DATA23	| MUX_PAD_CTRL(WEIM_NOR_PAD_CTRL)),
	IOMUX_PADS(PAD_EIM_D24__EIM_DATA24	| MUX_PAD_CTRL(WEIM_NOR_PAD_CTRL)),
	IOMUX_PADS(PAD_EIM_D25__EIM_DATA25	| MUX_PAD_CTRL(WEIM_NOR_PAD_CTRL)),
	IOMUX_PADS(PAD_EIM_D26__EIM_DATA26	| MUX_PAD_CTRL(WEIM_NOR_PAD_CTRL)),
	IOMUX_PADS(PAD_EIM_D27__EIM_DATA27	| MUX_PAD_CTRL(WEIM_NOR_PAD_CTRL)),
	IOMUX_PADS(PAD_EIM_D28__EIM_DATA28	| MUX_PAD_CTRL(WEIM_NOR_PAD_CTRL)),
	IOMUX_PADS(PAD_EIM_D29__EIM_DATA29	| MUX_PAD_CTRL(WEIM_NOR_PAD_CTRL)),
	IOMUX_PADS(PAD_EIM_D30__EIM_DATA30	| MUX_PAD_CTRL(WEIM_NOR_PAD_CTRL)),
	IOMUX_PADS(PAD_EIM_D31__EIM_DATA31	| MUX_PAD_CTRL(WEIM_NOR_PAD_CTRL)),
	IOMUX_PADS(PAD_EIM_DA0__EIM_AD00	| MUX_PAD_CTRL(WEIM_NOR_PAD_CTRL)),
	IOMUX_PADS(PAD_EIM_DA1__EIM_AD01	| MUX_PAD_CTRL(WEIM_NOR_PAD_CTRL)),
	IOMUX_PADS(PAD_EIM_DA2__EIM_AD02	| MUX_PAD_CTRL(WEIM_NOR_PAD_CTRL)),
	IOMUX_PADS(PAD_EIM_DA3__EIM_AD03	| MUX_PAD_CTRL(WEIM_NOR_PAD_CTRL)),
	IOMUX_PADS(PAD_EIM_DA4__EIM_AD04	| MUX_PAD_CTRL(WEIM_NOR_PAD_CTRL)),
	IOMUX_PADS(PAD_EIM_DA5__EIM_AD05	| MUX_PAD_CTRL(WEIM_NOR_PAD_CTRL)),
	IOMUX_PADS(PAD_EIM_DA6__EIM_AD06	| MUX_PAD_CTRL(WEIM_NOR_PAD_CTRL)),
	IOMUX_PADS(PAD_EIM_DA7__EIM_AD07	| MUX_PAD_CTRL(WEIM_NOR_PAD_CTRL)),
	IOMUX_PADS(PAD_EIM_DA8__EIM_AD08	| MUX_PAD_CTRL(WEIM_NOR_PAD_CTRL)),
	IOMUX_PADS(PAD_EIM_DA9__EIM_AD09	| MUX_PAD_CTRL(WEIM_NOR_PAD_CTRL)),
	IOMUX_PADS(PAD_EIM_DA10__EIM_AD10	| MUX_PAD_CTRL(WEIM_NOR_PAD_CTRL)),
	IOMUX_PADS(PAD_EIM_DA11__EIM_AD11	| MUX_PAD_CTRL(WEIM_NOR_PAD_CTRL)),
	IOMUX_PADS(PAD_EIM_DA12__EIM_AD12	| MUX_PAD_CTRL(WEIM_NOR_PAD_CTRL)),
	IOMUX_PADS(PAD_EIM_DA13__EIM_AD13	| MUX_PAD_CTRL(WEIM_NOR_PAD_CTRL)),
	IOMUX_PADS(PAD_EIM_DA14__EIM_AD14	| MUX_PAD_CTRL(WEIM_NOR_PAD_CTRL)),
	IOMUX_PADS(PAD_EIM_DA15__EIM_AD15	| MUX_PAD_CTRL(WEIM_NOR_PAD_CTRL)),
	IOMUX_PADS(PAD_EIM_A16__EIM_ADDR16	| MUX_PAD_CTRL(WEIM_NOR_PAD_CTRL)),
	IOMUX_PADS(PAD_EIM_A17__EIM_ADDR17	| MUX_PAD_CTRL(WEIM_NOR_PAD_CTRL)),
	IOMUX_PADS(PAD_EIM_A18__EIM_ADDR18	| MUX_PAD_CTRL(WEIM_NOR_PAD_CTRL)),
	IOMUX_PADS(PAD_EIM_A19__EIM_ADDR19	| MUX_PAD_CTRL(WEIM_NOR_PAD_CTRL)),
	IOMUX_PADS(PAD_EIM_A20__EIM_ADDR20	| MUX_PAD_CTRL(WEIM_NOR_PAD_CTRL)),
	IOMUX_PADS(PAD_EIM_A21__EIM_ADDR21	| MUX_PAD_CTRL(WEIM_NOR_PAD_CTRL)),
	IOMUX_PADS(PAD_EIM_A22__EIM_ADDR22	| MUX_PAD_CTRL(WEIM_NOR_PAD_CTRL)),
	IOMUX_PADS(PAD_EIM_A23__EIM_ADDR23	| MUX_PAD_CTRL(WEIM_NOR_PAD_CTRL)),
	IOMUX_PADS(PAD_EIM_OE__EIM_OE_B		| MUX_PAD_CTRL(NO_PAD_CTRL)),
	IOMUX_PADS(PAD_EIM_RW__EIM_RW		| MUX_PAD_CTRL(NO_PAD_CTRL)),
	IOMUX_PADS(PAD_EIM_CS0__EIM_CS0_B	| MUX_PAD_CTRL(NO_PAD_CTRL)),
};

static void eimnor_cs_setup(void)
{
	struct weim *weim_regs = (struct weim *)WEIM_BASE_ADDR;

	writel(0x00020181, &weim_regs->cs0gcr1);
	writel(0x00000001, &weim_regs->cs0gcr2);
	writel(0x0a020000, &weim_regs->cs0rcr1);
	writel(0x0000c000, &weim_regs->cs0rcr2);
	writel(0x0804a240, &weim_regs->cs0wcr1);
	writel(0x00000120, &weim_regs->wcr);

	set_chipselect_size(CS0_128);
}

static void eim_clk_setup(void)
{
	struct mxc_ccm_reg *imx_ccm = (struct mxc_ccm_reg *)CCM_BASE_ADDR;
	int cscmr1, ccgr6;


	/* Turn off EIM clock */
	ccgr6 = readl(&imx_ccm->CCGR6);
	ccgr6 &= ~(0x3 << 10);
	writel(ccgr6, &imx_ccm->CCGR6);

	/*
	 * Configure clk_eim_slow_sel = 00 --> derive clock from AXI clk root
	 * and aclk_eim_slow_podf = 01 --> divide by 2
	 * so that we can have EIM at the maximum clock of 132MHz
	 */
	cscmr1 = readl(&imx_ccm->cscmr1);
	cscmr1 &= ~(MXC_CCM_CSCMR1_ACLK_EMI_SLOW_MASK |
		    MXC_CCM_CSCMR1_ACLK_EMI_SLOW_PODF_MASK);
	cscmr1 |= (1 << MXC_CCM_CSCMR1_ACLK_EMI_SLOW_PODF_OFFSET);
	writel(cscmr1, &imx_ccm->cscmr1);

	/* Turn on EIM clock */
	ccgr6 |= (0x3 << 10);
	writel(ccgr6, &imx_ccm->CCGR6);
}

static void setup_iomux_eimnor(void)
{
	SETUP_IOMUX_PADS(eimnor_pads);

	gpio_direction_output(IMX_GPIO_NR(5, 4), 0);

	eimnor_cs_setup();
}
#endif

static void setup_iomux_enet(void)
{
	SETUP_IOMUX_PADS(enet_pads);
}

static iomux_v3_cfg_t const usdhc3_pads[] = {
	IOMUX_PADS(PAD_SD3_CLK__SD3_CLK		| MUX_PAD_CTRL(USDHC_PAD_CTRL)),
	IOMUX_PADS(PAD_SD3_CMD__SD3_CMD		| MUX_PAD_CTRL(USDHC_PAD_CTRL)),
	IOMUX_PADS(PAD_SD3_DAT0__SD3_DATA0	| MUX_PAD_CTRL(USDHC_PAD_CTRL)),
	IOMUX_PADS(PAD_SD3_DAT1__SD3_DATA1	| MUX_PAD_CTRL(USDHC_PAD_CTRL)),
	IOMUX_PADS(PAD_SD3_DAT2__SD3_DATA2	| MUX_PAD_CTRL(USDHC_PAD_CTRL)),
	IOMUX_PADS(PAD_SD3_DAT3__SD3_DATA3	| MUX_PAD_CTRL(USDHC_PAD_CTRL)),
	IOMUX_PADS(PAD_SD3_DAT4__SD3_DATA4	| MUX_PAD_CTRL(USDHC_PAD_CTRL)),
	IOMUX_PADS(PAD_SD3_DAT5__SD3_DATA5	| MUX_PAD_CTRL(USDHC_PAD_CTRL)),
	IOMUX_PADS(PAD_SD3_DAT6__SD3_DATA6	| MUX_PAD_CTRL(USDHC_PAD_CTRL)),
	IOMUX_PADS(PAD_SD3_DAT7__SD3_DATA7	| MUX_PAD_CTRL(USDHC_PAD_CTRL)),
	IOMUX_PADS(PAD_GPIO_18__SD3_VSELECT	| MUX_PAD_CTRL(USDHC_PAD_CTRL)),
	IOMUX_PADS(PAD_NANDF_CS2__GPIO6_IO15	| MUX_PAD_CTRL(NO_PAD_CTRL)),
};

static void setup_iomux_uart(void)
{
	SETUP_IOMUX_PADS(uart4_pads);
}

#ifdef CONFIG_FSL_ESDHC
static struct fsl_esdhc_cfg usdhc_cfg[1] = {
	{USDHC3_BASE_ADDR},
};

int board_mmc_getcd(struct mmc *mmc)
{
	gpio_direction_input(IMX_GPIO_NR(6, 15));
	return !gpio_get_value(IMX_GPIO_NR(6, 15));
}

int board_mmc_init(bd_t *bis)
{
	SETUP_IOMUX_PADS(usdhc3_pads);

	usdhc_cfg[0].sdhc_clk = mxc_get_clock(MXC_ESDHC3_CLK);
	return fsl_esdhc_initialize(bis, &usdhc_cfg[0]);
}
#endif

#ifdef CONFIG_NAND_MXS
static iomux_v3_cfg_t gpmi_pads[] = {
	IOMUX_PADS(PAD_NANDF_CLE__NAND_CLE	| MUX_PAD_CTRL(GPMI_PAD_CTRL2)),
	IOMUX_PADS(PAD_NANDF_ALE__NAND_ALE	| MUX_PAD_CTRL(GPMI_PAD_CTRL2)),
	IOMUX_PADS(PAD_NANDF_WP_B__NAND_WP_B	| MUX_PAD_CTRL(GPMI_PAD_CTRL2)),
	IOMUX_PADS(PAD_NANDF_RB0__NAND_READY_B	| MUX_PAD_CTRL(GPMI_PAD_CTRL0)),
	IOMUX_PADS(PAD_NANDF_CS0__NAND_CE0_B	| MUX_PAD_CTRL(GPMI_PAD_CTRL2)),
	IOMUX_PADS(PAD_SD4_CMD__NAND_RE_B	| MUX_PAD_CTRL(GPMI_PAD_CTRL2)),
	IOMUX_PADS(PAD_SD4_CLK__NAND_WE_B	| MUX_PAD_CTRL(GPMI_PAD_CTRL2)),
	IOMUX_PADS(PAD_NANDF_D0__NAND_DATA00	| MUX_PAD_CTRL(GPMI_PAD_CTRL2)),
	IOMUX_PADS(PAD_NANDF_D1__NAND_DATA01	| MUX_PAD_CTRL(GPMI_PAD_CTRL2)),
	IOMUX_PADS(PAD_NANDF_D2__NAND_DATA02	| MUX_PAD_CTRL(GPMI_PAD_CTRL2)),
	IOMUX_PADS(PAD_NANDF_D3__NAND_DATA03	| MUX_PAD_CTRL(GPMI_PAD_CTRL2)),
	IOMUX_PADS(PAD_NANDF_D4__NAND_DATA04	| MUX_PAD_CTRL(GPMI_PAD_CTRL2)),
	IOMUX_PADS(PAD_NANDF_D5__NAND_DATA05	| MUX_PAD_CTRL(GPMI_PAD_CTRL2)),
	IOMUX_PADS(PAD_NANDF_D6__NAND_DATA06	| MUX_PAD_CTRL(GPMI_PAD_CTRL2)),
	IOMUX_PADS(PAD_NANDF_D7__NAND_DATA07	| MUX_PAD_CTRL(GPMI_PAD_CTRL2)),
	IOMUX_PADS(PAD_SD4_DAT0__NAND_DQS	| MUX_PAD_CTRL(GPMI_PAD_CTRL1)),
};

static void setup_gpmi_nand(void)
{
	struct mxc_ccm_reg *mxc_ccm = (struct mxc_ccm_reg *)CCM_BASE_ADDR;

	/* config gpmi nand iomux */
	SETUP_IOMUX_PADS(gpmi_pads);

	setup_gpmi_io_clk((MXC_CCM_CS2CDR_ENFC_CLK_PODF(0) |
			MXC_CCM_CS2CDR_ENFC_CLK_PRED(3) |
			MXC_CCM_CS2CDR_ENFC_CLK_SEL(3)));

	/* enable apbh clock gating */
	setbits_le32(&mxc_ccm->CCGR0, MXC_CCM_CCGR0_APBHDMA_MASK);
}
#endif

static void setup_fec(void)
{
	if (is_mx6dqp()) {
		/*
		 * select ENET MAC0 TX clock from PLL
		 */
		imx_iomux_set_gpr_register(5, 9, 1, 1);
		enable_fec_anatop_clock(0, ENET_125MHZ);
	}

	setup_iomux_enet();
}

int board_eth_init(bd_t *bis)
{
	setup_fec();

	return cpu_eth_init(bis);
}

u32 get_board_rev(void)
{
	int rev = nxp_board_rev();

	return (get_cpu_rev() & ~(0xF << 8)) | rev;
}

static int ar8031_phy_fixup(struct phy_device *phydev)
{
	unsigned short val;

	/* To enable AR8031 ouput a 125MHz clk from CLK_25M */
	phy_write(phydev, MDIO_DEVAD_NONE, 0xd, 0x7);
	phy_write(phydev, MDIO_DEVAD_NONE, 0xe, 0x8016);
	phy_write(phydev, MDIO_DEVAD_NONE, 0xd, 0x4007);

	val = phy_read(phydev, MDIO_DEVAD_NONE, 0xe);
	val &= 0xffe3;
	val |= 0x18;
	phy_write(phydev, MDIO_DEVAD_NONE, 0xe, val);

	/* introduce tx clock delay */
	phy_write(phydev, MDIO_DEVAD_NONE, 0x1d, 0x5);
	val = phy_read(phydev, MDIO_DEVAD_NONE, 0x1e);
	val |= 0x0100;
	phy_write(phydev, MDIO_DEVAD_NONE, 0x1e, val);

	return 0;
}

int board_phy_config(struct phy_device *phydev)
{
	ar8031_phy_fixup(phydev);

	if (phydev->drv->config)
		phydev->drv->config(phydev);

	return 0;
}

#if defined(CONFIG_VIDEO_IPUV3)
static void disable_lvds(struct display_info_t const *dev)
{
	struct iomuxc *iomux = (struct iomuxc *)IOMUXC_BASE_ADDR;

	clrbits_le32(&iomux->gpr[2],
		     IOMUXC_GPR2_LVDS_CH0_MODE_MASK |
		     IOMUXC_GPR2_LVDS_CH1_MODE_MASK);
}

static void do_enable_hdmi(struct display_info_t const *dev)
{
	disable_lvds(dev);
	imx_enable_hdmi_phy();
}

struct display_info_t const displays[] = {{
	.bus	= -1,
	.addr	= 0,
	.pixfmt	= IPU_PIX_FMT_RGB666,
	.detect	= NULL,
	.enable	= NULL,
	.mode	= {
		.name           = "Hannstar-XGA",
		.refresh        = 60,
		.xres           = 1024,
		.yres           = 768,
		.pixclock       = 15385,
		.left_margin    = 220,
		.right_margin   = 40,
		.upper_margin   = 21,
		.lower_margin   = 7,
		.hsync_len      = 60,
		.vsync_len      = 10,
		.sync           = FB_SYNC_EXT,
		.vmode          = FB_VMODE_NONINTERLACED
} }, {
	.bus	= -1,
	.addr	= 0,
	.pixfmt	= IPU_PIX_FMT_RGB24,
	.detect	= detect_hdmi,
	.enable	= do_enable_hdmi,
	.mode	= {
		.name           = "HDMI",
		.refresh        = 60,
		.xres           = 1024,
		.yres           = 768,
		.pixclock       = 15385,
		.left_margin    = 220,
		.right_margin   = 40,
		.upper_margin   = 21,
		.lower_margin   = 7,
		.hsync_len      = 60,
		.vsync_len      = 10,
		.sync           = FB_SYNC_EXT,
		.vmode          = FB_VMODE_NONINTERLACED,
} } };
size_t display_count = ARRAY_SIZE(displays);

iomux_v3_cfg_t const backlight_pads[] = {
	IOMUX_PADS(PAD_SD4_DAT1__GPIO2_IO09 | MUX_PAD_CTRL(ENET_PAD_CTRL)),
};

static void setup_iomux_backlight(void)
{
	gpio_request(IMX_GPIO_NR(2, 9), "backlight");
	gpio_direction_output(IMX_GPIO_NR(2, 9), 1);
	SETUP_IOMUX_PADS(backlight_pads);
}

static void setup_display(void)
{
	struct mxc_ccm_reg *mxc_ccm = (struct mxc_ccm_reg *)CCM_BASE_ADDR;
	struct iomuxc *iomux = (struct iomuxc *)IOMUXC_BASE_ADDR;
	int reg;

	setup_iomux_backlight();
	enable_ipu_clock();
	imx_setup_hdmi();

	/* Turn on LDB_DI0 and LDB_DI1 clocks */
	reg = readl(&mxc_ccm->CCGR3);
	reg |= MXC_CCM_CCGR3_LDB_DI0_MASK | MXC_CCM_CCGR3_LDB_DI1_MASK;
	writel(reg, &mxc_ccm->CCGR3);

	/* Set LDB_DI0 and LDB_DI1 clk select to 3b'011 */
	reg = readl(&mxc_ccm->cs2cdr);
	reg &= ~(MXC_CCM_CS2CDR_LDB_DI0_CLK_SEL_MASK |
		 MXC_CCM_CS2CDR_LDB_DI1_CLK_SEL_MASK);
	reg |= (3 << MXC_CCM_CS2CDR_LDB_DI0_CLK_SEL_OFFSET) |
	       (3 << MXC_CCM_CS2CDR_LDB_DI1_CLK_SEL_OFFSET);
	writel(reg, &mxc_ccm->cs2cdr);

	reg = readl(&mxc_ccm->cscmr2);
	reg |= MXC_CCM_CSCMR2_LDB_DI0_IPU_DIV | MXC_CCM_CSCMR2_LDB_DI1_IPU_DIV;
	writel(reg, &mxc_ccm->cscmr2);

	reg = readl(&mxc_ccm->chsccdr);
	reg |= (CHSCCDR_CLK_SEL_LDB_DI0
		<< MXC_CCM_CHSCCDR_IPU1_DI0_CLK_SEL_OFFSET);
	reg |= (CHSCCDR_CLK_SEL_LDB_DI0 <<
		MXC_CCM_CHSCCDR_IPU1_DI1_CLK_SEL_OFFSET);
	writel(reg, &mxc_ccm->chsccdr);

	reg = IOMUXC_GPR2_DI1_VS_POLARITY_ACTIVE_LOW |
	      IOMUXC_GPR2_DI0_VS_POLARITY_ACTIVE_LOW |
	      IOMUXC_GPR2_BIT_MAPPING_CH1_SPWG |
	      IOMUXC_GPR2_DATA_WIDTH_CH1_18BIT |
	      IOMUXC_GPR2_BIT_MAPPING_CH0_SPWG |
	      IOMUXC_GPR2_DATA_WIDTH_CH0_18BIT |
	      IOMUXC_GPR2_LVDS_CH0_MODE_ENABLED_DI0 |
	      IOMUXC_GPR2_LVDS_CH1_MODE_DISABLED;
	writel(reg, &iomux->gpr[2]);

	reg = readl(&iomux->gpr[3]);
	reg &= ~(IOMUXC_GPR3_LVDS0_MUX_CTL_MASK |
		 IOMUXC_GPR3_HDMI_MUX_CTL_MASK);
	reg |= (IOMUXC_GPR3_MUX_SRC_IPU1_DI0 <<
		IOMUXC_GPR3_LVDS0_MUX_CTL_OFFSET) |
	       (IOMUXC_GPR3_MUX_SRC_IPU1_DI0 <<
		IOMUXC_GPR3_HDMI_MUX_CTL_OFFSET);
	writel(reg, &iomux->gpr[3]);
}
#endif /* CONFIG_VIDEO_IPUV3 */

/*
 * Do not overwrite the console
 * Use always serial for U-Boot console
 */
int overwrite_console(void)
{
	return 1;
}

int board_early_init_f(void)
{
	setup_iomux_uart();

#ifdef CONFIG_NAND_MXS
	setup_gpmi_nand();
#endif

#ifdef CONFIG_MTD_NOR_FLASH
	eim_clk_setup();
#endif
	return 0;
}

int board_init(void)
{
	/* address of boot parameters */
	gd->bd->bi_boot_params = PHYS_SDRAM + 0x100;

	/* I2C 2 and 3 setup - I2C 3 hw mux with EIM */
	if (is_mx6dq() || is_mx6dqp())
		setup_i2c(1, CONFIG_SYS_I2C_SPEED, 0x7f, &mx6q_i2c_pad_info1);
	else
		setup_i2c(1, CONFIG_SYS_I2C_SPEED, 0x7f, &mx6dl_i2c_pad_info1);
	/* I2C 3 Steer */
	gpio_request(IMX_GPIO_NR(5, 4), "steer logic");
	gpio_direction_output(IMX_GPIO_NR(5, 4), 1);
	SETUP_IOMUX_PADS(i2c3_pads);
#ifndef CONFIG_SYS_FLASH_CFI
	if (is_mx6dq() || is_mx6dqp())
		setup_i2c(2, CONFIG_SYS_I2C_SPEED, 0x7f, &mx6q_i2c_pad_info2);
	else
		setup_i2c(2, CONFIG_SYS_I2C_SPEED, 0x7f, &mx6dl_i2c_pad_info2);
#endif
	gpio_request(IMX_GPIO_NR(1, 15), "expander en");
	gpio_direction_output(IMX_GPIO_NR(1, 15), 1);
	SETUP_IOMUX_PADS(port_exp);

#ifdef CONFIG_VIDEO_IPUV3
	setup_display();
#endif

#ifdef CONFIG_MTD_NOR_FLASH
	setup_iomux_eimnor();
#endif
	return 0;
}

#ifdef CONFIG_MXC_SPI
int board_spi_cs_gpio(unsigned bus, unsigned cs)
{
	return (bus == 0 && cs == 0) ? (IMX_GPIO_NR(4, 9)) : -1;
}
#endif

int power_init_board(void)
{
	struct pmic *p;
	unsigned int value;

	p = pfuze_common_init(I2C_PMIC);
	if (!p)
		return -ENODEV;

	if (is_mx6dqp()) {
		/* set SW2 staby volatage 0.975V*/
		pmic_reg_read(p, PFUZE100_SW2STBY, &value);
		value &= ~0x3f;
		value |= 0x17;
		pmic_reg_write(p, PFUZE100_SW2STBY, value);
	}

	return pfuze_mode_init(p, APS_PFM);
}

#ifdef CONFIG_CMD_BMODE
static const struct boot_mode board_boot_modes[] = {
	/* 4 bit bus width */
	{"mmc0", MAKE_CFGVAL(0x40, 0x30, 0x00, 0x00)},
	{NULL,   0},
};
#endif

int board_late_init(void)
{
#ifdef CONFIG_CMD_BMODE
	add_board_boot_modes(board_boot_modes);
#endif

#ifdef CONFIG_ENV_VARS_UBOOT_RUNTIME_CONFIG
	env_set("board_name", "SABREAUTO");

	if (is_mx6dqp())
		env_set("board_rev", "MX6QP");
	else if (is_mx6dq())
		env_set("board_rev", "MX6Q");
	else if (is_mx6sdl())
		env_set("board_rev", "MX6DL");
#endif

	return 0;
}

int checkboard(void)
{
	printf("Board: MX6Q-Sabreauto rev%c\n", nxp_board_rev_string());

	return 0;
}

#ifdef CONFIG_USB_EHCI_MX6
int board_ehci_hcd_init(int port)
{
	switch (port) {
	case 0:
		/*
		  * Set daisy chain for otg_pin_id on 6q.
		 *  For 6dl, this bit is reserved.
		 */
		imx_iomux_set_gpr_register(1, 13, 1, 0);
		break;
	case 1:
		break;
	default:
		printf("MXC USB port %d not yet supported\n", port);
		return -EINVAL;
	}
	return 0;
}
#endif

#ifdef CONFIG_SPL_BUILD
#include <asm/arch/mx6-ddr.h>
#include <spl.h>
#include <linux/libfdt.h>

#ifdef CONFIG_SPL_OS_BOOT
int spl_start_uboot(void)
{
	return 0;
}
#endif

static void ccgr_init(void)
{
	struct mxc_ccm_reg *ccm = (struct mxc_ccm_reg *)CCM_BASE_ADDR;

	writel(0x00C03F3F, &ccm->CCGR0);
	writel(0x0030FC03, &ccm->CCGR1);
	writel(0x0FFFC000, &ccm->CCGR2);
	writel(0x3FF00000, &ccm->CCGR3);
	writel(0x00FFF300, &ccm->CCGR4);
	writel(0x0F0000C3, &ccm->CCGR5);
	writel(0x000003FF, &ccm->CCGR6);
}

static int mx6q_dcd_table[] = {
	0x020e0798, 0x000C0000,
	0x020e0758, 0x00000000,
	0x020e0588, 0x00000030,
	0x020e0594, 0x00000030,
	0x020e056c, 0x00000030,
	0x020e0578, 0x00000030,
	0x020e074c, 0x00000030,
	0x020e057c, 0x00000030,
	0x020e058c, 0x00000000,
	0x020e059c, 0x00000030,
	0x020e05a0, 0x00000030,
	0x020e078c, 0x00000030,
	0x020e0750, 0x00020000,
	0x020e05a8, 0x00000028,
	0x020e05b0, 0x00000028,
	0x020e0524, 0x00000028,
	0x020e051c, 0x00000028,
	0x020e0518, 0x00000028,
	0x020e050c, 0x00000028,
	0x020e05b8, 0x00000028,
	0x020e05c0, 0x00000028,
	0x020e0774, 0x00020000,
	0x020e0784, 0x00000028,
	0x020e0788, 0x00000028,
	0x020e0794, 0x00000028,
	0x020e079c, 0x00000028,
	0x020e07a0, 0x00000028,
	0x020e07a4, 0x00000028,
	0x020e07a8, 0x00000028,
	0x020e0748, 0x00000028,
	0x020e05ac, 0x00000028,
	0x020e05b4, 0x00000028,
	0x020e0528, 0x00000028,
	0x020e0520, 0x00000028,
	0x020e0514, 0x00000028,
	0x020e0510, 0x00000028,
	0x020e05bc, 0x00000028,
	0x020e05c4, 0x00000028,
	0x021b0800, 0xa1390003,
	0x021b080c, 0x001F001F,
	0x021b0810, 0x001F001F,
	0x021b480c, 0x001F001F,
	0x021b4810, 0x001F001F,
	0x021b083c, 0x43260335,
	0x021b0840, 0x031A030B,
	0x021b483c, 0x4323033B,
	0x021b4840, 0x0323026F,
	0x021b0848, 0x483D4545,
	0x021b4848, 0x44433E48,
	0x021b0850, 0x41444840,
	0x021b4850, 0x4835483E,
	0x021b081c, 0x33333333,
	0x021b0820, 0x33333333,
	0x021b0824, 0x33333333,
	0x021b0828, 0x33333333,
	0x021b481c, 0x33333333,
	0x021b4820, 0x33333333,
	0x021b4824, 0x33333333,
	0x021b4828, 0x33333333,
	0x021b08b8, 0x00000800,
	0x021b48b8, 0x00000800,
	0x021b0004, 0x00020036,
	0x021b0008, 0x09444040,
	0x021b000c, 0x8A8F7955,
	0x021b0010, 0xFF328F64,
	0x021b0014, 0x01FF00DB,
	0x021b0018, 0x00001740,
	0x021b001c, 0x00008000,
	0x021b002c, 0x000026d2,
	0x021b0030, 0x008F1023,
	0x021b0040, 0x00000047,
	0x021b0000, 0x841A0000,
	0x021b001c, 0x04088032,
	0x021b001c, 0x00008033,
	0x021b001c, 0x00048031,
	0x021b001c, 0x09408030,
	0x021b001c, 0x04008040,
	0x021b0020, 0x00005800,
	0x021b0818, 0x00011117,
	0x021b4818, 0x00011117,
	0x021b0004, 0x00025576,
	0x021b0404, 0x00011006,
	0x021b001c, 0x00000000,
	0x020c4068, 0x00C03F3F,
	0x020c406c, 0x0030FC03,
	0x020c4070, 0x0FFFC000,
	0x020c4074, 0x3FF00000,
	0x020c4078, 0xFFFFF300,
	0x020c407c, 0x0F0000F3,
	0x020c4080, 0x00000FFF,
	0x020e0010, 0xF00000CF,
	0x020e0018, 0x007F007F,
	0x020e001c, 0x007F007F,
};

static int mx6qp_dcd_table[] = {
	0x020e0798, 0x000C0000,
	0x020e0758, 0x00000000,
	0x020e0588, 0x00000030,
	0x020e0594, 0x00000030,
	0x020e056c, 0x00000030,
	0x020e0578, 0x00000030,
	0x020e074c, 0x00000030,
	0x020e057c, 0x00000030,
	0x020e058c, 0x00000000,
	0x020e059c, 0x00000030,
	0x020e05a0, 0x00000030,
	0x020e078c, 0x00000030,
	0x020e0750, 0x00020000,
	0x020e05a8, 0x00000030,
	0x020e05b0, 0x00000030,
	0x020e0524, 0x00000030,
	0x020e051c, 0x00000030,
	0x020e0518, 0x00000030,
	0x020e050c, 0x00000030,
	0x020e05b8, 0x00000030,
	0x020e05c0, 0x00000030,
	0x020e0774, 0x00020000,
	0x020e0784, 0x00000030,
	0x020e0788, 0x00000030,
	0x020e0794, 0x00000030,
	0x020e079c, 0x00000030,
	0x020e07a0, 0x00000030,
	0x020e07a4, 0x00000030,
	0x020e07a8, 0x00000030,
	0x020e0748, 0x00000030,
	0x020e05ac, 0x00000030,
	0x020e05b4, 0x00000030,
	0x020e0528, 0x00000030,
	0x020e0520, 0x00000030,
	0x020e0514, 0x00000030,
	0x020e0510, 0x00000030,
	0x020e05bc, 0x00000030,
	0x020e05c4, 0x00000030,
	0x021b0800, 0xa1390003,
	0x021b080c, 0x001b001e,
	0x021b0810, 0x002e0029,
	0x021b480c, 0x001b002a,
	0x021b4810, 0x0019002c,
	0x021b083c, 0x43240334,
	0x021b0840, 0x0324031a,
	0x021b483c, 0x43340344,
	0x021b4840, 0x03280276,
	0x021b0848, 0x44383A3E,
	0x021b4848, 0x3C3C3846,
	0x021b0850, 0x2e303230,
	0x021b4850, 0x38283E34,
	0x021b081c, 0x33333333,
	0x021b0820, 0x33333333,
	0x021b0824, 0x33333333,
	0x021b0828, 0x33333333,
	0x021b481c, 0x33333333,
	0x021b4820, 0x33333333,
	0x021b4824, 0x33333333,
	0x021b4828, 0x33333333,
	0x021b08c0, 0x24912492,
	0x021b48c0, 0x24912492,
	0x021b08b8, 0x00000800,
	0x021b48b8, 0x00000800,
	0x021b0004, 0x00020036,
	0x021b0008, 0x09444040,
	0x021b000c, 0x898E7955,
	0x021b0010, 0xFF328F64,
	0x021b0014, 0x01FF00DB,
	0x021b0018, 0x00001740,
	0x021b001c, 0x00008000,
	0x021b002c, 0x000026d2,
	0x021b0030, 0x008E1023,
	0x021b0040, 0x00000047,
	0x021b0400, 0x14420000,
	0x021b0000, 0x841A0000,
	0x00bb0008, 0x00000004,
	0x00bb000c, 0x2891E41A,
	0x00bb0038, 0x00000564,
	0x00bb0014, 0x00000040,
	0x00bb0028, 0x00000020,
	0x00bb002c, 0x00000020,
	0x021b001c, 0x04088032,
	0x021b001c, 0x00008033,
	0x021b001c, 0x00048031,
	0x021b001c, 0x09408030,
	0x021b001c, 0x04008040,
	0x021b0020, 0x00005800,
	0x021b0818, 0x00011117,
	0x021b4818, 0x00011117,
	0x021b0004, 0x00025576,
	0x021b0404, 0x00011006,
	0x021b001c, 0x00000000,
	0x020c4068, 0x00C03F3F,
	0x020c406c, 0x0030FC03,
	0x020c4070, 0x0FFFC000,
	0x020c4074, 0x3FF00000,
	0x020c4078, 0xFFFFF300,
	0x020c407c, 0x0F0000F3,
	0x020c4080, 0x00000FFF,
	0x020e0010, 0xF00000CF,
	0x020e0018, 0x77177717,
	0x020e001c, 0x77177717,
};

static int mx6dl_dcd_table[] = {
	0x020e0774, 0x000C0000,
	0x020e0754, 0x00000000,
	0x020e04ac, 0x00000030,
	0x020e04b0, 0x00000030,
	0x020e0464, 0x00000030,
	0x020e0490, 0x00000030,
	0x020e074c, 0x00000030,
	0x020e0494, 0x00000030,
	0x020e04a0, 0x00000000,
	0x020e04b4, 0x00000030,
	0x020e04b8, 0x00000030,
	0x020e076c, 0x00000030,
	0x020e0750, 0x00020000,
	0x020e04bc, 0x00000028,
	0x020e04c0, 0x00000028,
	0x020e04c4, 0x00000028,
	0x020e04c8, 0x00000028,
	0x020e04cc, 0x00000028,
	0x020e04d0, 0x00000028,
	0x020e04d4, 0x00000028,
	0x020e04d8, 0x00000028,
	0x020e0760, 0x00020000,
	0x020e0764, 0x00000028,
	0x020e0770, 0x00000028,
	0x020e0778, 0x00000028,
	0x020e077c, 0x00000028,
	0x020e0780, 0x00000028,
	0x020e0784, 0x00000028,
	0x020e078c, 0x00000028,
	0x020e0748, 0x00000028,
	0x020e0470, 0x00000028,
	0x020e0474, 0x00000028,
	0x020e0478, 0x00000028,
	0x020e047c, 0x00000028,
	0x020e0480, 0x00000028,
	0x020e0484, 0x00000028,
	0x020e0488, 0x00000028,
	0x020e048c, 0x00000028,
	0x021b0800, 0xa1390003,
	0x021b080c, 0x001F001F,
	0x021b0810, 0x001F001F,
	0x021b480c, 0x001F001F,
	0x021b4810, 0x001F001F,
	0x021b083c, 0x42190217,
	0x021b0840, 0x017B017B,
	0x021b483c, 0x4176017B,
	0x021b4840, 0x015F016C,
	0x021b0848, 0x4C4C4D4C,
	0x021b4848, 0x4A4D4C48,
	0x021b0850, 0x3F3F3F40,
	0x021b4850, 0x3538382E,
	0x021b081c, 0x33333333,
	0x021b0820, 0x33333333,
	0x021b0824, 0x33333333,
	0x021b0828, 0x33333333,
	0x021b481c, 0x33333333,
	0x021b4820, 0x33333333,
	0x021b4824, 0x33333333,
	0x021b4828, 0x33333333,
	0x021b08b8, 0x00000800,
	0x021b48b8, 0x00000800,
	0x021b0004, 0x00020025,
	0x021b0008, 0x00333030,
	0x021b000c, 0x676B5313,
	0x021b0010, 0xB66E8B63,
	0x021b0014, 0x01FF00DB,
	0x021b0018, 0x00001740,
	0x021b001c, 0x00008000,
	0x021b002c, 0x000026d2,
	0x021b0030, 0x006B1023,
	0x021b0040, 0x00000047,
	0x021b0000, 0x841A0000,
	0x021b001c, 0x04008032,
	0x021b001c, 0x00008033,
	0x021b001c, 0x00048031,
	0x021b001c, 0x05208030,
	0x021b001c, 0x04008040,
	0x021b0020, 0x00005800,
	0x021b0818, 0x00011117,
	0x021b4818, 0x00011117,
	0x021b0004, 0x00025565,
	0x021b0404, 0x00011006,
	0x021b001c, 0x00000000,
	0x020c4068, 0x00C03F3F,
	0x020c406c, 0x0030FC03,
	0x020c4070, 0x0FFFC000,
	0x020c4074, 0x3FF00000,
	0x020c4078, 0xFFFFF300,
	0x020c407c, 0x0F0000C3,
	0x020c4080, 0x00000FFF,
	0x020e0010, 0xF00000CF,
	0x020e0018, 0x007F007F,
	0x020e001c, 0x007F007F,
};

static void ddr_init(int *table, int size)
{
	int i;

	for (i = 0; i < size / 2 ; i++)
		writel(table[2 * i + 1], table[2 * i]);
}

static void spl_dram_init(void)
{
	if (is_mx6dq())
		ddr_init(mx6q_dcd_table, ARRAY_SIZE(mx6q_dcd_table));
	else if (is_mx6dqp())
		ddr_init(mx6qp_dcd_table, ARRAY_SIZE(mx6qp_dcd_table));
	else if (is_mx6sdl())
		ddr_init(mx6dl_dcd_table, ARRAY_SIZE(mx6dl_dcd_table));
}

void board_init_f(ulong dummy)
{
	/* DDR initialization */
	spl_dram_init();

	/* setup AIPS and disable watchdog */
	arch_cpu_init();

	ccgr_init();
	gpr_init();

	/* iomux and setup of i2c */
	board_early_init_f();

	/* setup GP timer */
	timer_init();

	/* UART clocks enabled and gd valid - init serial console */
	preloader_console_init();

	/* Clear the BSS. */
	memset(__bss_start, 0, __bss_end - __bss_start);

	/* load/boot image from boot device */
	board_init_r(NULL, 0);
}
#endif

#ifdef CONFIG_SPL_LOAD_FIT
int board_fit_config_name_match(const char *name)
{
	if (is_mx6dq()) {
		if (!strcmp(name, "imx6q-sabreauto"))
			return 0;
	} else if (is_mx6dqp()) {
		if (!strcmp(name, "imx6qp-sabreauto"))
			return 0;
	} else if (is_mx6dl()) {
		if (!strcmp(name, "imx6dl-sabreauto"))
			return 0;
	}

	return -1;
}
#endif

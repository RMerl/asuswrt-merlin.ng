// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright (C) 2016 Stefano Babic <sbabic@denx.de>
 */

/*
 * Please note: there are two version of the board
 * one with NAND and the other with eMMC.
 * Both NAND and eMMC cannot be set because they share the
 * same pins (SD4)
 */
#include <common.h>
#include <asm/io.h>
#include <asm/arch/clock.h>
#include <asm/arch/imx-regs.h>
#include <asm/arch/crm_regs.h>
#include <asm/arch/mx6-ddr.h>
#include <asm/arch/iomux.h>
#include <asm/arch/mx6-pins.h>
#include <asm/mach-imx/iomux-v3.h>
#include <asm/mach-imx/boot_mode.h>
#include <asm/mach-imx/mxc_i2c.h>
#include <asm/mach-imx/spi.h>
#include <linux/errno.h>
#include <asm/gpio.h>
#include <mmc.h>
#include <i2c.h>
#include <fsl_esdhc.h>
#include <nand.h>
#include <miiphy.h>
#include <netdev.h>
#include <asm/arch/sys_proto.h>
#include <asm/sections.h>

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

#define I2C_PAD_CTRL  (PAD_CTL_PUS_100K_UP |			\
	PAD_CTL_SPEED_MED | PAD_CTL_DSE_40ohm | PAD_CTL_HYS |	\
	PAD_CTL_ODE | PAD_CTL_SRE_FAST)

#define I2C_PAD MUX_PAD_CTRL(I2C_PAD_CTRL)

#define ASRC_PAD_CTRL (PAD_CTL_HYS | PAD_CTL_PUS_100K_UP  |	\
		      PAD_CTL_DSE_40ohm | PAD_CTL_SRE_FAST)

#define NAND_PAD_CTRL (PAD_CTL_PUS_100K_UP | PAD_CTL_SPEED_MED | \
	       PAD_CTL_DSE_40ohm | PAD_CTL_SRE_FAST)

#define ENET_PHY_RESET_GPIO IMX_GPIO_NR(1, 14)
#define USDHC1_CD_GPIO	IMX_GPIO_NR(6, 31)
#define USER_LED	IMX_GPIO_NR(1, 4)
#define IMX6Q_DRIVE_STRENGTH	0x30

int dram_init(void)
{
	gd->ram_size = imx_ddr_size();
	return 0;
}

void board_turn_off_led(void)
{
	gpio_direction_output(USER_LED, 0);
}

static iomux_v3_cfg_t const uart1_pads[] = {
	MX6_PAD_EIM_D26__UART2_TX_DATA | MUX_PAD_CTRL(UART_PAD_CTRL),
	MX6_PAD_EIM_D27__UART2_RX_DATA | MUX_PAD_CTRL(UART_PAD_CTRL),
};

static iomux_v3_cfg_t const enet_pads[] = {
	MX6_PAD_ENET_MDIO__ENET_MDIO		| MUX_PAD_CTRL(ENET_PAD_CTRL),
	MX6_PAD_ENET_MDC__ENET_MDC		| MUX_PAD_CTRL(ENET_PAD_CTRL),
	MX6_PAD_RGMII_TXC__RGMII_TXC	| MUX_PAD_CTRL(ENET_PAD_CTRL),
	MX6_PAD_RGMII_TD0__RGMII_TD0	| MUX_PAD_CTRL(ENET_PAD_CTRL),
	MX6_PAD_RGMII_TD1__RGMII_TD1	| MUX_PAD_CTRL(ENET_PAD_CTRL),
	MX6_PAD_RGMII_TD2__RGMII_TD2	| MUX_PAD_CTRL(ENET_PAD_CTRL),
	MX6_PAD_RGMII_TD3__RGMII_TD3	| MUX_PAD_CTRL(ENET_PAD_CTRL),
	MX6_PAD_RGMII_TX_CTL__RGMII_TX_CTL	| MUX_PAD_CTRL(ENET_PAD_CTRL),
	MX6_PAD_ENET_REF_CLK__ENET_TX_CLK	| MUX_PAD_CTRL(ENET_PAD_CTRL),
	MX6_PAD_RGMII_RXC__RGMII_RXC	| MUX_PAD_CTRL(ENET_PAD_CTRL),
	MX6_PAD_RGMII_RD0__RGMII_RD0	| MUX_PAD_CTRL(ENET_PAD_CTRL),
	MX6_PAD_RGMII_RD1__RGMII_RD1	| MUX_PAD_CTRL(ENET_PAD_CTRL),
	MX6_PAD_RGMII_RD2__RGMII_RD2	| MUX_PAD_CTRL(ENET_PAD_CTRL),
	MX6_PAD_RGMII_RD3__RGMII_RD3	| MUX_PAD_CTRL(ENET_PAD_CTRL),
	MX6_PAD_RGMII_RX_CTL__RGMII_RX_CTL	| MUX_PAD_CTRL(ENET_PAD_CTRL),
	MX6_PAD_SD2_DAT1__GPIO1_IO14	| MUX_PAD_CTRL(NO_PAD_CTRL),
};

static iomux_v3_cfg_t const ecspi1_pads[] = {
	MX6_PAD_EIM_D16__ECSPI1_SCLK | MUX_PAD_CTRL(SPI_PAD_CTRL),
	MX6_PAD_EIM_D17__ECSPI1_MISO | MUX_PAD_CTRL(SPI_PAD_CTRL),
	MX6_PAD_EIM_D18__ECSPI1_MOSI | MUX_PAD_CTRL(SPI_PAD_CTRL),
	MX6_PAD_EIM_D19__GPIO3_IO19 | MUX_PAD_CTRL(NO_PAD_CTRL),
};

#ifdef CONFIG_CMD_NAND
/* NAND */
static iomux_v3_cfg_t const nfc_pads[] = {
	MX6_PAD_NANDF_CLE__NAND_CLE     | MUX_PAD_CTRL(NAND_PAD_CTRL),
	MX6_PAD_NANDF_ALE__NAND_ALE     | MUX_PAD_CTRL(NAND_PAD_CTRL),
	MX6_PAD_NANDF_WP_B__NAND_WP_B   | MUX_PAD_CTRL(NAND_PAD_CTRL),
	MX6_PAD_NANDF_RB0__NAND_READY_B | MUX_PAD_CTRL(NAND_PAD_CTRL),
	MX6_PAD_NANDF_CS0__NAND_CE0_B   | MUX_PAD_CTRL(NAND_PAD_CTRL),
	MX6_PAD_NANDF_CS1__NAND_CE1_B	| MUX_PAD_CTRL(NAND_PAD_CTRL),
	MX6_PAD_NANDF_CS2__NAND_CE2_B	| MUX_PAD_CTRL(NAND_PAD_CTRL),
	MX6_PAD_NANDF_CS3__NAND_CE3_B	| MUX_PAD_CTRL(NAND_PAD_CTRL),
	MX6_PAD_SD4_CMD__NAND_RE_B      | MUX_PAD_CTRL(NAND_PAD_CTRL),
	MX6_PAD_SD4_CLK__NAND_WE_B      | MUX_PAD_CTRL(NAND_PAD_CTRL),
	MX6_PAD_NANDF_D0__NAND_DATA00   | MUX_PAD_CTRL(NAND_PAD_CTRL),
	MX6_PAD_NANDF_D1__NAND_DATA01   | MUX_PAD_CTRL(NAND_PAD_CTRL),
	MX6_PAD_NANDF_D2__NAND_DATA02   | MUX_PAD_CTRL(NAND_PAD_CTRL),
	MX6_PAD_NANDF_D3__NAND_DATA03   | MUX_PAD_CTRL(NAND_PAD_CTRL),
	MX6_PAD_NANDF_D4__NAND_DATA04   | MUX_PAD_CTRL(NAND_PAD_CTRL),
	MX6_PAD_NANDF_D5__NAND_DATA05   | MUX_PAD_CTRL(NAND_PAD_CTRL),
	MX6_PAD_NANDF_D6__NAND_DATA06   | MUX_PAD_CTRL(NAND_PAD_CTRL),
	MX6_PAD_NANDF_D7__NAND_DATA07   | MUX_PAD_CTRL(NAND_PAD_CTRL),
	MX6_PAD_SD4_DAT0__NAND_DQS	| MUX_PAD_CTRL(NAND_PAD_CTRL),
};
#endif

static struct i2c_pads_info i2c_pad_info2 = {
	.scl = {
		.i2c_mode = MX6_PAD_GPIO_5__I2C3_SCL | I2C_PAD,
		.gpio_mode = MX6_PAD_GPIO_5__GPIO1_IO05 | I2C_PAD,
		.gp = IMX_GPIO_NR(1, 5)
	},
	.sda = {
		.i2c_mode = MX6_PAD_GPIO_6__I2C3_SDA | I2C_PAD,
		.gpio_mode = MX6_PAD_GPIO_6__GPIO1_IO06 | I2C_PAD,
		.gp = IMX_GPIO_NR(1, 6)
	}
};

static struct fsl_esdhc_cfg usdhc_cfg[] = {
	{.esdhc_base = USDHC1_BASE_ADDR,
	.max_bus_width = 4},
#ifndef CONFIG_CMD_NAND
	{USDHC4_BASE_ADDR},
#endif
};

static iomux_v3_cfg_t const usdhc1_pads[] = {
	MX6_PAD_SD1_CLK__SD1_CLK	| MUX_PAD_CTRL(USDHC_PAD_CTRL),
	MX6_PAD_SD1_CMD__SD1_CMD	| MUX_PAD_CTRL(USDHC_PAD_CTRL),
	MX6_PAD_SD1_DAT0__SD1_DATA0	| MUX_PAD_CTRL(USDHC_PAD_CTRL),
	MX6_PAD_SD1_DAT1__SD1_DATA1	| MUX_PAD_CTRL(USDHC_PAD_CTRL),
	MX6_PAD_SD1_DAT2__SD1_DATA2	| MUX_PAD_CTRL(USDHC_PAD_CTRL),
	MX6_PAD_SD1_DAT3__SD1_DATA3	| MUX_PAD_CTRL(USDHC_PAD_CTRL),
	MX6_PAD_EIM_BCLK__GPIO6_IO31	| MUX_PAD_CTRL(NO_PAD_CTRL), /* CD */
};

#if !defined(CONFIG_CMD_NAND) && !defined(CONFIG_SPL_BUILD)
static iomux_v3_cfg_t const usdhc4_pads[] = {
	MX6_PAD_SD4_CLK__SD4_CLK	| MUX_PAD_CTRL(USDHC_PAD_CTRL),
	MX6_PAD_SD4_CMD__SD4_CMD	| MUX_PAD_CTRL(USDHC_PAD_CTRL),
	MX6_PAD_SD4_DAT0__SD4_DATA0	| MUX_PAD_CTRL(USDHC_PAD_CTRL),
	MX6_PAD_SD4_DAT1__SD4_DATA1	| MUX_PAD_CTRL(USDHC_PAD_CTRL),
	MX6_PAD_SD4_DAT2__SD4_DATA2	| MUX_PAD_CTRL(USDHC_PAD_CTRL),
	MX6_PAD_SD4_DAT3__SD4_DATA3	| MUX_PAD_CTRL(USDHC_PAD_CTRL),
	MX6_PAD_SD4_DAT4__SD4_DATA4	| MUX_PAD_CTRL(USDHC_PAD_CTRL),
	MX6_PAD_SD4_DAT5__SD4_DATA5	| MUX_PAD_CTRL(USDHC_PAD_CTRL),
	MX6_PAD_SD4_DAT6__SD4_DATA6	| MUX_PAD_CTRL(USDHC_PAD_CTRL),
	MX6_PAD_SD4_DAT7__SD4_DATA7	| MUX_PAD_CTRL(USDHC_PAD_CTRL),
};
#endif

int board_mmc_get_env_dev(int devno)
{
	return devno - 1;
}

int board_mmc_getcd(struct mmc *mmc)
{
	struct fsl_esdhc_cfg *cfg = (struct fsl_esdhc_cfg *)mmc->priv;
	int ret = 0;

	switch (cfg->esdhc_base) {
	case USDHC1_BASE_ADDR:
		ret = !gpio_get_value(USDHC1_CD_GPIO);
		break;
	case USDHC4_BASE_ADDR:
		ret = 1; /* eMMC/uSDHC4 is always present */
		break;
	}

	return ret;
}

int board_mmc_init(bd_t *bis)
{
#ifndef CONFIG_SPL_BUILD
	int ret;
	int i;

	for (i = 0; i < CONFIG_SYS_FSL_USDHC_NUM; i++) {
		switch (i) {
		case 0:
			imx_iomux_v3_setup_multiple_pads(
				usdhc1_pads, ARRAY_SIZE(usdhc1_pads));
			gpio_direction_input(USDHC1_CD_GPIO);
			usdhc_cfg[0].sdhc_clk = mxc_get_clock(MXC_ESDHC_CLK);
			break;
#ifndef CONFIG_CMD_NAND
		case 1:
			imx_iomux_v3_setup_multiple_pads(
				usdhc4_pads, ARRAY_SIZE(usdhc4_pads));
			usdhc_cfg[1].sdhc_clk = mxc_get_clock(MXC_ESDHC4_CLK);
			break;
#endif
		default:
			printf("Warning: you configured more USDHC controllers"
			       "(%d) then supported by the board (%d)\n",
			       i + 1, CONFIG_SYS_FSL_USDHC_NUM);
			return -EINVAL;
		}

		ret = fsl_esdhc_initialize(bis, &usdhc_cfg[i]);
		if (ret)
			return ret;
	}

	return 0;
#else
	struct src *psrc = (struct src *)SRC_BASE_ADDR;
	unsigned reg = readl(&psrc->sbmr1) >> 11;
	/*
	 * Upon reading BOOT_CFG register the following map is done:
	 * Bit 11 and 12 of BOOT_CFG register can determine the current
	 * mmc port
	 * 0x1                  SD1
	 * 0x2                  SD2
	 * 0x3                  SD4
	 */

	switch (reg & 0x3) {
	case 0x0:
		imx_iomux_v3_setup_multiple_pads(
			usdhc1_pads, ARRAY_SIZE(usdhc1_pads));
		gpio_direction_input(USDHC1_CD_GPIO);
		usdhc_cfg[0].esdhc_base = USDHC1_BASE_ADDR;
		usdhc_cfg[0].sdhc_clk = mxc_get_clock(MXC_ESDHC_CLK);
		usdhc_cfg[0].max_bus_width = 4;
		gd->arch.sdhc_clk = usdhc_cfg[0].sdhc_clk;
		break;
	}
	return fsl_esdhc_initialize(bis, &usdhc_cfg[0]);
#endif
}

static void setup_iomux_uart(void)
{
	imx_iomux_v3_setup_multiple_pads(uart1_pads, ARRAY_SIZE(uart1_pads));
}

static void setup_iomux_enet(void)
{
	imx_iomux_v3_setup_multiple_pads(enet_pads, ARRAY_SIZE(enet_pads));

	gpio_direction_output(ENET_PHY_RESET_GPIO, 0);
	mdelay(10);
	gpio_set_value(ENET_PHY_RESET_GPIO, 1);
	mdelay(30);
}

static void setup_spi(void)
{
	gpio_request(IMX_GPIO_NR(3, 19), "spi_cs0");
	gpio_direction_output(IMX_GPIO_NR(3, 19), 1);

	imx_iomux_v3_setup_multiple_pads(ecspi1_pads, ARRAY_SIZE(ecspi1_pads));

	enable_spi_clk(true, 0);
}

#ifdef CONFIG_CMD_NAND
static void setup_gpmi_nand(void)
{
	struct mxc_ccm_reg *mxc_ccm = (struct mxc_ccm_reg *)CCM_BASE_ADDR;

	/* config gpmi nand iomux */
	imx_iomux_v3_setup_multiple_pads(nfc_pads, ARRAY_SIZE(nfc_pads));

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
#endif

int board_spi_cs_gpio(unsigned bus, unsigned cs)
{
	if (bus != 0 || (cs != 0))
		return -EINVAL;

	return IMX_GPIO_NR(3, 19);
}

int board_eth_init(bd_t *bis)
{
	setup_iomux_enet();

	return cpu_eth_init(bis);
}

int board_early_init_f(void)
{
	setup_iomux_uart();

	return 0;
}

int board_init(void)
{
	/* address of boot parameters */
	gd->bd->bi_boot_params = PHYS_SDRAM + 0x100;

#ifdef CONFIG_SYS_I2C_MXC
	setup_i2c(2, CONFIG_SYS_I2C_SPEED, 0x7f, &i2c_pad_info2);
#endif

#ifdef CONFIG_MXC_SPI
	setup_spi();
#endif

#ifdef CONFIG_CMD_NAND
	setup_gpmi_nand();
#endif
	return 0;
}


#ifdef CONFIG_CMD_BMODE
/*
 * BOOT_CFG1, BOOT_CFG2, BOOT_CFG3, BOOT_CFG4
 * see Table 8-11 and Table 5-9
 *  BOOT_CFG1[7] = 1 (boot from NAND)
 *  BOOT_CFG1[5] = 0 - raw NAND
 *  BOOT_CFG1[4] = 0 - default pad settings
 *  BOOT_CFG1[3:2] = 00 - devices = 1
 *  BOOT_CFG1[1:0] = 00 - Row Address Cycles = 3
 *  BOOT_CFG2[4:3] = 00 - Boot Search Count = 2
 *  BOOT_CFG2[2:1] = 01 - Pages In Block = 64
 *  BOOT_CFG2[0] = 0 - Reset time 12ms
 */
static const struct boot_mode board_boot_modes[] = {
	/* NAND: 64pages per block, 3 row addr cycles, 2 copies of FCB/DBBT */
	{"nand", MAKE_CFGVAL(0x80, 0x02, 0x00, 0x00)},
	{"mmc0",  MAKE_CFGVAL(0x40, 0x20, 0x00, 0x00)},
	{NULL, 0},
};
#endif

int board_late_init(void)
{
#ifdef CONFIG_CMD_BMODE
	add_board_boot_modes(board_boot_modes);
#endif

	return 0;
}

#ifdef CONFIG_SPL_BUILD
#include <spl.h>
#include <linux/libfdt.h>

static const struct mx6dq_iomux_ddr_regs mx6_ddr_ioregs = {
	.dram_sdclk_0 = 0x00000030,
	.dram_sdclk_1 = 0x00000030,
	.dram_cas = 0x00000030,
	.dram_ras = 0x00000030,
	.dram_reset = 0x00000030,
	.dram_sdcke0 = 0x00000030,
	.dram_sdcke1 = 0x00000030,
	.dram_sdba2 = 0x00000000,
	.dram_sdodt0 = 0x00000030,
	.dram_sdodt1 = 0x00000030,
	.dram_sdqs0 = 0x00000030,
	.dram_sdqs1 = 0x00000030,
	.dram_sdqs2 = 0x00000030,
	.dram_sdqs3 = 0x00000030,
	.dram_sdqs4 = 0x00000030,
	.dram_sdqs5 = 0x00000030,
	.dram_sdqs6 = 0x00000030,
	.dram_sdqs7 = 0x00000030,
	.dram_dqm0 = 0x00000030,
	.dram_dqm1 = 0x00000030,
	.dram_dqm2 = 0x00000030,
	.dram_dqm3 = 0x00000030,
	.dram_dqm4 = 0x00000030,
	.dram_dqm5 = 0x00000030,
	.dram_dqm6 = 0x00000030,
	.dram_dqm7 = 0x00000030,
};

static const struct mx6dq_iomux_grp_regs mx6_grp_ioregs = {
	.grp_ddr_type =  0x000C0000,
	.grp_ddrmode_ctl =  0x00020000,
	.grp_ddrpke =  0x00000000,
	.grp_addds = IMX6Q_DRIVE_STRENGTH,
	.grp_ctlds = IMX6Q_DRIVE_STRENGTH,
	.grp_ddrmode =  0x00020000,
	.grp_b0ds = IMX6Q_DRIVE_STRENGTH,
	.grp_b1ds = IMX6Q_DRIVE_STRENGTH,
	.grp_b2ds = IMX6Q_DRIVE_STRENGTH,
	.grp_b3ds = IMX6Q_DRIVE_STRENGTH,
	.grp_b4ds = IMX6Q_DRIVE_STRENGTH,
	.grp_b5ds = IMX6Q_DRIVE_STRENGTH,
	.grp_b6ds = IMX6Q_DRIVE_STRENGTH,
	.grp_b7ds = IMX6Q_DRIVE_STRENGTH,
};

static const struct mx6_mmdc_calibration mx6_mmcd_calib = {
	.p0_mpwldectrl0 =  0x00140014,
	.p0_mpwldectrl1 =  0x000A0015,
	.p1_mpwldectrl0 =  0x000A001E,
	.p1_mpwldectrl1 =  0x000A0015,
	.p0_mpdgctrl0 =  0x43080314,
	.p0_mpdgctrl1 =  0x02680300,
	.p1_mpdgctrl0 =  0x430C0318,
	.p1_mpdgctrl1 =  0x03000254,
	.p0_mprddlctl =  0x3A323234,
	.p1_mprddlctl =  0x3E3C3242,
	.p0_mpwrdlctl =  0x2A2E3632,
	.p1_mpwrdlctl =  0x3C323E34,
};

static struct mx6_ddr3_cfg mem_ddr = {
	.mem_speed = 1600,
	.density = 2,
	.width = 16,
	.banks = 8,
	.rowaddr = 14,
	.coladdr = 10,
	.pagesz = 2,
	.trcd = 1375,
	.trcmin = 4875,
	.trasmin = 3500,
	.SRT       = 1,
};

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

static void spl_dram_init(void)
{
	struct mx6_ddr_sysinfo sysinfo = {
		/* width of data bus:0=16,1=32,2=64 */
		.dsize = 2,
		/* config for full 4GB range so that get_mem_size() works */
		.cs_density = 32, /* 32Gb per CS */
		/* single chip select */
		.ncs = 1,
		.cs1_mirror = 0,
		.rtt_wr = 1 /*DDR3_RTT_60_OHM*/,	/* RTT_Wr = RZQ/4 */
		.rtt_nom = 1 /*DDR3_RTT_60_OHM*/,	/* RTT_Nom = RZQ/4 */
		.walat = 1,	/* Write additional latency */
		.ralat = 5,	/* Read additional latency */
		.mif3_mode = 3,	/* Command prediction working mode */
		.bi_on = 1,	/* Bank interleaving enabled */
		.sde_to_rst = 0x10,	/* 14 cycles, 200us (JEDEC default) */
		.rst_to_cke = 0x23,	/* 33 cycles, 500us (JEDEC default) */
		.ddr_type = DDR_TYPE_DDR3,
		.refsel = 1,	/* Refresh cycles at 32KHz */
		.refr = 7,	/* 8 refresh commands per refresh cycle */
	};

	mx6dq_dram_iocfg(64, &mx6_ddr_ioregs, &mx6_grp_ioregs);
	mx6_dram_cfg(&sysinfo, &mx6_mmcd_calib, &mem_ddr);
}

void board_boot_order(u32 *spl_boot_list)
{
	spl_boot_list[0] = spl_boot_device();
	printf("Boot device %x\n", spl_boot_list[0]);
	switch (spl_boot_list[0]) {
	case BOOT_DEVICE_SPI:
		spl_boot_list[1] = BOOT_DEVICE_UART;
		break;
	case BOOT_DEVICE_MMC1:
		spl_boot_list[1] = BOOT_DEVICE_SPI;
		spl_boot_list[2] = BOOT_DEVICE_UART;
		break;
	default:
		printf("Boot device %x\n", spl_boot_list[0]);
	}
}

void board_init_f(ulong dummy)
{
#ifdef CONFIG_CMD_NAND
	/* Enable NAND */
	setup_gpmi_nand();
#endif

	/* setup clock gating */
	ccgr_init();

	/* setup AIPS and disable watchdog */
	arch_cpu_init();

	/* setup AXI */
	gpr_init();

	board_early_init_f();

	/* setup GP timer */
	timer_init();

	setup_spi();

	/* UART clocks enabled and gd valid - init serial console */
	preloader_console_init();

	/* DDR initialization */
	spl_dram_init();

	/* Clear the BSS. */
	memset(__bss_start, 0, __bss_end - __bss_start);

	/* load/boot image from boot device */
	board_init_r(NULL, 0);
}
#endif

// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright (C) 2017 Stefano Babic <sbabic@denx.de>
 */

#include <common.h>
#include <asm/io.h>
#include <asm/arch/clock.h>
#include <asm/arch/imx-regs.h>
#include <asm/arch/iomux.h>
#include <asm/arch/crm_regs.h>
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
#define USDHC2_CD_GPIO	IMX_GPIO_NR(1, 4)
#define GREEN_LED	IMX_GPIO_NR(2, 31)
#define RED_LED		IMX_GPIO_NR(1, 30)
#define IMX6Q_DRIVE_STRENGTH	0x30

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
	IOMUX_PADS(PAD_ENET_MDIO__ENET_MDIO	| MUX_PAD_CTRL(ENET_PAD_CTRL)),
	IOMUX_PADS(PAD_ENET_MDC__ENET_MDC	| MUX_PAD_CTRL(ENET_PAD_CTRL)),
	IOMUX_PADS(PAD_RGMII_TXC__RGMII_TXC	| MUX_PAD_CTRL(ENET_PAD_CTRL)),
	IOMUX_PADS(PAD_RGMII_TD0__RGMII_TD0	| MUX_PAD_CTRL(ENET_PAD_CTRL)),
	IOMUX_PADS(PAD_RGMII_TD1__RGMII_TD1	| MUX_PAD_CTRL(ENET_PAD_CTRL)),
	IOMUX_PADS(PAD_RGMII_TD2__RGMII_TD2	| MUX_PAD_CTRL(ENET_PAD_CTRL)),
	IOMUX_PADS(PAD_RGMII_TD3__RGMII_TD3	| MUX_PAD_CTRL(ENET_PAD_CTRL)),
	IOMUX_PADS(PAD_RGMII_TX_CTL__RGMII_TX_CTL |
			MUX_PAD_CTRL(ENET_PAD_CTRL)),
	IOMUX_PADS(PAD_ENET_REF_CLK__ENET_TX_CLK |
			MUX_PAD_CTRL(ENET_PAD_CTRL)),
	IOMUX_PADS(PAD_RGMII_RXC__RGMII_RXC	| MUX_PAD_CTRL(ENET_PAD_CTRL)),
	IOMUX_PADS(PAD_RGMII_RD0__RGMII_RD0	| MUX_PAD_CTRL(ENET_PAD_CTRL)),
	IOMUX_PADS(PAD_RGMII_RD1__RGMII_RD1	| MUX_PAD_CTRL(ENET_PAD_CTRL)),
	IOMUX_PADS(PAD_RGMII_RD2__RGMII_RD2	| MUX_PAD_CTRL(ENET_PAD_CTRL)),
	IOMUX_PADS(PAD_RGMII_RD3__RGMII_RD3	| MUX_PAD_CTRL(ENET_PAD_CTRL)),
	IOMUX_PADS(PAD_RGMII_RX_CTL__RGMII_RX_CTL |
			MUX_PAD_CTRL(ENET_PAD_CTRL)),
	IOMUX_PADS(PAD_SD2_DAT1__GPIO1_IO14	| MUX_PAD_CTRL(NO_PAD_CTRL)),
};

static iomux_v3_cfg_t const ecspi3_pads[] = {
	IOMUX_PADS(PAD_DISP0_DAT0__ECSPI3_SCLK | MUX_PAD_CTRL(SPI_PAD_CTRL)),
	IOMUX_PADS(PAD_DISP0_DAT1__ECSPI3_MOSI | MUX_PAD_CTRL(SPI_PAD_CTRL)),
	IOMUX_PADS(PAD_DISP0_DAT2__ECSPI3_MISO | MUX_PAD_CTRL(SPI_PAD_CTRL)),
	IOMUX_PADS(PAD_DISP0_DAT3__GPIO4_IO24  | MUX_PAD_CTRL(NO_PAD_CTRL)),
};

static iomux_v3_cfg_t const gpios_pads[] = {
	IOMUX_PADS(PAD_SD4_DAT3__GPIO2_IO11 | MUX_PAD_CTRL(NO_PAD_CTRL)),
	IOMUX_PADS(PAD_SD4_DAT4__GPIO2_IO12 | MUX_PAD_CTRL(NO_PAD_CTRL)),
	IOMUX_PADS(PAD_SD4_DAT5__GPIO2_IO13 | MUX_PAD_CTRL(NO_PAD_CTRL)),
	IOMUX_PADS(PAD_SD4_DAT6__GPIO2_IO14 | MUX_PAD_CTRL(NO_PAD_CTRL)),
	IOMUX_PADS(PAD_SD4_DAT7__GPIO2_IO15 | MUX_PAD_CTRL(NO_PAD_CTRL)),
	IOMUX_PADS(PAD_EIM_EB3__GPIO2_IO31 | MUX_PAD_CTRL(NO_PAD_CTRL)),
	IOMUX_PADS(PAD_ENET_TXD0__GPIO1_IO30 | MUX_PAD_CTRL(NO_PAD_CTRL)),
	IOMUX_PADS(PAD_SD4_DAT3__GPIO2_IO11 | MUX_PAD_CTRL(NO_PAD_CTRL)),
};

#ifdef CONFIG_CMD_NAND
/* NAND */
static iomux_v3_cfg_t const nfc_pads[] = {
	IOMUX_PADS(PAD_NANDF_CLE__NAND_CLE	| MUX_PAD_CTRL(NAND_PAD_CTRL)),
	IOMUX_PADS(PAD_NANDF_ALE__NAND_ALE	| MUX_PAD_CTRL(NAND_PAD_CTRL)),
	IOMUX_PADS(PAD_NANDF_WP_B__NAND_WP_B	| MUX_PAD_CTRL(NAND_PAD_CTRL)),
	IOMUX_PADS(PAD_NANDF_RB0__NAND_READY_B	| MUX_PAD_CTRL(NAND_PAD_CTRL)),
	IOMUX_PADS(PAD_NANDF_CS0__NAND_CE0_B	| MUX_PAD_CTRL(NAND_PAD_CTRL)),
	IOMUX_PADS(PAD_NANDF_CS1__NAND_CE1_B	| MUX_PAD_CTRL(NAND_PAD_CTRL)),
	IOMUX_PADS(PAD_NANDF_CS2__NAND_CE2_B	| MUX_PAD_CTRL(NAND_PAD_CTRL)),
	IOMUX_PADS(PAD_NANDF_CS3__NAND_CE3_B	| MUX_PAD_CTRL(NAND_PAD_CTRL)),
	IOMUX_PADS(PAD_SD4_CMD__NAND_RE_B	| MUX_PAD_CTRL(NAND_PAD_CTRL)),
	IOMUX_PADS(PAD_SD4_CLK__NAND_WE_B	| MUX_PAD_CTRL(NAND_PAD_CTRL)),
	IOMUX_PADS(PAD_NANDF_D0__NAND_DATA00	| MUX_PAD_CTRL(NAND_PAD_CTRL)),
	IOMUX_PADS(PAD_NANDF_D1__NAND_DATA01	| MUX_PAD_CTRL(NAND_PAD_CTRL)),
	IOMUX_PADS(PAD_NANDF_D2__NAND_DATA02	| MUX_PAD_CTRL(NAND_PAD_CTRL)),
	IOMUX_PADS(PAD_NANDF_D3__NAND_DATA03	| MUX_PAD_CTRL(NAND_PAD_CTRL)),
	IOMUX_PADS(PAD_NANDF_D4__NAND_DATA04	| MUX_PAD_CTRL(NAND_PAD_CTRL)),
	IOMUX_PADS(PAD_NANDF_D5__NAND_DATA05	| MUX_PAD_CTRL(NAND_PAD_CTRL)),
	IOMUX_PADS(PAD_NANDF_D6__NAND_DATA06	| MUX_PAD_CTRL(NAND_PAD_CTRL)),
	IOMUX_PADS(PAD_NANDF_D7__NAND_DATA07	| MUX_PAD_CTRL(NAND_PAD_CTRL)),
	IOMUX_PADS(PAD_SD4_DAT0__NAND_DQS	| MUX_PAD_CTRL(NAND_PAD_CTRL)),
};
#endif

static struct i2c_pads_info i2c_pad_info = {
	.scl = {
		.i2c_mode = MX6Q_PAD_EIM_D21__I2C1_SCL | I2C_PAD,
		.gpio_mode = MX6Q_PAD_EIM_D21__GPIO3_IO21 | I2C_PAD,
		.gp = IMX_GPIO_NR(3, 21)
	},
	.sda = {
		.i2c_mode = MX6Q_PAD_EIM_D28__I2C1_SDA | I2C_PAD,
		.gpio_mode = MX6Q_PAD_EIM_D28__GPIO3_IO28 | I2C_PAD,
		.gp = IMX_GPIO_NR(3, 28)
	}
};

static struct fsl_esdhc_cfg usdhc_cfg[] = {
	{USDHC3_BASE_ADDR,
	.max_bus_width = 4},
	{.esdhc_base = USDHC2_BASE_ADDR,
	.max_bus_width = 4},
};

#if !defined(CONFIG_SPL_BUILD)
static iomux_v3_cfg_t const usdhc2_pads[] = {
	IOMUX_PADS(PAD_SD2_CLK__SD2_CLK	| MUX_PAD_CTRL(USDHC_PAD_CTRL)),
	IOMUX_PADS(PAD_SD2_CMD__SD2_CMD	| MUX_PAD_CTRL(USDHC_PAD_CTRL)),
	IOMUX_PADS(PAD_SD2_DAT0__SD2_DATA0	| MUX_PAD_CTRL(USDHC_PAD_CTRL)),
	IOMUX_PADS(PAD_SD2_DAT1__SD2_DATA1	| MUX_PAD_CTRL(USDHC_PAD_CTRL)),
	IOMUX_PADS(PAD_SD2_DAT2__SD2_DATA2	| MUX_PAD_CTRL(USDHC_PAD_CTRL)),
	IOMUX_PADS(PAD_SD2_DAT3__SD2_DATA3	| MUX_PAD_CTRL(USDHC_PAD_CTRL)),
	IOMUX_PADS(PAD_EIM_BCLK__GPIO6_IO31	| MUX_PAD_CTRL(NO_PAD_CTRL)),
	IOMUX_PADS(PAD_GPIO_4__GPIO1_IO04	| MUX_PAD_CTRL(NO_PAD_CTRL)),
};
#endif

static iomux_v3_cfg_t const usdhc3_pads[] = {
	IOMUX_PADS(PAD_SD3_CLK__SD3_CLK	| MUX_PAD_CTRL(USDHC_PAD_CTRL)),
	IOMUX_PADS(PAD_SD3_CMD__SD3_CMD	| MUX_PAD_CTRL(USDHC_PAD_CTRL)),
	IOMUX_PADS(PAD_SD3_DAT0__SD3_DATA0	| MUX_PAD_CTRL(USDHC_PAD_CTRL)),
	IOMUX_PADS(PAD_SD3_DAT1__SD3_DATA1	| MUX_PAD_CTRL(USDHC_PAD_CTRL)),
	IOMUX_PADS(PAD_SD3_DAT2__SD3_DATA2	| MUX_PAD_CTRL(USDHC_PAD_CTRL)),
	IOMUX_PADS(PAD_SD3_DAT3__SD3_DATA3	| MUX_PAD_CTRL(USDHC_PAD_CTRL)),
	IOMUX_PADS(PAD_SD3_DAT4__SD3_DATA4	| MUX_PAD_CTRL(USDHC_PAD_CTRL)),
	IOMUX_PADS(PAD_SD3_DAT5__SD3_DATA5	| MUX_PAD_CTRL(USDHC_PAD_CTRL)),
	IOMUX_PADS(PAD_SD3_DAT6__SD3_DATA6	| MUX_PAD_CTRL(USDHC_PAD_CTRL)),
	IOMUX_PADS(PAD_SD3_DAT7__SD3_DATA7	| MUX_PAD_CTRL(USDHC_PAD_CTRL)),
};

int board_mmc_get_env_dev(int devno)
{
	return devno - 1;
}

int board_mmc_getcd(struct mmc *mmc)
{
	struct fsl_esdhc_cfg *cfg = (struct fsl_esdhc_cfg *)mmc->priv;
	int ret = 0;

	switch (cfg->esdhc_base) {
	case USDHC2_BASE_ADDR:
		ret = !gpio_get_value(USDHC2_CD_GPIO);
		ret = 1;
		break;
	case USDHC3_BASE_ADDR:
		ret = 1;
		break;
	}

	return ret;
}

#ifndef CONFIG_SPL_BUILD
int board_mmc_init(bd_t *bis)
{
	int ret;
	int i;

	for (i = 0; i < CONFIG_SYS_FSL_USDHC_NUM; i++) {
		switch (i) {
		case 0:
			SETUP_IOMUX_PADS(usdhc3_pads);
			usdhc_cfg[0].sdhc_clk = mxc_get_clock(MXC_ESDHC3_CLK);
			break;
		case 1:
			SETUP_IOMUX_PADS(usdhc2_pads);
			gpio_direction_input(USDHC2_CD_GPIO);
			usdhc_cfg[1].sdhc_clk = mxc_get_clock(MXC_ESDHC2_CLK);
			break;
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
}
#endif

static void setup_iomux_uart(void)
{
	SETUP_IOMUX_PADS(uart4_pads);
}

static void setup_iomux_enet(void)
{
	SETUP_IOMUX_PADS(enet_pads);

	gpio_direction_output(ENET_PHY_RESET_GPIO, 0);
	mdelay(10);
	gpio_set_value(ENET_PHY_RESET_GPIO, 1);
	mdelay(30);
}

static void setup_spi(void)
{
	gpio_request(IMX_GPIO_NR(4, 24), "spi_cs0");
	gpio_direction_output(IMX_GPIO_NR(4, 24), 1);

	SETUP_IOMUX_PADS(ecspi3_pads);

	enable_spi_clk(true, 2);
}

static void setup_gpios(void)
{
	SETUP_IOMUX_PADS(gpios_pads);
}

#ifdef CONFIG_CMD_NAND
static void setup_gpmi_nand(void)
{
	struct mxc_ccm_reg *mxc_ccm = (struct mxc_ccm_reg *)CCM_BASE_ADDR;

	/* config gpmi nand iomux */
	SETUP_IOMUX_PADS(nfc_pads);

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

/*
 * Board revision is coded in 4 GPIOs
 */
u32 get_board_rev(void)
{
	u32 rev;
	int i;

	for (i = 0, rev = 0; i < 4; i++)
		rev |= (gpio_get_value(IMX_GPIO_NR(2, 12 + i)) << i);

	return 16 - rev;
}

int board_spi_cs_gpio(unsigned bus, unsigned cs)
{
	if (bus != 2 || (cs != 0))
		return -EINVAL;

	return IMX_GPIO_NR(4, 24);
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
	setup_i2c(0, CONFIG_SYS_I2C_SPEED, 0x7f, &i2c_pad_info);
#endif

#ifdef CONFIG_MXC_SPI
	setup_spi();
#endif

	setup_gpios();

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
	char buf[10];
#ifdef CONFIG_CMD_BMODE
	add_board_boot_modes(board_boot_modes);
#endif

	snprintf(buf, sizeof(buf), "%d", get_board_rev());
	env_set("board_rev", buf);

	return 0;
}

#ifdef CONFIG_SPL_BUILD
#include <asm/arch/mx6-ddr.h>
#include <spl.h>
#include <linux/libfdt.h>

#define MX6_PHYFLEX_ERR006282	IMX_GPIO_NR(2, 11)
static void phyflex_err006282_workaround(void)
{
	/*
	 * Boards beginning with 1362.2 have the SD4_DAT3 pin connected
	 * to the CMIC. If this pin isn't toggled within 10s the boards
	 * reset. The pin is unconnected on older boards, so we do not
	 * need a check for older boards before applying this fixup.
	 */

	gpio_direction_output(MX6_PHYFLEX_ERR006282, 0);
	mdelay(2);
	gpio_direction_output(MX6_PHYFLEX_ERR006282, 1);
	mdelay(2);
	gpio_set_value(MX6_PHYFLEX_ERR006282, 0);

	gpio_direction_input(MX6_PHYFLEX_ERR006282);
}

static const struct mx6dq_iomux_ddr_regs mx6_ddr_ioregs = {
	.dram_sdclk_0 = 0x00000030,
	.dram_sdclk_1 = 0x00000030,
	.dram_cas = 0x00000030,
	.dram_ras = 0x00000030,
	.dram_reset = 0x00000030,
	.dram_sdcke0 = 0x00003000,
	.dram_sdcke1 = 0x00003000,
	.dram_sdba2 = 0x00000030,
	.dram_sdodt0 = 0x00000030,
	.dram_sdodt1 = 0x00000030,

	.dram_sdqs0 = 0x00000028,
	.dram_sdqs1 = 0x00000028,
	.dram_sdqs2 = 0x00000028,
	.dram_sdqs3 = 0x00000028,
	.dram_sdqs4 = 0x00000028,
	.dram_sdqs5 = 0x00000028,
	.dram_sdqs6 = 0x00000028,
	.dram_sdqs7 = 0x00000028,
	.dram_dqm0 = 0x00000028,
	.dram_dqm1 = 0x00000028,
	.dram_dqm2 = 0x00000028,
	.dram_dqm3 = 0x00000028,
	.dram_dqm4 = 0x00000028,
	.dram_dqm5 = 0x00000028,
	.dram_dqm6 = 0x00000028,
	.dram_dqm7 = 0x00000028,
};

static const struct mx6dq_iomux_grp_regs mx6_grp_ioregs = {
	.grp_ddr_type =  0x000C0000,
	.grp_ddrmode_ctl =  0x00020000,
	.grp_ddrpke =  0x00000000,
	.grp_addds = IMX6Q_DRIVE_STRENGTH,
	.grp_ctlds = IMX6Q_DRIVE_STRENGTH,
	.grp_ddrmode =  0x00020000,
	.grp_b0ds = 0x00000028,
	.grp_b1ds = 0x00000028,
	.grp_b2ds = 0x00000028,
	.grp_b3ds = 0x00000028,
	.grp_b4ds = 0x00000028,
	.grp_b5ds = 0x00000028,
	.grp_b6ds = 0x00000028,
	.grp_b7ds = 0x00000028,
};

static const struct mx6_mmdc_calibration mx6_mmcd_calib = {
	.p0_mpwldectrl0 =  0x00110011,
	.p0_mpwldectrl1 =  0x00240024,
	.p1_mpwldectrl0 =  0x00260038,
	.p1_mpwldectrl1 =  0x002C0038,
	.p0_mpdgctrl0 =  0x03400350,
	.p0_mpdgctrl1 =  0x03440340,
	.p1_mpdgctrl0 =  0x034C0354,
	.p1_mpdgctrl1 =  0x035C033C,
	.p0_mprddlctl =  0x322A2A2A,
	.p1_mprddlctl =  0x302C2834,
	.p0_mpwrdlctl =  0x34303834,
	.p1_mpwrdlctl =  0x422A3E36,
};

/* Index in RAM Chip array */
enum {
	RAM_MT64K,
	RAM_MT128K,
	RAM_MT256K
};

static struct mx6_ddr3_cfg mt41k_xx[] = {
/* MT41K64M16JT-125 (1Gb density) */
	{
	.mem_speed = 1600,
	.density = 1,
	.width = 16,
	.banks = 8,
	.rowaddr = 13,
	.coladdr = 10,
	.pagesz = 2,
	.trcd = 1375,
	.trcmin = 4875,
	.trasmin = 3500,
	.SRT       = 1,
	},

/* MT41K256M16JT-125 (2Gb density) */
	{
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
	},

/* MT41K256M16JT-125 (4Gb density) */
	{
	.mem_speed = 1600,
	.density = 4,
	.width = 16,
	.banks = 8,
	.rowaddr = 15,
	.coladdr = 10,
	.pagesz = 2,
	.trcd = 1375,
	.trcmin = 4875,
	.trasmin = 3500,
	.SRT       = 1,
	}
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

static void spl_dram_init(struct mx6_ddr_sysinfo *sysinfo,
				struct mx6_ddr3_cfg *mem_ddr)
{
	mx6dq_dram_iocfg(64, &mx6_ddr_ioregs, &mx6_grp_ioregs);
	mx6_dram_cfg(sysinfo, &mx6_mmcd_calib, mem_ddr);
}

int board_mmc_init(bd_t *bis)
{
	if (spl_boot_device() == BOOT_DEVICE_SPI)
		printf("MMC SEtup, Boot SPI");

	SETUP_IOMUX_PADS(usdhc3_pads);
	usdhc_cfg[0].esdhc_base = USDHC3_BASE_ADDR;
	usdhc_cfg[0].sdhc_clk = mxc_get_clock(MXC_ESDHC3_CLK);
	usdhc_cfg[0].max_bus_width = 4;
	gd->arch.sdhc_clk = usdhc_cfg[0].sdhc_clk;

	return fsl_esdhc_initialize(bis, &usdhc_cfg[0]);
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

/*
 * This is used because get_ram_size() does not
 * take care of cache, resulting a wrong size
 * pfla02 has just 1, 2 or 4 GB option
 * Function checks for mirrors in the first CS
 */
#define RAM_TEST_PATTERN	0xaa5555aa
#define MIN_BANK_SIZE		(512 * 1024 * 1024)

static unsigned int pfla02_detect_chiptype(void)
{
	u32 *p, *p1;
	unsigned int offset = MIN_BANK_SIZE;
	int i;

	for (i = 0; i < 2; i++) {
		p = (u32 *)PHYS_SDRAM;
		p1 = (u32 *)(PHYS_SDRAM + (i + 1) * offset);

		*p1 = 0;
		*p = RAM_TEST_PATTERN;

		/*
		 *  This is required to detect mirroring
		 *  else we read back values from cache
		 */
		flush_dcache_all();

		if (*p == *p1)
			return i;
	}
	return RAM_MT256K;
}

void board_init_f(ulong dummy)
{
	unsigned int ramchip;

	struct mx6_ddr_sysinfo sysinfo = {
		/* width of data bus:0=16,1=32,2=64 */
		.dsize = 2,
		/* config for full 4GB range so that get_mem_size() works */
		.cs_density = 32, /* 512 MB */
		/* single chip select */
#if IS_ENABLED(CONFIG_SPL_DRAM_1_BANK)
		.ncs = 1,
#else
		.ncs = 2,
#endif
		.cs1_mirror = 1,
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

	/* UART clocks enabled and gd valid - init serial console */
	preloader_console_init();

	setup_spi();

	setup_gpios();

	/* DDR initialization */
	spl_dram_init(&sysinfo, &mt41k_xx[RAM_MT256K]);
	ramchip = pfla02_detect_chiptype();
	debug("Detected chip %d\n", ramchip);
#if !IS_ENABLED(CONFIG_SPL_DRAM_1_BANK)
	switch (ramchip) {
		case RAM_MT64K:
			sysinfo.cs_density = 6;
			break;
		case RAM_MT128K:
			sysinfo.cs_density = 10;
			break;
		case RAM_MT256K:
			sysinfo.cs_density = 18;
			break;
	}
#endif
	spl_dram_init(&sysinfo, &mt41k_xx[ramchip]);

	/* Clear the BSS. */
	memset(__bss_start, 0, __bss_end - __bss_start);

	phyflex_err006282_workaround();

	/* load/boot image from boot device */
	board_init_r(NULL, 0);
}
#endif

// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright (C) 2014-2015 Freescale Semiconductor, Inc.
 * Copyright (C) Jasbir Matharu
 * Copyright (C) UDOO Team
 *
 * Author: Breno Lima <breno.lima@nxp.com>
 * Author: Francesco Montefoschi <francesco.monte@gmail.com>
 */

#include <asm/arch/clock.h>
#include <asm/arch/crm_regs.h>
#include <asm/arch/imx-regs.h>
#include <asm/arch/iomux.h>
#include <asm/arch/mx6-pins.h>
#include <asm/gpio.h>
#include <asm/mach-imx/iomux-v3.h>
#include <mmc.h>
#include <fsl_esdhc.h>
#include <asm/arch/crm_regs.h>
#include <asm/io.h>
#include <asm/mach-imx/mxc_i2c.h>
#include <asm/arch/sys_proto.h>
#include <spl.h>
#include <linux/sizes.h>
#include <common.h>
#include <i2c.h>
#include <miiphy.h>
#include <netdev.h>
#include <power/pmic.h>
#include <power/pfuze3000_pmic.h>
#include <malloc.h>

DECLARE_GLOBAL_DATA_PTR;

enum {
	UDOO_NEO_TYPE_BASIC,
	UDOO_NEO_TYPE_BASIC_KS,
	UDOO_NEO_TYPE_FULL,
	UDOO_NEO_TYPE_EXTENDED,
};

#define UART_PAD_CTRL  (PAD_CTL_PKE | PAD_CTL_PUE |		\
	PAD_CTL_PUS_100K_UP | PAD_CTL_SPEED_MED |		\
	PAD_CTL_DSE_40ohm   | PAD_CTL_SRE_FAST  | PAD_CTL_HYS)

#define USDHC_PAD_CTRL (PAD_CTL_PKE | PAD_CTL_PUE |		\
	PAD_CTL_PUS_22K_UP  | PAD_CTL_SPEED_LOW |		\
	PAD_CTL_DSE_80ohm   | PAD_CTL_SRE_FAST  | PAD_CTL_HYS)

#define I2C_PAD_CTRL (PAD_CTL_PKE | PAD_CTL_PUE |		\
	PAD_CTL_PUS_100K_UP | PAD_CTL_SPEED_MED |		\
	PAD_CTL_DSE_40ohm | PAD_CTL_HYS |		\
	PAD_CTL_ODE)

#define ENET_PAD_CTRL  (PAD_CTL_PUS_100K_UP | PAD_CTL_PUE |     \
	PAD_CTL_SPEED_MED   |                                   \
	PAD_CTL_DSE_40ohm   | PAD_CTL_SRE_FAST)

#define ENET_CLK_PAD_CTRL  (PAD_CTL_SPEED_MED | \
	PAD_CTL_DSE_120ohm   | PAD_CTL_SRE_FAST)

#define ENET_RX_PAD_CTRL  (PAD_CTL_PKE | PAD_CTL_PUE |          \
	PAD_CTL_SPEED_MED   | PAD_CTL_SRE_FAST)

#define WDOG_PAD_CTRL (PAD_CTL_PUE | PAD_CTL_PKE | PAD_CTL_SPEED_MED |	\
	PAD_CTL_DSE_40ohm)

#define BOARD_DETECT_PAD_CTRL (PAD_CTL_PKE | PAD_CTL_PUE |		\
	PAD_CTL_PUS_100K_UP | PAD_CTL_SPEED_MED |		\
	PAD_CTL_DSE_34ohm | PAD_CTL_HYS | PAD_CTL_SRE_FAST)
#define BOARD_DETECT_PAD_CFG (MUX_PAD_CTRL(BOARD_DETECT_PAD_CTRL) |	\
	MUX_MODE_SION)

int dram_init(void)
{
	gd->ram_size = imx_ddr_size();
	return 0;
}

#ifdef CONFIG_SYS_I2C_MXC
#define PC MUX_PAD_CTRL(I2C_PAD_CTRL)
/* I2C1 for PMIC */
static struct i2c_pads_info i2c_pad_info1 = {
	.scl = {
		.i2c_mode = MX6_PAD_GPIO1_IO00__I2C1_SCL | PC,
		.gpio_mode = MX6_PAD_GPIO1_IO00__GPIO1_IO_0 | PC,
		.gp = IMX_GPIO_NR(1, 0),
	},
	.sda = {
		.i2c_mode = MX6_PAD_GPIO1_IO01__I2C1_SDA | PC,
		.gpio_mode = MX6_PAD_GPIO1_IO01__GPIO1_IO_1 | PC,
		.gp = IMX_GPIO_NR(1, 1),
	},
};
#endif

#ifdef CONFIG_POWER
int power_init_board(void)
{
	struct pmic *p;
	int ret;
	unsigned int reg, rev_id;

	ret = power_pfuze3000_init(PFUZE3000_I2C_BUS);
	if (ret)
		return ret;

	p = pmic_get("PFUZE3000");
	ret = pmic_probe(p);
	if (ret)
		return ret;

	pmic_reg_read(p, PFUZE3000_DEVICEID, &reg);
	pmic_reg_read(p, PFUZE3000_REVID, &rev_id);
	printf("PMIC:  PFUZE3000 DEV_ID=0x%x REV_ID=0x%x\n", reg, rev_id);

	/* disable Low Power Mode during standby mode */
	pmic_reg_read(p, PFUZE3000_LDOGCTL, &reg);
	reg |= 0x1;
	ret = pmic_reg_write(p, PFUZE3000_LDOGCTL, reg);
	if (ret)
		return ret;

	ret = pmic_reg_write(p, PFUZE3000_SW1AMODE, 0xc);
	if (ret)
		return ret;

	ret = pmic_reg_write(p, PFUZE3000_SW1BMODE, 0xc);
	if (ret)
		return ret;

	ret = pmic_reg_write(p, PFUZE3000_SW2MODE, 0xc);
	if (ret)
		return ret;

	ret = pmic_reg_write(p, PFUZE3000_SW3MODE, 0xc);
	if (ret)
		return ret;

	/* set SW1A standby voltage 0.975V */
	pmic_reg_read(p, PFUZE3000_SW1ASTBY, &reg);
	reg &= ~0x3f;
	reg |= PFUZE3000_SW1AB_SETP(9750);
	ret = pmic_reg_write(p, PFUZE3000_SW1ASTBY, reg);
	if (ret)
		return ret;

	/* set SW1B standby voltage 0.975V */
	pmic_reg_read(p, PFUZE3000_SW1BSTBY, &reg);
	reg &= ~0x3f;
	reg |= PFUZE3000_SW1AB_SETP(9750);
	ret = pmic_reg_write(p, PFUZE3000_SW1BSTBY, reg);
	if (ret)
		return ret;

	/* set SW1A/VDD_ARM_IN step ramp up time from 16us to 4us/25mV */
	pmic_reg_read(p, PFUZE3000_SW1ACONF, &reg);
	reg &= ~0xc0;
	reg |= 0x40;
	ret = pmic_reg_write(p, PFUZE3000_SW1ACONF, reg);
	if (ret)
		return ret;

	/* set SW1B/VDD_SOC_IN step ramp up time from 16us to 4us/25mV */
	pmic_reg_read(p, PFUZE3000_SW1BCONF, &reg);
	reg &= ~0xc0;
	reg |= 0x40;
	ret = pmic_reg_write(p, PFUZE3000_SW1BCONF, reg);
	if (ret)
		return ret;

	/* set VDD_ARM_IN to 1.350V */
	pmic_reg_read(p, PFUZE3000_SW1AVOLT, &reg);
	reg &= ~0x3f;
	reg |= PFUZE3000_SW1AB_SETP(13500);
	ret = pmic_reg_write(p, PFUZE3000_SW1AVOLT, reg);
	if (ret)
		return ret;

	/* set VDD_SOC_IN to 1.350V */
	pmic_reg_read(p, PFUZE3000_SW1BVOLT, &reg);
	reg &= ~0x3f;
	reg |= PFUZE3000_SW1AB_SETP(13500);
	ret = pmic_reg_write(p, PFUZE3000_SW1BVOLT, reg);
	if (ret)
		return ret;

	/* set DDR_1_5V to 1.350V */
	pmic_reg_read(p, PFUZE3000_SW3VOLT, &reg);
	reg &= ~0x0f;
	reg |= PFUZE3000_SW3_SETP(13500);
	ret = pmic_reg_write(p, PFUZE3000_SW3VOLT, reg);
	if (ret)
		return ret;

	/* set VGEN2_1V5 to 1.5V */
	pmic_reg_read(p, PFUZE3000_VLDO2CTL, &reg);
	reg &= ~0x0f;
	reg |= PFUZE3000_VLDO_SETP(15000);
	/*  enable  */
	reg |= 0x10;
	ret = pmic_reg_write(p, PFUZE3000_VLDO2CTL, reg);
	if (ret)
		return ret;

	return 0;
}
#endif

static iomux_v3_cfg_t const uart1_pads[] = {
	MX6_PAD_GPIO1_IO04__UART1_TX | MUX_PAD_CTRL(UART_PAD_CTRL),
	MX6_PAD_GPIO1_IO05__UART1_RX | MUX_PAD_CTRL(UART_PAD_CTRL),
};

static iomux_v3_cfg_t const usdhc2_pads[] = {
	MX6_PAD_SD2_CLK__USDHC2_CLK | MUX_PAD_CTRL(USDHC_PAD_CTRL),
	MX6_PAD_SD2_CMD__USDHC2_CMD | MUX_PAD_CTRL(USDHC_PAD_CTRL),
	MX6_PAD_SD2_DATA0__USDHC2_DATA0 | MUX_PAD_CTRL(USDHC_PAD_CTRL),
	MX6_PAD_SD2_DATA1__USDHC2_DATA1 | MUX_PAD_CTRL(USDHC_PAD_CTRL),
	MX6_PAD_SD2_DATA2__USDHC2_DATA2 | MUX_PAD_CTRL(USDHC_PAD_CTRL),
	MX6_PAD_SD2_DATA3__USDHC2_DATA3 | MUX_PAD_CTRL(USDHC_PAD_CTRL),
	/* CD pin */
	MX6_PAD_SD1_DATA0__GPIO6_IO_2 | MUX_PAD_CTRL(NO_PAD_CTRL),
	/* Power */
	MX6_PAD_SD1_CMD__GPIO6_IO_1 | MUX_PAD_CTRL(NO_PAD_CTRL),
};

static iomux_v3_cfg_t const fec1_pads[] = {
	MX6_PAD_ENET1_MDC__ENET1_MDC | MUX_PAD_CTRL(ENET_PAD_CTRL),
	MX6_PAD_ENET1_MDIO__ENET1_MDIO | MUX_PAD_CTRL(ENET_PAD_CTRL),
	MX6_PAD_RGMII1_RX_CTL__ENET1_RX_EN | MUX_PAD_CTRL(ENET_RX_PAD_CTRL),
	MX6_PAD_RGMII1_RD0__ENET1_RX_DATA_0 | MUX_PAD_CTRL(ENET_RX_PAD_CTRL),
	MX6_PAD_RGMII1_RD1__ENET1_RX_DATA_1 | MUX_PAD_CTRL(ENET_RX_PAD_CTRL),
	MX6_PAD_RGMII1_TX_CTL__ENET1_TX_EN | MUX_PAD_CTRL(ENET_PAD_CTRL),
	MX6_PAD_RGMII1_RXC__ENET1_RX_ER | MUX_PAD_CTRL(ENET_RX_PAD_CTRL),
	MX6_PAD_RGMII1_TD0__ENET1_TX_DATA_0 | MUX_PAD_CTRL(ENET_PAD_CTRL),
	MX6_PAD_RGMII1_TD1__ENET1_TX_DATA_1 | MUX_PAD_CTRL(ENET_PAD_CTRL),
	MX6_PAD_ENET1_TX_CLK__ENET1_REF_CLK1 | MUX_PAD_CTRL(ENET_RX_PAD_CTRL),
	MX6_PAD_ENET2_TX_CLK__GPIO2_IO_9 | MUX_PAD_CTRL(ENET_RX_PAD_CTRL),
	MX6_PAD_ENET1_CRS__GPIO2_IO_1 | MUX_PAD_CTRL(ENET_PAD_CTRL),
};

static iomux_v3_cfg_t const phy_control_pads[] = {
	/* 25MHz Ethernet PHY Clock */
	MX6_PAD_ENET2_RX_CLK__ENET2_REF_CLK_25M |
	MUX_PAD_CTRL(ENET_CLK_PAD_CTRL),
};

static iomux_v3_cfg_t const board_recognition_pads[] = {
	/*Connected to R184*/
	MX6_PAD_NAND_READY_B__GPIO4_IO_13 | BOARD_DETECT_PAD_CFG,
	/*Connected to R185*/
	MX6_PAD_NAND_ALE__GPIO4_IO_0 | BOARD_DETECT_PAD_CFG,
};

static iomux_v3_cfg_t const wdog_b_pad = {
	MX6_PAD_GPIO1_IO13__GPIO1_IO_13 | MUX_PAD_CTRL(WDOG_PAD_CTRL),
};

static iomux_v3_cfg_t const peri_3v3_pads[] = {
	MX6_PAD_QSPI1A_DATA0__GPIO4_IO_16 | MUX_PAD_CTRL(NO_PAD_CTRL),
};

static void setup_iomux_uart(void)
{
	imx_iomux_v3_setup_multiple_pads(uart1_pads, ARRAY_SIZE(uart1_pads));
}

static int setup_fec(int fec_id)
{
	struct anatop_regs *anatop = (struct anatop_regs *)ANATOP_BASE_ADDR;
	int reg;

	imx_iomux_v3_setup_multiple_pads(phy_control_pads,
					 ARRAY_SIZE(phy_control_pads));

	/* Reset PHY */
	gpio_direction_output(IMX_GPIO_NR(2, 1) , 0);
	udelay(10000);
	gpio_set_value(IMX_GPIO_NR(2, 1), 1);
	udelay(100);

	reg = readl(&anatop->pll_enet);
	reg |= BM_ANADIG_PLL_ENET_REF_25M_ENABLE;
	writel(reg, &anatop->pll_enet);

	return enable_fec_anatop_clock(fec_id, ENET_25MHZ);
}

int board_eth_init(bd_t *bis)
{
	uint32_t base = IMX_FEC_BASE;
	struct mii_dev *bus = NULL;
	struct phy_device *phydev = NULL;
	int ret;

	imx_iomux_v3_setup_multiple_pads(fec1_pads, ARRAY_SIZE(fec1_pads));

	setup_fec(CONFIG_FEC_ENET_DEV);

	bus = fec_get_miibus(base, CONFIG_FEC_ENET_DEV);
	if (!bus)
		return -EINVAL;

	phydev = phy_find_by_mask(bus, (0x1 << CONFIG_FEC_MXC_PHYADDR),
					PHY_INTERFACE_MODE_RMII);
	if (!phydev) {
		free(bus);
		return -EINVAL;
	}

	ret  = fec_probe(bis, CONFIG_FEC_ENET_DEV, base, bus, phydev);
	if (ret) {
		free(bus);
		free(phydev);
		return ret;
	}
	return 0;
}

int board_phy_config(struct phy_device *phydev)
{
	if (phydev->drv->config)
		phydev->drv->config(phydev);

	return 0;
}

int board_init(void)
{
	/* Address of boot parameters */
	gd->bd->bi_boot_params = PHYS_SDRAM + 0x100;

	/*
	 * Because kernel set WDOG_B mux before pad with the commone pinctrl
	 * framwork now and wdog reset will be triggered once set WDOG_B mux
	 * with default pad setting, we set pad setting here to workaround this.
	 * Since imx_iomux_v3_setup_pad also set mux before pad setting, we set
	 * as GPIO mux firstly here to workaround it.
	 */
	imx_iomux_v3_setup_pad(wdog_b_pad);

	/* Enable PERI_3V3, which is used by SD2, ENET, LVDS, BT */
	imx_iomux_v3_setup_multiple_pads(peri_3v3_pads,
					 ARRAY_SIZE(peri_3v3_pads));

	/* Active high for ncp692 */
	gpio_direction_output(IMX_GPIO_NR(4, 16) , 1);

#ifdef CONFIG_SYS_I2C_MXC
	setup_i2c(0, CONFIG_SYS_I2C_SPEED, 0x7f, &i2c_pad_info1);
#endif

	return 0;
}

static int get_board_value(void)
{
	int r184, r185;

	imx_iomux_v3_setup_multiple_pads(board_recognition_pads,
					 ARRAY_SIZE(board_recognition_pads));

	gpio_direction_input(IMX_GPIO_NR(4, 13));
	gpio_direction_input(IMX_GPIO_NR(4, 0));

	r184 = gpio_get_value(IMX_GPIO_NR(4, 13));
	r185 = gpio_get_value(IMX_GPIO_NR(4, 0));

	/*
	 * Machine selection -
	 * Machine          r184,    r185
	 * ---------------------------------
	 * Basic              0        0
	 * Basic Ks           0        1
	 * Full               1        0
	 * Extended           1        1
	 */

	return (r184 << 1) + r185;
}

int board_early_init_f(void)
{
	setup_iomux_uart();

	return 0;
}

static struct fsl_esdhc_cfg usdhc_cfg[1] = {
	{USDHC2_BASE_ADDR, 0, 4},
};

#define USDHC2_PWR_GPIO IMX_GPIO_NR(6, 1)
#define USDHC2_CD_GPIO	IMX_GPIO_NR(6, 2)

int board_mmc_getcd(struct mmc *mmc)
{
	return !gpio_get_value(USDHC2_CD_GPIO);
}

int board_mmc_init(bd_t *bis)
{
	imx_iomux_v3_setup_multiple_pads(usdhc2_pads, ARRAY_SIZE(usdhc2_pads));
	usdhc_cfg[0].sdhc_clk = mxc_get_clock(MXC_ESDHC2_CLK);
	usdhc_cfg[0].esdhc_base = USDHC2_BASE_ADDR;
	gpio_direction_input(USDHC2_CD_GPIO);
	gpio_direction_output(USDHC2_PWR_GPIO, 1);

	gd->arch.sdhc_clk = usdhc_cfg[0].sdhc_clk;
	return fsl_esdhc_initialize(bis, &usdhc_cfg[0]);
}

static char *board_string(void)
{
	switch (get_board_value()) {
	case UDOO_NEO_TYPE_BASIC:
		return "BASIC";
	case UDOO_NEO_TYPE_BASIC_KS:
		return "BASICKS";
	case UDOO_NEO_TYPE_FULL:
		return "FULL";
	case UDOO_NEO_TYPE_EXTENDED:
		return "EXTENDED";
	}
	return "UNDEFINED";
}

int checkboard(void)
{
	printf("Board: UDOO Neo %s\n", board_string());
	return 0;
}

int board_late_init(void)
{
#ifdef CONFIG_ENV_VARS_UBOOT_RUNTIME_CONFIG
	env_set("board_name", board_string());
#endif

	return 0;
}

#ifdef CONFIG_SPL_BUILD

#include <linux/libfdt.h>
#include <asm/arch/mx6-ddr.h>

static const struct mx6sx_iomux_ddr_regs mx6_ddr_ioregs = {
	.dram_dqm0 = 0x00000028,
	.dram_dqm1 = 0x00000028,
	.dram_dqm2 = 0x00000028,
	.dram_dqm3 = 0x00000028,
	.dram_ras = 0x00000020,
	.dram_cas = 0x00000020,
	.dram_odt0 = 0x00000020,
	.dram_odt1 = 0x00000020,
	.dram_sdba2 = 0x00000000,
	.dram_sdcke0 = 0x00003000,
	.dram_sdcke1 = 0x00003000,
	.dram_sdclk_0 = 0x00000030,
	.dram_sdqs0 = 0x00000028,
	.dram_sdqs1 = 0x00000028,
	.dram_sdqs2 = 0x00000028,
	.dram_sdqs3 = 0x00000028,
	.dram_reset = 0x00000020,
};

static const struct mx6sx_iomux_grp_regs mx6_grp_ioregs = {
	.grp_addds = 0x00000020,
	.grp_ddrmode_ctl = 0x00020000,
	.grp_ddrpke = 0x00000000,
	.grp_ddrmode = 0x00020000,
	.grp_b0ds = 0x00000028,
	.grp_b1ds = 0x00000028,
	.grp_ctlds = 0x00000020,
	.grp_ddr_type = 0x000c0000,
	.grp_b2ds = 0x00000028,
	.grp_b3ds = 0x00000028,
};

static const struct mx6_mmdc_calibration neo_mmcd_calib = {
	.p0_mpwldectrl0 = 0x000E000B,
	.p0_mpwldectrl1 = 0x000E0010,
	.p0_mpdgctrl0 = 0x41600158,
	.p0_mpdgctrl1 = 0x01500140,
	.p0_mprddlctl = 0x3A383E3E,
	.p0_mpwrdlctl = 0x3A383C38,
};

static const struct mx6_mmdc_calibration neo_basic_mmcd_calib = {
	.p0_mpwldectrl0 = 0x001E0022,
	.p0_mpwldectrl1 = 0x001C0019,
	.p0_mpdgctrl0 = 0x41540150,
	.p0_mpdgctrl1 = 0x01440138,
	.p0_mprddlctl = 0x403E4644,
	.p0_mpwrdlctl = 0x3C3A4038,
};

/* MT41K256M16 */
static struct mx6_ddr3_cfg neo_mem_ddr = {
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
};

/* MT41K128M16 */
static struct mx6_ddr3_cfg neo_basic_mem_ddr = {
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
};

static void ccgr_init(void)
{
	struct mxc_ccm_reg *ccm = (struct mxc_ccm_reg *)CCM_BASE_ADDR;

	writel(0xFFFFFFFF, &ccm->CCGR0);
	writel(0xFFFFFFFF, &ccm->CCGR1);
	writel(0xFFFFFFFF, &ccm->CCGR2);
	writel(0xFFFFFFFF, &ccm->CCGR3);
	writel(0xFFFFFFFF, &ccm->CCGR4);
	writel(0xFFFFFFFF, &ccm->CCGR5);
	writel(0xFFFFFFFF, &ccm->CCGR6);
	writel(0xFFFFFFFF, &ccm->CCGR7);
}

static void spl_dram_init(void)
{
	int board = get_board_value();

	struct mx6_ddr_sysinfo sysinfo = {
		.dsize = 1, /* width of data bus: 1 = 32 bits */
		.cs_density = 24,
		.ncs = 1,
		.cs1_mirror = 0,
		.rtt_wr = 2,
		.rtt_nom = 2,		/* RTT_Nom = RZQ/2 */
		.walat = 1,		/* Write additional latency */
		.ralat = 5,		/* Read additional latency */
		.mif3_mode = 3,		/* Command prediction working mode */
		.bi_on = 1,		/* Bank interleaving enabled */
		.sde_to_rst = 0x10,	/* 14 cycles, 200us (JEDEC default) */
		.rst_to_cke = 0x23,	/* 33 cycles, 500us (JEDEC default) */
	};

	mx6sx_dram_iocfg(32, &mx6_ddr_ioregs, &mx6_grp_ioregs);
	if (board == UDOO_NEO_TYPE_BASIC || board == UDOO_NEO_TYPE_BASIC_KS)
		mx6_dram_cfg(&sysinfo, &neo_basic_mmcd_calib,
			     &neo_basic_mem_ddr);
	else
		mx6_dram_cfg(&sysinfo, &neo_mmcd_calib, &neo_mem_ddr);
}

void board_init_f(ulong dummy)
{
	ccgr_init();

	/* setup AIPS and disable watchdog */
	arch_cpu_init();

	board_early_init_f();

	/* setup GP timer */
	timer_init();

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

// SPDX-License-Identifier: GPL-2.0+
/*
 * U-Boot board functions for CompuLab CL-SOM-iMX7 module
 *
 * (C) Copyright 2017 CompuLab, Ltd. http://www.compulab.com
 *
 * Author: Uri Mashiach <uri.mashiach@compulab.co.il>
 */

#include <common.h>
#include <environment.h>
#include <mmc.h>
#include <phy.h>
#include <netdev.h>
#include <fsl_esdhc.h>
#include <power/pmic.h>
#include <power/pfuze3000_pmic.h>
#include <asm/mach-imx/mxc_i2c.h>
#include <asm/mach-imx/iomux-v3.h>
#include <asm/arch-mx7/mx7-pins.h>
#include <asm/arch-mx7/sys_proto.h>
#include <asm/arch-mx7/clock.h>
#include "../common/eeprom.h"
#include "common.h"

DECLARE_GLOBAL_DATA_PTR;

#ifdef CONFIG_SYS_I2C_MXC

#define I2C_PAD_CTRL		(PAD_CTL_DSE_3P3V_32OHM | PAD_CTL_SRE_SLOW | \
				PAD_CTL_HYS)

#define CL_SOM_IMX7_GPIO_I2C2_SCL	IMX_GPIO_NR(1, 6)
#define CL_SOM_IMX7_GPIO_I2C2_SDA	IMX_GPIO_NR(1, 7)

static struct i2c_pads_info cl_som_imx7_i2c_pad_info2 = {
	.scl = {
		.i2c_mode = MX7D_PAD_GPIO1_IO06__I2C2_SCL |
			MUX_PAD_CTRL(I2C_PAD_CTRL),
		.gpio_mode = MX7D_PAD_GPIO1_IO06__GPIO1_IO6 |
			MUX_PAD_CTRL(I2C_PAD_CTRL),
		.gp = CL_SOM_IMX7_GPIO_I2C2_SCL,
	},
	.sda = {
		.i2c_mode = MX7D_PAD_GPIO1_IO07__I2C2_SDA |
			MUX_PAD_CTRL(I2C_PAD_CTRL),
		.gpio_mode = MX7D_PAD_GPIO1_IO07__GPIO1_IO7 |
			MUX_PAD_CTRL(I2C_PAD_CTRL),
		.gp = CL_SOM_IMX7_GPIO_I2C2_SDA,
	},
};

/*
 * cl_som_imx7_setup_i2c() - I2C  pinmux configuration.
 */
static void cl_som_imx7_setup_i2c(void)
{
	setup_i2c(0, CONFIG_SYS_I2C_SPEED, 0x7f, &cl_som_imx7_i2c_pad_info2);
}
#else /* !CONFIG_SYS_I2C_MXC */
static void cl_som_imx7_setup_i2c(void) {}
#endif /* CONFIG_SYS_I2C_MXC */

int dram_init(void)
{
	gd->ram_size = imx_ddr_size();

	return 0;
}

#ifdef CONFIG_FSL_ESDHC

#define CL_SOM_IMX7_GPIO_USDHC3_PWR	IMX_GPIO_NR(6, 11)

static struct fsl_esdhc_cfg cl_som_imx7_usdhc_cfg[3] = {
	{USDHC1_BASE_ADDR, 0, 4},
	{USDHC3_BASE_ADDR},
};

int board_mmc_init(bd_t *bis)
{
	int i, ret;
	/*
	 * According to the board_mmc_init() the following map is done:
	 * (U-boot device node)    (Physical Port)
	 * mmc0                    USDHC1
	 * mmc2                    USDHC3 (eMMC)
	 */
	for (i = 0; i < CONFIG_SYS_FSL_USDHC_NUM; i++) {
		switch (i) {
		case 0:
			cl_som_imx7_usdhc1_pads_set();
			gpio_request(CL_SOM_IMX7_GPIO_USDHC1_CD, "usdhc1_cd");
			cl_som_imx7_usdhc_cfg[0].sdhc_clk =
				mxc_get_clock(MXC_ESDHC_CLK);
			break;
		case 1:
			cl_som_imx7_usdhc3_emmc_pads_set();
			gpio_request(CL_SOM_IMX7_GPIO_USDHC3_PWR, "usdhc3_pwr");
			gpio_direction_output(CL_SOM_IMX7_GPIO_USDHC3_PWR, 0);
			udelay(500);
			gpio_direction_output(CL_SOM_IMX7_GPIO_USDHC3_PWR, 1);
			cl_som_imx7_usdhc_cfg[1].sdhc_clk =
				mxc_get_clock(MXC_ESDHC3_CLK);
			break;
		default:
			printf("Warning: you configured more USDHC controllers "
				"(%d) than supported by the board\n", i + 1);
			return -EINVAL;
		}

		ret = fsl_esdhc_initialize(bis, &cl_som_imx7_usdhc_cfg[i]);
		if (ret)
			return ret;
	}

	return 0;
}
#endif /* CONFIG_FSL_ESDHC */

#ifdef CONFIG_FEC_MXC

#define CL_SOM_IMX7_ETH1_PHY_NRST	IMX_GPIO_NR(1, 4)

/*
 * cl_som_imx7_rgmii_rework() - Ethernet PHY configuration.
 */
static void cl_som_imx7_rgmii_rework(struct phy_device *phydev)
{
	unsigned short val;

	/* Ar8031 phy SmartEEE feature cause link status generates glitch,
	 * which cause ethernet link down/up issue, so disable SmartEEE
	 */
	phy_write(phydev, MDIO_DEVAD_NONE, 0xd, 0x3);
	phy_write(phydev, MDIO_DEVAD_NONE, 0xe, 0x805d);
	phy_write(phydev, MDIO_DEVAD_NONE, 0xd, 0x4003);
	val = phy_read(phydev, MDIO_DEVAD_NONE, 0xe);
	val &= ~(0x1 << 8);
	phy_write(phydev, MDIO_DEVAD_NONE, 0xe, val);

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
}

int board_phy_config(struct phy_device *phydev)
{
	cl_som_imx7_rgmii_rework(phydev);

	if (phydev->drv->config)
		phydev->drv->config(phydev);

	return 0;
}

/*
 * cl_som_imx7_handle_mac_address() - set Ethernet MAC address environment.
 *
 * @env_var: MAC address environment variable
 * @eeprom_bus: I2C bus of the environment EEPROM
 *
 * @return: 0 on success, < 0 on failure
 */
static int cl_som_imx7_handle_mac_address(char *env_var, uint eeprom_bus)
{
	int ret;
	unsigned char enetaddr[6];

	ret = eth_env_get_enetaddr(env_var, enetaddr);
	if (ret)
		return 0;

	ret = cl_eeprom_read_mac_addr(enetaddr, eeprom_bus);
	if (ret)
		return ret;

	ret = is_valid_ethaddr(enetaddr);
	if (!ret)
		return -1;

	return eth_env_set_enetaddr(env_var, enetaddr);
}

#define CL_SOM_IMX7_FEC_DEV_ID_PRI 0

int board_eth_init(bd_t *bis)
{
	/* set Ethernet MAC address environment */
	cl_som_imx7_handle_mac_address("ethaddr", CONFIG_SYS_I2C_EEPROM_BUS);
	/* Ethernet interface pinmux configuration  */
	cl_som_imx7_phy1_rst_pads_set();
	cl_som_imx7_fec1_pads_set();
	/* PHY reset */
	gpio_request(CL_SOM_IMX7_ETH1_PHY_NRST, "eth1_phy_nrst");
	gpio_direction_output(CL_SOM_IMX7_ETH1_PHY_NRST, 0);
	mdelay(10);
	gpio_set_value(CL_SOM_IMX7_ETH1_PHY_NRST, 1);
	/* MAC initialization */
	return fecmxc_initialize_multi(bis, CL_SOM_IMX7_FEC_DEV_ID_PRI,
				       CONFIG_FEC_MXC_PHYADDR, IMX_FEC_BASE);
}

/*
 * cl_som_imx7_setup_fec() - Ethernet MAC 1 clock configuration.
 * - ENET1 reference clock mode select.
 * - ENET1_TX_CLK output driver is disabled when configured for ALT1.
 */
static void cl_som_imx7_setup_fec(void)
{
	struct iomuxc_gpr_base_regs *const iomuxc_gpr_regs
		= (struct iomuxc_gpr_base_regs *)IOMUXC_GPR_BASE_ADDR;

	/* Use 125M anatop REF_CLK1 for ENET1, clear gpr1[13], gpr1[17]*/
	clrsetbits_le32(&iomuxc_gpr_regs->gpr[1],
			(IOMUXC_GPR_GPR1_GPR_ENET1_TX_CLK_SEL_MASK |
			 IOMUXC_GPR_GPR1_GPR_ENET1_CLK_DIR_MASK), 0);

	set_clk_enet(ENET_125MHZ);
}
#else /* !CONFIG_FEC_MXC */
static void cl_som_imx7_setup_fec(void) {}
#endif /* CONFIG_FEC_MXC */

#ifdef CONFIG_SPI

static void cl_som_imx7_spi_init(void)
{
	cl_som_imx7_espi1_pads_set();
}
#else /* !CONFIG_SPI */
static void cl_som_imx7_spi_init(void) {}
#endif /* CONFIG_SPI */

int board_early_init_f(void)
{
	cl_som_imx7_uart1_pads_set();
	cl_som_imx7_usb_otg1_pads_set();

	return 0;
}

int board_init(void)
{
	/* address of boot parameters */
	gd->bd->bi_boot_params = PHYS_SDRAM + 0x100;
	cl_som_imx7_setup_i2c();
	cl_som_imx7_setup_fec();
	cl_som_imx7_spi_init();

	return 0;
}

#ifdef CONFIG_POWER
#define I2C_PMIC	0
int power_init_board(void)
{
	struct pmic *p;
	int ret;
	unsigned int reg, rev_id;

	ret = power_pfuze3000_init(I2C_PMIC);
	if (ret)
		return ret;

	p = pmic_get("PFUZE3000");
	ret = pmic_probe(p);
	if (ret)
		return ret;

	pmic_reg_read(p, PFUZE3000_DEVICEID, &reg);
	pmic_reg_read(p, PFUZE3000_REVID, &rev_id);
	printf("PMIC: PFUZE3000 DEV_ID=0x%x REV_ID=0x%x\n", reg, rev_id);

	/* disable Low Power Mode during standby mode */
	pmic_reg_write(p, PFUZE3000_LDOGCTL, 0x1);

	return 0;
}
#endif /* CONFIG_POWER */

/*
 * cl_som_imx7_setup_wdog() - watchdog configuration.
 * - Output WDOG_B signal to reset external pmic.
 * - Suspend the watchdog timer during low-power modes.
 */
void cl_som_imx7_setup_wdog(void)
{
	struct wdog_regs *wdog = (struct wdog_regs *)WDOG1_BASE_ADDR;

	cl_som_imx7_wdog_pads_set();
	set_wdog_reset(wdog);
       /*
	* Do not assert internal WDOG_RESET_B_DEB(controlled by bit 4),
	* since we use PMIC_PWRON to reset the board.
	*/
	clrsetbits_le16(&wdog->wcr, 0, 0x10);
}

int board_late_init(void)
{
	env_set("board_name", "CL-SOM-iMX7");
	cl_som_imx7_setup_wdog();
	return 0;
}

int checkboard(void)
{
	char *mode;

	if (IS_ENABLED(CONFIG_ARMV7_BOOT_SEC_DEFAULT))
		mode = "secure";
	else
		mode = "non-secure";

	printf("Board: CL-SOM-iMX7 in %s mode\n", mode);

	return 0;
}

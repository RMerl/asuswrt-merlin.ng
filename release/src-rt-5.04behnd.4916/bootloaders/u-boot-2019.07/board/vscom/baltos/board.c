// SPDX-License-Identifier: GPL-2.0+
/*
 * board.c
 *
 * Board functions for TI AM335X based boards
 *
 * Copyright (C) 2011, Texas Instruments, Incorporated - http://www.ti.com/
 */

#include <common.h>
#include <errno.h>
#include <linux/libfdt.h>
#include <spl.h>
#include <asm/arch/cpu.h>
#include <asm/arch/hardware.h>
#include <asm/arch/omap.h>
#include <asm/arch/ddr_defs.h>
#include <asm/arch/clock.h>
#include <asm/arch/gpio.h>
#include <asm/arch/mmc_host_def.h>
#include <asm/arch/sys_proto.h>
#include <asm/arch/mem.h>
#include <asm/arch/mux.h>
#include <asm/io.h>
#include <asm/emif.h>
#include <asm/gpio.h>
#include <i2c.h>
#include <miiphy.h>
#include <cpsw.h>
#include <power/tps65910.h>
#include <environment.h>
#include <watchdog.h>
#include "board.h"

DECLARE_GLOBAL_DATA_PTR;

/* GPIO that controls DIP switch and mPCIe slot */
#define DIP_S1			44
#define MPCIE_SW		100

static struct ctrl_dev *cdev = (struct ctrl_dev *)CTRL_DEVICE_BASE;

static int baltos_set_console(void)
{
	int val, i, dips = 0;
	char buf[7];

	for (i = 0; i < 4; i++) {
		sprintf(buf, "dip_s%d", i + 1);

		if (gpio_request(DIP_S1 + i, buf)) {
			printf("failed to export GPIO %d\n", DIP_S1 + i);
			return 0;
		}

		if (gpio_direction_input(DIP_S1 + i)) {
			printf("failed to set GPIO %d direction\n", DIP_S1 + i);
			return 0;
		}

		val = gpio_get_value(DIP_S1 + i);
		dips |= val << i;
	}

	printf("DIPs: 0x%1x\n", (~dips) & 0xf);

	if ((dips & 0xf) == 0xe)
		env_set("console", "ttyUSB0,115200n8");

	return 0;
}

static int read_eeprom(BSP_VS_HWPARAM *header)
{
	i2c_set_bus_num(1);

	/* Check if baseboard eeprom is available */
	if (i2c_probe(CONFIG_SYS_I2C_EEPROM_ADDR)) {
		puts("Could not probe the EEPROM; something fundamentally "
			"wrong on the I2C bus.\n");
		return -ENODEV;
	}

	/* read the eeprom using i2c */
	if (i2c_read(CONFIG_SYS_I2C_EEPROM_ADDR, 0, 1, (uchar *)header,
		     sizeof(BSP_VS_HWPARAM))) {
		puts("Could not read the EEPROM; something fundamentally"
			" wrong on the I2C bus.\n");
		return -EIO;
	}

	if (header->Magic != 0xDEADBEEF) {

		printf("Incorrect magic number (0x%x) in EEPROM\n",
				header->Magic);

		/* fill default values */
		header->SystemId = 211;
		header->MAC1[0] = 0x00;
		header->MAC1[1] = 0x00;
		header->MAC1[2] = 0x00;
		header->MAC1[3] = 0x00;
		header->MAC1[4] = 0x00;
		header->MAC1[5] = 0x01;

		header->MAC2[0] = 0x00;
		header->MAC2[1] = 0x00;
		header->MAC2[2] = 0x00;
		header->MAC2[3] = 0x00;
		header->MAC2[4] = 0x00;
		header->MAC2[5] = 0x02;

		header->MAC3[0] = 0x00;
		header->MAC3[1] = 0x00;
		header->MAC3[2] = 0x00;
		header->MAC3[3] = 0x00;
		header->MAC3[4] = 0x00;
		header->MAC3[5] = 0x03;
	}

	return 0;
}

#if defined(CONFIG_SPL_BUILD) || defined(CONFIG_NOR_BOOT)

static const struct ddr_data ddr3_baltos_data = {
	.datardsratio0 = MT41K256M16HA125E_RD_DQS,
	.datawdsratio0 = MT41K256M16HA125E_WR_DQS,
	.datafwsratio0 = MT41K256M16HA125E_PHY_FIFO_WE,
	.datawrsratio0 = MT41K256M16HA125E_PHY_WR_DATA,
};

static const struct cmd_control ddr3_baltos_cmd_ctrl_data = {
	.cmd0csratio = MT41K256M16HA125E_RATIO,
	.cmd0iclkout = MT41K256M16HA125E_INVERT_CLKOUT,

	.cmd1csratio = MT41K256M16HA125E_RATIO,
	.cmd1iclkout = MT41K256M16HA125E_INVERT_CLKOUT,

	.cmd2csratio = MT41K256M16HA125E_RATIO,
	.cmd2iclkout = MT41K256M16HA125E_INVERT_CLKOUT,
};

static struct emif_regs ddr3_baltos_emif_reg_data = {
	.sdram_config = MT41K256M16HA125E_EMIF_SDCFG,
	.ref_ctrl = MT41K256M16HA125E_EMIF_SDREF,
	.sdram_tim1 = MT41K256M16HA125E_EMIF_TIM1,
	.sdram_tim2 = MT41K256M16HA125E_EMIF_TIM2,
	.sdram_tim3 = MT41K256M16HA125E_EMIF_TIM3,
	.zq_config = MT41K256M16HA125E_ZQ_CFG,
	.emif_ddr_phy_ctlr_1 = MT41K256M16HA125E_EMIF_READ_LATENCY,
};

#ifdef CONFIG_SPL_OS_BOOT
int spl_start_uboot(void)
{
	/* break into full u-boot on 'c' */
	return (serial_tstc() && serial_getc() == 'c');
}
#endif

#define OSC	(V_OSCK/1000000)
const struct dpll_params dpll_ddr = {
		266, OSC-1, 1, -1, -1, -1, -1};
const struct dpll_params dpll_ddr_evm_sk = {
		303, OSC-1, 1, -1, -1, -1, -1};
const struct dpll_params dpll_ddr_baltos = {
		400, OSC-1, 1, -1, -1, -1, -1};

void am33xx_spl_board_init(void)
{
	int mpu_vdd;
	int sil_rev;

	/* Get the frequency */
	dpll_mpu_opp100.m = am335x_get_efuse_mpu_max_freq(cdev);

	/*
	 * The GP EVM, IDK and EVM SK use a TPS65910 PMIC.  For all
	 * MPU frequencies we support we use a CORE voltage of
	 * 1.1375V.  For MPU voltage we need to switch based on
	 * the frequency we are running at.
	 */
	i2c_set_bus_num(1);

	printf("I2C speed: %d Hz\n", CONFIG_SYS_OMAP24_I2C_SPEED);

	if (i2c_probe(TPS65910_CTRL_I2C_ADDR)) {
		puts("i2c: cannot access TPS65910\n");
		return;
	}

	/*
	 * Depending on MPU clock and PG we will need a different
	 * VDD to drive at that speed.
	 */
	sil_rev = readl(&cdev->deviceid) >> 28;
	mpu_vdd = am335x_get_tps65910_mpu_vdd(sil_rev,
					      dpll_mpu_opp100.m);

	/* Tell the TPS65910 to use i2c */
	tps65910_set_i2c_control();

	/* First update MPU voltage. */
	if (tps65910_voltage_update(MPU, mpu_vdd))
		return;

	/* Second, update the CORE voltage. */
	if (tps65910_voltage_update(CORE, TPS65910_OP_REG_SEL_1_1_3))
		return;

	/* Set CORE Frequencies to OPP100 */
	do_setup_dpll(&dpll_core_regs, &dpll_core_opp100);

	/* Set MPU Frequency to what we detected now that voltages are set */
	do_setup_dpll(&dpll_mpu_regs, &dpll_mpu_opp100);

	writel(0x000010ff, PRM_DEVICE_INST + 4);
}

const struct dpll_params *get_dpll_ddr_params(void)
{
	enable_i2c1_pin_mux();
	i2c_set_bus_num(1);

	return &dpll_ddr_baltos;
}

void set_uart_mux_conf(void)
{
	enable_uart0_pin_mux();
}

void set_mux_conf_regs(void)
{
	enable_board_pin_mux();
}

const struct ctrl_ioregs ioregs_baltos = {
	.cm0ioctl		= MT41K256M16HA125E_IOCTRL_VALUE,
	.cm1ioctl		= MT41K256M16HA125E_IOCTRL_VALUE,
	.cm2ioctl		= MT41K256M16HA125E_IOCTRL_VALUE,
	.dt0ioctl		= MT41K256M16HA125E_IOCTRL_VALUE,
	.dt1ioctl		= MT41K256M16HA125E_IOCTRL_VALUE,
};

void sdram_init(void)
{
	config_ddr(400, &ioregs_baltos,
		   &ddr3_baltos_data,
		   &ddr3_baltos_cmd_ctrl_data,
		   &ddr3_baltos_emif_reg_data, 0);
}
#endif

/*
 * Basic board specific setup.  Pinmux has been handled already.
 */
int board_init(void)
{
#if defined(CONFIG_HW_WATCHDOG)
	hw_watchdog_init();
#endif

	gd->bd->bi_boot_params = CONFIG_SYS_SDRAM_BASE + 0x100;
#if defined(CONFIG_NOR) || defined(CONFIG_NAND)
	gpmc_init();
#endif
	return 0;
}

int ft_board_setup(void *blob, bd_t *bd)
{
	int node, ret;
	unsigned char mac_addr[6];
	BSP_VS_HWPARAM header;

	/* get production data */
	if (read_eeprom(&header))
		return 0;

	/* setup MAC1 */
	mac_addr[0] = header.MAC1[0];
	mac_addr[1] = header.MAC1[1];
	mac_addr[2] = header.MAC1[2];
	mac_addr[3] = header.MAC1[3];
	mac_addr[4] = header.MAC1[4];
	mac_addr[5] = header.MAC1[5];


	node = fdt_path_offset(blob, "/ocp/ethernet/slave@4a100200");
	if (node < 0) {
		printf("no /soc/fman/ethernet path offset\n");
		return -ENODEV;
	}

	ret = fdt_setprop(blob, node, "mac-address", &mac_addr, 6);
	if (ret) {
		printf("error setting local-mac-address property\n");
		return -ENODEV;
	}

	/* setup MAC2 */
	mac_addr[0] = header.MAC2[0];
	mac_addr[1] = header.MAC2[1];
	mac_addr[2] = header.MAC2[2];
	mac_addr[3] = header.MAC2[3];
	mac_addr[4] = header.MAC2[4];
	mac_addr[5] = header.MAC2[5];

	node = fdt_path_offset(blob, "/ocp/ethernet/slave@4a100300");
	if (node < 0) {
		printf("no /soc/fman/ethernet path offset\n");
		return -ENODEV;
	}

	ret = fdt_setprop(blob, node, "mac-address", &mac_addr, 6);
	if (ret) {
		printf("error setting local-mac-address property\n");
		return -ENODEV;
	}

	printf("\nFDT was successfully setup\n");

	return 0;
}

static struct module_pin_mux pcie_sw_pin_mux[] = {
	{OFFSET(mii1_rxdv), (MODE(7) | PULLUDEN )},     /* GPIO3_4 */
	{-1},
};

static struct module_pin_mux dip_pin_mux[] = {
	{OFFSET(gpmc_ad12), (MODE(7) | RXACTIVE )},	/* GPIO1_12 */
	{OFFSET(gpmc_ad13), (MODE(7)  | RXACTIVE )},	/* GPIO1_13 */
	{OFFSET(gpmc_ad14), (MODE(7)  | RXACTIVE )},	/* GPIO1_14 */
	{OFFSET(gpmc_ad15), (MODE(7)  | RXACTIVE )},	/* GPIO1_15 */
	{-1},
};

#ifdef CONFIG_BOARD_LATE_INIT
int board_late_init(void)
{
#ifdef CONFIG_ENV_VARS_UBOOT_RUNTIME_CONFIG
	BSP_VS_HWPARAM header;
	char model[4];

	/* get production data */
	if (read_eeprom(&header)) {
		strcpy(model, "211");
	} else {
		sprintf(model, "%d", header.SystemId);
		if (header.SystemId == 215) {
			configure_module_pin_mux(dip_pin_mux);
			baltos_set_console();
		}
	}

	/* turn power for the mPCIe slot */
	configure_module_pin_mux(pcie_sw_pin_mux);
	if (gpio_request(MPCIE_SW, "mpcie_sw")) {
		printf("failed to export GPIO %d\n", MPCIE_SW);
		return -ENODEV;
	}
	if (gpio_direction_output(MPCIE_SW, 1)) {
		printf("failed to set GPIO %d direction\n", MPCIE_SW);
		return -ENODEV;
	}

	env_set("board_name", model);
#endif

	return 0;
}
#endif

#if (defined(CONFIG_DRIVER_TI_CPSW) && !defined(CONFIG_SPL_BUILD)) || \
	(defined(CONFIG_SPL_ETH_SUPPORT) && defined(CONFIG_SPL_BUILD))
static void cpsw_control(int enabled)
{
	/* VTP can be added here */

	return;
}

static struct cpsw_slave_data cpsw_slaves[] = {
	{
		.slave_reg_ofs	= 0x208,
		.sliver_reg_ofs	= 0xd80,
		.phy_addr	= 0,
	},
	{
		.slave_reg_ofs	= 0x308,
		.sliver_reg_ofs	= 0xdc0,
		.phy_addr	= 7,
	},
};

static struct cpsw_platform_data cpsw_data = {
	.mdio_base		= CPSW_MDIO_BASE,
	.cpsw_base		= CPSW_BASE,
	.mdio_div		= 0xff,
	.channels		= 8,
	.cpdma_reg_ofs		= 0x800,
	.slaves			= 2,
	.slave_data		= cpsw_slaves,
	.active_slave		= 1,
	.ale_reg_ofs		= 0xd00,
	.ale_entries		= 1024,
	.host_port_reg_ofs	= 0x108,
	.hw_stats_reg_ofs	= 0x900,
	.bd_ram_ofs		= 0x2000,
	.mac_control		= (1 << 5),
	.control		= cpsw_control,
	.host_port_num		= 0,
	.version		= CPSW_CTRL_VERSION_2,
};
#endif

#if ((defined(CONFIG_SPL_ETH_SUPPORT) || defined(CONFIG_SPL_USB_ETHER)) \
		&& defined(CONFIG_SPL_BUILD)) || \
	((defined(CONFIG_DRIVER_TI_CPSW) || \
	  defined(CONFIG_USB_ETHER) && defined(CONFIG_USB_MUSB_GADGET)) && \
	 !defined(CONFIG_SPL_BUILD))
int board_eth_init(bd_t *bis)
{
	int rv, n = 0;
	uint8_t mac_addr[6];
	uint32_t mac_hi, mac_lo;

	/*
	 * Note here that we're using CPSW1 since that has a 1Gbit PHY while
	 * CSPW0 has a 100Mbit PHY.
	 *
	 * On product, CPSW1 maps to port labeled WAN.
	 */

	/* try reading mac address from efuse */
	mac_lo = readl(&cdev->macid1l);
	mac_hi = readl(&cdev->macid1h);
	mac_addr[0] = mac_hi & 0xFF;
	mac_addr[1] = (mac_hi & 0xFF00) >> 8;
	mac_addr[2] = (mac_hi & 0xFF0000) >> 16;
	mac_addr[3] = (mac_hi & 0xFF000000) >> 24;
	mac_addr[4] = mac_lo & 0xFF;
	mac_addr[5] = (mac_lo & 0xFF00) >> 8;

#if (defined(CONFIG_DRIVER_TI_CPSW) && !defined(CONFIG_SPL_BUILD)) || \
	(defined(CONFIG_SPL_ETH_SUPPORT) && defined(CONFIG_SPL_BUILD))
	if (!env_get("ethaddr")) {
		printf("<ethaddr> not set. Validating first E-fuse MAC\n");

		if (is_valid_ethaddr(mac_addr))
			eth_env_set_enetaddr("ethaddr", mac_addr);
	}

#ifdef CONFIG_DRIVER_TI_CPSW
	writel((GMII1_SEL_RMII | GMII2_SEL_RGMII | RGMII2_IDMODE), &cdev->miisel);
	cpsw_slaves[1].phy_if = PHY_INTERFACE_MODE_RGMII;
	rv = cpsw_register(&cpsw_data);
	if (rv < 0)
		printf("Error %d registering CPSW switch\n", rv);
	else
		n += rv;
#endif

	/*
	 *
	 * CPSW RGMII Internal Delay Mode is not supported in all PVT
	 * operating points.  So we must set the TX clock delay feature
	 * in the AR8051 PHY.  Since we only support a single ethernet
	 * device in U-Boot, we only do this for the first instance.
	 */
#define AR8051_PHY_DEBUG_ADDR_REG	0x1d
#define AR8051_PHY_DEBUG_DATA_REG	0x1e
#define AR8051_DEBUG_RGMII_CLK_DLY_REG	0x5
#define AR8051_RGMII_TX_CLK_DLY		0x100
	const char *devname;
	devname = miiphy_get_current_dev();

	miiphy_write(devname, 0x7, AR8051_PHY_DEBUG_ADDR_REG,
			AR8051_DEBUG_RGMII_CLK_DLY_REG);
	miiphy_write(devname, 0x7, AR8051_PHY_DEBUG_DATA_REG,
			AR8051_RGMII_TX_CLK_DLY);
#endif
	return n;
}
#endif

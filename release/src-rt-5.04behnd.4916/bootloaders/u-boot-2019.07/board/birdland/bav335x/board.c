// SPDX-License-Identifier: GPL-2.0+
/*
 * board.c
 *
 * Board functions for Birdland Audio BAV335x Network Processor
 *
 * Copyright (c) 2012-2014 Birdland Audio - http://birdland.com/oem
 */

#include <common.h>
#include <errno.h>
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
#include <asm/io.h>
#include <asm/emif.h>
#include <asm/gpio.h>
#include <i2c.h>
#include <miiphy.h>
#include <cpsw.h>
#include <power/tps65217.h>
#include <power/tps65910.h>
#include <environment.h>
#include <watchdog.h>
#include <environment.h>
#include "board.h"

DECLARE_GLOBAL_DATA_PTR;

/* GPIO that controls power to DDR on EVM-SK */
#define GPIO_DDR_VTT_EN		7

static __maybe_unused struct ctrl_dev *cdev =
		(struct ctrl_dev *)CTRL_DEVICE_BASE;



/*
 * Read header information from EEPROM into global structure.
 */
static int read_eeprom(struct board_eeconfig *header)
{
	/* Check if baseboard eeprom is available */
	if (i2c_probe(CONFIG_SYS_I2C_EEPROM_ADDR))
		return -ENODEV;

	/* read the eeprom using i2c */
	if (i2c_read(CONFIG_SYS_I2C_EEPROM_ADDR, 0, 2, (uchar *)header,
		     sizeof(struct board_eeconfig)))
		return -EIO;

	if (header->magic != BOARD_MAGIC) {
		/* read the i2c eeprom again using only a 1 byte address */
		if (i2c_read(CONFIG_SYS_I2C_EEPROM_ADDR, 0, 1, (uchar *)header,
			     sizeof(struct board_eeconfig)))
			return -EIO;

		if (header->magic != BOARD_MAGIC)
			return -EINVAL;
	}
	return 0;
}




enum board_type get_board_type(bool debug)
{
	int ecode;
	struct board_eeconfig header;

	ecode = read_eeprom(&header);
	if (ecode == 0) {
		if (header.version[1] == 'A') {
			if (debug)
				puts("=== Detected Board model BAV335x Rev.A");
			return BAV335A;
		} else if (header.version[1] == 'B') {
			if (debug)
				puts("=== Detected Board model BAV335x Rev.B");
			return BAV335B;
		} else if (debug) {
			puts("### Un-known board model in serial-EE\n");
		}
	} else if (debug) {
		switch (ecode) {
		case -ENODEV:
			puts("### Board doesn't have a serial-EE\n");
			break;
		case -EINVAL:
			puts("### Board serial-EE signature is incorrect.\n");
			break;
		default:
			puts("### IO Error reading serial-EE.\n");
			break;
		}
	}

#if (CONFIG_BAV_VERSION == 1)
	if (debug)
		puts("### Selecting BAV335A as per config\n");
	return BAV335A;
#elif (CONFIG_BAV_VERSION == 2)
	if (debug)
		puts("### Selecting BAV335B as per config\n");
	return BAV335B;
#endif
#if (NOT_DEFINED == 2)
#error "SHOULD NEVER DISPLAY THIS"
#endif

	if (debug)
		puts("### Defaulting to model BAV335x Rev.B\n");
	return BAV335B;
}



#ifndef CONFIG_SKIP_LOWLEVEL_INIT
static const struct ddr_data ddr3_bav335x_data = {
	.datardsratio0 = MT41K256M16HA125E_RD_DQS,
	.datawdsratio0 = MT41K256M16HA125E_WR_DQS,
	.datafwsratio0 = MT41K256M16HA125E_PHY_FIFO_WE,
	.datawrsratio0 = MT41K256M16HA125E_PHY_WR_DATA,
};

static const struct cmd_control ddr3_bav335x_cmd_ctrl_data = {
	.cmd0csratio = MT41K256M16HA125E_RATIO,
	.cmd0iclkout = MT41K256M16HA125E_INVERT_CLKOUT,
	.cmd1csratio = MT41K256M16HA125E_RATIO,
	.cmd1iclkout = MT41K256M16HA125E_INVERT_CLKOUT,
	.cmd2csratio = MT41K256M16HA125E_RATIO,
	.cmd2iclkout = MT41K256M16HA125E_INVERT_CLKOUT,
};


static struct emif_regs ddr3_bav335x_emif_reg_data = {
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
	if (serial_tstc() && serial_getc() == 'c')
		return 1;

#ifdef CONFIG_SPL_ENV_SUPPORT
	env_init();
	env_load();
	if (env_get_yesno("boot_os") != 1)
		return 1;
#endif

	return 0;
}
#endif

#define OSC	(V_OSCK/1000000)
const struct dpll_params dpll_ddr = {
		266, OSC-1, 1, -1, -1, -1, -1};
const struct dpll_params dpll_ddr_evm_sk = {
		303, OSC-1, 1, -1, -1, -1, -1};
const struct dpll_params dpll_ddr_bone_black = {
		400, OSC-1, 1, -1, -1, -1, -1};

void am33xx_spl_board_init(void)
{
	/* debug print detect status */
	(void)get_board_type(true);

	/* Get the frequency */
	/* dpll_mpu_opp100.m = am335x_get_efuse_mpu_max_freq(cdev); */
	dpll_mpu_opp100.m = MPUPLL_M_1000;

	if (i2c_probe(TPS65217_CHIP_PM))
		return;

	/* Set the USB Current Limit */
	if (tps65217_reg_write(TPS65217_PROT_LEVEL_NONE, TPS65217_POWER_PATH,
			       TPS65217_USB_INPUT_CUR_LIMIT_1800MA,
			       TPS65217_USB_INPUT_CUR_LIMIT_MASK))
		puts("! tps65217_reg_write: could not set USB limit\n");

	/* Set the Core Voltage (DCDC3) to 1.125V */
	if (tps65217_voltage_update(TPS65217_DEFDCDC3,
				    TPS65217_DCDC_VOLT_SEL_1125MV)) {
		puts("! tps65217_reg_write: could not set Core Voltage\n");
		return;
	}

	/* Set CORE Frequencies to OPP100 */
	do_setup_dpll(&dpll_core_regs, &dpll_core_opp100);

	/* Set the MPU Voltage (DCDC2) */
	if (tps65217_voltage_update(TPS65217_DEFDCDC2,
				    TPS65217_DCDC_VOLT_SEL_1325MV)) {
		puts("! tps65217_reg_write: could not set MPU Voltage\n");
		return;
	}

	/*
	 * Set LDO3, LDO4 output voltage to 3.3V for Beaglebone.
	 * Set LDO3 to 1.8V and LDO4 to 3.3V for Beaglebone Black.
	 */
	if (tps65217_reg_write(TPS65217_PROT_LEVEL_2, TPS65217_DEFLS1,
			       TPS65217_LDO_VOLTAGE_OUT_1_8, TPS65217_LDO_MASK))
		puts("! tps65217_reg_write: could not set LDO3\n");

	if (tps65217_reg_write(TPS65217_PROT_LEVEL_2, TPS65217_DEFLS2,
			       TPS65217_LDO_VOLTAGE_OUT_3_3, TPS65217_LDO_MASK))
		puts("! tps65217_reg_write: could not set LDO4\n");

	/* Set MPU Frequency to what we detected now that voltages are set */
	do_setup_dpll(&dpll_mpu_regs, &dpll_mpu_opp100);
}

const struct dpll_params *get_dpll_ddr_params(void)
{
	enable_i2c0_pin_mux();
	i2c_init(CONFIG_SYS_OMAP24_I2C_SPEED, CONFIG_SYS_OMAP24_I2C_SLAVE);

	return &dpll_ddr_bone_black;
}

void set_uart_mux_conf(void)
{
#if CONFIG_CONS_INDEX == 1
	enable_uart0_pin_mux();
#elif CONFIG_CONS_INDEX == 2
	enable_uart1_pin_mux();
#elif CONFIG_CONS_INDEX == 3
	enable_uart2_pin_mux();
#elif CONFIG_CONS_INDEX == 4
	enable_uart3_pin_mux();
#elif CONFIG_CONS_INDEX == 5
	enable_uart4_pin_mux();
#elif CONFIG_CONS_INDEX == 6
	enable_uart5_pin_mux();
#endif
}

void set_mux_conf_regs(void)
{
	enum board_type board;

	board = get_board_type(false);
	enable_board_pin_mux(board);
}

const struct ctrl_ioregs ioregs_bonelt = {
	.cm0ioctl		= MT41K256M16HA125E_IOCTRL_VALUE,
	.cm1ioctl		= MT41K256M16HA125E_IOCTRL_VALUE,
	.cm2ioctl		= MT41K256M16HA125E_IOCTRL_VALUE,
	.dt0ioctl		= MT41K256M16HA125E_IOCTRL_VALUE,
	.dt1ioctl		= MT41K256M16HA125E_IOCTRL_VALUE,
};


void sdram_init(void)
{
	config_ddr(400, &ioregs_bonelt,
		   &ddr3_bav335x_data,
		   &ddr3_bav335x_cmd_ctrl_data,
		   &ddr3_bav335x_emif_reg_data, 0);
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

#ifdef CONFIG_BOARD_LATE_INIT
int board_late_init(void)
{
#ifdef CONFIG_ENV_VARS_UBOOT_RUNTIME_CONFIG
	env_set("board_name", "BAV335xB");
	env_set("board_rev", "B"); /* Fix me, but why bother.. */
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
		.phy_addr	= 1,
	},
};

static struct cpsw_platform_data cpsw_data = {
	.mdio_base		= CPSW_MDIO_BASE,
	.cpsw_base		= CPSW_BASE,
	.mdio_div		= 0xff,
	.channels		= 8,
	.cpdma_reg_ofs	= 0x800,
	.slaves			= 1,
	.slave_data		= cpsw_slaves,
	.ale_reg_ofs	= 0xd00,
	.ale_entries	= 1024,
	.host_port_reg_ofs	= 0x108,
	.hw_stats_reg_ofs	= 0x900,
	.bd_ram_ofs		= 0x2000,
	.mac_control	= (1 << 5),
	.control		= cpsw_control,
	.host_port_num	= 0,
	.version		= CPSW_CTRL_VERSION_2,
};
#endif


/*
 * This function will:
 * Perform fixups to the PHY present on certain boards.  We only need this
 * function in:
 * - SPL with either CPSW or USB ethernet support
 * - Full U-Boot, with either CPSW or USB ethernet
 * Build in only these cases to avoid warnings about unused variables
 * when we build an SPL that has neither option but full U-Boot will.
 */
#if ((defined(CONFIG_SPL_ETH_SUPPORT) || defined(CONFIG_SPL_USB_ETHER)) &&\
		defined(CONFIG_SPL_BUILD)) || \
	((defined(CONFIG_DRIVER_TI_CPSW) || \
	  defined(CONFIG_USB_ETHER) && defined(CONFIG_USB_MUSB_GADGET)) && \
	 !defined(CONFIG_SPL_BUILD))
int board_eth_init(bd_t *bis)
{
	int ecode, rv, n;
	uint8_t mac_addr[6];
	struct board_eeconfig header;
	__maybe_unused enum board_type board;

	/* Default manufacturing address; used when no EE or invalid */
	n = 0;
	mac_addr[0] = 0;
	mac_addr[1] = 0x20;
	mac_addr[2] = 0x18;
	mac_addr[3] = 0x1C;
	mac_addr[4] = 0x00;
	mac_addr[5] = 0x01;

	ecode = read_eeprom(&header);
	/* if we have a valid EE, get mac address from there */
	if ((ecode == 0) &&
	    is_valid_ethaddr((const u8 *)&header.mac_addr[0][0])) {
		memcpy(mac_addr, (const void *)&header.mac_addr[0][0], 6);
	}


#if (defined(CONFIG_DRIVER_TI_CPSW) && !defined(CONFIG_SPL_BUILD)) || \
	(defined(CONFIG_SPL_ETH_SUPPORT) && defined(CONFIG_SPL_BUILD))

	if (!env_get("ethaddr")) {
		printf("<ethaddr> not set. Validating first E-fuse MAC\n");

		if (is_valid_ethaddr(mac_addr))
			eth_env_set_enetaddr("ethaddr", mac_addr);
	}

#ifdef CONFIG_DRIVER_TI_CPSW

	board = get_board_type(false);

	/* Rev.A uses 10/100 PHY in mii mode */
	if (board == BAV335A) {
		writel(MII_MODE_ENABLE, &cdev->miisel);
		cpsw_slaves[0].phy_if = PHY_INTERFACE_MODE_MII;
		cpsw_slaves[1].phy_if = PHY_INTERFACE_MODE_MII;
	}
	/* Rev.B (default) uses GB PHY in rmii mode */
	else {
		writel((RGMII_MODE_ENABLE | RGMII_INT_DELAY), &cdev->miisel);
		cpsw_slaves[0].phy_if = cpsw_slaves[1].phy_if
				= PHY_INTERFACE_MODE_RGMII;
	}

	rv = cpsw_register(&cpsw_data);
	if (rv < 0)
		printf("Error %d registering CPSW switch\n", rv);
	else
		n += rv;
#endif

#endif

	return n;
}
#endif

// SPDX-License-Identifier: GPL-2.0+
/*
 * board.c
 *
 * Board functions for EETS PDU001 board
 *
 * Copyright (C) 2018, EETS GmbH, http://www.eets.ch/
 *
 * Copyright (C) 2011, Texas Instruments, Incorporated - http://www.ti.com/
 */

#include <common.h>
#include <errno.h>
#include <spl.h>
#include <i2c.h>
#include <environment.h>
#include <watchdog.h>
#include <debug_uart.h>
#include <dm/ofnode.h>
#include <power/pmic.h>
#include <power/regulator.h>
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
#include "board.h"

DECLARE_GLOBAL_DATA_PTR;

#define I2C_ADDR_NODE_ID	0x50
#define I2C_REG_NODE_ID_BASE	0xfa
#define NODE_ID_BYTE_COUNT	6

#define I2C_ADDR_LEDS		0x60
#define I2C_REG_RUN_LED		0x06
#define RUN_LED_OFF		0x0
#define RUN_LED_RED		0x1
#define RUN_LED_GREEN		(0x1 << 2)

#define VDD_MPU_REGULATOR	"regulator@2"
#define VDD_CORE_REGULATOR	"regulator@3"
#define DEFAULT_CORE_VOLTAGE	1137500

/*
 *  boot device save register
 * -------------------------
 * The boot device can be quired by 'spl_boot_device()' in
 * 'am33xx_spl_board_init'. However it can't be saved in the u-boot
 * environment here. In turn 'spl_boot_device' can't be called in
 * 'board_late_init' which allows writing to u-boot environment.
 * To get the boot device from 'am33xx_spl_board_init' to
 * 'board_late_init' we therefore use a scratch register from the RTC.
 */
#define CONFIG_SYS_RTC_SCRATCH0 0x60
#define BOOT_DEVICE_SAVE_REGISTER (RTC_BASE + CONFIG_SYS_RTC_SCRATCH0)

#ifdef CONFIG_SPL_BUILD
static void save_boot_device(void)
{
	*((u32 *)(BOOT_DEVICE_SAVE_REGISTER)) = spl_boot_device();
}
#endif

u32 boot_device(void)
{
	return *((u32 *)(BOOT_DEVICE_SAVE_REGISTER));
}

/* Store the boot device in the environment variable 'boot_device' */
static void env_set_boot_device(void)
{
	switch (boot_device()) {
		case BOOT_DEVICE_MMC1: {
			env_set("boot_device", "emmc");
			break;
		}
		case BOOT_DEVICE_MMC2: {
			env_set("boot_device", "sdcard");
			break;
		}
		default: {
			env_set("boot_device", "unknown");
			break;
		}
	}
}

static void set_run_led(struct udevice *dev)
{
	int val = RUN_LED_OFF;

	if (IS_ENABLED(CONFIG_RUN_LED_RED))
		val = RUN_LED_RED;
	else if (IS_ENABLED(CONFIG_RUN_LED_GREEN))
		val = RUN_LED_GREEN;

	dm_i2c_reg_write(dev, I2C_REG_RUN_LED, val);
}

/* Set 'serial#' to the EUI-48 value of board node ID chip */
static void env_set_serial(struct udevice *dev)
{
	int val;
	char serial[2 * NODE_ID_BYTE_COUNT + 1];
	int n;

	for (n = 0; n < sizeof(serial); n += 2) {
		val = dm_i2c_reg_read(dev, I2C_REG_NODE_ID_BASE + n / 2);
		sprintf(serial + n, "%02X", val);
	}
	serial[2 * NODE_ID_BYTE_COUNT] = '\0';
	env_set("serial#", serial);
}

static void set_mpu_and_core_voltage(void)
{
	int mpu_vdd;
	int sil_rev;
	struct udevice *dev;
	struct ctrl_dev *cdev = (struct ctrl_dev *)CTRL_DEVICE_BASE;

	/*
	 * The PDU001 (more precisely the computing module m2) uses a
	 * TPS65910 PMIC.  For all MPU frequencies we support we use a CORE
	 * voltage of 1.1375V.  For MPU voltage we need to switch based on
	 * the frequency we are running at.
	 */

	/*
	 * Depending on MPU clock and PG we will need a different VDD
	 * to drive at that speed.
	 */
	sil_rev = readl(&cdev->deviceid) >> 28;
	mpu_vdd = am335x_get_mpu_vdd(sil_rev, dpll_mpu_opp100.m);

	/* first update the MPU voltage */
	if (!regulator_get_by_devname(VDD_MPU_REGULATOR, &dev)) {
		if (regulator_set_value(dev, mpu_vdd))
			debug("failed to set MPU voltage\n");
	} else {
		debug("invalid MPU voltage ragulator %s\n", VDD_MPU_REGULATOR);
	}

	/* second update the CORE voltage */
	if (!regulator_get_by_devname(VDD_CORE_REGULATOR, &dev)) {
		if (regulator_set_value(dev, DEFAULT_CORE_VOLTAGE))
			debug("failed to set CORE voltage\n");
	} else {
		debug("invalid CORE voltage ragulator %s\n",
		      VDD_CORE_REGULATOR);
	}
}

#ifndef CONFIG_SKIP_LOWLEVEL_INIT
static const struct ddr_data ddr2_data = {
	.datardsratio0 = MT47H128M16RT25E_RD_DQS,
	.datafwsratio0 = MT47H128M16RT25E_PHY_FIFO_WE,
	.datawrsratio0 = MT47H128M16RT25E_PHY_WR_DATA,
};

static const struct cmd_control ddr2_cmd_ctrl_data = {
	.cmd0csratio = MT47H128M16RT25E_RATIO,
	.cmd1csratio = MT47H128M16RT25E_RATIO,
	.cmd2csratio = MT47H128M16RT25E_RATIO,
};

static const struct emif_regs ddr2_emif_reg_data = {
	.sdram_config = MT47H128M16RT25E_EMIF_SDCFG,
	.ref_ctrl = MT47H128M16RT25E_EMIF_SDREF,
	.sdram_tim1 = MT47H128M16RT25E_EMIF_TIM1,
	.sdram_tim2 = MT47H128M16RT25E_EMIF_TIM2,
	.sdram_tim3 = MT47H128M16RT25E_EMIF_TIM3,
	.emif_ddr_phy_ctlr_1 = MT47H128M16RT25E_EMIF_READ_LATENCY,
};

#define OSC	(V_OSCK / 1000000)
const struct dpll_params dpll_ddr = {
		266, OSC - 1, 1, -1, -1, -1, -1};
const struct dpll_params dpll_ddr_evm_sk = {
		303, OSC - 1, 1, -1, -1, -1, -1};
const struct dpll_params dpll_ddr_bone_black = {
		400, OSC - 1, 1, -1, -1, -1, -1};

void am33xx_spl_board_init(void)
{
	struct ctrl_dev *cdev = (struct ctrl_dev *)CTRL_DEVICE_BASE;

	/* Get the frequency */
	dpll_mpu_opp100.m = am335x_get_efuse_mpu_max_freq(cdev);

	/* Set CORE Frequencies to OPP100 */
	do_setup_dpll(&dpll_core_regs, &dpll_core_opp100);

	/* Set MPU Frequency to what we detected now that voltages are set */
	do_setup_dpll(&dpll_mpu_regs, &dpll_mpu_opp100);

	/* save boot device for later use by 'board_late_init' */
	save_boot_device();
}

const struct dpll_params *get_dpll_ddr_params(void)
{
	enable_i2c0_pin_mux();

	return &dpll_ddr;
}

void set_mux_conf_regs(void)
{
	/* done first by the ROM and afterwards by the pin controller driver */
	enable_i2c0_pin_mux();
}

const struct ctrl_ioregs ioregs = {
	.cm0ioctl		= MT47H128M16RT25E_IOCTRL_VALUE,
	.cm1ioctl		= MT47H128M16RT25E_IOCTRL_VALUE,
	.cm2ioctl		= MT47H128M16RT25E_IOCTRL_VALUE,
	.dt0ioctl		= MT47H128M16RT25E_IOCTRL_VALUE,
	.dt1ioctl		= MT47H128M16RT25E_IOCTRL_VALUE,
};

void sdram_init(void)
{
	config_ddr(266, &ioregs, &ddr2_data,
		   &ddr2_cmd_ctrl_data, &ddr2_emif_reg_data, 0);
}
#endif /* CONFIG_SKIP_LOWLEVEL_INIT */

#ifdef CONFIG_DEBUG_UART
void board_debug_uart_init(void)
{
	/* done by pin controller driver if not debugging */
	enable_uart_pin_mux(CONFIG_DEBUG_UART_BASE);
}
#endif

/*
 * Basic board specific setup.  Pinmux has been handled already.
 */
int board_init(void)
{
#ifdef CONFIG_HW_WATCHDOG
	hw_watchdog_init();
#endif

	gd->bd->bi_boot_params = CONFIG_SYS_SDRAM_BASE + 0x100;
	return 0;
}

#ifdef CONFIG_BOARD_LATE_INIT
int board_late_init(void)
{
	struct udevice *dev;

	set_mpu_and_core_voltage();
	env_set_boot_device();

	/* second I2C bus connects to node ID and front panel LED chip */
	if (!i2c_get_chip_for_busnum(1, I2C_ADDR_LEDS, 1, &dev))
		set_run_led(dev);
	if (!i2c_get_chip_for_busnum(1, I2C_ADDR_NODE_ID, 1, &dev))
		env_set_serial(dev);

	return 0;
}
#endif

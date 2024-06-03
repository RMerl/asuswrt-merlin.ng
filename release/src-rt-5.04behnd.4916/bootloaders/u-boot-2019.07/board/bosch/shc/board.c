// SPDX-License-Identifier: GPL-2.0+
/*
 * board.c
 *
 * (C) Copyright 2016
 * Heiko Schocher, DENX Software Engineering, hs@denx.de.
 *
 * Based on:
 * Board functions for TI AM335X based boards
 *
 * Copyright (C) 2011, Texas Instruments, Incorporated - http://www.ti.com/
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
#include <environment.h>
#include <watchdog.h>
#include <environment.h>
#include "mmc.h"
#include "board.h"

DECLARE_GLOBAL_DATA_PTR;

static struct shc_eeprom __attribute__((section(".data"))) header;
static int shc_eeprom_valid;

/*
 * Read header information from EEPROM into global structure.
 */
static int read_eeprom(void)
{
	/* Check if baseboard eeprom is available */
	if (i2c_probe(CONFIG_SYS_I2C_EEPROM_ADDR)) {
		puts("Could not probe the EEPROM; something fundamentally wrong on the I2C bus.\n");
		return -ENODEV;
	}

	/* read the eeprom using i2c */
	if (i2c_read(CONFIG_SYS_I2C_EEPROM_ADDR, 0, 2, (uchar *)&header,
		     sizeof(header))) {
		puts("Could not read the EEPROM; something fundamentally wrong on the I2C bus.\n");
		return -EIO;
	}

	if (header.magic != HDR_MAGIC) {
		printf("Incorrect magic number (0x%x) in EEPROM\n",
		       header.magic);
		return -EIO;
	}

	shc_eeprom_valid = 1;

	return 0;
}

static void shc_request_gpio(void)
{
	gpio_request(LED_PWR_BL_GPIO, "LED PWR BL");
	gpio_request(LED_PWR_RD_GPIO, "LED PWR RD");
	gpio_request(RESET_GPIO, "reset");
	gpio_request(WIFI_REGEN_GPIO, "WIFI REGEN");
	gpio_request(WIFI_RST_GPIO, "WIFI rst");
	gpio_request(ZIGBEE_RST_GPIO, "ZigBee rst");
	gpio_request(BIDCOS_RST_GPIO, "BIDCOS rst");
	gpio_request(ENOC_RST_GPIO, "ENOC rst");
#if defined CONFIG_B_SAMPLE
	gpio_request(LED_PWR_GN_GPIO, "LED PWR GN");
	gpio_request(LED_CONN_BL_GPIO, "LED CONN BL");
	gpio_request(LED_CONN_RD_GPIO, "LED CONN RD");
	gpio_request(LED_CONN_GN_GPIO, "LED CONN GN");
#else
	gpio_request(LED_LAN_BL_GPIO, "LED LAN BL");
	gpio_request(LED_LAN_RD_GPIO, "LED LAN RD");
	gpio_request(LED_CLOUD_BL_GPIO, "LED CLOUD BL");
	gpio_request(LED_CLOUD_RD_GPIO, "LED CLOUD RD");
	gpio_request(LED_PWM_GPIO, "LED PWM");
	gpio_request(Z_WAVE_RST_GPIO, "Z WAVE rst");
#endif
	gpio_request(BACK_BUTTON_GPIO, "Back button");
	gpio_request(FRONT_BUTTON_GPIO, "Front button");
}

/*
 * Function which forces all installed modules into running state for ICT
 * testing. Called by SPL.
 */
static void __maybe_unused force_modules_running(void)
{
	/* Wi-Fi power regulator enable - high = enabled */
	gpio_direction_output(WIFI_REGEN_GPIO, 1);
	/*
	 * Wait for Wi-Fi power regulator to reach a stable voltage
	 * (soft-start time, max. 350 µs)
	 */
	__udelay(350);

	/* Wi-Fi module reset - high = running */
	gpio_direction_output(WIFI_RST_GPIO, 1);

	/* ZigBee reset - high = running */
	gpio_direction_output(ZIGBEE_RST_GPIO, 1);

	/* BidCos reset - high = running */
	gpio_direction_output(BIDCOS_RST_GPIO, 1);

#if !defined(CONFIG_B_SAMPLE)
	/* Z-Wave reset - high = running */
	gpio_direction_output(Z_WAVE_RST_GPIO, 1);
#endif

	/* EnOcean reset - low = running */
	gpio_direction_output(ENOC_RST_GPIO, 0);
}

/*
 * Function which forces all installed modules into reset - to be released by
 * the OS, called by SPL
 */
static void __maybe_unused force_modules_reset(void)
{
	/* Wi-Fi module reset - low = reset */
	gpio_direction_output(WIFI_RST_GPIO, 0);

	/* Wi-Fi power regulator enable - low = disabled */
	gpio_direction_output(WIFI_REGEN_GPIO, 0);

	/* ZigBee reset - low = reset */
	gpio_direction_output(ZIGBEE_RST_GPIO, 0);

	/* BidCos reset - low = reset */
	/*gpio_direction_output(BIDCOS_RST_GPIO, 0);*/

#if !defined(CONFIG_B_SAMPLE)
	/* Z-Wave reset - low = reset */
	gpio_direction_output(Z_WAVE_RST_GPIO, 0);
#endif

	/* EnOcean reset - high = reset*/
	gpio_direction_output(ENOC_RST_GPIO, 1);
}

/*
 * Function to set the LEDs in the state "Bootloader booting"
 */
static void __maybe_unused leds_set_booting(void)
{
#if defined(CONFIG_B_SAMPLE)

	/* Turn all red LEDs on */
	gpio_direction_output(LED_PWR_RD_GPIO, 1);
	gpio_direction_output(LED_CONN_RD_GPIO, 1);

#else /* All other SHCs starting with B2-Sample */
	/* Set the PWM GPIO */
	gpio_direction_output(LED_PWM_GPIO, 1);
	/* Turn all red LEDs on */
	gpio_direction_output(LED_PWR_RD_GPIO, 1);
	gpio_direction_output(LED_LAN_RD_GPIO, 1);
	gpio_direction_output(LED_CLOUD_RD_GPIO, 1);

#endif
}

/*
 * Function to set the LEDs in the state "Bootloader error"
 */
static void leds_set_failure(int state)
{
#if defined(CONFIG_B_SAMPLE)
	/* Turn all blue and green LEDs off */
	gpio_set_value(LED_PWR_BL_GPIO, 0);
	gpio_set_value(LED_PWR_GN_GPIO, 0);
	gpio_set_value(LED_CONN_BL_GPIO, 0);
	gpio_set_value(LED_CONN_GN_GPIO, 0);

	/* Turn all red LEDs to 'state' */
	gpio_set_value(LED_PWR_RD_GPIO, state);
	gpio_set_value(LED_CONN_RD_GPIO, state);

#else /* All other SHCs starting with B2-Sample */
	/* Set the PWM GPIO */
	gpio_direction_output(LED_PWM_GPIO, 1);

	/* Turn all blue LEDs off */
	gpio_set_value(LED_PWR_BL_GPIO, 0);
	gpio_set_value(LED_LAN_BL_GPIO, 0);
	gpio_set_value(LED_CLOUD_BL_GPIO, 0);

	/* Turn all red LEDs to 'state' */
	gpio_set_value(LED_PWR_RD_GPIO, state);
	gpio_set_value(LED_LAN_RD_GPIO, state);
	gpio_set_value(LED_CLOUD_RD_GPIO, state);
#endif
}

/*
 * Function to set the LEDs in the state "Bootloader finished"
 */
static void leds_set_finish(void)
{
#if defined(CONFIG_B_SAMPLE)
	/* Turn all LEDs off */
	gpio_set_value(LED_PWR_BL_GPIO, 0);
	gpio_set_value(LED_PWR_RD_GPIO, 0);
	gpio_set_value(LED_PWR_GN_GPIO, 0);
	gpio_set_value(LED_CONN_BL_GPIO, 0);
	gpio_set_value(LED_CONN_RD_GPIO, 0);
	gpio_set_value(LED_CONN_GN_GPIO, 0);
#else /* All other SHCs starting with B2-Sample */
	/* Turn all LEDs off */
	gpio_set_value(LED_PWR_BL_GPIO, 0);
	gpio_set_value(LED_PWR_RD_GPIO, 0);
	gpio_set_value(LED_LAN_BL_GPIO, 0);
	gpio_set_value(LED_LAN_RD_GPIO, 0);
	gpio_set_value(LED_CLOUD_BL_GPIO, 0);
	gpio_set_value(LED_CLOUD_RD_GPIO, 0);

	/* Turn off the PWM GPIO and mux it to EHRPWM */
	gpio_set_value(LED_PWM_GPIO, 0);
	enable_shc_board_pwm_pin_mux();
#endif
}

static void check_button_status(void)
{
	ulong value;
	gpio_direction_input(FRONT_BUTTON_GPIO);
	value = gpio_get_value(FRONT_BUTTON_GPIO);

	if (value == 0) {
		printf("front button activated !\n");
		env_set("harakiri", "1");
	}
}

#if defined(CONFIG_SPL_BUILD)
#ifdef CONFIG_SPL_OS_BOOT
int spl_start_uboot(void)
{
	return 1;
}
#endif

static void shc_board_early_init(void)
{
	shc_request_gpio();
# ifdef CONFIG_SHC_ICT
	/* Force all modules into enabled state for ICT testing */
	force_modules_running();
# else
	/* Force all modules to enter Reset state until released by the OS */
	force_modules_reset();
# endif
	leds_set_booting();
}

static struct ctrl_dev *cdev = (struct ctrl_dev *)CTRL_DEVICE_BASE;

#define MPU_SPREADING_PERMILLE 18 /* Spread 1.8 percent */
#define OSC	(V_OSCK/1000000)
/* Bosch: Predivider must be fixed to 4, so N = 4-1 */
#define MPUPLL_N        (4-1)
/* Bosch: Fref = 24 MHz / (N+1) = 24 MHz / 4 = 6 MHz */
#define MPUPLL_FREF (OSC / (MPUPLL_N + 1))

const struct dpll_params dpll_ddr_shc = {
		400, OSC-1, 1, -1, -1, -1, -1};

const struct dpll_params *get_dpll_ddr_params(void)
{
	return &dpll_ddr_shc;
}

/*
 * As we enabled downspread SSC with 1.8%, the values needed to be corrected
 * such that the 20% overshoot will not lead to too high frequencies.
 * In all cases, this is achieved by subtracting one from M (6 MHz less).
 * Example: 600 MHz CPU
 *   Step size: 24 MHz OSC, N = 4 (fix) --> Fref = 6 MHz
 *   600 MHz - 6 MHz (1x Fref) = 594 MHz
 *   SSC: 594 MHz * 1.8% = 10.7 MHz SSC
 *   Overshoot: 10.7 MHz * 20 % = 2.2 MHz
 *   --> Fmax = 594 MHz + 2.2 MHz = 596.2 MHz, lower than 600 MHz --> OK!
 */
const struct dpll_params dpll_mpu_shc_opp100 = {
		99, MPUPLL_N, 1, -1, -1, -1, -1};

void am33xx_spl_board_init(void)
{
	int sil_rev;
	int mpu_vdd;

	puts(BOARD_ID_STR);

	/*
	 * Set CORE Frequency to OPP100
	 * Hint: DCDC3 (CORE) defaults to 1.100V (for OPP100)
	 */
	do_setup_dpll(&dpll_core_regs, &dpll_core_opp100);

	sil_rev = readl(&cdev->deviceid) >> 28;
	if (sil_rev < 2) {
		puts("We do not support Silicon Revisions below 2.0!\n");
		return;
	}

	dpll_mpu_opp100.m = am335x_get_efuse_mpu_max_freq(cdev);
	if (i2c_probe(TPS65217_CHIP_PM))
		return;

	/*
	 * Retrieve the CPU max frequency by reading the efuse
	 * SHC-Default: 600 MHz
	 */
	switch (dpll_mpu_opp100.m) {
	case MPUPLL_M_1000:
		mpu_vdd = TPS65217_DCDC_VOLT_SEL_1325MV;
		break;
	case MPUPLL_M_800:
		mpu_vdd = TPS65217_DCDC_VOLT_SEL_1275MV;
		break;
	case MPUPLL_M_720:
		mpu_vdd = TPS65217_DCDC_VOLT_SEL_1200MV;
		break;
	case MPUPLL_M_600:
		mpu_vdd = TPS65217_DCDC_VOLT_SEL_1100MV;
		break;
	case MPUPLL_M_300:
		mpu_vdd = TPS65217_DCDC_VOLT_SEL_950MV;
		break;
	default:
		puts("Cannot determine the frequency, failing!\n");
		return;
	}

	if (tps65217_voltage_update(TPS65217_DEFDCDC2, mpu_vdd)) {
		puts("tps65217_voltage_update failure\n");
		return;
	}

	/* Set MPU Frequency to what we detected */
	printf("MPU reference clock runs at %d MHz\n", MPUPLL_FREF);
	printf("Setting MPU clock to %d MHz\n", MPUPLL_FREF *
	       dpll_mpu_shc_opp100.m);
	do_setup_dpll(&dpll_mpu_regs, &dpll_mpu_shc_opp100);

	/* Enable Spread Spectrum for this freq to be clean on EMI side */
	set_mpu_spreadspectrum(MPU_SPREADING_PERMILLE);

	/*
	 * Using the default voltages for the PMIC (TPS65217D)
	 * LS1 = 1.8V (VDD_1V8)
	 * LS2 = 3.3V (VDD_3V3A)
	 * LDO1 = 1.8V (VIO and VRTC)
	 * LDO2 = 3.3V (VDD_3V3AUX)
	 */
	shc_board_early_init();
}

void set_uart_mux_conf(void)
{
	enable_uart0_pin_mux();
}

void set_mux_conf_regs(void)
{
	enable_shc_board_pin_mux();
}

const struct ctrl_ioregs ioregs_evmsk = {
	.cm0ioctl		= MT41K256M16HA125E_IOCTRL_VALUE,
	.cm1ioctl		= MT41K256M16HA125E_IOCTRL_VALUE,
	.cm2ioctl		= MT41K256M16HA125E_IOCTRL_VALUE,
	.dt0ioctl		= MT41K256M16HA125E_IOCTRL_VALUE,
	.dt1ioctl		= MT41K256M16HA125E_IOCTRL_VALUE,
};

static const struct ddr_data ddr3_shc_data = {
	.datardsratio0 = MT41K256M16HA125E_RD_DQS,
	.datawdsratio0 = MT41K256M16HA125E_WR_DQS,
	.datafwsratio0 = MT41K256M16HA125E_PHY_FIFO_WE,
	.datawrsratio0 = MT41K256M16HA125E_PHY_WR_DATA,
};

static const struct cmd_control ddr3_shc_cmd_ctrl_data = {
	.cmd0csratio = MT41K256M16HA125E_RATIO,
	.cmd0iclkout = MT41K256M16HA125E_INVERT_CLKOUT,

	.cmd1csratio = MT41K256M16HA125E_RATIO,
	.cmd1iclkout = MT41K256M16HA125E_INVERT_CLKOUT,

	.cmd2csratio = MT41K256M16HA125E_RATIO,
	.cmd2iclkout = MT41K256M16HA125E_INVERT_CLKOUT,
};

static struct emif_regs ddr3_shc_emif_reg_data = {
	.sdram_config = MT41K256M16HA125E_EMIF_SDCFG,
	.ref_ctrl = MT41K256M16HA125E_EMIF_SDREF,
	.sdram_tim1 = MT41K256M16HA125E_EMIF_TIM1,
	.sdram_tim2 = MT41K256M16HA125E_EMIF_TIM2,
	.sdram_tim3 = MT41K256M16HA125E_EMIF_TIM3,
	.zq_config = MT41K256M16HA125E_ZQ_CFG,
	.emif_ddr_phy_ctlr_1 = MT41K256M16HA125E_EMIF_READ_LATENCY |
				PHY_EN_DYN_PWRDN,
};

void sdram_init(void)
{
	/* Configure the DDR3 RAM */
	config_ddr(400, &ioregs_evmsk, &ddr3_shc_data,
		   &ddr3_shc_cmd_ctrl_data, &ddr3_shc_emif_reg_data, 0);
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
	i2c_init(CONFIG_SYS_I2C_SPEED, CONFIG_SYS_I2C_SLAVE);
	if (read_eeprom() < 0)
		puts("EEPROM Content Invalid.\n");

	gd->bd->bi_boot_params = CONFIG_SYS_SDRAM_BASE + 0x100;
#if defined(CONFIG_NOR) || defined(CONFIG_NAND)
	gpmc_init();
#endif
	shc_request_gpio();

	return 0;
}

#ifdef CONFIG_BOARD_LATE_INIT
int board_late_init(void)
{
	check_button_status();
#ifdef CONFIG_ENV_VARS_UBOOT_RUNTIME_CONFIG
	if (shc_eeprom_valid)
		if (is_valid_ethaddr(header.mac_addr))
			eth_env_set_enetaddr("ethaddr", header.mac_addr);
#endif

	return 0;
}
#endif

#if defined(CONFIG_USB_ETHER) && \
	(!defined(CONFIG_SPL_BUILD) || defined(CONFIG_SPL_USB_ETHER))
int board_eth_init(bd_t *bis)
{
	return usb_eth_initialize(bis);
}
#endif

#ifdef CONFIG_SHOW_BOOT_PROGRESS
static void bosch_check_reset_pin(void)
{
	if (readl(GPIO1_BASE + OMAP_GPIO_IRQSTATUS_SET_0) & RESET_MASK) {
		printf("Resetting ...\n");
		writel(RESET_MASK, GPIO1_BASE + OMAP_GPIO_IRQSTATUS_SET_0);
		disable_interrupts();
		reset_cpu(0);
		/*NOTREACHED*/
	}
}

static void hang_bosch(const char *cause, int code)
{
	int lv;

	gpio_direction_input(RESET_GPIO);

	/* Enable reset pin interrupt on falling edge */
	writel(RESET_MASK, GPIO1_BASE + OMAP_GPIO_IRQSTATUS_SET_0);
	writel(RESET_MASK, GPIO1_BASE + OMAP_GPIO_FALLINGDETECT);
	enable_interrupts();

	puts(cause);
	for (;;) {
		for (lv = 0; lv < code; lv++) {
			bosch_check_reset_pin();
			leds_set_failure(1);
			__udelay(150 * 1000);
			leds_set_failure(0);
			__udelay(150 * 1000);
		}
#if defined(BLINK_CODE)
		__udelay(300 * 1000);
#endif
	}
}

void show_boot_progress(int val)
{
	switch (val) {
	case BOOTSTAGE_ID_NEED_RESET:
		hang_bosch("need reset", 4);
		break;
	}
}

void arch_preboot_os(void)
{
	leds_set_finish();
}
#endif

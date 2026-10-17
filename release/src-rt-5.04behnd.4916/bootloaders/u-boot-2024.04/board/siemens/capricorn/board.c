// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright 2017-2019 NXP
 *
 * Copyright 2019 Siemens AG
 *
 */
#include <common.h>
#include <command.h>
#include <dm.h>
#include <env.h>
#include <errno.h>
#include <init.h>
#include <log.h>
#include <netdev.h>
#include <env_internal.h>
#include <fsl_esdhc_imx.h>
#include <i2c.h>
#include <led.h>
#include <pca953x.h>
#include <power-domain.h>
#include <asm/gpio.h>
#include <asm/arch/imx8-pins.h>
#include <asm/arch/iomux.h>
#include <asm/arch/sys_proto.h>
#ifndef CONFIG_SPL
#include <asm/arch-imx8/clock.h>
#endif
#include <linux/delay.h>
#include "../common/eeprom.h"
#include "../common/factoryset.h"

#define GPIO_PAD_CTRL \
		((SC_PAD_CONFIG_NORMAL << PADRING_CONFIG_SHIFT) | \
		 (SC_PAD_ISO_OFF << PADRING_LPCONFIG_SHIFT) | \
		 (SC_PAD_28FDSOI_DSE_DV_HIGH << PADRING_DSE_SHIFT) | \
		 (SC_PAD_28FDSOI_PS_PU << PADRING_PULL_SHIFT))

#define ENET_NORMAL_PAD_CTRL \
		((SC_PAD_CONFIG_NORMAL << PADRING_CONFIG_SHIFT) | \
		 (SC_PAD_ISO_OFF << PADRING_LPCONFIG_SHIFT) | \
		 (SC_PAD_28FDSOI_DSE_18V_10MA << PADRING_DSE_SHIFT) | \
		 (SC_PAD_28FDSOI_PS_PU << PADRING_PULL_SHIFT))

#define UART_PAD_CTRL \
		((SC_PAD_CONFIG_OUT_IN << PADRING_CONFIG_SHIFT) | \
		 (SC_PAD_ISO_OFF << PADRING_LPCONFIG_SHIFT) | \
		 (SC_PAD_28FDSOI_DSE_DV_HIGH << PADRING_DSE_SHIFT) | \
		 (SC_PAD_28FDSOI_PS_PU << PADRING_PULL_SHIFT))

static iomux_cfg_t uart2_pads[] = {
	SC_P_UART2_RX | MUX_PAD_CTRL(UART_PAD_CTRL),
	SC_P_UART2_TX | MUX_PAD_CTRL(UART_PAD_CTRL),
};

static void setup_iomux_uart(void)
{
	imx8_iomux_setup_multiple_pads(uart2_pads, ARRAY_SIZE(uart2_pads));
}

int board_early_init_f(void)
{
	/* Set UART clock root to 80 MHz */
	sc_pm_clock_rate_t rate = SC_80MHZ;
	int ret;

	ret = sc_pm_setup_uart(SC_R_UART_0, rate);
	ret |= sc_pm_setup_uart(SC_R_UART_2, rate);
	if (ret)
		return ret;

	setup_iomux_uart();

	return 0;
}

#define ENET_PHY_RESET	IMX_GPIO_NR(0, 3)
#define ENET_TEST_1	IMX_GPIO_NR(0, 8)
#define ENET_TEST_2	IMX_GPIO_NR(0, 9)

/*#define ETH_IO_TEST*/
static iomux_cfg_t enet_reset[] = {
	SC_P_ESAI0_SCKT | MUX_MODE_ALT(4) | MUX_PAD_CTRL(GPIO_PAD_CTRL),
#ifdef ETH_IO_TEST
	/* GPIO0.IO08 MODE3: TXD0 */
	SC_P_ESAI0_TX4_RX1 | MUX_MODE_ALT(4) |
	MUX_PAD_CTRL(ENET_NORMAL_PAD_CTRL),
	/* GPIO0.IO09 MODE3: TXD1 */
	SC_P_ESAI0_TX5_RX0 | MUX_MODE_ALT(4) |
	MUX_PAD_CTRL(ENET_NORMAL_PAD_CTRL),
#endif
};

static void enet_device_phy_reset(void)
{
	int ret = 0;

	imx8_iomux_setup_multiple_pads(enet_reset, ARRAY_SIZE(enet_reset));

	ret = gpio_request(ENET_PHY_RESET, "enet_phy_reset");
	if (!ret) {
		gpio_direction_output(ENET_PHY_RESET, 1);
		gpio_set_value(ENET_PHY_RESET, 0);
		/* SMSC9303 TRM chapter 14.5.2 */
		udelay(200);
		gpio_set_value(ENET_PHY_RESET, 1);
	} else {
		printf("ENET RESET failed!\n");
	}

#ifdef ETH_IO_TEST
	ret =  gpio_request(ENET_TEST_1, "enet_test1");
	if (!ret) {
		int i;

		printf("ENET TEST 1!\n");
		for (i = 0; i < 20; i++) {
			gpio_direction_output(ENET_TEST_1, 1);
			gpio_set_value(ENET_TEST_1, 0);
			udelay(50);
			gpio_set_value(ENET_TEST_1, 1);
			udelay(50);
		}
		gpio_free(ENET_TEST_1);
	} else {
		printf("GPIO for ENET TEST 1 failed!\n");
	}
	ret =  gpio_request(ENET_TEST_2, "enet_test2");
	if (!ret) {
		int i;

		printf("ENET TEST 2!\n");
		for (i = 0; i < 20; i++) {
			gpio_direction_output(ENET_TEST_2, 1);
			gpio_set_value(ENET_TEST_2, 0);
			udelay(50);
			gpio_set_value(ENET_TEST_2, 1);
			udelay(50);
		}
		gpio_free(ENET_TEST_2);
	} else {
		printf("GPIO for ENET TEST 2 failed!\n");
	}
#endif
}

int setup_gpr_fec(void)
{
	sc_ipc_t ipc_handle = -1;
	int err = 0;
	unsigned int test;

	/*
	 * TX_CLK_SEL: it controls a mux between clock coming from the pad 50M
	 * input pin and clock generated internally to connectivity subsystem
	 *	0: internal clock
	 *	1: external clock --->  your choice for RMII
	 *
	 * CLKDIV_SEL: it controls a div by 2 on the internal clock path à
	 *	it should be don’t care when using external clock
	 *	0: non-divided clock
	 *	1: clock divided by 2
	 * 50_DISABLE or 125_DISABLE:
	 *	it’s used to disable the clock tree going outside the chip
	 *	when reference clock is generated internally.
	 *	It should be don’t care when reference clock is provided
	 *	externally.
	 *	0: clock is enabled
	 *	1: clock is disabled
	 *
	 * SC_C_TXCLK		= 24,
	 * SC_C_CLKDIV		= 25,
	 * SC_C_DISABLE_50	= 26,
	 * SC_C_DISABLE_125	= 27,
	 */

	err = sc_misc_set_control(ipc_handle, SC_R_ENET_1, SC_C_TXCLK, 1);
	if (err)
		printf("Error in setting up SC_C %d\n\r", SC_C_TXCLK);

	sc_misc_get_control(ipc_handle, SC_R_ENET_1, SC_C_TXCLK, &test);
	debug("TEST SC_C %d-->%d\n\r", SC_C_TXCLK, test);

	err = sc_misc_set_control(ipc_handle, SC_R_ENET_1, SC_C_CLKDIV, 0);
	if (err)
		printf("Error in setting up SC_C %d\n\r", SC_C_CLKDIV);

	sc_misc_get_control(ipc_handle, SC_R_ENET_1, SC_C_CLKDIV, &test);
	debug("TEST SC_C %d-->%d\n\r", SC_C_CLKDIV, test);

	err = sc_misc_set_control(ipc_handle, SC_R_ENET_1, SC_C_DISABLE_50, 0);
	if (err)
		printf("Error in setting up SC_C %d\n\r", SC_C_DISABLE_50);

	sc_misc_get_control(ipc_handle, SC_R_ENET_1, SC_C_TXCLK, &test);
	debug("TEST SC_C %d-->%d\n\r", SC_C_DISABLE_50, test);

	err = sc_misc_set_control(ipc_handle, SC_R_ENET_1, SC_C_DISABLE_125, 1);
	if (err)
		printf("Error in setting up SC_C %d\n\r", SC_C_DISABLE_125);

	sc_misc_get_control(ipc_handle, SC_R_ENET_1, SC_C_TXCLK, &test);
	debug("TEST SC_C %d-->%d\n\r", SC_C_DISABLE_125, test);

	err = sc_misc_set_control(ipc_handle, SC_R_ENET_1, SC_C_SEL_125, 1);
	if (err)
		printf("Error in setting up SC_C %d\n\r", SC_C_SEL_125);

	sc_misc_get_control(ipc_handle, SC_R_ENET_1, SC_C_SEL_125, &test);
	debug("TEST SC_C %d-->%d\n\r", SC_C_SEL_125, test);

	return 0;
}

#if IS_ENABLED(CONFIG_FEC_MXC)
#include <miiphy.h>
int board_phy_config(struct phy_device *phydev)
{
	if (phydev->drv->config)
		phydev->drv->config(phydev);

	return 0;
}

#endif

static int setup_fec(void)
{
	setup_gpr_fec();
	/* Reset ENET PHY */
	enet_device_phy_reset();
	return 0;
}

void reset_cpu(void)
{
}

#ifndef CONFIG_SPL_BUILD
/* LED's */
static int board_led_init(void)
{
	struct udevice *bus, *dev;
	u8 pca_led[2] = { 0x00, 0x00 };
	int ret;

	/* enable all leds on PCA9552 */
	ret = uclass_get_device_by_seq(UCLASS_I2C, PCA9552_1_I2C_BUS, &bus);
	if (ret) {
		printf("ERROR: I2C get %d\n", ret);
		return ret;
	}

	ret = dm_i2c_probe(bus, PCA9552_1_I2C_ADDR, 0, &dev);
	if (ret) {
		printf("ERROR: PCA9552 probe failed\n");
		return ret;
	}

	ret = dm_i2c_write(dev, 0x16, pca_led, sizeof(pca_led));
	if (ret) {
		printf("ERROR: PCA9552 write failed\n");
		return ret;
	}

	mdelay(1);
	return ret;
}
#endif /* !CONFIG_SPL_BUILD */

int checkboard(void)
{
	puts("Board: Capricorn\n");

	/*
	 * Running build_info() doesn't work with current SCFW blob.
	 * Uncomment below call when new blob is available.
	 */
	/*build_info();*/

	print_bootinfo();
	return 0;
}

int board_init(void)
{
	setup_fec();
	return 0;
}

#ifdef CONFIG_OF_BOARD_SETUP
int ft_board_setup(void *blob, struct bd_info *bd)
{
	return 0;
}
#endif

int board_mmc_get_env_dev(int devno)
{
	return devno;
}

static int check_mmc_autodetect(void)
{
	char *autodetect_str = env_get("mmcautodetect");

	if (autodetect_str && (strcmp(autodetect_str, "yes") == 0))
		return 1;

	return 0;
}

/* This should be defined for each board */
__weak int mmc_map_to_kernel_blk(int dev_no)
{
	return dev_no;
}

void board_late_mmc_env_init(void)
{
	char cmd[32];
	char mmcblk[32];
	u32 dev_no = mmc_get_env_dev();

	if (!check_mmc_autodetect())
		return;

	env_set_ulong("mmcdev", dev_no);

	/* Set mmcblk env */
	sprintf(mmcblk, "/dev/mmcblk%dp2 rootwait rw",
		mmc_map_to_kernel_blk(dev_no));
	env_set("mmcroot", mmcblk);

	sprintf(cmd, "mmc dev %d", dev_no);
	run_command(cmd, 0);
}

#ifndef CONFIG_SPL_BUILD
static int load_parameters_from_factoryset(void)
{
	int ret;

	ret = factoryset_read_eeprom(SIEMENS_EE_I2C_ADDR);
	if (ret)
		return ret;

	return factoryset_env_set();
}

int board_late_init(void)
{
	env_set("sec_boot", "no");
#ifdef CONFIG_AHAB_BOOT
	env_set("sec_boot", "yes");
#endif

#ifdef CONFIG_ENV_IS_IN_MMC
	board_late_mmc_env_init();
#endif
	/* Init LEDs */
	if (board_led_init())
		printf("I2C LED init failed\n");

	/* Set environment from factoryset */
	if (load_parameters_from_factoryset())
		printf("Loading factoryset parameters failed!\n");

	return 0;
}

/* Service button */
#define MAX_PIN_NUMBER			128
#define BOARD_DEFAULT_BUTTON_GPIO	IMX_GPIO_NR(1, 31)

unsigned char get_button_state(char * const envname, unsigned char def)
{
	int button = 0;
	int gpio;
	char *ptr_env;

	/* If button is not found we take default */
	ptr_env = env_get(envname);
	if (!ptr_env) {
		printf("Using default: %u\n", def);
		gpio = def;
	} else {
		gpio = (unsigned char)simple_strtoul(ptr_env, NULL, 0);
		if (gpio > MAX_PIN_NUMBER)
			gpio = def;
	}

	gpio_request(gpio, "");
	gpio_direction_input(gpio);
	if (gpio_get_value(gpio))
		button = 1;
	else
		button = 0;

	gpio_free(gpio);

	return button;
}

/*
 * This command returns the status of the user button on
 * Input - none
 * Returns -	1 if button is held down
 *		0 if button is not held down
 */
static int
do_userbutton(struct cmd_tbl *cmdtp, int flag, int argc, char *const argv[])
{
	int button = 0;

	button = get_button_state("button_usr1", BOARD_DEFAULT_BUTTON_GPIO);

	if (argc > 1)
		printf("Button state: %u\n", button);

	return button;
}

U_BOOT_CMD(
	usrbutton, CONFIG_SYS_MAXARGS, 2, do_userbutton,
	"Return the status of user button",
	"[print]"
);

#define ERST	IMX_GPIO_NR(0, 3)

static int
do_eth_reset(struct cmd_tbl *cmdtp, int flag, int argc, char *const argv[])
{
	gpio_request(ERST, "ERST");
	gpio_direction_output(ERST, 0);
	udelay(200);
	gpio_set_value(ERST, 1);
	return 0;
}

U_BOOT_CMD(
	switch_rst, CONFIG_SYS_MAXARGS, 2, do_eth_reset,
	"Reset eth phy",
	"[print]"
);
#endif /* ! CONFIG_SPL_BUILD */

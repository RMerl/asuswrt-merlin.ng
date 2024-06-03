// SPDX-License-Identifier: GPL-2.0+
/*
 * Maintainer : Steve Sakoman <steve@sakoman.com>
 *
 * Derived from Beagle Board, 3430 SDP, and OMAP3EVM code by
 *	Richard Woodruff <r-woodruff2@ti.com>
 *	Syed Mohammed Khasim <khasim@ti.com>
 *	Sunil Kumar <sunilsaini05@gmail.com>
 *	Shashi Ranjan <shashiranjanmca05@gmail.com>
 *
 * (C) Copyright 2004-2008
 * Texas Instruments, <www.ti.com>
 */
#include <common.h>
#include <dm.h>
#include <ns16550.h>
#include <netdev.h>
#include <twl4030.h>
#include <linux/mtd/rawnand.h>
#include <asm/io.h>
#include <asm/arch/mmc_host_def.h>
#include <asm/arch/mux.h>
#include <asm/arch/mem.h>
#include <asm/arch/sys_proto.h>
#include <asm/gpio.h>
#include <asm/mach-types.h>
#include "overo.h"

#ifdef CONFIG_USB_EHCI_HCD
#include <usb.h>
#include <asm/ehci-omap.h>
#endif

#define TWL4030_I2C_BUS			0
#define EXPANSION_EEPROM_I2C_BUS	2
#define EXPANSION_EEPROM_I2C_ADDRESS	0x51

#define GUMSTIX_EMPTY_EEPROM		0x0

#define GUMSTIX_SUMMIT			0x01000200
#define GUMSTIX_TOBI			0x02000200
#define GUMSTIX_TOBI_DUO		0x03000200
#define GUMSTIX_PALO35			0x04000200
#define GUMSTIX_PALO43			0x05000200
#define GUMSTIX_CHESTNUT43		0x06000200
#define GUMSTIX_PINTO			0x07000200
#define GUMSTIX_GALLOP43		0x08000200
#define GUMSTIX_ALTO35			0x09000200
#define GUMSTIX_STAGECOACH		0x0A000200
#define GUMSTIX_THUMBO			0x0B000200
#define GUMSTIX_TURTLECORE		0x0C000200
#define GUMSTIX_ARBOR43C		0x0D000200

#define ETTUS_USRP_E			0x01000300

#define GUMSTIX_NO_EEPROM		0xffffffff

static struct {
	unsigned int device_vendor;
	unsigned char revision;
	unsigned char content;
	char fab_revision[8];
	char env_var[16];
	char env_setting[64];
} expansion_config = {0x0};

static const struct ns16550_platdata overo_serial = {
	.base = OMAP34XX_UART3,
	.reg_shift = 2,
	.clock = V_NS16550_CLK,
	.fcr = UART_FCR_DEFVAL,
};

U_BOOT_DEVICE(overo_uart) = {
	"ns16550_serial",
	&overo_serial
};

/*
 * Routine: get_sdio2_config
 * Description: Return information about the wifi module connection
 *              Returns 0 if the module connects though a level translator
 *              Returns 1 if the module connects directly
 */
int get_sdio2_config(void)
{
	int sdio_direct;

	if (!gpio_request(130, "") && !gpio_request(139, "")) {

		gpio_direction_output(130, 0);
		gpio_direction_input(139);

		sdio_direct = 1;
		gpio_set_value(130, 0);
		if (gpio_get_value(139) == 0) {
			gpio_set_value(130, 1);
			if (gpio_get_value(139) == 1)
				sdio_direct = 0;
		}

		gpio_direction_input(130);
	} else {
		puts("Error: unable to acquire sdio2 clk GPIOs\n");
		sdio_direct = -1;
	}

	return sdio_direct;
}

/*
 * Routine: get_expansion_id
 * Description: This function checks for expansion board by checking I2C
 *		bus 2 for the availability of an AT24C01B serial EEPROM.
 *		returns the device_vendor field from the EEPROM
 */
unsigned int get_expansion_id(void)
{
	if (expansion_config.device_vendor != 0x0)
		return expansion_config.device_vendor;

	i2c_set_bus_num(EXPANSION_EEPROM_I2C_BUS);

	/* return GUMSTIX_NO_EEPROM if eeprom doesn't respond */
	if (i2c_probe(EXPANSION_EEPROM_I2C_ADDRESS) == 1) {
		i2c_set_bus_num(TWL4030_I2C_BUS);
		return GUMSTIX_NO_EEPROM;
	}

	/* read configuration data */
	i2c_read(EXPANSION_EEPROM_I2C_ADDRESS, 0, 1, (u8 *)&expansion_config,
		 sizeof(expansion_config));

	i2c_set_bus_num(TWL4030_I2C_BUS);

	return expansion_config.device_vendor;
}

/*
 * Routine: misc_init_r
 * Description: Configure board specific parts
 */
int misc_init_r(void)
{
	unsigned int expansion_id;

	twl4030_power_init();
	twl4030_led_init(TWL4030_LED_LEDEN_LEDAON | TWL4030_LED_LEDEN_LEDBON);

	printf("Board revision: %d\n", get_board_revision());

	switch (get_sdio2_config()) {
	case 0:
		puts("Tranceiver detected on mmc2\n");
		MUX_OVERO_SDIO2_TRANSCEIVER();
		break;
	case 1:
		puts("Direct connection on mmc2\n");
		MUX_OVERO_SDIO2_DIRECT();
		break;
	default:
		puts("Unable to detect mmc2 connection type\n");
	}

	expansion_id = get_expansion_id();
	switch (expansion_id) {
	case GUMSTIX_SUMMIT:
		printf("Recognized Summit expansion board (rev %d %s)\n",
			expansion_config.revision,
			expansion_config.fab_revision);
		MUX_GUMSTIX();
		env_set("defaultdisplay", "dvi");
		env_set("expansionname", "summit");
		break;
	case GUMSTIX_TOBI:
		printf("Recognized Tobi expansion board (rev %d %s)\n",
			expansion_config.revision,
			expansion_config.fab_revision);
		MUX_GUMSTIX();
		env_set("defaultdisplay", "dvi");
		env_set("expansionname", "tobi");
		break;
	case GUMSTIX_TOBI_DUO:
		printf("Recognized Tobi Duo expansion board (rev %d %s)\n",
			expansion_config.revision,
			expansion_config.fab_revision);
		MUX_GUMSTIX();
		env_set("expansionname", "tobiduo");
		break;
	case GUMSTIX_PALO35:
		printf("Recognized Palo35 expansion board (rev %d %s)\n",
			expansion_config.revision,
			expansion_config.fab_revision);
		MUX_GUMSTIX();
		env_set("defaultdisplay", "lcd35");
		env_set("expansionname", "palo35");
		break;
	case GUMSTIX_PALO43:
		printf("Recognized Palo43 expansion board (rev %d %s)\n",
			expansion_config.revision,
			expansion_config.fab_revision);
		MUX_GUMSTIX();
		env_set("defaultdisplay", "lcd43");
		env_set("expansionname", "palo43");
		break;
	case GUMSTIX_CHESTNUT43:
		printf("Recognized Chestnut43 expansion board (rev %d %s)\n",
			expansion_config.revision,
			expansion_config.fab_revision);
		MUX_GUMSTIX();
		env_set("defaultdisplay", "lcd43");
		env_set("expansionname", "chestnut43");
		break;
	case GUMSTIX_PINTO:
		printf("Recognized Pinto expansion board (rev %d %s)\n",
			expansion_config.revision,
			expansion_config.fab_revision);
		MUX_GUMSTIX();
		break;
	case GUMSTIX_GALLOP43:
		printf("Recognized Gallop43 expansion board (rev %d %s)\n",
			expansion_config.revision,
			expansion_config.fab_revision);
		MUX_GUMSTIX();
		env_set("defaultdisplay", "lcd43");
		env_set("expansionname", "gallop43");
		break;
	case GUMSTIX_ALTO35:
		printf("Recognized Alto35 expansion board (rev %d %s)\n",
			expansion_config.revision,
			expansion_config.fab_revision);
		MUX_GUMSTIX();
		MUX_ALTO35();
		env_set("defaultdisplay", "lcd35");
		env_set("expansionname", "alto35");
		break;
	case GUMSTIX_STAGECOACH:
		printf("Recognized Stagecoach expansion board (rev %d %s)\n",
			expansion_config.revision,
			expansion_config.fab_revision);
		MUX_GUMSTIX();
		break;
	case GUMSTIX_THUMBO:
		printf("Recognized Thumbo expansion board (rev %d %s)\n",
			expansion_config.revision,
			expansion_config.fab_revision);
		MUX_GUMSTIX();
		break;
	case GUMSTIX_TURTLECORE:
		printf("Recognized Turtlecore expansion board (rev %d %s)\n",
			expansion_config.revision,
			expansion_config.fab_revision);
		MUX_GUMSTIX();
		break;
	case GUMSTIX_ARBOR43C:
		printf("Recognized Arbor43C expansion board (rev %d %s)\n",
			expansion_config.revision,
			expansion_config.fab_revision);
		MUX_GUMSTIX();
		MUX_ARBOR43C();
		env_set("defaultdisplay", "lcd43");
		env_set("expansionname", "arbor43c");
		break;
	case ETTUS_USRP_E:
		printf("Recognized Ettus Research USRP-E (rev %d %s)\n",
			expansion_config.revision,
			expansion_config.fab_revision);
		MUX_GUMSTIX();
		MUX_USRP_E();
		env_set("defaultdisplay", "dvi");
		break;
	case GUMSTIX_NO_EEPROM:
	case GUMSTIX_EMPTY_EEPROM:
		puts("No or empty EEPROM on expansion board\n");
		MUX_GUMSTIX();
		env_set("expansionname", "tobi");
		break;
	default:
		printf("Unrecognized expansion board 0x%08x\n", expansion_id);
		break;
	}

	if (expansion_config.content == 1)
		env_set(expansion_config.env_var, expansion_config.env_setting);

	omap_die_id_display();

	if (get_cpu_family() == CPU_OMAP34XX)
		env_set("boardname", "overo");
	else
		env_set("boardname", "overo-storm");

	return 0;
}

#if defined(CONFIG_CMD_NET)
/* GPMC definitions for LAN9221 chips on Tobi expansion boards */
static const u32 gpmc_lan_config[] = {
	NET_LAN9221_GPMC_CONFIG1,
	NET_LAN9221_GPMC_CONFIG2,
	NET_LAN9221_GPMC_CONFIG3,
	NET_LAN9221_GPMC_CONFIG4,
	NET_LAN9221_GPMC_CONFIG5,
	NET_LAN9221_GPMC_CONFIG6,
	/*CONFIG7- computed as params */
};

/*
 * Routine: setup_net_chip
 * Description: Setting up the configuration GPMC registers specific to the
 *	      Ethernet hardware.
 */
static void setup_net_chip(void)
{
	struct ctrl *ctrl_base = (struct ctrl *)OMAP34XX_CTRL_BASE;

	/* Enable off mode for NWE in PADCONF_GPMC_NWE register */
	writew(readw(&ctrl_base ->gpmc_nwe) | 0x0E00, &ctrl_base->gpmc_nwe);
	/* Enable off mode for NOE in PADCONF_GPMC_NADV_ALE register */
	writew(readw(&ctrl_base->gpmc_noe) | 0x0E00, &ctrl_base->gpmc_noe);
	/* Enable off mode for ALE in PADCONF_GPMC_NADV_ALE register */
	writew(readw(&ctrl_base->gpmc_nadv_ale) | 0x0E00,
		&ctrl_base->gpmc_nadv_ale);
}

/*
 * Routine: reset_net_chip
 * Description: Reset the Ethernet hardware.
 */
static void reset_net_chip(void)
{
	/* Make GPIO 64 as output pin and send a magic pulse through it */
	if (!gpio_request(64, "")) {
		gpio_direction_output(64, 0);
		gpio_set_value(64, 1);
		udelay(1);
		gpio_set_value(64, 0);
		udelay(1);
		gpio_set_value(64, 1);
	}
}

int board_eth_init(bd_t *bis)
{
	unsigned int expansion_id;
	int rc = 0;

#ifdef CONFIG_SMC911X
	expansion_id = get_expansion_id();
	switch (expansion_id) {
	case GUMSTIX_TOBI_DUO:
		/* second lan chip */
		enable_gpmc_cs_config(gpmc_lan_config, &gpmc_cfg->cs[4],
				      0x2B000000, GPMC_SIZE_16M);
		/* no break */
	case GUMSTIX_TOBI:
	case GUMSTIX_CHESTNUT43:
	case GUMSTIX_STAGECOACH:
	case GUMSTIX_NO_EEPROM:
	case GUMSTIX_EMPTY_EEPROM:
		/* first lan chip */
		enable_gpmc_cs_config(gpmc_lan_config, &gpmc_cfg->cs[5],
				      0x2C000000, GPMC_SIZE_16M);

		setup_net_chip();
		reset_net_chip();

		rc = smc911x_initialize(0, CONFIG_SMC911X_BASE);
		break;
	default:
		break;
	}
#endif

	return rc;
}
#endif

#if defined(CONFIG_MMC)
int board_mmc_init(bd_t *bis)
{
	return omap_mmc_init(0, 0, 0, -1, -1);
}
#endif

#if defined(CONFIG_MMC)
void board_mmc_power_init(void)
{
	twl4030_power_mmc_init(0);
}
#endif

#if defined(CONFIG_USB_EHCI_HCD)
static struct omap_usbhs_board_data usbhs_bdata = {
	.port_mode[0] = OMAP_USBHS_PORT_MODE_UNUSED,
	.port_mode[1] = OMAP_EHCI_PORT_MODE_PHY,
	.port_mode[2] = OMAP_USBHS_PORT_MODE_UNUSED
};

#define GUMSTIX_GPIO_USBH_CPEN		168
int ehci_hcd_init(int index, enum usb_init_type init,
		  struct ehci_hccr **hccr, struct ehci_hcor **hcor)
{
	/* Enable USB power */
	if (!gpio_request(GUMSTIX_GPIO_USBH_CPEN, "usbh_cpen"))
		gpio_direction_output(GUMSTIX_GPIO_USBH_CPEN, 1);

	return omap_ehci_hcd_init(index, &usbhs_bdata, hccr, hcor);
}

int ehci_hcd_stop(void)
{
	/* Disable USB power */
	gpio_set_value(GUMSTIX_GPIO_USBH_CPEN, 0);
	gpio_free(GUMSTIX_GPIO_USBH_CPEN);

	return omap_ehci_hcd_stop();
}

#endif /* CONFIG_USB_EHCI_HCD */

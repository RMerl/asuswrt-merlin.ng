// SPDX-License-Identifier: GPL-2.0+
/*
 * Board specific initialization for AM654 EVM
 *
 * Copyright (C) 2017-2018 Texas Instruments Incorporated - https://www.ti.com/
 *	Lokesh Vutla <lokeshvutla@ti.com>
 *
 */

#include <dm.h>
#include <fdt_support.h>
#include <image.h>
#include <init.h>
#include <net.h>
#include <asm/arch/hardware.h>
#include <asm/global_data.h>
#include <asm/gpio.h>
#include <asm/io.h>
#include <asm/omap_common.h>
#include <env.h>
#include <spl.h>
#include <linux/printk.h>

#include "../common/board_detect.h"
#include "../common/fdt_ops.h"

#define board_is_am65x_base_board()	board_ti_is("AM6-COMPROCEVM")

/* Daughter card presence detection signals */
enum {
	AM65X_EVM_APP_BRD_DET,
	AM65X_EVM_LCD_BRD_DET,
	AM65X_EVM_SERDES_BRD_DET,
	AM65X_EVM_HDMI_GPMC_BRD_DET,
	AM65X_EVM_BRD_DET_COUNT,
};

/* Max number of MAC addresses that are parsed/processed per daughter card */
#define DAUGHTER_CARD_NO_OF_MAC_ADDR	8

/* Regiter that controls the SERDES0 lane and clock assignment */
#define CTRLMMR_SERDES0_CTRL    0x00104080
#define PCIE_LANE0              0x1

DECLARE_GLOBAL_DATA_PTR;

int board_init(void)
{
	return 0;
}

int dram_init(void)
{
#ifdef CONFIG_PHYS_64BIT
	gd->ram_size = 0x100000000;
#else
	gd->ram_size = 0x80000000;
#endif

	return 0;
}

phys_addr_t board_get_usable_ram_top(phys_size_t total_size)
{
#ifdef CONFIG_PHYS_64BIT
	/* Limit RAM used by U-Boot to the DDR low region */
	if (gd->ram_top > 0x100000000)
		return 0x100000000;
#endif

	return gd->ram_top;
}

int dram_init_banksize(void)
{
	/* Bank 0 declares the memory available in the DDR low region */
	gd->bd->bi_dram[0].start = 0x80000000;
	gd->bd->bi_dram[0].size = 0x80000000;
	gd->ram_size = 0x80000000;

#ifdef CONFIG_PHYS_64BIT
	/* Bank 1 declares the memory available in the DDR high region */
	gd->bd->bi_dram[1].start = 0x880000000;
	gd->bd->bi_dram[1].size = 0x80000000;
	gd->ram_size = 0x100000000;
#endif

	return 0;
}

#ifdef CONFIG_SPL_LOAD_FIT
int board_fit_config_name_match(const char *name)
{
#ifdef CONFIG_TARGET_AM654_A53_EVM
	if (!strcmp(name, "k3-am654-base-board"))
		return 0;
#endif

	return -1;
}
#endif

#ifdef CONFIG_TI_I2C_BOARD_DETECT
int do_board_detect(void)
{
	int ret;

	ret = ti_i2c_eeprom_am6_get_base(CONFIG_EEPROM_BUS_ADDRESS,
					 CONFIG_EEPROM_CHIP_ADDRESS);
	if (ret)
		pr_err("Reading on-board EEPROM at 0x%02x failed %d\n",
		       CONFIG_EEPROM_CHIP_ADDRESS, ret);

	return ret;
}

int checkboard(void)
{
	struct ti_am6_eeprom *ep = TI_AM6_EEPROM_DATA;

	if (do_board_detect())
		/* EEPROM not populated */
		printf("Board: %s rev %s\n", "AM6-COMPROCEVM", "E3");
	else
		printf("Board: %s rev %s\n", ep->name, ep->version);

	return 0;
}

static void setup_board_eeprom_env(void)
{
	char *name = "am65x";

	if (do_board_detect())
		goto invalid_eeprom;

	if (board_is_am65x_base_board())
		name = "am65x";
	else
		printf("Unidentified board claims %s in eeprom header\n",
		       board_ti_get_name());

invalid_eeprom:
	set_board_info_env_am6(name);
	ti_set_fdt_env(NULL, NULL);
}

static int init_daughtercard_det_gpio(char *gpio_name, struct gpio_desc *desc)
{
	int ret;

	memset(desc, 0, sizeof(*desc));

	ret = dm_gpio_lookup_name(gpio_name, desc);
	if (ret < 0)
		return ret;

	/* Request GPIO, simply re-using the name as label */
	ret = dm_gpio_request(desc, gpio_name);
	if (ret < 0)
		return ret;

	return dm_gpio_set_dir_flags(desc, GPIOD_IS_IN);
}

static int probe_daughtercards(void)
{
	struct ti_am6_eeprom ep;
	struct gpio_desc board_det_gpios[AM65X_EVM_BRD_DET_COUNT];
	char mac_addr[DAUGHTER_CARD_NO_OF_MAC_ADDR][TI_EEPROM_HDR_ETH_ALEN];
	u8 mac_addr_cnt;
	char name_overlays[1024] = { 0 };
	int i, j;
	int ret;

	/*
	 * Daughter card presence detection signal name to GPIO (via I2C I/O
	 * expander @ address 0x38) name and EEPROM I2C address mapping.
	 */
	const struct {
		char *gpio_name;
		u8 i2c_addr;
	} slot_map[AM65X_EVM_BRD_DET_COUNT] = {
		{ "gpio@38_0", 0x52, },	/* AM65X_EVM_APP_BRD_DET */
		{ "gpio@38_1", 0x55, },	/* AM65X_EVM_LCD_BRD_DET */
		{ "gpio@38_2", 0x54, },	/* AM65X_EVM_SERDES_BRD_DET */
		{ "gpio@38_3", 0x53, },	/* AM65X_EVM_HDMI_GPMC_BRD_DET */
	};

	/* Declaration of daughtercards to probe */
	const struct {
		u8 slot_index;		/* Slot the card is installed */
		char *card_name;	/* EEPROM-programmed card name */
		char *dtbo_name;	/* Device tree overlay to apply */
		u8 eth_offset;		/* ethXaddr MAC address index offset */
	} cards[] = {
		{
			AM65X_EVM_APP_BRD_DET,
			"AM6-GPAPPEVM",
			"k3-am654-gp.dtbo",
			0,
		},
		{
			AM65X_EVM_APP_BRD_DET,
			"AM6-IDKAPPEVM",
			"k3-am654-idk.dtbo",
			3,
		},
		{
			AM65X_EVM_SERDES_BRD_DET,
			"SER-PCIE2LEVM",
			"k3-am654-pcie-usb2.dtbo",
			0,
		},
		{
			AM65X_EVM_SERDES_BRD_DET,
			"SER-PCIEUSBEVM",
			"k3-am654-pcie-usb3.dtbo",
			0,
		},
		{
			AM65X_EVM_LCD_BRD_DET,
			"OLDI-LCD1EVM",
			"k3-am654-evm-oldi-lcd1evm.dtbo",
			0,
		},
	};

	/*
	 * Initialize GPIO used for daughtercard slot presence detection and
	 * keep the resulting handles in local array for easier access.
	 */
	for (i = 0; i < AM65X_EVM_BRD_DET_COUNT; i++) {
		ret = init_daughtercard_det_gpio(slot_map[i].gpio_name,
						 &board_det_gpios[i]);
		if (ret < 0)
			return ret;
	}

	for (i = 0; i < ARRAY_SIZE(cards); i++) {
		/* Obtain card-specific slot index and associated I2C address */
		u8 slot_index = cards[i].slot_index;
		u8 i2c_addr = slot_map[slot_index].i2c_addr;

		/*
		 * The presence detection signal is active-low, hence skip
		 * over this card slot if anything other than 0 is returned.
		 */
		ret = dm_gpio_get_value(&board_det_gpios[slot_index]);
		if (ret < 0)
			return ret;
		else if (ret)
			continue;

		/* Get and parse the daughter card EEPROM record */
		ret = ti_i2c_eeprom_am6_get(CONFIG_EEPROM_BUS_ADDRESS, i2c_addr,
					    &ep,
					    (char **)mac_addr,
					    DAUGHTER_CARD_NO_OF_MAC_ADDR,
					    &mac_addr_cnt);
		if (ret) {
			pr_err("Reading daughtercard EEPROM at 0x%02x failed %d\n",
			       i2c_addr, ret);
			/*
			 * Even this is pretty serious let's just skip over
			 * this particular daughtercard, rather than ending
			 * the probing process altogether.
			 */
			continue;
		}

		/* Only process the parsed data if we found a match */
		if (strncmp(ep.name, cards[i].card_name, sizeof(ep.name)))
			continue;

		printf("Detected: %s rev %s\n", ep.name, ep.version);

		/*
		 * Populate any MAC addresses from daughtercard into the U-Boot
		 * environment, starting with a card-specific offset so we can
		 * have multiple cards contribute to the MAC pool in a well-
		 * defined manner.
		 */
		for (j = 0; j < mac_addr_cnt; j++) {
			if (!is_valid_ethaddr((u8 *)mac_addr[j]))
				continue;

			eth_env_set_enetaddr_by_index("eth",
						      cards[i].eth_offset + j,
						      (uchar *)mac_addr[j]);
		}

		/*
		 * It has been observed that setting SERDES0 lane mux to USB prevents USB
		 * 2.0 operation on USB0. Setting SERDES0 lane mux to non-USB when USB0 is
		 * used in USB 2.0 only mode solves this issue. For USB3.0+2.0 operation
		 * this issue is not present.
		 *
		 * Implement this workaround by writing 1 to LANE_FUNC_SEL field in
		 * CTRLMMR_SERDES0_CTRL register.
		 */
		if (!strncmp(ep.name, "SER-PCIE2LEVM", sizeof(ep.name)))
			writel(PCIE_LANE0, CTRLMMR_SERDES0_CTRL);

		/* Skip if no overlays are to be added */
		if (!strlen(cards[i].dtbo_name))
			continue;

		/*
		 * Make sure we are not running out of buffer space by checking
		 * if we can fit the new overlay, a trailing space to be used
		 * as a separator, plus the terminating zero.
		 */
		if (strlen(name_overlays) + strlen(cards[i].dtbo_name) + 2 >
		    sizeof(name_overlays))
			return -ENOMEM;

		/* Append to our list of overlays */
		strcat(name_overlays, cards[i].dtbo_name);
		strcat(name_overlays, " ");
	}

	/* Apply device tree overlay(s) to the U-Boot environment, if any */
	if (strlen(name_overlays))
		return env_set("name_overlays", name_overlays);

	return 0;
}
#endif

int board_late_init(void)
{
	if (IS_ENABLED(CONFIG_TI_I2C_BOARD_DETECT)) {
		struct ti_am6_eeprom *ep = TI_AM6_EEPROM_DATA;

		setup_board_eeprom_env();

		/*
		 * The first MAC address for ethernet a.k.a. ethernet0 comes from
		 * efuse populated via the am654 gigabit eth switch subsystem driver.
		 * All the other ones are populated via EEPROM, hence continue with
		 * an index of 1.
		 */
		board_ti_am6_set_ethaddr(1, ep->mac_addr_cnt);

		/* Check for and probe any plugged-in daughtercards */
		probe_daughtercards();
	}

	return 0;
}

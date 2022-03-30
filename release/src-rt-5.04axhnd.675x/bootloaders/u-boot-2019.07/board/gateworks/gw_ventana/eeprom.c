// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright (C) 2014 Gateworks Corporation
 * Author: Tim Harvey <tharvey@gateworks.com>
 */

#include <common.h>
#include <errno.h>
#include <hexdump.h>
#include <i2c.h>
#include <malloc.h>
#include <asm/bitops.h>

#include "gsc.h"
#include "ventana_eeprom.h"

/* read ventana EEPROM, check for validity, and return baseboard type */
int
read_eeprom(int bus, struct ventana_board_info *info)
{
	int i;
	int chksum;
	char baseboard;
	int type;
	unsigned char *buf = (unsigned char *)info;

	memset(info, 0, sizeof(*info));

	/*
	 * On a board with a missing/depleted backup battery for GSC, the
	 * board may be ready to probe the GSC before its firmware is
	 * running.  We will wait here indefinately for the GSC/EEPROM.
	 */
	while (1) {
		if (0 == i2c_set_bus_num(bus) &&
		    0 == i2c_probe(GSC_EEPROM_ADDR))
			break;
		mdelay(1);
	}

	/* read eeprom config section */
	if (gsc_i2c_read(GSC_EEPROM_ADDR, 0x00, 1, buf, sizeof(*info))) {
		puts("EEPROM: Failed to read EEPROM\n");
		return GW_UNKNOWN;
	}

	/* sanity checks */
	if (info->model[0] != 'G' || info->model[1] != 'W') {
		puts("EEPROM: Invalid Model in EEPROM\n");
		print_hex_dump_bytes("", DUMP_PREFIX_OFFSET, buf,
				     sizeof(*info));
		return GW_UNKNOWN;
	}

	/* validate checksum */
	for (chksum = 0, i = 0; i < sizeof(*info)-2; i++)
		chksum += buf[i];
	if ((info->chksum[0] != chksum>>8) ||
	    (info->chksum[1] != (chksum&0xff))) {
		puts("EEPROM: Failed EEPROM checksum\n");
		print_hex_dump_bytes("", DUMP_PREFIX_OFFSET, buf,
				     sizeof(*info));
		return GW_UNKNOWN;
	}

	/* original GW5400-A prototype */
	baseboard = info->model[3];
	if (strncasecmp((const char *)info->model, "GW5400-A", 8) == 0)
		baseboard = '0';

	type = GW_UNKNOWN;
	switch (baseboard) {
	case '0': /* original GW5400-A prototype */
		type = GW54proto;
		break;
	case '1':
		type = GW51xx;
		break;
	case '2':
		type = GW52xx;
		break;
	case '3':
		type = GW53xx;
		break;
	case '4':
		type = GW54xx;
		break;
	case '5':
		if (info->model[4] == '1') {
			type = GW551x;
			break;
		} else if (info->model[4] == '2') {
			type = GW552x;
			break;
		} else if (info->model[4] == '3') {
			type = GW553x;
			break;
		}
		break;
	case '6':
		if (info->model[4] == '0')
			type = GW560x;
		break;
	case '9':
		if (info->model[4] == '0' && info->model[5] == '1')
			type = GW5901;
		else if (info->model[4] == '0' && info->model[5] == '2')
			type = GW5902;
		else if (info->model[4] == '0' && info->model[5] == '3')
			type = GW5903;
		else if (info->model[4] == '0' && info->model[5] == '4')
			type = GW5904;
		else if (info->model[4] == '0' && info->model[5] == '5')
			type = GW5905;
		else if (info->model[4] == '0' && info->model[5] == '6')
			type = GW5906;
		else if (info->model[4] == '0' && info->model[5] == '7')
			type = GW5907;
		else if (info->model[4] == '0' && info->model[5] == '8')
			type = GW5908;
		else if (info->model[4] == '0' && info->model[5] == '9')
			type = GW5909;
		break;
	default:
		printf("EEPROM: Unknown model in EEPROM: %s\n", info->model);
		print_hex_dump_bytes("", DUMP_PREFIX_OFFSET, buf,
				     sizeof(*info));
		break;
	}
	return type;
}

/* list of config bits that the bootloader will remove from dtb if not set */
struct ventana_eeprom_config econfig[] = {
	{ "eth0", "ethernet0", EECONFIG_ETH0 },
	{ "usb0", NULL, EECONFIG_USB0 },
	{ "usb1", NULL, EECONFIG_USB1 },
	{ "mmc0", NULL, EECONFIG_SD0 },
	{ "mmc1", NULL, EECONFIG_SD1 },
	{ "mmc2", NULL, EECONFIG_SD2 },
	{ "mmc3", NULL, EECONFIG_SD3 },
	{ /* Sentinel */ }
};

#if defined(CONFIG_CMD_EECONFIG) && !defined(CONFIG_SPL_BUILD)
static struct ventana_eeprom_config *get_config(const char *name)
{
	struct ventana_eeprom_config *cfg = econfig;

	while (cfg->name) {
		if (0 == strcmp(name, cfg->name))
			return cfg;
		cfg++;
	}
	return NULL;
}

static u8 econfig_bytes[sizeof(ventana_info.config)];
static int econfig_init = -1;

static int do_econfig(cmd_tbl_t *cmdtp, int flag, int argc, char * const argv[])
{
	struct ventana_eeprom_config *cfg;
	struct ventana_board_info *info = &ventana_info;
	int i;

	if (argc < 2)
		return CMD_RET_USAGE;

	/* initialize */
	if (econfig_init != 1) {
		memcpy(econfig_bytes, info->config, sizeof(econfig_bytes));
		econfig_init = 1;
	}

	/* list configs */
	if ((strncmp(argv[1], "list", 4) == 0)) {
		cfg = econfig;
		while (cfg->name) {
			printf("%s: %d\n", cfg->name,
			       test_bit(cfg->bit, econfig_bytes) ?  1 : 0);
			cfg++;
		}
	}

	/* save */
	else if ((strncmp(argv[1], "save", 4) == 0)) {
		unsigned char *buf = (unsigned char *)info;
		int chksum;

		/* calculate new checksum */
		memcpy(info->config, econfig_bytes, sizeof(econfig_bytes));
		for (chksum = 0, i = 0; i < sizeof(*info)-2; i++)
			chksum += buf[i];
		debug("old chksum:0x%04x\n",
		      (info->chksum[0] << 8) | info->chksum[1]);
		debug("new chksum:0x%04x\n", chksum);
		info->chksum[0] = chksum >> 8;
		info->chksum[1] = chksum & 0xff;

		/* write new config data */
		if (gsc_i2c_write(GSC_EEPROM_ADDR, info->config - (u8 *)info,
				  1, econfig_bytes, sizeof(econfig_bytes))) {
			printf("EEPROM: Failed updating config\n");
			return CMD_RET_FAILURE;
		}

		/* write new config data */
		if (gsc_i2c_write(GSC_EEPROM_ADDR, info->chksum - (u8 *)info,
				  1, info->chksum, 2)) {
			printf("EEPROM: Failed updating checksum\n");
			return CMD_RET_FAILURE;
		}

		printf("Config saved to EEPROM\n");
	}

	/* get config */
	else if (argc == 2) {
		cfg = get_config(argv[1]);
		if (cfg) {
			printf("%s: %d\n", cfg->name,
			       test_bit(cfg->bit, econfig_bytes) ? 1 : 0);
		} else {
			printf("invalid config: %s\n", argv[1]);
			return CMD_RET_FAILURE;
		}
	}

	/* set config */
	else if (argc == 3) {
		cfg = get_config(argv[1]);
		if (cfg) {
			if (simple_strtol(argv[2], NULL, 10)) {
				test_and_set_bit(cfg->bit, econfig_bytes);
				printf("Enabled %s\n", cfg->name);
			} else {
				test_and_clear_bit(cfg->bit, econfig_bytes);
				printf("Disabled %s\n", cfg->name);
			}
		} else {
			printf("invalid config: %s\n", argv[1]);
			return CMD_RET_FAILURE;
		}
	}

	else
		return CMD_RET_USAGE;

	return CMD_RET_SUCCESS;
}

U_BOOT_CMD(
	econfig, 3, 0, do_econfig,
	"EEPROM configuration",
	"list - list config\n"
	"save - save config to EEPROM\n"
	"<name> - get config 'name'\n"
	"<name> [0|1] - set config 'name' to value\n"
);

#endif /* CONFIG_CMD_EECONFIG */

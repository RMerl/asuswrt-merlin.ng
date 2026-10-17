// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright (C) 2014 Gateworks Corporation
 * Author: Tim Harvey <tharvey@gateworks.com>
 */

#include <command.h>
#include <common.h>
#include <gsc.h>
#include <hexdump.h>
#include <i2c.h>
#include <asm/arch/sys_proto.h>
#include <dm/device.h>
#include <dm/uclass.h>
#include <linux/ctype.h>
#include <linux/delay.h>

#include "eeprom.h"

/*
 * EEPROM board info struct populated by read_eeprom so that we only have to
 * read it once.
 */
struct ventana_board_info ventana_info;
int board_type;

#if CONFIG_IS_ENABLED(DM_I2C)
struct udevice *i2c_get_dev(int busno, int slave)
{
	struct udevice *dev, *bus;
	int ret;

	ret = uclass_get_device_by_seq(UCLASS_I2C, busno, &bus);
	if (ret)
		return NULL;
	ret = dm_i2c_probe(bus, slave, 0, &dev);
	if (ret)
		return NULL;

	return dev;
}
#endif

/*
 * The Gateworks System Controller will fail to ACK a master transaction if
 * it is busy, which can occur during its 1HZ timer tick while reading ADC's.
 * When this does occur, it will never be busy long enough to fail more than
 * 2 back-to-back transfers.  Thus we wrap i2c_read and i2c_write with
 * 3 retries.
 */
int gsc_i2c_read(uchar chip, uint addr, int alen, uchar *buf, int len)
{
	int retry = 3;
	int n = 0;
	int ret;
#if CONFIG_IS_ENABLED(DM_I2C)
	struct udevice *dev;

	dev = i2c_get_dev(BOARD_EEPROM_BUSNO, chip);
	if (!dev)
		return -ENODEV;
	ret = i2c_set_chip_offset_len(dev, alen);
	if (ret) {
		puts("EEPROM: Failed to set alen\n");
		return ret;
	}
#else
	i2c_set_bus_num(BOARD_EEPROM_BUSNO);
#endif

	while (n++ < retry) {
#if CONFIG_IS_ENABLED(DM_I2C)
		ret = dm_i2c_read(dev, addr, buf, len);
#else
		ret = i2c_read(chip, addr, alen, buf, len);
#endif
		if (!ret)
			break;
		debug("%s: 0x%02x 0x%02x retry%d: %d\n", __func__, chip, addr,
		      n, ret);
		if (ret != -ENODEV)
			break;
		mdelay(10);
	}
	return ret;
}

int gsc_i2c_write(uchar chip, uint addr, int alen, uchar *buf, int len)
{
	int retry = 3;
	int n = 0;
	int ret;
#if CONFIG_IS_ENABLED(DM_I2C)
	struct udevice *dev;

	dev = i2c_get_dev(BOARD_EEPROM_BUSNO, chip);
	if (!dev)
		return -ENODEV;
	ret = i2c_set_chip_offset_len(dev, alen);
	if (ret) {
		puts("EEPROM: Failed to set alen\n");
		return ret;
	}
#endif

	while (n++ < retry) {
#if CONFIG_IS_ENABLED(DM_I2C)
		ret = dm_i2c_write(dev, addr, buf, len);
#else
		ret = i2c_write(chip, addr, alen, buf, len);
#endif
		if (!ret)
			break;
		debug("%s: 0x%02x 0x%02x retry%d: %d\n", __func__, chip, addr,
		      n, ret);
		if (ret != -ENODEV)
			break;
		mdelay(10);
	}
	mdelay(100);
	return ret;
}

/* determine BOM revision from model */
int get_bom_rev(const char *str)
{
	int  rev_bom = 0;
	int i;

	for (i = strlen(str) - 1; i > 0; i--) {
		if (str[i] == '-')
			break;
		if (str[i] >= '1' && str[i] <= '9') {
			rev_bom = str[i] - '0';
			break;
		}
	}
	return rev_bom;
}

/* determine PCB revision from model */
char get_pcb_rev(const char *str)
{
	char rev_pcb = 'A';
	int i;

	for (i = strlen(str) - 1; i > 0; i--) {
		if (str[i] == '-')
			break;
		if (str[i] >= 'A') {
			rev_pcb = str[i];
			break;
		}
	}
	return rev_pcb;
}

/*
 * get dt name based on model and detail level:
 */
const char *gsc_get_dtb_name(int level, char *buf, int sz)
{
	const char *model = (const char *)ventana_info.model;
	const char *pre = is_mx6dq() ? "imx6q-" : "imx6dl-";
	int modelno, rev_pcb, rev_bom;

	/* a few board models are dt equivalents to other models */
	if (strncasecmp(model, "gw5906", 6) == 0)
		model = "gw552x-d";
	else if (strncasecmp(model, "gw5908", 6) == 0)
		model = "gw53xx-f";
	else if (strncasecmp(model, "gw5905", 6) == 0)
		model = "gw5904-a";

	modelno = ((model[2] - '0') * 1000)
		  + ((model[3] - '0') * 100)
		  + ((model[4] - '0') * 10)
		  + (model[5] - '0');
	rev_pcb = tolower(get_pcb_rev(model));
	rev_bom = get_bom_rev(model);

	/* compare model/rev/bom in order of most specific to least */
	snprintf(buf, sz, "%s%04d", pre, modelno);
	switch (level) {
	case 0: /* full model first (ie gw5400-a1) */
		if (rev_bom) {
			snprintf(buf, sz, "%sgw%04d-%c%d", pre, modelno, rev_pcb, rev_bom);
			break;
		}
		fallthrough;
	case 1: /* don't care about bom rev (ie gw5400-a) */
		snprintf(buf, sz, "%sgw%04d-%c", pre, modelno, rev_pcb);
		break;
	case 2: /* don't care about the pcb rev (ie gw5400) */
		snprintf(buf, sz, "%sgw%04d", pre, modelno);
		break;
	case 3: /* look for generic model (ie gw540x) */
		snprintf(buf, sz, "%sgw%03dx", pre, modelno / 10);
		break;
	case 4: /* look for more generic model (ie gw54xx) */
		snprintf(buf, sz, "%sgw%02dxx", pre, modelno / 100);
		break;
	default: /* give up */
		return NULL;
	}

	return buf;
}
/* read ventana EEPROM, check for validity, and return baseboard type */
int
read_eeprom(struct ventana_board_info *info)
{
	int i;
	int chksum;
	char baseboard;
	int type;
	unsigned char *buf = (unsigned char *)info;

	memset(info, 0, sizeof(*info));

	/* read eeprom config section */
	if (gsc_i2c_read(BOARD_EEPROM_ADDR, 0x00, 1, buf, sizeof(*info))) {
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
		else if (info->model[4] == '1' && info->model[5] == '0')
			type = GW5910;
		else if (info->model[4] == '1' && info->model[5] == '2')
			type = GW5912;
		else if (info->model[4] == '1' && info->model[5] == '3')
			type = GW5913;
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

static int do_econfig(struct cmd_tbl *cmdtp, int flag, int argc,
		      char *const argv[])
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
		if (gsc_i2c_write(BOARD_EEPROM_ADDR, info->config - (u8 *)info,
				  1, econfig_bytes, sizeof(econfig_bytes))) {
			printf("EEPROM: Failed updating config\n");
			return CMD_RET_FAILURE;
		}

		/* write new config data */
		if (gsc_i2c_write(BOARD_EEPROM_ADDR, info->chksum - (u8 *)info,
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

// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright 2021 Gateworks Corporation
 */

#include <common.h>
#include <gsc.h>
#include <hexdump.h>
#include <i2c.h>
#include <dm/uclass.h>

#include "eeprom.h"

/* I2C */
#define SOM_EEPROM_BUSNO		0
#define SOM_EEPROM_ADDR			0x51
#define BASEBOARD_EEPROM_BUSNO		1
#define BASEBOARD_EEPROM_ADDR		0x52

struct venice_board_info som_info;
struct venice_board_info base_info;
char venice_model[32];
char venice_baseboard_model[32];
u32 venice_serial;

/* return a mac address from EEPROM info */
int eeprom_getmac(int index, uint8_t *address)
{
	int i, j;
	u32 maclow, machigh;
	u64 mac;

	j = 0;
	if (som_info.macno) {
		maclow = som_info.mac[5];
		maclow |= som_info.mac[4] << 8;
		maclow |= som_info.mac[3] << 16;
		maclow |= som_info.mac[2] << 24;
		machigh = som_info.mac[1];
		machigh |= som_info.mac[0] << 8;
		mac = machigh;
		mac <<= 32;
		mac |= maclow;
		for (i = 0; i < som_info.macno; i++, j++) {
			if (index == j)
				goto out;
		}
	}

	maclow = base_info.mac[5];
	maclow |= base_info.mac[4] << 8;
	maclow |= base_info.mac[3] << 16;
	maclow |= base_info.mac[2] << 24;
	machigh = base_info.mac[1];
	machigh |= base_info.mac[0] << 8;
	mac = machigh;
	mac <<= 32;
	mac |= maclow;
	for (i = 0; i < base_info.macno; i++, j++) {
		if (index == j)
			goto out;
	}

	return -EINVAL;

out:
	mac += i;
	address[0] = (mac >> 40) & 0xff;
	address[1] = (mac >> 32) & 0xff;
	address[2] = (mac >> 24) & 0xff;
	address[3] = (mac >> 16) & 0xff;
	address[4] = (mac >> 8) & 0xff;
	address[5] = (mac >> 0) & 0xff;

	return 0;
}

static int eeprom_read(int busno, int slave, int alen, struct venice_board_info *info)
{
	int i;
	int chksum;
	unsigned char *buf = (unsigned char *)info;
	struct udevice *dev, *bus;
	int ret;

	/* probe device */
	ret = uclass_get_device_by_seq(UCLASS_I2C, busno, &bus);
	if (ret)
		return ret;
	ret = dm_i2c_probe(bus, slave, 0, &dev);
	if (ret)
		return ret;

	/* read eeprom config section */
	memset(info, 0, sizeof(*info));
	ret = i2c_set_chip_offset_len(dev, alen);
	if (ret) {
		puts("EEPROM: Failed to set alen\n");
		return ret;
	}
	ret = dm_i2c_read(dev, 0x00, buf, sizeof(*info));
	if (ret) {
		if (slave == SOM_EEPROM_ADDR)
			printf("EEPROM: Failed to read EEPROM\n");
		return ret;
	}

	/* validate checksum */
	for (chksum = 0, i = 0; i < (int)sizeof(*info) - 2; i++)
		chksum += buf[i];
	if ((info->chksum[0] != chksum >> 8) ||
	    (info->chksum[1] != (chksum & 0xff))) {
		printf("EEPROM: I2C%d@0x%02x: Invalid Checksum\n", busno, slave);
		print_hex_dump_bytes("", DUMP_PREFIX_NONE, buf, sizeof(*info));
		memset(info, 0, sizeof(*info));
		return -EINVAL;
	}

	/* sanity check valid model */
	if (info->model[0] != 'G' || info->model[1] != 'W') {
		printf("EEPROM: I2C%d@0x%02x: Invalid Model in EEPROM\n", busno, slave);
		print_hex_dump_bytes("", DUMP_PREFIX_NONE, buf, sizeof(*info));
		memset(info, 0, sizeof(*info));
		return -EINVAL;
	}

	return 0;
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
 *
 * For boards that are a combination of a SoM plus a Baseboard:
 *   Venice SoM part numbers are GW70xx where xx is:
 *    7000-7019: same PCB with som dt of '0x'
 *    7020-7039: same PCB with som dt of '2x'
 *    7040-7059: same PCB with som dt of '4x'
 *    7060-7079: same PCB with som dt of '6x'
 *    7080-7099: same PCB with som dt of '8x'
 *   Venice Baseboard part numbers are GW7xxx where xxx is:
 *    7100-7199: same PCB with base dt of '71xx'
 *    7200-7299: same PCB with base dt of '72xx'
 *    7300-7399: same PCB with base dt of '73xx'
 *    7400-7499: same PCB with base dt of '74xx'
 *    7500-7599: same PCB with base dt of '75xx'
 *    7600-7699: same PCB with base dt of '76xx'
 *    7700-7799: same PCB with base dt of '77xx'
 *    7800-7899: same PCB with base dt of '78xx'
 *   DT name is comprised of:
 *    gw<base dt>-<som dt>-[base-pcb-rev][base-bom-rev][som-pcb-rev][som-bom-rev]
 *
 * For board models from 7900-7999 each PCB is unique with its own dt:
 *   DT name is comprised:
 *    gw<model>-[pcb-rev][bom-rev]
 *
 */
#define snprintfcat(dest, sz, fmt, ...) \
	snprintf((dest) + strlen(dest), (sz) - strlen(dest), fmt, ##__VA_ARGS__)
const char *eeprom_get_dtb_name(int level, char *buf, int sz)
{
#ifdef CONFIG_IMX8MM
	const char *pre = "imx8mm-venice-gw";
#elif CONFIG_IMX8MN
	const char *pre = "imx8mn-venice-gw";
#elif CONFIG_IMX8MP
	const char *pre = "imx8mp-venice-gw";
#endif
	int model, rev_pcb, rev_bom;

	model = ((som_info.model[2] - '0') * 1000)
		+ ((som_info.model[3] - '0') * 100)
		+ ((som_info.model[4] - '0') * 10)
		+ (som_info.model[5] - '0');
	rev_pcb = tolower(get_pcb_rev(som_info.model));
	rev_bom = get_bom_rev(som_info.model);

	/* som + baseboard*/
	if (base_info.model[0]) {
		/* baseboard id: 7100-7199->71; 7200-7299->72; etc */
		int base = ((base_info.model[2] - '0') * 10) + (base_info.model[3] - '0');
		/* som id: 7000-7019->1; 7020-7039->2; etc */
		int som = ((model % 100) / 20) * 2;
		int rev_base_pcb = tolower(get_pcb_rev(base_info.model));
		int rev_base_bom = get_bom_rev(base_info.model);

		snprintf(buf, sz, "%s%2dxx-%dx", pre, base, som);
		/* GW79xx baseboards have no build options */
		if (base == 79) {
			base = (int)strtoul(base_info.model + 2, NULL, 10);
			snprintf(buf, sz, "%s%4d-%dx", pre, base, som);
		}
		switch (level) {
		case 0: /* full model (ie gw73xx-0x-a1a1) */
			if (rev_base_bom)
				snprintfcat(buf, sz, "-%c%d", rev_base_pcb, rev_base_bom);
			else
				snprintfcat(buf, sz, "-%c", rev_base_pcb);
			if (rev_bom)
				snprintfcat(buf, sz, "%c%d", rev_pcb, rev_bom);
			else
				snprintfcat(buf, sz, "%c", rev_pcb);
			break;
		case 1: /* don't care about SoM revision */
			if (rev_base_bom)
				snprintfcat(buf, sz, "-%c%d", rev_base_pcb, rev_base_bom);
			else
				snprintfcat(buf, sz, "-%c", rev_base_pcb);
			snprintfcat(buf, sz, "xx");
			break;
		case 2: /* don't care about baseboard revision */
			snprintfcat(buf, sz, "-xx");
			if (rev_bom)
				snprintfcat(buf, sz, "%c%d", rev_pcb, rev_bom);
			else
				snprintfcat(buf, sz, "%c", rev_pcb);
			break;
		case 3: /* don't care about SoM/baseboard revision */
			break;
		default:
			return NULL;
		}
	} else {
		snprintf(buf, sz, "%s%04d", pre, model);
		switch (level) {
		case 0: /* full model wth PCB and BOM revision first (ie gw7901-a1) */
			if (rev_bom)
				snprintfcat(buf, sz, "-%c%d", rev_pcb, rev_bom);
			else
				snprintfcat(buf, sz, "-%c", rev_pcb);
			break;
		case 1: /* don't care about BOM revision */
			snprintfcat(buf, sz, "-%c", rev_pcb);
			break;
		case 2: /* don't care about PCB or BOM revision */
			break;
		case 3: /* don't care about last digit of model */
			buf[strlen(buf) - 1] = 'x';
			break;
		case 4: /* don't care about last two digits of model */
			buf[strlen(buf) - 1] = 'x';
			buf[strlen(buf) - 2] = 'x';
			break;
		default:
			return NULL;
			break;
		}
	}

	return buf;
}

static int eeprom_info(bool verbose)
{
	printf("Model   : %s\n", venice_model);
	printf("Serial  : %d\n", som_info.serial);
	printf("MFGDate : %02x-%02x-%02x%02x\n",
	       som_info.mfgdate[0], som_info.mfgdate[1],
	       som_info.mfgdate[2], som_info.mfgdate[3]);
	if (base_info.model[0] && verbose) {
		printf("SOM     : %s %d %02x-%02x-%02x%02x\n",
		       som_info.model, som_info.serial,
		       som_info.mfgdate[0], som_info.mfgdate[1],
		       som_info.mfgdate[2], som_info.mfgdate[3]);
		printf("BASE    : %s %d %02x-%02x-%02x%02x\n",
		       base_info.model, base_info.serial,
		       base_info.mfgdate[0], base_info.mfgdate[1],
		       base_info.mfgdate[2], base_info.mfgdate[3]);
	}

	return 0;
}

int venice_eeprom_init(int quiet)
{
	char rev_pcb;
	int rev_bom;
	int ret;

	ret = eeprom_read(SOM_EEPROM_BUSNO, SOM_EEPROM_ADDR, 1, &som_info);
	if (ret) {
		puts("ERROR: Failed to probe EEPROM\n");
		memset(&som_info, 0, sizeof(som_info));
		return 0;
	}

	/* read optional baseboard EEPROM */
	eeprom_read(BASEBOARD_EEPROM_BUSNO, BASEBOARD_EEPROM_ADDR, 2, &base_info);

	/* create model strings */
	if (base_info.model[0]) {
		sprintf(venice_model, "GW%c%c%c%c-%c%c-",
			som_info.model[2], /* family */
			base_info.model[3], /* baseboard */
			base_info.model[4], base_info.model[5], /* subload of baseboard */
			som_info.model[4], som_info.model[5]); /* last 2digits of SOM */
		strlcpy(venice_baseboard_model, base_info.model, sizeof(venice_baseboard_model));

		/* baseboard revision */
		rev_pcb = get_pcb_rev(base_info.model);
		rev_bom = get_bom_rev(base_info.model);
		if (rev_bom)
			sprintf(venice_model + strlen(venice_model), "%c%d", rev_pcb, rev_bom);
		else
			sprintf(venice_model + strlen(venice_model), "%c", rev_pcb);
		/* som revision */
		rev_pcb = get_pcb_rev(som_info.model);
		rev_bom = get_bom_rev(som_info.model);
		if (rev_bom)
			sprintf(venice_model + strlen(venice_model), "%c%d", rev_pcb, rev_bom);
		else
			sprintf(venice_model + strlen(venice_model), "%c", rev_pcb);
	} else {
		strcpy(venice_model, som_info.model);
	}
	venice_serial = som_info.serial;

	if (!quiet)
		eeprom_info(false);

	return (16 << som_info.sdram_size);
}

void board_gsc_info(void)
{
	eeprom_info(true);
}

const char *eeprom_get_model(void)
{
	return venice_model;
}

const char *eeprom_get_baseboard_model(void)
{
	return venice_baseboard_model;
}

u32 eeprom_get_serial(void)
{
	return venice_serial;
}

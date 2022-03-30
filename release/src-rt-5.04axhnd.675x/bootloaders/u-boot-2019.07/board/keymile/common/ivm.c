// SPDX-License-Identifier: GPL-2.0+
/*
 * (C) Copyright 2011
 * Holger Brunck, Keymile GmbH Hannover, holger.brunck@keymile.com
 */

#include <common.h>
#include <cli_hush.h>
#include <i2c.h>
#include "common.h"

#define MAC_STR_SZ	20

static int ivm_calc_crc(unsigned char *buf, int len)
{
	const unsigned short crc_tab[16] = {
		0x0000, 0xCC01, 0xD801, 0x1400,
		0xF001, 0x3C00, 0x2800, 0xE401,
		0xA001, 0x6C00, 0x7800, 0xB401,
		0x5000, 0x9C01, 0x8801, 0x4400};

	unsigned short crc     = 0;   /* final result */
	unsigned short r1      = 0;   /* temp */
	unsigned char  byte    = 0;   /* input buffer */
	int	i;

	/* calculate CRC from array data */
	for (i = 0; i < len; i++) {
		byte = buf[i];

		/* lower 4 bits */
		r1 = crc_tab[crc & 0xF];
		crc = ((crc) >> 4) & 0x0FFF;
		crc = crc ^ r1 ^ crc_tab[byte & 0xF];

		/* upper 4 bits */
		r1 = crc_tab[crc & 0xF];
		crc = (crc >> 4) & 0x0FFF;
		crc = crc ^ r1 ^ crc_tab[(byte >> 4) & 0xF];
	}
	return crc;
}

static int ivm_set_value(char *name, char *value)
{
	char tempbuf[256];

	if (value != NULL) {
		sprintf(tempbuf, "%s=%s", name, value);
		return set_local_var(tempbuf, 0);
	} else {
		unset_local_var(name);
	}
	return 0;
}

static int ivm_get_value(unsigned char *buf, int len, char *name, int off,
				int check)
{
	unsigned short	val;
	unsigned char	valbuf[30];

	if ((buf[off + 0] != buf[off + 2]) &&
	    (buf[off + 2] != buf[off + 4])) {
		printf("%s Error corrupted %s\n", __func__, name);
		val = -1;
	} else {
		val = buf[off + 0] + (buf[off + 1] << 8);
		if ((val == 0) && (check == 1))
			val = -1;
	}
	sprintf((char *)valbuf, "%x", val);
	ivm_set_value(name, (char *)valbuf);
	return val;
}

#define INV_BLOCKSIZE		0x100
#define INV_DATAADDRESS		0x21
#define INVENTORYDATASIZE	(INV_BLOCKSIZE - INV_DATAADDRESS - 3)

#define IVM_POS_SHORT_TEXT		0
#define IVM_POS_MANU_ID			1
#define IVM_POS_MANU_SERIAL		2
#define IVM_POS_PART_NUMBER		3
#define IVM_POS_BUILD_STATE		4
#define IVM_POS_SUPPLIER_PART_NUMBER	5
#define IVM_POS_DELIVERY_DATE		6
#define IVM_POS_SUPPLIER_BUILD_STATE	7
#define IVM_POS_CUSTOMER_ID		8
#define IVM_POS_CUSTOMER_PROD_ID	9
#define IVM_POS_HISTORY			10
#define IVM_POS_SYMBOL_ONLY		11

static char convert_char(char c)
{
	return (c < ' ' || c > '~') ? '.' : c;
}

static int ivm_findinventorystring(int type,
					unsigned char *const string,
					unsigned long maxlen,
					unsigned char *buf)
{
	int xcode = 0;
	unsigned long cr = 0;
	unsigned long addr = INV_DATAADDRESS;
	unsigned long size = 0;
	unsigned long nr = type;
	int stop = 0;	/* stop on semicolon */

	memset(string, '\0', maxlen);
	switch (type) {
	case IVM_POS_SYMBOL_ONLY:
		nr = 0;
		stop = 1;
	break;
	default:
		nr = type;
		stop = 0;
	}

	/* Look for the requested number of CR. */
	while ((cr != nr) && (addr < INVENTORYDATASIZE)) {
		if (buf[addr] == '\r')
			cr++;
		addr++;
	}

	/*
	 * the expected number of CR was found until the end of the IVM
	 *  content --> fill string
	 */
	if (addr < INVENTORYDATASIZE) {
		/* Copy the IVM string in the corresponding string */
		for (; (buf[addr] != '\r')			&&
			((buf[addr] != ';') ||  (!stop))	&&
			(size < (maxlen - 1)			&&
			(addr < INVENTORYDATASIZE)); addr++) {
			size += sprintf((char *)string + size, "%c",
						convert_char (buf[addr]));
		}

		/*
		 * copy phase is done: check if everything is ok. If not,
		 * the inventory data is most probably corrupted: tell
		 * the world there is a problem!
		 */
		if (addr == INVENTORYDATASIZE) {
			xcode = -1;
			printf("Error end of string not found\n");
		} else if ((size > (maxlen - 1)) &&
			   (buf[addr] != '\r')) {
			xcode = -1;
			printf("string too long till next CR\n");
		}
	} else {
		/*
		 * some CR are missing...
		 * the inventory data is most probably corrupted
		 */
		xcode = -1;
		printf("not enough cr found\n");
	}
	return xcode;
}

#define GET_STRING(name, which, len) \
	if (ivm_findinventorystring(which, valbuf, len, buf) == 0) { \
		ivm_set_value(name, (char *)valbuf); \
	}

static int ivm_check_crc(unsigned char *buf, int block)
{
	unsigned long	crc;
	unsigned long	crceeprom;

	crc = ivm_calc_crc(buf, CONFIG_SYS_IVM_EEPROM_PAGE_LEN - 2);
	crceeprom = (buf[CONFIG_SYS_IVM_EEPROM_PAGE_LEN - 1] + \
			buf[CONFIG_SYS_IVM_EEPROM_PAGE_LEN - 2] * 256);
	if (crc != crceeprom) {
		if (block == 0)
			printf("Error CRC Block: %d EEprom: calculated: \
			%lx EEprom: %lx\n", block, crc, crceeprom);
		return -1;
	}
	return 0;
}

/* take care of the possible MAC address offset and the IVM content offset */
static int process_mac(unsigned char *valbuf, unsigned char *buf,
				int offset, bool unique)
{
	unsigned char mac[6];
	unsigned long val = (buf[4] << 16) + (buf[5] << 8) + buf[6];

	/* use an intermediate buffer, to not change IVM content
	 * MAC address is at offset 1
	 */
	memcpy(mac, buf+1, 6);

	/* MAC adress can be set to locally administred, this is only allowed
	 * for interfaces which have now connection to the outside. For these
	 * addresses we need to set the second bit in the first byte.
	 */
	if (!unique)
		mac[0] |= 0x2;

	if (offset) {
		val += offset;
		mac[3] = (val >> 16) & 0xff;
		mac[4] = (val >> 8) & 0xff;
		mac[5] = val & 0xff;
	}

	sprintf((char *)valbuf, "%pM", mac);
	return 0;
}

static int ivm_analyze_block2(unsigned char *buf, int len)
{
	unsigned char	valbuf[MAC_STR_SZ];
	unsigned long	count;

	/* IVM_MAC Adress begins at offset 1 */
	sprintf((char *)valbuf, "%pM", buf + 1);
	ivm_set_value("IVM_MacAddress", (char *)valbuf);
	/* IVM_MacCount */
	count = (buf[10] << 24) +
		   (buf[11] << 16) +
		   (buf[12] << 8)  +
		    buf[13];
	if (count == 0xffffffff)
		count = 1;
	sprintf((char *)valbuf, "%lx", count);
	ivm_set_value("IVM_MacCount", (char *)valbuf);
	return 0;
}

int ivm_analyze_eeprom(unsigned char *buf, int len)
{
	unsigned short	val;
	unsigned char	valbuf[CONFIG_SYS_IVM_EEPROM_PAGE_LEN];
	unsigned char	*tmp;

	if (ivm_check_crc(buf, 0) != 0)
		return -1;

	ivm_get_value(buf, CONFIG_SYS_IVM_EEPROM_PAGE_LEN,
			"IVM_BoardId", 0, 1);
	val = ivm_get_value(buf, CONFIG_SYS_IVM_EEPROM_PAGE_LEN,
			"IVM_HWKey", 6, 1);
	if (val != 0xffff) {
		sprintf((char *)valbuf, "%x", ((val / 100) % 10));
		ivm_set_value("IVM_HWVariant", (char *)valbuf);
		sprintf((char *)valbuf, "%x", (val % 100));
		ivm_set_value("IVM_HWVersion", (char *)valbuf);
	}
	ivm_get_value(buf, CONFIG_SYS_IVM_EEPROM_PAGE_LEN,
		"IVM_Functions", 12, 0);

	GET_STRING("IVM_Symbol", IVM_POS_SYMBOL_ONLY, 8)
	GET_STRING("IVM_DeviceName", IVM_POS_SHORT_TEXT, 64)
	tmp = (unsigned char *)env_get("IVM_DeviceName");
	if (tmp) {
		int	len = strlen((char *)tmp);
		int	i = 0;

		while (i < len) {
			if (tmp[i] == ';') {
				ivm_set_value("IVM_ShortText",
					(char *)&tmp[i + 1]);
				break;
			}
			i++;
		}
		if (i >= len)
			ivm_set_value("IVM_ShortText", NULL);
	} else {
		ivm_set_value("IVM_ShortText", NULL);
	}
	GET_STRING("IVM_ManufacturerID", IVM_POS_MANU_ID, 32)
	GET_STRING("IVM_ManufacturerSerialNumber", IVM_POS_MANU_SERIAL, 20)
	GET_STRING("IVM_ManufacturerPartNumber", IVM_POS_PART_NUMBER, 32)
	GET_STRING("IVM_ManufacturerBuildState", IVM_POS_BUILD_STATE, 32)
	GET_STRING("IVM_SupplierPartNumber", IVM_POS_SUPPLIER_PART_NUMBER, 32)
	GET_STRING("IVM_DelieveryDate", IVM_POS_DELIVERY_DATE, 32)
	GET_STRING("IVM_SupplierBuildState", IVM_POS_SUPPLIER_BUILD_STATE, 32)
	GET_STRING("IVM_CustomerID", IVM_POS_CUSTOMER_ID, 32)
	GET_STRING("IVM_CustomerProductID", IVM_POS_CUSTOMER_PROD_ID, 32)

	if (ivm_check_crc(&buf[CONFIG_SYS_IVM_EEPROM_PAGE_LEN * 2], 2) != 0)
		return 0;
	ivm_analyze_block2(&buf[CONFIG_SYS_IVM_EEPROM_PAGE_LEN * 2],
		CONFIG_SYS_IVM_EEPROM_PAGE_LEN);

	return 0;
}

static int ivm_populate_env(unsigned char *buf, int len)
{
	unsigned char	*page2;
	unsigned char	valbuf[MAC_STR_SZ];

	/* do we have the page 2 filled ? if not return */
	if (ivm_check_crc(buf, 2))
		return 0;
	page2 = &buf[CONFIG_SYS_IVM_EEPROM_PAGE_LEN*2];

#ifndef CONFIG_KMTEGR1
	/* if an offset is defined, add it */
	process_mac(valbuf, page2, CONFIG_PIGGY_MAC_ADRESS_OFFSET, true);
	env_set((char *)"ethaddr", (char *)valbuf);
#ifdef CONFIG_KMVECT1
/* KMVECT1 has two ethernet interfaces */
	process_mac(valbuf, page2, 1, true);
	env_set((char *)"eth1addr", (char *)valbuf);
#endif
#else
/* KMTEGR1 has a special setup. eth0 has no connection to the outside and
 * gets an locally administred MAC address, eth1 is the debug interface and
 * gets the official MAC address from the IVM
 */
	process_mac(valbuf, page2, CONFIG_PIGGY_MAC_ADRESS_OFFSET, false);
	env_set((char *)"ethaddr", (char *)valbuf);
	process_mac(valbuf, page2, CONFIG_PIGGY_MAC_ADRESS_OFFSET, true);
	env_set((char *)"eth1addr", (char *)valbuf);
#endif

	return 0;
}

int ivm_read_eeprom(unsigned char *buf, int len)
{
	int ret;

	i2c_set_bus_num(CONFIG_KM_IVM_BUS);
	/* add deblocking here */
	i2c_make_abort();

	ret = i2c_read(CONFIG_SYS_IVM_EEPROM_ADR, 0, 1, buf, len);
	if (ret != 0) {
		printf("Error reading EEprom\n");
		return -2;
	}

	return ivm_populate_env(buf, len);
}

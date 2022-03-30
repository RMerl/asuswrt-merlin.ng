// SPDX-License-Identifier: GPL-2.0+
/*
 *
 * Read FactorySet information from EEPROM into global structure.
 * (C) Copyright 2013 Siemens Schweiz AG
 */

#if !defined(CONFIG_SPL_BUILD)

#include <common.h>
#include <environment.h>
#include <i2c.h>
#include <asm/io.h>
#include <asm/arch/cpu.h>
#include <asm/arch/sys_proto.h>
#include <asm/unaligned.h>
#include <net.h>
#include <errno.h>
#include <g_dnl.h>
#include "factoryset.h"

#define EEPR_PG_SZ		0x80
#define EEPROM_FATORYSET_OFFSET	0x400
#define OFF_PG            EEPROM_FATORYSET_OFFSET/EEPR_PG_SZ

/* Global variable that contains necessary information from FactorySet */
struct factorysetcontainer factory_dat;

#define fact_get_char(i) *((char *)&eeprom_buf[i])

static int fact_match(unsigned char *eeprom_buf, uchar *s1, int i2)
{
	if (s1 == NULL)
		return -1;

	while (*s1 == fact_get_char(i2++))
		if (*s1++ == '=')
			return i2;

	if (*s1 == '\0' && fact_get_char(i2-1) == '=')
		return i2;

	return -1;
}

static int get_factory_val(unsigned char *eeprom_buf, int size, uchar *name,
			uchar *buf, int len)
{
	int i, nxt = 0;

	for (i = 0; fact_get_char(i) != '\0'; i = nxt + 1) {
		int val, n;

		for (nxt = i; fact_get_char(nxt) != '\0'; ++nxt) {
			if (nxt >= size)
				return -1;
		}

		val = fact_match(eeprom_buf, (uchar *)name, i);
		if (val < 0)
			continue;

		/* found; copy out */
		for (n = 0; n < len; ++n, ++buf) {
			*buf = fact_get_char(val++);
			if (*buf == '\0')
				return n;
		}

		if (n)
			*--buf = '\0';

		printf("env_buf [%d bytes] too small for value of \"%s\"\n",
		       len, name);

		return n;
	}
	return -1;
}

static
int get_factory_record_val(unsigned char *eeprom_buf, int size,	uchar *record,
	uchar *name, uchar *buf, int len)
{
	int ret = -1;
	int i, nxt = 0;
	int c;
	unsigned char end = 0xff;
	unsigned char tmp;

	for (i = 0; fact_get_char(i) != end; i = nxt) {
		nxt = i + 1;
		if (fact_get_char(i) == '>') {
			int pos;
			int endpos;
			int z;
			int level = 0;

			c = strncmp((char *)&eeprom_buf[i + 1], (char *)record,
				    strlen((char *)record));
			if (c == 0) {
				/* record found */
				pos = i + strlen((char *)record) + 2;
				nxt = pos;
				/* search for "<" */
				c = -1;
				for (z = pos; fact_get_char(z) != end; z++) {
					if (fact_get_char(z) == '<') {
						if (level == 0) {
							endpos = z;
							nxt = endpos;
							c = 0;
							break;
						} else {
							level--;
						}
					}
					if (fact_get_char(z) == '>')
						level++;
				}
			} else {
				continue;
			}
			if (c == 0) {
				/* end found -> call get_factory_val */
				tmp = eeprom_buf[endpos];
				eeprom_buf[endpos] = end;
				ret = get_factory_val(&eeprom_buf[pos],
					endpos - pos, name, buf, len);
				/* fix buffer */
				eeprom_buf[endpos] = tmp;
				debug("%s: %s.%s = %s\n",
				      __func__, record, name, buf);
				return ret;
			}
		}
	}
	return ret;
}

int factoryset_read_eeprom(int i2c_addr)
{
	int i, pages = 0, size = 0;
	unsigned char eeprom_buf[0x3c00], hdr[4], buf[MAX_STRING_LENGTH];
	unsigned char *cp, *cp1;

#if defined(CONFIG_DFU_OVER_USB)
	factory_dat.usb_vendor_id = CONFIG_USB_GADGET_VENDOR_NUM;
	factory_dat.usb_product_id = CONFIG_USB_GADGET_PRODUCT_NUM;
#endif
	if (i2c_probe(i2c_addr))
		goto err;

	if (i2c_read(i2c_addr, EEPROM_FATORYSET_OFFSET, 2, hdr, sizeof(hdr)))
		goto err;

	if ((hdr[0] != 0x99) || (hdr[1] != 0x80)) {
		printf("FactorySet is not right in eeprom.\n");
		return 1;
	}

	/* get FactorySet size */
	size = (hdr[2] << 8) + hdr[3] + sizeof(hdr);
	if (size > 0x3bfa)
		size = 0x3bfa;

	pages = size / EEPR_PG_SZ;

	/*
	 * read the eeprom using i2c
	 * I can not read entire eeprom in once, so separate into several
	 * times. Furthermore, fetch eeprom take longer time, so we fetch
	 * data after every time we got a record from eeprom
	 */
	debug("Read eeprom page :\n");
	for (i = 0; i < pages; i++)
		if (i2c_read(i2c_addr, (OFF_PG + i) * EEPR_PG_SZ, 2,
			     eeprom_buf + (i * EEPR_PG_SZ), EEPR_PG_SZ))
			goto err;

	if (size % EEPR_PG_SZ)
		if (i2c_read(i2c_addr, (OFF_PG + pages) * EEPR_PG_SZ, 2,
			     eeprom_buf + (pages * EEPR_PG_SZ),
			     (size % EEPR_PG_SZ)))
			goto err;

	/* we do below just for eeprom align */
	for (i = 0; i < size; i++)
		if (eeprom_buf[i] == '\n')
			eeprom_buf[i] = 0;

	/* skip header */
	size -= sizeof(hdr);
	cp = (uchar *)eeprom_buf + sizeof(hdr);

	/* get mac address */
	get_factory_record_val(cp, size, (uchar *)"ETH1", (uchar *)"mac",
			       buf, MAX_STRING_LENGTH);
	cp1 = buf;
	for (i = 0; i < 6; i++) {
		factory_dat.mac[i] = simple_strtoul((char *)cp1, NULL, 16);
		cp1 += 3;
	}

#if defined(CONFIG_DFU_OVER_USB)
	/* read vid and pid for dfu mode */
	if (0 <= get_factory_record_val(cp, size, (uchar *)"USBD1",
					(uchar *)"vid", buf,
					MAX_STRING_LENGTH)) {
		factory_dat.usb_vendor_id = simple_strtoul((char *)buf,
							   NULL, 16);
	}

	if (0 <= get_factory_record_val(cp, size, (uchar *)"USBD1",
					(uchar *)"pid", buf,
					MAX_STRING_LENGTH)) {
		factory_dat.usb_product_id = simple_strtoul((char *)buf,
							    NULL, 16);
	}
	printf("DFU USB: VID = 0x%4x, PID = 0x%4x\n", factory_dat.usb_vendor_id,
	       factory_dat.usb_product_id);
#endif
#if defined(CONFIG_VIDEO)
	if (0 <= get_factory_record_val(cp, size, (uchar *)"DISP1",
					(uchar *)"name", factory_dat.disp_name,
					MAX_STRING_LENGTH)) {
		debug("display name: %s\n", factory_dat.disp_name);
	}
#endif
	if (0 <= get_factory_record_val(cp, size, (uchar *)"DEV",
					(uchar *)"num", factory_dat.serial,
					MAX_STRING_LENGTH)) {
		debug("serial number: %s\n", factory_dat.serial);
	}
	if (0 <= get_factory_record_val(cp, size, (uchar *)"DEV",
					(uchar *)"ver", buf,
					MAX_STRING_LENGTH)) {
		factory_dat.version = simple_strtoul((char *)buf,
							    NULL, 16);
		debug("version number: %d\n", factory_dat.version);
	}
	/* Get ASN from factory set if available */
	if (0 <= get_factory_record_val(cp, size, (uchar *)"DEV",
					(uchar *)"id", factory_dat.asn,
					MAX_STRING_LENGTH)) {
		debug("factoryset asn: %s\n", factory_dat.asn);
	} else {
		factory_dat.asn[0] = 0;
	}
	/* Get COMP/ver from factory set if available */
	if (0 <= get_factory_record_val(cp, size, (uchar *)"COMP",
					(uchar *)"ver",
					factory_dat.comp_version,
					MAX_STRING_LENGTH)) {
		debug("factoryset COMP/ver: %s\n", factory_dat.comp_version);
	} else {
		strcpy((char *)factory_dat.comp_version, "1.0");
	}

	return 0;

err:
	printf("Could not read the EEPROM; something fundamentally wrong on the I2C bus.\n");
	return 1;
}

static struct ctrl_dev *cdev = (struct ctrl_dev *)CTRL_DEVICE_BASE;

static int factoryset_mac_env_set(void)
{
	uint8_t mac_addr[6];

	debug("FactorySet: Set mac address\n");
	if (is_valid_ethaddr(factory_dat.mac)) {
		memcpy(mac_addr, factory_dat.mac, 6);
	} else {
		uint32_t mac_hi, mac_lo;

		debug("Warning: FactorySet: <ethaddr> not set. Fallback to E-fuse\n");
		mac_lo = readl(&cdev->macid0l);
		mac_hi = readl(&cdev->macid0h);

		mac_addr[0] = mac_hi & 0xFF;
		mac_addr[1] = (mac_hi & 0xFF00) >> 8;
		mac_addr[2] = (mac_hi & 0xFF0000) >> 16;
		mac_addr[3] = (mac_hi & 0xFF000000) >> 24;
		mac_addr[4] = mac_lo & 0xFF;
		mac_addr[5] = (mac_lo & 0xFF00) >> 8;
		if (!is_valid_ethaddr(mac_addr)) {
			printf("Warning: ethaddr not set by FactorySet or E-fuse. Set <ethaddr> variable to overcome this.\n");
			return -1;
		}
	}

	eth_env_set_enetaddr("ethaddr", mac_addr);
	return 0;
}

int factoryset_env_set(void)
{
	int ret = 0;

	if (factoryset_mac_env_set() < 0)
		ret = -1;

	return ret;
}

int g_dnl_bind_fixup(struct usb_device_descriptor *dev, const char *name)
{
	put_unaligned(factory_dat.usb_vendor_id, &dev->idVendor);
	put_unaligned(factory_dat.usb_product_id, &dev->idProduct);
	g_dnl_set_serialnumber((char *)factory_dat.serial);

	return 0;
}

int g_dnl_get_board_bcd_device_number(int gcnum)
{
	return factory_dat.version;
}
#endif /* defined(CONFIG_SPL_BUILD) */

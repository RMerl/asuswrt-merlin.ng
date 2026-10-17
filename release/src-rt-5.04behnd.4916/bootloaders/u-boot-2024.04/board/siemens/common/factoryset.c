// SPDX-License-Identifier: GPL-2.0+
/*
 *
 * Read FactorySet information from EEPROM into global structure.
 * (C) Copyright 2013 Siemens Schweiz AG
 */

#if !defined(CONFIG_SPL_BUILD)

#include <env.h>
#include <g_dnl.h>
#include <net.h>
#include <asm/io.h>
#if CONFIG_IS_ENABLED(AM33XX)
#include <asm/arch/cpu.h>
#endif
#include "eeprom.h"
#include "factoryset.h"

#define EEPR_PG_SZ	0x80
#define OFF_PG		(SIEMENS_EE_ADDR_FACTORYSET / EEPR_PG_SZ)

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

	if (siemens_ee_read_data(SIEMENS_EE_ADDR_FACTORYSET, hdr, sizeof(hdr)))
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
		if (siemens_ee_read_data((OFF_PG + i) * EEPR_PG_SZ,
					 eeprom_buf + (i * EEPR_PG_SZ), EEPR_PG_SZ))
			goto err;

	if (size % EEPR_PG_SZ)
		if (siemens_ee_read_data((OFF_PG + pages) * EEPR_PG_SZ,
					 eeprom_buf + (pages * EEPR_PG_SZ),
					 size % EEPR_PG_SZ))
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
		factory_dat.mac[i] = hextoul((char *)cp1, NULL);
		cp1 += 3;
	}

#if CONFIG_IS_ENABLED(IMX8)
	/* get mac address for WLAN */
	if (get_factory_record_val(cp, size, (uchar *)"WLAN1", (uchar *)"mac",
				   buf, MAX_STRING_LENGTH) > 0) {
		cp1 = buf;
		for (i = 0; i < 6; i++) {
			factory_dat.mac_wlan[i] = hextoul((char *)cp1, NULL);
			cp1 += 3;
		}
	}
#endif

#if defined(CONFIG_DFU_OVER_USB)
	/* read vid and pid for dfu mode */
	if (0 <= get_factory_record_val(cp, size, (uchar *)"USBD1",
					(uchar *)"vid", buf,
					MAX_STRING_LENGTH)) {
		factory_dat.usb_vendor_id = hextoul((char *)buf, NULL);
	}

	if (0 <= get_factory_record_val(cp, size, (uchar *)"USBD1",
					(uchar *)"pid", buf,
					MAX_STRING_LENGTH)) {
		factory_dat.usb_product_id = hextoul((char *)buf, NULL);
	}
	printf("DFU USB: VID = 0x%4x, PID = 0x%4x\n", factory_dat.usb_vendor_id,
	       factory_dat.usb_product_id);
#endif
	if (0 <= get_factory_record_val(cp, size, (uchar *)"DEV",
					(uchar *)"num", factory_dat.serial,
					MAX_STRING_LENGTH)) {
		debug("serial number: %s\n", factory_dat.serial);
	}
	if (0 <= get_factory_record_val(cp, size, (uchar *)"DEV",
					(uchar *)"ver", buf,
					MAX_STRING_LENGTH)) {
		factory_dat.version = hextoul((char *)buf, NULL);
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

static int get_mac_from_efuse(uint8_t mac[6])
{
#ifdef CONFIG_AM33XX
	struct ctrl_dev *cdev = (struct ctrl_dev *)CTRL_DEVICE_BASE;
	uint32_t mac_hi, mac_lo;

	mac_lo = readl(&cdev->macid0l);
	mac_hi = readl(&cdev->macid0h);

	mac[0] = mac_hi & 0xFF;
	mac[1] = (mac_hi & 0xFF00) >> 8;
	mac[2] = (mac_hi & 0xFF0000) >> 16;
	mac[3] = (mac_hi & 0xFF000000) >> 24;
	mac[4] = mac_lo & 0xFF;
	mac[5] = (mac_lo & 0xFF00) >> 8;
#else
	/* unhandled */
	memset(mac, 0, 6);
#endif
	if (!is_valid_ethaddr(mac)) {
		puts("Warning: ethaddr not set by FactorySet or E-fuse. ");
		puts("Set <ethaddr> variable to overcome this.\n");
		return -1;
	}
	return 0;
}

static int factoryset_mac_env_set(void)
{
	uint8_t mac_addr[6];

	/* Set mac from factoryset or try reading E-fuse */
	debug("FactorySet: Set mac address\n");
	if (is_valid_ethaddr(factory_dat.mac)) {
		memcpy(mac_addr, factory_dat.mac, 6);
	} else {
		debug("Warning: FactorySet: <ethaddr> not set. Fallback to E-fuse\n");
		if (get_mac_from_efuse(mac_addr) < 0)
			return -1;
	}

	eth_env_set_enetaddr("ethaddr", mac_addr);

#if CONFIG_IS_ENABLED(IMX8)
	eth_env_set_enetaddr("eth1addr", mac_addr);

	/* wlan mac */
	if (is_valid_ethaddr(factory_dat.mac_wlan))
		eth_env_set_enetaddr("eth2addr", factory_dat.mac_wlan);
#endif
	return 0;
}

static void factoryset_dtb_env_set(void)
{
	/* Set ASN in environment*/
	if (factory_dat.asn[0] != 0) {
		env_set("dtb_name", (char *)factory_dat.asn);
	} else {
		/* dtb suffix gets added in load script */
		env_set("dtb_name", "default");
	}
}

int factoryset_env_set(void)
{
	int ret = 0;

	factoryset_dtb_env_set();

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

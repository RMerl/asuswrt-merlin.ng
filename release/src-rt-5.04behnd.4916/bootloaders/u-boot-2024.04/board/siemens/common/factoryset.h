/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * Common board functions for siemens AM335X based boards
 * (C) Copyright 2013 Siemens Schweiz AG
 */

#ifndef __FACTORYSET_H
#define __FACTORYSET_H

#define MAX_STRING_LENGTH	32

struct factorysetcontainer {
	uchar mac[6];
#if CONFIG_IS_ENABLED(IMX8)
	uchar mac_wlan[6];
#endif
	int usb_vendor_id;
	int usb_product_id;
	int pxm50;
	unsigned char serial[MAX_STRING_LENGTH];
	int version;
	uchar asn[MAX_STRING_LENGTH];
	uchar comp_version[MAX_STRING_LENGTH];
};

int factoryset_read_eeprom(int i2c_addr);
int factoryset_env_set(void);
extern struct factorysetcontainer factory_dat;

#endif /* __FACTORYSET_H */

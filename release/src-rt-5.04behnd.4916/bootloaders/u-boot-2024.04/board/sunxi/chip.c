// SPDX-License-Identifier: GPL-2.0+
/*
 * (C) Copyright 2021
 * Köry Maincent, Bootlin, <kory.maincent@bootlin.com>
 * Based on initial code from Maxime Ripard
 */

#include <common.h>
#include <malloc.h>
#include <dm.h>
#include <w1.h>
#include <w1-eeprom.h>
#include <dm/device-internal.h>

#include <sunxi_gpio.h>

#include <extension_board.h>

#define for_each_w1_device(b, d) \
	for (device_find_first_child(b, d); *d; device_find_next_child(d))

#define dip_convert(field)						\
	(								\
		(sizeof(field) == 1) ? field :				\
		(sizeof(field) == 2) ? be16_to_cpu(field) :		\
		(sizeof(field) == 4) ? be32_to_cpu(field) :		\
		-1							\
	)

#define DIP_MAGIC	0x50494843	/* CHIP */

struct dip_w1_header {
	u32     magic;                  /* CHIP */
	u8      version;                /* spec version */
	u32     vendor_id;
	u16     product_id;
	u8      product_version;
	char    vendor_name[32];
	char    product_name[32];
	u8      rsvd[36];               /* rsvd for future spec versions */
	u8      data[16];               /* user data, per-dip specific */
} __packed;

int extension_board_scan(struct list_head *extension_list)
{
	struct extension *dip;
	struct dip_w1_header w1_header;
	struct udevice *bus, *dev;
	u32 vid;
	u16 pid;
	int ret;

	int num_dip = 0;

	sunxi_gpio_set_pull(SUNXI_GPD(2), SUNXI_GPIO_PULL_UP);

	ret = w1_get_bus(0, &bus);
	if (ret) {
		printf("one wire interface not found\n");
		return 0;
	}

	for_each_w1_device(bus, &dev) {
		if (w1_get_device_family(dev) != W1_FAMILY_DS2431)
			continue;

		ret = device_probe(dev);
		if (ret) {
			printf("Couldn't probe device %s: error %d",
			       dev->name, ret);
			continue;
		}

		w1_eeprom_read_buf(dev, 0, (u8 *)&w1_header, sizeof(w1_header));

		if (w1_header.magic != DIP_MAGIC)
			continue;

		vid = dip_convert(w1_header.vendor_id);
		pid = dip_convert(w1_header.product_id);

		printf("DIP: %s (0x%x) from %s (0x%x)\n",
		       w1_header.product_name, pid,
		       w1_header.vendor_name, vid);

		dip = calloc(1, sizeof(struct extension));
		if (!dip) {
			printf("Error in memory allocation\n");
			return num_dip;
		}

		snprintf(dip->overlay, sizeof(dip->overlay), "dip-%x-%x.dtbo",
			 vid, pid);
		strncpy(dip->name, w1_header.product_name, 32);
		strncpy(dip->owner, w1_header.vendor_name, 32);
		list_add_tail(&dip->list, extension_list);
		num_dip++;
	}
	return num_dip;
}

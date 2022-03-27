/*
 *
 *  BlueZ - Bluetooth protocol stack for Linux
 *
 *  Copyright (C) 2004-2010  Marcel Holtmann <marcel@holtmann.org>
 *
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
 *
 */

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include <stdlib.h>
#include <stdio.h>
#include <stdbool.h>
#include <errno.h>
#include <arpa/inet.h>

#include "lib/bluetooth.h"
#include "lib/sdp.h"
#include "lib/sdp_lib.h"

#include "uuid-helper.h"

char *bt_modalias(uint16_t source, uint16_t vendor,
					uint16_t product, uint16_t version)
{
	char *str;
	int err;

	switch (source) {
	case 0x0001:
		err = asprintf(&str, "%s:v%04Xp%04Xd%04X",
					"bluetooth", vendor, product, version);
		break;
	case 0x0002:
		err = asprintf(&str, "%s:v%04Xp%04Xd%04X",
					"usb", vendor, product, version);
		break;
	default:
		return NULL;
	}

	if (err < 0)
		return NULL;

	return str;
}

char *bt_uuid2string(uuid_t *uuid)
{
	char *str;
	uuid_t uuid128;
	unsigned int data0;
	unsigned short data1;
	unsigned short data2;
	unsigned short data3;
	unsigned int data4;
	unsigned short data5;
	int err;

	if (!uuid)
		return NULL;

	switch (uuid->type) {
	case SDP_UUID16:
		sdp_uuid16_to_uuid128(&uuid128, uuid);
		break;
	case SDP_UUID32:
		sdp_uuid32_to_uuid128(&uuid128, uuid);
		break;
	case SDP_UUID128:
		memcpy(&uuid128, uuid, sizeof(uuid_t));
		break;
	default:
		/* Type of UUID unknown */
		return NULL;
	}

	memcpy(&data0, &uuid128.value.uuid128.data[0], 4);
	memcpy(&data1, &uuid128.value.uuid128.data[4], 2);
	memcpy(&data2, &uuid128.value.uuid128.data[6], 2);
	memcpy(&data3, &uuid128.value.uuid128.data[8], 2);
	memcpy(&data4, &uuid128.value.uuid128.data[10], 4);
	memcpy(&data5, &uuid128.value.uuid128.data[14], 2);

	err = asprintf(&str, "%.8x-%.4x-%.4x-%.4x-%.8x%.4x",
			ntohl(data0), ntohs(data1),
			ntohs(data2), ntohs(data3),
			ntohl(data4), ntohs(data5));
	if (err < 0)
		return NULL;

	return str;
}

static struct {
	const char	*name;
	uint16_t	class;
} bt_services[] = {
	{ "pbap",	PBAP_SVCLASS_ID			},
	{ "sap",	SAP_SVCLASS_ID			},
	{ "ftp",	OBEX_FILETRANS_SVCLASS_ID	},
	{ "bpp",	BASIC_PRINTING_SVCLASS_ID	},
	{ "bip",	IMAGING_SVCLASS_ID		},
	{ "synch",	IRMC_SYNC_SVCLASS_ID		},
	{ "dun",	DIALUP_NET_SVCLASS_ID		},
	{ "opp",	OBEX_OBJPUSH_SVCLASS_ID		},
	{ "fax",	FAX_SVCLASS_ID			},
	{ "spp",	SERIAL_PORT_SVCLASS_ID		},
	{ "hsp",	HEADSET_SVCLASS_ID		},
	{ "hsp-hs",	HEADSET_SVCLASS_ID		},
	{ "hsp-ag",	HEADSET_AGW_SVCLASS_ID		},
	{ "hfp",	HANDSFREE_SVCLASS_ID		},
	{ "hfp-hf",	HANDSFREE_SVCLASS_ID		},
	{ "hfp-ag",	HANDSFREE_AGW_SVCLASS_ID	},
	{ "pbap-pce",	PBAP_PCE_SVCLASS_ID		},
	{ "pbap-pse",	PBAP_PSE_SVCLASS_ID		},
	{ "map-mse",	MAP_MSE_SVCLASS_ID		},
	{ "map-mas",	MAP_MSE_SVCLASS_ID		},
	{ "map-mce",	MAP_MCE_SVCLASS_ID		},
	{ "map-mns",	MAP_MCE_SVCLASS_ID		},
	{ "gnss",	GNSS_SERVER_SVCLASS_ID		},
	{ }
};

static uint16_t name2class(const char *pattern)
{
	int i;

	for (i = 0; bt_services[i].name; i++) {
		if (strcasecmp(bt_services[i].name, pattern) == 0)
			return bt_services[i].class;
	}

	return 0;
}

static inline bool is_uuid128(const char *string)
{
	return (strlen(string) == 36 &&
			string[8] == '-' &&
			string[13] == '-' &&
			string[18] == '-' &&
			string[23] == '-');
}

static int string2uuid16(uuid_t *uuid, const char *string)
{
	int length = strlen(string);
	char *endptr = NULL;
	uint16_t u16;

	if (length != 4 && length != 6)
		return -EINVAL;

	u16 = strtol(string, &endptr, 16);
	if (endptr && *endptr == '\0') {
		sdp_uuid16_create(uuid, u16);
		return 0;
	}

	return -EINVAL;
}

char *bt_name2string(const char *pattern)
{
	uuid_t uuid;
	uint16_t uuid16;
	int i;

	/* UUID 128 string format */
	if (is_uuid128(pattern))
		return strdup(pattern);

	/* Friendly service name format */
	uuid16 = name2class(pattern);
	if (uuid16)
		goto proceed;

	/* HEX format */
	uuid16 = strtol(pattern, NULL, 16);
	for (i = 0; bt_services[i].class; i++) {
		if (bt_services[i].class == uuid16)
			goto proceed;
	}

	return NULL;

proceed:
	sdp_uuid16_create(&uuid, uuid16);

	return bt_uuid2string(&uuid);
}

int bt_string2uuid(uuid_t *uuid, const char *string)
{
	uint32_t data0, data4;
	uint16_t data1, data2, data3, data5;

	if (is_uuid128(string) &&
			sscanf(string, "%08x-%04hx-%04hx-%04hx-%08x%04hx",
				&data0, &data1, &data2, &data3, &data4, &data5) == 6) {
		uint8_t val[16];

		data0 = htonl(data0);
		data1 = htons(data1);
		data2 = htons(data2);
		data3 = htons(data3);
		data4 = htonl(data4);
		data5 = htons(data5);

		memcpy(&val[0], &data0, 4);
		memcpy(&val[4], &data1, 2);
		memcpy(&val[6], &data2, 2);
		memcpy(&val[8], &data3, 2);
		memcpy(&val[10], &data4, 4);
		memcpy(&val[14], &data5, 2);

		sdp_uuid128_create(uuid, val);

		return 0;
	} else {
		uint16_t class = name2class(string);
		if (class) {
			sdp_uuid16_create(uuid, class);
			return 0;
		}

		return string2uuid16(uuid, string);
	}
}

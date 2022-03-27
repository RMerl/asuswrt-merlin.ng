/*
 *
 *  BlueZ - Bluetooth protocol stack for Linux
 *
 *  Copyright (C) 2011  Nokia Corporation
 *  Copyright (C) 2011  Marcel Holtmann <marcel@holtmann.org>
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

#include <errno.h>
#include <ctype.h>
#include <stdlib.h>
#include <stdint.h>
#include <stdbool.h>
#include <glib.h>

#include "lib/bluetooth.h"
#include "lib/hci.h"
#include "lib/sdp.h"

#include "src/shared/util.h"
#include "uuid-helper.h"
#include "eir.h"

#define EIR_OOB_MIN (2 + 6)

static void sd_free(void *data)
{
	struct eir_sd *sd = data;

	free(sd->uuid);
	g_free(sd);
}

void eir_data_free(struct eir_data *eir)
{
	g_slist_free_full(eir->services, free);
	eir->services = NULL;
	g_free(eir->name);
	eir->name = NULL;
	g_free(eir->hash);
	eir->hash = NULL;
	g_free(eir->randomizer);
	eir->randomizer = NULL;
	g_slist_free_full(eir->msd_list, g_free);
	eir->msd_list = NULL;
	g_slist_free_full(eir->sd_list, sd_free);
	eir->sd_list = NULL;
}

static void eir_parse_uuid16(struct eir_data *eir, const void *data,
								uint8_t len)
{
	const uint16_t *uuid16 = data;
	uuid_t service;
	char *uuid_str;
	unsigned int i;

	service.type = SDP_UUID16;
	for (i = 0; i < len / 2; i++, uuid16++) {
		service.value.uuid16 = get_le16(uuid16);

		uuid_str = bt_uuid2string(&service);
		if (!uuid_str)
			continue;
		eir->services = g_slist_append(eir->services, uuid_str);
	}
}

static void eir_parse_uuid32(struct eir_data *eir, const void *data,
								uint8_t len)
{
	const uint32_t *uuid32 = data;
	uuid_t service;
	char *uuid_str;
	unsigned int i;

	service.type = SDP_UUID32;
	for (i = 0; i < len / 4; i++, uuid32++) {
		service.value.uuid32 = get_le32(uuid32);

		uuid_str = bt_uuid2string(&service);
		if (!uuid_str)
			continue;
		eir->services = g_slist_append(eir->services, uuid_str);
	}
}

static void eir_parse_uuid128(struct eir_data *eir, const uint8_t *data,
								uint8_t len)
{
	const uint8_t *uuid_ptr = data;
	uuid_t service;
	char *uuid_str;
	unsigned int i;
	int k;

	service.type = SDP_UUID128;
	for (i = 0; i < len / 16; i++) {
		for (k = 0; k < 16; k++)
			service.value.uuid128.data[k] = uuid_ptr[16 - k - 1];
		uuid_str = bt_uuid2string(&service);
		if (!uuid_str)
			continue;
		eir->services = g_slist_append(eir->services, uuid_str);
		uuid_ptr += 16;
	}
}

static char *name2utf8(const uint8_t *name, uint8_t len)
{
	char utf8_name[HCI_MAX_NAME_LENGTH + 2];
	int i;

	if (g_utf8_validate((const char *) name, len, NULL))
		return g_strndup((char *) name, len);

	memset(utf8_name, 0, sizeof(utf8_name));
	strncpy(utf8_name, (char *) name, len);

	/* Assume ASCII, and replace all non-ASCII with spaces */
	for (i = 0; utf8_name[i] != '\0'; i++) {
		if (!isascii(utf8_name[i]))
			utf8_name[i] = ' ';
	}

	/* Remove leading and trailing whitespace characters */
	g_strstrip(utf8_name);

	return g_strdup(utf8_name);
}

static void eir_parse_msd(struct eir_data *eir, const uint8_t *data,
								uint8_t len)
{
	struct eir_msd *msd;

	if (len < 2 || len > 2 + sizeof(msd->data))
		return;

	msd = g_malloc(sizeof(*msd));
	msd->company = get_le16(data);
	msd->data_len = len - 2;
	memcpy(&msd->data, data + 2, msd->data_len);

	eir->msd_list = g_slist_append(eir->msd_list, msd);
}

static void eir_parse_sd(struct eir_data *eir, uuid_t *service,
					const uint8_t *data, uint8_t len)
{
	struct eir_sd *sd;
	char *uuid;

	uuid = bt_uuid2string(service);
	if (!uuid)
		return;

	sd = g_malloc(sizeof(*sd));
	sd->uuid = uuid;
	sd->data_len = len;
	memcpy(&sd->data, data, sd->data_len);

	eir->sd_list = g_slist_append(eir->sd_list, sd);
}

static void eir_parse_uuid16_data(struct eir_data *eir, const uint8_t *data,
								uint8_t len)
{
	uuid_t service;

	if (len < 2 || len > EIR_SD_MAX_LEN)
		return;

	service.type = SDP_UUID16;
	service.value.uuid16 = get_le16(data);
	eir_parse_sd(eir, &service, data + 2, len - 2);
}

static void eir_parse_uuid32_data(struct eir_data *eir, const uint8_t *data,
								uint8_t len)
{
	uuid_t service;

	if (len < 4 || len > EIR_SD_MAX_LEN)
		return;

	service.type = SDP_UUID32;
	service.value.uuid32 = get_le32(data);
	eir_parse_sd(eir, &service, data + 4, len - 4);
}

static void eir_parse_uuid128_data(struct eir_data *eir, const uint8_t *data,
								uint8_t len)
{
	uuid_t service;
	int k;

	if (len < 16 || len > EIR_SD_MAX_LEN)
		return;

	service.type = SDP_UUID128;

	for (k = 0; k < 16; k++)
		service.value.uuid128.data[k] = data[16 - k - 1];

	eir_parse_sd(eir, &service, data + 16, len - 16);
}

void eir_parse(struct eir_data *eir, const uint8_t *eir_data, uint8_t eir_len)
{
	uint16_t len = 0;

	eir->flags = 0;
	eir->tx_power = 127;

	/* No EIR data to parse */
	if (eir_data == NULL)
		return;

	while (len < eir_len - 1) {
		uint8_t field_len = eir_data[0];
		const uint8_t *data;
		uint8_t data_len;

		/* Check for the end of EIR */
		if (field_len == 0)
			break;

		len += field_len + 1;

		/* Do not continue EIR Data parsing if got incorrect length */
		if (len > eir_len)
			break;

		data = &eir_data[2];
		data_len = field_len - 1;

		switch (eir_data[1]) {
		case EIR_UUID16_SOME:
		case EIR_UUID16_ALL:
			eir_parse_uuid16(eir, data, data_len);
			break;

		case EIR_UUID32_SOME:
		case EIR_UUID32_ALL:
			eir_parse_uuid32(eir, data, data_len);
			break;

		case EIR_UUID128_SOME:
		case EIR_UUID128_ALL:
			eir_parse_uuid128(eir, data, data_len);
			break;

		case EIR_FLAGS:
			if (data_len > 0)
				eir->flags = *data;
			break;

		case EIR_NAME_SHORT:
		case EIR_NAME_COMPLETE:
			/* Some vendors put a NUL byte terminator into
			 * the name */
			while (data_len > 0 && data[data_len - 1] == '\0')
				data_len--;

			g_free(eir->name);

			eir->name = name2utf8(data, data_len);
			eir->name_complete = eir_data[1] == EIR_NAME_COMPLETE;
			break;

		case EIR_TX_POWER:
			if (data_len < 1)
				break;
			eir->tx_power = (int8_t) data[0];
			break;

		case EIR_CLASS_OF_DEV:
			if (data_len < 3)
				break;
			eir->class = data[0] | (data[1] << 8) |
							(data[2] << 16);
			break;

		case EIR_GAP_APPEARANCE:
			if (data_len < 2)
				break;
			eir->appearance = get_le16(data);
			break;

		case EIR_SSP_HASH:
			if (data_len < 16)
				break;
			eir->hash = g_memdup(data, 16);
			break;

		case EIR_SSP_RANDOMIZER:
			if (data_len < 16)
				break;
			eir->randomizer = g_memdup(data, 16);
			break;

		case EIR_DEVICE_ID:
			if (data_len < 8)
				break;

			eir->did_source = data[0] | (data[1] << 8);
			eir->did_vendor = data[2] | (data[3] << 8);
			eir->did_product = data[4] | (data[5] << 8);
			eir->did_version = data[6] | (data[7] << 8);
			break;

		case EIR_SVC_DATA16:
			eir_parse_uuid16_data(eir, data, data_len);
			break;

		case EIR_SVC_DATA32:
			eir_parse_uuid32_data(eir, data, data_len);
			break;

		case EIR_SVC_DATA128:
			eir_parse_uuid128_data(eir, data, data_len);
			break;

		case EIR_MANUFACTURER_DATA:
			eir_parse_msd(eir, data, data_len);
			break;

		}

		eir_data += field_len + 1;
	}
}

int eir_parse_oob(struct eir_data *eir, uint8_t *eir_data, uint16_t eir_len)
{

	if (eir_len < EIR_OOB_MIN)
		return -1;

	if (eir_len != get_le16(eir_data))
		return -1;

	eir_data += sizeof(uint16_t);
	eir_len -= sizeof(uint16_t);

	memcpy(&eir->addr, eir_data, sizeof(bdaddr_t));
	eir_data += sizeof(bdaddr_t);
	eir_len -= sizeof(bdaddr_t);

	/* optional OOB EIR data */
	if (eir_len > 0)
		eir_parse(eir, eir_data, eir_len);

	return 0;
}

#define SIZEOF_UUID128 16

static void eir_generate_uuid128(sdp_list_t *list, uint8_t *ptr,
							uint16_t *eir_len)
{
	int i, k, uuid_count = 0;
	uint16_t len = *eir_len;
	uint8_t *uuid128;
	bool truncated = false;

	/* Store UUIDs in place, skip 2 bytes to write type and length later */
	uuid128 = ptr + 2;

	for (; list; list = list->next) {
		sdp_record_t *rec = list->data;
		uuid_t *uuid = &rec->svclass;
		uint8_t *uuid128_data = uuid->value.uuid128.data;

		if (uuid->type != SDP_UUID128)
			continue;

		/* Stop if not enough space to put next UUID128 */
		if ((len + 2 + SIZEOF_UUID128) > HCI_MAX_EIR_LENGTH) {
			truncated = true;
			break;
		}

		/* Check for duplicates, EIR data is Little Endian */
		for (i = 0; i < uuid_count; i++) {
			for (k = 0; k < SIZEOF_UUID128; k++) {
				if (uuid128[i * SIZEOF_UUID128 + k] !=
					uuid128_data[SIZEOF_UUID128 - 1 - k])
					break;
			}
			if (k == SIZEOF_UUID128)
				break;
		}

		if (i < uuid_count)
			continue;

		/* EIR data is Little Endian */
		for (k = 0; k < SIZEOF_UUID128; k++)
			uuid128[uuid_count * SIZEOF_UUID128 + k] =
				uuid128_data[SIZEOF_UUID128 - 1 - k];

		len += SIZEOF_UUID128;
		uuid_count++;
	}

	if (uuid_count > 0 || truncated) {
		/* EIR Data length */
		ptr[0] = (uuid_count * SIZEOF_UUID128) + 1;
		/* EIR Data type */
		ptr[1] = truncated ? EIR_UUID128_SOME : EIR_UUID128_ALL;
		len += 2;
		*eir_len = len;
	}
}

int eir_create_oob(const bdaddr_t *addr, const char *name, uint32_t cod,
			const uint8_t *hash, const uint8_t *randomizer,
			uint16_t did_vendor, uint16_t did_product,
			uint16_t did_version, uint16_t did_source,
			sdp_list_t *uuids, uint8_t *data)
{
	sdp_list_t *l;
	uint8_t *ptr = data;
	uint16_t eir_optional_len = 0;
	uint16_t eir_total_len;
	uint16_t uuid16[HCI_MAX_EIR_LENGTH / 2];
	int i, uuid_count = 0;
	bool truncated = false;
	size_t name_len;

	eir_total_len =  sizeof(uint16_t) + sizeof(bdaddr_t);
	ptr += sizeof(uint16_t);

	memcpy(ptr, addr, sizeof(bdaddr_t));
	ptr += sizeof(bdaddr_t);

	if (cod > 0) {
		uint8_t class[3];

		class[0] = (uint8_t) cod;
		class[1] = (uint8_t) (cod >> 8);
		class[2] = (uint8_t) (cod >> 16);

		*ptr++ = 4;
		*ptr++ = EIR_CLASS_OF_DEV;

		memcpy(ptr, class, sizeof(class));
		ptr += sizeof(class);

		eir_optional_len += sizeof(class) + 2;
	}

	if (hash) {
		*ptr++ = 17;
		*ptr++ = EIR_SSP_HASH;

		memcpy(ptr, hash, 16);
		ptr += 16;

		eir_optional_len += 16 + 2;
	}

	if (randomizer) {
		*ptr++ = 17;
		*ptr++ = EIR_SSP_RANDOMIZER;

		memcpy(ptr, randomizer, 16);
		ptr += 16;

		eir_optional_len += 16 + 2;
	}

	name_len = strlen(name);

	if (name_len > 0) {
		/* EIR Data type */
		if (name_len > 48) {
			name_len = 48;
			ptr[1] = EIR_NAME_SHORT;
		} else
			ptr[1] = EIR_NAME_COMPLETE;

		/* EIR Data length */
		ptr[0] = name_len + 1;

		memcpy(ptr + 2, name, name_len);

		eir_optional_len += (name_len + 2);
		ptr += (name_len + 2);
	}

	if (did_vendor != 0x0000) {
		*ptr++ = 9;
		*ptr++ = EIR_DEVICE_ID;
		*ptr++ = (did_source & 0x00ff);
		*ptr++ = (did_source & 0xff00) >> 8;
		*ptr++ = (did_vendor & 0x00ff);
		*ptr++ = (did_vendor & 0xff00) >> 8;
		*ptr++ = (did_product & 0x00ff);
		*ptr++ = (did_product & 0xff00) >> 8;
		*ptr++ = (did_version & 0x00ff);
		*ptr++ = (did_version & 0xff00) >> 8;
		eir_optional_len += 10;
	}

	/* Group all UUID16 types */
	for (l = uuids; l != NULL; l = l->next) {
		sdp_record_t *rec = l->data;
		uuid_t *uuid = &rec->svclass;

		if (uuid->type != SDP_UUID16)
			continue;

		if (uuid->value.uuid16 < 0x1100)
			continue;

		if (uuid->value.uuid16 == PNP_INFO_SVCLASS_ID)
			continue;

		/* Stop if not enough space to put next UUID16 */
		if ((eir_optional_len + 2 + sizeof(uint16_t)) >
				HCI_MAX_EIR_LENGTH) {
			truncated = true;
			break;
		}

		/* Check for duplicates */
		for (i = 0; i < uuid_count; i++)
			if (uuid16[i] == uuid->value.uuid16)
				break;

		if (i < uuid_count)
			continue;

		uuid16[uuid_count++] = uuid->value.uuid16;
		eir_optional_len += sizeof(uint16_t);
	}

	if (uuid_count > 0) {
		/* EIR Data length */
		ptr[0] = (uuid_count * sizeof(uint16_t)) + 1;
		/* EIR Data type */
		ptr[1] = truncated ? EIR_UUID16_SOME : EIR_UUID16_ALL;

		ptr += 2;
		eir_optional_len += 2;

		for (i = 0; i < uuid_count; i++) {
			*ptr++ = (uuid16[i] & 0x00ff);
			*ptr++ = (uuid16[i] & 0xff00) >> 8;
		}
	}

	/* Group all UUID128 types */
	if (eir_optional_len <= HCI_MAX_EIR_LENGTH - 2)
		eir_generate_uuid128(uuids, ptr, &eir_optional_len);

	eir_total_len += eir_optional_len;

	/* store total length */
	put_le16(eir_total_len, data);

	return eir_total_len;
}

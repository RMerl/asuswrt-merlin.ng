// SPDX-License-Identifier: GPL-2.0-or-later
/*
 *
 *  BlueZ - Bluetooth protocol stack for Linux
 *
 *  Copyright (C) 2011  Nokia Corporation
 *  Copyright (C) 2011  Marcel Holtmann <marcel@holtmann.org>
 *
 *
 */

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include <string.h>
#include <stdlib.h>
#include <errno.h>

#include "lib/bluetooth.h"
#include "uuid.h"

static uint128_t bluetooth_base_uuid = {
	.data = {	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x10, 0x00,
			0x80, 0x00, 0x00, 0x80, 0x5F, 0x9B, 0x34, 0xFB }
};

#define BASE_UUID16_OFFSET	2
#define BASE_UUID32_OFFSET	0

static void bt_uuid16_to_uuid128(const bt_uuid_t *src, bt_uuid_t *dst)
{
	uint16_t be16;

	dst->value.u128 = bluetooth_base_uuid;
	dst->type = BT_UUID128;

	/*
	 * No matter the system: 128-bit UUIDs should be stored
	 * as big-endian. 16-bit UUIDs are stored on host order.
	 */

	be16 = htons(src->value.u16);
	memcpy(&dst->value.u128.data[BASE_UUID16_OFFSET], &be16, sizeof(be16));
}

static void bt_uuid32_to_uuid128(const bt_uuid_t *src, bt_uuid_t *dst)
{
	uint32_t be32;

	dst->value.u128 = bluetooth_base_uuid;
	dst->type = BT_UUID128;

	/*
	 * No matter the system: 128-bit UUIDs should be stored
	 * as big-endian. 32-bit UUIDs are stored on host order.
	 */

	be32 = htonl(src->value.u32);
	memcpy(&dst->value.u128.data[BASE_UUID32_OFFSET], &be32, sizeof(be32));
}

void bt_uuid_to_uuid128(const bt_uuid_t *src, bt_uuid_t *dst)
{
	switch (src->type) {
	case BT_UUID128:
		*dst = *src;
		break;
	case BT_UUID32:
		bt_uuid32_to_uuid128(src, dst);
		break;
	case BT_UUID16:
		bt_uuid16_to_uuid128(src, dst);
		break;
	case BT_UUID_UNSPEC:
	default:
		break;
	}
}

static int bt_uuid128_cmp(const bt_uuid_t *u1, const bt_uuid_t *u2)
{
	return memcmp(&u1->value.u128, &u2->value.u128, sizeof(uint128_t));
}

int bt_uuid16_create(bt_uuid_t *btuuid, uint16_t value)
{
	memset(btuuid, 0, sizeof(bt_uuid_t));
	btuuid->type = BT_UUID16;
	btuuid->value.u16 = value;

	return 0;
}

int bt_uuid32_create(bt_uuid_t *btuuid, uint32_t value)
{
	memset(btuuid, 0, sizeof(bt_uuid_t));
	btuuid->type = BT_UUID32;
	btuuid->value.u32 = value;

	return 0;
}

int bt_uuid128_create(bt_uuid_t *btuuid, uint128_t value)
{
	memset(btuuid, 0, sizeof(bt_uuid_t));
	btuuid->type = BT_UUID128;
	btuuid->value.u128 = value;

	return 0;
}

int bt_uuid_cmp(const bt_uuid_t *uuid1, const bt_uuid_t *uuid2)
{
	bt_uuid_t u1, u2;

	bt_uuid_to_uuid128(uuid1, &u1);
	bt_uuid_to_uuid128(uuid2, &u2);

	return bt_uuid128_cmp(&u1, &u2);
}

/*
 * convert the UUID to string, copying a maximum of n characters.
 */
int bt_uuid_to_string(const bt_uuid_t *uuid, char *str, size_t n)
{
	bt_uuid_t tmp;
	unsigned int   data0;
	unsigned short data1;
	unsigned short data2;
	unsigned short data3;
	unsigned int   data4;
	unsigned short data5;
	const uint8_t *data;

	if (!uuid || uuid->type == BT_UUID_UNSPEC) {
		snprintf(str, n, "NULL");
		return -EINVAL;
	}

	/* Convert to 128 Bit format */
	bt_uuid_to_uuid128(uuid, &tmp);
	data = (uint8_t *) &tmp.value.u128;

	memcpy(&data0, &data[0], 4);
	memcpy(&data1, &data[4], 2);
	memcpy(&data2, &data[6], 2);
	memcpy(&data3, &data[8], 2);
	memcpy(&data4, &data[10], 4);
	memcpy(&data5, &data[14], 2);

	snprintf(str, n, "%.8x-%.4x-%.4x-%.4x-%.8x%.4x",
				ntohl(data0), ntohs(data1),
				ntohs(data2), ntohs(data3),
				ntohl(data4), ntohs(data5));

	return 0;
}

static inline int is_uuid128(const char *string)
{
	return (strlen(string) == 36 &&
			string[8] == '-' &&
			string[13] == '-' &&
			string[18] == '-' &&
			string[23] == '-');
}

static inline int is_base_uuid128(const char *string)
{
	uint16_t uuid;
	char dummy[2];

	if (!is_uuid128(string))
		return 0;

	return sscanf(string,
		"0000%04hx-0000-1000-8000-00805%1[fF]9%1[bB]34%1[fF]%1[bB]",
		&uuid, dummy, dummy, dummy, dummy) == 5;
}

static inline int is_uuid32(const char *string)
{
	return (strlen(string) == 8 || strlen(string) == 10);
}

static inline int is_uuid16(const char *string)
{
	return (strlen(string) == 4 || strlen(string) == 6);
}

static int bt_string_to_uuid16(bt_uuid_t *uuid, const char *string)
{
	uint16_t u16;
	char *endptr = NULL;

	u16 = strtol(string, &endptr, 16);
	if (endptr && (*endptr == '\0' || *endptr == '-')) {
		bt_uuid16_create(uuid, u16);
		return 0;
	}

	return -EINVAL;
}

static int bt_string_to_uuid32(bt_uuid_t *uuid, const char *string)
{
	uint32_t u32;
	char *endptr = NULL;

	u32 = strtol(string, &endptr, 16);
	if (endptr && *endptr == '\0') {
		bt_uuid32_create(uuid, u32);
		return 0;
	}

	return -EINVAL;
}

static int bt_string_to_uuid128(bt_uuid_t *uuid, const char *string)
{
	uint32_t data0, data4;
	uint16_t data1, data2, data3, data5;
	uint128_t u128;
	uint8_t *val = (uint8_t *) &u128;

	if (sscanf(string, "%08x-%04hx-%04hx-%04hx-%08x%04hx",
				&data0, &data1, &data2,
				&data3, &data4, &data5) != 6)
		return -EINVAL;

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

	bt_uuid128_create(uuid, u128);

	return 0;
}

int bt_string_to_uuid(bt_uuid_t *uuid, const char *string)
{
	if (is_base_uuid128(string))
		return bt_string_to_uuid16(uuid, string + 4);
	else if (is_uuid128(string))
		return bt_string_to_uuid128(uuid, string);
	else if (is_uuid32(string))
		return bt_string_to_uuid32(uuid, string);
	else if (is_uuid16(string))
		return bt_string_to_uuid16(uuid, string);

	return -EINVAL;
}

int bt_uuid_strcmp(const void *a, const void *b)
{
	bt_uuid_t u1, u2;

	if (bt_string_to_uuid(&u1, a) < 0)
		return -EINVAL;

	if (bt_string_to_uuid(&u2, b) < 0)
		return -EINVAL;

	return bt_uuid_cmp(&u1, &u2);
}

int bt_uuid_to_le(const bt_uuid_t *src, void *dst)
{
	bt_uuid_t uuid;

	switch (src->type) {
	case BT_UUID16:
		bt_put_le16(src->value.u16, dst);
		return 0;
	case BT_UUID32:
		bt_uuid32_to_uuid128(src, &uuid);
		src = &uuid;
		/* Fallthrough */
	case BT_UUID128:
		/* Convert from 128-bit BE to LE */
		bswap_128(&src->value.u128, dst);
		return 0;
	case BT_UUID_UNSPEC:
	default:
		return -EINVAL;
	}
}

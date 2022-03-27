/*
 *
 *  BlueZ - Bluetooth protocol stack for Linux
 *
 *  Copyright (C) 2011  Intel Corporation
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

#include <glib.h>

#include "lib/bluetooth.h"
#include "lib/uuid.h"
#include "src/shared/tester.h"

struct uuid_test_data {
	const char *str;
	uint16_t val16;
	uint32_t val32;
	unsigned char *binary;
	uint8_t type;
	const char *str128;
	unsigned char *binary128;
};

static unsigned char uuid_base_binary[] = {
			0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x10, 0x00,
			0x80, 0x00, 0x00, 0x80, 0x5f, 0x9b, 0x34, 0xfb };

static struct uuid_test_data uuid_base = {
	.str = "0000",
	.val16 = 0x0000,
	.type = BT_UUID16,
	.str128 = "00000000-0000-1000-8000-00805f9b34fb",
	.binary128 = uuid_base_binary,
};

static unsigned char uuid_sixteen_binary[] = {
			0x00, 0x00, 0x12, 0x34, 0x00, 0x00, 0x10, 0x00,
			0x80, 0x00, 0x00, 0x80, 0x5f, 0x9b, 0x34, 0xfb };

static struct uuid_test_data uuid_sixteen1 = {
	.str = "0x1234",
	.val16 = 0x1234,
	.type = BT_UUID16,
	.str128 = "00001234-0000-1000-8000-00805F9B34FB",
	.binary128 = uuid_sixteen_binary,
};

static struct uuid_test_data uuid_sixteen2 = {
	.str = "1234",
	.val16 = 0x1234,
	.type = BT_UUID16,
	.str128 = "00001234-0000-1000-8000-00805F9B34FB",
	.binary128 = uuid_sixteen_binary,
};

static unsigned char uuid_32_binary[] = {
			0x12, 0x34, 0x56, 0x78, 0x00, 0x00, 0x10, 0x00,
			0x80, 0x00, 0x00, 0x80, 0x5f, 0x9b, 0x34, 0xfb };

static struct uuid_test_data uuid_32_1 = {
	.str = "0x12345678",
	.val32 = 0x12345678,
	.type = BT_UUID32,
        .str128 = "12345678-0000-1000-8000-00805F9B34FB",
	.binary128 = uuid_32_binary,
};

static struct uuid_test_data uuid_32_2 = {
	.str = "12345678",
	.val32 = 0x12345678,
	.type = BT_UUID32,
	.str128 = "12345678-0000-1000-8000-00805F9B34FB",
	.binary128 = uuid_32_binary,
};

static unsigned char uuid_128_binary[] = {
			0xF0, 0x00, 0x00, 0x00, 0x00, 0x00, 0x10, 0x00,
			0x80, 0x00, 0x00, 0x80, 0x5f, 0x9b, 0x34, 0xfb };

static struct uuid_test_data uuid_128 = {
	.str = "F0000000-0000-1000-8000-00805f9b34fb",
	.binary = uuid_128_binary,
	.type = BT_UUID128,
	.str128 = "F0000000-0000-1000-8000-00805f9b34fb",
	.binary128 = uuid_128_binary,
};

static void test_uuid(gconstpointer data)
{
	const struct uuid_test_data *test_data = data;
	bt_uuid_t uuid;

	g_assert(bt_string_to_uuid(&uuid, test_data->str) == 0);
	g_assert(uuid.type == test_data->type);

	switch (uuid.type) {
	case BT_UUID16:
		g_assert(uuid.value.u16 == test_data->val16);
		break;
	case BT_UUID32:
		g_assert(uuid.value.u32 == test_data->val32);
		break;
	case BT_UUID128:
		/*
		 * No matter the system type: 128-bit UUID should use
		 * big-endian (human readable format).
		 */
		g_assert(memcmp(&uuid.value.u128, test_data->binary, 16) == 0);
		break;
	case BT_UUID_UNSPEC:
	default:
		tester_test_passed();
		return;
        }

	tester_test_passed();
}

static void test_str(gconstpointer data)
{
	const struct uuid_test_data *test_data = data;
	const char *str;
	char buf[128];
	bt_uuid_t uuid;

	if (g_str_has_prefix(test_data->str, "0x") == TRUE)
		str = test_data->str + 2;
	else
		str = test_data->str;

	g_assert(bt_string_to_uuid(&uuid, test_data->str) == 0);

	bt_uuid_to_string(&uuid, buf, sizeof(buf));
	g_assert(bt_uuid_strcmp(buf, str) == 0);

	switch (test_data->type) {
	case BT_UUID16:
		bt_uuid16_create(&uuid, test_data->val16);
		break;
	case BT_UUID32:
		bt_uuid32_create(&uuid, test_data->val32);
		break;
	default:
		tester_test_passed();
		return;
	}

	bt_uuid_to_string(&uuid, buf, sizeof(buf));
	g_assert(bt_uuid_strcmp(buf, str) == 0);
	tester_test_passed();
}

static void test_cmp(gconstpointer data)
{
	const struct uuid_test_data *test_data = data;
	bt_uuid_t uuid1, uuid2;

	g_assert(bt_string_to_uuid(&uuid1, test_data->str) == 0);
	g_assert(bt_string_to_uuid(&uuid2, test_data->str128) == 0);

	g_assert(bt_uuid_cmp(&uuid1, &uuid2) == 0);
	tester_test_passed();
}

static const struct uuid_test_data compress[] = {
	{
		.str = "00001234-0000-1000-8000-00805f9b34fb",
		.type = BT_UUID16,
		.val16 = 0x1234,
	}, {
		.str = "0000FFFF-0000-1000-8000-00805f9b34fb",
		.type = BT_UUID16,
		.val16 = 0xFFFF,
	}, {
		.str = "0000FFFF-0000-1000-8000-00805F9B34FB",
		.type = BT_UUID16,
		.val16 = 0xFFFF,
	}, {
		.str = "F0000000-0000-1000-8000-00805f9b34fb",
		.type = BT_UUID128,
		.binary = uuid_128_binary,
	},
};

static const char *malformed[] = {
	"0",
	"01",
	"012",
	"xxxx",
	"xxxxx",
	"0xxxxx",
	"0123456",
	"012g4567",
	"012345678",
	"0x234567u9",
	"01234567890",
	"00001234-0000-1000-8000-00805F9B34F",
	"00001234-0000-1000-8000 00805F9B34FB",
	"00001234-0000-1000-8000-00805F9B34FBC",
	"00001234-0000-1000-800G-00805F9B34FB",
	NULL,
};

static void test_malformed(gconstpointer data)
{
	const char *str = data;
	bt_uuid_t uuid;

	g_assert(bt_string_to_uuid(&uuid, str) != 0);
	tester_test_passed();
}

int main(int argc, char *argv[])
{
	size_t i;

	tester_init(&argc, &argv);

	tester_add("/uuid/base", &uuid_base, NULL, test_uuid, NULL);
	tester_add("/uuid/base/str", &uuid_base, NULL, test_str, NULL);
	tester_add("/uuid/base/cmp", &uuid_base, NULL, test_cmp, NULL);

	tester_add("/uuid/sixteen1", &uuid_sixteen1, NULL, test_uuid, NULL);
	tester_add("/uuid/sixteen1/str", &uuid_sixteen1, NULL, test_str, NULL);
	tester_add("/uuid/sixteen1/cmp", &uuid_sixteen1, NULL, test_cmp, NULL);

	tester_add("/uuid/sixteen2", &uuid_sixteen2, NULL, test_uuid, NULL);
	tester_add("/uuid/sixteen2/str", &uuid_sixteen2, NULL, test_str, NULL);
	tester_add("/uuid/sixteen2/cmp", &uuid_sixteen2, NULL, test_cmp, NULL);

	tester_add("/uuid/thirtytwo1", &uuid_32_1, NULL, test_uuid, NULL);
	tester_add("/uuid/thirtytwo1/str", &uuid_32_1, NULL, test_str, NULL);
	tester_add("/uuid/thirtytwo1/cmp", &uuid_32_1, NULL, test_cmp, NULL);

	tester_add("/uuid/thirtytwo2", &uuid_32_2, NULL, test_uuid, NULL);
	tester_add("/uuid/thritytwo2/str", &uuid_32_2, NULL, test_str, NULL);
	tester_add("/uuid/thirtytwo2/cmp", &uuid_32_2, NULL, test_cmp, NULL);

	tester_add("/uuid/onetwentyeight", &uuid_128, NULL, test_uuid, NULL);
	tester_add("/uuid/onetwentyeight/str", &uuid_128, NULL, test_str, NULL);
	tester_add("/uuid/onetwentyeight/cmp", &uuid_128, NULL, test_cmp, NULL);

	for (i = 0; malformed[i]; i++) {
		char *testpath;

		testpath = g_strdup_printf("/uuid/malformed/%s", malformed[i]);
		tester_add(testpath, malformed[i], NULL, test_malformed, NULL);
		g_free(testpath);
	}

	for (i = 0; i < (sizeof(compress) / sizeof(compress[0])); i++) {
		char *testpath;

		testpath = g_strdup_printf("/uuid/compress/%s",
							compress[i].str);
		tester_add(testpath, compress + i, NULL, test_uuid, NULL);
		g_free(testpath);
	}

	return tester_run();
}

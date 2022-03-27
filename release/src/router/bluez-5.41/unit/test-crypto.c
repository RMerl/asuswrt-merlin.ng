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

#include "src/shared/crypto.h"
#include "src/shared/util.h"
#include "src/shared/tester.h"

#include <string.h>
#include <glib.h>

static struct bt_crypto *crypto;

static void print_debug(const char *str, void *user_data)
{
	tester_debug("%s", str);
}

static void test_h6(gconstpointer data)
{
	const uint8_t w[16] = {
			0x9b, 0x7d, 0x39, 0x0a, 0xa6, 0x10, 0x10, 0x34,
			0x05, 0xad, 0xc8, 0x57, 0xa3, 0x34, 0x02, 0xec };
	const uint8_t m[4] = { 0x72, 0x62, 0x65, 0x6c };
	const uint8_t exp[16] = {
			0x99, 0x63, 0xb1, 0x80, 0xe2, 0xa9, 0xd3, 0xe8,
			0x1c, 0xc9, 0x6d, 0xe7, 0x02, 0xe1, 0x9a, 0x2d };
	uint8_t res[16];

	tester_debug("W:");
	util_hexdump(' ', w, 16, print_debug, NULL);

	tester_debug("M:");
	util_hexdump(' ', m, 4, print_debug, NULL);

	if (!bt_crypto_h6(crypto, w, m, res)) {
		tester_test_failed();
		return;
	}

	tester_debug("Expected:");
	util_hexdump(' ', exp, 16, print_debug, NULL);

	tester_debug("Result:");
	util_hexdump(' ', res, 16, print_debug, NULL);


	if (memcmp(res, exp, 16)) {
		tester_test_failed();
		return;
	}

	tester_test_passed();
}

struct test_data {
	const uint8_t *msg;
	uint16_t msg_len;
	const uint8_t *t;
	const uint8_t *key;
	uint32_t cnt;
};

static const uint8_t key[] = {
	0x3c, 0x4f, 0xcf, 0x09, 0x88, 0x15, 0xf7, 0xab, 0xa6, 0xd2, 0xae, 0x28,
	0x16, 0x15, 0x7e, 0x2b
};

static const uint8_t msg_1[] = { 0x00 };

static const uint8_t t_msg_1[] = {
	0x00, 0x00, 0x00, 0x00, 0xb3, 0xa8, 0x59, 0x41, 0x27, 0xeb, 0xc2, 0xc0
};

static const struct test_data test_data_1 = {
	.msg = msg_1,
	.msg_len = 0,
	.t = t_msg_1,
	.key = key,
};

static const uint8_t msg_2[] = {
	0x6b, 0xc1, 0xbe, 0xe2, 0x2e, 0x40, 0x9f, 0x96, 0xe9, 0x3d, 0x7e, 0x11,
	0x73, 0x93, 0x17, 0x2a

};

static const uint8_t t_msg_2[] = {
	0x00, 0x00, 0x00, 0x00, 0x27, 0x39, 0x74, 0xf4, 0x39, 0x2a, 0x23, 0x2a
};

static const struct test_data test_data_2 = {
	.msg = msg_2,
	.msg_len = 16,
	.t = t_msg_2,
	.key = key,
};

static const uint8_t msg_3[] = {
	0x6b, 0xc1, 0xbe, 0xe2, 0x2e, 0x40, 0x9f, 0x96, 0xe9, 0x3d, 0x7e, 0x11,
	0x73, 0x93, 0x17, 0x2a, 0xae, 0x2d, 0x8a, 0x57, 0x1e, 0x03, 0xac, 0x9c,
	0x9e, 0xb7, 0x6f, 0xac, 0x45, 0xaf, 0x8e, 0x51, 0x30, 0xc8, 0x1c, 0x46,
	0xa3, 0x5c, 0xe4, 0x11
};

static const uint8_t t_msg_3[12] = {
	0x00, 0x00, 0x00, 0x00, 0xb7, 0xca, 0x94, 0xab, 0x87, 0xc7, 0x82, 0x18
};

static const struct test_data test_data_3 = {
	.msg = msg_3,
	.msg_len = 40,
	.t = t_msg_3,
	.key = key,
};

static const uint8_t msg_4[] = {
	0x6b, 0xc1, 0xbe, 0xe2, 0x2e, 0x40, 0x9f, 0x96, 0xe9, 0x3d, 0x7e, 0x11,
	0x73, 0x93, 0x17, 0x2a, 0xae, 0x2d, 0x8a, 0x57, 0x1e, 0x03, 0xac, 0x9c,
	0x9e, 0xb7, 0x6f, 0xac, 0x45, 0xaf, 0x8e, 0x51, 0x30, 0xc8, 0x1c, 0x46,
	0xa3, 0x5c, 0xe4, 0x11, 0xe5, 0xfb, 0xc1, 0x19, 0x1a, 0x0a, 0x52, 0xef,
	0xf6, 0x9f, 0x24, 0x45, 0xdf, 0x4f, 0x9b, 0x17, 0xad, 0x2b, 0x41, 0x7b,
	0xe6, 0x6c, 0x37, 0x10
};

static const uint8_t t_msg_4[12] = {
	0x00, 0x00, 0x00, 0x00, 0x44, 0xe1, 0xe6, 0xce, 0x1d, 0xf5, 0x13, 0x68
};

static const struct test_data test_data_4 = {
	.msg = msg_4,
	.msg_len = 64,
	.t = t_msg_4,
	.key = key,
};

static const uint8_t msg_5[] = {
		0xd2, 0x12, 0x00, 0x13, 0x37
};

static const uint8_t key_5[] = {
		0x50, 0x5E, 0x42, 0xDF, 0x96, 0x91, 0xEC, 0x72, 0xD3, 0x1F,
		0xCD, 0xFB, 0xEB, 0x64, 0x1B, 0x61
};

static const uint8_t t_msg_5[] = {
		0x01, 0x00, 0x00, 0x00, 0xF1, 0x87, 0x1E, 0x93, 0x3C, 0x90,
		0x0F, 0xf2
};

static const struct test_data test_data_5 = {
	.msg = msg_5,
	.msg_len = sizeof(msg_5),
	.t = t_msg_5,
	.cnt = 1,
	.key = key_5,
};

static bool result_compare(const uint8_t exp[12], uint8_t res[12])
{
	int i;
	for (i = 0; i < 12; i++)
		if (exp[i] != res[i])
			return false;

	return true;
}

static void test_sign(gconstpointer data)
{
	uint8_t t[12];
	const struct test_data *d = data;

	memset(t, 0, 12);
	if (!bt_crypto_sign_att(crypto, d->key, d->msg, d->msg_len, d->cnt, t))
		g_assert(true);

	tester_debug("Result T:");
	util_hexdump(' ', t, 12, print_debug, NULL);
	tester_debug("Expected T:");
	util_hexdump(' ', d->t, 12, print_debug, NULL);

	g_assert(result_compare(d->t, t));

	tester_test_passed();
}

int main(int argc, char *argv[])
{
	int exit_status;

	crypto = bt_crypto_new();
	if (!crypto)
		return 0;

	tester_init(&argc, &argv);

	tester_add("/crypto/h6", NULL, NULL, test_h6, NULL);

	tester_add("/crypto/sign_att_1", &test_data_1, NULL, test_sign, NULL);
	tester_add("/crypto/sign_att_2", &test_data_2, NULL, test_sign, NULL);
	tester_add("/crypto/sign_att_3", &test_data_3, NULL, test_sign, NULL);
	tester_add("/crypto/sign_att_4", &test_data_4, NULL, test_sign, NULL);
	tester_add("/crypto/sign_att_5", &test_data_5, NULL, test_sign, NULL);

	exit_status = tester_run();

	bt_crypto_unref(crypto);

	return exit_status;
}

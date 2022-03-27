/*
 *
 *  BlueZ - Bluetooth protocol stack for Linux
 *
 *  Copyright (C) 2013-2014  Intel Corporation
 *
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Lesser General Public
 *  License as published by the Free Software Foundation; either
 *  version 2.1 of the License, or (at your option) any later version.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public
 *  License along with this library; if not, write to the Free Software
 *  Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
 *
 */

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include <string.h>

#include "monitor/bt.h"
#include "src/shared/util.h"
#include "src/shared/hci.h"
#include "src/shared/hci-crypto.h"

struct crypto_data {
	uint8_t size;
	bt_hci_crypto_func_t callback;
	void *user_data;
};

static void le_encrypt_callback(const void *response, uint8_t size,
							void *user_data)
{
	struct crypto_data *data = user_data;
	const struct bt_hci_rsp_le_encrypt *rsp = response;

	if (rsp->status) {
		data->callback(NULL, 0, data->user_data);
		return;
	}

	data->callback(rsp->data, data->size, data->user_data);
}

static bool le_encrypt(struct bt_hci *hci, uint8_t size,
			const uint8_t key[16], const uint8_t plaintext[16],
			bt_hci_crypto_func_t callback, void *user_data)
{
	struct crypto_data *data;
	struct bt_hci_cmd_le_encrypt cmd;

	if (!callback || !size || size > 16)
		return false;

	memcpy(cmd.key, key, 16);
	memcpy(cmd.plaintext, plaintext, 16);

	data = new0(struct crypto_data, 1);
	data->size = size;
	data->callback = callback;
	data->user_data = user_data;

	if (!bt_hci_send(hci, BT_HCI_CMD_LE_ENCRYPT, &cmd, sizeof(cmd),
					le_encrypt_callback, data, free)) {
		free(data);
		return false;
	}

	return true;
}

static void prand_callback(const void *response, uint8_t size,
							void *user_data)
{
	struct crypto_data *data = user_data;
	const struct bt_hci_rsp_le_rand *rsp = response;
	uint8_t prand[3];

	if (rsp->status) {
		data->callback(NULL, 0, data->user_data);
		return;
	}

	prand[0] = (rsp->number & 0xff0000) >> 16;
	prand[1] = (rsp->number & 0x00ff00) >> 8;
	prand[2] = (rsp->number & 0x00003f) | 0x40;

	data->callback(prand, 3, data->user_data);
}

bool bt_hci_crypto_prand(struct bt_hci *hci,
			bt_hci_crypto_func_t callback, void *user_data)
{
	struct crypto_data *data;

	if (!callback)
		return false;

	data = new0(struct crypto_data, 1);
	data->callback = callback;
	data->user_data = user_data;

	if (!bt_hci_send(hci, BT_HCI_CMD_LE_RAND, NULL, 0,
					prand_callback, data, free)) {
		free(data);
		return false;
	}

	return true;
}

bool bt_hci_crypto_e(struct bt_hci *hci,
			const uint8_t key[16], const uint8_t plaintext[16],
			bt_hci_crypto_func_t callback, void *user_data)
{
	return le_encrypt(hci, 16, key, plaintext, callback, user_data);
}

bool bt_hci_crypto_d1(struct bt_hci *hci,
			const uint8_t k[16], uint16_t d, uint16_t r,
			bt_hci_crypto_func_t callback, void *user_data)
{
	uint8_t dp[16];

	/* d' = padding || r || d */
	dp[0] = d & 0xff;
	dp[1] = d >> 8;
	dp[2] = r & 0xff;
	dp[3] = r >> 8;
	memset(dp + 4, 0, 12);

	/* d1(k, d, r) = e(k, d') */
	return le_encrypt(hci, 16, k, dp, callback, user_data);
}

bool bt_hci_crypto_dm(struct bt_hci *hci,
			const uint8_t k[16], const uint8_t r[8],
			bt_hci_crypto_func_t callback, void *user_data)
{
	uint8_t rp[16];

	/* r' = padding || r */
	memcpy(rp, r, 8);
	memset(rp + 8, 0, 8);

	/* dm(k, r) = e(k, r') mod 2^16 */
	return le_encrypt(hci, 8, k, rp, callback, user_data);
}

bool bt_hci_crypto_ah(struct bt_hci *hci,
			const uint8_t k[16], const uint8_t r[3],
			bt_hci_crypto_func_t callback, void *user_data)
{
	uint8_t rp[16];

	/* r' = padding || r */
	memcpy(rp, r, 3);
	memset(rp + 3, 0, 13);

	/* ah(k, r) = e(k, r') mod 2^24 */
	return le_encrypt(hci, 3, k, rp, callback, user_data);
}

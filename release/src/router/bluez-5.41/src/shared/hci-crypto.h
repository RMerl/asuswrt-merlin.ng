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

#include <stdbool.h>
#include <stdint.h>

struct bt_hci;

typedef void (*bt_hci_crypto_func_t)(const void *data, uint8_t size,
							void *user_data);

bool bt_hci_crypto_prand(struct bt_hci *hci,
			bt_hci_crypto_func_t callback, void *user_data);
bool bt_hci_crypto_e(struct bt_hci *hci,
			const uint8_t key[16], const uint8_t plaintext[16],
			bt_hci_crypto_func_t callback, void *user_data);
bool bt_hci_crypto_d1(struct bt_hci *hci,
			const uint8_t k[16], uint16_t d, uint16_t r,
			bt_hci_crypto_func_t callback, void *user_data);
bool bt_hci_crypto_dm(struct bt_hci *hci,
			const uint8_t k[16], const uint8_t r[8],
			bt_hci_crypto_func_t callback, void *user_data);
bool bt_hci_crypto_ah(struct bt_hci *hci,
			const uint8_t k[16], const uint8_t r[3],
			bt_hci_crypto_func_t callback, void *user_data);

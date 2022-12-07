/* SPDX-License-Identifier: LGPL-2.1-or-later */
/*
 *
 *  BlueZ - Bluetooth protocol stack for Linux
 *
 *  Copyright (C) 2013-2014  Intel Corporation
 *
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

/*
 *
 *  BlueZ - Bluetooth protocol stack for Linux
 *
 *  Copyright (C) 2012-2014  Intel Corporation. All rights reserved.
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

struct bt_crypto;

struct bt_crypto *bt_crypto_new(void);

struct bt_crypto *bt_crypto_ref(struct bt_crypto *crypto);
void bt_crypto_unref(struct bt_crypto *crypto);

bool bt_crypto_random_bytes(struct bt_crypto *crypto,
					uint8_t *buf, uint8_t num_bytes);

bool bt_crypto_e(struct bt_crypto *crypto, const uint8_t key[16],
			const uint8_t plaintext[16], uint8_t encrypted[16]);
bool bt_crypto_ah(struct bt_crypto *crypto, const uint8_t k[16],
					const uint8_t r[3], uint8_t hash[3]);
bool bt_crypto_c1(struct bt_crypto *crypto, const uint8_t k[16],
			const uint8_t r[16], const uint8_t pres[7],
			const uint8_t preq[7], uint8_t iat,
			const uint8_t ia[6], uint8_t rat,
			const uint8_t ra[6], uint8_t res[16]);
bool bt_crypto_s1(struct bt_crypto *crypto, const uint8_t k[16],
			const uint8_t r1[16], const uint8_t r2[16],
			uint8_t res[16]);
bool bt_crypto_f4(struct bt_crypto *crypto, uint8_t u[32], uint8_t v[32],
				uint8_t x[16], uint8_t z, uint8_t res[16]);
bool bt_crypto_f5(struct bt_crypto *crypto, uint8_t w[32], uint8_t n1[16],
				uint8_t n2[16], uint8_t a1[7], uint8_t a2[7],
				uint8_t mackey[16], uint8_t ltk[16]);
bool bt_crypto_f6(struct bt_crypto *crypto, uint8_t w[16], uint8_t n1[16],
			uint8_t n2[16], uint8_t r[16], uint8_t io_cap[3],
			uint8_t a1[7], uint8_t a2[7], uint8_t res[16]);
bool bt_crypto_g2(struct bt_crypto *crypto, uint8_t u[32], uint8_t v[32],
				uint8_t x[16], uint8_t y[16], uint32_t *val);
bool bt_crypto_h6(struct bt_crypto *crypto, const uint8_t w[16],
				const uint8_t keyid[4], uint8_t res[16]);
bool bt_crypto_sign_att(struct bt_crypto *crypto, const uint8_t key[16],
				const uint8_t *m, uint16_t m_len,
				uint32_t sign_cnt, uint8_t signature[12]);

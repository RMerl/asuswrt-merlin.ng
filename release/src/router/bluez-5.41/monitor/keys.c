/*
 *
 *  BlueZ - Bluetooth protocol stack for Linux
 *
 *  Copyright (C) 2011-2014  Intel Corporation
 *  Copyright (C) 2002-2010  Marcel Holtmann <marcel@holtmann.org>
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

#include "src/shared/util.h"
#include "src/shared/queue.h"
#include "src/shared/crypto.h"

#include "keys.h"

static const uint8_t empty_key[16] = { 0x00, };
static const uint8_t empty_addr[6] = { 0x00, };

static struct bt_crypto *crypto;

struct irk_data {
	uint8_t key[16];
	uint8_t addr[6];
	uint8_t addr_type;
};

static struct queue *irk_list;

void keys_setup(void)
{
	crypto = bt_crypto_new();

	irk_list = queue_new();
}

void keys_cleanup(void)
{
	bt_crypto_unref(crypto);

	queue_destroy(irk_list, free);
}

void keys_update_identity_key(const uint8_t key[16])
{
	struct irk_data *irk;

	irk = queue_peek_tail(irk_list);
	if (irk && !memcmp(irk->key, empty_key, 16)) {
		memcpy(irk->key, key, 16);
		return;
	}

	irk = new0(struct irk_data, 1);
	if (irk) {
		memcpy(irk->key, key, 16);
		if (!queue_push_tail(irk_list, irk))
			free(irk);
	}
}

void keys_update_identity_addr(const uint8_t addr[6], uint8_t addr_type)
{
	struct irk_data *irk;

	irk = queue_peek_tail(irk_list);
	if (irk && !memcmp(irk->addr, empty_addr, 6)) {
		memcpy(irk->addr, addr, 6);
		irk->addr_type = addr_type;
		return;
	}

	irk = new0(struct irk_data, 1);
	if (irk) {
		memcpy(irk->addr, addr, 6);
		irk->addr_type = addr_type;
		if (!queue_push_tail(irk_list, irk))
			free(irk);
	}
}

static bool match_resolve_irk(const void *data, const void *match_data)
{
	const struct irk_data *irk = data;
	const uint8_t *addr = match_data;
	uint8_t local_hash[3];

	bt_crypto_ah(crypto, irk->key, addr + 3, local_hash);

	return !memcmp(addr, local_hash, 3);
}

bool keys_resolve_identity(const uint8_t addr[6], uint8_t ident[6],
							uint8_t *ident_type)
{
	struct irk_data *irk;

	irk = queue_find(irk_list, match_resolve_irk, addr);

	if (irk) {
		memcpy(ident, irk->addr, 6);
		*ident_type = irk->addr_type;
		return true;
	}

	return false;
}

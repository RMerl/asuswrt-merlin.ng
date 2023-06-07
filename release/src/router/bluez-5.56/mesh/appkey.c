// SPDX-License-Identifier: LGPL-2.1-or-later
/*
 *
 *  BlueZ - Bluetooth protocol stack for Linux
 *
 *  Copyright (C) 2017-2019  Intel Corporation. All rights reserved.
 *
 *
 */

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#define _GNU_SOURCE
#include <ell/ell.h>

#include "mesh/mesh-defs.h"

#include "mesh/node.h"
#include "mesh/net.h"
#include "mesh/crypto.h"
#include "mesh/util.h"
#include "mesh/model.h"
#include "mesh/mesh-config.h"
#include "mesh/appkey.h"

struct mesh_app_key {
	uint16_t net_idx;
	uint16_t app_idx;
	uint8_t key[16];
	uint8_t key_aid;
	uint8_t new_key[16];
	uint8_t new_key_aid;
};

static bool match_key_index(const void *a, const void *b)
{
	const struct mesh_app_key *key = a;
	uint16_t idx = L_PTR_TO_UINT(b);

	return key->app_idx == idx;
}

static bool match_bound_key(const void *a, const void *b)
{
	const struct mesh_app_key *key = a;
	uint16_t idx = L_PTR_TO_UINT(b);

	return key->net_idx == idx;
}

static void finalize_key(void *a, void *b)
{
	struct mesh_app_key *key = a;
	uint16_t net_idx = L_PTR_TO_UINT(b);

	if (key->net_idx != net_idx)
		return;

	if (key->new_key_aid == APP_AID_INVALID)
		return;

	key->key_aid = key->new_key_aid;

	key->new_key_aid = APP_AID_INVALID;

	memcpy(key->key, key->new_key, 16);
}

void appkey_finalize(struct mesh_net *net, uint16_t net_idx)
{
	struct l_queue *app_keys;

	app_keys = mesh_net_get_app_keys(net);
	if (!app_keys)
		return;

	l_queue_foreach(app_keys, finalize_key, L_UINT_TO_PTR(net_idx));
}

static struct mesh_app_key *app_key_new(void)
{
	struct mesh_app_key *key = l_new(struct mesh_app_key, 1);

	key->new_key_aid = APP_AID_INVALID;
	return key;
}

static bool set_key(struct mesh_app_key *key, uint16_t app_idx,
			const uint8_t *key_value, bool is_new)
{
	uint8_t key_aid;

	if (!mesh_crypto_k4(key_value, &key_aid))
		return false;

	key_aid = KEY_ID_AKF | (key_aid << KEY_AID_SHIFT);
	if (!is_new)
		key->key_aid = key_aid;
	else
		key->new_key_aid = key_aid;

	memcpy(is_new ? key->new_key : key->key, key_value, 16);

	return true;
}

void appkey_key_free(void *data)
{
	struct mesh_app_key *key = data;

	if (!key)
		return;

	l_free(key);
}

bool appkey_key_init(struct mesh_net *net, uint16_t net_idx, uint16_t app_idx,
				uint8_t *key_value, uint8_t *new_key_value)
{
	struct mesh_app_key *key;
	struct l_queue *app_keys;

	if (net_idx > MAX_KEY_IDX || app_idx > MAX_KEY_IDX)
		return false;

	app_keys = mesh_net_get_app_keys(net);
	if (!app_keys)
		return NULL;

	if (!mesh_net_have_key(net, net_idx))
		return false;

	key = app_key_new();
	if (!key)
		return false;

	key->net_idx = net_idx;
	key->app_idx = app_idx;

	if (key_value && !set_key(key, app_idx, key_value, false))
		return false;

	if (new_key_value && !set_key(key, app_idx, new_key_value, true))
		return false;

	l_queue_push_tail(app_keys, key);

	return true;
}

const uint8_t *appkey_get_key(struct mesh_net *net, uint16_t app_idx,
							uint8_t *key_aid)
{
	struct mesh_app_key *app_key;
	uint8_t phase;
	struct l_queue *app_keys;

	app_keys = mesh_net_get_app_keys(net);
	if (!app_keys)
		return NULL;

	app_key = l_queue_find(app_keys, match_key_index,
							L_UINT_TO_PTR(app_idx));
	if (!app_key)
		return NULL;

	if (mesh_net_key_refresh_phase_get(net, app_key->net_idx, &phase) !=
							MESH_STATUS_SUCCESS)
		return NULL;

	if (phase != KEY_REFRESH_PHASE_TWO) {
		*key_aid = app_key->key_aid;
		return app_key->key;
	}

	if (app_key->new_key_aid == APP_AID_INVALID)
		return NULL;

	*key_aid = app_key->new_key_aid;
	return app_key->new_key;
}

int appkey_get_key_idx(struct mesh_app_key *app_key,
				const uint8_t **key, uint8_t *key_aid,
				const uint8_t **new_key, uint8_t *new_key_aid)
{
	if (!app_key)
		return -1;

	if (key && key_aid) {
		*key = app_key->key;
		*key_aid = app_key->key_aid;
	}

	if (new_key && new_key_aid) {
		*new_key = app_key->new_key;
		*new_key_aid = app_key->new_key_aid;
	}

	return app_key->app_idx;
}

bool appkey_have_key(struct mesh_net *net, uint16_t app_idx)
{
	struct mesh_app_key *key;
	struct l_queue *app_keys;

	app_keys = mesh_net_get_app_keys(net);
	if (!app_keys)
		return false;

	key = l_queue_find(app_keys, match_key_index, L_UINT_TO_PTR(app_idx));

	if (!key)
		return false;
	else
		return true;
}

uint16_t appkey_net_idx(struct mesh_net *net, uint16_t app_idx)
{
	struct mesh_app_key *key;
	struct l_queue *app_keys;

	app_keys = mesh_net_get_app_keys(net);
	if (!app_keys)
		return NET_IDX_INVALID;

	key = l_queue_find(app_keys, match_key_index, L_UINT_TO_PTR(app_idx));

	if (!key)
		return NET_IDX_INVALID;
	else
		return key->net_idx;
}

int appkey_key_update(struct mesh_net *net, uint16_t net_idx, uint16_t app_idx,
							const uint8_t *new_key)
{
	struct mesh_app_key *key;
	struct l_queue *app_keys;
	uint8_t phase = KEY_REFRESH_PHASE_NONE;
	struct mesh_node *node;

	app_keys = mesh_net_get_app_keys(net);
	if (!app_keys)
		return MESH_STATUS_INSUFF_RESOURCES;

	if (!mesh_net_have_key(net, net_idx))
		return MESH_STATUS_INVALID_NETKEY;

	key = l_queue_find(app_keys, match_key_index, L_UINT_TO_PTR(app_idx));

	if (!key)
		return MESH_STATUS_INVALID_APPKEY;

	if (key->net_idx != net_idx)
		return MESH_STATUS_INVALID_BINDING;

	mesh_net_key_refresh_phase_get(net, net_idx, &phase);
	if (phase != KEY_REFRESH_PHASE_ONE)
		return MESH_STATUS_CANNOT_UPDATE;

	/* Check if the key has been already successfully updated */
	if (memcmp(new_key, key->new_key, 16) == 0)
		return MESH_STATUS_SUCCESS;

	if (!set_key(key, app_idx, new_key, true))
		return MESH_STATUS_INSUFF_RESOURCES;

	node = mesh_net_node_get(net);

	if (!mesh_config_app_key_update(node_config_get(node), app_idx,
								new_key))
		return MESH_STATUS_STORAGE_FAIL;

	return MESH_STATUS_SUCCESS;
}

int appkey_key_add(struct mesh_net *net, uint16_t net_idx, uint16_t app_idx,
							const uint8_t *new_key)
{
	struct mesh_app_key *key;
	struct l_queue *app_keys;
	struct mesh_node *node;

	app_keys = mesh_net_get_app_keys(net);
	if (!app_keys)
		return MESH_STATUS_INSUFF_RESOURCES;

	key = l_queue_find(app_keys, match_key_index, L_UINT_TO_PTR(app_idx));
	if (key) {
		if (memcmp(new_key, key->key, 16) == 0)
			return MESH_STATUS_SUCCESS;
		else
			return MESH_STATUS_IDX_ALREADY_STORED;
	}

	if (!mesh_net_have_key(net, net_idx))
		return MESH_STATUS_INVALID_NETKEY;

	if (l_queue_length(app_keys) >= MAX_APP_KEYS)
		return MESH_STATUS_INSUFF_RESOURCES;

	key = app_key_new();

	if (!set_key(key, app_idx, new_key, false)) {
		appkey_key_free(key);
		return MESH_STATUS_INSUFF_RESOURCES;
	}

	node = mesh_net_node_get(net);

	if (!mesh_config_app_key_add(node_config_get(node), net_idx, app_idx,
								new_key)) {
		appkey_key_free(key);
		return MESH_STATUS_STORAGE_FAIL;
	}

	key->net_idx = net_idx;
	key->app_idx = app_idx;
	l_queue_push_tail(app_keys, key);

	return MESH_STATUS_SUCCESS;
}

int appkey_key_delete(struct mesh_net *net, uint16_t net_idx,
							uint16_t app_idx)
{
	struct mesh_app_key *key;
	struct l_queue *app_keys;
	struct mesh_node *node;

	app_keys = mesh_net_get_app_keys(net);
	if (!app_keys)
		return MESH_STATUS_INVALID_APPKEY;

	key = l_queue_find(app_keys, match_key_index, L_UINT_TO_PTR(app_idx));

	if (!key)
		return MESH_STATUS_SUCCESS;

	if (key->net_idx != net_idx)
		return MESH_STATUS_INVALID_NETKEY;

	node = mesh_net_node_get(net);

	node_app_key_delete(node, net_idx, app_idx);

	l_queue_remove(app_keys, key);
	appkey_key_free(key);

	if (!mesh_config_app_key_del(node_config_get(node), net_idx, app_idx))
		return MESH_STATUS_STORAGE_FAIL;

	return MESH_STATUS_SUCCESS;
}

void appkey_delete_bound_keys(struct mesh_net *net, uint16_t net_idx)
{
	struct l_queue *app_keys;
	struct mesh_node *node;
	struct mesh_app_key *key;

	app_keys = mesh_net_get_app_keys(net);
	if (!app_keys)
		return;

	node = mesh_net_node_get(net);

	key = l_queue_remove_if(app_keys, match_bound_key,
					L_UINT_TO_PTR(net_idx));

	while (key) {
		node_app_key_delete(node, net_idx, key->app_idx);
		mesh_config_app_key_del(node_config_get(node), net_idx,
								key->app_idx);
		appkey_key_free(key);

		key = l_queue_remove_if(app_keys, match_bound_key,
					L_UINT_TO_PTR(net_idx));
	}
}

uint8_t appkey_list(struct mesh_net *net, uint16_t net_idx, uint8_t *buf,
					uint16_t buf_size, uint16_t *size)
{
	const struct l_queue_entry *entry;
	uint32_t idx_pair;
	int i;
	uint16_t datalen;
	struct l_queue *app_keys;

	*size = 0;

	if (!mesh_net_have_key(net, net_idx))
		return MESH_STATUS_INVALID_NETKEY;

	app_keys = mesh_net_get_app_keys(net);
	if (!app_keys || l_queue_isempty(app_keys))
		return MESH_STATUS_SUCCESS;

	idx_pair = 0;
	i = 0;
	datalen = 0;
	entry = l_queue_get_entries(app_keys);

	for (; entry; entry = entry->next) {
		struct mesh_app_key *key = entry->data;

		if (net_idx != key->net_idx)
			continue;

		if (!(i & 0x1)) {
			idx_pair = key->app_idx;
		} else {
			idx_pair <<= 12;
			idx_pair += key->app_idx;
			/* Unlikely, but check for overflow*/
			if ((datalen + 3) > buf_size) {
				l_warn("Appkey list too large");
				goto done;
			}
			l_put_le32(idx_pair, buf);
			buf += 3;
			datalen += 3;
		}
		i++;
	}

	/* Process the last app key if present */
	if (i & 0x1 && ((datalen + 2) <= buf_size)) {
		l_put_le16(idx_pair, buf);
		datalen += 2;
	}

done:
	*size = datalen;

	return MESH_STATUS_SUCCESS;
}

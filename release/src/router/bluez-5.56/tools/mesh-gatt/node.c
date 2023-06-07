// SPDX-License-Identifier: LGPL-2.1-or-later
/*
 *
 *  BlueZ - Bluetooth protocol stack for Linux
 *
 *  Copyright (C) 2017  Intel Corporation. All rights reserved.
 *
 *
 */

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include <stdio.h>
#include <errno.h>
#include <unistd.h>
#include <stdlib.h>
#include <stdbool.h>
#include <sys/uio.h>
#include <wordexp.h>
#include <inttypes.h>

#include <glib.h>

#include "src/shared/util.h"
#include "src/shared/shell.h"
#include "gdbus/gdbus.h"

#include "tools/mesh/config-model.h"

#include "tools/mesh-gatt/mesh-net.h"
#include "tools/mesh-gatt/node.h"
#include "tools/mesh-gatt/keys.h"
#include "tools/mesh-gatt/gatt.h"
#include "tools/mesh-gatt/net.h"
#include "tools/mesh-gatt/prov-db.h"
#include "tools/mesh-gatt/util.h"

struct mesh_model {
	struct mesh_model_ops cbs;
	void *user_data;
	GList *bindings;
	GList *subscriptions;
	uint32_t id;
	struct mesh_publication *pub;
};

struct mesh_element {
	GList *models;
	uint16_t loc;
	uint8_t index;
};

struct mesh_node {
	const char *name;
	GList *net_keys;
	GList *app_keys;
	void *prov;
	GList *elements;
	uint32_t iv_index;
	uint32_t seq_number;
	uint16_t primary_net_idx;
	uint16_t primary;
	uint16_t oob;
	uint16_t features;
	uint8_t dev_uuid[16];
	uint8_t dev_key[16];
	uint8_t num_ele;
	uint8_t ttl;
	bool provisioner;
	struct mesh_node_composition *comp;
};

static GList *nodes;

static struct mesh_node *local_node;

static int match_node_unicast(const void *a, const void *b)
{
	const struct mesh_node *node = a;
	uint16_t dst = GPOINTER_TO_UINT(b);

	if (dst >= node->primary &&
				dst <= (node->primary + node->num_ele - 1))
		return 0;

	return -1;
}

static int match_device_uuid(const void *a, const void *b)
{
	const struct mesh_node *node = a;
	const uint8_t *uuid = b;

	return memcmp(node->dev_uuid, uuid, 16);
}

static int match_element_idx(const void *a, const void *b)
{
	const struct mesh_element *element = a;
	uint32_t index = GPOINTER_TO_UINT(b);

	return (element->index == index) ? 0 : -1;
}

static int match_model_id(const void *a, const void *b)
{
	const struct mesh_model *model = a;
	uint32_t id = GPOINTER_TO_UINT(b);

	return (model->id == id) ? 0 : -1;
}

struct mesh_node *node_find_by_addr(uint16_t addr)
{
	GList *l;

	if (!IS_UNICAST(addr))
		return NULL;

	l = g_list_find_custom(nodes, GUINT_TO_POINTER(addr),
			match_node_unicast);

	if (l)
		return l->data;
	else
		return NULL;
}

struct mesh_node *node_find_by_uuid(uint8_t uuid[16])
{
	GList *l;

	l = g_list_find_custom(nodes, uuid, match_device_uuid);

	if (l)
		return l->data;
	else
		return NULL;
}

struct mesh_node *node_create_new(struct prov_svc_data *prov)
{
	struct mesh_node *node;

	if (node_find_by_uuid(prov->dev_uuid))
		return NULL;

	node = g_malloc0(sizeof(struct mesh_node));
	if (!node)
		return NULL;

	memcpy(node->dev_uuid, prov->dev_uuid, 16);
	node->oob = prov->oob;
	nodes = g_list_append(nodes, node);

	return node;
}

struct mesh_node *node_new(void)
{
	struct mesh_node *node;

	node = g_malloc0(sizeof(struct mesh_node));
	if (!node)
		return NULL;

	nodes = g_list_append(nodes, node);

	return node;
}

static void model_free(void *data)
{
	struct mesh_model *model = data;

	g_list_free(model->bindings);
	g_list_free(model->subscriptions);
	g_free(model->pub);
	g_free(model);
}

static void element_free(void *data)
{
	struct mesh_element *element = data;

	g_list_free_full(element->models, model_free);
	g_free(element);
}

static void free_node_resources(void *data)
{
	struct mesh_node *node = data;
	g_list_free(node->net_keys);
	g_list_free(node->app_keys);

	g_list_free_full(node->elements, element_free);

	if(node->comp)
		g_free(node->comp);

	g_free(node);
}

void node_free(struct mesh_node *node)
{
	if (!node)
		return;
	nodes = g_list_remove(nodes, node);
	free_node_resources(node);
}

void node_cleanup(void)
{
	g_list_free_full(nodes, free_node_resources);
	local_node = NULL;
}

bool node_is_provisioned(struct mesh_node *node)
{
	return (!IS_UNASSIGNED(node->primary));
}

void *node_get_prov(struct mesh_node *node)
{
	return node->prov;
}

void node_set_prov(struct mesh_node *node, void *prov)
{
	node->prov = prov;
}

bool node_app_key_add(struct mesh_node *node, uint16_t idx)
{
	uint32_t index;
	uint16_t net_idx;

	if (!node)
		return false;

	net_idx = keys_app_key_get_bound(idx);
	if (net_idx == NET_IDX_INVALID)
		return false;

	if (!g_list_find(node->net_keys, GUINT_TO_POINTER(net_idx)))
		return false;

	index = (net_idx << 16) + idx;

	if (g_list_find(node->app_keys, GUINT_TO_POINTER(index)))
		return false;

	node->app_keys = g_list_append(node->app_keys, GUINT_TO_POINTER(index));

	return true;
}

bool node_net_key_add(struct mesh_node *node, uint16_t index)
{
	if(!node)
		return false;

	if (g_list_find(node->net_keys, GUINT_TO_POINTER(index)))
		return false;

	node->net_keys = g_list_append(node->net_keys, GUINT_TO_POINTER(index));
	return true;
}

bool node_net_key_delete(struct mesh_node *node, uint16_t index)
{
	GList *l;

	if(!node)
		return false;

	l = g_list_find(node->net_keys, GUINT_TO_POINTER(index));
	if (!l)
		return false;

	node->net_keys = g_list_remove(node->net_keys,
						GUINT_TO_POINTER(index));
	/* TODO: remove all associated app keys and bindings */
	return true;
}

bool node_app_key_delete(struct mesh_node *node, uint16_t net_idx,
				uint16_t idx)
{
	GList *l;
	uint32_t index;

	if(!node)
		return false;

	index = (net_idx << 16) + idx;

	l = g_list_find(node->app_keys, GUINT_TO_POINTER(index));
	if (!l)
		return false;

	node->app_keys = g_list_remove(node->app_keys,
					GUINT_TO_POINTER(index));
	/* TODO: remove all associated bindings */
	return true;
}

void node_set_primary(struct mesh_node *node, uint16_t unicast)
{
	node->primary = unicast;
}

uint16_t node_get_primary(struct mesh_node *node)
{
	if (!node)
		return UNASSIGNED_ADDRESS;
	else
		return node->primary;
}

void node_set_device_key(struct mesh_node *node, uint8_t *key)

{
	if (!node || !key)
		return;

	memcpy(node->dev_key, key, 16);
}

uint8_t *node_get_device_key(struct mesh_node *node)
{
	if (!node)
		return NULL;
	else
		return node->dev_key;
}

void node_set_num_elements(struct mesh_node *node, uint8_t num_ele)
{
	node->num_ele = num_ele;
}

uint8_t node_get_num_elements(struct mesh_node *node)
{
	return node->num_ele;
}

GList *node_get_net_keys(struct mesh_node *node)
{
	if (!node)
		return NULL;
	else
		return node->net_keys;
}

GList *node_get_app_keys(struct mesh_node *node)
{
	if (!node)
		return NULL;
	else
		return node->app_keys;
}

bool node_parse_composition(struct mesh_node *node, uint8_t *data, uint16_t len)
{
	struct mesh_node_composition *comp;
	uint16_t features;
	int i;

	comp = g_malloc0(sizeof(struct mesh_node_composition));
	if (!comp)
		return false;

	/* skip page -- We only support Page Zero */
	data++;
	len--;

	comp->cid = get_le16(&data[0]);
	comp->pid = get_le16(&data[2]);
	comp->vid = get_le16(&data[4]);
	comp->crpl = get_le16(&data[6]);
	features = get_le16(&data[8]);
	data += 10;
	len -= 10;

	comp->relay = !!(features & MESH_FEATURE_RELAY);
	comp->proxy = !!(features & MESH_FEATURE_PROXY);
	comp->friend = !!(features & MESH_FEATURE_FRIEND);
	comp->lpn =  !!(features & MESH_FEATURE_LPN);

	for (i = 0;  i< node->num_ele; i++) {
		uint8_t m, v;
		uint32_t mod_id;
		uint16_t vendor_id;
		struct mesh_element *ele;
		ele = g_malloc0(sizeof(struct mesh_element));
		if (!ele)
			return false;

		ele->index = i;
		ele->loc = get_le16(data);
		data += 2;
		node->elements = g_list_append(node->elements, ele);

		m = *data++;
		v = *data++;
		len -= 2;

		while (len >= 2 && m--) {
			mod_id = get_le16(data);
			/* initialize uppper 16 bits to 0xffff for SIG models */
			mod_id |= 0xffff0000;
			if (!node_set_model(node, ele->index, mod_id))
				return false;
			data += 2;
			len -= 2;
		}
		while (len >= 4 && v--) {
			mod_id = get_le16(data + 2);
			vendor_id = get_le16(data);
			mod_id |= (vendor_id << 16);
			if (!node_set_model(node, ele->index, mod_id))
				return false;
			data += 4;
			len -= 4;
		}

	}

	node->comp = comp;
	return true;
}

bool node_set_local_node(struct mesh_node *node)
{
	if (local_node) {
		bt_shell_printf("Local node already registered\n");
		return false;
	}
	net_register_unicast(node->primary, node->num_ele);

	local_node = node;
	local_node->provisioner = true;

	return true;
}

struct mesh_node *node_get_local_node()
{
	return local_node;
}

uint16_t node_get_primary_net_idx(struct mesh_node *node)
{
	if (node == NULL)
		return NET_IDX_INVALID;

	return node->primary_net_idx;
}

static bool deliver_model_data(struct mesh_element* element, uint16_t src,
				uint16_t app_idx, uint8_t *data, uint16_t len)
{
	GList *l;

	for(l = element->models; l; l = l->next) {
		struct mesh_model *model = l->data;

		if (!g_list_find(model->bindings, GUINT_TO_POINTER(app_idx)))
			continue;

		if (model->cbs.recv &&
			model->cbs.recv(src, data, len, model->user_data))
			return true;
	}

	return false;
}

void node_local_data_handler(uint16_t src, uint32_t dst,
		uint32_t iv_index, uint32_t seq_num,
		uint16_t app_idx, uint8_t *data, uint16_t len)
{
	GList *l;
	bool res;
	uint64_t iv_seq;
	uint64_t iv_seq_remote;
	uint8_t ele_idx;
	struct mesh_element *element;
	struct mesh_node *remote;
	bool loopback;

	if (!local_node || seq_num > 0xffffff)
		return;

	iv_seq = iv_index << 24;
	iv_seq |= seq_num;

	remote = node_find_by_addr(src);

	if (!remote) {
		if (local_node->provisioner) {
			bt_shell_printf("Remote node unknown (%4.4x)\n", src);
			return;
		}

		remote = g_new0(struct mesh_node, 1);
		if (!remote)
			return;

		/* Not Provisioner; Assume all SRC elements stand alone */
		remote->primary = src;
		remote->num_ele = 1;
		nodes = g_list_append(nodes, remote);
	}

	loopback = (src < (local_node->primary + local_node->num_ele) &&
						src >= local_node->primary);

	if (!loopback) {
		iv_seq_remote = remote->iv_index << 24;
		iv_seq |= remote->seq_number;

		if (iv_seq_remote >= iv_seq) {
			bt_shell_printf("Replayed message detected "
					"(%016" PRIx64 " >= %016" PRIx64 ")\n",
							iv_seq_remote, iv_seq);
			return;
		}
	}

	if (IS_GROUP(dst) || IS_VIRTUAL(dst)) {
		/* TODO: if subscription address, deliver to subscribers */
		return;
	}

	if (IS_ALL_NODES(dst)) {
		ele_idx = 0;
	} else {
		if (dst >= (local_node->primary + local_node->num_ele) ||
			dst < local_node->primary)
				return;

		ele_idx = dst - local_node->primary;
	}

	l = g_list_find_custom(local_node->elements,
				GUINT_TO_POINTER(ele_idx), match_element_idx);

	/* This should not happen */
	if (!l)
		return;

	element = l->data;
	res = deliver_model_data(element, src, app_idx, data, len);

	if (res && !loopback) {
		/* TODO: Save remote in Replay Protection db */
		remote->iv_index = iv_index;
		remote->seq_number = seq_num;
		prov_db_node_set_iv_seq(remote, iv_index, seq_num);
	}
}

static gboolean restore_model_state(gpointer data)
{
	struct mesh_model *model = data;
	GList *l;
	struct mesh_model_ops *ops;

	ops = &model->cbs;

	if (model->bindings && ops->bind) {
		for (l = model->bindings; l; l = l->next) {
			if (ops->bind(GPOINTER_TO_UINT(l->data), ACTION_ADD) !=
				MESH_STATUS_SUCCESS)
				break;
		}
	}

	if (model->pub && ops->pub)
		ops->pub(model->pub);

	g_idle_remove_by_data(data);

	return true;

}

bool node_local_model_register(uint8_t ele_idx, uint16_t model_id,
				struct mesh_model_ops *ops, void *user_data)
{
	uint32_t id = 0xffff0000 | model_id;

	return node_local_vendor_model_register(ele_idx, id, ops, user_data);
}

bool node_local_vendor_model_register(uint8_t ele_idx, uint32_t model_id,
				struct mesh_model_ops *ops, void *user_data)
{
	struct mesh_element *ele;
	struct mesh_model *model;
	GList *l;

	if (!local_node)
		return false;

	l = g_list_find_custom(local_node->elements, GUINT_TO_POINTER(ele_idx),
				match_element_idx);
	if (!l)
		return false;

	ele = l->data;

	l = g_list_find_custom(ele->models, GUINT_TO_POINTER(model_id),
				match_model_id);
	if (!l)
		return false;

	model = l->data;
	model->cbs = *ops;
	model->user_data = user_data;

	if (model_id >= 0xffff0000)
		model_id = model_id & 0xffff;

	/* Silently assign device key binding to configuration models */
	if (model_id == CONFIG_SERVER_MODEL_ID ||
					model_id == CONFIG_CLIENT_MODEL_ID) {
		model->bindings = g_list_append(model->bindings,
						GUINT_TO_POINTER(APP_IDX_DEV));
	} else {
		g_idle_add(restore_model_state, model);
	}

	return true;
}

bool node_set_element(struct mesh_node *node, uint8_t ele_idx)
{
	struct mesh_element *ele;
	GList *l;

	if (!node)
		return false;

	l = g_list_find_custom(node->elements, GUINT_TO_POINTER(ele_idx),
				match_element_idx);
	if (l)
		return false;

	ele = g_malloc0(sizeof(struct mesh_element));
	if (!ele)
		return false;

	ele->index = ele_idx;
	node->elements = g_list_append(node->elements, ele);

	return true;
}

bool node_set_model(struct mesh_node *node, uint8_t ele_idx, uint32_t id)
{
	struct mesh_element *ele;
	struct mesh_model *model;
	GList *l;

	if (!node)
		return false;

	l = g_list_find_custom(node->elements, GUINT_TO_POINTER(ele_idx),
				match_element_idx);
	if (!l)
		return false;

	ele = l->data;

	l = g_list_find_custom(ele->models, GUINT_TO_POINTER(id),
				match_model_id);
	if (l)
		return true;

	model = g_malloc0(sizeof(struct mesh_model));
	if (!model)
		return false;

	model->id = id;
	ele->models = g_list_append(ele->models, model);

	return true;
}

bool node_set_composition(struct mesh_node *node,
				struct mesh_node_composition *comp)
{
	if (!node || !comp || node->comp)
		return false;

	node->comp = g_malloc0(sizeof(struct mesh_node_composition));
	if (!node->comp)
		return false;

	*(node->comp) = *comp;
	return true;
}

struct mesh_node_composition *node_get_composition(struct mesh_node *node)
{
	if (!node)
		return NULL;

	return node->comp;
}

static struct mesh_model *get_model(struct mesh_node *node, uint8_t ele_idx,
							uint32_t model_id)
{
	struct mesh_element *ele;
	GList *l;

	if (!node)
		return NULL;

	l = g_list_find_custom(node->elements, GUINT_TO_POINTER(ele_idx),
				match_element_idx);
	if (!l)
		return NULL;

	ele = l->data;

	l = g_list_find_custom(ele->models, GUINT_TO_POINTER(model_id),
				match_model_id);
	if (!l)
		return NULL;

	return l->data;

}

bool node_add_binding(struct mesh_node *node, uint8_t ele_idx,
			uint32_t model_id, uint16_t app_idx)
{
	struct mesh_model *model;
	GList *l;

	model = get_model(node, ele_idx, model_id);
	if (!model)
		return false;

	l = g_list_find(model->bindings, GUINT_TO_POINTER(app_idx));
	if (l)
		return false;

	if ((node == local_node) && model->cbs.bind) {
		if (!model->cbs.bind(app_idx, ACTION_ADD))
			return false;
	}

	model->bindings = g_list_append(model->bindings,
					GUINT_TO_POINTER(app_idx));
	return true;
}

bool node_add_subscription(struct mesh_node *node, uint8_t ele_idx,
			   uint32_t model_id, uint16_t addr)
{
	struct mesh_model *model;
	GList *l;

	model = get_model(node, ele_idx, model_id);
	if (!model)
		return false;

	l = g_list_find(model->subscriptions, GUINT_TO_POINTER(addr));
	if (l)
		return false;

	model->subscriptions = g_list_append(model->subscriptions,
					     GUINT_TO_POINTER(addr));
	return true;
}

uint8_t node_get_default_ttl(struct mesh_node *node)
{
	if (!node)
		return DEFAULT_TTL;
	else if (node == local_node)
		return net_get_default_ttl();
	else
		return node->ttl;
}

bool node_set_default_ttl(struct mesh_node *node, uint8_t ttl)
{
	if (!node)
		return false;

	node->ttl = ttl;

	if (node == local_node || local_node == NULL)
		return net_set_default_ttl(ttl);

	return true;
}

bool node_set_sequence_number(struct mesh_node *node, uint32_t seq)
{
	if (!node)
		return false;

	node->seq_number = seq;

	if (node == local_node || local_node == NULL)
		return net_set_seq_num(seq);

	return true;
}

uint32_t node_get_sequence_number(struct mesh_node *node)
{
	if (!node)
		return 0xffffffff;
	else if (node == local_node)
		return net_get_seq_num();

	return node->seq_number;
}

bool node_set_iv_index(struct mesh_node *node, uint32_t iv_index)
{
	if (!node)
		return false;

	node->iv_index = iv_index;
	return true;
}

uint32_t node_get_iv_index(struct mesh_node *node)
{
	bool update;

	if (!node)
		return 0xffffffff;
	else if (node == local_node)
		return net_get_iv_index(&update);
	return node->iv_index;
}

bool node_model_pub_set(struct mesh_node *node, uint8_t ele, uint32_t model_id,
						struct mesh_publication *pub)
{
	struct mesh_model *model;

	model = get_model(node, ele, model_id);
	if(!model)
		return false;

	if (!model->pub)
		model->pub = g_malloc0(sizeof(struct mesh_publication));
	if (!model)
		return false;

	memcpy(model->pub, pub, (sizeof(struct mesh_publication)));

	if((node == local_node) && model->cbs.pub)
		model->cbs.pub(pub);
	return true;
}

struct mesh_publication *node_model_pub_get(struct mesh_node *node, uint8_t ele,
							uint32_t model_id)
{
	struct mesh_model *model;

	model = get_model(node, ele, model_id);
	if(!model)
		return NULL;
	else
		return model->pub;
}

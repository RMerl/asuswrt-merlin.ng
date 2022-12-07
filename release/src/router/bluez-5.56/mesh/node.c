// SPDX-License-Identifier: LGPL-2.1-or-later
/*
 *
 *  BlueZ - Bluetooth protocol stack for Linux
 *
 *  Copyright (C) 2017-2020  Intel Corporation. All rights reserved.
 *
 *
 */

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#define _GNU_SOURCE
#include <dirent.h>
#include <limits.h>
#include <stdio.h>
#include <sys/time.h>

#include <ell/ell.h>

#include "mesh/mesh-defs.h"
#include "mesh/mesh.h"
#include "mesh/net.h"
#include "mesh/net-keys.h"
#include "mesh/appkey.h"
#include "mesh/mesh-config.h"
#include "mesh/provision.h"
#include "mesh/keyring.h"
#include "mesh/model.h"
#include "mesh/cfgmod.h"
#include "mesh/util.h"
#include "mesh/error.h"
#include "mesh/dbus.h"
#include "mesh/agent.h"
#include "mesh/manager.h"
#include "mesh/rpl.h"
#include "mesh/node.h"

#define MESH_NODE_PATH_PREFIX "/node"

/* Default values for a new locally created node */
#define DEFAULT_NEW_UNICAST 0x0001
#define DEFAULT_IV_INDEX 0x0000

/* Default element location: unknown */
#define DEFAULT_LOCATION 0x0000

enum request_type {
	REQUEST_TYPE_JOIN,
	REQUEST_TYPE_ATTACH,
	REQUEST_TYPE_CREATE,
	REQUEST_TYPE_IMPORT,
};

struct node_element {
	char *path;
	struct l_queue *models;
	uint16_t location;
	uint8_t idx;
};

struct node_composition {
	uint16_t cid;
	uint16_t pid;
	uint16_t vid;
	uint16_t crpl;
};

struct mesh_node {
	struct mesh_net *net;
	struct l_queue *elements;
	struct l_queue *pages;
	char *app_path;
	char *owner;
	char *obj_path;
	struct mesh_agent *agent;
	struct mesh_config *cfg;
	char *storage_dir;
	uint32_t disc_watch;
	uint32_t seq_number;
	bool busy;
	bool provisioner;
	uint16_t primary;
	struct node_composition comp;
	struct {
		uint16_t interval;
		uint8_t cnt;
		uint8_t mode;
	} relay;
	uint8_t uuid[16];
	uint8_t dev_key[16];
	uint8_t token[8];
	uint8_t num_ele;
	uint8_t ttl;
	uint8_t lpn;
	uint8_t proxy;
	uint8_t friend;
	uint8_t beacon;
};

struct node_import {
	uint8_t dev_key[16];
	uint8_t net_key[16];
	uint16_t net_idx;
	struct {
		bool ivu;
		bool kr;
	} flags;
	uint32_t iv_index;
	uint16_t unicast;
};

struct managed_obj_request {
	struct mesh_node *node;
	union {
		node_ready_func_t ready_cb;
		node_join_ready_func_t join_ready_cb;
	};
	struct l_dbus_message *pending_msg;
	enum request_type type;
	union {
		struct mesh_node *attach;
		struct node_import *import;
	};
};

struct send_options {
	bool segmented;
	uint16_t vendor_id;
};

static struct l_queue *nodes;

static bool match_device_uuid(const void *a, const void *b)
{
	const struct mesh_node *node = a;
	const uint8_t *uuid = b;

	return (memcmp(node->uuid, uuid, 16) == 0);
}

static bool match_token(const void *a, const void *b)
{
	const struct mesh_node *node = a;
	const uint64_t *token = b;
	const uint64_t tmp = l_get_be64(node->token);

	return *token == tmp;
}

static bool match_element_idx(const void *a, const void *b)
{
	const struct node_element *element = a;
	uint32_t index = L_PTR_TO_UINT(b);

	return (element->idx == index);
}

static int compare_element_idx(const void *a, const void *b, void *user_data)
{
	uint32_t a_idx = ((const struct node_element *)a)->idx;
	uint32_t b_idx = ((const struct node_element *)b)->idx;

	if (a_idx < b_idx)
		return -1;

	if (a_idx > b_idx)
		return 1;

	return 0;
}

static bool match_element_path(const void *a, const void *b)
{
	const struct node_element *element = a;
	const char *path = b;

	if (!element->path)
		return false;

	return (!strcmp(element->path, path));
}

struct mesh_node *node_find_by_uuid(uint8_t uuid[16])
{
	return l_queue_find(nodes, match_device_uuid, uuid);
}

struct mesh_node *node_find_by_token(uint64_t token)
{
	return l_queue_find(nodes, match_token, (void *) &token);
}

uint8_t *node_uuid_get(struct mesh_node *node)
{
	if (!node)
		return NULL;
	return node->uuid;
}

static void set_defaults(struct mesh_node *node)
{
	node->lpn = MESH_MODE_UNSUPPORTED;
	node->proxy = MESH_MODE_UNSUPPORTED;
	node->friend = (mesh_friendship_supported()) ? MESH_MODE_DISABLED :
							MESH_MODE_UNSUPPORTED;
	node->beacon = (mesh_beacon_enabled()) ? MESH_MODE_ENABLED :
							MESH_MODE_DISABLED;
	node->relay.mode = (mesh_relay_supported()) ? MESH_MODE_DISABLED :
							MESH_MODE_UNSUPPORTED;
	node->ttl = TTL_MASK;
	node->seq_number = DEFAULT_SEQUENCE_NUMBER;
}

static struct mesh_node *node_new(const uint8_t uuid[16])
{
	struct mesh_node *node;

	node = l_new(struct mesh_node, 1);
	node->net = mesh_net_new(node);
	node->elements = l_queue_new();
	node->pages = l_queue_new();
	memcpy(node->uuid, uuid, sizeof(node->uuid));
	set_defaults(node);

	return node;
}

static void free_element_path(void *a, void *b)
{
	struct node_element *element = a;

	l_free(element->path);
	element->path = NULL;
}

static void element_free(void *data)
{
	struct node_element *element = data;

	l_queue_destroy(element->models, mesh_model_free);
	l_free(element->path);
	l_free(element);
}

static void free_node_dbus_resources(struct mesh_node *node)
{
	if (!node)
		return;

	if (node->disc_watch) {
		l_dbus_remove_watch(dbus_get_bus(), node->disc_watch);
		node->disc_watch = 0;
	}

	l_queue_foreach(node->elements, free_element_path, NULL);
	l_free(node->owner);
	node->owner = NULL;
	l_free(node->app_path);
	node->app_path = NULL;

	if (node->obj_path) {
		l_dbus_object_remove_interface(dbus_get_bus(), node->obj_path,
							MESH_NODE_INTERFACE);

		l_dbus_object_remove_interface(dbus_get_bus(), node->obj_path,
						MESH_MANAGEMENT_INTERFACE);

		l_dbus_object_remove_interface(dbus_get_bus(), node->obj_path,
						L_DBUS_INTERFACE_PROPERTIES);

		l_free(node->obj_path);
		node->obj_path = NULL;
	}
}

static void free_node_resources(void *data)
{
	struct mesh_node *node = data;

	/* Unregister io callbacks */
	mesh_net_detach(node->net);


	/* In case of a provisioner, stop active scanning */
	if (node->provisioner)
		manager_scan_cancel(node);

	/* Free dynamic resources */
	free_node_dbus_resources(node);
	l_queue_destroy(node->elements, element_free);
	l_queue_destroy(node->pages, l_free);
	mesh_agent_remove(node->agent);
	mesh_config_release(node->cfg);
	mesh_net_free(node->net);
	l_free(node->storage_dir);
	l_free(node);
}

/*
 * This function is called to free resources and remove the
 * configuration files for the specified node.
 */
void node_remove(struct mesh_node *node)
{
	if (!node)
		return;

	l_queue_remove(nodes, node);

	mesh_config_destroy_nvm(node->cfg);

	free_node_resources(node);
}

static bool add_element_from_storage(struct mesh_node *node,
					struct mesh_config_element *db_ele)
{
	struct node_element *ele;

	ele = l_new(struct node_element, 1);
	if (!ele)
		return false;

	ele->idx = db_ele->index;
	ele->location = db_ele->location;
	ele->models = l_queue_new();
	l_queue_push_tail(node->elements, ele);

	if (!mesh_model_add_from_storage(node, ele->idx, ele->models,
							db_ele->models))
		return false;

	return true;
}

static bool add_elements_from_storage(struct mesh_node *node,
					struct mesh_config_node *db_node)
{
	const struct l_queue_entry *entry;

	entry = l_queue_get_entries(db_node->elements);

	for (; entry; entry = entry->next)
		if (!add_element_from_storage(node, entry->data))
			return false;

	return true;
}

static void set_net_key(void *a, void *b)
{
	struct mesh_config_netkey *netkey = a;
	struct mesh_node *node = b;

	mesh_net_set_key(node->net, netkey->idx, netkey->key, netkey->new_key,
								netkey->phase);
}

static void set_appkey(void *a, void *b)
{
	struct mesh_config_appkey *appkey = a;
	struct mesh_node *node = b;

	appkey_key_init(node->net, appkey->net_idx, appkey->app_idx,
						appkey->key, appkey->new_key);
}

static bool init_storage_dir(struct mesh_node *node)
{
	char uuid[33];
	char dir_name[PATH_MAX];

	if (node->storage_dir)
		return true;

	if (!hex2str(node->uuid, 16, uuid, sizeof(uuid)))
		return false;

	snprintf(dir_name, PATH_MAX, "%s/%s", mesh_get_storage_dir(), uuid);

	if (strlen(dir_name) >= PATH_MAX)
		return false;

	create_dir(dir_name);

	node->storage_dir = l_strdup(dir_name);

	/* Initialize directory for storing RPL info */
	return rpl_init(node->storage_dir);
}

static void update_net_settings(struct mesh_node *node)
{
	struct mesh_net *net = node->net;

	mesh_net_set_proxy_mode(net, node->proxy == MESH_MODE_ENABLED);

	mesh_net_set_friend_mode(net, node->friend == MESH_MODE_ENABLED);

	mesh_net_set_relay_mode(net, node->relay.mode == MESH_MODE_ENABLED,
					node->relay.cnt, node->relay.interval);

	mesh_net_set_beacon_mode(net, node->beacon == MESH_MODE_ENABLED);
}

static bool init_from_storage(struct mesh_config_node *db_node,
			const uint8_t uuid[16], struct mesh_config *cfg,
			void *user_data)
{
	unsigned int num_ele;

	struct mesh_node *node = node_new(uuid);

	if (!nodes)
		nodes = l_queue_new();

	l_queue_push_tail(nodes, node);

	node->comp.cid = db_node->cid;
	node->comp.pid = db_node->pid;
	node->comp.vid = db_node->vid;
	node->comp.crpl = db_node->crpl;
	node->lpn = db_node->modes.lpn;

	node->proxy = db_node->modes.proxy;
	node->friend = db_node->modes.friend;
	node->relay.mode = db_node->modes.relay.state;
	node->relay.cnt = db_node->modes.relay.cnt;
	node->relay.interval = db_node->modes.relay.interval;
	node->beacon = db_node->modes.beacon;

	l_debug("relay %2.2x, proxy %2.2x, lpn %2.2x, friend %2.2x",
			node->relay.mode, node->proxy, node->lpn, node->friend);
	node->ttl = db_node->ttl;
	node->seq_number = db_node->seq_number;

	memcpy(node->dev_key, db_node->dev_key, 16);
	memcpy(node->token, db_node->token, 8);

	num_ele = l_queue_length(db_node->elements);
	if (num_ele > MAX_ELE_COUNT)
		goto fail;

	node->num_ele = num_ele;

	if (num_ele != 0 && !add_elements_from_storage(node, db_node))
		goto fail;

	node->primary = db_node->unicast;

	if (!db_node->netkeys)
		goto fail;

	if (!IS_UNASSIGNED(node->primary) &&
		!mesh_net_register_unicast(node->net, node->primary, num_ele))
		goto fail;

	mesh_net_set_iv_index(node->net, db_node->iv_index, db_node->iv_update);

	/* Initialize directory for storing keyring and RPL info */
	if (!init_storage_dir(node) || !mesh_net_load_rpl(node->net))
		goto fail;

	if (db_node->net_transmit)
		mesh_net_transmit_params_set(node->net,
					db_node->net_transmit->count,
					db_node->net_transmit->interval);

	l_queue_foreach(db_node->netkeys, set_net_key, node);

	l_queue_foreach(db_node->appkeys, set_appkey, node);

	while (l_queue_length(db_node->pages)) {
		struct mesh_config_comp_page *page;

		/* Move the composition pages to the node struct */
		page = l_queue_pop_head(db_node->pages);
		l_queue_push_tail(node->pages, page);
	}

	mesh_net_set_seq_num(node->net, node->seq_number);
	mesh_net_set_default_ttl(node->net, node->ttl);

	update_net_settings(node);

	/* Initialize configuration server model */
	cfgmod_server_init(node, PRIMARY_ELE_IDX);

	node->cfg = cfg;

	return true;
fail:
	node_remove(node);
	return false;
}

static void cleanup_node(void *data)
{
	struct mesh_node *node = data;
	uint32_t seq_num = mesh_net_get_seq_num(node->net);

	/* Preserve the last used sequence number */
	mesh_config_write_seq_number(node->cfg, seq_num, false);

	free_node_resources(node);
}

/*
 * This function is called to free resources and write the current
 * sequence numbers to the configuration file for each known node.
 */
void node_cleanup_all(void)
{
	l_queue_destroy(nodes, cleanup_node);
	l_dbus_unregister_interface(dbus_get_bus(), MESH_NODE_INTERFACE);
	l_dbus_unregister_interface(dbus_get_bus(), MESH_MANAGEMENT_INTERFACE);
}

bool node_is_provisioner(struct mesh_node *node)
{
	return node->provisioner;
}

bool node_is_busy(struct mesh_node *node)
{
	return node->busy;
}

void node_app_key_delete(struct mesh_node *node, uint16_t net_idx,
							uint16_t app_idx)
{
	const struct l_queue_entry *entry;

	entry = l_queue_get_entries(node->elements);
	for (; entry; entry = entry->next) {
		struct node_element *ele = entry->data;

		mesh_model_app_key_delete(node, ele->idx, ele->models, app_idx);
	}
}

uint16_t node_get_primary(struct mesh_node *node)
{
	if (!node)
		return UNASSIGNED_ADDRESS;
	else
		return node->primary;
}

const uint8_t *node_get_device_key(struct mesh_node *node)
{
	if (!node)
		return NULL;
	else
		return node->dev_key;
}

void node_set_token(struct mesh_node *node, uint8_t token[8])
{
	memcpy(node->token, token, 8);
}

const uint8_t *node_get_token(struct mesh_node *node)
{
	if (!node)
		return NULL;
	else
		return node->token;
}

uint8_t node_get_num_elements(struct mesh_node *node)
{
	return node->num_ele;
}

struct l_queue *node_get_element_models(struct mesh_node *node, uint8_t ele_idx)
{
	struct node_element *ele;

	if (!node)
		return NULL;

	ele = l_queue_find(node->elements, match_element_idx,
							L_UINT_TO_PTR(ele_idx));
	if (!ele)
		return NULL;

	return ele->models;
}

uint8_t node_default_ttl_get(struct mesh_node *node)
{
	if (!node)
		return TTL_MASK;
	return node->ttl;
}

bool node_default_ttl_set(struct mesh_node *node, uint8_t ttl)
{
	bool res;

	if (!node)
		return false;

	res = mesh_config_write_ttl(node->cfg, ttl);

	if (res) {
		node->ttl = ttl;
		mesh_net_set_default_ttl(node->net, ttl);
	}

	return res;
}

bool node_set_sequence_number(struct mesh_node *node, uint32_t seq)
{
	if (!node)
		return false;

	node->seq_number = seq;

	return mesh_config_write_seq_number(node->cfg, node->seq_number, true);
}

uint32_t node_get_sequence_number(struct mesh_node *node)
{
	if (!node)
		return 0xffffffff;

	return node->seq_number;
}

int node_get_element_idx(struct mesh_node *node, uint16_t ele_addr)
{
	uint16_t addr;
	uint8_t num_ele;

	if (!node)
		return -1;

	num_ele = node_get_num_elements(node);
	if (!num_ele)
		return -2;

	addr = node_get_primary(node);

	if (ele_addr < addr || ele_addr >= addr + num_ele)
		return -3;
	else
		return ele_addr - addr;
}

uint16_t node_get_crpl(struct mesh_node *node)
{
	if (!node)
		return 0;

	return node->comp.crpl;
}

uint8_t node_relay_mode_get(struct mesh_node *node, uint8_t *count,
							uint16_t *interval)
{
	if (!node) {
		*count = 0;
		*interval = 0;
		return MESH_MODE_DISABLED;
	}

	*count = node->relay.cnt;
	*interval = node->relay.interval;
	return node->relay.mode;
}

uint8_t node_lpn_mode_get(struct mesh_node *node)
{
	if (!node)
		return MESH_MODE_DISABLED;

	return node->lpn;
}

bool node_relay_mode_set(struct mesh_node *node, bool enable, uint8_t cnt,
							uint16_t interval)
{
	bool res;

	if (!node || node->relay.mode == MESH_MODE_UNSUPPORTED)
		return false;

	res = mesh_config_write_relay_mode(node->cfg, enable, cnt, interval);

	if (res) {
		node->relay.mode = enable ? MESH_MODE_ENABLED :
							MESH_MODE_DISABLED;
		node->relay.cnt = cnt;
		node->relay.interval = interval;
		mesh_net_set_relay_mode(node->net, enable, cnt, interval);
	}

	return res;
}

bool node_proxy_mode_set(struct mesh_node *node, bool enable)
{
	bool res;
	uint8_t proxy;

	if (!node || node->proxy == MESH_MODE_UNSUPPORTED)
		return false;

	proxy = enable ? MESH_MODE_ENABLED : MESH_MODE_DISABLED;
	res = mesh_config_write_mode(node->cfg, "proxy", proxy);

	if (res) {
		node->proxy = proxy;
		mesh_net_set_proxy_mode(node->net, enable);
	}

	return res;
}

uint8_t node_proxy_mode_get(struct mesh_node *node)
{
	if (!node)
		return MESH_MODE_DISABLED;

	return node->proxy;
}

bool node_beacon_mode_set(struct mesh_node *node, bool enable)
{
	bool res;
	uint8_t beacon;

	if (!node)
		return false;

	beacon = enable ? MESH_MODE_ENABLED : MESH_MODE_DISABLED;
	res = mesh_config_write_mode(node->cfg, "beacon", beacon);

	if (res) {
		node->beacon = beacon;
		mesh_net_set_beacon_mode(node->net, enable);
	}

	return res;
}

uint8_t node_beacon_mode_get(struct mesh_node *node)
{
	if (!node)
		return MESH_MODE_DISABLED;

	return node->beacon;
}

bool node_friend_mode_set(struct mesh_node *node, bool enable)
{
	bool res;
	uint8_t friend;

	if (!node || node->friend == MESH_MODE_UNSUPPORTED)
		return false;

	friend = enable ? MESH_MODE_ENABLED : MESH_MODE_DISABLED;
	res = mesh_config_write_mode(node->cfg, "friend", friend);

	if (res) {
		node->friend = friend;
		mesh_net_set_friend_mode(node->net, enable);
	}

	return res;
}

uint8_t node_friend_mode_get(struct mesh_node *node)
{
	if (!node)
		return MESH_MODE_DISABLED;

	return node->friend;
}

static uint16_t generate_node_comp(struct mesh_node *node, uint8_t *buf,
								uint16_t sz)
{
	uint16_t n, features, num_ele = 0;
	const struct l_queue_entry *entry;

	n = 0;

	l_put_le16(node->comp.cid, buf + n);
	n += 2;
	l_put_le16(node->comp.pid, buf + n);
	n += 2;
	l_put_le16(node->comp.vid, buf + n);
	n += 2;
	l_put_le16(node->comp.crpl, buf + n);
	n += 2;

	features = 0;

	if (node->relay.mode != MESH_MODE_UNSUPPORTED)
		features |= FEATURE_RELAY;
	if (node->proxy != MESH_MODE_UNSUPPORTED)
		features |= FEATURE_PROXY;
	if (node->friend != MESH_MODE_UNSUPPORTED)
		features |= FEATURE_FRIEND;
	if (node->lpn != MESH_MODE_UNSUPPORTED)
		features |= FEATURE_LPN;

	l_put_le16(features, buf + n);
	n += 2;

	entry = l_queue_get_entries(node->elements);

	for (; entry; entry = entry->next) {
		struct node_element *ele = entry->data;

		if (ele->idx != num_ele)
			return 0;

		num_ele++;

		/* At least fit location and zeros for number of models */
		if ((n + 4) > sz)
			return n;

		l_put_le16(ele->location, buf + n);
		n += 2;

		n += mesh_model_generate_composition(ele->models, sz - n,
								buf + n);
	}

	if (!num_ele)
		return 0;

	return n;
}

static bool match_page(const void *a, const void *b)
{
	const struct mesh_config_comp_page *page = a;
	uint8_t page_num = L_PTR_TO_UINT(b);

	return page->page_num == page_num;
}

static void convert_node_to_storage(struct mesh_node *node,
					struct mesh_config_node *db_node)
{
	const struct l_queue_entry *entry;

	memset(db_node, 0, sizeof(struct mesh_config_node));

	db_node->cid = node->comp.cid;
	db_node->pid = node->comp.pid;
	db_node->vid = node->comp.vid;
	db_node->crpl = node->comp.crpl;
	db_node->modes.lpn = node->lpn;
	db_node->modes.proxy = node->proxy;

	db_node->modes.friend = node->friend;
	db_node->modes.relay.state = node->relay.mode;
	db_node->modes.relay.cnt = node->relay.cnt;
	db_node->modes.relay.interval = node->relay.interval;
	db_node->modes.beacon = node->beacon;

	db_node->ttl = node->ttl;
	db_node->seq_number = node->seq_number;

	db_node->elements = l_queue_new();

	entry = l_queue_get_entries(node->elements);

	for (; entry; entry = entry->next) {
		struct node_element *ele = entry->data;
		struct mesh_config_element *db_ele;

		db_ele = l_new(struct mesh_config_element, 1);

		db_ele->index = ele->idx;
		db_ele->location = ele->location;
		db_ele->models = l_queue_new();

		mesh_model_convert_to_storage(db_ele->models, ele->models);

		l_queue_push_tail(db_node->elements, db_ele);
	}

}

static bool create_node_config(struct mesh_node *node, const uint8_t uuid[16])
{
	struct mesh_config_node db_node;
	const struct l_queue_entry *entry;
	const char *storage_dir;

	convert_node_to_storage(node, &db_node);
	storage_dir = mesh_get_storage_dir();
	node->cfg = mesh_config_create(storage_dir, uuid, &db_node);

	if (node->cfg)
		init_storage_dir(node);

	/* Free temporarily allocated resources */
	entry = l_queue_get_entries(db_node.elements);

	for (; entry; entry = entry->next) {
		struct mesh_config_element *db_ele = entry->data;

		l_queue_destroy(db_ele->models, l_free);
	}

	l_queue_destroy(db_node.elements, l_free);

	return node->cfg != NULL;
}

static bool set_node_comp(struct mesh_node *node, uint8_t page_num,
					const uint8_t *data, uint16_t len)
{
	struct mesh_config_comp_page *page;

	if (len < MIN_COMP_SIZE)
		return false;

	page = l_queue_remove_if(node->pages, match_page,
						L_UINT_TO_PTR(page_num));

	l_free(page);

	page = l_malloc(sizeof(struct mesh_config_comp_page) + len);
	page->len = len;
	page->page_num = page_num;
	memcpy(page->data, data, len);
	l_queue_push_tail(node->pages, page);

	return mesh_config_comp_page_add(node->cfg, page_num, page->data, len);
}

static bool create_node_comp(struct mesh_node *node)
{
	uint16_t len;
	uint8_t comp[MAX_MSG_LEN - 2];

	len = generate_node_comp(node, comp, sizeof(comp));

	return set_node_comp(node, 0, comp, len);
}

const uint8_t *node_get_comp(struct mesh_node *node, uint8_t page_num,
								uint16_t *len)
{
	struct mesh_config_comp_page *page = NULL;

	if (node)
		page = l_queue_find(node->pages, match_page,
						L_UINT_TO_PTR(page_num));

	if (!page) {
		*len = 0;
		return NULL;
	}

	*len = page->len;
	return page->data;
}

bool node_replace_comp(struct mesh_node *node, uint8_t retire, uint8_t with)
{
	struct mesh_config_comp_page *old_page, *keep;

	if (!node)
		return false;

	keep = l_queue_find(node->pages, match_page, L_UINT_TO_PTR(with));

	if (!keep)
		return false;

	old_page = l_queue_remove_if(node->pages, match_page,
							L_UINT_TO_PTR(retire));

	l_free(old_page);
	keep->page_num = retire;
	mesh_config_comp_page_mv(node->cfg, with, retire);

	return true;
}

static void attach_io(void *a, void *b)
{
	struct mesh_node *node = a;
	struct mesh_io *io = b;

	if (node->net)
		mesh_net_attach(node->net, io);
}

/* Register callbacks for all nodes io */
void node_attach_io_all(struct mesh_io *io)
{
	l_queue_foreach(nodes, attach_io, io);
}

/* Register node object with D-Bus */
static bool register_node_object(struct mesh_node *node)
{
	char uuid[33];

	if (!hex2str(node->uuid, sizeof(node->uuid), uuid, sizeof(uuid)))
		return false;

	node->obj_path = l_strdup_printf(BLUEZ_MESH_PATH MESH_NODE_PATH_PREFIX
								"%s", uuid);

	if (!l_dbus_object_add_interface(dbus_get_bus(), node->obj_path,
						MESH_NODE_INTERFACE, node))
		return false;

	if (!l_dbus_object_add_interface(dbus_get_bus(), node->obj_path,
					MESH_MANAGEMENT_INTERFACE, node))
		return false;

	if (!l_dbus_object_add_interface(dbus_get_bus(), node->obj_path,
					L_DBUS_INTERFACE_PROPERTIES, NULL))
		return false;

	return true;
}

static void app_disc_cb(struct l_dbus *bus, void *user_data)
{
	struct mesh_node *node = user_data;

	l_info("App %s disconnected (%u)", node->owner, node->disc_watch);

	node->disc_watch = 0;

	/* In case of a provisioner, stop active scanning */
	if (node->provisioner)
		manager_scan_cancel(node);

	free_node_dbus_resources(node);
}

static bool get_sig_models_from_properties(struct mesh_node *node,
					struct node_element *ele,
					struct l_dbus_message_iter *property)
{
	struct l_dbus_message_iter mods, var;
	uint16_t m_id;

	if (!ele->models)
		ele->models = l_queue_new();

	if (!l_dbus_message_iter_get_variant(property, "a(qa{sv})", &mods))
		return false;

	/* Bluetooth SIG defined models */
	while (l_dbus_message_iter_next_entry(&mods, &m_id, &var)) {
		uint32_t id = SET_ID(SIG_VENDOR, m_id);

		/* Allow Config Server Model only on the primary element */
		if (ele->idx != PRIMARY_ELE_IDX && id == CONFIG_SRV_MODEL)
			return false;

		if (!mesh_model_add(node, ele->models, id, &var))
			return false;
	}

	return true;
}

static bool get_vendor_models_from_properties(struct mesh_node *node,
					struct node_element *ele,
					struct l_dbus_message_iter *property)
{
	struct l_dbus_message_iter mods, var;
	uint16_t m_id, v_id;

	if (!ele->models)
		ele->models = l_queue_new();

	if (!l_dbus_message_iter_get_variant(property, "a(qqa{sv})", &mods))
		return false;

	/* Vendor defined models */
	while (l_dbus_message_iter_next_entry(&mods, &v_id, &m_id, &var)) {
		uint32_t id = SET_ID(v_id, m_id);

		if (!mesh_model_add(node, ele->models, id, &var))
			return false;
	}

	return true;
}

static bool get_element_properties(struct mesh_node *node, const char *path,
					struct l_dbus_message_iter *properties)
{
	struct node_element *ele = l_new(struct node_element, 1);
	const char *key;
	struct l_dbus_message_iter var;
	bool idx = false;
	bool mods = false;
	bool vendor_mods = false;

	l_debug("path %s", path);

	ele->location = DEFAULT_LOCATION;

	while (l_dbus_message_iter_next_entry(properties, &key, &var)) {
		if (!strcmp(key, "Index")) {

			if (idx || !l_dbus_message_iter_get_variant(&var, "y",
								&ele->idx))
				goto fail;

			idx = true;

		} else if (!strcmp(key, "Models")) {

			if (mods)
				goto fail;

			if (!get_sig_models_from_properties(node, ele, &var))
				goto fail;

			mods = true;
		} else if (!strcmp(key, "VendorModels")) {

			if (vendor_mods)
				goto fail;

			if (!get_vendor_models_from_properties(node, ele, &var))
				goto fail;

			vendor_mods = true;

		} else if (!strcmp(key, "Location")) {
			if (!l_dbus_message_iter_get_variant(&var, "q",
							&ele->location))
				goto fail;
		}
	}

	/* Check for the presence of the required properties */
	if (!idx || !mods || !vendor_mods)
		goto fail;

	if (l_queue_find(node->elements, match_element_idx,
						L_UINT_TO_PTR(ele->idx)))
		goto fail;

	l_queue_insert(node->elements, ele, compare_element_idx, NULL);

	ele->path = l_strdup(path);

	/*
	 * Add configuration server model on the primary element.
	 * We allow the application not to specify the presense of
	 * the Configuration Server model, since it's implemented by the
	 * daemon. If the model is present in the application properties,
	 * the operation below will be a "no-op".
	 */
	if (ele->idx == PRIMARY_ELE_IDX)
		mesh_model_add(node, ele->models, CONFIG_SRV_MODEL, NULL);

	return true;
fail:
	l_free(ele);

	return false;
}

static bool get_app_properties(struct mesh_node *node, const char *path,
					struct l_dbus_message_iter *properties)
{
	const char *key;
	struct l_dbus_message_iter variant;
	bool cid = false;
	bool pid = false;
	bool vid = false;

	l_debug("path %s", path);

	node->comp.crpl = mesh_get_crpl();

	while (l_dbus_message_iter_next_entry(properties, &key, &variant)) {
		if (!cid && !strcmp(key, "CompanyID")) {
			if (!l_dbus_message_iter_get_variant(&variant, "q",
							&node->comp.cid))
				return false;
			cid = true;
			continue;
		}

		if (!pid && !strcmp(key, "ProductID")) {
			if (!l_dbus_message_iter_get_variant(&variant, "q",
							&node->comp.pid))
				return false;
			pid = true;
			continue;
		}

		if (!vid && !strcmp(key, "VersionID")) {
			if (!l_dbus_message_iter_get_variant(&variant, "q",
							&node->comp.vid))
				return false;
			vid = true;
			continue;
		}

		if (!strcmp(key, "CRPL")) {
			if (!l_dbus_message_iter_get_variant(&variant, "q",
							&node->comp.crpl))
				return false;
			continue;
		}
	}

	if (!cid || !pid || !vid)
		return false;

	return true;
}

static bool add_local_node(struct mesh_node *node, uint16_t unicast, bool kr,
				bool ivu, uint32_t iv_idx, uint8_t dev_key[16],
				uint16_t net_key_idx, uint8_t net_key[16])
{
	if (!nodes)
		nodes = l_queue_new();

	l_queue_push_tail(nodes, node);

	if (!mesh_config_write_iv_index(node->cfg, iv_idx, ivu))
		return false;

	mesh_net_set_iv_index(node->net, iv_idx, ivu);

	if (!mesh_config_write_unicast(node->cfg, unicast))
		return false;

	l_getrandom(node->token, sizeof(node->token));
	if (!mesh_config_write_token(node->cfg, node->token))
		return false;

	memcpy(node->dev_key, dev_key, 16);
	if (!mesh_config_write_device_key(node->cfg, dev_key))
		return false;

	node->primary = unicast;
	mesh_net_register_unicast(node->net, unicast, node->num_ele);

	if (mesh_net_add_key(node->net, net_key_idx, net_key) !=
							MESH_STATUS_SUCCESS)
		return false;

	if (kr) {
		/* Duplicate net key, if the key refresh is on */
		if (mesh_net_update_key(node->net, net_key_idx, net_key) !=
							MESH_STATUS_SUCCESS)
			return false;

		if (!mesh_config_net_key_set_phase(node->cfg, net_key_idx,
							KEY_REFRESH_PHASE_TWO))
			return false;
	}

	update_net_settings(node);

	/* Initialize configuration server model */
	cfgmod_server_init(node, PRIMARY_ELE_IDX);

	node->busy = true;

	return true;
}

static void update_composition(struct mesh_node *node, struct mesh_node *attach)
{
	if (node->comp.cid != attach->comp.cid)
		mesh_config_update_company_id(attach->cfg, node->comp.cid);

	if (node->comp.pid != attach->comp.pid)
		mesh_config_update_product_id(attach->cfg, node->comp.pid);

	if (node->comp.vid != attach->comp.vid)
		mesh_config_update_version_id(attach->cfg, node->comp.vid);

	if (node->comp.crpl != attach->comp.crpl)
		mesh_config_update_crpl(attach->cfg, node->comp.crpl);

	attach->comp = node->comp;
}

static void update_model_options(struct mesh_node *node,
						struct mesh_node *attach)
{
	uint32_t len, i;
	struct node_element *ele, *ele_attach;

	len = l_queue_length(node->elements);

	for (i = 0; i < len; i++) {

		ele = l_queue_find(node->elements, match_element_idx,
							L_UINT_TO_PTR(i));
		ele_attach = l_queue_find(attach->elements, match_element_idx,
							L_UINT_TO_PTR(i));
		if (!ele || !ele_attach)
			continue;

		mesh_model_update_opts(node, ele->idx, ele_attach->models,
								ele->models);
	}
}

static bool check_req_node(struct managed_obj_request *req)
{
	const int offset = 8;
	uint16_t node_len, len;
	uint8_t comp[MAX_MSG_LEN - 2];
	const uint8_t *node_comp;

	len = generate_node_comp(req->node, comp, sizeof(comp));

	if (len < MIN_COMP_SIZE)
		return false;

	node_comp = node_get_comp(req->attach, 0, &node_len);

	/* If no page 0 exists, create it and accept */
	if (!node_len || !node_comp)
		return set_node_comp(req->attach, 0, comp, len);

	/* Test Element/Model part of composition and reject if changed */
	if (node_len != len || memcmp(&node_comp[offset], &comp[offset],
							node_len - offset))
		return false;

	/* If comp has changed, but not Element/Models, resave and accept */
	else if (memcmp(node_comp, comp, node_len))
		return set_node_comp(req->attach, 0, comp, len);

	/* Nothing has changed */
	return true;
}

static bool attach_req_node(struct mesh_node *attach, struct mesh_node *node)
{
	const struct l_queue_entry *attach_entry;
	const struct l_queue_entry *node_entry;

	attach->obj_path = node->obj_path;
	node->obj_path = NULL;

	if (!register_node_object(attach)) {
		free_node_dbus_resources(attach);
		return false;
	}

	attach_entry = l_queue_get_entries(attach->elements);
	node_entry = l_queue_get_entries(node->elements);

	/*
	 * Update existing node with paths collected in temporary node,
	 * then remove the temporary.
	 */
	while (attach_entry && node_entry) {
		struct node_element *attach_ele = attach_entry->data;
		struct node_element *node_ele = node_entry->data;

		attach_ele->path = node_ele->path;
		node_ele->path = NULL;

		attach_entry = attach_entry->next;
		node_entry = node_entry->next;
	}

	mesh_agent_remove(attach->agent);
	attach->agent = node->agent;
	node->agent = NULL;

	attach->provisioner = node->provisioner;

	attach->app_path = node->app_path;
	node->app_path = NULL;

	attach->owner = node->owner;
	node->owner = NULL;

	update_composition(node, attach);
	update_model_options(node, attach);

	node_remove(node);

	return true;
}

static void get_managed_objects_cb(struct l_dbus_message *msg, void *user_data)
{
	struct l_dbus_message_iter objects, interfaces;
	struct managed_obj_request *req = user_data;
	const char *path;
	struct mesh_node *node = req->node;
	struct node_import *import;
	bool have_app = false;
	unsigned int num_ele;
	struct keyring_net_key net_key;
	uint8_t dev_key[16];

	if (req->type == REQUEST_TYPE_ATTACH)
		req->attach->busy = false;

	if (!msg || l_dbus_message_is_error(msg)) {
		l_error("Failed to get app's dbus objects");
		goto fail;
	}

	if (!l_dbus_message_get_arguments(msg, "a{oa{sa{sv}}}", &objects)) {
		l_error("Failed to parse app's dbus objects");
		goto fail;
	}

	while (l_dbus_message_iter_next_entry(&objects, &path, &interfaces)) {
		struct l_dbus_message_iter properties;
		const char *interface;

		while (l_dbus_message_iter_next_entry(&interfaces, &interface,
								&properties)) {
			bool res;

			if (!strcmp(MESH_ELEMENT_INTERFACE, interface)) {
				res = get_element_properties(node, path,
								&properties);
				if (!res)
					goto fail;
			} else if (!strcmp(MESH_APPLICATION_INTERFACE,
								interface)) {
				if (have_app)
					goto fail;

				req->node->app_path = l_strdup(path);

				res = get_app_properties(node, path,
								&properties);
				if (!res)
					goto fail;

				have_app = true;

			} else if (!strcmp(MESH_PROVISION_AGENT_INTERFACE,
								interface)) {
				const char *sender;

				sender = l_dbus_message_get_sender(msg);
				node->agent = mesh_agent_create(path, sender,
								&properties);
				if (!node->agent)
					goto fail;

			} else if (!strcmp(MESH_PROVISIONER_INTERFACE,
								interface)) {
				node->provisioner = true;
			}
		}
	}

	if (!have_app) {
		l_error("Interface %s not found", MESH_APPLICATION_INTERFACE);
		goto fail;
	}

	if (l_queue_isempty(node->elements)) {
		l_error("Interface %s not found", MESH_ELEMENT_INTERFACE);
		goto fail;
	}

	if (!l_queue_find(node->elements, match_element_idx,
				L_UINT_TO_PTR(PRIMARY_ELE_IDX))) {

		l_debug("Primary element not detected");
		goto fail;
	}

	num_ele = l_queue_length(node->elements);

	if (num_ele > MAX_ELE_COUNT)
		goto fail;

	node->num_ele = num_ele;

	if (req->type != REQUEST_TYPE_ATTACH) {
		/* Generate node configuration for a brand new node */
		if (!create_node_config(node, node->uuid))
			goto fail;

		/* Create node composition */
		if (!create_node_comp(node))
			goto fail;
	} else if (!check_req_node(req))
		/* Check the integrity of the node composition */
		goto fail;

	switch (req->type) {
	case REQUEST_TYPE_ATTACH:
		if (!attach_req_node(req->attach, node))
			goto fail;

		req->attach->disc_watch = l_dbus_add_disconnect_watch(
					dbus_get_bus(), req->attach->owner,
					app_disc_cb, req->attach, NULL);

		req->ready_cb(req->pending_msg, MESH_ERROR_NONE, req->attach);
		return;

	case REQUEST_TYPE_JOIN:
		if (!node->agent) {
			l_error("Interface %s not found",
						MESH_PROVISION_AGENT_INTERFACE);
			goto fail;
		}

		req->join_ready_cb(node, node->agent);

		return;

	case REQUEST_TYPE_IMPORT:
		import = req->import;
		if (!add_local_node(node, import->unicast, import->flags.kr,
					import->flags.ivu,
					import->iv_index, import->dev_key,
					import->net_idx, import->net_key))
			goto fail;

		req->ready_cb(req->pending_msg, MESH_ERROR_NONE, node);
		l_free(import);

		return;

	case REQUEST_TYPE_CREATE:
		/* Generate device and primary network keys */
		l_getrandom(dev_key, sizeof(dev_key));
		l_getrandom(net_key.old_key, sizeof(net_key.old_key));
		memcpy(net_key.new_key, net_key.old_key,
						sizeof(net_key.old_key));
		net_key.net_idx = PRIMARY_NET_IDX;
		net_key.phase = KEY_REFRESH_PHASE_NONE;

		if (!add_local_node(node, DEFAULT_NEW_UNICAST, false, false,
						DEFAULT_IV_INDEX, dev_key,
						PRIMARY_NET_IDX,
						net_key.old_key))
			goto fail;

		if (!keyring_put_remote_dev_key(node, DEFAULT_NEW_UNICAST,
						node->num_ele, dev_key))
			goto fail;

		if (!keyring_put_net_key(node, PRIMARY_NET_IDX, &net_key))
			goto fail;

		req->ready_cb(req->pending_msg, MESH_ERROR_NONE, node);
		return;

	default:
		goto fail;
	}

fail:
	/* Handle failed requests */
	node_remove(node);

	if (req->type == REQUEST_TYPE_JOIN)
		req->join_ready_cb(NULL, NULL);
	else
		req->ready_cb(req->pending_msg, MESH_ERROR_FAILED, NULL);

	if (req->type == REQUEST_TYPE_IMPORT)
		l_free(req->import);
}

static void send_managed_objects_request(const char *destination,
						const char *path,
						struct managed_obj_request *req)
{
	struct l_dbus_message *msg;

	msg = l_dbus_message_new_method_call(dbus_get_bus(), destination, path,
						L_DBUS_INTERFACE_OBJECT_MANAGER,
						"GetManagedObjects");
	l_dbus_message_set_arguments(msg, "");
	dbus_send_with_timeout(dbus_get_bus(), msg, get_managed_objects_cb,
					req, l_free, DEFAULT_DBUS_TIMEOUT);
}

/* Establish relationship between application and mesh node */
void node_attach(const char *app_root, const char *sender, uint64_t token,
					node_ready_func_t cb, void *user_data)
{
	struct managed_obj_request *req;
	struct mesh_node *node;

	node = l_queue_find(nodes, match_token, (void *) &token);
	if (!node) {
		cb(user_data, MESH_ERROR_NOT_FOUND, NULL);
		return;
	}

	/* Check if there is a pending request associated with this node */
	if (node->busy) {
		cb(user_data, MESH_ERROR_BUSY, NULL);
		return;
	}

	/* Check if the node is already in use */
	if (node->owner) {
		l_warn("The node is already in use");
		cb(user_data, MESH_ERROR_ALREADY_EXISTS, NULL);
		return;
	}

	req = l_new(struct managed_obj_request, 1);

	/*
	 * Create a temporary node to collect composition data from attaching
	 * application. Existing node is passed in req->attach.
	 */
	req->node = node_new(node->uuid);
	req->node->owner = l_strdup(sender);
	req->ready_cb = cb;
	req->pending_msg = user_data;
	req->attach = node;
	req->type = REQUEST_TYPE_ATTACH;

	node->busy = true;

	send_managed_objects_request(sender, app_root, req);
}

/* Create a temporary pre-provisioned node */
void node_join(const char *app_root, const char *sender, const uint8_t *uuid,
						node_join_ready_func_t cb)
{
	struct managed_obj_request *req;

	l_debug("");

	req = l_new(struct managed_obj_request, 1);
	req->node = node_new(uuid);
	req->join_ready_cb = cb;
	req->type = REQUEST_TYPE_JOIN;

	send_managed_objects_request(sender, app_root, req);
}

void node_import(const char *app_root, const char *sender, const uint8_t *uuid,
			const uint8_t dev_key[16], const uint8_t net_key[16],
			uint16_t net_idx, bool kr, bool ivu,
			uint32_t iv_index, uint16_t unicast,
			node_ready_func_t cb, void *user_data)
{
	struct managed_obj_request *req;

	l_debug("");

	req = l_new(struct managed_obj_request, 1);

	req->node = node_new(uuid);
	req->ready_cb = cb;
	req->pending_msg = user_data;

	req->import = l_new(struct node_import, 1);
	memcpy(req->import->dev_key, dev_key, 16);
	memcpy(req->import->net_key, net_key, 16);
	req->import->net_idx = net_idx;
	req->import->flags.kr = kr;
	req->import->flags.ivu = ivu;
	req->import->iv_index = iv_index;
	req->import->unicast = unicast;

	req->type = REQUEST_TYPE_IMPORT;

	send_managed_objects_request(sender, app_root, req);
}

void node_create(const char *app_root, const char *sender, const uint8_t *uuid,
					node_ready_func_t cb, void *user_data)
{
	struct managed_obj_request *req;

	l_debug("");

	req = l_new(struct managed_obj_request, 1);
	req->node = node_new(uuid);
	req->ready_cb = cb;
	req->pending_msg = user_data;
	req->type = REQUEST_TYPE_CREATE;

	send_managed_objects_request(sender, app_root, req);
}

static void build_element_config(void *a, void *b)
{
	struct node_element *ele = a;
	struct l_dbus_message_builder *builder = b;

	l_debug("Element %u", ele->idx);

	l_dbus_message_builder_enter_struct(builder, "ya(qa{sv})");

	/* Element index */
	l_dbus_message_builder_append_basic(builder, 'y', &ele->idx);

	l_dbus_message_builder_enter_array(builder, "(qa{sv})");

	/* Iterate over models */
	l_queue_foreach(ele->models, mesh_model_build_config, builder);

	l_dbus_message_builder_leave_array(builder);

	l_dbus_message_builder_leave_struct(builder);
}

void node_build_attach_reply(struct mesh_node *node,
						struct l_dbus_message *reply)
{
	struct l_dbus_message_builder *builder;

	builder = l_dbus_message_builder_new(reply);

	/* Node object path */
	l_dbus_message_builder_append_basic(builder, 'o', node->obj_path);

	/* Array of element configurations "a*/
	l_dbus_message_builder_enter_array(builder, "(ya(qa{sv}))");
	l_queue_foreach(node->elements, build_element_config, builder);
	l_dbus_message_builder_leave_array(builder);
	l_dbus_message_builder_finalize(builder);
	l_dbus_message_builder_destroy(builder);
}

static bool parse_send_options(struct l_dbus_message_iter *itr,
						struct send_options *opts)
{
	const char *key;
	struct l_dbus_message_iter var;

	opts->segmented = false;
	opts->vendor_id = SIG_VENDOR;

	while (l_dbus_message_iter_next_entry(itr, &key, &var)) {
		if (!strcmp(key, "ForceSegmented")) {
			if (!l_dbus_message_iter_get_variant(&var, "b",
							&opts->segmented))
				return false;
		}

		if (!strcmp(key, "Vendor")) {
			if (!l_dbus_message_iter_get_variant(&var, "q",
							&opts->vendor_id))
				return false;
		}
	}

	return true;
}

static struct l_dbus_message *send_call(struct l_dbus *dbus,
						struct l_dbus_message *msg,
						void *user_data)
{
	struct mesh_node *node = user_data;
	const char *sender, *ele_path;
	struct l_dbus_message_iter dict, iter_data;
	struct send_options opts;
	struct node_element *ele;
	uint16_t dst, app_idx, net_idx, src;
	uint8_t *data;
	uint32_t len;

	l_debug("Send");

	sender = l_dbus_message_get_sender(msg);

	if (strcmp(sender, node->owner))
		return dbus_error(msg, MESH_ERROR_NOT_AUTHORIZED, NULL);

	if (!l_dbus_message_get_arguments(msg, "oqqa{sv}ay", &ele_path, &dst,
						&app_idx, &dict, &iter_data))
		return dbus_error(msg, MESH_ERROR_INVALID_ARGS, NULL);

	ele = l_queue_find(node->elements, match_element_path, ele_path);
	if (!ele)
		return dbus_error(msg, MESH_ERROR_NOT_FOUND,
							"Element not found");

	if (!parse_send_options(&dict, &opts))
		return dbus_error(msg, MESH_ERROR_INVALID_ARGS, NULL);

	src = node_get_primary(node) + ele->idx;

	if (!l_dbus_message_iter_get_fixed_array(&iter_data, &data, &len) ||
					!len || len > MAX_MSG_LEN)
		return dbus_error(msg, MESH_ERROR_INVALID_ARGS,
							"Incorrect data");

	if (app_idx & ~APP_IDX_MASK)
		return dbus_error(msg, MESH_ERROR_INVALID_ARGS,
						"Invalid key index");

	net_idx = appkey_net_idx(node_get_net(node), app_idx);
	if (net_idx == NET_IDX_INVALID)
		return dbus_error(msg, MESH_ERROR_INVALID_ARGS,
							"Key not found");

	if (!mesh_model_send(node, src, dst, app_idx, net_idx, DEFAULT_TTL,
						opts.segmented, len, data))
		return dbus_error(msg, MESH_ERROR_FAILED, NULL);

	return l_dbus_message_new_method_return(msg);
}

static struct l_dbus_message *dev_key_send_call(struct l_dbus *dbus,
						struct l_dbus_message *msg,
						void *user_data)
{
	struct mesh_node *node = user_data;
	const char *sender, *ele_path;
	struct l_dbus_message_iter iter_data, dict;
	struct send_options opts;
	struct node_element *ele;
	uint16_t dst, app_idx, net_idx, src;
	bool remote;
	uint8_t *data;
	uint32_t len;

	l_debug("DevKeySend");

	sender = l_dbus_message_get_sender(msg);

	if (strcmp(sender, node->owner))
		return dbus_error(msg, MESH_ERROR_NOT_AUTHORIZED, NULL);

	if (!l_dbus_message_get_arguments(msg, "oqbqa{sv}ay", &ele_path, &dst,
					&remote, &net_idx, &dict, &iter_data))
		return dbus_error(msg, MESH_ERROR_INVALID_ARGS, NULL);

	/* Loopbacks to local servers must use *remote* addressing */
	if (!remote && mesh_net_is_local_address(node->net, dst, 1))
		return dbus_error(msg, MESH_ERROR_INVALID_ARGS, NULL);

	ele = l_queue_find(node->elements, match_element_path, ele_path);
	if (!ele)
		return dbus_error(msg, MESH_ERROR_NOT_FOUND,
							"Element not found");

	if (!parse_send_options(&dict, &opts))
		return dbus_error(msg, MESH_ERROR_INVALID_ARGS, NULL);

	src = node_get_primary(node) + ele->idx;

	if (!l_dbus_message_iter_get_fixed_array(&iter_data, &data, &len) ||
						!len || len > MAX_MSG_LEN)
		return dbus_error(msg, MESH_ERROR_INVALID_ARGS,
							"Incorrect data");

	app_idx = remote ? APP_IDX_DEV_REMOTE : APP_IDX_DEV_LOCAL;
	if (!mesh_model_send(node, src, dst, app_idx, net_idx, DEFAULT_TTL,
						opts.segmented, len, data))
		return dbus_error(msg, MESH_ERROR_NOT_FOUND, NULL);

	return l_dbus_message_new_method_return(msg);
}

static struct l_dbus_message *add_netkey_call(struct l_dbus *dbus,
						struct l_dbus_message *msg,
						void *user_data)
{
	struct mesh_node *node = user_data;
	const char *sender, *ele_path;
	struct node_element *ele;
	uint16_t dst, sub_idx, net_idx, src;
	bool update;
	struct keyring_net_key key;
	uint8_t data[20];

	l_debug("AddNetKey");

	sender = l_dbus_message_get_sender(msg);

	if (strcmp(sender, node->owner))
		return dbus_error(msg, MESH_ERROR_NOT_AUTHORIZED, NULL);

	if (!l_dbus_message_get_arguments(msg, "oqqqb", &ele_path, &dst,
						&sub_idx, &net_idx, &update))
		return dbus_error(msg, MESH_ERROR_INVALID_ARGS, NULL);

	ele = l_queue_find(node->elements, match_element_path, ele_path);
	if (!ele)
		return dbus_error(msg, MESH_ERROR_NOT_FOUND,
							"Element not found");

	src = node_get_primary(node) + ele->idx;

	if (!keyring_get_net_key(node, sub_idx, &key))
		return dbus_error(msg, MESH_ERROR_NOT_FOUND,
							"NetKey not found");

	if (!update) {
		l_put_be16(OP_NETKEY_ADD, data);

		if (key.phase != KEY_REFRESH_PHASE_TWO)
			memcpy(data + 4, key.old_key, 16);
		else
			memcpy(data + 4, key.new_key, 16);
	} else {
		if (key.phase != KEY_REFRESH_PHASE_ONE)
			return dbus_error(msg, MESH_ERROR_FAILED,
							"Cannot update");
		l_put_be16(OP_NETKEY_UPDATE, data);
		memcpy(data + 4, key.new_key, 16);
	}

	l_put_le16(sub_idx, &data[2]);

	if (!mesh_model_send(node, src, dst, APP_IDX_DEV_REMOTE, net_idx,
						DEFAULT_TTL, false, 20, data))
		return dbus_error(msg, MESH_ERROR_NOT_FOUND, NULL);

	return l_dbus_message_new_method_return(msg);
}

static struct l_dbus_message *add_appkey_call(struct l_dbus *dbus,
						struct l_dbus_message *msg,
						void *user_data)
{
	struct mesh_node *node = user_data;
	const char *sender, *ele_path;
	struct node_element *ele;
	uint16_t dst, app_idx, net_idx, src;
	bool update;
	struct keyring_net_key net_key;
	struct keyring_app_key app_key;
	uint8_t data[20];

	l_debug("AddAppKey");

	sender = l_dbus_message_get_sender(msg);

	if (strcmp(sender, node->owner))
		return dbus_error(msg, MESH_ERROR_NOT_AUTHORIZED, NULL);

	if (!l_dbus_message_get_arguments(msg, "oqqqb", &ele_path, &dst,
						&app_idx, &net_idx, &update))
		return dbus_error(msg, MESH_ERROR_INVALID_ARGS, NULL);

	ele = l_queue_find(node->elements, match_element_path, ele_path);
	if (!ele)
		return dbus_error(msg, MESH_ERROR_NOT_FOUND,
							"Element not found");

	src = node_get_primary(node) + ele->idx;

	if (!keyring_get_app_key(node, app_idx, &app_key))
		return dbus_error(msg, MESH_ERROR_NOT_FOUND,
							"AppKey not found");

	if (!keyring_get_net_key(node, app_key.net_idx, &net_key)) {
		return dbus_error(msg, MESH_ERROR_NOT_FOUND,
						"Bound NetKey not found");
	}

	if (!update) {
		data[0] = OP_APPKEY_ADD;
		if (net_key.phase != KEY_REFRESH_PHASE_TWO)
			memcpy(data + 4, app_key.old_key, 16);
		else
			memcpy(data + 4, app_key.new_key, 16);
	} else {
		if (net_key.phase != KEY_REFRESH_PHASE_ONE)
			return dbus_error(msg, MESH_ERROR_FAILED,
							"Cannot update");
		data[0] = OP_APPKEY_UPDATE;
		memcpy(data + 4, app_key.new_key, 16);
	}

	/* Pack bound NetKey and AppKey into 3 octets */
	data[1] = app_key.net_idx;
	data[2] = ((app_key.net_idx >> 8) & 0xf) | ((app_idx << 4) & 0xf0);
	data[3] = app_idx >> 4;

	if (!mesh_model_send(node, src, dst, APP_IDX_DEV_REMOTE, net_idx,
						DEFAULT_TTL, false, 20, data))
		return dbus_error(msg, MESH_ERROR_NOT_FOUND, NULL);

	return l_dbus_message_new_method_return(msg);
}

static struct l_dbus_message *publish_call(struct l_dbus *dbus,
						struct l_dbus_message *msg,
						void *user_data)
{
	struct mesh_node *node = user_data;
	const char *sender, *ele_path;
	struct l_dbus_message_iter iter_data, dict;
	uint16_t mod_id, src;
	struct send_options opts;
	struct node_element *ele;
	uint8_t *data;
	uint32_t len, id;
	int result;

	l_debug("Publish");

	sender = l_dbus_message_get_sender(msg);

	if (strcmp(sender, node->owner))
		return dbus_error(msg, MESH_ERROR_NOT_AUTHORIZED, NULL);

	if (!l_dbus_message_get_arguments(msg, "oqa{sv}ay", &ele_path, &mod_id,
							&dict, &iter_data))
		return dbus_error(msg, MESH_ERROR_INVALID_ARGS, NULL);

	ele = l_queue_find(node->elements, match_element_path, ele_path);
	if (!ele)
		return dbus_error(msg, MESH_ERROR_NOT_FOUND,
							"Element not found");

	if (!parse_send_options(&dict, &opts))
		return dbus_error(msg, MESH_ERROR_INVALID_ARGS, NULL);

	src = node_get_primary(node) + ele->idx;

	if (!l_dbus_message_iter_get_fixed_array(&iter_data, &data, &len) ||
					!len || len > MAX_MSG_LEN)
		return dbus_error(msg, MESH_ERROR_INVALID_ARGS,
							"Incorrect data");

	id = SET_ID(opts.vendor_id, mod_id);

	result = mesh_model_publish(node, id, src, opts.segmented, len, data);

	if (result != MESH_ERROR_NONE)
		return dbus_error(msg, result, NULL);

	return l_dbus_message_new_method_return(msg);
}

static bool features_getter(struct l_dbus *dbus, struct l_dbus_message *msg,
					struct l_dbus_message_builder *builder,
					void *user_data)
{
	struct mesh_node *node = user_data;
	uint8_t friend = node_friend_mode_get(node);
	uint8_t lpn = node_lpn_mode_get(node);
	uint8_t proxy = node_proxy_mode_get(node);
	uint8_t count;
	uint16_t interval;
	uint8_t relay = node_relay_mode_get(node, &count, &interval);

	l_dbus_message_builder_enter_array(builder, "{sv}");

	if (friend != MESH_MODE_UNSUPPORTED)
		dbus_append_dict_entry_basic(builder, "Friend", "b", &friend);

	if (lpn != MESH_MODE_UNSUPPORTED)
		dbus_append_dict_entry_basic(builder, "LowPower", "b", &lpn);

	if (proxy != MESH_MODE_UNSUPPORTED)
		dbus_append_dict_entry_basic(builder, "Proxy", "b", &proxy);

	if (relay != MESH_MODE_UNSUPPORTED)
		dbus_append_dict_entry_basic(builder, "Relay", "b", &relay);

	l_dbus_message_builder_leave_array(builder);

	return true;
}

static bool beacon_getter(struct l_dbus *dbus, struct l_dbus_message *msg,
					struct l_dbus_message_builder *builder,
					void *user_data)
{
	struct mesh_node *node = user_data;
	bool beacon_mode = node_beacon_mode_get(node) == MESH_MODE_ENABLED;

	l_dbus_message_builder_append_basic(builder, 'b', &beacon_mode);

	return true;
}

static bool ivupdate_getter(struct l_dbus *dbus, struct l_dbus_message *msg,
					struct l_dbus_message_builder *builder,
					void *user_data)
{
	struct mesh_node *node = user_data;
	struct mesh_net *net = node_get_net(node);
	uint8_t flags;
	uint32_t iv_index;
	bool ivu;

	mesh_net_get_snb_state(net, &flags, &iv_index);

	ivu = flags & IV_INDEX_UPDATE;

	l_dbus_message_builder_append_basic(builder, 'b', &ivu);

	return true;
}

static bool ivindex_getter(struct l_dbus *dbus, struct l_dbus_message *msg,
					struct l_dbus_message_builder *builder,
					void *user_data)
{
	struct mesh_node *node = user_data;
	struct mesh_net *net = node_get_net(node);
	uint8_t flags;
	uint32_t iv_index;

	mesh_net_get_snb_state(net, &flags, &iv_index);

	l_dbus_message_builder_append_basic(builder, 'u', &iv_index);

	return true;
}

static bool seq_num_getter(struct l_dbus *dbus, struct l_dbus_message *msg,
				struct l_dbus_message_builder *builder,
				void *user_data)
{
	struct mesh_node *node = user_data;
	struct mesh_net *net = node_get_net(node);
	uint32_t seq_nr = mesh_net_get_seq_num(net);

	l_dbus_message_builder_append_basic(builder, 'u', &seq_nr);

	return true;
}

static bool lastheard_getter(struct l_dbus *dbus, struct l_dbus_message *msg,
					struct l_dbus_message_builder *builder,
					void *user_data)
{
	struct mesh_node *node = user_data;
	struct mesh_net *net = node_get_net(node);
	struct timeval now;
	uint32_t last_heard;

	gettimeofday(&now, NULL);

	last_heard = now.tv_sec - mesh_net_get_instant(net);

	l_dbus_message_builder_append_basic(builder, 'u', &last_heard);

	return true;

}

static bool addresses_getter(struct l_dbus *dbus, struct l_dbus_message *msg,
					struct l_dbus_message_builder *builder,
					void *user_data)
{
	struct mesh_node *node = user_data;
	const struct l_queue_entry *entry;

	l_dbus_message_builder_enter_array(builder, "q");

	entry = l_queue_get_entries(node->elements);
	for (; entry; entry = entry->next) {
		const struct node_element *ele = entry->data;
		uint16_t address = node->primary + ele->idx;

		l_dbus_message_builder_append_basic(builder, 'q', &address);
	}

	l_dbus_message_builder_leave_array(builder);

	return true;
}

static void setup_node_interface(struct l_dbus_interface *iface)
{
	l_dbus_interface_method(iface, "Send", 0, send_call, "", "oqqa{sv}ay",
						"element_path", "destination",
						"key_index", "options", "data");
	l_dbus_interface_method(iface, "DevKeySend", 0, dev_key_send_call, "",
						"oqbqa{sv}ay", "element_path",
						"destination", "remote",
						"net_index", "options", "data");
	l_dbus_interface_method(iface, "AddNetKey", 0, add_netkey_call, "",
					"oqqqb", "element_path", "destination",
					"subnet_index", "net_index", "update");
	l_dbus_interface_method(iface, "AddAppKey", 0, add_appkey_call, "",
					"oqqqb", "element_path", "destination",
					"app_index", "net_index", "update");
	l_dbus_interface_method(iface, "Publish", 0, publish_call, "",
					"oqa{sv}ay", "element_path", "model_id",
							"options", "data");
	l_dbus_interface_property(iface, "Features", 0, "a{sv}",
							features_getter, NULL);
	l_dbus_interface_property(iface, "Beacon", 0, "b", beacon_getter, NULL);
	l_dbus_interface_property(iface, "IvUpdate", 0, "b", ivupdate_getter,
									NULL);
	l_dbus_interface_property(iface, "IvIndex", 0, "u", ivindex_getter,
									NULL);
	l_dbus_interface_property(iface, "SequenceNumber", 0, "u",
							seq_num_getter, NULL);
	l_dbus_interface_property(iface, "SecondsSinceLastHeard", 0, "u",
					lastheard_getter, NULL);
	l_dbus_interface_property(iface, "Addresses", 0, "aq", addresses_getter,
									NULL);
}

void node_property_changed(struct mesh_node *node, const char *property)
{
	struct l_dbus *bus = dbus_get_bus();

	if (bus && node->obj_path)
		l_dbus_property_changed(dbus_get_bus(), node->obj_path,
						MESH_NODE_INTERFACE, property);
}

bool node_dbus_init(struct l_dbus *bus)
{
	if (!l_dbus_register_interface(bus, MESH_NODE_INTERFACE,
						setup_node_interface,
						NULL, false)) {
		l_info("Unable to register %s interface", MESH_NODE_INTERFACE);
		return false;
	}

	return true;
}

const char *node_get_owner(struct mesh_node *node)
{
	return node->owner;
}

const char *node_get_element_path(struct mesh_node *node, uint8_t ele_idx)
{
	struct node_element *ele;

	ele = l_queue_find(node->elements, match_element_idx,
							L_UINT_TO_PTR(ele_idx));

	if (!ele)
		return NULL;

	return ele->path;
}

bool node_add_pending_local(struct mesh_node *node, void *prov_node_info)
{
	struct mesh_prov_node_info *info = prov_node_info;
	bool kr = !!(info->flags & PROV_FLAG_KR);
	bool ivu = !!(info->flags & PROV_FLAG_IVU);

	return add_local_node(node, info->unicast, kr, ivu, info->iv_index,
			info->device_key, info->net_index, info->net_key);
}

struct mesh_config *node_config_get(struct mesh_node *node)
{
	return node->cfg;
}

const char *node_get_storage_dir(struct mesh_node *node)
{
	return node->storage_dir;
}

const char *node_get_app_path(struct mesh_node *node)
{
	if (!node)
		return NULL;

	return node->app_path;
}

struct mesh_net *node_get_net(struct mesh_node *node)
{
	return node->net;
}

struct mesh_agent *node_get_agent(struct mesh_node *node)
{
	return node->agent;
}

bool node_load_from_storage(const char *storage_dir)
{
	return mesh_config_load_nodes(storage_dir, init_from_storage, NULL);
}

/*
 * This is called for a new node that:
 *         - has been created as a result of successful completion of Join()
 *           or Create() or Import() methods
 *     and
 *         - has been confirmed via successful token delivery to the application
 *
 * After a node has been created, the information gathered during initial
 * GetManagedObjects() call is cleared. The subsequent call to Attach() would
 * verify node's integrity and re-initialize node's D-Bus resources.
 */
void node_finalize_new_node(struct mesh_node *node, struct mesh_io *io)
{
	if (!node)
		return;

	free_node_dbus_resources(node);
	mesh_agent_remove(node->agent);
	node->agent = NULL;

	node->busy = false;

	/* Register callback for the node's io */
	attach_io(node, io);
}

// SPDX-License-Identifier: LGPL-2.1-or-later
/*
 *
 *  BlueZ - Bluetooth protocol stack for Linux
 *
 *  Copyright (C) 2018-2019  Intel Corporation. All rights reserved.
 *
 *
 */

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#define _GNU_SOURCE
#include <dirent.h>
#include <errno.h>
#include <fcntl.h>
#include <ftw.h>
#include <libgen.h>
#include <limits.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>

#include <sys/time.h>

#include <ell/ell.h>
#include <json-c/json.h>

#include "mesh/mesh-defs.h"
#include "mesh/util.h"
#include "mesh/mesh-config.h"

/* To prevent local node JSON cache thrashing, minimum update times */
#define MIN_SEQ_CACHE_TRIGGER	32
#define MIN_SEQ_CACHE_VALUE	(2 * 32)
#define MIN_SEQ_CACHE_TIME	(5 * 60)

#define CHECK_KEY_IDX_RANGE(x) ((x) <= 4095)

struct mesh_config {
	json_object *jnode;
	char *node_dir_path;
	uint8_t uuid[16];
	uint32_t write_seq;
	struct timeval write_time;
	struct l_queue *idles;
};

struct write_info {
	struct mesh_config *cfg;
	void *user_data;
	mesh_config_status_func_t cb;
};

static const char *cfgnode_name = "/node.json";
static const char *bak_ext = ".bak";
static const char *tmp_ext = ".tmp";

static bool save_config(json_object *jnode, const char *fname)
{
	FILE *outfile;
	const char *str;
	bool result = false;

	outfile = fopen(fname, "w");
	if (!outfile) {
		l_error("Failed to save configuration to %s", fname);
		return false;
	}

	str = json_object_to_json_string_ext(jnode, JSON_C_TO_STRING_PRETTY);

	if (fwrite(str, sizeof(char), strlen(str), outfile) < strlen(str))
		l_warn("Incomplete write of mesh configuration");
	else
		result = true;

	fclose(outfile);

	return result;
}

static bool get_int(json_object *jobj, const char *keyword, int *value)
{
	json_object *jvalue;

	if (!json_object_object_get_ex(jobj, keyword, &jvalue))
		return false;

	*value = json_object_get_int(jvalue);
	if (errno == EINVAL)
		return false;

	return true;
}

static bool add_u64_value(json_object *jobj, const char *desc,
					const uint8_t u64[8])
{
	json_object *jstring;
	char hexstr[17];

	hex2str((uint8_t *) u64, 8, hexstr, 17);
	jstring = json_object_new_string(hexstr);
	if (!jstring)
		return false;

	json_object_object_del(jobj, desc);
	json_object_object_add(jobj, desc, jstring);
	return true;
}

static bool add_key_value(json_object *jobj, const char *desc,
					const uint8_t key[16])
{
	json_object *jstring;
	char hexstr[33];

	hex2str((uint8_t *) key, 16, hexstr, 33);
	jstring = json_object_new_string(hexstr);
	if (!jstring)
		return false;

	json_object_object_del(jobj, desc);
	json_object_object_add(jobj, desc, jstring);
	return true;
}

static int get_element_index(json_object *jnode, uint16_t ele_addr)
{
	json_object *jvalue, *jelements;
	uint16_t addr, num_ele;
	char *str;

	if (!json_object_object_get_ex(jnode, "unicastAddress", &jvalue))
		return -1;

	str = (char *)json_object_get_string(jvalue);
	if (sscanf(str, "%04hx", &addr) != 1)
		return -1;

	if (!json_object_object_get_ex(jnode, "elements", &jelements))
		return -1;

	num_ele = json_object_array_length(jelements);

	if (ele_addr >= addr + num_ele || ele_addr < addr)
		return -1;

	return ele_addr - addr;
}

static json_object *get_element_model(json_object *jnode, int ele_idx,
						uint32_t mod_id, bool vendor)
{
	json_object *jelements, *jelement, *jmodels;
	int i, num_mods;
	size_t len;
	char buf[9];

	if (!json_object_object_get_ex(jnode, "elements", &jelements))
		return NULL;

	jelement = json_object_array_get_idx(jelements, ele_idx);
	if (!jelement)
		return NULL;

	if (!json_object_object_get_ex(jelement, "models", &jmodels))
		return NULL;

	num_mods = json_object_array_length(jmodels);
	if (!num_mods)
		return NULL;

	if (!vendor) {
		snprintf(buf, 5, "%4.4x", (uint16_t)mod_id);
		len = 4;
	} else {
		snprintf(buf, 9, "%8.8x", mod_id);
		len = 8;
	}

	for (i = 0; i < num_mods; ++i) {
		json_object *jmodel, *jvalue;
		char *str;

		jmodel = json_object_array_get_idx(jmodels, i);
		if (!json_object_object_get_ex(jmodel, "modelId", &jvalue))
			return NULL;

		str = (char *)json_object_get_string(jvalue);
		if (!str)
			return NULL;

		if (!strncmp(str, buf, len))
			return jmodel;
	}

	return NULL;
}

static bool jarray_has_string(json_object *jarray, char *str, size_t len)
{
	int i, sz = json_object_array_length(jarray);

	for (i = 0; i < sz; ++i) {
		json_object *jentry;
		char *str_entry;

		jentry = json_object_array_get_idx(jarray, i);
		str_entry = (char *)json_object_get_string(jentry);
		if (!str_entry)
			continue;

		if (!strncmp(str, str_entry, len))
			return true;
	}

	return false;
}

static void jarray_string_del(json_object *jarray, char *str, size_t len)
{
	int i, sz = json_object_array_length(jarray);

	for (i = 0; i < sz; ++i) {
		json_object *jentry;
		char *str_entry;

		jentry = json_object_array_get_idx(jarray, i);
		str_entry = (char *)json_object_get_string(jentry);

		if (str_entry && !strncmp(str, str_entry, len)) {
			json_object_array_del_idx(jarray, i, 1);
			return;
		}

	}
}

static bool get_key_index(json_object *jobj, const char *keyword,
								uint16_t *index)
{
	int idx;

	if (!get_int(jobj, keyword, &idx))
		return false;

	if (!CHECK_KEY_IDX_RANGE(idx))
		return false;

	*index = (uint16_t) idx;
	return true;
}

static json_object *get_key_object(json_object *jarray, uint16_t idx)
{
	int i, sz = json_object_array_length(jarray);

	for (i = 0; i < sz; ++i) {
		json_object *jentry;
		uint16_t jidx;

		jentry = json_object_array_get_idx(jarray, i);
		if (!get_key_index(jentry, "index", &jidx))
			return NULL;

		if (jidx == idx)
			return jentry;
	}

	return NULL;
}

static void jarray_key_del(json_object *jarray, int16_t idx)
{
	int i, sz = json_object_array_length(jarray);

	for (i = 0; i < sz; ++i) {
		json_object *jentry;
		uint16_t nidx;

		jentry = json_object_array_get_idx(jarray, i);

		if (get_key_index(jentry, "index", &nidx) && nidx == idx) {
			json_object_array_del_idx(jarray, i, 1);
			return;
		}
	}
}

static bool read_unicast_address(json_object *jobj, uint16_t *unicast)
{
	json_object *jvalue;
	char *str;

	if (!json_object_object_get_ex(jobj, "unicastAddress", &jvalue))
		return false;

	str = (char *)json_object_get_string(jvalue);
	if (sscanf(str, "%04hx", unicast) != 1)
		return false;

	return true;
}

static bool read_default_ttl(json_object *jobj, uint8_t *ttl)
{
	json_object *jvalue;
	int val;

	/* defaultTTL is optional */
	if (!json_object_object_get_ex(jobj, "defaultTTL", &jvalue))
		return true;

	val = json_object_get_int(jvalue);

	if (!val && errno == EINVAL)
		return false;

	if (val < 0 || val == 1 || val > TTL_MASK)
		return false;

	*ttl = (uint8_t) val;

	return true;
}

static bool read_seq_number(json_object *jobj, uint32_t *seq_number)
{
	json_object *jvalue;
	int val;

	/* sequenceNumber is optional */
	if (!json_object_object_get_ex(jobj, "sequenceNumber", &jvalue))
		return true;

	val = json_object_get_int(jvalue);

	if (!val && errno == EINVAL)
		return false;

	if (val < 0 || val > SEQ_MASK + 1)
		return false;

	*seq_number = (uint32_t) val;

	return true;
}

static bool read_iv_index(json_object *jobj, uint32_t *idx, bool *update)
{
	int tmp;

	/* IV index */
	if (!get_int(jobj, "IVindex", &tmp))
		return false;

	*idx = (uint32_t) tmp;

	if (!get_int(jobj, "IVupdate", &tmp))
		return false;

	*update = tmp ? true : false;

	return true;
}

static bool read_token(json_object *jobj, uint8_t token[8])
{
	json_object *jvalue;
	char *str;

	if (!token)
		return false;

	if (!json_object_object_get_ex(jobj, "token", &jvalue))
		return false;

	str = (char *)json_object_get_string(jvalue);
	if (!str2hex(str, strlen(str), token, 8))
		return false;

	return true;
}

static bool read_device_key(json_object *jobj, uint8_t key_buf[16])
{
	json_object *jvalue;
	char *str;

	if (!key_buf)
		return false;

	if (!json_object_object_get_ex(jobj, "deviceKey", &jvalue))
		return false;

	str = (char *)json_object_get_string(jvalue);
	if (!str2hex(str, strlen(str), key_buf, 16))
		return false;

	return true;
}

static bool read_comp_pages(json_object *jobj, struct mesh_config_node *node)
{
	json_object *jarray, *jentry;
	struct mesh_config_comp_page *page;
	int len;
	int i;

	if (!json_object_object_get_ex(jobj, "pages", &jarray))
		return true;

	if (json_object_get_type(jarray) != json_type_array)
		return false;

	len = json_object_array_length(jarray);

	for (i = 0; i < len; i++) {
		size_t clen;
		char *str;

		jentry = json_object_array_get_idx(jarray, i);
		str = (char *)json_object_get_string(jentry);
		clen = strlen(str);

		if (clen < ((MIN_COMP_SIZE * 2) + 1))
			continue;

		clen = (clen / 2) - 1;

		page = l_malloc(sizeof(struct mesh_config_comp_page) + clen);

		if (!str2hex(str + 2, clen * 2, page->data, clen))
			goto parse_fail;

		if (sscanf(str, "%02hhx", &page->page_num) != 1)
			goto parse_fail;

		page->len = clen;

		l_queue_push_tail(node->pages, page);
	}

	return true;

parse_fail:
	l_free(page);
	return false;
}

static bool read_app_keys(json_object *jobj, struct mesh_config_node *node)
{
	json_object *jarray;
	int len;
	int i;

	if (!json_object_object_get_ex(jobj, "appKeys", &jarray))
		return true;

	if (json_object_get_type(jarray) != json_type_array)
		return false;

	/* Allow empty AppKey array */
	len = json_object_array_length(jarray);
	if (!len)
		return true;

	for (i = 0; i < len; ++i) {
		json_object *jtemp, *jvalue;
		char *str;
		struct mesh_config_appkey *appkey;

		appkey = l_new(struct mesh_config_appkey, 1);
		l_queue_push_tail(node->appkeys, appkey);

		jtemp = json_object_array_get_idx(jarray, i);

		if (!get_key_index(jtemp, "index", &appkey->app_idx))
			goto fail;

		if (!get_key_index(jtemp, "boundNetKey", &appkey->net_idx))
			goto fail;

		if (!json_object_object_get_ex(jtemp, "key", &jvalue))
			goto fail;

		str = (char *)json_object_get_string(jvalue);
		if (!str2hex(str, strlen(str), appkey->new_key, 16))
			goto fail;

		if (json_object_object_get_ex(jtemp, "oldKey", &jvalue))
			str = (char *)json_object_get_string(jvalue);

		if (!str2hex(str, strlen(str), appkey->key, 16))
			goto fail;
	}

	return true;
fail:
	l_queue_destroy(node->appkeys, l_free);
	node->appkeys = NULL;

	return false;
}

static bool read_net_keys(json_object *jobj, struct mesh_config_node *node)
{
	json_object *jarray;
	int len;
	int i;

	/* At least one NetKey must be present for a provisioned node */
	if (!json_object_object_get_ex(jobj, "netKeys", &jarray))
		return false;

	if (json_object_get_type(jarray) != json_type_array)
		return false;

	len = json_object_array_length(jarray);
	if (!len)
		return false;

	for (i = 0; i < len; ++i) {
		json_object *jtemp, *jvalue;
		char *str;
		struct mesh_config_netkey *netkey;

		netkey = l_new(struct mesh_config_netkey, 1);

		l_queue_push_tail(node->netkeys, netkey);

		jtemp = json_object_array_get_idx(jarray, i);

		if (!get_key_index(jtemp, "index", &netkey->idx))
			goto fail;

		if (!json_object_object_get_ex(jtemp, "key", &jvalue))
			goto fail;

		str = (char *)json_object_get_string(jvalue);
		if (!str2hex(str, strlen(str), netkey->new_key, 16))
			goto fail;

		if (!json_object_object_get_ex(jtemp, "keyRefresh", &jvalue))
			netkey->phase = KEY_REFRESH_PHASE_NONE;
		else
			netkey->phase = (uint8_t) json_object_get_int(jvalue);

		if (netkey->phase > KEY_REFRESH_PHASE_TWO)
			goto fail;

		if (json_object_object_get_ex(jtemp, "oldKey", &jvalue)) {
			if (netkey->phase == KEY_REFRESH_PHASE_NONE)
				goto fail;

			str = (char *)json_object_get_string(jvalue);
		}

		if (!str2hex(str, strlen(str), netkey->key, 16))
			goto fail;
	}

	return true;
fail:
	l_queue_destroy(node->netkeys, l_free);
	node->netkeys = NULL;

	return false;
}

static bool write_int(json_object *jobj, const char *desc, int val)
{
	json_object *jvalue;

	jvalue = json_object_new_int(val);
	if (!jvalue)
		return false;

	json_object_object_del(jobj, desc);
	json_object_object_add(jobj, desc, jvalue);
	return true;
}

bool mesh_config_net_key_add(struct mesh_config *cfg, uint16_t idx,
							const uint8_t key[16])
{
	json_object *jnode, *jarray = NULL, *jentry = NULL;

	if (!cfg)
		return false;

	jnode = cfg->jnode;

	l_debug("netKey %4.4x", idx);
	json_object_object_get_ex(jnode, "netKeys", &jarray);
	if (jarray)
		jentry = get_key_object(jarray, idx);

	/* Do not allow direct overwrite */
	if (jentry)
		return false;

	jentry = json_object_new_object();
	if (!jentry)
		return false;

	if (!write_int(jentry, "index", idx))
		goto fail;

	if (!add_key_value(jentry, "key", key))
		goto fail;

	json_object_object_add(jentry, "keyRefresh",
				json_object_new_int(KEY_REFRESH_PHASE_NONE));

	if (!jarray) {
		jarray = json_object_new_array();
		if (!jarray)
			goto fail;
		json_object_object_add(jnode, "netKeys", jarray);
	}

	json_object_array_add(jarray, jentry);

	return save_config(jnode, cfg->node_dir_path);

fail:
	if (jentry)
		json_object_put(jentry);

	return false;
}

bool mesh_config_net_key_update(struct mesh_config *cfg, uint16_t idx,
							const uint8_t key[16])
{
	json_object *jnode, *jarray, *jentry, *jstring;
	const char *str;

	if (!cfg)
		return false;

	jnode = cfg->jnode;

	if (!json_object_object_get_ex(jnode, "netKeys", &jarray))
		return false;

	jentry = get_key_object(jarray, idx);
	/* Net key must be already recorded */
	if (!jentry)
		return false;

	if (!json_object_object_get_ex(jentry, "key", &jstring))
		return false;

	str = json_object_get_string(jstring);
	jstring = json_object_new_string(str);
	json_object_object_add(jentry, "oldKey", jstring);
	json_object_object_del(jentry, "key");

	if (!add_key_value(jentry, "key", key))
		return false;

	json_object_object_add(jentry, "keyRefresh",
				json_object_new_int(KEY_REFRESH_PHASE_ONE));

	return save_config(jnode, cfg->node_dir_path);
}

bool mesh_config_net_key_del(struct mesh_config *cfg, uint16_t idx)
{
	json_object *jnode, *jarray;

	if (!cfg)
		return false;

	jnode = cfg->jnode;

	if (!json_object_object_get_ex(jnode, "netKeys", &jarray))
		return true;

	jarray_key_del(jarray, idx);

	if (!json_object_array_length(jarray))
		json_object_object_del(jnode, "netKeys");

	return save_config(jnode, cfg->node_dir_path);
}

bool mesh_config_write_device_key(struct mesh_config *cfg, uint8_t *key)
{
	if (!cfg || !add_key_value(cfg->jnode, "deviceKey", key))
		return false;

	return save_config(cfg->jnode, cfg->node_dir_path);
}

bool mesh_config_write_token(struct mesh_config *cfg, uint8_t *token)
{
	if (!cfg || !add_u64_value(cfg->jnode, "token", token))
		return false;

	return save_config(cfg->jnode, cfg->node_dir_path);
}

bool mesh_config_app_key_add(struct mesh_config *cfg, uint16_t net_idx,
					uint16_t app_idx, const uint8_t key[16])
{
	json_object *jnode, *jarray = NULL, *jentry = NULL;

	if (!cfg)
		return false;

	jnode = cfg->jnode;

	json_object_object_get_ex(jnode, "appKeys", &jarray);
	if (jarray)
		jentry = get_key_object(jarray, app_idx);

	/* Do not allow direct overrwrite */
	if (jentry)
		return false;

	jentry = json_object_new_object();
	if (!jentry)
		return false;

	if (!write_int(jentry, "index", app_idx))
		goto fail;

	if (!write_int(jentry, "boundNetKey", net_idx))
		goto fail;

	if (!add_key_value(jentry, "key", key))
		goto fail;

	if (!jarray) {
		jarray = json_object_new_array();
		if (!jarray)
			goto fail;
		json_object_object_add(jnode, "appKeys", jarray);
	}

	json_object_array_add(jarray, jentry);

	return save_config(jnode, cfg->node_dir_path);

fail:

	if (jentry)
		json_object_put(jentry);

	return false;
}

bool mesh_config_app_key_update(struct mesh_config *cfg, uint16_t app_idx,
							const uint8_t key[16])
{
	json_object *jnode, *jarray, *jentry = NULL, *jstring = NULL;
	const char *str;

	if (!cfg)
		return false;

	jnode = cfg->jnode;

	if (!json_object_object_get_ex(jnode, "appKeys", &jarray))
		return false;

	/* The key entry should exist if the key is updated */
	jentry = get_key_object(jarray, app_idx);
	if (!jentry)
		return false;

	if (!json_object_object_get_ex(jentry, "key", &jstring))
		return false;

	str = json_object_get_string(jstring);
	jstring = json_object_new_string(str);
	json_object_object_add(jentry, "oldKey", jstring);

	json_object_object_del(jentry, "key");

	/* TODO: "Rewind" if add_key_value fails */
	if (!add_key_value(jentry, "key", key))
		return false;

	return save_config(jnode, cfg->node_dir_path);
}

bool mesh_config_app_key_del(struct mesh_config *cfg, uint16_t net_idx,
								uint16_t idx)
{
	json_object *jnode, *jarray;

	if (!cfg)
		return false;

	jnode = cfg->jnode;

	if (!json_object_object_get_ex(jnode, "appKeys", &jarray))
		return true;

	jarray_key_del(jarray, idx);

	if (!json_object_array_length(jarray))
		json_object_object_del(jnode, "appKeys");

	return save_config(jnode, cfg->node_dir_path);
}

bool mesh_config_model_binding_add(struct mesh_config *cfg, uint16_t ele_addr,
						uint32_t mod_id, bool vendor,
							uint16_t app_idx)
{
	json_object *jnode, *jmodel, *jstring, *jarray = NULL;
	int ele_idx;
	char buf[5];

	if (!cfg)
		return false;

	jnode = cfg->jnode;

	ele_idx = get_element_index(jnode, ele_addr);
	if (ele_idx < 0)
		return false;

	jmodel = get_element_model(jnode, ele_idx, mod_id, vendor);
	if (!jmodel)
		return false;

	snprintf(buf, 5, "%4.4x", app_idx);

	json_object_object_get_ex(jmodel, "bind", &jarray);
	if (jarray && jarray_has_string(jarray, buf, 4))
		return true;

	jstring = json_object_new_string(buf);
	if (!jstring)
		return false;

	if (!jarray) {
		jarray = json_object_new_array();
		if (!jarray) {
			json_object_put(jstring);
			return false;
		}
		json_object_object_add(jmodel, "bind", jarray);
	}

	json_object_array_add(jarray, jstring);

	return save_config(jnode, cfg->node_dir_path);
}

bool mesh_config_model_binding_del(struct mesh_config *cfg, uint16_t ele_addr,
						uint32_t mod_id, bool vendor,
							uint16_t app_idx)
{
	json_object *jnode, *jmodel, *jarray;
	int ele_idx;
	char buf[5];

	if (!cfg)
		return false;

	jnode = cfg->jnode;

	ele_idx = get_element_index(jnode, ele_addr);
	if (ele_idx < 0)
		return false;

	jmodel = get_element_model(jnode, ele_idx, mod_id, vendor);
	if (!jmodel)
		return false;

	if (!json_object_object_get_ex(jmodel, "bind", &jarray))
		return true;

	snprintf(buf, 5, "%4.4x", app_idx);

	jarray_string_del(jarray, buf, 4);

	if (!json_object_array_length(jarray))
		json_object_object_del(jmodel, "bind");

	return save_config(jnode, cfg->node_dir_path);
}

static void free_model(void *data)
{
	struct mesh_config_model *mod = data;

	l_free(mod->bindings);
	l_free(mod->subs);
	l_free(mod->pub);
	l_free(mod);
}

static void free_element(void *data)
{
	struct mesh_config_element *ele = data;

	l_queue_destroy(ele->models, free_model);
	l_free(ele);
}

static bool parse_bindings(json_object *jarray, struct mesh_config_model *mod)
{
	int cnt;
	int i;

	cnt = json_object_array_length(jarray);
	if (cnt > 0xffff)
		return false;

	mod->num_bindings = cnt;

	/* Allow empty bindings list */
	if (!cnt)
		return true;

	mod->bindings = l_new(uint16_t, cnt);

	for (i = 0; i < cnt; ++i) {
		uint16_t idx;
		const char *str;
		json_object *jvalue;

		jvalue = json_object_array_get_idx(jarray, i);
		if (!jvalue)
			return false;

		str = json_object_get_string(jvalue);

		if (sscanf(str, "%04hx", &idx) != 1)
			return false;

		if (!CHECK_KEY_IDX_RANGE(idx))
			return false;

		mod->bindings[i] = (uint16_t) idx;
	}

	return true;
}

static struct mesh_config_pub *parse_model_publication(json_object *jpub)
{
	json_object *jvalue;
	struct mesh_config_pub *pub;
	int len, value;
	char *str;

	if (!json_object_object_get_ex(jpub, "address", &jvalue))
		return NULL;

	str = (char *)json_object_get_string(jvalue);
	len = strlen(str);
	pub = l_new(struct mesh_config_pub, 1);

	switch (len) {
	case 4:
		if (sscanf(str, "%04hx", &pub->addr) != 1)
			goto fail;
		break;
	case 32:
		if (!str2hex(str, len, pub->virt_addr, 16))
			goto fail;

		pub->virt = true;
		break;
	default:
		goto fail;
	}

	if (!get_key_index(jpub, "index", &pub->idx))
		goto fail;

	if (!get_int(jpub, "ttl", &value))
		goto fail;
	pub->ttl = (uint8_t) value;

	if (!get_int(jpub, "period", &value))
		goto fail;
	pub->period = value;

	if (!get_int(jpub, "credentials", &value))
		goto fail;
	pub->credential = (uint8_t) value;

	if (!json_object_object_get_ex(jpub, "retransmit", &jvalue))
		goto fail;

	if (!get_int(jvalue, "count", &value))
		goto fail;
	pub->cnt = (uint8_t) value;

	if (!get_int(jvalue, "interval", &value))
		goto fail;
	pub->interval = (uint8_t) value;

	return pub;

fail:
	l_free(pub);
	return NULL;
}

static bool parse_model_subscriptions(json_object *jsubs,
						struct mesh_config_model *mod)
{
	struct mesh_config_sub *subs;
	int i, cnt;

	if (json_object_get_type(jsubs) != json_type_array)
		return NULL;

	cnt = json_object_array_length(jsubs);
	/* Allow empty array */
	if (!cnt)
		return true;

	subs = l_new(struct mesh_config_sub, cnt);

	for (i = 0; i < cnt; ++i) {
		char *str;
		int len;
		json_object *jvalue;

		jvalue = json_object_array_get_idx(jsubs, i);
		if (!jvalue)
			goto fail;

		str = (char *)json_object_get_string(jvalue);
		len = strlen(str);

		switch (len) {
		case 4:
			if (sscanf(str, "%04hx", &subs[i].addr.grp) != 1)
				goto fail;
			break;
		case 32:
			if (!str2hex(str, len, subs[i].addr.label, 16))
				goto fail;
			subs[i].virt = true;
			break;
		default:
			goto fail;
		}
	}

	mod->num_subs = cnt;
	mod->subs = subs;

	return true;
fail:
	l_free(subs);
	return false;
}

static bool parse_models(json_object *jmodels, struct mesh_config_element *ele)
{
	int i, num_models;

	num_models = json_object_array_length(jmodels);
	if (!num_models)
		return true;

	for (i = 0; i < num_models; ++i) {
		json_object *jmodel, *jarray, *jvalue;
		struct mesh_config_model *mod;
		uint32_t id;
		int len;
		char *str;

		jmodel = json_object_array_get_idx(jmodels, i);
		if (!jmodel)
			goto fail;

		mod = l_new(struct mesh_config_model, 1);

		l_queue_push_tail(ele->models, mod);

		if (!json_object_object_get_ex(jmodel, "modelId", &jvalue))
			goto fail;

		str = (char *)json_object_get_string(jvalue);

		len = strlen(str);

		if (len == 4) {
			if (sscanf(str, "%04x", &id) != 1)
				goto fail;
		} else if (len == 8) {
			if (sscanf(str, "%08x", &id) != 1)
				goto fail;
			mod->vendor = true;
		} else
			goto fail;

		mod->id = id;

		if (json_object_object_get_ex(jmodel, "bind", &jarray)) {
			if (json_object_get_type(jarray) != json_type_array ||
					!parse_bindings(jarray, mod))
				goto fail;
		}

		if (json_object_object_get_ex(jmodel, "pubEnabled", &jvalue))
			mod->pub_enabled = json_object_get_boolean(jvalue);
		else
			mod->pub_enabled = true;

		if (json_object_object_get_ex(jmodel, "subEnabled", &jvalue))
			mod->sub_enabled = json_object_get_boolean(jvalue);
		else
			mod->sub_enabled = true;

		if (json_object_object_get_ex(jmodel, "publish", &jvalue)) {
			mod->pub = parse_model_publication(jvalue);
			if (!mod->pub)
				goto fail;
		}

		if (json_object_object_get_ex(jmodel, "subscribe", &jarray)) {
			if (!parse_model_subscriptions(jarray, mod))
				goto fail;
		}
	}

	return true;

fail:
	l_queue_destroy(ele->models, free_model);
	return false;
}

static bool parse_elements(json_object *jelems, struct mesh_config_node *node)
{
	int i, num_ele;

	if (json_object_get_type(jelems) != json_type_array)
		return false;

	num_ele = json_object_array_length(jelems);
	if (!num_ele)
		/* Allow "empty" nodes */
		return true;

	for (i = 0; i < num_ele; ++i) {
		json_object *jelement;
		json_object *jmodels;
		json_object *jvalue;
		struct mesh_config_element *ele;
		int index;
		char *str;

		jelement = json_object_array_get_idx(jelems, i);
		if (!jelement)
			goto fail;

		if (!get_int(jelement, "elementIndex", &index) ||
								index > num_ele)
			goto fail;

		ele = l_new(struct mesh_config_element, 1);
		ele->index = index;
		ele->models = l_queue_new();
		l_queue_push_tail(node->elements, ele);

		if (!json_object_object_get_ex(jelement, "location", &jvalue))
			goto fail;

		str = (char *)json_object_get_string(jvalue);
		if (sscanf(str, "%04hx", &(ele->location)) != 1)
			goto fail;

		if (json_object_object_get_ex(jelement, "models", &jmodels)) {
			if (json_object_get_type(jmodels) != json_type_array ||
						!parse_models(jmodels, ele))
				goto fail;
		}
	}

	return true;

fail:
	l_queue_destroy(node->elements, free_element);
	node->elements = NULL;

	return false;
}

static int get_mode(json_object *jvalue)
{
	const char *str;

	str = json_object_get_string(jvalue);
	if (!str)
		return 0xffffffff;

	if (!strncasecmp(str, "disabled", strlen("disabled")))
		return MESH_MODE_DISABLED;

	if (!strncasecmp(str, "enabled", strlen("enabled")))
		return MESH_MODE_ENABLED;

	if (!strncasecmp(str, "unsupported", strlen("unsupported")))
		return MESH_MODE_UNSUPPORTED;

	return 0xffffffff;
}

static void parse_features(json_object *jconfig, struct mesh_config_node *node)
{
	json_object *jvalue, *jrelay;
	int mode, count;
	uint16_t interval;

	if (json_object_object_get_ex(jconfig, "proxy", &jvalue)) {
		mode = get_mode(jvalue);
		if (mode <= MESH_MODE_UNSUPPORTED)
			node->modes.proxy = mode;
	}

	if (json_object_object_get_ex(jconfig, "friend", &jvalue)) {
		mode = get_mode(jvalue);
		if (mode <= MESH_MODE_UNSUPPORTED)
			node->modes.friend = mode;
	}

	if (json_object_object_get_ex(jconfig, "lowPower", &jvalue)) {
		mode = get_mode(jvalue);
		if (mode <= MESH_MODE_UNSUPPORTED)
			node->modes.lpn = mode;
	}

	if (json_object_object_get_ex(jconfig, "beacon", &jvalue)) {
		mode = get_mode(jvalue);
		if (mode <= MESH_MODE_UNSUPPORTED)
			node->modes.beacon = mode;
	}

	if (!json_object_object_get_ex(jconfig, "relay", &jrelay))
		return;

	if (json_object_object_get_ex(jrelay, "mode", &jvalue)) {
		mode = get_mode(jvalue);
		if (mode <= MESH_MODE_UNSUPPORTED)
			node->modes.relay.state = mode;
		else
			return;
	} else
		return;

	if (!json_object_object_get_ex(jrelay, "count", &jvalue))
		return;

	/* TODO: check range */
	count = json_object_get_int(jvalue);
	node->modes.relay.cnt = count;

	if (!json_object_object_get_ex(jrelay, "interval", &jvalue))
		return;

	/* TODO: check range */
	interval = json_object_get_int(jvalue);
	node->modes.relay.interval = interval;
}

static bool parse_composition(json_object *jcomp, struct mesh_config_node *node)
{
	json_object *jvalue;
	char *str;

	/* All the fields in node composition are mandatory */
	if (!json_object_object_get_ex(jcomp, "cid", &jvalue))
		return false;

	str = (char *)json_object_get_string(jvalue);
	if (sscanf(str, "%04hx", &node->cid) != 1)
		return false;

	if (!json_object_object_get_ex(jcomp, "pid", &jvalue))
		return false;

	str = (char *)json_object_get_string(jvalue);
	if (sscanf(str, "%04hx", &node->pid) != 1)
		return false;

	if (!json_object_object_get_ex(jcomp, "vid", &jvalue))
		return false;

	str = (char *)json_object_get_string(jvalue);
	if (sscanf(str, "%04hx", &node->vid) != 1)
		return false;

	if (!json_object_object_get_ex(jcomp, "crpl", &jvalue))
		return false;

	str = (char *)json_object_get_string(jvalue);
	if (sscanf(str, "%04hx", &node->crpl) != 1)
		return false;

	return true;
}

static bool read_net_transmit(json_object *jobj, struct mesh_config_node *node)
{
	json_object *jrtx, *jvalue;
	uint16_t interval;
	uint8_t cnt;

	if (!json_object_object_get_ex(jobj, "retransmit", &jrtx))
		return true;

	if (!json_object_object_get_ex(jrtx, "count", &jvalue))
		return false;

	/* TODO: add range checking */
	cnt = (uint8_t) json_object_get_int(jvalue);

	if (!json_object_object_get_ex(jrtx, "interval", &jvalue))
		return false;

	interval = (uint16_t) json_object_get_int(jvalue);

	node->net_transmit = l_new(struct mesh_config_transmit, 1);
	node->net_transmit->count = cnt;
	node->net_transmit->interval = interval;

	return true;
}

static bool read_node(json_object *jnode, struct mesh_config_node *node)
{
	json_object *jvalue;

	if (!read_iv_index(jnode, &node->iv_index, &node->iv_update)) {
		l_info("Failed to read IV index");
		return false;
	}

	if (!read_token(jnode, node->token)) {
		l_info("Failed to read node token");
		return false;
	}

	if (!read_device_key(jnode, node->dev_key)) {
		l_info("Failed to read node device key");
		return false;
	}

	if (!parse_composition(jnode, node)) {
		l_info("Failed to parse local node composition");
		return false;
	}

	parse_features(jnode, node);

	if (!read_unicast_address(jnode, &node->unicast)) {
		l_info("Failed to parse unicast address");
		return false;
	}

	if (!read_default_ttl(jnode, &node->ttl)) {
		l_info("Failed to parse default ttl");
		return false;
	}

	if (!read_seq_number(jnode, &node->seq_number)) {
		l_info("Failed to parse sequence number");
		return false;
	}

	/* Check for required "elements" property */
	if (!json_object_object_get_ex(jnode, "elements", &jvalue))
		return false;

	if (!read_net_transmit(jnode, node)) {
		l_info("Failed to read node net transmit parameters");
		return false;
	}

	if (!read_net_keys(jnode, node)) {
		l_info("Failed to read net keys");
		return false;
	}

	if (!read_app_keys(jnode, node)) {
		l_info("Failed to read app keys");
		return false;
	}

	if (!read_comp_pages(jnode, node)) {
		l_info("Failed to read Composition Pages");
		return false;
	}

	if (!parse_elements(jvalue, node)) {
		l_info("Failed to parse elements");
		return false;
	}

	return true;
}

static bool write_uint16_hex(json_object *jobj, const char *desc,
								uint16_t value)
{
	json_object *jstring;
	char buf[5];

	snprintf(buf, 5, "%4.4x", value);
	jstring = json_object_new_string(buf);
	if (!jstring)
		return false;

	json_object_object_del(jobj, desc);
	json_object_object_add(jobj, desc, jstring);
	return true;
}

static bool write_uint32_hex(json_object *jobj, const char *desc, uint32_t val)
{
	json_object *jstring;
	char buf[9];

	snprintf(buf, 9, "%8.8x", val);
	jstring = json_object_new_string(buf);
	if (!jstring)
		return false;

	json_object_object_del(jobj, desc);
	json_object_object_add(jobj, desc, jstring);
	return true;
}

static const char *mode_to_string(int mode)
{
	switch (mode) {
	case MESH_MODE_DISABLED:
		return "disabled";
	case MESH_MODE_ENABLED:
		return "enabled";
	default:
		return "unsupported";
	}
}

static bool write_mode(json_object *jobj, const char *desc, int value)
{
	json_object *jstring;

	jstring = json_object_new_string(mode_to_string(value));

	if (!jstring)
		return false;

	json_object_object_del(jobj, desc);
	json_object_object_add(jobj, desc, jstring);

	return true;
}

bool mesh_config_write_mode(struct mesh_config *cfg, const char *keyword,
								int value)
{
	if (!cfg || !write_mode(cfg->jnode, keyword, value))
		return false;

	return save_config(cfg->jnode, cfg->node_dir_path);
}

static bool write_relay_mode(json_object *jobj, uint8_t mode,
					uint8_t count, uint16_t interval)
{
	json_object *jrelay;

	json_object_object_del(jobj, "relay");

	jrelay = json_object_new_object();
	if (!jrelay)
		return false;

	if (!write_mode(jrelay, "mode", mode))
		goto fail;

	if (!write_int(jrelay, "count", count))
		goto fail;

	if (!write_int(jrelay, "interval", interval))
		goto fail;

	json_object_object_add(jobj, "relay", jrelay);

	return true;
fail:
	json_object_put(jrelay);
	return false;
}

bool mesh_config_write_unicast(struct mesh_config *cfg, uint16_t unicast)
{
	if (!cfg || !write_uint16_hex(cfg->jnode, "unicastAddress", unicast))
		return false;

	return save_config(cfg->jnode, cfg->node_dir_path);
}

bool mesh_config_write_relay_mode(struct mesh_config *cfg, uint8_t mode,
					uint8_t count, uint16_t interval)
{

	if (!cfg || !write_relay_mode(cfg->jnode, mode, count, interval))
		return false;

	return save_config(cfg->jnode, cfg->node_dir_path);
}

bool mesh_config_write_net_transmit(struct mesh_config *cfg, uint8_t cnt,
							uint16_t interval)
{
	json_object *jnode, *jrtx;

	if (!cfg)
		return false;

	jnode = cfg->jnode;

	jrtx = json_object_new_object();
	if (!jrtx)
		return false;

	if (!write_int(jrtx, "count", cnt))
		goto fail;

	if (!write_int(jrtx, "interval", interval))
		goto fail;

	json_object_object_del(jnode, "retransmit");
	json_object_object_add(jnode, "retransmit", jrtx);

	return save_config(cfg->jnode, cfg->node_dir_path);

fail:
	json_object_put(jrtx);
	return false;

}

bool mesh_config_write_iv_index(struct mesh_config *cfg, uint32_t idx,
								bool update)
{
	json_object *jnode;
	int tmp = update ? 1 : 0;

	if (!cfg)
		return false;

	jnode = cfg->jnode;

	if (!write_int(jnode, "IVindex", idx))
		return false;

	if (!write_int(jnode, "IVupdate", tmp))
		return false;

	return save_config(jnode, cfg->node_dir_path);
}

static void add_model(void *a, void *b)
{
	struct mesh_config_model *mod = a;
	json_object *jmodels = b, *jmodel, *jval;
	bool result;

	jmodel = json_object_new_object();
	if (!jmodel)
		return;

	result = (mod->vendor) ? write_uint32_hex(jmodel, "modelId", mod->id) :
			write_uint16_hex(jmodel, "modelId", (uint16_t) mod->id);

	if (!result) {
		json_object_put(jmodel);
		return;
	}

	jval = json_object_new_boolean(mod->sub_enabled);
	json_object_object_add(jmodel, "subEnabled", jval);

	jval = json_object_new_boolean(mod->pub_enabled);
	json_object_object_add(jmodel, "pubEnabled", jval);

	json_object_array_add(jmodels, jmodel);
}

/* Add unprovisioned node (local) */
static struct mesh_config *create_config(const char *cfg_path,
					const uint8_t uuid[16],
					struct mesh_config_node *node)
{
	struct mesh_config_modes *modes = &node->modes;
	const struct l_queue_entry *entry;
	json_object *jnode, *jelems;
	struct mesh_config *cfg;

	if (!cfg_path || !node)
		return NULL;

	jnode = json_object_new_object();

	/* CID, PID, VID, crpl */
	if (!write_uint16_hex(jnode, "cid", node->cid))
		return NULL;

	if (!write_uint16_hex(jnode, "pid", node->pid))
		return NULL;

	if (!write_uint16_hex(jnode, "vid", node->vid))
		return NULL;

	if (!write_uint16_hex(jnode, "crpl", node->crpl))
		return NULL;

	/* Features: relay, LPN, friend, proxy*/
	if (!write_relay_mode(jnode, modes->relay.state,
				modes->relay.cnt, modes->relay.interval))
		return NULL;

	if (!write_mode(jnode, "lowPower", modes->lpn))
		return NULL;

	if (!write_mode(jnode, "friend", modes->friend))
		return NULL;

	if (!write_mode(jnode, "proxy", modes->proxy))
		return NULL;

	/* Beaconing state */
	if (!write_mode(jnode, "beacon", modes->beacon))
		return NULL;

	/* Sequence number */
	json_object_object_add(jnode, "sequenceNumber",
					json_object_new_int(node->seq_number));

	/* Default TTL */
	json_object_object_add(jnode, "defaultTTL",
						json_object_new_int(node->ttl));

	/* Elements */
	jelems = json_object_new_array();
	if (!jelems)
		return NULL;

	entry = l_queue_get_entries(node->elements);

	for (; entry; entry = entry->next) {
		struct mesh_config_element *ele = entry->data;
		json_object *jelement, *jmodels;

		jelement = json_object_new_object();

		if (!jelement)
			goto fail;

		json_object_array_add(jelems, jelement);

		if (!write_int(jelement, "elementIndex", ele->index))
			goto fail;

		if (!write_uint16_hex(jelement, "location", ele->location))
			goto fail;

		/* Models */
		if (l_queue_isempty(ele->models))
			continue;

		jmodels = json_object_new_array();
		if (!jmodels)
			goto fail;

		json_object_object_add(jelement, "models", jmodels);
		l_queue_foreach(ele->models, add_model, jmodels);
	}

	json_object_object_add(jnode, "elements", jelems);

	cfg = l_new(struct mesh_config, 1);

	cfg->jnode = jnode;
	memcpy(cfg->uuid, uuid, 16);
	cfg->node_dir_path = l_strdup(cfg_path);
	cfg->write_seq = node->seq_number;
	cfg->idles = l_queue_new();
	gettimeofday(&cfg->write_time, NULL);

	return cfg;

fail:
		json_object_put(jelems);
		return NULL;
}

struct mesh_config *mesh_config_create(const char *cfgdir_name,
		const uint8_t uuid[16], struct mesh_config_node *db_node)
{
	char uuid_buf[33];
	char name_buf[PATH_MAX];
	struct mesh_config *cfg;
	size_t max_len = strlen(cfgnode_name) + strlen(bak_ext);

	if (!hex2str((uint8_t *) uuid, 16, uuid_buf, sizeof(uuid_buf)))
		return NULL;

	snprintf(name_buf, PATH_MAX, "%s/%s", cfgdir_name, uuid_buf);

	if (strlen(name_buf) + max_len >= PATH_MAX)
		return NULL;

	/* Create a new directory and node.json file */
	if (mkdir(name_buf, 0755) != 0)
		return NULL;

	snprintf(name_buf, PATH_MAX, "%s/%s%s", cfgdir_name, uuid_buf,
								cfgnode_name);
	l_debug("New node config %s", name_buf);

	cfg = create_config(name_buf, uuid, db_node);
	if (!cfg)
		return NULL;

	if (!mesh_config_save(cfg, true, NULL, NULL)) {
		mesh_config_release(cfg);
		return NULL;
	}

	return cfg;
}

static void finish_key_refresh(json_object *jobj, uint16_t net_idx)
{
	json_object *jarray;
	int i, len;

	/* Clean up all the bound appkeys */
	if (!json_object_object_get_ex(jobj, "appKeys", &jarray))
		return;

	len = json_object_array_length(jarray);

	for (i = 0; i < len; ++i) {
		json_object *jentry;
		uint16_t idx;

		jentry = json_object_array_get_idx(jarray, i);

		if (!get_key_index(jentry, "boundNetKey", &idx))
			continue;

		if (idx != net_idx)
			continue;

		json_object_object_del(jentry, "oldKey");

		if (!get_key_index(jentry, "index", &idx))
			continue;
	}

}

bool mesh_config_net_key_set_phase(struct mesh_config *cfg, uint16_t idx,
								uint8_t phase)
{
	json_object *jnode, *jarray, *jentry = NULL;

	if (!cfg)
		return false;

	jnode = cfg->jnode;

	if (json_object_object_get_ex(jnode, "netKeys", &jarray))
		jentry = get_key_object(jarray, idx);

	if (!jentry)
		return false;

	json_object_object_del(jentry, "keyRefresh");
	json_object_object_add(jentry, "keyRefresh",
					json_object_new_int(phase));

	if (phase == KEY_REFRESH_PHASE_NONE) {
		json_object_object_del(jentry, "oldKey");
		finish_key_refresh(jnode, idx);
	}

	return save_config(jnode, cfg->node_dir_path);
}

bool mesh_config_model_pub_add(struct mesh_config *cfg, uint16_t ele_addr,
					uint32_t mod_id, bool vendor,
					struct mesh_config_pub *pub)
{
	json_object *jnode, *jmodel, *jpub, *jrtx;
	bool res;
	int ele_idx;

	if (!cfg)
		return false;

	jnode = cfg->jnode;

	ele_idx = get_element_index(jnode, ele_addr);
	if (ele_idx < 0)
		return false;

	jmodel = get_element_model(jnode, ele_idx, mod_id, vendor);
	if (!jmodel)
		return false;

	json_object_object_del(jmodel, "publish");

	jpub = json_object_new_object();
	if (!jpub)
		return false;

	if (pub->virt)
		res = add_key_value(jpub, "address", pub->virt_addr);
	else
		res = write_uint16_hex(jpub, "address", pub->addr);

	if (!res)
		goto fail;

	if (!write_int(jpub, "index", pub->idx))
		goto fail;

	if (!write_int(jpub, "ttl", pub->ttl))
		goto fail;

	if (!write_int(jpub, "period", pub->period))
		goto fail;

	if (!write_int(jpub, "credentials", pub->credential ? 1 : 0))
		goto fail;

	jrtx = json_object_new_object();
	if (!jrtx)
		goto fail;

	if (!write_int(jrtx, "count", pub->cnt))
		goto fail;

	if (!write_int(jrtx, "interval", pub->interval))
		goto fail;

	json_object_object_add(jpub, "retransmit", jrtx);
	json_object_object_add(jmodel, "publish", jpub);

	return save_config(jnode, cfg->node_dir_path);

fail:
	json_object_put(jpub);
	return false;
}

static bool delete_model_property(json_object *jnode, uint16_t ele_addr,
			uint32_t mod_id, bool vendor, const char *keyword)
{
	json_object *jmodel;
	int ele_idx;

	ele_idx = get_element_index(jnode, ele_addr);
	if (ele_idx < 0)
		return false;

	jmodel = get_element_model(jnode, ele_idx, mod_id, vendor);
	if (!jmodel)
		return false;

	json_object_object_del(jmodel, keyword);

	return true;
}

bool mesh_config_model_pub_del(struct mesh_config *cfg, uint16_t addr,
						uint32_t mod_id, bool vendor)
{
	if (!cfg || !delete_model_property(cfg->jnode, addr, mod_id, vendor,
								"publish"))
		return false;

	return save_config(cfg->jnode, cfg->node_dir_path);
}

static void del_page(json_object *jarray, uint8_t page)
{
	char buf[3];
	int i, len;

	if (!jarray)
		return;

	snprintf(buf, 3, "%2.2x", page);

	len = json_object_array_length(jarray);

	for (i = 0; i < len; i++) {
		json_object *jentry;
		char *str;

		jentry = json_object_array_get_idx(jarray, i);
		str = (char *)json_object_get_string(jentry);

		/* Delete matching page(s) */
		if (!memcmp(str, buf, 2))
			json_object_array_del_idx(jarray, i, 1);
	}
}

bool mesh_config_comp_page_add(struct mesh_config *cfg, uint8_t page,
						uint8_t *data, uint16_t size)
{
	json_object *jnode, *jstring, *jarray = NULL;
	char *buf;
	int len;

	if (!cfg)
		return false;

	jnode = cfg->jnode;

	json_object_object_get_ex(jnode, "pages", &jarray);

	len = (size * 2) + 3;
	buf = l_malloc(len);
	snprintf(buf, len, "%2.2x", page);
	hex2str(data, size, buf + 2, len - 2);

	if (jarray && jarray_has_string(jarray, buf, len)) {
		l_free(buf);
		return true;
	} else if (!jarray) {
		jarray = json_object_new_array();
		json_object_object_add(jnode, "pages", jarray);
	} else
		del_page(jarray, page);

	jstring = json_object_new_string(buf);
	json_object_array_add(jarray, jstring);
	l_free(buf);

	return save_config(jnode, cfg->node_dir_path);
}

bool mesh_config_comp_page_mv(struct mesh_config *cfg, uint8_t old, uint8_t nw)
{
	json_object *jnode, *jarray = NULL;
	uint8_t *data;
	char *str;
	char old_buf[3];
	int i, len, dlen = 0;
	bool status = true;

	if (!cfg || old == nw)
		return false;

	jnode = cfg->jnode;

	json_object_object_get_ex(jnode, "pages", &jarray);

	if (!jarray)
		return false;

	snprintf(old_buf, 3, "%2.2x", old);
	data = l_malloc(MAX_MSG_LEN);

	len = json_object_array_length(jarray);

	for (i = 0; i < len; i++) {
		json_object *jentry;

		jentry = json_object_array_get_idx(jarray, i);
		str = (char *)json_object_get_string(jentry);

		/* Delete matching page(s) but save data*/
		if (!memcmp(str, old_buf, 2)) {
			dlen = strlen(str + 2);
			str2hex(str + 2, dlen, data, MAX_MSG_LEN);
			dlen /= 2;
			json_object_array_del_idx(jarray, i, 1);
		}
	}

	if (dlen)
		status = mesh_config_comp_page_add(cfg, nw, data, dlen);

	l_free(data);

	return status;
}

bool mesh_config_model_sub_add(struct mesh_config *cfg, uint16_t ele_addr,
						uint32_t mod_id, bool vendor,
						struct mesh_config_sub *sub)
{
	json_object *jnode, *jmodel, *jstring, *jarray = NULL;
	int ele_idx, len;
	char buf[33];

	if (!cfg)
		return false;

	jnode = cfg->jnode;

	ele_idx = get_element_index(jnode, ele_addr);
	if (ele_idx < 0)
		return false;

	jmodel = get_element_model(jnode, ele_idx, mod_id, vendor);
	if (!jmodel)
		return false;

	if (!sub->virt) {
		snprintf(buf, 5, "%4.4x", sub->addr.grp);
		len = 4;
	} else {
		hex2str(sub->addr.label, 16, buf, 33);
		len = 32;
	}

	json_object_object_get_ex(jmodel, "subscribe", &jarray);
	if (jarray && jarray_has_string(jarray, buf, len))
		return true;

	jstring = json_object_new_string(buf);
	if (!jstring)
		return false;

	if (!jarray) {
		jarray = json_object_new_array();
		if (!jarray) {
			json_object_put(jstring);
			return false;
		}
		json_object_object_add(jmodel, "subscribe", jarray);
	}

	json_object_array_add(jarray, jstring);

	return save_config(jnode, cfg->node_dir_path);
}

bool mesh_config_model_sub_del(struct mesh_config *cfg, uint16_t ele_addr,
						uint32_t mod_id, bool vendor,
						struct mesh_config_sub *sub)
{
	json_object *jnode, *jmodel, *jarray;
	char buf[33];
	int len, ele_idx;

	if (!cfg)
		return false;

	jnode = cfg->jnode;

	ele_idx = get_element_index(jnode, ele_addr);
	if (ele_idx < 0)
		return false;

	jmodel = get_element_model(jnode, ele_idx, mod_id, vendor);
	if (!jmodel)
		return false;

	if (!json_object_object_get_ex(jmodel, "subscribe", &jarray))
		return true;

	if (!sub->virt) {
		snprintf(buf, 5, "%4.4x", sub->addr.grp);
		len = 4;
	} else {
		hex2str(sub->addr.label, 16, buf, 33);
		len = 32;
	}

	jarray_string_del(jarray, buf, len);

	if (!json_object_array_length(jarray))
		json_object_object_del(jmodel, "subscribe");

	return save_config(jnode, cfg->node_dir_path);
}

bool mesh_config_model_sub_del_all(struct mesh_config *cfg, uint16_t addr,
						uint32_t mod_id, bool vendor)
{
	if (!cfg || !delete_model_property(cfg->jnode, addr, mod_id, vendor,
								"subscribe"))
		return false;

	return save_config(cfg->jnode, cfg->node_dir_path);
}

bool mesh_config_model_pub_enable(struct mesh_config *cfg, uint16_t ele_addr,
						uint32_t mod_id, bool vendor,
						bool enable)
{
	json_object *jmodel, *jval;
	int ele_idx;

	if (!cfg)
		return false;

	ele_idx = get_element_index(cfg->jnode, ele_addr);
	if (ele_idx < 0)
		return false;

	jmodel = get_element_model(cfg->jnode, ele_idx, mod_id, vendor);
	if (!jmodel)
		return false;

	json_object_object_del(jmodel, "pubDisabled");

	jval = json_object_new_boolean(!enable);
	json_object_object_add(jmodel, "pubDisabled", jval);

	if (!enable)
		json_object_object_del(jmodel, "publish");

	return save_config(cfg->jnode, cfg->node_dir_path);
}

bool mesh_config_model_sub_enable(struct mesh_config *cfg, uint16_t ele_addr,
						uint32_t mod_id, bool vendor,
						bool enable)
{
	json_object *jmodel, *jval;
	int ele_idx;

	if (!cfg)
		return false;

	ele_idx = get_element_index(cfg->jnode, ele_addr);
	if (ele_idx < 0)
		return false;

	jmodel = get_element_model(cfg->jnode, ele_idx, mod_id, vendor);
	if (!jmodel)
		return false;

	json_object_object_del(jmodel, "subEnabled");

	jval = json_object_new_boolean(enable);
	json_object_object_add(jmodel, "subEnabled", jval);

	if (!enable)
		json_object_object_del(jmodel, "subscribe");

	return save_config(cfg->jnode, cfg->node_dir_path);
}

bool mesh_config_write_seq_number(struct mesh_config *cfg, uint32_t seq,
								bool cache)
{
	int value = 0;
	uint32_t cached = 0;

	if (!cfg)
		return false;

	if (!cache) {
		if (!write_int(cfg->jnode, "sequenceNumber", seq))
			return false;

		return mesh_config_save(cfg, true, NULL, NULL);
	}

	/* If resetting seq to Zero, make sure cached value reset as well */
	if (seq && get_int(cfg->jnode, "sequenceNumber", &value))
		cached = (uint32_t)value;

	/*
	 * When sequence number approaches value stored on disk, calculate
	 * average time between sequence number updates, then overcommit the
	 * sequence number by MIN_SEQ_CACHE_TIME seconds worth of traffic or
	 * MIN_SEQ_CACHE_VALUE (whichever is greater) to avoid frequent writes
	 * to disk and to protect against crashes.
	 *
	 * The real value will be saved when daemon shuts down properly.
	 */
	if (seq + MIN_SEQ_CACHE_TRIGGER >= cached) {
		struct timeval now;
		struct timeval elapsed;
		uint64_t elapsed_ms;

		gettimeofday(&now, NULL);
		timersub(&now, &cfg->write_time, &elapsed);
		elapsed_ms = elapsed.tv_sec * 1000 + elapsed.tv_usec / 1000;

		/*
		 * If time since last write is zero, this means that
		 * idle_save_config is already pending, so we don't need to do
		 * anything.
		 */
		if (!elapsed_ms)
			return true;

		cached = seq + (seq - cfg->write_seq) *
					1000 * MIN_SEQ_CACHE_TIME / elapsed_ms;

		if (cached < seq + MIN_SEQ_CACHE_VALUE)
			cached = seq + MIN_SEQ_CACHE_VALUE;

		/* Cap the seq cache maximum to fixed out-of-range value.
		 * If daemon restarts with out-of-range value, no packets
		 * are to be sent until IV Update procedure completes.
		 */
		if (cached > SEQ_MASK)
			cached = SEQ_MASK + 1;

		cfg->write_seq = seq;

		/* Don't rewrite NVM storage if unchanged */
		if (value == (int) cached)
			return true;

		l_debug("Seq Cache: %d -> %d", seq, cached);

		if (!write_int(cfg->jnode, "sequenceNumber", cached))
		    return false;

		return mesh_config_save(cfg, false, NULL, NULL);
	}

	return true;
}

bool mesh_config_write_ttl(struct mesh_config *cfg, uint8_t ttl)
{
	if (!cfg || !write_int(cfg->jnode, "defaultTTL", ttl))
		return false;

	return save_config(cfg->jnode, cfg->node_dir_path);
}

bool mesh_config_update_company_id(struct mesh_config *cfg, uint16_t cid)
{
	if (!cfg || !write_uint16_hex(cfg->jnode, "cid", cid))
		return false;

	return save_config(cfg->jnode, cfg->node_dir_path);
}

bool mesh_config_update_product_id(struct mesh_config *cfg, uint16_t pid)
{
	if (!cfg || !write_uint16_hex(cfg->jnode, "pid", pid))
		return false;

	return save_config(cfg->jnode, cfg->node_dir_path);
}

bool mesh_config_update_version_id(struct mesh_config *cfg, uint16_t vid)
{
	if (!cfg || !write_uint16_hex(cfg->jnode, "vid", vid))
		return false;

	return save_config(cfg->jnode, cfg->node_dir_path);
}

bool mesh_config_update_crpl(struct mesh_config *cfg, uint16_t crpl)
{
	if (!cfg || !write_uint16_hex(cfg->jnode, "crpl", crpl))
		return false;

	return save_config(cfg->jnode, cfg->node_dir_path);
}

static bool load_node(const char *fname, const uint8_t uuid[16],
				mesh_config_node_func_t cb, void *user_data)
{
	int fd;
	char *str;
	struct stat st;
	ssize_t sz;
	bool result = false;
	json_object *jnode;
	struct mesh_config_node node;

	if (!cb) {
		l_info("Node read callback is required");
		return false;
	}

	l_info("Loading configuration from %s", fname);

	fd = open(fname, O_RDONLY);
	if (fd < 0)
		return false;

	if (fstat(fd, &st) == -1) {
		close(fd);
		return false;
	}

	str = (char *) l_new(char, st.st_size + 1);
	if (!str) {
		close(fd);
		return false;
	}

	sz = read(fd, str, st.st_size);
	if (sz != st.st_size) {
		l_error("Failed to read configuration file %s", fname);
		goto done;
	}

	jnode = json_tokener_parse(str);
	if (!jnode)
		goto done;

	memset(&node, 0, sizeof(node));

	node.elements = l_queue_new();
	node.netkeys = l_queue_new();
	node.appkeys = l_queue_new();
	node.pages = l_queue_new();

	result = read_node(jnode, &node);

	if (result) {
		struct mesh_config *cfg = l_new(struct mesh_config, 1);

		cfg->jnode = jnode;
		memcpy(cfg->uuid, uuid, 16);
		cfg->node_dir_path = l_strdup(fname);
		cfg->write_seq = node.seq_number;
		cfg->idles = l_queue_new();
		gettimeofday(&cfg->write_time, NULL);

		result = cb(&node, uuid, cfg, user_data);

		if (!result) {
			l_free(cfg->idles);
			l_free(cfg->node_dir_path);
			l_free(cfg);
		}
	}

	/* Done with the node: free resources */
	l_free(node.net_transmit);
	l_queue_destroy(node.netkeys, l_free);
	l_queue_destroy(node.appkeys, l_free);
	l_queue_destroy(node.pages, l_free);
	l_queue_destroy(node.elements, free_element);

	if (!result)
		json_object_put(jnode);

done:
	close(fd);
	if (str)
		l_free(str);

	return result;
}

static void release_idle(void *data)
{
	struct l_idle *idle = data;

	l_idle_remove(idle);
}

void mesh_config_release(struct mesh_config *cfg)
{
	if (!cfg)
		return;

	l_queue_destroy(cfg->idles, release_idle);

	l_free(cfg->node_dir_path);
	json_object_put(cfg->jnode);
	l_free(cfg);
}

static void idle_save_config(struct l_idle *idle, void *user_data)
{
	struct write_info *info = user_data;
	char *fname_tmp, *fname_bak, *fname_cfg;
	bool result = false;

	fname_cfg = info->cfg->node_dir_path;
	fname_tmp = l_strdup_printf("%s%s", fname_cfg, tmp_ext);
	fname_bak = l_strdup_printf("%s%s", fname_cfg, bak_ext);
	remove(fname_tmp);

	result = save_config(info->cfg->jnode, fname_tmp);

	if (result) {
		remove(fname_bak);

		if (rename(fname_cfg, fname_bak) < 0 ||
					rename(fname_tmp, fname_cfg) < 0)
			result = false;
	}

	remove(fname_tmp);

	l_free(fname_tmp);
	l_free(fname_bak);

	gettimeofday(&info->cfg->write_time, NULL);

	if (info->cb)
		info->cb(info->user_data, result);

	if (idle) {
		l_queue_remove(info->cfg->idles, idle);
		l_idle_remove(idle);
	}

	l_free(info);

}

bool mesh_config_save(struct mesh_config *cfg, bool no_wait,
				mesh_config_status_func_t cb, void *user_data)
{
	struct write_info *info;

	if (!cfg)
		return false;

	info = l_new(struct write_info, 1);
	info->cfg = cfg;
	info->cb = cb;
	info->user_data = user_data;

	if (no_wait) {
		idle_save_config(NULL, info);
	} else {
		struct l_idle *idle;

		idle = l_idle_create(idle_save_config, info, NULL);
		l_queue_push_tail(cfg->idles, idle);
	}

	return true;
}

bool mesh_config_load_nodes(const char *cfgdir_name, mesh_config_node_func_t cb,
								void *user_data)
{
	DIR *cfgdir;
	struct dirent *entry;
	size_t path_len = strlen(cfgdir_name) + strlen(cfgnode_name) +
								strlen(bak_ext);

	create_dir(cfgdir_name);
	cfgdir = opendir(cfgdir_name);
	if (!cfgdir) {
		l_error("Failed to open mesh node storage directory: %s",
								cfgdir_name);
		return false;
	}

	while ((entry = readdir(cfgdir)) != NULL) {
		char *dirname, *fname, *bak;
		uint8_t uuid[16];
		size_t node_len;

		if (entry->d_type != DT_DIR)
			continue;

		/* Check path length */
		node_len = strlen(entry->d_name);

		if (path_len + node_len + 1 >= PATH_MAX)
			continue;

		if (!str2hex(entry->d_name, node_len, uuid, sizeof(uuid)))
			continue;

		dirname = l_strdup_printf("%s/%s", cfgdir_name, entry->d_name);
		fname = l_strdup_printf("%s%s", dirname, cfgnode_name);

		if (!load_node(fname, uuid, cb, user_data)) {

			/* Fall-back to Backup version */
			bak = l_strdup_printf("%s%s", fname, bak_ext);

			if (load_node(bak, uuid, cb, user_data)) {
				remove(fname);
				rename(bak, fname);
			}

			l_free(bak);
		}

		l_free(fname);
		l_free(dirname);
	}

	closedir(cfgdir);

	return true;
}

void mesh_config_destroy_nvm(struct mesh_config *cfg)
{
	char *node_dir, *node_name;
	char uuid[33];

	if (!cfg)
		return;

	node_dir = dirname(cfg->node_dir_path);
	l_debug("Delete node config %s", node_dir);

	if (!hex2str(cfg->uuid, 16, uuid, sizeof(uuid)))
		return;

	node_name = basename(node_dir);

	/* Make sure path name of node follows expected guidelines */
	if (strcmp(node_name, uuid))
		return;

	del_path(node_dir);
}

// SPDX-License-Identifier: LGPL-2.1-or-later
/*
 *
 *  BlueZ - Bluetooth protocol stack for Linux
 *
 *  Copyright (C) 2019  Intel Corporation. All rights reserved.
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
#include <stdio.h>
#include <string.h>
#include <unistd.h>

#include <sys/time.h>

#include <ell/ell.h>
#include <json-c/json.h>

#include "mesh/mesh-defs.h"
#include "mesh/util.h"

#include "tools/mesh/keys.h"
#include "tools/mesh/remote.h"
#include "tools/mesh/cfgcli.h"
#include "tools/mesh/mesh-db.h"

#define KEY_IDX_INVALID NET_IDX_INVALID
#define DEFAULT_LOCATION 0x0000

struct mesh_db {
	json_object *jcfg;
	char *cfg_fname;
	uint8_t token[8];
	struct timeval write_time;
};

static struct mesh_db *cfg;
static const char *bak_ext = ".bak";
static const char *tmp_ext = ".tmp";

static bool save_config_file(const char *fname)
{
	FILE *outfile;
	const char *str;
	bool result = false;

	outfile = fopen(fname, "w");
	if (!outfile) {
		l_error("Failed to save configuration to %s", cfg->cfg_fname);
		return false;
	}

	str = json_object_to_json_string_ext(cfg->jcfg,
						JSON_C_TO_STRING_PRETTY);

	if (fwrite(str, sizeof(char), strlen(str), outfile) < strlen(str))
		l_warn("Incomplete write of mesh configuration");
	else
		result = true;

	fclose(outfile);

	return result;
}

static bool save_config(void)
{
	char *fname_tmp, *fname_bak, *fname_cfg;
	bool result = false;

	fname_cfg = cfg->cfg_fname;
	fname_tmp = l_strdup_printf("%s%s", fname_cfg, tmp_ext);
	fname_bak = l_strdup_printf("%s%s", fname_cfg, bak_ext);
	remove(fname_tmp);

	result = save_config_file(fname_tmp);

	if (result) {
		remove(fname_bak);
		rename(fname_cfg, fname_bak);
		rename(fname_tmp, fname_cfg);
	}

	remove(fname_tmp);

	l_free(fname_tmp);
	l_free(fname_bak);

	gettimeofday(&cfg->write_time, NULL);

	return result;
}

static void release_config(void)
{
	l_free(cfg->cfg_fname);
	json_object_put(cfg->jcfg);
	l_free(cfg);
	cfg = NULL;
}

static json_object *get_node_by_unicast(uint16_t unicast)
{
	json_object *jarray;
	int i, sz;

	if (!json_object_object_get_ex(cfg->jcfg, "nodes", &jarray))
		return NULL;

	if (!jarray || json_object_get_type(jarray) != json_type_array)
		return NULL;

	sz = json_object_array_length(jarray);

	for (i = 0; i < sz; ++i) {
		json_object *jentry, *jval;
		uint16_t addr;
		const char *str;

		jentry = json_object_array_get_idx(jarray, i);
		if (!json_object_object_get_ex(jentry, "unicastAddress",
								&jval))
			return NULL;

		str = json_object_get_string(jval);
		if (sscanf(str, "%04hx", &addr) != 1)
			continue;

		if (addr == unicast)
			return jentry;
	}

	return NULL;
}

static bool get_int(json_object *jobj, const char *keyword, int *value)
{
	json_object *jvalue;

	if (!json_object_object_get_ex(jobj, keyword, &jvalue))
		return false;

	*value = json_object_get_int(jvalue);
	if (errno == EINVAL) {
		l_error("Error: %s should contain an integer value\n",
								keyword);
		return false;
	}

	return true;
}

static bool write_int(json_object *jobj, const char *keyword, int val)
{
	json_object *jval;

	json_object_object_del(jobj, keyword);

	jval = json_object_new_int(val);
	if (!jval)
		return false;

	json_object_object_add(jobj, keyword, jval);
	return true;
}

static json_object *get_key_object(json_object *jarray, uint16_t idx)
{
	int i, sz = json_object_array_length(jarray);

	for (i = 0; i < sz; ++i) {
		json_object *jentry;
		int jidx;

		jentry = json_object_array_get_idx(jarray, i);
		if (!get_int(jentry, "index", &jidx))
			return NULL;

		if (jidx == idx)
			return jentry;
	}

	return NULL;
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

	/* Overwrite old value if present */
	json_object_object_del(jobj, desc);

	json_object_object_add(jobj, desc, jstring);
	return true;
}

static json_object *get_node_by_uuid(json_object *jcfg, uint8_t uuid[16])
{
	json_object *jarray = NULL;
	char buf[33];
	int i, sz;

	hex2str(uuid, 16, buf, sizeof(buf));

	json_object_object_get_ex(jcfg, "nodes", &jarray);
	if (!jarray || json_object_get_type(jarray) != json_type_array)
		return NULL;

	sz = json_object_array_length(jarray);

	for (i = 0; i < sz; ++i) {
		json_object *jentry, *jval;
		const char *str;

		jentry = json_object_array_get_idx(jarray, i);
		if (!json_object_object_get_ex(jentry, "uuid", &jval))
			return NULL;

		str = json_object_get_string(jval);
		if (strlen(str) != 32)
			continue;

		if (!strcmp(buf, str))
			return jentry;
	}

	return NULL;
}

static bool add_u8_8(json_object *jobj, const char *desc,
							const uint8_t value[8])
{
	json_object *jstring;
	char buf[17];

	hex2str((uint8_t *) value, 8, buf, 17);
	jstring = json_object_new_string(buf);
	if (!jstring)
		return false;

	json_object_object_add(jobj, desc, jstring);
	return true;
}

static bool add_u8_16(json_object *jobj, const char *desc,
							const uint8_t value[16])
{
	json_object *jstring;
	char buf[33];

	hex2str((uint8_t *) value, 16, buf, 33);
	jstring = json_object_new_string(buf);
	if (!jstring)
		return false;

	json_object_object_add(jobj, desc, jstring);
	return true;
}

static bool add_string(json_object *jobj, const char *desc, const char *str)
{
	json_object *jstring = json_object_new_string(str);

	if (!jstring)
		return false;

	json_object_object_add(jobj, desc, jstring);
	return true;
}

static bool get_token(json_object *jobj, uint8_t token[8])
{
	json_object *jval;
	const char *str;

	if (!token)
		return false;

	if (!json_object_object_get_ex(jobj, "token", &jval))
		return false;

	str = json_object_get_string(jval);
	if (!str2hex(str, strlen(str), token, 8))
		return false;

	return true;
}

static uint16_t node_parse_key(json_object *jarray, int i)
{
	json_object *jkey;
	int idx;

	jkey = json_object_array_get_idx(jarray, i);
	if (!jkey)
		return KEY_IDX_INVALID;

	if (!get_int(jkey, "index", &idx))
		return KEY_IDX_INVALID;

	return (uint16_t)idx;
}

static int compare_group_addr(const void *a, const void *b, void *user_data)
{
	const struct mesh_group *grp0 = a;
	const struct mesh_group *grp1 = b;

	if (grp0->addr < grp1->addr)
		return -1;

	if (grp0->addr > grp1->addr)
		return 1;

	return 0;
}

static bool load_composition(json_object *jnode, uint16_t unicast)
{
	json_object *jarray;
	int i, ele_cnt;

	if (!json_object_object_get_ex(jnode, "elements", &jarray))
		return false;

	if (json_object_get_type(jarray) != json_type_array)
		return false;

	ele_cnt = json_object_array_length(jarray);

	for (i = 0; i < ele_cnt; ++i) {
		json_object *jentry, *jval, *jmods;
		int32_t index;
		int k, mod_cnt;

		jentry = json_object_array_get_idx(jarray, i);
		if (!json_object_object_get_ex(jentry, "index", &jval))
			return false;

		index = json_object_get_int(jval);
		if (index > 0xff)
			return false;

		if (!json_object_object_get_ex(jentry, "models", &jmods))
			return false;

		mod_cnt = json_object_array_length(jmods);

		for (k = 0; k < mod_cnt; ++k) {
			json_object *jmod, *jid;
			uint32_t mod_id, len;
			const char *str;

			jmod = json_object_array_get_idx(jmods, k);
			if (!json_object_object_get_ex(jmod, "modelId", &jid))
				return false;

			str = json_object_get_string(jid);
			len = strlen(str);

			if (len != 4 && len != 8)
				return false;

			if ((len == 4) && (sscanf(str, "%04x", &mod_id) != 1))
				return false;

			if ((len == 8) && (sscanf(str, "%08x", &mod_id) != 1))
				return false;

			remote_set_model(unicast, index, mod_id, len == 8);
		}
	}

	return true;
}

static void load_remotes(json_object *jcfg)
{
	json_object *jnodes;
	int i, sz, node_count = 0;

	json_object_object_get_ex(jcfg, "nodes", &jnodes);
	if (!jnodes || json_object_get_type(jnodes) != json_type_array)
		return;

	sz = json_object_array_length(jnodes);

	for (i = 0; i < sz; ++i) {
		json_object *jnode, *jval, *jarray;
		uint8_t uuid[16];
		uint16_t unicast, key_idx;
		const char *str;
		int ele_cnt, key_cnt;
		int j;

		jnode = json_object_array_get_idx(jnodes, i);
		if (!jnode)
			continue;

		if (!json_object_object_get_ex(jnode, "uuid", &jval))
			continue;

		str = json_object_get_string(jval);
		if (strlen(str) != 32)
			continue;

		str2hex(str, 32, uuid, 16);

		if (!json_object_object_get_ex(jnode, "unicastAddress", &jval))
			continue;

		str = json_object_get_string(jval);
		if (sscanf(str, "%04hx", &unicast) != 1)
			continue;

		json_object_object_get_ex(jnode, "elements", &jarray);
		if (!jarray || json_object_get_type(jarray) != json_type_array)
			continue;

		ele_cnt = json_object_array_length(jarray);

		if (ele_cnt > MAX_ELE_COUNT)
			continue;

		json_object_object_get_ex(jnode, "netKeys", &jarray);
		if (!jarray || json_object_get_type(jarray) != json_type_array)
			continue;

		key_cnt = json_object_array_length(jarray);
		if (key_cnt < 0)
			continue;

		key_idx = node_parse_key(jarray, 0);
		if (key_idx == KEY_IDX_INVALID)
			continue;

		remote_add_node((const uint8_t *)uuid, unicast, ele_cnt,
								key_idx);
		for (j = 1; j < key_cnt; j++) {
			key_idx = node_parse_key(jarray, j);

			if (key_idx != KEY_IDX_INVALID)
				remote_add_net_key(unicast, key_idx);
		}

		json_object_object_get_ex(jnode, "appKeys", &jarray);
		if (!jarray || json_object_get_type(jarray) != json_type_array)
			continue;

		key_cnt = json_object_array_length(jarray);

		for (j = 0; j < key_cnt; j++) {
			key_idx = node_parse_key(jarray, j);

			if (key_idx != KEY_IDX_INVALID)
				remote_add_app_key(unicast, key_idx);
		}

		load_composition(jnode, unicast);

		node_count++;

		/* TODO: Add the rest of the configuration */
	}

	if (node_count != sz)
		l_warn("The remote node configuration load is incomplete!");

}

static bool add_app_key(json_object *jobj, uint16_t net_idx, uint16_t app_idx)
{
	json_object *jkey, *jarray;

	json_object_object_get_ex(jobj, "appKeys", &jarray);
	if (!jarray || json_object_get_type(jarray) != json_type_array)
		return false;

	jkey = json_object_new_object();

	if (!write_int(jkey, "boundNetKey", (int)net_idx))
		goto fail;

	if (!write_int(jkey, "index", (int)app_idx))
		goto fail;

	json_object_array_add(jarray, jkey);

	return true;
fail:
	json_object_put(jkey);
	return false;
}

static bool add_node_key(json_object *jobj, const char *desc, uint16_t idx)
{
	json_object *jkey, *jarray;

	json_object_object_get_ex(jobj, desc, &jarray);
	if (!jarray || json_object_get_type(jarray) != json_type_array)
		return false;

	jkey = json_object_new_object();

	if (!write_int(jkey, "index", (int)idx)) {
		json_object_put(jkey);
		return false;
	}

	json_object_array_add(jarray, jkey);

	return save_config();
}

bool mesh_db_node_net_key_add(uint16_t unicast, uint16_t idx)
{
	json_object *jnode;

	if (!cfg || !cfg->jcfg)
		return false;

	jnode = get_node_by_unicast(unicast);
	if (!jnode)
		return false;

	return add_node_key(jnode, "netKeys", idx);
}

bool mesh_db_node_ttl_set(uint16_t unicast, uint8_t ttl)
{
	json_object *jnode;

	if (!cfg || !cfg->jcfg)
		return false;

	jnode = get_node_by_unicast(unicast);
	if (!jnode)
		return false;

	if (!write_int(jnode, "defaultTTL", ttl))
		return false;

	return save_config();
}

static void jarray_key_del(json_object *jarray, int16_t idx)
{
	int i, sz = json_object_array_length(jarray);

	for (i = 0; i < sz; ++i) {
		json_object *jentry;
		int val;

		jentry = json_object_array_get_idx(jarray, i);

		if (!get_int(jentry, "index", &val))
			continue;

		if (val == idx) {
			json_object_array_del_idx(jarray, i, 1);
			return;
		}

	}
}

static bool delete_key(json_object *jobj, const char *desc, uint16_t idx)
{
	json_object *jarray;

	if (!json_object_object_get_ex(jobj, desc, &jarray))
		return true;

	jarray_key_del(jarray, idx);

	return save_config();
}

bool mesh_db_node_net_key_del(uint16_t unicast, uint16_t net_idx)
{
	json_object *jnode;

	if (!cfg || !cfg->jcfg)
		return false;

	jnode = get_node_by_unicast(unicast);
	if (!jnode)
		return false;

	return delete_key(jnode, "netKeys", net_idx);
}

bool mesh_db_node_app_key_add(uint16_t unicast, uint16_t idx)
{
	json_object *jnode;

	if (!cfg || !cfg->jcfg)
		return false;

	jnode = get_node_by_unicast(unicast);
	if (!jnode)
		return false;

	return add_node_key(jnode, "appKeys", idx);
}

bool mesh_db_node_app_key_del(uint16_t unicast, uint16_t idx)
{
	json_object *jnode;

	if (!cfg || !cfg->jcfg)
		return false;

	jnode = get_node_by_unicast(unicast);
	if (!jnode)
		return false;

	return delete_key(jnode, "appKeys", idx);
}

static bool load_keys(json_object *jobj)
{
	json_object *jarray, *jentry;
	int net_idx, app_idx;
	int i, key_cnt;

	json_object_object_get_ex(jobj, "netKeys", &jarray);
	if (!jarray || json_object_get_type(jarray) != json_type_array)
		return false;

	key_cnt = json_object_array_length(jarray);
	if (key_cnt < 0)
		return false;

	for (i = 0; i < key_cnt; ++i) {
		int phase;

		jentry = json_object_array_get_idx(jarray, i);

		if (!get_int(jentry, "index", &net_idx))
			return false;

		keys_add_net_key((uint16_t) net_idx);

		if (!get_int(jentry, "phase", &phase))
			return false;

		keys_set_net_key_phase(net_idx, (uint8_t) phase, false);
	}

	json_object_object_get_ex(jobj, "appKeys", &jarray);
	if (!jarray || json_object_get_type(jarray) != json_type_array)
		return false;

	key_cnt = json_object_array_length(jarray);
	if (key_cnt < 0)
		return false;

	for (i = 0; i < key_cnt; ++i) {

		jentry = json_object_array_get_idx(jarray, i);

		if (!get_int(jentry, "boundNetKey", &net_idx))
			return false;

		if (!get_int(jentry, "index", &app_idx))
			return false;

		keys_add_app_key((uint16_t) net_idx, (uint16_t) app_idx);
	}

	return true;
}

bool mesh_db_net_key_add(uint16_t net_idx)
{
	json_object *jkey, *jarray;

	if (!cfg || !cfg->jcfg)
		return false;

	json_object_object_get_ex(cfg->jcfg, "netKeys", &jarray);
	if (!jarray || json_object_get_type(jarray) != json_type_array)
		return false;

	if (get_key_object(jarray, net_idx))
		return true;

	jkey = json_object_new_object();

	if (!write_int(jkey, "index", net_idx))
		goto fail;

	if (!write_int(jkey, "phase", KEY_REFRESH_PHASE_NONE))
		goto fail;

	json_object_array_add(jarray, jkey);

	return save_config();

fail:
	json_object_put(jkey);
	return false;
}

bool mesh_db_net_key_del(uint16_t net_idx)
{
	if (!cfg || !cfg->jcfg)
		return false;

	return delete_key(cfg->jcfg, "netKeys", net_idx);
}

bool mesh_db_net_key_phase_set(uint16_t net_idx, uint8_t phase)
{
	json_object *jval, *jarray, *jkey;

	if (!cfg || !cfg->jcfg)
		return false;

	json_object_object_get_ex(cfg->jcfg, "netKeys", &jarray);
	if (!jarray || json_object_get_type(jarray) != json_type_array)
		return false;

	jkey = get_key_object(jarray, net_idx);
	if (!jkey)
		return false;

	jval = json_object_new_int(phase);
	if (!jval)
		return false;

	json_object_object_add(jkey, "phase", jval);

	return save_config();
}

bool mesh_db_app_key_add(uint16_t net_idx, uint16_t app_idx)
{
	if (!cfg || !cfg->jcfg)
		return false;

	if (!add_app_key(cfg->jcfg, net_idx, app_idx))
		return false;

	return save_config();
}

bool mesh_db_app_key_del(uint16_t app_idx)
{
	if (!cfg || !cfg->jcfg)
		return false;

	return delete_key(cfg->jcfg, "appKeys", app_idx);
}

bool mesh_db_add_group(struct mesh_group *grp)
{
	json_object *jgroup, *jgroups, *jval;
	char buf[16];

	if (!cfg || !cfg->jcfg)
		return false;

	if (!json_object_object_get_ex(cfg->jcfg, "groups", &jgroups))
		return false;

	jgroup = json_object_new_object();
	if (!jgroup)
		return false;

	snprintf(buf, 11, "Group_%4.4x", grp->addr);
	jval = json_object_new_string(buf);
	json_object_object_add(jgroup, "name", jval);

	if (IS_VIRTUAL(grp->addr)) {
		if (!add_u8_16(jgroup, "address", grp->label))
			goto fail;
	} else {
		if (!write_uint16_hex(jgroup, "address", grp->addr))
			goto fail;
	}

	json_object_array_add(jgroups, jgroup);

	return save_config();

fail:
	json_object_put(jgroup);
	return false;
}

struct l_queue *mesh_db_load_groups(void)
{
	json_object *jgroups;
	struct l_queue *groups;
	int i, sz;

	if (!cfg || !cfg->jcfg)
		return NULL;

	if (!json_object_object_get_ex(cfg->jcfg, "groups", &jgroups)) {
		jgroups = json_object_new_array();
		if (!jgroups)
			return NULL;

		json_object_object_add(cfg->jcfg, "groups", jgroups);
	}

	groups = l_queue_new();

	sz = json_object_array_length(jgroups);

	for (i = 0; i < sz; ++i) {
		json_object *jgroup, *jval;
		struct mesh_group *grp;
		uint16_t addr, addr_len;
		const char *str;

		jgroup = json_object_array_get_idx(jgroups, i);
		if (!jgroup)
			continue;

		if (!json_object_object_get_ex(jgroup, "name", &jval))
			continue;

		str = json_object_get_string(jval);
		if (strlen(str) != 10)
			continue;

		if (sscanf(str + 6, "%04hx", &addr) != 1)
			continue;

		if (!json_object_object_get_ex(jgroup, "address", &jval))
			continue;

		str = json_object_get_string(jval);
		addr_len = strlen(str);
		if (addr_len != 4 && addr_len != 32)
			continue;

		if (addr_len == 32 && !IS_VIRTUAL(addr))
			continue;

		grp = l_new(struct mesh_group, 1);

		if (addr_len == 4)
			sscanf(str, "%04hx", &grp->addr);
		else {
			str2hex(str, 32, grp->label, 16);
			grp->addr = addr;
		}

		l_queue_insert(groups, grp, compare_group_addr, NULL);
	}

	return groups;
}

static json_object *init_elements(uint8_t num_els)
{
	json_object *jelements;
	uint8_t i;

	jelements = json_object_new_array();

	for (i = 0; i < num_els; ++i) {
		json_object *jelement, *jmods;

		jelement = json_object_new_object();

		write_int(jelement, "index", i);
		write_uint16_hex(jelement, "location", DEFAULT_LOCATION);
		jmods = json_object_new_array();
		json_object_object_add(jelement, "models", jmods);

		json_object_array_add(jelements, jelement);
	}

	return jelements;
}

bool mesh_db_add_node(uint8_t uuid[16], uint8_t num_els, uint16_t unicast,
							uint16_t net_idx)
{
	json_object *jnode;
	json_object *jelements, *jnodes, *jnetkeys, *jappkeys;

	if (!cfg || !cfg->jcfg)
		return false;

	jnode = get_node_by_uuid(cfg->jcfg, uuid);
	if (jnode) {
		l_error("Node already exists");
		return false;
	}

	jnode = json_object_new_object();
	if (!jnode)
		return false;

	if (!add_u8_16(jnode, "uuid", uuid))
		goto fail;

	jelements = init_elements(num_els);
	json_object_object_add(jnode, "elements", jelements);

	jnetkeys = json_object_new_array();
	if (!jnetkeys)
		goto fail;

	json_object_object_add(jnode, "netKeys", jnetkeys);

	if (!add_node_key(jnode, "netKeys", net_idx))
		goto fail;

	jappkeys = json_object_new_array();
	if (!jappkeys)
		goto fail;

	json_object_object_add(jnode, "appKeys", jappkeys);

	if (!write_uint16_hex(jnode, "unicastAddress", unicast))
		goto fail;

	if (!json_object_object_get_ex(cfg->jcfg, "nodes", &jnodes))
		goto fail;

	json_object_array_add(jnodes, jnode);

	if (!save_config())
		goto fail;

	return true;

fail:
	json_object_put(jnode);
	return false;
}

bool mesh_db_del_node(uint16_t unicast)
{
	json_object *jarray;
	int i, sz;

	if (!json_object_object_get_ex(cfg->jcfg, "nodes", &jarray))
		return false;

	if (!jarray || json_object_get_type(jarray) != json_type_array)
		return false;

	sz = json_object_array_length(jarray);

	for (i = 0; i < sz; ++i) {
		json_object *jentry, *jval;
		uint16_t addr;
		const char *str;

		jentry = json_object_array_get_idx(jarray, i);
		if (!json_object_object_get_ex(jentry, "unicastAddress",
								&jval))
			continue;

		str = json_object_get_string(jval);
		if (sscanf(str, "%04hx", &addr) != 1)
			continue;

		if (addr == unicast)
			break;
	}

	if (i == sz)
		return true;

	json_object_array_del_idx(jarray, i, 1);

	return save_config();
}

static json_object *init_model(uint16_t mod_id)
{
	json_object *jmod;

	jmod = json_object_new_object();

	if (!write_uint16_hex(jmod, "modelId", mod_id)) {
		json_object_put(jmod);
		return NULL;
	}

	return jmod;
}

static json_object *init_vendor_model(uint32_t mod_id)
{
	json_object *jmod;

	jmod = json_object_new_object();

	if (!write_uint32_hex(jmod, "modelId", mod_id)) {
		json_object_put(jmod);
		return NULL;
	}

	return jmod;
}

bool mesh_db_node_set_composition(uint16_t unicast, uint8_t *data, uint16_t len)
{
	uint16_t features;
	int sz, i = 0;
	json_object *jnode, *jobj, *jelements;
	uint16_t crpl;

	if (!cfg || !cfg->jcfg)
		return false;

	jnode = get_node_by_unicast(unicast);
	if (!jnode)
		return false;

	/* skip page -- We only support Page Zero */
	data++;
	len--;

	/* If "crpl" property is present, composition is already recorded */
	if (json_object_object_get_ex(jnode, "crpl", &jobj))
		return true;

	if (!write_uint16_hex(jnode, "cid", l_get_le16(&data[0])))
		return false;

	if (!write_uint16_hex(jnode, "pid", l_get_le16(&data[2])))
		return false;

	if (!write_uint16_hex(jnode, "vid", l_get_le16(&data[4])))
		return false;

	crpl = l_get_le16(&data[6]);

	features = l_get_le16(&data[8]);
	data += 10;
	len -= 10;

	jobj = json_object_object_get(jnode, "features");
	if (!jobj) {
		jobj = json_object_new_object();
		json_object_object_add(jnode, "features", jobj);
	}

	if (!(features & FEATURE_RELAY))
		write_int(jobj, "relay", 2);

	if (!(features & FEATURE_FRIEND))
		write_int(jobj, "friend", 2);

	if (!(features & FEATURE_PROXY))
		write_int(jobj, "proxy", 2);

	if (!(features & FEATURE_LPN))
		write_int(jobj, "lowPower", 2);

	jelements = json_object_object_get(jnode, "elements");
	if (!jelements)
		return false;

	sz = json_object_array_length(jelements);

	while (len) {
		json_object *jentry, *jmods;
		uint32_t mod_id;
		uint8_t m, v;

		/* Mismatch in the element count */
		if (i >= sz)
			return false;

		jentry = json_object_array_get_idx(jelements, i);

		write_int(jentry, "index", i);

		if (!write_uint16_hex(jentry, "location", l_get_le16(data)))
			return false;

		data += 2;
		len -= 2;

		m = *data++;
		v = *data++;
		len -= 2;

		jmods = json_object_object_get(jentry, "models");
		if (!jmods) {
			/* For backwards compatibility */
			jmods = json_object_new_array();
			json_object_object_add(jentry, "models", jmods);
		}

		while (len >= 2 && m--) {
			mod_id = l_get_le16(data);

			jobj = init_model(mod_id);
			if (!jobj)
				goto fail;

			json_object_array_add(jmods, jobj);
			data += 2;
			len -= 2;
		}

		while (len >= 4 && v--) {
			jobj = json_object_new_object();
			mod_id = l_get_le16(data + 2);
			mod_id = l_get_le16(data) << 16 | mod_id;

			jobj = init_vendor_model(mod_id);
			if (!jobj)
				goto fail;

			json_object_array_add(jmods, jobj);

			data += 4;
			len -= 4;
		}

		i++;
	}

	/* CRPL is written last. Will be used to check composition's presence */
	if (!write_uint16_hex(jnode, "crpl", crpl))
		goto fail;

	/* Initiate remote's composition from storage */
	if (!load_composition(jnode, unicast))
		goto fail;

	return save_config();

fail:
	/* Reset elements array */
	json_object_object_del(jnode, "elements");
	init_elements(sz);

	return false;
}

bool mesh_db_get_token(uint8_t token[8])
{
	if (!cfg || !cfg->jcfg)
		return false;

	memcpy(token, cfg->token, 8);

	return true;
}

bool mesh_db_get_addr_range(uint16_t *low, uint16_t *high)
{
	json_object *jlow, *jhigh;
	const char *str;

	if (!cfg || !cfg->jcfg)
		return false;

	if (!json_object_object_get_ex(cfg->jcfg, "low", &jlow) ||
			!json_object_object_get_ex(cfg->jcfg, "high", &jhigh))
		return false;

	str = json_object_get_string(jlow);
	if (sscanf(str, "%04hx", low) != 1)
		return false;

	str = json_object_get_string(jhigh);
	if (sscanf(str, "%04hx", high) != 1)
		return false;

	return true;
}

bool mesh_db_set_addr_range(uint16_t low, uint16_t high)
{
	if (!cfg || !cfg->jcfg)
		return false;

	if (!write_uint16_hex(cfg->jcfg, "low", low))
		return false;

	if (!write_uint16_hex(cfg->jcfg, "high", high))
		return false;

	return save_config();
}

uint32_t mesh_db_get_iv_index(void)
{
	int ivi;

	if (!cfg || !cfg->jcfg)
		return 0;

	if (!get_int(cfg->jcfg, "ivIndex", &ivi))
		return 0;

	return (uint32_t) ivi;
}

bool mesh_db_set_iv_index(uint32_t ivi)
{
	if (!cfg || !cfg->jcfg)
		return false;

	write_int(cfg->jcfg, "ivIndex", ivi);

	return save_config();
}

static int get_blacklisted_by_iv_index(json_object *jarray, uint32_t iv_index)
{
	int i, cnt;

	cnt = json_object_array_length(jarray);

	for (i = 0; i < cnt; i++) {
		json_object *jentry;
		int index;

		jentry = json_object_array_get_idx(jarray, i);

		if (!get_int(jentry, "ivIndex", &index))
			continue;

		if (iv_index == (uint32_t)index)
			return i;
	}

	return -1;
}

static bool load_blacklisted(json_object *jobj)
{
	json_object *jarray;
	int i, cnt;

	json_object_object_get_ex(jobj, "blacklistedAddresses", &jarray);
	if (!jarray || json_object_get_type(jarray) != json_type_array)
		return true;

	cnt = json_object_array_length(jarray);

	for (i = 0; i < cnt; i++) {
		json_object *jaddrs, *jentry, *jval;
		int iv_index, addr_cnt, j;

		jentry = json_object_array_get_idx(jarray, i);

		if (!get_int(jentry, "ivIndex", &iv_index))
			return false;

		if (!json_object_object_get_ex(jentry, "addresses",
								&jaddrs))
			return false;

		addr_cnt = json_object_array_length(jaddrs);

		for (j = 0; j < addr_cnt; j++) {
			const char *str;
			uint16_t unicast;

			jval = json_object_array_get_idx(jaddrs, j);
			str = json_object_get_string(jval);

			if (sscanf(str, "%04hx", &unicast) != 1)
				return false;

			remote_add_blacklisted_address(unicast, iv_index,
								false);
		}
	}

	return true;
}

bool mesh_db_add_blacklisted_addr(uint16_t unicast, uint32_t iv_index)
{
	json_object *jarray, *jobj, *jaddrs, *jstring;
	int idx;
	char buf[5];

	if (!cfg || !cfg->jcfg)
		return false;

	json_object_object_get_ex(cfg->jcfg, "blacklistedAddresses", &jarray);
	if (!jarray) {
		jarray = json_object_new_array();
		json_object_object_add(cfg->jcfg, "blacklistedAddresses",
									jarray);
	}

	idx = get_blacklisted_by_iv_index(jarray, iv_index);

	if (idx < 0) {
		jobj = json_object_new_object();

		if (!write_int(jobj, "ivIndex", iv_index))
			goto fail;

		jaddrs = json_object_new_array();
		json_object_object_add(jobj, "addresses", jaddrs);

	} else {
		jobj = json_object_array_get_idx(jarray, idx);
	}

	json_object_object_get_ex(jobj, "addresses", &jaddrs);

	snprintf(buf, 5, "%4.4x", unicast);
	jstring = json_object_new_string(buf);
	if (!jstring)
		goto fail;

	json_object_array_add(jaddrs, jstring);

	if (idx < 0)
		json_object_array_add(jarray, jobj);

	return save_config();

fail:
	json_object_put(jobj);
	return false;
}

bool mesh_db_clear_blacklisted(uint32_t iv_index)
{
	json_object *jarray;
	int idx;

	if (!cfg || !cfg->jcfg)
		return false;

	json_object_object_get_ex(cfg->jcfg, "blacklistedAddresses", &jarray);
	if (!jarray || json_object_get_type(jarray) != json_type_array)
		return false;

	idx = get_blacklisted_by_iv_index(jarray, iv_index);
	if (idx < 0)
		return true;

	json_object_array_del_idx(jarray, idx, 1);

	return save_config();
}

bool mesh_db_create(const char *fname, const uint8_t token[8],
							const char *mesh_name)
{
	json_object *jcfg, *jarray;
	uint8_t uuid[16];

	if (cfg)
		return false;

	if (!fname)
		return false;

	jcfg = json_object_new_object();
	if (!jcfg)
		return false;

	cfg = l_new(struct mesh_db, 1);
	cfg->jcfg = jcfg;
	cfg->cfg_fname = l_strdup(fname);
	memcpy(cfg->token, token, 8);

	if (!add_u8_8(jcfg, "token", token))
		goto fail;

	l_uuid_v4(uuid);

	if (!add_u8_16(jcfg, "uuid", uuid))
		goto fail;

	if (mesh_name && !add_string(jcfg, "name", mesh_name))
		goto fail;

	jarray = json_object_new_array();
	if (!jarray)
		goto fail;

	json_object_object_add(jcfg, "nodes", jarray);

	jarray = json_object_new_array();
	if (!jarray)
		goto fail;

	json_object_object_add(jcfg, "netKeys", jarray);

	jarray = json_object_new_array();
	if (!jarray)
		goto fail;

	json_object_object_add(jcfg, "appKeys", jarray);

	jarray = json_object_new_array();
	if (!jarray)
		goto fail;

	json_object_object_add(jcfg, "blacklistedAddresses", jarray);

	write_int(jcfg, "ivIndex", 0);

	if (!save_config())
		goto fail;

	return true;

fail:
	release_config();

	return false;
}

bool mesh_db_load(const char *fname)
{
	int fd;
	char *str;
	struct stat st;
	ssize_t sz;
	json_object *jcfg;

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
		return false;
	}

	jcfg = json_tokener_parse(str);

	close(fd);
	l_free(str);

	if (!jcfg)
		return false;

	cfg = l_new(struct mesh_db, 1);

	cfg->jcfg = jcfg;
	cfg->cfg_fname = l_strdup(fname);

	if (!get_token(jcfg, cfg->token)) {
		l_error("Configuration file missing token");
		goto fail;
	}

	if (!load_keys(jcfg))
		goto fail;

	load_remotes(jcfg);

	load_blacklisted(jcfg);

	return true;
fail:
	release_config();

	return false;
}

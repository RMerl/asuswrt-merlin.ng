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

#include <errno.h>
#include <fcntl.h>
#include <glib.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include <json-c/json.h>
#include <sys/stat.h>

#include <glib.h>

#include "src/shared/util.h"
#include "src/shared/shell.h"

#include "tools/mesh-gatt/mesh-net.h"
#include "tools/mesh-gatt/crypto.h"
#include "tools/mesh-gatt/keys.h"
#include "tools/mesh-gatt/net.h"
#include "tools/mesh-gatt/node.h"
#include "tools/mesh-gatt/util.h"
#include "tools/mesh-gatt/prov-db.h"

#define CHECK_KEY_IDX_RANGE(x) (((x) >= 0) && ((x) <= 4095))

static const char *prov_filename;
static const char *local_filename;

static char* prov_file_read(const char *filename)
{
	int fd;
	char *str;
	struct stat st;
	ssize_t sz;

	if (!filename)
		return NULL;

	fd = open(filename,O_RDONLY);
	if (!fd)
		return NULL;

	if (fstat(fd, &st) == -1) {
		close(fd);
		return NULL;
	}

	str = (char *) g_malloc0(st.st_size + 1);
	if (!str) {
		close(fd);
		return NULL;
	}

	sz = read(fd, str, st.st_size);
	if (sz != st.st_size)
		bt_shell_printf("Incomplete read: %d vs %d\n", (int)sz,
							(int)(st.st_size));

	close(fd);

	return str;
}

static void prov_file_write(json_object *jmain, bool local)
{
	FILE *outfile;
	const char *out_str;
	const char *out_filename;

	if (local)
		out_filename = local_filename;
	else
		out_filename = prov_filename;

	outfile = fopen(out_filename, "wr");
	if (!outfile) {
		bt_shell_printf("Failed to open file %s for writing\n", out_filename);
		return;
	}

	out_str = json_object_to_json_string_ext(jmain,
						JSON_C_TO_STRING_PRETTY);

	fwrite(out_str, sizeof(char), strlen(out_str), outfile);
	fclose(outfile);
}

static void put_uint16(json_object *jobject, const char *desc, uint16_t value)
{
	json_object *jstring;
	char buf[5];

	snprintf(buf, 5, "%4.4x", value);
	jstring = json_object_new_string(buf);
	json_object_object_add(jobject, desc, jstring);
}

static void put_uint32(json_object *jobject, const char *desc, uint32_t value)
{
	json_object *jstring;
	char buf[9];

	snprintf(buf, 9, "%8.8x", value);
	jstring = json_object_new_string(buf);
	json_object_object_add(jobject, desc, jstring);
}

static void put_uint16_array_entry(json_object *jarray, uint16_t value)
{
	json_object *jstring;
	char buf[5];

	snprintf(buf, 5, "%4.4x", value);
	jstring = json_object_new_string(buf);
	json_object_array_add(jarray, jstring);
}

static void put_uint32_array_entry(json_object *jarray, uint32_t value)
{
	json_object *jstring;
	char buf[9];

	snprintf(buf, 9, "%8.8x", value);
	jstring = json_object_new_string(buf);
	json_object_array_add(jarray, jstring);
}

static void put_uint16_list(json_object *jarray, GList *list)
{
	GList *l;

	if (!list)
		return;

	for (l = list; l; l = l->next) {
		uint32_t ivalue = GPOINTER_TO_UINT(l->data);
		put_uint16_array_entry(jarray, ivalue);
	}
}

static void add_node_idxs(json_object *jnode, const char *desc,
				GList *idxs)
{
	json_object *jarray;

	jarray = json_object_new_array();

	put_uint16_list(jarray, idxs);

	json_object_object_add(jnode, desc, jarray);
}

static bool parse_unicast_range(json_object *jobject)
{
	int cnt;
	int i;

	cnt = json_object_array_length(jobject);

	for (i = 0; i < cnt; ++i) {
		json_object *jrange;
		json_object *jvalue;
		uint16_t low, high;
		char *str;

		jrange = json_object_array_get_idx(jobject, i);
		json_object_object_get_ex(jrange, "lowAddress", &jvalue);
		str = (char *)json_object_get_string(jvalue);
		if (sscanf(str, "%04hx", &low) != 1)
			return false;

		json_object_object_get_ex(jrange, "highAddress", &jvalue);
		str = (char *)json_object_get_string(jvalue);
		if (sscanf(str, "%04hx", &high) != 1)
			return false;

		if(high < low)
			return false;

		net_add_address_pool(low, high);
	}
	return true;
}

static int parse_node_keys(struct mesh_node *node, json_object *jidxs,
				bool is_app_key)
{
	int idx_cnt;
	int i;

	idx_cnt = json_object_array_length(jidxs);
	for (i = 0; i < idx_cnt; ++i) {
		int idx;
		json_object *jvalue;

		jvalue = json_object_array_get_idx(jidxs, i);
		if (!jvalue)
			break;
		idx = json_object_get_int(jvalue);
		if (!CHECK_KEY_IDX_RANGE(idx))
			break;

		if (is_app_key)
			node_app_key_add(node, idx);
		else
			node_net_key_add(node, idx);
	}

	return i;
}

static bool parse_composition_models(struct mesh_node *node, int index,
					json_object *jmodels)
{
	int model_cnt;
	int i;

	model_cnt = json_object_array_length(jmodels);

	for (i = 0; i < model_cnt; ++i) {
		json_object *jmodel;
		char *str;
		uint32_t model_id;
		int len;

		jmodel = json_object_array_get_idx(jmodels, i);
		str = (char *)json_object_get_string(jmodel);
		len = strlen(str);

		if (len != 4 && len != 8)
			return false;

		if (sscanf(str, "%08x", &model_id) != 1)
			return false;
		if (len == 4)
			model_id += 0xffff0000;

		node_set_model(node, index, model_id);
	}

	return true;
}

static bool parse_composition_elements(struct mesh_node *node,
					json_object *jelements)
{
	int el_cnt;
	int i;

	el_cnt = json_object_array_length(jelements);
	node_set_num_elements(node, el_cnt);

	for (i = 0; i < el_cnt; ++i) {
		json_object *jelement;
		json_object *jmodels;
		json_object *jvalue;
		int index;

		jelement = json_object_array_get_idx(jelements, i);
		json_object_object_get_ex(jelement, "elementIndex", &jvalue);
		if (jvalue) {
			index = json_object_get_int(jvalue);
			if (index >= el_cnt) {
				return false;
			}
		} else
			return false;

		if (!node_set_element(node, index))
			return false;

		json_object_object_get_ex(jelement, "models", &jmodels);
		if (!jmodels)
			continue;

		if(!parse_composition_models(node, index, jmodels))
			return false;
	}
	return true;
}

static bool parse_model_pub(struct mesh_node *node, int ele_idx,
				uint32_t model_id, json_object *jpub)
{
	json_object *jvalue;
	struct mesh_publication pub;
	char *str;

	memset(&pub, 0, sizeof(struct mesh_publication));

	/* Read only required fields */
	json_object_object_get_ex(jpub, "address", &jvalue);
	if (!jvalue)
		return false;

	str = (char *)json_object_get_string(jvalue);
	if (sscanf(str, "%04hx", &pub.u.addr16) != 1)
		return false;

	json_object_object_get_ex(jpub, "index", &jvalue);
	if (!jvalue)
		return false;

	str = (char *)json_object_get_string(jvalue);
	if (sscanf(str, "%04hx", &pub.app_idx) != 1)
		return false;


	json_object_object_get_ex(jpub, "ttl", &jvalue);
	pub.ttl = json_object_get_int(jvalue);

	if (!node_model_pub_set(node, ele_idx, model_id, &pub))
			return false;

	return true;
}

static bool parse_bindings(struct mesh_node *node, int ele_idx,
				uint32_t model_id, json_object *jbindings)
{
	int cnt;
	int i;

	cnt = json_object_array_length(jbindings);

	for (i = 0; i < cnt; ++i) {
		int key_idx;
		json_object *jvalue;

		jvalue = json_object_array_get_idx(jbindings, i);
		if (!jvalue)
			return true;

		key_idx = json_object_get_int(jvalue);
		if (!CHECK_KEY_IDX_RANGE(key_idx))
			return false;

		if (!node_add_binding(node, ele_idx, model_id, key_idx))
			return false;
	}

	return true;
}

static json_object* find_configured_model(struct mesh_node *node, int ele_idx,
				       json_object *jmodels, uint32_t target_id)
{
	int model_cnt;
	int i;

	model_cnt = json_object_array_length(jmodels);

	for (i = 0; i < model_cnt; ++i) {
		json_object *jmodel;
		json_object *jvalue;
		char *str;
		int len;
		uint32_t model_id;

		jmodel = json_object_array_get_idx(jmodels, i);

		json_object_object_get_ex(jmodel, "modelId", &jvalue);
		str = (char *)json_object_get_string(jvalue);

		len = strlen(str);

		if (len != 4 && len != 8)
			return NULL;

		if (sscanf(str, "%08x", &model_id) != 1)
			return NULL;

		if (len == 4)
			model_id += 0xffff0000;

		if (model_id == target_id)
			return jmodel;
	}

	return NULL;
}

static bool parse_subscriptions(struct mesh_node *node, int ele_idx,
				uint32_t model_id, json_object *jsubscriptions)

{
	int cnt;
	int i;
	int addr;

	cnt = json_object_array_length(jsubscriptions);

	for (i = 0; i < cnt; ++i) {
		char *str;
		json_object *jsubscription;

		jsubscription = json_object_array_get_idx(jsubscriptions, i);
		if (!jsubscription)
			return false;

		str = (char *)json_object_get_string(jsubscription);

		if (sscanf(str, "%04x", &addr) != 1)
			return false;

		if (!node_add_subscription(node, ele_idx, model_id, addr))
			return false;
	}

	return true;
}

static bool parse_configuration_models(struct mesh_node *node, int ele_idx,
							json_object *jmodels)
{
	int model_cnt;
	int i;

	model_cnt = json_object_array_length(jmodels);

	for (i = 0; i < model_cnt; ++i) {
		json_object *jmodel;
		json_object *jvalue;
		json_object *jarray;
		char *str;
		int len;
		uint32_t model_id;

		jmodel = json_object_array_get_idx(jmodels, i);

		json_object_object_get_ex(jmodel, "modelId", &jvalue);
		str = (char *)json_object_get_string(jvalue);

		len = strlen(str);

		if (len != 4 && len != 8)
			return false;

		if (sscanf(str, "%08x", &model_id) != 1)
			return false;
		if (len == 4)
			model_id += 0xffff0000;

		json_object_object_get_ex(jmodel, "bind", &jarray);

		if (jarray && !parse_bindings(node, ele_idx, model_id, jarray))
			return false;

		json_object_object_get_ex(jmodel, "subscribe", &jarray);
		if (jarray && !parse_subscriptions(node, ele_idx, model_id, jarray))
			return false;

		json_object_object_get_ex(jmodel, "publish", &jvalue);
		if (jvalue && !parse_model_pub(node, ele_idx, model_id, jvalue))
			return false;
	}

	return true;
}

static bool parse_configuration_elements(struct mesh_node *node,
				json_object *jelements, bool local)
{
	int el_cnt;
	int i;

	el_cnt = json_object_array_length(jelements);
	node_set_num_elements(node, el_cnt);

	for (i = 0; i < el_cnt; ++i) {
		json_object *jelement;
		json_object *jmodels;
		json_object *jvalue;
		int index;
		uint16_t addr;

		jelement = json_object_array_get_idx(jelements, i);
		json_object_object_get_ex(jelement, "elementIndex", &jvalue);
		if (jvalue) {
			index = json_object_get_int(jvalue);
			if (index >= el_cnt) {
				return false;
			}
		} else
			return false;

		if (index == 0) {
			char *str;

			json_object_object_get_ex(jelement, "unicastAddress",
							&jvalue);
			str = (char *)json_object_get_string(jvalue);
			if (sscanf(str, "%04hx", &addr) != 1)
				return false;

			if (!local && !net_reserve_address_range(addr, el_cnt))
				return false;

			node_set_primary(node, addr);
		}

		json_object_object_get_ex(jelement, "models", &jmodels);
		if (!jmodels)
			continue;

		if(!parse_configuration_models(node, index, jmodels))
			return false;
	}
	return true;
}

static void add_key(json_object *jobject, const char *desc, uint8_t* key)
{
	json_object *jstring;
	char hexstr[33];

	hex2str(key, 16, hexstr, 33);
	jstring = json_object_new_string(hexstr);
	json_object_object_add(jobject, desc, jstring);
}

static json_object *find_node_by_primary(json_object *jmain, uint16_t primary)
{
	json_object *jarray;
	int i, len;

	json_object_object_get_ex(jmain, "nodes", &jarray);

	if (!jarray)
		return NULL;
	len = json_object_array_length(jarray);

	for (i = 0; i < len; ++i) {
		json_object *jnode;
		json_object *jconfig;
		json_object *jelements;
		json_object *jelement;
		json_object *jvalue;
		char *str;
		uint16_t addr;

		jnode = json_object_array_get_idx(jarray, i);
		if (!jnode)
			return NULL;

		json_object_object_get_ex(jnode, "configuration", &jconfig);
		if (!jconfig)
			return NULL;

		json_object_object_get_ex(jconfig, "elements", &jelements);
		if (!jelements)
			return NULL;

		jelement = json_object_array_get_idx(jelements, 0);
		if (!jelement)
			return NULL;

		json_object_object_get_ex(jelement, "unicastAddress",
								&jvalue);
		str = (char *)json_object_get_string(jvalue);
		if (sscanf(str, "%04hx", &addr) != 1)
				return NULL;

		if (addr == primary)
			return jnode;
	}

	return NULL;

}

void prov_db_print_node_composition(struct mesh_node *node)
{
	char *in_str;
	const char *comp_str;
	json_object *jmain;
	json_object *jnode;
	json_object *jcomp;
	uint16_t primary = node_get_primary(node);
	const char *filename;
	bool res = false;

	if (!node || !node_get_composition(node))
		return;

	if (node == node_get_local_node())
		filename = local_filename;
	else
		filename = prov_filename;

	in_str = prov_file_read(filename);
	if (!in_str)
		return;

	jmain = json_tokener_parse(in_str);
	if (!jmain)
		goto done;

	jnode = find_node_by_primary(jmain, primary);
	if (!jnode)
		goto done;

	json_object_object_get_ex(jnode, "composition", &jcomp);
	if (!jcomp)
		goto done;

	comp_str = json_object_to_json_string_ext(jcomp,
						JSON_C_TO_STRING_PRETTY);

	res = true;

done:
	if (res)
		bt_shell_printf("\tComposition data for node %4.4x %s\n",
							primary, comp_str);
	else
		bt_shell_printf("\tComposition data for node %4.4x not present\n",
								primary);
	g_free(in_str);

	if (jmain)
		json_object_put(jmain);
}

bool prov_db_add_node_composition(struct mesh_node *node, uint8_t *data,
								uint16_t len)
{
	char *in_str;
	json_object *jmain;
	json_object *jnode;
	json_object *jcomp;
	json_object *jbool;
	json_object *jfeatures;
	json_object *jelements;
	struct mesh_node_composition *comp;
	uint8_t num_ele;
	int i;
	uint16_t primary = node_get_primary(node);
	bool res = NULL;

	comp = node_get_composition(node);
	if (!comp)
		return false;

	in_str = prov_file_read(prov_filename);
	if (!in_str)
		return false;

	jmain = json_tokener_parse(in_str);
	if (!jmain)
		goto done;

	jnode = find_node_by_primary(jmain, primary);
	if (!jnode)
		goto done;

	jcomp = json_object_new_object();

	put_uint16(jcomp, "cid", comp->cid);
	put_uint16(jcomp, "pid", comp->pid);
	put_uint16(jcomp, "vid", comp->vid);
	put_uint16(jcomp, "crpl", comp->crpl);

	jfeatures = json_object_new_object();
	jbool = json_object_new_boolean(comp->relay);
	json_object_object_add(jfeatures, "relay", jbool);
	jbool = json_object_new_boolean(comp->proxy);
	json_object_object_add(jfeatures, "proxy", jbool);
	jbool = json_object_new_boolean(comp->friend);
	json_object_object_add(jfeatures, "friend", jbool);
	jbool = json_object_new_boolean(comp->lpn);
	json_object_object_add(jfeatures, "lpn", jbool);
	json_object_object_add(jcomp, "features", jfeatures);

	data += 11;
	len -= 11;

	num_ele =  node_get_num_elements(node);

	jelements = json_object_new_array();

	for (i = 0; i < num_ele; ++i) {
		json_object *jelement;
		json_object *jmodels;
		json_object *jint;
		uint32_t mod_id;
		uint16_t vendor_id;
		uint8_t m, v;

		jelement = json_object_new_object();

		/* Element Index */
		jint = json_object_new_int(i);
		json_object_object_add(jelement, "elementIndex", jint);

		/* Location */
		put_uint16(jelement, "location", get_le16(data));
		data += 2;
		m = *data++;
		v = *data++;
		len -= 4;

		/* Models */
		jmodels = json_object_new_array();
		while (len >= 2 && m--) {
			mod_id = get_le16(data);
			data += 2;
			len -= 2;
			put_uint16_array_entry(jmodels, (uint16_t) mod_id);
		}

		while (len >= 4 && v--) {
			mod_id = get_le16(data + 2);
			vendor_id = get_le16(data);
			mod_id |= (vendor_id << 16);
			data += 4;
			len -= 4;
			put_uint32_array_entry(jmodels, mod_id);
		}

		json_object_object_add(jelement, "models", jmodels);
		json_object_array_add(jelements, jelement);
	}

	json_object_object_add(jcomp, "elements", jelements);

	json_object_object_add(jnode, "composition", jcomp);

	prov_file_write(jmain, false);

	res = true;;
done:

	g_free(in_str);

	if(jmain)
		json_object_put(jmain);

	return res;
}

bool prov_db_node_set_ttl(struct mesh_node *node, uint8_t ttl)
{
	char *in_str;
	json_object *jmain;
	json_object *jnode;
	json_object *jconfig;
	json_object *jvalue;
	uint16_t primary = node_get_primary(node);
	const char *filename;
	bool local = node == node_get_local_node();
	bool res = false;

	if (local)
		filename = local_filename;
	else
		filename = prov_filename;

	in_str = prov_file_read(filename);
	if (!in_str)
		return false;

	jmain = json_tokener_parse(in_str);
	if (!jmain)
		goto done;

	if (local)
		json_object_object_get_ex(jmain, "node", &jnode);
	else
		jnode = find_node_by_primary(jmain, primary);

	if (!jnode)
		goto done;

	json_object_object_get_ex(jnode, "configuration", &jconfig);
	if (!jconfig)
		goto done;

	json_object_object_del(jconfig, "defaultTTL");

	jvalue = json_object_new_int(ttl);
	json_object_object_add(jconfig, "defaultTTL", jvalue);

	prov_file_write(jmain, local);

	res = true;
done:

	g_free(in_str);

	if(jmain)
		json_object_put(jmain);

	return res;

}

static void set_local_iv_index(json_object *jobj, uint32_t idx, bool update)
{
	json_object *jvalue;

	json_object_object_del(jobj, "IVindex");
	jvalue = json_object_new_int(idx);
	json_object_object_add(jobj, "IVindex", jvalue);

	json_object_object_del(jobj, "IVupdate");
	jvalue = json_object_new_int((update) ? 1 : 0);
	json_object_object_add(jobj, "IVupdate", jvalue);

}

bool prov_db_local_set_iv_index(uint32_t iv_index, bool update, bool prov)
{
	char *in_str;
	json_object *jmain;
	json_object *jnode;
	bool res = false;

	in_str = prov_file_read(local_filename);
	if (!in_str)
		return false;

	jmain = json_tokener_parse(in_str);
	if (!jmain)
		goto done;

	json_object_object_get_ex(jmain, "node", &jnode);
	set_local_iv_index(jnode, iv_index, update);
	prov_file_write(jmain, true);

	g_free(in_str);
	json_object_put(jmain);

	/* If provisioner, save to global DB as well */
	if (prov) {
		in_str = prov_file_read(prov_filename);
		if (!in_str)
			return false;

		jmain = json_tokener_parse(in_str);
		if (!jmain)
			goto done;

		set_local_iv_index(jmain, iv_index, update);
		prov_file_write(jmain, false);
	}

	res = true;
done:

	g_free(in_str);

	if(jmain)
		json_object_put(jmain);

	return res;

}

bool prov_db_local_set_seq_num(uint32_t seq_num)
{
	char *in_str;
	json_object *jmain;
	json_object *jnode;
	json_object *jvalue;
	bool res = false;

	in_str = prov_file_read(local_filename);
	if (!in_str)
		return false;

	jmain = json_tokener_parse(in_str);
	if (!jmain)
		goto done;

	json_object_object_get_ex(jmain, "node", &jnode);

	json_object_object_del(jnode, "sequenceNumber");
	jvalue = json_object_new_int(seq_num);
	json_object_object_add(jnode, "sequenceNumber", jvalue);

	prov_file_write(jmain, true);

	res = true;
done:

	g_free(in_str);

	if(jmain)
		json_object_put(jmain);

	return res;
}

bool prov_db_node_set_iv_seq(struct mesh_node *node, uint32_t iv, uint32_t seq)
{
	char *in_str;
	json_object *jmain;
	json_object *jnode;
	json_object *jvalue;
	uint16_t primary = node_get_primary(node);
	bool res = false;

	in_str = prov_file_read(prov_filename);
	if (!in_str)
		return false;

	jmain = json_tokener_parse(in_str);
	if (!jmain)
		goto done;

	jnode = find_node_by_primary(jmain, primary);
	if (!jnode)
		goto done;

	json_object_object_del(jnode, "IVindex");

	jvalue = json_object_new_int(iv);
	json_object_object_add(jnode, "IVindex", jvalue);

	json_object_object_del(jnode, "sequenceNumber");

	jvalue = json_object_new_int(seq);
	json_object_object_add(jnode, "sequenceNumber", jvalue);

	prov_file_write(jmain, false);

	res = true;
done:

	g_free(in_str);

	if(jmain)
		json_object_put(jmain);

	return res;

}

bool prov_db_node_keys(struct mesh_node *node, GList *idxs, const char *desc)
{
	char *in_str;
	json_object *jmain;
	json_object *jnode;
	json_object *jconfig;
	json_object *jidxs;
	uint16_t primary = node_get_primary(node);
	const char *filename;
	bool local = (node == node_get_local_node());
	bool res = false;

	if (local)
		filename = local_filename;
	else
		filename = prov_filename;

	in_str = prov_file_read(filename);
	if (!in_str)
		return false;

	jmain = json_tokener_parse(in_str);
	if (!jmain)
		goto done;

	jnode = find_node_by_primary(jmain, primary);
	if (!jnode)
		goto done;

	json_object_object_get_ex(jnode, "configuration", &jconfig);
	if (!jconfig)
		goto done;

	json_object_object_del(jconfig, desc);

	if (idxs) {
		jidxs = json_object_new_array();
		put_uint16_list(jidxs, idxs);
		json_object_object_add(jconfig, desc, jidxs);
	}

	prov_file_write(jmain, local);

	res = true;
done:

	g_free(in_str);

	if(jmain)
		json_object_put(jmain);

	return res;

}

static json_object *get_jmodel_obj(struct mesh_node *node, uint8_t ele_idx,
					uint32_t model_id, json_object **jmain)
{
	char *in_str;
	json_object *jnode;
	json_object *jconfig;
	json_object *jelements, *jelement;
	json_object *jmodels, *jmodel = NULL;
	uint16_t primary = node_get_primary(node);
	const char *filename;
	bool local = (node == node_get_local_node());

	if (local)
		filename = local_filename;
	else
		filename = prov_filename;

	in_str = prov_file_read(filename);
	if (!in_str)
		return NULL;

	*jmain = json_tokener_parse(in_str);
	if (!(*jmain))
		goto done;

	if (local)
		json_object_object_get_ex(*jmain, "node", &jnode);
	else
		jnode = find_node_by_primary(*jmain, primary);

	if (!jnode)
		goto done;

	/* Configuration is mandatory for nodes in provisioning database */
	json_object_object_get_ex(jnode, "configuration", &jconfig);
	if (!jconfig)
		goto done;

	json_object_object_get_ex(jconfig, "elements", &jelements);
	if (!jelements) {
		goto done;
	}

	jelement = json_object_array_get_idx(jelements, ele_idx);
	if (!jelement) {
		goto done;
	}

	json_object_object_get_ex(jelement, "models", &jmodels);

	if (!jmodels)  {
		jmodels = json_object_new_array();
		json_object_object_add(jelement, "models", jmodels);
	} else {
		jmodel = find_configured_model(node, ele_idx, jmodels,
								model_id);
	}

	if (!jmodel) {
		jmodel = json_object_new_object();

		if ((model_id & 0xffff0000) == 0xffff0000)
			put_uint16(jmodel, "modelId", model_id & 0xffff);
		else
			put_uint32(jmodel, "modelId", model_id);

		json_object_array_add(jmodels, jmodel);
	}

done:

	g_free(in_str);

	if(!jmodel && *jmain)
		json_object_put(*jmain);

	return jmodel;

}

bool prov_db_add_binding(struct mesh_node *node, uint8_t ele_idx,
			uint32_t model_id, uint16_t app_idx)
{
	bool local = (node == node_get_local_node());
	json_object *jbindings = NULL;
	json_object *jmodel;
	json_object *jvalue;
	json_object *jmain;

	jmodel = get_jmodel_obj(node, ele_idx, model_id, &jmain);
	if (!jmodel)
		return false;

	json_object_object_get_ex(jmodel, "bind", &jbindings);

	if (!jbindings) {
		jbindings = json_object_new_array();
		json_object_object_add(jmodel, "bind", jbindings);
	}

	jvalue = json_object_new_int(app_idx);
	json_object_array_add(jbindings, jvalue);

	prov_file_write(jmain, local);

	json_object_put(jmain);

	return true;
}

bool prov_db_add_subscription(struct mesh_node *node, uint8_t ele_idx,
			      uint32_t model_id, uint16_t addr)
{
	bool local = (node == node_get_local_node());
	json_object *jsubscriptions = NULL;
	json_object *jmodel;
	json_object *jmain;

	jmodel = get_jmodel_obj(node, ele_idx, model_id, &jmain);
	if (!jmodel)
		return false;

	json_object_object_get_ex(jmodel, "subscribe", &jsubscriptions);

	if (!jsubscriptions) {
		jsubscriptions = json_object_new_array();
		json_object_object_add(jmodel, "subscribe", jsubscriptions);
	}

	put_uint16_array_entry(jsubscriptions, addr);

	prov_file_write(jmain, local);

	json_object_put(jmain);

	return true;
}

bool prov_db_node_set_model_pub(struct mesh_node *node, uint8_t ele_idx,
							uint32_t model_id,
						struct mesh_publication *pub)
{
	json_object *jmain;
	json_object *jmodel;
	json_object *jpub;
	json_object *jvalue;
	bool local = (node == node_get_local_node());

	jmodel = get_jmodel_obj(node, ele_idx, model_id, &jmain);

	if (!jmodel)
		return false;

	json_object_object_del(jmodel, "publish");
	if (!pub)
		goto done;

	jpub = json_object_new_object();

	/* Save only required fields */
	put_uint16(jpub, "address", pub->u.addr16);
	put_uint16(jpub, "index", pub->app_idx);
	jvalue = json_object_new_int(pub->ttl);
	json_object_object_add(jpub, "ttl", jvalue);

	json_object_object_add(jmodel, "publish", jpub);

done:
	prov_file_write(jmain, local);

	json_object_put(jmain);

	return true;
}

bool prov_db_add_new_node(struct mesh_node *node)
{
	char *in_str;
	json_object *jmain;
	json_object *jarray;
	json_object *jnode;
	json_object *jconfig;
	json_object *jelements;
	uint8_t num_ele;
	uint16_t primary;
	int i;
	bool first_node;
	bool res = false;

	in_str = prov_file_read(prov_filename);
	if (!in_str)
		return false;

	jmain = json_tokener_parse(in_str);
	if (!jmain)
		goto done;
	json_object_object_get_ex(jmain, "nodes", &jarray);

	if (!jarray) {
		jarray = json_object_new_array();
		first_node = true;
	} else
		first_node = false;

	jnode = json_object_new_object();

	/* Device key */
	add_key(jnode, "deviceKey", node_get_device_key(node));

	/* Net key */
	jconfig = json_object_new_object();
	add_node_idxs(jconfig, "netKeys", node_get_net_keys(node));

	num_ele = node_get_num_elements(node);
	if (num_ele == 0)
		goto done;

	jelements = json_object_new_array();

	primary = node_get_primary(node);
	if (IS_UNASSIGNED(primary))
		goto done;

	for (i = 0; i < num_ele; ++i) {
		json_object *jelement;
		json_object *jint;

		jelement = json_object_new_object();

		/* Element Index */
		jint = json_object_new_int(i);
		json_object_object_add(jelement, "elementIndex", jint);

		/* Unicast */
		put_uint16(jelement, "unicastAddress", primary + i);

		json_object_array_add(jelements, jelement);
	}

	json_object_object_add(jconfig, "elements", jelements);

	json_object_object_add(jnode, "configuration", jconfig);

	json_object_array_add(jarray, jnode);

	if (first_node)
		json_object_object_add(jmain, "nodes", jarray);

	prov_file_write(jmain, false);

	res = true;
done:

	g_free(in_str);

	if (jmain)
		json_object_put(jmain);

	return res;
}

static bool parse_node_composition(struct mesh_node *node, json_object *jcomp)
{
	json_object *jvalue;
	json_object *jelements;
	json_object *jfeatures;
	json_bool enable;
	char *str;
	struct mesh_node_composition comp;

	json_object_object_get_ex(jcomp, "cid", &jvalue);
	if (!jvalue)
		return false;

	str = (char *)json_object_get_string(jvalue);

	if (sscanf(str, "%04hx", &comp.cid) != 1)
		return false;

	json_object_object_get_ex(jcomp, "pid", &jvalue);
	if (!jvalue)
		return false;

	str = (char *)json_object_get_string(jvalue);

	if (sscanf(str, "%04hx", &comp.pid) != 1)
		return false;

	json_object_object_get_ex(jcomp, "vid", &jvalue);
	if (!jvalue)
		return false;

	str = (char *)json_object_get_string(jvalue);

	if (sscanf(str, "%04hx", &comp.vid) != 1)
		return false;

	json_object_object_get_ex(jcomp, "crpl", &jvalue);
	if (!jvalue)
		return false;

	str = (char *)json_object_get_string(jvalue);

	if (sscanf(str, "%04hx", &comp.crpl) != 1)
		return false;

	/* Extract features */

	json_object_object_get_ex(jcomp, "features", &jfeatures);
	if (!jfeatures)
		return false;

	json_object_object_get_ex(jfeatures, "relay", &jvalue);
	enable = json_object_get_boolean(jvalue);
	comp.relay = (enable) ? true : false;

	json_object_object_get_ex(jfeatures, "proxy", &jvalue);
	enable = json_object_get_boolean(jvalue);
	comp.proxy = (enable) ? true : false;

	json_object_object_get_ex(jfeatures, "friend", &jvalue);
	enable = json_object_get_boolean(jvalue);
	comp.friend = (enable) ? true : false;

	json_object_object_get_ex(jfeatures, "lowPower", &jvalue);
	enable = json_object_get_boolean(jvalue);
	comp.lpn = (enable) ? true : false;

	if (!node_set_composition(node, &comp))
		return false;

	json_object_object_get_ex(jcomp, "elements", &jelements);
	if (!jelements)
		return false;

	return parse_composition_elements(node, jelements);
}

static bool parse_node(json_object *jnode, bool local)
{
	json_object *jconfig;
	json_object *jelements;
	json_object *jidxs;
	json_object *jvalue;
	json_object *jint;
	uint8_t key[16];
	char *value_str;
	uint32_t idx;
	struct mesh_node *node;

	/* Device key */
	if (!json_object_object_get_ex(jnode, "deviceKey", &jvalue) ||
								!jvalue) {
		if (!mesh_get_random_bytes(key, 16))
			return false;

		add_key(jnode, "deviceKey", key);
	} else {
		value_str = (char *)json_object_get_string(jvalue);
		if (!str2hex(value_str, strlen(value_str), key, 16))
			return false;;
	}

	node = node_new();

	if (!node)
		return false;

	node_set_device_key(node, key);

	json_object_object_get_ex(jnode, "IVindex", &jint);
	if (jint)
		idx = json_object_get_int(jint);
	else
		idx = 0;

	node_set_iv_index(node, idx);
	if (local) {
		bool update = false;
		json_object_object_get_ex(jnode, "IVupdate", &jint);
		if (jint)
			update = json_object_get_int(jint) ? true : false;
		net_set_iv_index(idx, update);
	}

	if (json_object_object_get_ex(jnode, "sequenceNumber", &jint) &&
									jint) {
		int seq = json_object_get_int(jint);
		node_set_sequence_number(node, seq);
	}

	/* Composition is mandatory for local node */
	json_object_object_get_ex(jnode, "composition", &jconfig);
	if ((jconfig && !parse_node_composition(node, jconfig)) ||
							(!jconfig && local)) {
		node_free(node);
		return false;
	}

	/* Configuration is mandatory for nodes in provisioning database */
	json_object_object_get_ex(jnode, "configuration", &jconfig);
	if (!jconfig) {
		if (local) {
			/* This is an unprovisioned local device */
			goto done;
		} else {
			node_free(node);
			return false;
		}
	}

	json_object_object_get_ex(jconfig, "elements", &jelements);
	if (!jelements) {
		node_free(node);
		return false;
	}

	if (!parse_configuration_elements(node, jelements, local)) {
		node_free(node);
		return false;;
	}

	json_object_object_get_ex(jconfig, "netKeys", &jidxs);
	if (!jidxs || (parse_node_keys(node, jidxs, false) == 0)) {
		node_free(node);
		return false;
	}

	json_object_object_get_ex(jconfig, "appKeys", &jidxs);
	if (jidxs)
		parse_node_keys(node, jidxs, true);

	json_object_object_get_ex(jconfig, "defaultTTL", &jvalue);
	if (jvalue) {
		int ttl = json_object_get_int(jvalue);
		node_set_default_ttl(node, ttl &TTL_MASK);
	}

done:
	if (local && !node_set_local_node(node)) {
		node_free(node);
		return false;
	}

	return true;
}

bool prov_db_show(const char *filename)
{
	char *str;

	str = prov_file_read(filename);
	if (!str)
		return false;

	bt_shell_printf("%s\n", str);
	g_free(str);
	return true;
}

static bool read_json_db(const char *filename, bool provisioner, bool local)
{
	char *str;
	json_object *jmain;
	json_object *jarray;
	json_object *jprov;
	json_object *jvalue;
	json_object *jtemp;
	uint8_t key[16];
	int value_int;
	char *value_str;
	int len;
	int i;
	uint32_t index;
	bool refresh = false;
	bool res = false;

	str = prov_file_read(filename);
	if (!str) return false;

	jmain = json_tokener_parse(str);
	if (!jmain)
		goto done;

	if (local) {
		json_object *jnode;
		bool result;

		json_object_object_get_ex(jmain, "node", &jnode);
		if (!jnode) {
			bt_shell_printf("Cannot find \"node\" object");
			goto done;
		} else
			result = parse_node(jnode, true);

		/*
		* If local node is provisioner, the rest of mesh settings
		* are read from provisioning database.
		*/
		if (provisioner) {
			res = result;
			goto done;
		}
	}

	/* IV index */
	json_object_object_get_ex(jmain, "IVindex", &jvalue);
	if (!jvalue)
		goto done;

	index = json_object_get_int(jvalue);

	json_object_object_get_ex(jmain, "IVupdate", &jvalue);
	if (!jvalue)
		goto done;

	value_int = json_object_get_int(jvalue);

	net_set_iv_index(index, value_int);

	/* Network key(s) */
	json_object_object_get_ex(jmain, "netKeys", &jarray);
	if (!jarray)
		goto done;

	len = json_object_array_length(jarray);
	bt_shell_printf("# netkeys = %d\n", len);

	for (i = 0; i < len; ++i) {
		uint32_t idx;

		jtemp = json_object_array_get_idx(jarray, i);
		json_object_object_get_ex(jtemp, "index", &jvalue);
		if (!jvalue)
			goto done;
		idx = json_object_get_int(jvalue);

		json_object_object_get_ex(jtemp, "key", &jvalue);
		if (!jvalue) {
			if (!mesh_get_random_bytes(key, 16))
				goto done;
			add_key(jtemp, "key", key);
			refresh = true;
		} else {
			value_str = (char *)json_object_get_string(jvalue);
			if (!str2hex(value_str, strlen(value_str), key, 16)) {
				goto done;
			}
		}

		if (!keys_net_key_add(idx, key, false))
			goto done;

		json_object_object_get_ex(jtemp, "keyRefresh", &jvalue);
		if (!jvalue)
			goto done;

		keys_set_kr_phase(idx, (uint8_t) json_object_get_int(jvalue));
	}

	/* App keys */
	json_object_object_get_ex(jmain, "appKeys", &jarray);
	if (jarray) {
		len = json_object_array_length(jarray);
		bt_shell_printf("# appkeys = %d\n", len);

		for (i = 0; i < len; ++i) {
			int app_idx;
			int net_idx;

			jtemp = json_object_array_get_idx(jarray, i);
			json_object_object_get_ex(jtemp, "index",
						&jvalue);
			if (!jvalue)
				goto done;

			app_idx = json_object_get_int(jvalue);
			if (!CHECK_KEY_IDX_RANGE(app_idx))
				goto done;

			json_object_object_get_ex(jtemp, "key", &jvalue);
			if (!jvalue) {
				if (!mesh_get_random_bytes(key, 16))
					goto done;
				add_key(jtemp, "key", key);
				refresh = true;
			} else {
				value_str =
					(char *)json_object_get_string(jvalue);
				str2hex(value_str, strlen(value_str), key, 16);
			}

			json_object_object_get_ex(jtemp, "boundNetKey",
							&jvalue);
			if (!jvalue)
				goto done;

			net_idx = json_object_get_int(jvalue);
			if (!CHECK_KEY_IDX_RANGE(net_idx))
				goto done;

			keys_app_key_add(net_idx, app_idx, key, false);
		}
	}

	/* Provisioner info */
	json_object_object_get_ex(jmain, "provisioners", &jarray);
	if (!jarray)
		goto done;

	len = json_object_array_length(jarray);
	bt_shell_printf("# provisioners = %d\n", len);

	for (i = 0; i < len; ++i) {

		jprov = json_object_array_get_idx(jarray, i);

		/* Allocated unicast range */
		json_object_object_get_ex(jprov, "allocatedUnicastRange",
						&jtemp);
		if (!jtemp) {
			goto done;
		}

		if (!parse_unicast_range(jtemp)) {
			bt_shell_printf("Doneed to parse unicast range\n");
			goto done;
		}
	}

	json_object_object_get_ex(jmain, "nodes", &jarray);
	if (!jarray) {
		res = true;
		goto done;
	}

	len = json_object_array_length(jarray);

	bt_shell_printf("# provisioned nodes = %d\n", len);
	for (i = 0; i < len; ++i) {
		json_object *jnode;
		jnode = json_object_array_get_idx(jarray, i);

		if (!jnode || !parse_node(jnode, false))
			goto done;
	}

	res = true;
done:

	g_free(str);

	if (res && refresh)
		prov_file_write(jmain, false);

	if (jmain)
		json_object_put(jmain);

	return res;
}

bool prov_db_read(const char *filename)
{
	prov_filename = filename;
	return read_json_db(filename, true, false);
}

bool prov_db_read_local_node(const char *filename, bool provisioner)
{
	local_filename = filename;
	return read_json_db(filename, provisioner, true);
}

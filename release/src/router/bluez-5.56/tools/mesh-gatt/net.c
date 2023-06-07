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

#include <inttypes.h>
#include <ctype.h>
#include <stdbool.h>
#include <stdio.h>
#include <string.h>
#include <glib.h>

#include "src/shared/util.h"
#include "src/shared/shell.h"

#include "tools/mesh-gatt/crypto.h"
#include "tools/mesh-gatt/gatt.h"
#include "tools/mesh-gatt/mesh-net.h"
#include "tools/mesh-gatt/util.h"
#include "tools/mesh-gatt/keys.h"
#include "tools/mesh-gatt/node.h"
#include "tools/mesh-gatt/prov-db.h"
#include "tools/mesh-gatt/net.h"

struct address_range
{
	uint16_t min;
	uint16_t max;
};

struct mesh_net {
	uint32_t iv_index;
	uint32_t seq_num;
	uint32_t seq_num_reserved;
	uint16_t primary_addr;
	uint8_t iv_upd_state;
	uint8_t num_elements;
	uint8_t default_ttl;
	bool iv_update;
	bool provisioner;
	bool blacklist;
	guint iv_update_timeout;
	GDBusProxy *proxy_in;
	GList *address_pool;
	GList *dest;	/* List of valid local destinations for Whitelist */
	GList *sar_in;	/* Incoming segmented messages in progress */
	GList *msg_out;	/* Pre-Network encoded, might be multi-segment */
	GList *pkt_out; /* Fully encoded packets awaiting Tx in order */
	net_mesh_session_open_callback open_cb;
};

struct generic_key {
	uint16_t	idx;
};

struct net_key_parts {
	uint8_t nid;
	uint8_t enc_key[16];
	uint8_t privacy_key[16];
	uint8_t	net_key[16];
	uint8_t	beacon_key[16];
	uint8_t	net_id[8];
};

struct mesh_net_key {
	struct generic_key	generic;
	uint8_t			phase;
	struct net_key_parts	current;
	struct net_key_parts	new;
};

struct app_key_parts {
	uint8_t key[16];
	uint8_t akf_aid;
};

struct mesh_app_key {
	struct generic_key	generic;
	uint16_t		net_idx;
	struct app_key_parts	current;
	struct app_key_parts	new;
};

struct mesh_virt_addr {
	uint16_t	va16;
	uint32_t	va32;
	uint8_t		va128[16];
};

struct mesh_pkt {
	uint8_t		data[30];
	uint8_t		len;
};

struct mesh_sar_msg {
	guint		ack_to;
	guint		msg_to;
	uint32_t	iv_index;
	uint32_t	seqAuth;
	uint32_t	ack;
	uint32_t	dst;
	uint16_t	src;
	uint16_t	net_idx;
	uint16_t	len;
	uint8_t		akf_aid;
	uint8_t		ttl;
	uint8_t		segN;
	uint8_t		activity_cnt;
	bool		ctl;
	bool		segmented;
	bool		szmic;
	bool		proxy;
	uint8_t		data[20]; /* Open ended, min 20 */
};

struct mesh_destination {
	uint16_t	cnt;
	uint16_t	dst;
};

/* Network Packet Layer based Offsets */
#define AKF_BIT			0x40

#define PKT_IVI(p)		!!((p)[0] & 0x80)
#define SET_PKT_IVI(p,v)	do {(p)[0] &= 0x7f; \
					(p)[0] |= ((v) ? 0x80 : 0);} while(0)
#define PKT_NID(p)		((p)[0] & 0x7f)
#define SET_PKT_NID(p,v)	do {(p)[0] &= 0x80; (p)[0] |= (v);} while(0)
#define PKT_CTL(p)		(!!((p)[1] & 0x80))
#define SET_PKT_CTL(p,v)	do {(p)[1] &= 0x7f; \
					(p)[1] |= ((v) ? 0x80 : 0);} while(0)
#define PKT_TTL(p)		((p)[1] & 0x7f)
#define SET_PKT_TTL(p,v)	do {(p)[1] &= 0x80; (p)[1] |= (v);} while(0)
#define PKT_SEQ(p)		(get_be32((p) + 1) & 0xffffff)
#define SET_PKT_SEQ(p,v)	put_be32(((p)[1] << 24) + ((v) & 0xffffff), \
									(p) + 1)
#define PKT_SRC(p)		get_be16((p) + 5)
#define SET_PKT_SRC(p,v)	put_be16(v, (p) + 5)
#define PKT_DST(p)		get_be16((p) + 7)
#define SET_PKT_DST(p,v)	put_be16(v, (p) + 7)
#define PKT_TRANS(p)		((p) + 9)
#define PKT_TRANS_LEN(l)	((l) - 9)

#define PKT_SEGMENTED(p)	(!!((p)[9] & 0x80))
#define SET_PKT_SEGMENTED(p,v)	do {(p)[9] &= 0x7f; \
					(p)[9] |= ((v) ? 0x80 : 0);} while(0)
#define PKT_AKF_AID(p)		((p)[9] & 0x7f)
#define SET_PKT_AKF_AID(p,v)	do {(p)[9] &= 0x80; (p)[9] |= (v);} while(0)
#define PKT_OPCODE(p)		((p)[9] & 0x7f)
#define SET_PKT_OPCODE(p,v)	do {(p)[9] &= 0x80; (p)[9] |= (v);} while(0)
#define PKT_OBO(p)		(!!((p)[10] & 0x80))
#define PKT_SZMIC(p)		(!!(PKT_SEGMENTED(p) ? ((p)[10] & 0x40) : 0))
#define SET_PKT_SZMIC(p,v)	do {(p)[10] &= 0x7f; \
					(p)[10] |= ((v) ? 0x80 : 0);} while(0)
#define PKT_SEQ0(p)		((get_be16((p) + 10) >> 2) & 0x1fff)
#define SET_PKT_SEQ0(p,v)	do {put_be16((get_be16((p) + 10) & 0x8003) \
					| (((v) & 0x1fff) << 2), \
					(p) + 10);} while(0)
#define SET_PKT_SEGO(p,v)	do {put_be16((get_be16( \
					(p) + 11) & 0xfc1f) | ((v) << 5), \
					(p) + 11);} while(0)
#define SET_PKT_SEGN(p,v)	do {(p)[12] = ((p)[12] & 0xe0) | (v);} while(0)
#define PKT_ACK(p)		(get_be32((p) + 12))
#define SET_PKT_ACK(p,v)	(put_be32((v)(p) + 12))

/* Transport Layer based offsets */
#define TRANS_SEGMENTED(t)	(!!((t)[0] & 0x80))
#define SET_TRANS_SEGMENTD(t,v)	do {(t)[0] &= 0x7f; \
					(t)[0] |= ((v) ? 0x80 : 0);} while(0)
#define TRANS_OPCODE(t)		((t)[0] & 0x7f)
#define SET_TRANS_OPCODE(t,v)	do {(t)[0] &= 0x80; (t)[0] |= (v);} while(0)
#define TRANS_AKF_AID(t)		((t)[0] & 0x7f)
#define SET_TRANS_AKF_AID(t,v)	do {(t)[0] &= 0xc0; (t)[0] |= (v);} while(0)
#define TRANS_AKF(t)		(!!((t)[0] & AKF_BIT))
#define TRANS_SZMIC(t)		(!!(TRANS_SEGMENTED(t) ? ((t)[1] & 0x80) : 0))
#define TRANS_SEQ0(t)		((get_be16((t) + 1) >> 2) & 0x1fff)
#define SET_TRANS_SEQ0(t,v)	do {put_be16((get_be16((t) + 1) & 0x8003) \
					| (((v) & 0x1fff) << 2), \
					(t) + 1);} while(0)
#define SET_TRANS_ACK(t,v)	put_be32((v), (t) + 3)
#define TRANS_SEGO(t)		((get_be16((t) + 2) >> 5) & 0x1f)
#define TRANS_SEGN(t)		((t)[3] & 0x1f)

#define TRANS_PAYLOAD(t)	((t) + (TRANS_SEGMENTED(t) ? 4 : 1))
#define TRANS_LEN(t,l)		((l) -(TRANS_SEGMENTED(t) ? 4 : 1))

/* Proxy Config Opcodes */
#define FILTER_SETUP		0x00
#define FILTER_ADD		0x01
#define FILTER_DEL		0x02
#define FILTER_STATUS		0x03

/* Proxy Filter Types */
#define WHITELIST_FILTER	0x00
#define BLACKLIST_FILTER	0x01

/* IV Updating states for timing enforcement */
#define IV_UPD_INIT 		0
#define IV_UPD_NORMAL		1
#define IV_UPD_UPDATING		2
#define IV_UPD_NORMAL_HOLD	3

#define IV_IDX_DIFF_RANGE	42

static struct mesh_net net;
static GList *virt_addrs = NULL;
static GList *net_keys = NULL;
static GList *app_keys = NULL;

/* Forward static declarations */
static void resend_segs(struct mesh_sar_msg *sar);

static int match_net_id(const void *a, const void *net_id)
{
	const struct mesh_net_key *net_key = a;

	if (net_key->current.nid != 0xff &&
			!memcmp(net_key->current.net_id, net_id, 8))
		return 0;

	if (net_key->new.nid != 0xff &&
			!memcmp(net_key->new.net_id, net_id, 8))
		return 0;

	return -1;
}

static struct mesh_net_key *find_net_key_by_id(const uint8_t *net_id)
{
	GList *l;

	l = g_list_find_custom(net_keys, net_id, match_net_id);

	if (!l)
		return NULL;

	return l->data;
}

uint16_t net_validate_proxy_beacon(const uint8_t *proxy_beacon)
{
	struct mesh_net_key *net_key = find_net_key_by_id(proxy_beacon);

	if (net_key == NULL)
		return NET_IDX_INVALID;

	return net_key->generic.idx;
}

static int match_sar_dst(const void *a, const void *b)
{
	const struct mesh_sar_msg *sar = a;
	uint16_t dst = GPOINTER_TO_UINT(b);

	return (sar->dst == dst) ? 0 : -1;
}

static struct mesh_sar_msg *find_sar_out_by_dst(uint16_t dst)
{
	GList *l;

	l = g_list_find_custom(net.msg_out, GUINT_TO_POINTER(dst),
			match_sar_dst);

	if (!l)
		return NULL;

	return l->data;
}

static int match_sar_src(const void *a, const void *b)
{
	const struct mesh_sar_msg *sar = a;
	uint16_t src = GPOINTER_TO_UINT(b);

	return (sar->src == src) ? 0 : -1;
}

static struct mesh_sar_msg *find_sar_in_by_src(uint16_t src)
{
	GList *l;

	l = g_list_find_custom(net.sar_in, GUINT_TO_POINTER(src),
			match_sar_src);

	if (!l)
		return NULL;

	return l->data;
}

static int match_key_index(const void *a, const void *b)
{
	const struct generic_key *generic = a;
	uint16_t index = GPOINTER_TO_UINT(b);

	return (generic->idx == index) ? 0 : -1;
}

static bool delete_key(GList **list, uint16_t index)
{
	GList *l;

	l = g_list_find_custom(*list, GUINT_TO_POINTER(index),
				match_key_index);

	if (!l)
		return false;

	*list = g_list_delete_link(*list, l);

	return true;

}

static uint8_t *get_key(GList *list, uint16_t index)
{
	GList *l;
	struct mesh_app_key *app_key;
	struct mesh_net_key *net_key;

	l = g_list_find_custom(list, GUINT_TO_POINTER(index),
				match_key_index);

	if (!l) return NULL;

	if (list == app_keys) {
		app_key = l->data;

		/* All App Keys must belong to a valid Net Key */
		l = g_list_find_custom(net_keys,
				GUINT_TO_POINTER(app_key->net_idx),
				match_key_index);

		if (!l) return NULL;

		net_key = l->data;

		if (net_key->phase == 2 && app_key->new.akf_aid != 0xff)
			return app_key->new.key;

		if (app_key->current.akf_aid != 0xff)
			return app_key->current.key;

		return NULL;
	}

	net_key = l->data;

	if (net_key->phase == 2 && net_key->new.nid != 0xff)
		return net_key->new.net_key;

	if (net_key->current.nid != 0xff)
		return net_key->current.net_key;

	return NULL;
}

bool keys_app_key_add(uint16_t net_idx, uint16_t app_idx, uint8_t *key,
			bool update)
{
	struct mesh_app_key *app_key = NULL;
	uint8_t akf_aid;
	GList *l = g_list_find_custom(app_keys, GUINT_TO_POINTER(app_idx),
				match_key_index);

	if (!mesh_crypto_k4(key, &akf_aid))
		return false;

	akf_aid |= AKF_BIT;

	if (l && update) {

		app_key = l->data;

		if (app_key->net_idx != net_idx)
			return false;

		memcpy(app_key->new.key, key, 16);
		app_key->new.akf_aid = akf_aid;

	} else if (l) {

		app_key = l->data;

		if (memcmp(app_key->current.key, key, 16) ||
				app_key->net_idx != net_idx)
			return false;

	} else {

		app_key = g_new(struct mesh_app_key, 1);
		memcpy(app_key->current.key, key, 16);
		app_key->net_idx = net_idx;
		app_key->generic.idx = app_idx;
		app_key->current.akf_aid = akf_aid;

		/* Invalidate "New" version */
		app_key->new.akf_aid = 0xff;

		app_keys = g_list_append(app_keys, app_key);

	}

	return true;
}

bool keys_net_key_add(uint16_t net_idx, uint8_t *key, bool update)
{
	struct mesh_net_key *net_key = NULL;
	uint8_t p = 0;
	GList *l = g_list_find_custom(net_keys, GUINT_TO_POINTER(net_idx),
				match_key_index);

	if (l && update) {
		bool result;

		net_key = l->data;

		memcpy(net_key->new.net_key, key, 16);

		/* Calculate the many component parts */
		result = mesh_crypto_nkbk(key, net_key->new.beacon_key);
		if (!result)
			return false;

		result = mesh_crypto_k3(key, net_key->new.net_id);
		if (!result)
			return false;

		result = mesh_crypto_k2(key, &p, 1,
				&net_key->new.nid,
				net_key->new.enc_key,
				net_key->new.privacy_key);
		if (!result)
			net_key->new.nid = 0xff;

		return result;

	} else if (l) {
		net_key = l->data;

		if (memcmp(net_key->current.net_key, key, 16))
			return false;
	} else {
		bool result;

		net_key = g_new(struct mesh_net_key, 1);
		memcpy(net_key->current.net_key, key, 16);
		net_key->generic.idx = net_idx;

		/* Invalidate "New" version */
		net_key->new.nid = 0xff;

		/* Calculate the many component parts */
		result = mesh_crypto_nkbk(key, net_key->current.beacon_key);
		if (!result) {
			g_free(net_key);
			return false;
		}

		result = mesh_crypto_k3(key, net_key->current.net_id);
		if (!result) {
			g_free(net_key);
			return false;
		}

		result = mesh_crypto_k2(key, &p, 1,
				&net_key->current.nid,
				net_key->current.enc_key,
				net_key->current.privacy_key);

		if (!result) {
			g_free(net_key);
			return false;
		}

		net_keys = g_list_append(net_keys, net_key);
	}

	return true;
}

static struct mesh_app_key *find_app_key_by_idx(uint16_t app_idx)
{
	GList *l;

	l = g_list_find_custom(app_keys, GUINT_TO_POINTER(app_idx),
				match_key_index);

	if (!l) return NULL;

	return l->data;
}

static struct mesh_net_key *find_net_key_by_idx(uint16_t net_idx)
{
	GList *l;

	l = g_list_find_custom(net_keys, GUINT_TO_POINTER(net_idx),
				match_key_index);

	if (!l) return NULL;

	return l->data;
}

static int match_virt_dst(const void *a, const void *b)
{
	const struct mesh_virt_addr *virt = a;
	uint32_t dst = GPOINTER_TO_UINT(b);

	if (dst < 0x10000 && dst == virt->va16)
		return 0;

	if (dst == virt->va32)
		return 0;

	return -1;
}

static struct mesh_virt_addr *find_virt_by_dst(uint32_t dst)
{
	GList *l;

	l = g_list_find_custom(virt_addrs, GUINT_TO_POINTER(dst),
				match_virt_dst);

	if (!l) return NULL;

	return l->data;
}

uint8_t *keys_net_key_get(uint16_t net_idx, bool current)
{
	GList *l;


	l = g_list_find_custom(net_keys, GUINT_TO_POINTER(net_idx),
				match_key_index);
	if (!l) {
		return NULL;
	} else {
		struct mesh_net_key *key = l->data;
		if (current)
			return key->current.net_key;
		else
			return key->new.net_key;
	}
}

bool keys_app_key_delete(uint16_t app_idx)
{
	/* TODO: remove all associated bindings */
	return delete_key(&app_keys, app_idx);
}

bool keys_net_key_delete(uint16_t net_idx)
{
	/* TODO: remove all associated app keys and bindings */
	return delete_key(&net_keys, net_idx);
}

uint8_t keys_get_kr_phase(uint16_t net_idx)
{
	GList *l;
	struct mesh_net_key *key;

	l = g_list_find_custom(net_keys, GUINT_TO_POINTER(net_idx),
				match_key_index);

	if (!l)
		return KR_PHASE_INVALID;

	key = l->data;

	return key->phase;
}

bool keys_set_kr_phase(uint16_t index, uint8_t phase)
{
	GList *l;
	struct mesh_net_key *net_key;

	l = g_list_find_custom(net_keys, GUINT_TO_POINTER(index),
				match_key_index);

	if (!l)
		return false;

	net_key = l->data;
	net_key->phase = phase;

	return true;
}

uint16_t keys_app_key_get_bound(uint16_t app_idx)
{
	GList *l;

	l = g_list_find_custom(app_keys, GUINT_TO_POINTER(app_idx),
				match_key_index);
	if (!l)
		return NET_IDX_INVALID;
	else {
		struct mesh_app_key *key = l->data;
		return key->net_idx;
	}
}

uint8_t *keys_app_key_get(uint16_t app_idx, bool current)
{
	GList *l;


	l = g_list_find_custom(app_keys, GUINT_TO_POINTER(app_idx),
				match_key_index);
	if (!l) {
		return NULL;
	} else {
		struct mesh_app_key *key = l->data;
		if (current)
			return key->current.key;
		else
			return key->new.key;
	}
}

void keys_cleanup_all(void)
{
	g_list_free_full(app_keys, g_free);
	g_list_free_full(net_keys, g_free);
	app_keys = net_keys = NULL;
}

bool net_get_key(uint16_t net_idx, uint8_t *key)
{
	uint8_t *buf;

	buf = get_key(net_keys, net_idx);

	if (!buf)
		return false;

	memcpy(key, buf, 16);
	return true;
}

bool net_get_flags(uint16_t net_idx, uint8_t *out_flags)
{
	uint8_t phase;

	phase = keys_get_kr_phase(net_idx);

	if (phase == KR_PHASE_INVALID || !out_flags)
		return false;

	if (phase != KR_PHASE_NONE)
		*out_flags = 0x01;
	else
		*out_flags = 0x00;

	if (net.iv_update)
		*out_flags |= 0x02;

	return true;
}

uint32_t net_get_iv_index(bool *update)
{
	if (update)
		*update = net.iv_update;

	return net.iv_index;
}

void net_set_iv_index(uint32_t iv_index, bool update)
{
	net.iv_index = iv_index;
	net.iv_update = update;
}

void set_sequence_number(uint32_t seq_num)
{
	net.seq_num = seq_num;
}

uint32_t get_sequence_number(void)
{
	return net.seq_num;
}

bool net_add_address_pool(uint16_t min, uint16_t max)
{
	uint32_t range;
	if (max < min)
		return false;
	range = min + (max << 16);
	net.address_pool = g_list_append(net.address_pool,
						GUINT_TO_POINTER(range));
	return true;
}

static int match_address_range(const void *a, const void *b)
{
	uint32_t range = GPOINTER_TO_UINT(a);
	uint8_t num_elements = (uint8_t) (GPOINTER_TO_UINT(b));
	uint16_t max = range >> 16;
	uint16_t min = range & 0xffff;

	return ((max - min + 1) >= num_elements) ? 0 : -1;

}

uint16_t net_obtain_address(uint8_t num_eles)
{
	uint16_t addr;
	GList *l;

	l = g_list_find_custom(net.address_pool, GUINT_TO_POINTER(num_eles),
				match_address_range);
	if (l) {
		uint32_t range = GPOINTER_TO_UINT(l->data);
		uint16_t max = range >> 16;
		uint16_t min = range & 0xffff;

		addr = min;
		min += num_eles;

		if (min > max) {
			net.address_pool = g_list_delete_link(net.address_pool,
								l);
		} else {
			range = min + (max << 16);
			l->data = GUINT_TO_POINTER(range);
		}

		return addr;
	}

	return UNASSIGNED_ADDRESS;
}

static int range_cmp(const void *a, const void *b)
{
	uint32_t range1 = GPOINTER_TO_UINT(a);
	uint32_t range2 = GPOINTER_TO_UINT(b);

	return range2 - range1;
}

void net_release_address(uint16_t addr, uint8_t num_elements)
{
	GList *l;
	uint32_t range;

	if (addr == UNASSIGNED_ADDRESS)
		return;

	for (l = net.address_pool; l != NULL; l = l->next)
	{
		uint16_t max;
		uint16_t min;
		GList *l1 = l->next;

		range = GPOINTER_TO_UINT(l->data);

		max = range >> 16;
		min = range & 0xffff;

		if (min == (addr + num_elements))
			min  = addr;
		else if (max == (addr - 1))
			max = addr + num_elements;
		else
			continue;

		/* Check if range pools need to be merged */
		if (l1) {
			uint16_t min1;

			range = GPOINTER_TO_UINT(l1->data);
			min1 = range & 0xffff;

			if (min1 == (max + 1)) {
				max = range >> 16;
				l->next = l1->next;
				net.address_pool = g_list_delete_link(
							net.address_pool, l1);
			}
		}

		range = min + (max << 16);
		l->data = GUINT_TO_POINTER(range);
		return;
	}

	range = addr + ((addr + num_elements - 1) << 16);
	net.address_pool = g_list_insert_sorted(net.address_pool,
						GUINT_TO_POINTER(range),
						range_cmp);
}

bool net_reserve_address_range(uint16_t base, uint8_t num_elements)
{
	GList *l;
	uint32_t range;
	uint16_t max;
	uint16_t min;
	bool shrink;

	for (l = net.address_pool; l != NULL; l = l->next) {

		range = GPOINTER_TO_UINT(l->data);

		max = range >> 16;
		min = range & 0xffff;

		if (base >= min && (base + num_elements - 1) <= max)
			break;
	}

	if (!l)
		return false;

	net.address_pool = g_list_delete_link(net.address_pool, l);

	shrink = false;

	if (base == min) {
		shrink = true;
		min = base + num_elements;
	}

	if (max == base + num_elements - 1) {
		shrink = true;
		max -= num_elements;
	}

	if (min > max)
		return true;

	if (shrink)
		range = min + (max << 16);
	else
		range = min + ((base - 1) << 16);

	net.address_pool = g_list_insert_sorted(net.address_pool,
						GUINT_TO_POINTER(range),
						range_cmp);

	if (shrink)
		return true;

	range = (base + num_elements) + (max << 16);
	net.address_pool = g_list_insert_sorted(net.address_pool,
						GUINT_TO_POINTER(range),
						range_cmp);

	return true;
}

static int match_destination(const void *a, const void *b)
{
	const struct mesh_destination *dest = a;
	uint16_t dst = GPOINTER_TO_UINT(b);

	return (dest->dst == dst) ? 0 : -1;
}

void net_dest_ref(uint16_t dst)
{
	struct mesh_destination *dest;
	GList *l;

	if (!dst) return;

	l = g_list_find_custom(net.dest, GUINT_TO_POINTER(dst),
			match_destination);

	if (l) {
		dest = l->data;
		dest->cnt++;
		return;
	}

	dest = g_new0(struct mesh_destination, 1);
	dest->dst = dst;
	dest->cnt++;
	net.dest = g_list_append(net.dest, dest);
}

void net_dest_unref(uint16_t dst)
{
	struct mesh_destination *dest;
	GList *l;

	l = g_list_find_custom(net.dest, GUINT_TO_POINTER(dst),
			match_destination);

	if (!l)
		return;

	dest = l->data;
	dest->cnt--;

	if (dest->cnt == 0) {
		net.dest = g_list_remove(net.dest, dest);
		g_free(dest);
	}
}

struct build_whitelist {
	uint8_t len;
	uint8_t data[12];
};

static void whitefilter_add(gpointer data, gpointer user_data)
{
	struct mesh_destination	*dest = data;
	struct build_whitelist *white = user_data;

	if (white->len == 0)
		white->data[white->len++] = FILTER_ADD;

	put_be16(dest->dst, white->data + white->len);
	white->len += 2;

	if (white->len > (sizeof(white->data) - sizeof(uint16_t))) {
		net_ctl_msg_send(0, 0, 0, white->data, white->len);
		white->len = 0;
	}
}

static void setup_whitelist()
{
	struct build_whitelist white;

	white.len = 0;

	/* Enable (and Clear) Proxy Whitelist */
	white.data[white.len++] = FILTER_SETUP;
	white.data[white.len++] = WHITELIST_FILTER;

	net_ctl_msg_send(0, 0, 0, white.data, white.len);

	white.len = 0;
	g_list_foreach(net.dest, whitefilter_add, &white);

	if (white.len)
		net_ctl_msg_send(0, 0, 0, white.data, white.len);
}

static void beacon_update(bool first, bool iv_update, uint32_t iv_index)
{

	/* Enforcement of 96 hour and 192 hour IVU time windows */
	if (iv_update && !net.iv_update) {
		bt_shell_printf("iv_upd_state = IV_UPD_UPDATING\n");
		net.iv_upd_state = IV_UPD_UPDATING;
		/* TODO: Start timer to enforce IV Update parameters */
	} else if (first) {
		if (iv_update)
			net.iv_upd_state = IV_UPD_UPDATING;
		else
			net.iv_upd_state = IV_UPD_NORMAL;

		bt_shell_printf("iv_upd_state = IV_UPD_%s\n",
				iv_update ? "UPDATING" : "NORMAL");

	} else if (iv_update && iv_index != net.iv_index) {
		bt_shell_printf("IV Update too soon -- Rejecting\n");
		return;
	}

	if (iv_index > net.iv_index ||
			iv_update != net.iv_update) {

		/* Don't reset our seq_num unless
		 * we start using new iv_index */
		if (!(iv_update && (net.iv_index + 1 == iv_index))) {
			net.seq_num = 0;
			net.seq_num_reserved = 100;
		}
	}

	if (!net.seq_num || net.iv_index != iv_index ||
			net.iv_update != iv_update) {

		if (net.seq_num_reserved <= net.seq_num)
			net.seq_num_reserved = net.seq_num + 100;

		prov_db_local_set_iv_index(iv_index, iv_update,
				net.provisioner);
		prov_db_local_set_seq_num(net.seq_num_reserved);
	}

	net.iv_index = iv_index;
	net.iv_update = iv_update;

	if (first) {
		/* Must be done once per Proxy Connection after Beacon RXed */
		setup_whitelist();
		if (net.open_cb)
			net.open_cb(0);
	}
}

static bool process_beacon(uint8_t *data, uint8_t size)
{
	struct mesh_net_key *net_key;
	struct net_key_parts *key_part;
	bool rxed_iv_update, rxed_key_refresh, iv_update;
	bool  my_krf;
	uint32_t rxed_iv_index, iv_index;
	uint64_t cmac;

	if (size != 22)
		return false;

	rxed_key_refresh = (data[1] & 0x01) == 0x01;
	iv_update = rxed_iv_update = (data[1] & 0x02) == 0x02;
	iv_index = rxed_iv_index = get_be32(data + 10);

	/* Inhibit recognizing iv_update true-->false
	 * if we have outbound SAR messages in flight */
	if (net.msg_out != NULL) {
		if (net.iv_update && !rxed_iv_update)
			iv_update = true;
	}

	/* Don't bother going further if nothing has changed */
	if (iv_index == net.iv_index && iv_update == net.iv_update &&
			net.iv_upd_state != IV_UPD_INIT)
		return true;

	/* Find key we are using for SNBs */
	net_key = find_net_key_by_id(data + 2);

	if (net_key == NULL)
		return false;

	/* We are Provisioner, and control the key_refresh flag */
	if (rxed_key_refresh != !!(net_key->phase == 2))
		return false;

	if (net_key->phase != 2) {
		my_krf = false;
		key_part = &net_key->current;
	} else {
		my_krf = true;
		key_part = &net_key->new;
	}

	/* Ignore for incorrect KR state */
	if (memcmp(key_part->net_id, data + 2, 8))
		return false;

	if ((net.iv_index + IV_IDX_DIFF_RANGE < iv_index) ||
			(iv_index < net.iv_index)) {
		bt_shell_printf("iv index outside range\n");
		return false;
	}

	/* Any behavioral changes must pass CMAC test */
	if (!mesh_crypto_beacon_cmac(key_part->beacon_key, key_part->net_id,
				rxed_iv_index, my_krf,
				rxed_iv_update, &cmac)) {
		return false;
	}

	if (cmac != get_be64(data + 14))
		return false;

	if (iv_update && (net.iv_upd_state > IV_UPD_UPDATING)) {
		if (iv_index != net.iv_index) {
			bt_shell_printf("Update too soon -- Rejecting\n");
		}
		/* Silently ignore old beacons */
		return true;
	}

	beacon_update(net.iv_upd_state == IV_UPD_INIT, iv_update, iv_index);

	return true;
}

struct decode_params {
	struct mesh_net_key	*net_key;
	uint8_t			*packet;
	uint32_t		iv_index;
	uint8_t			size;
	bool			proxy;
};

static void try_decode(gpointer data, gpointer user_data)
{
	struct mesh_net_key *net_key = data;
	struct decode_params *decode = user_data;
	uint8_t nid = decode->packet[0] & 0x7f;
	uint8_t tmp[29];
	bool status = false;

	if (decode->net_key)
		return;

	if (net_key->current.nid == nid)
		status = mesh_crypto_packet_decode(decode->packet,
				decode->size, decode->proxy, tmp,
				decode->iv_index,
				net_key->current.enc_key,
				net_key->current.privacy_key);

	if (!status && net_key->new.nid == nid)
		status = mesh_crypto_packet_decode(decode->packet,
				decode->size, decode->proxy, tmp,
				decode->iv_index,
				net_key->new.enc_key,
				net_key->new.privacy_key);

	if (status) {
		decode->net_key = net_key;
		memcpy(decode->packet, tmp, decode->size);
		return;
	}
}

static struct mesh_net_key *net_packet_decode(bool proxy, uint32_t iv_index,
				uint8_t *packet, uint8_t size)
{
	struct decode_params decode = {
		.proxy = proxy,
		.iv_index = iv_index,
		.packet = packet,
		.size = size,
		.net_key = NULL,
	};

	g_list_foreach(net_keys, try_decode, &decode);

	return decode.net_key;
}

static void flush_sar(GList **list, struct mesh_sar_msg *sar)
{
	*list = g_list_remove(*list, sar);

	if (sar->msg_to)
		g_source_remove(sar->msg_to);

	if (sar->ack_to)
		g_source_remove(sar->ack_to);

	g_free(sar);
}

static void flush_sar_list(GList **list)
{
	struct mesh_sar_msg *sar;
	GList *l = g_list_first(*list);

	while (l) {
		sar = l->data;
		flush_sar(list, sar);
		l = g_list_first(*list);
	}
}

static void flush_pkt_list(GList **list)
{
	struct mesh_pkt *pkt;
	GList *l = g_list_first(*list);

	while (l) {
		pkt = l->data;
		*list = g_list_remove(*list, pkt);
		g_free(pkt);
		l = g_list_first(*list);
	}
}

static void resend_unacked_segs(gpointer data, gpointer user_data)
{
	struct mesh_sar_msg *sar = data;

	if (sar->activity_cnt)
		resend_segs(sar);
}

static void send_pkt_cmplt(DBusMessage *message, void *user_data)
{
	struct mesh_pkt *pkt = user_data;
	GList *l = g_list_first(net.pkt_out);

	if (l && user_data == l->data) {
		net.pkt_out = g_list_delete_link(net.pkt_out, l);
		g_free(pkt);
	} else {
		/* This is a serious error, and probable memory leak */
		bt_shell_printf("ERR: send_pkt_cmplt %p not head of queue\n", pkt);
	}

	l = g_list_first(net.pkt_out);

	if (l == NULL) {
		/* If queue is newly empty, resend all SAR outbound packets */
		g_list_foreach(net.msg_out, resend_unacked_segs, NULL);
		return;
	}

	pkt = l->data;

	mesh_gatt_write(net.proxy_in, pkt->data, pkt->len,
			send_pkt_cmplt, pkt);
}

static void send_mesh_pkt(struct mesh_pkt *pkt)
{
	bool queued = !!(net.pkt_out);

	net.pkt_out = g_list_append(net.pkt_out, pkt);

	if (queued)
		return;

	mesh_gatt_write(net.proxy_in, pkt->data, pkt->len,
			send_pkt_cmplt, pkt);
}

static uint32_t get_next_seq()
{
	uint32_t this_seq = net.seq_num++;

	if (net.seq_num + 32 >= net.seq_num_reserved) {
		net.seq_num_reserved = net.seq_num + 100;
		prov_db_local_set_seq_num(net.seq_num_reserved);
	}

	return this_seq;
}

static void send_seg(struct mesh_sar_msg *sar, uint8_t seg)
{
	struct mesh_net_key *net_key;
	struct net_key_parts *part;
	struct mesh_pkt *pkt;
	uint8_t *data;

	net_key = find_net_key_by_idx(sar->net_idx);

	if (net_key == NULL)
		return;

	/* Choose which components to use to secure pkt */
	if (net_key->phase == 2 && net_key->new.nid != 0xff)
		part = &net_key->new;
	else
		part = &net_key->current;

	pkt = g_new0(struct mesh_pkt, 1);

	if (pkt == NULL)
		return;

	/* leave extra byte at start for GATT Proxy type */
	data = pkt->data + 1;

	SET_PKT_NID(data, part->nid);
	SET_PKT_IVI(data, sar->iv_index & 1);
	SET_PKT_CTL(data, sar->ctl);
	SET_PKT_TTL(data, sar->ttl);
	SET_PKT_SEQ(data, get_next_seq());
	SET_PKT_SRC(data, sar->src);
	SET_PKT_DST(data, sar->dst);
	SET_PKT_SEGMENTED(data, sar->segmented);

	if (sar->ctl)
		SET_PKT_OPCODE(data, sar->data[0]);
	else
		SET_PKT_AKF_AID(data, sar->akf_aid);

	if (sar->segmented) {

		if (!sar->ctl)
			SET_PKT_SZMIC(data, sar->szmic);

		SET_PKT_SEQ0(data, sar->seqAuth);
		SET_PKT_SEGO(data, seg);
		SET_PKT_SEGN(data, sar->segN);

		memcpy(PKT_TRANS(data) + 4,
				sar->data + sar->ctl + (seg * 12), 12);

		pkt->len = 9 + 4;

		if (sar->segN == seg)
			pkt->len += (sar->len - sar->ctl) % 12;

		if (pkt->len == (9 + 4))
			pkt->len += 12;

	} else {
		memcpy(PKT_TRANS(data) + 1,
				sar->data + sar->ctl, 15);

		pkt->len = 9 + 1 + sar->len - sar->ctl;
	}

	pkt->len += (sar->ctl ? 8 : 4);
	mesh_crypto_packet_encode(data, pkt->len,
			part->enc_key,
			sar->iv_index,
			part->privacy_key);


	/* Prepend GATT_Proxy packet type */
	if (sar->proxy)
		pkt->data[0] = PROXY_CONFIG_PDU;
	else
		pkt->data[0] = PROXY_NETWORK_PDU;

	pkt->len++;

	send_mesh_pkt(pkt);
}

static void resend_segs(struct mesh_sar_msg *sar)
{
	uint32_t ack = 1;
	uint8_t i;

	sar->activity_cnt = 0;

	for (i = 0; i <= sar->segN; i++, ack <<= 1) {
		if (!(ack & sar->ack))
			send_seg(sar, i);
	}
}

static bool ack_rxed(bool to, uint16_t src, uint16_t dst, bool obo,
				uint16_t seq0, uint32_t ack_flags)
{
	struct mesh_sar_msg *sar = find_sar_out_by_dst(src);
	uint32_t full_ack;

	/* Silently ignore unknown (stale?) ACKs */
	if (sar == NULL)
		return true;

	full_ack = 0xffffffff >> (31 - sar->segN);

	sar->ack |= (ack_flags & full_ack);

	if (sar->ack == full_ack) {
		/* Outbound message 100% received by remote node */
		flush_sar(&net.msg_out, sar);
		return true;
	}

	/* Because we are GATT, and slow, only resend PKTs if it is
	 * time *and* our outbound PKT queue is empty.  */
	sar->activity_cnt++;

	if (net.pkt_out == NULL)
		resend_segs(sar);

	return true;
}

static bool proxy_ctl_rxed(uint16_t net_idx, uint32_t iv_index,
		uint8_t ttl, uint32_t seq_num, uint16_t src, uint16_t dst,
		uint8_t *trans, uint16_t len)
{
	if (len < 1)
		return false;

	switch(trans[0]) {
		case FILTER_STATUS:
			if (len != 4)
				return false;

			net.blacklist = !!(trans[1] == BLACKLIST_FILTER);
			bt_shell_printf("Proxy %slist filter length: %d\n",
					net.blacklist ? "Black" : "White",
					get_be16(trans + 2));

			return true;

		default:
			return false;
	}

	return false;
}

static bool ctl_rxed(uint16_t net_idx, uint32_t iv_index,
		uint8_t ttl, uint32_t seq_num, uint16_t src, uint16_t dst,
		uint8_t *trans, uint16_t len)
{
	/* TODO: Handle control messages */

	/* Per Mesh Profile 3.6.5.10 */
	if (trans[0] == NET_OP_HEARTBEAT) {
		uint16_t feat = get_be16(trans + 2);

		bt_shell_printf("HEARTBEAT src: %4.4x dst: %4.4x \
				TTL: %2.2x feat: %s%s%s%s\n",
				src, dst, trans[1],
				(feat & MESH_FEATURE_RELAY) ? "relay " : "",
				(feat & MESH_FEATURE_PROXY) ? "proxy " : "",
				(feat & MESH_FEATURE_FRIEND) ? "friend " : "",
				(feat & MESH_FEATURE_LPN) ? "lpn" : "");
		return true;
	}

	bt_shell_printf("unrecognized control message src:%4.4x dst:%4.4x len:%d\n",
			src, dst, len);
	print_byte_array("msg: ", trans, len);
	return false;
}

struct decrypt_params {
	uint8_t		*nonce;
	uint8_t		*aad;
	uint8_t		*out_msg;
	uint8_t		*trans;
	uint32_t	iv_index;
	uint32_t	seq_num;
	uint16_t	src;
	uint16_t	dst;
	uint16_t	len;
	uint16_t	net_idx;
	uint16_t	app_idx;
	uint8_t		akf_aid;
	bool		szmic;
};


static void try_decrypt(gpointer data, gpointer user_data)
{
	struct mesh_app_key *app_key = data;
	struct decrypt_params *decrypt = user_data;
	size_t mic_size = decrypt->szmic ? sizeof(uint64_t) : sizeof(uint32_t);
	bool status = false;

	/* Already done... Nothing to do */
	if (decrypt->app_idx != APP_IDX_INVALID)
		return;

	/* Don't decrypt on Appkeys not owned by this NetKey */
	if (app_key->net_idx != decrypt->net_idx)
		return;

	/* Test and decrypt against current key copy */
	if (app_key->current.akf_aid == decrypt->akf_aid)
		status = mesh_crypto_aes_ccm_decrypt(decrypt->nonce,
				app_key->current.key,
				decrypt->aad, decrypt->aad ? 16 : 0,
				decrypt->trans, decrypt->len,
				decrypt->out_msg, NULL, mic_size);

	/* Test and decrypt against new key copy */
	if (!status && app_key->new.akf_aid == decrypt->akf_aid)
		status = mesh_crypto_aes_ccm_decrypt(decrypt->nonce,
				app_key->new.key,
				decrypt->aad, decrypt->aad ? 16 : 0,
				decrypt->trans, decrypt->len,
				decrypt->out_msg, NULL, mic_size);

	/* If successful, terminate with successful App IDX */
	if (status)
		decrypt->app_idx = app_key->generic.idx;
}

static uint16_t access_pkt_decrypt(uint8_t *nonce, uint8_t *aad,
		uint16_t net_idx, uint8_t akf_aid, bool szmic,
		uint8_t *trans, uint16_t len)
{
	uint8_t *out_msg;
	struct decrypt_params decrypt = {
		.nonce = nonce,
		.aad = aad,
		.net_idx = net_idx,
		.akf_aid = akf_aid,
		.szmic = szmic,
		.trans = trans,
		.len = len,
		.app_idx = APP_IDX_INVALID,
	};

	out_msg = g_malloc(len);

	if (out_msg == NULL)
		return false;

	decrypt.out_msg = out_msg;

	g_list_foreach(app_keys, try_decrypt, &decrypt);

	if (decrypt.app_idx != APP_IDX_INVALID)
		memcpy(trans, out_msg, len);

	g_free(out_msg);

	return decrypt.app_idx;
}

static bool access_rxed(uint8_t *nonce, uint16_t net_idx,
		uint32_t iv_index, uint32_t seq_num,
		uint16_t src, uint16_t dst,
		uint8_t akf_aid, bool szmic, uint8_t *trans, uint16_t len)
{
	uint16_t app_idx = access_pkt_decrypt(nonce, NULL,
			net_idx, akf_aid, szmic, trans, len);

	if (app_idx != APP_IDX_INVALID) {
		len -= szmic ? sizeof(uint64_t) : sizeof(uint32_t);

		node_local_data_handler(src, dst, iv_index, seq_num,
				app_idx, trans, len);
		return true;
	}

	return false;
}

static void try_virt_decrypt(gpointer data, gpointer user_data)
{
	struct mesh_virt_addr *virt = data;
	struct decrypt_params *decrypt = user_data;

	if (decrypt->app_idx != APP_IDX_INVALID || decrypt->dst != virt->va16)
		return;

	decrypt->app_idx = access_pkt_decrypt(decrypt->nonce,
			virt->va128,
			decrypt->net_idx, decrypt->akf_aid,
			decrypt->szmic, decrypt->trans, decrypt->len);

	if (decrypt->app_idx != APP_IDX_INVALID) {
		uint16_t len = decrypt->len;

		len -= decrypt->szmic ? sizeof(uint64_t) : sizeof(uint32_t);

		node_local_data_handler(decrypt->src, virt->va32,
				decrypt->iv_index, decrypt->seq_num,
				decrypt->app_idx, decrypt->trans, len);
	}
}

static bool virtual_rxed(uint8_t *nonce, uint16_t net_idx,
		uint32_t iv_index, uint32_t seq_num,
		uint16_t src, uint16_t dst,
		uint8_t akf_aid, bool szmic, uint8_t *trans, uint16_t len)
{
	struct decrypt_params decrypt = {
		.nonce = nonce,
		.net_idx = net_idx,
		.iv_index = iv_index,
		.seq_num = seq_num,
		.src = dst,
		.dst = dst,
		.akf_aid = akf_aid,
		.szmic = szmic,
		.trans = trans,
		.len = len,
		.app_idx = APP_IDX_INVALID,
	};

	/* Cycle through known virtual addresses */
	g_list_foreach(virt_addrs, try_virt_decrypt, &decrypt);

	if (decrypt.app_idx != APP_IDX_INVALID)
		return true;

	return false;
}

static bool msg_rxed(uint16_t net_idx, uint32_t iv_index, bool szmic,
		uint8_t ttl, uint32_t seq_num, uint32_t seq_auth,
		uint16_t src, uint16_t dst,
		uint8_t *trans, uint16_t len)
{
	uint8_t akf_aid = TRANS_AKF_AID(trans);
	bool result;
	size_t mic_size = szmic ? sizeof(uint64_t) : sizeof(uint32_t);
	uint8_t nonce[13];
	uint8_t *dev_key;
	uint8_t *out = NULL;

	if (!TRANS_AKF(trans)) {
		/* Compose Nonce */
		result = mesh_crypto_device_nonce(seq_auth, src, dst,
				iv_index, szmic, nonce);

		if (!result) return false;

		out = g_malloc0(TRANS_LEN(trans, len));
		if (out == NULL) return false;

		/* If we are provisioner, we probably RXed on remote Dev Key */
		if (net.provisioner) {
			dev_key = node_get_device_key(node_find_by_addr(src));

			if (dev_key == NULL)
				goto local_dev_key;
		} else
			goto local_dev_key;

		result = mesh_crypto_aes_ccm_decrypt(nonce, dev_key,
				NULL, 0,
				TRANS_PAYLOAD(trans), TRANS_LEN(trans, len),
				out, NULL, mic_size);

		if (result) {
			node_local_data_handler(src, dst,
					iv_index, seq_num, APP_IDX_DEV,
					out, TRANS_LEN(trans, len) - mic_size);
			goto done;
		}

local_dev_key:
		/* Always fallback to the local Dev Key */
		dev_key = node_get_device_key(node_get_local_node());

		if (dev_key == NULL)
			goto done;

		result = mesh_crypto_aes_ccm_decrypt(nonce, dev_key,
				NULL, 0,
				TRANS_PAYLOAD(trans), TRANS_LEN(trans, len),
				out, NULL, mic_size);

		if (result) {
			node_local_data_handler(src, dst,
					iv_index, seq_num, APP_IDX_DEV,
					out, TRANS_LEN(trans, len) - mic_size);
			goto done;
		}

		goto done;
	}

	result = mesh_crypto_application_nonce(seq_auth, src, dst,
			iv_index, szmic, nonce);

	if (!result) goto done;

	/* If Virtual destination wrap the Access decoder with Virtual */
	if (IS_VIRTUAL(dst)) {
		result = virtual_rxed(nonce, net_idx, iv_index, seq_num,
				src, dst, akf_aid, szmic,
				TRANS_PAYLOAD(trans), TRANS_LEN(trans, len));
		goto done;
	}

	/* Try all matching App Keys until success or exhaustion */
	result = access_rxed(nonce, net_idx, iv_index, seq_num,
			src, dst, akf_aid, szmic,
			TRANS_PAYLOAD(trans), TRANS_LEN(trans, len));

done:
	if (out != NULL)
		g_free(out);

	return result;
}

static void send_sar_ack(struct mesh_sar_msg *sar)
{
	uint8_t ack[7];

	sar->activity_cnt = 0;

	memset(ack, 0, sizeof(ack));
	SET_TRANS_OPCODE(ack, NET_OP_SEG_ACKNOWLEDGE);
	SET_TRANS_SEQ0(ack, sar->seqAuth);
	SET_TRANS_ACK(ack, sar->ack);

	net_ctl_msg_send(0xff, sar->dst, sar->src, ack, sizeof(ack));
}

static gboolean sar_out_ack_timeout(void *user_data)
{
	struct mesh_sar_msg *sar = user_data;

	sar->activity_cnt++;

	/* Because we are GATT, and slow, only resend PKTs if it is
	 * time *and* our outbound PKT queue is empty.  */
	if (net.pkt_out == NULL)
		resend_segs(sar);

	/* Only add resent SAR pkts to empty queue */
	return true;
}

static gboolean sar_out_msg_timeout(void *user_data)
{
	struct mesh_sar_msg *sar = user_data;

	/* msg_to will expire when we return false */
	sar->msg_to = 0;

	flush_sar(&net.msg_out, sar);

	return false;
}

static gboolean sar_in_ack_timeout(void *user_data)
{
	struct mesh_sar_msg *sar = user_data;
	uint32_t full_ack = 0xffffffff >> (31 - sar->segN);

	if (sar->activity_cnt || sar->ack != full_ack)
		send_sar_ack(sar);

	return true;
}

static gboolean sar_in_msg_timeout(void *user_data)
{
	struct mesh_sar_msg *sar = user_data;

	/* msg_to will expire when we return false */
	sar->msg_to = 0;

	flush_sar(&net.sar_in, sar);

	return false;
}

static uint32_t calc_seqAuth(uint32_t seq_num, uint8_t *trans)
{
	uint32_t seqAuth = seq_num & ~0x1fff;

	seqAuth |= TRANS_SEQ0(trans);

	return seqAuth;
}

static bool seg_rxed(uint16_t net_idx, uint32_t iv_index, bool ctl,
		uint8_t ttl, uint32_t seq_num, uint16_t src, uint16_t dst,
		uint8_t *trans, uint16_t len)
{
	struct mesh_sar_msg *sar;
	uint32_t seqAuth = calc_seqAuth(seq_num, trans);
	uint8_t segN, segO;
	uint32_t old_ack, full_ack, last_ack_mask;
	bool send_ack, result = false;

	segN = TRANS_SEGN(trans);
	segO = TRANS_SEGO(trans);

	/* Only support single incoming SAR'd message per SRC */
	sar = find_sar_in_by_src(src);

	/* Reuse existing SAR structure if appropriate */
	if (sar) {
		uint64_t iv_seqAuth = (uint64_t)iv_index << 32 | seqAuth;
		uint64_t old_iv_seqAuth = (uint64_t)sar->iv_index << 32 |
			sar->seqAuth;
		if (old_iv_seqAuth < iv_seqAuth) {

			flush_sar(&net.sar_in, sar);
			sar = NULL;

		} else if (old_iv_seqAuth > iv_seqAuth) {

			/* New segment is Stale. Silently ignore */
			return false;

		} else if (segN != sar->segN) {

			/* Remote side sent conflicting data: abandon */
			flush_sar(&net.sar_in, sar);
			sar = NULL;

		}
	}

	if (sar == NULL) {
		sar = g_malloc0(sizeof(*sar) + (12 * segN));

		if (sar == NULL)
			return false;

		sar->net_idx = net_idx;
		sar->iv_index = iv_index;
		sar->ctl = ctl;
		sar->ttl = ttl;
		sar->seqAuth = seqAuth;
		sar->src = src;
		sar->dst = dst;
		sar->segmented = true;
		sar->szmic = TRANS_SZMIC(trans);
		sar->segN = segN;

		/* In all cases, the reassembled packet should begin with the
		 * same first octet of all segments, minus the SEGMENTED flag */
		sar->data[0] = trans[0] & 0x7f;

		net.sar_in = g_list_append(net.sar_in, sar);

		/* Setup expiration timers */
		if (IS_UNICAST(dst))
			sar->ack_to = g_timeout_add(5000,
					sar_in_ack_timeout, sar);

		sar->msg_to = g_timeout_add(60000, sar_in_msg_timeout, sar);
	}

	/* If last segment, calculate full msg size */
	if (segN == segO)
		sar->len = (segN * 12) + len - 3;

	/* Copy to correct offset */
	memcpy(sar->data + 1 + (12 * segO), trans + 4, 12);

	full_ack = 0xffffffff >> (31 - segN);
	last_ack_mask = 0xffffffff << segO;
	old_ack = sar->ack;
	sar->ack |= 1 << segO;
	send_ack = false;

	/* Determine if we should forward message */
	if (sar->ack == full_ack && old_ack != full_ack) {

		/* First time we have seen this complete message */
		send_ack = true;

		if (ctl)
			result = ctl_rxed(sar->net_idx, sar->iv_index,
					sar->ttl, sar->seqAuth, sar->src,
					sar->dst, sar->data, sar->len);
		else
			result = msg_rxed(sar->net_idx, sar->iv_index,
					sar->szmic, sar->ttl,
					seq_num, sar->seqAuth, sar->src,
					sar->dst, sar->data, sar->len);
	}

	/* Never Ack Group addressed SAR messages */
	if (!IS_UNICAST(dst))
		return result;

	/* Tickle the ACK system so it knows we are still RXing segments */
	sar->activity_cnt++;

	/* Determine if we should ACK */
	if (old_ack == sar->ack)
		/* Let the timer generate repeat ACKs as needed */
		send_ack = false;
	else if ((last_ack_mask & sar->ack) == (last_ack_mask & full_ack))
		/* If this was largest segO outstanding segment, we ACK */
		send_ack = true;

	if (send_ack)
		send_sar_ack(sar);

	return result;
}

bool net_data_ready(uint8_t *msg, uint8_t len)
{
	uint8_t type = *msg++;
	uint32_t iv_index = net.iv_index;
	struct mesh_net_key *net_key;

	if (len-- < 10) return false;

	if (type == PROXY_MESH_BEACON)
		return process_beacon(msg, len);
	else if (type > PROXY_CONFIG_PDU)
		return false;

	/* RXed iv_index must be equal or 1 less than local iv_index */
	/* With the clue being high-order bit of first octet */
	if (!!(iv_index & 0x01) != !!(msg[0] & 0x80)) {
		if (iv_index)
			iv_index--;
		else
			return false;
	}

	net_key = net_packet_decode(type == PROXY_CONFIG_PDU,
			iv_index, msg, len);

	if (net_key == NULL)
		return false;

	/* CTL packets have 64 bit network MIC, otherwise 32 bit MIC */
	len -= PKT_CTL(msg) ? sizeof(uint64_t) : sizeof(uint32_t);

	if (type == PROXY_CONFIG_PDU) {

		/* Proxy Configuration DST messages must be 0x0000 */
		if (PKT_DST(msg))
			return false;

		return proxy_ctl_rxed(net_key->generic.idx,
				iv_index, PKT_TTL(msg), PKT_SEQ(msg),
				PKT_SRC(msg), PKT_DST(msg),
				PKT_TRANS(msg), PKT_TRANS_LEN(len));

	} if (PKT_CTL(msg) && PKT_OPCODE(msg) == NET_OP_SEG_ACKNOWLEDGE) {

		return ack_rxed(false, PKT_SRC(msg), PKT_DST(msg),
				PKT_OBO(msg), PKT_SEQ0(msg), PKT_ACK(msg));

	} else if (PKT_SEGMENTED(msg)) {

		return seg_rxed(net_key->generic.idx, iv_index, PKT_CTL(msg),
				PKT_TTL(msg), PKT_SEQ(msg),
				PKT_SRC(msg), PKT_DST(msg),
				PKT_TRANS(msg), PKT_TRANS_LEN(len));

	} else if (!PKT_CTL(msg)){

		return msg_rxed(net_key->generic.idx,
				iv_index, false, PKT_TTL(msg), PKT_SEQ(msg),
				PKT_SEQ(msg), PKT_SRC(msg), PKT_DST(msg),
				PKT_TRANS(msg), PKT_TRANS_LEN(len));
	} else {

		return ctl_rxed(net_key->generic.idx,
				iv_index, PKT_TTL(msg), PKT_SEQ(msg),
				PKT_SRC(msg), PKT_DST(msg),
				PKT_TRANS(msg), PKT_TRANS_LEN(len));

	}

	return false;
}

bool net_session_open(GDBusProxy *data_in, bool provisioner,
					net_mesh_session_open_callback cb)
{
	if (net.proxy_in)
		return false;

	net.proxy_in = data_in;
	net.iv_upd_state = IV_UPD_INIT;
	net.blacklist = false;
	net.provisioner = provisioner;
	net.open_cb = cb;
	flush_pkt_list(&net.pkt_out);
	return true;
}

void net_session_close(GDBusProxy *data_in)
{
	if (net.proxy_in == data_in)
		net.proxy_in = NULL;

	flush_sar_list(&net.sar_in);
	flush_sar_list(&net.msg_out);
	flush_pkt_list(&net.pkt_out);
}

bool net_register_unicast(uint16_t unicast, uint8_t count)
{
	/* TODO */
	return true;
}

bool net_register_group(uint16_t group_addr)
{
	/* TODO */
	return true;
}

uint32_t net_register_virtual(uint8_t buf[16])
{
	/* TODO */
	return 0;
}

static bool get_enc_keys(uint16_t app_idx, uint16_t dst,
		uint8_t *akf_aid, uint8_t **app_enc_key,
		uint16_t *net_idx)
{
	if (app_idx == APP_IDX_DEV) {
		struct mesh_node *node;
		uint8_t *enc_key = NULL;

		if (net.provisioner) {
			/* Default to Remote Device Key when Provisioner */
			node = node_find_by_addr(dst);
			enc_key = node_get_device_key(node);
		}

		if (enc_key == NULL) {
			/* Use Local node Device Key */
			node = node_get_local_node();
			enc_key = node_get_device_key(node);
		}

		if (enc_key == NULL || node == NULL)
			return false;

		if (akf_aid) *akf_aid = 0;
		if (app_enc_key) *app_enc_key = enc_key;
		if (net_idx) *net_idx = node_get_primary_net_idx(node);

	} else {
		struct mesh_app_key *app_key = find_app_key_by_idx(app_idx);
		struct mesh_net_key *net_key;
		bool phase_two;


		if (app_key == NULL)
			return false;

		net_key = find_net_key_by_idx(app_key->net_idx);

		if (net_key == NULL)
			return false;

		if (net_idx) *net_idx = app_key->net_idx;

		phase_two = !!(net_key->phase == 2);

		if (phase_two && app_key->new.akf_aid != 0xff) {
			if (app_enc_key) *app_enc_key = app_key->new.key;
			if (akf_aid) *akf_aid = app_key->new.akf_aid;
		} else {
			if (app_enc_key) *app_enc_key = app_key->current.key;
			if (akf_aid) *akf_aid = app_key->current.akf_aid;
		}
	}

	return true;
}

bool net_ctl_msg_send(uint8_t ttl, uint16_t src, uint16_t dst,
					uint8_t *buf, uint16_t len)
{
	struct mesh_node *node = node_get_local_node();
	struct mesh_sar_msg sar_ctl;

	/* For simplicity, we will reject segmented OB CTL messages */
	if (len > 12 || node == NULL || buf == NULL || buf[0] & 0x80)
		return false;

	if (!src) {
		src = node_get_primary(node);

		if (!src)
			return false;
	}

	if (ttl == 0xff)
		ttl = net.default_ttl;

	memset(&sar_ctl, 0, sizeof(sar_ctl));

	if (!dst)
		sar_ctl.proxy = true;

	/* Get the default net_idx for remote device (or local) */
	get_enc_keys(APP_IDX_DEV, dst, NULL, NULL, &sar_ctl.net_idx);
	sar_ctl.ctl = true;
	sar_ctl.iv_index = net.iv_index - net.iv_update;
	sar_ctl.ttl = ttl;
	sar_ctl.src = src;
	sar_ctl.dst = dst;
	sar_ctl.len = len;
	memcpy(sar_ctl.data, buf, len);
	send_seg(&sar_ctl, 0);

	return true;
}

bool net_access_layer_send(uint8_t ttl, uint16_t src, uint32_t dst,
				uint16_t app_idx, uint8_t *buf, uint16_t len)
{
	struct mesh_node *node = node_get_local_node();
	struct mesh_sar_msg *sar;
	uint8_t *app_enc_key = NULL;
	uint8_t *aad = NULL;
	uint32_t mic32;
	uint8_t aad_len = 0;
	uint8_t i, j, ackless_retries = 0;
	uint8_t segN, akf_aid;
	uint16_t net_idx;
	bool result;

	if (len > 384 || node == NULL)
		return false;

	if (!src)
		src = node_get_primary(node);

	if (!src || !dst)
		return false;

	if (ttl == 0xff)
		ttl = net.default_ttl;

	if (IS_VIRTUAL(dst)) {
		struct mesh_virt_addr *virt = find_virt_by_dst(dst);

		if (virt == NULL)
			return false;

		dst = virt->va16;
		aad = virt->va128;
		aad_len = sizeof(virt->va128);
	}

	result = get_enc_keys(app_idx, dst,
			&akf_aid, &app_enc_key, &net_idx);

	if (!result)
		return false;

	segN = SEG_MAX(len + sizeof(mic32));

	/* Only one ACK required SAR message per destination at a time */
	if (segN && IS_UNICAST(dst)) {
		sar = find_sar_out_by_dst(dst);

		if (sar)
			flush_sar(&net.msg_out, sar);
	}

	sar = g_malloc0(sizeof(struct mesh_sar_msg) + (segN * 12));

	if (sar == NULL)
		return false;

	if (segN)
		sar->segmented = true;

	sar->ttl = ttl;
	sar->segN = segN;
	sar->seqAuth = net.seq_num;
	sar->iv_index = net.iv_index - net.iv_update;
	sar->net_idx = net_idx;
	sar->src = src;
	sar->dst = dst;
	sar->akf_aid = akf_aid;
	sar->len = len + sizeof(uint32_t);

	mesh_crypto_application_encrypt(akf_aid,
			sar->seqAuth, src,
			dst, sar->iv_index,
			app_enc_key,
			aad, aad_len,
			buf, len,
			sar->data, &mic32,
			sizeof(uint32_t));

	/* If sending as a segmented message to a non-Unicast (thus non-ACKing)
	 * destination, send each segments multiple times. */
	if (!IS_UNICAST(dst) && segN)
		ackless_retries = 4;

	for (j = 0; j <= ackless_retries; j++) {
		for (i = 0; i <= segN; i++)
			send_seg(sar, i);
	}

	if (IS_UNICAST(dst) && segN) {
		net.msg_out = g_list_append(net.msg_out, sar);
		sar->ack_to = g_timeout_add(2000, sar_out_ack_timeout, sar);
		sar->msg_to = g_timeout_add(60000, sar_out_msg_timeout, sar);
	} else
		g_free(sar);

	return true;
}

bool net_set_default_ttl(uint8_t ttl)
{
	if (ttl > 0x7f)
		return false;

	net.default_ttl = ttl;
	return true;
}

uint8_t net_get_default_ttl()
{
	return net.default_ttl;
}

bool net_set_seq_num(uint32_t seq_num)
{
	if (seq_num > 0xffffff)
		return false;

	net.seq_num = seq_num;
	return true;
}

uint32_t net_get_seq_num()
{
	return net.seq_num;
}

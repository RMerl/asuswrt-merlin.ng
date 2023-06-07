// SPDX-License-Identifier: LGPL-2.1-or-later
/*
 *
 *  BlueZ - Bluetooth protocol stack for Linux
 *
 *  Copyright (C) 2018-2020  Intel Corporation. All rights reserved.
 *
 *
 */

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include <sys/time.h>
#include <ell/ell.h>

#include "mesh/mesh-defs.h"
#include "mesh/node.h"
#include "mesh/net.h"
#include "mesh/appkey.h"
#include "mesh/model.h"
#include "mesh/mesh-config.h"
#include "mesh/cfgmod.h"

#define CREDFLAG_MASK	0x1000

#define CFG_GET_ID(vendor, pkt)	((vendor) ?	\
		(SET_ID(l_get_le16((pkt)), l_get_le16((pkt) + 2))) :	\
		(SET_ID(SIG_VENDOR, l_get_le16(pkt))))

/* Supported composition pages, sorted high to low */
/* Only page 0 is currently supported */
static const uint8_t supported_pages[] = {
	0
};

static uint8_t msg[MAX_MSG_LEN];

static uint16_t set_pub_status(uint8_t status, uint16_t ele_addr, uint32_t id,
				uint16_t pub_addr, uint16_t idx, bool cred_flag,
				uint8_t ttl, uint8_t period, uint8_t rtx)
{
	size_t n;

	n = mesh_model_opcode_set(OP_CONFIG_MODEL_PUB_STATUS, msg);
	msg[n++] = status;
	l_put_le16(ele_addr, msg + n);
	l_put_le16(pub_addr, msg + n + 2);
	idx |= cred_flag ? CREDFLAG_MASK : 0;
	l_put_le16(idx, msg + n + 4);
	n += 6;
	msg[n++] = ttl;
	msg[n++] = period;
	msg[n++] = rtx;

	if (!IS_VENDOR(id)) {
		l_put_le16(MODEL_ID(id), msg + n);
		n += 2;
	} else {
		l_put_le16(VENDOR_ID(id), msg + n);
		l_put_le16(MODEL_ID(id), msg + n + 2);
		n += 4;
	}

	return n;
}

static uint16_t config_pub_get(struct mesh_node *node, const uint8_t *pkt,
								bool vendor)
{
	uint32_t id;
	uint16_t ele_addr;
	uint8_t rtx;
	struct mesh_model_pub *pub;
	int status;

	ele_addr = l_get_le16(pkt);
	id = CFG_GET_ID(vendor, pkt + 2);

	pub = mesh_model_pub_get(node, ele_addr, id, &status);

	if (pub && status == MESH_STATUS_SUCCESS) {
		rtx = pub->rtx.cnt + (((pub->rtx.interval / 50) - 1) << 3);
		return set_pub_status(status, ele_addr, id, pub->addr, pub->idx,
					pub->credential, pub->ttl, pub->period,
					rtx);
	} else
		return set_pub_status(status, ele_addr, id, 0, 0, 0, 0, 0, 0);
}

static uint16_t config_pub_set(struct mesh_node *node, const uint8_t *pkt,
							bool virt, bool vendor)
{
	uint32_t id;
	uint16_t ele_addr, idx, pub_dst;
	const uint8_t *pub_addr;
	uint8_t ttl, period, rtx, cnt, interval;
	int status;
	bool cred_flag;

	ele_addr = l_get_le16(pkt);
	pub_addr = pkt + 2;

	pub_dst = l_get_le16(pub_addr);

	if (!virt && IS_VIRTUAL(pub_dst))
		return 0;

	pkt += (virt ? 14 : 0);

	idx = l_get_le16(pkt + 4);
	if (idx > CREDFLAG_MASK)
		return 0;

	cred_flag = !!(CREDFLAG_MASK & idx);
	idx &= APP_IDX_MASK;
	ttl = pkt[6];
	period = pkt[7];
	rtx = pkt[8];
	id = CFG_GET_ID(vendor, pkt + 9);

	cnt = rtx & 0x7;
	interval = ((rtx >> 3) + 1) * 50;

	status = mesh_model_pub_set(node, ele_addr, id, pub_addr, idx,
					cred_flag, ttl, period, cnt,
					interval, virt, &pub_dst);

	l_debug("pub_set: status %d, ea %4.4x, ota: %4.4x, id: %x, idx: %3.3x",
					status, ele_addr, pub_dst, id, idx);

	if (status != MESH_STATUS_SUCCESS)
		return set_pub_status(status, ele_addr, id, 0, 0, 0, 0, 0, 0);

	if (IS_UNASSIGNED(pub_dst) && !virt) {
		ttl = period = idx = 0;

		/* Remove model publication from config file */
		if (!mesh_config_model_pub_del(node_config_get(node), ele_addr,
						vendor ? id : MODEL_ID(id),
									vendor))
			status = MESH_STATUS_STORAGE_FAIL;
	} else {
		struct mesh_config_pub db_pub = {
			.virt = virt,
			.addr = pub_dst,
			.idx = idx,
			.ttl = ttl,
			.credential = cred_flag,
			.period = period,
			.cnt = cnt,
			.interval = interval
		};

		if (virt)
			memcpy(db_pub.virt_addr, pub_addr, 16);

		/* Save model publication to config file */
		if (!mesh_config_model_pub_add(node_config_get(node), ele_addr,
						vendor ? id : MODEL_ID(id),
						vendor, &db_pub))
			status = MESH_STATUS_STORAGE_FAIL;
	}

	return set_pub_status(status, ele_addr, id, pub_dst, idx, cred_flag,
						ttl, period, rtx);
}

static uint16_t cfg_sub_get_msg(struct mesh_node *node, const uint8_t *pkt,
								uint16_t size)
{
	uint16_t ele_addr, n, sub_len;
	uint32_t id;
	int opcode;
	bool vendor = (size == 6);

	ele_addr = l_get_le16(pkt);
	id = CFG_GET_ID(vendor, pkt + 2);
	opcode = vendor ? OP_CONFIG_VEND_MODEL_SUB_LIST :
						OP_CONFIG_MODEL_SUB_LIST;
	n = mesh_model_opcode_set(opcode, msg);
	memcpy(msg + n + 1, pkt, size);

	msg[n] = mesh_model_sub_get(node, ele_addr, id, msg + n + 1 + size,
					MAX_MSG_LEN - (n + 1 + size), &sub_len);

	if (msg[n] == MESH_STATUS_SUCCESS)
		n += sub_len;

	n += (size + 1);
	return n;
}

static bool save_cfg_sub(struct mesh_node *node, uint16_t ele_addr,
				uint32_t id, bool vendor, const uint8_t *label,
				bool virt, uint16_t grp, uint32_t opcode)
{
	struct mesh_config *cfg = node_config_get(node);
	struct mesh_config_sub db_sub = {
				.virt = virt,
				.addr.grp = grp
				};

	id = (vendor) ? id : MODEL_ID(id);

	if (virt)
		memcpy(db_sub.addr.label, label, 16);

	if (opcode == OP_CONFIG_MODEL_SUB_VIRT_DELETE ||
			opcode == OP_CONFIG_MODEL_SUB_DELETE)
		return mesh_config_model_sub_del(cfg, ele_addr, id, vendor,
								&db_sub);

	if (opcode == OP_CONFIG_MODEL_SUB_VIRT_OVERWRITE ||
					opcode == OP_CONFIG_MODEL_SUB_OVERWRITE)

		if (!mesh_config_model_sub_del_all(cfg, ele_addr, id, vendor))
			return false;

	return mesh_config_model_sub_add(cfg, ele_addr, id, vendor, &db_sub);
}

static uint16_t cfg_sub_add_msg(struct mesh_node *node, const uint8_t *pkt,
					bool vendor, uint32_t opcode)
{
	uint16_t addr, ele_addr, n;
	uint32_t id;

	addr = l_get_le16(pkt + 2);

	if (!IS_GROUP(addr))
		return 0;

	ele_addr = l_get_le16(pkt);
	id = CFG_GET_ID(vendor, pkt + 4);

	n = mesh_model_opcode_set(OP_CONFIG_MODEL_SUB_STATUS, msg);

	if (opcode == OP_CONFIG_MODEL_SUB_OVERWRITE)
		msg[n] = mesh_model_sub_ovrt(node, ele_addr, id, addr);
	else if (opcode == OP_CONFIG_MODEL_SUB_ADD)
		msg[n] = mesh_model_sub_add(node, ele_addr, id, addr);
	else
		msg[n] = mesh_model_sub_del(node, ele_addr, id, addr);

	if (msg[n] == MESH_STATUS_SUCCESS &&
			!save_cfg_sub(node, ele_addr, id, vendor, NULL, false,
								addr, opcode))
		msg[n] = MESH_STATUS_STORAGE_FAIL;

	if (vendor) {
		memcpy(msg + n + 1, pkt, 8);
		n += 9;
	} else {
		memcpy(msg + n + 1, pkt, 6);
		n += 7;
	}

	return n;
}

static uint16_t cfg_virt_sub_add_msg(struct mesh_node *node, const uint8_t *pkt,
						bool vendor, uint32_t opcode)
{
	uint16_t addr, ele_addr, n;
	uint32_t id;
	const uint8_t *label;

	n = mesh_model_opcode_set(OP_CONFIG_MODEL_SUB_STATUS, msg);

	ele_addr = l_get_le16(pkt);
	label = pkt + 2;
	id = CFG_GET_ID(vendor, pkt + 18);

	if (opcode == OP_CONFIG_MODEL_SUB_VIRT_OVERWRITE)
		msg[n] = mesh_model_virt_sub_ovrt(node, ele_addr, id, label,
									&addr);
	else if (opcode == OP_CONFIG_MODEL_SUB_VIRT_ADD)
		msg[n] = mesh_model_virt_sub_add(node, ele_addr, id, label,
									&addr);
	else
		msg[n] = mesh_model_virt_sub_del(node, ele_addr, id, label,
									&addr);

	if (msg[n] == MESH_STATUS_SUCCESS &&
				!save_cfg_sub(node, ele_addr, id, vendor,
						label, true, addr, opcode))
		msg[n] = MESH_STATUS_STORAGE_FAIL;

	l_put_le16(ele_addr, msg + n + 1);
	l_put_le16(addr, msg + n + 3);

	if (vendor) {
		l_put_le16(VENDOR_ID(id), msg + n + 5);
		l_put_le16(MODEL_ID(id), msg + n + 7);
		n += 9;
	} else {
		l_put_le16(MODEL_ID(id), msg + n + 5);
		n += 7;
	}

	return n;
}

static uint16_t config_sub_del_all(struct mesh_node *node, const uint8_t *pkt,
								bool vendor)
{
	uint16_t ele_addr, n, grp = UNASSIGNED_ADDRESS;
	uint32_t id;

	n = mesh_model_opcode_set(OP_CONFIG_MODEL_SUB_STATUS, msg);

	ele_addr = l_get_le16(pkt);
	id = CFG_GET_ID(vendor, pkt + 2);

	msg[n] = mesh_model_sub_del_all(node, ele_addr, id);

	if (msg[n] == MESH_STATUS_SUCCESS) {
		struct mesh_config *cfg = node_config_get(node);

		if (!mesh_config_model_sub_del_all(cfg, ele_addr,
						vendor ? id : MODEL_ID(id),
						vendor))
			msg[n] = MESH_STATUS_STORAGE_FAIL;
	}

	l_put_le16(ele_addr, msg + n + 1);
	l_put_le16(grp, msg + n + 3);

	if (vendor) {
		l_put_le16(VENDOR_ID(id), msg + n + 5);
		l_put_le16(MODEL_ID(id), msg + n + 7);
		n += 9;
	} else {
		l_put_le16(MODEL_ID(id), msg + n + 5);
		n += 7;
	}

	return n;
}

static uint16_t model_app_list(struct mesh_node *node,
					const uint8_t *pkt, uint16_t size)
{
	uint16_t ele_addr, n, bnd_len;
	uint32_t id;
	int opcode;

	opcode = (size == 4) ? OP_MODEL_APP_LIST : OP_VEND_MODEL_APP_LIST;
	ele_addr = l_get_le16(pkt);

	n = mesh_model_opcode_set(opcode, msg);
	memcpy(msg + n + 1, pkt, size);

	id = CFG_GET_ID(size == 6, pkt + 2);

	msg[n] = mesh_model_get_bindings(node, ele_addr, id, msg + n + 1 + size,
					MAX_MSG_LEN - (n + 1 + size), &bnd_len);

	if (msg[n] == MESH_STATUS_SUCCESS)
		n += bnd_len;

	n += (size + 1);
	return n;
}

static uint16_t model_app_bind(struct mesh_node *node, const uint8_t *pkt,
						uint16_t size, bool unbind)
{
	uint16_t ele_addr, idx, n;
	uint32_t id;


	idx = l_get_le16(pkt + 2);
	if (idx > APP_IDX_MAX)
		return 0;

	ele_addr = l_get_le16(pkt);
	id = CFG_GET_ID(size == 8, pkt + 4);

	n = mesh_model_opcode_set(OP_MODEL_APP_STATUS, msg);

	if (unbind)
		msg[n] = mesh_model_binding_del(node, ele_addr, id, idx);
	else
		msg[n] = mesh_model_binding_add(node, ele_addr, id, idx);

	memcpy(msg + n + 1, pkt, size);
	n += (size + 1);

	return n;
}

static uint16_t cfg_relay_msg(struct mesh_node *node, const uint8_t *pkt,
								int opcode)
{
	uint8_t count;
	uint16_t interval;
	uint16_t n;

	if (opcode == OP_CONFIG_RELAY_SET) {
		count = (pkt[1] & 0x7) + 1;
		interval = ((pkt[1] >> 3) + 1) * 10;
		node_relay_mode_set(node, !!pkt[0], count, interval);
	}

	n = mesh_model_opcode_set(OP_CONFIG_RELAY_STATUS, msg);

	msg[n++] = node_relay_mode_get(node, &count, &interval);
	msg[n++] = (count - 1) + ((interval/10 - 1) << 3);

	return n;
}

static uint16_t cfg_key_refresh_phase(struct mesh_node *node,
						const uint8_t *pkt, int opcode)
{
	struct mesh_net *net = node_get_net(node);
	uint16_t n, idx = l_get_le16(pkt);
	uint8_t phase;
	int status;

	n = mesh_model_opcode_set(OP_CONFIG_KEY_REFRESH_PHASE_STATUS, msg);
	status = mesh_net_key_refresh_phase_get(net, idx, &phase);

	if (status == MESH_STATUS_SUCCESS &&
				opcode == OP_CONFIG_KEY_REFRESH_PHASE_SET) {

		if (pkt[2] == KEY_REFRESH_TRANS_TWO) {
			if (phase == KEY_REFRESH_PHASE_TWO)
				goto done;
			else if (phase != KEY_REFRESH_PHASE_ONE)
				return 0;
		}

		status = mesh_net_key_refresh_phase_set(net, idx, pkt[2]);
		l_debug("Set KR Phase: net=%3.3x transition=%d", idx, pkt[2]);

		if (status == MESH_STATUS_SUCCESS)
			mesh_net_key_refresh_phase_get(net, idx, &phase);
	}

done:
	msg[n] = status;
	l_put_le16(idx, &msg[n + 1]);
	msg[n + 3] = (status != MESH_STATUS_SUCCESS) ?
						KEY_REFRESH_PHASE_NONE : phase;

	return n + 4;
}

static uint8_t uint32_to_log(uint32_t value)
{
	uint32_t val = 1;
	uint8_t ret = 1;

	if (!value)
		return 0;
	else if (value > 0x10000)
		return 0xff;

	while (val < value) {
		val <<= 1;
		ret++;
	}

	return ret;
}

static uint16_t hb_subscription_get(struct mesh_node *node, int status)
{
	struct mesh_net *net = node_get_net(node);
	struct mesh_net_heartbeat_sub *sub = mesh_net_get_heartbeat_sub(net);
	struct timeval time_now;
	uint16_t n;

	gettimeofday(&time_now, NULL);
	time_now.tv_sec -= sub->start;

	if (time_now.tv_sec >= (long) sub->period)
		time_now.tv_sec = 0;
	else
		time_now.tv_sec = sub->period - time_now.tv_sec;

	l_debug("Sub Period (Log %2.2x) %d sec", uint32_to_log(time_now.tv_sec),
							(int) time_now.tv_sec);

	n = mesh_model_opcode_set(OP_CONFIG_HEARTBEAT_SUB_STATUS, msg);
	msg[n++] = status;
	l_put_le16(sub->src, msg + n);
	n += 2;
	l_put_le16(sub->dst, msg + n);
	n += 2;
	msg[n++] = uint32_to_log(time_now.tv_sec);
	msg[n++] = uint32_to_log(sub->count);
	msg[n++] = sub->count ? sub->min_hops : 0;
	msg[n++] = sub->max_hops;

	return n;
}

static uint16_t hb_subscription_set(struct mesh_node *node, const uint8_t *pkt)
{
	uint16_t src, dst;
	uint8_t period_log;
	struct mesh_net *net;
	int status;

	src = l_get_le16(pkt);
	dst = l_get_le16(pkt + 2);

	/* SRC must be Unicast, DST can be any legal address except Virtual */
	if ((!IS_UNASSIGNED(src) && !IS_UNICAST(src)) || IS_VIRTUAL(dst))
		return 0;

	period_log = pkt[4];

	if (period_log > 0x11)
		return 0;

	net = node_get_net(node);

	status = mesh_net_set_heartbeat_sub(net, src, dst, period_log);

	return hb_subscription_get(node, status);
}

static uint16_t hb_publication_get(struct mesh_node *node, int status)
{
	struct mesh_net *net = node_get_net(node);
	struct mesh_net_heartbeat_pub *pub = mesh_net_get_heartbeat_pub(net);
	uint16_t n;

	n = mesh_model_opcode_set(OP_CONFIG_HEARTBEAT_PUB_STATUS, msg);
	msg[n++] = status;
	l_put_le16(pub->dst, msg + n);
	n += 2;
	msg[n++] = uint32_to_log(pub->count);
	msg[n++] = uint32_to_log(pub->period);
	msg[n++] = pub->ttl;
	l_put_le16(pub->features, msg + n);
	n += 2;
	l_put_le16(pub->net_idx, msg + n);
	n += 2;

	return n;
}

static uint16_t hb_publication_set(struct mesh_node *node, const uint8_t *pkt)
{
	uint16_t dst, features, net_idx;
	uint8_t period_log, count_log, ttl;
	struct mesh_net *net;
	int status;

	dst = l_get_le16(pkt);
	count_log = pkt[2];
	period_log = pkt[3];
	ttl = pkt[4];

	if (count_log > 0x11 && count_log != 0xff)
		return 0;

	if (period_log > 0x11 || ttl > TTL_MASK || IS_VIRTUAL(dst))
		return 0;

	features = l_get_le16(pkt + 5) & 0xf;
	net_idx = l_get_le16(pkt + 7);

	net = node_get_net(node);

	status = mesh_net_set_heartbeat_pub(net, dst, features, net_idx, ttl,
						count_log, period_log);

	return hb_publication_get(node, status);
}

static void node_reset(void *user_data)
{
	struct mesh_node *node = user_data;

	l_debug("Node Reset");
	node_remove(node);
}

static uint16_t cfg_appkey_msg(struct mesh_node *node, const uint8_t *pkt,
								int opcode)
{
	uint16_t n_idx, a_idx, n;
	struct mesh_net *net = node_get_net(node);

	n_idx = l_get_le16(pkt) & 0xfff;
	a_idx = l_get_le16(pkt + 1) >> 4;

	n = mesh_model_opcode_set(OP_APPKEY_STATUS, msg);

	if (opcode == OP_APPKEY_ADD)
		msg[n] = appkey_key_add(net, n_idx, a_idx, pkt + 3);
	else if (opcode == OP_APPKEY_UPDATE)
		msg[n] = appkey_key_update(net, n_idx, a_idx, pkt + 3);
	else
		msg[n] = appkey_key_delete(net, n_idx, a_idx);

	l_debug("AppKey Command %s: Net_Idx %3.3x, App_Idx %3.3x",
			(msg[n] == MESH_STATUS_SUCCESS) ? "success" : "fail",
								n_idx, a_idx);

	memcpy(msg + n + 1, &pkt[0], 3);

	return n + 4;
}

static uint16_t cfg_netkey_msg(struct mesh_node *node, const uint8_t *pkt,
								int opcode)
{
	uint16_t n_idx, n;
	struct mesh_net *net = node_get_net(node);

	n_idx = l_get_le16(pkt);
	if (n_idx > NET_IDX_MAX)
		return 0;

	n = mesh_model_opcode_set(OP_NETKEY_STATUS, msg);

	if (opcode == OP_NETKEY_ADD)
		msg[n] = mesh_net_add_key(net, n_idx, pkt + 2);
	else if (opcode == OP_NETKEY_UPDATE)
		msg[n] = mesh_net_update_key(net, n_idx, pkt + 2);
	else
		msg[n] = mesh_net_del_key(net, n_idx);

	l_debug("NetKey Command %s: Net_Idx %3.3x",
			(msg[n] == MESH_STATUS_SUCCESS) ? "success" : "fail",
									n_idx);

	memcpy(msg + n + 1, &pkt[0], 2);

	return n + 3;
}

static uint16_t cfg_get_appkeys_msg(struct mesh_node *node, const uint8_t *pkt)
{
	uint16_t n_idx, sz, n;

	n_idx = l_get_le16(pkt);

	n = mesh_model_opcode_set(OP_APPKEY_LIST, msg);
	l_put_le16(n_idx, msg + n + 1);

	msg[n] = appkey_list(node_get_net(node), n_idx, msg + n + 3,
						MAX_MSG_LEN - (n + 3), &sz);

	return n + 3 + sz;
}

static uint16_t cfg_poll_timeout_msg(struct mesh_node *node, const uint8_t *pkt)
{
	uint16_t n, addr = l_get_le16(pkt);
	uint32_t poll_to;

	if (!IS_UNICAST(addr))
		return 0;

	n = mesh_model_opcode_set(OP_CONFIG_POLL_TIMEOUT_STATUS, msg);
	l_put_le16(addr, msg + n);
	n += 2;

	poll_to = mesh_net_friend_timeout(node_get_net(node), addr);
	msg[n++] = poll_to;
	msg[n++] = poll_to >> 8;
	msg[n++] = poll_to >> 16;
	return n;
}

static uint16_t cfg_net_tx_msg(struct mesh_node *node, const uint8_t *pkt,
								int opcode)
{
	uint8_t cnt;
	uint16_t interval, n;
	struct mesh_net *net = node_get_net(node);

	cnt = (pkt[0] & 0x7) + 1;
	interval = ((pkt[0] >> 3) + 1) * 10;

	if (opcode == OP_CONFIG_NETWORK_TRANSMIT_SET &&
			mesh_config_write_net_transmit(node_config_get(node),
							cnt, interval))
		mesh_net_transmit_params_set(net, cnt, interval);

	n = mesh_model_opcode_set(OP_CONFIG_NETWORK_TRANSMIT_STATUS, msg);

	mesh_net_transmit_params_get(net, &cnt, &interval);
	msg[n++] = (cnt - 1) + ((interval/10 - 1) << 3);
	return n;
}

static uint16_t get_composition(struct mesh_node *node, uint8_t page,
								uint8_t *buf)
{
	const uint8_t *comp;
	uint16_t len = 0;
	size_t i;

	for (i = 0; i < sizeof(supported_pages); i++) {
		if (page < supported_pages[i])
			continue;

		page = supported_pages[i];
		comp = node_get_comp(node, page, &len);

		if (!page || len)
			break;
	}

	if (!len)
		return 0;

	*buf++ = page;
	memcpy(buf, comp, len);

	return len + 1;
}

static bool cfg_srv_pkt(uint16_t src, uint16_t dst, uint16_t app_idx,
				uint16_t net_idx, const uint8_t *data,
				uint16_t size, const void *user_data)
{
	struct mesh_node *node = (struct mesh_node *) user_data;
	struct mesh_net *net;
	const uint8_t *pkt = data;
	uint32_t opcode;
	uint16_t n_idx;
	uint8_t state;
	bool virt = false;
	uint16_t n;

	if (app_idx != APP_IDX_DEV_LOCAL)
		return false;

	if (mesh_model_opcode_get(pkt, size, &opcode, &n)) {
		size -= n;
		pkt += n;
	} else
		return false;

	net = node_get_net(node);

	l_debug("CONFIG-SRV-opcode 0x%x size %u idx %3.3x", opcode, size,
								net_idx);

	n = 0;

	switch (opcode) {
	default:
		return false;

	case OP_DEV_COMP_GET:
		if (size != 1)
			return true;

		n = mesh_model_opcode_set(OP_DEV_COMP_STATUS, msg);
		n += get_composition(node, pkt[0], msg + n);

		break;

	case OP_CONFIG_DEFAULT_TTL_SET:
		if (size != 1 || pkt[0] > TTL_MASK || pkt[0] == 1)
			return true;

		node_default_ttl_set(node, pkt[0]);
		/* Fall Through */

	case OP_CONFIG_DEFAULT_TTL_GET:
		if (opcode == OP_CONFIG_DEFAULT_TTL_GET && size != 0)
			return true;

		l_debug("Get/Set Default TTL");

		n = mesh_model_opcode_set(OP_CONFIG_DEFAULT_TTL_STATUS, msg);
		msg[n++] = node_default_ttl_get(node);
		break;

	case OP_CONFIG_MODEL_PUB_VIRT_SET:
		if (size != 25 && size != 27)
			return true;

		virt = true;
		/* Fall Through */

	case OP_CONFIG_MODEL_PUB_SET:
		if (!virt && (size != 11 && size != 13))
			return true;

		n = config_pub_set(node, pkt, virt, size == 13 || size == 27);
		break;

	case OP_CONFIG_MODEL_PUB_GET:
		if (size != 4 && size != 6)
			return true;

		n = config_pub_get(node, pkt, size == 6);
		break;

	case OP_CONFIG_VEND_MODEL_SUB_GET:
		if (size != 6)
			return true;
		/* Fall Through */

	case OP_CONFIG_MODEL_SUB_GET:
		if (size != 4 && opcode == OP_CONFIG_MODEL_SUB_GET)
			return true;

		n = cfg_sub_get_msg(node, pkt, size);
		break;

	case OP_CONFIG_MODEL_SUB_VIRT_OVERWRITE:
	case OP_CONFIG_MODEL_SUB_VIRT_DELETE:
	case OP_CONFIG_MODEL_SUB_VIRT_ADD:
		if (size != 20 && size != 22)
			return true;

		n = cfg_virt_sub_add_msg(node, pkt, size == 22, opcode);
		break;

	case OP_CONFIG_MODEL_SUB_OVERWRITE:
	case OP_CONFIG_MODEL_SUB_DELETE:
	case OP_CONFIG_MODEL_SUB_ADD:
		if (size != 6 && size != 8)
			return true;

		n = cfg_sub_add_msg(node, pkt, size == 8, opcode);
		break;

	case OP_CONFIG_MODEL_SUB_DELETE_ALL:
		if (size != 4 && size != 6)
			return true;

		n = config_sub_del_all(node, pkt, size == 6);
		break;

	case OP_CONFIG_RELAY_SET:
		if (size != 2 || pkt[0] > 0x01)
			return true;
		/* Fall Through */

	case OP_CONFIG_RELAY_GET:
		if (opcode == OP_CONFIG_RELAY_GET && size != 0)
			return true;

		n = cfg_relay_msg(node, pkt, opcode);
		break;

	case OP_CONFIG_NETWORK_TRANSMIT_SET:
		if (size != 1)
			return true;
		/* Fall Through */

	case OP_CONFIG_NETWORK_TRANSMIT_GET:
		if (opcode == OP_CONFIG_NETWORK_TRANSMIT_GET && size != 0)
			return true;

		n = cfg_net_tx_msg(node, pkt, opcode);
		break;

	case OP_CONFIG_PROXY_SET:
		if (size != 1 || pkt[0] > 0x01)
			return true;

		node_proxy_mode_set(node, !!pkt[0]);
		/* Fall Through */

	case OP_CONFIG_PROXY_GET:
		if (opcode == OP_CONFIG_PROXY_GET && size != 0)
			return true;

		n = mesh_model_opcode_set(OP_CONFIG_PROXY_STATUS, msg);

		msg[n++] = node_proxy_mode_get(node);
		l_debug("Get/Set Config Proxy (%d)", msg[n-1]);
		break;

	case OP_NODE_IDENTITY_SET:
		if (size != 3)
			return true;

		/* Currently setting node identity not supported */

		/* Fall Through */

	case OP_NODE_IDENTITY_GET:
		if (opcode == OP_NODE_IDENTITY_GET && size != 2)
			return true;

		n_idx = l_get_le16(pkt);
		if (n_idx > NET_IDX_MAX)
			return true;

		n = mesh_model_opcode_set(OP_NODE_IDENTITY_STATUS, msg);
		msg[n++] = mesh_net_get_identity_mode(net, n_idx, &state);

		l_put_le16(n_idx, msg + n);
		n += 2;

		msg[n++] = state;
		l_debug("Get/Set Config Identity (%d)", state);
		break;

	case OP_CONFIG_BEACON_SET:
		if (size != 1 || pkt[0] > 0x01)
			return true;

		node_beacon_mode_set(node, !!pkt[0]);
		/* Fall Through */

	case OP_CONFIG_BEACON_GET:
		if (opcode == OP_CONFIG_BEACON_GET && size != 0)
			return true;

		n = mesh_model_opcode_set(OP_CONFIG_BEACON_STATUS, msg);

		msg[n++] = node_beacon_mode_get(node);
		l_debug("Get/Set Config Beacon (%d)", msg[n-1]);
		break;

	case OP_CONFIG_FRIEND_SET:
		if (size != 1 || pkt[0] > 0x01)
			return true;

		node_friend_mode_set(node, !!pkt[0]);
		/* Fall Through */

	case OP_CONFIG_FRIEND_GET:
		if (opcode == OP_CONFIG_FRIEND_GET && size != 0)
			return true;

		n = mesh_model_opcode_set(OP_CONFIG_FRIEND_STATUS, msg);

		msg[n++] = node_friend_mode_get(node);
		l_debug("Get/Set Friend (%d)", msg[n-1]);
		break;

	case OP_CONFIG_KEY_REFRESH_PHASE_SET:
		if (size != 3 || (pkt[2] != KEY_REFRESH_TRANS_THREE &&
					pkt[2] != KEY_REFRESH_TRANS_TWO))
			return true;

		/* Fall Through */

	case OP_CONFIG_KEY_REFRESH_PHASE_GET:
		if (size != 2 && opcode == OP_CONFIG_KEY_REFRESH_PHASE_GET)
			return true;

		n = cfg_key_refresh_phase(node, pkt, opcode);
		break;

	case OP_APPKEY_ADD:
	case OP_APPKEY_UPDATE:
		if (size != 19)
			return true;

		/* Fall Through */
	case OP_APPKEY_DELETE:
		if (opcode == OP_APPKEY_DELETE && size != 3)
			return true;

		n = cfg_appkey_msg(node, pkt, opcode);
		break;

	case OP_APPKEY_GET:
		if (size != 2)
			return true;

		n = cfg_get_appkeys_msg(node, pkt);
		break;

	case OP_NETKEY_ADD:
	case OP_NETKEY_UPDATE:
		if (size != 18)
			return true;

		/* Fall Through */
	case OP_NETKEY_DELETE:
		if (opcode == OP_NETKEY_DELETE && size != 2)
			return true;

		n = cfg_netkey_msg(node, pkt, opcode);
		break;

	case OP_NETKEY_GET:
		if (size != 0)
			return true;

		n = mesh_model_opcode_set(OP_NETKEY_LIST, msg);
		size = MAX_MSG_LEN - n;

		if (mesh_net_key_list_get(net, msg + n, &size))
			n += size;
		break;

	case OP_MODEL_APP_BIND:
	case OP_MODEL_APP_UNBIND:
		if (size != 6 && size != 8)
			return true;

		n = model_app_bind(node, pkt, size,
						opcode != OP_MODEL_APP_BIND);
		break;

	case OP_VEND_MODEL_APP_GET:
		if (size != 6)
			return true;

		n = model_app_list(node, pkt, size);
		break;

	case OP_MODEL_APP_GET:
		if (size != 4)
			return true;

		n = model_app_list(node, pkt, size);
		break;

	case OP_CONFIG_HEARTBEAT_PUB_SET:
		l_debug("Config Heartbeat Publication Set");
		if (size != 9)
			return true;

		n = hb_publication_set(node, pkt);
		break;

	case OP_CONFIG_HEARTBEAT_PUB_GET:
		if (size != 0)
			return true;

		n = hb_publication_get(node, MESH_STATUS_SUCCESS);
		break;

	case OP_CONFIG_HEARTBEAT_SUB_SET:
		if (size != 5)
			return true;

		l_debug("Set HB Sub Period Log %2.2x", pkt[4]);

		n = hb_subscription_set(node, pkt);
		break;

	case OP_CONFIG_HEARTBEAT_SUB_GET:

		if (size != 0)
			return true;

		n = hb_subscription_get(node, MESH_STATUS_SUCCESS);
		break;

	case OP_CONFIG_POLL_TIMEOUT_GET:
		if (size != 2)
			return true;

		n = cfg_poll_timeout_msg(node, pkt);
		break;

	case OP_NODE_RESET:
		if (size != 0)
			return true;

		n = mesh_model_opcode_set(OP_NODE_RESET_STATUS, msg);

		/* Delay node removal to give it a chance to send the status */
		l_idle_oneshot(node_reset, node, NULL);
		break;
	}

	if (n)
		mesh_model_send(node, dst, src, APP_IDX_DEV_LOCAL, net_idx,
						DEFAULT_TTL, false, n, msg);

	return true;
}

static void cfgmod_srv_unregister(void *user_data)
{
}

static const struct mesh_model_ops ops = {
	.unregister = cfgmod_srv_unregister,
	.recv = cfg_srv_pkt,
	.bind = NULL,
	.sub = NULL,
	.pub = NULL
};

void cfgmod_server_init(struct mesh_node *node, uint8_t ele_idx)
{
	l_debug("%2.2x", ele_idx);
	mesh_model_register(node, ele_idx, CONFIG_SRV_MODEL, &ops, node);
}

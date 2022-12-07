// SPDX-License-Identifier: LGPL-2.1-or-later
/*
 *
 *  BlueZ - Bluetooth protocol stack for Linux
 *
 *  Copyright (C) 2011-2012  Intel Corporation
 *  Copyright (C) 2004-2010  Marcel Holtmann <marcel@holtmann.org>
 *
 *
 */

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include <stdio.h>
#include <ctype.h>
#include <stdlib.h>
#include <string.h>
#include <alloca.h>
#include <sys/uio.h>
#include <stdint.h>
#include <errno.h>

#include "lib/bluetooth.h"
#include "lib/hci.h"

#include "src/shared/util.h"
#include "src/shared/timeout.h"
#include "src/shared/crypto.h"
#include "src/shared/ecc.h"
#include "src/shared/queue.h"
#include "monitor/bt.h"
#include "btdev.h"

#define has_bredr(btdev)	(!((btdev)->features[4] & 0x20))
#define has_le(btdev)		(!!((btdev)->features[4] & 0x40))

#define ACL_HANDLE 42
#define ISO_HANDLE 257
#define SCO_HANDLE 257

struct hook {
	btdev_hook_func handler;
	void *user_data;
	enum btdev_hook_type type;
	uint16_t opcode;
};

#define MAX_HOOK_ENTRIES 16

struct btdev_conn {
	uint16_t handle;
	uint8_t  type;
	struct btdev *dev;
	struct btdev_conn *link;
};

struct btdev {
	enum btdev_type type;

	struct queue *conns;

	bool auth_init;
	uint8_t link_key[16];
	uint16_t pin[16];
	uint8_t pin_len;
	uint8_t io_cap;
	uint8_t auth_req;
	bool ssp_auth_complete;
	uint8_t ssp_status;

	btdev_command_func command_handler;
	void *command_data;

	btdev_send_func send_handler;
	void *send_data;

	unsigned int inquiry_id;
	unsigned int inquiry_timeout_id;

	struct hook *hook_list[MAX_HOOK_ENTRIES];

	struct bt_crypto *crypto;

        uint16_t manufacturer;
        uint8_t  version;
	uint16_t revision;
	uint8_t  commands[64];
	uint8_t  max_page;
	uint8_t  features[8];
	uint8_t  feat_page_2[8];
	uint16_t acl_mtu;
	uint16_t acl_max_pkt;
	uint16_t iso_mtu;
	uint16_t iso_max_pkt;
	uint8_t  country_code;
	uint8_t  bdaddr[6];
	uint8_t  random_addr[6];
	uint8_t  le_features[8];
	uint8_t  le_states[8];
	const struct btdev_cmd *cmds;

	uint16_t default_link_policy;
	uint8_t  event_mask[8];
	uint8_t  event_mask_page2[8];
	uint8_t  event_filter;
	uint8_t  name[248];
	uint8_t  dev_class[3];
	uint16_t voice_setting;
	uint16_t conn_accept_timeout;
	uint16_t page_timeout;
	uint8_t  scan_enable;
	uint16_t page_scan_interval;
	uint16_t page_scan_window;
	uint16_t page_scan_type;
	uint8_t  auth_enable;
	uint16_t inquiry_scan_interval;
	uint16_t inquiry_scan_window;
	uint8_t  inquiry_mode;
	uint8_t  afh_assessment_mode;
	uint8_t  ext_inquiry_fec;
	uint8_t  ext_inquiry_rsp[240];
	uint8_t  simple_pairing_mode;
	uint8_t  ssp_debug_mode;
	uint8_t  secure_conn_support;
	uint8_t  host_flow_control;
	uint8_t  le_supported;
	uint8_t  le_simultaneous;
	uint8_t  le_event_mask[8];
	uint8_t  le_adv_data[31];
	uint8_t  le_adv_data_len;
	uint8_t  le_adv_type;
	uint8_t  le_adv_own_addr;
	uint8_t  le_adv_direct_addr_type;
	uint8_t  le_adv_direct_addr[6];
	uint8_t  le_scan_data[31];
	uint8_t  le_scan_data_len;
	uint8_t  le_scan_enable;
	uint8_t  le_scan_type;
	uint8_t  le_scan_own_addr_type;
	uint8_t  le_filter_dup;
	uint8_t  le_adv_enable;
	uint8_t  le_ltk[16];
	struct {
		struct bt_hci_cmd_le_set_cig_params params;
		struct bt_hci_cis_params cis;
	} __attribute__ ((packed)) le_cig;
	uint8_t  le_iso_path[2];

	uint8_t le_local_sk256[32];

	uint16_t sync_train_interval;
	uint32_t sync_train_timeout;
	uint8_t  sync_train_service_data;

	uint16_t le_ext_adv_type;

	btdev_debug_func_t debug_callback;
	btdev_destroy_func_t debug_destroy;
	void *debug_data;
};

struct inquiry_data {
	struct btdev *btdev;
	int num_resp;

	int sent_count;
	int iter;
};

#define DEFAULT_INQUIRY_INTERVAL 100 /* 100 miliseconds */

#define MAX_BTDEV_ENTRIES 16

static const uint8_t LINK_KEY_NONE[16] = { 0 };
static const uint8_t LINK_KEY_DUMMY[16] = {	0, 1, 2, 3, 4, 5, 6, 7,
						8, 9, 0, 1, 2, 3, 4, 5 };

static struct btdev *btdev_list[MAX_BTDEV_ENTRIES] = { };

static int get_hook_index(struct btdev *btdev, enum btdev_hook_type type,
								uint16_t opcode)
{
	int i;

	for (i = 0; i < MAX_HOOK_ENTRIES; i++) {
		if (btdev->hook_list[i] == NULL)
			continue;

		if (btdev->hook_list[i]->type == type &&
					btdev->hook_list[i]->opcode == opcode)
			return i;
	}

	return -1;
}

static bool run_hooks(struct btdev *btdev, enum btdev_hook_type type,
				uint16_t opcode, const void *data, uint16_t len)
{
	int index = get_hook_index(btdev, type, opcode);
	if (index < 0)
		return true;

	return btdev->hook_list[index]->handler(data, len,
					btdev->hook_list[index]->user_data);
}

static inline int add_btdev(struct btdev *btdev)
{
	int i, index = -1;

	for (i = 0; i < MAX_BTDEV_ENTRIES; i++) {
		if (btdev_list[i] == NULL) {
			index = i;
			btdev_list[index] = btdev;
			break;
		}
	}

	return index;
}

static inline int del_btdev(struct btdev *btdev)
{
	int i, index = -1;

	for (i = 0; i < MAX_BTDEV_ENTRIES; i++) {
		if (btdev_list[i] == btdev) {
			index = i;
			btdev_list[index] = NULL;
			break;
		}
	}

	return index;
}

static inline struct btdev *find_btdev_by_bdaddr(const uint8_t *bdaddr)
{
	int i;

	for (i = 0; i < MAX_BTDEV_ENTRIES; i++) {
		if (btdev_list[i] && !memcmp(btdev_list[i]->bdaddr, bdaddr, 6))
			return btdev_list[i];
	}

	return NULL;
}

static inline struct btdev *find_btdev_by_bdaddr_type(const uint8_t *bdaddr,
							uint8_t bdaddr_type)
{
	int i;

	for (i = 0; i < MAX_BTDEV_ENTRIES; i++) {
		int cmp;

		if (!btdev_list[i])
			continue;

		if (bdaddr_type == 0x01)
			cmp = memcmp(btdev_list[i]->random_addr, bdaddr, 6);
		else
			cmp = memcmp(btdev_list[i]->bdaddr, bdaddr, 6);

		if (!cmp)
			return btdev_list[i];
	}

	return NULL;
}

static void get_bdaddr(uint16_t id, uint8_t index, uint8_t *bdaddr)
{
	bdaddr[0] = id & 0xff;
	bdaddr[1] = id >> 8;
	bdaddr[2] = index;
	bdaddr[3] = 0x01;
	bdaddr[4] = 0xaa;
	bdaddr[5] = 0x00;
}

struct btdev_cmd {
	uint16_t opcode;
	int (*func)(struct btdev *dev, const void *data, uint8_t len);
	int (*complete)(struct btdev *dev, const void *data, uint8_t len);
};

#define CMD(_opcode, _func, _complete) \
	{ \
		.opcode = _opcode, \
		.func = _func, \
		.complete = _complete, \
	}

static void send_packet(struct btdev *btdev, const struct iovec *iov,
								int iovlen)
{
	int i;

	if (!btdev->send_handler)
		return;

	for (i = 0; i < iovlen; i++) {
		if (!i)
			util_hexdump('<', iov[i].iov_base, iov[i].iov_len,
				btdev->debug_callback, btdev->debug_data);
		else
			util_hexdump(' ', iov[i].iov_base, iov[i].iov_len,
				btdev->debug_callback, btdev->debug_data);
	}

	btdev->send_handler(iov, iovlen, btdev->send_data);
}

static void send_cmd(struct btdev *btdev, uint8_t evt, uint16_t opcode,
					const struct iovec *iov, int iovlen)
{
	struct bt_hci_evt_hdr hdr;
	struct iovec iov2[2 + iovlen];
	uint8_t pkt = BT_H4_EVT_PKT;
	int i;

	util_debug(btdev->debug_callback, btdev->debug_data,
				"event 0x%02x opcode 0x%04x", evt, opcode);

	iov2[0].iov_base = &pkt;
	iov2[0].iov_len = sizeof(pkt);

	hdr.evt = evt;
	hdr.plen = 0;

	iov2[1].iov_base = &hdr;
	iov2[1].iov_len = sizeof(hdr);

	for (i = 0; i < iovlen; i++) {
		hdr.plen += iov[i].iov_len;
		iov2[2 + i].iov_base = iov[i].iov_base;
		iov2[2 + i].iov_len = iov[i].iov_len;
	}

	if (run_hooks(btdev, BTDEV_HOOK_POST_CMD, opcode, iov[i -1].iov_base,
							iov[i -1].iov_len))
		send_packet(btdev, iov2, 2 + iovlen);
}

static void cmd_complete(struct btdev *btdev, uint16_t opcode,
						const void *data, uint8_t len)
{
	struct bt_hci_evt_cmd_complete cc;
	struct iovec iov[2];

	cc.ncmd = 0x01;
	cc.opcode = cpu_to_le16(opcode);

	iov[0].iov_base = &cc;
	iov[0].iov_len = sizeof(cc);

	iov[1].iov_base = (void *) data;
	iov[1].iov_len = len;

	send_cmd(btdev, BT_HCI_EVT_CMD_COMPLETE, opcode, iov, 2);
}

static int cmd_set_event_mask(struct btdev *dev, const void *data, uint8_t len)
{
	const struct bt_hci_cmd_set_event_mask *cmd = data;
	uint8_t status;

	memcpy(dev->event_mask, cmd->mask, 8);
	status = BT_HCI_ERR_SUCCESS;

	cmd_complete(dev, BT_HCI_CMD_SET_EVENT_MASK, &status, sizeof(status));

	return 0;
}

static void btdev_reset(struct btdev *btdev)
{
	/* FIXME: include here clearing of all states that should be
	 * cleared upon HCI_Reset
	 */

	btdev->le_scan_enable		= 0x00;
	btdev->le_adv_enable		= 0x00;
}

static int cmd_reset(struct btdev *dev, const void *data, uint8_t len)
{
	uint8_t status;

	btdev_reset(dev);
	status = BT_HCI_ERR_SUCCESS;

	cmd_complete(dev, BT_HCI_CMD_RESET, &status, sizeof(status));

	return 0;
}

static int cmd_read_local_version(struct btdev *dev, const void *data,
							uint8_t len)
{
	struct bt_hci_rsp_read_local_version rsp;

	memset(&rsp, 0, sizeof(rsp));

	rsp.status = BT_HCI_ERR_SUCCESS;
	rsp.hci_ver = dev->version;
	rsp.hci_rev = cpu_to_le16(dev->revision);
	rsp.lmp_ver = dev->version;
	rsp.manufacturer = cpu_to_le16(dev->manufacturer);
	rsp.lmp_subver = cpu_to_le16(dev->revision);

	cmd_complete(dev, BT_HCI_CMD_READ_LOCAL_VERSION, &rsp, sizeof(rsp));

	return 0;
}

static int cmd_read_local_commands(struct btdev *dev, const void *data,
							uint8_t len)
{
	struct bt_hci_rsp_read_local_commands rsp;

	rsp.status = BT_HCI_ERR_SUCCESS;
	memcpy(rsp.commands, dev->commands, 64);

	cmd_complete(dev, BT_HCI_CMD_READ_LOCAL_COMMANDS, &rsp, sizeof(rsp));

	return 0;
}

static int cmd_read_local_features(struct btdev *dev, const void *data,
							uint8_t len)
{
	struct bt_hci_rsp_read_local_features rsp;

	rsp.status = BT_HCI_ERR_SUCCESS;
	memcpy(rsp.features, dev->features, 8);

	cmd_complete(dev, BT_HCI_CMD_READ_LOCAL_FEATURES, &rsp, sizeof(rsp));

	return 0;
}

static int cmd_read_buffer_size(struct btdev *dev, const void *data,
							uint8_t len)
{
	struct bt_hci_rsp_read_buffer_size rsp;

	rsp.status = BT_HCI_ERR_SUCCESS;
	rsp.acl_mtu = cpu_to_le16(dev->acl_mtu);
	rsp.sco_mtu = 0;
	rsp.acl_max_pkt = cpu_to_le16(dev->acl_max_pkt);
	rsp.sco_max_pkt = cpu_to_le16(0);

	cmd_complete(dev, BT_HCI_CMD_READ_BUFFER_SIZE, &rsp, sizeof(rsp));

	return 0;
}

#define CMD_COMMON_ALL \
	CMD(BT_HCI_CMD_SET_EVENT_MASK, cmd_set_event_mask, NULL), \
	CMD(BT_HCI_CMD_RESET, cmd_reset, NULL), \
	CMD(BT_HCI_CMD_READ_LOCAL_VERSION, cmd_read_local_version, NULL), \
	CMD(BT_HCI_CMD_READ_LOCAL_COMMANDS, cmd_read_local_commands, NULL), \
	CMD(BT_HCI_CMD_READ_LOCAL_FEATURES, cmd_read_local_features, NULL), \
	CMD(BT_HCI_CMD_READ_BUFFER_SIZE, cmd_read_buffer_size, NULL)

static void set_common_commands_all(struct btdev *btdev)
{
	btdev->commands[5]  |= 0x40;	/* Set Event Mask */
	btdev->commands[5]  |= 0x80;	/* Reset */
	btdev->commands[14] |= 0x08;	/* Read Local Version */
	btdev->commands[14] |= 0x10;	/* Read Local Supported Commands */
	btdev->commands[14] |= 0x20;	/* Read Local Supported Features */
	btdev->commands[14] |= 0x80;	/* Read Buffer Size */
}

static void cmd_status(struct btdev *btdev, uint8_t status, uint16_t opcode)
{
	struct bt_hci_evt_cmd_status cs;
	struct iovec iov;

	cs.status = status;
	cs.ncmd = 0x01;
	cs.opcode = cpu_to_le16(opcode);

	iov.iov_base = &cs;
	iov.iov_len = sizeof(cs);

	send_cmd(btdev, BT_HCI_EVT_CMD_STATUS, opcode, &iov, 1);
}

static int cmd_disconnect(struct btdev *dev, const void *data, uint8_t len)
{
	cmd_status(dev, BT_HCI_ERR_SUCCESS, BT_HCI_CMD_DISCONNECT);

	return 0;
}

static void send_event(struct btdev *btdev, uint8_t event,
						const void *data, uint8_t len)
{
	struct bt_hci_evt_hdr hdr;
	struct iovec iov[3];
	uint8_t pkt = BT_H4_EVT_PKT;

	util_debug(btdev->debug_callback, btdev->debug_data,
				"event 0x%02x", event);

	iov[0].iov_base = &pkt;
	iov[0].iov_len = sizeof(pkt);

	hdr.evt = event;
	hdr.plen = len;

	iov[1].iov_base = &hdr;
	iov[1].iov_len = sizeof(hdr);

	if (len > 0) {
		iov[2].iov_base = (void *) data;
		iov[2].iov_len = len;
	}

	if (run_hooks(btdev, BTDEV_HOOK_POST_EVT, event, data, len))
		send_packet(btdev, iov, len > 0 ? 3 : 2);
}

static bool match_handle(const void *data, const void *match_data)
{
	const struct btdev_conn *conn = data;
	uint16_t handle = PTR_TO_UINT(match_data);

	return conn->handle == handle;
}

static void conn_unlink(struct btdev_conn *conn1, struct btdev_conn *conn2)
{
	conn1->link = NULL;
	conn2->link = NULL;
}

static void conn_remove(struct btdev_conn *conn)
{
	if (conn->link) {
		struct btdev_conn *link = conn->link;

		conn_unlink(conn, conn->link);
		conn_remove(link);
	}

	queue_remove(conn->dev->conns, conn);

	free(conn);
}

static void disconnect_complete(struct btdev *dev, uint16_t handle,
					uint8_t status, uint8_t reason)
{
	struct bt_hci_evt_disconnect_complete rsp;

	memset(&rsp, 0, sizeof(rsp));

	rsp.status = status;
	rsp.handle = cpu_to_le16(handle);
	rsp.reason = reason;

	send_event(dev, BT_HCI_EVT_DISCONNECT_COMPLETE, &rsp, sizeof(rsp));
}

static int cmd_disconnect_complete(struct btdev *dev, const void *data,
						uint8_t len)
{
	const struct bt_hci_cmd_disconnect *cmd = data;
	struct bt_hci_evt_disconnect_complete rsp;
	struct btdev_conn *conn;

	memset(&rsp, 0, sizeof(rsp));

	conn = queue_remove_if(dev->conns, match_handle,
				UINT_TO_PTR(cpu_to_le16(cmd->handle)));
	if (!conn) {
		disconnect_complete(dev, 0x0000, BT_HCI_ERR_UNKNOWN_CONN_ID,
								0x00);
		return 0;
	}

	disconnect_complete(dev, conn->handle, BT_HCI_ERR_SUCCESS, cmd->reason);
	disconnect_complete(conn->link->dev, conn->link->handle,
				BT_HCI_ERR_SUCCESS, cmd->reason);

	conn_remove(conn);

	return 0;
}

static int cmd_remote_version(struct btdev *dev, const void *data, uint8_t len)
{
	cmd_status(dev, BT_HCI_ERR_SUCCESS, BT_HCI_CMD_READ_REMOTE_VERSION);

	return 0;
}

static int cmd_remote_version_complete(struct btdev *dev, const void *data,
							uint8_t len)
{
	const struct bt_hci_cmd_read_remote_version *cmd = data;
	struct bt_hci_evt_remote_version_complete ev;
	struct btdev_conn *conn;

	memset(&ev, 0, sizeof(ev));

	conn = queue_find(dev->conns, match_handle,
				UINT_TO_PTR(cpu_to_le16(cmd->handle)));
	if (conn) {
		ev.status = BT_HCI_ERR_SUCCESS;
		ev.handle = cpu_to_le16(cmd->handle);
		ev.lmp_ver = conn->link->dev->version;
		ev.manufacturer = cpu_to_le16(conn->link->dev->manufacturer);
		ev.lmp_subver = cpu_to_le16(conn->link->dev->revision);
	} else {
		ev.status = BT_HCI_ERR_UNKNOWN_CONN_ID;
		ev.handle = cpu_to_le16(cmd->handle);
		ev.lmp_ver = 0x00;
		ev.manufacturer = cpu_to_le16(0);
		ev.lmp_subver = cpu_to_le16(0);
	}

	send_event(dev, BT_HCI_EVT_REMOTE_VERSION_COMPLETE, &ev, sizeof(ev));

	return 0;
}

static int cmd_set_host_flowctl(struct btdev *dev, const void *data,
							uint8_t len)
{
	const struct bt_hci_cmd_set_host_flow_control *cmd = data;
	uint8_t status;

	if (cmd->enable > 0x03) {
		status = BT_HCI_ERR_INVALID_PARAMETERS;
	} else {
		dev->host_flow_control = cmd->enable;
		status = BT_HCI_ERR_SUCCESS;
	}

	cmd_complete(dev, BT_HCI_CMD_SET_HOST_FLOW_CONTROL, &status,
							sizeof(status));

	return 0;
}

static int cmd_host_buffer_size(struct btdev *dev, const void *data,
							uint8_t len)
{
	uint8_t status = BT_HCI_ERR_SUCCESS;

	cmd_complete(dev, BT_HCI_CMD_HOST_BUFFER_SIZE, &status, sizeof(status));

	return 0;
}

static int cmd_host_num_completed_pkts(struct btdev *dev, const void *data,
							uint8_t len)
{
	/* This command is special in the sense that no event is
	 * normally generated after the command has completed.
	 */
	return 0;
}

static int cmd_read_bdaddr(struct btdev *dev, const void *data, uint8_t len)
{
	struct bt_hci_rsp_read_bd_addr rsp;

	memset(&rsp, 0, sizeof(rsp));

	rsp.status = BT_HCI_ERR_SUCCESS;
	memcpy(rsp.bdaddr, dev->bdaddr, 6);

	cmd_complete(dev, BT_HCI_CMD_READ_BD_ADDR, &rsp, sizeof(rsp));

	return 0;
}

#define CMD_COMMON_BREDR_LE \
	CMD(BT_HCI_CMD_DISCONNECT, cmd_disconnect, cmd_disconnect_complete), \
	CMD(BT_HCI_CMD_READ_REMOTE_VERSION, cmd_remote_version, \
					cmd_remote_version_complete), \
	CMD(BT_HCI_CMD_SET_HOST_FLOW_CONTROL, cmd_set_host_flowctl, NULL), \
	CMD(BT_HCI_CMD_HOST_BUFFER_SIZE, cmd_host_buffer_size, NULL), \
	CMD(BT_HCI_CMD_HOST_NUM_COMPLETED_PACKETS, \
					cmd_host_num_completed_pkts, NULL), \
	CMD(BT_HCI_CMD_READ_BD_ADDR, cmd_read_bdaddr, NULL)

static void set_common_commands_bredrle(struct btdev *btdev)
{
	btdev->commands[0]  |= 0x20;	/* Disconnect */
	btdev->commands[2]  |= 0x80;	/* Read Remote Version Information */
	btdev->commands[10] |= 0x20;    /* Set Host Flow Control */
	btdev->commands[10] |= 0x40;	/* Host Buffer Size */
	btdev->commands[15] |= 0x02;	/* Read BD ADDR */
}

static int cmd_inquiry(struct btdev *dev, const void *data, uint8_t len)
{
	cmd_status(dev, BT_HCI_ERR_SUCCESS, BT_HCI_CMD_INQUIRY);

	return 0;
}

static bool inquiry_callback(void *user_data)
{
	struct inquiry_data *data = user_data;
	struct btdev *btdev = data->btdev;
	struct bt_hci_evt_inquiry_complete ic;
	int sent = data->sent_count;
	int i;

	/*Report devices only once and wait for inquiry timeout*/
	if (data->iter == MAX_BTDEV_ENTRIES)
		return true;

	for (i = data->iter; i < MAX_BTDEV_ENTRIES; i++) {
		/*Lets sent 10 inquiry results at once */
		if (sent + 10 == data->sent_count)
			break;

		if (!btdev_list[i] || btdev_list[i] == btdev)
			continue;

		if (!(btdev_list[i]->scan_enable & 0x02))
			continue;

		if (btdev->inquiry_mode == 0x02 &&
					btdev_list[i]->ext_inquiry_rsp[0]) {
			struct bt_hci_evt_ext_inquiry_result ir;

			ir.num_resp = 0x01;
			memcpy(ir.bdaddr, btdev_list[i]->bdaddr, 6);
			ir.pscan_rep_mode = 0x00;
			ir.pscan_period_mode = 0x00;
			memcpy(ir.dev_class, btdev_list[i]->dev_class, 3);
			ir.clock_offset = 0x0000;
			ir.rssi = -60;
			memcpy(ir.data, btdev_list[i]->ext_inquiry_rsp, 240);

			send_event(btdev, BT_HCI_EVT_EXT_INQUIRY_RESULT,
							&ir, sizeof(ir));
			data->sent_count++;
			continue;
		}

		if (btdev->inquiry_mode > 0x00) {
			struct bt_hci_evt_inquiry_result_with_rssi ir;

			ir.num_resp = 0x01;
			memcpy(ir.bdaddr, btdev_list[i]->bdaddr, 6);
			ir.pscan_rep_mode = 0x00;
			ir.pscan_period_mode = 0x00;
			memcpy(ir.dev_class, btdev_list[i]->dev_class, 3);
			ir.clock_offset = 0x0000;
			ir.rssi = -60;

			send_event(btdev, BT_HCI_EVT_INQUIRY_RESULT_WITH_RSSI,
							&ir, sizeof(ir));
			data->sent_count++;
		} else {
			struct bt_hci_evt_inquiry_result ir;

			ir.num_resp = 0x01;
			memcpy(ir.bdaddr, btdev_list[i]->bdaddr, 6);
			ir.pscan_rep_mode = 0x00;
			ir.pscan_period_mode = 0x00;
			ir.pscan_mode = 0x00;
			memcpy(ir.dev_class, btdev_list[i]->dev_class, 3);
			ir.clock_offset = 0x0000;

			send_event(btdev, BT_HCI_EVT_INQUIRY_RESULT,
							&ir, sizeof(ir));
			data->sent_count++;
		}
	}
	data->iter = i;

	/* Check if we sent already required amount of responses*/
	if (data->num_resp && data->sent_count == data->num_resp)
		goto finish;

	return true;

finish:
	/* Note that destroy will be called */
	ic.status = BT_HCI_ERR_SUCCESS;
	send_event(btdev, BT_HCI_EVT_INQUIRY_COMPLETE, &ic, sizeof(ic));

	return false;
}

static void inquiry_destroy(void *user_data)
{
	struct inquiry_data *data = user_data;
	struct btdev *btdev = data->btdev;

	if (!btdev)
		goto finish;

	btdev->inquiry_id = 0;

	if (btdev->inquiry_timeout_id > 0) {
		timeout_remove(btdev->inquiry_timeout_id);
		btdev->inquiry_timeout_id = 0;
	}

finish:
	free(data);
}

static bool inquiry_timeout(void *user_data)
{
	struct inquiry_data *data = user_data;
	struct btdev *btdev = data->btdev;
	struct bt_hci_evt_inquiry_complete ic;

	timeout_remove(btdev->inquiry_id);
	btdev->inquiry_timeout_id = 0;

	/* Inquiry is stopped, send Inquiry complete event. */
	ic.status = BT_HCI_ERR_SUCCESS;
	send_event(btdev, BT_HCI_EVT_INQUIRY_COMPLETE, &ic, sizeof(ic));

	return false;
}

static int cmd_inquiry_complete(struct btdev *dev, const void *data,
						uint8_t len)
{
	const struct bt_hci_cmd_inquiry *cmd = data;
	struct inquiry_data *idata;
	struct bt_hci_evt_inquiry_complete ic;
	int status = BT_HCI_ERR_HARDWARE_FAILURE;
	unsigned int inquiry_len_ms;

	if (dev->inquiry_id > 0) {
		status = BT_HCI_ERR_COMMAND_DISALLOWED;
		goto failed;
	}

	idata = malloc0(sizeof(*idata));
	if (!idata)
		goto failed;

	idata->btdev = dev;
	idata->num_resp = cmd->num_resp;

	/* Add timeout to cancel inquiry */
	inquiry_len_ms = 1280 * cmd->length;
	if (inquiry_len_ms)
		dev->inquiry_timeout_id = timeout_add(inquiry_len_ms,
							inquiry_timeout,
							idata, NULL);

	dev->inquiry_id = timeout_add(DEFAULT_INQUIRY_INTERVAL,
							inquiry_callback, idata,
							inquiry_destroy);
	/* Return if success */
	if (dev->inquiry_id > 0)
		return 0;

failed:
	ic.status = status;
	send_event(dev, BT_HCI_EVT_INQUIRY_COMPLETE, &ic, sizeof(ic));

	return 0;
}

static int cmd_inquiry_cancel(struct btdev *dev, const void *data, uint8_t len)
{
	uint8_t status;

	if (!dev->inquiry_id) {
		status = BT_HCI_ERR_COMMAND_DISALLOWED;
		goto done;
	}

	timeout_remove(dev->inquiry_timeout_id);
	dev->inquiry_timeout_id = 0;
	timeout_remove(dev->inquiry_id);
	dev->inquiry_id = 0;

	status = BT_HCI_ERR_SUCCESS;

done:
	cmd_complete(dev, BT_HCI_CMD_INQUIRY_CANCEL, &status, sizeof(status));

	return 0;
}

static int cmd_create_conn(struct btdev *dev, const void *data, uint8_t len)
{
	cmd_status(dev, BT_HCI_ERR_SUCCESS, BT_HCI_CMD_CREATE_CONN);

	return 0;
}

static struct btdev_conn *conn_new(struct btdev *dev, uint16_t handle,
							uint8_t type)
{
	struct btdev_conn *conn;

	while ((conn = queue_find(dev->conns, match_handle,
					UINT_TO_PTR(handle))))
		handle++;

	conn = new0(struct btdev_conn, 1);
	conn->handle = handle;
	conn->type = type;
	conn->dev = dev;

	if (!queue_push_tail(dev->conns, conn)) {
		free(conn);
		return NULL;
	}

	return conn;
}

static struct btdev_conn *conn_link(struct btdev *dev, struct btdev *remote,
					uint16_t handle, uint8_t type)
{
	struct btdev_conn *conn1, *conn2;

	conn1 = conn_new(dev, handle, type);
	if (!conn1)
		return NULL;

	conn2 = conn_new(remote, handle, type);
	if (!conn2) {
		free(conn1);
		return NULL;
	}

	conn1->link = conn2;
	conn2->link = conn1;

	util_debug(dev->debug_callback, dev->debug_data,
				"conn1 %p handle 0x%04x conn2 %p handle 0x%04x",
				conn1, conn1->handle, conn2, conn2->handle);

	return conn1;
}

static struct btdev_conn *conn_add(struct btdev *dev,
				const uint8_t *bdaddr, uint8_t bdaddr_type,
				uint16_t handle, uint8_t type)
{
	struct btdev *remote;

	remote = find_btdev_by_bdaddr_type(bdaddr, bdaddr_type);
	if (!remote)
		return NULL;

	return conn_link(dev, remote, handle, type);
}

static struct btdev_conn *conn_add_acl(struct btdev *dev,
				const uint8_t *bdaddr, uint8_t bdaddr_type)
{
	return conn_add(dev, bdaddr, bdaddr_type, ACL_HANDLE, HCI_ACLDATA_PKT);
}

static struct btdev_conn *conn_add_sco(struct btdev_conn *acl)
{
	return conn_link(acl->dev, acl->link->dev, SCO_HANDLE, HCI_SCODATA_PKT);
}

static struct btdev_conn *conn_add_iso(struct btdev_conn *acl, uint16_t handle)
{
	return conn_link(acl->dev, acl->link->dev, handle, HCI_ISODATA_PKT);
}

static void conn_complete(struct btdev *btdev,
					const uint8_t *bdaddr, uint8_t status)
{
	struct bt_hci_evt_conn_complete cc;

	if (!status) {
		struct btdev_conn *conn;

		conn = conn_add_acl(btdev, bdaddr, BDADDR_BREDR);
		if (!conn)
			return;

		cc.status = status;
		memcpy(cc.bdaddr, btdev->bdaddr, 6);
		cc.encr_mode = 0x00;

		cc.handle = cpu_to_le16(conn->link->handle);
		cc.link_type = 0x01;

		send_event(conn->link->dev, BT_HCI_EVT_CONN_COMPLETE, &cc,
						sizeof(cc));

		cc.handle = cpu_to_le16(conn->handle);
		cc.link_type = 0x01;
	} else {
		cc.handle = cpu_to_le16(0x0000);
		cc.link_type = 0x01;
	}

	cc.status = status;
	memcpy(cc.bdaddr, bdaddr, 6);
	cc.encr_mode = 0x00;

	send_event(btdev, BT_HCI_EVT_CONN_COMPLETE, &cc, sizeof(cc));
}

static int cmd_create_conn_complete(struct btdev *dev, const void *data,
						uint8_t len)
{
	const struct bt_hci_cmd_create_conn *cmd = data;
	struct btdev *remote = find_btdev_by_bdaddr(cmd->bdaddr);

	if (remote && remote->scan_enable & 0x02) {
		struct bt_hci_evt_conn_request cr;

		memcpy(cr.bdaddr, dev->bdaddr, 6);
		memcpy(cr.dev_class, dev->dev_class, 3);
		cr.link_type = 0x01;

		send_event(remote, BT_HCI_EVT_CONN_REQUEST, &cr, sizeof(cr));
	} else {
		conn_complete(dev, cmd->bdaddr, BT_HCI_ERR_PAGE_TIMEOUT);
	}

	return 0;
}

static int cmd_add_sco_conn(struct btdev *dev, const void *data, uint8_t len)
{
	const struct bt_hci_cmd_add_sco_conn *cmd = data;
	struct bt_hci_evt_conn_complete cc;
	struct btdev_conn *conn;

	memset(&cc, 0, sizeof(cc));

	conn = queue_find(dev->conns, match_handle,
				UINT_TO_PTR(cpu_to_le16(cmd->handle)));
	if (!conn) {
		cc.status = BT_HCI_ERR_UNKNOWN_CONN_ID;
		goto done;
	}

	conn = conn_add_sco(conn);
	if (!conn) {
		cc.status = BT_HCI_ERR_MEM_CAPACITY_EXCEEDED;
		goto done;
	}

	cc.status = BT_HCI_ERR_SUCCESS;
	memcpy(cc.bdaddr, conn->link->dev->bdaddr, 6);
	cc.handle = cpu_to_le16(conn->handle);
	cc.link_type = 0x00;
	cc.encr_mode = 0x00;

done:
	send_event(dev, BT_HCI_EVT_CONN_COMPLETE, &cc, sizeof(cc));

	return 0;
}

static int cmd_create_conn_cancel(struct btdev *dev, const void *data,
							uint8_t len)
{
	cmd_status(dev, BT_HCI_ERR_SUCCESS, BT_HCI_CMD_CREATE_CONN_CANCEL);

	return 0;
}

static int cmd_create_conn_cancel_complete(struct btdev *dev, const void *data,
							uint8_t len)
{
	const struct bt_hci_cmd_create_conn_cancel *cmd = data;

	conn_complete(dev, cmd->bdaddr, BT_HCI_ERR_UNKNOWN_CONN_ID);

	return 0;
}

static int cmd_accept_conn(struct btdev *dev, const void *data, uint8_t len)
{
	cmd_status(dev, BT_HCI_ERR_SUCCESS, BT_HCI_CMD_ACCEPT_CONN_REQUEST);

	return 0;
}

static int cmd_accept_conn_complete(struct btdev *dev, const void *data,
							uint8_t len)
{
	const struct bt_hci_cmd_accept_conn_request *cmd = data;
	struct btdev *remote = find_btdev_by_bdaddr(cmd->bdaddr);

	if (!remote)
		return 0;

	if (dev->auth_enable || remote->auth_enable)
		send_event(remote, BT_HCI_EVT_LINK_KEY_REQUEST, dev->bdaddr, 6);
	else
		conn_complete(dev, cmd->bdaddr, BT_HCI_ERR_SUCCESS);

	return 0;
}

static int cmd_reject_conn(struct btdev *dev, const void *data, uint8_t len)
{
	cmd_status(dev, BT_HCI_ERR_SUCCESS, BT_HCI_CMD_REJECT_CONN_REQUEST);

	return 0;
}

static int cmd_reject_conn_complete(struct btdev *dev, const void *data,
							uint8_t len)
{
	const struct bt_hci_cmd_reject_conn_request *cmd = data;

	conn_complete(dev, cmd->bdaddr, BT_HCI_ERR_UNKNOWN_CONN_ID);

	return 0;
}

static int cmd_link_key_reply(struct btdev *dev, const void *data, uint8_t len)
{
	struct bt_hci_rsp_link_key_request_reply rsp;

	memset(&rsp, 0, sizeof(rsp));
	rsp.status = BT_HCI_ERR_SUCCESS;
	memcpy(rsp.bdaddr, data, 6);
	cmd_complete(dev, BT_HCI_CMD_LINK_KEY_REQUEST_REPLY, &rsp, sizeof(rsp));

	return 0;
}

static bool match_bdaddr(const void *data, const void *match_data)
{
	const struct btdev_conn *conn = data;
	const uint8_t *bdaddr = match_data;

	return !memcmp(conn->link->dev->bdaddr, bdaddr, 6);
}

static void auth_complete(struct btdev_conn *conn, uint8_t status)
{
	struct bt_hci_evt_auth_complete ev;

	memset(&ev, 0, sizeof(ev));

	ev.handle = conn ? cpu_to_le16(conn->handle) : 0x0000;
	ev.status = status;

	send_event(conn->dev, BT_HCI_EVT_AUTH_COMPLETE, &ev, sizeof(ev));
}

static int cmd_link_key_reply_complete(struct btdev *dev, const void *data,
						uint8_t len)
{
	const struct bt_hci_cmd_link_key_request_reply *cmd = data;
	struct btdev_conn *conn;
	uint8_t status;

	conn = queue_find(dev->conns, match_bdaddr, cmd->bdaddr);
	if (!conn) {
		status = BT_HCI_ERR_INVALID_PARAMETERS;
		goto done;
	}

	memcpy(dev->link_key, cmd->link_key, 16);

	if (!memcmp(conn->link->dev->link_key, LINK_KEY_NONE, 16)) {
		send_event(conn->link->dev, BT_HCI_EVT_LINK_KEY_REQUEST,
					dev->bdaddr, 6);
		return 0;
	}

	if (!memcmp(dev->link_key, conn->link->dev->link_key, 16))
		status = BT_HCI_ERR_SUCCESS;
	else
		status = BT_HCI_ERR_AUTH_FAILURE;

done:
	auth_complete(conn, status);

	if (conn)
		auth_complete(conn->link, status);

	return 0;
}

static int cmd_link_key_neg_reply(struct btdev *dev, const void *data,
							uint8_t len)
{
	struct bt_hci_rsp_link_key_request_neg_reply rsp;

	memset(&rsp, 0, sizeof(rsp));
	rsp.status = BT_HCI_ERR_SUCCESS;
	memcpy(rsp.bdaddr, data, 6);
	cmd_complete(dev, BT_HCI_CMD_LINK_KEY_REQUEST_NEG_REPLY, &rsp,
					sizeof(rsp));

	return 0;
}

static bool use_ssp(struct btdev *btdev1, struct btdev *btdev2)
{
	if (btdev1->auth_enable || btdev2->auth_enable)
		return false;

	return (btdev1->simple_pairing_mode && btdev2->simple_pairing_mode);
}

static int cmd_link_key_neg_reply_complete(struct btdev *dev, const void *data,
							uint8_t len)
{
	const struct bt_hci_cmd_link_key_request_neg_reply *cmd = data;
	struct btdev *remote;

	remote = find_btdev_by_bdaddr(cmd->bdaddr);
	if (!remote)
		return 0;

	if (use_ssp(dev, remote)) {
		struct bt_hci_evt_io_capability_request io_req;

		memcpy(io_req.bdaddr, cmd->bdaddr, 6);
		send_event(dev, BT_HCI_EVT_IO_CAPABILITY_REQUEST, &io_req,
							sizeof(io_req));
	} else {
		struct bt_hci_evt_pin_code_request pin_req;

		memcpy(pin_req.bdaddr, cmd->bdaddr, 6);
		send_event(dev, BT_HCI_EVT_PIN_CODE_REQUEST, &pin_req,
							sizeof(pin_req));
	}

	return 0;
}

static int cmd_pin_code_reply(struct btdev *dev, const void *data, uint8_t len)
{
	struct bt_hci_rsp_pin_code_request_neg_reply rsp;

	memset(&rsp, 0, sizeof(rsp));
	rsp.status = BT_HCI_ERR_SUCCESS;
	memcpy(rsp.bdaddr, data, 6);
	cmd_complete(dev, BT_HCI_CMD_PIN_CODE_REQUEST_REPLY, &rsp, sizeof(rsp));

	return 0;
}

static uint8_t get_link_key_type(struct btdev *btdev, const uint8_t *bdaddr)
{
	struct btdev_conn *conn;
	uint8_t auth, unauth;

	conn = queue_find(btdev->conns, match_bdaddr, bdaddr);
	if (!conn)
		return 0x00;

	if (!btdev->simple_pairing_mode)
		return 0x00;

	if (btdev->ssp_debug_mode || conn->link->dev->ssp_debug_mode)
		return 0x03;

	if (btdev->secure_conn_support &&
			conn->link->dev->secure_conn_support) {
		unauth = 0x07;
		auth = 0x08;
	} else {
		unauth = 0x04;
		auth = 0x05;
	}

	if (btdev->io_cap == 0x03 || conn->link->dev->io_cap == 0x03)
		return unauth;

	if (!(btdev->auth_req & 0x01) && !(conn->link->dev->auth_req & 0x01))
		return unauth;

	/* DisplayOnly only produces authenticated with KeyboardOnly */
	if (btdev->io_cap == 0x00 && conn->link->dev->io_cap != 0x02)
		return unauth;

	/* DisplayOnly only produces authenticated with KeyboardOnly */
	if (conn->link->dev->io_cap == 0x00 && btdev->io_cap != 0x02)
		return unauth;

	return auth;
}

static void link_key_notify(struct btdev *btdev, const uint8_t *bdaddr,
							const uint8_t *key)
{
	struct bt_hci_evt_link_key_notify ev;

	memcpy(btdev->link_key, key, 16);

	memcpy(ev.bdaddr, bdaddr, 6);
	memcpy(ev.link_key, key, 16);
	ev.key_type = get_link_key_type(btdev, bdaddr);

	send_event(btdev, BT_HCI_EVT_LINK_KEY_NOTIFY, &ev, sizeof(ev));
}

static int cmd_pin_code_reply_complete(struct btdev *dev, const void *data,
							uint8_t len)
{
	const struct bt_hci_cmd_pin_code_request_reply *cmd = data;
	struct btdev *remote;
	struct btdev_conn *conn;
	uint8_t status;

	conn = queue_find(dev->conns, match_bdaddr, cmd->bdaddr);
	if (!conn) {
		remote = find_btdev_by_bdaddr(cmd->bdaddr);
		if (!remote)
			return 0;
	} else
		remote = conn->link->dev;

	memcpy(dev->pin, cmd->pin_code, cmd->pin_len);
	dev->pin_len = cmd->pin_len;

	if (!remote->pin_len) {
		struct bt_hci_evt_pin_code_request pin_req;

		memcpy(pin_req.bdaddr, dev->bdaddr, 6);
		send_event(remote, BT_HCI_EVT_PIN_CODE_REQUEST,
					&pin_req, sizeof(pin_req));
		return 0;
	}

	if (dev->pin_len == remote->pin_len &&
			!memcmp(dev->pin, remote->pin, dev->pin_len)) {
		link_key_notify(dev, remote->bdaddr, LINK_KEY_DUMMY);
		link_key_notify(remote, dev->bdaddr, LINK_KEY_DUMMY);
		status = BT_HCI_ERR_SUCCESS;
	} else {
		status = BT_HCI_ERR_AUTH_FAILURE;
	}

	if (conn)
		auth_complete(conn->link, status);
	else
		conn_complete(remote, dev->bdaddr, status);

	dev->pin_len = 0;
	remote->pin_len = 0;

	return 0;
}

static int cmd_pin_code_neg_reply(struct btdev *dev, const void *data,
							uint8_t len)
{
	struct bt_hci_rsp_pin_code_request_neg_reply rsp;

	memset(&rsp, 0, sizeof(rsp));
	rsp.status = BT_HCI_ERR_SUCCESS;
	memcpy(rsp.bdaddr, data, 6);
	cmd_complete(dev, BT_HCI_CMD_PIN_CODE_REQUEST_NEG_REPLY, &rsp,
					sizeof(rsp));

	return 0;
}

static int cmd_pin_code_neg_reply_complete(struct btdev *dev, const void *data,
							uint8_t len)
{
	const struct bt_hci_cmd_pin_code_request_neg_reply *cmd = data;
	struct btdev *remote;
	struct btdev_conn *conn;
	uint8_t status;

	remote = find_btdev_by_bdaddr(cmd->bdaddr);
	if (!remote)
		return 0;

	status = BT_HCI_ERR_PIN_OR_KEY_MISSING;

	conn = queue_find(dev->conns, match_bdaddr, cmd->bdaddr);
	if (conn)
		auth_complete(conn, status);
	else
		conn_complete(dev, cmd->bdaddr, BT_HCI_ERR_PIN_OR_KEY_MISSING);

	if (conn) {
		if (remote->pin_len)
			auth_complete(conn->link, status);
	} else {
		conn_complete(remote, dev->bdaddr,
					BT_HCI_ERR_PIN_OR_KEY_MISSING);
	}

	return 0;
}

static int cmd_auth_requested(struct btdev *dev, const void *data, uint8_t len)
{
	cmd_status(dev, BT_HCI_ERR_SUCCESS, BT_HCI_CMD_AUTH_REQUESTED);

	return 0;
}

static int cmd_auth_requested_complete(struct btdev *dev, const void *data,
						uint8_t len)
{
	const struct bt_hci_cmd_auth_requested *cmd = data;
	struct btdev_conn *conn;

	conn = queue_find(dev->conns, match_handle,
				UINT_TO_PTR(le16_to_cpu(cmd->handle)));
	if (!conn) {
		struct bt_hci_evt_auth_complete ev;

		ev.status = BT_HCI_ERR_UNKNOWN_CONN_ID;
		ev.handle = cpu_to_le16(cmd->handle);

		send_event(dev, BT_HCI_EVT_AUTH_COMPLETE, &ev, sizeof(ev));

		return 0;
	}

	dev->auth_init = true;

	send_event(dev, BT_HCI_EVT_LINK_KEY_REQUEST, conn->link->dev->bdaddr,
					sizeof(conn->link->dev->bdaddr));

	return 0;
}

static int cmd_set_conn_encrypt(struct btdev *dev, const void *data,
							uint8_t len)
{
	cmd_status(dev, BT_HCI_ERR_SUCCESS, BT_HCI_CMD_SET_CONN_ENCRYPT);

	return 0;
}

static void encrypt_change(struct btdev_conn *conn, uint8_t mode,
					uint8_t status)
{
	struct bt_hci_evt_encrypt_change ev;

	if (!conn)
		return;

	memset(&ev, 0, sizeof(ev));

	ev.status = status;
	ev.handle = cpu_to_le16(conn->handle);
	ev.encr_mode = mode;

	send_event(conn->dev, BT_HCI_EVT_ENCRYPT_CHANGE, &ev, sizeof(ev));
}

static int cmd_set_conn_encrypt_complete(struct btdev *dev, const void *data,
							uint8_t len)
{
	const struct bt_hci_cmd_set_conn_encrypt *cmd = data;
	struct btdev_conn *conn;
	uint8_t mode;

	conn = queue_find(dev->conns, match_handle,
				UINT_TO_PTR(le16_to_cpu(cmd->handle)));
	if (!conn)
		return 0;

	if (!cmd->encr_mode)
		mode = 0x00;
	else if (dev->secure_conn_support &&
				conn->link->dev->secure_conn_support)
		mode = 0x02;
	else
		mode = 0x01;

	encrypt_change(conn, mode, BT_HCI_ERR_SUCCESS);
	encrypt_change(conn->link, mode, BT_HCI_ERR_SUCCESS);

	return 0;
}

static int cmd_remote_name(struct btdev *dev, const void *data, uint8_t len)
{
	cmd_status(dev, BT_HCI_ERR_SUCCESS, BT_HCI_CMD_REMOTE_NAME_REQUEST);

	return 0;
}

static void name_request_complete(struct btdev *btdev,
					const uint8_t *bdaddr, uint8_t status)
{
	struct bt_hci_evt_remote_name_request_complete nc;

	nc.status = status;
	memcpy(nc.bdaddr, bdaddr, 6);
	memset(nc.name, 0, 248);

	if (!status) {
		struct btdev *remote = find_btdev_by_bdaddr(bdaddr);

		if (remote)
			memcpy(nc.name, remote->name, 248);
		else
			nc.status = BT_HCI_ERR_UNKNOWN_CONN_ID;
	}

	send_event(btdev, BT_HCI_EVT_REMOTE_NAME_REQUEST_COMPLETE,
							&nc, sizeof(nc));
}

static int cmd_remote_name_complete(struct btdev *dev, const void *data,
						uint8_t len)
{
	const struct bt_hci_cmd_remote_name_request *cmd = data;

	name_request_complete(dev, cmd->bdaddr, BT_HCI_ERR_SUCCESS);

	return 0;
}

static int cmd_remote_name_cancel(struct btdev *dev, const void *data,
							uint8_t len)
{
	const struct bt_hci_cmd_remote_name_request_cancel *cmd = data;
	struct bt_hci_rsp_remote_name_request_cancel rsp;

	memset(&rsp, 0, sizeof(rsp));
	rsp.status = BT_HCI_ERR_SUCCESS;
	memcpy(rsp.bdaddr, cmd->bdaddr, 6);
	cmd_complete(dev, BT_HCI_CMD_REMOTE_NAME_REQUEST_CANCEL, &rsp,
					sizeof(rsp));

	return 0;
}

static int cmd_remote_name_cancel_complete(struct btdev *dev, const void *data,
						uint8_t len)
{
	const struct bt_hci_cmd_remote_name_request_cancel *cmd = data;

	name_request_complete(dev, cmd->bdaddr, BT_HCI_ERR_UNKNOWN_CONN_ID);

	return 0;
}

static int cmd_read_remote_features(struct btdev *dev, const void *data,
							uint8_t len)
{
	cmd_status(dev, BT_HCI_ERR_SUCCESS, BT_HCI_CMD_READ_REMOTE_FEATURES);

	return 0;
}

static int cmd_read_remote_features_complete(struct btdev *dev,
						const void *data, uint8_t len)
{
	const struct bt_hci_cmd_read_remote_features *cmd = data;
	struct bt_hci_evt_remote_features_complete rfc;
	struct btdev_conn *conn;

	conn = queue_find(dev->conns, match_handle,
				UINT_TO_PTR(le16_to_cpu(cmd->handle)));
	if (conn) {
		rfc.status = BT_HCI_ERR_SUCCESS;
		rfc.handle = cpu_to_le16(cmd->handle);
		memcpy(rfc.features, conn->link->dev->features, 8);
	} else {
		rfc.status = BT_HCI_ERR_UNKNOWN_CONN_ID;
		rfc.handle = cpu_to_le16(cmd->handle);
		memset(rfc.features, 0, 8);
	}

	send_event(dev, BT_HCI_EVT_REMOTE_FEATURES_COMPLETE, &rfc, sizeof(rfc));

	return 0;
}

static int cmd_read_remote_ext_features(struct btdev *dev, const void *data,
							uint8_t len)
{
	cmd_status(dev, BT_HCI_ERR_SUCCESS,
					BT_HCI_CMD_READ_REMOTE_EXT_FEATURES);

	return 0;
}

static void btdev_get_host_features(struct btdev *btdev, uint8_t features[8])
{
	memset(features, 0, 8);
	if (btdev->simple_pairing_mode)
		features[0] |= 0x01;
	if (btdev->le_supported)
		features[0] |= 0x02;
	if (btdev->le_simultaneous)
		features[0] |= 0x04;
	if (btdev->secure_conn_support)
		features[0] |= 0x08;
}

static int cmd_read_remote_ext_features_compl(struct btdev *dev,
						const void *data, uint8_t len)
{
	const struct bt_hci_cmd_read_remote_ext_features *cmd = data;
	struct bt_hci_evt_remote_ext_features_complete ev;
	struct btdev_conn *conn;

	memset(&ev, 0, sizeof(ev));

	conn = queue_find(dev->conns, match_handle,
				UINT_TO_PTR(le16_to_cpu(cmd->handle)));
	if (conn && cmd->page < 0x02) {
		ev.handle = cpu_to_le16(cmd->handle);
		ev.page = cmd->page;
		ev.max_page = 0x01;

		switch (cmd->page) {
		case 0x00:
			ev.status = BT_HCI_ERR_SUCCESS;
			memcpy(ev.features, conn->link->dev->features, 8);
			break;
		case 0x01:
			ev.status = BT_HCI_ERR_SUCCESS;
			btdev_get_host_features(conn->link->dev, ev.features);
			break;
		default:
			ev.status = BT_HCI_ERR_INVALID_PARAMETERS;
			memset(ev.features, 0, 8);
			break;
		}
	} else {
		ev.status = BT_HCI_ERR_UNKNOWN_CONN_ID;
		ev.handle = cpu_to_le16(cmd->handle);
		ev.page = cmd->page;
		ev.max_page = 0x01;
		memset(ev.features, 0, 8);
	}

	send_event(dev, BT_HCI_EVT_REMOTE_EXT_FEATURES_COMPLETE, &ev,
						sizeof(ev));

	return 0;
}

static int cmd_read_clock_offset(struct btdev *dev, const void *data,
							uint8_t len)
{
	cmd_status(dev, BT_HCI_ERR_SUCCESS, BT_HCI_CMD_READ_CLOCK_OFFSET);

	return 0;
}

static int cmd_read_clock_offset_complete(struct btdev *dev, const void *data,
							uint8_t len)
{
	const struct bt_hci_cmd_read_clock_offset *cmd = data;
	struct bt_hci_evt_clock_offset_complete ev;
	struct btdev_conn *conn;

	memset(&ev, 0, sizeof(ev));

	conn = queue_find(dev->conns, match_handle,
				UINT_TO_PTR(le16_to_cpu(cmd->handle)));
	if (conn) {
		ev.status = BT_HCI_ERR_SUCCESS;
		ev.handle = cpu_to_le16(cmd->handle);
		ev.clock_offset = 0;
	} else {
		ev.status = BT_HCI_ERR_UNKNOWN_CONN_ID;
		ev.handle = cpu_to_le16(cmd->handle);
		ev.clock_offset = 0;
	}

	send_event(dev, BT_HCI_EVT_CLOCK_OFFSET_COMPLETE, &ev, sizeof(ev));

	return 0;
}

static int cmd_read_link_policy(struct btdev *dev, const void *data,
							uint8_t len)
{
	struct bt_hci_rsp_read_default_link_policy rsp;

	memset(&rsp, 0, sizeof(rsp));
	rsp.status = BT_HCI_ERR_SUCCESS;
	rsp.policy = cpu_to_le16(dev->default_link_policy);
	cmd_complete(dev, BT_HCI_CMD_READ_DEFAULT_LINK_POLICY, &rsp,
					sizeof(rsp));

	return 0;
}

static int cmd_write_link_policy(struct btdev *dev, const void *data,
							uint8_t len)
{
	const struct bt_hci_cmd_write_default_link_policy *cmd = data;
	uint8_t status;

	dev->default_link_policy = le16_to_cpu(cmd->policy);
	status = BT_HCI_ERR_SUCCESS;
	cmd_complete(dev, BT_HCI_CMD_WRITE_DEFAULT_LINK_POLICY, &status,
					sizeof(status));

	return 0;
}

static int cmd_set_event_filter(struct btdev *dev, const void *data,
							uint8_t len)
{
	const struct bt_hci_cmd_set_event_filter *cmd = data;
	uint8_t status;

	dev->event_filter = cmd->type;
	status = BT_HCI_ERR_SUCCESS;
	cmd_complete(dev, BT_HCI_CMD_SET_EVENT_FILTER, &status, sizeof(status));

	return 0;
}

static int cmd_read_link_key(struct btdev *dev, const void *data, uint8_t len)
{
	struct bt_hci_rsp_read_stored_link_key rsp;

	memset(&rsp, 0, sizeof(rsp));
	rsp.status = BT_HCI_ERR_SUCCESS;
	rsp.max_num_keys = cpu_to_le16(0);
	rsp.num_keys = cpu_to_le16(0);
	cmd_complete(dev, BT_HCI_CMD_READ_STORED_LINK_KEY, &rsp, sizeof(rsp));

	return 0;
}

static int cmd_write_link_key(struct btdev *dev, const void *data, uint8_t len)
{
	struct bt_hci_rsp_write_stored_link_key rsp;

	memset(&rsp, 0, sizeof(rsp));
	rsp.status = BT_HCI_ERR_SUCCESS;
	rsp.num_keys = 0;
	cmd_complete(dev, BT_HCI_CMD_WRITE_STORED_LINK_KEY, &rsp, sizeof(rsp));

	return 0;
}

static int cmd_delete_link_key(struct btdev *dev, const void *data, uint8_t len)
{
	struct bt_hci_rsp_delete_stored_link_key rsp;

	memset(&rsp, 0, sizeof(rsp));
	rsp.status = BT_HCI_ERR_SUCCESS;
	rsp.num_keys = cpu_to_le16(0);
	cmd_complete(dev, BT_HCI_CMD_DELETE_STORED_LINK_KEY, &rsp, sizeof(rsp));

	return 0;
}

static int cmd_read_local_name(struct btdev *dev, const void *data, uint8_t len)
{
	struct bt_hci_rsp_read_local_name rsp;

	memset(&rsp, 0, sizeof(rsp));
	rsp.status = BT_HCI_ERR_SUCCESS;
	memcpy(rsp.name, dev->name, 248);
	cmd_complete(dev, BT_HCI_CMD_READ_LOCAL_NAME, &rsp, sizeof(rsp));

	return 0;
}

static int cmd_write_local_name(struct btdev *dev, const void *data,
							uint8_t len)
{
	const struct bt_hci_cmd_write_local_name *cmd = data;
	uint8_t status;

	memcpy(dev->name, cmd->name, 248);
	status = BT_HCI_ERR_SUCCESS;
	cmd_complete(dev, BT_HCI_CMD_WRITE_LOCAL_NAME, &status, sizeof(status));

	return 0;
}

static int cmd_read_accept_timeout(struct btdev *dev, const void *data,
							uint8_t len)
{
	struct bt_hci_rsp_read_conn_accept_timeout rsp;

	memset(&rsp, 0, sizeof(rsp));
	rsp.status = BT_HCI_ERR_SUCCESS;
	rsp.timeout = cpu_to_le16(dev->conn_accept_timeout);
	cmd_complete(dev, BT_HCI_CMD_READ_CONN_ACCEPT_TIMEOUT, &rsp,
					sizeof(rsp));

	return 0;
}

static int cmd_write_accept_timeout(struct btdev *dev, const void *data,
							uint8_t len)
{
	const struct bt_hci_cmd_write_conn_accept_timeout *cmd = data;
	uint8_t status;

	dev->conn_accept_timeout = le16_to_cpu(cmd->timeout);
	status = BT_HCI_ERR_SUCCESS;
	cmd_complete(dev, BT_HCI_CMD_WRITE_CONN_ACCEPT_TIMEOUT, &status,
					sizeof(status));

	return 0;
}

static int cmd_read_page_timeout(struct btdev *dev, const void *data,
							uint8_t len)
{
	struct bt_hci_rsp_read_page_timeout rsp;

	memset(&rsp, 0, sizeof(rsp));
	rsp.status = BT_HCI_ERR_SUCCESS;
	rsp.timeout = cpu_to_le16(dev->page_timeout);
	cmd_complete(dev, BT_HCI_CMD_READ_PAGE_TIMEOUT, &rsp, sizeof(rsp));

	return 0;
}

static int cmd_write_page_timeout(struct btdev *dev, const void *data,
							uint8_t len)
{
	const struct bt_hci_cmd_write_page_timeout *cmd = data;
	uint8_t status = BT_HCI_ERR_SUCCESS;

	dev->page_timeout = le16_to_cpu(cmd->timeout);
	cmd_complete(dev, BT_HCI_CMD_WRITE_PAGE_TIMEOUT, &status,
					sizeof(status));

	return 0;
}

static int cmd_read_scan_enable(struct btdev *dev, const void *data,
							uint8_t len)
{
	struct bt_hci_rsp_read_scan_enable rsp;

	memset(&rsp, 0, sizeof(rsp));
	rsp.status = BT_HCI_ERR_SUCCESS;
	rsp.enable = dev->scan_enable;
	cmd_complete(dev, BT_HCI_CMD_READ_SCAN_ENABLE, &rsp, sizeof(rsp));

	return 0;
}

static int cmd_write_scan_enable(struct btdev *dev, const void *data,
							uint8_t len)
{
	const struct bt_hci_cmd_write_scan_enable *cmd = data;
	uint8_t status = BT_HCI_ERR_SUCCESS;

	dev->scan_enable = cmd->enable;
	cmd_complete(dev, BT_HCI_CMD_WRITE_SCAN_ENABLE, &status,
					sizeof(status));

	return 0;
}

static int cmd_read_page_scan(struct btdev *dev, const void *data,
							uint8_t len)
{
	struct bt_hci_rsp_read_page_scan_activity rsp;

	memset(&rsp, 0, sizeof(rsp));
	rsp.status = BT_HCI_ERR_SUCCESS;
	rsp.interval = cpu_to_le16(dev->page_scan_interval);
	rsp.window = cpu_to_le16(dev->page_scan_window);
	cmd_complete(dev, BT_HCI_CMD_READ_PAGE_SCAN_ACTIVITY, &rsp,
					sizeof(rsp));

	return 0;
}

static int cmd_write_page_scan(struct btdev *dev, const void *data,
							uint8_t len)
{
	const struct bt_hci_cmd_write_page_scan_activity *cmd = data;
	uint8_t status = BT_HCI_ERR_SUCCESS;

	dev->page_scan_interval = le16_to_cpu(cmd->interval);
	dev->page_scan_window = le16_to_cpu(cmd->window);
	cmd_complete(dev, BT_HCI_CMD_WRITE_PAGE_SCAN_ACTIVITY, &status,
					sizeof(status));

	return 0;
}

static int cmd_read_inquiry_scan(struct btdev *dev, const void *data,
							uint8_t len)
{
	struct bt_hci_rsp_read_inquiry_scan_activity rsp;

	memset(&rsp, 0, sizeof(rsp));
	rsp.status = BT_HCI_ERR_SUCCESS;
	rsp.interval = cpu_to_le16(dev->inquiry_scan_interval);
	rsp.window = cpu_to_le16(dev->inquiry_scan_window);
	cmd_complete(dev, BT_HCI_CMD_READ_INQUIRY_SCAN_ACTIVITY, &rsp,
					sizeof(rsp));

	return 0;
}

static int cmd_write_inquiry_scan(struct btdev *dev, const void *data,
							uint8_t len)
{
	const struct bt_hci_cmd_write_inquiry_scan_activity *cmd = data;
	uint8_t status = BT_HCI_ERR_SUCCESS;

	dev->inquiry_scan_interval = le16_to_cpu(cmd->interval);
	dev->inquiry_scan_window = le16_to_cpu(cmd->window);
	cmd_complete(dev, BT_HCI_CMD_WRITE_INQUIRY_SCAN_ACTIVITY, &status,
					sizeof(status));

	return 0;
}

static int cmd_read_auth_enable(struct btdev *dev, const void *data,
							uint8_t len)
{
	struct bt_hci_rsp_read_auth_enable rsp;

	memset(&rsp, 0, sizeof(rsp));
	rsp.status = BT_HCI_ERR_SUCCESS;
	rsp.enable = dev->auth_enable;
	cmd_complete(dev, BT_HCI_CMD_READ_AUTH_ENABLE, &rsp, sizeof(rsp));

	return 0;
}

static int cmd_write_auth_enable(struct btdev *dev, const void *data,
							uint8_t len)
{
	const struct bt_hci_cmd_write_auth_enable *cmd = data;
	uint8_t status = BT_HCI_ERR_SUCCESS;

	dev->auth_enable = cmd->enable;
	cmd_complete(dev, BT_HCI_CMD_WRITE_AUTH_ENABLE, &status,
					sizeof(status));

	return 0;
}

static int cmd_read_class(struct btdev *dev, const void *data, uint8_t len)
{
	struct bt_hci_rsp_read_class_of_dev rsp;

	memset(&rsp, 0, sizeof(rsp));
	rsp.status = BT_HCI_ERR_SUCCESS;
	memcpy(rsp.dev_class, dev->dev_class, 3);
	cmd_complete(dev, BT_HCI_CMD_READ_CLASS_OF_DEV, &rsp, sizeof(rsp));

	return 0;
}

static int cmd_write_class(struct btdev *dev, const void *data, uint8_t len)
{
	const struct bt_hci_cmd_write_class_of_dev *cmd = data;
	uint8_t status = BT_HCI_ERR_SUCCESS;

	memcpy(dev->dev_class, cmd->dev_class, 3);
	cmd_complete(dev, BT_HCI_CMD_WRITE_CLASS_OF_DEV, &status,
					sizeof(status));

	return 0;
}

static int cmd_read_voice(struct btdev *dev, const void *data, uint8_t len)
{
	struct bt_hci_rsp_read_voice_setting rsp;

	memset(&rsp, 0, sizeof(rsp));
	rsp.status = BT_HCI_ERR_SUCCESS;
	rsp.setting = cpu_to_le16(dev->voice_setting);
	cmd_complete(dev, BT_HCI_CMD_READ_VOICE_SETTING, &rsp, sizeof(rsp));

	return 0;
}

static int cmd_write_voice(struct btdev *dev, const void *data, uint8_t len)
{
	const struct bt_hci_cmd_write_voice_setting *cmd = data;
	uint8_t status = BT_HCI_ERR_SUCCESS;

	dev->voice_setting = le16_to_cpu(cmd->setting);
	cmd_complete(dev, BT_HCI_CMD_WRITE_VOICE_SETTING, &status,
					sizeof(status));

	return 0;
}

static int cmd_read_num_iac(struct btdev *dev, const void *data, uint8_t len)
{
	struct bt_hci_rsp_read_num_supported_iac rsp;

	memset(&rsp, 0, sizeof(rsp));
	rsp.status = BT_HCI_ERR_SUCCESS;
	rsp.num_iac = 0x01;
	cmd_complete(dev, BT_HCI_CMD_READ_NUM_SUPPORTED_IAC, &rsp, sizeof(rsp));

	return 0;
}

static int cmd_read_current_iac_lap(struct btdev *dev, const void *data,
							uint8_t len)
{
	struct bt_hci_rsp_read_current_iac_lap *rsp;

	rsp = alloca(sizeof(*rsp) + 3);
	rsp->status = BT_HCI_ERR_SUCCESS;
	rsp->num_iac = 0x01;
	rsp->iac_lap[0] = 0x33;
	rsp->iac_lap[1] = 0x8b;
	rsp->iac_lap[2] = 0x9e;
	cmd_complete(dev, BT_HCI_CMD_READ_CURRENT_IAC_LAP, rsp,
					sizeof(*rsp) + 3);

	return 0;
}

static int cmd_write_current_iac_lap(struct btdev *dev, const void *data,
							uint8_t len)
{
	uint8_t status = BT_HCI_ERR_SUCCESS;

	cmd_complete(dev, BT_HCI_CMD_WRITE_CURRENT_IAC_LAP, &status,
					sizeof(status));

	return 0;
}

static int cmd_read_inquiry_mode(struct btdev *dev, const void *data,
							uint8_t len)
{
	struct bt_hci_rsp_read_inquiry_mode rsp;

	memset(&rsp, 0, sizeof(rsp));
	rsp.status = BT_HCI_ERR_SUCCESS;
	rsp.mode = dev->inquiry_mode;
	cmd_complete(dev, BT_HCI_CMD_READ_INQUIRY_MODE, &rsp, sizeof(rsp));

	return 0;
}

static int cmd_write_inquiry_mode(struct btdev *dev, const void *data,
							uint8_t len)
{
	const struct bt_hci_cmd_write_inquiry_mode *cmd = data;
	uint8_t status = BT_HCI_ERR_SUCCESS;

	dev->inquiry_mode = cmd->mode;
	cmd_complete(dev, BT_HCI_CMD_WRITE_INQUIRY_MODE, &status,
					sizeof(status));

	return 0;
}

static int cmd_read_page_scan_type(struct btdev *dev, const void *data,
							uint8_t len)
{
	struct bt_hci_rsp_read_page_scan_type rsp;

	memset(&rsp, 0, sizeof(rsp));
	rsp.status = BT_HCI_ERR_SUCCESS;
	rsp.type = dev->page_scan_type;
	cmd_complete(dev, BT_HCI_CMD_READ_PAGE_SCAN_TYPE, &rsp, sizeof(rsp));

	return 0;
}

static int cmd_write_page_scan_type(struct btdev *dev, const void *data,
							uint8_t len)
{
	const struct bt_hci_cmd_write_page_scan_type *cmd = data;
	uint8_t status = BT_HCI_ERR_SUCCESS;

	dev->page_scan_type = cmd->type;
	cmd_complete(dev, BT_HCI_CMD_WRITE_PAGE_SCAN_TYPE, &status,
					sizeof(status));

	return 0;
}

static int cmd_read_afh_mode(struct btdev *dev, const void *data, uint8_t len)
{
	struct bt_hci_rsp_read_afh_assessment_mode rsp;

	memset(&rsp, 0, sizeof(rsp));
	rsp.status = BT_HCI_ERR_SUCCESS;
	rsp.mode = dev->afh_assessment_mode;
	cmd_complete(dev, BT_HCI_CMD_READ_AFH_ASSESSMENT_MODE, &rsp,
					sizeof(rsp));

	return 0;
}

static int cmd_write_afh_mode(struct btdev *dev, const void *data, uint8_t len)
{
	const struct bt_hci_cmd_write_afh_assessment_mode *cmd = data;
	uint8_t status = BT_HCI_ERR_SUCCESS;

	dev->afh_assessment_mode = cmd->mode;
	cmd_complete(dev, BT_HCI_CMD_WRITE_AFH_ASSESSMENT_MODE, &status,
					sizeof(status));

	return 0;
}

static int cmd_read_local_ext_features(struct btdev *dev, const void *data,
							uint8_t len)
{
	struct bt_hci_rsp_read_local_ext_features rsp;
	uint8_t page = ((const uint8_t *) data)[0];

	memset(&rsp, 0, sizeof(rsp));
	rsp.page = page;
	rsp.max_page = dev->max_page;

	if (page > dev->max_page) {
		rsp.status = BT_HCI_ERR_INVALID_PARAMETERS;
		goto done;
	}

	rsp.status = BT_HCI_ERR_SUCCESS;

	switch (page) {
	case 0x00:
		memcpy(rsp.features, dev->features, 8);
		break;
	case 0x01:
		btdev_get_host_features(dev, rsp.features);
		break;
	case 0x02:
		memcpy(rsp.features, dev->feat_page_2, 8);
		break;
	default:
		rsp.status = BT_HCI_ERR_INVALID_PARAMETERS;
		break;
	}

done:
	cmd_complete(dev, BT_HCI_CMD_READ_LOCAL_EXT_FEATURES, &rsp,
					sizeof(rsp));

	return 0;
}

static int cmd_read_country_code(struct btdev *dev, const void *data,
							uint8_t len)
{
	struct bt_hci_rsp_read_country_code rsp;

	memset(&rsp, 0, sizeof(rsp));
	rsp.status = BT_HCI_ERR_SUCCESS;
	rsp.code = dev->country_code;
	cmd_complete(dev, BT_HCI_CMD_READ_COUNTRY_CODE, &rsp, sizeof(rsp));

	return 0;
}

static int cmd_enable_dut_mode(struct btdev *dev, const void *data,
							uint8_t len)
{
	uint8_t status = BT_HCI_ERR_SUCCESS;

	cmd_complete(dev, BT_HCI_CMD_ENABLE_DUT_MODE, &status,
					sizeof(status));

	return 0;
}

#define CMD_COMMON_BREDR_20 \
	CMD(BT_HCI_CMD_INQUIRY, cmd_inquiry, cmd_inquiry_complete), \
	CMD(BT_HCI_CMD_INQUIRY_CANCEL, cmd_inquiry_cancel, NULL), \
	CMD(BT_HCI_CMD_CREATE_CONN, cmd_create_conn, \
					cmd_create_conn_complete), \
	CMD(BT_HCI_CMD_ADD_SCO_CONN, cmd_add_sco_conn, NULL), \
	CMD(BT_HCI_CMD_CREATE_CONN_CANCEL, cmd_create_conn_cancel, \
					cmd_create_conn_cancel_complete), \
	CMD(BT_HCI_CMD_ACCEPT_CONN_REQUEST, cmd_accept_conn, \
					cmd_accept_conn_complete), \
	CMD(BT_HCI_CMD_REJECT_CONN_REQUEST, cmd_reject_conn, \
					cmd_reject_conn_complete), \
	CMD(BT_HCI_CMD_LINK_KEY_REQUEST_REPLY, cmd_link_key_reply, \
					cmd_link_key_reply_complete), \
	CMD(BT_HCI_CMD_LINK_KEY_REQUEST_NEG_REPLY, \
					cmd_link_key_neg_reply, \
					cmd_link_key_neg_reply_complete), \
	CMD(BT_HCI_CMD_PIN_CODE_REQUEST_REPLY, cmd_pin_code_reply, \
					cmd_pin_code_reply_complete), \
	CMD(BT_HCI_CMD_PIN_CODE_REQUEST_NEG_REPLY, \
					cmd_pin_code_neg_reply, \
					cmd_pin_code_neg_reply_complete), \
	CMD(BT_HCI_CMD_AUTH_REQUESTED, cmd_auth_requested, \
					cmd_auth_requested_complete), \
	CMD(BT_HCI_CMD_SET_CONN_ENCRYPT, cmd_set_conn_encrypt, \
					cmd_set_conn_encrypt_complete), \
	CMD(BT_HCI_CMD_REMOTE_NAME_REQUEST, cmd_remote_name, \
					cmd_remote_name_complete), \
	CMD(BT_HCI_CMD_REMOTE_NAME_REQUEST_CANCEL, cmd_remote_name_cancel, \
					cmd_remote_name_cancel_complete), \
	CMD(BT_HCI_CMD_READ_REMOTE_FEATURES, cmd_read_remote_features, \
					cmd_read_remote_features_complete), \
	CMD(BT_HCI_CMD_READ_REMOTE_EXT_FEATURES, \
					cmd_read_remote_ext_features, \
					cmd_read_remote_ext_features_compl), \
	CMD(BT_HCI_CMD_READ_CLOCK_OFFSET, cmd_read_clock_offset, \
					cmd_read_clock_offset_complete), \
	CMD(BT_HCI_CMD_READ_DEFAULT_LINK_POLICY, cmd_read_link_policy, NULL), \
	CMD(BT_HCI_CMD_WRITE_DEFAULT_LINK_POLICY, cmd_write_link_policy, \
					NULL), \
	CMD(BT_HCI_CMD_SET_EVENT_FILTER, cmd_set_event_filter, NULL), \
	CMD(BT_HCI_CMD_READ_STORED_LINK_KEY, cmd_read_link_key, NULL), \
	CMD(BT_HCI_CMD_WRITE_STORED_LINK_KEY, cmd_write_link_key, NULL), \
	CMD(BT_HCI_CMD_DELETE_STORED_LINK_KEY, cmd_delete_link_key, NULL), \
	CMD(BT_HCI_CMD_READ_LOCAL_NAME, cmd_read_local_name, NULL), \
	CMD(BT_HCI_CMD_WRITE_LOCAL_NAME, cmd_write_local_name, NULL), \
	CMD(BT_HCI_CMD_READ_CONN_ACCEPT_TIMEOUT, cmd_read_accept_timeout, \
					NULL), \
	CMD(BT_HCI_CMD_WRITE_CONN_ACCEPT_TIMEOUT, cmd_write_accept_timeout, \
					NULL), \
	CMD(BT_HCI_CMD_READ_PAGE_TIMEOUT, cmd_read_page_timeout, NULL), \
	CMD(BT_HCI_CMD_WRITE_PAGE_TIMEOUT, cmd_write_page_timeout, NULL), \
	CMD(BT_HCI_CMD_READ_SCAN_ENABLE, cmd_read_scan_enable, NULL), \
	CMD(BT_HCI_CMD_WRITE_SCAN_ENABLE, cmd_write_scan_enable, NULL), \
	CMD(BT_HCI_CMD_READ_PAGE_SCAN_ACTIVITY, cmd_read_page_scan, NULL), \
	CMD(BT_HCI_CMD_WRITE_PAGE_SCAN_ACTIVITY, cmd_write_page_scan, NULL), \
	CMD(BT_HCI_CMD_READ_INQUIRY_SCAN_ACTIVITY, cmd_read_inquiry_scan, \
					NULL), \
	CMD(BT_HCI_CMD_WRITE_INQUIRY_SCAN_ACTIVITY, cmd_write_inquiry_scan, \
					NULL), \
	CMD(BT_HCI_CMD_READ_AUTH_ENABLE, cmd_read_auth_enable, NULL), \
	CMD(BT_HCI_CMD_WRITE_AUTH_ENABLE, cmd_write_auth_enable, NULL), \
	CMD(BT_HCI_CMD_READ_CLASS_OF_DEV, cmd_read_class, NULL), \
	CMD(BT_HCI_CMD_WRITE_CLASS_OF_DEV, cmd_write_class, NULL), \
	CMD(BT_HCI_CMD_READ_VOICE_SETTING, cmd_read_voice, NULL), \
	CMD(BT_HCI_CMD_WRITE_VOICE_SETTING, cmd_write_voice, NULL), \
	CMD(BT_HCI_CMD_READ_NUM_SUPPORTED_IAC, cmd_read_num_iac, NULL), \
	CMD(BT_HCI_CMD_READ_CURRENT_IAC_LAP, cmd_read_current_iac_lap, \
					NULL), \
	CMD(BT_HCI_CMD_WRITE_CURRENT_IAC_LAP, cmd_write_current_iac_lap, \
					NULL), \
	CMD(BT_HCI_CMD_READ_INQUIRY_MODE, cmd_read_inquiry_mode, NULL), \
	CMD(BT_HCI_CMD_WRITE_INQUIRY_MODE, cmd_write_inquiry_mode, NULL), \
	CMD(BT_HCI_CMD_READ_PAGE_SCAN_TYPE, cmd_read_page_scan_type, NULL), \
	CMD(BT_HCI_CMD_WRITE_PAGE_SCAN_TYPE, cmd_write_page_scan_type, NULL), \
	CMD(BT_HCI_CMD_READ_AFH_ASSESSMENT_MODE, cmd_read_afh_mode, NULL), \
	CMD(BT_HCI_CMD_WRITE_AFH_ASSESSMENT_MODE, cmd_write_afh_mode, NULL), \
	CMD(BT_HCI_CMD_READ_LOCAL_EXT_FEATURES, cmd_read_local_ext_features, \
					NULL), \
	CMD(BT_HCI_CMD_READ_COUNTRY_CODE, cmd_read_country_code, NULL), \
	CMD(BT_HCI_CMD_ENABLE_DUT_MODE, cmd_enable_dut_mode, NULL)

static void set_common_commands_bredr20(struct btdev *btdev)
{
	btdev->commands[0]  |= 0x01;	/* Inquiry */
	btdev->commands[0]  |= 0x02;	/* Inquiry Cancel */
	btdev->commands[0]  |= 0x10;	/* Create Connection */
	btdev->commands[0]  |= 0x40;	/* Add SCO Connection */
	btdev->commands[0]  |= 0x80;	/* Cancel Create Connection */
	btdev->commands[1]  |= 0x01;	/* Accept Connection Request */
	btdev->commands[1]  |= 0x02;	/* Reject Connection Request */
	btdev->commands[1]  |= 0x04;	/* Link Key Request Reply */
	btdev->commands[1]  |= 0x08;	/* Link Key Request Negative Reply */
	btdev->commands[1]  |= 0x10;	/* PIN Code Request Reply */
	btdev->commands[1]  |= 0x20;	/* PIN Code Request Negative Reply */
	btdev->commands[1]  |= 0x80;	/* Authentication Requested */
	btdev->commands[2]  |= 0x01;	/* Set Connection Encryption */
	btdev->commands[2]  |= 0x08;	/* Remote Name Request */
	btdev->commands[2]  |= 0x10;	/* Cancel Remote Name Request */
	btdev->commands[2]  |= 0x20;	/* Read Remote Supported Features */
	btdev->commands[2]  |= 0x40;	/* Read Remote Extended Features */
	btdev->commands[3]  |= 0x01;	/* Read Clock Offset */
	btdev->commands[5]  |= 0x08;	/* Read Default Link Policy */
	btdev->commands[5]  |= 0x10;	/* Write Default Link Policy */
	btdev->commands[6]  |= 0x01;	/* Set Event Filter */
	btdev->commands[6]  |= 0x20;	/* Read Stored Link Key */
	btdev->commands[6]  |= 0x40;	/* Write Stored Link Key */
	btdev->commands[6]  |= 0x80;	/* Delete Stored Link Key */
	btdev->commands[7]  |= 0x01;	/* Write Local Name */
	btdev->commands[7]  |= 0x02;	/* Read Local Name */
	btdev->commands[7]  |= 0x04;	/* Read Connection Accept Timeout */
	btdev->commands[7]  |= 0x08;	/* Write Connection Accept Timeout */
	btdev->commands[7]  |= 0x10;	/* Read Page Timeout */
	btdev->commands[7]  |= 0x20;	/* Write Page Timeout */
	btdev->commands[7]  |= 0x40;	/* Read Scan Enable */
	btdev->commands[7]  |= 0x80;	/* Write Scan Enable */
	btdev->commands[8]  |= 0x01;	/* Read Page Scan Activity */
	btdev->commands[8]  |= 0x02;	/* Write Page Scan Activity */
	btdev->commands[8]  |= 0x04;	/* Read Inquiry Scan Activity */
	btdev->commands[8]  |= 0x08;	/* Write Inquiry Scan Activity */
	btdev->commands[8]  |= 0x10;	/* Read Authentication Enable */
	btdev->commands[8]  |= 0x20;	/* Write Authentication Enable */
	btdev->commands[9]  |= 0x01;	/* Read Class Of Device */
	btdev->commands[9]  |= 0x02;	/* Write Class Of Device */
	btdev->commands[9]  |= 0x04;	/* Read Voice Setting */
	btdev->commands[9]  |= 0x08;	/* Write Voice Setting */
	btdev->commands[11] |= 0x04;	/* Read Number of Supported IAC */
	btdev->commands[11] |= 0x08;	/* Read Current IAC LAP */
	btdev->commands[11] |= 0x10;	/* Write Current IAC LAP */
	btdev->commands[12] |= 0x40;	/* Read Inquiry Mode */
	btdev->commands[12] |= 0x80;	/* Write Inquiry Mode */
	btdev->commands[13] |= 0x01;	/* Read Page Scan Type */
	btdev->commands[13] |= 0x02;	/* Write Page Scan Type */
	btdev->commands[13] |= 0x04;	/* Read AFH Assess Mode */
	btdev->commands[13] |= 0x08;	/* Write AFH Assess Mode */
	btdev->commands[14] |= 0x40;	/* Read Local Extended Features */
	btdev->commands[15] |= 0x01;	/* Read Country Code */
	btdev->commands[16] |= 0x04;	/* Enable Device Under Test Mode */
}

static int cmd_setup_sync_conn(struct btdev *dev, const void *data, uint8_t len)
{
	cmd_status(dev, BT_HCI_ERR_SUCCESS, BT_HCI_EVT_SYNC_CONN_COMPLETE);

	return 0;
}

static int cmd_setup_sync_conn_complete(struct btdev *dev, const void *data,
							uint8_t len)
{
	const struct bt_hci_cmd_setup_sync_conn *cmd = data;
	struct bt_hci_evt_sync_conn_complete cc;
	struct btdev_conn *conn;

	memset(&cc, 0, sizeof(cc));

	conn = queue_find(dev->conns, match_handle,
				UINT_TO_PTR(le16_to_cpu(cmd->handle)));
	if (!conn) {
		cc.status = BT_HCI_ERR_UNKNOWN_CONN_ID;
		goto done;
	}

	conn = conn_add_sco(conn);
	if (!conn) {
		cc.status = BT_HCI_ERR_MEM_CAPACITY_EXCEEDED;
		goto done;
	}

	cc.status = BT_HCI_ERR_SUCCESS;
	memcpy(cc.bdaddr, conn->link->dev->bdaddr, 6);

	cc.handle = cpu_to_le16(conn->handle);
	cc.link_type = 0x02;
	cc.tx_interval = 0x000c;
	cc.retrans_window = 0x06;
	cc.rx_pkt_len = 60;
	cc.tx_pkt_len = 60;
	cc.air_mode = (cmd->voice_setting == 0x0060) ? 0x02 : 0x03;

done:
	send_event(dev, BT_HCI_EVT_SYNC_CONN_COMPLETE, &cc, sizeof(cc));

	return 0;
}

static int cmd_read_ext_inquiry(struct btdev *dev, const void *data,
							uint8_t len)
{
	struct bt_hci_rsp_read_ext_inquiry_response rsp;

	memset(&rsp, 0, sizeof(rsp));
	rsp.status = BT_HCI_ERR_SUCCESS;
	rsp.fec = dev->ext_inquiry_fec;
	memcpy(rsp.data, dev->ext_inquiry_rsp, 240);
	cmd_complete(dev, BT_HCI_CMD_READ_EXT_INQUIRY_RESPONSE, &rsp,
					sizeof(rsp));

	return 0;
}

static int cmd_write_ext_inquiry(struct btdev *dev, const void *data,
							uint8_t len)
{
	const struct bt_hci_cmd_write_ext_inquiry_response *cmd = data;
	uint8_t status = BT_HCI_ERR_SUCCESS;

	dev->ext_inquiry_fec = cmd->fec;
	memcpy(dev->ext_inquiry_rsp, cmd->data, 240);
	cmd_complete(dev, BT_HCI_CMD_WRITE_EXT_INQUIRY_RESPONSE, &status,
					sizeof(status));

	return 0;
}

static int cmd_read_ssp_mode(struct btdev *dev, const void *data, uint8_t len)
{
	struct bt_hci_rsp_read_simple_pairing_mode rsp;

	memset(&rsp, 0, sizeof(rsp));
	rsp.status = BT_HCI_ERR_SUCCESS;
	rsp.mode = dev->simple_pairing_mode;
	cmd_complete(dev, BT_HCI_CMD_READ_SIMPLE_PAIRING_MODE, &rsp,
					sizeof(rsp));

	return 0;
}

static int cmd_write_ssp_mode(struct btdev *dev, const void *data, uint8_t len)
{
	const struct bt_hci_cmd_write_simple_pairing_mode *cmd = data;
	uint8_t status = BT_HCI_ERR_SUCCESS;

	dev->simple_pairing_mode = cmd->mode;
	cmd_complete(dev, BT_HCI_CMD_WRITE_SIMPLE_PAIRING_MODE, &status,
					sizeof(status));

	return 0;
}

static int cmd_read_oob_data(struct btdev *dev, const void *data, uint8_t len)
{
	struct bt_hci_rsp_read_local_oob_data rsp;

	memset(&rsp, 0, sizeof(rsp));
	rsp.status = BT_HCI_ERR_SUCCESS;
	cmd_complete(dev, BT_HCI_CMD_READ_LOCAL_OOB_DATA, &rsp, sizeof(rsp));

	return 0;
}

static int cmd_read_inquiry_tx_power(struct btdev *dev, const void *data,
							uint8_t len)
{
	struct bt_hci_rsp_read_inquiry_resp_tx_power rsp;

	memset(&rsp, 0, sizeof(rsp));
	rsp.status = BT_HCI_ERR_SUCCESS;
	rsp.level = 0;
	cmd_complete(dev, BT_HCI_CMD_READ_INQUIRY_RESP_TX_POWER, &rsp,
					sizeof(rsp));

	return 0;
}

static int cmd_write_inquiry_tx_power(struct btdev *dev, const void *data,
							uint8_t len)
{
	return -ENOTSUP;
}

static int cmd_io_cap_reply(struct btdev *dev, const void *data, uint8_t len)
{
	const struct bt_hci_cmd_io_capability_request_reply *cmd = data;
	struct bt_hci_evt_io_capability_response ev;
	struct bt_hci_rsp_io_capability_request_reply rsp;
	struct btdev_conn *conn;
	uint8_t status;

	conn = queue_find(dev->conns, match_bdaddr, cmd->bdaddr);
	if (!conn) {
		status = BT_HCI_ERR_UNKNOWN_CONN_ID;
		goto done;
	}

	status = BT_HCI_ERR_SUCCESS;

	dev->io_cap = cmd->capability;
	dev->auth_req = cmd->authentication;

	memcpy(ev.bdaddr, dev->bdaddr, 6);
	ev.capability = cmd->capability;
	ev.oob_data = cmd->oob_data;
	ev.authentication = cmd->authentication;

	send_event(conn->link->dev, BT_HCI_EVT_IO_CAPABILITY_RESPONSE, &ev,
					sizeof(ev));

	if (conn->link->dev->io_cap) {
		struct bt_hci_evt_user_confirm_request cfm;

		memcpy(cfm.bdaddr, dev->bdaddr, 6);
		cfm.passkey = 0;

		send_event(conn->link->dev, BT_HCI_EVT_USER_CONFIRM_REQUEST,
							&cfm, sizeof(cfm));

		memcpy(cfm.bdaddr, cmd->bdaddr, 6);
		send_event(dev, BT_HCI_EVT_USER_CONFIRM_REQUEST,
							&cfm, sizeof(cfm));
	} else {
		send_event(conn->link->dev, BT_HCI_EVT_IO_CAPABILITY_REQUEST,
							dev->bdaddr, 6);
	}

done:
	rsp.status = status;
	memcpy(rsp.bdaddr, cmd->bdaddr, 6);
	cmd_complete(dev, BT_HCI_CMD_IO_CAPABILITY_REQUEST_REPLY,
							&rsp, sizeof(rsp));

	return 0;
}

static void ssp_complete(struct btdev *btdev, const uint8_t *bdaddr,
						uint8_t status, bool wait)
{
	struct bt_hci_evt_simple_pairing_complete iev, aev;
	struct btdev_conn *conn;
	struct btdev_conn *init, *accp;

	conn = queue_find(btdev->conns, match_bdaddr, bdaddr);
	if (!conn)
		return;

	btdev->ssp_status = status;
	btdev->ssp_auth_complete = true;

	if (!conn->link->dev->ssp_auth_complete && wait)
		return;

	if (status == BT_HCI_ERR_SUCCESS &&
			conn->link->dev->ssp_status != BT_HCI_ERR_SUCCESS)
		status = conn->link->dev->ssp_status;

	iev.status = status;
	aev.status = status;

	if (btdev->auth_init) {
		init = conn;
		accp = conn->link;
		memcpy(iev.bdaddr, bdaddr, 6);
		memcpy(aev.bdaddr, btdev->bdaddr, 6);
	} else {
		init = conn->link;
		accp = conn;
		memcpy(iev.bdaddr, btdev->bdaddr, 6);
		memcpy(aev.bdaddr, bdaddr, 6);
	}

	send_event(init->dev, BT_HCI_EVT_SIMPLE_PAIRING_COMPLETE, &iev,
							sizeof(iev));
	send_event(accp->dev, BT_HCI_EVT_SIMPLE_PAIRING_COMPLETE, &aev,
							sizeof(aev));

	if (status == BT_HCI_ERR_SUCCESS) {
		link_key_notify(init->dev, iev.bdaddr, LINK_KEY_DUMMY);
		link_key_notify(accp->dev, aev.bdaddr, LINK_KEY_DUMMY);
	}

	auth_complete(init, status);
}

static int cmd_user_confirm_reply(struct btdev *dev, const void *data,
							uint8_t len)
{
	struct bt_hci_rsp_user_confirm_request_reply rsp;

	memset(&rsp, 0, sizeof(rsp));
	rsp.status = BT_HCI_ERR_SUCCESS;
	memcpy(rsp.bdaddr, data, 6);
	cmd_complete(dev, BT_HCI_CMD_USER_CONFIRM_REQUEST_REPLY, &rsp,
					sizeof(rsp));
	ssp_complete(dev, data, BT_HCI_ERR_SUCCESS, true);

	return 0;
}

static int cmd_user_confirm_negative_reply(struct btdev *dev, const void *data,
							uint8_t len)
{
	struct bt_hci_rsp_user_confirm_request_neg_reply rsp;

	memset(&rsp, 0, sizeof(rsp));
	rsp.status = BT_HCI_ERR_SUCCESS;
	memcpy(rsp.bdaddr, data, 6);
	cmd_complete(dev, BT_HCI_CMD_USER_CONFIRM_REQUEST_NEG_REPLY,
					&rsp, sizeof(rsp));
	ssp_complete(dev, data, BT_HCI_ERR_AUTH_FAILURE, true);

	return 0;
}

static int cmd_user_passkey_reply(struct btdev *dev, const void *data,
							uint8_t len)
{
	return -ENOTSUP;
}

static int cmd_user_passkey_negative_reply(struct btdev *dev, const void *data,
							uint8_t len)
{
	return -ENOTSUP;
}

static int cmd_io_cap_negative_reply(struct btdev *dev, const void *data,
							uint8_t len)
{
	const struct bt_hci_cmd_io_capability_request_neg_reply *cmd = data;
	struct bt_hci_rsp_io_capability_request_neg_reply rsp;

	memset(&rsp, 0, sizeof(rsp));
	rsp.status = BT_HCI_ERR_SUCCESS;
	memcpy(rsp.bdaddr, cmd->bdaddr, 6);
	cmd_complete(dev, BT_HCI_CMD_IO_CAPABILITY_REQUEST_NEG_REPLY,
							&rsp, sizeof(rsp));

	ssp_complete(dev, cmd->bdaddr, BT_HCI_ERR_AUTH_FAILURE, false);

	return 0;
}

static int cmd_read_encrypt_key_size(struct btdev *dev, const void *data,
							uint8_t len)
{
	const struct bt_hci_cmd_read_encrypt_key_size *cmd = data;
	struct bt_hci_rsp_read_encrypt_key_size rsp;
	struct btdev_conn *conn;

	memset(&rsp, 0, sizeof(rsp));

	rsp.handle = cmd->handle;

	conn = queue_find(dev->conns, match_handle,
					UINT_TO_PTR(le16_to_cpu(cmd->handle)));
	if (conn) {
		rsp.status = BT_HCI_ERR_SUCCESS;
		rsp.key_size = 16;
	} else {
		rsp.status = BT_HCI_ERR_UNKNOWN_CONN_ID;
		rsp.key_size = 0;
	}

	cmd_complete(dev, BT_HCI_CMD_READ_ENCRYPT_KEY_SIZE, &rsp, sizeof(rsp));

	return 0;
}

static int cmd_read_data_block_size(struct btdev *dev, const void *data,
							uint8_t len)
{
	struct bt_hci_rsp_read_data_block_size rsp;

	memset(&rsp, 0, sizeof(rsp));
	rsp.status = BT_HCI_ERR_SUCCESS;
	rsp.max_acl_len = cpu_to_le16(dev->acl_mtu);
	rsp.block_len = cpu_to_le16(dev->acl_mtu);
	rsp.num_blocks = cpu_to_le16(dev->acl_max_pkt);
	cmd_complete(dev, BT_HCI_CMD_READ_DATA_BLOCK_SIZE, &rsp, sizeof(rsp));

	return 0;
}

static int cmd_read_local_codecs(struct btdev *dev, const void *data,
							uint8_t len)
{
	struct bt_hci_rsp_read_local_codecs *rsp;

	rsp = alloca(sizeof(*rsp) + 7);
	rsp->status = BT_HCI_ERR_SUCCESS;
	rsp->num_codecs = 0x06;
	rsp->codec[0] = 0x00;
	rsp->codec[1] = 0x01;
	rsp->codec[2] = 0x02;
	rsp->codec[3] = 0x03;
	rsp->codec[4] = 0x04;
	rsp->codec[5] = 0x05;
	rsp->codec[6] = 0x00;
	cmd_complete(dev, BT_HCI_CMD_READ_LOCAL_CODECS, rsp, sizeof(*rsp) + 7);

	return 0;
}

static int cmd_get_mws_transport_config(struct btdev *dev, const void *data,
							uint8_t len)
{
	struct bt_hci_rsp_get_mws_transport_config *rsp;

	rsp = alloca(sizeof(*rsp));
	rsp->status = BT_HCI_ERR_SUCCESS;
	rsp->num_transports = 0x00;
	cmd_complete(dev, BT_HCI_CMD_GET_MWS_TRANSPORT_CONFIG, rsp,
					sizeof(*rsp));

	return 0;
}

#define CMD_BREDR \
	CMD(BT_HCI_CMD_SETUP_SYNC_CONN, cmd_setup_sync_conn, \
					cmd_setup_sync_conn_complete), \
	CMD(BT_HCI_CMD_READ_EXT_INQUIRY_RESPONSE, cmd_read_ext_inquiry, NULL), \
	CMD(BT_HCI_CMD_WRITE_EXT_INQUIRY_RESPONSE, cmd_write_ext_inquiry, \
					NULL), \
	CMD(BT_HCI_CMD_READ_SIMPLE_PAIRING_MODE, cmd_read_ssp_mode, NULL), \
	CMD(BT_HCI_CMD_WRITE_SIMPLE_PAIRING_MODE, cmd_write_ssp_mode, NULL), \
	CMD(BT_HCI_CMD_READ_LOCAL_OOB_DATA, cmd_read_oob_data, NULL), \
	CMD(BT_HCI_CMD_READ_INQUIRY_RESP_TX_POWER, cmd_read_inquiry_tx_power, \
					NULL), \
	CMD(BT_HCI_CMD_WRITE_INQUIRY_TX_POWER, cmd_write_inquiry_tx_power, \
					NULL), \
	CMD(BT_HCI_CMD_IO_CAPABILITY_REQUEST_REPLY, cmd_io_cap_reply, NULL), \
	CMD(BT_HCI_CMD_USER_CONFIRM_REQUEST_REPLY, cmd_user_confirm_reply, \
					NULL), \
	CMD(BT_HCI_CMD_USER_CONFIRM_REQUEST_NEG_REPLY, \
					cmd_user_confirm_negative_reply, \
					NULL), \
	CMD(BT_HCI_CMD_USER_PASSKEY_REQUEST_NEG_REPLY, cmd_user_passkey_reply, \
					NULL), \
	CMD(BT_HCI_CMD_USER_PASSKEY_REQUEST_NEG_REPLY, \
					cmd_user_passkey_negative_reply, \
					NULL), \
	CMD(BT_HCI_CMD_IO_CAPABILITY_REQUEST_NEG_REPLY, \
					cmd_io_cap_negative_reply, NULL), \
	CMD(BT_HCI_CMD_READ_ENCRYPT_KEY_SIZE, cmd_read_encrypt_key_size, \
					NULL), \
	CMD(BT_HCI_CMD_READ_DATA_BLOCK_SIZE, cmd_read_data_block_size, NULL), \
	CMD(BT_HCI_CMD_READ_LOCAL_CODECS, cmd_read_local_codecs, NULL), \
	CMD(BT_HCI_CMD_GET_MWS_TRANSPORT_CONFIG, cmd_get_mws_transport_config, \
					NULL)

static const struct btdev_cmd cmd_bredr[] = {
	CMD_COMMON_ALL,
	CMD_COMMON_BREDR_LE,
	CMD_COMMON_BREDR_20,
	CMD_BREDR,
	{}
};

static void set_bredr_commands(struct btdev *btdev)
{
	set_common_commands_all(btdev);
	set_common_commands_bredrle(btdev);
	set_common_commands_bredr20(btdev);

	btdev->commands[16] |= 0x08;	/* Setup Synchronous Connection */
	btdev->commands[17] |= 0x01;	/* Read Extended Inquiry Response */
	btdev->commands[17] |= 0x02;	/* Write Extended Inquiry Response */
	btdev->commands[17] |= 0x20;	/* Read Simple Pairing Mode */
	btdev->commands[17] |= 0x40;	/* Write Simple Pairing Mode */
	btdev->commands[17] |= 0x80;	/* Read Local OOB Data */
	btdev->commands[18] |= 0x01;	/* Read Inquiry Response TX Power */
	btdev->commands[18] |= 0x02;	/* Write Inquiry Response TX Power */
	btdev->commands[18] |= 0x80;	/* IO Capability Request Reply */
	btdev->commands[19] |= 0x01;	/* User Confirmation Request Reply */
	btdev->commands[19] |= 0x02;	/* User Confirmation Request N Reply */
	btdev->commands[19] |= 0x04;	/* User Passkey Request Reply */
	btdev->commands[19] |= 0x08;	/* User Passkey Request N Reply */
	btdev->commands[20] |= 0x08;	/* IO Capability Request N Reply */
	btdev->commands[20] |= 0x10;	/* Read Encryption Key Size */
	btdev->commands[23] |= 0x04;	/* Read Data Block Size */
	btdev->commands[29] |= 0x20;	/* Read Local Supported Codecs */
	btdev->commands[30] |= 0x08;	/* Get MWS Transport Layer Config */
	btdev->cmds = cmd_bredr;
}

static const struct btdev_cmd cmd_bredr_20[] = {
	CMD_COMMON_ALL,
	CMD_COMMON_BREDR_LE,
	CMD_COMMON_BREDR_20,
	{}
};

static void set_bredr20_commands(struct btdev *btdev)
{
	set_common_commands_all(btdev);
	set_common_commands_bredrle(btdev);
	set_common_commands_bredr20(btdev);
	btdev->cmds = cmd_bredr_20;
}

static int cmd_read_le_host_supported(struct btdev *dev, const void *data,
							uint8_t len)
{
	struct bt_hci_rsp_read_le_host_supported rsp;

	memset(&rsp, 0, sizeof(rsp));

	rsp.status = BT_HCI_ERR_SUCCESS;
	rsp.supported = dev->le_supported;
	rsp.simultaneous = dev->le_simultaneous;
	cmd_complete(dev, BT_HCI_CMD_READ_LE_HOST_SUPPORTED, &rsp, sizeof(rsp));

	return 0;
}

static int cmd_write_le_host_supported(struct btdev *dev, const void *data,
							uint8_t len)
{
	const struct bt_hci_cmd_write_le_host_supported *cmd = data;
	uint8_t status;

	dev->le_supported = cmd->supported;
	dev->le_simultaneous = cmd->simultaneous;
	status = BT_HCI_ERR_SUCCESS;
	cmd_complete(dev, BT_HCI_CMD_WRITE_LE_HOST_SUPPORTED, &status,
						sizeof(status));

	return 0;
}

static int cmd_le_set_event_mask(struct btdev *dev, const void *data,
							uint8_t len)
{
	const struct bt_hci_cmd_le_set_event_mask *cmd = data;
	uint8_t status;

	memcpy(dev->le_event_mask, cmd->mask, 8);
	status = BT_HCI_ERR_SUCCESS;
	cmd_complete(dev, BT_HCI_CMD_LE_SET_EVENT_MASK, &status,
						sizeof(status));

	return 0;
}

static int cmd_le_read_buffer_size(struct btdev *dev, const void *data,
							uint8_t len)
{
	struct bt_hci_rsp_le_read_buffer_size rsp;

	memset(&rsp, 0, sizeof(rsp));

	rsp.status = BT_HCI_ERR_SUCCESS;
	rsp.le_mtu = cpu_to_le16(dev->acl_mtu);
	rsp.le_max_pkt = dev->acl_max_pkt;
	cmd_complete(dev, BT_HCI_CMD_LE_READ_BUFFER_SIZE, &rsp, sizeof(rsp));

	return 0;
}

static int cmd_le_read_local_features(struct btdev *dev, const void *data,
							uint8_t len)
{
	struct bt_hci_rsp_le_read_local_features rsp;

	memset(&rsp, 0, sizeof(rsp));

	rsp.status = BT_HCI_ERR_SUCCESS;
	memcpy(rsp.features, dev->le_features, 8);
	cmd_complete(dev, BT_HCI_CMD_LE_READ_LOCAL_FEATURES, &rsp, sizeof(rsp));

	return 0;
}

static int cmd_set_random_address(struct btdev *dev, const void *data,
							uint8_t len)
{
	const struct bt_hci_cmd_le_set_random_address *cmd = data;
	uint8_t status;

	memcpy(dev->random_addr, cmd->addr, 6);
	status = BT_HCI_ERR_SUCCESS;
	cmd_complete(dev, BT_HCI_CMD_LE_SET_RANDOM_ADDRESS, &status,
						sizeof(status));

	return 0;
}

static uint16_t ext_legacy_adv_type(uint8_t type)
{
	switch (type) {
	case 0x00:
		/* Connectable undirected - ADV_IND" */
		return 0x0013;
	case 0x01:
		/* Connectable directed - ADV_DIRECT_IND */
		return 0x0015;
	case 0x02:
		/* Scannable undirected - ADV_SCAN_IND */
		return 0x0012;
	case 0x03:
		/* Non connectable undirected - ADV_NONCONN_IND */
		return 0x0010;
	case 0x04:
		/* Scan response - SCAN_RSP */
		return 0x0012;
	}

	return 0x0000;
}

static int cmd_set_adv_params(struct btdev *dev, const void *data, uint8_t len)
{
	const struct bt_hci_cmd_le_set_adv_parameters *cmd = data;
	uint8_t status;

	if (dev->le_adv_enable) {
		status = BT_HCI_ERR_COMMAND_DISALLOWED;
		goto done;
	}

	dev->le_adv_type = cmd->type;
	/* Use Legacy PDU if the remote is using EXT Scan */
	dev->le_ext_adv_type = ext_legacy_adv_type(cmd->type);
	dev->le_adv_own_addr = cmd->own_addr_type;
	dev->le_adv_direct_addr_type = cmd->direct_addr_type;
	memcpy(dev->le_adv_direct_addr, cmd->direct_addr, 6);

	status = BT_HCI_ERR_SUCCESS;

done:
	cmd_complete(dev, BT_HCI_CMD_LE_SET_ADV_PARAMETERS, &status,
		     sizeof(status));

	return 0;
}

static int cmd_read_adv_tx_power(struct btdev *dev, const void *data,
							uint8_t len)
{
	struct bt_hci_rsp_le_read_adv_tx_power rsp;

	rsp.status = BT_HCI_ERR_SUCCESS;
	rsp.level = 0;
	cmd_complete(dev, BT_HCI_CMD_LE_READ_ADV_TX_POWER, &rsp, sizeof(rsp));

	return 0;
}

static int cmd_set_adv_data(struct btdev *dev, const void *data, uint8_t len)
{
	const struct bt_hci_cmd_le_set_adv_data *cmd = data;
	uint8_t status;

	dev->le_adv_data_len = cmd->len;
	memcpy(dev->le_adv_data, cmd->data, 31);
	status = BT_HCI_ERR_SUCCESS;

	cmd_complete(dev, BT_HCI_CMD_LE_SET_ADV_DATA, &status, sizeof(status));

	return 0;
}

static int cmd_set_scan_rsp_data(struct btdev *dev, const void *data,
							uint8_t len)
{
	const struct bt_hci_cmd_le_set_scan_rsp_data *cmd = data;
	uint8_t status;

	dev->le_scan_data_len = cmd->len;
	memcpy(dev->le_scan_data, cmd->data, 31);
	status = BT_HCI_ERR_SUCCESS;
	cmd_complete(dev, BT_HCI_CMD_LE_SET_SCAN_RSP_DATA, &status,
						sizeof(status));

	return 0;
}

static uint8_t get_ext_adv_type(uint8_t ext_adv_type)
{
	/*
	 * If legacy bit is not set then just reset high duty cycle directed
	 * bit.
	 */
	if (!(ext_adv_type & 0x10))
		return (ext_adv_type & 0xf7);

	/*
	 * Connectable low duty cycle directed advertising creates a
	 * connectable directed advertising report type.
	 */
	if (ext_adv_type == 0x001d)
		return 0x0015;

	return ext_adv_type;
}

static const uint8_t *scan_addr(const struct btdev *btdev)
{
	if (btdev->le_scan_own_addr_type == 0x01)
		return btdev->random_addr;

	return btdev->bdaddr;
}

static bool ext_adv_match(struct btdev *scan, struct btdev *adv)
{
	/* Match everything if this is not directed advertising */
	if (!(adv->le_ext_adv_type & 0x04))
		return true;

	if (scan->le_scan_own_addr_type != adv->le_adv_direct_addr_type)
		return false;

	return !memcmp(scan_addr(scan), adv->le_adv_direct_addr, 6);
}

static const uint8_t *adv_addr(const struct btdev *btdev)
{
	if (btdev->le_adv_own_addr == 0x01)
		return btdev->random_addr;

	return btdev->bdaddr;
}

static void le_send_adv_report(struct btdev *btdev, const struct btdev *remote,
								uint8_t type)
{
	struct __packed {
		uint8_t subevent;
		union {
			struct bt_hci_evt_le_adv_report lar;
			uint8_t raw[10 + 31 + 1];
		};
	} meta_event;

	meta_event.subevent = BT_HCI_EVT_LE_ADV_REPORT;

	memset(&meta_event.lar, 0, sizeof(meta_event.lar));
	meta_event.lar.num_reports = 1;
	meta_event.lar.event_type = type;
	meta_event.lar.addr_type = remote->le_adv_own_addr;
	memcpy(meta_event.lar.addr, adv_addr(remote), 6);

	/* Scan or advertising response */
	if (type == 0x04) {
		meta_event.lar.data_len = remote->le_scan_data_len;
		memcpy(meta_event.lar.data, remote->le_scan_data,
						meta_event.lar.data_len);
	} else {
		meta_event.lar.data_len = remote->le_adv_data_len;
		memcpy(meta_event.lar.data, remote->le_adv_data,
						meta_event.lar.data_len);
	}
	/* Not available */
	meta_event.raw[10 + meta_event.lar.data_len] = 127;
	send_event(btdev, BT_HCI_EVT_LE_META_EVENT, &meta_event,
					1 + 10 + meta_event.lar.data_len + 1);
}

static uint8_t get_adv_report_type(uint8_t adv_type)
{
	/*
	 * Connectable low duty cycle directed advertising creates a
	 * connectable directed advertising report type.
	 */
	if (adv_type == 0x04)
		return 0x01;

	return adv_type;
}

static bool adv_match(struct btdev *scan, struct btdev *adv)
{
	/* Match everything if this is not directed advertising */
	if (adv->le_adv_type != 0x01 && adv->le_adv_type != 0x04)
		return true;

	if (scan->le_scan_own_addr_type != adv->le_adv_direct_addr_type)
		return false;

	return !memcmp(scan_addr(scan), adv->le_adv_direct_addr, 6);
}

static void le_set_adv_enable_complete(struct btdev *btdev)
{
	uint8_t report_type;
	int i;

	report_type = get_adv_report_type(btdev->le_adv_type);

	for (i = 0; i < MAX_BTDEV_ENTRIES; i++) {
		if (!btdev_list[i] || btdev_list[i] == btdev)
			continue;

		if (!btdev_list[i]->le_scan_enable)
			continue;

		if (!adv_match(btdev_list[i], btdev))
			continue;

		le_send_adv_report(btdev_list[i], btdev, report_type);

		if (btdev_list[i]->le_scan_type != 0x01)
			continue;

		/* ADV_IND & ADV_SCAN_IND generate a scan response */
		if (btdev->le_adv_type == 0x00 || btdev->le_adv_type == 0x02)
			le_send_adv_report(btdev_list[i], btdev, 0x04);
	}
}

static int cmd_set_adv_enable(struct btdev *dev, const void *data, uint8_t len)
{
	const struct bt_hci_cmd_le_set_ext_adv_enable *cmd = data;
	uint8_t status;

	if (dev->le_adv_enable == cmd->enable) {
		status = BT_HCI_ERR_COMMAND_DISALLOWED;
		goto done;
	}

	dev->le_adv_enable = cmd->enable;
	status = BT_HCI_ERR_SUCCESS;

done:
	cmd_complete(dev, BT_HCI_CMD_LE_SET_ADV_ENABLE, &status,
						sizeof(status));

	if (!status && dev->le_adv_enable)
		le_set_adv_enable_complete(dev);

	return 0;
}

static int cmd_set_scan_params(struct btdev *dev, const void *data, uint8_t len)
{
	const struct bt_hci_cmd_le_set_scan_parameters *cmd = data;
	uint8_t status;

	if (dev->le_scan_enable) {
		status = BT_HCI_ERR_COMMAND_DISALLOWED;
		goto done;
	}

	status = BT_HCI_ERR_SUCCESS;
	dev->le_scan_type = cmd->type;
	dev->le_scan_own_addr_type = cmd->own_addr_type;

done:
	cmd_complete(dev, BT_HCI_CMD_LE_SET_SCAN_PARAMETERS, &status,
						sizeof(status));

	return 0;
}

static int cmd_set_scan_enable(struct btdev *dev, const void *data, uint8_t len)
{
	const struct bt_hci_cmd_le_set_scan_enable *cmd = data;
	uint8_t status;

	if (dev->le_scan_enable == cmd->enable) {
		status = BT_HCI_ERR_COMMAND_DISALLOWED;
		goto done;
	}

	dev->le_scan_enable = cmd->enable;
	dev->le_filter_dup = cmd->filter_dup;
	status = BT_HCI_ERR_SUCCESS;

done:
	cmd_complete(dev, BT_HCI_CMD_LE_SET_SCAN_ENABLE, &status,
						sizeof(status));

	return 0;
}

static int cmd_set_scan_enable_complete(struct btdev *dev, const void *data,
							uint8_t len)
{
	const struct bt_hci_cmd_le_set_scan_enable *cmd = data;
	int i;

	if (!dev->le_scan_enable || !cmd->enable)
		return 0;

	for (i = 0; i < MAX_BTDEV_ENTRIES; i++) {
		uint8_t report_type;

		if (!btdev_list[i] || btdev_list[i] == dev)
			continue;

		if (!btdev_list[i]->le_adv_enable)
			continue;

		if (!adv_match(dev, btdev_list[i]))
			continue;

		report_type = get_adv_report_type(btdev_list[i]->le_adv_type);
		le_send_adv_report(dev, btdev_list[i], report_type);

		if (dev->le_scan_type != 0x01)
			continue;

		/* ADV_IND & ADV_SCAN_IND generate a scan response */
		if (btdev_list[i]->le_adv_type == 0x00 ||
					btdev_list[i]->le_adv_type == 0x02)
			le_send_adv_report(dev, btdev_list[i], 0x04);
	}

	return 0;
}

static bool adv_connectable(struct btdev *btdev)
{
	if (!btdev->le_adv_enable)
		return false;

	return btdev->le_adv_type != 0x03;
}

static void le_meta_event(struct btdev *btdev, uint8_t event,
						void *data, uint8_t len)
{
	void *pkt_data;

	util_debug(btdev->debug_callback, btdev->debug_data,
				"meta event 0x%02x", event);

	pkt_data = alloca(1 + len);
	if (!pkt_data)
		return;

	((uint8_t *) pkt_data)[0] = event;

	if (len > 0)
		memcpy(pkt_data + 1, data, len);

	send_event(btdev, BT_HCI_EVT_LE_META_EVENT, pkt_data, 1 + len);
}

static void le_conn_complete(struct btdev *btdev,
				const struct bt_hci_cmd_le_create_conn *lecc,
				uint8_t status)
{
	struct bt_hci_evt_le_conn_complete cc;

	memset(&cc, 0, sizeof(cc));

	if (!status) {
		struct btdev_conn *conn;

		conn = conn_add_acl(btdev, lecc->peer_addr,
						lecc->peer_addr_type);
		if (!conn)
			return;

		btdev->le_adv_enable = 0;
		conn->link->dev->le_adv_enable = 0;

		cc.status = status;
		cc.peer_addr_type = btdev->le_scan_own_addr_type;
		if (cc.peer_addr_type == 0x01)
			memcpy(cc.peer_addr, btdev->random_addr, 6);
		else
			memcpy(cc.peer_addr, btdev->bdaddr, 6);

		cc.role = 0x01;
		cc.handle = cpu_to_le16(conn->handle);
		cc.interval = lecc->max_interval;
		cc.latency = lecc->latency;
		cc.supv_timeout = lecc->supv_timeout;
		le_meta_event(conn->link->dev, BT_HCI_EVT_LE_CONN_COMPLETE,
					&cc, sizeof(cc));
	}

	cc.status = status;
	cc.peer_addr_type = lecc->peer_addr_type;
	memcpy(cc.peer_addr, lecc->peer_addr, 6);
	cc.role = 0x00;

	le_meta_event(btdev, BT_HCI_EVT_LE_CONN_COMPLETE, &cc, sizeof(cc));
}

static int cmd_le_create_conn(struct btdev *dev, const void *data, uint8_t len)
{
	cmd_status(dev, BT_HCI_ERR_SUCCESS, BT_HCI_CMD_LE_CREATE_CONN);

	return 0;
}

static int cmd_le_create_conn_complete(struct btdev *dev, const void *data,
					uint8_t len)
{
	const struct bt_hci_cmd_le_create_conn *cmd = data;
	struct btdev *remote;

	dev->le_scan_own_addr_type = cmd->own_addr_type;

	remote = find_btdev_by_bdaddr_type(cmd->peer_addr, cmd->peer_addr_type);
	if (remote && adv_connectable(remote) && adv_match(dev, remote) &&
				remote->le_adv_own_addr == cmd->peer_addr_type)
		le_conn_complete(dev, cmd, 0);
	else
		le_conn_complete(dev, cmd, BT_HCI_ERR_CONN_FAILED_TO_ESTABLISH);

	return 0;
}

static int cmd_read_wl_size(struct btdev *dev, const void *data, uint8_t len)
{
	struct bt_hci_rsp_le_read_white_list_size rsp;

	rsp.status = BT_HCI_ERR_SUCCESS;
	rsp.size = 0;
	cmd_complete(dev, BT_HCI_CMD_LE_READ_WHITE_LIST_SIZE, &rsp,
						sizeof(rsp));

	return 0;
}

static int cmd_clear_wl(struct btdev *dev, const void *data, uint8_t len)
{
	uint8_t status;

	status = BT_HCI_ERR_SUCCESS;
	cmd_complete(dev, BT_HCI_CMD_LE_CLEAR_WHITE_LIST, &status,
						sizeof(status));

	return 0;
}

static int cmd_conn_update(struct btdev *dev, const void *data, uint8_t len)
{
	cmd_status(dev, BT_HCI_ERR_SUCCESS, BT_HCI_CMD_LE_CONN_UPDATE);

	return 0;
}

static void le_conn_update(struct btdev *btdev, uint16_t handle,
				uint16_t min_interval, uint16_t max_interval,
				uint16_t latency, uint16_t supv_timeout,
				uint16_t min_length, uint16_t max_length)
{
	struct bt_hci_evt_le_conn_update_complete ev;
	struct btdev_conn *conn;

	memset(&ev, 0, sizeof(ev));

	ev.handle = cpu_to_le16(handle);
	ev.interval = cpu_to_le16(min_interval);
	ev.latency = cpu_to_le16(latency);
	ev.supv_timeout = cpu_to_le16(supv_timeout);

	conn = queue_find(btdev->conns, match_handle, UINT_TO_PTR(handle));
	if (conn)
		ev.status = BT_HCI_ERR_SUCCESS;
	else
		ev.status = BT_HCI_ERR_UNKNOWN_CONN_ID;

	le_meta_event(btdev, BT_HCI_EVT_LE_CONN_UPDATE_COMPLETE, &ev,
					sizeof(ev));

	if (conn)
		le_meta_event(conn->link->dev,
					BT_HCI_EVT_LE_CONN_UPDATE_COMPLETE,
					&ev, sizeof(ev));
}

static void le_conn_param_req(struct btdev *btdev, uint16_t handle,
				uint16_t min_interval, uint16_t max_interval,
				uint16_t latency, uint16_t supv_timeout,
				uint16_t min_length, uint16_t max_length)
{
	struct bt_hci_evt_le_conn_param_request ev;
	struct btdev_conn *conn;

	conn = queue_find(btdev->conns, match_handle, UINT_TO_PTR(handle));
	if (!conn)
		return;

	memset(&ev, 0, sizeof(ev));

	ev.handle = cpu_to_le16(handle);
	ev.min_interval = cpu_to_le16(min_interval);
	ev.max_interval = cpu_to_le16(max_interval);
	ev.latency = cpu_to_le16(latency);
	ev.supv_timeout = cpu_to_le16(supv_timeout);

	le_meta_event(conn->link->dev, BT_HCI_EVT_LE_CONN_PARAM_REQUEST, &ev,
					sizeof(ev));
}

static int cmd_conn_update_complete(struct btdev *dev, const void *data,
						uint8_t len)
{
	const struct bt_hci_cmd_le_conn_update *cmd = data;

	if (dev->le_features[0] & 0x02)
		le_conn_param_req(dev, le16_to_cpu(cmd->handle),
					le16_to_cpu(cmd->min_interval),
					le16_to_cpu(cmd->max_interval),
					le16_to_cpu(cmd->latency),
					le16_to_cpu(cmd->supv_timeout),
					le16_to_cpu(cmd->min_length),
					le16_to_cpu(cmd->max_length));
	else
		le_conn_update(dev, le16_to_cpu(cmd->handle),
					le16_to_cpu(cmd->min_interval),
					le16_to_cpu(cmd->max_interval),
					le16_to_cpu(cmd->latency),
					le16_to_cpu(cmd->supv_timeout),
					le16_to_cpu(cmd->min_length),
					le16_to_cpu(cmd->max_length));

	return 0;
}

static int cmd_le_read_remote_features(struct btdev *dev, const void *data,
							uint8_t len)
{
	const struct bt_hci_cmd_read_remote_features *cmd = data;
	struct bt_hci_evt_le_remote_features_complete ev;
	struct btdev_conn *conn;
	uint8_t status = BT_HCI_ERR_SUCCESS;

	conn = queue_find(dev->conns, match_handle,
				UINT_TO_PTR(le16_to_cpu(cmd->handle)));
	if (!conn)
		status = BT_HCI_ERR_UNKNOWN_CONN_ID;

	cmd_status(dev, status, BT_HCI_CMD_LE_READ_REMOTE_FEATURES);

	if (status)
		return 0;

	memset(&ev, 0, sizeof(ev));
	ev.status = BT_HCI_ERR_SUCCESS;
	ev.handle = cpu_to_le16(conn->handle);
	memcpy(ev.features, conn->link->dev->le_features, 8);

	le_meta_event(dev, BT_HCI_EVT_LE_REMOTE_FEATURES_COMPLETE, &ev,
						sizeof(ev));

	return 0;
}

static int cmd_encrypt(struct btdev *dev, const void *data, uint8_t len)
{
	const struct bt_hci_cmd_le_encrypt *cmd = data;
	struct bt_hci_rsp_le_encrypt rsp;

	if (!bt_crypto_e(dev->crypto, cmd->key, cmd->plaintext, rsp.data)) {
		cmd_status(dev, BT_HCI_ERR_COMMAND_DISALLOWED,
			   BT_HCI_CMD_LE_ENCRYPT);
		return 0;
	}

	rsp.status = BT_HCI_ERR_SUCCESS;
	cmd_complete(dev, BT_HCI_CMD_LE_ENCRYPT, &rsp, sizeof(rsp));

	return 0;
}

static int cmd_rand(struct btdev *dev, const void *data, uint8_t len)
{
	struct bt_hci_rsp_le_rand rsp;

	if (!bt_crypto_random_bytes(dev->crypto,
				    (uint8_t *)&rsp.number, 8)) {
		cmd_status(dev, BT_HCI_ERR_COMMAND_DISALLOWED,
			   BT_HCI_CMD_LE_RAND);
		return 0;
	}

	rsp.status = BT_HCI_ERR_SUCCESS;
	cmd_complete(dev, BT_HCI_CMD_LE_RAND, &rsp, sizeof(rsp));

	return 0;
}

static int cmd_start_encrypt(struct btdev *dev, const void *data, uint8_t len)
{
	const struct bt_hci_cmd_le_start_encrypt *cmd = data;
	struct bt_hci_evt_le_long_term_key_request ev;
	struct btdev_conn *conn;

	conn = queue_find(dev->conns, match_handle,
				UINT_TO_PTR(le16_to_cpu(cmd->handle)));
	if (!conn) {
		cmd_status(dev, BT_HCI_ERR_UNKNOWN_CONN_ID,
				BT_HCI_CMD_LE_START_ENCRYPT);
		return 0;
	}

	cmd_status(dev, BT_HCI_ERR_SUCCESS, BT_HCI_CMD_LE_START_ENCRYPT);

	memcpy(dev->le_ltk, cmd->ltk, 16);

	ev.handle = cpu_to_le16(conn->handle);
	ev.ediv = cmd->ediv;
	ev.rand = cmd->rand;

	le_meta_event(conn->link->dev, BT_HCI_EVT_LE_LONG_TERM_KEY_REQUEST, &ev,
					sizeof(ev));

	return 0;
}

static int cmd_ltk_reply(struct btdev *dev, const void *data, uint8_t len)
{
	const struct bt_hci_cmd_le_ltk_req_reply *cmd = data;
	struct bt_hci_rsp_le_ltk_req_reply rp;
	struct btdev_conn *conn;
	uint8_t mode, status;

	conn = queue_find(dev->conns, match_handle,
				UINT_TO_PTR(le16_to_cpu(cmd->handle)));
	if (!conn) {
		rp.status = BT_HCI_ERR_UNKNOWN_CONN_ID;
		cmd_complete(dev, BT_HCI_CMD_LE_LTK_REQ_REPLY, &rp, sizeof(rp));
		return 0;
	}

	memcpy(dev->le_ltk, cmd->ltk, 16);

	memset(&rp, 0, sizeof(rp));
	rp.handle = cpu_to_le16(conn->handle);

	rp.status = BT_HCI_ERR_SUCCESS;
	cmd_complete(dev, BT_HCI_CMD_LE_LTK_REQ_REPLY, &rp, sizeof(rp));

	if (memcmp(dev->le_ltk, conn->link->dev->le_ltk, 16)) {
		status = BT_HCI_ERR_AUTH_FAILURE;
		mode = 0x00;
	} else {
		status = BT_HCI_ERR_SUCCESS;
		mode = 0x01;
	}

	encrypt_change(conn, mode, status);
	encrypt_change(conn->link, mode, status);

	return 0;
}

static int cmd_ltk_neg_reply(struct btdev *dev, const void *data, uint8_t len)
{
	const struct bt_hci_cmd_le_ltk_req_neg_reply *cmd = data;
	struct bt_hci_rsp_le_ltk_req_neg_reply rp;
	struct btdev_conn *conn;

	conn = queue_find(dev->conns, match_handle,
				UINT_TO_PTR(le16_to_cpu(cmd->handle)));
	if (!conn) {
		rp.status = BT_HCI_ERR_UNKNOWN_CONN_ID;
		cmd_complete(dev, BT_HCI_CMD_LE_LTK_REQ_NEG_REPLY, &rp,
							sizeof(rp));
		return 0;
	}

	memset(&rp, 0, sizeof(rp));
	rp.handle = cpu_to_le16(conn->handle);
	rp.status = BT_HCI_ERR_SUCCESS;
	cmd_complete(dev, BT_HCI_CMD_LE_LTK_REQ_NEG_REPLY, &rp, sizeof(rp));

	encrypt_change(conn->link, 0x00, BT_HCI_ERR_PIN_OR_KEY_MISSING);

	return 0;
}

static int cmd_le_read_supported_states(struct btdev *dev, const void *data,
							uint8_t len)
{
	struct bt_hci_rsp_le_read_supported_states rsp;

	memset(&rsp, 0, sizeof(0));
	rsp.status = BT_HCI_ERR_SUCCESS;
	memcpy(rsp.states, dev->le_states, 8);
	cmd_complete(dev, BT_HCI_CMD_LE_READ_SUPPORTED_STATES, &rsp,
						sizeof(rsp));

	return 0;
}

static int cmd_rx_test(struct btdev *dev, const void *data, uint8_t len)
{
	return -ENOTSUP;
}

static int cmd_tx_test(struct btdev *dev, const void *data, uint8_t len)
{
	return -ENOTSUP;
}

static int cmd_test_end(struct btdev *dev, const void *data, uint8_t len)
{
	return -ENOTSUP;
}

static int cmd_conn_param_reply(struct btdev *dev, const void *data,
						uint8_t len)
{
	uint8_t status = BT_HCI_ERR_SUCCESS;

	cmd_complete(dev, BT_HCI_CMD_LE_CONN_PARAM_REQ_REPLY, &status,
						sizeof(status));

	return 0;
}

static int cmd_conn_param_reply_complete(struct btdev *dev, const void *data,
						uint8_t len)
{
	const struct bt_hci_cmd_le_conn_param_req_reply *cmd = data;

	le_conn_update(dev, le16_to_cpu(cmd->handle),
				le16_to_cpu(cmd->min_interval),
				le16_to_cpu(cmd->max_interval),
				le16_to_cpu(cmd->latency),
				le16_to_cpu(cmd->supv_timeout),
				le16_to_cpu(cmd->min_length),
				le16_to_cpu(cmd->max_length));

	return 0;
}

static int cmd_conn_param_neg_reply(struct btdev *dev, const void *data,
						uint8_t len)
{
	const struct bt_hci_cmd_le_conn_param_req_neg_reply *cmd = data;
	struct bt_hci_rsp_le_conn_param_req_neg_reply rsp;

	memset(&rsp, 0, sizeof(rsp));

	rsp.handle = cmd->handle;
	rsp.status = BT_HCI_ERR_SUCCESS;
	cmd_complete(dev, BT_HCI_CMD_LE_CONN_PARAM_REQ_NEG_REPLY, &rsp,
						sizeof(rsp));

	return 0;
}

static int cmd_conn_param_neg_reply_complete(struct btdev *dev,
						const void *data, uint8_t len)
{
	const struct bt_hci_cmd_le_conn_param_req_neg_reply *cmd = data;
	struct btdev_conn *conn;
	struct bt_hci_evt_le_conn_update_complete ev;

	conn = queue_find(dev->conns, match_handle,
				UINT_TO_PTR(le16_to_cpu(cmd->handle)));
	if (!conn)
		return 0;

	memset(&ev, 0, sizeof(ev));

	ev.handle = cpu_to_le16(cmd->handle);
	ev.status = cpu_to_le16(cmd->reason);

	le_meta_event(conn->link->dev, BT_HCI_EVT_LE_CONN_UPDATE_COMPLETE, &ev,
						sizeof(ev));

	return 0;
}

static int cmd_read_local_pk256(struct btdev *dev, const void *data,
						uint8_t len)
{
	return -ENOTSUP;
}

static int cmd_gen_dhkey(struct btdev *dev, const void *data, uint8_t len)
{
	return -ENOTSUP;
}

#define CMD_LE \
	CMD(BT_HCI_CMD_READ_LE_HOST_SUPPORTED, cmd_read_le_host_supported, \
					NULL), \
	CMD(BT_HCI_CMD_WRITE_LE_HOST_SUPPORTED, cmd_write_le_host_supported, \
					NULL), \
	CMD(BT_HCI_CMD_LE_SET_EVENT_MASK, cmd_le_set_event_mask, NULL), \
	CMD(BT_HCI_CMD_LE_READ_BUFFER_SIZE, cmd_le_read_buffer_size, NULL), \
	CMD(BT_HCI_CMD_LE_READ_LOCAL_FEATURES, cmd_le_read_local_features, \
					NULL), \
	CMD(BT_HCI_CMD_LE_SET_RANDOM_ADDRESS, cmd_set_random_address, NULL), \
	CMD(BT_HCI_CMD_LE_SET_ADV_PARAMETERS, cmd_set_adv_params, NULL), \
	CMD(BT_HCI_CMD_LE_READ_ADV_TX_POWER, cmd_read_adv_tx_power, NULL), \
	CMD(BT_HCI_CMD_LE_SET_ADV_DATA, cmd_set_adv_data, NULL), \
	CMD(BT_HCI_CMD_LE_SET_SCAN_RSP_DATA, cmd_set_scan_rsp_data, NULL), \
	CMD(BT_HCI_CMD_LE_SET_ADV_ENABLE, cmd_set_adv_enable, NULL), \
	CMD(BT_HCI_CMD_LE_SET_SCAN_PARAMETERS, cmd_set_scan_params, NULL), \
	CMD(BT_HCI_CMD_LE_SET_SCAN_ENABLE, cmd_set_scan_enable, \
					cmd_set_scan_enable_complete), \
	CMD(BT_HCI_CMD_LE_CREATE_CONN, cmd_le_create_conn, \
					cmd_le_create_conn_complete), \
	CMD(BT_HCI_CMD_LE_READ_WHITE_LIST_SIZE, cmd_read_wl_size, NULL), \
	CMD(BT_HCI_CMD_LE_CLEAR_WHITE_LIST, cmd_clear_wl, NULL), \
	CMD(BT_HCI_CMD_LE_CONN_UPDATE, cmd_conn_update, \
					cmd_conn_update_complete), \
	CMD(BT_HCI_CMD_LE_READ_REMOTE_FEATURES, cmd_le_read_remote_features, \
					NULL), \
	CMD(BT_HCI_CMD_LE_ENCRYPT, cmd_encrypt, NULL), \
	CMD(BT_HCI_CMD_LE_RAND, cmd_rand, NULL), \
	CMD(BT_HCI_CMD_LE_START_ENCRYPT, cmd_start_encrypt, NULL), \
	CMD(BT_HCI_CMD_LE_LTK_REQ_REPLY, cmd_ltk_reply, NULL), \
	CMD(BT_HCI_CMD_LE_LTK_REQ_NEG_REPLY, cmd_ltk_neg_reply, NULL), \
	CMD(BT_HCI_CMD_LE_READ_SUPPORTED_STATES, cmd_le_read_supported_states, \
					NULL), \
	CMD(BT_HCI_CMD_LE_RECEIVER_TEST, cmd_rx_test, NULL), \
	CMD(BT_HCI_CMD_LE_TRANSMITTER_TEST, cmd_tx_test, NULL), \
	CMD(BT_HCI_CMD_LE_ISO_TEST_END, cmd_test_end, NULL), \
	CMD(BT_HCI_CMD_LE_CONN_PARAM_REQ_REPLY, cmd_conn_param_reply, \
					cmd_conn_param_reply_complete), \
	CMD(BT_HCI_CMD_LE_CONN_PARAM_REQ_NEG_REPLY, cmd_conn_param_neg_reply, \
					cmd_conn_param_neg_reply_complete), \
	CMD(BT_HCI_CMD_LE_READ_LOCAL_PK256, cmd_read_local_pk256, NULL), \
	CMD(BT_HCI_CMD_LE_GENERATE_DHKEY, cmd_gen_dhkey, NULL)

static int cmd_set_default_phy(struct btdev *dev, const void *data,
							uint8_t len)
{
	const struct bt_hci_cmd_le_set_default_phy *cmd = data;
	uint8_t status;

	if (cmd->all_phys > 0x03 || (!(cmd->all_phys & 0x01) &&
			(!cmd->tx_phys || cmd->tx_phys > 0x07)) ||
			(!(cmd->all_phys & 0x02) &&
			(!cmd->rx_phys || cmd->rx_phys > 0x07)))
		status = BT_HCI_ERR_INVALID_PARAMETERS;
	else
		status = BT_HCI_ERR_SUCCESS;

	cmd_complete(dev, BT_HCI_CMD_LE_SET_DEFAULT_PHY, &status,
					sizeof(status));

	return 0;
}

static int cmd_set_adv_rand_addr(struct btdev *dev, const void *data,
							uint8_t len)
{
	const struct bt_hci_cmd_le_set_adv_set_rand_addr *cmd = data;
	uint8_t status = BT_HCI_ERR_SUCCESS;

	memcpy(dev->random_addr, cmd->bdaddr, 6);
	cmd_complete(dev, BT_HCI_CMD_LE_SET_ADV_SET_RAND_ADDR, &status,
						sizeof(status));

	return 0;
}

static int cmd_set_ext_adv_params(struct btdev *dev, const void *data,
							uint8_t len)
{
	const struct bt_hci_cmd_le_set_ext_adv_params *cmd = data;
	struct bt_hci_rsp_le_set_ext_adv_params rsp;
	uint8_t status;

	if (dev->le_adv_enable) {
		status = BT_HCI_ERR_COMMAND_DISALLOWED;
		cmd_complete(dev, BT_HCI_CMD_LE_SET_EXT_ADV_PARAMS, &status,
							sizeof(status));
		return 0;
	}

	dev->le_ext_adv_type = le16_to_cpu(cmd->evt_properties);
	dev->le_adv_own_addr = cmd->own_addr_type;
	dev->le_adv_direct_addr_type = cmd->peer_addr_type;
	memcpy(dev->le_adv_direct_addr, cmd->peer_addr, 6);

	rsp.status = BT_HCI_ERR_SUCCESS;
	rsp.tx_power = 0;
	cmd_complete(dev, BT_HCI_CMD_LE_SET_EXT_ADV_PARAMS, &rsp, sizeof(rsp));

	return 0;
}

static int cmd_set_ext_adv_data(struct btdev *dev, const void *data,
							uint8_t len)
{
	const struct bt_hci_cmd_le_set_ext_adv_data *cmd = data;
	uint8_t status = BT_HCI_ERR_SUCCESS;

	dev->le_adv_data_len = cmd->data_len;
	memcpy(dev->le_adv_data, cmd->data, 31);
	cmd_complete(dev, BT_HCI_CMD_LE_SET_EXT_ADV_DATA, &status,
						sizeof(status));

	return 0;
}

static int cmd_set_ext_scan_rsp_data(struct btdev *dev, const void *data,
							uint8_t len)
{
	const struct bt_hci_cmd_le_set_ext_scan_rsp_data *cmd = data;
	uint8_t status = BT_HCI_ERR_SUCCESS;

	dev->le_scan_data_len = cmd->data_len;
	memcpy(dev->le_scan_data, cmd->data, 31);
	cmd_complete(dev, BT_HCI_CMD_LE_SET_EXT_SCAN_RSP_DATA, &status,
						sizeof(status));

	return 0;
}

static void send_ext_adv(struct btdev *btdev, const struct btdev *remote,
					uint16_t type, bool is_scan_rsp)
{
	struct __packed {
		uint8_t num_reports;
		union {
			struct bt_hci_le_ext_adv_report lear;
			uint8_t raw[24 + 31];
		};
	} meta_event;

	memset(&meta_event.lear, 0, sizeof(meta_event.lear));
	meta_event.num_reports = 1;
	meta_event.lear.event_type = cpu_to_le16(type);
	meta_event.lear.addr_type = remote->le_adv_own_addr;
	memcpy(meta_event.lear.addr, adv_addr(remote), 6);
	meta_event.lear.rssi = 127;
	meta_event.lear.tx_power = 127;
	/* Right now we dont care about phy in adv report */
	meta_event.lear.primary_phy = 0x01;
	meta_event.lear.secondary_phy = 0x01;

	/* Scan or advertising response */
	if (is_scan_rsp) {
		meta_event.lear.data_len = remote->le_scan_data_len;
		memcpy(meta_event.lear.data, remote->le_scan_data,
						meta_event.lear.data_len);
	} else {
		meta_event.lear.data_len = remote->le_adv_data_len;
		memcpy(meta_event.lear.data, remote->le_adv_data,
						meta_event.lear.data_len);
	}

	le_meta_event(btdev, BT_HCI_EVT_LE_EXT_ADV_REPORT, &meta_event,
					1 + 24 + meta_event.lear.data_len);
}

static void le_set_ext_adv_enable_complete(struct btdev *btdev)
{
	uint16_t report_type;
	int i;

	report_type = get_ext_adv_type(btdev->le_ext_adv_type);

	for (i = 0; i < MAX_BTDEV_ENTRIES; i++) {
		if (!btdev_list[i] || btdev_list[i] == btdev)
			continue;

		if (!btdev_list[i]->le_scan_enable)
			continue;

		if (!ext_adv_match(btdev_list[i], btdev))
			continue;

		send_ext_adv(btdev_list[i], btdev, report_type, false);

		if (btdev_list[i]->le_scan_type != 0x01)
			continue;

		/* if scannable bit is set the send scan response */
		if (btdev->le_ext_adv_type & 0x02) {
			if (btdev->le_ext_adv_type == 0x13)
				report_type = 0x1b;
			else if (btdev->le_ext_adv_type == 0x12)
				report_type = 0x1a;
			else if (!(btdev->le_ext_adv_type & 0x10))
				report_type &= 0x08;
			else
				continue;

			send_ext_adv(btdev_list[i], btdev, report_type, true);
		}
	}
}

static int cmd_set_ext_adv_enable(struct btdev *dev, const void *data,
							uint8_t len)
{
	const struct bt_hci_cmd_le_set_ext_adv_enable *cmd = data;
	uint8_t status;

	if (dev->le_adv_enable == cmd->enable)
		status = BT_HCI_ERR_COMMAND_DISALLOWED;
	else {
		const struct bt_hci_cmd_ext_adv_set *eas;

		if (cmd->num_of_sets) {
			eas = data + sizeof(*cmd);
			if (eas->duration || cmd->num_of_sets > 1)
				status = BT_HCI_ERR_INVALID_PARAMETERS;
			else
				status = BT_HCI_ERR_SUCCESS;
		} else
			status = BT_HCI_ERR_SUCCESS;

		if (status == BT_HCI_ERR_SUCCESS)
			dev->le_adv_enable = cmd->enable;
	}

	cmd_complete(dev, BT_HCI_CMD_LE_SET_EXT_ADV_ENABLE, &status,
							sizeof(status));

	if (status == BT_HCI_ERR_SUCCESS && dev->le_adv_enable)
		le_set_ext_adv_enable_complete(dev);

	return 0;
}

static int cmd_read_max_adv_data_len(struct btdev *dev, const void *data,
							uint8_t len)
{
	/* TODO */
	return -ENOTSUP;
}

static int cmd_read_num_adv_sets(struct btdev *dev, const void *data,
							uint8_t len)
{
	struct bt_hci_rsp_le_read_num_supported_adv_sets rsp;

	memset(&rsp, 0, sizeof(rsp));

	rsp.status = BT_HCI_ERR_SUCCESS;
	/* Support one set as of now */
	rsp.num_of_sets = 1;
	cmd_complete(dev, BT_HCI_CMD_LE_READ_NUM_SUPPORTED_ADV_SETS, &rsp,
							sizeof(rsp));

	return 0;
}

static int cmd_remove_adv_set(struct btdev *dev, const void *data,
							uint8_t len)
{
	uint8_t status = BT_HCI_ERR_SUCCESS;

	cmd_complete(dev, BT_HCI_CMD_LE_REMOVE_ADV_SET, &status,
							sizeof(status));

	return 0;
}

static int cmd_clear_adv_sets(struct btdev *dev, const void *data,
							uint8_t len)
{
	uint8_t status = BT_HCI_ERR_SUCCESS;

	cmd_complete(dev, BT_HCI_CMD_LE_CLEAR_ADV_SETS, &status,
							sizeof(status));

	return 0;
}

static int cmd_set_per_adv_params(struct btdev *dev, const void *data,
							uint8_t len)
{
	/* TODO */
	return -ENOTSUP;
}

static int cmd_set_per_adv_data(struct btdev *dev, const void *data,
							uint8_t len)
{
	/* TODO */
	return -ENOTSUP;
}

static int cmd_set_per_adv_enable(struct btdev *dev, const void *data,
							uint8_t len)
{
	/* TODO */
	return -ENOTSUP;
}

static int cmd_set_ext_scan_params(struct btdev *dev, const void *data,
							uint8_t len)
{
	const struct bt_hci_cmd_le_set_ext_scan_params *cmd = data;
	const struct bt_hci_le_scan_phy *scan = (void *)cmd->data;
	uint8_t status;

	if (dev->le_scan_enable)
		status = BT_HCI_ERR_COMMAND_DISALLOWED;
	else if (cmd->num_phys == 0)
		status = BT_HCI_ERR_INVALID_PARAMETERS;
	else {
		status = BT_HCI_ERR_SUCCESS;
		/* Currently we dont support multiple types in single
		 * command So just take the first one will do.
		 */
		dev->le_scan_type = scan->type;
		dev->le_scan_own_addr_type = cmd->own_addr_type;
	}

	cmd_complete(dev, BT_HCI_CMD_LE_SET_EXT_SCAN_PARAMS, &status,
							sizeof(status));

	return 0;
}

static int cmd_set_ext_scan_enable(struct btdev *dev, const void *data,
							uint8_t len)
{
	const struct bt_hci_cmd_le_set_ext_scan_enable *cmd = data;
	uint8_t status;

	if (dev->le_scan_enable == cmd->enable)
		status = BT_HCI_ERR_COMMAND_DISALLOWED;
	else {
		dev->le_scan_enable = cmd->enable;
		dev->le_filter_dup = cmd->filter_dup;
		status = BT_HCI_ERR_SUCCESS;
	}

	cmd_complete(dev, BT_HCI_CMD_LE_SET_EXT_SCAN_ENABLE, &status,
							sizeof(status));

	return 0;
}

static int cmd_set_ext_scan_enable_complete(struct btdev *dev, const void *data,
							uint8_t len)
{
	const struct bt_hci_cmd_le_set_ext_scan_enable *cmd = data;
	int i;

	if (!dev->le_scan_enable || !cmd->enable)
		return 0;

	for (i = 0; i < MAX_BTDEV_ENTRIES; i++) {
		uint16_t report_type;

		if (!btdev_list[i] || btdev_list[i] == dev)
			continue;

		if (!btdev_list[i]->le_adv_enable)
			continue;

		if (!ext_adv_match(dev, btdev_list[i]))
			continue;

		report_type = get_ext_adv_type(btdev_list[i]->le_ext_adv_type);
		send_ext_adv(dev, btdev_list[i], report_type, false);

		if (dev->le_scan_type != 0x01)
			continue;

		/* if scannable bit is set the send scan response */
		if (btdev_list[i]->le_ext_adv_type & 0x02) {
			if (btdev_list[i]->le_ext_adv_type == 0x13)
				report_type = 0x1b;
			else if (btdev_list[i]->le_ext_adv_type == 0x12)
				report_type = 0x1a;
			else if (!(btdev_list[i]->le_ext_adv_type & 0x10))
				report_type &= 0x08;
			else
				continue;

			send_ext_adv(dev, btdev_list[i], report_type, true);
		}
	}

	return 0;
}

static int cmd_ext_create_conn(struct btdev *dev, const void *data, uint8_t len)
{
	cmd_status(dev, BT_HCI_ERR_SUCCESS, BT_HCI_CMD_LE_EXT_CREATE_CONN);

	return 0;
}

static bool ext_adv_connectable(struct btdev *btdev)
{
	if (!btdev->le_adv_enable)
		return false;

	return btdev->le_ext_adv_type & 0x01;
}

static void le_ext_conn_complete(struct btdev *btdev,
			const struct bt_hci_cmd_le_ext_create_conn *cmd,
			uint8_t status)
{
	struct bt_hci_evt_le_enhanced_conn_complete ev;
	struct bt_hci_le_ext_create_conn *lecc = (void *)cmd->data;

	memset(&ev, 0, sizeof(ev));

	if (!status) {
		struct btdev_conn *conn;

		conn = conn_add_acl(btdev, cmd->peer_addr, cmd->peer_addr_type);
		if (!conn)
			return;

		if (!btdev->le_ext_adv_type) {
			btdev->le_adv_enable = 0;
			conn->link->dev->le_adv_enable = 0;
		}

		ev.status = status;
		ev.peer_addr_type = btdev->le_scan_own_addr_type;
		if (ev.peer_addr_type == 0x01)
			memcpy(ev.peer_addr, btdev->random_addr, 6);
		else
			memcpy(ev.peer_addr, btdev->bdaddr, 6);

		ev.role = 0x01;
		ev.handle = cpu_to_le16(conn->handle);
		ev.interval = lecc->max_interval;
		ev.latency = lecc->latency;
		ev.supv_timeout = lecc->supv_timeout;

		le_meta_event(conn->link->dev,
				BT_HCI_EVT_LE_ENHANCED_CONN_COMPLETE, &ev,
				sizeof(ev));
	}

	ev.status = status;
	ev.peer_addr_type = cmd->peer_addr_type;
	memcpy(ev.peer_addr, cmd->peer_addr, 6);
	ev.role = 0x00;

	le_meta_event(btdev, BT_HCI_EVT_LE_ENHANCED_CONN_COMPLETE, &ev,
						sizeof(ev));
}

static int cmd_ext_create_conn_complete(struct btdev *dev, const void *data,
							uint8_t len)
{
	const struct bt_hci_cmd_le_ext_create_conn *cmd = data;
	struct btdev *remote;

	dev->le_scan_own_addr_type = cmd->own_addr_type;

	remote = find_btdev_by_bdaddr_type(cmd->peer_addr, cmd->peer_addr_type);
	if (remote && ext_adv_connectable(remote) &&
			ext_adv_match(dev, remote) &&
			remote->le_adv_own_addr == cmd->peer_addr_type)
		le_ext_conn_complete(dev, cmd, 0);
	else
		le_ext_conn_complete(dev, cmd,
					BT_HCI_ERR_CONN_FAILED_TO_ESTABLISH);

	return 0;
}

static int cmd_per_adv_create_sync(struct btdev *dev, const void *data,
							uint8_t len)
{
	/* TODO */
	return -ENOTSUP;
}

static int cmd_per_adv_create_sync_cancel(struct btdev *dev, const void *data,
							uint8_t len)
{
	/* TODO */
	return -ENOTSUP;
}

static int cmd_per_adv_term_sync(struct btdev *dev, const void *data,
							uint8_t len)
{
	/* TODO */
	return -ENOTSUP;
}

static int cmd_per_adv_add(struct btdev *dev, const void *data, uint8_t len)
{
	/* TODO */
	return -ENOTSUP;
}

static int cmd_per_adv_remove(struct btdev *dev, const void *data, uint8_t len)
{
	/* TODO */
	return -ENOTSUP;
}

static int cmd_per_adv_clear(struct btdev *dev, const void *data, uint8_t len)
{
	/* TODO */
	return -ENOTSUP;
}

static int cmd_read_per_adv_list_size(struct btdev *dev, const void *data,
							uint8_t len)
{
	/* TODO */
	return -ENOTSUP;
}

static int cmd_read_tx_power(struct btdev *dev, const void *data, uint8_t len)
{
	struct bt_hci_rsp_le_read_tx_power rsp;

	memset(&rsp, 0, sizeof(rsp));

	rsp.status = BT_HCI_ERR_SUCCESS;
	/* a random default value */
	rsp.max_tx_power = 0x07;
	rsp.min_tx_power = 0xDE;

	cmd_complete(dev, BT_HCI_CMD_LE_READ_TX_POWER, &rsp, sizeof(rsp));

	return 0;
}

#define CMD_LE_50 \
	CMD(BT_HCI_CMD_LE_SET_DEFAULT_PHY, cmd_set_default_phy,	NULL), \
	CMD(BT_HCI_CMD_LE_SET_ADV_SET_RAND_ADDR, cmd_set_adv_rand_addr, NULL), \
	CMD(BT_HCI_CMD_LE_SET_EXT_ADV_PARAMS, cmd_set_ext_adv_params, NULL), \
	CMD(BT_HCI_CMD_LE_SET_EXT_ADV_DATA, cmd_set_ext_adv_data, NULL), \
	CMD(BT_HCI_CMD_LE_SET_EXT_SCAN_RSP_DATA, cmd_set_ext_scan_rsp_data, \
					NULL), \
	CMD(BT_HCI_CMD_LE_SET_EXT_ADV_ENABLE, cmd_set_ext_adv_enable, NULL), \
	CMD(BT_HCI_CMD_LE_READ_MAX_ADV_DATA_LEN, cmd_read_max_adv_data_len, \
					NULL), \
	CMD(BT_HCI_CMD_LE_READ_NUM_SUPPORTED_ADV_SETS, cmd_read_num_adv_sets, \
					NULL), \
	CMD(BT_HCI_CMD_LE_REMOVE_ADV_SET, cmd_remove_adv_set, NULL), \
	CMD(BT_HCI_CMD_LE_CLEAR_ADV_SETS, cmd_clear_adv_sets, NULL), \
	CMD(BT_HCI_CMD_LE_SET_PERIODIC_ADV_PARAMS, cmd_set_per_adv_params, \
					NULL), \
	CMD(BT_HCI_CMD_LE_SET_PERIODIC_ADV_DATA, cmd_set_per_adv_data, NULL), \
	CMD(BT_HCI_CMD_LE_SET_PERIODIC_ADV_ENABLE, cmd_set_per_adv_enable, \
					NULL), \
	CMD(BT_HCI_CMD_LE_SET_EXT_SCAN_PARAMS, cmd_set_ext_scan_params, NULL), \
	CMD(BT_HCI_CMD_LE_SET_EXT_SCAN_ENABLE, cmd_set_ext_scan_enable, \
					cmd_set_ext_scan_enable_complete), \
	CMD(BT_HCI_CMD_LE_EXT_CREATE_CONN, cmd_ext_create_conn, \
					cmd_ext_create_conn_complete), \
	CMD(BT_HCI_CMD_LE_PERIODIC_ADV_CREATE_SYNC, cmd_per_adv_create_sync, \
					NULL), \
	CMD(BT_HCI_CMD_LE_PERIODIC_ADV_CREATE_SYNC_CANCEL, \
					cmd_per_adv_create_sync_cancel, NULL), \
	CMD(BT_HCI_CMD_LE_PERIODIC_ADV_TERM_SYNC, cmd_per_adv_term_sync, \
					NULL), \
	CMD(BT_HCI_CMD_LE_ADD_DEV_PERIODIC_ADV_LIST, cmd_per_adv_add, NULL), \
	CMD(BT_HCI_CMD_LE_REMOVE_DEV_PERIODIC_ADV_LIST, cmd_per_adv_remove, \
					NULL), \
	CMD(BT_HCI_CMD_LE_CLEAR_PERIODIC_ADV_LIST, cmd_per_adv_clear, NULL), \
	CMD(BT_HCI_CMD_LE_READ_PERIODIC_ADV_LIST_SIZE, \
					cmd_read_per_adv_list_size, NULL), \
	CMD(BT_HCI_CMD_LE_READ_TX_POWER, cmd_read_tx_power, NULL)

static const struct btdev_cmd cmd_le_5_0[] = {
	CMD_COMMON_ALL,
	CMD_COMMON_BREDR_LE,
	CMD_LE,
	CMD_LE_50,
	{}
};

static void set_le_50_commands(struct btdev *btdev)
{
	btdev->commands[35] |= 0x20;	/* LE Set Default PHY */
	btdev->commands[36] |= 0x02;	/* LE Set Adv Set Random Address */
	btdev->commands[36] |= 0x04;	/* LE Set Ext Adv Parameters */
	btdev->commands[36] |= 0x08;	/* LE Set Ext Adv Data */
	btdev->commands[36] |= 0x10;	/* LE Set Ext Scan Response Data */
	btdev->commands[36] |= 0x20;	/* LE Set Ext Adv Enable */
	btdev->commands[36] |= 0x40;	/* LE Read Maximum Adv Data Length */
	btdev->commands[36] |= 0x80;	/* LE Read Num of Supported Adv Sets */
	btdev->commands[37] |= 0x01;	/* LE Remove Adv Set */
	btdev->commands[37] |= 0x02;	/* LE Clear Adv Sets */
	btdev->commands[37] |= 0x04;	/* LE Set Periodic Adv Parameters */
	btdev->commands[37] |= 0x08;	/* LE Set Periodic Adv Data */
	btdev->commands[37] |= 0x10;	/* LE Set Periodic Adv Enable */
	btdev->commands[37] |= 0x20;	/* LE Set Ext Scan Parameters */
	btdev->commands[37] |= 0x40;	/* LE Set Ext Scan Enable */
	btdev->commands[37] |= 0x80;	/* LE Ext Create Connection */
	btdev->commands[38] |= 0x01;	/* LE Periodic Adv Create Sync */
	btdev->commands[38] |= 0x02;	/* LE Periodic Adv Create Sync Cancel */
	btdev->commands[38] |= 0x04;	/* LE Periodic Adv Terminate Sync */
	btdev->commands[38] |= 0x08;	/* LE Add Device To Periodic Adv List */
	btdev->commands[38] |= 0x10;	/* LE Remove Periodic Adv List */
	btdev->commands[38] |= 0x20;	/* LE Clear Periodic Adv List */
	btdev->commands[38] |= 0x40;	/* LE Read Periodic Adv List Size */
	btdev->commands[38] |= 0x80;	/* LE Read Transmit Power */
	btdev->cmds = cmd_le_5_0;
}

static int cmd_read_size_v2(struct btdev *dev, const void *data,
							uint8_t len)
{
	struct bt_hci_rsp_le_read_buffer_size_v2 rsp;

	memset(&rsp, 0, sizeof(rsp));

	rsp.status = BT_HCI_ERR_SUCCESS;
	rsp.acl_mtu = cpu_to_le16(dev->acl_mtu);
	rsp.acl_max_pkt = dev->acl_max_pkt;
	rsp.iso_mtu = cpu_to_le16(dev->iso_mtu);
	rsp.iso_max_pkt = dev->iso_max_pkt;
	cmd_complete(dev, BT_HCI_CMD_LE_READ_BUFFER_SIZE_V2, &rsp, sizeof(rsp));

	return 0;
}

static int cmd_read_iso_tx_sync(struct btdev *dev, const void *data,
							uint8_t len)
{
	/* TODO */
	return -ENOTSUP;
}

static int cmd_set_cig_params(struct btdev *dev, const void *data,
							uint8_t len)
{
	const struct bt_hci_cmd_le_set_cig_params *cmd = data;
	struct lescp {
		struct bt_hci_rsp_le_set_cig_params params;
		uint16_t handle[3];
	} __attribute__ ((packed)) rsp;
	int i = 0;

	memset(&rsp, 0, sizeof(rsp));

	memcpy(&dev->le_cig, data, len);

	if (cmd->num_cis > ARRAY_SIZE(rsp.handle)) {
		rsp.params.status = BT_HCI_ERR_MEM_CAPACITY_EXCEEDED;
		goto done;
	}

	rsp.params.status = BT_HCI_ERR_SUCCESS;
	rsp.params.cig_id = cmd->cig_id;

	for (i = 0; i < cmd->num_cis; i++) {
		rsp.params.num_handles++;
		rsp.handle[i] = cpu_to_le16(ISO_HANDLE + i);
	}

done:
	cmd_complete(dev, BT_HCI_CMD_LE_SET_CIG_PARAMS, &rsp,
				sizeof(rsp.params) + (i * sizeof(uint16_t)));

	return 0;
}

static int cmd_set_cig_params_test(struct btdev *dev, const void *data,
							uint8_t len)
{
	/* TODO */
	return -ENOTSUP;
}

static int cmd_create_cis(struct btdev *dev, const void *data, uint8_t len)
{
	cmd_status(dev, BT_HCI_ERR_SUCCESS, BT_HCI_CMD_LE_CREATE_CIS);

	return 0;
}

static void le_cis_estabilished(struct btdev *dev, struct btdev_conn *conn,
						uint8_t status)
{
	struct bt_hci_evt_le_cis_established evt;

	memset(&evt, 0, sizeof(evt));

	evt.status = status;

	if (!evt.status) {
		struct btdev *remote = conn->link->dev;

		evt.conn_handle = cpu_to_le16(conn->handle);
		/* TODO: Figure out if these values makes sense */
		memcpy(evt.cig_sync_delay, remote->le_cig.params.m_interval,
				sizeof(remote->le_cig.params.m_interval));
		memcpy(evt.cis_sync_delay, remote->le_cig.params.s_interval,
				sizeof(remote->le_cig.params.s_interval));
		memcpy(evt.m_latency, &remote->le_cig.params.m_latency,
				sizeof(remote->le_cig.params.m_latency));
		memcpy(evt.s_latency, &remote->le_cig.params.s_latency,
				sizeof(remote->le_cig.params.s_latency));
		evt.m_phy = remote->le_cig.cis.m_phy;
		evt.s_phy = remote->le_cig.cis.s_phy;
		evt.nse = 0x01;
		evt.m_bn = 0x01;
		evt.s_bn = 0x01;
		evt.m_ft = 0x01;
		evt.s_ft = 0x01;
		evt.m_mtu = remote->le_cig.cis.m_sdu;
		evt.s_mtu = remote->le_cig.cis.s_sdu;
		evt.interval = remote->le_cig.params.m_latency;
	}

	le_meta_event(dev, BT_HCI_EVT_LE_CIS_ESTABLISHED, &evt, sizeof(evt));

	if (conn)
		le_meta_event(conn->link->dev, BT_HCI_EVT_LE_CIS_ESTABLISHED,
						&evt, sizeof(evt));
}

static int cmd_create_cis_complete(struct btdev *dev, const void *data,
							uint8_t len)
{
	const struct bt_hci_cmd_le_create_cis *cmd = data;
	int i;

	for (i = 0; i < cmd->num_cis; i++) {
		const struct bt_hci_cis *cis = &cmd->cis[i];
		struct btdev_conn *acl;
		struct btdev_conn *iso;
		struct bt_hci_evt_le_cis_req evt;

		acl = queue_find(dev->conns, match_handle,
				UINT_TO_PTR(cpu_to_le16(cis->acl_handle)));
		if (!acl) {
			le_cis_estabilished(dev, NULL,
						BT_HCI_ERR_UNKNOWN_CONN_ID);
			break;
		}

		iso = queue_find(dev->conns, match_handle,
				UINT_TO_PTR(cpu_to_le16(cis->cis_handle)));
		if (!iso) {
			iso = conn_add_iso(acl, cpu_to_le16(cis->cis_handle));
			if (!iso) {
				le_cis_estabilished(dev, NULL,
						BT_HCI_ERR_UNKNOWN_CONN_ID);
				break;
			}
		}

		evt.acl_handle = cpu_to_le16(acl->handle);
		evt.cis_handle = cpu_to_le16(iso->handle);
		evt.cig_id = 0x00;
		evt.cis_id = 0x00;

		le_meta_event(iso->link->dev, BT_HCI_EVT_LE_CIS_REQ, &evt,
					sizeof(evt));
	}

	return 0;
}

static int cmd_remove_cig(struct btdev *dev, const void *data, uint8_t len)
{
	struct bt_hci_rsp_le_remove_cig rsp;

	memset(&dev->le_cig, 0, sizeof(dev->le_cig));
	memset(&rsp, 0, sizeof(rsp));

	rsp.status = BT_HCI_ERR_SUCCESS;
	rsp.cig_id = 0x00;
	cmd_complete(dev, BT_HCI_CMD_LE_REMOVE_CIG, &rsp, sizeof(rsp));

	return 0;
}

static int cmd_accept_cis(struct btdev *dev, const void *data, uint8_t len)
{
	const struct bt_hci_cmd_le_accept_cis *cmd = data;
	struct btdev_conn *conn;

	conn = queue_find(dev->conns, match_handle,
				UINT_TO_PTR(cpu_to_le16(cmd->handle)));
	if (!conn) {
		cmd_status(dev, BT_HCI_ERR_UNKNOWN_CONN_ID,
					BT_HCI_CMD_LE_ACCEPT_CIS);
		return 0;
	}

	cmd_status(dev, BT_HCI_ERR_SUCCESS, BT_HCI_CMD_LE_ACCEPT_CIS);
	le_cis_estabilished(dev, conn, BT_HCI_ERR_SUCCESS);

	return 0;
}

static int cmd_reject_cis(struct btdev *dev, const void *data, uint8_t len)
{
	const struct bt_hci_cmd_le_reject_cis *cmd = data;
	struct btdev_conn *conn;

	conn = queue_find(dev->conns, match_handle,
				UINT_TO_PTR(cpu_to_le16(cmd->handle)));
	if (!conn) {
		cmd_status(dev, BT_HCI_ERR_UNKNOWN_CONN_ID,
					BT_HCI_CMD_LE_REJECT_CIS);
		return 0;
	}

	cmd_status(dev, BT_HCI_ERR_SUCCESS, BT_HCI_CMD_LE_REJECT_CIS);
	le_cis_estabilished(dev, conn, cmd->reason);

	return 0;
}

static int cmd_create_big(struct btdev *dev, const void *data, uint8_t len)
{
	/* TODO */
	return -ENOTSUP;
}

static int cmd_create_big_test(struct btdev *dev, const void *data, uint8_t len)
{
	/* TODO */
	return -ENOTSUP;
}

static int cmd_term_big(struct btdev *dev, const void *data, uint8_t len)
{
	/* TODO */
	return -ENOTSUP;
}

static int cmd_big_create_sync(struct btdev *dev, const void *data, uint8_t len)
{
	/* TODO */
	return -ENOTSUP;
}

static int cmd_big_term_sync(struct btdev *dev, const void *data, uint8_t len)
{
	/* TODO */
	return -ENOTSUP;
}

static int cmd_req_peer_sca(struct btdev *dev, const void *data, uint8_t len)
{
	/* TODO */
	return -ENOTSUP;
}

static int cmd_setup_iso_path(struct btdev *dev, const void *data, uint8_t len)
{
	const struct bt_hci_cmd_le_setup_iso_path *cmd = data;
	struct bt_hci_rsp_le_setup_iso_path rsp;
	struct btdev_conn *conn;

	memset(&rsp, 0, sizeof(rsp));

	conn = queue_find(dev->conns, match_handle,
				UINT_TO_PTR(cpu_to_le16(cmd->handle)));
	if (!conn) {
		rsp.status = BT_HCI_ERR_UNKNOWN_CONN_ID;
		goto done;
	}

	/* Only support HCI or disabled paths */
	if (cmd->path && cmd->path != 0xff) {
		rsp.status = BT_HCI_ERR_INVALID_PARAMETERS;
		goto done;
	}

	switch (cmd->direction) {
	case 0x00:
		dev->le_iso_path[0] = cmd->path;
		rsp.handle = cpu_to_le16(conn->handle);
		break;
	case 0x01:
		dev->le_iso_path[1] = cmd->path;
		rsp.handle = cpu_to_le16(conn->handle);
		break;
	default:
		rsp.status = BT_HCI_ERR_INVALID_PARAMETERS;
	}

done:
	cmd_complete(dev, BT_HCI_CMD_LE_SETUP_ISO_PATH, &rsp, sizeof(rsp));

	return 0;
}

static int cmd_remove_iso_path(struct btdev *dev, const void *data, uint8_t len)
{
	const struct bt_hci_cmd_le_remove_iso_path *cmd = data;
	uint8_t status = BT_HCI_ERR_SUCCESS;
	struct btdev_conn *conn;

	conn = queue_find(dev->conns, match_handle,
				UINT_TO_PTR(cpu_to_le16(cmd->handle)));
	if (!conn) {
		status = BT_HCI_ERR_UNKNOWN_CONN_ID;
		goto done;
	}

	switch (cmd->direction) {
	case 0x00:
		dev->le_iso_path[0] = 0x00;
		break;
	case 0x01:
		dev->le_iso_path[1] = 0x00;
		break;
	default:
		status = BT_HCI_ERR_INVALID_PARAMETERS;
	}

done:
	cmd_complete(dev, BT_HCI_CMD_LE_REMOVE_ISO_PATH, &status,
							sizeof(status));

	return 0;
}

static int cmd_iso_tx_test(struct btdev *dev, const void *data, uint8_t len)
{
	/* TODO */
	return -ENOTSUP;
}

static int cmd_iso_rx_test(struct btdev *dev, const void *data, uint8_t len)
{
	/* TODO */
	return -ENOTSUP;
}

static int cmd_iso_read_test_counter(struct btdev *dev, const void *data,
							uint8_t len)
{
	/* TODO */
	return -ENOTSUP;
}

static int cmd_iso_test_end(struct btdev *dev, const void *data, uint8_t len)
{
	/* TODO */
	return -ENOTSUP;
}

static int cmd_set_host_feature(struct btdev *dev, const void *data,
							uint8_t len)
{
	uint8_t status = BT_HCI_ERR_SUCCESS;

	cmd_complete(dev, BT_HCI_CMD_LE_SET_HOST_FEATURE, &status,
							sizeof(status));

	return 0;
}

#define CMD_LE_52 \
	CMD(BT_HCI_CMD_LE_READ_BUFFER_SIZE_V2, cmd_read_size_v2, NULL), \
	CMD(BT_HCI_CMD_LE_READ_ISO_TX_SYNC, cmd_read_iso_tx_sync, NULL), \
	CMD(BT_HCI_CMD_LE_SET_EVENT_MASK, cmd_le_set_event_mask, NULL), \
	CMD(BT_HCI_CMD_LE_SET_CIG_PARAMS, cmd_set_cig_params, NULL), \
	CMD(BT_HCI_CMD_LE_SET_CIG_PARAMS_TEST, cmd_set_cig_params_test, NULL), \
	CMD(BT_HCI_CMD_LE_CREATE_CIS, cmd_create_cis, \
					cmd_create_cis_complete), \
	CMD(BT_HCI_CMD_LE_REMOVE_CIG, cmd_remove_cig, NULL), \
	CMD(BT_HCI_CMD_LE_ACCEPT_CIS, cmd_accept_cis, NULL), \
	CMD(BT_HCI_CMD_LE_REJECT_CIS, cmd_reject_cis, NULL), \
	CMD(BT_HCI_CMD_LE_CREATE_BIG, cmd_create_big, NULL), \
	CMD(BT_HCI_CMD_LE_CREATE_BIG_TEST, cmd_create_big_test, NULL), \
	CMD(BT_HCI_CMD_LE_TERM_BIG, cmd_term_big, NULL), \
	CMD(BT_HCI_CMD_LE_BIG_CREATE_SYNC, cmd_big_create_sync, NULL), \
	CMD(BT_HCI_CMD_LE_BIG_TERM_SYNC, cmd_big_term_sync, NULL), \
	CMD(BT_HCI_CMD_LE_REQ_PEER_SCA, cmd_req_peer_sca, NULL), \
	CMD(BT_HCI_CMD_LE_SETUP_ISO_PATH, cmd_setup_iso_path, NULL), \
	CMD(BT_HCI_CMD_LE_REMOVE_ISO_PATH, cmd_remove_iso_path, NULL), \
	CMD(BT_HCI_CMD_LE_ISO_TX_TEST, cmd_iso_tx_test, NULL), \
	CMD(BT_HCI_CMD_LE_ISO_RX_TEST, cmd_iso_rx_test, NULL), \
	CMD(BT_HCI_CMD_LE_ISO_READ_TEST_COUNTER, cmd_iso_read_test_counter, \
					NULL), \
	CMD(BT_HCI_CMD_LE_ISO_TEST_END, cmd_iso_test_end, NULL), \
	CMD(BT_HCI_CMD_LE_SET_HOST_FEATURE, cmd_set_host_feature, NULL)

static const struct btdev_cmd cmd_le_5_2[] = {
	CMD_COMMON_ALL,
	CMD_COMMON_BREDR_LE,
	CMD_LE,
	CMD_LE_50,
	CMD_LE_52,
	{}
};

static void set_le_52_commands(struct btdev *btdev)
{
	btdev->commands[41] |= 0x20;	/* LE Read Buffer Size v2 */
	btdev->commands[41] |= 0x40;	/* LE Read ISO TX Sync */
	btdev->commands[41] |= 0x80;	/* LE Set CIG Parameters */
	btdev->commands[42] |= 0x01;	/* LE Set CIG Parameters Test */
	btdev->commands[42] |= 0x02;	/* LE Create CIS */
	btdev->commands[42] |= 0x04;	/* LE Remove CIG */
	btdev->commands[42] |= 0x08;	/* LE Accept CIS */
	btdev->commands[42] |= 0x10;	/* LE Reject CIS */
	btdev->commands[42] |= 0x20;	/* LE Create BIG */
	btdev->commands[42] |= 0x40;	/* LE Create BIG Test */
	btdev->commands[42] |= 0x80;	/* LE Terminate BIG */
	btdev->commands[43] |= 0x01;	/* LE BIG Create Sync */
	btdev->commands[43] |= 0x02;	/* LE BIG Terminate Sync */
	btdev->commands[43] |= 0x04;	/* LE Request Peer SCA */
	btdev->commands[43] |= 0x08;	/* LE Setup ISO Path */
	btdev->commands[43] |= 0x10;	/* LE Remove ISO Path */
	btdev->commands[43] |= 0x20;	/* LE ISO TX Test */
	btdev->commands[43] |= 0x40;	/* LE ISO RX Test */
	btdev->commands[43] |= 0x80;	/* LE ISO Read Test Counter */
	btdev->commands[44] |= 0x01;	/* LE ISO Test End */
	btdev->commands[44] |= 0x02;	/* LE ISO Set Host Feature */
	btdev->cmds = cmd_le_5_2;
}

static const struct btdev_cmd cmd_le[] = {
	CMD_COMMON_ALL,
	CMD_COMMON_BREDR_LE,
	CMD_LE,
	{}
};

static void set_le_commands(struct btdev *btdev)
{
	set_common_commands_all(btdev);
	set_common_commands_bredrle(btdev);

	btdev->commands[24] |= 0x20;	/* Read LE Host Supported */
	btdev->commands[24] |= 0x20;	/* Write LE Host Supported */
	btdev->commands[25] |= 0x01;	/* LE Set Event Mask */
	btdev->commands[25] |= 0x02;	/* LE Read Buffer Size */
	btdev->commands[25] |= 0x04;	/* LE Read Local Features */
	btdev->commands[25] |= 0x10;	/* LE Set Random Address */
	btdev->commands[25] |= 0x20;	/* LE Set Adv Parameters */
	btdev->commands[25] |= 0x40;	/* LE Read Adv TX Power */
	btdev->commands[25] |= 0x80;	/* LE Set Adv Data */
	btdev->commands[26] |= 0x01;	/* LE Set Scan Response Data */
	btdev->commands[26] |= 0x02;	/* LE Set Adv Enable */
	btdev->commands[26] |= 0x04;	/* LE Set Scan Parameters */
	btdev->commands[26] |= 0x08;	/* LE Set Scan Enable */
	btdev->commands[26] |= 0x10;	/* LE Create Connection */
	btdev->commands[26] |= 0x40;	/* LE Read White List Size */
	btdev->commands[26] |= 0x80;	/* LE Clear White List */
	btdev->commands[27] |= 0x04;	/* LE Connection Update */
	btdev->commands[27] |= 0x20;	/* LE Read Remote Used Features */
	btdev->commands[27] |= 0x40;	/* LE Encrypt */
	btdev->commands[27] |= 0x80;	/* LE Rand */
	btdev->commands[28] |= 0x01;	/* LE Start Encryption */
	btdev->commands[28] |= 0x02;	/* LE Long Term Key Request Reply */
	btdev->commands[28] |= 0x04;	/* LE Long Term Key Request Neg Reply */
	btdev->commands[28] |= 0x08;	/* LE Read Supported States */
	btdev->commands[28] |= 0x10;	/* LE Receiver Test */
	btdev->commands[28] |= 0x20;	/* LE Transmitter Test */
	btdev->commands[28] |= 0x40;	/* LE Test End */

	/* Extra LE commands for >= 4.1 adapters */
	btdev->commands[33] |= 0x10;	/* LE Remote Conn Param Req Reply */
	btdev->commands[33] |= 0x20;	/* LE Remote Conn Param Req Neg Reply */

	/* Extra LE commands for >= 4.2 adapters */
	btdev->commands[34] |= 0x02;	/* LE Read Local P-256 Public Key */
	btdev->commands[34] |= 0x04;	/* LE Generate DHKey */

	btdev->cmds = cmd_le;

	/* Extra LE commands for >= 5.0 adapters */
	if (btdev->type >= BTDEV_TYPE_BREDRLE50) {
		set_le_50_commands(btdev);
		btdev->cmds = cmd_le_5_0;
	}

	/* Extra LE commands for >= 5.2 adapters */
	if (btdev->type >= BTDEV_TYPE_BREDRLE52) {
		set_le_52_commands(btdev);
		btdev->cmds = cmd_le_5_2;
	}
}

static int cmd_set_event_mask_2(struct btdev *dev, const void *data,
							uint8_t len)
{
	const struct bt_hci_cmd_set_event_mask_page2 *cmd = data;
	uint8_t status = BT_HCI_ERR_SUCCESS;

	memcpy(dev->event_mask_page2, cmd->mask, 8);
	cmd_complete(dev, BT_HCI_CMD_SET_EVENT_MASK_PAGE2, &status,
						sizeof(status));

	return 0;
}

static int cmd_read_sync_train_params(struct btdev *dev, const void *data,
							uint8_t len)
{
	struct bt_hci_rsp_read_sync_train_params rsp;

	memset(&rsp, 0, sizeof(rsp));

	rsp.status = BT_HCI_ERR_SUCCESS;
	rsp.interval = cpu_to_le16(dev->sync_train_interval);
	rsp.timeout = cpu_to_le32(dev->sync_train_timeout);
	rsp.service_data = dev->sync_train_service_data;
	cmd_complete(dev, BT_HCI_CMD_READ_SYNC_TRAIN_PARAMS, &rsp, sizeof(rsp));

	return 0;
}

static int cmd_read_sc_support(struct btdev *dev, const void *data, uint8_t len)
{
	struct bt_hci_rsp_read_secure_conn_support rsp;

	memset(&rsp, 0, sizeof(rsp));

	rsp.status = BT_HCI_ERR_SUCCESS;
	rsp.support = dev->secure_conn_support;
	cmd_complete(dev, BT_HCI_CMD_READ_SECURE_CONN_SUPPORT, &rsp,
							sizeof(rsp));

	return 0;
}

static int cmd_write_sc_support(struct btdev *dev, const void *data,
							uint8_t len)
{
	const struct bt_hci_cmd_write_secure_conn_support *cmd = data;
	uint8_t status = BT_HCI_ERR_SUCCESS;

	dev->secure_conn_support = cmd->support;
	cmd_complete(dev, BT_HCI_CMD_WRITE_SECURE_CONN_SUPPORT, &status,
							sizeof(status));

	return 0;
}

static int cmd_read_auth_payload_timeout(struct btdev *dev, const void *data,
							uint8_t len)
{
	/* TODO */
	return -ENOTSUP;
}

static int cmd_write_auth_payload_timeout(struct btdev *dev, const void *data,
							uint8_t len)
{
	/* TODO */
	return -ENOTSUP;
}

static int cmd_read_local_oob_ext_data(struct btdev *dev, const void *data,
							uint8_t len)
{
	struct bt_hci_rsp_read_local_oob_ext_data rsp;

	memset(&rsp, 0, sizeof(rsp));

	rsp.status = BT_HCI_ERR_SUCCESS;
	cmd_complete(dev, BT_HCI_CMD_READ_LOCAL_OOB_EXT_DATA, &rsp,
							sizeof(rsp));

	return 0;
}

#define BT_BREDR_LE \
	CMD(BT_HCI_CMD_SET_EVENT_MASK_PAGE2, cmd_set_event_mask_2, NULL), \
	CMD(BT_HCI_CMD_READ_SYNC_TRAIN_PARAMS, cmd_read_sync_train_params, \
					NULL), \
	CMD(BT_HCI_CMD_READ_SECURE_CONN_SUPPORT, cmd_read_sc_support, NULL), \
	CMD(BT_HCI_CMD_WRITE_SECURE_CONN_SUPPORT, cmd_write_sc_support, NULL), \
	CMD(BT_HCI_CMD_READ_AUTH_PAYLOAD_TIMEOUT, \
					cmd_read_auth_payload_timeout, NULL), \
	CMD(BT_HCI_CMD_WRITE_AUTH_PAYLOAD_TIMEOUT, \
					cmd_write_auth_payload_timeout, NULL), \
	CMD(BT_HCI_CMD_READ_LOCAL_OOB_EXT_DATA, \
					cmd_read_local_oob_ext_data, NULL)

static const struct btdev_cmd cmd_bredr_le[] = {
	CMD_COMMON_ALL,
	CMD_COMMON_BREDR_LE,
	CMD_COMMON_BREDR_20,
	CMD_BREDR,
	CMD_LE,
	CMD_LE_50,
	CMD_LE_52,
	BT_BREDR_LE,
	{}
};

static void set_bredrle_commands(struct btdev *btdev)
{
	set_bredr_commands(btdev);
	set_le_commands(btdev);

	/* Extra BR/EDR commands we want to only support for >= 4.0
	 * adapters.
	 */
	btdev->commands[22] |= 0x04;	/* Set Event Mask Page 2 */
	btdev->commands[31] |= 0x80;	/* Read Sync Train Parameters */
	btdev->commands[32] |= 0x04;	/* Read Secure Connections Support */
	btdev->commands[32] |= 0x08;	/* Write Secure Connections Support */
	btdev->commands[32] |= 0x10;	/* Read Auth Payload Timeout */
	btdev->commands[32] |= 0x20;	/* Write Auth Payload Timeout */
	btdev->commands[32] |= 0x40;	/* Read Local OOB Extended Data */
	btdev->cmds = cmd_bredr_le;
}

static void set_amp_commands(struct btdev *btdev)
{
	set_common_commands_all(btdev);

	btdev->commands[22] |= 0x20;	/* Read Local AMP Info */
}

static void set_bredrle_features(struct btdev *btdev)
{
	btdev->features[0] |= 0x04;	/* Encryption */
	btdev->features[0] |= 0x20;	/* Role switch */
	btdev->features[0] |= 0x80;	/* Sniff mode */
	btdev->features[1] |= 0x08;	/* SCO link */
	btdev->features[2] |= 0x08;	/* Transparent SCO */
	btdev->features[3] |= 0x40;	/* RSSI with inquiry results */
	btdev->features[3] |= 0x80;	/* Extended SCO link */
	btdev->features[4] |= 0x08;	/* AFH capable slave */
	btdev->features[4] |= 0x10;	/* AFH classification slave */
	btdev->features[4] |= 0x40;	/* LE Supported */
	btdev->features[5] |= 0x02;	/* Sniff subrating */
	btdev->features[5] |= 0x04;	/* Pause encryption */
	btdev->features[5] |= 0x08;	/* AFH capable master */
	btdev->features[5] |= 0x10;	/* AFH classification master */
	btdev->features[6] |= 0x01;	/* Extended Inquiry Response */
	btdev->features[6] |= 0x02;	/* Simultaneous LE and BR/EDR */
	btdev->features[6] |= 0x08;	/* Secure Simple Pairing */
	btdev->features[6] |= 0x10;	/* Encapsulated PDU */
	btdev->features[6] |= 0x20;	/* Erroneous Data Reporting */
	btdev->features[6] |= 0x40;	/* Non-flushable Packet Boundary Flag */
	btdev->features[7] |= 0x01;	/* Link Supervision Timeout Event */
	btdev->features[7] |= 0x02;	/* Inquiry TX Power Level */
	btdev->features[7] |= 0x80;	/* Extended features */

	if (btdev->type >= BTDEV_TYPE_BREDRLE50) {
		/* These BREDR features are added to test new configuration
		 * command. If this is added above it will break existing tests
		 */
		btdev->features[0] |= 0x01;	/* 3 slot Packets */
		btdev->features[0] |= 0x02;	/* 5 slot Packets */
		btdev->features[3] |= 0x02;	/* EDR ACL 2M mode */
		btdev->features[3] |= 0x04;	/* EDR ACL 3M mode */
		btdev->features[4] |= 0x80;	/* 3 slot EDR ACL packets */
		btdev->features[5] |= 0x01;	/* 5 slot EDR ACL packets */

		btdev->le_features[1] |= 0x01;	/* LE 2M PHY */
		btdev->le_features[1] |= 0x08;	/* LE Coded PHY */
		btdev->le_features[1] |= 0x10;  /* LE EXT ADV */
	}

	if (btdev->type >= BTDEV_TYPE_BREDRLE52) {
		btdev->le_features[3] |= 0x10;  /* LE CIS Master */
		btdev->le_features[3] |= 0x20;  /* LE CIS Slave */
		btdev->le_features[3] |= 0x40;  /* LE ISO Broadcaster */
		btdev->le_features[3] |= 0x80;  /* LE Synchronized Receiver */
		btdev->le_features[4] |= 0x01;  /* LE ISO channels */
	}

	btdev->feat_page_2[0] |= 0x01;	/* CSB - Master Operation */
	btdev->feat_page_2[0] |= 0x02;	/* CSB - Slave Operation */
	btdev->feat_page_2[0] |= 0x04;	/* Synchronization Train */
	btdev->feat_page_2[0] |= 0x08;	/* Synchronization Scan */
	btdev->feat_page_2[0] |= 0x10;	/* Inquiry Response Notification */
	btdev->feat_page_2[1] |= 0x01;	/* Secure Connections */
	btdev->feat_page_2[1] |= 0x02;	/* Ping */

	btdev->max_page = 2;
}

static void set_bredr_features(struct btdev *btdev)
{
	btdev->features[0] |= 0x04;	/* Encryption */
	btdev->features[0] |= 0x20;	/* Role switch */
	btdev->features[0] |= 0x80;	/* Sniff mode */
	btdev->features[1] |= 0x08;	/* SCO link */
	btdev->features[3] |= 0x40;	/* RSSI with inquiry results */
	btdev->features[3] |= 0x80;	/* Extended SCO link */
	btdev->features[4] |= 0x08;	/* AFH capable slave */
	btdev->features[4] |= 0x10;	/* AFH classification slave */
	btdev->features[5] |= 0x02;	/* Sniff subrating */
	btdev->features[5] |= 0x04;	/* Pause encryption */
	btdev->features[5] |= 0x08;	/* AFH capable master */
	btdev->features[5] |= 0x10;	/* AFH classification master */
	btdev->features[6] |= 0x01;	/* Extended Inquiry Response */
	btdev->features[6] |= 0x08;	/* Secure Simple Pairing */
	btdev->features[6] |= 0x10;	/* Encapsulated PDU */
	btdev->features[6] |= 0x20;	/* Erroneous Data Reporting */
	btdev->features[6] |= 0x40;	/* Non-flushable Packet Boundary Flag */
	btdev->features[7] |= 0x01;	/* Link Supervision Timeout Event */
	btdev->features[7] |= 0x02;	/* Inquiry TX Power Level */
	btdev->features[7] |= 0x80;	/* Extended features */

	btdev->max_page = 1;
}

static void set_bredr20_features(struct btdev *btdev)
{
	btdev->features[0] |= 0x04;	/* Encryption */
	btdev->features[0] |= 0x20;	/* Role switch */
	btdev->features[0] |= 0x80;	/* Sniff mode */
	btdev->features[1] |= 0x08;	/* SCO link */
	btdev->features[3] |= 0x40;	/* RSSI with inquiry results */
	btdev->features[3] |= 0x80;	/* Extended SCO link */
	btdev->features[4] |= 0x08;	/* AFH capable slave */
	btdev->features[4] |= 0x10;	/* AFH classification slave */
	btdev->features[5] |= 0x02;	/* Sniff subrating */
	btdev->features[5] |= 0x04;	/* Pause encryption */
	btdev->features[5] |= 0x08;	/* AFH capable master */
	btdev->features[5] |= 0x10;	/* AFH classification master */
	btdev->features[7] |= 0x80;	/* Extended features */

	btdev->max_page = 1;
}

static void set_le_features(struct btdev *btdev)
{
	btdev->features[4] |= 0x20;	/* BR/EDR Not Supported */
	btdev->features[4] |= 0x40;	/* LE Supported */

	btdev->max_page = 1;

	btdev->le_features[0] |= 0x01;	/* LE Encryption */
	btdev->le_features[0] |= 0x02;	/* Connection Parameters Request */
	btdev->le_features[0] |= 0x08;	/* Slave-initiated Features Exchange */
}

static void set_le_states(struct btdev *btdev)
{
	/* Set all 41 bits as per Bluetooth 5.0 specification */
	btdev->le_states[0] = 0xff;
	btdev->le_states[1] = 0xff;
	btdev->le_states[2] = 0xff;
	btdev->le_states[3] = 0xff;
	btdev->le_states[4] = 0xff;
	btdev->le_states[5] = 0x03;
}

static void set_amp_features(struct btdev *btdev)
{
}

struct btdev *btdev_create(enum btdev_type type, uint16_t id)
{
	struct btdev *btdev;
	int index;

	btdev = malloc(sizeof(*btdev));
	if (!btdev)
		return NULL;

	memset(btdev, 0, sizeof(*btdev));

	if (type == BTDEV_TYPE_BREDRLE || type == BTDEV_TYPE_LE ||
			type == BTDEV_TYPE_BREDRLE50 ||
			type == BTDEV_TYPE_BREDRLE52) {
		btdev->crypto = bt_crypto_new();
		if (!btdev->crypto) {
			free(btdev);
			return NULL;
		}
	}

	btdev->type = type;

	btdev->manufacturer = 63;
	btdev->revision = 0x0000;

	switch (btdev->type) {
	case BTDEV_TYPE_BREDRLE:
	case BTDEV_TYPE_BREDRLE50:
	case BTDEV_TYPE_BREDRLE52:
		btdev->version = 0x09;
		set_bredrle_features(btdev);
		set_bredrle_commands(btdev);
		set_le_states(btdev);
		break;
	case BTDEV_TYPE_BREDR:
		btdev->version = 0x05;
		set_bredr_features(btdev);
		set_bredr_commands(btdev);
		break;
	case BTDEV_TYPE_LE:
		btdev->version = 0x09;
		set_le_features(btdev);
		set_le_commands(btdev);
		set_le_states(btdev);
		break;
	case BTDEV_TYPE_AMP:
		btdev->version = 0x01;
		set_amp_features(btdev);
		set_amp_commands(btdev);
		break;
	case BTDEV_TYPE_BREDR20:
		btdev->version = 0x03;
		set_bredr20_features(btdev);
		set_bredr20_commands(btdev);
		break;
	}

	btdev->page_scan_interval = 0x0800;
	btdev->page_scan_window = 0x0012;
	btdev->page_scan_type = 0x00;

	btdev->sync_train_interval = 0x0080;
	btdev->sync_train_timeout = 0x0002ee00;
	btdev->sync_train_service_data = 0x00;

	btdev->acl_mtu = 192;
	btdev->acl_max_pkt = 1;

	btdev->iso_mtu = 251;
	btdev->iso_max_pkt = 1;

	btdev->country_code = 0x00;

	index = add_btdev(btdev);
	if (index < 0) {
		bt_crypto_unref(btdev->crypto);
		free(btdev);
		return NULL;
	}

	get_bdaddr(id, index, btdev->bdaddr);

	btdev->conns = queue_new();

	return btdev;
}

void btdev_destroy(struct btdev *btdev)
{
	if (!btdev)
		return;

	if (btdev->inquiry_id > 0)
		timeout_remove(btdev->inquiry_id);

	bt_crypto_unref(btdev->crypto);
	del_btdev(btdev);

	free(btdev);
}

bool btdev_set_debug(struct btdev *btdev, btdev_debug_func_t callback,
			void *user_data, btdev_destroy_func_t destroy)
{
	if (!btdev)
		return false;

	if (btdev->debug_destroy)
		btdev->debug_destroy(btdev->debug_data);

	btdev->debug_callback = callback;
	btdev->debug_destroy = destroy;
	btdev->debug_data = user_data;

	return true;
}

const uint8_t *btdev_get_bdaddr(struct btdev *btdev)
{
	return btdev->bdaddr;
}

uint8_t *btdev_get_features(struct btdev *btdev)
{
	return btdev->features;
}

uint8_t btdev_get_scan_enable(struct btdev *btdev)
{
	return btdev->scan_enable;
}

uint8_t btdev_get_le_scan_enable(struct btdev *btdev)
{
	return btdev->le_scan_enable;
}

void btdev_set_le_states(struct btdev *btdev, const uint8_t *le_states)
{
	memcpy(btdev->le_states, le_states, sizeof(btdev->le_states));
}


void btdev_set_command_handler(struct btdev *btdev, btdev_command_func handler,
							void *user_data)
{
	if (!btdev)
		return;

	btdev->command_handler = handler;
	btdev->command_data = user_data;
}

void btdev_set_send_handler(struct btdev *btdev, btdev_send_func handler,
							void *user_data)
{
	if (!btdev)
		return;

	btdev->send_handler = handler;
	btdev->send_data = user_data;
}

static void num_completed_packets(struct btdev *btdev, uint16_t handle)
{
	struct btdev_conn *conn;

	conn = queue_find(btdev->conns, match_handle, UINT_TO_PTR(handle));
	if (conn) {
		struct bt_hci_evt_num_completed_packets ncp;

		ncp.num_handles = 1;
		ncp.handle = cpu_to_le16(handle);
		ncp.count = cpu_to_le16(1);

		send_event(btdev, BT_HCI_EVT_NUM_COMPLETED_PACKETS,
							&ncp, sizeof(ncp));
	}
}

static const struct btdev_cmd *default_cmd(struct btdev *btdev, uint16_t opcode,
						const void *data, uint8_t len)
{
	const struct btdev_cmd *cmd;
	uint8_t status = BT_HCI_ERR_UNKNOWN_COMMAND;
	int err;

	for (cmd = btdev->cmds; cmd->func; cmd++) {
		if (cmd->opcode != opcode)
			continue;

		err = cmd->func(btdev, data, len);
		switch (err) {
		case 0:
			return cmd;
		case -ENOTSUP:
			status = BT_HCI_ERR_UNKNOWN_COMMAND;
			goto failed;
		case -EINVAL:
			status = BT_HCI_ERR_INVALID_PARAMETERS;
			goto failed;
		default:
			status = BT_HCI_ERR_UNSPECIFIED_ERROR;
			goto failed;
		}
	}

failed:
	util_debug(btdev->debug_callback, btdev->debug_data,
			"Unsupported command 0x%4.4x\n", opcode);
	cmd_status(btdev, status, opcode);

	return NULL;
}

struct btdev_callback {
	void (*function)(btdev_callback callback, uint8_t response,
				uint8_t status, const void *data, uint8_t len);
	void *user_data;
	uint16_t opcode;
	const void *data;
	uint8_t len;
};

void btdev_command_response(btdev_callback callback, uint8_t response,
                                uint8_t status, const void *data, uint8_t len)
{
	callback->function(callback, response, status, data, len);
}

static void handler_callback(btdev_callback callback, uint8_t response,
				uint8_t status, const void *data, uint8_t len)
{
	struct btdev *btdev = callback->user_data;
	const struct btdev_cmd *cmd;

	switch (response) {
	case BTDEV_RESPONSE_DEFAULT:
		if (!run_hooks(btdev, BTDEV_HOOK_PRE_CMD, callback->opcode,
						callback->data, callback->len))
			return;

		cmd = default_cmd(btdev, callback->opcode,
					callback->data, callback->len);
		if (!cmd)
			return;

		if (!run_hooks(btdev, BTDEV_HOOK_PRE_EVT, callback->opcode,
						callback->data, callback->len))
			return;

		if (cmd->complete)
			cmd->complete(btdev, callback->data, callback->len);

		break;
	case BTDEV_RESPONSE_COMMAND_STATUS:
		cmd_status(btdev, status, callback->opcode);
		break;
	case BTDEV_RESPONSE_COMMAND_COMPLETE:
		cmd_complete(btdev, callback->opcode, data, len);
		break;
	default:
		cmd_status(btdev, BT_HCI_ERR_UNKNOWN_COMMAND,
						callback->opcode);
		break;
	}
}

static void process_cmd(struct btdev *btdev, const void *data, uint16_t len)
{
	struct btdev_callback callback;
	const struct bt_hci_cmd_hdr *hdr = data;
	const struct btdev_cmd *cmd;

	if (len < sizeof(*hdr))
		return;

	callback.function = handler_callback;
	callback.user_data = btdev;
	callback.opcode = le16_to_cpu(hdr->opcode);
	callback.data = data + sizeof(*hdr);
	callback.len = hdr->plen;

	util_debug(btdev->debug_callback, btdev->debug_data,
				"command 0x%04x", callback.opcode);

	if (btdev->command_handler)
		btdev->command_handler(callback.opcode,
					callback.data, callback.len,
					&callback, btdev->command_data);
	else {
		if (!run_hooks(btdev, BTDEV_HOOK_PRE_CMD, callback.opcode,
						callback.data, callback.len))
			return;

		cmd = default_cmd(btdev, callback.opcode,
					callback.data, callback.len);
		if (!cmd)
			return;

		if (!run_hooks(btdev, BTDEV_HOOK_PRE_EVT, callback.opcode,
						callback.data, callback.len))
			return;

		if (cmd->complete)
			cmd->complete(btdev, callback.data, callback.len);
	}
}

static void send_acl(struct btdev *dev, const void *data, uint16_t len)
{
	struct bt_hci_acl_hdr hdr;
	struct iovec iov[3];
	struct btdev_conn *conn;
	uint8_t pkt_type = BT_H4_ACL_PKT;

	/* Packet type */
	iov[0].iov_base = &pkt_type;
	iov[0].iov_len = sizeof(pkt_type);

	memcpy(&hdr, data, sizeof(hdr));

	conn = queue_find(dev->conns, match_handle,
					UINT_TO_PTR(acl_handle(hdr.handle)));
	if (!conn)
		return;

	num_completed_packets(dev, conn->handle);

	/* ACL_START_NO_FLUSH is only allowed from host to controller.
	 * From controller to host this should be converted to ACL_START.
	 */
	if (acl_flags(hdr.handle) == ACL_START_NO_FLUSH)
		hdr.handle = acl_handle_pack(conn->handle, ACL_START);

	iov[1].iov_base = &hdr;
	iov[1].iov_len = sizeof(hdr);

	iov[2].iov_base = (void *) (data + sizeof(hdr));
	iov[2].iov_len = len - sizeof(hdr);

	send_packet(conn->link->dev, iov, 3);
}

static void send_iso(struct btdev *dev, const void *data, uint16_t len)
{
	struct bt_hci_acl_hdr *hdr;
	struct iovec iov[2];
	struct btdev_conn *conn;
	uint8_t pkt_type = BT_H4_ISO_PKT;

	/* Packet type */
	iov[0].iov_base = &pkt_type;
	iov[0].iov_len = sizeof(pkt_type);

	iov[1].iov_base = hdr = (void *) (data);
	iov[1].iov_len = len;

	conn = queue_find(dev->conns, match_handle,
					UINT_TO_PTR(acl_handle(hdr->handle)));
	if (!conn)
		return;

	num_completed_packets(dev, conn->handle);

	send_packet(conn->link->dev, iov, 2);
}

void btdev_receive_h4(struct btdev *btdev, const void *data, uint16_t len)
{
	uint8_t pkt_type;

	if (!btdev)
		return;

	if (len < 1)
		return;

	util_hexdump('>', data, len, btdev->debug_callback,
					btdev->debug_data);

	pkt_type = ((const uint8_t *) data)[0];

	switch (pkt_type) {
	case BT_H4_CMD_PKT:
		process_cmd(btdev, data + 1, len - 1);
		break;
	case BT_H4_ACL_PKT:
		send_acl(btdev, data + 1, len - 1);
		break;
	case BT_H4_ISO_PKT:
		send_iso(btdev, data + 1, len - 1);
		break;
	default:
		util_debug(btdev->debug_callback, btdev->debug_data,
				"Unsupported packet 0x%2.2x\n", pkt_type);
		break;
	}
}

int btdev_add_hook(struct btdev *btdev, enum btdev_hook_type type,
				uint16_t opcode, btdev_hook_func handler,
				void *user_data)
{
	int i;

	if (!btdev)
		return -1;

	if (get_hook_index(btdev, type, opcode) > 0)
		return -1;

	for (i = 0; i < MAX_HOOK_ENTRIES; i++) {
		if (btdev->hook_list[i] == NULL) {
			btdev->hook_list[i] = malloc(sizeof(struct hook));
			if (btdev->hook_list[i] == NULL)
				return -1;

			btdev->hook_list[i]->handler = handler;
			btdev->hook_list[i]->user_data = user_data;
			btdev->hook_list[i]->opcode = opcode;
			btdev->hook_list[i]->type = type;
			return i;
		}
	}

	return -1;
}

bool btdev_del_hook(struct btdev *btdev, enum btdev_hook_type type,
								uint16_t opcode)
{
	int i;

	if (!btdev)
		return false;

	for (i = 0; i < MAX_HOOK_ENTRIES; i++) {
		if (btdev->hook_list[i] == NULL)
			continue;

		if (btdev->hook_list[i]->type != type ||
					btdev->hook_list[i]->opcode != opcode)
			continue;

		free(btdev->hook_list[i]);
		btdev->hook_list[i] = NULL;

		return true;
	}

	return false;
}

/*
 *
 *  BlueZ - Bluetooth protocol stack for Linux
 *
 *  Copyright (C) 2011-2012  Intel Corporation
 *  Copyright (C) 2004-2010  Marcel Holtmann <marcel@holtmann.org>
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

#include <fcntl.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <sys/uio.h>
#include <time.h>

#include "lib/bluetooth.h"
#include "lib/hci.h"

#include "src/shared/util.h"
#include "src/shared/crypto.h"
#include "src/shared/ecc.h"
#include "src/shared/mainloop.h"
#include "monitor/bt.h"

#include "phy.h"
#include "le.h"

#define WHITE_LIST_SIZE		16
#define RESOLV_LIST_SIZE	16
#define SCAN_CACHE_SIZE		64

#define DEFAULT_TX_LEN		0x001b
#define DEFAULT_TX_TIME		0x0148
#define MAX_TX_LEN		0x00fb
#define MAX_TX_TIME		0x0848
#define MAX_RX_LEN		0x00fb
#define MAX_RX_TIME		0x0848

struct bt_peer {
	uint8_t  addr_type;
	uint8_t  addr[6];
};

struct bt_le {
	volatile int ref_count;
	int vhci_fd;
	struct bt_phy *phy;
	struct bt_crypto *crypto;
	int adv_timeout_id;
	int scan_timeout_id;
	bool scan_window_active;
	uint8_t scan_chan_idx;

	uint8_t  event_mask[16];
	uint16_t manufacturer;
	uint8_t  commands[64];
	uint8_t  features[8];
	uint8_t  bdaddr[6];

	uint8_t  le_event_mask[8];
	uint16_t le_mtu;
	uint8_t  le_max_pkt;
	uint8_t  le_features[8];
	uint8_t  le_random_addr[6];
	uint16_t le_adv_min_interval;
	uint16_t le_adv_max_interval;
	uint8_t  le_adv_type;
	uint8_t  le_adv_own_addr_type;
	uint8_t  le_adv_direct_addr_type;
	uint8_t  le_adv_direct_addr[6];
	uint8_t  le_adv_channel_map;
	uint8_t  le_adv_filter_policy;
	int8_t   le_adv_tx_power;
	uint8_t  le_adv_data_len;
	uint8_t  le_adv_data[31];
	uint8_t  le_scan_rsp_data_len;
	uint8_t  le_scan_rsp_data[31];
	uint8_t  le_adv_enable;
	uint8_t  le_scan_type;
	uint16_t le_scan_interval;
	uint16_t le_scan_window;
	uint8_t  le_scan_own_addr_type;
	uint8_t  le_scan_filter_policy;
	uint8_t  le_scan_enable;
	uint8_t  le_scan_filter_dup;

	uint8_t  le_conn_peer_addr_type;
	uint8_t  le_conn_peer_addr[6];
	uint8_t  le_conn_own_addr_type;
	uint8_t  le_conn_enable;

	uint8_t  le_white_list_size;
	uint8_t  le_white_list[WHITE_LIST_SIZE][7];
	uint8_t  le_states[8];

	uint16_t le_default_tx_len;
	uint16_t le_default_tx_time;
	uint8_t  le_local_sk256[32];
	uint8_t  le_resolv_list[RESOLV_LIST_SIZE][39];
	uint8_t  le_resolv_list_size;
	uint8_t  le_resolv_enable;
	uint16_t le_resolv_timeout;

	struct bt_peer scan_cache[SCAN_CACHE_SIZE];
	uint8_t scan_cache_count;
};

static bool is_in_white_list(struct bt_le *hci, uint8_t addr_type,
							const uint8_t addr[6])
{
	int i;

	for (i = 0; i < hci->le_white_list_size; i++) {
		if (hci->le_white_list[i][0] == addr_type &&
				!memcmp(&hci->le_white_list[i][1], addr, 6))
			return true;
	}

	return false;
}

static void clear_white_list(struct bt_le *hci)
{
	int i;

	for (i = 0; i < hci->le_white_list_size; i++) {
		hci->le_white_list[i][0] = 0xff;
		memset(&hci->le_white_list[i][1], 0, 6);
	}
}

static void resolve_peer_addr(struct bt_le *hci, uint8_t peer_addr_type,
					const uint8_t peer_addr[6],
					uint8_t *addr_type, uint8_t addr[6])
{
	int i;

	if (!hci->le_resolv_enable)
		goto done;

	if (peer_addr_type != 0x01)
		goto done;

	if ((peer_addr[5] & 0xc0) != 0x40)
		goto done;

	for (i = 0; i < hci->le_resolv_list_size; i++) {
		uint8_t local_hash[3];

		if (hci->le_resolv_list[i][0] == 0xff)
			continue;

		bt_crypto_ah(hci->crypto, &hci->le_resolv_list[i][7],
						peer_addr + 3, local_hash);

		if (!memcmp(peer_addr, local_hash, 3)) {
			switch (hci->le_resolv_list[i][0]) {
			case 0x00:
				*addr_type = 0x02;
				break;
			case 0x01:
				*addr_type = 0x03;
				break;
			default:
				continue;
			}
			memcpy(addr, &hci->le_resolv_list[i][1], 6);
			return;
		}
	}

done:
	*addr_type = peer_addr_type;
	memcpy(addr, peer_addr, 6);
}

static void clear_resolv_list(struct bt_le *hci)
{
	int i;

	for (i = 0; i < hci->le_resolv_list_size; i++) {
		hci->le_resolv_list[i][0] = 0xff;
		memset(&hci->le_resolv_list[i][1], 0, 38);
	}
}

static void reset_defaults(struct bt_le *hci)
{
	memset(hci->event_mask, 0, sizeof(hci->event_mask));
	hci->event_mask[0] |= 0x10;	/* Disconnection Complete */
	hci->event_mask[0] |= 0x80;	/* Encryption Change */
	hci->event_mask[1] |= 0x08;	/* Read Remote Version Information Complete */
	hci->event_mask[1] |= 0x20;	/* Command Complete */
	hci->event_mask[1] |= 0x40;	/* Command Status */
	hci->event_mask[1] |= 0x80;	/* Hardware Error */
	hci->event_mask[2] |= 0x04;	/* Number of Completed Packets */
	hci->event_mask[3] |= 0x02;	/* Data Buffer Overflow */
	hci->event_mask[5] |= 0x80;	/* Encryption Key Refresh Complete */
	//hci->event_mask[7] |= 0x20;	/* LE Meta Event */

	hci->manufacturer = 0x003f;	/* Bluetooth SIG (63) */

	memset(hci->commands, 0, sizeof(hci->commands));
	hci->commands[0]  |= 0x20;	/* Disconnect */
	//hci->commands[2]  |= 0x80;	/* Read Remote Version Information */
	hci->commands[5]  |= 0x40;	/* Set Event Mask */
	hci->commands[5]  |= 0x80;	/* Reset */
	//hci->commands[10] |= 0x04;	/* Read Transmit Power Level */
	hci->commands[14] |= 0x08;	/* Read Local Version Information */
	hci->commands[14] |= 0x10;	/* Read Local Supported Commands */
	hci->commands[14] |= 0x20;	/* Read Local Supported Features */
	hci->commands[14] |= 0x80;	/* Read Buffer Size */
	hci->commands[15] |= 0x02;	/* Read BD ADDR */
	//hci->commands[15] |= 0x20;	/* Read RSSI */
	hci->commands[22] |= 0x04;	/* Set Event Mask Page 2 */
	hci->commands[25] |= 0x01;	/* LE Set Event Mask */
	hci->commands[25] |= 0x02;	/* LE Read Buffer Size */
	hci->commands[25] |= 0x04;	/* LE Read Local Supported Features */
	hci->commands[25] |= 0x10;	/* LE Set Random Address */
	hci->commands[25] |= 0x20;	/* LE Set Advertising Parameters */
	hci->commands[25] |= 0x40;	/* LE Read Advertising Channel TX Power */
	hci->commands[25] |= 0x80;	/* LE Set Advertising Data */
	hci->commands[26] |= 0x01;	/* LE Set Scan Response Data */
	hci->commands[26] |= 0x02;	/* LE Set Advertise Enable */
	hci->commands[26] |= 0x04;	/* LE Set Scan Parameters */
	hci->commands[26] |= 0x08;	/* LE Set Scan Enable */
	hci->commands[26] |= 0x10;	/* LE Create Connection */
	hci->commands[26] |= 0x20;	/* LE Create Connection Cancel */
	hci->commands[26] |= 0x40;	/* LE Read White List Size */
	hci->commands[26] |= 0x80;	/* LE Clear White List */
	hci->commands[27] |= 0x01;	/* LE Add Device To White List */
	hci->commands[27] |= 0x02;	/* LE Remove Device From White List */
	//hci->commands[27] |= 0x04;	/* LE Connection Update */
	//hci->commands[27] |= 0x08;	/* LE Set Host Channel Classification */
	//hci->commands[27] |= 0x10;	/* LE Read Channel Map */
	//hci->commands[27] |= 0x20;	/* LE Read Remote Used Features */
	hci->commands[27] |= 0x40;	/* LE Encrypt */
	hci->commands[27] |= 0x80;	/* LE Rand */
	//hci->commands[28] |= 0x01;	/* LE Start Encryption */
	//hci->commands[28] |= 0x02;	/* LE Long Term Key Request Reply */
	//hci->commands[28] |= 0x04;	/* LE Long Term Key Request Negative Reply */
	hci->commands[28] |= 0x08;	/* LE Read Supported States */
	//hci->commands[28] |= 0x10;	/* LE Receiver Test */
	//hci->commands[28] |= 0x20;	/* LE Transmitter Test */
	//hci->commands[28] |= 0x40;	/* LE Test End */
	//hci->commands[33] |= 0x10;	/* LE Remote Connection Parameter Request Reply */
	//hci->commands[33] |= 0x20;	/* LE Remote Connection Parameter Request Negative Reply */
	hci->commands[33] |= 0x40;	/* LE Set Data Length */
	hci->commands[33] |= 0x80;	/* LE Read Suggested Default Data Length */
	hci->commands[34] |= 0x01;	/* LE Write Suggested Default Data Length */
	hci->commands[34] |= 0x02;	/* LE Read Local P-256 Public Key */
	hci->commands[34] |= 0x04;	/* LE Generate DHKey */
	hci->commands[34] |= 0x08;	/* LE Add Device To Resolving List */
	hci->commands[34] |= 0x10;	/* LE Remove Device From Resolving List */
	hci->commands[34] |= 0x20;	/* LE Clear Resolving List */
	hci->commands[34] |= 0x40;	/* LE Read Resolving List Size */
	hci->commands[34] |= 0x80;	/* LE Read Peer Resolvable Address */
	hci->commands[35] |= 0x01;	/* LE Read Local Resolvable Address */
	hci->commands[35] |= 0x02;	/* LE Set Address Resolution Enable */
	hci->commands[35] |= 0x04;	/* LE Set Resolvable Private Address Timeout */
	hci->commands[35] |= 0x08;	/* LE Read Maximum Data Length */

	memset(hci->features, 0, sizeof(hci->features));
	hci->features[4] |= 0x20;	/* BR/EDR Not Supported */
	hci->features[4] |= 0x40;	/* LE Supported */

	memset(hci->bdaddr, 0, sizeof(hci->bdaddr));

	memset(hci->le_event_mask, 0, sizeof(hci->le_event_mask));
	hci->le_event_mask[0] |= 0x01;	/* LE Connection Complete */
	hci->le_event_mask[0] |= 0x02;	/* LE Advertising Report */
	hci->le_event_mask[0] |= 0x04;	/* LE Connection Update Complete */
	hci->le_event_mask[0] |= 0x08;	/* LE Read Remote Used Features Complete */
	hci->le_event_mask[0] |= 0x10;	/* LE Long Term Key Request */
	//hci->le_event_mask[0] |= 0x20;	/* LE Remote Connection Parameter Request */
	//hci->le_event_mask[0] |= 0x40;	/* LE Data Length Change */
	//hci->le_event_mask[0] |= 0x80;	/* LE Read Local P-256 Public Key Complete */
	//hci->le_event_mask[1] |= 0x01;	/* LE Generate DHKey Complete */
	//hci->le_event_mask[1] |= 0x02;	/* LE Enhanced Connection Complete */
	//hci->le_event_mask[1] |= 0x04;	/* LE Direct Advertising Report */

	hci->le_mtu = 64;
	hci->le_max_pkt = 1;

	memset(hci->le_features, 0, sizeof(hci->le_features));
	hci->le_features[0] |= 0x01;	/* LE Encryption */
	//hci->le_features[0] |= 0x02;	/* Connection Parameter Request Procedure */
	//hci->le_features[0] |= 0x04;	/* Extended Reject Indication */
	//hci->le_features[0] |= 0x08;	/* Slave-initiated Features Exchange */
	hci->le_features[0] |= 0x10;	/* LE Ping */
	hci->le_features[0] |= 0x20;	/* LE Data Packet Length Extension */
	hci->le_features[0] |= 0x40;	/* LL Privacy */
	hci->le_features[0] |= 0x80;	/* Extended Scanner Filter Policies */

	memset(hci->le_random_addr, 0, sizeof(hci->le_random_addr));

	hci->le_adv_min_interval = 0x0800;
	hci->le_adv_max_interval = 0x0800;
	hci->le_adv_type = 0x00;
	hci->le_adv_own_addr_type = 0x00;
	hci->le_adv_direct_addr_type = 0x00;
	memset(hci->le_adv_direct_addr, 0, 6);
	hci->le_adv_channel_map = 0x07;
	hci->le_adv_filter_policy = 0x00;

	hci->le_adv_tx_power = 0;

	memset(hci->le_adv_data, 0, sizeof(hci->le_adv_data));
	hci->le_adv_data_len = 0;

	memset(hci->le_scan_rsp_data, 0, sizeof(hci->le_scan_rsp_data));
	hci->le_scan_rsp_data_len = 0;

	hci->le_adv_enable = 0x00;

	hci->le_scan_type = 0x00;		/* Passive Scanning */
	hci->le_scan_interval = 0x0010;		/* 10 ms */
	hci->le_scan_window = 0x0010;		/* 10 ms */
	hci->le_scan_own_addr_type = 0x00;	/* Public Device Address */
	hci->le_scan_filter_policy = 0x00;
	hci->le_scan_enable = 0x00;
	hci->le_scan_filter_dup = 0x00;

	hci->le_conn_enable = 0x00;

	hci->le_white_list_size = WHITE_LIST_SIZE;
	clear_white_list(hci);

	memset(hci->le_states, 0, sizeof(hci->le_states));
	hci->le_states[0] |= 0x01;	/* Non-connectable Advertising */
	hci->le_states[0] |= 0x02;	/* Scannable Advertising */
	hci->le_states[0] |= 0x04;	/* Connectable Advertising */
	hci->le_states[0] |= 0x08;	/* High Duty Cycle Directed Advertising */
	hci->le_states[0] |= 0x10;	/* Passive Scanning */
	hci->le_states[0] |= 0x20;	/* Active Scanning */
	hci->le_states[0] |= 0x40;	/* Initiating + Connection (Master Role) */
	hci->le_states[0] |= 0x80;	/* Connection (Slave Role) */
	hci->le_states[1] |= 0x01;	/* Passive Scanning +
					 * Non-connectable Advertising */

	hci->le_default_tx_len = DEFAULT_TX_LEN;
	hci->le_default_tx_time = DEFAULT_TX_TIME;

	memset(hci->le_local_sk256, 0, sizeof(hci->le_local_sk256));

	hci->le_resolv_list_size = RESOLV_LIST_SIZE;
	clear_resolv_list(hci);
	hci->le_resolv_enable = 0x00;
	hci->le_resolv_timeout = 0x0384;	/* 900 secs or 15 minutes */
}

static void clear_scan_cache(struct bt_le *hci)
{
	memset(hci->scan_cache, 0, sizeof(hci->scan_cache));
	hci->scan_cache_count = 0;
}

static bool add_to_scan_cache(struct bt_le *hci, uint8_t addr_type,
							const uint8_t addr[6])
{
	int i;

	for (i = 0; i < hci->scan_cache_count; i++) {
		if (hci->scan_cache[i].addr_type == addr_type &&
				!memcmp(hci->scan_cache[i].addr, addr, 6))
			return false;
	}

	if (hci->scan_cache_count >= SCAN_CACHE_SIZE)
		return true;

	hci->scan_cache[hci->scan_cache_count].addr_type = addr_type;
	memcpy(hci->scan_cache[hci->scan_cache_count].addr, addr, 6);
	hci->scan_cache_count++;

	return true;
}

static void send_event(struct bt_le *hci, uint8_t event,
						void *data, uint8_t size)
{
	uint8_t type = BT_H4_EVT_PKT;
	struct bt_hci_evt_hdr hdr;
	struct iovec iov[3];
	int iovcnt;

	hdr.evt  = event;
	hdr.plen = size;

	iov[0].iov_base = &type;
	iov[0].iov_len  = 1;
	iov[1].iov_base = &hdr;
	iov[1].iov_len  = sizeof(hdr);

	if (size > 0) {
		iov[2].iov_base = data;
		iov[2].iov_len  = size;
		iovcnt = 3;
	} else
		iovcnt = 2;

	if (writev(hci->vhci_fd, iov, iovcnt) < 0)
		fprintf(stderr, "Write to /dev/vhci failed (%m)\n");
}

static void send_adv_pkt(struct bt_le *hci, uint8_t channel)
{
	struct bt_phy_pkt_adv pkt;

	memset(&pkt, 0, sizeof(pkt));
	pkt.chan_idx = channel;
	pkt.pdu_type = hci->le_adv_type;
	pkt.tx_addr_type = hci->le_adv_own_addr_type;
	switch (hci->le_adv_own_addr_type) {
	case 0x00:
	case 0x02:
		memcpy(pkt.tx_addr, hci->bdaddr, 6);
		break;
	case 0x01:
	case 0x03:
		memcpy(pkt.tx_addr, hci->le_random_addr, 6);
		break;
	}
	pkt.rx_addr_type = hci->le_adv_direct_addr_type;
	memcpy(pkt.rx_addr, hci->le_adv_direct_addr, 6);
	pkt.adv_data_len = hci->le_adv_data_len;
	pkt.scan_rsp_len = hci->le_scan_rsp_data_len;

	bt_phy_send_vector(hci->phy, BT_PHY_PKT_ADV, &pkt, sizeof(pkt),
				hci->le_adv_data, pkt.adv_data_len,
				hci->le_scan_rsp_data, pkt.scan_rsp_len);
}

static unsigned int get_adv_delay(void)
{
	/* The advertising delay is a pseudo-random value with a range
	 * of 0 ms to 10 ms generated for each advertising event.
	 */
	srand(time(NULL));
	return (rand() % 11);
}

static void adv_timeout_callback(int id, void *user_data)
{
	struct bt_le *hci = user_data;
	unsigned int msec, min_msec, max_msec;

	if (hci->le_adv_channel_map & 0x01)
		send_adv_pkt(hci, 37);
	if (hci->le_adv_channel_map & 0x02)
		send_adv_pkt(hci, 38);
	if (hci->le_adv_channel_map & 0x04)
		send_adv_pkt(hci, 39);

	min_msec = (hci->le_adv_min_interval * 625) / 1000;
	max_msec = (hci->le_adv_max_interval * 625) / 1000;

	msec = ((min_msec + max_msec) / 2) + get_adv_delay();

	if (mainloop_modify_timeout(id, msec) < 0) {
		fprintf(stderr, "Setting advertising timeout failed\n");
		hci->le_adv_enable = 0x00;
	}
}

static bool start_adv(struct bt_le *hci)
{
	unsigned int msec;

	if (hci->adv_timeout_id >= 0)
		return false;

	msec = ((hci->le_adv_min_interval * 625) / 1000) + get_adv_delay();

	hci->adv_timeout_id = mainloop_add_timeout(msec, adv_timeout_callback,
								hci, NULL);
	if (hci->adv_timeout_id < 0)
		return false;

	return true;
}

static bool stop_adv(struct bt_le *hci)
{
	if (hci->adv_timeout_id < 0)
		return false;

	mainloop_remove_timeout(hci->adv_timeout_id);
	hci->adv_timeout_id = -1;

	return true;
}

static void scan_timeout_callback(int id, void *user_data)
{
	struct bt_le *hci = user_data;
	unsigned int msec;

	if (hci->le_scan_window == hci->le_scan_interval ||
						!hci->scan_window_active) {
		msec = (hci->le_scan_window * 625) / 1000;
		hci->scan_window_active = true;

		hci->scan_chan_idx++;
		if (hci->scan_chan_idx > 39)
			hci->scan_chan_idx = 37;
	} else {
		msec = ((hci->le_scan_interval -
					hci->le_scan_window) * 625) / 1000;
		hci->scan_window_active = false;
	}

	if (mainloop_modify_timeout(id, msec) < 0) {
		fprintf(stderr, "Setting scanning timeout failed\n");
		hci->le_scan_enable = 0x00;
		hci->scan_window_active = false;
	}
}

static bool start_scan(struct bt_le *hci)
{
	unsigned int msec;

	if (hci->scan_timeout_id >= 0)
		return false;

	msec = (hci->le_scan_window * 625) / 1000;

	hci->scan_timeout_id = mainloop_add_timeout(msec, scan_timeout_callback,
								hci, NULL);
	if (hci->scan_timeout_id < 0)
		return false;

	hci->scan_window_active = true;
	hci->scan_chan_idx = 37;

	return true;
}

static bool stop_scan(struct bt_le *hci)
{
	if (hci->scan_timeout_id < 0)
		return false;

	mainloop_remove_timeout(hci->scan_timeout_id);
	hci->scan_timeout_id = -1;

	hci->scan_window_active = false;

	return true;
}

static void cmd_complete(struct bt_le *hci, uint16_t opcode,
						const void *data, uint8_t len)
{
	struct bt_hci_evt_cmd_complete *cc;
	void *pkt_data;

	pkt_data = alloca(sizeof(*cc) + len);
	if (!pkt_data)
		return;

	cc = pkt_data;
	cc->ncmd = 0x01;
	cc->opcode = cpu_to_le16(opcode);

	if (len > 0)
		memcpy(pkt_data + sizeof(*cc), data, len);

	send_event(hci, BT_HCI_EVT_CMD_COMPLETE, pkt_data, sizeof(*cc) + len);
}

static void cmd_status(struct bt_le *hci, uint8_t status, uint16_t opcode)
{
	struct bt_hci_evt_cmd_status cs;

	cs.status = status;
	cs.ncmd = 0x01;
	cs.opcode = cpu_to_le16(opcode);

	send_event(hci, BT_HCI_EVT_CMD_STATUS, &cs, sizeof(cs));
}

static void le_meta_event(struct bt_le *hci, uint8_t event,
						void *data, uint8_t len)
{
	void *pkt_data;

	if (!(hci->event_mask[7] & 0x20))
		return;

	pkt_data = alloca(1 + len);
	if (!pkt_data)
		return;

	((uint8_t *) pkt_data)[0] = event;

	if (len > 0)
		memcpy(pkt_data + 1, data, len);

	send_event(hci, BT_HCI_EVT_LE_META_EVENT, pkt_data, 1 + len);
}

static void cmd_disconnect(struct bt_le *hci, const void *data, uint8_t size)
{
	cmd_status(hci, BT_HCI_ERR_UNKNOWN_CONN_ID, BT_HCI_CMD_DISCONNECT);
}

static void cmd_set_event_mask(struct bt_le *hci,
						const void *data, uint8_t size)
{
	const struct bt_hci_cmd_set_event_mask *cmd = data;
	uint8_t status;

	memcpy(hci->event_mask, cmd->mask, 8);

	status = BT_HCI_ERR_SUCCESS;
	cmd_complete(hci, BT_HCI_CMD_SET_EVENT_MASK, &status, sizeof(status));
}

static void cmd_reset(struct bt_le *hci, const void *data, uint8_t size)
{
	uint8_t status;

	stop_adv(hci);
	stop_scan(hci);
	reset_defaults(hci);

	status = BT_HCI_ERR_SUCCESS;
	cmd_complete(hci, BT_HCI_CMD_RESET, &status, sizeof(status));
}

static void cmd_set_event_mask_page2(struct bt_le *hci,
						const void *data, uint8_t size)
{
	const struct bt_hci_cmd_set_event_mask_page2 *cmd = data;
	uint8_t status;

	memcpy(hci->event_mask + 8, cmd->mask, 8);

	status = BT_HCI_ERR_SUCCESS;
	cmd_complete(hci, BT_HCI_CMD_SET_EVENT_MASK_PAGE2,
						&status, sizeof(status));
}

static void cmd_read_local_version(struct bt_le *hci,
						const void *data, uint8_t size)
{
	struct bt_hci_rsp_read_local_version rsp;

	rsp.status = BT_HCI_ERR_SUCCESS;
	rsp.hci_ver = 0x08;
	rsp.hci_rev = cpu_to_le16(0x0000);
	rsp.lmp_ver = 0x08;
	rsp.manufacturer = cpu_to_le16(hci->manufacturer);
	rsp.lmp_subver = cpu_to_le16(0x0000);

	cmd_complete(hci, BT_HCI_CMD_READ_LOCAL_VERSION, &rsp, sizeof(rsp));
}

static void cmd_read_local_commands(struct bt_le *hci,
						const void *data, uint8_t size)
{
	struct bt_hci_rsp_read_local_commands rsp;

	rsp.status = BT_HCI_ERR_SUCCESS;
	memcpy(rsp.commands, hci->commands, 64);

	cmd_complete(hci, BT_HCI_CMD_READ_LOCAL_COMMANDS, &rsp, sizeof(rsp));
}

static void cmd_read_local_features(struct bt_le *hci,
						const void *data, uint8_t size)
{
	struct bt_hci_rsp_read_local_features rsp;

	rsp.status = BT_HCI_ERR_SUCCESS;
	memcpy(rsp.features, hci->features, 8);

	cmd_complete(hci, BT_HCI_CMD_READ_LOCAL_FEATURES, &rsp, sizeof(rsp));
}

static void cmd_read_buffer_size(struct bt_le *hci,
						const void *data, uint8_t size)
{
	struct bt_hci_rsp_read_buffer_size rsp;

	rsp.status = BT_HCI_ERR_SUCCESS;
	rsp.acl_mtu = cpu_to_le16(0x0000);
	rsp.sco_mtu = 0x00;
	rsp.acl_max_pkt = cpu_to_le16(0x0000);
	rsp.sco_max_pkt = cpu_to_le16(0x0000);

	cmd_complete(hci, BT_HCI_CMD_READ_BUFFER_SIZE, &rsp, sizeof(rsp));
}

static void cmd_read_bd_addr(struct bt_le *hci, const void *data, uint8_t size)
{
	struct bt_hci_rsp_read_bd_addr rsp;

	rsp.status = BT_HCI_ERR_SUCCESS;
	memcpy(rsp.bdaddr, hci->bdaddr, 6);

	cmd_complete(hci, BT_HCI_CMD_READ_BD_ADDR, &rsp, sizeof(rsp));
}

static void cmd_le_set_event_mask(struct bt_le *hci,
						const void *data, uint8_t size)
{
	const struct bt_hci_cmd_le_set_event_mask *cmd = data;
	uint8_t status;

	memcpy(hci->le_event_mask, cmd->mask, 8);

	status = BT_HCI_ERR_SUCCESS;
	cmd_complete(hci, BT_HCI_CMD_LE_SET_EVENT_MASK,
						&status, sizeof(status));
}

static void cmd_le_read_buffer_size(struct bt_le *hci,
						const void *data, uint8_t size)
{
	struct bt_hci_rsp_le_read_buffer_size rsp;

	rsp.status = BT_HCI_ERR_SUCCESS;
	rsp.le_mtu = cpu_to_le16(hci->le_mtu);
	rsp.le_max_pkt = hci->le_max_pkt;

	cmd_complete(hci, BT_HCI_CMD_LE_READ_BUFFER_SIZE, &rsp, sizeof(rsp));
}

static void cmd_le_read_local_features(struct bt_le *hci,
						const void *data, uint8_t size)
{
	struct bt_hci_rsp_le_read_local_features rsp;

	rsp.status = BT_HCI_ERR_SUCCESS;
	memcpy(rsp.features, hci->le_features, 8);

	cmd_complete(hci, BT_HCI_CMD_LE_READ_LOCAL_FEATURES,
							&rsp, sizeof(rsp));
}

static void cmd_le_set_random_address(struct bt_le *hci,
						const void *data, uint8_t size)
{
	const struct bt_hci_cmd_le_set_random_address *cmd = data;
	uint8_t status;

	memcpy(hci->le_random_addr, cmd->addr, 6);

	status = BT_HCI_ERR_SUCCESS;
	cmd_complete(hci, BT_HCI_CMD_LE_SET_RANDOM_ADDRESS,
						&status, sizeof(status));
}

static void cmd_le_set_adv_parameters(struct bt_le *hci,
						const void *data, uint8_t size)
{
	const struct bt_hci_cmd_le_set_adv_parameters *cmd = data;
	uint16_t min_interval, max_interval;
	uint8_t status;

	if (hci->le_adv_enable == 0x01) {
		cmd_status(hci, BT_HCI_ERR_COMMAND_DISALLOWED,
					BT_HCI_CMD_LE_SET_ADV_PARAMETERS);
		return;
	}

	min_interval = le16_to_cpu(cmd->min_interval);
	max_interval = le16_to_cpu(cmd->max_interval);

	/* Valid range for advertising type is 0x00 to 0x03 */
	switch (cmd->type) {
	case 0x00:	/* ADV_IND */
		/* Range for advertising interval min is 0x0020 to 0x4000 */
		if (min_interval < 0x0020 || min_interval > 0x4000) {
			cmd_status(hci, BT_HCI_ERR_INVALID_PARAMETERS,
					BT_HCI_CMD_LE_SET_ADV_PARAMETERS);
			return;
		}
		/* Range for advertising interval max is 0x0020 to 0x4000 */
		if (max_interval < 0x0020 || max_interval > 0x4000) {
			cmd_status(hci, BT_HCI_ERR_INVALID_PARAMETERS,
					BT_HCI_CMD_LE_SET_ADV_PARAMETERS);
			return;
		}
		/* Advertising interval max shall be less or equal */
		if (min_interval > max_interval) {
			cmd_status(hci, BT_HCI_ERR_INVALID_PARAMETERS,
					BT_HCI_CMD_LE_SET_ADV_PARAMETERS);
			return;
		}
		break;

	case 0x01:	/* ADV_DIRECT_IND */
		/* Range for direct address type is 0x00 to 0x01 */
		if (cmd->direct_addr_type > 0x01) {
			cmd_status(hci, BT_HCI_ERR_INVALID_PARAMETERS,
					BT_HCI_CMD_LE_SET_ADV_PARAMETERS);
			return;
		}
		break;

	case 0x02:	/* ADV_SCAN_IND */
	case 0x03:	/* ADV_NONCONN_IND */
		/* Range for advertising interval min is 0x00a0 to 0x4000 */
		if (min_interval < 0x00a0 || min_interval > 0x4000) {
			cmd_status(hci, BT_HCI_ERR_INVALID_PARAMETERS,
					BT_HCI_CMD_LE_SET_ADV_PARAMETERS);
			return;
		}
		/* Range for advertising interval max is 0x00a0 to 0x4000 */
		if (max_interval < 0x00a0 || max_interval > 0x4000) {
			cmd_status(hci, BT_HCI_ERR_INVALID_PARAMETERS,
					BT_HCI_CMD_LE_SET_ADV_PARAMETERS);
			return;
		}
		/* Advertising interval min shall be less or equal */
		if (min_interval > max_interval) {
			cmd_status(hci, BT_HCI_ERR_INVALID_PARAMETERS,
					BT_HCI_CMD_LE_SET_ADV_PARAMETERS);
			return;
		}
		break;

	default:
		cmd_status(hci, BT_HCI_ERR_INVALID_PARAMETERS,
					BT_HCI_CMD_LE_SET_ADV_PARAMETERS);
		return;
	}

	/* Valid range for own address type is 0x00 to 0x03 */
	if (cmd->own_addr_type > 0x03) {
		cmd_status(hci, BT_HCI_ERR_INVALID_PARAMETERS,
					BT_HCI_CMD_LE_SET_ADV_PARAMETERS);
		return;
	}

	/* Valid range for advertising channel map is 0x01 to 0x07 */
	if (cmd->channel_map < 0x01 || cmd->channel_map > 0x07) {
		cmd_status(hci, BT_HCI_ERR_INVALID_PARAMETERS,
					BT_HCI_CMD_LE_SET_ADV_PARAMETERS);
		return;
	}

	/* Valid range for advertising filter policy is 0x00 to 0x03 */
	if (cmd->filter_policy > 0x03) {
		cmd_status(hci, BT_HCI_ERR_INVALID_PARAMETERS,
					BT_HCI_CMD_LE_SET_ADV_PARAMETERS);
		return;
	}

	hci->le_adv_min_interval = min_interval;
	hci->le_adv_max_interval = max_interval;
	hci->le_adv_type = cmd->type;
	hci->le_adv_own_addr_type = cmd->own_addr_type;
	hci->le_adv_direct_addr_type = cmd->direct_addr_type;
	memcpy(hci->le_adv_direct_addr, cmd->direct_addr, 6);
	hci->le_adv_channel_map = cmd->channel_map;
	hci->le_adv_filter_policy = cmd->filter_policy;

	status = BT_HCI_ERR_SUCCESS;
	cmd_complete(hci, BT_HCI_CMD_LE_SET_ADV_PARAMETERS,
						&status, sizeof(status));
}

static void cmd_le_read_adv_tx_power(struct bt_le *hci,
						const void *data, uint8_t size)
{
	struct bt_hci_rsp_le_read_adv_tx_power rsp;

	rsp.status = BT_HCI_ERR_SUCCESS;
	rsp.level = hci->le_adv_tx_power;

	cmd_complete(hci, BT_HCI_CMD_LE_READ_ADV_TX_POWER, &rsp, sizeof(rsp));
}

static void cmd_le_set_adv_data(struct bt_le *hci,
						const void *data, uint8_t size)
{
	const struct bt_hci_cmd_le_set_adv_data *cmd = data;
	uint8_t status;

	/* Valid range for advertising data length is 0x00 to 0x1f */
	if (cmd->len > 0x1f) {
		cmd_status(hci, BT_HCI_ERR_INVALID_PARAMETERS,
					BT_HCI_CMD_LE_SET_ADV_DATA);
		return;
	}

	hci->le_adv_data_len = cmd->len;
	memcpy(hci->le_adv_data, cmd->data, 31);

	status = BT_HCI_ERR_SUCCESS;
	cmd_complete(hci, BT_HCI_CMD_LE_SET_ADV_DATA, &status, sizeof(status));
}

static void cmd_le_set_scan_rsp_data(struct bt_le *hci,
						const void *data, uint8_t size)
{
	const struct bt_hci_cmd_le_set_scan_rsp_data *cmd = data;
	uint8_t status;

	/* Valid range for scan response data length is 0x00 to 0x1f */
	if (cmd->len > 0x1f) {
		cmd_status(hci, BT_HCI_ERR_INVALID_PARAMETERS,
					BT_HCI_CMD_LE_SET_SCAN_RSP_DATA);
		return;
	}

	hci->le_scan_rsp_data_len = cmd->len;
	memcpy(hci->le_scan_rsp_data, cmd->data, 31);

	status = BT_HCI_ERR_SUCCESS;
	cmd_complete(hci, BT_HCI_CMD_LE_SET_SCAN_RSP_DATA,
						&status, sizeof(status));
}

static void cmd_le_set_adv_enable(struct bt_le *hci,
						const void *data, uint8_t size)
{
	const struct bt_hci_cmd_le_set_adv_enable *cmd = data;
	uint8_t status;
	bool result;

	/* Valid range for advertising enable is 0x00 to 0x01 */
	if (cmd->enable > 0x01) {
		cmd_status(hci, BT_HCI_ERR_INVALID_PARAMETERS,
					BT_HCI_CMD_LE_SET_ADV_ENABLE);
		return;
	}

	if (cmd->enable == hci->le_adv_enable) {
		cmd_status(hci, BT_HCI_ERR_COMMAND_DISALLOWED,
					BT_HCI_CMD_LE_SET_ADV_ENABLE);
		return;
	}

	if (cmd->enable == 0x01)
		result = start_adv(hci);
	else
		result = stop_adv(hci);

	if (!result) {
		cmd_status(hci, BT_HCI_ERR_UNSPECIFIED_ERROR,
					BT_HCI_CMD_LE_SET_ADV_ENABLE);
		return;
	}

	hci->le_adv_enable = cmd->enable;

	status = BT_HCI_ERR_SUCCESS;
	cmd_complete(hci, BT_HCI_CMD_LE_SET_ADV_ENABLE,
						&status, sizeof(status));
}

static void cmd_le_set_scan_parameters(struct bt_le *hci,
						const void *data, uint8_t size)
{
	const struct bt_hci_cmd_le_set_scan_parameters *cmd = data;
	uint16_t interval, window;
	uint8_t status;

	if (hci->le_scan_enable == 0x01) {
		cmd_status(hci, BT_HCI_ERR_COMMAND_DISALLOWED,
					BT_HCI_CMD_LE_SET_SCAN_PARAMETERS);
		return;
	}

	interval = le16_to_cpu(cmd->interval);
	window = le16_to_cpu(cmd->window);

	/* Valid range for scan type is 0x00 to 0x01 */
	if (cmd->type > 0x01) {
		cmd_status(hci, BT_HCI_ERR_INVALID_PARAMETERS,
					BT_HCI_CMD_LE_SET_SCAN_PARAMETERS);
		return;
	}

	/* Valid range for scan interval is 0x0004 to 0x4000 */
	if (interval < 0x0004 || interval > 0x4000) {
		cmd_status(hci, BT_HCI_ERR_INVALID_PARAMETERS,
					BT_HCI_CMD_LE_SET_SCAN_PARAMETERS);
		return;
	}

	/* Valid range for scan window is 0x0004 to 0x4000 */
	if (window < 0x0004 || window > 0x4000) {
		cmd_status(hci, BT_HCI_ERR_INVALID_PARAMETERS,
					BT_HCI_CMD_LE_SET_SCAN_PARAMETERS);
		return;
	}

	/* Scan window shall be less or equal than scan interval */
	if (window > interval) {
		cmd_status(hci, BT_HCI_ERR_INVALID_PARAMETERS,
					BT_HCI_CMD_LE_SET_SCAN_PARAMETERS);
		return;
	}

	/* Valid range for own address type is 0x00 to 0x03 */
	if (cmd->own_addr_type > 0x03) {
		cmd_status(hci, BT_HCI_ERR_INVALID_PARAMETERS,
					BT_HCI_CMD_LE_SET_SCAN_PARAMETERS);
		return;
	}

	/* Valid range for scanning filter policy is 0x00 to 0x03 */
	if (cmd->filter_policy > 0x03) {
		cmd_status(hci, BT_HCI_ERR_INVALID_PARAMETERS,
					BT_HCI_CMD_LE_SET_SCAN_PARAMETERS);
		return;
	}

	hci->le_scan_type = cmd->type;
	hci->le_scan_interval = interval;
	hci->le_scan_window = window;
	hci->le_scan_own_addr_type = cmd->own_addr_type;
	hci->le_scan_filter_policy = cmd->filter_policy;

	status = BT_HCI_ERR_SUCCESS;
	cmd_complete(hci, BT_HCI_CMD_LE_SET_SCAN_PARAMETERS,
						&status, sizeof(status));
}

static void cmd_le_set_scan_enable(struct bt_le *hci,
						const void *data, uint8_t size)
{
	const struct bt_hci_cmd_le_set_scan_enable *cmd = data;
	uint8_t status;
	bool result;

	/* Valid range for scan enable is 0x00 to 0x01 */
	if (cmd->enable > 0x01) {
		cmd_status(hci, BT_HCI_ERR_INVALID_PARAMETERS,
					BT_HCI_CMD_LE_SET_SCAN_ENABLE);
		return;
	}

	/* Valid range for filter duplicates is 0x00 to 0x01 */
	if (cmd->filter_dup > 0x01) {
		cmd_status(hci, BT_HCI_ERR_INVALID_PARAMETERS,
					BT_HCI_CMD_LE_SET_SCAN_ENABLE);
		return;
	}

	if (cmd->enable == hci->le_scan_enable) {
		cmd_status(hci, BT_HCI_ERR_COMMAND_DISALLOWED,
					BT_HCI_CMD_LE_SET_SCAN_ENABLE);
		return;
	}

	clear_scan_cache(hci);

	if (cmd->enable == 0x01)
		result = start_scan(hci);
	else
		result = stop_scan(hci);

	if (!result) {
		cmd_status(hci, BT_HCI_ERR_UNSPECIFIED_ERROR,
					BT_HCI_CMD_LE_SET_SCAN_ENABLE);
		return;
	}

	hci->le_scan_enable = cmd->enable;
	hci->le_scan_filter_dup = cmd->filter_dup;

	status = BT_HCI_ERR_SUCCESS;
	cmd_complete(hci, BT_HCI_CMD_LE_SET_SCAN_ENABLE,
						&status, sizeof(status));
}

static void cmd_le_create_conn(struct bt_le *hci,
						const void *data, uint8_t size)
{
	const struct bt_hci_cmd_le_create_conn *cmd = data;

	if (hci->le_conn_enable == 0x01) {
		cmd_status(hci, BT_HCI_ERR_COMMAND_DISALLOWED,
					BT_HCI_CMD_LE_CREATE_CONN);
		return;
	}

	/* Valid range for peer address type is 0x00 to 0x03 */
	if (cmd->peer_addr_type > 0x03) {
		cmd_status(hci, BT_HCI_ERR_INVALID_PARAMETERS,
					BT_HCI_CMD_LE_CREATE_CONN);
		return;
	}

	/* Valid range for own address type is 0x00 to 0x03 */
	if (cmd->own_addr_type > 0x03) {
		cmd_status(hci, BT_HCI_ERR_INVALID_PARAMETERS,
					BT_HCI_CMD_LE_CREATE_CONN);
		return;
	}

	hci->le_conn_peer_addr_type = cmd->peer_addr_type;
	memcpy(hci->le_conn_peer_addr, cmd->peer_addr, 6);
	hci->le_conn_own_addr_type = cmd->own_addr_type;
	hci->le_conn_enable = 0x01;

	cmd_status(hci, BT_HCI_ERR_SUCCESS, BT_HCI_CMD_LE_CREATE_CONN);
}

static void cmd_le_create_conn_cancel(struct bt_le *hci,
						const void *data, uint8_t size)
{
	struct bt_hci_evt_le_conn_complete evt;
	uint8_t status;

	if (hci->le_conn_enable == 0x00) {
		cmd_status(hci, BT_HCI_ERR_COMMAND_DISALLOWED,
					BT_HCI_CMD_LE_CREATE_CONN_CANCEL);
		return;
	}

	hci->le_conn_enable = 0x00;

	status = BT_HCI_ERR_SUCCESS;
	cmd_complete(hci, BT_HCI_CMD_LE_CREATE_CONN_CANCEL,
						&status, sizeof(status));

	evt.status = BT_HCI_ERR_UNKNOWN_CONN_ID;
	evt.handle = cpu_to_le16(0x0000);
	evt.role = 0x00;
	evt.peer_addr_type = 0x00;
	memset(evt.peer_addr, 0, 6);
	evt.interval = cpu_to_le16(0x0000);
	evt.latency = cpu_to_le16(0x0000);
	evt.supv_timeout = cpu_to_le16(0x0000);
	evt.clock_accuracy = 0x00;

	if (hci->le_event_mask[0] & 0x01)
		le_meta_event(hci, BT_HCI_EVT_LE_CONN_COMPLETE,
							&evt, sizeof(evt));
}

static void cmd_le_read_white_list_size(struct bt_le *hci,
						const void *data, uint8_t size)
{
	struct bt_hci_rsp_le_read_white_list_size rsp;

	rsp.status = BT_HCI_ERR_SUCCESS;
	rsp.size = hci->le_white_list_size;

	cmd_complete(hci, BT_HCI_CMD_LE_READ_WHITE_LIST_SIZE,
							&rsp, sizeof(rsp));
}

static void cmd_le_clear_white_list(struct bt_le *hci,
						const void *data, uint8_t size)
{
	uint8_t status;

	clear_white_list(hci);

	status = BT_HCI_ERR_SUCCESS;
	cmd_complete(hci, BT_HCI_CMD_LE_CLEAR_WHITE_LIST,
						&status, sizeof(status));
}

static void cmd_le_add_to_white_list(struct bt_le *hci,
						const void *data, uint8_t size)
{
	const struct bt_hci_cmd_le_add_to_white_list *cmd = data;
	uint8_t status;
	bool exists = false;
	int i, pos = -1;

	/* Valid range for address type is 0x00 to 0x01 */
	if (cmd->addr_type > 0x01) {
		cmd_status(hci, BT_HCI_ERR_INVALID_PARAMETERS,
					BT_HCI_CMD_LE_ADD_TO_WHITE_LIST);
		return;
	}

	for (i = 0; i < hci->le_white_list_size; i++) {
		if (hci->le_white_list[i][0] == cmd->addr_type &&
				!memcmp(&hci->le_white_list[i][1],
							cmd->addr, 6)) {
			exists = true;
			break;
		} else if (pos < 0 && hci->le_white_list[i][0] == 0xff)
			pos = i;
	}

	if (exists) {
		cmd_status(hci, BT_HCI_ERR_UNSPECIFIED_ERROR,
					BT_HCI_CMD_LE_ADD_TO_WHITE_LIST);
		return;
	}

	if (pos < 0) {
		cmd_status(hci, BT_HCI_ERR_MEM_CAPACITY_EXCEEDED,
					BT_HCI_CMD_LE_ADD_TO_WHITE_LIST);
		return;
	}

	hci->le_white_list[pos][0] = cmd->addr_type;
	memcpy(&hci->le_white_list[pos][1], cmd->addr, 6);

	status = BT_HCI_ERR_SUCCESS;
	cmd_complete(hci, BT_HCI_CMD_LE_ADD_TO_WHITE_LIST,
						&status, sizeof(status));
}

static void cmd_le_remove_from_white_list(struct bt_le *hci,
						const void *data, uint8_t size)
{
	const struct bt_hci_cmd_le_remove_from_white_list *cmd = data;
	uint8_t status;
	int i, pos = -1;

	/* Valid range for address type is 0x00 to 0x01 */
	if (cmd->addr_type > 0x01) {
		cmd_status(hci, BT_HCI_ERR_INVALID_PARAMETERS,
					BT_HCI_CMD_LE_REMOVE_FROM_WHITE_LIST);
		return;
	}

	for (i = 0; i < hci->le_white_list_size; i++) {
		if (hci->le_white_list[i][0] == cmd->addr_type &&
				!memcmp(&hci->le_white_list[i][1],
							cmd->addr, 6)) {
			pos = i;
			break;
		}
	}

	if (pos < 0) {
		cmd_status(hci, BT_HCI_ERR_INVALID_PARAMETERS,
					BT_HCI_CMD_LE_REMOVE_FROM_WHITE_LIST);
		return;
	}

	hci->le_white_list[pos][0] = 0xff;
	memset(&hci->le_white_list[pos][1], 0, 6);

	status = BT_HCI_ERR_SUCCESS;
	cmd_complete(hci, BT_HCI_CMD_LE_REMOVE_FROM_WHITE_LIST,
						&status, sizeof(status));
}

static void cmd_le_encrypt(struct bt_le *hci, const void *data, uint8_t size)
{
	const struct bt_hci_cmd_le_encrypt *cmd = data;
	struct bt_hci_rsp_le_encrypt rsp;

	if (!bt_crypto_e(hci->crypto, cmd->key, cmd->plaintext, rsp.data)) {
		cmd_status(hci, BT_HCI_ERR_COMMAND_DISALLOWED,
					BT_HCI_CMD_LE_ENCRYPT);
		return;
	}

	rsp.status = BT_HCI_ERR_SUCCESS;

	cmd_complete(hci, BT_HCI_CMD_LE_ENCRYPT, &rsp, sizeof(rsp));
}

static void cmd_le_rand(struct bt_le *hci, const void *data, uint8_t size)
{
	struct bt_hci_rsp_le_rand rsp;
	uint8_t value[8];

	if (!bt_crypto_random_bytes(hci->crypto, value, 8)) {
		cmd_status(hci, BT_HCI_ERR_COMMAND_DISALLOWED,
					BT_HCI_CMD_LE_RAND);
		return;
	}

	rsp.status = BT_HCI_ERR_SUCCESS;
	memcpy(&rsp.number, value, 8);

	cmd_complete(hci, BT_HCI_CMD_LE_RAND, &rsp, sizeof(rsp));
}

static void cmd_le_read_supported_states(struct bt_le *hci,
						const void *data, uint8_t size)
{
	struct bt_hci_rsp_le_read_supported_states rsp;

	rsp.status = BT_HCI_ERR_SUCCESS;
	memcpy(rsp.states, hci->le_states, 8);

	cmd_complete(hci, BT_HCI_CMD_LE_READ_SUPPORTED_STATES,
							&rsp, sizeof(rsp));
}

static void cmd_le_set_data_length(struct bt_le *hci,
						const void *data, uint8_t size)
{
	const struct bt_hci_cmd_le_set_data_length *cmd = data;
	struct bt_hci_rsp_le_set_data_length rsp;
	uint16_t handle, tx_len, tx_time;

	handle = le16_to_cpu(cmd->handle);
	tx_len = le16_to_cpu(cmd->tx_len);
	tx_time = le16_to_cpu(cmd->tx_time);

	/* Valid range for connection handle is 0x0000 to 0x0eff */
	if (handle > 0x0eff) {
		cmd_status(hci, BT_HCI_ERR_INVALID_PARAMETERS,
					BT_HCI_CMD_LE_SET_DATA_LENGTH);
		return;
	}

	/* Valid range for suggested max TX octets is 0x001b to 0x00fb */
	if (tx_len < 0x001b || tx_len > 0x00fb) {
		cmd_status(hci, BT_HCI_ERR_INVALID_PARAMETERS,
					BT_HCI_CMD_LE_SET_DATA_LENGTH);
		return;
	}

	/* Valid range for suggested max TX time is 0x0148 to 0x0848 */
	if (tx_time < 0x0148 || tx_time > 0x0848) {
		cmd_status(hci, BT_HCI_ERR_INVALID_PARAMETERS,
					BT_HCI_CMD_LE_SET_DATA_LENGTH);
		return;
	}

	/* Max TX len and time shall be less or equal supported */
	if (tx_len > MAX_TX_LEN || tx_time > MAX_TX_TIME) {
		cmd_status(hci, BT_HCI_ERR_INVALID_PARAMETERS,
					BT_HCI_CMD_LE_SET_DATA_LENGTH);
		return;
	}

	rsp.status = BT_HCI_ERR_SUCCESS;
	rsp.handle = cpu_to_le16(handle);

	cmd_complete(hci, BT_HCI_CMD_LE_SET_DATA_LENGTH, &rsp, sizeof(rsp));
}

static void cmd_le_read_default_data_length(struct bt_le *hci,
						const void *data, uint8_t size)
{
	struct bt_hci_rsp_le_read_default_data_length rsp;

	rsp.status = BT_HCI_ERR_SUCCESS;
	rsp.tx_len = cpu_to_le16(hci->le_default_tx_len);
	rsp.tx_time = cpu_to_le16(hci->le_default_tx_time);

	cmd_complete(hci, BT_HCI_CMD_LE_READ_DEFAULT_DATA_LENGTH,
							&rsp, sizeof(rsp));
}

static void cmd_le_write_default_data_length(struct bt_le *hci,
						const void *data, uint8_t size)
{
	const struct bt_hci_cmd_le_write_default_data_length *cmd = data;
	uint16_t tx_len, tx_time;
	uint8_t status;

	tx_len = le16_to_cpu(cmd->tx_len);
	tx_time = le16_to_cpu(cmd->tx_time);

	/* Valid range for suggested max TX octets is 0x001b to 0x00fb */
	if (tx_len < 0x001b || tx_len > 0x00fb) {
		cmd_status(hci, BT_HCI_ERR_INVALID_PARAMETERS,
				BT_HCI_CMD_LE_WRITE_DEFAULT_DATA_LENGTH);
		return;
	}

	/* Valid range for suggested max TX time is 0x0148 to 0x0848 */
	if (tx_time < 0x0148 || tx_time > 0x0848) {
		cmd_status(hci, BT_HCI_ERR_INVALID_PARAMETERS,
				BT_HCI_CMD_LE_WRITE_DEFAULT_DATA_LENGTH);
		return;
	}

	/* Suggested max TX len and time shall be less or equal supported */
	if (tx_len > MAX_TX_LEN || tx_time > MAX_TX_TIME) {
		cmd_status(hci, BT_HCI_ERR_INVALID_PARAMETERS,
				BT_HCI_CMD_LE_WRITE_DEFAULT_DATA_LENGTH);
		return;
	}

	hci->le_default_tx_len = tx_len;
	hci->le_default_tx_time = tx_time;

	status = BT_HCI_ERR_SUCCESS;
	cmd_complete(hci, BT_HCI_CMD_LE_WRITE_DEFAULT_DATA_LENGTH,
						&status, sizeof(status));
}

static void cmd_le_read_local_pk256(struct bt_le *hci,
						const void *data, uint8_t size)
{
	struct bt_hci_evt_le_read_local_pk256_complete evt;

	cmd_status(hci, BT_HCI_ERR_SUCCESS, BT_HCI_CMD_LE_READ_LOCAL_PK256);

	evt.status = BT_HCI_ERR_SUCCESS;
	ecc_make_key(evt.local_pk256, hci->le_local_sk256);

	if (hci->le_event_mask[0] & 0x80)
		le_meta_event(hci, BT_HCI_EVT_LE_READ_LOCAL_PK256_COMPLETE,
							&evt, sizeof(evt));
}

static void cmd_le_generate_dhkey(struct bt_le *hci,
						const void *data, uint8_t size)
{
	const struct bt_hci_cmd_le_generate_dhkey *cmd = data;
	struct bt_hci_evt_le_generate_dhkey_complete evt;

	cmd_status(hci, BT_HCI_ERR_SUCCESS, BT_HCI_CMD_LE_GENERATE_DHKEY);

	evt.status = BT_HCI_ERR_SUCCESS;
	ecdh_shared_secret(cmd->remote_pk256, hci->le_local_sk256, evt.dhkey);

	if (hci->le_event_mask[1] & 0x01)
		le_meta_event(hci, BT_HCI_EVT_LE_GENERATE_DHKEY_COMPLETE,
							&evt, sizeof(evt));
}

static void cmd_le_add_to_resolv_list(struct bt_le *hci,
						const void *data, uint8_t size)
{
	const struct bt_hci_cmd_le_add_to_resolv_list *cmd = data;
	uint8_t status;
	bool exists = false;
	int i, pos = -1;

	/* Valid range for address type is 0x00 to 0x01 */
	if (cmd->addr_type > 0x01) {
		cmd_status(hci, BT_HCI_ERR_INVALID_PARAMETERS,
					BT_HCI_CMD_LE_ADD_TO_RESOLV_LIST);
		return;
	}

	for (i = 0; i < hci->le_resolv_list_size; i++) {
		if (hci->le_resolv_list[i][0] == cmd->addr_type &&
				!memcmp(&hci->le_resolv_list[i][1],
							cmd->addr, 6)) {
			exists = true;
			break;
		} else if (pos < 0 && hci->le_resolv_list[i][0] == 0xff)
			pos = i;
	}

	if (exists) {
		cmd_status(hci, BT_HCI_ERR_UNSPECIFIED_ERROR,
					BT_HCI_CMD_LE_ADD_TO_RESOLV_LIST);
		return;
	}

	if (pos < 0) {
		cmd_status(hci, BT_HCI_ERR_MEM_CAPACITY_EXCEEDED,
					BT_HCI_CMD_LE_ADD_TO_RESOLV_LIST);
		return;
	}

	hci->le_resolv_list[pos][0] = cmd->addr_type;
	memcpy(&hci->le_resolv_list[pos][1], cmd->addr, 6);
	memcpy(&hci->le_resolv_list[pos][7], cmd->peer_irk, 16);
	memcpy(&hci->le_resolv_list[pos][23], cmd->local_irk, 16);

	status = BT_HCI_ERR_SUCCESS;
	cmd_complete(hci, BT_HCI_CMD_LE_ADD_TO_RESOLV_LIST,
						&status, sizeof(status));
}

static void cmd_le_remove_from_resolv_list(struct bt_le *hci,
						const void *data, uint8_t size)
{
	const struct bt_hci_cmd_le_remove_from_resolv_list *cmd = data;
	uint8_t status;
	int i, pos = -1;

	/* Valid range for address type is 0x00 to 0x01 */
	if (cmd->addr_type > 0x01) {
		cmd_status(hci, BT_HCI_ERR_INVALID_PARAMETERS,
					BT_HCI_CMD_LE_REMOVE_FROM_RESOLV_LIST);
		return;
	}

	for (i = 0; i < hci->le_resolv_list_size; i++) {
		if (hci->le_resolv_list[i][0] == cmd->addr_type &&
				!memcmp(&hci->le_resolv_list[i][1],
							cmd->addr, 6)) {
			pos = i;
			break;
		}
	}

	if (pos < 0) {
		cmd_status(hci, BT_HCI_ERR_INVALID_PARAMETERS,
					BT_HCI_CMD_LE_REMOVE_FROM_RESOLV_LIST);
		return;
	}

	hci->le_resolv_list[pos][0] = 0xff;
	memset(&hci->le_resolv_list[pos][1], 0, 38);

	status = BT_HCI_ERR_SUCCESS;
	cmd_complete(hci, BT_HCI_CMD_LE_REMOVE_FROM_RESOLV_LIST,
						&status, sizeof(status));
}

static void cmd_le_clear_resolv_list(struct bt_le *hci,
						const void *data, uint8_t size)
{
	uint8_t status;

	clear_resolv_list(hci);

	status = BT_HCI_ERR_SUCCESS;
	cmd_complete(hci, BT_HCI_CMD_LE_CLEAR_RESOLV_LIST,
						&status, sizeof(status));
}

static void cmd_le_read_resolv_list_size(struct bt_le *hci,
						const void *data, uint8_t size)
{
	struct bt_hci_rsp_le_read_resolv_list_size rsp;

	rsp.status = BT_HCI_ERR_SUCCESS;
	rsp.size = hci->le_resolv_list_size;

	cmd_complete(hci, BT_HCI_CMD_LE_READ_RESOLV_LIST_SIZE,
							&rsp, sizeof(rsp));
}

static void cmd_le_read_peer_resolv_addr(struct bt_le *hci,
						const void *data, uint8_t size)
{
	const struct bt_hci_cmd_le_read_peer_resolv_addr *cmd = data;
	struct bt_hci_rsp_le_read_peer_resolv_addr rsp;

	/* Valid range for address type is 0x00 to 0x01 */
	if (cmd->addr_type > 0x01) {
		cmd_status(hci, BT_HCI_ERR_INVALID_PARAMETERS,
					BT_HCI_CMD_LE_READ_PEER_RESOLV_ADDR);
		return;
	}

	rsp.status = BT_HCI_ERR_UNKNOWN_CONN_ID;
	memset(rsp.addr, 0, 6);

	cmd_complete(hci, BT_HCI_CMD_LE_READ_PEER_RESOLV_ADDR,
							&rsp, sizeof(rsp));
}

static void cmd_le_read_local_resolv_addr(struct bt_le *hci,
						const void *data, uint8_t size)
{
	const struct bt_hci_cmd_le_read_local_resolv_addr *cmd = data;
	struct bt_hci_rsp_le_read_local_resolv_addr rsp;

	/* Valid range for address type is 0x00 to 0x01 */
	if (cmd->addr_type > 0x01) {
		cmd_status(hci, BT_HCI_ERR_INVALID_PARAMETERS,
					BT_HCI_CMD_LE_READ_LOCAL_RESOLV_ADDR);
		return;
	}

	rsp.status = BT_HCI_ERR_UNKNOWN_CONN_ID;
	memset(rsp.addr, 0, 6);

	cmd_complete(hci, BT_HCI_CMD_LE_READ_LOCAL_RESOLV_ADDR,
							&rsp, sizeof(rsp));
}

static void cmd_le_set_resolv_enable(struct bt_le *hci,
						const void *data, uint8_t size)
{
	const struct bt_hci_cmd_le_set_resolv_enable *cmd = data;
	uint8_t status;

	/* Valid range for address resolution enable is 0x00 to 0x01 */
	if (cmd->enable > 0x01) {
		cmd_status(hci, BT_HCI_ERR_INVALID_PARAMETERS,
					BT_HCI_CMD_LE_SET_RESOLV_ENABLE);
		return;
	}

	hci->le_resolv_enable = cmd->enable;

	status = BT_HCI_ERR_SUCCESS;
	cmd_complete(hci, BT_HCI_CMD_LE_SET_RESOLV_ENABLE,
						&status, sizeof(status));
}

static void cmd_le_set_resolv_timeout(struct bt_le *hci,
						const void *data, uint8_t size)
{
	const struct bt_hci_cmd_le_set_resolv_timeout *cmd = data;
	uint16_t timeout;
	uint8_t status;

	timeout = le16_to_cpu(cmd->timeout);

	/* Valid range for RPA timeout is 0x0001 to 0xa1b8 */
	if (timeout < 0x0001 || timeout > 0xa1b8) {
		cmd_status(hci, BT_HCI_ERR_INVALID_PARAMETERS,
					BT_HCI_CMD_LE_SET_RESOLV_TIMEOUT);
		return;
	}

	hci->le_resolv_timeout = timeout;

	status = BT_HCI_ERR_SUCCESS;
	cmd_complete(hci, BT_HCI_CMD_LE_SET_RESOLV_TIMEOUT,
						&status, sizeof(status));
}

static void cmd_le_read_max_data_length(struct bt_le *hci,
						const void *data, uint8_t size)
{
	struct bt_hci_rsp_le_read_max_data_length rsp;

	rsp.status = BT_HCI_ERR_SUCCESS;
	rsp.max_tx_len = cpu_to_le16(MAX_TX_LEN);
	rsp.max_tx_time = cpu_to_le16(MAX_TX_TIME);
	rsp.max_rx_len = cpu_to_le16(MAX_RX_LEN);
	rsp.max_rx_time = cpu_to_le16(MAX_RX_TIME);

	cmd_complete(hci, BT_HCI_CMD_LE_READ_MAX_DATA_LENGTH,
							&rsp, sizeof(rsp));
}

static const struct {
	uint16_t opcode;
	void (*func) (struct bt_le *hci, const void *data, uint8_t size);
	uint8_t size;
	bool fixed;
} cmd_table[] = {
	{ BT_HCI_CMD_DISCONNECT,           cmd_disconnect,           3, true },

	{ BT_HCI_CMD_SET_EVENT_MASK,       cmd_set_event_mask,       8, true },
	{ BT_HCI_CMD_RESET,                cmd_reset,                0, true },
	{ BT_HCI_CMD_SET_EVENT_MASK_PAGE2, cmd_set_event_mask_page2, 8, true },

	{ BT_HCI_CMD_READ_LOCAL_VERSION,   cmd_read_local_version,   0, true },
	{ BT_HCI_CMD_READ_LOCAL_COMMANDS,  cmd_read_local_commands,  0, true },
	{ BT_HCI_CMD_READ_LOCAL_FEATURES,  cmd_read_local_features,  0, true },
	{ BT_HCI_CMD_READ_BUFFER_SIZE,     cmd_read_buffer_size,     0, true },
	{ BT_HCI_CMD_READ_BD_ADDR,         cmd_read_bd_addr,         0, true },

	{ BT_HCI_CMD_LE_SET_EVENT_MASK,
				cmd_le_set_event_mask, 8, true },
	{ BT_HCI_CMD_LE_READ_BUFFER_SIZE,
				cmd_le_read_buffer_size, 0, true },
	{ BT_HCI_CMD_LE_READ_LOCAL_FEATURES,
				cmd_le_read_local_features, 0, true },
	{ BT_HCI_CMD_LE_SET_RANDOM_ADDRESS,
				cmd_le_set_random_address, 6, true },
	{ BT_HCI_CMD_LE_SET_ADV_PARAMETERS,
				cmd_le_set_adv_parameters, 15, true },
	{ BT_HCI_CMD_LE_READ_ADV_TX_POWER,
				cmd_le_read_adv_tx_power, 0, true },
	{ BT_HCI_CMD_LE_SET_ADV_DATA,
				cmd_le_set_adv_data, 32, true },
	{ BT_HCI_CMD_LE_SET_SCAN_RSP_DATA,
				cmd_le_set_scan_rsp_data, 32, true },
	{ BT_HCI_CMD_LE_SET_ADV_ENABLE,
				cmd_le_set_adv_enable, 1, true },
	{ BT_HCI_CMD_LE_SET_SCAN_PARAMETERS,
				cmd_le_set_scan_parameters, 7, true },
	{ BT_HCI_CMD_LE_SET_SCAN_ENABLE,
				cmd_le_set_scan_enable, 2, true },
	{ BT_HCI_CMD_LE_CREATE_CONN,
				cmd_le_create_conn, 25, true },
	{ BT_HCI_CMD_LE_CREATE_CONN_CANCEL,
				cmd_le_create_conn_cancel, 0, true },
	{ BT_HCI_CMD_LE_READ_WHITE_LIST_SIZE,
				cmd_le_read_white_list_size, 0, true },
	{ BT_HCI_CMD_LE_CLEAR_WHITE_LIST,
				cmd_le_clear_white_list, 0, true },
	{ BT_HCI_CMD_LE_ADD_TO_WHITE_LIST,
				cmd_le_add_to_white_list,  7, true },
	{ BT_HCI_CMD_LE_REMOVE_FROM_WHITE_LIST,
				cmd_le_remove_from_white_list, 7, true },

	{ BT_HCI_CMD_LE_ENCRYPT, cmd_le_encrypt, 32, true },
	{ BT_HCI_CMD_LE_RAND, cmd_le_rand, 0, true },

	{ BT_HCI_CMD_LE_READ_SUPPORTED_STATES,
				cmd_le_read_supported_states, 0, true },

	{ BT_HCI_CMD_LE_SET_DATA_LENGTH,
				cmd_le_set_data_length, 6, true },
	{ BT_HCI_CMD_LE_READ_DEFAULT_DATA_LENGTH,
				cmd_le_read_default_data_length, 0, true },
	{ BT_HCI_CMD_LE_WRITE_DEFAULT_DATA_LENGTH,
				cmd_le_write_default_data_length, 4, true },
	{ BT_HCI_CMD_LE_READ_LOCAL_PK256,
				cmd_le_read_local_pk256, 0, true },
	{ BT_HCI_CMD_LE_GENERATE_DHKEY,
				cmd_le_generate_dhkey, 64, true },
	{ BT_HCI_CMD_LE_ADD_TO_RESOLV_LIST,
				cmd_le_add_to_resolv_list,  39, true },
	{ BT_HCI_CMD_LE_REMOVE_FROM_RESOLV_LIST,
				cmd_le_remove_from_resolv_list, 7, true },
	{ BT_HCI_CMD_LE_CLEAR_RESOLV_LIST,
				cmd_le_clear_resolv_list, 0, true },
	{ BT_HCI_CMD_LE_READ_RESOLV_LIST_SIZE,
				cmd_le_read_resolv_list_size, 0, true },
	{ BT_HCI_CMD_LE_READ_PEER_RESOLV_ADDR,
				cmd_le_read_peer_resolv_addr, 7, true },
	{ BT_HCI_CMD_LE_READ_LOCAL_RESOLV_ADDR,
				cmd_le_read_local_resolv_addr, 7, true },
	{ BT_HCI_CMD_LE_SET_RESOLV_ENABLE,
				cmd_le_set_resolv_enable, 1, true },
	{ BT_HCI_CMD_LE_SET_RESOLV_TIMEOUT,
				cmd_le_set_resolv_timeout, 2, true },
	{ BT_HCI_CMD_LE_READ_MAX_DATA_LENGTH,
				cmd_le_read_max_data_length, 0, true },

	{ }
};

static void process_command(struct bt_le *hci, const void *data, size_t size)
{
	const struct bt_hci_cmd_hdr *hdr = data;
	uint16_t opcode;
	unsigned int i;

	if (size < sizeof(*hdr))
		return;

	data += sizeof(*hdr);
	size -= sizeof(*hdr);

	opcode = le16_to_cpu(hdr->opcode);

	if (hdr->plen != size) {
		cmd_status(hci, BT_HCI_ERR_INVALID_PARAMETERS, opcode);
		return;
	}

	for (i = 0; cmd_table[i].func; i++) {
		if (cmd_table[i].opcode != opcode)
			continue;

		if ((cmd_table[i].fixed && size != cmd_table[i].size) ||
						size < cmd_table[i].size) {
			cmd_status(hci, BT_HCI_ERR_INVALID_PARAMETERS, opcode);
			return;
		}

		cmd_table[i].func(hci, data, size);
		return;
	}

	cmd_status(hci, BT_HCI_ERR_UNKNOWN_COMMAND, opcode);
}

static void vhci_read_callback(int fd, uint32_t events, void *user_data)
{
	struct bt_le *hci = user_data;
	unsigned char buf[4096];
	ssize_t len;

	if (events & (EPOLLERR | EPOLLHUP))
		return;

	len = read(hci->vhci_fd, buf, sizeof(buf));
	if (len < 1)
		return;

	switch (buf[0]) {
	case BT_H4_CMD_PKT:
		process_command(hci, buf + 1, len - 1);
		break;
	}
}

static void phy_recv_callback(uint16_t type, const void *data,
						size_t size, void *user_data)
{
	struct bt_le *hci = user_data;

	switch (type) {
	case BT_PHY_PKT_ADV:
		if (!(hci->le_event_mask[0] & 0x02))
			return;

		if (hci->scan_window_active) {
			const struct bt_phy_pkt_adv *pkt = data;
			uint8_t buf[100];
			struct bt_hci_evt_le_adv_report *evt = (void *) buf;
			uint8_t tx_addr_type, tx_addr[6];

			if (hci->scan_chan_idx != pkt->chan_idx)
				break;

			resolve_peer_addr(hci, pkt->tx_addr_type, pkt->tx_addr,
							&tx_addr_type, tx_addr);

			if (hci->le_scan_filter_policy == 0x01 ||
					hci->le_scan_filter_policy == 0x03) {
				if (!is_in_white_list(hci, tx_addr_type,
								tx_addr))
					break;
			}

			if (hci->le_scan_filter_dup) {
				if (!add_to_scan_cache(hci, tx_addr_type,
								tx_addr))
					break;
			}

			memset(buf, 0, sizeof(buf));
			evt->num_reports = 0x01;
			evt->event_type = pkt->pdu_type;
			evt->addr_type = tx_addr_type;
			memcpy(evt->addr, tx_addr, 6);
			evt->data_len = pkt->adv_data_len;
			memcpy(buf + sizeof(*evt), data + sizeof(*pkt),
							pkt->adv_data_len);

			le_meta_event(hci, BT_HCI_EVT_LE_ADV_REPORT, buf,
					sizeof(*evt) + pkt->adv_data_len + 1);

			if (hci->le_scan_type == 0x00)
				break;

			memset(buf, 0, sizeof(buf));
			evt->num_reports = 0x01;
			evt->event_type = 0x04;
			evt->addr_type = tx_addr_type;
			memcpy(evt->addr, tx_addr, 6);
			evt->data_len = pkt->scan_rsp_len;
			memcpy(buf + sizeof(*evt), data + sizeof(*pkt) +
							pkt->adv_data_len,
							pkt->scan_rsp_len);

			le_meta_event(hci, BT_HCI_EVT_LE_ADV_REPORT, buf,
					sizeof(*evt) + pkt->scan_rsp_len + 1);
		}
		break;
	}
}

struct bt_le *bt_le_new(void)
{
	unsigned char setup_cmd[2];
	struct bt_le *hci;

	hci = calloc(1, sizeof(*hci));
	if (!hci)
		return NULL;

	hci->adv_timeout_id = -1;
	hci->scan_timeout_id = -1;
	hci->scan_window_active = false;

	reset_defaults(hci);

	hci->vhci_fd = open("/dev/vhci", O_RDWR);
	if (hci->vhci_fd < 0) {
		free(hci);
		return NULL;
	}

	setup_cmd[0] = HCI_VENDOR_PKT;
	setup_cmd[1] = HCI_PRIMARY;

	if (write(hci->vhci_fd, setup_cmd, sizeof(setup_cmd)) < 0) {
		close(hci->vhci_fd);
		free(hci);
		return NULL;
	}

	mainloop_add_fd(hci->vhci_fd, EPOLLIN, vhci_read_callback, hci, NULL);

	hci->phy = bt_phy_new();
	hci->crypto = bt_crypto_new();

	bt_phy_register(hci->phy, phy_recv_callback, hci);

	return bt_le_ref(hci);
}

struct bt_le *bt_le_ref(struct bt_le *hci)
{
	if (!hci)
		return NULL;

	__sync_fetch_and_add(&hci->ref_count, 1);

	return hci;
}

void bt_le_unref(struct bt_le *hci)
{
	if (!hci)
		return;

	if (__sync_sub_and_fetch(&hci->ref_count, 1))
		return;

	stop_adv(hci);

	bt_crypto_unref(hci->crypto);
	bt_phy_unref(hci->phy);

	mainloop_remove_fd(hci->vhci_fd);

	close(hci->vhci_fd);

	free(hci);
}

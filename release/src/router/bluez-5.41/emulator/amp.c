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

#include "lib/bluetooth.h"
#include "lib/hci.h"

#include "src/shared/util.h"
#include "src/shared/mainloop.h"
#include "monitor/bt.h"

#include "amp.h"

#define PHY_MODE_IDLE		0x00
#define PHY_MODE_INITIATOR	0x01
#define PHY_MODE_ACCEPTOR	0x02

#define MAX_ASSOC_LEN	672

struct bt_amp {
	volatile int ref_count;
	int vhci_fd;

	char phylink_path[32];
	int phylink_fd;

	uint8_t  event_mask[16];
	uint16_t manufacturer;
	uint8_t  commands[64];
	uint8_t  features[8];

	uint8_t  amp_status;
	uint8_t  amp_type;
	uint8_t  local_assoc[MAX_ASSOC_LEN];
	uint16_t local_assoc_len;
	uint8_t  remote_assoc[MAX_ASSOC_LEN];
	uint16_t remote_assoc_len;

	uint8_t  phy_mode;
	uint8_t  phy_handle;
	uint16_t logic_handle;
};

static void reset_defaults(struct bt_amp *amp)
{
	memset(amp->event_mask, 0, sizeof(amp->event_mask));
	amp->event_mask[1] |= 0x20;	/* Command Complete */
	amp->event_mask[1] |= 0x40;	/* Command Status */
	amp->event_mask[1] |= 0x80;	/* Hardware Error */
	amp->event_mask[2] |= 0x01;	/* Flush Occurred */
	amp->event_mask[2] |= 0x04;	/* Number of Completed Packets */
	amp->event_mask[3] |= 0x02;	/* Data Buffer Overflow */
	amp->event_mask[3] |= 0x20;	/* QoS Violation */
	amp->event_mask[7] |= 0x01;	/* Enhanced Flush Complete */

	amp->event_mask[8] |= 0x01;	/* Physical Link Complete */
	amp->event_mask[8] |= 0x02;	/* Channel Selected */
	amp->event_mask[8] |= 0x04;	/* Disconnection Physical Link Complete */
	amp->event_mask[8] |= 0x08;	/* Physical Link Loss Early Warning */
	amp->event_mask[8] |= 0x10;	/* Physical Link Recovery */
	amp->event_mask[8] |= 0x20;	/* Logical Link Complete */
	amp->event_mask[8] |= 0x40;	/* Disconection Logical Link Complete */
	amp->event_mask[8] |= 0x80;	/* Flow Specification Modify Complete */
	amp->event_mask[9] |= 0x01;	/* Number of Completed Data Blocks */
	amp->event_mask[9] |= 0x02;	/* AMP Start Test */
	amp->event_mask[9] |= 0x04;	/* AMP Test End */
	amp->event_mask[9] |= 0x08;	/* AMP Receiver Report */
	amp->event_mask[9] |= 0x10;	/* Short Range Mode Change Complete */
	amp->event_mask[9] |= 0x20;	/* AMP Status Change */

	amp->manufacturer = 0x003f;	/* Bluetooth SIG (63) */

	memset(amp->commands, 0, sizeof(amp->commands));
	amp->commands[5]  |= 0x40;	/* Set Event Mask */
	amp->commands[5]  |= 0x80;	/* Reset */
	//amp->commands[6]  |= 0x01;	/* Set Event Filter */
	//amp->commands[7]  |= 0x04;	/* Read Connection Accept Timeout */
	//amp->commands[7]  |= 0x08;	/* Write Connection Accept Timeout */
	//amp->commands[10] |= 0x80;	/* Host Number of Completed Packets */
	//amp->commands[11] |= 0x01;	/* Read Link Supervision Timeout */
	//amp->commands[11] |= 0x02;	/* Write Link Supervision Timeout */
	amp->commands[14] |= 0x08;	/* Read Local Version Information */
	amp->commands[14] |= 0x10;	/* Read Local Supported Commands */
	amp->commands[14] |= 0x20;	/* Read Local Supported Features */
	amp->commands[14] |= 0x80;	/* Read Buffer Size */
	//amp->commands[15] |= 0x04;	/* Read Failed Contact Counter */
	//amp->commands[15] |= 0x08;	/* Reset Failed Contact Counter */
	//amp->commands[15] |= 0x10;	/* Read Link Quality */
	//amp->commands[15] |= 0x20;	/* Read RSSI */
	//amp->commands[16] |= 0x04;	/* Enable Device Under Test Mode */
	//amp->commands[19] |= 0x40;	/* Enhanced Flush */

	amp->commands[21] |= 0x01;	/* Create Physical Link */
	amp->commands[21] |= 0x02;	/* Accept Physical Link */
	amp->commands[21] |= 0x04;	/* Disconnect Phyiscal Link */
	amp->commands[21] |= 0x08;	/* Create Logical Link */
	amp->commands[21] |= 0x10;	/* Accept Logical Link */
	amp->commands[21] |= 0x20;	/* Disconnect Logical Link */
	amp->commands[21] |= 0x40;	/* Logical Link Cancel */
	//amp->commands[21] |= 0x80;	/* Flow Specification Modify */
	//amp->commands[22] |= 0x01;	/* Read Logical Link Accept Timeout */
	//amp->commands[22] |= 0x02;	/* Write Logical Link Accept Timeout */
	amp->commands[22] |= 0x04;	/* Set Event Mask Page 2 */
	amp->commands[22] |= 0x08;	/* Read Location Data */
	amp->commands[22] |= 0x10;	/* Write Location Data */
	amp->commands[22] |= 0x20;	/* Read Local AMP Info */
	amp->commands[22] |= 0x40;	/* Read Local AMP ASSOC */
	amp->commands[22] |= 0x80;	/* Write Remote AMP ASSOC */
	amp->commands[23] |= 0x01;	/* Read Flow Control Mode */
	amp->commands[23] |= 0x02;	/* Write Flow Control Mode */
	amp->commands[23] |= 0x04;	/* Read Data Block Size */
	//amp->commands[23] |= 0x20;	/* Enable AMP Receiver Reports */
	//amp->commands[23] |= 0x40;	/* AMP Test End */
	//amp->commands[23] |= 0x80;	/* AMP Test */
	//amp->commands[24] |= 0x04;	/* Read Best Effort Flush Timeout */
	//amp->commands[24] |= 0x08;	/* Write Best Effort Flush Timeout */
	//amp->commands[24] |= 0x10;	/* Short Range Mode */

	memset(amp->features, 0, sizeof(amp->features));

	amp->amp_status = 0x01;		/* Used for Bluetooth only */
	amp->amp_type = 0x42;		/* Fake virtual AMP type */

	memset(amp->local_assoc, 0, sizeof(amp->local_assoc));
	amp->local_assoc_len = 0;

	memset(amp->remote_assoc, 0, sizeof(amp->remote_assoc));
	amp->remote_assoc_len = 0;

	amp->phy_mode = PHY_MODE_IDLE;
	amp->phy_handle = 0x00;		/* Invalid physical link handle */
	amp->logic_handle = 0x0000;
}

static void send_packet(struct bt_amp *amp, const void *data, uint16_t len)
{
	if (write(amp->vhci_fd, data, len) < 0)
		fprintf(stderr, "Write to /dev/vhci failed\n");
}

static void send_event(struct bt_amp *amp, uint8_t event,
						const void *data, uint8_t len)
{
	struct bt_hci_evt_hdr *hdr;
	uint16_t pkt_len;
	void *pkt_data;

	pkt_len = 1 + sizeof(*hdr) + len;

	pkt_data = alloca(pkt_len);
	if (!pkt_data)
		return;

	((uint8_t *) pkt_data)[0] = BT_H4_EVT_PKT;

	hdr = pkt_data + 1;
	hdr->evt = event;
	hdr->plen = len;

	if (len > 0)
		memcpy(pkt_data + 1 + sizeof(*hdr), data, len);

	send_packet(amp, pkt_data, pkt_len);
}

static void cmd_complete(struct bt_amp *amp, uint16_t opcode,
						const void *data, uint8_t len)
{
	struct bt_hci_evt_hdr *hdr;
	struct bt_hci_evt_cmd_complete *cc;
	uint16_t pkt_len;
	void *pkt_data;

	pkt_len = 1 + sizeof(*hdr) + sizeof(*cc) + len;

	pkt_data = alloca(pkt_len);
	if (!pkt_data)
		return;

	((uint8_t *) pkt_data)[0] = BT_H4_EVT_PKT;

	hdr = pkt_data + 1;
	hdr->evt = BT_HCI_EVT_CMD_COMPLETE;
	hdr->plen = sizeof(*cc) + len;

	cc = pkt_data + 1 + sizeof(*hdr);
	cc->ncmd = 0x01;
	cc->opcode = cpu_to_le16(opcode);

	if (len > 0)
		memcpy(pkt_data + 1 + sizeof(*hdr) + sizeof(*cc), data, len);

	send_packet(amp, pkt_data, pkt_len);
}

static void cmd_status(struct bt_amp *amp, uint8_t status, uint16_t opcode)
{
	struct bt_hci_evt_hdr *hdr;
	struct bt_hci_evt_cmd_status *cs;
	uint16_t pkt_len;
	void *pkt_data;

	pkt_len = 1 + sizeof(*hdr) + sizeof(*cs);

	pkt_data = alloca(pkt_len);
	if (!pkt_data)
		return;

	((uint8_t *) pkt_data)[0] = BT_H4_EVT_PKT;

	hdr = pkt_data + 1;
	hdr->evt = BT_HCI_EVT_CMD_STATUS;
	hdr->plen = sizeof(*cs);

	cs = pkt_data + 1 + sizeof(*hdr);
	cs->status = status;
	cs->ncmd = 0x01;
	cs->opcode = cpu_to_le16(opcode);

	send_packet(amp, pkt_data, pkt_len);
}

static void cmd_set_event_mask(struct bt_amp *amp,
						const void *data, uint8_t size)
{
	const struct bt_hci_cmd_set_event_mask *cmd = data;
	uint8_t status;

	memcpy(amp->event_mask, cmd->mask, 8);

	status = BT_HCI_ERR_SUCCESS;
	cmd_complete(amp, BT_HCI_CMD_SET_EVENT_MASK, &status, sizeof(status));
}

static void cmd_reset(struct bt_amp *amp, const void *data, uint8_t size)
{
	uint8_t status;

	reset_defaults(amp);

	amp->local_assoc[0] = 0x00;
	amp->local_assoc_len = 1;

	status = BT_HCI_ERR_SUCCESS;
	cmd_complete(amp, BT_HCI_CMD_RESET, &status, sizeof(status));
}

static void cmd_read_local_version(struct bt_amp *amp,
						const void *data, uint8_t size)
{
	struct bt_hci_rsp_read_local_version rsp;

	rsp.status = BT_HCI_ERR_SUCCESS;
	rsp.hci_ver = 0x05;
	rsp.hci_rev = cpu_to_le16(0x0000);
	rsp.lmp_ver = 0x01;
	rsp.manufacturer = cpu_to_le16(amp->manufacturer);
	rsp.lmp_subver = cpu_to_le16(0x0000);

	cmd_complete(amp, BT_HCI_CMD_READ_LOCAL_VERSION, &rsp, sizeof(rsp));
}

static void cmd_read_local_commands(struct bt_amp *amp,
						const void *data, uint8_t size)
{
	struct bt_hci_rsp_read_local_commands rsp;

	rsp.status = BT_HCI_ERR_SUCCESS;
	memcpy(rsp.commands, amp->commands, 64);

	cmd_complete(amp, BT_HCI_CMD_READ_LOCAL_COMMANDS, &rsp, sizeof(rsp));
}

static void cmd_read_local_features(struct bt_amp *amp,
						const void *data, uint8_t size)
{
	struct bt_hci_rsp_read_local_features rsp;

	rsp.status = BT_HCI_ERR_SUCCESS;
	memcpy(rsp.features, amp->features, 8);

	cmd_complete(amp, BT_HCI_CMD_READ_LOCAL_FEATURES, &rsp, sizeof(rsp));
}

static void cmd_read_buffer_size(struct bt_amp *amp,
						const void *data, uint8_t size)
{
	struct bt_hci_rsp_read_buffer_size rsp;

	rsp.status = BT_HCI_ERR_SUCCESS;
	rsp.acl_mtu = cpu_to_le16(0x0000);
	rsp.sco_mtu = 0x00;
	rsp.acl_max_pkt = cpu_to_le16(0x0000);
	rsp.sco_max_pkt = cpu_to_le16(0x0000);

	cmd_complete(amp, BT_HCI_CMD_READ_BUFFER_SIZE, &rsp, sizeof(rsp));
}

static void evt_phy_link_complete(struct bt_amp *amp)
{
	struct bt_hci_evt_phy_link_complete evt;

	evt.status = BT_HCI_ERR_SUCCESS;
	evt.phy_handle = amp->phy_handle;

	send_event(amp, BT_HCI_EVT_PHY_LINK_COMPLETE, &evt, sizeof(evt));
}

static void evt_disconn_phy_link_complete(struct bt_amp *amp, uint8_t reason)
{
	struct bt_hci_evt_disconn_phy_link_complete evt;

	evt.status = BT_HCI_ERR_SUCCESS;
	evt.phy_handle = amp->phy_handle;
	evt.reason = reason;

	send_event(amp, BT_HCI_EVT_DISCONN_PHY_LINK_COMPLETE,
						&evt, sizeof(evt));
}

static void link_callback(int fd, uint32_t events, void *user_data)
{
	struct bt_amp *amp = user_data;

	if (events & (EPOLLERR | EPOLLHUP)) {
		close(fd);
		mainloop_remove_fd(fd);

		evt_disconn_phy_link_complete(amp, 0x13);

		amp->phy_mode = PHY_MODE_IDLE;
		amp->phy_handle = 0x00;
		return;
	}
}

static void cmd_create_phy_link(struct bt_amp *amp,
						const void *data, uint8_t size)
{
	const struct bt_hci_cmd_create_phy_link *cmd = data;

	if (cmd->phy_handle == 0x00) {
		cmd_status(amp, BT_HCI_ERR_INVALID_PARAMETERS,
					BT_HCI_CMD_CREATE_PHY_LINK);
		return;
	}

	if (amp->phy_mode != PHY_MODE_IDLE) {
		cmd_status(amp, BT_HCI_ERR_COMMAND_DISALLOWED,
					BT_HCI_CMD_CREATE_PHY_LINK);
		return;
	}

	amp->phy_mode = PHY_MODE_INITIATOR;
	amp->phy_handle = cmd->phy_handle;

	cmd_status(amp, BT_HCI_ERR_SUCCESS, BT_HCI_CMD_CREATE_PHY_LINK);
}

static void cmd_accept_phy_link(struct bt_amp *amp,
						const void *data, uint8_t size)
{
	const struct bt_hci_cmd_accept_phy_link *cmd = data;

	if (cmd->phy_handle == 0x00) {
		cmd_status(amp, BT_HCI_ERR_INVALID_PARAMETERS,
					BT_HCI_CMD_ACCEPT_PHY_LINK);
		return;
	}

	if (amp->phy_mode != PHY_MODE_IDLE) {
		cmd_status(amp, BT_HCI_ERR_COMMAND_DISALLOWED,
					BT_HCI_CMD_ACCEPT_PHY_LINK);
		return;
	}

	amp->phy_mode = PHY_MODE_ACCEPTOR;
	amp->phy_handle = cmd->phy_handle;

	cmd_status(amp, BT_HCI_ERR_SUCCESS, BT_HCI_CMD_ACCEPT_PHY_LINK);
}

static void cmd_disconn_phy_link(struct bt_amp *amp,
						const void *data, uint8_t size)
{
	const struct bt_hci_cmd_disconn_phy_link *cmd = data;

	if (cmd->phy_handle == 0x00) {
		cmd_status(amp, BT_HCI_ERR_INVALID_PARAMETERS,
					BT_HCI_CMD_DISCONN_PHY_LINK);
		return;
	}

	if (amp->phy_mode == PHY_MODE_IDLE) {
		cmd_status(amp, BT_HCI_ERR_COMMAND_DISALLOWED,
					BT_HCI_CMD_DISCONN_PHY_LINK);
		return;
	}

	if (cmd->phy_handle != amp->phy_handle) {
		cmd_status(amp, BT_HCI_ERR_INVALID_PARAMETERS,
					BT_HCI_CMD_DISCONN_PHY_LINK);
		return;
	}

	cmd_status(amp, BT_HCI_ERR_SUCCESS, BT_HCI_CMD_DISCONN_PHY_LINK);

	mainloop_remove_fd(amp->phylink_fd);
	close(amp->phylink_fd);

	evt_disconn_phy_link_complete(amp, cmd->reason);

	amp->phy_mode = PHY_MODE_IDLE;
	amp->phy_handle = 0x00;
}

static void evt_logic_link_complete(struct bt_amp *amp)
{
	struct bt_hci_evt_logic_link_complete evt;

	evt.status = BT_HCI_ERR_SUCCESS;
	evt.handle = htobs(amp->logic_handle);
	evt.phy_handle = amp->phy_handle;
	evt.flow_spec = 0x00;

	send_event(amp, BT_HCI_EVT_LOGIC_LINK_COMPLETE, &evt, sizeof(evt));
}

static void evt_disconn_logic_link_complete(struct bt_amp *amp, uint8_t reason)
{
	struct bt_hci_evt_disconn_logic_link_complete evt;

	evt.status = BT_HCI_ERR_SUCCESS;
	evt.handle = htobs(amp->logic_handle);
	evt.reason = reason;

	send_event(amp, BT_HCI_EVT_DISCONN_LOGIC_LINK_COMPLETE,
						&evt, sizeof(evt));
}

static void cmd_create_logic_link(struct bt_amp *amp,
						const void *data, uint8_t size)
{
	const struct bt_hci_cmd_create_logic_link *cmd = data;

	if (cmd->phy_handle == 0x00) {
		cmd_status(amp, BT_HCI_ERR_INVALID_PARAMETERS,
					BT_HCI_CMD_CREATE_LOGIC_LINK);
		return;
	}

	if (amp->phy_mode != PHY_MODE_IDLE) {
		cmd_status(amp, BT_HCI_ERR_COMMAND_DISALLOWED,
					BT_HCI_CMD_CREATE_LOGIC_LINK);
		return;
	}

	if (amp->logic_handle != 0x00) {
		cmd_status(amp, BT_HCI_ERR_COMMAND_DISALLOWED,
					BT_HCI_CMD_CREATE_LOGIC_LINK);
		return;
	}

	cmd_status(amp, BT_HCI_ERR_SUCCESS, BT_HCI_CMD_CREATE_LOGIC_LINK);

	amp->logic_handle = 0x0042;

	evt_logic_link_complete(amp);
}

static void cmd_accept_logic_link(struct bt_amp *amp,
						const void *data, uint8_t size)
{
	const struct bt_hci_cmd_accept_logic_link *cmd = data;

	if (cmd->phy_handle == 0x00) {
		cmd_status(amp, BT_HCI_ERR_INVALID_PARAMETERS,
					BT_HCI_CMD_ACCEPT_LOGIC_LINK);
		return;
	}

	if (amp->phy_mode != PHY_MODE_IDLE) {
		cmd_status(amp, BT_HCI_ERR_COMMAND_DISALLOWED,
					BT_HCI_CMD_ACCEPT_LOGIC_LINK);
		return;
	}

	if (amp->logic_handle != 0x00) {
		cmd_status(amp, BT_HCI_ERR_COMMAND_DISALLOWED,
					BT_HCI_CMD_ACCEPT_LOGIC_LINK);
		return;
	}

	cmd_status(amp, BT_HCI_ERR_SUCCESS, BT_HCI_CMD_ACCEPT_LOGIC_LINK);

	amp->logic_handle = 0x0023;

	evt_logic_link_complete(amp);
}

static void cmd_disconn_logic_link(struct bt_amp *amp,
						const void *data, uint8_t size)
{
	const struct bt_hci_cmd_disconn_logic_link *cmd = data;

	if (cmd->handle == 0x00) {
		cmd_status(amp, BT_HCI_ERR_INVALID_PARAMETERS,
					BT_HCI_CMD_DISCONN_LOGIC_LINK);
		return;
	}

	if (cmd->handle != amp->logic_handle) {
		cmd_status(amp, BT_HCI_ERR_INVALID_PARAMETERS,
					BT_HCI_CMD_DISCONN_LOGIC_LINK);
		return;
	}

	cmd_status(amp, BT_HCI_ERR_SUCCESS, BT_HCI_CMD_DISCONN_LOGIC_LINK);

	evt_disconn_logic_link_complete(amp, 0x13);

	amp->logic_handle = 0x0000;
}

static void cmd_logic_link_cancel(struct bt_amp *amp,
						const void *data, uint8_t size)
{
	const struct bt_hci_cmd_logic_link_cancel *cmd = data;
	struct bt_hci_rsp_logic_link_cancel rsp;

	if (cmd->phy_handle == 0x00) {
		cmd_status(amp, BT_HCI_ERR_INVALID_PARAMETERS,
					BT_HCI_CMD_LOGIC_LINK_CANCEL);
		return;
	}

	if (amp->phy_mode != PHY_MODE_IDLE) {
		cmd_status(amp, BT_HCI_ERR_COMMAND_DISALLOWED,
					BT_HCI_CMD_LOGIC_LINK_CANCEL);
		return;
	}

	amp->logic_handle = 0x0000;

	rsp.status = BT_HCI_ERR_SUCCESS;
	rsp.phy_handle = amp->phy_handle;
	rsp.flow_spec = 0x00;

	cmd_complete(amp, BT_HCI_CMD_LOGIC_LINK_CANCEL, &rsp, sizeof(rsp));
}

static void cmd_set_event_mask_page2(struct bt_amp *amp,
						const void *data, uint8_t size)
{
	const struct bt_hci_cmd_set_event_mask_page2 *cmd = data;
	uint8_t status;

	memcpy(amp->event_mask + 8, cmd->mask, 8);

	status = BT_HCI_ERR_SUCCESS;
	cmd_complete(amp, BT_HCI_CMD_SET_EVENT_MASK_PAGE2,
						&status, sizeof(status));
}

static void cmd_read_location_data(struct bt_amp *amp,
						const void *data, uint8_t size)
{
	struct bt_hci_rsp_read_location_data rsp;

	rsp.status = BT_HCI_ERR_SUCCESS;
	rsp.domain_aware = 0x00;
	rsp.domain[0] = 0x58;
	rsp.domain[1] = 0x58;
	rsp.domain_options = 0x58;
	rsp.options = 0x00;

	cmd_complete(amp, BT_HCI_CMD_READ_LOCATION_DATA, &rsp, sizeof(rsp));
}

static void cmd_write_location_data(struct bt_amp *amp,
						const void *data, uint8_t size)
{
	const struct bt_hci_cmd_write_location_data *cmd = data;
	uint8_t status;

	if (cmd->domain_aware > 0x01) {
		cmd_status(amp, BT_HCI_ERR_INVALID_PARAMETERS,
					BT_HCI_CMD_WRITE_LOCATION_DATA);
		return;
	}

	status = BT_HCI_ERR_SUCCESS;
	cmd_complete(amp, BT_HCI_CMD_WRITE_LOCATION_DATA,
						&status, sizeof(status));
}

static void cmd_read_flow_control_mode(struct bt_amp *amp,
						const void *data, uint8_t size)
{
	struct bt_hci_rsp_read_flow_control_mode rsp;

	rsp.status = BT_HCI_ERR_SUCCESS;
	rsp.mode = 0x01;

	cmd_complete(amp, BT_HCI_CMD_READ_FLOW_CONTROL_MODE,
						&rsp, sizeof(rsp));
}

static void cmd_write_flow_control_mode(struct bt_amp *amp,
						const void *data, uint8_t size)
{
	const struct bt_hci_cmd_write_flow_control_mode *cmd = data;
	uint8_t status;

	if (cmd->mode != 0x01) {
		cmd_status(amp, BT_HCI_ERR_INVALID_PARAMETERS,
					BT_HCI_CMD_WRITE_FLOW_CONTROL_MODE);
		return;
	}

	status = BT_HCI_ERR_SUCCESS;
	cmd_complete(amp, BT_HCI_CMD_WRITE_FLOW_CONTROL_MODE,
						&status, sizeof(status));
}

static void cmd_read_data_block_size(struct bt_amp *amp,
						const void *data, uint8_t size)
{
	struct bt_hci_rsp_read_data_block_size rsp;

	rsp.status = BT_HCI_ERR_SUCCESS;
	rsp.max_acl_len = cpu_to_le16(1492);
	rsp.block_len = cpu_to_le16(1492);
	rsp.num_blocks = cpu_to_le16(1);

	cmd_complete(amp, BT_HCI_CMD_READ_DATA_BLOCK_SIZE, &rsp, sizeof(rsp));
}

static void cmd_read_local_amp_info(struct bt_amp *amp,
						const void *data, uint8_t size)
{
	struct bt_hci_rsp_read_local_amp_info rsp;

	rsp.status = BT_HCI_ERR_SUCCESS;
	rsp.amp_status = amp->amp_status;
	rsp.total_bw = cpu_to_le32(24000);
	rsp.max_bw = cpu_to_le32(24000);
	rsp.min_latency = cpu_to_le32(100);
	rsp.max_pdu = cpu_to_le32(1492);
	rsp.amp_type = amp->amp_type;
	rsp.pal_cap = cpu_to_le16(0x0001);
	rsp.max_assoc_len = cpu_to_le16(MAX_ASSOC_LEN);
	rsp.max_flush_to = cpu_to_le32(20000);
	rsp.be_flush_to = cpu_to_le32(20000);

	cmd_complete(amp, BT_HCI_CMD_READ_LOCAL_AMP_INFO, &rsp, sizeof(rsp));
}

static void cmd_read_local_amp_assoc(struct bt_amp *amp,
						const void *data, uint8_t size)
{
	const struct bt_hci_cmd_read_local_amp_assoc *cmd = data;
	struct bt_hci_rsp_read_local_amp_assoc rsp;
	uint16_t len_so_far, remain_assoc_len, fragment_len;

	if (cmd->phy_handle != amp->phy_handle) {
		cmd_status(amp, BT_HCI_ERR_INVALID_PARAMETERS,
					BT_HCI_CMD_READ_LOCAL_AMP_ASSOC);
		return;
	}

	len_so_far = le16_to_cpu(cmd->len_so_far);
	remain_assoc_len = amp->local_assoc_len - len_so_far;
	fragment_len = remain_assoc_len > 248 ? 248 : remain_assoc_len;

	rsp.status = BT_HCI_ERR_SUCCESS;
	rsp.phy_handle = cmd->phy_handle;
	rsp.remain_assoc_len = cpu_to_le16(remain_assoc_len);
	memcpy(rsp.assoc_fragment, amp->local_assoc + len_so_far,
							fragment_len);

	cmd_complete(amp, BT_HCI_CMD_READ_LOCAL_AMP_ASSOC,
						&rsp, 4 + fragment_len);
}

static int create_unix_server(const char *path)
{
	struct sockaddr_un addr;
	int fd;

	fd = socket(PF_UNIX, SOCK_SEQPACKET, 0);
	if (fd < 0)
		return -1;

	memset(&addr, 0, sizeof(addr));
	addr.sun_family = AF_UNIX;
	addr.sun_path[0] = '\0';
	strcpy(addr.sun_path + 1, path);

	if (bind(fd, (struct sockaddr *) &addr, sizeof(addr)) < 0) {
		close(fd);
		return -1;
	}

	if (listen(fd, 1) < 0) {
		close(fd);
		return -1;
	}

	return fd;
}

static int connect_unix_client(const char *path)
{
	struct sockaddr_un addr;
	int fd;

	fd = socket(PF_UNIX, SOCK_SEQPACKET | SOCK_CLOEXEC | SOCK_NONBLOCK, 0);
	if (fd < 0)
		return -1;

	memset(&addr, 0, sizeof(addr));
	addr.sun_family = AF_UNIX;
	addr.sun_path[0] = '\0';
	strcpy(addr.sun_path + 1, path);

	if (connect(fd, (struct sockaddr *) &addr, sizeof(addr)) < 0) {
		close(fd);
		return -1;
	}

	return fd;
}

static void accept_callback(int fd, uint32_t events, void *user_data)
{
	struct bt_amp *amp = user_data;
	struct sockaddr_un addr;
	socklen_t len;
	int new_fd;

	if (events & (EPOLLERR | EPOLLHUP)) {
		mainloop_remove_fd(fd);
		return;
	}

	memset(&addr, 0, sizeof(addr));
	len = sizeof(addr);

	new_fd = accept4(fd, (struct sockaddr *) &addr, &len,
						SOCK_CLOEXEC | SOCK_NONBLOCK);
	if (new_fd < 0)
		return;

	mainloop_remove_fd(fd);
	close(fd);

	amp->phylink_fd = new_fd;

	evt_phy_link_complete(amp);

	mainloop_add_fd(new_fd, EPOLLIN, link_callback, amp, NULL);
}

static void connect_callback(int fd, uint32_t events, void *user_data)
{
	struct bt_amp *amp = user_data;

	if (events & (EPOLLERR | EPOLLHUP)) {
		mainloop_remove_fd(fd);
		return;
	}

	mainloop_remove_fd(fd);

	evt_phy_link_complete(amp);

	mainloop_add_fd(fd, EPOLLIN, link_callback, amp, NULL);
}

static void cmd_write_remote_amp_assoc(struct bt_amp *amp,
						const void *data, uint8_t size)
{
	const struct bt_hci_cmd_write_remote_amp_assoc *cmd = data;
	struct bt_hci_rsp_write_remote_amp_assoc rsp;
	int fd;

	if (cmd->phy_handle == 0x00) {
		cmd_status(amp, BT_HCI_ERR_INVALID_PARAMETERS,
					BT_HCI_CMD_WRITE_REMOTE_AMP_ASSOC);
		return;
	}

	if (cmd->phy_handle != amp->phy_handle) {
		cmd_status(amp, BT_HCI_ERR_INVALID_PARAMETERS,
					BT_HCI_CMD_WRITE_REMOTE_AMP_ASSOC);
		return;
	}

	switch (amp->phy_mode) {
	case PHY_MODE_INITIATOR:
		strcpy(amp->phylink_path, "amp");

		fd = create_unix_server(amp->phylink_path);
		if (fd < 0) {
			cmd_status(amp, BT_HCI_ERR_UNSPECIFIED_ERROR,
					BT_HCI_CMD_WRITE_REMOTE_AMP_ASSOC);
			return;
		}

		amp->local_assoc[0] = 0x01;
		memcpy(amp->local_assoc + 1, amp->phylink_path,
					strlen(amp->phylink_path) + 1);
		amp->local_assoc_len = strlen(amp->phylink_path) + 2;

		mainloop_add_fd(fd, EPOLLIN, accept_callback, amp, NULL);

		amp->phylink_fd = fd;
		break;

	case PHY_MODE_ACCEPTOR:
		if (cmd->assoc_fragment[0] != 0x01) {
			cmd_status(amp, BT_HCI_ERR_UNSPECIFIED_ERROR,
					BT_HCI_CMD_WRITE_REMOTE_AMP_ASSOC);
			return;
		}

		memcpy(amp->phylink_path, cmd->assoc_fragment + 1,
						cmd->remain_assoc_len - 1);

		fd = connect_unix_client(amp->phylink_path);
		if (fd < 0) {
			cmd_status(amp, BT_HCI_ERR_UNSPECIFIED_ERROR,
					BT_HCI_CMD_WRITE_REMOTE_AMP_ASSOC);
			return;
		}

		mainloop_add_fd(fd, EPOLLOUT, connect_callback, amp, NULL);

		amp->phylink_fd = fd;
		break;

	default:
		cmd_status(amp, BT_HCI_ERR_COMMAND_DISALLOWED,
					BT_HCI_CMD_WRITE_REMOTE_AMP_ASSOC);
		return;
	}

	rsp.status = BT_HCI_ERR_SUCCESS;
	rsp.phy_handle = amp->phy_handle;

	cmd_complete(amp, BT_HCI_CMD_WRITE_REMOTE_AMP_ASSOC, &rsp, sizeof(rsp));

	if (amp->phy_mode == PHY_MODE_INITIATOR) {
		struct bt_hci_evt_channel_selected evt;

		evt.phy_handle = amp->phy_handle;

		send_event(amp, BT_HCI_EVT_CHANNEL_SELECTED, &evt, sizeof(evt));
	}
}

static const struct {
	uint16_t opcode;
	void (*func) (struct bt_amp *amp, const void *data, uint8_t size);
	uint8_t size;
	bool fixed;
} cmd_table[] = {
	{ BT_HCI_CMD_SET_EVENT_MASK,       cmd_set_event_mask,      8, true },
	{ BT_HCI_CMD_RESET,                cmd_reset,               0, true },
	{ BT_HCI_CMD_READ_LOCAL_VERSION,   cmd_read_local_version,  0, true },
	{ BT_HCI_CMD_READ_LOCAL_COMMANDS,  cmd_read_local_commands, 0, true },
	{ BT_HCI_CMD_READ_LOCAL_FEATURES,  cmd_read_local_features, 0, true },
	{ BT_HCI_CMD_READ_BUFFER_SIZE,     cmd_read_buffer_size,    0, true },

	{ BT_HCI_CMD_CREATE_PHY_LINK,
				cmd_create_phy_link, 3, false },
	{ BT_HCI_CMD_ACCEPT_PHY_LINK,
				cmd_accept_phy_link, 3, false },
	{ BT_HCI_CMD_DISCONN_PHY_LINK,
				cmd_disconn_phy_link, 2, true },
	{ BT_HCI_CMD_CREATE_LOGIC_LINK,
				cmd_create_logic_link, 33, true },
	{ BT_HCI_CMD_ACCEPT_LOGIC_LINK,
				cmd_accept_logic_link, 33, true },
	{ BT_HCI_CMD_DISCONN_LOGIC_LINK,
				cmd_disconn_logic_link, 2, true },
	{ BT_HCI_CMD_LOGIC_LINK_CANCEL,
				cmd_logic_link_cancel, 2, true },
	{ BT_HCI_CMD_SET_EVENT_MASK_PAGE2,
				cmd_set_event_mask_page2, 8, true },
	{ BT_HCI_CMD_READ_LOCATION_DATA,
				cmd_read_location_data, 0, true },
	{ BT_HCI_CMD_WRITE_LOCATION_DATA,
				cmd_write_location_data, 5, true },
	{ BT_HCI_CMD_READ_FLOW_CONTROL_MODE,
				cmd_read_flow_control_mode, 0, true },
	{ BT_HCI_CMD_WRITE_FLOW_CONTROL_MODE,
				cmd_write_flow_control_mode, 1, true },
	{ BT_HCI_CMD_READ_DATA_BLOCK_SIZE,
				cmd_read_data_block_size, 0, true },
	{ BT_HCI_CMD_READ_LOCAL_AMP_INFO,
				cmd_read_local_amp_info, 0, true },
	{ BT_HCI_CMD_READ_LOCAL_AMP_ASSOC,
				cmd_read_local_amp_assoc, 5, true },
	{ BT_HCI_CMD_WRITE_REMOTE_AMP_ASSOC,
				cmd_write_remote_amp_assoc, 6, false },
	{ }
};

static void process_command(struct bt_amp *amp, const void *data, size_t size)
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
		cmd_status(amp, BT_HCI_ERR_INVALID_PARAMETERS, opcode);
		return;
	}

	for (i = 0; cmd_table[i].func; i++) {
		if (cmd_table[i].opcode != opcode)
			continue;

		if ((cmd_table[i].fixed && size != cmd_table[i].size) ||
						size < cmd_table[i].size) {
			cmd_status(amp, BT_HCI_ERR_INVALID_PARAMETERS, opcode);
			return;
		}

		cmd_table[i].func(amp, data, size);
		return;
	}

	cmd_status(amp, BT_HCI_ERR_UNKNOWN_COMMAND, opcode);
}

static void vhci_read_callback(int fd, uint32_t events, void *user_data)
{
	struct bt_amp *amp = user_data;
	unsigned char buf[4096];
	ssize_t len;

	if (events & (EPOLLERR | EPOLLHUP))
		return;

	len = read(amp->vhci_fd, buf, sizeof(buf));
	if (len < 1)
		return;

	switch (buf[0]) {
	case BT_H4_CMD_PKT:
		process_command(amp, buf + 1, len - 1);
		break;
	}
}

struct bt_amp *bt_amp_new(void)
{
	unsigned char setup_cmd[2];
	struct bt_amp *amp;

	amp = calloc(1, sizeof(*amp));
	if (!amp)
		return NULL;

	reset_defaults(amp);

	amp->vhci_fd = open("/dev/vhci", O_RDWR);
	if (amp->vhci_fd < 0) {
		free(amp);
		return NULL;
	}

	setup_cmd[0] = HCI_VENDOR_PKT;
	setup_cmd[1] = HCI_AMP;

	if (write(amp->vhci_fd, setup_cmd, sizeof(setup_cmd)) < 0) {
		close(amp->vhci_fd);
		free(amp);
		return NULL;
	}

	mainloop_add_fd(amp->vhci_fd, EPOLLIN, vhci_read_callback, amp, NULL);

	return bt_amp_ref(amp);
}

struct bt_amp *bt_amp_ref(struct bt_amp *amp)
{
	if (!amp)
		return NULL;

	__sync_fetch_and_add(&amp->ref_count, 1);

	return amp;
}

void bt_amp_unref(struct bt_amp *amp)
{
	if (!amp)
		return;

	if (__sync_sub_and_fetch(&amp->ref_count, 1))
		return;

	mainloop_remove_fd(amp->vhci_fd);

	close(amp->vhci_fd);

	free(amp);
}

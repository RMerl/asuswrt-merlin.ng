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

#include <stdio.h>
#include <ctype.h>
#include <stdlib.h>
#include <string.h>
#include <alloca.h>
#include <sys/uio.h>
#include <stdint.h>

#include "lib/bluetooth.h"
#include "lib/hci.h"

#include "src/shared/util.h"
#include "src/shared/timeout.h"
#include "src/shared/crypto.h"
#include "src/shared/ecc.h"
#include "monitor/bt.h"
#include "btdev.h"

#define has_bredr(btdev)	(!((btdev)->features[4] & 0x20))
#define has_le(btdev)		(!!((btdev)->features[4] & 0x40))

struct hook {
	btdev_hook_func handler;
	void *user_data;
	enum btdev_hook_type type;
	uint16_t opcode;
};

#define MAX_HOOK_ENTRIES 16

struct btdev {
	enum btdev_type type;

	struct btdev *conn;

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
	uint8_t  country_code;
	uint8_t  bdaddr[6];
	uint8_t  random_addr[6];
	uint8_t  le_features[8];
	uint8_t  le_states[8];

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

	uint8_t le_local_sk256[32];

	uint16_t sync_train_interval;
	uint32_t sync_train_timeout;
	uint8_t  sync_train_service_data;
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

static void hexdump(const unsigned char *buf, uint16_t len)
{
	static const char hexdigits[] = "0123456789abcdef";
	char str[68];
	uint16_t i;

	if (!len)
		return;

	for (i = 0; i < len; i++) {
		str[((i % 16) * 3) + 0] = hexdigits[buf[i] >> 4];
		str[((i % 16) * 3) + 1] = hexdigits[buf[i] & 0xf];
		str[((i % 16) * 3) + 2] = ' ';
		str[(i % 16) + 49] = isprint(buf[i]) ? buf[i] : '.';

		if ((i + 1) % 16 == 0) {
			str[47] = ' ';
			str[48] = ' ';
			str[65] = '\0';
			printf("%-12c%s\n", ' ', str);
			str[0] = ' ';
		}
	}

	if (i % 16 > 0) {
		uint16_t j;
		for (j = (i % 16); j < 16; j++) {
			str[(j * 3) + 0] = ' ';
			str[(j * 3) + 1] = ' ';
			str[(j * 3) + 2] = ' ';
			str[j + 49] = ' ';
		}
		str[47] = ' ';
		str[48] = ' ';
		str[65] = '\0';
		printf("%-12c%s\n", ' ', str);
	}
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

static void set_common_commands_all(struct btdev *btdev)
{
	btdev->commands[5]  |= 0x40;	/* Set Event Mask */
	btdev->commands[5]  |= 0x80;	/* Reset */
	btdev->commands[14] |= 0x08;	/* Read Local Version */
	btdev->commands[14] |= 0x10;	/* Read Local Supported Commands */
	btdev->commands[14] |= 0x20;	/* Read Local Supported Features */
	btdev->commands[14] |= 0x80;	/* Read Buffer Size */
}

static void set_common_commands_bredrle(struct btdev *btdev)
{
	btdev->commands[0]  |= 0x20;	/* Disconnect */
	btdev->commands[2]  |= 0x80;	/* Read Remote Version Information */
	btdev->commands[10] |= 0x20;    /* Set Host Flow Control */
	btdev->commands[10] |= 0x40;	/* Host Buffer Size */
	btdev->commands[15] |= 0x02;	/* Read BD ADDR */
}

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
	btdev->commands[20] |= 0x10;	/* Read Encryption Key Size */
	btdev->commands[23] |= 0x04;	/* Read Data Block Size */
	btdev->commands[29] |= 0x20;	/* Read Local Supported Codecs */
	btdev->commands[30] |= 0x08;	/* Get MWS Transport Layer Config */
}

static void set_bredr20_commands(struct btdev *btdev)
{
	set_common_commands_all(btdev);
	set_common_commands_bredrle(btdev);
	set_common_commands_bredr20(btdev);
}

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
}

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

	if (type == BTDEV_TYPE_BREDRLE || type == BTDEV_TYPE_LE) {
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
		btdev->version = 0x08;
		set_bredrle_features(btdev);
		set_bredrle_commands(btdev);
		break;
	case BTDEV_TYPE_BREDR:
		btdev->version = 0x05;
		set_bredr_features(btdev);
		set_bredr_commands(btdev);
		break;
	case BTDEV_TYPE_LE:
		btdev->version = 0x08;
		set_le_features(btdev);
		set_le_commands(btdev);
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

	btdev->country_code = 0x00;

	index = add_btdev(btdev);
	if (index < 0) {
		bt_crypto_unref(btdev->crypto);
		free(btdev);
		return NULL;
	}

	get_bdaddr(id, index, btdev->bdaddr);

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

static bool use_ssp(struct btdev *btdev1, struct btdev *btdev2)
{
	if (btdev1->auth_enable || btdev2->auth_enable)
		return false;

	return (btdev1->simple_pairing_mode && btdev2->simple_pairing_mode);
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

static void send_packet(struct btdev *btdev, const struct iovec *iov,
								int iovlen)
{
	if (!btdev->send_handler)
		return;

	btdev->send_handler(iov, iovlen, btdev->send_data);
}

static void send_event(struct btdev *btdev, uint8_t event,
						const void *data, uint8_t len)
{
	struct bt_hci_evt_hdr hdr;
	struct iovec iov[3];
	uint8_t pkt = BT_H4_EVT_PKT;

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

static void send_cmd(struct btdev *btdev, uint8_t evt, uint16_t opcode,
					const struct iovec *iov, int iovlen)
{
	struct bt_hci_evt_hdr hdr;
	struct iovec *iov2;
	uint8_t pkt = BT_H4_EVT_PKT;
	int i;

	iov2 = (struct iovec *)malloc(sizeof(struct iovec)*(2 + iovlen));
	if (!iov2) return;

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
	free(iov2);
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

static void le_meta_event(struct btdev *btdev, uint8_t event,
						void *data, uint8_t len)
{
	void *pkt_data;

	pkt_data = alloca(1 + len);
	if (!pkt_data)
		return;

	((uint8_t *) pkt_data)[0] = event;

	if (len > 0)
		memcpy(pkt_data + 1, data, len);

	send_event(btdev, BT_HCI_EVT_LE_META_EVENT, pkt_data, 1 + len);
}

static void num_completed_packets(struct btdev *btdev)
{
	if (btdev->conn) {
		struct bt_hci_evt_num_completed_packets ncp;

		ncp.num_handles = 1;
		ncp.handle = cpu_to_le16(42);
		ncp.count = cpu_to_le16(1);

		send_event(btdev, BT_HCI_EVT_NUM_COMPLETED_PACKETS,
							&ncp, sizeof(ncp));
	}
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

static void inquiry_cmd(struct btdev *btdev, const void *cmd)
{
	const struct bt_hci_cmd_inquiry *inq_cmd = cmd;
	struct inquiry_data *data;
	struct bt_hci_evt_inquiry_complete ic;
	int status = BT_HCI_ERR_HARDWARE_FAILURE;
	unsigned int inquiry_len_ms;

	if (btdev->inquiry_id > 0) {
		status = BT_HCI_ERR_COMMAND_DISALLOWED;
		goto failed;
	}

	data = malloc(sizeof(*data));
	if (!data)
		goto failed;

	memset(data, 0, sizeof(*data));
	data->btdev = btdev;
	data->num_resp = inq_cmd->num_resp;

	/* Add timeout to cancel inquiry */
	inquiry_len_ms = 1280 * inq_cmd->length;
	if (inquiry_len_ms)
		btdev->inquiry_timeout_id = timeout_add(inquiry_len_ms,
							inquiry_timeout,
							data, NULL);

	btdev->inquiry_id = timeout_add(DEFAULT_INQUIRY_INTERVAL,
							inquiry_callback, data,
							inquiry_destroy);
	/* Return if success */
	if (btdev->inquiry_id > 0)
		return;

failed:
	ic.status = status;
	send_event(btdev, BT_HCI_EVT_INQUIRY_COMPLETE, &ic, sizeof(ic));
}

static void inquiry_cancel(struct btdev *btdev)
{
	uint8_t status;

	if (!btdev->inquiry_id) {
		status = BT_HCI_ERR_COMMAND_DISALLOWED;
		cmd_complete(btdev, BT_HCI_CMD_INQUIRY_CANCEL, &status,
							sizeof(status));
		return;
	}

	timeout_remove(btdev->inquiry_timeout_id);
	btdev->inquiry_timeout_id = 0;
	timeout_remove(btdev->inquiry_id);
	btdev->inquiry_id = 0;

	status = BT_HCI_ERR_SUCCESS;
	cmd_complete(btdev, BT_HCI_CMD_INQUIRY_CANCEL, &status,
							sizeof(status));
}

static void conn_complete(struct btdev *btdev,
					const uint8_t *bdaddr, uint8_t status)
{
	struct bt_hci_evt_conn_complete cc;

	if (!status) {
		struct btdev *remote = find_btdev_by_bdaddr(bdaddr);

		btdev->conn = remote;
		remote->conn = btdev;

		cc.status = status;
		memcpy(cc.bdaddr, btdev->bdaddr, 6);
		cc.encr_mode = 0x00;

		cc.handle = cpu_to_le16(42);
		cc.link_type = 0x01;

		send_event(remote, BT_HCI_EVT_CONN_COMPLETE, &cc, sizeof(cc));

		cc.handle = cpu_to_le16(42);
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

static void accept_conn_request_complete(struct btdev *btdev,
							const uint8_t *bdaddr)
{
	struct btdev *remote = find_btdev_by_bdaddr(bdaddr);

	if (!remote)
		return;

	if (btdev->auth_enable || remote->auth_enable)
		send_event(remote, BT_HCI_EVT_LINK_KEY_REQUEST,
							btdev->bdaddr, 6);
	else
		conn_complete(btdev, bdaddr, BT_HCI_ERR_SUCCESS);
}

static void sync_conn_complete(struct btdev *btdev, uint16_t voice_setting,
								uint8_t status)
{
	struct bt_hci_evt_sync_conn_complete cc;

	if (!btdev->conn)
		return;

	cc.status = status;
	memcpy(cc.bdaddr, btdev->conn->bdaddr, 6);

	cc.handle = cpu_to_le16(status == BT_HCI_ERR_SUCCESS ? 257 : 0);
	cc.link_type = 0x02;
	cc.tx_interval = 0x000c;
	cc.retrans_window = 0x06;
	cc.rx_pkt_len = 60;
	cc.tx_pkt_len = 60;
	cc.air_mode = (voice_setting == 0x0060) ? 0x02 : 0x03;

	send_event(btdev, BT_HCI_EVT_SYNC_CONN_COMPLETE, &cc, sizeof(cc));
}

static void sco_conn_complete(struct btdev *btdev, uint8_t status)
{
	struct bt_hci_evt_conn_complete cc;

	if (!btdev->conn)
		return;

	cc.status = status;
	memcpy(cc.bdaddr, btdev->conn->bdaddr, 6);
	cc.handle = cpu_to_le16(status == BT_HCI_ERR_SUCCESS ? 257 : 0);
	cc.link_type = 0x00;
	cc.encr_mode = 0x00;

	send_event(btdev, BT_HCI_EVT_CONN_COMPLETE, &cc, sizeof(cc));
}

static void le_conn_complete(struct btdev *btdev,
				const struct bt_hci_cmd_le_create_conn *lecc,
				uint8_t status)
{
	char buf[1 + sizeof(struct bt_hci_evt_le_conn_complete)];
	struct bt_hci_evt_le_conn_complete *cc = (void *) &buf[1];

	memset(buf, 0, sizeof(buf));

	buf[0] = BT_HCI_EVT_LE_CONN_COMPLETE;

	if (!status) {
		struct btdev *remote;

		remote = find_btdev_by_bdaddr_type(lecc->peer_addr,
							lecc->peer_addr_type);

		btdev->conn = remote;
		btdev->le_adv_enable = 0;
		remote->conn = btdev;
		remote->le_adv_enable = 0;

		cc->status = status;
		cc->peer_addr_type = btdev->le_scan_own_addr_type;
		if (cc->peer_addr_type == 0x01)
			memcpy(cc->peer_addr, btdev->random_addr, 6);
		else
			memcpy(cc->peer_addr, btdev->bdaddr, 6);

		cc->role = 0x01;
		cc->handle = cpu_to_le16(42);
		cc->interval = lecc->max_interval;
		cc->latency = lecc->latency;
		cc->supv_timeout = lecc->supv_timeout;

		send_event(remote, BT_HCI_EVT_LE_META_EVENT, buf, sizeof(buf));
	}

	cc->status = status;
	cc->peer_addr_type = lecc->peer_addr_type;
	memcpy(cc->peer_addr, lecc->peer_addr, 6);
	cc->role = 0x00;

	send_event(btdev, BT_HCI_EVT_LE_META_EVENT, buf, sizeof(buf));
}

static const uint8_t *scan_addr(const struct btdev *btdev)
{
	if (btdev->le_scan_own_addr_type == 0x01)
		return btdev->random_addr;

	return btdev->bdaddr;
}

static const uint8_t *adv_addr(const struct btdev *btdev)
{
	if (btdev->le_adv_own_addr == 0x01)
		return btdev->random_addr;

	return btdev->bdaddr;
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

static bool adv_connectable(struct btdev *btdev)
{
	if (!btdev->le_adv_enable)
		return false;

	return btdev->le_adv_type != 0x03;
}

static void le_conn_request(struct btdev *btdev,
				const struct bt_hci_cmd_le_create_conn *lecc)
{
	struct btdev *remote = find_btdev_by_bdaddr_type(lecc->peer_addr,
							lecc->peer_addr_type);

	if (remote && adv_connectable(remote) && adv_match(btdev, remote) &&
				remote->le_adv_own_addr == lecc->peer_addr_type)
		le_conn_complete(btdev, lecc, 0);
	else
		le_conn_complete(btdev, lecc,
					BT_HCI_ERR_CONN_FAILED_TO_ESTABLISH);
}

static void conn_request(struct btdev *btdev, const uint8_t *bdaddr)
{
	struct btdev *remote = find_btdev_by_bdaddr(bdaddr);

	if (remote && remote->scan_enable & 0x02) {
		struct bt_hci_evt_conn_request cr;

		memcpy(cr.bdaddr, btdev->bdaddr, 6);
		memcpy(cr.dev_class, btdev->dev_class, 3);
		cr.link_type = 0x01;

		send_event(remote, BT_HCI_EVT_CONN_REQUEST, &cr, sizeof(cr));
	} else {
		conn_complete(btdev, bdaddr, BT_HCI_ERR_PAGE_TIMEOUT);
	}
}

static void rej_le_conn_update(struct btdev *btdev, uint16_t handle,
								uint8_t reason)
{
	struct btdev *remote = btdev->conn;
	struct __packed {
		uint8_t subevent;
		struct bt_hci_evt_le_conn_update_complete ev;
	} ev;

	if (!remote)
		return;

	ev.subevent = BT_HCI_EVT_LE_CONN_UPDATE_COMPLETE;
	ev.ev.handle = cpu_to_le16(handle);
	ev.ev.status = cpu_to_le16(reason);

	send_event(remote, BT_HCI_EVT_LE_META_EVENT, &ev, sizeof(ev));
}

static void le_conn_update(struct btdev *btdev, uint16_t handle,
				uint16_t min_interval, uint16_t max_interval,
				uint16_t latency, uint16_t supv_timeout,
				uint16_t min_length, uint16_t max_length)
{
	struct btdev *remote = btdev->conn;
	struct __packed {
		uint8_t subevent;
		struct bt_hci_evt_le_conn_update_complete ev;
	} ev;

	ev.subevent = BT_HCI_EVT_LE_CONN_UPDATE_COMPLETE;
	ev.ev.handle = cpu_to_le16(handle);
	ev.ev.interval = cpu_to_le16(min_interval);
	ev.ev.latency = cpu_to_le16(latency);
	ev.ev.supv_timeout = cpu_to_le16(supv_timeout);

	if (remote)
		ev.ev.status = BT_HCI_ERR_SUCCESS;
	else
		ev.ev.status = BT_HCI_ERR_UNKNOWN_CONN_ID;

	send_event(btdev, BT_HCI_EVT_LE_META_EVENT, &ev, sizeof(ev));

	if (remote)
		send_event(remote, BT_HCI_EVT_LE_META_EVENT, &ev, sizeof(ev));
}

static void le_conn_param_req(struct btdev *btdev, uint16_t handle,
				uint16_t min_interval, uint16_t max_interval,
				uint16_t latency, uint16_t supv_timeout,
				uint16_t min_length, uint16_t max_length)
{
	struct btdev *remote = btdev->conn;
	struct __packed {
		uint8_t subevent;
		struct bt_hci_evt_le_conn_param_request ev;
	} ev;

	if (!remote)
		return;

	ev.subevent = BT_HCI_EVT_LE_CONN_PARAM_REQUEST;
	ev.ev.handle = cpu_to_le16(handle);
	ev.ev.min_interval = cpu_to_le16(min_interval);
	ev.ev.max_interval = cpu_to_le16(max_interval);
	ev.ev.latency = cpu_to_le16(latency);
	ev.ev.supv_timeout = cpu_to_le16(supv_timeout);

	send_event(remote, BT_HCI_EVT_LE_META_EVENT, &ev, sizeof(ev));
}

static void disconnect_complete(struct btdev *btdev, uint16_t handle,
							uint8_t reason)
{
	struct bt_hci_evt_disconnect_complete dc;
	struct btdev *remote = btdev->conn;

	if (!remote) {
		dc.status = BT_HCI_ERR_UNKNOWN_CONN_ID;
		dc.handle = cpu_to_le16(handle);
		dc.reason = 0x00;

		send_event(btdev, BT_HCI_EVT_DISCONNECT_COMPLETE,
							&dc, sizeof(dc));
		return;
	}

	dc.status = BT_HCI_ERR_SUCCESS;
	dc.handle = cpu_to_le16(handle);
	dc.reason = reason;

	btdev->conn = NULL;
	remote->conn = NULL;

	send_event(btdev, BT_HCI_EVT_DISCONNECT_COMPLETE, &dc, sizeof(dc));
	send_event(remote, BT_HCI_EVT_DISCONNECT_COMPLETE, &dc, sizeof(dc));
}

static void link_key_req_reply_complete(struct btdev *btdev,
					const uint8_t *bdaddr,
					const uint8_t *link_key)
{
	struct btdev *remote = btdev->conn;
	struct bt_hci_evt_auth_complete ev;

	memcpy(btdev->link_key, link_key, 16);

	if (!remote) {
		remote = find_btdev_by_bdaddr(bdaddr);
		if (!remote)
			return;
	}

	if (!memcmp(remote->link_key, LINK_KEY_NONE, 16)) {
		send_event(remote, BT_HCI_EVT_LINK_KEY_REQUEST,
							btdev->bdaddr, 6);
		return;
	}

	ev.handle = cpu_to_le16(42);

	if (!memcmp(btdev->link_key, remote->link_key, 16))
		ev.status = BT_HCI_ERR_SUCCESS;
	else
		ev.status = BT_HCI_ERR_AUTH_FAILURE;

	send_event(btdev, BT_HCI_EVT_AUTH_COMPLETE, &ev, sizeof(ev));
	send_event(remote, BT_HCI_EVT_AUTH_COMPLETE, &ev, sizeof(ev));
}

static void link_key_req_neg_reply_complete(struct btdev *btdev,
							const uint8_t *bdaddr)
{
	struct btdev *remote = btdev->conn;

	if (!remote) {
		remote = find_btdev_by_bdaddr(bdaddr);
		if (!remote)
			return;
	}

	if (use_ssp(btdev, remote)) {
		struct bt_hci_evt_io_capability_request io_req;

		memcpy(io_req.bdaddr, bdaddr, 6);
		send_event(btdev, BT_HCI_EVT_IO_CAPABILITY_REQUEST, &io_req,
							sizeof(io_req));
	} else {
		struct bt_hci_evt_pin_code_request pin_req;

		memcpy(pin_req.bdaddr, bdaddr, 6);
		send_event(btdev, BT_HCI_EVT_PIN_CODE_REQUEST, &pin_req,
							sizeof(pin_req));
	}
}

static uint8_t get_link_key_type(struct btdev *btdev)
{
	struct btdev *remote = btdev->conn;
	uint8_t auth, unauth;

	if (!remote)
		return 0x00;

	if (!btdev->simple_pairing_mode)
		return 0x00;

	if (btdev->ssp_debug_mode || remote->ssp_debug_mode)
		return 0x03;

	if (btdev->secure_conn_support && remote->secure_conn_support) {
		unauth = 0x07;
		auth = 0x08;
	} else {
		unauth = 0x04;
		auth = 0x05;
	}

	if (btdev->io_cap == 0x03 || remote->io_cap == 0x03)
		return unauth;

	if (!(btdev->auth_req & 0x01) && !(remote->auth_req & 0x01))
		return unauth;

	/* DisplayOnly only produces authenticated with KeyboardOnly */
	if (btdev->io_cap == 0x00 && remote->io_cap != 0x02)
		return unauth;

	/* DisplayOnly only produces authenticated with KeyboardOnly */
	if (remote->io_cap == 0x00 && btdev->io_cap != 0x02)
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
	ev.key_type = get_link_key_type(btdev);

	send_event(btdev, BT_HCI_EVT_LINK_KEY_NOTIFY, &ev, sizeof(ev));
}

static void encrypt_change(struct btdev *btdev, uint8_t mode, uint8_t status)
{
	struct bt_hci_evt_encrypt_change ev;

	ev.status = status;
	ev.handle = cpu_to_le16(42);
	ev.encr_mode = mode;

	send_event(btdev, BT_HCI_EVT_ENCRYPT_CHANGE, &ev, sizeof(ev));
}

static void pin_code_req_reply_complete(struct btdev *btdev,
					const uint8_t *bdaddr, uint8_t pin_len,
					const uint8_t *pin_code)
{
	struct bt_hci_evt_auth_complete ev;
	struct btdev *remote = btdev->conn;

	if (!remote) {
		remote = find_btdev_by_bdaddr(bdaddr);
		if (!remote)
			return;
	}

	memcpy(btdev->pin, pin_code, pin_len);
	btdev->pin_len = pin_len;

	if (!remote->pin_len) {
		struct bt_hci_evt_pin_code_request pin_req;

		memcpy(pin_req.bdaddr, btdev->bdaddr, 6);
		send_event(remote, BT_HCI_EVT_PIN_CODE_REQUEST, &pin_req,
							sizeof(pin_req));
		return;
	}

	if (btdev->pin_len == remote->pin_len &&
			!memcmp(btdev->pin, remote->pin, btdev->pin_len)) {
		link_key_notify(btdev, remote->bdaddr, LINK_KEY_DUMMY);
		link_key_notify(remote, btdev->bdaddr, LINK_KEY_DUMMY);
		ev.status = BT_HCI_ERR_SUCCESS;
	} else {
		ev.status = BT_HCI_ERR_AUTH_FAILURE;
	}

	if (remote->conn) {
		ev.handle = cpu_to_le16(42);
		send_event(remote, BT_HCI_EVT_AUTH_COMPLETE, &ev, sizeof(ev));
	} else {
		conn_complete(remote, btdev->bdaddr, ev.status);
	}

	btdev->pin_len = 0;
	remote->pin_len = 0;
}

static void pin_code_req_neg_reply_complete(struct btdev *btdev,
							const uint8_t *bdaddr)
{
	struct bt_hci_evt_auth_complete ev;
	struct btdev *remote = btdev->conn;

	if (!remote) {
		remote = find_btdev_by_bdaddr(bdaddr);
		if (!remote)
			return;
	}

	ev.status = BT_HCI_ERR_PIN_OR_KEY_MISSING;
	ev.handle = cpu_to_le16(42);

	if (btdev->conn)
		send_event(btdev, BT_HCI_EVT_AUTH_COMPLETE, &ev, sizeof(ev));
	else
		conn_complete(btdev, bdaddr, BT_HCI_ERR_PIN_OR_KEY_MISSING);

	if (remote->conn) {
	        if (remote->pin_len)
			send_event(remote, BT_HCI_EVT_AUTH_COMPLETE, &ev,
								sizeof(ev));
	} else {
		conn_complete(remote, btdev->bdaddr,
					BT_HCI_ERR_PIN_OR_KEY_MISSING);
	}
}

static void auth_request_complete(struct btdev *btdev, uint16_t handle)
{
	struct btdev *remote = btdev->conn;

	if (!remote) {
		struct bt_hci_evt_auth_complete ev;

		ev.status = BT_HCI_ERR_UNKNOWN_CONN_ID;
		ev.handle = cpu_to_le16(handle);

		send_event(btdev, BT_HCI_EVT_AUTH_COMPLETE, &ev, sizeof(ev));

		return;
	}

	btdev->auth_init = true;

	send_event(btdev, BT_HCI_EVT_LINK_KEY_REQUEST, remote->bdaddr, 6);
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

static void remote_features_complete(struct btdev *btdev, uint16_t handle)
{
	struct bt_hci_evt_remote_features_complete rfc;

	if (btdev->conn) {
		rfc.status = BT_HCI_ERR_SUCCESS;
		rfc.handle = cpu_to_le16(handle);
		memcpy(rfc.features, btdev->conn->features, 8);
	} else {
		rfc.status = BT_HCI_ERR_UNKNOWN_CONN_ID;
		rfc.handle = cpu_to_le16(handle);
		memset(rfc.features, 0, 8);
	}

	send_event(btdev, BT_HCI_EVT_REMOTE_FEATURES_COMPLETE,
							&rfc, sizeof(rfc));
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

static void remote_ext_features_complete(struct btdev *btdev, uint16_t handle,
								uint8_t page)
{
	struct bt_hci_evt_remote_ext_features_complete refc;

	if (btdev->conn && page < 0x02) {
		refc.handle = cpu_to_le16(handle);
		refc.page = page;
		refc.max_page = 0x01;

		switch (page) {
		case 0x00:
			refc.status = BT_HCI_ERR_SUCCESS;
			memcpy(refc.features, btdev->conn->features, 8);
			break;
		case 0x01:
			refc.status = BT_HCI_ERR_SUCCESS;
			btdev_get_host_features(btdev->conn, refc.features);
			break;
		default:
			refc.status = BT_HCI_ERR_INVALID_PARAMETERS;
			memset(refc.features, 0, 8);
			break;
		}
	} else {
		refc.status = BT_HCI_ERR_UNKNOWN_CONN_ID;
		refc.handle = cpu_to_le16(handle);
		refc.page = page;
		refc.max_page = 0x01;
		memset(refc.features, 0, 8);
	}

	send_event(btdev, BT_HCI_EVT_REMOTE_EXT_FEATURES_COMPLETE,
							&refc, sizeof(refc));
}

static void remote_version_complete(struct btdev *btdev, uint16_t handle)
{
	struct bt_hci_evt_remote_version_complete rvc;

	if (btdev->conn) {
		rvc.status = BT_HCI_ERR_SUCCESS;
		rvc.handle = cpu_to_le16(handle);
		rvc.lmp_ver = btdev->conn->version;
		rvc.manufacturer = cpu_to_le16(btdev->conn->manufacturer);
		rvc.lmp_subver = cpu_to_le16(btdev->conn->revision);
	} else {
		rvc.status = BT_HCI_ERR_UNKNOWN_CONN_ID;
		rvc.handle = cpu_to_le16(handle);
		rvc.lmp_ver = 0x00;
		rvc.manufacturer = cpu_to_le16(0);
		rvc.lmp_subver = cpu_to_le16(0);
	}

	send_event(btdev, BT_HCI_EVT_REMOTE_VERSION_COMPLETE,
							&rvc, sizeof(rvc));
}

static void remote_clock_offset_complete(struct btdev *btdev, uint16_t handle)
{
	struct bt_hci_evt_clock_offset_complete coc;

	if (btdev->conn) {
		coc.status = BT_HCI_ERR_SUCCESS;
		coc.handle = cpu_to_le16(handle);
		coc.clock_offset = 0;
	} else {
		coc.status = BT_HCI_ERR_UNKNOWN_CONN_ID;
		coc.handle = cpu_to_le16(handle);
		coc.clock_offset = 0;
	}

	send_event(btdev, BT_HCI_EVT_CLOCK_OFFSET_COMPLETE,
							&coc, sizeof(coc));
}

static void read_enc_key_size_complete(struct btdev *btdev, uint16_t handle)
{
	struct bt_hci_rsp_read_encrypt_key_size rsp;

	rsp.handle = cpu_to_le16(handle);

	if (btdev->conn) {
		rsp.status = BT_HCI_ERR_SUCCESS;
		rsp.key_size = 16;
	} else {
		rsp.status = BT_HCI_ERR_UNKNOWN_CONN_ID;
		rsp.key_size = 0;
	}

	cmd_complete(btdev, BT_HCI_CMD_READ_ENCRYPT_KEY_SIZE,
							&rsp, sizeof(rsp));
}

static void io_cap_req_reply_complete(struct btdev *btdev,
					const uint8_t *bdaddr,
					uint8_t capability, uint8_t oob_data,
					uint8_t authentication)
{
	struct btdev *remote = btdev->conn;
	struct bt_hci_evt_io_capability_response ev;
	struct bt_hci_rsp_io_capability_request_reply rsp;
	uint8_t status;

	if (!remote) {
		status = BT_HCI_ERR_UNKNOWN_CONN_ID;
		goto done;
	}

	status = BT_HCI_ERR_SUCCESS;

	btdev->io_cap = capability;
	btdev->auth_req = authentication;

	memcpy(ev.bdaddr, btdev->bdaddr, 6);
	ev.capability = capability;
	ev.oob_data = oob_data;
	ev.authentication = authentication;

	send_event(remote, BT_HCI_EVT_IO_CAPABILITY_RESPONSE, &ev, sizeof(ev));

	if (remote->io_cap) {
		struct bt_hci_evt_user_confirm_request cfm;

		memcpy(cfm.bdaddr, btdev->bdaddr, 6);
		cfm.passkey = 0;

		send_event(remote, BT_HCI_EVT_USER_CONFIRM_REQUEST,
							&cfm, sizeof(cfm));

		memcpy(cfm.bdaddr, bdaddr, 6);
		send_event(btdev, BT_HCI_EVT_USER_CONFIRM_REQUEST,
							&cfm, sizeof(cfm));
	} else {
		send_event(remote, BT_HCI_EVT_IO_CAPABILITY_REQUEST,
							btdev->bdaddr, 6);
	}

done:
	rsp.status = status;
	memcpy(rsp.bdaddr, bdaddr, 6);
	cmd_complete(btdev, BT_HCI_CMD_IO_CAPABILITY_REQUEST_REPLY,
							&rsp, sizeof(rsp));
}

static void io_cap_req_neg_reply_complete(struct btdev *btdev,
							const uint8_t *bdaddr)
{
	struct bt_hci_rsp_io_capability_request_neg_reply rsp;

	rsp.status = BT_HCI_ERR_SUCCESS;
	memcpy(rsp.bdaddr, bdaddr, 6);
	cmd_complete(btdev, BT_HCI_CMD_IO_CAPABILITY_REQUEST_NEG_REPLY,
							&rsp, sizeof(rsp));
}

static void ssp_complete(struct btdev *btdev, const uint8_t *bdaddr,
						uint8_t status, bool wait)
{
	struct bt_hci_evt_simple_pairing_complete iev, aev;
	struct bt_hci_evt_auth_complete auth;
	struct btdev *remote = btdev->conn;
	struct btdev *init, *accp;

	if (!remote)
		return;

	btdev->ssp_status = status;
	btdev->ssp_auth_complete = true;

	if (!remote->ssp_auth_complete && wait)
		return;

	if (status == BT_HCI_ERR_SUCCESS &&
				remote->ssp_status != BT_HCI_ERR_SUCCESS)
		status = remote->ssp_status;

	iev.status = status;
	aev.status = status;

	if (btdev->auth_init) {
		init = btdev;
		accp = remote;
		memcpy(iev.bdaddr, bdaddr, 6);
		memcpy(aev.bdaddr, btdev->bdaddr, 6);
	} else {
		init = remote;
		accp = btdev;
		memcpy(iev.bdaddr, btdev->bdaddr, 6);
		memcpy(aev.bdaddr, bdaddr, 6);
	}

	send_event(init, BT_HCI_EVT_SIMPLE_PAIRING_COMPLETE, &iev,
								sizeof(iev));
	send_event(accp, BT_HCI_EVT_SIMPLE_PAIRING_COMPLETE, &aev,
								sizeof(aev));

	if (status == BT_HCI_ERR_SUCCESS) {
		link_key_notify(init, iev.bdaddr, LINK_KEY_DUMMY);
		link_key_notify(accp, aev.bdaddr, LINK_KEY_DUMMY);
	}

	auth.status = status;
	auth.handle = cpu_to_le16(42);
	send_event(init, BT_HCI_EVT_AUTH_COMPLETE, &auth, sizeof(auth));
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

static void le_set_scan_enable_complete(struct btdev *btdev)
{
	int i;

	for (i = 0; i < MAX_BTDEV_ENTRIES; i++) {
		uint8_t report_type;

		if (!btdev_list[i] || btdev_list[i] == btdev)
			continue;

		if (!btdev_list[i]->le_adv_enable)
			continue;

		if (!adv_match(btdev, btdev_list[i]))
			continue;

		report_type = get_adv_report_type(btdev_list[i]->le_adv_type);
		le_send_adv_report(btdev, btdev_list[i], report_type);

		if (btdev->le_scan_type != 0x01)
			continue;

		/* ADV_IND & ADV_SCAN_IND generate a scan response */
		if (btdev_list[i]->le_adv_type == 0x00 ||
					btdev_list[i]->le_adv_type == 0x02)
			le_send_adv_report(btdev, btdev_list[i], 0x04);
	}
}

static void le_read_remote_features_complete(struct btdev *btdev)
{
	char buf[1 + sizeof(struct bt_hci_evt_le_remote_features_complete)];
	struct bt_hci_evt_le_remote_features_complete *ev = (void *) &buf[1];
	struct btdev *remote = btdev->conn;

	if (!remote) {
		cmd_status(btdev, BT_HCI_ERR_UNKNOWN_CONN_ID,
					BT_HCI_CMD_LE_READ_REMOTE_FEATURES);
		return;
	}

	cmd_status(btdev, BT_HCI_ERR_SUCCESS,
				BT_HCI_CMD_LE_READ_REMOTE_FEATURES);

	memset(buf, 0, sizeof(buf));
	buf[0] = BT_HCI_EVT_LE_REMOTE_FEATURES_COMPLETE;
	ev->status = BT_HCI_ERR_SUCCESS;
	ev->handle = cpu_to_le16(42);
	memcpy(ev->features, remote->le_features, 8);

	send_event(btdev, BT_HCI_EVT_LE_META_EVENT, buf, sizeof(buf));
}

static void le_start_encrypt_complete(struct btdev *btdev, uint16_t ediv,
								uint64_t rand)
{
	char buf[1 + sizeof(struct bt_hci_evt_le_long_term_key_request)];
	struct bt_hci_evt_le_long_term_key_request *ev = (void *) &buf[1];
	struct btdev *remote = btdev->conn;

	if (!remote) {
		cmd_status(btdev, BT_HCI_ERR_UNKNOWN_CONN_ID,
						BT_HCI_CMD_LE_START_ENCRYPT);
		return;
	}

	cmd_status(btdev, BT_HCI_ERR_SUCCESS, BT_HCI_CMD_LE_START_ENCRYPT);

	memset(buf, 0, sizeof(buf));
	buf[0] = BT_HCI_EVT_LE_LONG_TERM_KEY_REQUEST;
	ev->handle = cpu_to_le16(42);
	ev->ediv = ediv;
	ev->rand = rand;

	send_event(remote, BT_HCI_EVT_LE_META_EVENT, buf, sizeof(buf));
}

static void le_encrypt_complete(struct btdev *btdev)
{
	struct bt_hci_evt_encrypt_change ev;
	struct bt_hci_rsp_le_ltk_req_reply rp;
	struct btdev *remote = btdev->conn;

	memset(&rp, 0, sizeof(rp));
	rp.handle = cpu_to_le16(42);

	if (!remote) {
		rp.status = BT_HCI_ERR_UNKNOWN_CONN_ID;
		cmd_complete(btdev, BT_HCI_CMD_LE_LTK_REQ_REPLY, &rp,
							sizeof(rp));
		return;
	}

	rp.status = BT_HCI_ERR_SUCCESS;
	cmd_complete(btdev, BT_HCI_CMD_LE_LTK_REQ_REPLY, &rp, sizeof(rp));

	memset(&ev, 0, sizeof(ev));

	if (memcmp(btdev->le_ltk, remote->le_ltk, 16)) {
		ev.status = BT_HCI_ERR_AUTH_FAILURE;
		ev.encr_mode = 0x00;
	} else {
		ev.status = BT_HCI_ERR_SUCCESS;
		ev.encr_mode = 0x01;
	}

	ev.handle = cpu_to_le16(42);

	send_event(btdev, BT_HCI_EVT_ENCRYPT_CHANGE, &ev, sizeof(ev));
	send_event(remote, BT_HCI_EVT_ENCRYPT_CHANGE, &ev, sizeof(ev));
}

static void ltk_neg_reply_complete(struct btdev *btdev)
{
	struct bt_hci_rsp_le_ltk_req_neg_reply rp;
	struct bt_hci_evt_encrypt_change ev;
	struct btdev *remote = btdev->conn;

	memset(&rp, 0, sizeof(rp));
	rp.handle = cpu_to_le16(42);

	if (!remote) {
		rp.status = BT_HCI_ERR_UNKNOWN_CONN_ID;
		cmd_complete(btdev, BT_HCI_CMD_LE_LTK_REQ_NEG_REPLY, &rp,
							sizeof(rp));
		return;
	}

	rp.status = BT_HCI_ERR_SUCCESS;
	cmd_complete(btdev, BT_HCI_CMD_LE_LTK_REQ_NEG_REPLY, &rp, sizeof(rp));

	memset(&ev, 0, sizeof(ev));
	ev.status = BT_HCI_ERR_PIN_OR_KEY_MISSING;
	ev.handle = cpu_to_le16(42);

	send_event(remote, BT_HCI_EVT_ENCRYPT_CHANGE, &ev, sizeof(ev));
}

static void btdev_reset(struct btdev *btdev)
{
	/* FIXME: include here clearing of all states that should be
	 * cleared upon HCI_Reset
	 */

	btdev->le_scan_enable		= 0x00;
	btdev->le_adv_enable		= 0x00;
}

static void default_cmd(struct btdev *btdev, uint16_t opcode,
						const void *data, uint8_t len)
{
	const struct bt_hci_cmd_remote_name_request_cancel *rnrc;
	const struct bt_hci_cmd_write_default_link_policy *wdlp;
	const struct bt_hci_cmd_set_event_mask *sem;
	const struct bt_hci_cmd_set_event_filter *sef;
	const struct bt_hci_cmd_write_local_name *wln;
	const struct bt_hci_cmd_write_conn_accept_timeout *wcat;
	const struct bt_hci_cmd_write_page_timeout *wpt;
	const struct bt_hci_cmd_write_scan_enable *wse;
	const struct bt_hci_cmd_write_page_scan_activity *wpsa;
	const struct bt_hci_cmd_write_inquiry_scan_activity *wisa;
	const struct bt_hci_cmd_write_page_scan_type *wpst;
	const struct bt_hci_cmd_write_auth_enable *wae;
	const struct bt_hci_cmd_write_class_of_dev *wcod;
	const struct bt_hci_cmd_write_voice_setting *wvs;
	const struct bt_hci_cmd_set_host_flow_control *shfc;
	const struct bt_hci_cmd_write_inquiry_mode *wim;
	const struct bt_hci_cmd_write_afh_assessment_mode *waam;
	const struct bt_hci_cmd_write_ext_inquiry_response *weir;
	const struct bt_hci_cmd_write_simple_pairing_mode *wspm;
	const struct bt_hci_cmd_io_capability_request_reply *icrr;
	const struct bt_hci_cmd_io_capability_request_reply *icrnr;
	const struct bt_hci_cmd_read_encrypt_key_size *reks;
	const struct bt_hci_cmd_write_le_host_supported *wlhs;
	const struct bt_hci_cmd_write_secure_conn_support *wscs;
	const struct bt_hci_cmd_set_event_mask_page2 *semp2;
	const struct bt_hci_cmd_le_set_event_mask *lsem;
	const struct bt_hci_cmd_le_set_random_address *lsra;
	const struct bt_hci_cmd_le_set_adv_parameters *lsap;
	const struct bt_hci_cmd_le_set_adv_data *lsad;
	const struct bt_hci_cmd_le_set_scan_rsp_data *lssrd;
	const struct bt_hci_cmd_setup_sync_conn *ssc;
	const struct bt_hci_cmd_write_ssp_debug_mode *wsdm;
	const struct bt_hci_cmd_le_set_adv_enable *lsae;
	const struct bt_hci_cmd_le_set_scan_parameters *lssp;
	const struct bt_hci_cmd_le_set_scan_enable *lsse;
	const struct bt_hci_cmd_le_start_encrypt *lse;
	const struct bt_hci_cmd_le_ltk_req_reply *llrr;
	const struct bt_hci_cmd_le_encrypt *lenc_cmd;
	const struct bt_hci_cmd_le_generate_dhkey *dh;
	const struct bt_hci_cmd_le_conn_param_req_reply *lcprr_cmd;
	const struct bt_hci_cmd_le_conn_param_req_neg_reply *lcprnr_cmd;
	const struct bt_hci_cmd_read_local_amp_assoc *rlaa_cmd;
	const struct bt_hci_cmd_read_rssi *rrssi;
	const struct bt_hci_cmd_read_tx_power *rtxp;
	struct bt_hci_rsp_read_default_link_policy rdlp;
	struct bt_hci_rsp_read_stored_link_key rslk;
	struct bt_hci_rsp_write_stored_link_key wslk;
	struct bt_hci_rsp_delete_stored_link_key dslk;
	struct bt_hci_rsp_read_local_name rln;
	struct bt_hci_rsp_read_conn_accept_timeout rcat;
	struct bt_hci_rsp_read_page_timeout rpt;
	struct bt_hci_rsp_read_scan_enable rse;
	struct bt_hci_rsp_read_page_scan_activity rpsa;
	struct bt_hci_rsp_read_inquiry_scan_activity risa;
	struct bt_hci_rsp_read_page_scan_type rpst;
	struct bt_hci_rsp_read_auth_enable rae;
	struct bt_hci_rsp_read_class_of_dev rcod;
	struct bt_hci_rsp_read_voice_setting rvs;
	struct bt_hci_rsp_read_num_supported_iac rnsi;
	struct bt_hci_rsp_read_current_iac_lap *rcil;
	struct bt_hci_rsp_read_inquiry_mode rim;
	struct bt_hci_rsp_read_afh_assessment_mode raam;
	struct bt_hci_rsp_read_ext_inquiry_response reir;
	struct bt_hci_rsp_read_simple_pairing_mode rspm;
	struct bt_hci_rsp_read_local_oob_data rlod;
	struct bt_hci_rsp_read_inquiry_resp_tx_power rirtp;
	struct bt_hci_rsp_read_le_host_supported rlhs;
	struct bt_hci_rsp_read_secure_conn_support rscs;
	struct bt_hci_rsp_read_local_oob_ext_data rloed;
	struct bt_hci_rsp_read_sync_train_params rstp;
	struct bt_hci_rsp_read_local_version rlv;
	struct bt_hci_rsp_read_local_commands rlc;
	struct bt_hci_rsp_read_local_features rlf;
	struct bt_hci_rsp_read_local_ext_features rlef;
	struct bt_hci_rsp_read_buffer_size rbs;
	struct bt_hci_rsp_read_country_code rcc;
	struct bt_hci_rsp_read_bd_addr rba;
	struct bt_hci_rsp_read_data_block_size rdbs;
	struct bt_hci_rsp_read_local_codecs *rlsc;
	struct bt_hci_rsp_read_local_amp_info rlai;
	struct bt_hci_rsp_read_local_amp_assoc rlaa_rsp;
	struct bt_hci_rsp_get_mws_transport_config *gmtc;
	struct bt_hci_rsp_le_conn_param_req_reply lcprr_rsp;
	struct bt_hci_rsp_le_conn_param_req_neg_reply lcprnr_rsp;
	struct bt_hci_rsp_le_read_buffer_size lrbs;
	struct bt_hci_rsp_le_read_local_features lrlf;
	struct bt_hci_rsp_le_read_adv_tx_power lratp;
	struct bt_hci_rsp_le_read_supported_states lrss;
	struct bt_hci_rsp_le_read_white_list_size lrwls;
	struct bt_hci_rsp_le_encrypt lenc;
	struct bt_hci_rsp_le_rand lr;
	struct bt_hci_rsp_le_test_end lte;
	struct bt_hci_rsp_remote_name_request_cancel rnrc_rsp;
	struct bt_hci_rsp_link_key_request_reply lkrr_rsp;
	struct bt_hci_rsp_link_key_request_neg_reply lkrnr_rsp;
	struct bt_hci_rsp_pin_code_request_neg_reply pcrr_rsp;
	struct bt_hci_rsp_pin_code_request_neg_reply pcrnr_rsp;
	struct bt_hci_rsp_user_confirm_request_reply ucrr_rsp;
	struct bt_hci_rsp_user_confirm_request_neg_reply ucrnr_rsp;
	struct bt_hci_rsp_read_rssi rrssi_rsp;
	struct bt_hci_rsp_read_tx_power rtxp_rsp;
	struct bt_hci_evt_le_read_local_pk256_complete pk_evt;
	struct bt_hci_evt_le_generate_dhkey_complete dh_evt;
	uint8_t status, page;

	switch (opcode) {
	case BT_HCI_CMD_INQUIRY:
		if (btdev->type == BTDEV_TYPE_LE)
			goto unsupported;
		cmd_status(btdev, BT_HCI_ERR_SUCCESS, opcode);
		break;

	case BT_HCI_CMD_INQUIRY_CANCEL:
		if (btdev->type == BTDEV_TYPE_LE)
			goto unsupported;
		inquiry_cancel(btdev);
		break;

	case BT_HCI_CMD_CREATE_CONN:
		if (btdev->type == BTDEV_TYPE_LE)
			goto unsupported;
		cmd_status(btdev, BT_HCI_ERR_SUCCESS, opcode);
		break;

	case BT_HCI_CMD_DISCONNECT:
		cmd_status(btdev, BT_HCI_ERR_SUCCESS, opcode);
		break;

	case BT_HCI_CMD_CREATE_CONN_CANCEL:
		if (btdev->type == BTDEV_TYPE_LE)
			goto unsupported;
		cmd_status(btdev, BT_HCI_ERR_SUCCESS, opcode);
		break;

	case BT_HCI_CMD_ACCEPT_CONN_REQUEST:
		if (btdev->type == BTDEV_TYPE_LE)
			goto unsupported;
		cmd_status(btdev, BT_HCI_ERR_SUCCESS, opcode);
		break;

	case BT_HCI_CMD_REJECT_CONN_REQUEST:
		if (btdev->type == BTDEV_TYPE_LE)
			goto unsupported;
		cmd_status(btdev, BT_HCI_ERR_SUCCESS, opcode);
		break;

	case BT_HCI_CMD_LINK_KEY_REQUEST_REPLY:
		if (btdev->type == BTDEV_TYPE_LE)
			goto unsupported;
		lkrr_rsp.status = BT_HCI_ERR_SUCCESS;
		memcpy(lkrr_rsp.bdaddr, data, 6);
		cmd_complete(btdev, opcode, &lkrr_rsp, sizeof(lkrr_rsp));
		break;

	case BT_HCI_CMD_LINK_KEY_REQUEST_NEG_REPLY:
		if (btdev->type == BTDEV_TYPE_LE)
			goto unsupported;
		lkrnr_rsp.status = BT_HCI_ERR_SUCCESS;
		memcpy(lkrnr_rsp.bdaddr, data, 6);
		cmd_complete(btdev, opcode, &lkrnr_rsp, sizeof(lkrnr_rsp));
		break;

	case BT_HCI_CMD_PIN_CODE_REQUEST_REPLY:
		if (btdev->type == BTDEV_TYPE_LE)
			goto unsupported;
		pcrr_rsp.status = BT_HCI_ERR_SUCCESS;
		memcpy(pcrr_rsp.bdaddr, data, 6);
		cmd_complete(btdev, opcode, &pcrr_rsp, sizeof(pcrr_rsp));
		break;

	case BT_HCI_CMD_PIN_CODE_REQUEST_NEG_REPLY:
		if (btdev->type == BTDEV_TYPE_LE)
			goto unsupported;
		pcrnr_rsp.status = BT_HCI_ERR_SUCCESS;
		memcpy(pcrnr_rsp.bdaddr, data, 6);
		cmd_complete(btdev, opcode, &pcrnr_rsp, sizeof(pcrnr_rsp));
		break;

	case BT_HCI_CMD_AUTH_REQUESTED:
		if (btdev->type == BTDEV_TYPE_LE)
			goto unsupported;
		cmd_status(btdev, BT_HCI_ERR_SUCCESS, opcode);
		break;

	case BT_HCI_CMD_SET_CONN_ENCRYPT:
		if (btdev->type == BTDEV_TYPE_LE)
			goto unsupported;
		cmd_status(btdev, BT_HCI_ERR_SUCCESS, opcode);
		break;

	case BT_HCI_CMD_REMOTE_NAME_REQUEST:
		if (btdev->type == BTDEV_TYPE_LE)
			goto unsupported;
		cmd_status(btdev, BT_HCI_ERR_SUCCESS, opcode);
		break;

	case BT_HCI_CMD_REMOTE_NAME_REQUEST_CANCEL:
		if (btdev->type == BTDEV_TYPE_LE)
			goto unsupported;
		rnrc = data;
		rnrc_rsp.status = BT_HCI_ERR_SUCCESS;
		memcpy(rnrc_rsp.bdaddr, rnrc->bdaddr, 6);
		cmd_complete(btdev, opcode, &rnrc_rsp, sizeof(rnrc_rsp));
		break;

	case BT_HCI_CMD_READ_REMOTE_FEATURES:
		if (btdev->type == BTDEV_TYPE_LE)
			goto unsupported;
		cmd_status(btdev, BT_HCI_ERR_SUCCESS, opcode);
		break;

	case BT_HCI_CMD_READ_REMOTE_EXT_FEATURES:
		if (btdev->type == BTDEV_TYPE_LE)
			goto unsupported;
		cmd_status(btdev, BT_HCI_ERR_SUCCESS, opcode);
		break;

	case BT_HCI_CMD_READ_REMOTE_VERSION:
		cmd_status(btdev, BT_HCI_ERR_SUCCESS, opcode);
		break;

	case BT_HCI_CMD_READ_CLOCK_OFFSET:
		if (btdev->type == BTDEV_TYPE_LE)
			goto unsupported;
		cmd_status(btdev, BT_HCI_ERR_SUCCESS, opcode);
		break;

	case BT_HCI_CMD_READ_DEFAULT_LINK_POLICY:
		if (btdev->type == BTDEV_TYPE_LE)
			goto unsupported;
		rdlp.status = BT_HCI_ERR_SUCCESS;
		rdlp.policy = cpu_to_le16(btdev->default_link_policy);
		cmd_complete(btdev, opcode, &rdlp, sizeof(rdlp));
		break;

	case BT_HCI_CMD_WRITE_DEFAULT_LINK_POLICY:
		if (btdev->type == BTDEV_TYPE_LE)
			goto unsupported;
		wdlp = data;
		btdev->default_link_policy = le16_to_cpu(wdlp->policy);
		status = BT_HCI_ERR_SUCCESS;
		cmd_complete(btdev, opcode, &status, sizeof(status));
		break;

	case BT_HCI_CMD_SET_EVENT_MASK:
		sem = data;
		memcpy(btdev->event_mask, sem->mask, 8);
		status = BT_HCI_ERR_SUCCESS;
		cmd_complete(btdev, opcode, &status, sizeof(status));
		break;

	case BT_HCI_CMD_RESET:
		btdev_reset(btdev);
		status = BT_HCI_ERR_SUCCESS;
		cmd_complete(btdev, opcode, &status, sizeof(status));
		break;

	case BT_HCI_CMD_SET_EVENT_FILTER:
		if (btdev->type == BTDEV_TYPE_LE)
			goto unsupported;
		sef = data;
		btdev->event_filter = sef->type;
		status = BT_HCI_ERR_SUCCESS;
		cmd_complete(btdev, opcode, &status, sizeof(status));
		break;

	case BT_HCI_CMD_READ_STORED_LINK_KEY:
		if (btdev->type == BTDEV_TYPE_LE)
			goto unsupported;
		rslk.status = BT_HCI_ERR_SUCCESS;
		rslk.max_num_keys = cpu_to_le16(0);
		rslk.num_keys = cpu_to_le16(0);
		cmd_complete(btdev, opcode, &rslk, sizeof(rslk));
		break;

	case BT_HCI_CMD_WRITE_STORED_LINK_KEY:
		if (btdev->type == BTDEV_TYPE_LE)
			goto unsupported;
		wslk.status = BT_HCI_ERR_SUCCESS;
		wslk.num_keys = 0;
		cmd_complete(btdev, opcode, &wslk, sizeof(wslk));
		break;

	case BT_HCI_CMD_DELETE_STORED_LINK_KEY:
		if (btdev->type == BTDEV_TYPE_LE)
			goto unsupported;
		dslk.status = BT_HCI_ERR_SUCCESS;
		dslk.num_keys = cpu_to_le16(0);
		cmd_complete(btdev, opcode, &dslk, sizeof(dslk));
		break;

	case BT_HCI_CMD_WRITE_LOCAL_NAME:
		if (btdev->type == BTDEV_TYPE_LE)
			goto unsupported;
		wln = data;
		memcpy(btdev->name, wln->name, 248);
		status = BT_HCI_ERR_SUCCESS;
		cmd_complete(btdev, opcode, &status, sizeof(status));
		break;

	case BT_HCI_CMD_READ_LOCAL_NAME:
		if (btdev->type == BTDEV_TYPE_LE)
			goto unsupported;
		rln.status = BT_HCI_ERR_SUCCESS;
		memcpy(rln.name, btdev->name, 248);
		cmd_complete(btdev, opcode, &rln, sizeof(rln));
		break;

	case BT_HCI_CMD_READ_CONN_ACCEPT_TIMEOUT:
		if (btdev->type == BTDEV_TYPE_LE)
			goto unsupported;
		rcat.status = BT_HCI_ERR_SUCCESS;
		rcat.timeout = cpu_to_le16(btdev->conn_accept_timeout);
		cmd_complete(btdev, opcode, &rcat, sizeof(rcat));
		break;

	case BT_HCI_CMD_WRITE_CONN_ACCEPT_TIMEOUT:
		if (btdev->type == BTDEV_TYPE_LE)
			goto unsupported;
		wcat = data;
		btdev->conn_accept_timeout = le16_to_cpu(wcat->timeout);
		status = BT_HCI_ERR_SUCCESS;
		cmd_complete(btdev, opcode, &status, sizeof(status));
		break;

	case BT_HCI_CMD_READ_PAGE_TIMEOUT:
		if (btdev->type == BTDEV_TYPE_LE)
			goto unsupported;
		rpt.status = BT_HCI_ERR_SUCCESS;
		rpt.timeout = cpu_to_le16(btdev->page_timeout);
		cmd_complete(btdev, opcode, &rpt, sizeof(rpt));
		break;

	case BT_HCI_CMD_WRITE_PAGE_TIMEOUT:
		if (btdev->type == BTDEV_TYPE_LE)
			goto unsupported;
		wpt = data;
		btdev->page_timeout = le16_to_cpu(wpt->timeout);
		status = BT_HCI_ERR_SUCCESS;
		cmd_complete(btdev, opcode, &status, sizeof(status));
		break;

	case BT_HCI_CMD_READ_SCAN_ENABLE:
		if (btdev->type == BTDEV_TYPE_LE)
			goto unsupported;
		rse.status = BT_HCI_ERR_SUCCESS;
		rse.enable = btdev->scan_enable;
		cmd_complete(btdev, opcode, &rse, sizeof(rse));
		break;

	case BT_HCI_CMD_WRITE_SCAN_ENABLE:
		if (btdev->type == BTDEV_TYPE_LE)
			goto unsupported;
		wse = data;
		btdev->scan_enable = wse->enable;
		status = BT_HCI_ERR_SUCCESS;
		cmd_complete(btdev, opcode, &status, sizeof(status));
		break;

	case BT_HCI_CMD_READ_PAGE_SCAN_ACTIVITY:
		if (btdev->type == BTDEV_TYPE_LE)
			goto unsupported;
		rpsa.status = BT_HCI_ERR_SUCCESS;
		rpsa.interval = cpu_to_le16(btdev->page_scan_interval);
		rpsa.window = cpu_to_le16(btdev->page_scan_window);
		cmd_complete(btdev, opcode, &rpsa, sizeof(rpsa));
		break;

	case BT_HCI_CMD_WRITE_PAGE_SCAN_ACTIVITY:
		if (btdev->type == BTDEV_TYPE_LE)
			goto unsupported;
		wpsa = data;
		btdev->page_scan_interval = le16_to_cpu(wpsa->interval);
		btdev->page_scan_window = le16_to_cpu(wpsa->window);
		status = BT_HCI_ERR_SUCCESS;
		cmd_complete(btdev, opcode, &status, sizeof(status));
		break;

	case BT_HCI_CMD_READ_INQUIRY_SCAN_ACTIVITY:
		if (btdev->type == BTDEV_TYPE_LE)
			goto unsupported;
		risa.status = BT_HCI_ERR_SUCCESS;
		risa.interval = cpu_to_le16(btdev->inquiry_scan_interval);
		risa.window = cpu_to_le16(btdev->inquiry_scan_window);
		cmd_complete(btdev, opcode, &risa, sizeof(risa));
		break;

	case BT_HCI_CMD_WRITE_INQUIRY_SCAN_ACTIVITY:
		if (btdev->type == BTDEV_TYPE_LE)
			goto unsupported;
		wisa = data;
		btdev->inquiry_scan_interval = le16_to_cpu(wisa->interval);
		btdev->inquiry_scan_window = le16_to_cpu(wisa->window);
		status = BT_HCI_ERR_SUCCESS;
		cmd_complete(btdev, opcode, &status, sizeof(status));
		break;

	case BT_HCI_CMD_READ_PAGE_SCAN_TYPE:
		if (btdev->type == BTDEV_TYPE_LE)
			goto unsupported;
		rpst.status = BT_HCI_ERR_SUCCESS;
		rpst.type = btdev->page_scan_type;
		cmd_complete(btdev, opcode, &rpst, sizeof(rpst));
		break;

	case BT_HCI_CMD_WRITE_PAGE_SCAN_TYPE:
		if (btdev->type == BTDEV_TYPE_LE)
			goto unsupported;
		wpst = data;
		btdev->page_scan_type = wpst->type;
		status = BT_HCI_ERR_SUCCESS;
		cmd_complete(btdev, opcode, &status, sizeof(status));
		break;

	case BT_HCI_CMD_READ_AUTH_ENABLE:
		if (btdev->type == BTDEV_TYPE_LE)
			goto unsupported;
		rae.status = BT_HCI_ERR_SUCCESS;
		rae.enable = btdev->auth_enable;
		cmd_complete(btdev, opcode, &rae, sizeof(rae));
		break;

	case BT_HCI_CMD_WRITE_AUTH_ENABLE:
		if (btdev->type == BTDEV_TYPE_LE)
			goto unsupported;
		wae = data;
		btdev->auth_enable = wae->enable;
		status = BT_HCI_ERR_SUCCESS;
		cmd_complete(btdev, opcode, &status, sizeof(status));
		break;

	case BT_HCI_CMD_READ_CLASS_OF_DEV:
		if (btdev->type == BTDEV_TYPE_LE)
			goto unsupported;
		rcod.status = BT_HCI_ERR_SUCCESS;
		memcpy(rcod.dev_class, btdev->dev_class, 3);
		cmd_complete(btdev, opcode, &rcod, sizeof(rcod));
		break;

	case BT_HCI_CMD_WRITE_CLASS_OF_DEV:
		if (btdev->type == BTDEV_TYPE_LE)
			goto unsupported;
		wcod = data;
		memcpy(btdev->dev_class, wcod->dev_class, 3);
		status = BT_HCI_ERR_SUCCESS;
		cmd_complete(btdev, opcode, &status, sizeof(status));
		break;

	case BT_HCI_CMD_READ_VOICE_SETTING:
		if (btdev->type == BTDEV_TYPE_LE)
			goto unsupported;
		rvs.status = BT_HCI_ERR_SUCCESS;
		rvs.setting = cpu_to_le16(btdev->voice_setting);
		cmd_complete(btdev, opcode, &rvs, sizeof(rvs));
		break;

	case BT_HCI_CMD_WRITE_VOICE_SETTING:
		if (btdev->type == BTDEV_TYPE_LE)
			goto unsupported;
		wvs = data;
		btdev->voice_setting = le16_to_cpu(wvs->setting);
		status = BT_HCI_ERR_SUCCESS;
		cmd_complete(btdev, opcode, &status, sizeof(status));
		break;

	case BT_HCI_CMD_SET_HOST_FLOW_CONTROL:
		shfc = data;
		if (shfc->enable > 0x03) {
			status = BT_HCI_ERR_INVALID_PARAMETERS;
		} else {
			btdev->host_flow_control = shfc->enable;
			status = BT_HCI_ERR_SUCCESS;
		}
		cmd_complete(btdev, opcode, &status, sizeof(status));
		break;

	case BT_HCI_CMD_HOST_BUFFER_SIZE:
		status = BT_HCI_ERR_SUCCESS;
		cmd_complete(btdev, opcode, &status, sizeof(status));
		break;

	case BT_HCI_CMD_HOST_NUM_COMPLETED_PACKETS:
		/* This command is special in the sense that no event is
		 * normally generated after the command has completed.
		 */
		break;

	case BT_HCI_CMD_READ_NUM_SUPPORTED_IAC:
		if (btdev->type == BTDEV_TYPE_LE)
			goto unsupported;
		rnsi.status = BT_HCI_ERR_SUCCESS;
		rnsi.num_iac = 0x01;
		cmd_complete(btdev, opcode, &rnsi, sizeof(rnsi));
		break;

	case BT_HCI_CMD_READ_CURRENT_IAC_LAP:
		if (btdev->type == BTDEV_TYPE_LE)
			goto unsupported;
		rcil = alloca(sizeof(*rcil) + 3);
		rcil->status = BT_HCI_ERR_SUCCESS;
		rcil->num_iac = 0x01;
		rcil->iac_lap[0] = 0x33;
		rcil->iac_lap[1] = 0x8b;
		rcil->iac_lap[2] = 0x9e;
		cmd_complete(btdev, opcode, rcil, sizeof(*rcil) + 3);
		break;

	case BT_HCI_CMD_WRITE_CURRENT_IAC_LAP:
		if (btdev->type == BTDEV_TYPE_LE)
			goto unsupported;
		status = BT_HCI_ERR_SUCCESS;
		cmd_complete(btdev, opcode, &status, sizeof(status));
		break;

	case BT_HCI_CMD_READ_INQUIRY_MODE:
		if (btdev->type == BTDEV_TYPE_LE)
			goto unsupported;
		rim.status = BT_HCI_ERR_SUCCESS;
		rim.mode = btdev->inquiry_mode;
		cmd_complete(btdev, opcode, &rim, sizeof(rim));
		break;

	case BT_HCI_CMD_WRITE_INQUIRY_MODE:
		if (btdev->type == BTDEV_TYPE_LE)
			goto unsupported;
		wim = data;
		btdev->inquiry_mode = wim->mode;
		status = BT_HCI_ERR_SUCCESS;
		cmd_complete(btdev, opcode, &status, sizeof(status));
		break;

	case BT_HCI_CMD_READ_AFH_ASSESSMENT_MODE:
		if (btdev->type == BTDEV_TYPE_LE)
			goto unsupported;
		raam.status = BT_HCI_ERR_SUCCESS;
		raam.mode = btdev->afh_assessment_mode;
		cmd_complete(btdev, opcode, &raam, sizeof(raam));
		break;

	case BT_HCI_CMD_WRITE_AFH_ASSESSMENT_MODE:
		if (btdev->type == BTDEV_TYPE_LE)
			goto unsupported;
		waam = data;
		btdev->afh_assessment_mode = waam->mode;
		status = BT_HCI_ERR_SUCCESS;
		cmd_complete(btdev, opcode, &status, sizeof(status));
		break;

	case BT_HCI_CMD_READ_EXT_INQUIRY_RESPONSE:
		if (btdev->type == BTDEV_TYPE_LE)
			goto unsupported;
		reir.status = BT_HCI_ERR_SUCCESS;
		reir.fec = btdev->ext_inquiry_fec;
		memcpy(reir.data, btdev->ext_inquiry_rsp, 240);
		cmd_complete(btdev, opcode, &reir, sizeof(reir));
		break;

	case BT_HCI_CMD_WRITE_EXT_INQUIRY_RESPONSE:
		if (btdev->type == BTDEV_TYPE_LE)
			goto unsupported;
		weir = data;
		btdev->ext_inquiry_fec = weir->fec;
		memcpy(btdev->ext_inquiry_rsp, weir->data, 240);
		status = BT_HCI_ERR_SUCCESS;
		cmd_complete(btdev, opcode, &status, sizeof(status));
		break;

	case BT_HCI_CMD_READ_SIMPLE_PAIRING_MODE:
		if (btdev->type == BTDEV_TYPE_LE)
			goto unsupported;
		rspm.status = BT_HCI_ERR_SUCCESS;
		rspm.mode = btdev->simple_pairing_mode;
		cmd_complete(btdev, opcode, &rspm, sizeof(rspm));
		break;

	case BT_HCI_CMD_WRITE_SIMPLE_PAIRING_MODE:
		if (btdev->type == BTDEV_TYPE_LE)
			goto unsupported;
		wspm = data;
		btdev->simple_pairing_mode = wspm->mode;
		status = BT_HCI_ERR_SUCCESS;
		cmd_complete(btdev, opcode, &status, sizeof(status));
		break;

	case BT_HCI_CMD_IO_CAPABILITY_REQUEST_REPLY:
		if (btdev->type == BTDEV_TYPE_LE)
			goto unsupported;
		icrr = data;
		io_cap_req_reply_complete(btdev, icrr->bdaddr,
							icrr->capability,
							icrr->oob_data,
							icrr->authentication);
		break;

	case BT_HCI_CMD_IO_CAPABILITY_REQUEST_NEG_REPLY:
		if (btdev->type == BTDEV_TYPE_LE)
			goto unsupported;
		icrnr = data;
		io_cap_req_neg_reply_complete(btdev, icrnr->bdaddr);
		ssp_complete(btdev, icrnr->bdaddr, BT_HCI_ERR_AUTH_FAILURE,
									false);
		break;

	case BT_HCI_CMD_USER_CONFIRM_REQUEST_REPLY:
		if (btdev->type == BTDEV_TYPE_LE)
			goto unsupported;
		ucrr_rsp.status = BT_HCI_ERR_SUCCESS;
		memcpy(ucrr_rsp.bdaddr, data, 6);
		cmd_complete(btdev, opcode, &ucrr_rsp, sizeof(ucrr_rsp));
		ssp_complete(btdev, data, BT_HCI_ERR_SUCCESS, true);
		break;

	case BT_HCI_CMD_USER_CONFIRM_REQUEST_NEG_REPLY:
		if (btdev->type == BTDEV_TYPE_LE)
			goto unsupported;
		ucrnr_rsp.status = BT_HCI_ERR_SUCCESS;
		memcpy(ucrnr_rsp.bdaddr, data, 6);
		cmd_complete(btdev, opcode, &ucrnr_rsp, sizeof(ucrnr_rsp));
		ssp_complete(btdev, data, BT_HCI_ERR_AUTH_FAILURE, true);
		break;

	case BT_HCI_CMD_READ_LOCAL_OOB_DATA:
		if (btdev->type == BTDEV_TYPE_LE)
			goto unsupported;
		rlod.status = BT_HCI_ERR_SUCCESS;
		cmd_complete(btdev, opcode, &rlod, sizeof(rlod));
		break;

	case BT_HCI_CMD_READ_INQUIRY_RESP_TX_POWER:
		if (btdev->type == BTDEV_TYPE_LE)
			goto unsupported;
		rirtp.status = BT_HCI_ERR_SUCCESS;
		rirtp.level = 0;
		cmd_complete(btdev, opcode, &rirtp, sizeof(rirtp));
		break;

	case BT_HCI_CMD_READ_LE_HOST_SUPPORTED:
		if (btdev->type != BTDEV_TYPE_BREDRLE)
			goto unsupported;
		rlhs.status = BT_HCI_ERR_SUCCESS;
		rlhs.supported = btdev->le_supported;
		rlhs.simultaneous = btdev->le_simultaneous;
		cmd_complete(btdev, opcode, &rlhs, sizeof(rlhs));
		break;

	case BT_HCI_CMD_WRITE_LE_HOST_SUPPORTED:
		if (btdev->type != BTDEV_TYPE_BREDRLE)
			goto unsupported;
		wlhs = data;
		btdev->le_supported = wlhs->supported;
		btdev->le_simultaneous = wlhs->simultaneous;
		status = BT_HCI_ERR_SUCCESS;
		cmd_complete(btdev, opcode, &status, sizeof(status));
		break;

	case BT_HCI_CMD_READ_SECURE_CONN_SUPPORT:
		if (btdev->type != BTDEV_TYPE_BREDRLE)
			goto unsupported;
		rscs.status = BT_HCI_ERR_SUCCESS;
		rscs.support = btdev->secure_conn_support;
		cmd_complete(btdev, opcode, &rscs, sizeof(rscs));
		break;

	case BT_HCI_CMD_WRITE_SECURE_CONN_SUPPORT:
		if (btdev->type != BTDEV_TYPE_BREDRLE)
			goto unsupported;
		wscs = data;
		btdev->secure_conn_support = wscs->support;
		status = BT_HCI_ERR_SUCCESS;
		cmd_complete(btdev, opcode, &status, sizeof(status));
		break;

	case BT_HCI_CMD_READ_LOCAL_OOB_EXT_DATA:
		if (btdev->type != BTDEV_TYPE_BREDRLE)
			goto unsupported;
		rloed.status = BT_HCI_ERR_SUCCESS;
		cmd_complete(btdev, opcode, &rloed, sizeof(rloed));
		break;

	case BT_HCI_CMD_READ_SYNC_TRAIN_PARAMS:
		if (btdev->type != BTDEV_TYPE_BREDRLE)
			goto unsupported;
		rstp.status = BT_HCI_ERR_SUCCESS;
		rstp.interval = cpu_to_le16(btdev->sync_train_interval);
		rstp.timeout = cpu_to_le32(btdev->sync_train_timeout);
		rstp.service_data = btdev->sync_train_service_data;
		cmd_complete(btdev, opcode, &rstp, sizeof(rstp));
		break;

	case BT_HCI_CMD_READ_LOCAL_VERSION:
		rlv.status = BT_HCI_ERR_SUCCESS;
		rlv.hci_ver = btdev->version;
		rlv.hci_rev = cpu_to_le16(btdev->revision);
		rlv.lmp_ver = btdev->version;
		rlv.manufacturer = cpu_to_le16(btdev->manufacturer);
		rlv.lmp_subver = cpu_to_le16(btdev->revision);
		cmd_complete(btdev, opcode, &rlv, sizeof(rlv));
		break;

	case BT_HCI_CMD_READ_LOCAL_COMMANDS:
		rlc.status = BT_HCI_ERR_SUCCESS;
		memcpy(rlc.commands, btdev->commands, 64);
		cmd_complete(btdev, opcode, &rlc, sizeof(rlc));
		break;

	case BT_HCI_CMD_READ_LOCAL_FEATURES:
		rlf.status = BT_HCI_ERR_SUCCESS;
		memcpy(rlf.features, btdev->features, 8);
		cmd_complete(btdev, opcode, &rlf, sizeof(rlf));
		break;

	case BT_HCI_CMD_READ_LOCAL_EXT_FEATURES:
		if (btdev->type == BTDEV_TYPE_LE)
			goto unsupported;

		page = ((const uint8_t *) data)[0];

		rlef.page = page;
		rlef.max_page = btdev->max_page;

		if (page > btdev->max_page) {
			rlef.status = BT_HCI_ERR_INVALID_PARAMETERS;
			memset(rlef.features, 0, 8);
			cmd_complete(btdev, opcode, &rlef, sizeof(rlef));
			break;
		}

		switch (page) {
		case 0x00:
			rlef.status = BT_HCI_ERR_SUCCESS;
			memcpy(rlef.features, btdev->features, 8);
			break;
		case 0x01:
			rlef.status = BT_HCI_ERR_SUCCESS;
			btdev_get_host_features(btdev, rlef.features);
			break;
		case 0x02:
			rlef.status = BT_HCI_ERR_SUCCESS;
			memcpy(rlef.features, btdev->feat_page_2, 8);
			break;
		default:
			rlef.status = BT_HCI_ERR_INVALID_PARAMETERS;
			memset(rlef.features, 0, 8);
			break;
		}
		cmd_complete(btdev, opcode, &rlef, sizeof(rlef));
		break;

	case BT_HCI_CMD_READ_BUFFER_SIZE:
		rbs.status = BT_HCI_ERR_SUCCESS;
		rbs.acl_mtu = cpu_to_le16(btdev->acl_mtu);
		rbs.sco_mtu = 0;
		rbs.acl_max_pkt = cpu_to_le16(btdev->acl_max_pkt);
		rbs.sco_max_pkt = cpu_to_le16(0);
		cmd_complete(btdev, opcode, &rbs, sizeof(rbs));
		break;

	case BT_HCI_CMD_READ_COUNTRY_CODE:
		rcc.status = BT_HCI_ERR_SUCCESS;
		rcc.code = btdev->country_code;
		cmd_complete(btdev, opcode, &rcc, sizeof(rcc));
		break;

	case BT_HCI_CMD_READ_BD_ADDR:
		rba.status = BT_HCI_ERR_SUCCESS;
		memcpy(rba.bdaddr, btdev->bdaddr, 6);
		cmd_complete(btdev, opcode, &rba, sizeof(rba));
		break;

	case BT_HCI_CMD_READ_DATA_BLOCK_SIZE:
		if (btdev->type == BTDEV_TYPE_LE)
			goto unsupported;
		rdbs.status = BT_HCI_ERR_SUCCESS;
		rdbs.max_acl_len = cpu_to_le16(btdev->acl_mtu);
		rdbs.block_len = cpu_to_le16(btdev->acl_mtu);
		rdbs.num_blocks = cpu_to_le16(btdev->acl_max_pkt);
		cmd_complete(btdev, opcode, &rdbs, sizeof(rdbs));
		break;

	case BT_HCI_CMD_READ_LOCAL_CODECS:
		if (btdev->type == BTDEV_TYPE_LE)
			goto unsupported;
		rlsc = alloca(sizeof(*rlsc) + 7);
		rlsc->status = BT_HCI_ERR_SUCCESS;
		rlsc->num_codecs = 0x06;
		rlsc->codec[0] = 0x00;
		rlsc->codec[1] = 0x01;
		rlsc->codec[2] = 0x02;
		rlsc->codec[3] = 0x03;
		rlsc->codec[4] = 0x04;
		rlsc->codec[5] = 0x05;
		rlsc->codec[6] = 0x00;
		cmd_complete(btdev, opcode, rlsc, sizeof(*rlsc) + 7);
		break;

	case BT_HCI_CMD_READ_RSSI:
		rrssi = data;

		rrssi_rsp.status = BT_HCI_ERR_SUCCESS;
		rrssi_rsp.handle = rrssi->handle;
		rrssi_rsp.rssi = -1; /* non-zero so we can see it in tester */
		cmd_complete(btdev, opcode, &rrssi_rsp, sizeof(rrssi_rsp));
		break;

	case BT_HCI_CMD_READ_TX_POWER:
		rtxp = data;

		switch (rtxp->type) {
		case 0x00:
			rtxp_rsp.status = BT_HCI_ERR_SUCCESS;
			rtxp_rsp.level =  -1; /* non-zero */
			break;

		case 0x01:
			rtxp_rsp.status = BT_HCI_ERR_SUCCESS;
			rtxp_rsp.level = 4; /* max for class 2 radio */
			break;

		default:
			rtxp_rsp.level = 0;
			rtxp_rsp.status = BT_HCI_ERR_INVALID_PARAMETERS;
			break;
		}

		rtxp_rsp.handle = rtxp->handle;
		cmd_complete(btdev, opcode, &rtxp_rsp, sizeof(rtxp_rsp));
		break;

	case BT_HCI_CMD_READ_ENCRYPT_KEY_SIZE:
		if (btdev->type != BTDEV_TYPE_BREDRLE &&
					btdev->type != BTDEV_TYPE_BREDR)
			goto unsupported;
		reks = data;
		read_enc_key_size_complete(btdev, le16_to_cpu(reks->handle));
		break;

	case BT_HCI_CMD_READ_LOCAL_AMP_INFO:
		if (btdev->type != BTDEV_TYPE_AMP)
			goto unsupported;
		rlai.status = BT_HCI_ERR_SUCCESS;
		rlai.amp_status = 0x01;		/* Used for Bluetooth only */
		rlai.total_bw = cpu_to_le32(0);
		rlai.max_bw = cpu_to_le32(0);
		rlai.min_latency = cpu_to_le32(0);
		rlai.max_pdu = cpu_to_le32(672);
		rlai.amp_type = 0x01;		/* 802.11 AMP Controller */
		rlai.pal_cap = cpu_to_le16(0x0000);
		rlai.max_assoc_len = cpu_to_le16(672);
		rlai.max_flush_to = cpu_to_le32(0xffffffff);
		rlai.be_flush_to = cpu_to_le32(0xffffffff);
		cmd_complete(btdev, opcode, &rlai, sizeof(rlai));
		break;

	case BT_HCI_CMD_READ_LOCAL_AMP_ASSOC:
		if (btdev->type != BTDEV_TYPE_AMP)
			goto unsupported;
		rlaa_cmd = data;
		rlaa_rsp.status = BT_HCI_ERR_SUCCESS;
		rlaa_rsp.phy_handle = rlaa_cmd->phy_handle;
		rlaa_rsp.remain_assoc_len = cpu_to_le16(1);
		rlaa_rsp.assoc_fragment[0] = 0x42;
		memset(rlaa_rsp.assoc_fragment + 1, 0,
					sizeof(rlaa_rsp.assoc_fragment) - 1);
		cmd_complete(btdev, opcode, &rlaa_rsp, sizeof(rlaa_rsp));
		break;

	case BT_HCI_CMD_GET_MWS_TRANSPORT_CONFIG:
		if (btdev->type == BTDEV_TYPE_LE)
			goto unsupported;
		gmtc = alloca(sizeof(*gmtc));
		gmtc->status = BT_HCI_ERR_SUCCESS;
		gmtc->num_transports = 0x00;
		cmd_complete(btdev, opcode, gmtc, sizeof(*gmtc));
		break;

	case BT_HCI_CMD_SET_EVENT_MASK_PAGE2:
		if (btdev->type != BTDEV_TYPE_BREDRLE)
			goto unsupported;
		semp2 = data;
		memcpy(btdev->event_mask_page2, semp2->mask, 8);
		status = BT_HCI_ERR_SUCCESS;
		cmd_complete(btdev, opcode, &status, sizeof(status));
		break;

	case BT_HCI_CMD_LE_SET_EVENT_MASK:
		if (btdev->type == BTDEV_TYPE_BREDR)
			goto unsupported;
		lsem = data;
		memcpy(btdev->le_event_mask, lsem->mask, 8);
		status = BT_HCI_ERR_SUCCESS;
		cmd_complete(btdev, opcode, &status, sizeof(status));
		break;

	case BT_HCI_CMD_LE_READ_BUFFER_SIZE:
		if (btdev->type == BTDEV_TYPE_BREDR)
			goto unsupported;
		lrbs.status = BT_HCI_ERR_SUCCESS;
		lrbs.le_mtu = cpu_to_le16(btdev->acl_mtu);
		lrbs.le_max_pkt = btdev->acl_max_pkt;
		cmd_complete(btdev, opcode, &lrbs, sizeof(lrbs));
		break;

	case BT_HCI_CMD_LE_READ_LOCAL_FEATURES:
		if (btdev->type == BTDEV_TYPE_BREDR)
			goto unsupported;
		lrlf.status = BT_HCI_ERR_SUCCESS;
		memcpy(lrlf.features, btdev->le_features, 8);
		cmd_complete(btdev, opcode, &lrlf, sizeof(lrlf));
		break;

	case BT_HCI_CMD_LE_SET_RANDOM_ADDRESS:
		if (btdev->type == BTDEV_TYPE_BREDR)
			goto unsupported;
		lsra = data;
		memcpy(btdev->random_addr, lsra->addr, 6);
		status = BT_HCI_ERR_SUCCESS;
		cmd_complete(btdev, opcode, &status, sizeof(status));
		break;

	case BT_HCI_CMD_LE_SET_ADV_PARAMETERS:
		if (btdev->type == BTDEV_TYPE_BREDR)
			goto unsupported;

		if (btdev->le_adv_enable) {
			status = BT_HCI_ERR_COMMAND_DISALLOWED;
			cmd_complete(btdev, opcode, &status, sizeof(status));
			break;
		}

		lsap = data;
		btdev->le_adv_type = lsap->type;
		btdev->le_adv_own_addr = lsap->own_addr_type;
		btdev->le_adv_direct_addr_type = lsap->direct_addr_type;
		memcpy(btdev->le_adv_direct_addr, lsap->direct_addr, 6);

		status = BT_HCI_ERR_SUCCESS;
		cmd_complete(btdev, opcode, &status, sizeof(status));
		break;

	case BT_HCI_CMD_LE_READ_ADV_TX_POWER:
		if (btdev->type == BTDEV_TYPE_BREDR)
			goto unsupported;
		lratp.status = BT_HCI_ERR_SUCCESS;
		lratp.level = 0;
		cmd_complete(btdev, opcode, &lratp, sizeof(lratp));
		break;

	case BT_HCI_CMD_LE_SET_ADV_ENABLE:
		if (btdev->type == BTDEV_TYPE_BREDR)
			goto unsupported;
		lsae = data;
		if (btdev->le_adv_enable == lsae->enable)
			status = BT_HCI_ERR_COMMAND_DISALLOWED;
		else {
			btdev->le_adv_enable = lsae->enable;
			status = BT_HCI_ERR_SUCCESS;
		}
		cmd_complete(btdev, opcode, &status, sizeof(status));
		if (status == BT_HCI_ERR_SUCCESS && btdev->le_adv_enable)
			le_set_adv_enable_complete(btdev);
		break;

	case BT_HCI_CMD_LE_SET_SCAN_PARAMETERS:
		if (btdev->type == BTDEV_TYPE_BREDR)
			goto unsupported;

		lssp = data;

		if (btdev->le_scan_enable)
			status = BT_HCI_ERR_COMMAND_DISALLOWED;
		else {
			status = BT_HCI_ERR_SUCCESS;
			btdev->le_scan_type = lssp->type;
			btdev->le_scan_own_addr_type = lssp->own_addr_type;
		}

		cmd_complete(btdev, opcode, &status, sizeof(status));
		break;

	case BT_HCI_CMD_LE_SET_SCAN_ENABLE:
		if (btdev->type == BTDEV_TYPE_BREDR)
			goto unsupported;
		lsse = data;
		if (btdev->le_scan_enable == lsse->enable)
			status = BT_HCI_ERR_COMMAND_DISALLOWED;
		else {
			btdev->le_scan_enable = lsse->enable;
			btdev->le_filter_dup = lsse->filter_dup;
			status = BT_HCI_ERR_SUCCESS;
		}
		cmd_complete(btdev, opcode, &status, sizeof(status));
		break;

	case BT_HCI_CMD_LE_CREATE_CONN:
		if (btdev->type == BTDEV_TYPE_BREDR)
			goto unsupported;
		cmd_status(btdev, BT_HCI_ERR_SUCCESS, opcode);
		break;

	case BT_HCI_CMD_LE_READ_WHITE_LIST_SIZE:
		if (btdev->type == BTDEV_TYPE_BREDR)
			goto unsupported;
		lrwls.status = BT_HCI_ERR_SUCCESS;
		lrwls.size = 0;
		cmd_complete(btdev, opcode, &lrwls, sizeof(lrwls));
		break;

	case BT_HCI_CMD_LE_CONN_UPDATE:
		if (btdev->type == BTDEV_TYPE_BREDR)
			goto unsupported;
		cmd_status(btdev, BT_HCI_ERR_SUCCESS, opcode);
		break;

	case BT_HCI_CMD_LE_CLEAR_WHITE_LIST:
		if (btdev->type == BTDEV_TYPE_BREDR)
			goto unsupported;
		status = BT_HCI_ERR_SUCCESS;
		cmd_complete(btdev, opcode, &status, sizeof(status));
		break;

	case BT_HCI_CMD_LE_ENCRYPT:
		if (btdev->type == BTDEV_TYPE_BREDR)
			goto unsupported;
		lenc_cmd = data;
		if (!bt_crypto_e(btdev->crypto, lenc_cmd->key,
				 lenc_cmd->plaintext, lenc.data)) {
			cmd_status(btdev, BT_HCI_ERR_COMMAND_DISALLOWED,
				   opcode);
			break;
		}
		lenc.status = BT_HCI_ERR_SUCCESS;
		cmd_complete(btdev, opcode, &lenc, sizeof(lenc));
		break;

	case BT_HCI_CMD_LE_RAND:
		if (btdev->type == BTDEV_TYPE_BREDR)
			goto unsupported;
		if (!bt_crypto_random_bytes(btdev->crypto,
					    (uint8_t *)&lr.number, 8)) {
			cmd_status(btdev, BT_HCI_ERR_COMMAND_DISALLOWED,
				   opcode);
			break;
		}
		lr.status = BT_HCI_ERR_SUCCESS;
		cmd_complete(btdev, opcode, &lr, sizeof(lr));
		break;

	case BT_HCI_CMD_LE_READ_LOCAL_PK256:
		if (btdev->type == BTDEV_TYPE_BREDR)
			goto unsupported;
		if (!ecc_make_key(pk_evt.local_pk256, btdev->le_local_sk256)) {
			cmd_status(btdev, BT_HCI_ERR_COMMAND_DISALLOWED,
									opcode);
			break;
		}
		cmd_status(btdev, BT_HCI_ERR_SUCCESS,
						BT_HCI_CMD_LE_READ_LOCAL_PK256);
		pk_evt.status = BT_HCI_ERR_SUCCESS;
		le_meta_event(btdev, BT_HCI_EVT_LE_READ_LOCAL_PK256_COMPLETE,
						&pk_evt, sizeof(pk_evt));
		break;

	case BT_HCI_CMD_LE_GENERATE_DHKEY:
		if (btdev->type == BTDEV_TYPE_BREDR)
			goto unsupported;
		dh = data;
		if (!ecdh_shared_secret(dh->remote_pk256, btdev->le_local_sk256,
								dh_evt.dhkey)) {
			cmd_status(btdev, BT_HCI_ERR_COMMAND_DISALLOWED,
									opcode);
			break;
		}
		cmd_status(btdev, BT_HCI_ERR_SUCCESS,
						BT_HCI_CMD_LE_GENERATE_DHKEY);
		dh_evt.status = BT_HCI_ERR_SUCCESS;
		le_meta_event(btdev, BT_HCI_EVT_LE_GENERATE_DHKEY_COMPLETE,
						&dh_evt, sizeof(dh_evt));
		break;

	case BT_HCI_CMD_LE_READ_SUPPORTED_STATES:
		if (btdev->type == BTDEV_TYPE_BREDR)
			goto unsupported;
		lrss.status = BT_HCI_ERR_SUCCESS;
		memcpy(lrss.states, btdev->le_states, 8);
		cmd_complete(btdev, opcode, &lrss, sizeof(lrss));
		break;

	case BT_HCI_CMD_LE_SET_ADV_DATA:
		if (btdev->type == BTDEV_TYPE_BREDR)
			goto unsupported;
		lsad = data;
		btdev->le_adv_data_len = lsad->len;
		memcpy(btdev->le_adv_data, lsad->data, 31);
		status = BT_HCI_ERR_SUCCESS;
		cmd_complete(btdev, opcode, &status, sizeof(status));
		break;

	case BT_HCI_CMD_LE_SET_SCAN_RSP_DATA:
		if (btdev->type == BTDEV_TYPE_BREDR)
			goto unsupported;
		lssrd = data;
		btdev->le_scan_data_len = lssrd->len;
		memcpy(btdev->le_scan_data, lssrd->data, 31);
		status = BT_HCI_ERR_SUCCESS;
		cmd_complete(btdev, opcode, &status, sizeof(status));
		break;

	case BT_HCI_CMD_LE_READ_REMOTE_FEATURES:
		if (btdev->type == BTDEV_TYPE_BREDR)
			goto unsupported;
		le_read_remote_features_complete(btdev);
		break;

	case BT_HCI_CMD_LE_START_ENCRYPT:
		if (btdev->type == BTDEV_TYPE_BREDR)
			goto unsupported;
		lse = data;
		memcpy(btdev->le_ltk, lse->ltk, 16);
		le_start_encrypt_complete(btdev, lse->ediv, lse->rand);
		break;

	case BT_HCI_CMD_LE_LTK_REQ_REPLY:
		if (btdev->type == BTDEV_TYPE_BREDR)
			goto unsupported;
		llrr = data;
		memcpy(btdev->le_ltk, llrr->ltk, 16);
		le_encrypt_complete(btdev);
		break;

	case BT_HCI_CMD_LE_LTK_REQ_NEG_REPLY:
		if (btdev->type == BTDEV_TYPE_BREDR)
			goto unsupported;
		ltk_neg_reply_complete(btdev);
		break;

	case BT_HCI_CMD_SETUP_SYNC_CONN:
		if (btdev->type == BTDEV_TYPE_LE)
			goto unsupported;
		ssc = data;
		status = BT_HCI_ERR_SUCCESS;
		cmd_status(btdev, BT_HCI_ERR_SUCCESS, opcode);
		sync_conn_complete(btdev, ssc->voice_setting,
							BT_HCI_ERR_SUCCESS);
		break;

	case BT_HCI_CMD_ADD_SCO_CONN:
		if (btdev->type == BTDEV_TYPE_LE)
			goto unsupported;
		sco_conn_complete(btdev, BT_HCI_ERR_SUCCESS);
		break;

	case BT_HCI_CMD_ENABLE_DUT_MODE:
		status = BT_HCI_ERR_SUCCESS;
		cmd_complete(btdev, opcode, &status, sizeof(status));
		break;

	case BT_HCI_CMD_WRITE_SSP_DEBUG_MODE:
		if (btdev->type == BTDEV_TYPE_LE)
			goto unsupported;
		wsdm = data;
		btdev->ssp_debug_mode = wsdm->mode;
		status = BT_HCI_ERR_SUCCESS;
		cmd_complete(btdev, opcode, &status, sizeof(status));
		break;

	case BT_HCI_CMD_LE_RECEIVER_TEST:
		if (btdev->type == BTDEV_TYPE_BREDR)
			goto unsupported;
		status = BT_HCI_ERR_SUCCESS;
		cmd_complete(btdev, opcode, &status, sizeof(status));
		break;

	case BT_HCI_CMD_LE_TRANSMITTER_TEST:
		if (btdev->type == BTDEV_TYPE_BREDR)
			goto unsupported;
		status = BT_HCI_ERR_SUCCESS;
		cmd_complete(btdev, opcode, &status, sizeof(status));
		break;

	case BT_HCI_CMD_LE_TEST_END:
		if (btdev->type == BTDEV_TYPE_BREDR)
			goto unsupported;
		lte.status = BT_HCI_ERR_SUCCESS;
		lte.num_packets = 0;
		cmd_complete(btdev, opcode, &lte, sizeof(lte));
		break;

	case BT_HCI_CMD_LE_CONN_PARAM_REQ_REPLY:
		if (btdev->type == BTDEV_TYPE_BREDR)
			goto unsupported;
		lcprr_cmd = data;
		lcprr_rsp.handle = lcprr_cmd->handle;
		lcprr_rsp.status = BT_HCI_ERR_SUCCESS;
		cmd_complete(btdev, opcode, &lcprr_rsp, sizeof(lcprr_rsp));
		break;
	case BT_HCI_CMD_LE_CONN_PARAM_REQ_NEG_REPLY:
		if (btdev->type == BTDEV_TYPE_BREDR)
			goto unsupported;
		lcprnr_cmd = data;
		lcprnr_rsp.handle = lcprnr_cmd->handle;
		lcprnr_rsp.status = BT_HCI_ERR_SUCCESS;
		cmd_complete(btdev, opcode, &lcprnr_rsp, sizeof(lcprnr_rsp));
		break;
	default:
		goto unsupported;
	}

	return;

unsupported:
	printf("Unsupported command 0x%4.4x\n", opcode);
	hexdump(data, len);
	cmd_status(btdev, BT_HCI_ERR_UNKNOWN_COMMAND, opcode);
}

static void default_cmd_completion(struct btdev *btdev, uint16_t opcode,
						const void *data, uint8_t len)
{
	const struct bt_hci_cmd_create_conn *cc;
	const struct bt_hci_cmd_disconnect *dc;
	const struct bt_hci_cmd_create_conn_cancel *ccc;
	const struct bt_hci_cmd_accept_conn_request *acr;
	const struct bt_hci_cmd_reject_conn_request *rcr;
	const struct bt_hci_cmd_auth_requested *ar;
	const struct bt_hci_cmd_set_conn_encrypt *sce;
	const struct bt_hci_cmd_link_key_request_reply *lkrr;
	const struct bt_hci_cmd_link_key_request_neg_reply *lkrnr;
	const struct bt_hci_cmd_pin_code_request_neg_reply *pcrnr;
	const struct bt_hci_cmd_pin_code_request_reply *pcrr;
	const struct bt_hci_cmd_remote_name_request *rnr;
	const struct bt_hci_cmd_remote_name_request_cancel *rnrc;
	const struct bt_hci_cmd_read_remote_features *rrf;
	const struct bt_hci_cmd_read_remote_ext_features *rref;
	const struct bt_hci_cmd_read_remote_version *rrv;
	const struct bt_hci_cmd_read_clock_offset *rco;
	const struct bt_hci_cmd_le_create_conn *lecc;
	const struct bt_hci_cmd_le_conn_update *lecu;
	const struct bt_hci_cmd_le_conn_param_req_reply *lcprr;
	const struct bt_hci_cmd_le_conn_param_req_neg_reply *lcprnr;
	const struct bt_hci_cmd_le_set_scan_enable *lsse;

	switch (opcode) {
	case BT_HCI_CMD_INQUIRY:
		if (btdev->type == BTDEV_TYPE_LE)
			return;
		inquiry_cmd(btdev, data);
		break;

	case BT_HCI_CMD_CREATE_CONN:
		if (btdev->type == BTDEV_TYPE_LE)
			return;
		cc = data;
		conn_request(btdev, cc->bdaddr);
		break;

	case BT_HCI_CMD_DISCONNECT:
		dc = data;
		disconnect_complete(btdev, le16_to_cpu(dc->handle), dc->reason);
		break;

	case BT_HCI_CMD_CREATE_CONN_CANCEL:
		if (btdev->type == BTDEV_TYPE_LE)
			return;
		ccc = data;
		conn_complete(btdev, ccc->bdaddr, BT_HCI_ERR_UNKNOWN_CONN_ID);
		break;

	case BT_HCI_CMD_ACCEPT_CONN_REQUEST:
		if (btdev->type == BTDEV_TYPE_LE)
			return;
		acr = data;
		accept_conn_request_complete(btdev, acr->bdaddr);
		break;

	case BT_HCI_CMD_REJECT_CONN_REQUEST:
		if (btdev->type == BTDEV_TYPE_LE)
			return;
		rcr = data;
		conn_complete(btdev, rcr->bdaddr, BT_HCI_ERR_UNKNOWN_CONN_ID);
		break;

	case BT_HCI_CMD_LINK_KEY_REQUEST_REPLY:
		if (btdev->type == BTDEV_TYPE_LE)
			return;
		lkrr = data;
		link_key_req_reply_complete(btdev, lkrr->bdaddr, lkrr->link_key);
		break;

	case BT_HCI_CMD_LINK_KEY_REQUEST_NEG_REPLY:
		if (btdev->type == BTDEV_TYPE_LE)
			return;
		lkrnr = data;
		link_key_req_neg_reply_complete(btdev, lkrnr->bdaddr);
		break;

	case BT_HCI_CMD_PIN_CODE_REQUEST_REPLY:
		if (btdev->type == BTDEV_TYPE_LE)
			return;
		pcrr = data;
		pin_code_req_reply_complete(btdev, pcrr->bdaddr, pcrr->pin_len,
							pcrr->pin_code);
		break;

	case BT_HCI_CMD_PIN_CODE_REQUEST_NEG_REPLY:
		if (btdev->type == BTDEV_TYPE_LE)
			return;
		pcrnr = data;
		pin_code_req_neg_reply_complete(btdev, pcrnr->bdaddr);
		break;

	case BT_HCI_CMD_AUTH_REQUESTED:
		if (btdev->type == BTDEV_TYPE_LE)
			return;
		ar = data;
		auth_request_complete(btdev, le16_to_cpu(ar->handle));
		break;

	case BT_HCI_CMD_SET_CONN_ENCRYPT:
		if (btdev->type == BTDEV_TYPE_LE)
			return;
		sce = data;
		if (btdev->conn) {
			uint8_t mode;

			if (!sce->encr_mode)
				mode = 0x00;
			else if (btdev->secure_conn_support &&
					btdev->conn->secure_conn_support)
				mode = 0x02;
			else
				mode = 0x01;

			encrypt_change(btdev, mode, BT_HCI_ERR_SUCCESS);
			encrypt_change(btdev->conn, mode, BT_HCI_ERR_SUCCESS);
		}
		break;

	case BT_HCI_CMD_REMOTE_NAME_REQUEST:
		if (btdev->type == BTDEV_TYPE_LE)
			return;
		rnr = data;
		name_request_complete(btdev, rnr->bdaddr, BT_HCI_ERR_SUCCESS);
		break;

	case BT_HCI_CMD_REMOTE_NAME_REQUEST_CANCEL:
		if (btdev->type == BTDEV_TYPE_LE)
			return;
		rnrc = data;
		name_request_complete(btdev, rnrc->bdaddr,
						BT_HCI_ERR_UNKNOWN_CONN_ID);
		break;

	case BT_HCI_CMD_READ_REMOTE_FEATURES:
		if (btdev->type == BTDEV_TYPE_LE)
			return;
		rrf = data;
		remote_features_complete(btdev, le16_to_cpu(rrf->handle));
		break;

	case BT_HCI_CMD_READ_REMOTE_EXT_FEATURES:
		if (btdev->type == BTDEV_TYPE_LE)
			return;
		rref = data;
		remote_ext_features_complete(btdev, le16_to_cpu(rref->handle),
								rref->page);
		break;

	case BT_HCI_CMD_READ_REMOTE_VERSION:
		rrv = data;
		remote_version_complete(btdev, le16_to_cpu(rrv->handle));
		break;

	case BT_HCI_CMD_READ_CLOCK_OFFSET:
		if (btdev->type == BTDEV_TYPE_LE)
			return;
		rco = data;
		remote_clock_offset_complete(btdev, le16_to_cpu(rco->handle));
		break;

	case BT_HCI_CMD_LE_CREATE_CONN:
		if (btdev->type == BTDEV_TYPE_BREDR)
			return;
		lecc = data;
		btdev->le_scan_own_addr_type = lecc->own_addr_type;
		le_conn_request(btdev, lecc);
		break;

	case BT_HCI_CMD_LE_CONN_UPDATE:
		if (btdev->type == BTDEV_TYPE_BREDR)
			return;
		lecu = data;
		if (btdev->le_features[0] & 0x02)
			le_conn_param_req(btdev, le16_to_cpu(lecu->handle),
					le16_to_cpu(lecu->min_interval),
					le16_to_cpu(lecu->max_interval),
					le16_to_cpu(lecu->latency),
					le16_to_cpu(lecu->supv_timeout),
					le16_to_cpu(lecu->min_length),
					le16_to_cpu(lecu->max_length));
		else
			le_conn_update(btdev, le16_to_cpu(lecu->handle),
					le16_to_cpu(lecu->min_interval),
					le16_to_cpu(lecu->max_interval),
					le16_to_cpu(lecu->latency),
					le16_to_cpu(lecu->supv_timeout),
					le16_to_cpu(lecu->min_length),
					le16_to_cpu(lecu->max_length));
		break;
	case BT_HCI_CMD_LE_CONN_PARAM_REQ_REPLY:
		if (btdev->type == BTDEV_TYPE_BREDR)
			return;
		lcprr = data;
		le_conn_update(btdev, le16_to_cpu(lcprr->handle),
				le16_to_cpu(lcprr->min_interval),
				le16_to_cpu(lcprr->max_interval),
				le16_to_cpu(lcprr->latency),
				le16_to_cpu(lcprr->supv_timeout),
				le16_to_cpu(lcprr->min_length),
				le16_to_cpu(lcprr->max_length));
		break;
	case BT_HCI_CMD_LE_CONN_PARAM_REQ_NEG_REPLY:
		if (btdev->type == BTDEV_TYPE_BREDR)
			return;
		lcprnr = data;
		rej_le_conn_update(btdev, le16_to_cpu(lcprnr->handle),
					le16_to_cpu(lcprnr->reason));
		break;
		break;
	case BT_HCI_CMD_LE_SET_SCAN_ENABLE:
		if (btdev->type == BTDEV_TYPE_BREDR)
			return;
		lsse = data;
		if (btdev->le_scan_enable && lsse->enable)
			le_set_scan_enable_complete(btdev);

	}
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

	switch (response) {
	case BTDEV_RESPONSE_DEFAULT:
		if (!run_hooks(btdev, BTDEV_HOOK_PRE_CMD, callback->opcode,
						callback->data, callback->len))
			return;
		default_cmd(btdev, callback->opcode,
					callback->data, callback->len);

		if (!run_hooks(btdev, BTDEV_HOOK_PRE_EVT, callback->opcode,
						callback->data, callback->len))
			return;
		default_cmd_completion(btdev, callback->opcode,
					callback->data, callback->len);
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

	if (len < sizeof(*hdr))
		return;

	callback.function = handler_callback;
	callback.user_data = btdev;
	callback.opcode = le16_to_cpu(hdr->opcode);
	callback.data = data + sizeof(*hdr);
	callback.len = hdr->plen;

	if (btdev->command_handler)
		btdev->command_handler(callback.opcode,
					callback.data, callback.len,
					&callback, btdev->command_data);
	else {
		if (!run_hooks(btdev, BTDEV_HOOK_PRE_CMD, callback.opcode,
						callback.data, callback.len))
			return;
		default_cmd(btdev, callback.opcode,
					callback.data, callback.len);

		if (!run_hooks(btdev, BTDEV_HOOK_PRE_EVT, callback.opcode,
						callback.data, callback.len))
			return;
		default_cmd_completion(btdev, callback.opcode,
					callback.data, callback.len);
	}
}

static void send_acl(struct btdev *conn, const void *data, uint16_t len)
{
	struct bt_hci_acl_hdr hdr;
	struct iovec iov[3];

	/* Packet type */
	iov[0].iov_base = (void *) data;
	iov[0].iov_len = 1;

	/* ACL_START_NO_FLUSH is only allowed from host to controller.
	 * From controller to host this should be converted to ACL_START.
	 */
	memcpy(&hdr, data + 1, sizeof(hdr));
	if (acl_flags(hdr.handle) == ACL_START_NO_FLUSH)
		hdr.handle = acl_handle_pack(acl_handle(hdr.handle), ACL_START);

	iov[1].iov_base = &hdr;
	iov[1].iov_len = sizeof(hdr);

	iov[2].iov_base = (void *) (data + 1 + sizeof(hdr));
	iov[2].iov_len = len - 1 - sizeof(hdr);

	send_packet(conn, iov, 3);
}

void btdev_receive_h4(struct btdev *btdev, const void *data, uint16_t len)
{
	uint8_t pkt_type;

	if (!btdev)
		return;

	if (len < 1)
		return;

	pkt_type = ((const uint8_t *) data)[0];

	switch (pkt_type) {
	case BT_H4_CMD_PKT:
		process_cmd(btdev, data + 1, len - 1);
		break;
	case BT_H4_ACL_PKT:
		if (btdev->conn)
			send_acl(btdev->conn, data, len);
		num_completed_packets(btdev);
		break;
	default:
		printf("Unsupported packet 0x%2.2x\n", pkt_type);
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

/*
 *
 *  BlueZ - Bluetooth protocol stack for Linux
 *
 *  Copyright (C) 2000-2002  Maxim Krasnyansky <maxk@qualcomm.com>
 *  Copyright (C) 2003-2011  Marcel Holtmann <marcel@holtmann.org>
 *
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
 *
 */

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include <stdio.h>
#include <errno.h>
#include <ctype.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>

#include "parser.h"
#include "lib/hci.h"
#include "lib/hci_lib.h"
#include "lib/amp.h"

static uint16_t manufacturer = DEFAULT_COMPID;

static inline uint16_t get_manufacturer(void)
{
	return (manufacturer == DEFAULT_COMPID ? parser.defcompid : manufacturer);
}

#define EVENT_NUM 77
static char *event_str[EVENT_NUM + 1] = {
	"Unknown",
	"Inquiry Complete",
	"Inquiry Result",
	"Connect Complete",
	"Connect Request",
	"Disconn Complete",
	"Auth Complete",
	"Remote Name Req Complete",
	"Encrypt Change",
	"Change Connection Link Key Complete",
	"Master Link Key Complete",
	"Read Remote Supported Features",
	"Read Remote Ver Info Complete",
	"QoS Setup Complete",
	"Command Complete",
	"Command Status",
	"Hardware Error",
	"Flush Occurred",
	"Role Change",
	"Number of Completed Packets",
	"Mode Change",
	"Return Link Keys",
	"PIN Code Request",
	"Link Key Request",
	"Link Key Notification",
	"Loopback Command",
	"Data Buffer Overflow",
	"Max Slots Change",
	"Read Clock Offset Complete",
	"Connection Packet Type Changed",
	"QoS Violation",
	"Page Scan Mode Change",
	"Page Scan Repetition Mode Change",
	"Flow Specification Complete",
	"Inquiry Result with RSSI",
	"Read Remote Extended Features",
	"Unknown",
	"Unknown",
	"Unknown",
	"Unknown",
	"Unknown",
	"Unknown",
	"Unknown",
	"Unknown",
	"Synchronous Connect Complete",
	"Synchronous Connect Changed",
	"Sniff Subrate",
	"Extended Inquiry Result",
	"Encryption Key Refresh Complete",
	"IO Capability Request",
	"IO Capability Response",
	"User Confirmation Request",
	"User Passkey Request",
	"Remote OOB Data Request",
	"Simple Pairing Complete",
	"Unknown",
	"Link Supervision Timeout Change",
	"Enhanced Flush Complete",
	"Unknown",
	"User Passkey Notification",
	"Keypress Notification",
	"Remote Host Supported Features Notification",
	"LE Meta Event",
	"Unknown",
	"Physical Link Complete",
	"Channel Selected",
	"Disconnection Physical Link Complete",
	"Physical Link Loss Early Warning",
	"Physical Link Recovery",
	"Logical Link Complete",
	"Disconnection Logical Link Complete",
	"Flow Spec Modify Complete",
	"Number Of Completed Data Blocks",
	"AMP Start Test",
	"AMP Test End",
	"AMP Receiver Report",
	"Short Range Mode Change Complete",
	"AMP Status Change",
};

#define LE_EV_NUM 5
static char *ev_le_meta_str[LE_EV_NUM + 1] = {
	"Unknown",
	"LE Connection Complete",
	"LE Advertising Report",
	"LE Connection Update Complete",
	"LE Read Remote Used Features Complete",
	"LE Long Term Key Request",
};

#define CMD_LINKCTL_NUM 60
static char *cmd_linkctl_str[CMD_LINKCTL_NUM + 1] = {
	"Unknown",
	"Inquiry",
	"Inquiry Cancel",
	"Periodic Inquiry Mode",
	"Exit Periodic Inquiry Mode",
	"Create Connection",
	"Disconnect",
	"Add SCO Connection",
	"Create Connection Cancel",
	"Accept Connection Request",
	"Reject Connection Request",
	"Link Key Request Reply",
	"Link Key Request Negative Reply",
	"PIN Code Request Reply",
	"PIN Code Request Negative Reply",
	"Change Connection Packet Type",
	"Unknown",
	"Authentication Requested",
	"Unknown",
	"Set Connection Encryption",
	"Unknown",
	"Change Connection Link Key",
	"Unknown",
	"Master Link Key",
	"Unknown",
	"Remote Name Request",
	"Remote Name Request Cancel",
	"Read Remote Supported Features",
	"Read Remote Extended Features",
	"Read Remote Version Information",
	"Unknown",
	"Read Clock Offset",
	"Read LMP Handle",
	"Unknown",
	"Unknown",
	"Unknown",
	"Unknown",
	"Unknown",
	"Unknown",
	"Unknown",
	"Setup Synchronous Connection",
	"Accept Synchronous Connection",
	"Reject Synchronous Connection",
	"IO Capability Request Reply",
	"User Confirmation Request Reply",
	"User Confirmation Request Negative Reply",
	"User Passkey Request Reply",
	"User Passkey Request Negative Reply",
	"Remote OOB Data Request Reply",
	"Unknown",
	"Unknown",
	"Remote OOB Data Request Negative Reply",
	"IO Capability Request Negative Reply",
	"Create Physical Link",
	"Accept Physical Link",
	"Disconnect Physical Link",
	"Create Logical Link",
	"Accept Logical Link",
	"Disconnect Logical Link",
	"Logical Link Cancel",
	"Flow Spec Modify",
};

#define CMD_LINKPOL_NUM 17
static char *cmd_linkpol_str[CMD_LINKPOL_NUM + 1] = {
	"Unknown",
	"Hold Mode",
	"Unknown",
	"Sniff Mode",
	"Exit Sniff Mode",
	"Park State",
	"Exit Park State",
	"QoS Setup",
	"Unknown",
	"Role Discovery",
	"Unknown",
	"Switch Role",
	"Read Link Policy Settings",
	"Write Link Policy Settings",
	"Read Default Link Policy Settings",
	"Write Default Link Policy Settings",
	"Flow Specification",
	"Sniff Subrating",
};

#define CMD_HOSTCTL_NUM 109
static char *cmd_hostctl_str[CMD_HOSTCTL_NUM + 1] = {
	"Unknown",
	"Set Event Mask",
	"Unknown",
	"Reset",
	"Unknown",
	"Set Event Filter",
	"Unknown",
	"Unknown",
	"Flush",
	"Read PIN Type ",
	"Write PIN Type",
	"Create New Unit Key",
	"Unknown",
	"Read Stored Link Key",
	"Unknown",
	"Unknown",
	"Unknown",
	"Write Stored Link Key",
	"Delete Stored Link Key",
	"Write Local Name",
	"Read Local Name",
	"Read Connection Accept Timeout",
	"Write Connection Accept Timeout",
	"Read Page Timeout",
	"Write Page Timeout",
	"Read Scan Enable",
	"Write Scan Enable",
	"Read Page Scan Activity",
	"Write Page Scan Activity",
	"Read Inquiry Scan Activity",
	"Write Inquiry Scan Activity",
	"Read Authentication Enable",
	"Write Authentication Enable",
	"Read Encryption Mode",
	"Write Encryption Mode",
	"Read Class of Device",
	"Write Class of Device",
	"Read Voice Setting",
	"Write Voice Setting",
	"Read Automatic Flush Timeout",
	"Write Automatic Flush Timeout",
	"Read Num Broadcast Retransmissions",
	"Write Num Broadcast Retransmissions",
	"Read Hold Mode Activity ",
	"Write Hold Mode Activity",
	"Read Transmit Power Level",
	"Read Synchronous Flow Control Enable",
	"Write Synchronous Flow Control Enable",
	"Unknown",
	"Set Host Controller To Host Flow Control",
	"Unknown",
	"Host Buffer Size",
	"Unknown",
	"Host Number of Completed Packets",
	"Read Link Supervision Timeout",
	"Write Link Supervision Timeout",
	"Read Number of Supported IAC",
	"Read Current IAC LAP",
	"Write Current IAC LAP",
	"Read Page Scan Period Mode",
	"Write Page Scan Period Mode",
	"Read Page Scan Mode",
	"Write Page Scan Mode",
	"Set AFH Host Channel Classification",
	"Unknown",
	"Unknown",
	"Read Inquiry Scan Type",
	"Write Inquiry Scan Type",
	"Read Inquiry Mode",
	"Write Inquiry Mode",
	"Read Page Scan Type",
	"Write Page Scan Type",
	"Read AFH Channel Assessment Mode",
	"Write AFH Channel Assessment Mode",
	"Unknown",
	"Unknown",
	"Unknown",
	"Unknown",
	"Unknown",
	"Unknown",
	"Unknown",
	"Read Extended Inquiry Response",
	"Write Extended Inquiry Response",
	"Refresh Encryption Key",
	"Unknown",
	"Read Simple Pairing Mode",
	"Write Simple Pairing Mode",
	"Read Local OOB Data",
	"Read Inquiry Response Transmit Power Level",
	"Write Inquiry Transmit Power Level",
	"Read Default Erroneous Data Reporting",
	"Write Default Erroneous Data Reporting",
	"Unknown",
	"Unknown",
	"Unknown",
	"Enhanced Flush",
	"Unknown",
	"Read Logical Link Accept Timeout",
	"Write Logical Link Accept Timeout",
	"Set Event Mask Page 2",
	"Read Location Data",
	"Write Location Data",
	"Read Flow Control Mode",
	"Write Flow Control Mode",
	"Read Enhanced Transmit Power Level",
	"Read Best Effort Flush Timeout",
	"Write Best Effort Flush Timeout",
	"Short Range Mode",
	"Read LE Host Supported",
	"Write LE Host Supported",
};

#define CMD_INFO_NUM 10
static char *cmd_info_str[CMD_INFO_NUM + 1] = {
	"Unknown",
	"Read Local Version Information",
	"Read Local Supported Commands",
	"Read Local Supported Features",
	"Read Local Extended Features",
	"Read Buffer Size",
	"Unknown",
	"Read Country Code",
	"Unknown",
	"Read BD ADDR",
	"Read Data Block Size",
};

#define CMD_STATUS_NUM 11
static char *cmd_status_str[CMD_STATUS_NUM + 1] = {
	"Unknown",
	"Read Failed Contact Counter",
	"Reset Failed Contact Counter",
	"Read Link Quality",
	"Unknown",
	"Read RSSI",
	"Read AFH Channel Map",
	"Read Clock",
	"Read Encryption Key Size",
	"Read Local AMP Info",
	"Read Local AMP ASSOC",
	"Write Remote AMP ASSOC"
};

#define CMD_TESTING_NUM 4
static char *cmd_testing_str[CMD_TESTING_NUM + 1] = {
	"Unknown",
	"Read Loopback Mode",
	"Write Loopback Mode",
	"Enable Device Under Test mode",
	"Unknown",
};

#define CMD_LE_NUM 31
static char *cmd_le_str[CMD_LE_NUM + 1] = {
	"Unknown",
	"LE Set Event Mask",
	"LE Read Buffer Size",
	"LE Read Local Supported Features",
	"Unknown",
	"LE Set Random Address",
	"LE Set Advertising Parameters",
	"LE Read Advertising Channel Tx Power",
	"LE Set Advertising Data",
	"LE Set Scan Response Data",
	"LE Set Advertise Enable",
	"LE Set Scan Parameters",
	"LE Set Scan Enable",
	"LE Create Connection",
	"LE Create Connection Cancel",
	"LE Read White List Size",
	"LE Clear White List",
	"LE Add Device To White List",
	"LE Remove Device From White List",
	"LE Connection Update",
	"LE Set Host Channel Classification",
	"LE Read Channel Map",
	"LE Read Remote Used Features",
	"LE Encrypt",
	"LE Rand",
	"LE Start Encryption",
	"LE Long Term Key Request Reply",
	"LE Long Term Key Request Negative Reply",
	"LE Read Supported States",
	"LE Receiver Test",
	"LE Transmitter Test",
	"LE Test End",
};

#define ERROR_CODE_NUM 63
static char *error_code_str[ERROR_CODE_NUM + 1] = {
	"Success",
	"Unknown HCI Command",
	"Unknown Connection Identifier",
	"Hardware Failure",
	"Page Timeout",
	"Authentication Failure",
	"PIN or Key Missing",
	"Memory Capacity Exceeded",
	"Connection Timeout",
	"Connection Limit Exceeded",
	"Synchronous Connection to a Device Exceeded",
	"ACL Connection Already Exists",
	"Command Disallowed",
	"Connection Rejected due to Limited Resources",
	"Connection Rejected due to Security Reasons",
	"Connection Rejected due to Unacceptable BD_ADDR",
	"Connection Accept Timeout Exceeded",
	"Unsupported Feature or Parameter Value",
	"Invalid HCI Command Parameters",
	"Remote User Terminated Connection",
	"Remote Device Terminated Connection due to Low Resources",
	"Remote Device Terminated Connection due to Power Off",
	"Connection Terminated by Local Host",
	"Repeated Attempts",
	"Pairing Not Allowed",
	"Unknown LMP PDU",
	"Unsupported Remote Feature / Unsupported LMP Feature",
	"SCO Offset Rejected",
	"SCO Interval Rejected",
	"SCO Air Mode Rejected",
	"Invalid LMP Parameters / Invalid LL Parameters",
	"Unspecified Error",
	"Unsupported LMP Parameter Value / Unsupported LL Parameter Value",
	"Role Change Not Allowed",
	"LMP Response Timeout",
	"LMP Error Transaction Collision",
	"LMP PDU Not Allowed",
	"Encryption Mode Not Acceptable",
	"Link Key Can Not be Changed",
	"Requested QoS Not Supported",
	"Instant Passed",
	"Pairing with Unit Key Not Supported",
	"Different Transaction Collision",
	"Reserved",
	"QoS Unacceptable Parameter",
	"QoS Rejected",
	"Channel Classification Not Supported",
	"Insufficient Security",
	"Parameter out of Mandatory Range",
	"Reserved",
	"Role Switch Pending",
	"Reserved",
	"Reserved Slot Violation",
	"Role Switch Failed",
	"Extended Inquiry Response Too Large",
	"Simple Pairing Not Supported by Host",
	"Host Busy - Pairing",
	"Connection Rejected due to No Suitable Channel Found",
	"Controller Busy",
	"Unacceptable Connection Parameters",
	"Directed Advertising Timeout",
	"Connection Terminated Due to MIC Failure",
	"Connection Failed to be Established",
	"MAC Connection Failed",
};

static char *status2str(uint8_t status)
{
	char *str;

	if (status <= ERROR_CODE_NUM)
		str = error_code_str[status];
	else
		str = "Unknown";

	return str;
}

static char *opcode2str(uint16_t opcode)
{
	uint16_t ogf = cmd_opcode_ogf(opcode);
	uint16_t ocf = cmd_opcode_ocf(opcode);
	char *cmd;

	switch (ogf) {
	case OGF_INFO_PARAM:
		if (ocf <= CMD_INFO_NUM)
			cmd = cmd_info_str[ocf];
		else
			cmd = "Unknown";
		break;

	case OGF_HOST_CTL:
		if (ocf <= CMD_HOSTCTL_NUM)
			cmd = cmd_hostctl_str[ocf];
		else
			cmd = "Unknown";
		break;

	case OGF_LINK_CTL:
		if (ocf <= CMD_LINKCTL_NUM)
			cmd = cmd_linkctl_str[ocf];
		else
			cmd = "Unknown";
		break;

	case OGF_LINK_POLICY:
		if (ocf <= CMD_LINKPOL_NUM)
			cmd = cmd_linkpol_str[ocf];
		else
			cmd = "Unknown";
		break;

	case OGF_STATUS_PARAM:
		if (ocf <= CMD_STATUS_NUM)
			cmd = cmd_status_str[ocf];
		else
			cmd = "Unknown";
		break;

	case OGF_TESTING_CMD:
		if (ocf <= CMD_TESTING_NUM)
			cmd = cmd_testing_str[ocf];
		else
			cmd = "Unknown";
		break;

	case OGF_LE_CTL:
		if (ocf <= CMD_LE_NUM)
			cmd = cmd_le_str[ocf];
		else
			cmd = "Unknown";
		break;

	case OGF_VENDOR_CMD:
		cmd = "Vendor";
		break;

	default:
		cmd = "Unknown";
		break;
	}

	return cmd;
}

static char *linktype2str(uint8_t type)
{
	switch (type) {
	case 0x00:
		return "SCO";
	case 0x01:
		return "ACL";
	case 0x02:
		return "eSCO";
	default:
		return "Unknown";
	}
}

static char *role2str(uint8_t role)
{
	switch (role) {
	case 0x00:
		return "Master";
	case 0x01:
		return "Slave";
	default:
		return "Unknown";
	}
}

static char *mode2str(uint8_t mode)
{
	switch (mode) {
	case 0x00:
		return "Active";
	case 0x01:
		return "Hold";
	case 0x02:
		return "Sniff";
	case 0x03:
		return "Park";
	default:
		return "Unknown";
	}
}

static char *airmode2str(uint8_t mode)
{
	switch (mode) {
	case 0x00:
		return "u-law log";
	case 0x01:
		return "A-law log";
	case 0x02:
		return "CVSD";
	case 0x04:
		return "Transparent data";
	default:
		return "Reserved";
	}
}

static const char *bdaddrtype2str(uint8_t type)
{
	switch (type) {
	case 0x00:
		return "Public";
	case 0x01:
		return "Random";
	default:
		return "Reserved";
	}
}

static const char *evttype2str(uint8_t type)
{
	switch (type) {
	case 0x00:
		return "ADV_IND - Connectable undirected advertising";
	case 0x01:
		return "ADV_DIRECT_IND - Connectable directed advertising";
	case 0x02:
		return "ADV_SCAN_IND - Scannable undirected advertising";
	case 0x03:
		return "ADV_NONCONN_IND - Non connectable undirected advertising";
	case 0x04:
		return "SCAN_RSP - Scan Response";
	default:
		return "Reserved";
	}
}

static char *keytype2str(uint8_t type)
{
	switch (type) {
	case 0x00:
		return "Combination Key";
	case 0x01:
		return "Local Unit Key";
	case 0x02:
		return "Remote Unit Key";
	case 0x03:
		return "Debug Combination Key";
	case 0x04:
		return "Unauthenticated Combination Key";
	case 0x05:
		return "Authenticated Combination Key";
	case 0x06:
		return "Changed Combination Key";
	default:
		return "Reserved";
	}
}

static char *capability2str(uint8_t capability)
{
	switch (capability) {
	case 0x00:
		return "DisplayOnly";
	case 0x01:
		return "DisplayYesNo";
	case 0x02:
		return "KeyboardOnly";
	case 0x03:
		return "NoInputNoOutput";
	default:
		return "Reserved";
	}
}

static char *authentication2str(uint8_t authentication)
{
	switch (authentication) {
	case 0x00:
		return "No Bonding (No MITM Protection)";
	case 0x01:
		return "No Bonding (MITM Protection)";
	case 0x02:
		return "Dedicated Bonding (No MITM Protection)";
	case 0x03:
		return "Dedicated Bonding (MITM Protection)";
	case 0x04:
		return "General Bonding (No MITM Protection)";
	case 0x05:
		return "General Bonding (MITM Protection)";
	default:
		return "Reserved";
	}
}

static char *eventmask2str(const uint8_t mask[8])
{
	int i;

	for (i = 0; i < 7; i++) {
		if (mask[i] != 0x00)
			return "Reserved";
	}

	switch (mask[7]) {
	case 0x00:
		return "No LE events specified";
	case 0x01:
		return "LE Connection Complete Event";
	case 0x02:
		return "LE Advertising Report Event";
	case 0x04:
		return "LE Connection Update Complete Event";
	case 0x08:
		return "LE Read Remote Used Features Complete Event";
	case 0x10:
		return "LE Long Term Key Request Event";
	case 0x1F:
		return "Default";
	default:
		return "Reserved";
	}
}

static char *lefeatures2str(const uint8_t features[8])
{
	if (features[0] & 0x01)
		return "Link Layer supports LE Encryption";

	return "RFU";
}

static char *filterpolicy2str(uint8_t policy)
{
	switch (policy) {
	case 0x00:
		return "Allow scan from any, connection from any";
	case 0x01:
		return "Allow scan from white list, connection from any";
	case 0x02:
		return "Allow scan from any, connection from white list";
	case 0x03:
		return "Allow scan and connection from white list";
	default:
		return "Reserved";
	}
}

static inline void ext_inquiry_data_dump(int level, struct frame *frm,
						uint8_t *data)
{
	uint8_t len = data[0];
	uint8_t type;
	char *str;
	int i;

	if (len == 0)
		return;

	type = data[1];
	data += 2;
	len -= 1;

	switch (type) {
	case 0x01:
		p_indent(level, frm);
		printf("Flags:");
		for (i = 0; i < len; i++)
			printf(" 0x%2.2x", data[i]);
		printf("\n");
		break;

	case 0x02:
	case 0x03:
		p_indent(level, frm);
		printf("%s service classes:",
				type == 0x02 ? "Shortened" : "Complete");

		for (i = 0; i < len / 2; i++)
			printf(" 0x%4.4x", get_le16(data + i * 2));

		printf("\n");
		break;

	case 0x08:
	case 0x09:
		str = malloc(len + 1);
		if (str) {
			snprintf(str, len + 1, "%s", (char *) data);
			for (i = 0; i < len; i++)
				if (!isprint(str[i]))
					str[i] = '.';
			p_indent(level, frm);
			printf("%s local name: \'%s\'\n",
				type == 0x08 ? "Shortened" : "Complete", str);
			free(str);
		}
		break;

	case 0x0a:
		p_indent(level, frm);
		printf("TX power level: %d\n", *((uint8_t *) data));
		break;

	default:
		p_indent(level, frm);
		printf("Unknown type 0x%02x with %d bytes data\n",
							type, len);
		break;
	}
}

static inline void ext_inquiry_response_dump(int level, struct frame *frm)
{
	void *ptr = frm->ptr;
	uint32_t len = frm->len;
	uint8_t *data;
	uint8_t length;

	data = frm->ptr;
	length = p_get_u8(frm);

	while (length > 0) {
		ext_inquiry_data_dump(level, frm, data);

		frm->ptr += length;
		frm->len -= length;

		data = frm->ptr;
		length = p_get_u8(frm);
	}

	frm->ptr = ptr +
		(EXTENDED_INQUIRY_INFO_SIZE - INQUIRY_INFO_WITH_RSSI_SIZE);
	frm->len = len +
		(EXTENDED_INQUIRY_INFO_SIZE - INQUIRY_INFO_WITH_RSSI_SIZE);
}

static inline void bdaddr_command_dump(int level, struct frame *frm)
{
	bdaddr_t *bdaddr = frm->ptr;
	char addr[18];

	frm->ptr += sizeof(bdaddr_t);
	frm->len -= sizeof(bdaddr_t);

	p_indent(level, frm);
	p_ba2str(bdaddr, addr);
	printf("bdaddr %s\n", addr);

	raw_dump(level, frm);
}

static inline void generic_command_dump(int level, struct frame *frm)
{
	uint16_t handle = btohs(htons(p_get_u16(frm)));

	p_indent(level, frm);
	printf("handle %d\n", handle);

	raw_dump(level, frm);
}

static inline void generic_write_mode_dump(int level, struct frame *frm)
{
	uint8_t mode = p_get_u8(frm);

	p_indent(level, frm);
	printf("mode 0x%2.2x\n", mode);
}

static inline void inquiry_dump(int level, struct frame *frm)
{
	inquiry_cp *cp = frm->ptr;

	p_indent(level, frm);
	printf("lap 0x%2.2x%2.2x%2.2x len %d num %d\n",
		cp->lap[2], cp->lap[1], cp->lap[0], cp->length, cp->num_rsp);
}

static inline void periodic_inquiry_dump(int level, struct frame *frm)
{
	periodic_inquiry_cp *cp = frm->ptr;

	p_indent(level, frm);
	printf("max %d min %d lap 0x%2.2x%2.2x%2.2x len %d num %d\n",
		btohs(cp->max_period), btohs(cp->min_period),
		cp->lap[2], cp->lap[1], cp->lap[0], cp->length, cp->num_rsp);
}

static inline void create_conn_dump(int level, struct frame *frm)
{
	create_conn_cp *cp = frm->ptr;
	uint16_t ptype = btohs(cp->pkt_type);
	uint16_t clkoffset = btohs(cp->clock_offset);
	char addr[18], *str;

	p_indent(level, frm);
	p_ba2str(&cp->bdaddr, addr);
	printf("bdaddr %s ptype 0x%4.4x rswitch 0x%2.2x clkoffset 0x%4.4x%s\n",
		addr, ptype, cp->role_switch,
		clkoffset & 0x7fff, clkoffset & 0x8000 ? " (valid)" : "");

	str = hci_ptypetostr(ptype);
	if (str) {
		p_indent(level, frm);
		printf("Packet type: %s\n", str);
		free(str);
	}
}

static inline void disconnect_dump(int level, struct frame *frm)
{
	disconnect_cp *cp = frm->ptr;

	p_indent(level, frm);
	printf("handle %d reason 0x%2.2x\n", btohs(cp->handle), cp->reason);

	p_indent(level, frm);
	printf("Reason: %s\n", status2str(cp->reason));
}

static inline void add_sco_dump(int level, struct frame *frm)
{
	add_sco_cp *cp = frm->ptr;
	uint16_t ptype = btohs(cp->pkt_type);
	char *str;

	p_indent(level, frm);
	printf("handle %d ptype 0x%4.4x\n", btohs(cp->handle), ptype);

	str = hci_ptypetostr(ptype);
	if (str) {
		p_indent(level, frm);
		printf("Packet type: %s\n", str);
		free(str);
	}
}

static inline void accept_conn_req_dump(int level, struct frame *frm)
{
	accept_conn_req_cp *cp = frm->ptr;
	char addr[18];

	p_indent(level, frm);
	p_ba2str(&cp->bdaddr, addr);
	printf("bdaddr %s role 0x%2.2x\n", addr, cp->role);

	p_indent(level, frm);
	printf("Role: %s\n", role2str(cp->role));
}

static inline void reject_conn_req_dump(int level, struct frame *frm)
{
	reject_conn_req_cp *cp = frm->ptr;
	char addr[18];

	p_indent(level, frm);
	p_ba2str(&cp->bdaddr, addr);
	printf("bdaddr %s reason 0x%2.2x\n", addr, cp->reason);

	p_indent(level, frm);
	printf("Reason: %s\n", status2str(cp->reason));
}

static inline void pin_code_reply_dump(int level, struct frame *frm)
{
	pin_code_reply_cp *cp = frm->ptr;
	char addr[18], pin[17];

	p_indent(level, frm);
	p_ba2str(&cp->bdaddr, addr);
	memset(pin, 0, sizeof(pin));
	if (parser.flags & DUMP_NOVENDOR)
		memset(pin, '*', cp->pin_len);
	else
		memcpy(pin, cp->pin_code, cp->pin_len);
	printf("bdaddr %s len %d pin \'%s\'\n", addr, cp->pin_len, pin);
}

static inline void link_key_reply_dump(int level, struct frame *frm)
{
	link_key_reply_cp *cp = frm->ptr;
	char addr[18];
	int i;

	p_indent(level, frm);
	p_ba2str(&cp->bdaddr, addr);
	printf("bdaddr %s key ", addr);
	for (i = 0; i < 16; i++)
		if (parser.flags & DUMP_NOVENDOR)
			printf("**");
		else
			printf("%2.2X", cp->link_key[i]);
	printf("\n");
}

static inline void pin_code_neg_reply_dump(int level, struct frame *frm)
{
	bdaddr_t *bdaddr = frm->ptr;
	char addr[18];

	p_indent(level, frm);
	p_ba2str(bdaddr, addr);
	printf("bdaddr %s\n", addr);
}

static inline void user_passkey_reply_dump(int level, struct frame *frm)
{
	user_passkey_reply_cp *cp = frm->ptr;
	char addr[18];

	p_indent(level, frm);
	p_ba2str(&cp->bdaddr, addr);
	printf("bdaddr %s passkey %d\n", addr, btohl(cp->passkey));
}

static inline void remote_oob_data_reply_dump(int level, struct frame *frm)
{
	remote_oob_data_reply_cp *cp = frm->ptr;
	char addr[18];
	int i;

	p_indent(level, frm);
	p_ba2str(&cp->bdaddr, addr);
	printf("bdaddr %s\n", addr);

	p_indent(level, frm);
	printf("hash 0x");
	for (i = 0; i < 16; i++)
		printf("%02x", cp->hash[i]);
	printf("\n");

	p_indent(level, frm);
	printf("randomizer 0x");
	for (i = 0; i < 16; i++)
			printf("%02x", cp->randomizer[i]);
	printf("\n");
}

static inline void io_capability_reply_dump(int level, struct frame *frm)
{
	io_capability_reply_cp *cp = frm->ptr;
	char addr[18];

	p_indent(level, frm);
	p_ba2str(&cp->bdaddr, addr);
	printf("bdaddr %s capability 0x%2.2x oob 0x%2.2x auth 0x%2.2x\n",
					addr, cp->capability, cp->oob_data,
							cp->authentication);

	p_indent(level, frm);
	printf("Capability: %s (OOB data %s)\n",
			capability2str(cp->capability),
			cp->oob_data == 0x00 ? "not present" : "available");

	p_indent(level, frm);
	printf("Authentication: %s\n", authentication2str(cp->authentication));
}

static inline void set_conn_encrypt_dump(int level, struct frame *frm)
{
	set_conn_encrypt_cp *cp = frm->ptr;

	p_indent(level, frm);
	printf("handle %d encrypt 0x%2.2x\n", btohs(cp->handle), cp->encrypt);
}

static inline void remote_name_req_dump(int level, struct frame *frm)
{
	remote_name_req_cp *cp = frm->ptr;
	uint16_t clkoffset = btohs(cp->clock_offset);
	char addr[18];

	p_indent(level, frm);
	p_ba2str(&cp->bdaddr, addr);
	printf("bdaddr %s mode %d clkoffset 0x%4.4x%s\n",
		addr, cp->pscan_rep_mode,
		clkoffset & 0x7fff, clkoffset & 0x8000 ? " (valid)" : "");
}

static inline void master_link_key_dump(int level, struct frame *frm)
{
	master_link_key_cp *cp = frm->ptr;

	p_indent(level, frm);
	printf("flag %d\n", cp->key_flag);
}

static inline void read_remote_ext_features_dump(int level, struct frame *frm)
{
	read_remote_ext_features_cp *cp = frm->ptr;

	p_indent(level, frm);
	printf("handle %d page %d\n", btohs(cp->handle), cp->page_num);
}

static inline void setup_sync_conn_dump(int level, struct frame *frm)
{
	setup_sync_conn_cp *cp = frm->ptr;

	p_indent(level, frm);
	printf("handle %d voice setting 0x%4.4x ptype 0x%4.4x\n",
		btohs(cp->handle), btohs(cp->voice_setting),
		btohs(cp->pkt_type));
}

static inline void create_physical_link_dump(int level, struct frame *frm)
{
	create_physical_link_cp *cp = frm->ptr;
	int i;

	p_indent(level, frm);
	printf("phy handle 0x%2.2x key length %d key type %d\n",
		cp->handle, cp->key_length, cp->key_type);
	p_indent(level, frm);
	printf("key ");
	for (i = 0; i < cp->key_length && cp->key_length <= 32; i++)
		printf("%2.2x", cp->key[i]);
	printf("\n");
}

static inline void create_logical_link_dump(int level, struct frame *frm)
{
	create_logical_link_cp *cp = frm->ptr;
	int i;

	p_indent(level, frm);
	printf("phy handle 0x%2.2x\n", cp->handle);

	p_indent(level, frm);
	printf("tx_flow ");
	for (i = 0; i < 16; i++)
		printf("%2.2x", cp->tx_flow[i]);
	printf("\n");

	p_indent(level, frm);
	printf("rx_flow ");
	for (i = 0; i < 16; i++)
		printf("%2.2x", cp->rx_flow[i]);
	printf("\n");
}

static inline void hold_mode_dump(int level, struct frame *frm)
{
	hold_mode_cp *cp = frm->ptr;

	p_indent(level, frm);
	printf("handle %d max %d min %d\n", btohs(cp->handle),
			btohs(cp->max_interval), btohs(cp->min_interval));
}

static inline void sniff_mode_dump(int level, struct frame *frm)
{
	sniff_mode_cp *cp = frm->ptr;

	p_indent(level, frm);
	printf("handle %d max %d min %d attempt %d timeout %d\n",
		btohs(cp->handle), btohs(cp->max_interval),
		btohs(cp->min_interval), btohs(cp->attempt), btohs(cp->timeout));
}

static inline void qos_setup_dump(int level, struct frame *frm)
{
	qos_setup_cp *cp = frm->ptr;

	p_indent(level, frm);
	printf("handle %d flags 0x%2.2x\n", btohs(cp->handle), cp->flags);

	p_indent(level, frm);
	printf("Service type: %d\n", cp->qos.service_type);
	p_indent(level, frm);
	printf("Token rate: %d\n", btohl(cp->qos.token_rate));
	p_indent(level, frm);
	printf("Peak bandwith: %d\n", btohl(cp->qos.peak_bandwidth));
	p_indent(level, frm);
	printf("Latency: %d\n", btohl(cp->qos.latency));
	p_indent(level, frm);
	printf("Delay variation: %d\n", btohl(cp->qos.delay_variation));
}

static inline void write_link_policy_dump(int level, struct frame *frm)
{
	write_link_policy_cp *cp = frm->ptr;
	uint16_t policy = btohs(cp->policy);
	char *str;

	p_indent(level, frm);
	printf("handle %d policy 0x%2.2x\n", btohs(cp->handle), policy);

	str = hci_lptostr(policy);
	if (str) {
		p_indent(level, frm);
		printf("Link policy: %s\n", str);
		free(str);
	}
}

static inline void write_default_link_policy_dump(int level, struct frame *frm)
{
	uint16_t policy = btohs(htons(p_get_u16(frm)));
	char *str;

	p_indent(level, frm);
	printf("policy 0x%2.2x\n", policy);

	str = hci_lptostr(policy);
	if (str) {
		p_indent(level, frm);
		printf("Link policy: %s\n", str);
		free(str);
	}
}

static inline void sniff_subrating_dump(int level, struct frame *frm)
{
	sniff_subrating_cp *cp = frm->ptr;

	p_indent(level, frm);
	printf("handle %d\n", btohs(cp->handle));

	p_indent(level, frm);
	printf("max latency %d\n", btohs(cp->max_latency));

	p_indent(level, frm);
	printf("min timeout remote %d local %d\n",
		btohs(cp->min_remote_timeout), btohs(cp->min_local_timeout));
}

static inline void set_event_mask_dump(int level, struct frame *frm)
{
	set_event_mask_cp *cp = frm->ptr;
	int i;

	p_indent(level, frm);
	printf("Mask: 0x");
	for (i = 0; i < 8; i++)
		printf("%2.2x", cp->mask[i]);
	printf("\n");
}

static inline void set_event_flt_dump(int level, struct frame *frm)
{
	set_event_flt_cp *cp = frm->ptr;
	uint8_t dev_class[3], dev_mask[3];
	char addr[18];

	p_indent(level, frm);
	printf("type %d condition %d\n", cp->flt_type,
				(cp->flt_type == 0) ? 0 : cp->cond_type);

	switch (cp->flt_type) {
	case FLT_CLEAR_ALL:
		p_indent(level, frm);
		printf("Clear all filters\n");
		break;
	case FLT_INQ_RESULT:
		p_indent(level, frm);
		printf("Inquiry result");
		switch (cp->cond_type) {
		case INQ_RESULT_RETURN_ALL:
			printf(" for all devices\n");
			break;
		case INQ_RESULT_RETURN_CLASS:
			memcpy(dev_class, cp->condition, 3);
			memcpy(dev_mask, cp->condition + 3, 3);
			printf(" with class 0x%2.2x%2.2x%2.2x mask 0x%2.2x%2.2x%2.2x\n",
				dev_class[2], dev_class[1], dev_class[0],
				dev_mask[2], dev_mask[1], dev_mask[0]);
			break;
		case INQ_RESULT_RETURN_BDADDR:
			p_ba2str((bdaddr_t *) cp->condition, addr);
			printf(" with bdaddr %s\n", addr);
			break;
		default:
			printf("\n");
			break;
		}
		break;
	case FLT_CONN_SETUP:
		p_indent(level, frm);
		printf("Connection setup");
		switch (cp->cond_type) {
		case CONN_SETUP_ALLOW_ALL:
		case CONN_SETUP_ALLOW_CLASS:
		case CONN_SETUP_ALLOW_BDADDR:
		default:
			printf("\n");
			break;
		}
		break;
	}
}

static inline void write_pin_type_dump(int level, struct frame *frm)
{
	write_pin_type_cp *cp = frm->ptr;

	p_indent(level, frm);
	printf("type %d\n", cp->pin_type);
}

static inline void request_stored_link_key_dump(int level, struct frame *frm)
{
	read_stored_link_key_cp *cp = frm->ptr;
	char addr[18];

	p_indent(level, frm);
	p_ba2str(&cp->bdaddr, addr);
	printf("bdaddr %s all %d\n", addr, cp->read_all);
}

static inline void return_link_keys_dump(int level, struct frame *frm)
{
	uint8_t num = p_get_u8(frm);
	uint8_t key[16];
	char addr[18];
	int i, n;

	for (n = 0; n < num; n++) {
		p_ba2str(frm->ptr, addr);
		memcpy(key, frm->ptr + 6, 16);

		p_indent(level, frm);
		printf("bdaddr %s key ", addr);
		for (i = 0; i < 16; i++)
			if (parser.flags & DUMP_NOVENDOR)
				printf("**");
			else
				printf("%2.2X", key[i]);
		printf("\n");

		frm->ptr += 2;
		frm->len -= 2;
	}
}

static inline void change_local_name_dump(int level, struct frame *frm)
{
	change_local_name_cp *cp = frm->ptr;
	char name[249];
	int i;

	memset(name, 0, sizeof(name));
	for (i = 0; i < 248 && cp->name[i]; i++)
		if (isprint(cp->name[i]))
			name[i] = cp->name[i];
		else
			name[i] = '.';

	p_indent(level, frm);
	printf("name \'%s\'\n", name);
}

static inline void write_class_of_dev_dump(int level, struct frame *frm)
{
	write_class_of_dev_cp *cp = frm->ptr;

	p_indent(level, frm);
	printf("class 0x%2.2x%2.2x%2.2x\n",
			cp->dev_class[2], cp->dev_class[1], cp->dev_class[0]);
}

static inline void write_voice_setting_dump(int level, struct frame *frm)
{
	write_voice_setting_cp *cp = frm->ptr;

	p_indent(level, frm);
	printf("voice setting 0x%4.4x\n", btohs(cp->voice_setting));
}

static inline void write_current_iac_lap_dump(int level, struct frame *frm)
{
	write_current_iac_lap_cp *cp = frm->ptr;
	int i;

	for (i = 0; i < cp->num_current_iac; i++) {
		p_indent(level, frm);
		printf("IAC 0x%2.2x%2.2x%2.2x", cp->lap[i][2], cp->lap[i][1], cp->lap[i][0]);
		if (cp->lap[i][2] == 0x9e && cp->lap[i][1] == 0x8b) {
			switch (cp->lap[i][0]) {
			case 0x00:
				printf(" (Limited Inquiry Access Code)");
				break;
			case 0x33:
				printf(" (General Inquiry Access Code)");
				break;
			}
		}
		printf("\n");
	}
}

static inline void write_scan_enable_dump(int level, struct frame *frm)
{
	uint8_t enable = p_get_u8(frm);

	p_indent(level, frm);
	printf("enable %d\n", enable);
}

static inline void write_page_timeout_dump(int level, struct frame *frm)
{
	write_page_timeout_cp *cp = frm->ptr;

	p_indent(level, frm);
	printf("timeout %d\n", btohs(cp->timeout));
}

static inline void write_page_activity_dump(int level, struct frame *frm)
{
	write_page_activity_cp *cp = frm->ptr;

	p_indent(level, frm);
	printf("interval %d window %d\n", btohs(cp->interval), btohs(cp->window));
}

static inline void write_inquiry_scan_type_dump(int level, struct frame *frm)
{
	write_inquiry_scan_type_cp *cp = frm->ptr;

	p_indent(level, frm);
	printf("type %d\n", cp->type);
}

static inline void write_inquiry_mode_dump(int level, struct frame *frm)
{
	write_inquiry_mode_cp *cp = frm->ptr;

	p_indent(level, frm);
	printf("mode %d\n", cp->mode);
}

static inline void set_afh_classification_dump(int level, struct frame *frm)
{
	set_afh_classification_cp *cp = frm->ptr;
	int i;

	p_indent(level, frm);
	printf("map 0x");
	for (i = 0; i < 10; i++)
		printf("%02x", cp->map[i]);
	printf("\n");
}

static inline void write_link_supervision_timeout_dump(int level, struct frame *frm)
{
	write_link_supervision_timeout_cp *cp = frm->ptr;

	p_indent(level, frm);
	printf("handle %d timeout %d\n",
				btohs(cp->handle), btohs(cp->timeout));
}

static inline void write_ext_inquiry_response_dump(int level, struct frame *frm)
{
	write_ext_inquiry_response_cp *cp = frm->ptr;

	p_indent(level, frm);
	printf("fec 0x%2.2x\n", cp->fec);

	frm->ptr++;
	frm->len--;

	ext_inquiry_response_dump(level, frm);
}

static inline void write_inquiry_transmit_power_level_dump(int level, struct frame *frm)
{
	write_inquiry_transmit_power_level_cp *cp = frm->ptr;

	p_indent(level, frm);
	printf("level %d\n", cp->level);
}

static inline void write_default_error_data_reporting_dump(int level, struct frame *frm)
{
	write_default_error_data_reporting_cp *cp = frm->ptr;

	p_indent(level, frm);
	printf("reporting %d\n", cp->reporting);
}

static inline void enhanced_flush_dump(int level, struct frame *frm)
{
	enhanced_flush_cp *cp = frm->ptr;

	p_indent(level, frm);
	printf("handle %d type %d\n", btohs(cp->handle), cp->type);
}

static inline void send_keypress_notify_dump(int level, struct frame *frm)
{
	send_keypress_notify_cp *cp = frm->ptr;
	char addr[18];

	p_indent(level, frm);
	p_ba2str(&cp->bdaddr, addr);
	printf("bdaddr %s type %d\n", addr, cp->type);
}

static inline void request_transmit_power_level_dump(int level, struct frame *frm)
{
	read_transmit_power_level_cp *cp = frm->ptr;

	p_indent(level, frm);
	printf("handle %d type %d (%s)\n",
					btohs(cp->handle), cp->type,
					cp->type ? "maximum" : "current");
}

static inline void request_local_ext_features_dump(int level, struct frame *frm)
{
	read_local_ext_features_cp *cp = frm->ptr;

	p_indent(level, frm);
	printf("page %d\n", cp->page_num);
}

static inline void request_clock_dump(int level, struct frame *frm)
{
	read_clock_cp *cp = frm->ptr;

	p_indent(level, frm);
	printf("handle %d which %d (%s)\n",
					btohs(cp->handle), cp->which_clock,
					cp->which_clock ? "piconet" : "local");
}

static inline void host_buffer_size_dump(int level, struct frame *frm)
{
	host_buffer_size_cp *cp = frm->ptr;

	p_indent(level, frm);
	printf("ACL MTU %d:%d SCO MTU %d:%d\n",
				btohs(cp->acl_mtu), btohs(cp->acl_max_pkt),
				cp->sco_mtu, btohs(cp->sco_max_pkt));
}

static inline void num_comp_pkts_dump(int level, struct frame *frm)
{
	uint8_t num = p_get_u8(frm);
	uint16_t handle, packets;
	int i;

	for (i = 0; i < num; i++) {
		handle = btohs(htons(p_get_u16(frm)));
		packets = btohs(htons(p_get_u16(frm)));

		p_indent(level, frm);
		printf("handle %d packets %d\n", handle, packets);
	}
}

static inline void le_create_connection_dump(int level, struct frame *frm)
{
	char addr[18];
	le_create_connection_cp *cp = frm->ptr;

	p_indent(level, frm);
	p_ba2str(&cp->peer_bdaddr, addr);
	printf("bdaddr %s type %d\n", addr, cp->peer_bdaddr_type);
	p_indent(level, frm);
	printf("interval %u window %u initiator_filter %u\n",
		btohs(cp->interval), btohs(cp->window), cp->initiator_filter);
	p_indent(level, frm);
	printf("own_bdaddr_type %u min_interval %u max_interval %u\n",
			cp->own_bdaddr_type, btohs(cp->min_interval),
			btohs(cp->max_interval));
	p_indent(level, frm);
	printf("latency %u supervision_to %u min_ce %u max_ce %u\n",
			btohs(cp->latency), btohs(cp->supervision_timeout),
			btohs(cp->min_ce_length), btohs(cp->max_ce_length));
}

static inline void le_set_event_mask_dump(int level, struct frame *frm)
{
	int i;
	le_set_event_mask_cp *cp = frm->ptr;

	p_indent(level, frm);
	printf("mask 0x");
	for (i = 0; i < 8; i++)
		printf("%.2x", cp->mask[i]);

	printf(" (%s)\n", eventmask2str(cp->mask));
}

static inline void le_set_random_address_dump(int level, struct frame *frm)
{
	char addr[18];
	le_set_random_address_cp *cp = frm->ptr;

	p_indent(level, frm);
	p_ba2str(&cp->bdaddr, addr);
	printf("bdaddr %s\n", addr);
}


static inline void le_set_advertising_parameters_dump(int level, struct frame *frm)
{
	char addr[18];
	le_set_advertising_parameters_cp *cp = frm->ptr;

	p_indent(level, frm);
	printf("min %.3fms, max %.3fms\n", btohs(cp->min_interval) * 0.625,
			btohs(cp->max_interval) * 0.625);

	p_indent(level, frm);
	printf("type 0x%02x (%s) ownbdaddr 0x%02x (%s)\n", cp->advtype,
			evttype2str(cp->advtype), cp->own_bdaddr_type,
			bdaddrtype2str(cp->own_bdaddr_type));

	p_indent(level, frm);
	p_ba2str(&cp->direct_bdaddr, addr);
	printf("directbdaddr 0x%02x (%s) %s\n", cp->direct_bdaddr_type,
			bdaddrtype2str(cp->direct_bdaddr_type), addr);

	p_indent(level, frm);
	printf("channelmap 0x%02x filterpolicy 0x%02x (%s)\n",
			cp->chan_map, cp->filter, filterpolicy2str(cp->filter));
}

static inline void le_set_scan_parameters_dump(int level, struct frame *frm)
{
	le_set_scan_parameters_cp *cp = frm->ptr;

	p_indent(level, frm);
	printf("type 0x%02x (%s)\n", cp->type,
		cp->type == 0x00 ? "passive" : "active");

	p_indent(level, frm);
	printf("interval %.3fms window %.3fms\n", btohs(cp->interval) * 0.625,
		btohs(cp->window) * 0.625);

	p_indent(level, frm);
	printf("own address: 0x%02x (%s) policy: %s\n", cp->own_bdaddr_type,
			bdaddrtype2str(cp->own_bdaddr_type),
		(cp->filter == 0x00 ? "All" :
			(cp->filter == 0x01 ? "white list only" : "reserved")));
}

static inline void le_set_scan_enable_dump(int level, struct frame *frm)
{
	le_set_scan_enable_cp *cp = frm->ptr;

	p_indent(level, frm);
	printf("value 0x%02x (%s)\n", cp->enable,
		(cp->enable == 0x00 ? "scanning disabled" :
		"scanning enabled"));

	p_indent(level, frm);
	printf("filter duplicates 0x%02x (%s)\n", cp->filter_dup,
		(cp->filter_dup == 0x00 ? "disabled" : "enabled"));
}

static inline void write_remote_amp_assoc_cmd_dump(int level,
							struct frame *frm)
{
	write_remote_amp_assoc_cp *cp = frm->ptr;

	p_indent(level, frm);
	printf("handle 0x%2.2x len_so_far %d remaining_len %d\n", cp->handle,
				cp->length_so_far, cp->remaining_length);

	amp_assoc_dump(level + 1, cp->fragment, frm->len - 5);
}

static inline void command_dump(int level, struct frame *frm)
{
	hci_command_hdr *hdr = frm->ptr;
	uint16_t opcode = btohs(hdr->opcode);
	uint16_t ogf = cmd_opcode_ogf(opcode);
	uint16_t ocf = cmd_opcode_ocf(opcode);

	if (p_filter(FILT_HCI))
		return;

	if (ogf == OGF_VENDOR_CMD && (parser.flags & DUMP_NOVENDOR))
		return;

	p_indent(level, frm);
	printf("HCI Command: %s (0x%2.2x|0x%4.4x) plen %d\n",
				opcode2str(opcode), ogf, ocf, hdr->plen);

	frm->ptr += HCI_COMMAND_HDR_SIZE;
	frm->len -= HCI_COMMAND_HDR_SIZE;

	if (ogf == OGF_VENDOR_CMD) {
		if (ocf == 0 && get_manufacturer() == 10) {
			csr_dump(level + 1, frm);
			return;
		}
	}

	if (!(parser.flags & DUMP_VERBOSE)) {
		raw_dump(level, frm);
		return;
	}

	switch (ogf) {
	case OGF_LINK_CTL:
		switch (ocf) {
		case OCF_INQUIRY:
			inquiry_dump(level + 1, frm);
			return;
		case OCF_PERIODIC_INQUIRY:
			periodic_inquiry_dump(level + 1, frm);
			return;
		case OCF_INQUIRY_CANCEL:
		case OCF_EXIT_PERIODIC_INQUIRY:
			return;
		case OCF_CREATE_CONN:
			create_conn_dump(level + 1, frm);
			return;
		case OCF_DISCONNECT:
			disconnect_dump(level + 1, frm);
			return;
		case OCF_CREATE_CONN_CANCEL:
		case OCF_REMOTE_NAME_REQ_CANCEL:
		case OCF_ACCEPT_SYNC_CONN_REQ:
			bdaddr_command_dump(level + 1, frm);
			return;
		case OCF_ADD_SCO:
		case OCF_SET_CONN_PTYPE:
			add_sco_dump(level + 1, frm);
			return;
		case OCF_ACCEPT_CONN_REQ:
			accept_conn_req_dump(level + 1, frm);
			return;
		case OCF_REJECT_CONN_REQ:
		case OCF_REJECT_SYNC_CONN_REQ:
		case OCF_IO_CAPABILITY_NEG_REPLY:
			reject_conn_req_dump(level + 1, frm);
			return;
		case OCF_PIN_CODE_REPLY:
			pin_code_reply_dump(level + 1, frm);
			return;
		case OCF_LINK_KEY_REPLY:
			link_key_reply_dump(level + 1, frm);
			return;
		case OCF_PIN_CODE_NEG_REPLY:
		case OCF_LINK_KEY_NEG_REPLY:
		case OCF_USER_CONFIRM_REPLY:
		case OCF_USER_CONFIRM_NEG_REPLY:
		case OCF_USER_PASSKEY_NEG_REPLY:
		case OCF_REMOTE_OOB_DATA_NEG_REPLY:
			pin_code_neg_reply_dump(level + 1, frm);
			return;
		case OCF_USER_PASSKEY_REPLY:
			user_passkey_reply_dump(level + 1, frm);
			return;
		case OCF_REMOTE_OOB_DATA_REPLY:
			remote_oob_data_reply_dump(level + 1, frm);
			return;
		case OCF_IO_CAPABILITY_REPLY:
			io_capability_reply_dump(level + 1, frm);
			return;
		case OCF_SET_CONN_ENCRYPT:
			set_conn_encrypt_dump(level + 1, frm);
			return;
		case OCF_AUTH_REQUESTED:
		case OCF_CHANGE_CONN_LINK_KEY:
		case OCF_READ_REMOTE_FEATURES:
		case OCF_READ_REMOTE_VERSION:
		case OCF_READ_CLOCK_OFFSET:
		case OCF_READ_LMP_HANDLE:
		case OCF_DISCONNECT_LOGICAL_LINK:
			generic_command_dump(level + 1, frm);
			return;
		case OCF_MASTER_LINK_KEY:
			master_link_key_dump(level + 1, frm);
			return;
		case OCF_READ_REMOTE_EXT_FEATURES:
			read_remote_ext_features_dump(level + 1, frm);
			return;
		case OCF_REMOTE_NAME_REQ:
			remote_name_req_dump(level + 1, frm);
			return;
		case OCF_SETUP_SYNC_CONN:
			setup_sync_conn_dump(level + 1, frm);
			return;
		case OCF_CREATE_PHYSICAL_LINK:
		case OCF_ACCEPT_PHYSICAL_LINK:
			create_physical_link_dump(level + 1, frm);
			return;
		case OCF_CREATE_LOGICAL_LINK:
		case OCF_ACCEPT_LOGICAL_LINK:
			create_logical_link_dump(level + 1, frm);
			return;
		}
		break;

	case OGF_LINK_POLICY:
		switch (ocf) {
		case OCF_HOLD_MODE:
		case OCF_PARK_MODE:
			hold_mode_dump(level + 1, frm);
			return;
		case OCF_SNIFF_MODE:
			sniff_mode_dump(level + 1, frm);
			return;
		case OCF_EXIT_SNIFF_MODE:
		case OCF_EXIT_PARK_MODE:
		case OCF_ROLE_DISCOVERY:
		case OCF_READ_LINK_POLICY:
			generic_command_dump(level + 1, frm);
			return;
		case OCF_READ_DEFAULT_LINK_POLICY:
			return;
		case OCF_SWITCH_ROLE:
			accept_conn_req_dump(level + 1, frm);
			return;
		case OCF_QOS_SETUP:
			qos_setup_dump(level + 1, frm);
			return;
		case OCF_WRITE_LINK_POLICY:
			write_link_policy_dump(level + 1, frm);
			return;
		case OCF_WRITE_DEFAULT_LINK_POLICY:
			write_default_link_policy_dump(level + 1, frm);
			return;
		case OCF_SNIFF_SUBRATING:
			sniff_subrating_dump(level + 1, frm);
			return;
		}
		break;

	case OGF_HOST_CTL:
		switch (ocf) {
		case OCF_RESET:
		case OCF_CREATE_NEW_UNIT_KEY:
			return;
		case OCF_SET_EVENT_MASK:
		case OCF_SET_EVENT_MASK_PAGE_2:
			set_event_mask_dump(level + 1, frm);
			return;
		case OCF_SET_EVENT_FLT:
			set_event_flt_dump(level + 1, frm);
			return;
		case OCF_WRITE_PIN_TYPE:
			write_pin_type_dump(level + 1, frm);
			return;
		case OCF_READ_STORED_LINK_KEY:
		case OCF_DELETE_STORED_LINK_KEY:
			request_stored_link_key_dump(level + 1, frm);
			return;
		case OCF_WRITE_STORED_LINK_KEY:
			return_link_keys_dump(level + 1, frm);
			return;
		case OCF_CHANGE_LOCAL_NAME:
			change_local_name_dump(level + 1, frm);
			return;
		case OCF_WRITE_CLASS_OF_DEV:
			write_class_of_dev_dump(level + 1, frm);
			return;
		case OCF_WRITE_VOICE_SETTING:
			write_voice_setting_dump(level + 1, frm);
			return;
		case OCF_WRITE_CURRENT_IAC_LAP:
			write_current_iac_lap_dump(level + 1, frm);
			return;
		case OCF_WRITE_SCAN_ENABLE:
		case OCF_WRITE_AUTH_ENABLE:
		case OCF_SET_CONTROLLER_TO_HOST_FC:
			write_scan_enable_dump(level + 1, frm);
			return;
		case OCF_WRITE_LOGICAL_LINK_ACCEPT_TIMEOUT:
		case OCF_WRITE_CONN_ACCEPT_TIMEOUT:
		case OCF_WRITE_PAGE_TIMEOUT:
			write_page_timeout_dump(level + 1, frm);
			return;
		case OCF_WRITE_PAGE_ACTIVITY:
		case OCF_WRITE_INQ_ACTIVITY:
			write_page_activity_dump(level + 1, frm);
			return;
		case OCF_WRITE_INQUIRY_SCAN_TYPE:
			write_inquiry_scan_type_dump(level + 1, frm);
			return;
		case OCF_WRITE_ENCRYPT_MODE:
		case OCF_WRITE_INQUIRY_MODE:
		case OCF_WRITE_AFH_MODE:
			write_inquiry_mode_dump(level + 1, frm);
			return;
		case OCF_SET_AFH_CLASSIFICATION:
			set_afh_classification_dump(level + 1, frm);
			return;
		case OCF_READ_TRANSMIT_POWER_LEVEL:
			request_transmit_power_level_dump(level + 1, frm);
			return;
		case OCF_HOST_BUFFER_SIZE:
			host_buffer_size_dump(level + 1, frm);
			return;
		case OCF_HOST_NUM_COMP_PKTS:
			num_comp_pkts_dump(level + 1, frm);
			return;
		case OCF_FLUSH:
		case OCF_READ_LINK_SUPERVISION_TIMEOUT:
		case OCF_REFRESH_ENCRYPTION_KEY:
		case OCF_READ_BEST_EFFORT_FLUSH_TIMEOUT:
			generic_command_dump(level + 1, frm);
			return;
		case OCF_WRITE_LINK_SUPERVISION_TIMEOUT:
			write_link_supervision_timeout_dump(level + 1, frm);
			return;
		case OCF_WRITE_EXT_INQUIRY_RESPONSE:
			write_ext_inquiry_response_dump(level + 1, frm);
			return;
		case OCF_WRITE_SIMPLE_PAIRING_MODE:
		case OCF_WRITE_FLOW_CONTROL_MODE:
			generic_write_mode_dump(level + 1, frm);
			return;
		case OCF_WRITE_INQUIRY_TRANSMIT_POWER_LEVEL:
			write_inquiry_transmit_power_level_dump(level + 1, frm);
			return;
		case OCF_WRITE_DEFAULT_ERROR_DATA_REPORTING:
			write_default_error_data_reporting_dump(level + 1, frm);
			return;
		case OCF_ENHANCED_FLUSH:
			enhanced_flush_dump(level + 1, frm);
			return;
		case OCF_SEND_KEYPRESS_NOTIFY:
			send_keypress_notify_dump(level + 1, frm);
			return;
		}
		break;

	case OGF_INFO_PARAM:
		switch (ocf) {
		case OCF_READ_LOCAL_EXT_FEATURES:
			request_local_ext_features_dump(level + 1, frm);
			return;
		}
		break;

	case OGF_STATUS_PARAM:
		switch (ocf) {
		case OCF_READ_LINK_QUALITY:
		case OCF_READ_RSSI:
		case OCF_READ_AFH_MAP:
			generic_command_dump(level + 1, frm);
			return;
		case OCF_READ_CLOCK:
			request_clock_dump(level + 1, frm);
			return;
		case OCF_WRITE_REMOTE_AMP_ASSOC:
			write_remote_amp_assoc_cmd_dump(level + 1, frm);
			return;
		}
		break;

	case OGF_TESTING_CMD:
		switch (ocf) {
		case OCF_WRITE_LOOPBACK_MODE:
		case OCF_WRITE_SIMPLE_PAIRING_DEBUG_MODE:
			generic_write_mode_dump(level + 1, frm);
			return;
		}
		break;

	case OGF_LE_CTL:
		switch (ocf) {
		case OCF_LE_SET_EVENT_MASK:
			le_set_event_mask_dump(level + 1, frm);
			return;
		case OCF_LE_READ_BUFFER_SIZE:
		case OCF_LE_READ_LOCAL_SUPPORTED_FEATURES:
		case OCF_LE_READ_ADVERTISING_CHANNEL_TX_POWER:
			return;
		case OCF_LE_SET_RANDOM_ADDRESS:
			le_set_random_address_dump(level + 1, frm);
			return;
		case OCF_LE_SET_ADVERTISING_PARAMETERS:
			le_set_advertising_parameters_dump(level + 1, frm);
			return;
		case OCF_LE_SET_SCAN_PARAMETERS:
			le_set_scan_parameters_dump(level + 1, frm);
			return;
		case OCF_LE_SET_SCAN_ENABLE:
			le_set_scan_enable_dump(level + 1, frm);
			return;
		case OCF_LE_CREATE_CONN:
			le_create_connection_dump(level + 1, frm);
			return;
		}
		break;
	}

	raw_dump(level, frm);
}

static inline void status_response_dump(int level, struct frame *frm)
{
	uint8_t status = p_get_u8(frm);

	p_indent(level, frm);
	printf("status 0x%2.2x\n", status);

	if (status > 0) {
		p_indent(level, frm);
		printf("Error: %s\n", status2str(status));
	}

	raw_dump(level, frm);
}

static inline void handle_response_dump(int level, struct frame *frm)
{
	uint16_t handle = btohs(htons(p_get_u16(frm)));

	p_indent(level, frm);
	printf("handle %d\n", handle);

	raw_dump(level, frm);
}

static inline void bdaddr_response_dump(int level, struct frame *frm)
{
	uint8_t status = p_get_u8(frm);
	bdaddr_t *bdaddr = frm->ptr;
	char addr[18];

	frm->ptr += sizeof(bdaddr_t);
	frm->len -= sizeof(bdaddr_t);

	p_indent(level, frm);
	p_ba2str(bdaddr, addr);
	printf("status 0x%2.2x bdaddr %s\n", status, addr);

	if (status > 0) {
		p_indent(level, frm);
		printf("Error: %s\n", status2str(status));
	}

	raw_dump(level, frm);
}

static inline void read_data_block_size_dump(int level, struct frame *frm)
{
	read_data_block_size_rp *rp = frm->ptr;

	p_indent(level, frm);
	printf("status 0x%2.2x\n", rp->status);

	if (rp->status > 0) {
		p_indent(level, frm);
		printf("Error: %s\n", status2str(rp->status));
	} else {
		p_indent(level, frm);
		printf("Max ACL %d Block len %d Num blocks %d\n",
			btohs(rp->max_acl_len), btohs(rp->data_block_len),
							btohs(rp->num_blocks));
	}
}

static inline void generic_response_dump(int level, struct frame *frm)
{
	uint8_t status = p_get_u8(frm);
	uint16_t handle = btohs(htons(p_get_u16(frm)));

	p_indent(level, frm);
	printf("status 0x%2.2x handle %d\n", status, handle);

	if (status > 0) {
		p_indent(level, frm);
		printf("Error: %s\n", status2str(status));
	}

	raw_dump(level, frm);
}

static inline void status_mode_dump(int level, struct frame *frm)
{
	uint8_t status = p_get_u8(frm);
	uint8_t mode = p_get_u8(frm);

	p_indent(level, frm);
	printf("status 0x%2.2x mode 0x%2.2x\n", status, mode);

	if (status > 0) {
		p_indent(level, frm);
		printf("Error: %s\n", status2str(status));
	}
}

static inline void read_link_policy_dump(int level, struct frame *frm)
{
	read_link_policy_rp *rp = frm->ptr;
	uint16_t policy = btohs(rp->policy);
	char *str;

	p_indent(level, frm);
	printf("status 0x%2.2x handle %d policy 0x%2.2x\n",
				rp->status, btohs(rp->handle), policy);

	if (rp->status > 0) {
		p_indent(level, frm);
		printf("Error: %s\n", status2str(rp->status));
	} else {
		str = hci_lptostr(policy);
		if (str) {
			p_indent(level, frm);
			printf("Link policy: %s\n", str);
			free(str);
		}
	}
}

static inline void read_default_link_policy_dump(int level, struct frame *frm)
{
	uint8_t status = p_get_u8(frm);
	uint16_t policy = btohs(htons(p_get_u16(frm)));
	char *str;

	p_indent(level, frm);
	printf("status 0x%2.2x policy 0x%2.2x\n", status, policy);

	if (status > 0) {
		p_indent(level, frm);
		printf("Error: %s\n", status2str(status));
	} else {
		str = hci_lptostr(policy);
		if (str) {
			p_indent(level, frm);
			printf("Link policy: %s\n", str);
			free(str);
		}
	}
}

static inline void read_pin_type_dump(int level, struct frame *frm)
{
	read_pin_type_rp *rp = frm->ptr;

	p_indent(level, frm);
	printf("status 0x%2.2x type %d\n", rp->status, rp->pin_type);

	if (rp->status > 0) {
		p_indent(level, frm);
		printf("Error: %s\n", status2str(rp->status));
	}
}

static inline void read_stored_link_key_dump(int level, struct frame *frm)
{
	read_stored_link_key_rp *rp = frm->ptr;

	p_indent(level, frm);
	printf("status 0x%2.2x max %d num %d\n",
				rp->status, rp->max_keys, rp->num_keys);

	if (rp->status > 0) {
		p_indent(level, frm);
		printf("Error: %s\n", status2str(rp->status));
	}
}

static inline void write_stored_link_key_dump(int level, struct frame *frm)
{
	write_stored_link_key_rp *rp = frm->ptr;

	p_indent(level, frm);
	printf("status 0x%2.2x written %d\n", rp->status, rp->num_keys);

	if (rp->status > 0) {
		p_indent(level, frm);
		printf("Error: %s\n", status2str(rp->status));
	}
}

static inline void delete_stored_link_key_dump(int level, struct frame *frm)
{
	delete_stored_link_key_rp *rp = frm->ptr;

	p_indent(level, frm);
	printf("status 0x%2.2x deleted %d\n", rp->status, btohs(rp->num_keys));

	if (rp->status > 0) {
		p_indent(level, frm);
		printf("Error: %s\n", status2str(rp->status));
	}
}

static inline void read_local_name_dump(int level, struct frame *frm)
{
	read_local_name_rp *rp = frm->ptr;
	char name[249];
	int i;

	memset(name, 0, sizeof(name));
	for (i = 0; i < 248 && rp->name[i]; i++)
		if (isprint(rp->name[i]))
			name[i] = rp->name[i];
		else
			name[i] = '.';

	p_indent(level, frm);
	printf("status 0x%2.2x name \'%s\'\n", rp->status, name);

	if (rp->status > 0) {
		p_indent(level, frm);
		printf("Error: %s\n", status2str(rp->status));
	}
}

static inline void read_class_of_dev_dump(int level, struct frame *frm)
{
	read_class_of_dev_rp *rp = frm->ptr;

	p_indent(level, frm);
	printf("status 0x%2.2x class 0x%2.2x%2.2x%2.2x\n", rp->status,
			rp->dev_class[2], rp->dev_class[1], rp->dev_class[0]);

	if (rp->status > 0) {
		p_indent(level, frm);
		printf("Error: %s\n", status2str(rp->status));
	}
}

static inline void read_voice_setting_dump(int level, struct frame *frm)
{
	read_voice_setting_rp *rp = frm->ptr;

	p_indent(level, frm);
	printf("status 0x%2.2x voice setting 0x%4.4x\n",
					rp->status, btohs(rp->voice_setting));

	if (rp->status > 0) {
		p_indent(level, frm);
		printf("Error: %s\n", status2str(rp->status));
	}
}

static inline void read_current_iac_lap_dump(int level, struct frame *frm)
{
	read_current_iac_lap_rp *rp = frm->ptr;
	int i;

	for (i = 0; i < rp->num_current_iac; i++) {
		p_indent(level, frm);
		printf("IAC 0x%2.2x%2.2x%2.2x", rp->lap[i][2], rp->lap[i][1], rp->lap[i][0]);
		if (rp->lap[i][2] == 0x9e && rp->lap[i][1] == 0x8b) {
			switch (rp->lap[i][0]) {
			case 0x00:
				printf(" (Limited Inquiry Access Code)");
				break;
			case 0x33:
				printf(" (General Inquiry Access Code)");
				break;
			}
		}
		printf("\n");
	}
}

static inline void read_scan_enable_dump(int level, struct frame *frm)
{
	uint8_t status = p_get_u8(frm);
	uint8_t enable = p_get_u8(frm);

	p_indent(level, frm);
	printf("status 0x%2.2x enable %d\n", status, enable);

	if (status > 0) {
		p_indent(level, frm);
		printf("Error: %s\n", status2str(status));
	}
}

static inline void read_page_timeout_dump(int level, struct frame *frm)
{
	read_page_timeout_rp *rp = frm->ptr;

	p_indent(level, frm);
	printf("status 0x%2.2x timeout %d\n", rp->status, btohs(rp->timeout));

	if (rp->status > 0) {
		p_indent(level, frm);
		printf("Error: %s\n", status2str(rp->status));
	}
}

static inline void read_page_activity_dump(int level, struct frame *frm)
{
	read_page_activity_rp *rp = frm->ptr;

	p_indent(level, frm);
	printf("status 0x%2.2x interval %d window %d\n",
			rp->status, btohs(rp->interval), btohs(rp->window));

	if (rp->status > 0) {
		p_indent(level, frm);
		printf("Error: %s\n", status2str(rp->status));
	}
}

static inline void read_inquiry_scan_type_dump(int level, struct frame *frm)
{
	read_inquiry_scan_type_rp *rp = frm->ptr;

	p_indent(level, frm);
	printf("status 0x%2.2x type %d\n", rp->status, rp->type);

	if (rp->status > 0) {
		p_indent(level, frm);
		printf("Error: %s\n", status2str(rp->status));
	}
}

static inline void read_inquiry_mode_dump(int level, struct frame *frm)
{
	read_inquiry_mode_rp *rp = frm->ptr;

	p_indent(level, frm);
	printf("status 0x%2.2x mode %d\n", rp->status, rp->mode);

	if (rp->status > 0) {
		p_indent(level, frm);
		printf("Error: %s\n", status2str(rp->status));
	}
}

static inline void read_link_supervision_timeout_dump(int level, struct frame *frm)
{
	read_link_supervision_timeout_rp *rp = frm->ptr;

	p_indent(level, frm);
	printf("status 0x%2.2x handle %d timeout %d\n",
			rp->status, btohs(rp->handle), btohs(rp->timeout));

	if (rp->status > 0) {
		p_indent(level, frm);
		printf("Error: %s\n", status2str(rp->status));
	}
}

static inline void read_transmit_power_level_dump(int level, struct frame *frm)
{
	read_transmit_power_level_rp *rp = frm->ptr;

	p_indent(level, frm);
	printf("status 0x%2.2x handle %d level %d\n",
				rp->status, btohs(rp->handle), rp->level);

	if (rp->status > 0) {
		p_indent(level, frm);
		printf("Error: %s\n", status2str(rp->status));
	}
}

static inline void read_ext_inquiry_response_dump(int level, struct frame *frm)
{
	read_ext_inquiry_response_rp *rp = frm->ptr;

	p_indent(level, frm);
	printf("status 0x%2.2x fec 0x%2.2x\n", rp->status, rp->fec);

	if (rp->status > 0) {
		p_indent(level, frm);
		printf("Error: %s\n", status2str(rp->status));
	} else {
		frm->ptr += 2;
		frm->len -= 2;

		ext_inquiry_response_dump(level, frm);
	}
}

static inline void read_inquiry_transmit_power_level_dump(int level, struct frame *frm)
{
	read_inquiry_transmit_power_level_rp *rp = frm->ptr;

	p_indent(level, frm);
	printf("status 0x%2.2x level %d\n", rp->status, rp->level);

	if (rp->status > 0) {
		p_indent(level, frm);
		printf("Error: %s\n", status2str(rp->status));
	}
}

static inline void read_default_error_data_reporting_dump(int level, struct frame *frm)
{
	read_default_error_data_reporting_rp *rp = frm->ptr;

	p_indent(level, frm);
	printf("status 0x%2.2x reporting %d\n", rp->status, rp->reporting);

	if (rp->status > 0) {
		p_indent(level, frm);
		printf("Error: %s\n", status2str(rp->status));
	}
}

static inline void read_local_oob_data_dump(int level, struct frame *frm)
{
	read_local_oob_data_rp *rp = frm->ptr;
	int i;

	p_indent(level, frm);
	printf("status 0x%2.2x\n", rp->status);

	if (rp->status > 0) {
		p_indent(level, frm);
		printf("Error: %s\n", status2str(rp->status));
	} else {
		p_indent(level, frm);
		printf("hash 0x");
		for (i = 0; i < 16; i++)
			printf("%02x", rp->hash[i]);
		printf("\n");

		p_indent(level, frm);
		printf("randomizer 0x");
		for (i = 0; i < 16; i++)
			printf("%02x", rp->randomizer[i]);
		printf("\n");
	}
}

static inline void read_local_version_dump(int level, struct frame *frm)
{
	read_local_version_rp *rp = frm->ptr;
	uint16_t manufacturer = btohs(rp->manufacturer);

	p_indent(level, frm);
	printf("status 0x%2.2x\n", rp->status);

	if (rp->status > 0) {
		p_indent(level, frm);
		printf("Error: %s\n", status2str(rp->status));
	} else {
		char *lmpver = lmp_vertostr(rp->lmp_ver);
		char *hciver = hci_vertostr(rp->hci_ver);

		p_indent(level, frm);
		printf("HCI Version: %s (0x%x) HCI Revision: 0x%x\n",
					hciver ? hciver : "n/a",
					rp->hci_ver, btohs(rp->hci_rev));
		p_indent(level, frm);
		printf("LMP Version: %s (0x%x) LMP Subversion: 0x%x\n",
					lmpver ? lmpver : "n/a",
					rp->lmp_ver, btohs(rp->lmp_subver));
		p_indent(level, frm);
		printf("Manufacturer: %s (%d)\n",
				bt_compidtostr(manufacturer), manufacturer);

		if (lmpver)
			free(lmpver);
		if (hciver)
			free(hciver);
	}
}

static inline void read_local_commands_dump(int level, struct frame *frm)
{
	read_local_commands_rp *rp = frm->ptr;
	int i, max = 0;

	p_indent(level, frm);
	printf("status 0x%2.2x\n", rp->status);

	if (rp->status > 0) {
		p_indent(level, frm);
		printf("Error: %s\n", status2str(rp->status));
	} else {
		for (i = 0; i < 64; i++)
			if (rp->commands[i])
				max = i + 1;
		p_indent(level, frm);
		printf("Commands: ");
		for (i = 0; i < (max > 32 ? 32 : max); i++)
			printf("%2.2x", rp->commands[i]);
		printf("\n");
		if (max > 32) {
			p_indent(level, frm);
			printf("          ");
			for (i = 32; i < max; i++)
				printf("%2.2x", rp->commands[i]);
			printf("\n");
		}
	}
}

static inline void read_local_features_dump(int level, struct frame *frm)
{
	read_local_features_rp *rp = frm->ptr;
	int i;

	p_indent(level, frm);
	printf("status 0x%2.2x\n", rp->status);

	if (rp->status > 0) {
		p_indent(level, frm);
		printf("Error: %s\n", status2str(rp->status));
	} else {
		p_indent(level, frm);
		printf("Features:");
		for (i = 0; i < 8; i++)
			printf(" 0x%2.2x", rp->features[i]);
		printf("\n");
	}
}

static inline void read_local_ext_features_dump(int level, struct frame *frm)
{
	read_local_ext_features_rp *rp = frm->ptr;
	int i;

	p_indent(level, frm);
	printf("status 0x%2.2x page %d max %d\n",
		rp->status, rp->page_num, rp->max_page_num);

	if (rp->status > 0) {
		p_indent(level, frm);
		printf("Error: %s\n", status2str(rp->status));
	} else {
		p_indent(level, frm);
		printf("Features:");
		for (i = 0; i < 8; i++)
			 printf(" 0x%2.2x", rp->features[i]);
		printf("\n");
	}
}

static inline void read_buffer_size_dump(int level, struct frame *frm)
{
	read_buffer_size_rp *rp = frm->ptr;

	p_indent(level, frm);
	printf("status 0x%2.2x\n", rp->status);

	if (rp->status > 0) {
		p_indent(level, frm);
		printf("Error: %s\n", status2str(rp->status));
	} else {
		p_indent(level, frm);
		printf("ACL MTU %d:%d SCO MTU %d:%d\n",
				btohs(rp->acl_mtu), btohs(rp->acl_max_pkt),
				rp->sco_mtu, btohs(rp->sco_max_pkt));
	}
}

static inline void read_link_quality_dump(int level, struct frame *frm)
{
	read_link_quality_rp *rp = frm->ptr;

	p_indent(level, frm);
	printf("status 0x%2.2x handle %d lq %d\n",
			rp->status, btohs(rp->handle), rp->link_quality);

	if (rp->status > 0) {
		p_indent(level, frm);
		printf("Error: %s\n", status2str(rp->status));
	}
}

static inline void read_rssi_dump(int level, struct frame *frm)
{
	read_rssi_rp *rp = frm->ptr;

	p_indent(level, frm);
	printf("status 0x%2.2x handle %d rssi %d\n",
				rp->status, btohs(rp->handle), rp->rssi);

	if (rp->status > 0) {
		p_indent(level, frm);
		printf("Error: %s\n", status2str(rp->status));
	}
}

static inline void read_afh_map_dump(int level, struct frame *frm)
{
	read_afh_map_rp *rp = frm->ptr;
	int i;

	p_indent(level, frm);
	printf("status 0x%2.2x handle %d mode %d\n",
				rp->status, btohs(rp->handle), rp->mode);

	if (rp->status > 0) {
		p_indent(level, frm);
		printf("Error: %s\n", status2str(rp->status));
	} else {
		p_indent(level, frm);
		printf("AFH map: 0x");
		for (i = 0; i < 10; i++)
			printf("%2.2x", rp->map[i]);
		printf("\n");
	}
}

static inline void read_clock_dump(int level, struct frame *frm)
{
	read_clock_rp *rp = frm->ptr;

	p_indent(level, frm);
	printf("status 0x%2.2x handle %d clock 0x%4.4x accuracy %d\n",
					rp->status, btohs(rp->handle),
					btohl(rp->clock), btohs(rp->accuracy));

	if (rp->status > 0) {
		p_indent(level, frm);
		printf("Error: %s\n", status2str(rp->status));
	}
}

static inline void read_local_amp_info_dump(int level, struct frame *frm)
{
	read_local_amp_info_rp *rp = frm->ptr;

	p_indent(level, frm);
	printf("status 0x%2.2x amp status 0x%2.2x\n",
			rp->status, rp->amp_status);
	if (rp->status > 0) {
		p_indent(level, frm);
		printf("Error: %s\n", status2str(rp->status));
	} else {
		p_indent(level, frm);
		printf("total bandwidth %d, max guaranteed bandwidth %d\n",
			btohl(rp->total_bandwidth),
			btohl(rp->max_guaranteed_bandwidth));
		p_indent(level, frm);
		printf("min latency %d, max PDU %d, controller type 0x%2.2x\n",
			btohl(rp->min_latency), btohl(rp->max_pdu_size),
			rp->controller_type);
		p_indent(level, frm);
		printf("pal caps 0x%4.4x, max assoc len %d\n",
			btohs(rp->pal_caps), btohs(rp->max_amp_assoc_length));
		p_indent(level, frm);
		printf("max flush timeout %d, best effort flush timeout %d\n",
			btohl(rp->max_flush_timeout),
			btohl(rp->best_effort_flush_timeout));
	}
}

static inline void read_local_amp_assoc_dump(int level, struct frame *frm)
{
	read_local_amp_assoc_rp *rp = frm->ptr;
	uint16_t len = btohs(rp->length);

	p_indent(level, frm);
	printf("status 0x%2.2x handle 0x%2.2x remaining len %d\n",
			rp->status, rp->handle, len);
	if (rp->status > 0) {
		p_indent(level, frm);
		printf("Error: %s\n", status2str(rp->status));
	} else {
		amp_assoc_dump(level + 1, rp->fragment, len);
	}
}

static inline void write_remote_amp_assoc_dump(int level, struct frame *frm)
{
	write_remote_amp_assoc_rp *rp = frm->ptr;

	p_indent(level, frm);
	printf("status 0x%2.2x handle 0x%2.2x\n", rp->status, rp->handle);
	if (rp->status > 0) {
		p_indent(level, frm);
		printf("Error: %s\n", status2str(rp->status));
	}
}

static inline void le_read_buffer_size_response_dump(int level, struct frame *frm)
{
	le_read_buffer_size_rp *rp = frm->ptr;

	p_indent(level, frm);
	printf("status 0x%2.2x pktlen 0x%4.4x maxpkt 0x%2.2x\n", rp->status,
			rp->pkt_len, rp->max_pkt);

	if (rp->status > 0) {
		p_indent(level, frm);
		printf("Error: %s\n", status2str(rp->status));
	}
}

static inline void le_read_local_supported_features_dump(int level, struct frame *frm)
{
	int i;
	le_read_local_supported_features_rp *rp = frm->ptr;

	p_indent(level, frm);
	printf("status 0x%2.2x features 0x", rp->status);
	for (i = 0; i < 8; i++)
		printf("%2.2x", rp->features[i]);
	printf(" (%s)\n", lefeatures2str(rp->features));

	if (rp->status > 0) {
		p_indent(level, frm);
		printf("Error: %s\n", status2str(rp->status));
	}
}

static inline void le_read_advertising_channel_tx_power_dump(int level, struct frame *frm)
{
	le_read_advertising_channel_tx_power_rp *rp = frm->ptr;

	p_indent(level, frm);
	printf("status 0x%2.2x level 0x%x (dBm)\n", rp->status, rp->level);

	if (rp->status > 0) {
		p_indent(level, frm);
		printf("Error: %s\n", status2str(rp->status));
	}
}

static inline void cmd_complete_dump(int level, struct frame *frm)
{
	evt_cmd_complete *evt = frm->ptr;
	uint16_t opcode = btohs(evt->opcode);
	uint16_t ogf = cmd_opcode_ogf(opcode);
	uint16_t ocf = cmd_opcode_ocf(opcode);

	if (ogf == OGF_VENDOR_CMD && (parser.flags & DUMP_NOVENDOR))
		return;

	p_indent(level, frm);
	printf("%s (0x%2.2x|0x%4.4x) ncmd %d\n",
				opcode2str(opcode), ogf, ocf, evt->ncmd);

	frm->ptr += EVT_CMD_COMPLETE_SIZE;
	frm->len -= EVT_CMD_COMPLETE_SIZE;

	if (!(parser.flags & DUMP_VERBOSE)) {
		raw_dump(level, frm);
		return;
	}

	switch (ogf) {
	case OGF_LINK_CTL:
		switch (ocf) {
		case OCF_INQUIRY_CANCEL:
		case OCF_PERIODIC_INQUIRY:
		case OCF_EXIT_PERIODIC_INQUIRY:
		case OCF_READ_REMOTE_EXT_FEATURES:
			status_response_dump(level, frm);
			return;
		case OCF_CREATE_CONN_CANCEL:
		case OCF_REMOTE_NAME_REQ_CANCEL:
		case OCF_PIN_CODE_REPLY:
		case OCF_LINK_KEY_REPLY:
		case OCF_PIN_CODE_NEG_REPLY:
		case OCF_LINK_KEY_NEG_REPLY:
		case OCF_USER_CONFIRM_REPLY:
		case OCF_USER_CONFIRM_NEG_REPLY:
		case OCF_USER_PASSKEY_REPLY:
		case OCF_USER_PASSKEY_NEG_REPLY:
		case OCF_REMOTE_OOB_DATA_REPLY:
		case OCF_REMOTE_OOB_DATA_NEG_REPLY:
		case OCF_IO_CAPABILITY_REPLY:
		case OCF_IO_CAPABILITY_NEG_REPLY:
			bdaddr_response_dump(level, frm);
			return;
		}
		break;

	case OGF_LINK_POLICY:
		switch (ocf) {
		case OCF_READ_LINK_POLICY:
			read_link_policy_dump(level, frm);
			return;
		case OCF_WRITE_LINK_POLICY:
		case OCF_SNIFF_SUBRATING:
			generic_response_dump(level, frm);
			return;
		case OCF_READ_DEFAULT_LINK_POLICY:
			read_default_link_policy_dump(level, frm);
			return;
		case OCF_WRITE_DEFAULT_LINK_POLICY:
			status_response_dump(level, frm);
			return;
		}
		break;

	case OGF_HOST_CTL:
		switch (ocf) {
		case OCF_READ_PIN_TYPE:
			read_pin_type_dump(level, frm);
			return;
		case OCF_READ_STORED_LINK_KEY:
			read_stored_link_key_dump(level, frm);
			return;
		case OCF_WRITE_STORED_LINK_KEY:
			write_stored_link_key_dump(level, frm);
			return;
		case OCF_DELETE_STORED_LINK_KEY:
			delete_stored_link_key_dump(level, frm);
			return;
		case OCF_READ_LOCAL_NAME:
			read_local_name_dump(level, frm);
			return;
		case OCF_READ_CLASS_OF_DEV:
			read_class_of_dev_dump(level, frm);
			return;
		case OCF_READ_VOICE_SETTING:
			read_voice_setting_dump(level, frm);
			return;
		case OCF_READ_CURRENT_IAC_LAP:
			read_current_iac_lap_dump(level, frm);
			return;
		case OCF_READ_SCAN_ENABLE:
		case OCF_READ_AUTH_ENABLE:
			read_scan_enable_dump(level, frm);
			return;
		case OCF_READ_CONN_ACCEPT_TIMEOUT:
		case OCF_READ_PAGE_TIMEOUT:
		case OCF_READ_LOGICAL_LINK_ACCEPT_TIMEOUT:
			read_page_timeout_dump(level, frm);
			return;
		case OCF_READ_PAGE_ACTIVITY:
		case OCF_READ_INQ_ACTIVITY:
			read_page_activity_dump(level, frm);
			return;
		case OCF_READ_INQUIRY_SCAN_TYPE:
			read_inquiry_scan_type_dump(level, frm);
			return;
		case OCF_READ_ENCRYPT_MODE:
		case OCF_READ_INQUIRY_MODE:
		case OCF_READ_AFH_MODE:
			read_inquiry_mode_dump(level, frm);
			return;
		case OCF_READ_LINK_SUPERVISION_TIMEOUT:
			read_link_supervision_timeout_dump(level, frm);
			return;
		case OCF_READ_TRANSMIT_POWER_LEVEL:
			read_transmit_power_level_dump(level, frm);
			return;
		case OCF_READ_EXT_INQUIRY_RESPONSE:
			read_ext_inquiry_response_dump(level, frm);
			return;
		case OCF_READ_INQUIRY_TRANSMIT_POWER_LEVEL:
			read_inquiry_transmit_power_level_dump(level, frm);
			return;
		case OCF_READ_DEFAULT_ERROR_DATA_REPORTING:
			read_default_error_data_reporting_dump(level, frm);
			return;
		case OCF_READ_LOCAL_OOB_DATA:
			read_local_oob_data_dump(level, frm);
			return;
		case OCF_READ_SIMPLE_PAIRING_MODE:
		case OCF_READ_FLOW_CONTROL_MODE:
			status_mode_dump(level, frm);
			return;
		case OCF_FLUSH:
		case OCF_WRITE_LINK_SUPERVISION_TIMEOUT:
			generic_response_dump(level, frm);
			return;
		case OCF_RESET:
		case OCF_SET_EVENT_MASK:
		case OCF_SET_EVENT_FLT:
		case OCF_WRITE_PIN_TYPE:
		case OCF_CREATE_NEW_UNIT_KEY:
		case OCF_CHANGE_LOCAL_NAME:
		case OCF_WRITE_CLASS_OF_DEV:
		case OCF_WRITE_VOICE_SETTING:
		case OCF_WRITE_CURRENT_IAC_LAP:
		case OCF_WRITE_SCAN_ENABLE:
		case OCF_WRITE_AUTH_ENABLE:
		case OCF_WRITE_ENCRYPT_MODE:
		case OCF_WRITE_CONN_ACCEPT_TIMEOUT:
		case OCF_WRITE_PAGE_TIMEOUT:
		case OCF_WRITE_PAGE_ACTIVITY:
		case OCF_WRITE_INQ_ACTIVITY:
		case OCF_WRITE_INQUIRY_SCAN_TYPE:
		case OCF_WRITE_INQUIRY_MODE:
		case OCF_WRITE_AFH_MODE:
		case OCF_SET_AFH_CLASSIFICATION:
		case OCF_WRITE_EXT_INQUIRY_RESPONSE:
		case OCF_WRITE_SIMPLE_PAIRING_MODE:
		case OCF_WRITE_INQUIRY_TRANSMIT_POWER_LEVEL:
		case OCF_WRITE_DEFAULT_ERROR_DATA_REPORTING:
		case OCF_SET_CONTROLLER_TO_HOST_FC:
		case OCF_HOST_BUFFER_SIZE:
		case OCF_REFRESH_ENCRYPTION_KEY:
		case OCF_SEND_KEYPRESS_NOTIFY:
		case OCF_WRITE_LOGICAL_LINK_ACCEPT_TIMEOUT:
		case OCF_SET_EVENT_MASK_PAGE_2:
		case OCF_WRITE_LOCATION_DATA:
		case OCF_WRITE_FLOW_CONTROL_MODE:
		case OCF_READ_BEST_EFFORT_FLUSH_TIMEOUT:
		case OCF_WRITE_BEST_EFFORT_FLUSH_TIMEOUT:
			status_response_dump(level, frm);
			return;
		}
		break;

	case OGF_INFO_PARAM:
		switch (ocf) {
		case OCF_READ_LOCAL_VERSION:
			read_local_version_dump(level, frm);
			return;
		case OCF_READ_LOCAL_COMMANDS:
			read_local_commands_dump(level, frm);
			return;
		case OCF_READ_LOCAL_FEATURES:
			read_local_features_dump(level, frm);
			return;
		case OCF_READ_LOCAL_EXT_FEATURES:
			read_local_ext_features_dump(level, frm);
			return;
		case OCF_READ_BUFFER_SIZE:
			read_buffer_size_dump(level, frm);
			return;
		case OCF_READ_BD_ADDR:
			bdaddr_response_dump(level, frm);
			return;
		case OCF_READ_DATA_BLOCK_SIZE:
			read_data_block_size_dump(level, frm);
			return;
		}
		break;

	case OGF_STATUS_PARAM:
		switch (ocf) {
		case OCF_READ_FAILED_CONTACT_COUNTER:
		case OCF_RESET_FAILED_CONTACT_COUNTER:
			status_response_dump(level, frm);
			return;
		case OCF_READ_LINK_QUALITY:
			read_link_quality_dump(level, frm);
			return;
		case OCF_READ_RSSI:
			read_rssi_dump(level, frm);
			return;
		case OCF_READ_AFH_MAP:
			read_afh_map_dump(level, frm);
			return;
		case OCF_READ_CLOCK:
			read_clock_dump(level, frm);
			return;
		case OCF_READ_LOCAL_AMP_INFO:
			read_local_amp_info_dump(level, frm);
			return;
		case OCF_READ_LOCAL_AMP_ASSOC:
			read_local_amp_assoc_dump(level, frm);
			return;
		case OCF_WRITE_REMOTE_AMP_ASSOC:
			write_remote_amp_assoc_dump(level, frm);
			return;
		}
		break;

	case OGF_TESTING_CMD:
		switch (ocf) {
		case OCF_READ_LOOPBACK_MODE:
			status_mode_dump(level, frm);
			return;
		case OCF_WRITE_LOOPBACK_MODE:
		case OCF_ENABLE_DEVICE_UNDER_TEST_MODE:
		case OCF_WRITE_SIMPLE_PAIRING_DEBUG_MODE:
			status_response_dump(level, frm);
			return;
		}
		break;

	case OGF_LE_CTL:
		switch (ocf) {
		case OCF_LE_SET_EVENT_MASK:
		case OCF_LE_SET_RANDOM_ADDRESS:
		case OCF_LE_SET_ADVERTISING_PARAMETERS:
		case OCF_LE_SET_ADVERTISING_DATA:
		case OCF_LE_SET_SCAN_RESPONSE_DATA:
		case OCF_LE_SET_ADVERTISE_ENABLE:
		case OCF_LE_SET_SCAN_PARAMETERS:
		case OCF_LE_SET_SCAN_ENABLE:
		case OCF_LE_CREATE_CONN:
		case OCF_LE_CLEAR_WHITE_LIST:
		case OCF_LE_ADD_DEVICE_TO_WHITE_LIST:
		case OCF_LE_REMOVE_DEVICE_FROM_WHITE_LIST:
		case OCF_LE_SET_HOST_CHANNEL_CLASSIFICATION:
		case OCF_LE_RECEIVER_TEST:
		case OCF_LE_TRANSMITTER_TEST:
			status_response_dump(level, frm);
			return;
		case OCF_LE_READ_BUFFER_SIZE:
			le_read_buffer_size_response_dump(level, frm);
			return;
		case OCF_LE_READ_LOCAL_SUPPORTED_FEATURES:
			le_read_local_supported_features_dump(level, frm);
			return;
		case OCF_LE_READ_ADVERTISING_CHANNEL_TX_POWER:
			le_read_advertising_channel_tx_power_dump(level, frm);
			return;
		}
		break;
	}

	raw_dump(level, frm);
}

static inline void cmd_status_dump(int level, struct frame *frm)
{
	evt_cmd_status *evt = frm->ptr;
	uint16_t opcode = btohs(evt->opcode);
	uint16_t ogf = cmd_opcode_ogf(opcode);
	uint16_t ocf = cmd_opcode_ocf(opcode);

	if (ogf == OGF_VENDOR_CMD && (parser.flags & DUMP_NOVENDOR))
		return;

	p_indent(level, frm);
	printf("%s (0x%2.2x|0x%4.4x) status 0x%2.2x ncmd %d\n",
			opcode2str(opcode), ogf, ocf, evt->status, evt->ncmd);

	if (evt->status > 0) {
		p_indent(level, frm);
		printf("Error: %s\n", status2str(evt->status));
	}
}

static inline void hardware_error_dump(int level, struct frame *frm)
{
	evt_hardware_error *evt = frm->ptr;

	p_indent(level, frm);
	printf("code %d\n", evt->code);
}

static inline void inq_result_dump(int level, struct frame *frm)
{
	uint8_t num = p_get_u8(frm);
	char addr[18];
	int i;

	for (i = 0; i < num; i++) {
		inquiry_info *info = frm->ptr;

		p_ba2str(&info->bdaddr, addr);

		p_indent(level, frm);
		printf("bdaddr %s mode %d clkoffset 0x%4.4x class 0x%2.2x%2.2x%2.2x\n",
			addr, info->pscan_rep_mode, btohs(info->clock_offset),
			info->dev_class[2], info->dev_class[1], info->dev_class[0]);

		frm->ptr += INQUIRY_INFO_SIZE;
		frm->len -= INQUIRY_INFO_SIZE;
	}
}

static inline void conn_complete_dump(int level, struct frame *frm)
{
	evt_conn_complete *evt = frm->ptr;
	char addr[18];

	p_ba2str(&evt->bdaddr, addr);

	p_indent(level, frm);
	printf("status 0x%2.2x handle %d bdaddr %s type %s encrypt 0x%2.2x\n",
			evt->status, btohs(evt->handle), addr,
			linktype2str(evt->link_type), evt->encr_mode);

	if (evt->status > 0) {
		p_indent(level, frm);
		printf("Error: %s\n", status2str(evt->status));
	}
}

static inline void conn_request_dump(int level, struct frame *frm)
{
	evt_conn_request *evt = frm->ptr;
	char addr[18];

	p_ba2str(&evt->bdaddr, addr);

	p_indent(level, frm);
	printf("bdaddr %s class 0x%2.2x%2.2x%2.2x type %s\n",
			addr, evt->dev_class[2], evt->dev_class[1],
			evt->dev_class[0], linktype2str(evt->link_type));
}

static inline void disconn_complete_dump(int level, struct frame *frm)
{
	evt_disconn_complete *evt = frm->ptr;

	p_indent(level, frm);
	printf("status 0x%2.2x handle %d reason 0x%2.2x\n",
				evt->status, btohs(evt->handle), evt->reason);

	if (evt->status > 0) {
		p_indent(level, frm);
		printf("Error: %s\n", status2str(evt->status));
	} else if (evt->reason > 0) {
		p_indent(level, frm);
		printf("Reason: %s\n", status2str(evt->reason));
	}
}

static inline void remote_name_req_complete_dump(int level, struct frame *frm)
{
	evt_remote_name_req_complete *evt = frm->ptr;
	char addr[18], name[249];
	int i;

	p_ba2str(&evt->bdaddr, addr);

	memset(name, 0, sizeof(name));
	for (i = 0; i < 248 && evt->name[i]; i++)
		if (isprint(evt->name[i]))
			name[i] = evt->name[i];
		else
			name[i] = '.';

	p_indent(level, frm);
	printf("status 0x%2.2x bdaddr %s name '%s'\n", evt->status, addr, name);

	if (evt->status > 0) {
		p_indent(level, frm);
		printf("Error: %s\n", status2str(evt->status));
	}
}

static inline void master_link_key_complete_dump(int level, struct frame *frm)
{
	evt_master_link_key_complete *evt = frm->ptr;

	p_indent(level, frm);
	printf("status 0x%2.2x handle %d flag %d\n",
				evt->status, btohs(evt->handle), evt->key_flag);

	if (evt->status > 0) {
		p_indent(level, frm);
		printf("Error: %s\n", status2str(evt->status));
	}
}

static inline void encrypt_change_dump(int level, struct frame *frm)
{
	evt_encrypt_change *evt = frm->ptr;

	p_indent(level, frm);
	printf("status 0x%2.2x handle %d encrypt 0x%2.2x\n",
				evt->status, btohs(evt->handle), evt->encrypt);

	if (evt->status > 0) {
		p_indent(level, frm);
		printf("Error: %s\n", status2str(evt->status));
	}
}

static inline void read_remote_features_complete_dump(int level, struct frame *frm)
{
	evt_read_remote_features_complete *evt = frm->ptr;
	int i;

	p_indent(level, frm);
	printf("status 0x%2.2x handle %d\n", evt->status, btohs(evt->handle));

	if (evt->status > 0) {
		p_indent(level, frm);
		printf("Error: %s\n", status2str(evt->status));
	} else {
		p_indent(level, frm);
		printf("Features:");
		for (i = 0; i < 8; i++)
			printf(" 0x%2.2x", evt->features[i]);
		printf("\n");
	}
}

static inline void read_remote_version_complete_dump(int level, struct frame *frm)
{
	evt_read_remote_version_complete *evt = frm->ptr;
	uint16_t manufacturer = btohs(evt->manufacturer);

	p_indent(level, frm);
	printf("status 0x%2.2x handle %d\n", evt->status, btohs(evt->handle));

	if (evt->status > 0) {
		p_indent(level, frm);
		printf("Error: %s\n", status2str(evt->status));
	} else {
		char *lmpver = lmp_vertostr(evt->lmp_ver);

		p_indent(level, frm);
		printf("LMP Version: %s (0x%x) LMP Subversion: 0x%x\n",
			lmpver ? lmpver : "n/a", evt->lmp_ver,
			btohs(evt->lmp_subver));
		p_indent(level, frm);
		printf("Manufacturer: %s (%d)\n",
			bt_compidtostr(manufacturer), manufacturer);

		if (lmpver)
			free(lmpver);
	}
}

static inline void qos_setup_complete_dump(int level, struct frame *frm)
{
	evt_qos_setup_complete *evt = frm->ptr;

	p_indent(level, frm);
	printf("status 0x%2.2x handle %d flags %d\n",
				evt->status, btohs(evt->handle), evt->flags);

	if (evt->status > 0) {
		p_indent(level, frm);
		printf("Error: %s\n", status2str(evt->status));
	} else {
		p_indent(level, frm);
		printf("Service type: %d\n", evt->qos.service_type);
		p_indent(level, frm);
		printf("Token rate: %d\n", btohl(evt->qos.token_rate));
		p_indent(level, frm);
		printf("Peak bandwith: %d\n", btohl(evt->qos.peak_bandwidth));
		p_indent(level, frm);
		printf("Latency: %d\n", btohl(evt->qos.latency));
		p_indent(level, frm);
		printf("Delay variation: %d\n", btohl(evt->qos.delay_variation));
	}
}

static inline void role_change_dump(int level, struct frame *frm)
{
	evt_role_change *evt = frm->ptr;
	char addr[18];

	p_indent(level, frm);
	p_ba2str(&evt->bdaddr, addr);
	printf("status 0x%2.2x bdaddr %s role 0x%2.2x\n",
						evt->status, addr, evt->role);

	if (evt->status > 0) {
		p_indent(level, frm);
		printf("Error: %s\n", status2str(evt->status));
	} else {
		p_indent(level, frm);
		printf("Role: %s\n", role2str(evt->role));
	}
}

static inline void mode_change_dump(int level, struct frame *frm)
{
	evt_mode_change *evt = frm->ptr;

	p_indent(level, frm);
	printf("status 0x%2.2x handle %d mode 0x%2.2x interval %d\n",
		evt->status, btohs(evt->handle), evt->mode, btohs(evt->interval));

	if (evt->status > 0) {
		p_indent(level, frm);
		printf("Error: %s\n", status2str(evt->status));
	} else {
		p_indent(level, frm);
		printf("Mode: %s\n", mode2str(evt->mode));
	}
}

static inline void pin_code_req_dump(int level, struct frame *frm)
{
	evt_pin_code_req *evt = frm->ptr;
	char addr[18];

	p_indent(level, frm);
	p_ba2str(&evt->bdaddr, addr);
	printf("bdaddr %s\n", addr);
}

static inline void link_key_notify_dump(int level, struct frame *frm)
{
	evt_link_key_notify *evt = frm->ptr;
	char addr[18];
	int i;

	p_indent(level, frm);
	p_ba2str(&evt->bdaddr, addr);
	printf("bdaddr %s key ", addr);
	for (i = 0; i < 16; i++)
		if (parser.flags & DUMP_NOVENDOR)
			printf("**");
		else
			printf("%2.2X", evt->link_key[i]);
	printf(" type %d\n", evt->key_type);

	p_indent(level, frm);
	printf("Type: %s\n", keytype2str(evt->key_type));
}

static inline void max_slots_change_dump(int level, struct frame *frm)
{
	evt_max_slots_change *evt = frm->ptr;

	p_indent(level, frm);
	printf("handle %d slots %d\n", btohs(evt->handle), evt->max_slots);
}

static inline void data_buffer_overflow_dump(int level, struct frame *frm)
{
	evt_data_buffer_overflow *evt = frm->ptr;

	p_indent(level, frm);
	printf("type %s\n", linktype2str(evt->link_type));
}

static inline void read_clock_offset_complete_dump(int level, struct frame *frm)
{
	evt_read_clock_offset_complete *evt = frm->ptr;

	p_indent(level, frm);
	printf("status 0x%2.2x handle %d clkoffset 0x%4.4x\n",
		evt->status, btohs(evt->handle), btohs(evt->clock_offset));

	if (evt->status > 0) {
		p_indent(level, frm);
		printf("Error: %s\n", status2str(evt->status));
	}
}

static inline void conn_ptype_changed_dump(int level, struct frame *frm)
{
	evt_conn_ptype_changed *evt = frm->ptr;
	uint16_t ptype = btohs(evt->ptype);
	char *str;

	p_indent(level, frm);
	printf("status 0x%2.2x handle %d ptype 0x%4.4x\n",
				evt->status, btohs(evt->handle), ptype);

	if (evt->status > 0) {
		p_indent(level, frm);
		printf("Error: %s\n", status2str(evt->status));
	} else {
		str = hci_ptypetostr(ptype);
		if (str) {
			p_indent(level, frm);
			printf("Packet type: %s\n", str);
			free(str);
		}
	}
}

static inline void pscan_rep_mode_change_dump(int level, struct frame *frm)
{
	evt_pscan_rep_mode_change *evt = frm->ptr;
	char addr[18];

	p_indent(level, frm);
	p_ba2str(&evt->bdaddr, addr);
	printf("bdaddr %s mode %d\n", addr, evt->pscan_rep_mode);
}

static inline void flow_spec_complete_dump(int level, struct frame *frm)
{
	evt_flow_spec_complete *evt = frm->ptr;

	p_indent(level, frm);
	printf("status 0x%2.2x handle 0x%4.4x flags %d %s\n",
				evt->status, btohs(evt->handle), evt->flags,
				evt->direction == 0 ? "outgoing" : "incoming");

	if (evt->status > 0) {
		p_indent(level, frm);
		printf("Error: %s\n", status2str(evt->status));
	} else {
		p_indent(level, frm);
		printf("Service type: %d\n", evt->qos.service_type);
		p_indent(level, frm);
		printf("Token rate: %d\n", btohl(evt->qos.token_rate));
		p_indent(level, frm);
		printf("Peak bandwith: %d\n", btohl(evt->qos.peak_bandwidth));
		p_indent(level, frm);
		printf("Latency: %d\n", btohl(evt->qos.latency));
		p_indent(level, frm);
		printf("Delay variation: %d\n", btohl(evt->qos.delay_variation));
	}
}

static inline void inq_result_with_rssi_dump(int level, struct frame *frm)
{
	uint8_t num = p_get_u8(frm);
	char addr[18];
	int i;

	if (!num)
		return;

	if (frm->len / num == INQUIRY_INFO_WITH_RSSI_AND_PSCAN_MODE_SIZE) {
		for (i = 0; i < num; i++) {
			inquiry_info_with_rssi_and_pscan_mode *info = frm->ptr;

			p_indent(level, frm);

			p_ba2str(&info->bdaddr, addr);
			printf("bdaddr %s mode %d clkoffset 0x%4.4x class 0x%2.2x%2.2x%2.2x rssi %d\n",
				addr, info->pscan_rep_mode, btohs(info->clock_offset),
				info->dev_class[2], info->dev_class[1], info->dev_class[0], info->rssi);

			frm->ptr += INQUIRY_INFO_WITH_RSSI_AND_PSCAN_MODE_SIZE;
			frm->len -= INQUIRY_INFO_WITH_RSSI_AND_PSCAN_MODE_SIZE;
		}
	} else {
		for (i = 0; i < num; i++) {
			inquiry_info_with_rssi *info = frm->ptr;

			p_indent(level, frm);

			p_ba2str(&info->bdaddr, addr);
			printf("bdaddr %s mode %d clkoffset 0x%4.4x class 0x%2.2x%2.2x%2.2x rssi %d\n",
				addr, info->pscan_rep_mode, btohs(info->clock_offset),
				info->dev_class[2], info->dev_class[1], info->dev_class[0], info->rssi);

			frm->ptr += INQUIRY_INFO_WITH_RSSI_SIZE;
			frm->len -= INQUIRY_INFO_WITH_RSSI_SIZE;
		}
	}
}

static inline void read_remote_ext_features_complete_dump(int level, struct frame *frm)
{
	evt_read_remote_ext_features_complete *evt = frm->ptr;
	int i;

	p_indent(level, frm);
	printf("status 0x%2.2x handle %d page %d max %d\n",
					evt->status, btohs(evt->handle),
					evt->page_num, evt->max_page_num);

	if (evt->status > 0) {
		p_indent(level, frm);
		printf("Error: %s\n", status2str(evt->status));
	} else {
		p_indent(level, frm);
		printf("Features:");
		for (i = 0; i < 8; i++)
			printf(" 0x%2.2x", evt->features[i]);
		printf("\n");
	}
}

static inline void sync_conn_complete_dump(int level, struct frame *frm)
{
	evt_sync_conn_complete *evt = frm->ptr;
	char addr[18];

	p_ba2str(&evt->bdaddr, addr);

	p_indent(level, frm);
	printf("status 0x%2.2x handle %d bdaddr %s type %s\n",
					evt->status, btohs(evt->handle), addr,
					evt->link_type == 0 ? "SCO" : "eSCO");

	if (evt->status > 0) {
		p_indent(level, frm);
		printf("Error: %s\n", status2str(evt->status));
	} else {
		p_indent(level, frm);
		printf("Air mode: %s\n", airmode2str(evt->air_mode));
	}
}

static inline void sync_conn_changed_dump(int level, struct frame *frm)
{
	evt_sync_conn_changed *evt = frm->ptr;

	p_indent(level, frm);
	printf("status 0x%2.2x handle %d\n", evt->status, btohs(evt->handle));

	if (evt->status > 0) {
		p_indent(level, frm);
		printf("Error: %s\n", status2str(evt->status));
	}
}

static inline void sniff_subrating_event_dump(int level, struct frame *frm)
{
	evt_sniff_subrating *evt = frm->ptr;

	p_indent(level, frm);
	printf("status 0x%2.2x handle %d\n", evt->status, btohs(evt->handle));

	if (evt->status > 0) {
		p_indent(level, frm);
		printf("Error: %s\n", status2str(evt->status));
	} else {
		p_indent(level, frm);
		printf("max latency transmit %d receive %d\n",
					btohs(evt->max_tx_latency),
					btohs(evt->max_rx_latency));

		p_indent(level, frm);
		printf("min timeout remote %d local %d\n",
					btohs(evt->min_remote_timeout),
					btohs(evt->min_local_timeout));
	}
}

static inline void extended_inq_result_dump(int level, struct frame *frm)
{
	uint8_t num = p_get_u8(frm);
	char addr[18];
	int i;

	for (i = 0; i < num; i++) {
		extended_inquiry_info *info = frm->ptr;

		p_ba2str(&info->bdaddr, addr);

		p_indent(level, frm);
		printf("bdaddr %s mode %d clkoffset 0x%4.4x class 0x%2.2x%2.2x%2.2x rssi %d\n",
			addr, info->pscan_rep_mode, btohs(info->clock_offset),
			info->dev_class[2], info->dev_class[1], info->dev_class[0], info->rssi);

		frm->ptr += INQUIRY_INFO_WITH_RSSI_SIZE;
		frm->len -= INQUIRY_INFO_WITH_RSSI_SIZE;

		ext_inquiry_response_dump(level, frm);
	}
}

static inline void link_supervision_timeout_changed_dump(int level, struct frame *frm)
{
	evt_link_supervision_timeout_changed *evt = frm->ptr;

	p_indent(level, frm);
	printf("handle %d timeout %d\n",
				btohs(evt->handle), btohs(evt->timeout));
}

static inline void user_passkey_notify_dump(int level, struct frame *frm)
{
	evt_user_passkey_notify *evt = frm->ptr;
	char addr[18];

	p_indent(level, frm);
	p_ba2str(&evt->bdaddr, addr);
	printf("bdaddr %s passkey %d\n", addr, btohl(evt->passkey));
}

static inline void keypress_notify_dump(int level, struct frame *frm)
{
	evt_keypress_notify *evt = frm->ptr;
	char addr[18];

	p_indent(level, frm);
	p_ba2str(&evt->bdaddr, addr);
	printf("bdaddr %s type %d\n", addr, evt->type);
}

static inline void remote_host_features_notify_dump(int level, struct frame *frm)
{
	evt_remote_host_features_notify *evt = frm->ptr;
	char addr[18];
	int i;

	p_indent(level, frm);
	p_ba2str(&evt->bdaddr, addr);
	printf("bdaddr %s\n", addr);

	p_indent(level, frm);
	printf("Features:");
	for (i = 0; i < 8; i++)
		printf(" 0x%2.2x", evt->features[i]);
	printf("\n");
}

static inline void evt_le_conn_complete_dump(int level, struct frame *frm)
{
	evt_le_connection_complete *evt = frm->ptr;
	char addr[18];

	p_indent(level, frm);
	printf("status 0x%2.2x handle %d, role %s\n",
					evt->status, btohs(evt->handle),
					evt->role ? "slave" : "master");

	p_indent(level, frm);
	p_ba2str(&evt->peer_bdaddr, addr);
	printf("bdaddr %s (%s)\n", addr, bdaddrtype2str(evt->peer_bdaddr_type));
}

static inline void evt_le_advertising_report_dump(int level, struct frame *frm)
{
	uint8_t num_reports = p_get_u8(frm);
	const uint8_t RSSI_SIZE = 1;

	while (num_reports--) {
		char addr[18];
		le_advertising_info *info = frm->ptr;
		int offset = 0;

		p_ba2str(&info->bdaddr, addr);

		p_indent(level, frm);
		printf("%s (%d)\n", evttype2str(info->evt_type), info->evt_type);

		p_indent(level, frm);
		printf("bdaddr %s (%s)\n", addr,
					bdaddrtype2str(info->bdaddr_type));

		while (offset < info->length) {
			int eir_data_len = info->data[offset];

			ext_inquiry_data_dump(level, frm, &info->data[offset]);

			offset += eir_data_len + 1;
		}

		frm->ptr += LE_ADVERTISING_INFO_SIZE + info->length;
		frm->len -= LE_ADVERTISING_INFO_SIZE + info->length;

		p_indent(level, frm);
		printf("RSSI: %d\n", ((int8_t *) frm->ptr)[frm->len - 1]);

		frm->ptr += RSSI_SIZE;
		frm->len -= RSSI_SIZE;
	}
}

static inline void evt_le_conn_update_complete_dump(int level,
							struct frame *frm)
{
	evt_le_connection_update_complete *uevt = frm->ptr;

	p_indent(level, frm);
	printf("status 0x%2.2x handle %d\n", uevt->status, btohs(uevt->handle));

	p_indent(level, frm);
	printf("interval %.2fms, latency %.2fms, superv. timeout %.2fms\n",
			btohs(uevt->interval) * 1.25, btohs(uevt->latency) * 1.25,
			btohs(uevt->supervision_timeout) * 10.0);
}

static inline void evt_le_read_remote_used_features_complete_dump(int level, struct frame *frm)
{
	int i;
	evt_le_read_remote_used_features_complete *revt = frm->ptr;

	p_indent(level, frm);
	printf("status 0x%2.2x handle %d\n", revt->status, btohs(revt->handle));

	if (revt->status > 0) {
		p_indent(level, frm);
		printf("Error: %s\n", status2str(revt->status));
	} else {
		p_indent(level, frm);
		printf("Features:");
		for (i = 0; i < 8; i++)
			printf(" 0x%2.2x", revt->features[i]);
		printf("\n");
	}
}

static inline void le_meta_ev_dump(int level, struct frame *frm)
{
	evt_le_meta_event *mevt = frm->ptr;
	uint8_t subevent;

	subevent = mevt->subevent;

	frm->ptr += EVT_LE_META_EVENT_SIZE;
	frm->len -= EVT_LE_META_EVENT_SIZE;

	p_indent(level, frm);
	printf("%s\n", ev_le_meta_str[subevent]);

	switch (mevt->subevent) {
	case EVT_LE_CONN_COMPLETE:
		evt_le_conn_complete_dump(level + 1, frm);
		break;
	case EVT_LE_ADVERTISING_REPORT:
		evt_le_advertising_report_dump(level + 1, frm);
		break;
	case EVT_LE_CONN_UPDATE_COMPLETE:
		evt_le_conn_update_complete_dump(level + 1, frm);
		break;
	case EVT_LE_READ_REMOTE_USED_FEATURES_COMPLETE:
		evt_le_read_remote_used_features_complete_dump(level + 1, frm);
		break;
	default:
		raw_dump(level, frm);
		break;
	}
}

static inline void phys_link_complete_dump(int level, struct frame *frm)
{
	evt_physical_link_complete *evt = frm->ptr;

	p_indent(level, frm);
	printf("status 0x%2.2x phy handle 0x%2.2x\n", evt->status, evt->handle);

	if (evt->status > 0) {
		p_indent(level, frm);
		printf("Error: %s\n", status2str(evt->status));
	}
}

static inline void disconn_phys_link_complete_dump(int level, struct frame *frm)
{
	evt_disconn_physical_link_complete *evt = frm->ptr;

	p_indent(level, frm);
	printf("status 0x%2.2x handle 0x%2.2x reason 0x%2.2x\n",
				evt->status, evt->handle, evt->reason);

	if (evt->status > 0) {
		p_indent(level, frm);
		printf("Error: %s\n", status2str(evt->status));
	} else if (evt->reason > 0) {
		p_indent(level, frm);
		printf("Reason: %s\n", status2str(evt->reason));
	}
}

static inline void phys_link_loss_warning_dump(int level, struct frame *frm)
{
	evt_physical_link_loss_warning *evt = frm->ptr;

	p_indent(level, frm);
	printf("phy handle 0x%2.2x reason 0x%2.2x\n", evt->handle, evt->reason);
}

static inline void phys_link_handle_dump(int level, struct frame *frm)
{
	evt_physical_link_recovery *evt = frm->ptr;

	p_indent(level, frm);
	printf("phy handle 0x%2.2x\n", evt->handle);
}

static inline void logical_link_complete_dump(int level, struct frame *frm)
{
	evt_logical_link_complete *evt = frm->ptr;

	p_indent(level, frm);
	printf("status 0x%2.2x log handle 0x%4.4x phy handle 0x%2.2x"
							" tx_flow_id %d\n",
			evt->status, btohs(evt->log_handle), evt->handle,
			evt->tx_flow_id);

	if (evt->status > 0) {
		p_indent(level, frm);
		printf("Error: %s\n", status2str(evt->status));
	}
}

static inline void flow_spec_modify_dump(int level, struct frame *frm)
{
	evt_flow_spec_modify_complete *evt = frm->ptr;

	p_indent(level, frm);
	printf("status 0x%2.2x handle 0x%4.4x\n",
					evt->status, btohs(evt->handle));

	if (evt->status > 0) {
		p_indent(level, frm);
		printf("Error: %s\n", status2str(evt->status));
	}
}

static inline void num_completed_blocks_dump(int level, struct frame *frm)
{
	evt_num_completed_blocks *evt = frm->ptr;
	int i;

	p_indent(level, frm);
	printf("Total num blocks %d Num handles %d\n",
			btohs(evt->total_num_blocks), evt->num_handles);

	for (i = 0; i < evt->num_handles; i++) {
		cmplt_handle *h = &evt->handles[i];

		p_indent(level + 1, frm);
		printf("Handle 0x%4.4x: Num complt pkts %d Num complt blks %d\n",
				btohs(h->handle), btohs(h->num_cmplt_pkts),
				btohs(h->num_cmplt_blks));
	}
}

static inline void event_dump(int level, struct frame *frm)
{
	hci_event_hdr *hdr = frm->ptr;
	uint8_t event = hdr->evt;

	if (p_filter(FILT_HCI))
		return;

	if (event <= EVENT_NUM) {
		p_indent(level, frm);
		printf("HCI Event: %s (0x%2.2x) plen %d\n",
					event_str[hdr->evt], hdr->evt, hdr->plen);
	} else if (hdr->evt == EVT_TESTING) {
		p_indent(level, frm);
		printf("HCI Event: Testing (0x%2.2x) plen %d\n", hdr->evt, hdr->plen);
	} else if (hdr->evt == EVT_VENDOR) {
		uint16_t manufacturer;

		if (parser.flags & DUMP_NOVENDOR)
			return;

		p_indent(level, frm);
		printf("HCI Event: Vendor (0x%2.2x) plen %d\n", hdr->evt, hdr->plen);

		manufacturer = get_manufacturer();

		switch (manufacturer) {
		case 0:
		case 37:
		case 48:
			frm->ptr += HCI_EVENT_HDR_SIZE;
			frm->len -= HCI_EVENT_HDR_SIZE;
			ericsson_dump(level + 1, frm);
			return;
		case 10:
			frm->ptr += HCI_EVENT_HDR_SIZE;
			frm->len -= HCI_EVENT_HDR_SIZE;
			csr_dump(level + 1, frm);
			return;
		}
	} else {
		p_indent(level, frm);
		printf("HCI Event: code 0x%2.2x plen %d\n", hdr->evt, hdr->plen);
	}

	frm->ptr += HCI_EVENT_HDR_SIZE;
	frm->len -= HCI_EVENT_HDR_SIZE;

	if (event == EVT_CMD_COMPLETE) {
		evt_cmd_complete *cc = frm->ptr;
		if (cc->opcode == cmd_opcode_pack(OGF_INFO_PARAM, OCF_READ_LOCAL_VERSION)) {
			read_local_version_rp *rp = frm->ptr + EVT_CMD_COMPLETE_SIZE;
			manufacturer = rp->manufacturer;
		}
	}

	if (event == EVT_DISCONN_COMPLETE) {
		evt_disconn_complete *evt = frm->ptr;
		l2cap_clear(btohs(evt->handle));
	}

	if (!(parser.flags & DUMP_VERBOSE)) {
		raw_dump(level, frm);
		return;
	}

	switch (event) {
	case EVT_LOOPBACK_COMMAND:
		command_dump(level + 1, frm);
		break;
	case EVT_CMD_COMPLETE:
		cmd_complete_dump(level + 1, frm);
		break;
	case EVT_CMD_STATUS:
		cmd_status_dump(level + 1, frm);
		break;
	case EVT_HARDWARE_ERROR:
		hardware_error_dump(level + 1, frm);
		break;
	case EVT_FLUSH_OCCURRED:
	case EVT_QOS_VIOLATION:
		handle_response_dump(level + 1, frm);
		break;
	case EVT_INQUIRY_COMPLETE:
		status_response_dump(level + 1, frm);
		break;
	case EVT_INQUIRY_RESULT:
		inq_result_dump(level + 1, frm);
		break;
	case EVT_CONN_COMPLETE:
		conn_complete_dump(level + 1, frm);
		break;
	case EVT_CONN_REQUEST:
		conn_request_dump(level + 1, frm);
		break;
	case EVT_DISCONN_COMPLETE:
	case EVT_DISCONNECT_LOGICAL_LINK_COMPLETE:
		disconn_complete_dump(level + 1, frm);
		break;
	case EVT_AUTH_COMPLETE:
	case EVT_CHANGE_CONN_LINK_KEY_COMPLETE:
		generic_response_dump(level + 1, frm);
		break;
	case EVT_MASTER_LINK_KEY_COMPLETE:
		master_link_key_complete_dump(level + 1, frm);
		break;
	case EVT_REMOTE_NAME_REQ_COMPLETE:
		remote_name_req_complete_dump(level + 1, frm);
		break;
	case EVT_ENCRYPT_CHANGE:
		encrypt_change_dump(level + 1, frm);
		break;
	case EVT_READ_REMOTE_FEATURES_COMPLETE:
		read_remote_features_complete_dump(level + 1, frm);
		break;
	case EVT_READ_REMOTE_VERSION_COMPLETE:
		read_remote_version_complete_dump(level + 1, frm);
		break;
	case EVT_QOS_SETUP_COMPLETE:
		qos_setup_complete_dump(level + 1, frm);
		break;
	case EVT_ROLE_CHANGE:
		role_change_dump(level + 1, frm);
		break;
	case EVT_NUM_COMP_PKTS:
		num_comp_pkts_dump(level + 1, frm);
		break;
	case EVT_MODE_CHANGE:
		mode_change_dump(level + 1, frm);
		break;
	case EVT_RETURN_LINK_KEYS:
		return_link_keys_dump(level + 1, frm);
		break;
	case EVT_PIN_CODE_REQ:
	case EVT_LINK_KEY_REQ:
	case EVT_IO_CAPABILITY_REQUEST:
	case EVT_USER_PASSKEY_REQUEST:
	case EVT_REMOTE_OOB_DATA_REQUEST:
		pin_code_req_dump(level + 1, frm);
		break;
	case EVT_LINK_KEY_NOTIFY:
		link_key_notify_dump(level + 1, frm);
		break;
	case EVT_DATA_BUFFER_OVERFLOW:
		data_buffer_overflow_dump(level + 1, frm);
		break;
	case EVT_MAX_SLOTS_CHANGE:
		max_slots_change_dump(level + 1, frm);
		break;
	case EVT_READ_CLOCK_OFFSET_COMPLETE:
		read_clock_offset_complete_dump(level + 1, frm);
		break;
	case EVT_CONN_PTYPE_CHANGED:
		conn_ptype_changed_dump(level + 1, frm);
		break;
	case EVT_PSCAN_REP_MODE_CHANGE:
		pscan_rep_mode_change_dump(level + 1, frm);
		break;
	case EVT_FLOW_SPEC_COMPLETE:
		flow_spec_complete_dump(level + 1, frm);
		break;
	case EVT_INQUIRY_RESULT_WITH_RSSI:
		inq_result_with_rssi_dump(level + 1, frm);
		break;
	case EVT_READ_REMOTE_EXT_FEATURES_COMPLETE:
		read_remote_ext_features_complete_dump(level + 1, frm);
		break;
	case EVT_SYNC_CONN_COMPLETE:
		sync_conn_complete_dump(level + 1, frm);
		break;
	case EVT_SYNC_CONN_CHANGED:
		sync_conn_changed_dump(level + 1, frm);
		break;
	case EVT_SNIFF_SUBRATING:
		sniff_subrating_event_dump(level + 1, frm);
		break;
	case EVT_EXTENDED_INQUIRY_RESULT:
		extended_inq_result_dump(level + 1, frm);
		break;
	case EVT_ENCRYPTION_KEY_REFRESH_COMPLETE:
		generic_response_dump(level + 1, frm);
		break;
	case EVT_SIMPLE_PAIRING_COMPLETE:
		bdaddr_response_dump(level + 1, frm);
		break;
	case EVT_LINK_SUPERVISION_TIMEOUT_CHANGED:
		link_supervision_timeout_changed_dump(level + 1, frm);
		break;
	case EVT_ENHANCED_FLUSH_COMPLETE:
		generic_command_dump(level + 1, frm);
		break;
	case EVT_IO_CAPABILITY_RESPONSE:
		io_capability_reply_dump(level + 1, frm);
		break;
	case EVT_USER_CONFIRM_REQUEST:
	case EVT_USER_PASSKEY_NOTIFY:
		user_passkey_notify_dump(level + 1, frm);
		break;
	case EVT_KEYPRESS_NOTIFY:
		keypress_notify_dump(level + 1, frm);
		break;
	case EVT_REMOTE_HOST_FEATURES_NOTIFY:
		remote_host_features_notify_dump(level + 1, frm);
		break;
	case EVT_LE_META_EVENT:
		le_meta_ev_dump(level + 1, frm);
		break;
	case EVT_PHYSICAL_LINK_COMPLETE:
		phys_link_complete_dump(level + 1, frm);
		break;
	case EVT_DISCONNECT_PHYSICAL_LINK_COMPLETE:
		disconn_phys_link_complete_dump(level + 1, frm);
		break;
	case EVT_PHYSICAL_LINK_LOSS_EARLY_WARNING:
		phys_link_loss_warning_dump(level + 1, frm);
		break;
	case EVT_PHYSICAL_LINK_RECOVERY:
	case EVT_CHANNEL_SELECTED:
		phys_link_handle_dump(level + 1, frm);
		break;
	case EVT_LOGICAL_LINK_COMPLETE:
		logical_link_complete_dump(level + 1, frm);
		break;
	case EVT_FLOW_SPEC_MODIFY_COMPLETE:
		flow_spec_modify_dump(level + 1, frm);
		break;
	case EVT_NUMBER_COMPLETED_BLOCKS:
		num_completed_blocks_dump(level + 1, frm);
		break;
	default:
		raw_dump(level, frm);
		break;
	}
}

static inline void acl_dump(int level, struct frame *frm)
{
	hci_acl_hdr *hdr = (void *) frm->ptr;
	uint16_t handle = btohs(hdr->handle);
	uint16_t dlen = btohs(hdr->dlen);
	uint8_t flags = acl_flags(handle);

	if (!p_filter(FILT_HCI)) {
		p_indent(level, frm);
		printf("ACL data: handle %d flags 0x%2.2x dlen %d\n",
			acl_handle(handle), flags, dlen);
		level++;
	}

	frm->ptr += HCI_ACL_HDR_SIZE;
	frm->len -= HCI_ACL_HDR_SIZE;
	frm->flags  = flags;
	frm->handle = acl_handle(handle);

	if (parser.filter & ~FILT_HCI)
		l2cap_dump(level, frm);
	else
		raw_dump(level, frm);
}

static inline void sco_dump(int level, struct frame *frm)
{
	hci_sco_hdr *hdr = (void *) frm->ptr;
	uint16_t handle = btohs(hdr->handle);
	uint8_t flags = acl_flags(handle);
	int len;

	if (frm->audio_fd > fileno(stderr)) {
		len = write(frm->audio_fd, frm->ptr + HCI_SCO_HDR_SIZE, hdr->dlen);
		if (len < 0)
			return;
	}

	if (!p_filter(FILT_SCO)) {
		p_indent(level, frm);
		printf("SCO data: handle %d flags 0x%2.2x dlen %d\n",
				acl_handle(handle), flags, hdr->dlen);
		level++;

		frm->ptr += HCI_SCO_HDR_SIZE;
		frm->len -= HCI_SCO_HDR_SIZE;
		raw_dump(level, frm);
	}
}

static inline void vendor_dump(int level, struct frame *frm)
{
	if (p_filter(FILT_HCI))
		return;

	if (frm->dev_id == HCI_DEV_NONE) {
		uint16_t device = btohs(htons(p_get_u16(frm)));
		uint16_t proto = btohs(htons(p_get_u16(frm)));
		uint16_t type = btohs(htons(p_get_u16(frm)));
		uint16_t plen = btohs(htons(p_get_u16(frm)));

		p_indent(level, frm);

		printf("System %s: device hci%d proto 0x%2.2x type 0x%2.2x plen %d\n",
			frm->in ? "event" : "command", device, proto, type, plen);

		raw_dump(level, frm);
		return;
	}

	if (parser.flags & DUMP_NOVENDOR)
		return;

	if (get_manufacturer() == 12) {
		bpa_dump(level, frm);
		return;
	}

	p_indent(level, frm);
	printf("Vendor data: len %d\n", frm->len);
	raw_dump(level, frm);
}

void hci_dump(int level, struct frame *frm)
{
	uint8_t type = *(uint8_t *)frm->ptr;

	frm->ptr++; frm->len--;

	switch (type) {
	case HCI_COMMAND_PKT:
		command_dump(level, frm);
		break;

	case HCI_EVENT_PKT:
		event_dump(level, frm);
		break;

	case HCI_ACLDATA_PKT:
		acl_dump(level, frm);
		break;

	case HCI_SCODATA_PKT:
		sco_dump(level, frm);
		break;

	case HCI_VENDOR_PKT:
		vendor_dump(level, frm);
		break;

	default:
		if (p_filter(FILT_HCI))
			break;

		p_indent(level, frm);
		printf("Unknown: type 0x%2.2x len %d\n", type, frm->len);
		raw_dump(level, frm);
		break;
	}
}

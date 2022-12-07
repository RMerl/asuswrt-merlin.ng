// SPDX-License-Identifier: LGPL-2.1-or-later
/*
 *
 *  BlueZ - Bluetooth protocol stack for Linux
 *
 *  Copyright (C) 2011-2014  Intel Corporation
 *  Copyright (C) 2002-2010  Marcel Holtmann <marcel@holtmann.org>
 *
 *
 */

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#define _GNU_SOURCE
#include <stdio.h>
#include <errno.h>
#include <ctype.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <inttypes.h>
#include <time.h>
#include <sys/time.h>
#include <sys/socket.h>

#include "lib/bluetooth.h"
#include "lib/uuid.h"
#include "lib/hci.h"
#include "lib/hci_lib.h"

#include "src/shared/util.h"
#include "src/shared/btsnoop.h"
#include "display.h"
#include "bt.h"
#include "ll.h"
#include "hwdb.h"
#include "keys.h"
#include "l2cap.h"
#include "control.h"
#include "vendor.h"
#include "intel.h"
#include "broadcom.h"
#include "packet.h"

#define COLOR_CHANNEL_LABEL		COLOR_WHITE
#define COLOR_FRAME_LABEL		COLOR_WHITE
#define COLOR_INDEX_LABEL		COLOR_WHITE
#define COLOR_TIMESTAMP			COLOR_YELLOW

#define COLOR_NEW_INDEX			COLOR_GREEN
#define COLOR_DEL_INDEX			COLOR_RED
#define COLOR_OPEN_INDEX		COLOR_GREEN
#define COLOR_CLOSE_INDEX		COLOR_RED
#define COLOR_INDEX_INFO		COLOR_GREEN
#define COLOR_VENDOR_DIAG		COLOR_YELLOW
#define COLOR_SYSTEM_NOTE		COLOR_OFF

#define COLOR_HCI_COMMAND		COLOR_BLUE
#define COLOR_HCI_COMMAND_UNKNOWN	COLOR_WHITE_BG
#define COLOR_HCI_EVENT			COLOR_MAGENTA
#define COLOR_HCI_EVENT_UNKNOWN		COLOR_WHITE_BG
#define COLOR_HCI_ACLDATA		COLOR_CYAN
#define COLOR_HCI_SCODATA		COLOR_YELLOW

#define COLOR_UNKNOWN_ERROR		COLOR_WHITE_BG
#define COLOR_UNKNOWN_FEATURE_BIT	COLOR_WHITE_BG
#define COLOR_UNKNOWN_COMMAND_BIT	COLOR_WHITE_BG
#define COLOR_UNKNOWN_EVENT_MASK	COLOR_WHITE_BG
#define COLOR_UNKNOWN_LE_STATES		COLOR_WHITE_BG
#define COLOR_UNKNOWN_SERVICE_CLASS	COLOR_WHITE_BG
#define COLOR_UNKNOWN_PKT_TYPE_BIT	COLOR_WHITE_BG

#define COLOR_CTRL_OPEN			COLOR_GREEN_BOLD
#define COLOR_CTRL_CLOSE		COLOR_RED_BOLD
#define COLOR_CTRL_COMMAND		COLOR_BLUE_BOLD
#define COLOR_CTRL_COMMAND_UNKNOWN	COLOR_WHITE_BG
#define COLOR_CTRL_EVENT		COLOR_MAGENTA_BOLD
#define COLOR_CTRL_EVENT_UNKNOWN	COLOR_WHITE_BG

#define COLOR_UNKNOWN_OPTIONS_BIT	COLOR_WHITE_BG
#define COLOR_UNKNOWN_SETTINGS_BIT	COLOR_WHITE_BG
#define COLOR_UNKNOWN_ADDRESS_TYPE	COLOR_WHITE_BG
#define COLOR_UNKNOWN_DEVICE_FLAG	COLOR_WHITE_BG
#define COLOR_UNKNOWN_EXP_FEATURE_FLAG	COLOR_WHITE_BG
#define COLOR_UNKNOWN_ADV_FLAG		COLOR_WHITE_BG
#define COLOR_UNKNOWN_PHY		COLOR_WHITE_BG
#define COLOR_UNKNOWN_ADDED_DEVICE_FLAG	COLOR_WHITE_BG
#define COLOR_UNKNOWN_ADVMON_FEATURES	COLOR_WHITE_BG

#define COLOR_PHY_PACKET		COLOR_BLUE

#define UNKNOWN_MANUFACTURER 0xffff

static time_t time_offset = ((time_t) -1);
static int priority_level = BTSNOOP_PRIORITY_INFO;
static unsigned long filter_mask = 0;
static bool index_filter = false;
static uint16_t index_current = 0;
static uint16_t fallback_manufacturer = UNKNOWN_MANUFACTURER;

#define CTRL_RAW  0x0000
#define CTRL_USER 0x0001
#define CTRL_MGMT 0x0002

#define MAX_CTRL 64

struct ctrl_data {
	bool used;
	uint32_t cookie;
	uint16_t format;
	char name[20];
};

static struct ctrl_data ctrl_list[MAX_CTRL];

static void assign_ctrl(uint32_t cookie, uint16_t format, const char *name)
{
	int i;

	for (i = 0; i < MAX_CTRL; i++) {
		if (!ctrl_list[i].used) {
			ctrl_list[i].used = true;
			ctrl_list[i].cookie = cookie;
			ctrl_list[i].format = format;
			if (name) {
				strncpy(ctrl_list[i].name, name, 19);
				ctrl_list[i].name[19] = '\0';
			} else
				strcpy(ctrl_list[i].name, "null");
			break;
		}
	}
}

static void release_ctrl(uint32_t cookie, uint16_t *format, char *name)
{
	int i;

	if (format)
		*format = 0xffff;

	for (i = 0; i < MAX_CTRL; i++) {
		if (ctrl_list[i].used && ctrl_list[i].cookie == cookie) {
			ctrl_list[i].used = false;
			if (format)
				*format = ctrl_list[i].format;
			if (name)
				strncpy(name, ctrl_list[i].name, 20);
			break;
		}
	}
}

static uint16_t get_format(uint32_t cookie)
{
	int i;

	for (i = 0; i < MAX_CTRL; i++) {
		if (ctrl_list[i].used && ctrl_list[i].cookie == cookie)
			return ctrl_list[i].format;
	}

	return 0xffff;
}

#define MAX_CONN 16

struct conn_data {
	uint16_t handle;
	uint8_t  type;
};

static struct conn_data conn_list[MAX_CONN];

static void assign_handle(uint16_t handle, uint8_t type)
{
	int i;

	for (i = 0; i < MAX_CONN; i++) {
		if (conn_list[i].handle == 0x0000) {
			conn_list[i].handle = handle;
			conn_list[i].type = type;
			break;
		}
	}
}

static void release_handle(uint16_t handle)
{
	int i;

	for (i = 0; i < MAX_CONN; i++) {
		if (conn_list[i].handle == handle) {
			conn_list[i].handle = 0x0000;
			conn_list[i].type = 0x00;
			break;
		}
	}
}

static uint8_t get_type(uint16_t handle)
{
	int i;

	for (i = 0; i < MAX_CONN; i++) {
		if (conn_list[i].handle == handle)
			return conn_list[i].type;
	}

	return 0xff;
}

bool packet_has_filter(unsigned long filter)
{
	return filter_mask & filter;
}

void packet_set_filter(unsigned long filter)
{
	filter_mask = filter;
}

void packet_add_filter(unsigned long filter)
{
	if (index_filter)
		filter &= ~PACKET_FILTER_SHOW_INDEX;

	filter_mask |= filter;
}

void packet_del_filter(unsigned long filter)
{
	filter_mask &= ~filter;
}

void packet_set_priority(const char *priority)
{
	if (!priority)
		return;

	if (!strcasecmp(priority, "debug"))
		priority_level = BTSNOOP_PRIORITY_DEBUG;
	else
		priority_level = atoi(priority);
}

void packet_select_index(uint16_t index)
{
	filter_mask &= ~PACKET_FILTER_SHOW_INDEX;

	control_filter_index(index);

	index_filter = true;
}

#define print_space(x) printf("%*c", (x), ' ');

#define MAX_INDEX 16

struct index_data {
	uint8_t  type;
	uint8_t  bdaddr[6];
	uint16_t manufacturer;
	uint16_t msft_opcode;
	size_t   frame;
};

static struct index_data index_list[MAX_INDEX];

void packet_set_fallback_manufacturer(uint16_t manufacturer)
{
	int i;

	for (i = 0; i < MAX_INDEX; i++)
		index_list[i].manufacturer = manufacturer;

	fallback_manufacturer = manufacturer;
}

static void print_packet(struct timeval *tv, struct ucred *cred, char ident,
					uint16_t index, const char *channel,
					const char *color, const char *label,
					const char *text, const char *extra)
{
	int col = num_columns();
	char line[256], ts_str[96];
	int n, ts_len = 0, ts_pos = 0, len = 0, pos = 0;
	static size_t last_frame;

	if (channel) {
		if (use_color()) {
			n = sprintf(ts_str + ts_pos, "%s", COLOR_CHANNEL_LABEL);
			if (n > 0)
				ts_pos += n;
		}

		n = sprintf(ts_str + ts_pos, " {%s}", channel);
		if (n > 0) {
			ts_pos += n;
			ts_len += n;
		}
	} else if (index != HCI_DEV_NONE && index < MAX_INDEX &&
				index_list[index].frame != last_frame) {
		if (use_color()) {
			n = sprintf(ts_str + ts_pos, "%s", COLOR_FRAME_LABEL);
			if (n > 0)
				ts_pos += n;
		}

		n = sprintf(ts_str + ts_pos, " #%zu", index_list[index].frame);
		if (n > 0) {
			ts_pos += n;
			ts_len += n;
		}
		last_frame = index_list[index].frame;
	}

	if ((filter_mask & PACKET_FILTER_SHOW_INDEX) &&
					index != HCI_DEV_NONE) {
		if (use_color()) {
			n = sprintf(ts_str + ts_pos, "%s", COLOR_INDEX_LABEL);
			if (n > 0)
				ts_pos += n;
		}

		n = sprintf(ts_str + ts_pos, " [hci%d]", index);
		if (n > 0) {
			ts_pos += n;
			ts_len += n;
		}
	}

	if (tv) {
		time_t t = tv->tv_sec;
		struct tm tm;

		localtime_r(&t, &tm);

		if (use_color()) {
			n = sprintf(ts_str + ts_pos, "%s", COLOR_TIMESTAMP);
			if (n > 0)
				ts_pos += n;
		}

		if (filter_mask & PACKET_FILTER_SHOW_DATE) {
			n = sprintf(ts_str + ts_pos, " %04d-%02d-%02d",
				tm.tm_year + 1900, tm.tm_mon + 1, tm.tm_mday);
			if (n > 0) {
				ts_pos += n;
				ts_len += n;
			}
		}

		if (filter_mask & PACKET_FILTER_SHOW_TIME) {
			n = sprintf(ts_str + ts_pos, " %02d:%02d:%02d.%06lu",
				tm.tm_hour, tm.tm_min, tm.tm_sec, tv->tv_usec);
			if (n > 0) {
				ts_pos += n;
				ts_len += n;
			}
		}

		if (filter_mask & PACKET_FILTER_SHOW_TIME_OFFSET) {
			n = sprintf(ts_str + ts_pos, " %lu.%06lu",
					tv->tv_sec - time_offset, tv->tv_usec);
			if (n > 0) {
				ts_pos += n;
				ts_len += n;
			}
		}
	}

	if (use_color()) {
		n = sprintf(ts_str + ts_pos, "%s", COLOR_OFF);
		if (n > 0)
			ts_pos += n;
	}

	if (use_color()) {
		n = sprintf(line + pos, "%s", color);
		if (n > 0)
			pos += n;
	}

	n = sprintf(line + pos, "%c %s", ident, label ? label : "");
	if (n > 0) {
		pos += n;
		len += n;
	}

	if (text) {
		int extra_len = extra ? strlen(extra) : 0;
		int max_len = col - len - extra_len - ts_len - 3;

		n = snprintf(line + pos, max_len + 1, "%s%s",
						label ? ": " : "", text);
		if (n > max_len) {
			line[pos + max_len - 1] = '.';
			line[pos + max_len - 2] = '.';
			if (line[pos + max_len - 3] == ' ')
				line[pos + max_len - 3] = '.';

			n = max_len;
		}

		if (n > 0) {
			pos += n;
			len += n;
		}
	}

	if (use_color()) {
		n = sprintf(line + pos, "%s", COLOR_OFF);
		if (n > 0)
			pos += n;
	}

	if (extra) {
		n = sprintf(line + pos, " %s", extra);
		if (n > 0) {
			pos += n;
			len += n;
		}
	}

	if (ts_len > 0) {
		printf("%s", line);
		if (len < col)
			print_space(col - len - ts_len - 1);
		printf("%s%s\n", use_color() ? COLOR_TIMESTAMP : "", ts_str);
	} else
		printf("%s\n", line);
}

static const struct {
	uint8_t error;
	const char *str;
} error2str_table[] = {
	{ 0x00, "Success"						},
	{ 0x01, "Unknown HCI Command"					},
	{ 0x02, "Unknown Connection Identifier"				},
	{ 0x03, "Hardware Failure"					},
	{ 0x04, "Page Timeout"						},
	{ 0x05, "Authentication Failure"				},
	{ 0x06, "PIN or Key Missing"					},
	{ 0x07, "Memory Capacity Exceeded"				},
	{ 0x08, "Connection Timeout"					},
	{ 0x09, "Connection Limit Exceeded"				},
	{ 0x0a, "Synchronous Connection Limit to a Device Exceeded"	},
	{ 0x0b, "ACL Connection Already Exists"				},
	{ 0x0c, "Command Disallowed"					},
	{ 0x0d, "Connection Rejected due to Limited Resources"		},
	{ 0x0e, "Connection Rejected due to Security Reasons"		},
	{ 0x0f, "Connection Rejected due to Unacceptable BD_ADDR"	},
	{ 0x10, "Connection Accept Timeout Exceeded"			},
	{ 0x11, "Unsupported Feature or Parameter Value"		},
	{ 0x12, "Invalid HCI Command Parameters"			},
	{ 0x13, "Remote User Terminated Connection"			},
	{ 0x14, "Remote Device Terminated due to Low Resources"		},
	{ 0x15, "Remote Device Terminated due to Power Off"		},
	{ 0x16, "Connection Terminated By Local Host"			},
	{ 0x17, "Repeated Attempts"					},
	{ 0x18, "Pairing Not Allowed"					},
	{ 0x19, "Unknown LMP PDU"					},
	{ 0x1a, "Unsupported Remote Feature / Unsupported LMP Feature"	},
	{ 0x1b, "SCO Offset Rejected"					},
	{ 0x1c, "SCO Interval Rejected"					},
	{ 0x1d, "SCO Air Mode Rejected"					},
	{ 0x1e, "Invalid LMP Parameters / Invalid LL Parameters"	},
	{ 0x1f, "Unspecified Error"					},
	{ 0x20, "Unsupported LMP Parameter Value / "
		"Unsupported LL Parameter Value"			},
	{ 0x21, "Role Change Not Allowed"				},
	{ 0x22, "LMP Response Timeout / LL Response Timeout"		},
	{ 0x23, "LMP Error Transaction Collision"			},
	{ 0x24, "LMP PDU Not Allowed"					},
	{ 0x25, "Encryption Mode Not Acceptable"			},
	{ 0x26, "Link Key cannot be Changed"				},
	{ 0x27, "Requested QoS Not Supported"				},
	{ 0x28, "Instant Passed"					},
	{ 0x29, "Pairing With Unit Key Not Supported"			},
	{ 0x2a, "Different Transaction Collision"			},
	{ 0x2b, "Reserved"						},
	{ 0x2c, "QoS Unacceptable Parameter"				},
	{ 0x2d, "QoS Rejected"						},
	{ 0x2e, "Channel Classification Not Supported"			},
	{ 0x2f, "Insufficient Security"					},
	{ 0x30, "Parameter Out Of Manadatory Range"			},
	{ 0x31, "Reserved"						},
	{ 0x32, "Role Switch Pending"					},
	{ 0x33, "Reserved"						},
	{ 0x34, "Reserved Slot Violation"				},
	{ 0x35, "Role Switch Failed"					},
	{ 0x36, "Extended Inquiry Response Too Large"			},
	{ 0x37, "Secure Simple Pairing Not Supported By Host"		},
	{ 0x38, "Host Busy - Pairing"					},
	{ 0x39, "Connection Rejected due to No Suitable Channel Found"	},
	{ 0x3a, "Controller Busy"					},
	{ 0x3b, "Unacceptable Connection Parameters"			},
	{ 0x3c, "Advertising Timeout"					},
	{ 0x3d, "Connection Terminated due to MIC Failure"		},
	{ 0x3e, "Connection Failed to be Established"			},
	{ 0x3f, "MAC Connection Failed"					},
	{ 0x40, "Coarse Clock Adjustment Rejected "
		"but Will Try to Adjust Using Clock Dragging"		},
	{ 0x41, "Type0 Submap Not Defined"				},
	{ 0x42, "Unknown Advertising Identifier"			},
	{ 0x43, "Limit Reached"						},
	{ 0x44, "Operation Cancelled by Host"				},
	{ 0x45, "Packet Too Long"					},
	{ }
};

static void print_error(const char *label, uint8_t error)
{
	const char *str = "Unknown";
	const char *color_on, *color_off;
	bool unknown = true;
	int i;

	for (i = 0; error2str_table[i].str; i++) {
		if (error2str_table[i].error == error) {
			str = error2str_table[i].str;
			unknown = false;
			break;
		}
	}

	if (use_color()) {
		if (error) {
			if (unknown)
				color_on = COLOR_UNKNOWN_ERROR;
			else
				color_on = COLOR_RED;
		} else
			color_on = COLOR_GREEN;
		color_off = COLOR_OFF;
	} else {
		color_on = "";
		color_off = "";
	}

	print_field("%s: %s%s%s (0x%2.2x)", label,
				color_on, str, color_off, error);
}

static void print_status(uint8_t status)
{
	print_error("Status", status);
}

static void print_reason(uint8_t reason)
{
	print_error("Reason", reason);
}

void packet_print_error(const char *label, uint8_t error)
{
	print_error(label, error);
}

static void print_enable(const char *label, uint8_t enable)
{
	const char *str;

	switch (enable) {
	case 0x00:
		str = "Disabled";
		break;
	case 0x01:
		str = "Enabled";
		break;
	default:
		str = "Reserved";
		break;
	}

	print_field("%s: %s (0x%2.2x)", label, str, enable);
}

static void print_addr_type(const char *label, uint8_t addr_type)
{
	const char *str;

	switch (addr_type) {
	case 0x00:
		str = "Public";
		break;
	case 0x01:
		str = "Random";
		break;
	default:
		str = "Reserved";
		break;
	}

	print_field("%s: %s (0x%2.2x)", label, str, addr_type);
}

static void print_own_addr_type(uint8_t addr_type)
{
	const char *str;

	switch (addr_type) {
	case 0x00:
	case 0x02:
		str = "Public";
		break;
	case 0x01:
	case 0x03:
		str = "Random";
		break;
	default:
		str = "Reserved";
		break;
	}

	print_field("Own address type: %s (0x%2.2x)", str, addr_type);
}

static void print_peer_addr_type(const char *label, uint8_t addr_type)
{
	const char *str;

	switch (addr_type) {
	case 0x00:
		str = "Public";
		break;
	case 0x01:
		str = "Random";
		break;
	case 0x02:
		str = "Resolved Public";
		break;
	case 0x03:
		str = "Resolved Random";
		break;
	default:
		str = "Reserved";
		break;
	}

	print_field("%s: %s (0x%2.2x)", label, str, addr_type);
}

static void print_addr_resolve(const char *label, const uint8_t *addr,
					uint8_t addr_type, bool resolve)
{
	const char *str;
	char *company;

	switch (addr_type) {
	case 0x00:
	case 0x02:
		if (!hwdb_get_company(addr, &company))
			company = NULL;

		if (company) {
			print_field("%s: %2.2X:%2.2X:%2.2X:%2.2X:%2.2X:%2.2X"
					" (%s)", label, addr[5], addr[4],
							addr[3], addr[2],
							addr[1], addr[0],
							company);
			free(company);
		} else {
			print_field("%s: %2.2X:%2.2X:%2.2X:%2.2X:%2.2X:%2.2X"
					" (OUI %2.2X-%2.2X-%2.2X)", label,
						addr[5], addr[4], addr[3],
						addr[2], addr[1], addr[0],
						addr[5], addr[4], addr[3]);
		}
		break;
	case 0x01:
	case 0x03:
		switch ((addr[5] & 0xc0) >> 6) {
		case 0x00:
			str = "Non-Resolvable";
			break;
		case 0x01:
			str = "Resolvable";
			break;
		case 0x03:
			str = "Static";
			break;
		default:
			str = "Reserved";
			break;
		}

		print_field("%s: %2.2X:%2.2X:%2.2X:%2.2X:%2.2X:%2.2X (%s)",
					label, addr[5], addr[4], addr[3],
					addr[2], addr[1], addr[0], str);

		if (resolve && (addr[5] & 0xc0) == 0x40) {
			uint8_t ident[6], ident_type;

			if (keys_resolve_identity(addr, ident, &ident_type)) {
				print_addr_type("  Identity type", ident_type);
				print_addr_resolve("  Identity", ident,
							ident_type, false);
			}
		}
		break;
	default:
		print_field("%s: %2.2X-%2.2X-%2.2X-%2.2X-%2.2X-%2.2X",
					label, addr[5], addr[4], addr[3],
					addr[2], addr[1], addr[0]);
		break;
	}
}

static void print_addr(const char *label, const uint8_t *addr,
						uint8_t addr_type)
{
	print_addr_resolve(label, addr, addr_type, true);
}

static void print_bdaddr(const uint8_t *bdaddr)
{
	print_addr("Address", bdaddr, 0x00);
}

static void print_lt_addr(uint8_t lt_addr)
{
	print_field("LT address: %d", lt_addr);
}

static void print_handle_native(uint16_t handle)
{
	print_field("Handle: %d", handle);
}

static void print_handle(uint16_t handle)
{
	print_handle_native(le16_to_cpu(handle));
}

static void print_phy_handle(uint8_t phy_handle)
{
	print_field("Physical handle: %d", phy_handle);
}

static const struct bitfield_data pkt_type_table[] = {
	{  1, "2-DH1 may not be used"	},
	{  2, "3-DH1 may not be used"	},
	{  3, "DM1 may be used"		},
	{  4, "DH1 may be used"		},
	{  8, "2-DH3 may not be used"	},
	{  9, "3-DH3 may not be used"	},
	{ 10, "DM3 may be used"		},
	{ 11, "DH3 may be used"		},
	{ 12, "2-DH5 may not be used"	},
	{ 13, "3-DH5 may not be used"	},
	{ 14, "DM5 may be used"		},
	{ 15, "DH5 may be used"		},
	{ }
};

static void print_pkt_type(uint16_t pkt_type)
{
	uint16_t mask = le16_to_cpu(pkt_type);

	print_field("Packet type: 0x%4.4x", mask);

	mask = print_bitfield(2, mask, pkt_type_table);
	if (mask)
		print_text(COLOR_UNKNOWN_PKT_TYPE_BIT,
				"  Unknown packet types (0x%4.4x)", mask);
}

static const struct bitfield_data pkt_type_sco_table[] = {
	{  0, "HV1 may be used"		},
	{  1, "HV2 may be used"		},
	{  2, "HV3 may be used"		},
	{  3, "EV3 may be used"		},
	{  4, "EV4 may be used"		},
	{  5, "EV5 may be used"		},
	{  6, "2-EV3 may not be used"	},
	{  7, "3-EV3 may not be used"	},
	{  8, "2-EV5 may not be used"	},
	{  9, "3-EV5 may not be used"	},
	{ }
};

static void print_pkt_type_sco(uint16_t pkt_type)
{
	uint16_t mask = le16_to_cpu(pkt_type);

	print_field("Packet type: 0x%4.4x", mask);

	mask = print_bitfield(2, mask, pkt_type_sco_table);
	if (mask)
		print_text(COLOR_UNKNOWN_PKT_TYPE_BIT,
				"  Unknown packet types (0x%4.4x)", mask);
}

static void print_iac(const uint8_t *lap)
{
	const char *str = "";

	if (lap[2] == 0x9e && lap[1] == 0x8b) {
		switch (lap[0]) {
		case 0x33:
			str = " (General Inquiry)";
			break;
		case 0x00:
			str = " (Limited Inquiry)";
			break;
		}
	}

	print_field("Access code: 0x%2.2x%2.2x%2.2x%s",
						lap[2], lap[1], lap[0], str);
}

static void print_auth_enable(uint8_t enable)
{
	const char *str;

	switch (enable) {
	case 0x00:
		str = "Authentication not required";
		break;
	case 0x01:
		str = "Authentication required for all connections";
		break;
	default:
		str = "Reserved";
		break;
	}

	print_field("Enable: %s (0x%2.2x)", str, enable);
}

static void print_encrypt_mode(uint8_t mode)
{
	const char *str;

	switch (mode) {
	case 0x00:
		str = "Encryption not required";
		break;
	case 0x01:
		str = "Encryption required for all connections";
		break;
	default:
		str = "Reserved";
		break;
	}

	print_field("Mode: %s (0x%2.2x)", str, mode);
}

static const struct bitfield_data svc_class_table[] = {
	{ 0, "Positioning (Location identification)"		},
	{ 1, "Networking (LAN, Ad hoc)"				},
	{ 2, "Rendering (Printing, Speaker)"			},
	{ 3, "Capturing (Scanner, Microphone)"			},
	{ 4, "Object Transfer (v-Inbox, v-Folder)"		},
	{ 5, "Audio (Speaker, Microphone, Headset)"		},
	{ 6, "Telephony (Cordless telephony, Modem, Headset)"	},
	{ 7, "Information (WEB-server, WAP-server)"		},
	{ }
};

static const struct {
	uint8_t val;
	const char *str;
} major_class_computer_table[] = {
	{ 0x00, "Uncategorized, code for device not assigned"	},
	{ 0x01, "Desktop workstation"				},
	{ 0x02, "Server-class computer"				},
	{ 0x03, "Laptop"					},
	{ 0x04, "Handheld PC/PDA (clam shell)"			},
	{ 0x05, "Palm sized PC/PDA"				},
	{ 0x06, "Wearable computer (Watch sized)"		},
	{ 0x07, "Tablet"					},
	{ }
};

static const char *major_class_computer(uint8_t minor)
{
	int i;

	for (i = 0; major_class_computer_table[i].str; i++) {
		if (major_class_computer_table[i].val == minor)
			return major_class_computer_table[i].str;
	}

	return NULL;
}

static const struct {
	uint8_t val;
	const char *str;
} major_class_phone_table[] = {
	{ 0x00, "Uncategorized, code for device not assigned"	},
	{ 0x01, "Cellular"					},
	{ 0x02, "Cordless"					},
	{ 0x03, "Smart phone"					},
	{ 0x04, "Wired modem or voice gateway"			},
	{ 0x05, "Common ISDN Access"				},
	{ }
};

static const char *major_class_phone(uint8_t minor)
{
	int i;

	for (i = 0; major_class_phone_table[i].str; i++) {
		if (major_class_phone_table[i].val == minor)
			return major_class_phone_table[i].str;
	}

	return NULL;
}

static const struct {
	uint8_t val;
	const char *str;
} major_class_av_table[] = {
	{ 0x00, "Uncategorized, code for device not assigned"	},
	{ 0x01, "Wearable Headset Device"			},
	{ 0x02, "Hands-free Device"				},
	{ 0x04, "Microphone"					},
	{ 0x05, "Loudspeaker"					},
	{ 0x06, "Headphones"					},
	{ 0x07, "Portable Audio"				},
	{ 0x08, "Car audio"					},
	{ 0x09, "Set-top box"					},
	{ 0x0a, "HiFi Audio Device"				},
	{ 0x0b, "VCR"						},
	{ 0x0c, "Video Camera"					},
	{ 0x0d, "Camcorder"					},
	{ 0x0e, "Video Monitor"					},
	{ 0x0f, "Video Display and Loudspeaker"			},
	{ 0x10, "Video Conferencing"				},
	{ 0x12, "Gaming/Toy"					},
	{ }
};

static const char *major_class_av(uint8_t minor)
{
	int i;

	for (i = 0; major_class_av_table[i].str; i++) {
		if (major_class_av_table[i].val == minor)
			return major_class_av_table[i].str;
	}

	return NULL;
}

static const struct {
	uint8_t val;
	const char *str;
} major_class_wearable_table[] = {
	{ 0x01, "Wrist Watch"	},
	{ 0x02, "Pager"		},
	{ 0x03, "Jacket"	},
	{ 0x04, "Helmet"	},
	{ 0x05, "Glasses"	},
	{ }
};

static const char *major_class_wearable(uint8_t minor)
{
	int i;

	for (i = 0; major_class_wearable_table[i].str; i++) {
		if (major_class_wearable_table[i].val == minor)
			return major_class_wearable_table[i].str;
	}

	return NULL;
}

static const struct {
	uint8_t val;
	const char *str;
	const char *(*func)(uint8_t minor);
} major_class_table[] = {
	{ 0x00, "Miscellaneous"						},
	{ 0x01, "Computer (desktop, notebook, PDA, organizers)",
						major_class_computer	},
	{ 0x02, "Phone (cellular, cordless, payphone, modem)",
						major_class_phone	},
	{ 0x03, "LAN /Network Access point"				},
	{ 0x04, "Audio/Video (headset, speaker, stereo, video, vcr)",
						major_class_av		},
	{ 0x05, "Peripheral (mouse, joystick, keyboards)"		},
	{ 0x06, "Imaging (printing, scanner, camera, display)"		},
	{ 0x07, "Wearable",			major_class_wearable	},
	{ 0x08, "Toy"							},
	{ 0x09, "Health"						},
	{ 0x1f, "Uncategorized, specific device code not specified"	},
	{ }
};

static void print_dev_class(const uint8_t *dev_class)
{
	uint8_t mask, major_cls, minor_cls;
	const char *major_str = NULL;
	const char *minor_str = NULL;
	int i;

	print_field("Class: 0x%2.2x%2.2x%2.2x",
			dev_class[2], dev_class[1], dev_class[0]);

	if ((dev_class[0] & 0x03) != 0x00) {
		print_field("  Format type: 0x%2.2x", dev_class[0] & 0x03);
		print_text(COLOR_ERROR, "  invalid format type");
		return;
	}

	major_cls = dev_class[1] & 0x1f;
	minor_cls = (dev_class[0] & 0xfc) >> 2;

	for (i = 0; major_class_table[i].str; i++) {
		if (major_class_table[i].val == major_cls) {
			major_str = major_class_table[i].str;

			if (!major_class_table[i].func)
				break;

			minor_str = major_class_table[i].func(minor_cls);
			break;
		}
	}

	if (major_str) {
		print_field("  Major class: %s", major_str);
		if (minor_str)
			print_field("  Minor class: %s", minor_str);
		else
			print_field("  Minor class: 0x%2.2x", minor_cls);
	} else {
		print_field("  Major class: 0x%2.2x", major_cls);
		print_field("  Minor class: 0x%2.2x", minor_cls);
	}

	if (dev_class[1] & 0x20)
		print_field("  Limited Discoverable Mode");

	if ((dev_class[1] & 0xc0) != 0x00) {
		print_text(COLOR_ERROR, "  invalid service class");
		return;
	}

	mask = print_bitfield(2, dev_class[2], svc_class_table);
	if (mask)
		print_text(COLOR_UNKNOWN_SERVICE_CLASS,
				"  Unknown service class (0x%2.2x)", mask);
}

static void print_appearance(uint16_t appearance)
{
	print_field("Appearance: %s (0x%4.4x)", bt_appear_to_str(appearance),
								appearance);
}

static void print_num_broadcast_retrans(uint8_t num_retrans)
{
	print_field("Number of broadcast retransmissions: %u", num_retrans);
}

static void print_hold_mode_activity(uint8_t activity)
{
	print_field("Activity: 0x%2.2x", activity);

	if (activity == 0x00) {
		print_field("  Maintain current Power State");
		return;
	}

	if (activity & 0x01)
		print_field("  Suspend Page Scan");
	if (activity & 0x02)
		print_field("  Suspend Inquiry Scan");
	if (activity & 0x04)
		print_field("  Suspend Periodic Inquiries");
}

static void print_power_type(uint8_t type)
{
	const char *str;

	switch (type) {
	case 0x00:
		str = "Current Transmit Power Level";
		break;
	case 0x01:
		str = "Maximum Transmit Power Level";
		break;
	default:
		str = "Reserved";
		break;
	}

	print_field("Type: %s (0x%2.2x)", str, type);
}

static void print_power_level(int8_t level, const char *type)
{
	print_field("TX power%s%s%s: %d dbm (0x%2.2x)",
		type ? " (" : "", type ? type : "", type ? ")" : "",
							level, (uint8_t) level);
}

static void print_host_flow_control(uint8_t enable)
{
	const char *str;

	switch (enable) {
	case 0x00:
		str = "Off";
		break;
	case 0x01:
		str = "ACL Data Packets";
		break;
	case 0x02:
		str = "Synchronous Data Packets";
		break;
	case 0x03:
		str = "ACL and Synchronous Data Packets";
		break;
	default:
		str = "Reserved";
		break;
	}

	print_field("Flow control: %s (0x%2.2x)", str, enable);
}

static void print_voice_setting(uint16_t setting)
{
	uint8_t input_coding = (le16_to_cpu(setting) & 0x0300) >> 8;
	uint8_t input_data_format = (le16_to_cpu(setting) & 0xc0) >> 6;
	uint8_t air_coding_format = le16_to_cpu(setting) & 0x0003;
	const char *str;

	print_field("Setting: 0x%4.4x", le16_to_cpu(setting));

	switch (input_coding) {
	case 0x00:
		str = "Linear";
		break;
	case 0x01:
		str = "u-law";
		break;
	case 0x02:
		str = "A-law";
		break;
	default:
		str = "Reserved";
		break;
	}

	print_field("  Input Coding: %s", str);

	switch (input_data_format) {
	case 0x00:
		str = "1's complement";
		break;
	case 0x01:
		str = "2's complement";
		break;
	case 0x02:
		str = "Sign-Magnitude";
		break;
	case 0x03:
		str = "Unsigned";
		break;
	default:
		str = "Reserved";
		break;
	}

	print_field("  Input Data Format: %s", str);

	if (input_coding == 0x00) {
		print_field("  Input Sample Size: %s",
			le16_to_cpu(setting) & 0x20 ? "16-bit" : "8-bit");
		print_field("  # of bits padding at MSB: %d",
					(le16_to_cpu(setting) & 0x1c) >> 2);
	}

	switch (air_coding_format) {
	case 0x00:
		str = "CVSD";
		break;
	case 0x01:
		str = "u-law";
		break;
	case 0x02:
		str = "A-law";
		break;
	case 0x03:
		str = "Transparent Data";
		break;
	default:
		str = "Reserved";
		break;
	}

	print_field("  Air Coding Format: %s", str);
}

static void print_retransmission_effort(uint8_t effort)
{
	const char *str;

	switch (effort) {
	case 0x00:
		str = "No retransmissions";
		break;
	case 0x01:
		str = "Optimize for power consumption";
		break;
	case 0x02:
		str = "Optimize for link quality";
		break;
	case 0xff:
		str = "Don't care";
		break;
	default:
		str = "Reserved";
		break;
	}

	print_field("Retransmission effort: %s (0x%2.2x)", str, effort);
}

static void print_scan_enable(uint8_t scan_enable)
{
	const char *str;

	switch (scan_enable) {
	case 0x00:
		str = "No Scans";
		break;
	case 0x01:
		str = "Inquiry Scan";
		break;
	case 0x02:
		str = "Page Scan";
		break;
	case 0x03:
		str = "Inquiry Scan + Page Scan";
		break;
	default:
		str = "Reserved";
		break;
	}

	print_field("Scan enable: %s (0x%2.2x)", str, scan_enable);
}

static void print_link_policy(uint16_t link_policy)
{
	uint16_t policy = le16_to_cpu(link_policy);

	print_field("Link policy: 0x%4.4x", policy);

	if (policy == 0x0000) {
		print_field("  Disable All Modes");
		return;
	}

	if (policy & 0x0001)
		print_field("  Enable Role Switch");
	if (policy & 0x0002)
		print_field("  Enable Hold Mode");
	if (policy & 0x0004)
		print_field("  Enable Sniff Mode");
	if (policy & 0x0008)
		print_field("  Enable Park State");
}

static void print_air_mode(uint8_t mode)
{
	const char *str;

	switch (mode) {
	case 0x00:
		str = "u-law log";
		break;
	case 0x01:
		str = "A-law log";
		break;
	case 0x02:
		str = "CVSD";
		break;
	case 0x03:
		str = "Transparent";
		break;
	default:
		str = "Reserved";
		break;
	}

	print_field("Air mode: %s (0x%2.2x)", str, mode);
}

static void print_codec(const char *label, uint8_t codec)
{
	const char *str;

	switch (codec) {
	case 0x00:
		str = "u-law log";
		break;
	case 0x01:
		str = "A-law log";
		break;
	case 0x02:
		str = "CVSD";
		break;
	case 0x03:
		str = "Transparent";
		break;
	case 0x04:
		str = "Linear PCM";
		break;
	case 0x05:
		str = "mSBC";
		break;
	case 0xff:
		str = "Vendor specific";
		break;
	default:
		str = "Reserved";
		break;
	}

	print_field("%s: %s (0x%2.2x)", label, str, codec);
}

static void print_inquiry_mode(uint8_t mode)
{
	const char *str;

	switch (mode) {
	case 0x00:
		str = "Standard Inquiry Result";
		break;
	case 0x01:
		str = "Inquiry Result with RSSI";
		break;
	case 0x02:
		str = "Inquiry Result with RSSI or Extended Inquiry Result";
		break;
	default:
		str = "Reserved";
		break;
	}

	print_field("Mode: %s (0x%2.2x)", str, mode);
}

static void print_inquiry_scan_type(uint8_t type)
{
	const char *str;

	switch (type) {
	case 0x00:
		str = "Standard Scan";
		break;
	case 0x01:
		str = "Interlaced Scan";
		break;
	default:
		str = "Reserved";
		break;
	}

	print_field("Type: %s (0x%2.2x)", str, type);
}

static void print_pscan_type(uint8_t type)
{
	const char *str;

	switch (type) {
	case 0x00:
		str = "Standard Scan";
		break;
	case 0x01:
		str = "Interlaced Scan";
		break;
	default:
		str = "Reserved";
		break;
	}

	print_field("Type: %s (0x%2.2x)", str, type);
}

static void print_loopback_mode(uint8_t mode)
{
	const char *str;

	switch (mode) {
	case 0x00:
		str = "No Loopback";
		break;
	case 0x01:
		str = "Local Loopback";
		break;
	case 0x02:
		str = "Remote Loopback";
		break;
	default:
		str = "Reserved";
		break;
	}

	print_field("Mode: %s (0x%2.2x)", str, mode);
}

static void print_auth_payload_timeout(uint16_t timeout)
{
	print_field("Timeout: %d msec (0x%4.4x)",
			le16_to_cpu(timeout) * 10, le16_to_cpu(timeout));
}

static void print_pscan_rep_mode(uint8_t pscan_rep_mode)
{
	const char *str;

	switch (pscan_rep_mode) {
	case 0x00:
		str = "R0";
		break;
	case 0x01:
		str = "R1";
		break;
	case 0x02:
		str = "R2";
		break;
	default:
		str = "Reserved";
		break;
	}

	print_field("Page scan repetition mode: %s (0x%2.2x)",
						str, pscan_rep_mode);
}

static void print_pscan_period_mode(uint8_t pscan_period_mode)
{
	const char *str;

	switch (pscan_period_mode) {
	case 0x00:
		str = "P0";
		break;
	case 0x01:
		str = "P1";
		break;
	case 0x02:
		str = "P2";
		break;
	default:
		str = "Reserved";
		break;
	}

	print_field("Page period mode: %s (0x%2.2x)", str, pscan_period_mode);
}

static void print_pscan_mode(uint8_t pscan_mode)
{
	const char *str;

	switch (pscan_mode) {
	case 0x00:
		str = "Mandatory";
		break;
	case 0x01:
		str = "Optional I";
		break;
	case 0x02:
		str = "Optional II";
		break;
	case 0x03:
		str = "Optional III";
		break;
	default:
		str = "Reserved";
		break;
	}

	print_field("Page scan mode: %s (0x%2.2x)", str, pscan_mode);
}

static void print_clock_offset(uint16_t clock_offset)
{
	print_field("Clock offset: 0x%4.4x", le16_to_cpu(clock_offset));
}

static void print_clock(uint32_t clock)
{
	print_field("Clock: 0x%8.8x", le32_to_cpu(clock));
}

static void print_clock_type(uint8_t type)
{
	const char *str;

	switch (type) {
	case 0x00:
		str = "Local clock";
		break;
	case 0x01:
		str = "Piconet clock";
		break;
	default:
		str = "Reserved";
		break;
	}

	print_field("Type: %s (0x%2.2x)", str, type);
}

static void print_clock_accuracy(uint16_t accuracy)
{
	if (le16_to_cpu(accuracy) == 0xffff)
		print_field("Accuracy: Unknown (0x%4.4x)",
						le16_to_cpu(accuracy));
	else
		print_field("Accuracy: %.4f msec (0x%4.4x)",
						le16_to_cpu(accuracy) * 0.3125,
						le16_to_cpu(accuracy));
}

static void print_lpo_allowed(uint8_t lpo_allowed)
{
	print_field("LPO allowed: 0x%2.2x", lpo_allowed);
}

static void print_broadcast_fragment(uint8_t fragment)
{
	const char *str;

	switch (fragment) {
	case 0x00:
		str = "Continuation fragment";
		break;
	case 0x01:
		str = "Starting fragment";
		break;
	case 0x02:
		str = "Ending fragment";
		break;
	case 0x03:
		str = "No fragmentation";
		break;
	default:
		str = "Reserved";
		break;
	}

	print_field("Fragment: %s (0x%2.2x)", str, fragment);
}

static void print_link_type(uint8_t link_type)
{
	const char *str;

	switch (link_type) {
	case 0x00:
		str = "SCO";
		break;
	case 0x01:
		str = "ACL";
		break;
	case 0x02:
		str = "eSCO";
		break;
	default:
		str = "Reserved";
		break;
	}

	print_field("Link type: %s (0x%2.2x)", str, link_type);
}

static void print_encr_mode_change(uint8_t encr_mode, uint16_t handle)
{
	const char *str;
	uint8_t conn_type;

	conn_type = get_type(le16_to_cpu(handle));

	switch (encr_mode) {
	case 0x00:
		str = "Disabled";
		break;
	case 0x01:
		switch (conn_type) {
		case 0x00:
			str = "Enabled with E0";
			break;
		case 0x01:
			str = "Enabled with AES-CCM";
			break;
		default:
			str = "Enabled";
			break;
		}
		break;
	case 0x02:
		str = "Enabled with AES-CCM";
		break;
	default:
		str = "Reserved";
		break;
	}

	print_field("Encryption: %s (0x%2.2x)", str, encr_mode);
}

static void print_pin_type(uint8_t pin_type)
{
	const char *str;

	switch (pin_type) {
	case 0x00:
		str = "Variable";
		break;
	case 0x01:
		str = "Fixed";
		break;
	default:
		str = "Reserved";
		break;
	}

	print_field("PIN type: %s (0x%2.2x)", str, pin_type);
}

static void print_key_flag(uint8_t key_flag)
{
	const char *str;

	switch (key_flag) {
	case 0x00:
		str = "Semi-permanent";
		break;
	case 0x01:
		str = "Temporary";
		break;
	default:
		str = "Reserved";
		break;
	}

	print_field("Key flag: %s (0x%2.2x)", str, key_flag);
}

static void print_key_len(uint8_t key_len)
{
	const char *str;

	switch (key_len) {
	case 32:
		str = "802.11 PAL";
		break;
	default:
		str = "Reserved";
		break;
	}

	print_field("Key length: %s (%d)", str, key_len);
}

static void print_key_type(uint8_t key_type)
{
	const char *str;

	switch (key_type) {
	case 0x00:
		str = "Combination key";
		break;
	case 0x01:
		str = "Local Unit key";
		break;
	case 0x02:
		str = "Remote Unit key";
		break;
	case 0x03:
		str = "Debug Combination key";
		break;
	case 0x04:
		str = "Unauthenticated Combination key from P-192";
		break;
	case 0x05:
		str = "Authenticated Combination key from P-192";
		break;
	case 0x06:
		str = "Changed Combination key";
		break;
	case 0x07:
		str = "Unauthenticated Combination key from P-256";
		break;
	case 0x08:
		str = "Authenticated Combination key from P-256";
		break;
	default:
		str = "Reserved";
		break;
	}

	print_field("Key type: %s (0x%2.2x)", str, key_type);
}

static void print_key_size(uint8_t key_size)
{
	print_field("Key size: %d", key_size);
}

static void print_hex_field(const char *label, const uint8_t *data,
								uint8_t len)
{
	char str[len * 2 + 1];
	uint8_t i;

	str[0] = '\0';

	for (i = 0; i < len; i++)
		sprintf(str + (i * 2), "%2.2x", data[i]);

	print_field("%s: %s", label, str);
}

static void print_key(const char *label, const uint8_t *link_key)
{
	print_hex_field(label, link_key, 16);
}

static void print_link_key(const uint8_t *link_key)
{
	print_key("Link key", link_key);
}

static void print_pin_code(const uint8_t *pin_code, uint8_t pin_len)
{
	char str[pin_len + 1];
	uint8_t i;

	for (i = 0; i < pin_len; i++)
		sprintf(str + i, "%c", (const char) pin_code[i]);

	print_field("PIN code: %s", str);
}

static void print_hash_p192(const uint8_t *hash)
{
	print_key("Hash C from P-192", hash);
}

static void print_hash_p256(const uint8_t *hash)
{
	print_key("Hash C from P-256", hash);
}

static void print_randomizer_p192(const uint8_t *randomizer)
{
	print_key("Randomizer R with P-192", randomizer);
}

static void print_randomizer_p256(const uint8_t *randomizer)
{
	print_key("Randomizer R with P-256", randomizer);
}

static void print_pk256(const char *label, const uint8_t *key)
{
	print_field("%s:", label);
	print_hex_field("  X", &key[0], 32);
	print_hex_field("  Y", &key[32], 32);
}

static void print_dhkey(const uint8_t *dhkey)
{
	print_hex_field("Diffie-Hellman key", dhkey, 32);
}

static void print_passkey(uint32_t passkey)
{
	print_field("Passkey: %06d", le32_to_cpu(passkey));
}

static void print_io_capability(uint8_t capability)
{
	const char *str;

	switch (capability) {
	case 0x00:
		str = "DisplayOnly";
		break;
	case 0x01:
		str = "DisplayYesNo";
		break;
	case 0x02:
		str = "KeyboardOnly";
		break;
	case 0x03:
		str = "NoInputNoOutput";
		break;
	default:
		str = "Reserved";
		break;
	}

	print_field("IO capability: %s (0x%2.2x)", str, capability);
}

static void print_oob_data(uint8_t oob_data)
{
	const char *str;

	switch (oob_data) {
	case 0x00:
		str = "Authentication data not present";
		break;
	case 0x01:
		str = "P-192 authentication data present";
		break;
	case 0x02:
		str = "P-256 authentication data present";
		break;
	case 0x03:
		str = "P-192 and P-256 authentication data present";
		break;
	default:
		str = "Reserved";
		break;
	}

	print_field("OOB data: %s (0x%2.2x)", str, oob_data);
}

static void print_oob_data_response(uint8_t oob_data)
{
	const char *str;

	switch (oob_data) {
	case 0x00:
		str = "Authentication data not present";
		break;
	case 0x01:
		str = "Authentication data present";
		break;
	default:
		str = "Reserved";
		break;
	}

	print_field("OOB data: %s (0x%2.2x)", str, oob_data);
}

static void print_authentication(uint8_t authentication)
{
	const char *str;

	switch (authentication) {
	case 0x00:
		str = "No Bonding - MITM not required";
		break;
	case 0x01:
		str = "No Bonding - MITM required";
		break;
	case 0x02:
		str = "Dedicated Bonding - MITM not required";
		break;
	case 0x03:
		str = "Dedicated Bonding - MITM required";
		break;
	case 0x04:
		str = "General Bonding - MITM not required";
		break;
	case 0x05:
		str = "General Bonding - MITM required";
		break;
	default:
		str = "Reserved";
		break;
	}

	print_field("Authentication: %s (0x%2.2x)", str, authentication);
}

void packet_print_io_capability(uint8_t capability)
{
	print_io_capability(capability);
}

void packet_print_io_authentication(uint8_t authentication)
{
	print_authentication(authentication);
}

static void print_location_domain_aware(uint8_t aware)
{
	const char *str;

	switch (aware) {
	case 0x00:
		str = "Regulatory domain unknown";
		break;
	case 0x01:
		str = "Regulatory domain known";
		break;
	default:
		str = "Reserved";
		break;
	}

	print_field("Domain aware: %s (0x%2.2x)", str, aware);
}

static void print_location_domain(const uint8_t *domain)
{
	print_field("Domain: %c%c (0x%2.2x%2.2x)",
		(char) domain[0], (char) domain[1], domain[0], domain[1]);
}

static void print_location_domain_options(uint8_t options)
{
	print_field("Domain options: %c (0x%2.2x)", (char) options, options);
}

static void print_location_options(uint8_t options)
{
	print_field("Options: 0x%2.2x", options);
}

static void print_flow_control_mode(uint8_t mode)
{
	const char *str;

	switch (mode) {
	case 0x00:
		str = "Packet based";
		break;
	case 0x01:
		str = "Data block based";
		break;
	default:
		str = "Reserved";
		break;
	}

	print_field("Flow control mode: %s (0x%2.2x)", str, mode);
}

static void print_flow_direction(uint8_t direction)
{
	const char *str;

	switch (direction) {
	case 0x00:
		str = "Outgoing";
		break;
	case 0x01:
		str = "Incoming";
		break;
	default:
		str = "Reserved";
		break;
	}

	print_field("Flow direction: %s (0x%2.2x)", str, direction);
}

static void print_service_type(uint8_t service_type)
{
	const char *str;

	switch (service_type) {
	case 0x00:
		str = "No Traffic";
		break;
	case 0x01:
		str = "Best Effort";
		break;
	case 0x02:
		str = "Guaranteed";
		break;
	default:
		str = "Reserved";
		break;
	}

	print_field("Service type: %s (0x%2.2x)", str, service_type);
}

static void print_flow_spec(const char *label, const uint8_t *data)
{
	const char *str;

	switch (data[1]) {
	case 0x00:
		str = "No traffic";
		break;
	case 0x01:
		str = "Best effort";
		break;
	case 0x02:
		str = "Guaranteed";
		break;
	default:
		str = "Reserved";
		break;
	}

	print_field("%s flow spec: 0x%2.2x", label, data[0]);
	print_field("  Service type: %s (0x%2.2x)", str, data[1]);
	print_field("  Maximum SDU size: 0x%4.4x", get_le16(data + 2));
	print_field("  SDU inter-arrival time: 0x%8.8x", get_le32(data + 4));
	print_field("  Access latency: 0x%8.8x", get_le32(data + 8));
	print_field("  Flush timeout: 0x%8.8x", get_le32(data + 12));
}

static void print_amp_status(uint8_t amp_status)
{
	const char *str;

	switch (amp_status) {
	case 0x00:
		str = "Present";
		break;
	case 0x01:
		str = "Bluetooth only";
		break;
	case 0x02:
		str = "No capacity";
		break;
	case 0x03:
		str = "Low capacity";
		break;
	case 0x04:
		str = "Medium capacity";
		break;
	case 0x05:
		str = "High capacity";
		break;
	case 0x06:
		str = "Full capacity";
		break;
	default:
		str = "Reserved";
		break;
	}

	print_field("AMP status: %s (0x%2.2x)", str, amp_status);
}

static void print_num_resp(uint8_t num_resp)
{
	print_field("Num responses: %d", num_resp);
}

static void print_num_reports(uint8_t num_reports)
{
	print_field("Num reports: %d", num_reports);
}

static void print_adv_event_type(const char *label, uint8_t type)
{
	const char *str;

	switch (type) {
	case 0x00:
		str = "Connectable undirected - ADV_IND";
		break;
	case 0x01:
		str = "Connectable directed - ADV_DIRECT_IND";
		break;
	case 0x02:
		str = "Scannable undirected - ADV_SCAN_IND";
		break;
	case 0x03:
		str = "Non connectable undirected - ADV_NONCONN_IND";
		break;
	case 0x04:
		str = "Scan response - SCAN_RSP";
		break;
	default:
		str = "Reserved";
		break;
	}

	print_field("%s: %s (0x%2.2x)", label, str, type);
}

static void print_adv_channel_map(const char *label, uint8_t value)
{
	const char *str;

	switch (value) {
	case 0x01:
		str = "37";
		break;
	case 0x02:
		str = "38";
		break;
	case 0x03:
		str = "37, 38";
		break;
	case 0x04:
		str = "39";
		break;
	case 0x05:
		str = "37, 39";
		break;
	case 0x06:
		str = "38, 39";
		break;
	case 0x07:
		str = "37, 38, 39";
		break;
	default:
		str = "Reserved";
		break;
	}

	print_field("%s: %s (0x%2.2x)", label, str, value);
}

static void print_adv_filter_policy(const char *label, uint8_t value)
{
	const char *str;

	switch (value) {
	case 0x00:
		str = "Allow Scan Request from Any, "
			"Allow Connect Request from Any";
		break;
	case 0x01:
		str = "Allow Scan Request from White List Only, "
			"Allow Connect Request from Any";
		break;
	case 0x02:
		str = "Allow Scan Request from Any, "
			"Allow Connect Request from White List Only";
		break;
	case 0x03:
		str = "Allow Scan Request from White List Only, "
			"Allow Connect Request from White List Only";
		break;
	default:
		str = "Reserved";
		break;
	}

	print_field("%s: %s (0x%2.2x)", label, str, value);
}

static void print_rssi(int8_t rssi)
{
	if ((uint8_t) rssi == 0x99 || rssi == 127)
		print_field("RSSI: invalid (0x%2.2x)", (uint8_t) rssi);
	else
		print_field("RSSI: %d dBm (0x%2.2x)", rssi, (uint8_t) rssi);
}

static void print_slot_625(const char *label, uint16_t value)
{
	 print_field("%s: %.3f msec (0x%4.4x)", label,
				le16_to_cpu(value) * 0.625, le16_to_cpu(value));
}

static void print_slot_125(const char *label, uint16_t value)
{
	print_field("%s: %.2f msec (0x%4.4x)", label,
				le16_to_cpu(value) * 1.25, le16_to_cpu(value));
}

static void print_timeout(uint16_t timeout)
{
	print_slot_625("Timeout", timeout);
}

static void print_interval(uint16_t interval)
{
	print_slot_625("Interval", interval);
}

static void print_window(uint16_t window)
{
	print_slot_625("Window", window);
}

static void print_conn_latency(const char *label, uint16_t value)
{
	print_field("%s: %u (0x%4.4x)", label, le16_to_cpu(value),
							le16_to_cpu(value));
}

static void print_role(uint8_t role)
{
	const char *str;

	switch (role) {
	case 0x00:
		str = "Master";
		break;
	case 0x01:
		str = "Slave";
		break;
	default:
		str = "Reserved";
		break;
	}

	print_field("Role: %s (0x%2.2x)", str, role);
}

static void print_mode(uint8_t mode)
{
	const char *str;

	switch (mode) {
	case 0x00:
		str = "Active";
		break;
	case 0x01:
		str = "Hold";
		break;
	case 0x02:
		str = "Sniff";
		break;
	case 0x03:
		str = "Park";
		break;
	default:
		str = "Reserved";
		break;
	}

	print_field("Mode: %s (0x%2.2x)", str, mode);
}

static void print_name(const uint8_t *name)
{
	char str[249];

	memcpy(str, name, 248);
	str[248] = '\0';

	print_field("Name: %s", str);
}

static void print_channel_map(const uint8_t *map)
{
	unsigned int count = 0, start = 0;
	char str[21];
	int i, n;

	for (i = 0; i < 10; i++)
		sprintf(str + (i * 2), "%2.2x", map[i]);

	print_field("Channel map: 0x%s", str);

	for (i = 0; i < 10; i++) {
		for (n = 0; n < 8; n++) {
			if (map[i] & (1 << n)) {
				if (count == 0)
					start = (i * 8) + n;
				count++;
				continue;
			}

			if (count > 1) {
				print_field("  Channel %u-%u",
						start, start + count - 1);
				count = 0;
			} else if (count > 0) {
				print_field("  Channel %u", start);
				count = 0;
			}
		}
	}
}

void packet_print_channel_map_lmp(const uint8_t *map)
{
	print_channel_map(map);
}

static void print_flush_timeout(uint16_t timeout)
{
	if (timeout)
		print_timeout(timeout);
	else
		print_field("Timeout: No Automatic Flush");
}

void packet_print_version(const char *label, uint8_t version,
				const char *sublabel, uint16_t subversion)
{
	const char *str;

	switch (version) {
	case 0x00:
		str = "Bluetooth 1.0b";
		break;
	case 0x01:
		str = "Bluetooth 1.1";
		break;
	case 0x02:
		str = "Bluetooth 1.2";
		break;
	case 0x03:
		str = "Bluetooth 2.0";
		break;
	case 0x04:
		str = "Bluetooth 2.1";
		break;
	case 0x05:
		str = "Bluetooth 3.0";
		break;
	case 0x06:
		str = "Bluetooth 4.0";
		break;
	case 0x07:
		str = "Bluetooth 4.1";
		break;
	case 0x08:
		str = "Bluetooth 4.2";
		break;
	case 0x09:
		str = "Bluetooth 5.0";
		break;
	case 0x0a:
		str = "Bluetooth 5.1";
		break;
	default:
		str = "Reserved";
		break;
	}

	if (sublabel)
		print_field("%s: %s (0x%2.2x) - %s %d (0x%4.4x)",
					label, str, version,
					sublabel, subversion, subversion);
	else
		print_field("%s: %s (0x%2.2x)", label, str, version);
}

static void print_hci_version(uint8_t version, uint16_t revision)
{
	packet_print_version("HCI version", version,
				"Revision", le16_to_cpu(revision));
}

static void print_lmp_version(uint8_t version, uint16_t subversion)
{
	packet_print_version("LMP version", version,
				"Subversion", le16_to_cpu(subversion));
}

static void print_pal_version(uint8_t version, uint16_t subversion)
{
	const char *str;

	switch (version) {
	case 0x01:
		str = "Bluetooth 3.0";
		break;
	default:
		str = "Reserved";
		break;
	}

	print_field("PAL version: %s (0x%2.2x) - Subversion %d (0x%4.4x)",
						str, version,
						le16_to_cpu(subversion),
						le16_to_cpu(subversion));
}

void packet_print_company(const char *label, uint16_t company)
{
	print_field("%s: %s (%d)", label, bt_compidtostr(company), company);
}

static void print_manufacturer(uint16_t manufacturer)
{
	packet_print_company("Manufacturer", le16_to_cpu(manufacturer));
}

static const struct {
	uint16_t ver;
	const char *str;
} broadcom_uart_subversion_table[] = {
	{ 0x210b, "BCM43142A0"	},	/* 001.001.011 */
	{ 0x410e, "BCM43341B0"	},	/* 002.001.014 */
	{ 0x4406, "BCM4324B3"	},	/* 002.004.006 */
	{ }
};

static const struct {
	uint16_t ver;
	const char *str;
} broadcom_usb_subversion_table[] = {
	{ 0x210b, "BCM43142A0"	},	/* 001.001.011 */
	{ 0x2112, "BCM4314A0"	},	/* 001.001.018 */
	{ 0x2118, "BCM20702A0"	},	/* 001.001.024 */
	{ 0x2126, "BCM4335A0"	},	/* 001.001.038 */
	{ 0x220e, "BCM20702A1"	},	/* 001.002.014 */
	{ 0x230f, "BCM4354A2"	},	/* 001.003.015 */
	{ 0x4106, "BCM4335B0"	},	/* 002.001.006 */
	{ 0x410e, "BCM20702B0"	},	/* 002.001.014 */
	{ 0x6109, "BCM4335C0"	},	/* 003.001.009 */
	{ 0x610c, "BCM4354"	},	/* 003.001.012 */
	{ }
};

static void print_manufacturer_broadcom(uint16_t subversion, uint16_t revision)
{
	uint16_t ver = le16_to_cpu(subversion);
	uint16_t rev = le16_to_cpu(revision);
	const char *str = NULL;
	int i;

	switch ((rev & 0xf000) >> 12) {
	case 0:
	case 3:
		for (i = 0; broadcom_uart_subversion_table[i].str; i++) {
			if (broadcom_uart_subversion_table[i].ver == ver) {
				str = broadcom_uart_subversion_table[i].str;
				break;
			}
		}
		break;
	case 1:
	case 2:
		for (i = 0; broadcom_usb_subversion_table[i].str; i++) {
			if (broadcom_usb_subversion_table[i].ver == ver) {
				str = broadcom_usb_subversion_table[i].str;
				break;
			}
		}
		break;
	}

	if (str)
		print_field("  Firmware: %3.3u.%3.3u.%3.3u (%s)",
				(ver & 0xe000) >> 13,
				(ver & 0x1f00) >> 8, ver & 0x00ff, str);
	else
		print_field("  Firmware: %3.3u.%3.3u.%3.3u",
				(ver & 0xe000) >> 13,
				(ver & 0x1f00) >> 8, ver & 0x00ff);

	if (rev != 0xffff)
		print_field("  Build: %4.4u", rev & 0x0fff);
}

static const char *get_supported_command(int bit);

static void print_commands(const uint8_t *commands)
{
	unsigned int count = 0;
	int i, n;

	for (i = 0; i < 64; i++) {
		for (n = 0; n < 8; n++) {
			if (commands[i] & (1 << n))
				count++;
		}
	}

	print_field("Commands: %u entr%s", count, count == 1 ? "y" : "ies");

	for (i = 0; i < 64; i++) {
		for (n = 0; n < 8; n++) {
			const char *cmd;

			if (!(commands[i] & (1 << n)))
				continue;

			cmd = get_supported_command((i * 8) + n);
			if (cmd)
				print_field("  %s (Octet %d - Bit %d)",
								cmd, i, n);
			else
				print_text(COLOR_UNKNOWN_COMMAND_BIT,
						"  Octet %d - Bit %d ", i, n);
		}
	}
}

static const struct bitfield_data features_page0[] = {
	{  0, "3 slot packets"				},
	{  1, "5 slot packets"				},
	{  2, "Encryption"				},
	{  3, "Slot offset"				},
	{  4, "Timing accuracy"				},
	{  5, "Role switch"				},
	{  6, "Hold mode"				},
	{  7, "Sniff mode"				},
	{  8, "Park state"				},
	{  9, "Power control requests"			},
	{ 10, "Channel quality driven data rate (CQDDR)"},
	{ 11, "SCO link"				},
	{ 12, "HV2 packets"				},
	{ 13, "HV3 packets"				},
	{ 14, "u-law log synchronous data"		},
	{ 15, "A-law log synchronous data"		},
	{ 16, "CVSD synchronous data"			},
	{ 17, "Paging parameter negotiation"		},
	{ 18, "Power control"				},
	{ 19, "Transparent synchronous data"		},
	{ 20, "Flow control lag (least significant bit)"},
	{ 21, "Flow control lag (middle bit)"		},
	{ 22, "Flow control lag (most significant bit)"	},
	{ 23, "Broadcast Encryption"			},
	{ 25, "Enhanced Data Rate ACL 2 Mbps mode"	},
	{ 26, "Enhanced Data Rate ACL 3 Mbps mode"	},
	{ 27, "Enhanced inquiry scan"			},
	{ 28, "Interlaced inquiry scan"			},
	{ 29, "Interlaced page scan"			},
	{ 30, "RSSI with inquiry results"		},
	{ 31, "Extended SCO link (EV3 packets)"		},
	{ 32, "EV4 packets"				},
	{ 33, "EV5 packets"				},
	{ 35, "AFH capable slave"			},
	{ 36, "AFH classification slave"		},
	{ 37, "BR/EDR Not Supported"			},
	{ 38, "LE Supported (Controller)"		},
	{ 39, "3-slot Enhanced Data Rate ACL packets"	},
	{ 40, "5-slot Enhanced Data Rate ACL packets"	},
	{ 41, "Sniff subrating"				},
	{ 42, "Pause encryption"			},
	{ 43, "AFH capable master"			},
	{ 44, "AFH classification master"		},
	{ 45, "Enhanced Data Rate eSCO 2 Mbps mode"	},
	{ 46, "Enhanced Data Rate eSCO 3 Mbps mode"	},
	{ 47, "3-slot Enhanced Data Rate eSCO packets"	},
	{ 48, "Extended Inquiry Response"		},
	{ 49, "Simultaneous LE and BR/EDR (Controller)"	},
	{ 51, "Secure Simple Pairing"			},
	{ 52, "Encapsulated PDU"			},
	{ 53, "Erroneous Data Reporting"		},
	{ 54, "Non-flushable Packet Boundary Flag"	},
	{ 56, "Link Supervision Timeout Changed Event"	},
	{ 57, "Inquiry TX Power Level"			},
	{ 58, "Enhanced Power Control"			},
	{ 63, "Extended features"			},
	{ }
};

static const struct bitfield_data features_page1[] = {
	{  0, "Secure Simple Pairing (Host Support)"	},
	{  1, "LE Supported (Host)"			},
	{  2, "Simultaneous LE and BR/EDR (Host)"	},
	{  3, "Secure Connections (Host Support)"	},
	{ }
};

static const struct bitfield_data features_page2[] = {
	{  0, "Connectionless Slave Broadcast - Master"	},
	{  1, "Connectionless Slave Broadcast - Slave"	},
	{  2, "Synchronization Train"			},
	{  3, "Synchronization Scan"			},
	{  4, "Inquiry Response Notification Event"	},
	{  5, "Generalized interlaced scan"		},
	{  6, "Coarse Clock Adjustment"			},
	{  8, "Secure Connections (Controller Support)"	},
	{  9, "Ping"					},
	{ 10, "Slot Availability Mask"			},
	{ 11, "Train nudging"				},
	{ }
};

static const struct bitfield_data features_le[] = {
	{  0, "LE Encryption"					},
	{  1, "Connection Parameter Request Procedure"		},
	{  2, "Extended Reject Indication"			},
	{  3, "Slave-initiated Features Exchange"		},
	{  4, "LE Ping"						},
	{  5, "LE Data Packet Length Extension"			},
	{  6, "LL Privacy"					},
	{  7, "Extended Scanner Filter Policies"		},
	{  8, "LE 2M PHY"					},
	{  9, "Stable Modulation Index - Transmitter"		},
	{ 10, "Stable Modulation Index - Receiver"		},
	{ 11, "LE Coded PHY"					},
	{ 12, "LE Extended Advertising"				},
	{ 13, "LE Periodic Advertising"				},
	{ 14, "Channel Selection Algorithm #2"			},
	{ 15, "LE Power Class 1"				},
	{ 16, "Minimum Number of Used Channels Procedure"	},
	{ 17, "Connection CTE Request"				},
	{ 18, "Connection CTE Response"				},
	{ 19, "Connectionless CTE Transmitter"			},
	{ 20, "Connectionless CTE Receiver"			},
	{ 21, "Antenna Switching During CTE Transmission (AoD)"	},
	{ 22, "Antenna Switching During CTE Reception (AoA)"	},
	{ 23, "Receiving Constant Tone Extensions"		},
	{ 24, "Periodic Advertising Sync Transfer - Sender"	},
	{ 25, "Periodic Advertising Sync Transfer - Recipient"	},
	{ 26, "Sleep Clock Accuracy Updates"			},
	{ 27, "Remote Public Key Validation"			},
	{ 28, "Connected Isochronous Stream - Master"		},
	{ 29, "Connected Isochronous Stream - Slave"		},
	{ 30, "Isochronous Broadcaster"				},
	{ 31, "Synchronized Receiver"				},
	{ 32, "Isochronous Channels (Host Support)"		},
	{ }
};

static void print_features(uint8_t page, const uint8_t *features_array,
								uint8_t type)
{
	const struct bitfield_data *features_table = NULL;
	uint64_t mask, features = 0;
	char str[41];
	int i;

	for (i = 0; i < 8; i++) {
		sprintf(str + (i * 5), " 0x%2.2x", features_array[i]);
		features |= ((uint64_t) features_array[i]) << (i * 8);
	}

	print_field("Features:%s", str);

	switch (type) {
	case 0x00:
		switch (page) {
		case 0:
			features_table = features_page0;
			break;
		case 1:
			features_table = features_page1;
			break;
		case 2:
			features_table = features_page2;
			break;
		}
		break;
	case 0x01:
		switch (page) {
		case 0:
			features_table = features_le;
			break;
		}
		break;
	}

	if (!features_table)
		return;

	mask = print_bitfield(2, features, features_table);
	if (mask)
		print_text(COLOR_UNKNOWN_FEATURE_BIT, "  Unknown features "
						"(0x%16.16" PRIx64 ")", mask);
}

void packet_print_features_lmp(const uint8_t *features, uint8_t page)
{
	print_features(page, features, 0x00);
}

void packet_print_features_ll(const uint8_t *features)
{
	print_features(0, features, 0x01);
}

#define LE_STATE_SCAN_ADV		0x0001
#define LE_STATE_CONN_ADV		0x0002
#define LE_STATE_NONCONN_ADV		0x0004
#define LE_STATE_HIGH_DIRECT_ADV	0x0008
#define LE_STATE_LOW_DIRECT_ADV		0x0010
#define LE_STATE_ACTIVE_SCAN		0x0020
#define LE_STATE_PASSIVE_SCAN		0x0040
#define LE_STATE_INITIATING		0x0080
#define LE_STATE_CONN_MASTER		0x0100
#define LE_STATE_CONN_SLAVE		0x0200
#define LE_STATE_MASTER_MASTER		0x0400
#define LE_STATE_SLAVE_SLAVE		0x0800
#define LE_STATE_MASTER_SLAVE		0x1000

static const struct bitfield_data le_states_desc_table[] = {
	{  0, "Scannable Advertising State"			},
	{  1, "Connectable Advertising State"			},
	{  2, "Non-connectable Advertising State"		},
	{  3, "High Duty Cycle Directed Advertising State"	},
	{  4, "Low Duty Cycle Directed Advertising State"	},
	{  5, "Active Scanning State"				},
	{  6, "Passive Scanning State"				},
	{  7, "Initiating State"				},
	{  8, "Connection State (Master Role)"			},
	{  9, "Connection State (Slave Role)"			},
	{ 10, "Master Role & Master Role"			},
	{ 11, "Slave Role & Slave Role"				},
	{ 12, "Master Role & Slave Role"			},
	{ }
};

static const struct {
	uint8_t bit;
	uint16_t states;
} le_states_comb_table[] = {
	{  0, LE_STATE_NONCONN_ADV				},
	{  1, LE_STATE_SCAN_ADV					},
	{  2, LE_STATE_CONN_ADV					},
	{  3, LE_STATE_HIGH_DIRECT_ADV				},
	{  4, LE_STATE_PASSIVE_SCAN				},
	{  5, LE_STATE_ACTIVE_SCAN				},
	{  6, LE_STATE_INITIATING | LE_STATE_CONN_MASTER	},
	{  7, LE_STATE_CONN_SLAVE				},
	{  8, LE_STATE_PASSIVE_SCAN | LE_STATE_NONCONN_ADV	},
	{  9, LE_STATE_PASSIVE_SCAN | LE_STATE_SCAN_ADV		},
	{ 10, LE_STATE_PASSIVE_SCAN | LE_STATE_CONN_ADV		},
	{ 11, LE_STATE_PASSIVE_SCAN | LE_STATE_HIGH_DIRECT_ADV	},
	{ 12, LE_STATE_ACTIVE_SCAN | LE_STATE_NONCONN_ADV	},
	{ 13, LE_STATE_ACTIVE_SCAN | LE_STATE_SCAN_ADV		},
	{ 14, LE_STATE_ACTIVE_SCAN | LE_STATE_CONN_ADV		},
	{ 15, LE_STATE_ACTIVE_SCAN | LE_STATE_HIGH_DIRECT_ADV	},
	{ 16, LE_STATE_INITIATING | LE_STATE_NONCONN_ADV	},
	{ 17, LE_STATE_INITIATING | LE_STATE_SCAN_ADV		},
	{ 18, LE_STATE_CONN_MASTER | LE_STATE_NONCONN_ADV	},
	{ 19, LE_STATE_CONN_MASTER | LE_STATE_SCAN_ADV		},
	{ 20, LE_STATE_CONN_SLAVE | LE_STATE_NONCONN_ADV	},
	{ 21, LE_STATE_CONN_SLAVE | LE_STATE_SCAN_ADV		},
	{ 22, LE_STATE_INITIATING | LE_STATE_PASSIVE_SCAN	},
	{ 23, LE_STATE_INITIATING | LE_STATE_ACTIVE_SCAN	},
	{ 24, LE_STATE_CONN_MASTER | LE_STATE_PASSIVE_SCAN	},
	{ 25, LE_STATE_CONN_MASTER | LE_STATE_ACTIVE_SCAN	},
	{ 26, LE_STATE_CONN_SLAVE | LE_STATE_PASSIVE_SCAN	},
	{ 27, LE_STATE_CONN_SLAVE | LE_STATE_ACTIVE_SCAN	},
	{ 28, LE_STATE_INITIATING | LE_STATE_CONN_MASTER |
					LE_STATE_MASTER_MASTER	},
	{ 29, LE_STATE_LOW_DIRECT_ADV				},
	{ 30, LE_STATE_LOW_DIRECT_ADV | LE_STATE_PASSIVE_SCAN	},
	{ 31, LE_STATE_LOW_DIRECT_ADV | LE_STATE_ACTIVE_SCAN	},
	{ 32, LE_STATE_INITIATING | LE_STATE_CONN_ADV |
					LE_STATE_MASTER_SLAVE	},
	{ 33, LE_STATE_INITIATING | LE_STATE_HIGH_DIRECT_ADV |
					LE_STATE_MASTER_SLAVE	},
	{ 34, LE_STATE_INITIATING | LE_STATE_LOW_DIRECT_ADV |
					LE_STATE_MASTER_SLAVE	},
	{ 35, LE_STATE_CONN_MASTER | LE_STATE_CONN_ADV |
					LE_STATE_MASTER_SLAVE	},
	{ 36, LE_STATE_CONN_MASTER | LE_STATE_HIGH_DIRECT_ADV |
					LE_STATE_MASTER_SLAVE	},
	{ 37, LE_STATE_CONN_MASTER | LE_STATE_LOW_DIRECT_ADV |
					LE_STATE_MASTER_SLAVE	},
	{ 38, LE_STATE_CONN_SLAVE | LE_STATE_CONN_ADV |
					LE_STATE_MASTER_SLAVE	},
	{ 39, LE_STATE_CONN_SLAVE | LE_STATE_HIGH_DIRECT_ADV |
					LE_STATE_SLAVE_SLAVE	},
	{ 40, LE_STATE_CONN_SLAVE | LE_STATE_LOW_DIRECT_ADV |
					LE_STATE_SLAVE_SLAVE	},
	{ 41, LE_STATE_INITIATING | LE_STATE_CONN_SLAVE |
					LE_STATE_MASTER_SLAVE	},
	{ }
};

static void print_le_states(const uint8_t *states_array)
{
	uint64_t mask, states = 0;
	int i, n;

	for (i = 0; i < 8; i++)
		states |= ((uint64_t) states_array[i]) << (i * 8);

	print_field("States: 0x%16.16" PRIx64, states);

	mask = states;

	for (i = 0; le_states_comb_table[i].states; i++) {
		uint64_t val = (((uint64_t) 1) << le_states_comb_table[i].bit);
		const char *str[3] = { NULL, };
		int num = 0;

		if (!(states & val))
			continue;

		for (n = 0; n < 16; n++) {
			if (le_states_comb_table[i].states & (1 << n))
				str[num++] = le_states_desc_table[n].str;
		}

		if (num > 0) {
			print_field("  %s", str[0]);
			for (n = 1; n < num; n++)
				print_field("    and %s", str[n]);
		}

		mask &= ~val;
	}

	if (mask)
		print_text(COLOR_UNKNOWN_LE_STATES, "  Unknown states "
						"(0x%16.16" PRIx64 ")", mask);
}

static void print_le_channel_map(const uint8_t *map)
{
	unsigned int count = 0, start = 0;
	char str[11];
	int i, n;

	for (i = 0; i < 5; i++)
		sprintf(str + (i * 2), "%2.2x", map[i]);

	print_field("Channel map: 0x%s", str);

	for (i = 0; i < 5; i++) {
		for (n = 0; n < 8; n++) {
			if (map[i] & (1 << n)) {
				if (count == 0)
					start = (i * 8) + n;
				count++;
				continue;
			}

			if (count > 1) {
				print_field("  Channel %u-%u",
						start, start + count - 1);
				count = 0;
			} else if (count > 0) {
				print_field("  Channel %u", start);
				count = 0;
			}
		}
	}
}

void packet_print_channel_map_ll(const uint8_t *map)
{
	print_le_channel_map(map);
}

static void print_random_number(uint64_t rand)
{
	print_field("Random number: 0x%16.16" PRIx64, le64_to_cpu(rand));
}

static void print_encrypted_diversifier(uint16_t ediv)
{
	print_field("Encrypted diversifier: 0x%4.4x", le16_to_cpu(ediv));
}

static const struct bitfield_data events_table[] = {
	{  0, "Inquiry Complete"					},
	{  1, "Inquiry Result"						},
	{  2, "Connection Complete"					},
	{  3, "Connection Request"					},
	{  4, "Disconnection Complete"					},
	{  5, "Authentication Complete"					},
	{  6, "Remote Name Request Complete"				},
	{  7, "Encryption Change"					},
	{  8, "Change Connection Link Key Complete"			},
	{  9, "Master Link Key Complete"				},
	{ 10, "Read Remote Supported Features Complete"			},
	{ 11, "Read Remote Version Information Complete"		},
	{ 12, "QoS Setup Complete"					},
	{ 13, "Command Complete"					},
	{ 14, "Command Status"						},
	{ 15, "Hardware Error"						},
	{ 16, "Flush Occurred"						},
	{ 17, "Role Change"						},
	{ 18, "Number of Completed Packets"				},
	{ 19, "Mode Change"						},
	{ 20, "Return Link Keys"					},
	{ 21, "PIN Code Request"					},
	{ 22, "Link Key Request"					},
	{ 23, "Link Key Notification"					},
	{ 24, "Loopback Command"					},
	{ 25, "Data Buffer Overflow"					},
	{ 26, "Max Slots Change"					},
	{ 27, "Read Clock Offset Complete"				},
	{ 28, "Connection Packet Type Changed"				},
	{ 29, "QoS Violation"						},
	{ 30, "Page Scan Mode Change"					},
	{ 31, "Page Scan Repetition Mode Change"			},
	{ 32, "Flow Specification Complete"				},
	{ 33, "Inquiry Result with RSSI"				},
	{ 34, "Read Remote Extended Features Complete"			},
	{ 43, "Synchronous Connection Complete"				},
	{ 44, "Synchronous Connection Changed"				},
	{ 45, "Sniff Subrating"						},
	{ 46, "Extended Inquiry Result"					},
	{ 47, "Encryption Key Refresh Complete"				},
	{ 48, "IO Capability Request"					},
	{ 49, "IO Capability Request Reply"				},
	{ 50, "User Confirmation Request"				},
	{ 51, "User Passkey Request"					},
	{ 52, "Remote OOB Data Request"					},
	{ 53, "Simple Pairing Complete"					},
	{ 55, "Link Supervision Timeout Changed"			},
	{ 56, "Enhanced Flush Complete"					},
	{ 58, "User Passkey Notification"				},
	{ 59, "Keypress Notification"					},
	{ 60, "Remote Host Supported Features Notification"		},
	{ 61, "LE Meta"							},
	{ }
};

static void print_event_mask(const uint8_t *events_array,
					const struct bitfield_data *table)
{
	uint64_t mask, events = 0;
	int i;

	for (i = 0; i < 8; i++)
		events |= ((uint64_t) events_array[i]) << (i * 8);

	print_field("Mask: 0x%16.16" PRIx64, events);

	mask = print_bitfield(2, events, table);
	if (mask)
		print_text(COLOR_UNKNOWN_EVENT_MASK, "  Unknown mask "
						"(0x%16.16" PRIx64 ")", mask);
}

static const struct bitfield_data events_page2_table[] = {
	{  0, "Physical Link Complete"					},
	{  1, "Channel Selected"					},
	{  2, "Disconnection Physical Link Complete"			},
	{  3, "Physical Link Loss Early Warning"			},
	{  4, "Physical Link Recovery"					},
	{  5, "Logical Link Complete"					},
	{  6, "Disconnection Logical Link Complete"			},
	{  7, "Flow Specification Modify Complete"			},
	{  8, "Number of Completed Data Blocks"				},
	{  9, "AMP Start Test"						},
	{ 10, "AMP Test End"						},
	{ 11, "AMP Receiver Report"					},
	{ 12, "Short Range Mode Change Complete"			},
	{ 13, "AMP Status Change"					},
	{ 14, "Triggered Clock Capture"					},
	{ 15, "Synchronization Train Complete"				},
	{ 16, "Synchronization Train Received"				},
	{ 17, "Connectionless Slave Broadcast Receive"			},
	{ 18, "Connectionless Slave Broadcast Timeout"			},
	{ 19, "Truncated Page Complete"					},
	{ 20, "Slave Page Response Timeout"				},
	{ 21, "Connectionless Slave Broadcast Channel Map Change"	},
	{ 22, "Inquiry Response Notification"				},
	{ 23, "Authenticated Payload Timeout Expired"			},
	{ 24, "SAM Status Change"					},
	{ }
};

static const struct bitfield_data events_le_table[] = {
	{  0, "LE Connection Complete"			},
	{  1, "LE Advertising Report"			},
	{  2, "LE Connection Update Complete"		},
	{  3, "LE Read Remote Used Features Complete"	},
	{  4, "LE Long Term Key Request"		},
	{  5, "LE Remote Connection Parameter Request"	},
	{  6, "LE Data Length Change"			},
	{  7, "LE Read Local P-256 Public Key Complete"	},
	{  8, "LE Generate DHKey Complete"		},
	{  9, "LE Enhanced Connection Complete"		},
	{ 10, "LE Direct Advertising Report"		},
	{ 11, "LE PHY Update Complete"			},
	{ 12, "LE Extended Advertising Report"		},
	{ 13, "LE Periodic Advertising Sync Established"},
	{ 14, "LE Periodic Advertising Report"		},
	{ 15, "LE Periodic Advertising Sync Lost"	},
	{ 16, "LE Extended Scan Timeout"		},
	{ 17, "LE Extended Advertising Set Terminated"	},
	{ 18, "LE Scan Request Received"		},
	{ 19, "LE Channel Selection Algorithm"		},
	{ 24, "LE CIS Established"			},
	{ 25, "LE CIS Request"				},
	{ 26, "LE Create BIG Complete"			},
	{ 27, "LE Terminate BIG Complete"		},
	{ 28, "LE BIG Sync Estabilished Complete"	},
	{ 29, "LE BIG Sync Lost"			},
	{ }
};

static void print_fec(uint8_t fec)
{
	const char *str;

	switch (fec) {
	case 0x00:
		str = "Not required";
		break;
	case 0x01:
		str = "Required";
		break;
	default:
		str = "Reserved";
		break;
	}

	print_field("FEC: %s (0x%02x)", str, fec);
}

#define BT_EIR_FLAGS			0x01
#define BT_EIR_UUID16_SOME		0x02
#define BT_EIR_UUID16_ALL		0x03
#define BT_EIR_UUID32_SOME		0x04
#define BT_EIR_UUID32_ALL		0x05
#define BT_EIR_UUID128_SOME		0x06
#define BT_EIR_UUID128_ALL		0x07
#define BT_EIR_NAME_SHORT		0x08
#define BT_EIR_NAME_COMPLETE		0x09
#define BT_EIR_TX_POWER			0x0a
#define BT_EIR_CLASS_OF_DEV		0x0d
#define BT_EIR_SSP_HASH_P192		0x0e
#define BT_EIR_SSP_RANDOMIZER_P192	0x0f
#define BT_EIR_DEVICE_ID		0x10
#define BT_EIR_SMP_TK			0x10
#define BT_EIR_SMP_OOB_FLAGS		0x11
#define BT_EIR_SLAVE_CONN_INTERVAL	0x12
#define BT_EIR_SERVICE_UUID16		0x14
#define BT_EIR_SERVICE_UUID128		0x15
#define BT_EIR_SERVICE_DATA		0x16
#define BT_EIR_PUBLIC_ADDRESS		0x17
#define BT_EIR_RANDOM_ADDRESS		0x18
#define BT_EIR_GAP_APPEARANCE		0x19
#define BT_EIR_ADVERTISING_INTERVAL	0x1a
#define BT_EIR_LE_DEVICE_ADDRESS	0x1b
#define BT_EIR_LE_ROLE			0x1c
#define BT_EIR_SSP_HASH_P256		0x1d
#define BT_EIR_SSP_RANDOMIZER_P256	0x1e
#define BT_EIR_SERVICE_UUID32		0x1f
#define BT_EIR_SERVICE_DATA32		0x20
#define BT_EIR_SERVICE_DATA128		0x21
#define BT_EIR_LE_SC_CONFIRM_VALUE	0x22
#define BT_EIR_LE_SC_RANDOM_VALUE	0x23
#define BT_EIR_URI			0x24
#define BT_EIR_INDOOR_POSITIONING	0x25
#define BT_EIR_TRANSPORT_DISCOVERY	0x26
#define BT_EIR_LE_SUPPORTED_FEATURES	0x27
#define BT_EIR_CHANNEL_MAP_UPDATE_IND	0x28
#define BT_EIR_MESH_PROV		0x29
#define BT_EIR_MESH_DATA		0x2a
#define BT_EIR_MESH_BEACON		0x2b
#define BT_EIR_3D_INFO_DATA		0x3d
#define BT_EIR_MANUFACTURER_DATA	0xff

static void print_manufacturer_apple(const void *data, uint8_t data_len)
{
	uint8_t type = *((uint8_t *) data);

	if (data_len < 1)
		return;

	if (type == 0x01) {
		char identifier[100];

		snprintf(identifier, sizeof(identifier) - 1, "%s",
						(const char *) (data + 1));

		print_field("  Identifier: %s", identifier);
		return;
	}

	while (data_len > 0) {
		uint8_t len;
		const char *str;

		type = *((uint8_t *) data);
		data++;
		data_len--;

		if (type == 0x00)
			continue;

		if (data_len < 1)
			break;

		switch (type) {
		case 0x02:
			str = "iBeacon";
			break;
		case 0x05:
			str = "AirDrop";
			break;
		case 0x09:
			str = "Apple TV";
			break;
		default:
			str = "Unknown";
			break;
		}

		print_field("  Type: %s (%u)", str, type);

		len = *((uint8_t *) data);
		data++;
		data_len--;

		if (len < 1)
			continue;

		if (len > data_len)
			break;

		if (type == 0x02 && len == 0x15) {
			const uint8_t *uuid;
			uint16_t minor, major;
			int8_t tx_power;

			uuid = data;
			print_field("  UUID: %8.8x-%4.4x-%4.4x-%4.4x-%8.8x%4.4x",
				get_le32(&uuid[12]), get_le16(&uuid[10]),
				get_le16(&uuid[8]), get_le16(&uuid[6]),
				get_le32(&uuid[2]), get_le16(&uuid[0]));

			major = get_le16(data + 16);
			minor = get_le16(data + 18);
			print_field("  Version: %u.%u", major, minor);

			tx_power = *(int8_t *) (data + 20);
			print_field("  TX power: %d dB", tx_power);
		} else
			print_hex_field("  Data", data, len);

		data += len;
		data_len -= len;
	}

	packet_hexdump(data, data_len);
}

static void print_manufacturer_data(const void *data, uint8_t data_len)
{
	uint16_t company = get_le16(data);

	packet_print_company("Company", company);

	switch (company) {
	case 76:
	case 19456:
		print_manufacturer_apple(data + 2, data_len - 2);
		break;
	default:
		print_hex_field("  Data", data + 2, data_len - 2);
		break;
	}
}

static void print_device_id(const void *data, uint8_t data_len)
{
	uint16_t source, vendor, product, version;
	char modalias[26], *vendor_str, *product_str;
	const char *str;

	if (data_len < 8)
		return;

	source = get_le16(data);
	vendor = get_le16(data + 2);
	product = get_le16(data + 4);
	version = get_le16(data + 6);

	switch (source) {
	case 0x0001:
		str = "Bluetooth SIG assigned";
		sprintf(modalias, "bluetooth:v%04Xp%04Xd%04X",
						vendor, product, version);
		break;
	case 0x0002:
		str = "USB Implementer's Forum assigned";
		sprintf(modalias, "usb:v%04Xp%04Xd%04X",
						vendor, product, version);
		break;
	default:
		str = "Reserved";
		modalias[0] = '\0';
		break;
	}

	print_field("Device ID: %s (0x%4.4x)", str, source);

	if (!hwdb_get_vendor_model(modalias, &vendor_str, &product_str)) {
		vendor_str = NULL;
		product_str = NULL;
	}

	if (source != 0x0001) {
		if (vendor_str)
			print_field("  Vendor: %s (0x%4.4x)",
						vendor_str, vendor);
		else
			print_field("  Vendor: 0x%4.4x", vendor);
	} else
		packet_print_company("  Vendor", vendor);

	if (product_str)
		print_field("  Product: %s (0x%4.4x)", product_str, product);
	else
		print_field("  Product: 0x%4.4x", product);

	print_field("  Version: %u.%u.%u (0x%4.4x)",
					(version & 0xff00) >> 8,
					(version & 0x00f0) >> 4,
					(version & 0x000f), version);

	free(vendor_str);
	free(product_str);
}

static void print_uuid16_list(const char *label, const void *data,
							uint8_t data_len)
{
	uint8_t count = data_len / sizeof(uint16_t);
	unsigned int i;

	print_field("%s: %u entr%s", label, count, count == 1 ? "y" : "ies");

	for (i = 0; i < count; i++) {
		uint16_t uuid = get_le16(data + (i * 2));
		print_field("  %s (0x%4.4x)", bt_uuid16_to_str(uuid), uuid);
	}
}

static void print_uuid32_list(const char *label, const void *data,
							uint8_t data_len)
{
	uint8_t count = data_len / sizeof(uint32_t);
	unsigned int i;

	print_field("%s: %u entr%s", label, count, count == 1 ? "y" : "ies");

	for (i = 0; i < count; i++) {
		uint32_t uuid = get_le32(data + (i * 4));
		print_field("  %s (0x%8.8x)", bt_uuid32_to_str(uuid), uuid);
	}
}

static void print_uuid128_list(const char *label, const void *data,
							uint8_t data_len)
{
	uint8_t count = data_len / 16;
	unsigned int i;
	char uuidstr[MAX_LEN_UUID_STR];

	print_field("%s: %u entr%s", label, count, count == 1 ? "y" : "ies");

	for (i = 0; i < count; i++) {
		const uint8_t *uuid = data + (i * 16);

		sprintf(uuidstr, "%8.8x-%4.4x-%4.4x-%4.4x-%8.8x%4.4x",
				get_le32(&uuid[12]), get_le16(&uuid[10]),
				get_le16(&uuid[8]), get_le16(&uuid[6]),
				get_le32(&uuid[2]), get_le16(&uuid[0]));
		print_field("  %s (%s)", bt_uuidstr_to_str(uuidstr), uuidstr);
	}
}

static const struct bitfield_data eir_flags_table[] = {
	{ 0, "LE Limited Discoverable Mode"		},
	{ 1, "LE General Discoverable Mode"		},
	{ 2, "BR/EDR Not Supported"			},
	{ 3, "Simultaneous LE and BR/EDR (Controller)"	},
	{ 4, "Simultaneous LE and BR/EDR (Host)"	},
	{ }
};

static const struct bitfield_data eir_3d_table[] = {
	{ 0, "Association Notification"					},
	{ 1, "Battery Level Reporting"					},
	{ 2, "Send Battery Level Report on Start-up Synchronization"	},
	{ 7, "Factory Test Mode"					},
	{ }
};

static const struct bitfield_data mesh_oob_table[] = {
	{ 0, "Other"							},
	{ 1, "Electronic / URI"						},
	{ 2, "2D machine-readable code"					},
	{ 3, "Bar code"							},
	{ 4, "Near Field Communication (NFC)"				},
	{ 5, "Number"							},
	{ 6, "String"							},
	{ 11, "On box"							},
	{ 12, "Inside box"						},
	{ 13, "On piece of paper"					},
	{ 14, "Inside manual"						},
	{ 15, "On device"						},
	{ }
};

static void print_mesh_beacon(const uint8_t *data, uint8_t len)
{
	uint16_t oob;

	print_hex_field("Mesh Beacon", data, len);

	if (len < 1)
		return;

	switch (data[0]) {
	case 0x00:
		print_field("  Unprovisioned Device Beacon (0x00)");
		if (len < 18) {
			packet_hexdump(data + 1, len - 1);
			break;
		}

		print_hex_field("  Device UUID", data + 1, 16);

		oob = get_be16(data + 17);
		print_field("  OOB Information: 0x%4.4x", oob);

		print_bitfield(4, oob, mesh_oob_table);

		if (len < 23) {
			packet_hexdump(data + 18, len - 18);
			break;
		}

		print_field("  URI Hash: 0x%8.8x", get_be32(data + 19));
		packet_hexdump(data + 23, len - 23);
		break;
	case 0x01:
		print_field("  Secure Network Beacon (0x01)");
		if (len < 22) {
			packet_hexdump(data + 1, len - 1);
			break;
		}

		print_field("  Flags: 0x%2.2x", data[0]);

		if (data[1] & 0x01)
			print_field("    Key Refresh");

		if (data[1] & 0x02)
			print_field("    IV Update");

		print_hex_field("  Network Id", data + 2, 8);
		print_field("  IV Index: 0x%08x", get_be32(data + 10));
		print_hex_field("  Authentication Value", data + 14, 8);
		packet_hexdump(data + 22, len - 22);
		break;
	default:
		print_field("  Invalid Beacon (0x%02x)", data[0]);
		packet_hexdump(data, len);
		break;
	}
}

static void print_mesh_prov(const uint8_t *data, uint8_t len)
{
	print_hex_field("Mesh Provisioning", data, len);

	if (len < 6) {
		packet_hexdump(data, len);
		return;
	}

	print_field("  Link ID: 0x%08x", get_be32(data));
	print_field("  Transaction Number: %u", data[4]);

	data += 5;
	len -= 5;

	switch (data[0] & 0x03) {
	case 0x00:
		print_field("  Transaction Start (0x00)");
		if (len < 5) {
			packet_hexdump(data + 1, len - 1);
			return;
		}
		print_field("  SeqN: %u", data[0] & 0xfc >> 2);
		print_field("  TotalLength: %u", get_be16(data + 1));
		print_field("  FCS: 0x%2.2x", data[3]);
		print_hex_field("  Data", data + 4, len - 4);
		packet_hexdump(data + 5, len - 5);
		break;
	case 0x01:
		print_field("  Transaction Acknowledgment (0x01)");
		packet_hexdump(data + 1, len - 1);
		break;
	case 0x02:
		print_field("  Transaction Continuation (0x02)");
		print_field("  SegmentIndex: %u", data[0] >> 2);
		if (len < 2) {
			packet_hexdump(data + 1, len - 1);
			return;
		}
		print_hex_field("  Data", data + 1, len - 1);
		packet_hexdump(data + 2, len - 2);
		break;
	case 0x03:
		print_field("  Provisioning Bearer Control (0x03)");
		switch (data[0] >> 2) {
		case 0x00:
			print_field("  Link Open (0x00)");
			if (len < 17) {
				packet_hexdump(data + 1, len - 1);
				break;
			}
			print_hex_field("  Device UUID", data, 16);
			break;
		case 0x01:
			print_field("  Link Ack (0x01)");
			break;
		case 0x02:
			print_field("  Link Close (0x02)");
			if (len < 2) {
				packet_hexdump(data + 1, len - 1);
				break;
			}

			switch (data[1]) {
			case 0x00:
				print_field("  Reason: Success (0x00)");
				break;
			case 0x01:
				print_field("  Reason: Timeout (0x01)");
				break;
			case 0x02:
				print_field("  Reason: Fail (0x02)");
				break;
			default:
				print_field("  Reason: Unrecognized (0x%2.2x)",
								data[1]);
			}
			packet_hexdump(data + 2, len - 2);
			break;
		default:
			packet_hexdump(data + 1, len - 1);
			break;
		}
		break;
	default:
		print_field("  Invalid Command (0x%02x)", data[0]);
		packet_hexdump(data, len);
		break;
	}
}

static void print_mesh_data(const uint8_t *data, uint8_t len)
{
	print_hex_field("Mesh Data", data, len);

	if (len < 1)
		return;

	print_field("  IVI: %u", data[0] >> 7);
	print_field("  NID: 0x%2.2x", data[0] & 0x7f);
	packet_hexdump(data + 1, len - 1);
}

static void print_transport_data(const uint8_t *data, uint8_t len)
{
	print_field("Transport Discovery Data");

	if (len < 3)
		return;

	print_field("  Organization: %s (0x%02x)",
			data[0] == 0x01 ? "Bluetooth SIG" : "RFU", data[0]);
	print_field("  Flags: 0x%2.2x", data[1]);
	print_field("    Role: 0x%2.2x", data[1] & 0x03);

	switch (data[1] & 0x03) {
	case 0x00:
		print_field("      Not Specified");
		break;
	case 0x01:
		print_field("      Seeker Only");
		break;
	case 0x02:
		print_field("      Provider Only");
		break;
	case 0x03:
		print_field("      Both Seeker an Provider");
		break;
	}

	print_field("    Transport Data Incomplete: %s (0x%2.2x)",
			data[1] & 0x04 ? "True" : "False", data[1] & 0x04);

	print_field("    Transport State: 0x%2.2x", data[1] & 0x18);

	switch (data[1] & 0x18) {
	case 0x00:
		print_field("      Off");
		break;
	case 0x08:
		print_field("      On");
		break;
	case 0x10:
		print_field("      Temporary Unavailable");
		break;
	case 0x18:
		print_field("      RFU");
		break;
	}

	print_field("  Length: %u", data[2]);
	print_hex_field("  Data", data + 3, len - 3);
}

static void print_eir(const uint8_t *eir, uint8_t eir_len, bool le)
{
	uint16_t len = 0;

	if (eir_len == 0)
		return;

	while (len < eir_len - 1) {
		uint8_t field_len = eir[0];
		const uint8_t *data = &eir[2];
		uint8_t data_len;
		char name[239], label[100];
		uint8_t flags, mask;

		/* Check for the end of EIR */
		if (field_len == 0)
			break;

		len += field_len + 1;

		/* Do not continue EIR Data parsing if got incorrect length */
		if (len > eir_len) {
			len -= field_len + 1;
			break;
		}

		data_len = field_len - 1;

		switch (eir[1]) {
		case BT_EIR_FLAGS:
			flags = *data;

			print_field("Flags: 0x%2.2x", flags);

			mask = print_bitfield(2, flags, eir_flags_table);
			if (mask)
				print_text(COLOR_UNKNOWN_SERVICE_CLASS,
					"  Unknown flags (0x%2.2x)", mask);
			break;

		case BT_EIR_UUID16_SOME:
			if (data_len < sizeof(uint16_t))
				break;
			print_uuid16_list("16-bit Service UUIDs (partial)",
							data, data_len);
			break;

		case BT_EIR_UUID16_ALL:
			if (data_len < sizeof(uint16_t))
				break;
			print_uuid16_list("16-bit Service UUIDs (complete)",
							data, data_len);
			break;

		case BT_EIR_UUID32_SOME:
			if (data_len < sizeof(uint32_t))
				break;
			print_uuid32_list("32-bit Service UUIDs (partial)",
							data, data_len);
			break;

		case BT_EIR_UUID32_ALL:
			if (data_len < sizeof(uint32_t))
				break;
			print_uuid32_list("32-bit Service UUIDs (complete)",
							data, data_len);
			break;

		case BT_EIR_UUID128_SOME:
			if (data_len < 16)
				break;
			print_uuid128_list("128-bit Service UUIDs (partial)",
								data, data_len);
			break;

		case BT_EIR_UUID128_ALL:
			if (data_len < 16)
				break;
			print_uuid128_list("128-bit Service UUIDs (complete)",
								data, data_len);
			break;

		case BT_EIR_NAME_SHORT:
			memset(name, 0, sizeof(name));
			memcpy(name, data, data_len);
			print_field("Name (short): %s", name);
			break;

		case BT_EIR_NAME_COMPLETE:
			memset(name, 0, sizeof(name));
			memcpy(name, data, data_len);
			print_field("Name (complete): %s", name);
			break;

		case BT_EIR_TX_POWER:
			if (data_len < 1)
				break;
			print_field("TX power: %d dBm", (int8_t) *data);
			break;

		case BT_EIR_CLASS_OF_DEV:
			if (data_len < 3)
				break;
			print_dev_class(data);
			break;

		case BT_EIR_SSP_HASH_P192:
			if (data_len < 16)
				break;
			print_hash_p192(data);
			break;

		case BT_EIR_SSP_RANDOMIZER_P192:
			if (data_len < 16)
				break;
			print_randomizer_p192(data);
			break;

		case BT_EIR_DEVICE_ID:
			/* SMP TK has the same value as Device ID */
			if (le)
				print_hex_field("SMP TK", data, data_len);
			else if (data_len >= 8)
				print_device_id(data, data_len);
			break;

		case BT_EIR_SMP_OOB_FLAGS:
			print_field("SMP OOB Flags: 0x%2.2x", *data);
			break;

		case BT_EIR_SLAVE_CONN_INTERVAL:
			if (data_len < 4)
				break;
			print_field("Slave Conn. Interval: 0x%4.4x - 0x%4.4x",
							get_le16(&data[0]),
							get_le16(&data[2]));
			break;

		case BT_EIR_SERVICE_UUID16:
			if (data_len < sizeof(uint16_t))
				break;
			print_uuid16_list("16-bit Service UUIDs",
							data, data_len);
			break;

		case BT_EIR_SERVICE_UUID128:
			if (data_len < 16)
				break;
			print_uuid128_list("128-bit Service UUIDs",
							data, data_len);
			break;

		case BT_EIR_SERVICE_DATA:
			if (data_len < 2)
				break;
			sprintf(label, "Service Data (UUID 0x%4.4x)",
							get_le16(&data[0]));
			print_hex_field(label, &data[2], data_len - 2);
			break;

		case BT_EIR_RANDOM_ADDRESS:
			if (data_len < 6)
				break;
			print_addr("Random Address", data, 0x01);
			break;

		case BT_EIR_PUBLIC_ADDRESS:
			if (data_len < 6)
				break;
			print_addr("Public Address", data, 0x00);
			break;

		case BT_EIR_GAP_APPEARANCE:
			if (data_len < 2)
				break;
			print_appearance(get_le16(data));
			break;

		case BT_EIR_SSP_HASH_P256:
			if (data_len < 16)
				break;
			print_hash_p256(data);
			break;

		case BT_EIR_SSP_RANDOMIZER_P256:
			if (data_len < 16)
				break;
			print_randomizer_p256(data);
			break;

		case BT_EIR_TRANSPORT_DISCOVERY:
			print_transport_data(data, data_len);
			break;

		case BT_EIR_3D_INFO_DATA:
			print_hex_field("3D Information Data", data, data_len);
			if (data_len < 2)
				break;

			flags = *data;

			print_field("  Features: 0x%2.2x", flags);

			mask = print_bitfield(4, flags, eir_3d_table);
			if (mask)
				print_text(COLOR_UNKNOWN_FEATURE_BIT,
					"      Unknown features (0x%2.2x)", mask);

			print_field("  Path Loss Threshold: %d", data[1]);
			break;

		case BT_EIR_MESH_DATA:
			print_mesh_data(data, data_len);
			break;

		case BT_EIR_MESH_PROV:
			print_mesh_prov(data, data_len);
			break;

		case BT_EIR_MESH_BEACON:
			print_mesh_beacon(data, data_len);
			break;

		case BT_EIR_MANUFACTURER_DATA:
			if (data_len < 2)
				break;
			print_manufacturer_data(data, data_len);
			break;

		default:
			sprintf(label, "Unknown EIR field 0x%2.2x", eir[1]);
			print_hex_field(label, data, data_len);
			break;
		}

		eir += field_len + 1;
	}

	if (len < eir_len && eir[0] != 0)
		packet_hexdump(eir, eir_len - len);
}

void packet_print_addr(const char *label, const void *data, bool random)
{
	print_addr(label ? : "Address", data, random ? 0x01 : 0x00);
}

void packet_print_handle(uint16_t handle)
{
	print_handle_native(handle);
}

void packet_print_rssi(int8_t rssi)
{
	print_rssi(rssi);
}

void packet_print_ad(const void *data, uint8_t size)
{
	print_eir(data, size, true);
}

struct broadcast_message {
	uint32_t frame_sync_instant;
	uint16_t bluetooth_clock_phase;
	uint16_t left_open_offset;
	uint16_t left_close_offset;
	uint16_t right_open_offset;
	uint16_t right_close_offset;
	uint16_t frame_sync_period;
	uint8_t  frame_sync_period_fraction;
} __attribute__ ((packed));

static void print_3d_broadcast(const void *data, uint8_t size)
{
	const struct broadcast_message *msg = data;
	uint32_t instant;
	uint16_t left_open, left_close, right_open, right_close;
	uint16_t phase, period;
	uint8_t period_frac;
	bool mode;

	instant = le32_to_cpu(msg->frame_sync_instant);
	mode = !!(instant & 0x40000000);
	phase = le16_to_cpu(msg->bluetooth_clock_phase);
	left_open = le16_to_cpu(msg->left_open_offset);
	left_close = le16_to_cpu(msg->left_close_offset);
	right_open = le16_to_cpu(msg->right_open_offset);
	right_close = le16_to_cpu(msg->right_close_offset);
	period = le16_to_cpu(msg->frame_sync_period);
	period_frac = msg->frame_sync_period_fraction;

	print_field("  Frame sync instant: 0x%8.8x", instant & 0x7fffffff);
	print_field("  Video mode: %s (%d)", mode ? "Dual View" : "3D", mode);
	print_field("  Bluetooth clock phase: %d usec (0x%4.4x)",
						phase, phase);
	print_field("  Left lense shutter open offset: %d usec (0x%4.4x)",
						left_open, left_open);
	print_field("  Left lense shutter close offset: %d usec (0x%4.4x)",
						left_close, left_close);
	print_field("  Right lense shutter open offset: %d usec (0x%4.4x)",
						right_open, right_open);
	print_field("  Right lense shutter close offset: %d usec (0x%4.4x)",
						right_close, right_close);
	print_field("  Frame sync period: %d.%d usec (0x%4.4x 0x%2.2x)",
						period, period_frac * 256,
						period, period_frac);
}

void packet_hexdump(const unsigned char *buf, uint16_t len)
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
			print_text(COLOR_WHITE, "%s", str);
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
		print_text(COLOR_WHITE, "%s", str);
	}
}

void packet_control(struct timeval *tv, struct ucred *cred,
					uint16_t index, uint16_t opcode,
					const void *data, uint16_t size)
{
	control_message(opcode, data, size);
}

static int addr2str(const uint8_t *addr, char *str)
{
	return sprintf(str, "%2.2X:%2.2X:%2.2X:%2.2X:%2.2X:%2.2X",
			addr[5], addr[4], addr[3], addr[2], addr[1], addr[0]);
}

void packet_monitor(struct timeval *tv, struct ucred *cred,
					uint16_t index, uint16_t opcode,
					const void *data, uint16_t size)
{
	const struct btsnoop_opcode_new_index *ni;
	const struct btsnoop_opcode_index_info *ii;
	const struct btsnoop_opcode_user_logging *ul;
	char str[18], extra_str[24];
	uint16_t manufacturer;
	const char *ident;

	if (index != HCI_DEV_NONE) {
		index_current = index;
	}

	if (index != HCI_DEV_NONE && index >= MAX_INDEX) {
		print_field("Invalid index (%d)", index);
		return;
	}

	if (tv && time_offset == ((time_t) -1))
		time_offset = tv->tv_sec;

	switch (opcode) {
	case BTSNOOP_OPCODE_NEW_INDEX:
		ni = data;

		if (index < MAX_INDEX) {
			index_list[index].type = ni->type;
			memcpy(index_list[index].bdaddr, ni->bdaddr, 6);
			index_list[index].manufacturer = fallback_manufacturer;
			index_list[index].msft_opcode = BT_HCI_CMD_NOP;
		}

		addr2str(ni->bdaddr, str);
		packet_new_index(tv, index, str, ni->type, ni->bus, ni->name);
		break;
	case BTSNOOP_OPCODE_DEL_INDEX:
		if (index < MAX_INDEX)
			addr2str(index_list[index].bdaddr, str);
		else
			sprintf(str, "00:00:00:00:00:00");

		packet_del_index(tv, index, str);
		break;
	case BTSNOOP_OPCODE_COMMAND_PKT:
		packet_hci_command(tv, cred, index, data, size);
		break;
	case BTSNOOP_OPCODE_EVENT_PKT:
		packet_hci_event(tv, cred, index, data, size);
		break;
	case BTSNOOP_OPCODE_ACL_TX_PKT:
		packet_hci_acldata(tv, cred, index, false, data, size);
		break;
	case BTSNOOP_OPCODE_ACL_RX_PKT:
		packet_hci_acldata(tv, cred, index, true, data, size);
		break;
	case BTSNOOP_OPCODE_SCO_TX_PKT:
		packet_hci_scodata(tv, cred, index, false, data, size);
		break;
	case BTSNOOP_OPCODE_SCO_RX_PKT:
		packet_hci_scodata(tv, cred, index, true, data, size);
		break;
	case BTSNOOP_OPCODE_ISO_TX_PKT:
		packet_hci_isodata(tv, cred, index, false, data, size);
		break;
	case BTSNOOP_OPCODE_ISO_RX_PKT:
		packet_hci_isodata(tv, cred, index, true, data, size);
		break;
	case BTSNOOP_OPCODE_OPEN_INDEX:
		if (index < MAX_INDEX)
			addr2str(index_list[index].bdaddr, str);
		else
			sprintf(str, "00:00:00:00:00:00");

		packet_open_index(tv, index, str);
		break;
	case BTSNOOP_OPCODE_CLOSE_INDEX:
		if (index < MAX_INDEX)
			addr2str(index_list[index].bdaddr, str);
		else
			sprintf(str, "00:00:00:00:00:00");

		packet_close_index(tv, index, str);
		break;
	case BTSNOOP_OPCODE_INDEX_INFO:
		ii = data;
		manufacturer = le16_to_cpu(ii->manufacturer);

		if (index < MAX_INDEX) {
			memcpy(index_list[index].bdaddr, ii->bdaddr, 6);
			index_list[index].manufacturer = manufacturer;

			if (manufacturer == 2) {
				/*
				 * All Intel controllers that support the
				 * Microsoft vendor extension are using
				 * 0xFC1E for VsMsftOpCode.
				 */
				index_list[index].msft_opcode = 0xFC1E;
			}
		}

		addr2str(ii->bdaddr, str);
		packet_index_info(tv, index, str, manufacturer);
		break;
	case BTSNOOP_OPCODE_VENDOR_DIAG:
		if (index < MAX_INDEX)
			manufacturer = index_list[index].manufacturer;
		else
			manufacturer = fallback_manufacturer;

		packet_vendor_diag(tv, index, manufacturer, data, size);
		break;
	case BTSNOOP_OPCODE_SYSTEM_NOTE:
		packet_system_note(tv, cred, index, data);
		break;
	case BTSNOOP_OPCODE_USER_LOGGING:
		ul = data;
		ident = ul->ident_len ? data + sizeof(*ul) : NULL;

		packet_user_logging(tv, cred, index, ul->priority, ident,
					data + sizeof(*ul) + ul->ident_len,
					size - (sizeof(*ul) + ul->ident_len));
		break;
	case BTSNOOP_OPCODE_CTRL_OPEN:
		control_disable_decoding();
		packet_ctrl_open(tv, cred, index, data, size);
		break;
	case BTSNOOP_OPCODE_CTRL_CLOSE:
		packet_ctrl_close(tv, cred, index, data, size);
		break;
	case BTSNOOP_OPCODE_CTRL_COMMAND:
		packet_ctrl_command(tv, cred, index, data, size);
		break;
	case BTSNOOP_OPCODE_CTRL_EVENT:
		packet_ctrl_event(tv, cred, index, data, size);
		break;
	default:
		sprintf(extra_str, "(code %d len %d)", opcode, size);
		print_packet(tv, cred, '*', index, NULL, COLOR_ERROR,
					"Unknown packet", NULL, extra_str);
		packet_hexdump(data, size);
		break;
	}
}

void packet_simulator(struct timeval *tv, uint16_t frequency,
					const void *data, uint16_t size)
{
	char str[10];

	if (tv && time_offset == ((time_t) -1))
		time_offset = tv->tv_sec;

	sprintf(str, "%u MHz", frequency);

	print_packet(tv, NULL, '*', 0, NULL, COLOR_PHY_PACKET,
					"Physical packet:", NULL, str);

	ll_packet(frequency, data, size, false);
}

static void null_cmd(const void *data, uint8_t size)
{
}

static void status_rsp(const void *data, uint8_t size)
{
	uint8_t status = *((const uint8_t *) data);

	print_status(status);
}

static void status_handle_rsp(const void *data, uint8_t size)
{
	uint8_t status = *((const uint8_t *) data);

	print_status(status);
	print_field("Connection handle: %d", get_u8(data + 1));
}

static void status_bdaddr_rsp(const void *data, uint8_t size)
{
	uint8_t status = *((const uint8_t *) data);

	print_status(status);
	print_bdaddr(data + 1);
}

static void inquiry_cmd(const void *data, uint8_t size)
{
	const struct bt_hci_cmd_inquiry *cmd = data;

	print_iac(cmd->lap);
	print_field("Length: %.2fs (0x%2.2x)",
				cmd->length * 1.28, cmd->length);
	print_num_resp(cmd->num_resp);
}

static void periodic_inquiry_cmd(const void *data, uint8_t size)
{
	const struct bt_hci_cmd_periodic_inquiry *cmd = data;

	print_field("Max period: %.2fs (0x%2.2x)",
				cmd->max_period * 1.28, cmd->max_period);
	print_field("Min period: %.2fs (0x%2.2x)",
				cmd->min_period * 1.28, cmd->min_period);
	print_iac(cmd->lap);
	print_field("Length: %.2fs (0x%2.2x)",
				cmd->length * 1.28, cmd->length);
	print_num_resp(cmd->num_resp);
}

static void create_conn_cmd(const void *data, uint8_t size)
{
	const struct bt_hci_cmd_create_conn *cmd = data;
	const char *str;

	print_bdaddr(cmd->bdaddr);
	print_pkt_type(cmd->pkt_type);
	print_pscan_rep_mode(cmd->pscan_rep_mode);
	print_pscan_mode(cmd->pscan_mode);
	print_clock_offset(cmd->clock_offset);

	switch (cmd->role_switch) {
	case 0x00:
		str = "Stay master";
		break;
	case 0x01:
		str = "Allow slave";
		break;
	default:
		str = "Reserved";
		break;
	}

	print_field("Role switch: %s (0x%2.2x)", str, cmd->role_switch);
}

static void disconnect_cmd(const void *data, uint8_t size)
{
	const struct bt_hci_cmd_disconnect *cmd = data;

	print_handle(cmd->handle);
	print_reason(cmd->reason);
}

static void add_sco_conn_cmd(const void *data, uint8_t size)
{
	const struct bt_hci_cmd_add_sco_conn *cmd = data;

	print_handle(cmd->handle);
	print_pkt_type_sco(cmd->pkt_type);
}

static void create_conn_cancel_cmd(const void *data, uint8_t size)
{
	const struct bt_hci_cmd_create_conn_cancel *cmd = data;

	print_bdaddr(cmd->bdaddr);
}

static void accept_conn_request_cmd(const void *data, uint8_t size)
{
	const struct bt_hci_cmd_accept_conn_request *cmd = data;

	print_bdaddr(cmd->bdaddr);
	print_role(cmd->role);
}

static void reject_conn_request_cmd(const void *data, uint8_t size)
{
	const struct bt_hci_cmd_reject_conn_request *cmd = data;

	print_bdaddr(cmd->bdaddr);
	print_reason(cmd->reason);
}

static void link_key_request_reply_cmd(const void *data, uint8_t size)
{
	const struct bt_hci_cmd_link_key_request_reply *cmd = data;

	print_bdaddr(cmd->bdaddr);
	print_link_key(cmd->link_key);
}

static void link_key_request_neg_reply_cmd(const void *data, uint8_t size)
{
	const struct bt_hci_cmd_link_key_request_neg_reply *cmd = data;

	print_bdaddr(cmd->bdaddr);
}

static void pin_code_request_reply_cmd(const void *data, uint8_t size)
{
	const struct bt_hci_cmd_pin_code_request_reply *cmd = data;

	print_bdaddr(cmd->bdaddr);
	print_field("PIN length: %d", cmd->pin_len);
	print_pin_code(cmd->pin_code, cmd->pin_len);
}

static void pin_code_request_neg_reply_cmd(const void *data, uint8_t size)
{
	const struct bt_hci_cmd_pin_code_request_neg_reply *cmd = data;

	print_bdaddr(cmd->bdaddr);
}

static void change_conn_pkt_type_cmd(const void *data, uint8_t size)
{
	const struct bt_hci_cmd_change_conn_pkt_type *cmd = data;

	print_handle(cmd->handle);
	print_pkt_type(cmd->pkt_type);
}

static void auth_requested_cmd(const void *data, uint8_t size)
{
	const struct bt_hci_cmd_auth_requested *cmd = data;

	print_handle(cmd->handle);
}

static void set_conn_encrypt_cmd(const void *data, uint8_t size)
{
	const struct bt_hci_cmd_set_conn_encrypt *cmd = data;

	print_handle(cmd->handle);
	print_enable("Encryption", cmd->encr_mode);
}

static void change_conn_link_key_cmd(const void *data, uint8_t size)
{
	const struct bt_hci_cmd_change_conn_link_key *cmd = data;

	print_handle(cmd->handle);
}

static void master_link_key_cmd(const void *data, uint8_t size)
{
	const struct bt_hci_cmd_master_link_key *cmd = data;

	print_key_flag(cmd->key_flag);
}

static void remote_name_request_cmd(const void *data, uint8_t size)
{
	const struct bt_hci_cmd_remote_name_request *cmd = data;

	print_bdaddr(cmd->bdaddr);
	print_pscan_rep_mode(cmd->pscan_rep_mode);
	print_pscan_mode(cmd->pscan_mode);
	print_clock_offset(cmd->clock_offset);
}

static void remote_name_request_cancel_cmd(const void *data, uint8_t size)
{
	const struct bt_hci_cmd_remote_name_request_cancel *cmd = data;

	print_bdaddr(cmd->bdaddr);
}

static void read_remote_features_cmd(const void *data, uint8_t size)
{
	const struct bt_hci_cmd_read_remote_features *cmd = data;

	print_handle(cmd->handle);
}

static void read_remote_ext_features_cmd(const void *data, uint8_t size)
{
	const struct bt_hci_cmd_read_remote_ext_features *cmd = data;

	print_handle(cmd->handle);
	print_field("Page: %d", cmd->page);
}

static void read_remote_version_cmd(const void *data, uint8_t size)
{
	const struct bt_hci_cmd_read_remote_version *cmd = data;

	print_handle(cmd->handle);
}

static void read_clock_offset_cmd(const void *data, uint8_t size)
{
	const struct bt_hci_cmd_read_clock_offset *cmd = data;

	print_handle(cmd->handle);
}

static void read_lmp_handle_cmd(const void *data, uint8_t size)
{
	const struct bt_hci_cmd_read_lmp_handle *cmd = data;

	print_handle(cmd->handle);
}

static void read_lmp_handle_rsp(const void *data, uint8_t size)
{
	const struct bt_hci_rsp_read_lmp_handle *rsp = data;

	print_status(rsp->status);
	print_handle(rsp->handle);
	print_field("LMP handle: %d", rsp->lmp_handle);
	print_field("Reserved: %d", le32_to_cpu(rsp->reserved));
}

static void setup_sync_conn_cmd(const void *data, uint8_t size)
{
	const struct bt_hci_cmd_setup_sync_conn *cmd = data;

	print_handle(cmd->handle);
	print_field("Transmit bandwidth: %d", le32_to_cpu(cmd->tx_bandwidth));
	print_field("Receive bandwidth: %d", le32_to_cpu(cmd->rx_bandwidth));
	print_field("Max latency: %d", le16_to_cpu(cmd->max_latency));
	print_voice_setting(cmd->voice_setting);
	print_retransmission_effort(cmd->retrans_effort);
	print_pkt_type_sco(cmd->pkt_type);
}

static void accept_sync_conn_request_cmd(const void *data, uint8_t size)
{
	const struct bt_hci_cmd_accept_sync_conn_request *cmd = data;

	print_bdaddr(cmd->bdaddr);
	print_field("Transmit bandwidth: %d", le32_to_cpu(cmd->tx_bandwidth));
	print_field("Receive bandwidth: %d", le32_to_cpu(cmd->rx_bandwidth));
	print_field("Max latency: %d", le16_to_cpu(cmd->max_latency));
	print_voice_setting(cmd->voice_setting);
	print_retransmission_effort(cmd->retrans_effort);
	print_pkt_type_sco(cmd->pkt_type);
}

static void reject_sync_conn_request_cmd(const void *data, uint8_t size)
{
	const struct bt_hci_cmd_reject_sync_conn_request *cmd = data;

	print_bdaddr(cmd->bdaddr);
	print_reason(cmd->reason);
}

static void io_capability_request_reply_cmd(const void *data, uint8_t size)
{
	const struct bt_hci_cmd_io_capability_request_reply *cmd = data;

	print_bdaddr(cmd->bdaddr);
	print_io_capability(cmd->capability);
	print_oob_data(cmd->oob_data);
	print_authentication(cmd->authentication);
}

static void user_confirm_request_reply_cmd(const void *data, uint8_t size)
{
	const struct bt_hci_cmd_user_confirm_request_reply *cmd = data;

	print_bdaddr(cmd->bdaddr);
}

static void user_confirm_request_neg_reply_cmd(const void *data, uint8_t size)
{
	const struct bt_hci_cmd_user_confirm_request_neg_reply *cmd = data;

	print_bdaddr(cmd->bdaddr);
}

static void user_passkey_request_reply_cmd(const void *data, uint8_t size)
{
	const struct bt_hci_cmd_user_passkey_request_reply *cmd = data;

	print_bdaddr(cmd->bdaddr);
	print_passkey(cmd->passkey);
}

static void user_passkey_request_neg_reply_cmd(const void *data, uint8_t size)
{
	const struct bt_hci_cmd_user_passkey_request_neg_reply *cmd = data;

	print_bdaddr(cmd->bdaddr);
}

static void remote_oob_data_request_reply_cmd(const void *data, uint8_t size)
{
	const struct bt_hci_cmd_remote_oob_data_request_reply *cmd = data;

	print_bdaddr(cmd->bdaddr);
	print_hash_p192(cmd->hash);
	print_randomizer_p192(cmd->randomizer);
}

static void remote_oob_data_request_neg_reply_cmd(const void *data, uint8_t size)
{
	const struct bt_hci_cmd_remote_oob_data_request_neg_reply *cmd = data;

	print_bdaddr(cmd->bdaddr);
}

static void io_capability_request_neg_reply_cmd(const void *data, uint8_t size)
{
	const struct bt_hci_cmd_io_capability_request_neg_reply *cmd = data;

	print_bdaddr(cmd->bdaddr);
	print_reason(cmd->reason);
}

static void create_phy_link_cmd(const void *data, uint8_t size)
{
	const struct bt_hci_cmd_create_phy_link *cmd = data;

	print_phy_handle(cmd->phy_handle);
	print_key_len(cmd->key_len);
	print_key_type(cmd->key_type);

	packet_hexdump(data + 3, size - 3);
}

static void accept_phy_link_cmd(const void *data, uint8_t size)
{
	const struct bt_hci_cmd_accept_phy_link *cmd = data;

	print_phy_handle(cmd->phy_handle);
	print_key_len(cmd->key_len);
	print_key_type(cmd->key_type);

	packet_hexdump(data + 3, size - 3);
}

static void disconn_phy_link_cmd(const void *data, uint8_t size)
{
	const struct bt_hci_cmd_disconn_phy_link *cmd = data;

	print_phy_handle(cmd->phy_handle);
	print_reason(cmd->reason);
}

static void create_logic_link_cmd(const void *data, uint8_t size)
{
	const struct bt_hci_cmd_create_logic_link *cmd = data;

	print_phy_handle(cmd->phy_handle);
	print_flow_spec("TX", cmd->tx_flow_spec);
	print_flow_spec("RX", cmd->rx_flow_spec);
}

static void accept_logic_link_cmd(const void *data, uint8_t size)
{
	const struct bt_hci_cmd_accept_logic_link *cmd = data;

	print_phy_handle(cmd->phy_handle);
	print_flow_spec("TX", cmd->tx_flow_spec);
	print_flow_spec("RX", cmd->rx_flow_spec);
}

static void disconn_logic_link_cmd(const void *data, uint8_t size)
{
	const struct bt_hci_cmd_disconn_logic_link *cmd = data;

	print_handle(cmd->handle);
}

static void logic_link_cancel_cmd(const void *data, uint8_t size)
{
	const struct bt_hci_cmd_logic_link_cancel *cmd = data;

	print_phy_handle(cmd->phy_handle);
	print_field("TX flow spec: 0x%2.2x", cmd->flow_spec);
}

static void logic_link_cancel_rsp(const void *data, uint8_t size)
{
	const struct bt_hci_rsp_logic_link_cancel *rsp = data;

	print_status(rsp->status);
	print_phy_handle(rsp->phy_handle);
	print_field("TX flow spec: 0x%2.2x", rsp->flow_spec);
}

static void flow_spec_modify_cmd(const void *data, uint8_t size)
{
	const struct bt_hci_cmd_flow_spec_modify *cmd = data;

	print_handle(cmd->handle);
	print_flow_spec("TX", cmd->tx_flow_spec);
	print_flow_spec("RX", cmd->rx_flow_spec);
}

static void enhanced_setup_sync_conn_cmd(const void *data, uint8_t size)
{
	const struct bt_hci_cmd_enhanced_setup_sync_conn *cmd = data;

	print_handle(cmd->handle);
	print_field("Transmit bandwidth: %d", le32_to_cpu(cmd->tx_bandwidth));
	print_field("Receive bandwidth: %d", le32_to_cpu(cmd->rx_bandwidth));

	/* TODO */

	print_field("Max latency: %d", le16_to_cpu(cmd->max_latency));
	print_pkt_type_sco(cmd->pkt_type);
	print_retransmission_effort(cmd->retrans_effort);
}

static void enhanced_accept_sync_conn_request_cmd(const void *data, uint8_t size)
{
	const struct bt_hci_cmd_enhanced_accept_sync_conn_request *cmd = data;

	print_bdaddr(cmd->bdaddr);
	print_field("Transmit bandwidth: %d", le32_to_cpu(cmd->tx_bandwidth));
	print_field("Receive bandwidth: %d", le32_to_cpu(cmd->rx_bandwidth));

	/* TODO */

	print_field("Max latency: %d", le16_to_cpu(cmd->max_latency));
	print_pkt_type_sco(cmd->pkt_type);
	print_retransmission_effort(cmd->retrans_effort);
}

static void truncated_page_cmd(const void *data, uint8_t size)
{
	const struct bt_hci_cmd_truncated_page *cmd = data;

	print_bdaddr(cmd->bdaddr);
	print_pscan_rep_mode(cmd->pscan_rep_mode);
	print_clock_offset(cmd->clock_offset);
}

static void truncated_page_cancel_cmd(const void *data, uint8_t size)
{
	const struct bt_hci_cmd_truncated_page_cancel *cmd = data;

	print_bdaddr(cmd->bdaddr);
}

static void set_slave_broadcast_cmd(const void *data, uint8_t size)
{
	const struct bt_hci_cmd_set_slave_broadcast *cmd = data;

	print_field("Enable: 0x%2.2x", cmd->enable);
	print_lt_addr(cmd->lt_addr);
	print_lpo_allowed(cmd->lpo_allowed);
	print_pkt_type(cmd->pkt_type);
	print_slot_625("Min interval", cmd->min_interval);
	print_slot_625("Max interval", cmd->max_interval);
	print_slot_625("Supervision timeout", cmd->timeout);
}

static void set_slave_broadcast_rsp(const void *data, uint8_t size)
{
	const struct bt_hci_rsp_set_slave_broadcast *rsp = data;

	print_status(rsp->status);
	print_lt_addr(rsp->lt_addr);
	print_interval(rsp->interval);
}

static void set_slave_broadcast_receive_cmd(const void *data, uint8_t size)
{
	const struct bt_hci_cmd_set_slave_broadcast_receive *cmd = data;

	print_field("Enable: 0x%2.2x", cmd->enable);
	print_bdaddr(cmd->bdaddr);
	print_lt_addr(cmd->lt_addr);
	print_interval(cmd->interval);
	print_field("Offset: 0x%8.8x", le32_to_cpu(cmd->offset));
	print_field("Next broadcast instant: 0x%4.4x",
					le16_to_cpu(cmd->instant));
	print_slot_625("Supervision timeout", cmd->timeout);
	print_field("Remote timing accuracy: %d ppm", cmd->accuracy);
	print_field("Skip: 0x%2.2x", cmd->skip);
	print_pkt_type(cmd->pkt_type);
	print_channel_map(cmd->map);
}

static void set_slave_broadcast_receive_rsp(const void *data, uint8_t size)
{
	const struct bt_hci_rsp_set_slave_broadcast_receive *rsp = data;

	print_status(rsp->status);
	print_bdaddr(rsp->bdaddr);
	print_lt_addr(rsp->lt_addr);
}

static void receive_sync_train_cmd(const void *data, uint8_t size)
{
	const struct bt_hci_cmd_receive_sync_train *cmd = data;

	print_bdaddr(cmd->bdaddr);
	print_timeout(cmd->timeout);
	print_window(cmd->window);
	print_interval(cmd->interval);
}

static void remote_oob_ext_data_request_reply_cmd(const void *data, uint8_t size)
{
	const struct bt_hci_cmd_remote_oob_ext_data_request_reply *cmd = data;

	print_bdaddr(cmd->bdaddr);
	print_hash_p192(cmd->hash192);
	print_randomizer_p192(cmd->randomizer192);
	print_hash_p256(cmd->hash256);
	print_randomizer_p256(cmd->randomizer256);
}

static void hold_mode_cmd(const void *data, uint8_t size)
{
	const struct bt_hci_cmd_hold_mode *cmd = data;

	print_handle(cmd->handle);
	print_slot_625("Hold max interval", cmd->max_interval);
	print_slot_625("Hold min interval", cmd->min_interval);
}

static void sniff_mode_cmd(const void *data, uint8_t size)
{
	const struct bt_hci_cmd_sniff_mode *cmd = data;

	print_handle(cmd->handle);
	print_slot_625("Sniff max interval", cmd->max_interval);
	print_slot_625("Sniff min interval", cmd->min_interval);
	print_slot_125("Sniff attempt", cmd->attempt);
	print_slot_125("Sniff timeout", cmd->timeout);
}

static void exit_sniff_mode_cmd(const void *data, uint8_t size)
{
	const struct bt_hci_cmd_exit_sniff_mode *cmd = data;

	print_handle(cmd->handle);
}

static void park_state_cmd(const void *data, uint8_t size)
{
	const struct bt_hci_cmd_park_state *cmd = data;

	print_handle(cmd->handle);
	print_slot_625("Beacon max interval", cmd->max_interval);
	print_slot_625("Beacon min interval", cmd->min_interval);
}

static void exit_park_state_cmd(const void *data, uint8_t size)
{
	const struct bt_hci_cmd_exit_park_state *cmd = data;

	print_handle(cmd->handle);
}

static void qos_setup_cmd(const void *data, uint8_t size)
{
	const struct bt_hci_cmd_qos_setup *cmd = data;

	print_handle(cmd->handle);
	print_field("Flags: 0x%2.2x", cmd->flags);

	print_service_type(cmd->service_type);

	print_field("Token rate: %d", le32_to_cpu(cmd->token_rate));
	print_field("Peak bandwidth: %d", le32_to_cpu(cmd->peak_bandwidth));
	print_field("Latency: %d", le32_to_cpu(cmd->latency));
	print_field("Delay variation: %d", le32_to_cpu(cmd->delay_variation));
}

static void role_discovery_cmd(const void *data, uint8_t size)
{
	const struct bt_hci_cmd_role_discovery *cmd = data;

	print_handle(cmd->handle);
}

static void role_discovery_rsp(const void *data, uint8_t size)
{
	const struct bt_hci_rsp_role_discovery *rsp = data;

	print_status(rsp->status);
	print_handle(rsp->handle);
	print_role(rsp->role);
}

static void switch_role_cmd(const void *data, uint8_t size)
{
	const struct bt_hci_cmd_switch_role *cmd = data;

	print_bdaddr(cmd->bdaddr);
	print_role(cmd->role);
}

static void read_link_policy_cmd(const void *data, uint8_t size)
{
	const struct bt_hci_cmd_read_link_policy *cmd = data;

	print_handle(cmd->handle);
}

static void read_link_policy_rsp(const void *data, uint8_t size)
{
	const struct bt_hci_rsp_read_link_policy *rsp = data;

	print_status(rsp->status);
	print_handle(rsp->handle);
	print_link_policy(rsp->policy);
}

static void write_link_policy_cmd(const void *data, uint8_t size)
{
	const struct bt_hci_cmd_write_link_policy *cmd = data;

	print_handle(cmd->handle);
	print_link_policy(cmd->policy);
}

static void write_link_policy_rsp(const void *data, uint8_t size)
{
	const struct bt_hci_rsp_write_link_policy *rsp = data;

	print_status(rsp->status);
	print_handle(rsp->handle);
}

static void read_default_link_policy_rsp(const void *data, uint8_t size)
{
	const struct bt_hci_rsp_read_default_link_policy *rsp = data;

	print_status(rsp->status);
	print_link_policy(rsp->policy);
}

static void write_default_link_policy_cmd(const void *data, uint8_t size)
{
	const struct bt_hci_cmd_write_default_link_policy *cmd = data;

	print_link_policy(cmd->policy);
}

static void flow_spec_cmd(const void *data, uint8_t size)
{
	const struct bt_hci_cmd_flow_spec *cmd = data;

	print_handle(cmd->handle);
	print_field("Flags: 0x%2.2x", cmd->flags);

	print_flow_direction(cmd->direction);
	print_service_type(cmd->service_type);

	print_field("Token rate: %d", le32_to_cpu(cmd->token_rate));
	print_field("Token bucket size: %d",
					le32_to_cpu(cmd->token_bucket_size));
	print_field("Peak bandwidth: %d", le32_to_cpu(cmd->peak_bandwidth));
	print_field("Access latency: %d", le32_to_cpu(cmd->access_latency));
}

static void sniff_subrating_cmd(const void *data, uint8_t size)
{
	const struct bt_hci_cmd_sniff_subrating *cmd = data;

	print_handle(cmd->handle);
	print_slot_625("Max latency", cmd->max_latency);
	print_slot_625("Min remote timeout", cmd->min_remote_timeout);
	print_slot_625("Min local timeout", cmd->min_local_timeout);
}

static void sniff_subrating_rsp(const void *data, uint8_t size)
{
	const struct bt_hci_rsp_sniff_subrating *rsp = data;

	print_status(rsp->status);
	print_handle(rsp->handle);
}

static void set_event_mask_cmd(const void *data, uint8_t size)
{
	const struct bt_hci_cmd_set_event_mask *cmd = data;

	print_event_mask(cmd->mask, events_table);
}

static void set_event_filter_cmd(const void *data, uint8_t size)
{
	uint8_t type = *((const uint8_t *) data);
	uint8_t filter;
	const char *str;

	switch (type) {
	case 0x00:
		str = "Clear All Filters";
		break;
	case 0x01:
		str = "Inquiry Result";
		break;
	case 0x02:
		str = "Connection Setup";
		break;
	default:
		str = "Reserved";
		break;
	}

	print_field("Type: %s (0x%2.2x)", str, type);

	switch (type) {
	case 0x00:
		if (size > 1) {
			print_text(COLOR_ERROR, "  invalid parameter size");
			packet_hexdump(data + 1, size - 1);
		}
		break;

	case 0x01:
		if (size < 2) {
			print_text(COLOR_ERROR, "  invalid parameter size");
			break;
		}
		filter = *((const uint8_t *) (data + 1));

		switch (filter) {
		case 0x00:
			str = "Return responses from all devices";
			break;
		case 0x01:
			str = "Device with specific Class of Device";
			break;
		case 0x02:
			str = "Device with specific BD_ADDR";
			break;
		default:
			str = "Reserved";
			break;
		}

		print_field("Filter: %s (0x%2.2x)", str, filter);
		packet_hexdump(data + 2, size - 2);
		break;

	case 0x02:
		filter = *((const uint8_t *) (data + 1));

		switch (filter) {
		case 0x00:
			str = "Allow connections all devices";
			break;
		case 0x01:
			str = "Allow connections with specific Class of Device";
			break;
		case 0x02:
			str = "Allow connections with specific BD_ADDR";
			break;
		default:
			str = "Reserved";
			break;
		}

		if (size < 2) {
                        print_text(COLOR_ERROR, "  invalid parameter size");
                        break;
                }

		print_field("Filter: %s (0x%2.2x)", str, filter);
		packet_hexdump(data + 2, size - 2);
		break;

	default:
		if (size < 2) {
                        print_text(COLOR_ERROR, "  invalid parameter size");
                        break;
                }

		filter = *((const uint8_t *) (data + 1));

		print_field("Filter: Reserved (0x%2.2x)", filter);
		packet_hexdump(data + 2, size - 2);
		break;
	}
}

static void flush_cmd(const void *data, uint8_t size)
{
	const struct bt_hci_cmd_flush *cmd = data;

	print_handle(cmd->handle);
}

static void flush_rsp(const void *data, uint8_t size)
{
	const struct bt_hci_rsp_flush *rsp = data;

	print_status(rsp->status);
	print_handle(rsp->handle);
}

static void read_pin_type_rsp(const void *data, uint8_t size)
{
	const struct bt_hci_rsp_read_pin_type *rsp = data;

	print_status(rsp->status);
	print_pin_type(rsp->pin_type);
}

static void write_pin_type_cmd(const void *data, uint8_t size)
{
	const struct bt_hci_cmd_write_pin_type *cmd = data;

	print_pin_type(cmd->pin_type);
}

static void read_stored_link_key_cmd(const void *data, uint8_t size)
{
	const struct bt_hci_cmd_read_stored_link_key *cmd = data;

	print_bdaddr(cmd->bdaddr);
	print_field("Read all: 0x%2.2x", cmd->read_all);
}

static void read_stored_link_key_rsp(const void *data, uint8_t size)
{
	const struct bt_hci_rsp_read_stored_link_key *rsp = data;

	print_status(rsp->status);
	print_field("Max num keys: %d", le16_to_cpu(rsp->max_num_keys));
	print_field("Num keys: %d", le16_to_cpu(rsp->num_keys));
}

static void write_stored_link_key_cmd(const void *data, uint8_t size)
{
	const struct bt_hci_cmd_write_stored_link_key *cmd = data;

	print_field("Num keys: %d", cmd->num_keys);

	packet_hexdump(data + 1, size - 1);
}

static void write_stored_link_key_rsp(const void *data, uint8_t size)
{
	const struct bt_hci_rsp_write_stored_link_key *rsp = data;

	print_status(rsp->status);
	print_field("Num keys: %d", rsp->num_keys);
}

static void delete_stored_link_key_cmd(const void *data, uint8_t size)
{
	const struct bt_hci_cmd_delete_stored_link_key *cmd = data;

	print_bdaddr(cmd->bdaddr);
	print_field("Delete all: 0x%2.2x", cmd->delete_all);
}

static void delete_stored_link_key_rsp(const void *data, uint8_t size)
{
	const struct bt_hci_rsp_delete_stored_link_key *rsp = data;

	print_status(rsp->status);
	print_field("Num keys: %d", le16_to_cpu(rsp->num_keys));
}

static void write_local_name_cmd(const void *data, uint8_t size)
{
	const struct bt_hci_cmd_write_local_name *cmd = data;

	print_name(cmd->name);
}

static void read_local_name_rsp(const void *data, uint8_t size)
{
	const struct bt_hci_rsp_read_local_name *rsp = data;

	print_status(rsp->status);
	print_name(rsp->name);
}

static void read_conn_accept_timeout_rsp(const void *data, uint8_t size)
{
	const struct bt_hci_rsp_read_conn_accept_timeout *rsp = data;

	print_status(rsp->status);
	print_timeout(rsp->timeout);
}

static void write_conn_accept_timeout_cmd(const void *data, uint8_t size)
{
	const struct bt_hci_cmd_write_conn_accept_timeout *cmd = data;

	print_timeout(cmd->timeout);
}

static void read_page_timeout_rsp(const void *data, uint8_t size)
{
	const struct bt_hci_rsp_read_page_timeout *rsp = data;

	print_status(rsp->status);
	print_timeout(rsp->timeout);
}

static void write_page_timeout_cmd(const void *data, uint8_t size)
{
	const struct bt_hci_cmd_write_page_timeout *cmd = data;

	print_timeout(cmd->timeout);
}

static void read_scan_enable_rsp(const void *data, uint8_t size)
{
	const struct bt_hci_rsp_read_scan_enable *rsp = data;

	print_status(rsp->status);
	print_scan_enable(rsp->enable);
}

static void write_scan_enable_cmd(const void *data, uint8_t size)
{
	const struct bt_hci_cmd_write_scan_enable *cmd = data;

	print_scan_enable(cmd->enable);
}

static void read_page_scan_activity_rsp(const void *data, uint8_t size)
{
	const struct bt_hci_rsp_read_page_scan_activity *rsp = data;

	print_status(rsp->status);
	print_interval(rsp->interval);
	print_window(rsp->window);
}

static void write_page_scan_activity_cmd(const void *data, uint8_t size)
{
	const struct bt_hci_cmd_write_page_scan_activity *cmd = data;

	print_interval(cmd->interval);
	print_window(cmd->window);
}

static void read_inquiry_scan_activity_rsp(const void *data, uint8_t size)
{
	const struct bt_hci_rsp_read_inquiry_scan_activity *rsp = data;

	print_status(rsp->status);
	print_interval(rsp->interval);
	print_window(rsp->window);
}

static void write_inquiry_scan_activity_cmd(const void *data, uint8_t size)
{
	const struct bt_hci_cmd_write_inquiry_scan_activity *cmd = data;

	print_interval(cmd->interval);
	print_window(cmd->window);
}

static void read_auth_enable_rsp(const void *data, uint8_t size)
{
	const struct bt_hci_rsp_read_auth_enable *rsp = data;

	print_status(rsp->status);
	print_auth_enable(rsp->enable);
}

static void write_auth_enable_cmd(const void *data, uint8_t size)
{
	const struct bt_hci_cmd_write_auth_enable *cmd = data;

	print_auth_enable(cmd->enable);
}

static void read_encrypt_mode_rsp(const void *data, uint8_t size)
{
	const struct bt_hci_rsp_read_encrypt_mode *rsp = data;

	print_status(rsp->status);
	print_encrypt_mode(rsp->mode);
}

static void write_encrypt_mode_cmd(const void *data, uint8_t size)
{
	const struct bt_hci_cmd_write_encrypt_mode *cmd = data;

	print_encrypt_mode(cmd->mode);
}

static void read_class_of_dev_rsp(const void *data, uint8_t size)
{
	const struct bt_hci_rsp_read_class_of_dev *rsp = data;

	print_status(rsp->status);
	print_dev_class(rsp->dev_class);
}

static void write_class_of_dev_cmd(const void *data, uint8_t size)
{
	const struct bt_hci_cmd_write_class_of_dev *cmd = data;

	print_dev_class(cmd->dev_class);
}

static void read_voice_setting_rsp(const void *data, uint8_t size)
{
	const struct bt_hci_rsp_read_voice_setting *rsp = data;

	print_status(rsp->status);
	print_voice_setting(rsp->setting);
}

static void write_voice_setting_cmd(const void *data, uint8_t size)
{
	const struct bt_hci_cmd_write_voice_setting *cmd = data;

	print_voice_setting(cmd->setting);
}

static void read_auto_flush_timeout_cmd(const void *data, uint8_t size)
{
	const struct bt_hci_cmd_read_auto_flush_timeout *cmd = data;

	print_handle(cmd->handle);
}

static void read_auto_flush_timeout_rsp(const void *data, uint8_t size)
{
	const struct bt_hci_rsp_read_auto_flush_timeout *rsp = data;

	print_status(rsp->status);
	print_handle(rsp->handle);
	print_flush_timeout(rsp->timeout);
}

static void write_auto_flush_timeout_cmd(const void *data, uint8_t size)
{
	const struct bt_hci_cmd_write_auto_flush_timeout *cmd = data;

	print_handle(cmd->handle);
	print_flush_timeout(cmd->timeout);
}

static void write_auto_flush_timeout_rsp(const void *data, uint8_t size)
{
	const struct bt_hci_rsp_write_auto_flush_timeout *rsp = data;

	print_status(rsp->status);
	print_handle(rsp->handle);
}

static void read_num_broadcast_retrans_rsp(const void *data, uint8_t size)
{
	const struct bt_hci_rsp_read_num_broadcast_retrans *rsp = data;

	print_status(rsp->status);
	print_num_broadcast_retrans(rsp->num_retrans);
}

static void write_num_broadcast_retrans_cmd(const void *data, uint8_t size)
{
	const struct bt_hci_cmd_write_num_broadcast_retrans *cmd = data;

	print_num_broadcast_retrans(cmd->num_retrans);
}

static void read_hold_mode_activity_rsp(const void *data, uint8_t size)
{
	const struct bt_hci_rsp_read_hold_mode_activity *rsp = data;

	print_status(rsp->status);
	print_hold_mode_activity(rsp->activity);
}

static void write_hold_mode_activity_cmd(const void *data, uint8_t size)
{
	const struct bt_hci_cmd_write_hold_mode_activity *cmd = data;

	print_hold_mode_activity(cmd->activity);
}

static void read_tx_power_cmd(const void *data, uint8_t size)
{
	const struct bt_hci_cmd_read_tx_power *cmd = data;

	print_handle(cmd->handle);
	print_power_type(cmd->type);
}

static void read_tx_power_rsp(const void *data, uint8_t size)
{
	const struct bt_hci_rsp_read_tx_power *rsp = data;

	print_status(rsp->status);
	print_handle(rsp->handle);
	print_power_level(rsp->level, NULL);
}

static void read_sync_flow_control_rsp(const void *data, uint8_t size)
{
	const struct bt_hci_rsp_read_sync_flow_control *rsp = data;

	print_status(rsp->status);
	print_enable("Flow control", rsp->enable);
}

static void write_sync_flow_control_cmd(const void *data, uint8_t size)
{
	const struct bt_hci_cmd_write_sync_flow_control *cmd = data;

	print_enable("Flow control", cmd->enable);
}

static void set_host_flow_control_cmd(const void *data, uint8_t size)
{
	const struct bt_hci_cmd_set_host_flow_control *cmd = data;

	print_host_flow_control(cmd->enable);
}

static void host_buffer_size_cmd(const void *data, uint8_t size)
{
	const struct bt_hci_cmd_host_buffer_size *cmd = data;

	print_field("ACL MTU: %-4d ACL max packet: %d",
					le16_to_cpu(cmd->acl_mtu),
					le16_to_cpu(cmd->acl_max_pkt));
	print_field("SCO MTU: %-4d SCO max packet: %d",
					cmd->sco_mtu,
					le16_to_cpu(cmd->sco_max_pkt));
}

static void host_num_completed_packets_cmd(const void *data, uint8_t size)
{
	const struct bt_hci_cmd_host_num_completed_packets *cmd = data;

	print_field("Num handles: %d", cmd->num_handles);
	print_handle(cmd->handle);
	print_field("Count: %d", le16_to_cpu(cmd->count));

	if (size > sizeof(*cmd))
		packet_hexdump(data + sizeof(*cmd), size - sizeof(*cmd));
}

static void read_link_supv_timeout_cmd(const void *data, uint8_t size)
{
	const struct bt_hci_cmd_read_link_supv_timeout *cmd = data;

	print_handle(cmd->handle);
}

static void read_link_supv_timeout_rsp(const void *data, uint8_t size)
{
	const struct bt_hci_rsp_read_link_supv_timeout *rsp = data;

	print_status(rsp->status);
	print_handle(rsp->handle);
	print_timeout(rsp->timeout);
}

static void write_link_supv_timeout_cmd(const void *data, uint8_t size)
{
	const struct bt_hci_cmd_write_link_supv_timeout *cmd = data;

	print_handle(cmd->handle);
	print_timeout(cmd->timeout);
}

static void write_link_supv_timeout_rsp(const void *data, uint8_t size)
{
	const struct bt_hci_rsp_write_link_supv_timeout *rsp = data;

	print_status(rsp->status);
	print_handle(rsp->handle);
}

static void read_num_supported_iac_rsp(const void *data, uint8_t size)
{
	const struct bt_hci_rsp_read_num_supported_iac *rsp = data;

	print_status(rsp->status);
	print_field("Number of IAC: %d", rsp->num_iac);
}

static void read_current_iac_lap_rsp(const void *data, uint8_t size)
{
	const struct bt_hci_rsp_read_current_iac_lap *rsp = data;
	uint8_t i;

	print_status(rsp->status);
	print_field("Number of IAC: %d", rsp->num_iac);

	for (i = 0; i < rsp->num_iac; i++)
		print_iac(rsp->iac_lap + (i * 3));
}

static void write_current_iac_lap_cmd(const void *data, uint8_t size)
{
	const struct bt_hci_cmd_write_current_iac_lap *cmd = data;
	uint8_t i;

	print_field("Number of IAC: %d", cmd->num_iac);

	for (i = 0; i < cmd->num_iac; i++)
		print_iac(cmd->iac_lap + (i * 3));
}

static void read_page_scan_period_mode_rsp(const void *data, uint8_t size)
{
	const struct bt_hci_rsp_read_page_scan_period_mode *rsp = data;

	print_status(rsp->status);
	print_pscan_period_mode(rsp->mode);
}

static void write_page_scan_period_mode_cmd(const void *data, uint8_t size)
{
	const struct bt_hci_cmd_write_page_scan_period_mode *cmd = data;

	print_pscan_period_mode(cmd->mode);
}

static void read_page_scan_mode_rsp(const void *data, uint8_t size)
{
	const struct bt_hci_rsp_read_page_scan_mode *rsp = data;

	print_status(rsp->status);
	print_pscan_mode(rsp->mode);
}

static void write_page_scan_mode_cmd(const void *data, uint8_t size)
{
	const struct bt_hci_cmd_write_page_scan_mode *cmd = data;

	print_pscan_mode(cmd->mode);
}

static void set_afh_host_classification_cmd(const void *data, uint8_t size)
{
	const struct bt_hci_cmd_set_afh_host_classification *cmd = data;

	print_channel_map(cmd->map);
}

static void read_inquiry_scan_type_rsp(const void *data, uint8_t size)
{
	const struct bt_hci_rsp_read_inquiry_scan_type *rsp = data;

	print_status(rsp->status);
	print_inquiry_scan_type(rsp->type);
}

static void write_inquiry_scan_type_cmd(const void *data, uint8_t size)
{
	const struct bt_hci_cmd_write_inquiry_scan_type *cmd = data;

	print_inquiry_scan_type(cmd->type);
}

static void read_inquiry_mode_rsp(const void *data, uint8_t size)
{
	const struct bt_hci_rsp_read_inquiry_mode *rsp = data;

	print_status(rsp->status);
	print_inquiry_mode(rsp->mode);
}

static void write_inquiry_mode_cmd(const void *data, uint8_t size)
{
	const struct bt_hci_cmd_write_inquiry_mode *cmd = data;

	print_inquiry_mode(cmd->mode);
}

static void read_page_scan_type_rsp(const void *data, uint8_t size)
{
	const struct bt_hci_rsp_read_page_scan_type *rsp = data;

	print_status(rsp->status);
	print_pscan_type(rsp->type);
}

static void write_page_scan_type_cmd(const void *data, uint8_t size)
{
	const struct bt_hci_cmd_write_page_scan_type *cmd = data;

	print_pscan_type(cmd->type);
}

static void read_afh_assessment_mode_rsp(const void *data, uint8_t size)
{
	const struct bt_hci_rsp_read_afh_assessment_mode *rsp = data;

	print_status(rsp->status);
	print_enable("Mode", rsp->mode);
}

static void write_afh_assessment_mode_cmd(const void *data, uint8_t size)
{
	const struct bt_hci_cmd_write_afh_assessment_mode *cmd = data;

	print_enable("Mode", cmd->mode);
}

static void read_ext_inquiry_response_rsp(const void *data, uint8_t size)
{
	const struct bt_hci_rsp_read_ext_inquiry_response *rsp = data;

	print_status(rsp->status);
	print_fec(rsp->fec);
	print_eir(rsp->data, sizeof(rsp->data), false);
}

static void write_ext_inquiry_response_cmd(const void *data, uint8_t size)
{
	const struct bt_hci_cmd_write_ext_inquiry_response *cmd = data;

	print_fec(cmd->fec);
	print_eir(cmd->data, sizeof(cmd->data), false);
}

static void refresh_encrypt_key_cmd(const void *data, uint8_t size)
{
	const struct bt_hci_cmd_refresh_encrypt_key *cmd = data;

	print_handle(cmd->handle);
}

static void read_simple_pairing_mode_rsp(const void *data, uint8_t size)
{
	const struct bt_hci_rsp_read_simple_pairing_mode *rsp = data;

	print_status(rsp->status);
	print_enable("Mode", rsp->mode);
}

static void write_simple_pairing_mode_cmd(const void *data, uint8_t size)
{
	const struct bt_hci_cmd_write_simple_pairing_mode *cmd = data;

	print_enable("Mode", cmd->mode);
}

static void read_local_oob_data_rsp(const void *data, uint8_t size)
{
	const struct bt_hci_rsp_read_local_oob_data *rsp = data;

	print_status(rsp->status);
	print_hash_p192(rsp->hash);
	print_randomizer_p192(rsp->randomizer);
}

static void read_inquiry_resp_tx_power_rsp(const void *data, uint8_t size)
{
	const struct bt_hci_rsp_read_inquiry_resp_tx_power *rsp = data;

	print_status(rsp->status);
	print_power_level(rsp->level, NULL);
}

static void write_inquiry_tx_power_cmd(const void *data, uint8_t size)
{
	const struct bt_hci_cmd_write_inquiry_tx_power *cmd = data;

	print_power_level(cmd->level, NULL);
}

static void read_erroneous_reporting_rsp(const void *data, uint8_t size)
{
	const struct bt_hci_rsp_read_erroneous_reporting *rsp = data;

	print_status(rsp->status);
	print_enable("Mode", rsp->mode);
}

static void write_erroneous_reporting_cmd(const void *data, uint8_t size)
{
	const struct bt_hci_cmd_write_erroneous_reporting *cmd = data;

	print_enable("Mode", cmd->mode);
}

static void enhanced_flush_cmd(const void *data, uint8_t size)
{
	const struct bt_hci_cmd_enhanced_flush *cmd = data;
	const char *str;

	print_handle(cmd->handle);

	switch (cmd->type) {
	case 0x00:
		str = "Automatic flushable only";
		break;
	default:
		str = "Reserved";
		break;
	}

	print_field("Type: %s (0x%2.2x)", str, cmd->type);
}

static void send_keypress_notify_cmd(const void *data, uint8_t size)
{
	const struct bt_hci_cmd_send_keypress_notify *cmd = data;
	const char *str;

	print_bdaddr(cmd->bdaddr);

	switch (cmd->type) {
	case 0x00:
		str = "Passkey entry started";
		break;
	case 0x01:
		str = "Passkey digit entered";
		break;
	case 0x02:
		str = "Passkey digit erased";
		break;
	case 0x03:
		str = "Passkey cleared";
		break;
	case 0x04:
		str = "Passkey entry completed";
		break;
	default:
		str = "Reserved";
		break;
	}

	print_field("Type: %s (0x%2.2x)", str, cmd->type);
}

static void send_keypress_notify_rsp(const void *data, uint8_t size)
{
	const struct bt_hci_rsp_send_keypress_notify *rsp = data;

	print_status(rsp->status);
	print_bdaddr(rsp->bdaddr);
}

static void set_event_mask_page2_cmd(const void *data, uint8_t size)
{
	const struct bt_hci_cmd_set_event_mask_page2 *cmd = data;

	print_event_mask(cmd->mask, events_page2_table);
}

static void read_location_data_rsp(const void *data, uint8_t size)
{
	const struct bt_hci_rsp_read_location_data *rsp = data;

	print_status(rsp->status);
	print_location_domain_aware(rsp->domain_aware);
	print_location_domain(rsp->domain);
	print_location_domain_options(rsp->domain_options);
	print_location_options(rsp->options);
}

static void write_location_data_cmd(const void *data, uint8_t size)
{
	const struct bt_hci_cmd_write_location_data *cmd = data;

	print_location_domain_aware(cmd->domain_aware);
	print_location_domain(cmd->domain);
	print_location_domain_options(cmd->domain_options);
	print_location_options(cmd->options);
}

static void read_flow_control_mode_rsp(const void *data, uint8_t size)
{
	const struct bt_hci_rsp_read_flow_control_mode *rsp = data;

	print_status(rsp->status);
	print_flow_control_mode(rsp->mode);
}

static void write_flow_control_mode_cmd(const void *data, uint8_t size)
{
	const struct bt_hci_cmd_write_flow_control_mode *cmd = data;

	print_flow_control_mode(cmd->mode);
}

static void read_enhanced_tx_power_cmd(const void *data, uint8_t size)
{
	const struct bt_hci_cmd_read_enhanced_tx_power *cmd = data;

	print_handle(cmd->handle);
	print_power_type(cmd->type);
}

static void read_enhanced_tx_power_rsp(const void *data, uint8_t size)
{
	const struct bt_hci_rsp_read_enhanced_tx_power *rsp = data;

	print_status(rsp->status);
	print_handle(rsp->handle);
	print_power_level(rsp->level_gfsk, "GFSK");
	print_power_level(rsp->level_dqpsk, "DQPSK");
	print_power_level(rsp->level_8dpsk, "8DPSK");
}

static void short_range_mode_cmd(const void *data, uint8_t size)
{
	const struct bt_hci_cmd_short_range_mode *cmd = data;

	print_phy_handle(cmd->phy_handle);
	print_enable("Short range mode", cmd->mode);
}

static void read_le_host_supported_rsp(const void *data, uint8_t size)
{
	const struct bt_hci_rsp_read_le_host_supported *rsp = data;

	print_status(rsp->status);
	print_field("Supported: 0x%2.2x", rsp->supported);
	print_field("Simultaneous: 0x%2.2x", rsp->simultaneous);
}

static void write_le_host_supported_cmd(const void *data, uint8_t size)
{
	const struct bt_hci_cmd_write_le_host_supported *cmd = data;

	print_field("Supported: 0x%2.2x", cmd->supported);
	print_field("Simultaneous: 0x%2.2x", cmd->simultaneous);
}

static void set_reserved_lt_addr_cmd(const void *data, uint8_t size)
{
	const struct bt_hci_cmd_set_reserved_lt_addr *cmd = data;

	print_lt_addr(cmd->lt_addr);
}

static void set_reserved_lt_addr_rsp(const void *data, uint8_t size)
{
	const struct bt_hci_rsp_set_reserved_lt_addr *rsp = data;

	print_status(rsp->status);
	print_lt_addr(rsp->lt_addr);
}

static void delete_reserved_lt_addr_cmd(const void *data, uint8_t size)
{
	const struct bt_hci_cmd_delete_reserved_lt_addr *cmd = data;

	print_lt_addr(cmd->lt_addr);
}

static void delete_reserved_lt_addr_rsp(const void *data, uint8_t size)
{
	const struct bt_hci_rsp_delete_reserved_lt_addr *rsp = data;

	print_status(rsp->status);
	print_lt_addr(rsp->lt_addr);
}

static void set_slave_broadcast_data_cmd(const void *data, uint8_t size)
{
	const struct bt_hci_cmd_set_slave_broadcast_data *cmd = data;

	print_lt_addr(cmd->lt_addr);
	print_broadcast_fragment(cmd->fragment);
	print_field("Length: %d", cmd->length);

	if (size - 3 != cmd->length)
		print_text(COLOR_ERROR, "invalid data size (%d != %d)",
						size - 3, cmd->length);

	packet_hexdump(data + 3, size - 3);
}

static void set_slave_broadcast_data_rsp(const void *data, uint8_t size)
{
	const struct bt_hci_rsp_set_slave_broadcast_data *rsp = data;

	print_status(rsp->status);
	print_lt_addr(rsp->lt_addr);
}

static void read_sync_train_params_rsp(const void *data, uint8_t size)
{
	const struct bt_hci_rsp_read_sync_train_params *rsp = data;

	print_status(rsp->status);
	print_interval(rsp->interval);
	print_field("Timeout: %.3f msec (0x%8.8x)",
					le32_to_cpu(rsp->timeout) * 0.625,
					le32_to_cpu(rsp->timeout));
	print_field("Service data: 0x%2.2x", rsp->service_data);
}

static void write_sync_train_params_cmd(const void *data, uint8_t size)
{
	const struct bt_hci_cmd_write_sync_train_params *cmd = data;

	print_slot_625("Min interval", cmd->min_interval);
	print_slot_625("Max interval", cmd->max_interval);
	print_field("Timeout: %.3f msec (0x%8.8x)",
					le32_to_cpu(cmd->timeout) * 0.625,
					le32_to_cpu(cmd->timeout));
	print_field("Service data: 0x%2.2x", cmd->service_data);
}

static void write_sync_train_params_rsp(const void *data, uint8_t size)
{
	const struct bt_hci_rsp_write_sync_train_params *rsp = data;

	print_status(rsp->status);
	print_interval(rsp->interval);
}

static void read_secure_conn_support_rsp(const void *data, uint8_t size)
{
	const struct bt_hci_rsp_read_secure_conn_support *rsp = data;

	print_status(rsp->status);
	print_enable("Support", rsp->support);
}

static void write_secure_conn_support_cmd(const void *data, uint8_t size)
{
	const struct bt_hci_cmd_write_secure_conn_support *cmd = data;

	print_enable("Support", cmd->support);
}

static void read_auth_payload_timeout_cmd(const void *data, uint8_t size)
{
	const struct bt_hci_cmd_read_auth_payload_timeout *cmd = data;

	print_handle(cmd->handle);
}

static void read_auth_payload_timeout_rsp(const void *data, uint8_t size)
{
	const struct bt_hci_rsp_read_auth_payload_timeout *rsp = data;

	print_status(rsp->status);
	print_handle(rsp->handle);
	print_auth_payload_timeout(rsp->timeout);
}

static void write_auth_payload_timeout_cmd(const void *data, uint8_t size)
{
	const struct bt_hci_cmd_write_auth_payload_timeout *cmd = data;

	print_handle(cmd->handle);
	print_auth_payload_timeout(cmd->timeout);
}

static void write_auth_payload_timeout_rsp(const void *data, uint8_t size)
{
	const struct bt_hci_rsp_write_auth_payload_timeout *rsp = data;

	print_status(rsp->status);
	print_handle(rsp->handle);
}

static void read_local_oob_ext_data_rsp(const void *data, uint8_t size)
{
	const struct bt_hci_rsp_read_local_oob_ext_data *rsp = data;

	print_status(rsp->status);
	print_hash_p192(rsp->hash192);
	print_randomizer_p192(rsp->randomizer192);
	print_hash_p256(rsp->hash256);
	print_randomizer_p256(rsp->randomizer256);
}

static void read_ext_page_timeout_rsp(const void *data, uint8_t size)
{
	const struct bt_hci_rsp_read_ext_page_timeout *rsp = data;

	print_status(rsp->status);
	print_timeout(rsp->timeout);
}

static void write_ext_page_timeout_cmd(const void *data, uint8_t size)
{
	const struct bt_hci_cmd_write_ext_page_timeout *cmd = data;

	print_timeout(cmd->timeout);
}

static void read_ext_inquiry_length_rsp(const void *data, uint8_t size)
{
	const struct bt_hci_rsp_read_ext_inquiry_length *rsp = data;

	print_status(rsp->status);
	print_interval(rsp->interval);
}

static void write_ext_inquiry_length_cmd(const void *data, uint8_t size)
{
	const struct bt_hci_cmd_write_ext_inquiry_length *cmd = data;

	print_interval(cmd->interval);
}

static void read_local_version_rsp(const void *data, uint8_t size)
{
	const struct bt_hci_rsp_read_local_version *rsp = data;
	uint16_t manufacturer;

	print_status(rsp->status);
	print_hci_version(rsp->hci_ver, rsp->hci_rev);

	manufacturer = le16_to_cpu(rsp->manufacturer);

	if (index_current < MAX_INDEX) {
		switch (index_list[index_current].type) {
		case HCI_PRIMARY:
			print_lmp_version(rsp->lmp_ver, rsp->lmp_subver);
			break;
		case HCI_AMP:
			print_pal_version(rsp->lmp_ver, rsp->lmp_subver);
			break;
		}

		index_list[index_current].manufacturer = manufacturer;
	}

	print_manufacturer(rsp->manufacturer);

	switch (manufacturer) {
	case 15:
		print_manufacturer_broadcom(rsp->lmp_subver, rsp->hci_rev);
		break;
	}
}

static void read_local_commands_rsp(const void *data, uint8_t size)
{
	const struct bt_hci_rsp_read_local_commands *rsp = data;

	print_status(rsp->status);
	print_commands(rsp->commands);
}

static void read_local_features_rsp(const void *data, uint8_t size)
{
	const struct bt_hci_rsp_read_local_features *rsp = data;

	print_status(rsp->status);
	print_features(0, rsp->features, 0x00);
}

static void read_local_ext_features_cmd(const void *data, uint8_t size)
{
	const struct bt_hci_cmd_read_local_ext_features *cmd = data;

	print_field("Page: %d", cmd->page);
}

static void read_local_ext_features_rsp(const void *data, uint8_t size)
{
	const struct bt_hci_rsp_read_local_ext_features *rsp = data;

	print_status(rsp->status);
	print_field("Page: %d/%d", rsp->page, rsp->max_page);
	print_features(rsp->page, rsp->features, 0x00);
}

static void read_buffer_size_rsp(const void *data, uint8_t size)
{
	const struct bt_hci_rsp_read_buffer_size *rsp = data;

	print_status(rsp->status);
	print_field("ACL MTU: %-4d ACL max packet: %d",
					le16_to_cpu(rsp->acl_mtu),
					le16_to_cpu(rsp->acl_max_pkt));
	print_field("SCO MTU: %-4d SCO max packet: %d",
					rsp->sco_mtu,
					le16_to_cpu(rsp->sco_max_pkt));
}

static void read_country_code_rsp(const void *data, uint8_t size)
{
	const struct bt_hci_rsp_read_country_code *rsp = data;
	const char *str;

	print_status(rsp->status);

	switch (rsp->code) {
	case 0x00:
		str = "North America, Europe*, Japan";
		break;
	case 0x01:
		str = "France";
		break;
	default:
		str = "Reserved";
		break;
	}

	print_field("Country code: %s (0x%2.2x)", str, rsp->code);
}

static void read_bd_addr_rsp(const void *data, uint8_t size)
{
	const struct bt_hci_rsp_read_bd_addr *rsp = data;

	print_status(rsp->status);
	print_bdaddr(rsp->bdaddr);

	if (index_current < MAX_INDEX)
		memcpy(index_list[index_current].bdaddr, rsp->bdaddr, 6);
}

static void read_data_block_size_rsp(const void *data, uint8_t size)
{
	const struct bt_hci_rsp_read_data_block_size *rsp = data;

	print_status(rsp->status);
	print_field("Max ACL length: %d", le16_to_cpu(rsp->max_acl_len));
	print_field("Block length: %d", le16_to_cpu(rsp->block_len));
	print_field("Num blocks: %d", le16_to_cpu(rsp->num_blocks));
}

static void read_local_codecs_rsp(const void *data, uint8_t size)
{
	const struct bt_hci_rsp_read_local_codecs *rsp = data;
	uint8_t i, num_vnd_codecs;

	if (rsp->num_codecs + 3 > size) {
		print_field("Invalid number of codecs.");
		return;
	}

	print_status(rsp->status);
	print_field("Number of supported codecs: %d", rsp->num_codecs);

	for (i = 0; i < rsp->num_codecs; i++)
		print_codec("  Codec", rsp->codec[i]);

	num_vnd_codecs = rsp->codec[rsp->num_codecs];

	print_field("Number of vendor codecs: %d", num_vnd_codecs);

	packet_hexdump(data + rsp->num_codecs + 3,
					size - rsp->num_codecs - 3);
}

static void read_local_pairing_options_rsp(const void *data, uint8_t size)
{
	const struct bt_hci_rsp_read_local_pairing_options *rsp = data;

	print_status(rsp->status);
	print_field("Pairing options: 0x%2.2x", rsp->pairing_options);
	print_field("Max encryption key size: %u octets", rsp->max_key_size);
}

static void read_failed_contact_counter_cmd(const void *data, uint8_t size)
{
	const struct bt_hci_cmd_read_failed_contact_counter *cmd = data;

	print_handle(cmd->handle);
}

static void read_failed_contact_counter_rsp(const void *data, uint8_t size)
{
	const struct bt_hci_rsp_read_failed_contact_counter *rsp = data;

	print_status(rsp->status);
	print_handle(rsp->handle);
	print_field("Counter: %u", le16_to_cpu(rsp->counter));
}

static void reset_failed_contact_counter_cmd(const void *data, uint8_t size)
{
	const struct bt_hci_cmd_reset_failed_contact_counter *cmd = data;

	print_handle(cmd->handle);
}

static void reset_failed_contact_counter_rsp(const void *data, uint8_t size)
{
	const struct bt_hci_rsp_reset_failed_contact_counter *rsp = data;

	print_status(rsp->status);
	print_handle(rsp->handle);
}

static void read_link_quality_cmd(const void *data, uint8_t size)
{
	const struct bt_hci_cmd_read_link_quality *cmd = data;

	print_handle(cmd->handle);
}

static void read_link_quality_rsp(const void *data, uint8_t size)
{
	const struct bt_hci_rsp_read_link_quality *rsp = data;

	print_status(rsp->status);
	print_handle(rsp->handle);
	print_field("Link quality: 0x%2.2x", rsp->link_quality);
}

static void read_rssi_cmd(const void *data, uint8_t size)
{
	const struct bt_hci_cmd_read_rssi *cmd = data;

	print_handle(cmd->handle);
}

static void read_rssi_rsp(const void *data, uint8_t size)
{
	const struct bt_hci_rsp_read_rssi *rsp = data;

	print_status(rsp->status);
	print_handle(rsp->handle);
	print_rssi(rsp->rssi);
}

static void read_afh_channel_map_cmd(const void *data, uint8_t size)
{
	const struct bt_hci_cmd_read_afh_channel_map *cmd = data;

	print_handle(cmd->handle);
}

static void read_afh_channel_map_rsp(const void *data, uint8_t size)
{
	const struct bt_hci_rsp_read_afh_channel_map *rsp = data;

	print_status(rsp->status);
	print_handle(rsp->handle);
	print_enable("Mode", rsp->mode);
	print_channel_map(rsp->map);
}

static void read_clock_cmd(const void *data, uint8_t size)
{
	const struct bt_hci_cmd_read_clock *cmd = data;

	print_handle(cmd->handle);
	print_clock_type(cmd->type);
}

static void read_clock_rsp(const void *data, uint8_t size)
{
	const struct bt_hci_rsp_read_clock *rsp = data;

	print_status(rsp->status);
	print_handle(rsp->handle);
	print_clock(rsp->clock);
	print_clock_accuracy(rsp->accuracy);
}

static void read_encrypt_key_size_cmd(const void *data, uint8_t size)
{
	const struct bt_hci_cmd_read_encrypt_key_size *cmd = data;

	print_handle(cmd->handle);
}

static void read_encrypt_key_size_rsp(const void *data, uint8_t size)
{
	const struct bt_hci_rsp_read_encrypt_key_size *rsp = data;

	print_status(rsp->status);
	print_handle(rsp->handle);
	print_key_size(rsp->key_size);
}

static void read_local_amp_info_rsp(const void *data, uint8_t size)
{
	const struct bt_hci_rsp_read_local_amp_info *rsp = data;
	const char *str;

	print_status(rsp->status);
	print_amp_status(rsp->amp_status);

	print_field("Total bandwidth: %d kbps", le32_to_cpu(rsp->total_bw));
	print_field("Max guaranteed bandwidth: %d kbps",
						le32_to_cpu(rsp->max_bw));
	print_field("Min latency: %d", le32_to_cpu(rsp->min_latency));
	print_field("Max PDU size: %d", le32_to_cpu(rsp->max_pdu));

	switch (rsp->amp_type) {
	case 0x00:
		str = "Primary BR/EDR Controller";
		break;
	case 0x01:
		str = "802.11 AMP Controller";
		break;
	default:
		str = "Reserved";
		break;
	}

	print_field("Controller type: %s (0x%2.2x)", str, rsp->amp_type);

	print_field("PAL capabilities: 0x%4.4x", le16_to_cpu(rsp->pal_cap));
	print_field("Max ASSOC length: %d", le16_to_cpu(rsp->max_assoc_len));
	print_field("Max flush timeout: %d", le32_to_cpu(rsp->max_flush_to));
	print_field("Best effort flush timeout: %d",
					le32_to_cpu(rsp->be_flush_to));
}

static void read_local_amp_assoc_cmd(const void *data, uint8_t size)
{
	const struct bt_hci_cmd_read_local_amp_assoc *cmd = data;

	print_phy_handle(cmd->phy_handle);
	print_field("Length so far: %d", le16_to_cpu(cmd->len_so_far));
	print_field("Max ASSOC length: %d", le16_to_cpu(cmd->max_assoc_len));
}

static void read_local_amp_assoc_rsp(const void *data, uint8_t size)
{
	const struct bt_hci_rsp_read_local_amp_assoc *rsp = data;

	print_status(rsp->status);
	print_phy_handle(rsp->phy_handle);
	print_field("Remaining ASSOC length: %d",
					le16_to_cpu(rsp->remain_assoc_len));

	packet_hexdump(data + 4, size - 4);
}

static void write_remote_amp_assoc_cmd(const void *data, uint8_t size)
{
	const struct bt_hci_cmd_write_remote_amp_assoc *cmd = data;

	print_phy_handle(cmd->phy_handle);
	print_field("Length so far: %d", le16_to_cpu(cmd->len_so_far));
	print_field("Remaining ASSOC length: %d",
					le16_to_cpu(cmd->remain_assoc_len));

	packet_hexdump(data + 5, size - 5);
}

static void write_remote_amp_assoc_rsp(const void *data, uint8_t size)
{
	const struct bt_hci_rsp_write_remote_amp_assoc *rsp = data;

	print_status(rsp->status);
	print_phy_handle(rsp->phy_handle);
}

static void get_mws_transport_config_rsp(const void *data, uint8_t size)
{
	const struct bt_hci_rsp_get_mws_transport_config *rsp = data;
	uint8_t sum_baud_rates = 0;
	int i;

	print_status(rsp->status);
	print_field("Number of transports: %d", rsp->num_transports);

	for (i = 0; i < rsp->num_transports; i++) {
		uint8_t transport = rsp->transport[0];
		uint8_t num_baud_rates = rsp->transport[1];
		const char *str;

		switch (transport) {
		case 0x00:
			str = "Disbabled";
			break;
		case 0x01:
			str = "WCI-1";
			break;
		case 0x02:
			str = "WCI-2";
			break;
		default:
			str = "Reserved";
			break;
		}

		print_field("  Transport layer: %s (0x%2.2x)", str, transport);
		print_field("  Number of baud rates: %d", num_baud_rates);

		sum_baud_rates += num_baud_rates;
	}

	print_field("Baud rate list: %u entr%s", sum_baud_rates,
					sum_baud_rates == 1 ? "y" : "ies");

	for (i = 0; i < sum_baud_rates; i++) {
		uint32_t to_baud_rate, from_baud_rate;

		to_baud_rate = get_le32(data + 2 +
					rsp->num_transports * 2 + i * 4);
		from_baud_rate = get_le32(data + 2 +
						rsp->num_transports * 2 +
						sum_baud_rates * 4 + i * 4);

		print_field("  Bluetooth to MWS: %d", to_baud_rate);
		print_field("  MWS to Bluetooth: %d", from_baud_rate);
	}

	packet_hexdump(data + 2 + rsp->num_transports * 2 + sum_baud_rates * 8,
		size - 2 - rsp->num_transports * 2 - sum_baud_rates * 8);
}

static void set_triggered_clock_capture_cmd(const void *data, uint8_t size)
{
	const struct bt_hci_cmd_set_triggered_clock_capture *cmd = data;

	print_handle(cmd->handle);
	print_enable("Capture", cmd->enable);
	print_clock_type(cmd->type);
	print_lpo_allowed(cmd->lpo_allowed);
	print_field("Clock captures to filter: %u", cmd->num_filter);
}

static void read_loopback_mode_rsp(const void *data, uint8_t size)
{
	const struct bt_hci_rsp_read_loopback_mode *rsp = data;

	print_status(rsp->status);
	print_loopback_mode(rsp->mode);
}

static void write_loopback_mode_cmd(const void *data, uint8_t size)
{
	const struct bt_hci_cmd_write_loopback_mode *cmd = data;

	print_loopback_mode(cmd->mode);
}

static void write_ssp_debug_mode_cmd(const void *data, uint8_t size)
{
	const struct bt_hci_cmd_write_ssp_debug_mode *cmd = data;

	print_enable("Debug Mode", cmd->mode);
}

static void le_set_event_mask_cmd(const void *data, uint8_t size)
{
	const struct bt_hci_cmd_le_set_event_mask *cmd = data;

	print_event_mask(cmd->mask, events_le_table);
}

static void le_read_buffer_size_rsp(const void *data, uint8_t size)
{
	const struct bt_hci_rsp_le_read_buffer_size *rsp = data;

	print_status(rsp->status);
	print_field("Data packet length: %d", le16_to_cpu(rsp->le_mtu));
	print_field("Num data packets: %d", rsp->le_max_pkt);
}

static void le_read_local_features_rsp(const void *data, uint8_t size)
{
	const struct bt_hci_rsp_le_read_local_features *rsp = data;

	print_status(rsp->status);
	print_features(0, rsp->features, 0x01);
}

static void le_set_random_address_cmd(const void *data, uint8_t size)
{
	const struct bt_hci_cmd_le_set_random_address *cmd = data;

	print_addr("Address", cmd->addr, 0x01);
}

static void le_set_adv_parameters_cmd(const void *data, uint8_t size)
{
	const struct bt_hci_cmd_le_set_adv_parameters *cmd = data;
	const char *str;

	print_slot_625("Min advertising interval", cmd->min_interval);
	print_slot_625("Max advertising interval", cmd->max_interval);

	switch (cmd->type) {
	case 0x00:
		str = "Connectable undirected - ADV_IND";
		break;
	case 0x01:
		str = "Connectable directed - ADV_DIRECT_IND (high duty cycle)";
		break;
	case 0x02:
		str = "Scannable undirected - ADV_SCAN_IND";
		break;
	case 0x03:
		str = "Non connectable undirected - ADV_NONCONN_IND";
		break;
	case 0x04:
		str = "Connectable directed - ADV_DIRECT_IND (low duty cycle)";
		break;
	default:
		str = "Reserved";
		break;
	}

	print_field("Type: %s (0x%2.2x)", str, cmd->type);

	print_own_addr_type(cmd->own_addr_type);
	print_addr_type("Direct address type", cmd->direct_addr_type);
	print_addr("Direct address", cmd->direct_addr, cmd->direct_addr_type);
	print_adv_channel_map("Channel map", cmd->channel_map);
	print_adv_filter_policy("Filter policy", cmd->filter_policy);
}

static void le_read_adv_tx_power_rsp(const void *data, uint8_t size)
{
	const struct bt_hci_rsp_le_read_adv_tx_power *rsp = data;

	print_status(rsp->status);
	print_power_level(rsp->level, NULL);
}

static void le_set_adv_data_cmd(const void *data, uint8_t size)
{
	const struct bt_hci_cmd_le_set_adv_data *cmd = data;

	print_field("Length: %d", cmd->len);
	print_eir(cmd->data, cmd->len, true);
}

static void le_set_scan_rsp_data_cmd(const void *data, uint8_t size)
{
	const struct bt_hci_cmd_le_set_scan_rsp_data *cmd = data;

	print_field("Length: %d", cmd->len);
	print_eir(cmd->data, cmd->len, true);
}

static void le_set_adv_enable_cmd(const void *data, uint8_t size)
{
	const struct bt_hci_cmd_le_set_adv_enable *cmd = data;

	print_enable("Advertising", cmd->enable);
}

static void print_scan_type(const char *label, uint8_t type)
{
	const char *str;

	switch (type) {
	case 0x00:
		str = "Passive";
		break;
	case 0x01:
		str = "Active";
		break;
	default:
		str = "Reserved";
		break;
	}

	print_field("%s: %s (0x%2.2x)", label, str, type);
}

static void print_scan_filter_policy(uint8_t policy)
{
	const char *str;

	switch (policy) {
	case 0x00:
		str = "Accept all advertisement";
		break;
	case 0x01:
		str = "Ignore not in white list";
		break;
	case 0x02:
		str = "Accept all advertisement, inc. directed unresolved RPA";
		break;
	case 0x03:
		str = "Ignore not in white list, exc. directed unresolved RPA";
		break;
	default:
		str = "Reserved";
		break;
	}

	print_field("Filter policy: %s (0x%2.2x)", str, policy);
}

static void le_set_scan_parameters_cmd(const void *data, uint8_t size)
{
	const struct bt_hci_cmd_le_set_scan_parameters *cmd = data;

	print_scan_type("Type", cmd->type);
	print_interval(cmd->interval);
	print_window(cmd->window);
	print_own_addr_type(cmd->own_addr_type);
	print_scan_filter_policy(cmd->filter_policy);
}

static void le_set_scan_enable_cmd(const void *data, uint8_t size)
{
	const struct bt_hci_cmd_le_set_scan_enable *cmd = data;

	print_enable("Scanning", cmd->enable);
	print_enable("Filter duplicates", cmd->filter_dup);
}

static void le_create_conn_cmd(const void *data, uint8_t size)
{
	const struct bt_hci_cmd_le_create_conn *cmd = data;
	const char *str;

	print_slot_625("Scan interval", cmd->scan_interval);
	print_slot_625("Scan window", cmd->scan_window);

	switch (cmd->filter_policy) {
	case 0x00:
		str = "White list is not used";
		break;
	case 0x01:
		str = "White list is used";
		break;
	default:
		str = "Reserved";
		break;
	}

	print_field("Filter policy: %s (0x%2.2x)", str, cmd->filter_policy);

	print_peer_addr_type("Peer address type", cmd->peer_addr_type);
	print_addr("Peer address", cmd->peer_addr, cmd->peer_addr_type);
	print_own_addr_type(cmd->own_addr_type);

	print_slot_125("Min connection interval", cmd->min_interval);
	print_slot_125("Max connection interval", cmd->max_interval);
	print_conn_latency("Connection latency", cmd->latency);
	print_field("Supervision timeout: %d msec (0x%4.4x)",
					le16_to_cpu(cmd->supv_timeout) * 10,
					le16_to_cpu(cmd->supv_timeout));
	print_slot_625("Min connection length", cmd->min_length);
	print_slot_625("Max connection length", cmd->max_length);
}

static void le_read_white_list_size_rsp(const void *data, uint8_t size)
{
	const struct bt_hci_rsp_le_read_white_list_size *rsp = data;

	print_status(rsp->status);
	print_field("Size: %u", rsp->size);
}

static void le_add_to_white_list_cmd(const void *data, uint8_t size)
{
	const struct bt_hci_cmd_le_add_to_white_list *cmd = data;

	print_addr_type("Address type", cmd->addr_type);
	print_addr("Address", cmd->addr, cmd->addr_type);
}

static void le_remove_from_white_list_cmd(const void *data, uint8_t size)
{
	const struct bt_hci_cmd_le_remove_from_white_list *cmd = data;

	print_addr_type("Address type", cmd->addr_type);
	print_addr("Address", cmd->addr, cmd->addr_type);
}

static void le_conn_update_cmd(const void *data, uint8_t size)
{
	const struct bt_hci_cmd_le_conn_update *cmd = data;

	print_handle(cmd->handle);
	print_slot_125("Min connection interval", cmd->min_interval);
	print_slot_125("Max connection interval", cmd->max_interval);
	print_conn_latency("Connection latency", cmd->latency);
	print_field("Supervision timeout: %d msec (0x%4.4x)",
					le16_to_cpu(cmd->supv_timeout) * 10,
					le16_to_cpu(cmd->supv_timeout));
	print_slot_625("Min connection length", cmd->min_length);
	print_slot_625("Max connection length", cmd->max_length);
}

static void le_set_host_classification_cmd(const void *data, uint8_t size)
{
	const struct bt_hci_cmd_le_set_host_classification *cmd = data;

	print_le_channel_map(cmd->map);
}

static void le_read_channel_map_cmd(const void *data, uint8_t size)
{
	const struct bt_hci_cmd_le_read_channel_map *cmd = data;

	print_handle(cmd->handle);
}

static void le_read_channel_map_rsp(const void *data, uint8_t size)
{
	const struct bt_hci_rsp_le_read_channel_map *rsp = data;

	print_status(rsp->status);
	print_handle(rsp->handle);
	print_le_channel_map(rsp->map);
}

static void le_read_remote_features_cmd(const void *data, uint8_t size)
{
	const struct bt_hci_cmd_le_read_remote_features *cmd = data;

	print_handle(cmd->handle);
}

static void le_encrypt_cmd(const void *data, uint8_t size)
{
	const struct bt_hci_cmd_le_encrypt *cmd = data;

	print_key("Key", cmd->key);
	print_key("Plaintext data", cmd->plaintext);
}

static void le_encrypt_rsp(const void *data, uint8_t size)
{
	const struct bt_hci_rsp_le_encrypt *rsp = data;

	print_status(rsp->status);
	print_key("Encrypted data", rsp->data);
}

static void le_rand_rsp(const void *data, uint8_t size)
{
	const struct bt_hci_rsp_le_rand *rsp = data;

	print_status(rsp->status);
	print_random_number(rsp->number);
}

static void le_start_encrypt_cmd(const void *data, uint8_t size)
{
	const struct bt_hci_cmd_le_start_encrypt *cmd = data;

	print_handle(cmd->handle);
	print_random_number(cmd->rand);
	print_encrypted_diversifier(cmd->ediv);
	print_key("Long term key", cmd->ltk);
}

static void le_ltk_req_reply_cmd(const void *data, uint8_t size)
{
	const struct bt_hci_cmd_le_ltk_req_reply *cmd = data;

	print_handle(cmd->handle);
	print_key("Long term key", cmd->ltk);
}

static void le_ltk_req_reply_rsp(const void *data, uint8_t size)
{
	const struct bt_hci_rsp_le_ltk_req_reply *rsp = data;

	print_status(rsp->status);
	print_handle(rsp->handle);
}

static void le_ltk_req_neg_reply_cmd(const void *data, uint8_t size)
{
	const struct bt_hci_cmd_le_ltk_req_neg_reply *cmd = data;

	print_handle(cmd->handle);
}

static void le_ltk_req_neg_reply_rsp(const void *data, uint8_t size)
{
	const struct bt_hci_rsp_le_ltk_req_neg_reply *rsp = data;

	print_status(rsp->status);
	print_handle(rsp->handle);
}

static void le_read_supported_states_rsp(const void *data, uint8_t size)
{
	const struct bt_hci_rsp_le_read_supported_states *rsp = data;

	print_status(rsp->status);
	print_le_states(rsp->states);
}

static void le_receiver_test_cmd(const void *data, uint8_t size)
{
	const struct bt_hci_cmd_le_receiver_test *cmd = data;

	print_field("RX frequency: %d MHz (0x%2.2x)",
				(cmd->frequency * 2) + 2402, cmd->frequency);
}

static void le_transmitter_test_cmd(const void *data, uint8_t size)
{
	const struct bt_hci_cmd_le_transmitter_test *cmd = data;

	print_field("TX frequency: %d MHz (0x%2.2x)",
				(cmd->frequency * 2) + 2402, cmd->frequency);
	print_field("Test data length: %d bytes", cmd->data_len);
	print_field("Packet payload: 0x%2.2x", cmd->payload);
}

static void le_test_end_rsp(const void *data, uint8_t size)
{
	const struct bt_hci_rsp_le_test_end *rsp = data;

	print_status(rsp->status);
	print_field("Number of packets: %d", le16_to_cpu(rsp->num_packets));
}

static void le_conn_param_req_reply_cmd(const void *data, uint8_t size)
{
	const struct bt_hci_cmd_le_conn_param_req_reply *cmd = data;

	print_handle(cmd->handle);
	print_slot_125("Min connection interval", cmd->min_interval);
	print_slot_125("Max connection interval", cmd->max_interval);
	print_conn_latency("Connection latency", cmd->latency);
	print_field("Supervision timeout: %d msec (0x%4.4x)",
					le16_to_cpu(cmd->supv_timeout) * 10,
					le16_to_cpu(cmd->supv_timeout));
	print_slot_625("Min connection length", cmd->min_length);
	print_slot_625("Max connection length", cmd->max_length);
}

static void le_conn_param_req_reply_rsp(const void *data, uint8_t size)
{
	const struct bt_hci_rsp_le_conn_param_req_reply *rsp = data;

	print_status(rsp->status);
	print_handle(rsp->handle);
}

static void le_conn_param_req_neg_reply_cmd(const void *data, uint8_t size)
{
	const struct bt_hci_cmd_le_conn_param_req_neg_reply *cmd = data;

	print_handle(cmd->handle);
	print_reason(cmd->reason);
}

static void le_conn_param_req_neg_reply_rsp(const void *data, uint8_t size)
{
	const struct bt_hci_rsp_le_conn_param_req_neg_reply *rsp = data;

	print_status(rsp->status);
	print_handle(rsp->handle);
}

static void le_set_data_length_cmd(const void *data, uint8_t size)
{
	const struct bt_hci_cmd_le_set_data_length *cmd = data;

	print_handle(cmd->handle);
	print_field("TX octets: %d", le16_to_cpu(cmd->tx_len));
	print_field("TX time: %d", le16_to_cpu(cmd->tx_time));
}

static void le_set_data_length_rsp(const void *data, uint8_t size)
{
	const struct bt_hci_rsp_le_set_data_length *rsp = data;

	print_status(rsp->status);
	print_handle(rsp->handle);
}

static void le_read_default_data_length_rsp(const void *data, uint8_t size)
{
	const struct bt_hci_rsp_le_read_default_data_length *rsp = data;

	print_status(rsp->status);
	print_field("TX octets: %d", le16_to_cpu(rsp->tx_len));
	print_field("TX time: %d", le16_to_cpu(rsp->tx_time));
}

static void le_write_default_data_length_cmd(const void *data, uint8_t size)
{
	const struct bt_hci_cmd_le_write_default_data_length *cmd = data;

	print_field("TX octets: %d", le16_to_cpu(cmd->tx_len));
	print_field("TX time: %d", le16_to_cpu(cmd->tx_time));
}

static void le_generate_dhkey_cmd(const void *data, uint8_t size)
{
	const struct bt_hci_cmd_le_generate_dhkey *cmd = data;

	print_pk256("Remote P-256 public key", cmd->remote_pk256);
}

static void le_add_to_resolv_list_cmd(const void *data, uint8_t size)
{
	const struct bt_hci_cmd_le_add_to_resolv_list *cmd = data;

	print_addr_type("Address type", cmd->addr_type);
	print_addr("Address", cmd->addr, cmd->addr_type);
	print_key("Peer identity resolving key", cmd->peer_irk);
	print_key("Local identity resolving key", cmd->local_irk);
}

static void le_remove_from_resolv_list_cmd(const void *data, uint8_t size)
{
	const struct bt_hci_cmd_le_remove_from_resolv_list *cmd = data;

	print_addr_type("Address type", cmd->addr_type);
	print_addr("Address", cmd->addr, cmd->addr_type);
}

static void le_read_resolv_list_size_rsp(const void *data, uint8_t size)
{
	const struct bt_hci_rsp_le_read_resolv_list_size *rsp = data;

	print_status(rsp->status);
	print_field("Size: %u", rsp->size);
}

static void le_read_peer_resolv_addr_cmd(const void *data, uint8_t size)
{
	const struct bt_hci_cmd_le_read_peer_resolv_addr *cmd = data;

	print_addr_type("Address type", cmd->addr_type);
	print_addr("Address", cmd->addr, cmd->addr_type);
}

static void le_read_peer_resolv_addr_rsp(const void *data, uint8_t size)
{
	const struct bt_hci_rsp_le_read_peer_resolv_addr *rsp = data;

	print_status(rsp->status);
	print_addr("Address", rsp->addr, 0x01);
}

static void le_read_local_resolv_addr_cmd(const void *data, uint8_t size)
{
	const struct bt_hci_cmd_le_read_local_resolv_addr *cmd = data;

	print_addr_type("Address type", cmd->addr_type);
	print_addr("Address", cmd->addr, cmd->addr_type);
}

static void le_read_local_resolv_addr_rsp(const void *data, uint8_t size)
{
	const struct bt_hci_rsp_le_read_local_resolv_addr *rsp = data;

	print_status(rsp->status);
	print_addr("Address", rsp->addr, 0x01);
}

static void le_set_resolv_enable_cmd(const void *data, uint8_t size)
{
	const struct bt_hci_cmd_le_set_resolv_enable *cmd = data;

	print_enable("Address resolution", cmd->enable);
}

static void le_set_resolv_timeout_cmd(const void *data, uint8_t size)
{
	const struct bt_hci_cmd_le_set_resolv_timeout *cmd = data;

	print_field("Timeout: %u seconds", le16_to_cpu(cmd->timeout));
}

static void le_read_max_data_length_rsp(const void *data, uint8_t size)
{
	const struct bt_hci_rsp_le_read_max_data_length *rsp = data;

	print_status(rsp->status);
	print_field("Max TX octets: %d", le16_to_cpu(rsp->max_tx_len));
	print_field("Max TX time: %d", le16_to_cpu(rsp->max_tx_time));
	print_field("Max RX octets: %d", le16_to_cpu(rsp->max_rx_len));
	print_field("Max RX time: %d", le16_to_cpu(rsp->max_rx_time));
}

static void le_read_phy_cmd(const void *data, uint8_t size)
{
	const struct bt_hci_cmd_le_read_phy *cmd = data;

	print_handle(cmd->handle);
}

static void print_le_phy(const char *prefix, uint8_t phy)
{
	const char *str;

	switch (phy) {
	case 0x01:
		str = "LE 1M";
		break;
	case 0x02:
		str = "LE 2M";
		break;
	case 0x03:
		str = "LE Coded";
		break;
	default:
		str = "Reserved";
		break;
	}

	print_field("%s: %s (0x%2.2x)", prefix, str, phy);
}

static void le_read_phy_rsp(const void *data, uint8_t size)
{
	const struct bt_hci_rsp_le_read_phy *rsp = data;

	print_status(rsp->status);
	print_handle(rsp->handle);
	print_le_phy("TX PHY", rsp->tx_phy);
	print_le_phy("RX PHY", rsp->rx_phy);
}

static const struct bitfield_data le_phys[] = {
	{  0, "LE 1M"	},
	{  1, "LE 2M"	},
	{  2, "LE Coded"},
	{ }
};

static const struct bitfield_data le_phy_preference[] = {
	{  0, "No TX PHY preference"	},
	{  1, "No RX PHY preference"	},
	{ }
};

static void print_le_phys_preference(uint8_t all_phys, uint8_t tx_phys,
							uint8_t rx_phys)
{
	uint8_t mask;

	print_field("All PHYs preference: 0x%2.2x", all_phys);

	mask = print_bitfield(2, all_phys, le_phy_preference);
	if (mask)
		print_text(COLOR_UNKNOWN_OPTIONS_BIT, "  Reserved"
							" (0x%2.2x)", mask);

	print_field("TX PHYs preference: 0x%2.2x", tx_phys);
	mask = tx_phys;

	mask = print_bitfield(2, tx_phys, le_phys);
	if (mask)
		print_text(COLOR_UNKNOWN_OPTIONS_BIT, "  Reserved"
							" (0x%2.2x)", mask);

	print_field("RX PHYs preference: 0x%2.2x", rx_phys);
	mask = rx_phys;

	mask = print_bitfield(2, rx_phys, le_phys);
	if (mask)
		print_text(COLOR_UNKNOWN_OPTIONS_BIT, "  Reserved"
							" (0x%2.2x)", mask);
}

static void le_set_default_phy_cmd(const void *data, uint8_t size)
{
	const struct bt_hci_cmd_le_set_default_phy *cmd = data;

	print_le_phys_preference(cmd->all_phys, cmd->tx_phys, cmd->rx_phys);
}

static void le_set_phy_cmd(const void *data, uint8_t size)
{
	const struct bt_hci_cmd_le_set_phy *cmd = data;
	const char *str;

	print_handle(cmd->handle);
	print_le_phys_preference(cmd->all_phys, cmd->tx_phys, cmd->rx_phys);
	switch (le16_to_cpu(cmd->phy_opts)) {
	case 0x0001:
		str = "S2 coding";
		break;
	case 0x0002:
		str = "S8 coding";
		break;
	default:
		str = "Reserved";
		break;
	}

	print_field("PHY options preference: %s (0x%4.4x)", str, cmd->phy_opts);
}

static void le_enhanced_receiver_test_cmd(const void *data, uint8_t size)
{
	const struct bt_hci_cmd_le_enhanced_receiver_test *cmd = data;
	const char *str;

	print_field("RX channel frequency: %d MHz (0x%2.2x)",
				(cmd->rx_channel * 2) + 2402, cmd->rx_channel);
	print_le_phy("PHY", cmd->phy);

	switch (cmd->modulation_index) {
	case 0x00:
		str = "Standard";
		break;
	case 0x01:
		str = "Stable";
		break;
	default:
		str = "Reserved";
		break;
	}

	print_field("Modulation index: %s (0x%2.2x)", str,
							cmd->modulation_index);
}

static void le_enhanced_transmitter_test_cmd(const void *data, uint8_t size)
{
	const struct bt_hci_cmd_le_enhanced_transmitter_test *cmd = data;
	const char *str;

	print_field("TX channel frequency: %d MHz (0x%2.2x)",
				(cmd->tx_channel * 2) + 2402, cmd->tx_channel);
	print_field("Test data length: %d bytes", cmd->data_len);
	print_field("Packet payload: 0x%2.2x", cmd->payload);

	switch (cmd->phy) {
	case 0x01:
		str = "LE 1M";
		break;
	case 0x02:
		str = "LE 2M";
		break;
	case 0x03:
		str = "LE Coded with S=8";
		break;
	case 0x04:
		str = "LE Coded with S=2";
		break;
	default:
		str = "Reserved";
		break;
	}

	print_field("PHY: %s (0x%2.2x)", str, cmd->phy);
}

static void le_set_adv_set_rand_addr(const void *data, uint8_t size)
{
	const struct bt_hci_cmd_le_set_adv_set_rand_addr *cmd = data;

	print_field("Advertising handle: 0x%2.2x", cmd->handle);
	print_addr("Advertising random address", cmd->bdaddr, 0x00);
}

static const struct bitfield_data ext_adv_properties_table[] = {
	{  0, "Connectable"		},
	{  1, "Scannable"		},
	{  2, "Directed"	},
	{  3, "High Duty Cycle Directed Connectable"	},
	{  4, "Use legacy advertising PDUs"	},
	{  5, "Anonymous advertising"	},
	{  6, "Include TxPower"		},
	{ }
};

static const char *get_adv_pdu_desc(uint16_t flags)
{
	const char *str;

	switch (flags) {
	case 0x10:
		str = "ADV_NONCONN_IND";
		break;
	case 0x12:
		str = "ADV_SCAN_IND";
		break;
	case 0x13:
		str = "ADV_IND";
		break;
	case 0x15:
		str = "ADV_DIRECT_IND (low duty cycle)";
		break;
	case 0x1d:
		str = "ADV_DIRECT_IND (high duty cycle)";
		break;
	default:
		str = "Reserved";
		break;
	}

	return str;
}

static void print_ext_adv_properties(uint16_t flags)
{
	uint16_t mask = flags;
	const char *property;
	int i;

	print_field("Properties: 0x%4.4x", flags);

	for (i = 0; ext_adv_properties_table[i].str; i++) {
		if (flags & (1 << ext_adv_properties_table[i].bit)) {
			property = ext_adv_properties_table[i].str;

			if (ext_adv_properties_table[i].bit == 4) {
				print_field("  %s: %s", property,
						get_adv_pdu_desc(flags));
			} else {
				print_field("  %s", property);
			}
			mask &= ~(1 << ext_adv_properties_table[i].bit);
		}
	}

	if (mask)
		print_text(COLOR_UNKNOWN_ADV_FLAG,
				"  Unknown advertising properties (0x%4.4x)",
									mask);
}

static void print_ext_slot_625(const char *label, const uint8_t value[3])
{
	uint32_t value_cpu = value[0];

	value_cpu |= value[1] << 8;
	value_cpu |= value[2] << 16;

	print_field("%s: %.3f msec (0x%4.4x)", label,
						value_cpu * 0.625, value_cpu);
}

static void le_set_ext_adv_params_cmd(const void *data, uint8_t size)
{
	const struct bt_hci_cmd_le_set_ext_adv_params *cmd = data;
	const char *str;

	print_field("Handle: 0x%2.2x", cmd->handle);
	print_ext_adv_properties(le16_to_cpu(cmd->evt_properties));

	print_ext_slot_625("Min advertising interval", cmd->min_interval);
	print_ext_slot_625("Max advertising interval", cmd->max_interval);
	print_adv_channel_map("Channel map", cmd->channel_map);
	print_own_addr_type(cmd->own_addr_type);
	print_peer_addr_type("Peer address type", cmd->peer_addr_type);
	print_addr("Peer address", cmd->peer_addr, cmd->peer_addr_type);
	print_adv_filter_policy("Filter policy", cmd->filter_policy);
	if (cmd->tx_power == 0x7f)
		print_field("TX power: Host has no preference (0x7f)");
	else
		print_power_level(cmd->tx_power, NULL);

	switch (cmd->primary_phy) {
	case 0x01:
		str = "LE 1M";
		break;
	case 0x03:
		str = "LE Coded";
		break;
	default:
		str = "Reserved";
		break;
	}

	print_field("Primary PHY: %s (0x%2.2x)", str, cmd->primary_phy);
	print_field("Secondary max skip: 0x%2.2x", cmd->secondary_max_skip);
	print_le_phy("Secondary PHY", cmd->secondary_phy);
	print_field("SID: 0x%2.2x", cmd->sid);
	print_enable("Scan request notifications", cmd->notif_enable);
}

static void le_set_ext_adv_params_rsp(const void *data, uint8_t size)
{
	const struct bt_hci_rsp_le_set_ext_adv_params *rsp = data;

	print_status(rsp->status);
	print_power_level(rsp->tx_power, "selected");
}

static void le_set_ext_adv_data_cmd(const void *data, uint8_t size)
{
	const struct bt_hci_cmd_le_set_ext_adv_data *cmd = data;
	const char *str;

	print_field("Handle: 0x%2.2x", cmd->handle);

	switch (cmd->operation) {
	case 0x00:
		str = "Immediate fragment";
		break;
	case 0x01:
		str = "First fragment";
		break;
	case 0x02:
		str = "Last fragment";
		break;
	case 0x03:
		str = "Complete extended advertising data";
		break;
	case 0x04:
		str = "Unchanged data";
		break;
	default:
		str = "Reserved";
		break;
	}

	print_field("Operation: %s (0x%2.2x)", str, cmd->operation);

	switch (cmd->fragment_preference) {
	case 0x00:
		str = "Fragment all";
		break;
	case 0x01:
		str = "Minimize fragmentation";
		break;
	default:
		str = "Reserved";
		break;
	}

	print_field("Fragment preference: %s (0x%2.2x)", str,
						cmd->fragment_preference);
	print_field("Data length: 0x%2.2x", cmd->data_len);
	packet_print_ad(cmd->data, size - sizeof(*cmd));
}

static void le_set_ext_scan_rsp_data_cmd(const void *data, uint8_t size)
{
	const struct bt_hci_cmd_le_set_ext_scan_rsp_data *cmd = data;
	const char *str;

	print_field("Handle: 0x%2.2x", cmd->handle);

	switch (cmd->operation) {
	case 0x00:
		str = "Immediate fragment";
		break;
	case 0x01:
		str = "First fragment";
		break;
	case 0x02:
		str = "Last fragment";
		break;
	case 0x03:
		str = "Complete scan response data";
		break;
	case 0x04:
		str = "Unchanged data";
		break;
	default:
		str = "Reserved";
		break;
	}

	print_field("Operation: %s (0x%2.2x)", str, cmd->operation);

	switch (cmd->fragment_preference) {
	case 0x00:
		str = "Fragment all";
		break;
	case 0x01:
		str = "Minimize fragmentation";
		break;
	default:
		str = "Reserved";
		break;
	}

	print_field("Fragment preference: %s (0x%2.2x)", str,
						cmd->fragment_preference);
	print_field("Data length: 0x%2.2x", cmd->data_len);
	packet_print_ad(cmd->data, size - sizeof(*cmd));
}

static void le_set_ext_adv_enable_cmd(const void *data, uint8_t size)
{
	const struct bt_hci_cmd_le_set_ext_adv_enable *cmd = data;
	const struct bt_hci_cmd_ext_adv_set *adv_set;
	int i;

	print_enable("Extended advertising", cmd->enable);

	if (cmd->num_of_sets == 0)
		print_field("Number of sets: Disable all sets (0x%2.2x)",
							cmd->num_of_sets);
	else if (cmd->num_of_sets > 0x3f)
		print_field("Number of sets: Reserved (0x%2.2x)",
							cmd->num_of_sets);
	else
		print_field("Number of sets: %u (0x%2.2x)", cmd->num_of_sets,
							cmd->num_of_sets);

	for (i = 0; i < cmd->num_of_sets; ++i) {
		adv_set = data + 2 + i * sizeof(struct bt_hci_cmd_ext_adv_set);
		print_field("Entry %d", i);
		print_field("  Handle: 0x%2.2x", adv_set->handle);
		print_field("  Duration: %d ms (0x%2.2x)",
				adv_set->duration * 10, adv_set->duration);
		print_field("  Max ext adv events: %d", adv_set->max_events);
	}
}

static void le_read_max_adv_data_len_rsp(const void *data, uint8_t size)
{
	const struct bt_hci_rsp_le_read_max_adv_data_len *rsp = data;

	print_status(rsp->status);
	print_field("Max length: %d", rsp->max_len);
}

static void le_read_num_supported_adv_sets_rsp(const void *data, uint8_t size)
{
	const struct bt_hci_rsp_le_read_num_supported_adv_sets *rsp = data;

	print_status(rsp->status);
	print_field("Num supported adv sets: %d", rsp->num_of_sets);
}

static void le_remove_adv_set_cmd(const void *data, uint8_t size)
{
	const struct bt_hci_cmd_le_remove_adv_set *cmd = data;

	print_handle(cmd->handle);
}

static const struct bitfield_data periodic_adv_properties_table[] = {
	{  6, "Include TxPower"		},
	{ }
};

static void print_periodic_adv_properties(uint16_t flags)
{
	uint16_t mask;

	print_field("Properties: 0x%4.4x", flags);

	mask = print_bitfield(2, flags, periodic_adv_properties_table);
	if (mask)
		print_text(COLOR_UNKNOWN_ADV_FLAG,
				"  Unknown advertising properties (0x%4.4x)",
									mask);
}

static void le_set_periodic_adv_params_cmd(const void *data, uint8_t size)
{
	const struct bt_hci_cmd_le_set_periodic_adv_params *cmd = data;

	print_handle(cmd->handle);
	print_slot_125("Min interval", cmd->min_interval);
	print_slot_125("Max interval", cmd->max_interval);
	print_periodic_adv_properties(cmd->properties);
}

static void le_set_periodic_adv_data_cmd(const void *data, uint8_t size)
{
	const struct bt_hci_cmd_le_set_periodic_adv_data *cmd = data;
	const char *str;

	print_handle(cmd->handle);

	switch (cmd->operation) {
	case 0x00:
		str = "Immediate fragment";
		break;
	case 0x01:
		str = "First fragment";
		break;
	case 0x02:
		str = "Last fragment";
		break;
	case 0x03:
		str = "Complete ext advertising data";
		break;
	default:
		str = "Reserved";
		break;
	}

	print_field("Operation: %s (0x%2.2x)", str, cmd->operation);
	print_field("Data length: 0x%2.2x", cmd->data_len);
	print_eir(cmd->data, cmd->data_len, true);
}

static void le_set_periodic_adv_enable_cmd(const void *data, uint8_t size)
{
	const struct bt_hci_cmd_le_set_periodic_adv_enable *cmd = data;

	print_enable("Periodic advertising", cmd->enable);
	print_handle(cmd->handle);
}

static const struct bitfield_data ext_scan_phys_table[] = {
	{  0, "LE 1M"		},
	{  2, "LE Coded"		},
	{ }
};

static void print_ext_scan_phys(const void *data, uint8_t flags)
{
	const struct bt_hci_le_scan_phy *scan_phy;
	uint8_t mask = flags;
	int bits_set = 0;
	int i;

	print_field("PHYs: 0x%2.2x", flags);

	for (i = 0; ext_scan_phys_table[i].str; i++) {
		if (flags & (1 << ext_scan_phys_table[i].bit)) {
			scan_phy = data + bits_set * sizeof(*scan_phy);
			mask &= ~(1 << ext_scan_phys_table[i].bit);

			print_field("Entry %d: %s", bits_set,
						ext_scan_phys_table[i].str);
			print_scan_type("  Type", scan_phy->type);
			print_slot_625("  Interval", scan_phy->interval);
			print_slot_625("  Window", scan_phy->window);

			++bits_set;
		}
	}

	if (mask)
		print_text(COLOR_UNKNOWN_ADV_FLAG, "  Unknown scanning PHYs"
							" (0x%2.2x)", mask);
}

static void le_set_ext_scan_params_cmd(const void *data, uint8_t size)
{
	const struct bt_hci_cmd_le_set_ext_scan_params *cmd = data;

	print_own_addr_type(cmd->own_addr_type);
	print_scan_filter_policy(cmd->filter_policy);
	print_ext_scan_phys(cmd->data, cmd->num_phys);
}

static void le_set_ext_scan_enable_cmd(const void *data, uint8_t size)
{
	const struct bt_hci_cmd_le_set_ext_scan_enable *cmd = data;

	print_enable("Extended scan", cmd->enable);
	print_enable("Filter duplicates", cmd->filter_dup);

	print_field("Duration: %d msec (0x%4.4x)",
						le16_to_cpu(cmd->duration) * 10,
						le16_to_cpu(cmd->duration));
	print_field("Period: %.2f sec (0x%4.4x)",
						le16_to_cpu(cmd->period) * 1.28,
						le16_to_cpu(cmd->period));
}

static const struct bitfield_data ext_conn_phys_table[] = {
	{  0, "LE 1M"		},
	{  1, "LE 2M"		},
	{  2, "LE Coded"	},
	{ }
};

static void print_ext_conn_phys(const void *data, uint8_t flags)
{
	const struct bt_hci_le_ext_create_conn *entry;
	uint8_t mask = flags;
	int bits_set = 0;
	int i;

	print_field("Initiating PHYs: 0x%2.2x", flags);

	for (i = 0; ext_conn_phys_table[i].str; i++) {
		if (flags & (1 << ext_conn_phys_table[i].bit)) {
			entry = data + bits_set * sizeof(*entry);
			mask &= ~(1 << ext_conn_phys_table[i].bit);

			print_field("Entry %d: %s", bits_set,
						ext_conn_phys_table[i].str);
			print_slot_625("  Scan interval", entry->scan_interval);
			print_slot_625("  Scan window", entry->scan_window);
			print_slot_125("  Min connection interval",
							entry->min_interval);
			print_slot_125("  Max connection interval",
							entry->max_interval);
			print_conn_latency("  Connection latency",
								entry->latency);
			print_field("  Supervision timeout: %d msec (0x%4.4x)",
					le16_to_cpu(entry->supv_timeout) * 10,
					le16_to_cpu(entry->supv_timeout));
			print_slot_625("  Min connection length",
							entry->min_length);
			print_slot_625("  Max connection length",
							entry->max_length);

			++bits_set;
		}
	}

	if (mask)
		print_text(COLOR_UNKNOWN_ADV_FLAG, "  Unknown scanning PHYs"
							" (0x%2.2x)", mask);
}

static void le_ext_create_conn_cmd(const void *data, uint8_t size)
{
	const struct bt_hci_cmd_le_ext_create_conn *cmd = data;
	const char *str;

	switch (cmd->filter_policy) {
	case 0x00:
		str = "White list is not used";
		break;
	case 0x01:
		str = "White list is used";
		break;
	default:
		str = "Reserved";
		break;
	}

	print_field("Filter policy: %s (0x%2.2x)", str, cmd->filter_policy);

	print_own_addr_type(cmd->own_addr_type);
	print_peer_addr_type("Peer address type", cmd->peer_addr_type);
	print_addr("Peer address", cmd->peer_addr, cmd->peer_addr_type);
	print_ext_conn_phys(cmd->data, cmd->phys);
}

static const struct bitfield_data create_sync_cte_type[] = {
	{  0, "Do not sync to packets with AoA CTE"	},
	{  1, "Do not sync to packets with AoD CTE 1us"	},
	{  2, "Do not sync to packets with AoD CTE 2us"	},
	{  3, "Do not sync to packets with type 3 AoD"	},
	{  4, "Do not sync to packets without CTE"	},
	{ },
};

static const struct bitfield_data create_sync_options[] = {
	{  0, "Use Periodic Advertiser List"	},
	{  1, "Reporting initially disabled"	},
	{ },
};

static const struct bitfield_data create_sync_options_alt[] = {
	{  0, "Use advertising SID, Advertiser Address Type and address"},
	{  1, "Reporting initially enabled"				},
	{ },
};

static void print_create_sync_cte_type(uint8_t flags)
{
	uint8_t mask = flags;

	print_field("Sync CTE type: 0x%4.4x", flags);

	mask = print_bitfield(2, flags, create_sync_cte_type);

	if (mask) {
		print_text(COLOR_UNKNOWN_ADV_FLAG,
				"Unknown sync CTE type properties (0x%4.4x)",
									mask);
	}
}

static void print_create_sync_options(uint8_t flags)
{
	uint8_t mask = flags;
	int i;

	print_field("Options: 0x%4.4x", flags);

	for (i = 0; create_sync_options[i].str; i++) {
		if (flags & (1 << create_sync_options[i].bit)) {
			print_field("%s", create_sync_options[i].str);
			mask  &= ~(1 << create_sync_options[i].bit);
		} else {
			print_field("%s", create_sync_options_alt[i].str);
			mask  &= ~(1 << create_sync_options_alt[i].bit);
		}
	}

	if (mask) {
		print_text(COLOR_UNKNOWN_ADV_FLAG,
					"  Unknown options (0x%4.4x)", mask);
	}
}

static void le_periodic_adv_create_sync_cmd(const void *data, uint8_t size)
{
	const struct bt_hci_cmd_le_periodic_adv_create_sync *cmd = data;

	print_create_sync_options(cmd->options);
	print_field("SID: 0x%2.2x", cmd->sid);
	print_addr_type("Adv address type", cmd->addr_type);
	print_addr("Adv address", cmd->addr, cmd->addr_type);
	print_field("Skip: 0x%4.4x", cmd->skip);
	print_field("Sync timeout: %d msec (0x%4.4x)",
					le16_to_cpu(cmd->sync_timeout) * 10,
					le16_to_cpu(cmd->sync_timeout));
	print_create_sync_cte_type(cmd->sync_cte_type);
}

static void le_periodic_adv_term_sync_cmd(const void *data, uint8_t size)
{
	const struct bt_hci_cmd_le_periodic_adv_term_sync *cmd = data;

	print_field("Sync handle: 0x%4.4x", cmd->sync_handle);
}

static void le_add_dev_periodic_adv_list_cmd(const void *data, uint8_t size)
{
	const struct bt_hci_cmd_le_add_dev_periodic_adv_list *cmd = data;

	print_addr_type("Adv address type", cmd->addr_type);
	print_addr("Adv address", cmd->addr, cmd->addr_type);
	print_field("SID: 0x%2.2x", cmd->sid);
}

static void le_remove_dev_periodic_adv_list_cmd(const void *data, uint8_t size)
{
	const struct bt_hci_cmd_le_remove_dev_periodic_adv_list *cmd = data;

	print_addr_type("Adv address type", cmd->addr_type);
	print_addr("Adv address", cmd->addr, cmd->addr_type);
	print_field("SID: 0x%2.2x", cmd->sid);
}

static void le_read_periodic_adv_list_size_rsp(const void *data, uint8_t size)
{
	const struct bt_hci_rsp_le_read_dev_periodic_adv_list_size *rsp = data;

	print_status(rsp->status);
	print_field("List size: 0x%2.2x", rsp->list_size);
}

static void le_read_tx_power_rsp(const void *data, uint8_t size)
{
	const struct bt_hci_rsp_le_read_tx_power *rsp = data;

	print_status(rsp->status);
	print_field("Min Tx power: %d dBm", rsp->min_tx_power);
	print_field("Max Tx power: %d dBm", rsp->max_tx_power);
}

static void le_read_rf_path_comp_rsp(const void *data, uint8_t size)
{
	const struct bt_hci_rsp_le_read_rf_path_comp *rsp = data;

	print_status(rsp->status);
	print_field("RF Tx Path Compensation Value: 0x%4.4x",
							rsp->rf_tx_path_comp);
	print_field("RF Rx Path Compensation Value: 0x%4.4x",
							rsp->rf_rx_path_comp);
}

static void le_write_rf_path_comp_cmd(const void *data, uint8_t size)
{
	const struct bt_hci_cmd_le_write_rf_path_comp *cmd = data;

	print_field("RF Tx Path Compensation Value: 0x%4.4x",
							cmd->rf_tx_path_comp);
	print_field("RF Rx Path Compensation Value: 0x%4.4x",
							cmd->rf_rx_path_comp);
}

static void le_set_priv_mode_cmd(const void *data, uint8_t size)
{
	const struct bt_hci_cmd_le_set_priv_mode *cmd = data;
	const char *str;

	print_addr_type("Peer Identity address type", cmd->peer_id_addr_type);
	print_addr("Peer Identity address", cmd->peer_id_addr,
							cmd->peer_id_addr_type);

	switch (cmd->priv_mode) {
	case 0x00:
		str = "Use Network Privacy";
		break;
	case 0x01:
		str = "Use Device Privacy";
		break;
	default:
		str = "Reserved";
		break;
	}

	print_field("Privacy Mode: %s (0x%2.2x)", str, cmd->priv_mode);
}

static void le_receiver_test_cmd_v3(const void *data, uint8_t size)
{
	const struct bt_hci_cmd_le_receiver_test_v3 *cmd = data;
	uint8_t i;

	print_field("RX Channel: %u MHz (0x%2.2x)", cmd->rx_chan * 2 + 2402,
							cmd->rx_chan);

	switch (cmd->phy) {
	case 0x01:
		print_field("PHY: LE 1M (0x%2.2x)", cmd->phy);
		break;
	case 0x02:
		print_field("PHY: LE 2M (0x%2.2x)", cmd->phy);
		break;
	case 0x03:
		print_field("PHY: LE Coded (0x%2.2x)", cmd->phy);
		break;
	}

	print_field("Modulation Index: %s (0x%2.2x)",
		cmd->mod_index ? "stable" : "standard", cmd->mod_index);
	print_field("Expected CTE Length: %u us (0x%2.2x)", cmd->cte_len * 8,
								cmd->cte_len);
	print_field("Expected CTE Type: %u us slots (0x%2.2x)", cmd->cte_type,
								cmd->cte_type);
	print_field("Slot Duration: %u us (0x%2.2x)", cmd->duration,
								cmd->duration);
	print_field("Number of Antenna IDs: %u", cmd->num_antenna_id);

	if (size < sizeof(*cmd) + cmd->num_antenna_id)
		return;

	for (i = 0; i < cmd->num_antenna_id; i++)
		print_field("  Antenna ID: %u", cmd->antenna_ids[i]);
}

static const char *parse_tx_test_payload(uint8_t payload)
{
	switch (payload) {
	case 0x00:
		return "PRBS9 sequence 11111111100000111101...";
	case 0x01:
		return "Repeated 11110000";
	case 0x02:
		return "Repeated 10101010";
	case 0x03:
		return "PRBS15";
	case 0x04:
		return "Repeated 11111111";
	case 0x05:
		return "Repeated 00000000";
	case 0x06:
		return "Repeated 00001111";
	case 0x07:
		return "Repeated 01010101";
	default:
		return "Reserved";
	}
}

static void le_tx_test_cmd_v3(const void *data, uint8_t size)
{
	const struct bt_hci_cmd_le_tx_test_v3 *cmd = data;
	uint8_t i;

	print_field("TX Channel: %u MHz (0x%2.2x)", cmd->chan * 2 + 2402,
								cmd->chan);
	print_field("Length of Test Data: %u", cmd->data_len);
	print_field("Packet Payload: %s (0x%2.2x)",
			parse_tx_test_payload(cmd->payload), cmd->payload);

	switch (cmd->phy) {
	case 0x01:
		print_field("PHY: LE 1M (0x%2.2x)", cmd->phy);
		break;
	case 0x02:
		print_field("PHY: LE 2M (0x%2.2x)", cmd->phy);
		break;
	case 0x03:
		print_field("PHY: LE Coded with S=8 (0x%2.2x)", cmd->phy);
		break;
	case 0x04:
		print_field("PHY: LE Coded with S=2 (0x%2.2x)", cmd->phy);
		break;
	}

	print_field("Expected CTE Length: %u us (0x%2.2x)", cmd->cte_len * 8,
								cmd->cte_len);
	print_field("Expected CTE Type: %u us slots (0x%2.2x)", cmd->cte_type,
								cmd->cte_type);
	print_field("Slot Duration: %u us (0x%2.2x)", cmd->duration,
								cmd->duration);
	print_field("Number of Antenna IDs: %u", cmd->num_antenna_id);

	if (size < sizeof(*cmd) + cmd->num_antenna_id)
		return;

	for (i = 0; i < cmd->num_antenna_id; i++)
		print_field("  Antenna ID: %u", cmd->antenna_ids[i]);
}

static void le_periodic_adv_rec_enable(const void *data, uint8_t size)
{
	const struct bt_hci_cmd_le_set_periodic_adv_enable *cmd = data;

	print_field("Sync handle: %d", cmd->handle);
	print_enable("Reporting", cmd->enable);
}

static void le_periodic_adv_sync_trans(const void *data, uint8_t size)
{
	const struct bt_hci_cmd_periodic_sync_trans *cmd = data;

	print_field("Connection handle: %d", cmd->handle);
	print_field("Service data: 0x%4.4x", cmd->service_data);
	print_field("Sync handle: %d", cmd->sync_handle);
}

static void le_periodic_adv_set_info_trans(const void *data, uint8_t size)
{
	const struct bt_hci_cmd_periodic_adv_set_info_trans *cmd = data;

	print_field("Connection handle: %d", cmd->handle);
	print_field("Service data: 0x%4.4x", cmd->service_data);
	print_field("Advertising handle: %d", cmd->adv_handle);
}

static void print_sync_mode(uint8_t mode)
{
	const char *str;

	switch (mode) {
	case 0x00:
		str = "Disabled";
		break;
	case 0x01:
		str = "Enabled with report events disabled";
		break;
	case 0x02:
		str = "Enabled with report events enabled";
		break;
	default:
		str = "RFU";
		break;
	}

	print_field("Mode: %s (0x%2.2x)", str, mode);
}

static void le_periodic_adv_sync_trans_params(const void *data, uint8_t size)
{
	const struct bt_hci_cmd_periodic_adv_sync_trans_params *cmd = data;

	print_field("Connection handle: %d", cmd->handle);
	print_sync_mode(cmd->mode);
	print_field("Skip: 0x%2.2x", cmd->skip);
	print_field("Sync timeout: %d msec (0x%4.4x)",
					le16_to_cpu(cmd->sync_timeout) * 10,
					le16_to_cpu(cmd->sync_timeout));
	print_create_sync_cte_type(cmd->cte_type);
}

static void le_set_default_periodic_adv_sync_trans_params(const void *data,
								uint8_t size)
{
	const struct bt_hci_cmd_default_periodic_adv_sync_trans_params *cmd = data;

	print_sync_mode(cmd->mode);
	print_field("Skip: 0x%2.2x", cmd->skip);
	print_field("Sync timeout: %d msec (0x%4.4x)",
					le16_to_cpu(cmd->sync_timeout) * 10,
					le16_to_cpu(cmd->sync_timeout));
	print_create_sync_cte_type(cmd->cte_type);
}

static void print_sca(uint8_t sca)
{
	switch (sca) {
	case 0x00:
		print_field("SCA: 201 - 500 ppm (0x%2.2x)", sca);
		return;
	case 0x01:
		print_field("SCA: 151 - 200 ppm (0x%2.2x)", sca);
		return;
	case 0x02:
		print_field("SCA: 101 - 150 ppm (0x%2.2x)", sca);
		return;
	case 0x03:
		print_field("SCA: 76 - 100 ppm (0x%2.2x)", sca);
		return;
	case 0x04:
		print_field("SCA: 51 - 75 ppm (0x%2.2x)", sca);
		return;
	case 0x05:
		print_field("SCA: 31 - 50 ppm (0x%2.2x)", sca);
		return;
	case 0x06:
		print_field("SCA: 21 - 30 ppm (0x%2.2x)", sca);
		return;
	case 0x07:
		print_field("SCA: 0 - 20 ppm (0x%2.2x)", sca);
		return;
	default:
		print_field("SCA: Reserved (0x%2.2x)", sca);
	}
}

static void print_packing(uint8_t value)
{
	switch (value) {
	case 0x00:
		print_field("Packing: Sequential (0x%2.2x)", value);
		return;
	case 0x01:
		print_field("Packing: Interleaved (0x%2.2x)", value);
		return;
	default:
		print_field("Packing: Reserved (0x%2.2x)", value);
	}
}

static void print_framing(uint8_t value)
{
	switch (value) {
	case 0x00:
		print_field("Framing: Unframed (0x%2.2x)", value);
		return;
	case 0x01:
		print_field("Framing: Framed (0x%2.2x)", value);
		return;
	default:
		print_field("Packing: Reserved (0x%2.2x)", value);
	}
}

static void le_read_buffer_size_v2_rsp(const void *data, uint8_t size)
{
	const struct bt_hci_rsp_le_read_buffer_size_v2 *rsp = data;

	print_status(rsp->status);

	if (size == 1)
		return;

	print_field("ACL MTU: %d", le16_to_cpu(rsp->acl_mtu));
	print_field("ACL max packet: %d", rsp->acl_max_pkt);
	print_field("ISO MTU: %d", le16_to_cpu(rsp->iso_mtu));
	print_field("ISO max packet: %d", rsp->iso_max_pkt);
}

static void le_read_iso_tx_sync_cmd(const void *data, uint8_t size)
{
	const struct bt_hci_cmd_le_read_iso_tx_sync *cmd = data;

	print_field("Handle: %d", le16_to_cpu(cmd->handle));
}

static void le_read_iso_tx_sync_rsp(const void *data, uint8_t size)
{
	const struct bt_hci_rsp_le_read_iso_tx_sync *rsp = data;
	uint32_t offset = 0;

	print_status(rsp->status);

	if (size == 1)
		return;

	print_field("Handle: %d", le16_to_cpu(rsp->handle));
	print_field("Sequence Number: %d", le16_to_cpu(rsp->seq));
	print_field("Timestamp: %d", le32_to_cpu(rsp->timestamp));

	memcpy(&offset, rsp->offset, sizeof(rsp->offset));

	print_field("Offset: %d", le32_to_cpu(offset));
}

static void print_cis_params(const void *data)
{
	const struct bt_hci_cis_params *cis = data;

	print_field("CIS ID: 0x%2.2x", cis->cis_id);
	print_field("Master to Slave Maximum SDU Size: %u",
						le16_to_cpu(cis->m_sdu));
	print_field("Slave to Master Maximum SDU Size: %u",
						le16_to_cpu(cis->s_sdu));
	print_le_phy("Master to Slave PHY", cis->m_phy);
	print_le_phy("Slave to Master PHY", cis->s_phy);
	print_field("Master to Slave Retransmission attempts: 0x%2.2x",
							cis->m_rtn);
	print_field("Slave to Master Retransmission attempts: 0x%2.2x",
							cis->s_rtn);
}

static void print_list(const void *data, uint8_t size, int num_items,
			size_t item_size, void (*callback)(const void *data))
{
	while (size >= item_size && num_items) {
		callback(data);
		data += item_size;
		size -= item_size;
		num_items--;
	}

	if (num_items)
		print_hex_field("", data, size);
}

static void print_usec_interval(const char *prefix, const uint8_t interval[3])
{
	uint32_t u24 = 0;

	memcpy(&u24, interval, 3);
	print_field("%s: %u us (0x%6.6x)", prefix, le32_to_cpu(u24),
						le32_to_cpu(u24));
}

static void le_set_cig_params_cmd(const void *data, uint8_t size)
{
	const struct bt_hci_cmd_le_set_cig_params *cmd = data;

	print_field("CIG ID: 0x%2.2x", cmd->cig_id);
	print_usec_interval("Master to Slave SDU Interval", cmd->m_interval);
	print_usec_interval("Slave to Master SDU Interval", cmd->s_interval);
	print_sca(cmd->sca);
	print_packing(cmd->packing);
	print_framing(cmd->framing);
	print_field("Master to Slave Maximum Latency: %d ms (0x%4.4x)",
		le16_to_cpu(cmd->m_latency), le16_to_cpu(cmd->m_latency));
	print_field("Slave to Master Maximum Latency: %d ms (0x%4.4x)",
		le16_to_cpu(cmd->s_latency), le16_to_cpu(cmd->s_latency));
	print_field("Number of CIS: %u", cmd->num_cis);

	size -= sizeof(*cmd);

	print_list(cmd->cis, size, cmd->num_cis, sizeof(*cmd->cis),
						print_cis_params);
}

static void print_cis_params_test(const void *data)
{
	const struct bt_hci_cis_params_test *cis = data;

	print_field("CIS ID: 0x%2.2x", cis->cis_id);
	print_field("NSE: 0x%2.2x", cis->nse);
	print_field("Master to Slave Maximum SDU: 0x%4.4x", cis->m_sdu);
	print_field("Slave to Master Maximum SDU: 0x%4.4x",
						le16_to_cpu(cis->s_sdu));
	print_field("Master to Slave Maximum PDU: 0x%2.2x",
						le16_to_cpu(cis->m_pdu));
	print_field("Slave to Master Maximum PDU: 0x%2.2x", cis->s_pdu);
	print_le_phy("Master to Slave PHY", cis->m_phy);
	print_le_phy("Slave to Master PHY", cis->s_phy);
	print_field("Master to Slave Burst Number: 0x%2.2x", cis->m_bn);
	print_field("Slave to Master Burst Number: 0x%2.2x", cis->s_bn);
}

static void le_set_cig_params_test_cmd(const void *data, uint8_t size)
{
	const struct bt_hci_cmd_le_set_cig_params_test *cmd = data;

	print_field("CIG ID: 0x%2.2x", cmd->cig_id);
	print_usec_interval("Master to Slave SDU Interval", cmd->m_interval);
	print_usec_interval("Master to Slave SDU Interval", cmd->s_interval);
	print_field("Master to Slave Flush Timeout: 0x%2.2x", cmd->m_ft);
	print_field("Slave to Master Flush Timeout: 0x%2.2x", cmd->s_ft);
	print_field("ISO Interval: %.2f ms (0x%4.4x)",
				le16_to_cpu(cmd->iso_interval) * 1.25,
				le16_to_cpu(cmd->iso_interval));
	print_sca(cmd->sca);
	print_packing(cmd->packing);
	print_framing(cmd->framing);
	print_field("Number of CIS: %u", cmd->num_cis);

	size -= sizeof(*cmd);

	print_list(cmd->cis, size, cmd->num_cis, sizeof(*cmd->cis),
						print_cis_params_test);
}

static void print_cig_handle(const void *data)
{
	const uint16_t *handle = data;

	print_field("Connection Handle: %d", le16_to_cpu(*handle));
}

static void le_set_cig_params_rsp(const void *data, uint8_t size)
{
	const struct bt_hci_rsp_le_set_cig_params *rsp = data;

	print_status(rsp->status);

	if (size == 1)
		return;

	print_field("CIG ID: 0x%2.2x", rsp->cig_id);
	print_field("Number of Handles: %u", rsp->num_handles);

	size -= sizeof(*rsp);

	print_list(rsp->handle, size, rsp->num_handles, sizeof(*rsp->handle),
						print_cig_handle);
}

static void print_cis(const void *data)
{
	const struct bt_hci_cis *cis = data;

	print_field("CIS Handle: %d", cis->cis_handle);
	print_field("ACL Handle: %d", cis->acl_handle);
}

static void le_create_cis_cmd(const void *data, uint8_t size)
{
	const struct bt_hci_cmd_le_create_cis *cmd = data;

	print_field("Number of CIS: %u", cmd->num_cis);

	size -= sizeof(*cmd);

	print_list(cmd->cis, size, cmd->num_cis, sizeof(*cmd->cis), print_cis);
}

static void le_remove_cig_cmd(const void *data, uint8_t size)
{
	const struct bt_hci_cmd_le_remove_cig *cmd = data;

	print_field("CIG ID: 0x%02x", cmd->cig_id);
}

static void le_remove_cig_rsp(const void *data, uint8_t size)
{
	const struct bt_hci_rsp_le_remove_cig *rsp = data;

	print_status(rsp->status);

	if (size == 1)
		return;

	print_field("CIG ID: 0x%2.2x", rsp->cig_id);
}

static void le_accept_cis_req_cmd(const void *data, uint8_t size)
{
	const struct bt_hci_cmd_le_accept_cis *cmd = data;

	print_field("CIS Handle: %d", le16_to_cpu(cmd->handle));
}

static void le_reject_cis_req_cmd(const void *data, uint8_t size)
{
	const struct bt_hci_cmd_le_reject_cis *cmd = data;

	print_field("CIS Handle: %d", le16_to_cpu(cmd->handle));
	print_reason(cmd->reason);
}

static void print_bis(const void *data)
{
	const struct bt_hci_bis *bis = data;

	print_usec_interval("SDU Interval", bis->sdu_interval);
	print_field("Maximum SDU size: %u", le16_to_cpu(bis->sdu));
	print_field("Maximum Latency: %u ms (0x%4.4x)",
			le16_to_cpu(bis->latency), le16_to_cpu(bis->latency));
	print_field("RTN: 0x%2.2x", bis->rtn);
	print_le_phy("PHY", bis->phy);
	print_packing(bis->packing);
	print_framing(bis->framing);
	print_field("Encryption: 0x%2.2x", bis->encryption);
	print_hex_field("Broadcast Code", bis->bcode, 16);
}

static void le_create_big_cmd(const void *data, uint8_t size)
{
	const struct bt_hci_cmd_le_create_big *cmd = data;

	print_field("BIG ID: 0x%2.2x", cmd->big_id);
	print_field("Advertising Handle: 0x%2.2x", cmd->adv_handle);
	print_field("Number of BIS: %u", cmd->num_bis);

	size -= sizeof(*cmd);

	print_list(cmd->bis, size, cmd->num_bis, sizeof(*cmd->bis), print_bis);
}

static void print_bis_test(const void *data)
{
	const struct bt_hci_bis_test *bis = data;

	print_usec_interval("SDU Interval", bis->sdu_interval);
	print_field("ISO Interval: %.2f ms (0x%4.4x)",
				le16_to_cpu(bis->iso_interval) * 1.25,
				le16_to_cpu(bis->iso_interval));
	print_field("Number of Subevents: %u", bis->nse);
	print_field("Maximum SDU: %u", bis->sdu);
	print_field("Maximum PDU: %u", bis->pdu);
	print_packing(bis->packing);
	print_framing(bis->framing);
	print_le_phy("PHY", bis->phy);
	print_field("Burst Number: %u", bis->bn);
	print_field("Immediate Repetition Count: %u", bis->irc);
	print_field("Pre Transmission Offset: 0x%2.2x", bis->pto);
	print_field("Encryption: 0x%2.2x", bis->encryption);
	print_hex_field("Broadcast Code", bis->bcode, 16);
}

static void le_create_big_cmd_test_cmd(const void *data, uint8_t size)
{
	const struct bt_hci_cmd_le_create_big_test *cmd = data;

	print_field("BIG ID: 0x%2.2x", cmd->big_id);
	print_field("Advertising Handle: 0x%2.2x", cmd->adv_handle);
	print_field("Number of BIS: %u", cmd->num_bis);

	size -= sizeof(*cmd);

	print_list(cmd->bis, size, cmd->num_bis, sizeof(*cmd->bis),
						print_bis_test);
}

static void le_terminate_big_cmd(const void *data, uint8_t size)
{
	const struct bt_hci_cmd_le_term_big *cmd = data;

	print_field("BIG ID: 0x%2.2x", cmd->big_id);
	print_reason(cmd->reason);
}

static void print_bis_sync(const void *data)
{
	const uint8_t *bis_id = data;

	print_field("BIS ID: 0x%2.2x", *bis_id);
}

static void le_big_create_sync_cmd(const void *data, uint8_t size)
{
	const struct bt_hci_cmd_le_big_create_sync *cmd = data;

	print_field("BIG ID: 0x%2.2x", cmd->big_id);
	print_field("Number of BIS: %u", cmd->num_bis);
	print_field("Encryption: 0x%2.2x", cmd->encryption);
	print_hex_field("Broadcast Code", cmd->bcode, 16);
	print_field("Number Subevents: 0x%2.2x", cmd->mse);
	print_field("Timeout: %d ms (0x%4.4x)", le16_to_cpu(cmd->timeout) * 10,
						le16_to_cpu(cmd->timeout));

	size -= sizeof(*cmd);

	print_list(cmd->bis, size, cmd->num_bis, sizeof(*cmd->bis),
						print_bis_sync);
}

static void le_big_term_sync_cmd(const void *data, uint8_t size)
{
	const struct bt_hci_cmd_le_big_term_sync *cmd = data;

	print_field("BIG ID: 0x%2.2x", cmd->big_id);
}

static void print_iso_dir(const char *prefix, uint8_t dir)
{
	switch (dir) {
	case 0x00:
		print_field("%s: Input (Host to Controller) (0x%2.2x)",
							prefix, dir);
		return;
	case 0x01:
		print_field("%s: Output (Controller to Host) (0x%2.2x)",
							prefix, dir);
		return;
	default:
		print_field("%s: Unknown (0x%2.2x)", prefix, dir);
	}
}

static void print_iso_path(const char *prefix, uint8_t path)
{
	switch (path) {
	case 0x00:
		print_field("%s: HCI (0x%2.2x)", prefix, path);
		return;
	case 0xff:
		print_field("%s: Disabled (0x%2.2x)", prefix, path);
		return;
	default:
		print_field("%s: Logical Channel Number %u", prefix, path);
	}
}

static void le_setup_iso_path_cmd(const void *data, uint8_t size)
{
	const struct bt_hci_cmd_le_setup_iso_path *cmd = data;

	print_field("Handle: %d", le16_to_cpu(cmd->handle));
	print_iso_dir("Data Path Direction", cmd->direction);
	print_iso_path("Data Path", cmd->path);
	print_codec("Coding Format", cmd->codec);
	packet_print_company("Company Codec ID", le16_to_cpu(cmd->codec_cid));
	print_field("Vendor Codec ID: %d", le16_to_cpu(cmd->codec_vid));
	print_usec_interval("Controller Delay", cmd->delay);
	print_field("Codec Configuration Length: %d", cmd->codec_cfg_len);
	print_hex_field("Codec Configuration", cmd->codec_cfg,
						cmd->codec_cfg_len);
}

static void le_setup_iso_path_rsp(const void *data, uint8_t size)
{
	const struct bt_hci_rsp_le_setup_iso_path *rsp = data;

	print_status(rsp->status);

	if (size == 1)
		return;

	print_field("Handle: %d", le16_to_cpu(rsp->handle));
}

static void le_remove_iso_path_cmd(const void *data, uint8_t size)
{
	const struct bt_hci_cmd_le_remove_iso_path *cmd = data;

	print_field("Connection Handle: %d", le16_to_cpu(cmd->handle));
	print_iso_dir("Data Path Direction", cmd->direction);
}

static void le_req_peer_sca_cmd(const void *data, uint8_t size)
{
	const struct bt_hci_cmd_le_req_peer_sca *cmd = data;

	print_field("Connection Handle: %d", le16_to_cpu(cmd->handle));
}

static void le_set_host_feature_cmd(const void *data, uint8_t size)
{
	const struct bt_hci_cmd_le_set_host_feature *cmd = data;
	uint64_t mask;

	print_field("Bit Number: %u", cmd->bit_number);

	mask = print_bitfield(2, (((uint64_t) 1) << cmd->bit_number),
							features_le);
	if (mask)
		print_text(COLOR_UNKNOWN_FEATURE_BIT, "  Unknown features "
						"(0x%16.16" PRIx64 ")", mask);

	print_field("Bit Value: %u", cmd->bit_value);
}

struct opcode_data {
	uint16_t opcode;
	int bit;
	const char *str;
	void (*cmd_func) (const void *data, uint8_t size);
	uint8_t cmd_size;
	bool cmd_fixed;
	void (*rsp_func) (const void *data, uint8_t size);
	uint8_t rsp_size;
	bool rsp_fixed;
};

static const struct opcode_data opcode_table[] = {
	{ 0x0000,  -1, "NOP" },

	/* OGF 1 - Link Control */
	{ 0x0401,   0, "Inquiry",
				inquiry_cmd, 5, true },
	{ 0x0402,   1, "Inquiry Cancel",
				null_cmd, 0, true,
				status_rsp, 1, true },
	{ 0x0403,   2, "Periodic Inquiry Mode",
				periodic_inquiry_cmd, 9, true,
				status_rsp, 1, true },
	{ 0x0404,   3, "Exit Periodic Inquiry Mode",
				null_cmd, 0, true,
				status_rsp, 1, true },
	{ 0x0405,   4, "Create Connection",
				create_conn_cmd, 13, true },
	{ 0x0406,   5, "Disconnect",
				disconnect_cmd, 3, true },
	{ 0x0407,   6, "Add SCO Connection",
				add_sco_conn_cmd, 4, true },
	{ 0x0408,   7, "Create Connection Cancel",
				create_conn_cancel_cmd, 6, true,
				status_bdaddr_rsp, 7, true },
	{ 0x0409,   8, "Accept Connection Request",
				accept_conn_request_cmd, 7, true },
	{ 0x040a,   9, "Reject Connection Request",
				reject_conn_request_cmd, 7, true },
	{ 0x040b,  10, "Link Key Request Reply",
				link_key_request_reply_cmd, 22, true,
				status_bdaddr_rsp, 7, true },
	{ 0x040c,  11, "Link Key Request Negative Reply",
				link_key_request_neg_reply_cmd, 6, true,
				status_bdaddr_rsp, 7, true },
	{ 0x040d,  12, "PIN Code Request Reply",
				pin_code_request_reply_cmd, 23, true,
				status_bdaddr_rsp, 7, true },
	{ 0x040e,  13, "PIN Code Request Negative Reply",
				pin_code_request_neg_reply_cmd, 6, true,
				status_bdaddr_rsp, 7, true },
	{ 0x040f,  14, "Change Connection Packet Type",
				change_conn_pkt_type_cmd, 4, true },
	{ 0x0411,  15, "Authentication Requested",
				auth_requested_cmd, 2, true },
	{ 0x0413,  16, "Set Connection Encryption",
				set_conn_encrypt_cmd, 3, true },
	{ 0x0415,  17, "Change Connection Link Key",
				change_conn_link_key_cmd, 2, true },
	{ 0x0417,  18, "Master Link Key",
				master_link_key_cmd, 1, true },
	{ 0x0419,  19, "Remote Name Request",
				remote_name_request_cmd, 10, true },
	{ 0x041a,  20, "Remote Name Request Cancel",
				remote_name_request_cancel_cmd, 6, true,
				status_bdaddr_rsp, 7, true },
	{ 0x041b,  21, "Read Remote Supported Features",
				read_remote_features_cmd, 2, true },
	{ 0x041c,  22, "Read Remote Extended Features",
				read_remote_ext_features_cmd, 3, true },
	{ 0x041d,  23, "Read Remote Version Information",
				read_remote_version_cmd, 2, true },
	{ 0x041f,  24, "Read Clock Offset",
				read_clock_offset_cmd, 2, true },
	{ 0x0420,  25, "Read LMP Handle",
				read_lmp_handle_cmd, 2, true,
				read_lmp_handle_rsp, 8, true },
	{ 0x0428, 131, "Setup Synchronous Connection",
				setup_sync_conn_cmd, 17, true },
	{ 0x0429, 132, "Accept Synchronous Connection Request",
				accept_sync_conn_request_cmd, 21, true },
	{ 0x042a, 133, "Reject Synchronous Connection Request",
				reject_sync_conn_request_cmd, 7, true },
	{ 0x042b, 151, "IO Capability Request Reply",
				io_capability_request_reply_cmd, 9, true,
				status_bdaddr_rsp, 7, true },
	{ 0x042c, 152, "User Confirmation Request Reply",
				user_confirm_request_reply_cmd, 6, true,
				status_bdaddr_rsp, 7, true },
	{ 0x042d, 153, "User Confirmation Request Neg Reply",
				user_confirm_request_neg_reply_cmd, 6, true,
				status_bdaddr_rsp, 7, true },
	{ 0x042e, 154, "User Passkey Request Reply",
				user_passkey_request_reply_cmd, 10, true,
				status_bdaddr_rsp, 7, true },
	{ 0x042f, 155, "User Passkey Request Negative Reply",
				user_passkey_request_neg_reply_cmd, 6, true,
				status_bdaddr_rsp, 7, true },
	{ 0x0430, 156, "Remote OOB Data Request Reply",
				remote_oob_data_request_reply_cmd, 38, true,
				status_bdaddr_rsp, 7, true },
	{ 0x0433, 159, "Remote OOB Data Request Neg Reply",
				remote_oob_data_request_neg_reply_cmd, 6, true,
				status_bdaddr_rsp, 7, true },
	{ 0x0434, 163, "IO Capability Request Negative Reply",
				io_capability_request_neg_reply_cmd, 7, true,
				status_bdaddr_rsp, 7, true },
	{ 0x0435, 168, "Create Physical Link",
				create_phy_link_cmd, 3, false },
	{ 0x0436, 169, "Accept Physical Link",
				accept_phy_link_cmd, 3, false },
	{ 0x0437, 170, "Disconnect Physical Link",
				disconn_phy_link_cmd, 2, true },
	{ 0x0438, 171, "Create Logical Link",
				create_logic_link_cmd, 33, true },
	{ 0x0439, 172, "Accept Logical Link",
				accept_logic_link_cmd, 33, true },
	{ 0x043a, 173, "Disconnect Logical Link",
				disconn_logic_link_cmd, 2, true },
	{ 0x043b, 174, "Logical Link Cancel",
				logic_link_cancel_cmd, 2, true,
				logic_link_cancel_rsp, 3, true },
	{ 0x043c, 175, "Flow Specifcation Modify",
				flow_spec_modify_cmd, 34, true },
	{ 0x043d, 235, "Enhanced Setup Synchronous Connection",
				enhanced_setup_sync_conn_cmd, 59, true },
	{ 0x043e, 236, "Enhanced Accept Synchronous Connection Request",
				enhanced_accept_sync_conn_request_cmd, 63, true },
	{ 0x043f, 246, "Truncated Page",
				truncated_page_cmd, 9, true },
	{ 0x0440, 247, "Truncated Page Cancel",
				truncated_page_cancel_cmd, 6, true,
				status_bdaddr_rsp, 7, true },
	{ 0x0441, 248, "Set Connectionless Slave Broadcast",
				set_slave_broadcast_cmd, 11, true,
				set_slave_broadcast_rsp, 4, true },
	{ 0x0442, 249, "Set Connectionless Slave Broadcast Receive",
				set_slave_broadcast_receive_cmd, 34, true,
				set_slave_broadcast_receive_rsp, 8, true },
	{ 0x0443, 250, "Start Synchronization Train",
				null_cmd, 0, true },
	{ 0x0444, 251, "Receive Synchronization Train",
				receive_sync_train_cmd, 12, true },
	{ 0x0445, 257, "Remote OOB Extended Data Request Reply",
				remote_oob_ext_data_request_reply_cmd, 70, true,
				status_bdaddr_rsp, 7, true },

	/* OGF 2 - Link Policy */
	{ 0x0801,  33, "Hold Mode",
				hold_mode_cmd, 6, true },
	{ 0x0803,  34, "Sniff Mode",
				sniff_mode_cmd, 10, true },
	{ 0x0804,  35, "Exit Sniff Mode",
				exit_sniff_mode_cmd, 2, true },
	{ 0x0805,  36, "Park State",
				park_state_cmd, 6, true },
	{ 0x0806,  37, "Exit Park State",
				exit_park_state_cmd, 2, true },
	{ 0x0807,  38, "QoS Setup",
				qos_setup_cmd, 20, true },
	{ 0x0809,  39, "Role Discovery",
				role_discovery_cmd, 2, true,
				role_discovery_rsp, 4, true },
	{ 0x080b,  40, "Switch Role",
				switch_role_cmd, 7, true },
	{ 0x080c,  41, "Read Link Policy Settings",
				read_link_policy_cmd, 2, true,
				read_link_policy_rsp, 5, true },
	{ 0x080d,  42, "Write Link Policy Settings",
				write_link_policy_cmd, 4, true,
				write_link_policy_rsp, 3, true },
	{ 0x080e,  43, "Read Default Link Policy Settings",
				null_cmd, 0, true,
				read_default_link_policy_rsp, 3, true },
	{ 0x080f,  44, "Write Default Link Policy Settings",
				write_default_link_policy_cmd, 2, true,
				status_rsp, 1, true },
	{ 0x0810,  45, "Flow Specification",
				flow_spec_cmd, 21, true },
	{ 0x0811, 140, "Sniff Subrating",
				sniff_subrating_cmd, 8, true,
				sniff_subrating_rsp, 3, true },

	/* OGF 3 - Host Control */
	{ 0x0c01,  46, "Set Event Mask",
				set_event_mask_cmd, 8, true,
				status_rsp, 1, true },
	{ 0x0c03,  47, "Reset",
				null_cmd, 0, true,
				status_rsp, 1, true },
	{ 0x0c05,  48, "Set Event Filter",
				set_event_filter_cmd, 1, false,
				status_rsp, 1, true },
	{ 0x0c08,  49, "Flush",
				flush_cmd, 2, true,
				flush_rsp, 3, true },
	{ 0x0c09,  50, "Read PIN Type",
				null_cmd, 0, true,
				read_pin_type_rsp, 2, true },
	{ 0x0c0a,  51, "Write PIN Type",
				write_pin_type_cmd, 1, true,
				status_rsp, 1, true },
	{ 0x0c0b,  52, "Create New Unit Key",
				null_cmd, 0, true,
				status_rsp, 1, true },
	{ 0x0c0d,  53, "Read Stored Link Key",
				read_stored_link_key_cmd, 7, true,
				read_stored_link_key_rsp, 5, true },
	{ 0x0c11,  54, "Write Stored Link Key",
				write_stored_link_key_cmd, 1, false,
				write_stored_link_key_rsp, 2, true },
	{ 0x0c12,  55, "Delete Stored Link Key",
				delete_stored_link_key_cmd, 7, true,
				delete_stored_link_key_rsp, 3, true },
	{ 0x0c13,  56, "Write Local Name",
				write_local_name_cmd, 248, true,
				status_rsp, 1, true },
	{ 0x0c14,  57, "Read Local Name",
				null_cmd, 0, true,
				read_local_name_rsp, 249, true },
	{ 0x0c15,  58, "Read Connection Accept Timeout",
				null_cmd, 0, true,
				read_conn_accept_timeout_rsp, 3, true },
	{ 0x0c16,  59, "Write Connection Accept Timeout",
				write_conn_accept_timeout_cmd, 2, true,
				status_rsp, 1, true },
	{ 0x0c17,  60, "Read Page Timeout",
				null_cmd, 0, true,
				read_page_timeout_rsp, 3, true },
	{ 0x0c18,  61, "Write Page Timeout",
				write_page_timeout_cmd, 2, true,
				status_rsp, 1, true },
	{ 0x0c19,  62, "Read Scan Enable",
				null_cmd, 0, true,
				read_scan_enable_rsp, 2, true },
	{ 0x0c1a,  63, "Write Scan Enable",
				write_scan_enable_cmd, 1, true,
				status_rsp, 1, true },
	{ 0x0c1b,  64, "Read Page Scan Activity",
				null_cmd, 0, true,
				read_page_scan_activity_rsp, 5, true },
	{ 0x0c1c,  65, "Write Page Scan Activity",
				write_page_scan_activity_cmd, 4, true,
				status_rsp, 1, true },
	{ 0x0c1d,  66, "Read Inquiry Scan Activity",
				null_cmd, 0, true,
				read_inquiry_scan_activity_rsp, 5, true },
	{ 0x0c1e,  67, "Write Inquiry Scan Activity",
				write_inquiry_scan_activity_cmd, 4, true,
				status_rsp, 1, true },
	{ 0x0c1f,  68, "Read Authentication Enable",
				null_cmd, 0, true,
				read_auth_enable_rsp, 2, true },
	{ 0x0c20,  69, "Write Authentication Enable",
				write_auth_enable_cmd, 1, true,
				status_rsp, 1, true },
	{ 0x0c21,  70, "Read Encryption Mode",
				null_cmd, 0, true,
				read_encrypt_mode_rsp, 2, true },
	{ 0x0c22,  71, "Write Encryption Mode",
				write_encrypt_mode_cmd, 1, true,
				status_rsp, 1, true },
	{ 0x0c23,  72, "Read Class of Device",
				null_cmd, 0, true,
				read_class_of_dev_rsp, 4, true },
	{ 0x0c24,  73, "Write Class of Device",
				write_class_of_dev_cmd, 3, true,
				status_rsp, 1, true },
	{ 0x0c25,  74, "Read Voice Setting",
				null_cmd, 0, true,
				read_voice_setting_rsp, 3, true },
	{ 0x0c26,  75, "Write Voice Setting",
				write_voice_setting_cmd, 2, true,
				status_rsp, 1, true },
	{ 0x0c27,  76, "Read Automatic Flush Timeout",
				read_auto_flush_timeout_cmd, 2, true,
				read_auto_flush_timeout_rsp, 5, true },
	{ 0x0c28,  77, "Write Automatic Flush Timeout",
				write_auto_flush_timeout_cmd, 4, true,
				write_auto_flush_timeout_rsp, 3, true },
	{ 0x0c29,  78, "Read Num Broadcast Retransmissions",
				null_cmd, 0, true,
				read_num_broadcast_retrans_rsp, 2, true },
	{ 0x0c2a,  79, "Write Num Broadcast Retransmissions",
				write_num_broadcast_retrans_cmd, 1, true,
				status_rsp, 1, true },
	{ 0x0c2b,  80, "Read Hold Mode Activity",
				null_cmd, 0, true,
				read_hold_mode_activity_rsp, 2, true },
	{ 0x0c2c,  81, "Write Hold Mode Activity",
				write_hold_mode_activity_cmd, 1, true,
				status_rsp, 1, true },
	{ 0x0c2d,  82, "Read Transmit Power Level",
				read_tx_power_cmd, 3, true,
				read_tx_power_rsp, 4, true },
	{ 0x0c2e,  83, "Read Sync Flow Control Enable",
				null_cmd, 0, true,
				read_sync_flow_control_rsp, 2, true },
	{ 0x0c2f,  84, "Write Sync Flow Control Enable",
				write_sync_flow_control_cmd, 1, true,
				status_rsp, 1, true },
	{ 0x0c31,  85, "Set Controller To Host Flow Control",
				set_host_flow_control_cmd, 1, true,
				status_rsp, 1, true },
	{ 0x0c33,  86, "Host Buffer Size",
				host_buffer_size_cmd, 7, true,
				status_rsp, 1, true },
	{ 0x0c35,  87, "Host Number of Completed Packets",
				host_num_completed_packets_cmd, 5, false },
	{ 0x0c36,  88, "Read Link Supervision Timeout",
				read_link_supv_timeout_cmd, 2, true,
				read_link_supv_timeout_rsp, 5, true },
	{ 0x0c37,  89, "Write Link Supervision Timeout",
				write_link_supv_timeout_cmd, 4, true,
				write_link_supv_timeout_rsp, 3, true },
	{ 0x0c38,  90, "Read Number of Supported IAC",
				null_cmd, 0, true,
				read_num_supported_iac_rsp, 2, true },
	{ 0x0c39,  91, "Read Current IAC LAP",
				null_cmd, 0, true,
				read_current_iac_lap_rsp, 2, false },
	{ 0x0c3a,  92, "Write Current IAC LAP",
				write_current_iac_lap_cmd, 1, false,
				status_rsp, 1, true },
	{ 0x0c3b,  93, "Read Page Scan Period Mode",
				null_cmd, 0, true,
				read_page_scan_period_mode_rsp, 2, true },
	{ 0x0c3c,  94, "Write Page Scan Period Mode",
				write_page_scan_period_mode_cmd, 1, true,
				status_rsp, 1, true },
	{ 0x0c3d,  95, "Read Page Scan Mode",
				null_cmd, 0, true,
				read_page_scan_mode_rsp, 2, true },
	{ 0x0c3e,  96, "Write Page Scan Mode",
				write_page_scan_mode_cmd, 1, true,
				status_rsp, 1, true },
	{ 0x0c3f,  97, "Set AFH Host Channel Classification",
				set_afh_host_classification_cmd, 10, true,
				status_rsp, 1, true },
	{ 0x0c42, 100, "Read Inquiry Scan Type",
				null_cmd, 0, true,
				read_inquiry_scan_type_rsp, 2, true },
	{ 0x0c43, 101, "Write Inquiry Scan Type",
				write_inquiry_scan_type_cmd, 1, true,
				status_rsp, 1, true },
	{ 0x0c44, 102, "Read Inquiry Mode",
				null_cmd, 0, true,
				read_inquiry_mode_rsp, 2, true },
	{ 0x0c45, 103, "Write Inquiry Mode",
				write_inquiry_mode_cmd, 1, true,
				status_rsp, 1, true },
	{ 0x0c46, 104, "Read Page Scan Type",
				null_cmd, 0, true,
				read_page_scan_type_rsp, 2, true },
	{ 0x0c47, 105, "Write Page Scan Type",
				write_page_scan_type_cmd, 1, true,
				status_rsp, 1, true },
	{ 0x0c48, 106, "Read AFH Channel Assessment Mode",
				null_cmd, 0, true,
				read_afh_assessment_mode_rsp, 2, true },
	{ 0x0c49, 107, "Write AFH Channel Assessment Mode",
				write_afh_assessment_mode_cmd, 1, true,
				status_rsp, 1, true },
	{ 0x0c51, 136, "Read Extended Inquiry Response",
				null_cmd, 0, true,
				read_ext_inquiry_response_rsp, 242, true },
	{ 0x0c52, 137, "Write Extended Inquiry Response",
				write_ext_inquiry_response_cmd, 241, true,
				status_rsp, 1, true },
	{ 0x0c53, 138, "Refresh Encryption Key",
				refresh_encrypt_key_cmd, 2, true },
	{ 0x0c55, 141, "Read Simple Pairing Mode",
				null_cmd, 0, true,
				read_simple_pairing_mode_rsp, 2, true },
	{ 0x0c56, 142, "Write Simple Pairing Mode",
				write_simple_pairing_mode_cmd, 1, true,
				status_rsp, 1, true },
	{ 0x0c57, 143, "Read Local OOB Data",
				null_cmd, 0, true,
				read_local_oob_data_rsp, 33, true },
	{ 0x0c58, 144, "Read Inquiry Response TX Power Level",
				null_cmd, 0, true,
				read_inquiry_resp_tx_power_rsp, 2, true },
	{ 0x0c59, 145, "Write Inquiry Transmit Power Level",
				write_inquiry_tx_power_cmd, 1, true,
				status_rsp, 1, true },
	{ 0x0c5a, 146, "Read Default Erroneous Data Reporting",
				null_cmd, 0, true,
				read_erroneous_reporting_rsp, 2, true },
	{ 0x0c5b, 147, "Write Default Erroneous Data Reporting",
				write_erroneous_reporting_cmd, 1, true,
				status_rsp, 1, true },
	{ 0x0c5f, 158, "Enhanced Flush",
				enhanced_flush_cmd, 3, true },
	{ 0x0c60, 162, "Send Keypress Notification",
				send_keypress_notify_cmd, 7, true,
				send_keypress_notify_rsp, 7, true },
	{ 0x0c61, 176, "Read Logical Link Accept Timeout" },
	{ 0x0c62, 177, "Write Logical Link Accept Timeout" },
	{ 0x0c63, 178, "Set Event Mask Page 2",
				set_event_mask_page2_cmd, 8, true,
				status_rsp, 1, true },
	{ 0x0c64, 179, "Read Location Data",
				null_cmd, 0, true,
				read_location_data_rsp, 6, true },
	{ 0x0c65, 180, "Write Location Data",
				write_location_data_cmd, 5, true,
				status_rsp, 1, true },
	{ 0x0c66, 184, "Read Flow Control Mode",
				null_cmd, 0, true,
				read_flow_control_mode_rsp, 2, true },
	{ 0x0c67, 185, "Write Flow Control Mode",
				write_flow_control_mode_cmd, 1, true,
				status_rsp, 1, true },
	{ 0x0c68, 192, "Read Enhanced Transmit Power Level",
				read_enhanced_tx_power_cmd, 3, true,
				read_enhanced_tx_power_rsp, 6, true },
	{ 0x0c69, 194, "Read Best Effort Flush Timeout" },
	{ 0x0c6a, 195, "Write Best Effort Flush Timeout" },
	{ 0x0c6b, 196, "Short Range Mode",
				short_range_mode_cmd, 2, true },
	{ 0x0c6c, 197, "Read LE Host Supported",
				null_cmd, 0, true,
				read_le_host_supported_rsp, 3, true },
	{ 0x0c6d, 198, "Write LE Host Supported",
				write_le_host_supported_cmd, 2, true,
				status_rsp, 1, true },
	{ 0x0c6e, 238, "Set MWS Channel Parameters" },
	{ 0x0c6f, 239, "Set External Frame Configuration" },
	{ 0x0c70, 240, "Set MWS Signaling" },
	{ 0x0c71, 241, "Set MWS Transport Layer" },
	{ 0x0c72, 242, "Set MWS Scan Frequency Table" },
	{ 0x0c73, 244, "Set MWS Pattern Configuration" },
	{ 0x0c74, 252, "Set Reserved LT_ADDR",
				set_reserved_lt_addr_cmd, 1, true,
				set_reserved_lt_addr_rsp, 2, true },
	{ 0x0c75, 253, "Delete Reserved LT_ADDR",
				delete_reserved_lt_addr_cmd, 1, true,
				delete_reserved_lt_addr_rsp, 2, true },
	{ 0x0c76, 254, "Set Connectionless Slave Broadcast Data",
				set_slave_broadcast_data_cmd, 3, false,
				set_slave_broadcast_data_rsp, 2, true },
	{ 0x0c77, 255, "Read Synchronization Train Parameters",
				null_cmd, 0, true,
				read_sync_train_params_rsp, 8, true },
	{ 0x0c78, 256, "Write Synchronization Train Parameters",
				write_sync_train_params_cmd, 9, true,
				write_sync_train_params_rsp, 3, true },
	{ 0x0c79, 258, "Read Secure Connections Host Support",
				null_cmd, 0, true,
				read_secure_conn_support_rsp, 2, true },
	{ 0x0c7a, 259, "Write Secure Connections Host Support",
				write_secure_conn_support_cmd, 1, true,
				status_rsp, 1, true },
	{ 0x0c7b, 260, "Read Authenticated Payload Timeout",
				read_auth_payload_timeout_cmd, 2, true,
				read_auth_payload_timeout_rsp, 5, true },
	{ 0x0c7c, 261, "Write Authenticated Payload Timeout",
				write_auth_payload_timeout_cmd, 4, true,
				write_auth_payload_timeout_rsp, 3, true },
	{ 0x0c7d, 262, "Read Local OOB Extended Data",
				null_cmd, 0, true,
				read_local_oob_ext_data_rsp, 65, true },
	{ 0x0c7e, 264, "Read Extended Page Timeout",
				null_cmd, 0, true,
				read_ext_page_timeout_rsp, 3, true },
	{ 0x0c7f, 265, "Write Extended Page Timeout",
				write_ext_page_timeout_cmd, 2, true,
				status_rsp, 1, true },
	{ 0x0c80, 266, "Read Extended Inquiry Length",
				null_cmd, 0, true,
				read_ext_inquiry_length_rsp, 3, true },
	{ 0x0c81, 267, "Write Extended Inquiry Length",
				write_ext_inquiry_length_cmd, 2, true,
				status_rsp, 1, true },

	/* OGF 4 - Information Parameter */
	{ 0x1001, 115, "Read Local Version Information",
				null_cmd, 0, true,
				read_local_version_rsp, 9, true },
	{ 0x1002, 116, "Read Local Supported Commands",
				null_cmd, 0, true,
				read_local_commands_rsp, 65, true },
	{ 0x1003, 117, "Read Local Supported Features",
				null_cmd, 0, true,
				read_local_features_rsp, 9, true },
	{ 0x1004, 118, "Read Local Extended Features",
				read_local_ext_features_cmd, 1, true,
				read_local_ext_features_rsp, 11, true },
	{ 0x1005, 119, "Read Buffer Size",
				null_cmd, 0, true,
				read_buffer_size_rsp, 8, true },
	{ 0x1007, 120, "Read Country Code",
				null_cmd, 0, true,
				read_country_code_rsp, 2, true },
	{ 0x1009, 121, "Read BD ADDR",
				null_cmd, 0, true,
				read_bd_addr_rsp, 7, true },
	{ 0x100a, 186, "Read Data Block Size",
				null_cmd, 0, true,
				read_data_block_size_rsp, 7, true },
	{ 0x100b, 237, "Read Local Supported Codecs",
				null_cmd, 0, true,
				read_local_codecs_rsp, 3, false },
	{ 0x100c, 331, "Read Local Simple Pairing Options",
				null_cmd, 0, true,
				read_local_pairing_options_rsp, 3, true },

	/* OGF 5 - Status Parameter */
	{ 0x1401, 122, "Read Failed Contact Counter",
				read_failed_contact_counter_cmd, 2, true,
				read_failed_contact_counter_rsp, 5, true },
	{ 0x1402, 123, "Reset Failed Contact Counter",
				reset_failed_contact_counter_cmd, 2, true,
				reset_failed_contact_counter_rsp, 3, true },
	{ 0x1403, 124, "Read Link Quality",
				read_link_quality_cmd, 2, true,
				read_link_quality_rsp, 4, true },
	{ 0x1405, 125, "Read RSSI",
				read_rssi_cmd, 2, true,
				read_rssi_rsp, 4, true },
	{ 0x1406, 126, "Read AFH Channel Map",
				read_afh_channel_map_cmd, 2, true,
				read_afh_channel_map_rsp, 14, true },
	{ 0x1407, 127, "Read Clock",
				read_clock_cmd, 3, true,
				read_clock_rsp, 9, true },
	{ 0x1408, 164, "Read Encryption Key Size",
				read_encrypt_key_size_cmd, 2, true,
				read_encrypt_key_size_rsp, 4, true },
	{ 0x1409, 181, "Read Local AMP Info",
				null_cmd, 0, true,
				read_local_amp_info_rsp, 31, true },
	{ 0x140a, 182, "Read Local AMP ASSOC",
				read_local_amp_assoc_cmd, 5, true,
				read_local_amp_assoc_rsp, 5, false },
	{ 0x140b, 183, "Write Remote AMP ASSOC",
				write_remote_amp_assoc_cmd, 6, false,
				write_remote_amp_assoc_rsp, 2, true },
	{ 0x140c, 243, "Get MWS Transport Layer Configuration",
				null_cmd, 0, true,
				get_mws_transport_config_rsp, 2, false },
	{ 0x140d, 245, "Set Triggered Clock Capture",
				set_triggered_clock_capture_cmd, 6, true,
				status_rsp, 1, true },

	/* OGF 6 - Testing */
	{ 0x1801, 128, "Read Loopback Mode",
				null_cmd, 0, true,
				read_loopback_mode_rsp, 2, true },
	{ 0x1802, 129, "Write Loopback Mode",
				write_loopback_mode_cmd, 1, true,
				status_rsp, 1, true },
	{ 0x1803, 130, "Enable Device Under Test Mode",
				null_cmd, 0, true,
				status_rsp, 1, true },
	{ 0x1804, 157, "Write Simple Pairing Debug Mode",
				write_ssp_debug_mode_cmd, 1, true,
				status_rsp, 1, true },
	{ 0x1807, 189, "Enable AMP Receiver Reports" },
	{ 0x1808, 190, "AMP Test End" },
	{ 0x1809, 191, "AMP Test" },
	{ 0x180a, 263, "Write Secure Connections Test Mode" },

	/* OGF 8 - LE Control */
	{ 0x2001, 200, "LE Set Event Mask",
				le_set_event_mask_cmd, 8, true,
				status_rsp, 1, true },
	{ 0x2002, 201, "LE Read Buffer Size",
				null_cmd, 0, true,
				le_read_buffer_size_rsp, 4, true },
	{ 0x2003, 202, "LE Read Local Supported Features",
				null_cmd, 0, true,
				le_read_local_features_rsp, 9, true },
	{ 0x2005, 204, "LE Set Random Address",
				le_set_random_address_cmd, 6, true,
				status_rsp, 1, true },
	{ 0x2006, 205, "LE Set Advertising Parameters",
				le_set_adv_parameters_cmd, 15, true,
				status_rsp, 1, true },
	{ 0x2007, 206, "LE Read Advertising Channel TX Power",
				null_cmd, 0, true,
				le_read_adv_tx_power_rsp, 2, true },
	{ 0x2008, 207, "LE Set Advertising Data",
				le_set_adv_data_cmd, 32, true,
				status_rsp, 1, true },
	{ 0x2009, 208, "LE Set Scan Response Data",
				le_set_scan_rsp_data_cmd, 32, true,
				status_rsp, 1, true },
	{ 0x200a, 209, "LE Set Advertise Enable",
				le_set_adv_enable_cmd, 1, true,
				status_rsp, 1, true },
	{ 0x200b, 210, "LE Set Scan Parameters",
				le_set_scan_parameters_cmd, 7, true,
				status_rsp, 1, true },
	{ 0x200c, 211, "LE Set Scan Enable",
				le_set_scan_enable_cmd, 2, true,
				status_rsp, 1, true },
	{ 0x200d, 212, "LE Create Connection",
				le_create_conn_cmd, 25, true },
	{ 0x200e, 213, "LE Create Connection Cancel",
				null_cmd, 0, true,
				status_rsp, 1, true },
	{ 0x200f, 214, "LE Read White List Size",
				null_cmd, 0, true,
				le_read_white_list_size_rsp, 2, true },
	{ 0x2010, 215, "LE Clear White List",
				null_cmd, 0, true,
				status_rsp, 1, true },
	{ 0x2011, 216, "LE Add Device To White List",
				le_add_to_white_list_cmd, 7, true,
				status_rsp, 1, true },
	{ 0x2012, 217, "LE Remove Device From White List",
				le_remove_from_white_list_cmd, 7, true,
				status_rsp, 1, true },
	{ 0x2013, 218, "LE Connection Update",
				le_conn_update_cmd, 14, true },
	{ 0x2014, 219, "LE Set Host Channel Classification",
				le_set_host_classification_cmd, 5, true,
				status_rsp, 1, true },
	{ 0x2015, 220, "LE Read Channel Map",
				le_read_channel_map_cmd, 2, true,
				le_read_channel_map_rsp, 8, true },
	{ 0x2016, 221, "LE Read Remote Used Features",
				le_read_remote_features_cmd, 2, true },
	{ 0x2017, 222, "LE Encrypt",
				le_encrypt_cmd, 32, true,
				le_encrypt_rsp, 17, true },
	{ 0x2018, 223, "LE Rand",
				null_cmd, 0, true,
				le_rand_rsp, 9, true },
	{ 0x2019, 224, "LE Start Encryption",
				le_start_encrypt_cmd, 28, true },
	{ 0x201a, 225, "LE Long Term Key Request Reply",
				le_ltk_req_reply_cmd, 18, true,
				le_ltk_req_reply_rsp, 3, true },
	{ 0x201b, 226, "LE Long Term Key Request Neg Reply",
				le_ltk_req_neg_reply_cmd, 2, true,
				le_ltk_req_neg_reply_rsp, 3, true },
	{ 0x201c, 227, "LE Read Supported States",
				null_cmd, 0, true,
				le_read_supported_states_rsp, 9, true },
	{ 0x201d, 228, "LE Receiver Test",
				le_receiver_test_cmd, 1, true,
				status_rsp, 1, true },
	{ 0x201e, 229, "LE Transmitter Test",
				le_transmitter_test_cmd, 3, true,
				status_rsp, 1, true },
	{ 0x201f, 230, "LE Test End",
				null_cmd, 0, true,
				le_test_end_rsp, 3, true },
	{ 0x2020, 268, "LE Remote Connection Parameter Request Reply",
				le_conn_param_req_reply_cmd, 14, true,
				le_conn_param_req_reply_rsp, 3, true },
	{ 0x2021, 269, "LE Remote Connection Parameter Request Negative Reply",
				le_conn_param_req_neg_reply_cmd, 3, true,
				le_conn_param_req_neg_reply_rsp, 3, true },
	{ 0x2022, 270, "LE Set Data Length",
				le_set_data_length_cmd, 6, true,
				le_set_data_length_rsp, 3, true },
	{ 0x2023, 271, "LE Read Suggested Default Data Length",
				null_cmd, 0, true,
				le_read_default_data_length_rsp, 5, true },
	{ 0x2024, 272, "LE Write Suggested Default Data Length",
				le_write_default_data_length_cmd, 4, true,
				status_rsp, 1, true },
	{ 0x2025, 273, "LE Read Local P-256 Public Key",
				null_cmd, 0, true },
	{ 0x2026, 274, "LE Generate DHKey",
				le_generate_dhkey_cmd, 64, true },
	{ 0x2027, 275, "LE Add Device To Resolving List",
				le_add_to_resolv_list_cmd, 39, true,
				status_rsp, 1, true },
	{ 0x2028, 276, "LE Remove Device From Resolving List",
				le_remove_from_resolv_list_cmd, 7, true,
				status_rsp, 1, true },
	{ 0x2029, 277, "LE Clear Resolving List",
				null_cmd, 0, true,
				status_rsp, 1, true },
	{ 0x202a, 278, "LE Read Resolving List Size",
				null_cmd, 0, true,
				le_read_resolv_list_size_rsp, 2, true },
	{ 0x202b, 279, "LE Read Peer Resolvable Address",
				le_read_peer_resolv_addr_cmd, 7, true,
				le_read_peer_resolv_addr_rsp, 7, true },
	{ 0x202c, 280, "LE Read Local Resolvable Address",
				le_read_local_resolv_addr_cmd, 7, true,
				le_read_local_resolv_addr_rsp, 7, true },
	{ 0x202d, 281, "LE Set Address Resolution Enable",
				le_set_resolv_enable_cmd, 1, true,
				status_rsp, 1, true },
	{ 0x202e, 282, "LE Set Resolvable Private Address Timeout",
				le_set_resolv_timeout_cmd, 2, true,
				status_rsp, 1, true },
	{ 0x202f, 283, "LE Read Maximum Data Length",
				null_cmd, 0, true,
				le_read_max_data_length_rsp, 9, true },
	{ 0x2030, 284, "LE Read PHY",
				le_read_phy_cmd, 2, true,
				le_read_phy_rsp, 5, true},
	{ 0x2031, 285, "LE Set Default PHY",
				le_set_default_phy_cmd, 3, true,
				status_rsp, 1, true },
	{ 0x2032, 286, "LE Set PHY",
				le_set_phy_cmd, 7, true},
	{ 0x2033, 287, "LE Enhanced Receiver Test",
				le_enhanced_receiver_test_cmd, 3, true,
				status_rsp, 1, true },
	{ 0x2034, 288, "LE Enhanced Transmitter Test",
				le_enhanced_transmitter_test_cmd, 4, true,
				status_rsp, 1, true },
	{ 0x2035, 289, "LE Set Advertising Set Random Address",
				le_set_adv_set_rand_addr, 7, true,
				status_rsp, 1, true },
	{ 0x2036, 290, "LE Set Extended Advertising Parameters",
				le_set_ext_adv_params_cmd, 25, true,
				le_set_ext_adv_params_rsp, 2, true },
	{ 0x2037, 291, "LE Set Extended Advertising Data",
				le_set_ext_adv_data_cmd, 4, false,
				status_rsp, 1, true },
	{ 0x2038, 292, "LE Set Extended Scan Response Data",
				le_set_ext_scan_rsp_data_cmd, 4, false,
				status_rsp, 1, true },
	{ 0x2039, 293, "LE Set Extended Advertising Enable",
				le_set_ext_adv_enable_cmd, 2, false,
				status_rsp, 1, true },
	{ 0x203a, 294, "LE Read Maximum Advertising Data Length",
				null_cmd, 0, true,
				le_read_max_adv_data_len_rsp, 3, true },
	{ 0x203b, 295, "LE Read Number of Supported Advertising Sets",
				null_cmd, 0, true,
				le_read_num_supported_adv_sets_rsp, 2, true },
	{ 0x203c, 296, "LE Remove Advertising Set",
				le_remove_adv_set_cmd, 1, true,
				status_rsp, 1, true },
	{ 0x203d, 297, "LE Clear Advertising Sets",
				null_cmd, 0, true,
				status_rsp, 1, true },
	{ 0x203e, 298, "LE Set Periodic Advertising Parameters",
				le_set_periodic_adv_params_cmd, 7, true,
				status_rsp, 1, true },
	{ 0x203f, 299, "LE Set Periodic Advertising Data",
				le_set_periodic_adv_data_cmd, 3, false,
				status_rsp, 1, true },
	{ 0x2040, 300, "LE Set Periodic Advertising Enable",
				le_set_periodic_adv_enable_cmd, 2, true,
				status_rsp, 1, true },
	{ 0x2041, 301, "LE Set Extended Scan Parameters",
				le_set_ext_scan_params_cmd, 3, false,
				status_rsp, 1, true },
	{ 0x2042, 302, "LE Set Extended Scan Enable",
				le_set_ext_scan_enable_cmd, 6, true,
				status_rsp, 1, true },
	{ 0x2043, 303, "LE Extended Create Connection",
				le_ext_create_conn_cmd, 10, false,
				status_rsp, 1, true },
	{ 0x2044, 304, "LE Periodic Advertising Create Sync",
				le_periodic_adv_create_sync_cmd, 14, true,
				status_rsp, 1, true },
	{ 0x2045, 305, "LE Periodic Advertising Create Sync Cancel",
				null_cmd, 0, true,
				status_rsp, 1, true },
	{ 0x2046, 306, "LE Periodic Advertising Terminate Sync",
				le_periodic_adv_term_sync_cmd, 2, true,
				status_rsp, 1, true },
	{ 0x2047, 307, "LE Add Device To Periodic Advertiser List",
				le_add_dev_periodic_adv_list_cmd, 8, true,
				status_rsp, 1, true },
	{ 0x2048, 308, "LE Remove Device From Periodic Advertiser List",
				le_remove_dev_periodic_adv_list_cmd, 8, true,
				status_rsp, 1, true },
	{ 0x2049, 309, "LE Clear Periodic Advertiser List",
				null_cmd, 0, true,
				status_rsp, 1, true },
	{ 0x204a, 310, "LE Read Periodic Advertiser List Size",
				null_cmd, 0, true,
				le_read_periodic_adv_list_size_rsp, 2, true },
	{ 0x204b, 311, "LE Read Transmit Power",
				null_cmd, 0, true,
				le_read_tx_power_rsp, 3, true },
	{ 0x204c, 312, "LE Read RF Path Compensation",
				null_cmd, 0, true,
				le_read_rf_path_comp_rsp, 5, true },
	{ 0x204d, 313, "LE Write RF Path Compensation",
				le_write_rf_path_comp_cmd, 4, true,
				status_rsp, 1, true },
	{ 0x204e, 314, "LE Set Privacy Mode",
				le_set_priv_mode_cmd, 8, true,
				status_rsp, 1, true },
	{ 0x204f, 315, "LE Receiver Test command [v3]",
				le_receiver_test_cmd_v3, 7, false,
				status_rsp, 1, true },
	{ 0x2050, 316, "LE Transmitter Test command [v3]",
				le_tx_test_cmd_v3, 9, false,
				status_rsp, 1, true },
	{ 0x2059, 325, "LE Periodic Advertising Receive Enable",
				le_periodic_adv_rec_enable, 3, true,
				status_rsp, 1, true },
	{ 0x205a, 326, "LE Periodic Advertising Sync Transfer",
				le_periodic_adv_sync_trans, 6, true,
				status_handle_rsp, 3, true },
	{ 0x205b, 327, "LE Periodic Advertising Set Info Transfer",
				le_periodic_adv_set_info_trans, 5, true,
				status_handle_rsp, 3, true },
	{ 0x205c, 328, "LE Periodic Advertising Sync Transfer Parameters",
				le_periodic_adv_sync_trans_params, 8, true,
				status_handle_rsp, 3, true},
	{ 0x205d, 329, "LE Set Default Periodic Advertisng Sync Transfer "
				"Parameters",
				le_set_default_periodic_adv_sync_trans_params,
				6, true, status_rsp, 1, true},
	{ BT_HCI_CMD_LE_READ_BUFFER_SIZE_V2,
				BT_HCI_BIT_LE_READ_BUFFER_SIZE_V2,
				"LE Read Buffer v2",
				null_cmd, 0, true,
				le_read_buffer_size_v2_rsp,
				sizeof(
				struct bt_hci_rsp_le_read_buffer_size_v2),
				true },
	{ BT_HCI_CMD_LE_READ_ISO_TX_SYNC,
				BT_HCI_BIT_LE_READ_ISO_TX_SYNC,
				"LE Read ISO TX Sync",
				le_read_iso_tx_sync_cmd,
				sizeof(struct bt_hci_cmd_le_read_iso_tx_sync),
				true,
				le_read_iso_tx_sync_rsp,
				sizeof(struct bt_hci_rsp_le_read_iso_tx_sync),
				true },
	{ BT_HCI_CMD_LE_SET_CIG_PARAMS, BT_HCI_BIT_LE_SET_CIG_PARAMS,
				"LE Set Connected Isochronous Group Parameters",
				le_set_cig_params_cmd,
				sizeof(struct bt_hci_cmd_le_set_cig_params),
				false,
				le_set_cig_params_rsp,
				sizeof(struct bt_hci_rsp_le_set_cig_params),
				false },
	{ BT_HCI_CMD_LE_SET_CIG_PARAMS_TEST, BT_HCI_BIT_LE_SET_CIG_PARAMS_TEST,
				"LE Set Connected Isochronous Group Parameters"
				" Test", le_set_cig_params_test_cmd,
				sizeof(
				struct bt_hci_cmd_le_set_cig_params_test),
				false,
				le_set_cig_params_rsp,
				sizeof(struct bt_hci_rsp_le_set_cig_params),
				false },
	{ BT_HCI_CMD_LE_CREATE_CIS, BT_HCI_BIT_LE_CREATE_CIS,
				"LE Create Connected Isochronous Stream",
				le_create_cis_cmd,
				sizeof(struct bt_hci_cmd_le_create_cis),
				false },
	{ BT_HCI_CMD_LE_REMOVE_CIG, BT_HCI_BIT_LE_REMOVE_CIG,
				"LE Remove Connected Isochronous Group",
				le_remove_cig_cmd,
				sizeof(struct bt_hci_cmd_le_remove_cig), false,
				le_remove_cig_rsp,
				sizeof(struct bt_hci_rsp_le_remove_cig),
				false },
	{ BT_HCI_CMD_LE_ACCEPT_CIS, BT_HCI_BIT_LE_ACCEPT_CIS,
				"LE Accept Connected Isochronous Stream Request",
				le_accept_cis_req_cmd,
				sizeof(struct bt_hci_cmd_le_accept_cis), true },
	{ BT_HCI_CMD_LE_REJECT_CIS, BT_HCI_BIT_LE_REJECT_CIS,
				"LE Reject Connected Isochronous Stream Request",
				le_reject_cis_req_cmd,
				sizeof(struct bt_hci_cmd_le_reject_cis), true,
				status_rsp, 1, true },
	{ BT_HCI_CMD_LE_CREATE_BIG, BT_HCI_BIT_LE_CREATE_BIG,
				"LE Create Broadcast Isochronous Group",
				le_create_big_cmd },
	{ BT_HCI_CMD_LE_CREATE_BIG_TEST, BT_HCI_BIT_LE_CREATE_BIG_TEST,
				"LE Create Broadcast Isochronous Group Test",
				le_create_big_cmd_test_cmd },
	{ BT_HCI_CMD_LE_TERM_BIG, BT_HCI_BIT_LE_TERM_BIG,
				"LE Terminate Broadcast Isochronous Group",
				le_terminate_big_cmd,
				sizeof(struct bt_hci_cmd_le_term_big), true,
				status_rsp, 1, true},
	{ BT_HCI_CMD_LE_BIG_CREATE_SYNC, BT_HCI_BIT_LE_BIG_CREATE_SYNC,
				"LE Broadcast Isochronous Group Create Sync",
				le_big_create_sync_cmd,
				sizeof(struct bt_hci_cmd_le_big_create_sync),
				true },
	{ BT_HCI_CMD_LE_BIG_TERM_SYNC, BT_HCI_BIT_LE_BIG_TERM_SYNC,
				"LE Broadcast Isochronous Group Terminate Sync",
				le_big_term_sync_cmd,
				sizeof(struct bt_hci_cmd_le_big_term_sync),
				true },
	{ BT_HCI_CMD_LE_REQ_PEER_SCA, BT_HCI_BIT_LE_REQ_PEER_SCA,
				"LE Request Peer SCA", le_req_peer_sca_cmd,
				sizeof(struct bt_hci_cmd_le_req_peer_sca),
				true },
	{ BT_HCI_CMD_LE_SETUP_ISO_PATH, BT_HCI_BIT_LE_SETUP_ISO_PATH,
				"LE Setup Isochronous Data Path",
				le_setup_iso_path_cmd,
				sizeof(struct bt_hci_cmd_le_setup_iso_path),
				true, le_setup_iso_path_rsp,
				sizeof(struct bt_hci_rsp_le_setup_iso_path),
				true },
	{ BT_HCI_CMD_LE_REMOVE_ISO_PATH, BT_HCI_BIT_LE_REMOVE_ISO_PATH,
				"LE Remove Isochronous Data Path",
				le_remove_iso_path_cmd,
				sizeof(struct bt_hci_cmd_le_remove_iso_path),
				true, status_rsp, 1, true },
	{ BT_HCI_CMD_LE_ISO_TX_TEST, BT_HCI_BIT_LE_ISO_TX_TEST,
				"LE Isochronous Transmit Test", NULL, 0,
				false },
	{ BT_HCI_CMD_LE_ISO_RX_TEST, BT_HCI_BIT_LE_ISO_RX_TEST,
				"LE Isochronous Receive Test", NULL, 0,
				false },
	{ BT_HCI_CMD_LE_ISO_READ_TEST_COUNTER,
				BT_HCI_BIT_LE_ISO_READ_TEST_COUNTER,
				"LE Isochronous Read Test Counters", NULL, 0,
				false },
	{ BT_HCI_CMD_LE_ISO_TEST_END, BT_HCI_BIT_LE_ISO_TEST_END,
				"LE Isochronous Read Test Counters", NULL, 0,
				false },
	{ BT_HCI_CMD_LE_SET_HOST_FEATURE, BT_HCI_BIT_LE_SET_HOST_FEATURE,
				"LE Set Host Feature", le_set_host_feature_cmd,
				sizeof(struct bt_hci_cmd_le_set_host_feature),
				true, status_rsp, 1, true },
	{ }
};

static const char *get_supported_command(int bit)
{
	int i;

	for (i = 0; opcode_table[i].str; i++) {
		if (opcode_table[i].bit == bit)
			return opcode_table[i].str;
	}

	return NULL;
}

static const char *current_vendor_str(void)
{
	uint16_t manufacturer;

	if (index_current < MAX_INDEX)
		manufacturer = index_list[index_current].manufacturer;
	else
		manufacturer = fallback_manufacturer;

	switch (manufacturer) {
	case 2:
		return "Intel";
	case 15:
		return "Broadcom";
	}

	return NULL;
}

static const struct vendor_ocf *current_vendor_ocf(uint16_t ocf)
{
	uint16_t manufacturer;

	if (index_current < MAX_INDEX)
		manufacturer = index_list[index_current].manufacturer;
	else
		manufacturer = fallback_manufacturer;

	switch (manufacturer) {
	case 2:
		return intel_vendor_ocf(ocf);
	case 15:
		return broadcom_vendor_ocf(ocf);
	}

	return NULL;
}

static const struct vendor_evt *current_vendor_evt(uint8_t evt)
{
	uint16_t manufacturer;

	if (index_current < MAX_INDEX)
		manufacturer = index_list[index_current].manufacturer;
	else
		manufacturer = fallback_manufacturer;

	switch (manufacturer) {
	case 2:
		return intel_vendor_evt(evt);
	case 15:
		return broadcom_vendor_evt(evt);
	}

	return NULL;
}

static void inquiry_complete_evt(const void *data, uint8_t size)
{
	const struct bt_hci_evt_inquiry_complete *evt = data;

	print_status(evt->status);
}

static void inquiry_result_evt(const void *data, uint8_t size)
{
	const struct bt_hci_evt_inquiry_result *evt = data;

	print_num_resp(evt->num_resp);
	print_bdaddr(evt->bdaddr);
	print_pscan_rep_mode(evt->pscan_rep_mode);
	print_pscan_period_mode(evt->pscan_period_mode);
	print_pscan_mode(evt->pscan_mode);
	print_dev_class(evt->dev_class);
	print_clock_offset(evt->clock_offset);

	if (size > sizeof(*evt))
		packet_hexdump(data + sizeof(*evt), size - sizeof(*evt));
}

static void conn_complete_evt(const void *data, uint8_t size)
{
	const struct bt_hci_evt_conn_complete *evt = data;

	print_status(evt->status);
	print_handle(evt->handle);
	print_bdaddr(evt->bdaddr);
	print_link_type(evt->link_type);
	print_enable("Encryption", evt->encr_mode);

	if (evt->status == 0x00)
		assign_handle(le16_to_cpu(evt->handle), 0x00);
}

static void conn_request_evt(const void *data, uint8_t size)
{
	const struct bt_hci_evt_conn_request *evt = data;

	print_bdaddr(evt->bdaddr);
	print_dev_class(evt->dev_class);
	print_link_type(evt->link_type);
}

static void disconnect_complete_evt(const void *data, uint8_t size)
{
	const struct bt_hci_evt_disconnect_complete *evt = data;

	print_status(evt->status);
	print_handle(evt->handle);
	print_reason(evt->reason);

	if (evt->status == 0x00)
		release_handle(le16_to_cpu(evt->handle));
}

static void auth_complete_evt(const void *data, uint8_t size)
{
	const struct bt_hci_evt_auth_complete *evt = data;

	print_status(evt->status);
	print_handle(evt->handle);
}

static void remote_name_request_complete_evt(const void *data, uint8_t size)
{
	const struct bt_hci_evt_remote_name_request_complete *evt = data;

	print_status(evt->status);
	print_bdaddr(evt->bdaddr);
	print_name(evt->name);
}

static void encrypt_change_evt(const void *data, uint8_t size)
{
	const struct bt_hci_evt_encrypt_change *evt = data;

	print_status(evt->status);
	print_handle(evt->handle);
	print_encr_mode_change(evt->encr_mode, evt->handle);
}

static void change_conn_link_key_complete_evt(const void *data, uint8_t size)
{
	const struct bt_hci_evt_change_conn_link_key_complete *evt = data;

	print_status(evt->status);
	print_handle(evt->handle);
}

static void master_link_key_complete_evt(const void *data, uint8_t size)
{
	const struct bt_hci_evt_master_link_key_complete *evt = data;

	print_status(evt->status);
	print_handle(evt->handle);
	print_key_flag(evt->key_flag);
}

static void remote_features_complete_evt(const void *data, uint8_t size)
{
	const struct bt_hci_evt_remote_features_complete *evt = data;

	print_status(evt->status);
	print_handle(evt->handle);
	print_features(0, evt->features, 0x00);
}

static void remote_version_complete_evt(const void *data, uint8_t size)
{
	const struct bt_hci_evt_remote_version_complete *evt = data;

	print_status(evt->status);
	print_handle(evt->handle);
	print_lmp_version(evt->lmp_ver, evt->lmp_subver);
	print_manufacturer(evt->manufacturer);

	switch (le16_to_cpu(evt->manufacturer)) {
	case 15:
		print_manufacturer_broadcom(evt->lmp_subver, 0xffff);
		break;
	}
}

static void qos_setup_complete_evt(const void *data, uint8_t size)
{
	const struct bt_hci_evt_qos_setup_complete *evt = data;

	print_status(evt->status);
	print_handle(evt->handle);
	print_field("Flags: 0x%2.2x", evt->flags);

	print_service_type(evt->service_type);

	print_field("Token rate: %d", le32_to_cpu(evt->token_rate));
	print_field("Peak bandwidth: %d", le32_to_cpu(evt->peak_bandwidth));
	print_field("Latency: %d", le32_to_cpu(evt->latency));
	print_field("Delay variation: %d", le32_to_cpu(evt->delay_variation));
}

static void cmd_complete_evt(const void *data, uint8_t size)
{
	const struct bt_hci_evt_cmd_complete *evt = data;
	uint16_t opcode = le16_to_cpu(evt->opcode);
	uint16_t ogf = cmd_opcode_ogf(opcode);
	uint16_t ocf = cmd_opcode_ocf(opcode);
	struct opcode_data vendor_data;
	const struct opcode_data *opcode_data = NULL;
	const char *opcode_color, *opcode_str;
	char vendor_str[150];
	int i;

	for (i = 0; opcode_table[i].str; i++) {
		if (opcode_table[i].opcode == opcode) {
			opcode_data = &opcode_table[i];
			break;
		}
	}

	if (opcode_data) {
		if (opcode_data->rsp_func)
			opcode_color = COLOR_HCI_COMMAND;
		else
			opcode_color = COLOR_HCI_COMMAND_UNKNOWN;
		opcode_str = opcode_data->str;
	} else {
		if (ogf == 0x3f) {
			const struct vendor_ocf *vnd = current_vendor_ocf(ocf);

			if (vnd) {
				const char *str = current_vendor_str();

				if (str) {
					snprintf(vendor_str, sizeof(vendor_str),
							"%s %s", str, vnd->str);
					vendor_data.str = vendor_str;
				} else
					vendor_data.str = vnd->str;
				vendor_data.rsp_func = vnd->rsp_func;
				vendor_data.rsp_size = vnd->rsp_size;
				vendor_data.rsp_fixed = vnd->rsp_fixed;

				opcode_data = &vendor_data;

				if (opcode_data->rsp_func)
					opcode_color = COLOR_HCI_COMMAND;
				else
					opcode_color = COLOR_HCI_COMMAND_UNKNOWN;
				opcode_str = opcode_data->str;
			} else {
				opcode_color = COLOR_HCI_COMMAND;
				opcode_str = "Vendor";
			}
		} else {
			opcode_color = COLOR_HCI_COMMAND_UNKNOWN;
			opcode_str = "Unknown";
		}
	}

	print_indent(6, opcode_color, "", opcode_str, COLOR_OFF,
			" (0x%2.2x|0x%4.4x) ncmd %d", ogf, ocf, evt->ncmd);

	if (!opcode_data || !opcode_data->rsp_func) {
		if (size > 3) {
			uint8_t status = *((uint8_t *) (data + 3));

			print_status(status);
			packet_hexdump(data + 4, size - 4);
		}
		return;
	}

	if (opcode_data->rsp_size > 1 && size - 3 == 1) {
		uint8_t status = *((uint8_t *) (data + 3));

		print_status(status);
		return;
	}

	if (opcode_data->rsp_fixed) {
		if (size - 3 != opcode_data->rsp_size) {
			print_text(COLOR_ERROR, "invalid packet size");
			packet_hexdump(data + 3, size - 3);
			return;
		}
	} else {
		if (size - 3 < opcode_data->rsp_size) {
			print_text(COLOR_ERROR, "too short packet");
			packet_hexdump(data + 3, size - 3);
			return;
		}
	}

	opcode_data->rsp_func(data + 3, size - 3);
}

static void cmd_status_evt(const void *data, uint8_t size)
{
	const struct bt_hci_evt_cmd_status *evt = data;
	uint16_t opcode = le16_to_cpu(evt->opcode);
	uint16_t ogf = cmd_opcode_ogf(opcode);
	uint16_t ocf = cmd_opcode_ocf(opcode);
	const struct opcode_data *opcode_data = NULL;
	const char *opcode_color, *opcode_str;
	char vendor_str[150];
	int i;

	for (i = 0; opcode_table[i].str; i++) {
		if (opcode_table[i].opcode == opcode) {
			opcode_data = &opcode_table[i];
			break;
		}
	}

	if (opcode_data) {
		opcode_color = COLOR_HCI_COMMAND;
		opcode_str = opcode_data->str;
	} else {
		if (ogf == 0x3f) {
			const struct vendor_ocf *vnd = current_vendor_ocf(ocf);

			if (vnd) {
				const char *str = current_vendor_str();

				if (str) {
					snprintf(vendor_str, sizeof(vendor_str),
							"%s %s", str, vnd->str);
					opcode_str = vendor_str;
				} else
					opcode_str = vnd->str;

				opcode_color = COLOR_HCI_COMMAND;
			} else {
				opcode_color = COLOR_HCI_COMMAND;
				opcode_str = "Vendor";
			}
		} else {
			opcode_color = COLOR_HCI_COMMAND_UNKNOWN;
			opcode_str = "Unknown";
		}
	}

	print_indent(6, opcode_color, "", opcode_str, COLOR_OFF,
			" (0x%2.2x|0x%4.4x) ncmd %d", ogf, ocf, evt->ncmd);

	print_status(evt->status);
}

static void hardware_error_evt(const void *data, uint8_t size)
{
	const struct bt_hci_evt_hardware_error *evt = data;

	print_field("Code: 0x%2.2x", evt->code);
}

static void flush_occurred_evt(const void *data, uint8_t size)
{
	const struct bt_hci_evt_flush_occurred *evt = data;

	print_handle(evt->handle);
}

static void role_change_evt(const void *data, uint8_t size)
{
	const struct bt_hci_evt_role_change *evt = data;

	print_status(evt->status);
	print_bdaddr(evt->bdaddr);
	print_role(evt->role);
}

static void num_completed_packets_evt(const void *data, uint8_t size)
{
	const struct bt_hci_evt_num_completed_packets *evt = data;

	print_field("Num handles: %d", evt->num_handles);
	print_handle(evt->handle);
	print_field("Count: %d", le16_to_cpu(evt->count));

	if (size > sizeof(*evt))
		packet_hexdump(data + sizeof(*evt), size - sizeof(*evt));
}

static void mode_change_evt(const void *data, uint8_t size)
{
	const struct bt_hci_evt_mode_change *evt = data;

	print_status(evt->status);
	print_handle(evt->handle);
	print_mode(evt->mode);
	print_interval(evt->interval);
}

static void return_link_keys_evt(const void *data, uint8_t size)
{
	const struct bt_hci_evt_return_link_keys *evt = data;
	uint8_t i;

	print_field("Num keys: %d", evt->num_keys);

	for (i = 0; i < evt->num_keys; i++) {
		print_bdaddr(evt->keys + (i * 22));
		print_link_key(evt->keys + (i * 22) + 6);
	}
}

static void pin_code_request_evt(const void *data, uint8_t size)
{
	const struct bt_hci_evt_pin_code_request *evt = data;

	print_bdaddr(evt->bdaddr);
}

static void link_key_request_evt(const void *data, uint8_t size)
{
	const struct bt_hci_evt_link_key_request *evt = data;

	print_bdaddr(evt->bdaddr);
}

static void link_key_notify_evt(const void *data, uint8_t size)
{
	const struct bt_hci_evt_link_key_notify *evt = data;

	print_bdaddr(evt->bdaddr);
	print_link_key(evt->link_key);
	print_key_type(evt->key_type);
}

static void loopback_command_evt(const void *data, uint8_t size)
{
	packet_hexdump(data, size);
}

static void data_buffer_overflow_evt(const void *data, uint8_t size)
{
	const struct bt_hci_evt_data_buffer_overflow *evt = data;

	print_link_type(evt->link_type);
}

static void max_slots_change_evt(const void *data, uint8_t size)
{
	const struct bt_hci_evt_max_slots_change *evt = data;

	print_handle(evt->handle);
	print_field("Max slots: %d", evt->max_slots);
}

static void clock_offset_complete_evt(const void *data, uint8_t size)
{
	const struct bt_hci_evt_clock_offset_complete *evt = data;

	print_status(evt->status);
	print_handle(evt->handle);
	print_clock_offset(evt->clock_offset);
}

static void conn_pkt_type_changed_evt(const void *data, uint8_t size)
{
	const struct bt_hci_evt_conn_pkt_type_changed *evt = data;

	print_status(evt->status);
	print_handle(evt->handle);
	print_pkt_type(evt->pkt_type);
}

static void qos_violation_evt(const void *data, uint8_t size)
{
	const struct bt_hci_evt_qos_violation *evt = data;

	print_handle(evt->handle);
}

static void pscan_mode_change_evt(const void *data, uint8_t size)
{
	const struct bt_hci_evt_pscan_mode_change *evt = data;

	print_bdaddr(evt->bdaddr);
	print_pscan_mode(evt->pscan_mode);
}

static void pscan_rep_mode_change_evt(const void *data, uint8_t size)
{
	const struct bt_hci_evt_pscan_rep_mode_change *evt = data;

	print_bdaddr(evt->bdaddr);
	print_pscan_rep_mode(evt->pscan_rep_mode);
}

static void flow_spec_complete_evt(const void *data, uint8_t size)
{
	const struct bt_hci_evt_flow_spec_complete *evt = data;

	print_status(evt->status);
	print_handle(evt->handle);
	print_field("Flags: 0x%2.2x", evt->flags);

	print_flow_direction(evt->direction);
	print_service_type(evt->service_type);

	print_field("Token rate: %d", le32_to_cpu(evt->token_rate));
	print_field("Token bucket size: %d",
					le32_to_cpu(evt->token_bucket_size));
	print_field("Peak bandwidth: %d", le32_to_cpu(evt->peak_bandwidth));
	print_field("Access latency: %d", le32_to_cpu(evt->access_latency));
}

static void inquiry_result_with_rssi_evt(const void *data, uint8_t size)
{
	const struct bt_hci_evt_inquiry_result_with_rssi *evt = data;

	print_num_resp(evt->num_resp);
	print_bdaddr(evt->bdaddr);
	print_pscan_rep_mode(evt->pscan_rep_mode);
	print_pscan_period_mode(evt->pscan_period_mode);
	print_dev_class(evt->dev_class);
	print_clock_offset(evt->clock_offset);
	print_rssi(evt->rssi);

	if (size > sizeof(*evt))
		packet_hexdump(data + sizeof(*evt), size - sizeof(*evt));
}

static void remote_ext_features_complete_evt(const void *data, uint8_t size)
{
	const struct bt_hci_evt_remote_ext_features_complete *evt = data;

	print_status(evt->status);
	print_handle(evt->handle);
	print_field("Page: %d/%d", evt->page, evt->max_page);
	print_features(evt->page, evt->features, 0x00);
}

static void sync_conn_complete_evt(const void *data, uint8_t size)
{
	const struct bt_hci_evt_sync_conn_complete *evt = data;

	print_status(evt->status);
	print_handle(evt->handle);
	print_bdaddr(evt->bdaddr);
	print_link_type(evt->link_type);
	print_field("Transmission interval: 0x%2.2x", evt->tx_interval);
	print_field("Retransmission window: 0x%2.2x", evt->retrans_window);
	print_field("RX packet length: %d", le16_to_cpu(evt->rx_pkt_len));
	print_field("TX packet length: %d", le16_to_cpu(evt->tx_pkt_len));
	print_air_mode(evt->air_mode);
}

static void sync_conn_changed_evt(const void *data, uint8_t size)
{
	const struct bt_hci_evt_sync_conn_changed *evt = data;

	print_status(evt->status);
	print_handle(evt->handle);
	print_field("Transmission interval: 0x%2.2x", evt->tx_interval);
	print_field("Retransmission window: 0x%2.2x", evt->retrans_window);
	print_field("RX packet length: %d", le16_to_cpu(evt->rx_pkt_len));
	print_field("TX packet length: %d", le16_to_cpu(evt->tx_pkt_len));
}

static void sniff_subrating_evt(const void *data, uint8_t size)
{
	const struct bt_hci_evt_sniff_subrating *evt = data;

	print_status(evt->status);
	print_handle(evt->handle);
	print_slot_625("Max transmit latency", evt->max_tx_latency);
	print_slot_625("Max receive latency", evt->max_rx_latency);
	print_slot_625("Min remote timeout", evt->min_remote_timeout);
	print_slot_625("Min local timeout", evt->min_local_timeout);
}

static void ext_inquiry_result_evt(const void *data, uint8_t size)
{
	const struct bt_hci_evt_ext_inquiry_result *evt = data;

	print_num_resp(evt->num_resp);
	print_bdaddr(evt->bdaddr);
	print_pscan_rep_mode(evt->pscan_rep_mode);
	print_pscan_period_mode(evt->pscan_period_mode);
	print_dev_class(evt->dev_class);
	print_clock_offset(evt->clock_offset);
	print_rssi(evt->rssi);
	print_eir(evt->data, sizeof(evt->data), false);
}

static void encrypt_key_refresh_complete_evt(const void *data, uint8_t size)
{
	const struct bt_hci_evt_encrypt_key_refresh_complete *evt = data;

	print_status(evt->status);
	print_handle(evt->handle);
}

static void io_capability_request_evt(const void *data, uint8_t size)
{
	const struct bt_hci_evt_io_capability_request *evt = data;

	print_bdaddr(evt->bdaddr);
}

static void io_capability_response_evt(const void *data, uint8_t size)
{
	const struct bt_hci_evt_io_capability_response *evt = data;

	print_bdaddr(evt->bdaddr);
	print_io_capability(evt->capability);
	print_oob_data_response(evt->oob_data);
	print_authentication(evt->authentication);
}

static void user_confirm_request_evt(const void *data, uint8_t size)
{
	const struct bt_hci_evt_user_confirm_request *evt = data;

	print_bdaddr(evt->bdaddr);
	print_passkey(evt->passkey);
}

static void user_passkey_request_evt(const void *data, uint8_t size)
{
	const struct bt_hci_evt_user_passkey_request *evt = data;

	print_bdaddr(evt->bdaddr);
}

static void remote_oob_data_request_evt(const void *data, uint8_t size)
{
	const struct bt_hci_evt_remote_oob_data_request *evt = data;

	print_bdaddr(evt->bdaddr);
}

static void simple_pairing_complete_evt(const void *data, uint8_t size)
{
	const struct bt_hci_evt_simple_pairing_complete *evt = data;

	print_status(evt->status);
	print_bdaddr(evt->bdaddr);
}

static void link_supv_timeout_changed_evt(const void *data, uint8_t size)
{
	const struct bt_hci_evt_link_supv_timeout_changed *evt = data;

	print_handle(evt->handle);
	print_timeout(evt->timeout);
}

static void enhanced_flush_complete_evt(const void *data, uint8_t size)
{
	const struct bt_hci_evt_enhanced_flush_complete *evt = data;

	print_handle(evt->handle);
}

static void user_passkey_notify_evt(const void *data, uint8_t size)
{
	const struct bt_hci_evt_user_passkey_notify *evt = data;

	print_bdaddr(evt->bdaddr);
	print_passkey(evt->passkey);
}

static void keypress_notify_evt(const void *data, uint8_t size)
{
	const struct bt_hci_evt_keypress_notify *evt = data;
	const char *str;

	print_bdaddr(evt->bdaddr);

	switch (evt->type) {
	case 0x00:
		str = "Passkey entry started";
		break;
	case 0x01:
		str = "Passkey digit entered";
		break;
	case 0x02:
		str = "Passkey digit erased";
		break;
	case 0x03:
		str = "Passkey clared";
		break;
	case 0x04:
		str = "Passkey entry completed";
		break;
	default:
		str = "Reserved";
		break;
	}

	print_field("Notification type: %s (0x%2.2x)", str, evt->type);
}

static void remote_host_features_notify_evt(const void *data, uint8_t size)
{
	const struct bt_hci_evt_remote_host_features_notify *evt = data;

	print_bdaddr(evt->bdaddr);
	print_features(1, evt->features, 0x00);
}

static void phy_link_complete_evt(const void *data, uint8_t size)
{
	const struct bt_hci_evt_phy_link_complete *evt = data;

	print_status(evt->status);
	print_phy_handle(evt->phy_handle);
}

static void channel_selected_evt(const void *data, uint8_t size)
{
	const struct bt_hci_evt_channel_selected *evt = data;

	print_phy_handle(evt->phy_handle);
}

static void disconn_phy_link_complete_evt(const void *data, uint8_t size)
{
	const struct bt_hci_evt_disconn_phy_link_complete *evt = data;

	print_status(evt->status);
	print_phy_handle(evt->phy_handle);
	print_reason(evt->reason);
}

static void phy_link_loss_early_warning_evt(const void *data, uint8_t size)
{
	const struct bt_hci_evt_phy_link_loss_early_warning *evt = data;
	const char *str;

	print_phy_handle(evt->phy_handle);

	switch (evt->reason) {
	case 0x00:
		str = "Unknown";
		break;
	case 0x01:
		str = "Range related";
		break;
	case 0x02:
		str = "Bandwidth related";
		break;
	case 0x03:
		str = "Resolving conflict";
		break;
	case 0x04:
		str = "Interference";
		break;
	default:
		str = "Reserved";
		break;
	}

	print_field("Reason: %s (0x%2.2x)", str, evt->reason);
}

static void phy_link_recovery_evt(const void *data, uint8_t size)
{
	const struct bt_hci_evt_phy_link_recovery *evt = data;

	print_phy_handle(evt->phy_handle);
}

static void logic_link_complete_evt(const void *data, uint8_t size)
{
	const struct bt_hci_evt_logic_link_complete *evt = data;

	print_status(evt->status);
	print_handle(evt->handle);
	print_phy_handle(evt->phy_handle);
	print_field("TX flow spec: 0x%2.2x", evt->flow_spec);
}

static void disconn_logic_link_complete_evt(const void *data, uint8_t size)
{
	const struct bt_hci_evt_disconn_logic_link_complete *evt = data;

	print_status(evt->status);
	print_handle(evt->handle);
	print_reason(evt->reason);
}

static void flow_spec_modify_complete_evt(const void *data, uint8_t size)
{
	const struct bt_hci_evt_flow_spec_modify_complete *evt = data;

	print_status(evt->status);
	print_handle(evt->handle);
}

static void num_completed_data_blocks_evt(const void *data, uint8_t size)
{
	const struct bt_hci_evt_num_completed_data_blocks *evt = data;

	print_field("Total num data blocks: %d",
				le16_to_cpu(evt->total_num_blocks));
	print_field("Num handles: %d", evt->num_handles);
	print_handle(evt->handle);
	print_field("Num packets: %d", evt->num_packets);
	print_field("Num blocks: %d", evt->num_blocks);

	if (size > sizeof(*evt))
		packet_hexdump(data + sizeof(*evt), size - sizeof(*evt));
}

static void short_range_mode_change_evt(const void *data, uint8_t size)
{
	const struct bt_hci_evt_short_range_mode_change *evt = data;

	print_status(evt->status);
	print_phy_handle(evt->phy_handle);
	print_enable("Short range mode", evt->mode);
}

static void amp_status_change_evt(const void *data, uint8_t size)
{
	const struct bt_hci_evt_amp_status_change *evt = data;

	print_status(evt->status);
	print_amp_status(evt->amp_status);
}

static void triggered_clock_capture_evt(const void *data, uint8_t size)
{
	const struct bt_hci_evt_triggered_clock_capture *evt = data;

	print_handle(evt->handle);
	print_clock_type(evt->type);
	print_clock(evt->clock);
	print_clock_offset(evt->clock_offset);
}

static void sync_train_complete_evt(const void *data, uint8_t size)
{
	const struct bt_hci_evt_sync_train_complete *evt = data;

	print_status(evt->status);
}

static void sync_train_received_evt(const void *data, uint8_t size)
{
	const struct bt_hci_evt_sync_train_received *evt = data;

	print_status(evt->status);
	print_bdaddr(evt->bdaddr);
	print_field("Offset: 0x%8.8x", le32_to_cpu(evt->offset));
	print_channel_map(evt->map);
	print_lt_addr(evt->lt_addr);
	print_field("Next broadcast instant: 0x%4.4x",
					le16_to_cpu(evt->instant));
	print_interval(evt->interval);
	print_field("Service Data: 0x%2.2x", evt->service_data);
}

static void slave_broadcast_receive_evt(const void *data, uint8_t size)
{
	const struct bt_hci_evt_slave_broadcast_receive *evt = data;

	print_bdaddr(evt->bdaddr);
	print_lt_addr(evt->lt_addr);
	print_field("Clock: 0x%8.8x", le32_to_cpu(evt->clock));
	print_field("Offset: 0x%8.8x", le32_to_cpu(evt->offset));
	print_field("Receive status: 0x%2.2x", evt->status);
	print_broadcast_fragment(evt->fragment);
	print_field("Length: %d", evt->length);

	if (size - 18 != evt->length)
		print_text(COLOR_ERROR, "invalid data size (%d != %d)",
						size - 18, evt->length);

	if (evt->lt_addr == 0x01 && evt->length == 17)
		print_3d_broadcast(data + 18, size - 18);
	else
		packet_hexdump(data + 18, size - 18);
}

static void slave_broadcast_timeout_evt(const void *data, uint8_t size)
{
	const struct bt_hci_evt_slave_broadcast_timeout *evt = data;

	print_bdaddr(evt->bdaddr);
	print_lt_addr(evt->lt_addr);
}

static void truncated_page_complete_evt(const void *data, uint8_t size)
{
	const struct bt_hci_evt_truncated_page_complete *evt = data;

	print_status(evt->status);
	print_bdaddr(evt->bdaddr);
}

static void slave_page_response_timeout_evt(const void *data, uint8_t size)
{
}

static void slave_broadcast_channel_map_change_evt(const void *data, uint8_t size)
{
	const struct bt_hci_evt_slave_broadcast_channel_map_change *evt = data;

	print_channel_map(evt->map);
}

static void inquiry_response_notify_evt(const void *data, uint8_t size)
{
	const struct bt_hci_evt_inquiry_response_notify *evt = data;

	print_iac(evt->lap);
	print_rssi(evt->rssi);
}

static void auth_payload_timeout_expired_evt(const void *data, uint8_t size)
{
	const struct bt_hci_evt_auth_payload_timeout_expired *evt = data;

	print_handle(evt->handle);
}

static void le_conn_complete_evt(const void *data, uint8_t size)
{
	const struct bt_hci_evt_le_conn_complete *evt = data;

	print_status(evt->status);
	print_handle(evt->handle);
	print_role(evt->role);
	print_peer_addr_type("Peer address type", evt->peer_addr_type);
	print_addr("Peer address", evt->peer_addr, evt->peer_addr_type);
	print_slot_125("Connection interval", evt->interval);
	print_conn_latency("Connection latency", evt->latency);
	print_field("Supervision timeout: %d msec (0x%4.4x)",
					le16_to_cpu(evt->supv_timeout) * 10,
					le16_to_cpu(evt->supv_timeout));
	print_field("Master clock accuracy: 0x%2.2x", evt->clock_accuracy);

	if (evt->status == 0x00)
		assign_handle(le16_to_cpu(evt->handle), 0x01);
}

static void le_adv_report_evt(const void *data, uint8_t size)
{
	const struct bt_hci_evt_le_adv_report *evt = data;
	uint8_t evt_len;
	int8_t *rssi;

	print_num_reports(evt->num_reports);

report:
	print_adv_event_type("Event type", evt->event_type);
	print_peer_addr_type("Address type", evt->addr_type);
	print_addr("Address", evt->addr, evt->addr_type);
	print_field("Data length: %d", evt->data_len);
	print_eir(evt->data, evt->data_len, true);

	rssi = (int8_t *) (evt->data + evt->data_len);
	print_rssi(*rssi);

	evt_len = sizeof(*evt) + evt->data_len + 1;

	if (size > evt_len) {
		data += evt_len - 1;
		size -= evt_len - 1;
		evt = data;
		goto report;
	}
}

static void le_conn_update_complete_evt(const void *data, uint8_t size)
{
	const struct bt_hci_evt_le_conn_update_complete *evt = data;

	print_status(evt->status);
	print_handle(evt->handle);
	print_slot_125("Connection interval", evt->interval);
	print_conn_latency("Connection latency", evt->latency);
	print_field("Supervision timeout: %d msec (0x%4.4x)",
					le16_to_cpu(evt->supv_timeout) * 10,
					le16_to_cpu(evt->supv_timeout));
}

static void le_remote_features_complete_evt(const void *data, uint8_t size)
{
	const struct bt_hci_evt_le_remote_features_complete *evt = data;

	print_status(evt->status);
	print_handle(evt->handle);
	print_features(0, evt->features, 0x01);
}

static void le_long_term_key_request_evt(const void *data, uint8_t size)
{
	const struct bt_hci_evt_le_long_term_key_request *evt = data;

	print_handle(evt->handle);
	print_random_number(evt->rand);
	print_encrypted_diversifier(evt->ediv);
}

static void le_conn_param_request_evt(const void *data, uint8_t size)
{
	const struct bt_hci_evt_le_conn_param_request *evt = data;

	print_handle(evt->handle);
	print_slot_125("Min connection interval", evt->min_interval);
	print_slot_125("Max connection interval", evt->max_interval);
	print_conn_latency("Connection latency", evt->latency);
	print_field("Supervision timeout: %d msec (0x%4.4x)",
					le16_to_cpu(evt->supv_timeout) * 10,
					le16_to_cpu(evt->supv_timeout));
}

static void le_data_length_change_evt(const void *data, uint8_t size)
{
	const struct bt_hci_evt_le_data_length_change *evt = data;

	print_handle(evt->handle);
	print_field("Max TX octets: %d", le16_to_cpu(evt->max_tx_len));
	print_field("Max TX time: %d", le16_to_cpu(evt->max_tx_time));
	print_field("Max RX octets: %d", le16_to_cpu(evt->max_rx_len));
	print_field("Max RX time: %d", le16_to_cpu(evt->max_rx_time));
}

static void le_read_local_pk256_complete_evt(const void *data, uint8_t size)
{
	const struct bt_hci_evt_le_read_local_pk256_complete *evt = data;

	print_status(evt->status);
	print_pk256("Local P-256 public key", evt->local_pk256);
}

static void le_generate_dhkey_complete_evt(const void *data, uint8_t size)
{
	const struct bt_hci_evt_le_generate_dhkey_complete *evt = data;

	print_status(evt->status);
	print_dhkey(evt->dhkey);
}

static void le_enhanced_conn_complete_evt(const void *data, uint8_t size)
{
	const struct bt_hci_evt_le_enhanced_conn_complete *evt = data;

	print_status(evt->status);
	print_handle(evt->handle);
	print_role(evt->role);
	print_peer_addr_type("Peer address type", evt->peer_addr_type);
	print_addr("Peer address", evt->peer_addr, evt->peer_addr_type);
	print_addr("Local resolvable private address", evt->local_rpa, 0x01);
	print_addr("Peer resolvable private address", evt->peer_rpa, 0x01);
	print_slot_125("Connection interval", evt->interval);
	print_conn_latency("Connection latency", evt->latency);
	print_field("Supervision timeout: %d msec (0x%4.4x)",
					le16_to_cpu(evt->supv_timeout) * 10,
					le16_to_cpu(evt->supv_timeout));
	print_field("Master clock accuracy: 0x%2.2x", evt->clock_accuracy);

	if (evt->status == 0x00)
		assign_handle(le16_to_cpu(evt->handle), 0x01);
}

static void le_direct_adv_report_evt(const void *data, uint8_t size)
{
	const struct bt_hci_evt_le_direct_adv_report *evt = data;

	print_num_reports(evt->num_reports);

	print_adv_event_type("Event type", evt->event_type);
	print_peer_addr_type("Address type", evt->addr_type);
	print_addr("Address", evt->addr, evt->addr_type);
	print_addr_type("Direct address type", evt->direct_addr_type);
	print_addr("Direct address", evt->direct_addr, evt->direct_addr_type);
	print_rssi(evt->rssi);

	if (size > sizeof(*evt))
		packet_hexdump(data + sizeof(*evt), size - sizeof(*evt));
}

static void le_phy_update_complete_evt(const void *data, uint8_t size)
{
	const struct bt_hci_evt_le_phy_update_complete *evt = data;

	print_status(evt->status);
	print_handle(evt->handle);
	print_le_phy("TX PHY", evt->tx_phy);
	print_le_phy("RX PHY", evt->rx_phy);
}

static const struct bitfield_data ext_adv_report_evt_type[] = {
	{  0, "Connectable"		},
	{  1, "Scannable"		},
	{  2, "Directed"	},
	{  3, "Scan response"	},
	{  4, "Use legacy advertising PDUs"	},
	{ }
};

static void print_ext_adv_report_evt_type(const char *indent, uint16_t flags)
{
	uint16_t mask = flags;
	uint16_t props = flags;
	uint8_t data_status;
	const char *str;
	const char *color_on;
	int i;

	print_field("%sEvent type: 0x%4.4x", indent, flags);

	props &= 0x1f;
	print_field("%s  Props: 0x%4.4x", indent, props);
	for (i = 0; ext_adv_report_evt_type[i].str; i++) {
		if (flags & (1 << ext_adv_report_evt_type[i].bit)) {
			print_field("%s    %s", indent,
						ext_adv_report_evt_type[i].str);
			mask &= ~(1 << ext_adv_report_evt_type[i].bit);
		}
	}

	data_status = (flags >> 5) & 3;
	mask &= ~(data_status << 5);

	switch (data_status) {
	case 0x00:
		str = "Complete";
		color_on = COLOR_GREEN;
		break;
	case 0x01:
		str = "Incomplete, more data to come";
		color_on = COLOR_YELLOW;
		break;
	case 0x02:
		str = "Incomplete, data truncated, no more to come";
		color_on = COLOR_RED;
		break;
	default:
		str = "Reserved";
		color_on = COLOR_RED;
		break;
	}

	print_field("%s  Data status: %s%s%s", indent, color_on, str, COLOR_OFF);

	if (mask)
		print_text(COLOR_UNKNOWN_ADV_FLAG,
				"%s  Reserved (0x%4.4x)", indent, mask);
}

static void print_legacy_adv_report_pdu(uint16_t flags)
{
	const char *str;

	if (!(flags & (1 << 4)))
		return;

	switch (flags) {
	case 0x10:
		str = "ADV_NONCONN_IND";
		break;
	case 0x12:
		str = "ADV_SCAN_IND";
		break;
	case 0x13:
		str = "ADV_IND";
		break;
	case 0x15:
		str = "ADV_DIRECT_IND";
		break;
	case 0x1a:
		str = "SCAN_RSP to an ADV_IND";
		break;
	case 0x1b:
		str = "SCAN_RSP to an ADV_SCAN_IND";
		break;
	default:
		str = "Reserved";
		break;
	}

	print_field("  Legacy PDU Type: %s (0x%4.4x)", str, flags);
}

static void le_ext_adv_report_evt(const void *data, uint8_t size)
{
	const struct bt_hci_evt_le_ext_adv_report *evt = data;
	const struct bt_hci_le_ext_adv_report *report;
	const char *str;
	int i;

	print_num_reports(evt->num_reports);

	data += sizeof(evt->num_reports);

	for (i = 0; i < evt->num_reports; ++i) {
		report = data;
		print_field("Entry %d", i);
		print_ext_adv_report_evt_type("  ", report->event_type);
		print_legacy_adv_report_pdu(report->event_type);
		print_peer_addr_type("  Address type", report->addr_type);
		print_addr("  Address", report->addr, report->addr_type);

		switch (report->primary_phy) {
		case 0x01:
			str = "LE 1M";
			break;
		case 0x03:
			str = "LE Coded";
			break;
		default:
			str = "Reserved";
			break;
		}

		print_field("  Primary PHY: %s", str);

		switch (report->secondary_phy) {
		case 0x00:
			str = "No packets";
			break;
		case 0x01:
			str = "LE 1M";
			break;
		case 0x02:
			str = "LE 2M";
			break;
		case 0x03:
			str = "LE Coded";
			break;
		default:
			str = "Reserved";
			break;
		}

		print_field("  Secondary PHY: %s", str);

		if (report->sid == 0xff)
			print_field("  SID: no ADI field (0x%2.2x)",
								report->sid);
		else if (report->sid > 0x0f)
			print_field("  SID: Reserved (0x%2.2x)", report->sid);
		else
			print_field("  SID: 0x%2.2x", report->sid);

		print_field("  TX power: %d dBm", report->tx_power);

		if (report->rssi == 127)
			print_field("  RSSI: not available (0x%2.2x)",
							(uint8_t) report->rssi);
		else if (report->rssi >= -127 && report->rssi <= 20)
			print_field("  RSSI: %d dBm (0x%2.2x)",
					report->rssi, (uint8_t) report->rssi);
		else
			print_field("  RSSI: reserved (0x%2.2x)",
							(uint8_t) report->rssi);

		print_slot_125("  Periodic advertising invteral",
							report->interval);
		print_peer_addr_type("  Direct address type",
						report->direct_addr_type);
		print_addr("  Direct address", report->direct_addr,
						report->direct_addr_type);
		print_field("  Data length: 0x%2.2x", report->data_len);
		data += sizeof(struct bt_hci_le_ext_adv_report);
		packet_hexdump(data, report->data_len);
		data += report->data_len;
	}
}

static void le_per_adv_sync(const void *data, uint8_t size)
{
	const struct bt_hci_evt_le_per_sync_established *evt = data;

	print_status(evt->status);
	print_field("Sync handle: %d", evt->handle);
	if (evt->sid > 0x0f)
		print_field("Advertising SID: Reserved (0x%2.2x)", evt->sid);
	else
		print_field("Advertising SID: 0x%2.2x", evt->sid);

	print_peer_addr_type("Advertiser address type", evt->addr_type);
	print_addr("Advertiser address", evt->addr, evt->addr_type);
	print_le_phy("Advertiser PHY", evt->phy);
	print_slot_125("Periodic advertising invteral", evt->interval);
	print_field("Advertiser clock accuracy: 0x%2.2x", evt->clock_accuracy);
}

static void le_per_adv_report_evt(const void *data, uint8_t size)
{
	const struct bt_hci_le_per_adv_report *evt = data;
	const char *color_on;
	const char *str;

	print_field("Sync handle: %d", evt->handle);
	print_power_level(evt->tx_power, NULL);
	if (evt->rssi == 127)
		print_field("RSSI: not available (0x%2.2x)",
						(uint8_t) evt->rssi);
	else if (evt->rssi >= -127 && evt->rssi <= 20)
		print_field("RSSI: %d dBm (0x%2.2x)",
				evt->rssi, (uint8_t) evt->rssi);
	else
		print_field("RSSI: reserved (0x%2.2x)",
						(uint8_t) evt->rssi);

	switch (evt->cte_type) {
	case 0x00:
		str = "AoA Constant Tone Extension";
		break;
	case 0x01:
		str = "AoA Constant Tone Extension with 1us slots";
		break;
	case 0x02:
		str = "AoD Constant Tone Extension with 2us slots";
		break;
	case 0xff:
		str = "No Constant Tone Extension";
		break;
	default:
		str = "Reserved";
		color_on = COLOR_RED;
		break;
	}

	switch (evt->data_status) {
	case 0x00:
		str = "Complete";
		color_on = COLOR_GREEN;
		break;
	case 0x01:
		str = "Incomplete, more data to come";
		color_on = COLOR_YELLOW;
		break;
	case 0x02:
		str = "Incomplete, data truncated, no more to come";
		color_on = COLOR_RED;
		break;
	default:
		str = "Reserved";
		color_on = COLOR_RED;
		break;
	}

	print_field("Data status: %s%s%s", color_on, str, COLOR_OFF);
	print_field("Data length: 0x%2.2x", evt->data_len);
	packet_hexdump(evt->data, evt->data_len);
}

static void le_per_adv_sync_lost(const void *data, uint8_t size)
{
	const struct bt_hci_evt_le_per_sync_lost *evt = data;

	print_field("Sync handle: %d", evt->handle);
}

static void le_adv_set_term_evt(const void *data, uint8_t size)
{
	const struct bt_hci_evt_le_adv_set_term *evt = data;

	print_status(evt->status);
	print_field("Handle: %d", evt->handle);
	print_field("Connection handle: %d", evt->conn_handle);
	print_field("Number of completed extended advertising events: %d",
			evt->num_evts);
}

static void le_scan_req_received_evt(const void *data, uint8_t size)
{
	const struct bt_hci_evt_le_scan_req_received *evt = data;

	print_field("Handle: %d", evt->handle);
	print_peer_addr_type("Scanner address type", evt->scanner_addr_type);
	print_addr("Scanner address", evt->scanner_addr,
							evt->scanner_addr_type);
}

static void le_chan_select_alg_evt(const void *data, uint8_t size)
{
	const struct bt_hci_evt_le_chan_select_alg *evt = data;
	const char *str;

	print_handle(evt->handle);

	switch (evt->algorithm) {
	case 0x00:
		str = "#1";
		break;
	case 0x01:
		str = "#2";
		break;
	default:
		str = "Reserved";
		break;
	}

	print_field("Algorithm: %s (0x%2.2x)", str, evt->algorithm);
}

static void le_cte_request_failed_evt(const void *data, uint8_t size)
{
	const struct bt_hci_evt_le_cte_request_failed *evt = data;

	print_status(evt->status);
	print_field("Connection handle: %d", evt->handle);
}

static void le_per_adv_sync_trans_rec_evt(const void *data, uint8_t size)
{
	const struct bt_hci_evt_le_per_adv_sync_trans_rec *evt = data;

	print_status(evt->status);
	print_field("Handle: %d", evt->handle);
	print_field("Connection handle: %d", evt->handle);
	print_field("Service data: 0x%4.4x", evt->service_data);
	print_field("Sync handle: %d", evt->sync_handle);
	print_field("SID: 0x%2.2x", evt->sid);
	print_peer_addr_type("Address type:", evt->addr_type);
	print_addr("Address:", evt->addr, evt->addr_type);
	print_le_phy("PHY:", evt->phy);
	print_field("Periodic advertising Interval: %.3f",
							1.25 * evt->interval);
	print_clock_accuracy(evt->clock_accuracy);
}

static void le_cis_established_evt(const void *data, uint8_t size)
{
	const struct bt_hci_evt_le_cis_established *evt = data;

	print_status(evt->status);
	print_field("Connection Handle: %d", le16_to_cpu(evt->conn_handle));
	print_usec_interval("CIG Synchronization Delay", evt->cig_sync_delay);
	print_usec_interval("CIS Synchronization Delay", evt->cis_sync_delay);
	print_usec_interval("Master to Slave Latency", evt->m_latency);
	print_usec_interval("Slave to Master Latency", evt->s_latency);
	print_le_phy("Master to Slave PHY", evt->m_phy);
	print_le_phy("Slave to Master PHY", evt->s_phy);
	print_field("Number of Subevents: %u", evt->nse);
	print_field("Master to Slave Burst Number: %u", evt->m_bn);
	print_field("Slave to Master Burst Number: %u", evt->s_bn);
	print_field("Master to Slave Flush Timeout: %u", evt->m_ft);
	print_field("Slave to Master Flush Timeout: %u", evt->s_ft);
	print_field("Master to Slave MTU: %u", le16_to_cpu(evt->m_mtu));
	print_field("Slave to Master MTU: %u", le16_to_cpu(evt->s_mtu));
	print_field("ISO Interval: %u", le16_to_cpu(evt->interval));
}

static void le_req_cis_evt(const void *data, uint8_t size)
{
	const struct bt_hci_evt_le_cis_req *evt = data;

	print_field("ACL Handle: %d", le16_to_cpu(evt->acl_handle));
	print_field("CIS Handle: %d", le16_to_cpu(evt->cis_handle));
	print_field("CIG ID: 0x%2.2x", evt->cig_id);
	print_field("CIS ID: 0x%2.2x", evt->cis_id);
}

static void print_bis_handle(const void *data)
{
	const uint16_t *handle = data;

	print_field("Connection Handle: %d", le16_to_cpu(*handle));
}

static void le_big_complete_evt(const void *data, uint8_t size)
{
	const struct bt_hci_evt_le_big_complete *evt = data;

	print_status(evt->status);
	print_field("BIG ID: 0x%2.2x", evt->big_id);
	print_usec_interval("BIG Synchronization Delay", evt->sync_delay);
	print_usec_interval("Transport Latency", evt->latency);
	print_le_phy("PHY", evt->phy);
	print_list(evt->handle, size, evt->num_bis, sizeof(*evt->handle),
						print_bis_handle);
}

static void le_big_terminate_evt(const void *data, uint8_t size)
{
	const struct bt_hci_evt_le_big_terminate *evt = data;

	print_reason(evt->reason);
	print_field("BIG ID: 0x%2.2x", evt->big_id);
}

static void le_big_sync_estabilished_evt(const void *data, uint8_t size)
{
	const struct bt_hci_evt_le_big_sync_estabilished *evt = data;

	print_status(evt->status);
	print_field("BIG ID: 0x%2.2x", evt->big_id);
	print_usec_interval("Transport Latency", evt->latency);
	print_list(evt->handle, size, evt->num_bis, sizeof(*evt->handle),
						print_bis_handle);
}

static void le_big_sync_lost_evt(const void *data, uint8_t size)
{
	const struct bt_hci_evt_le_big_sync_lost *evt = data;

	print_field("BIG ID: 0x%2.2x", evt->big_id);
	print_reason(evt->reason);
}

static void le_req_sca_complete_evt(const void *data, uint8_t size)
{
	const struct bt_hci_evt_le_req_peer_sca_complete *evt = data;

	print_status(evt->status);
	print_field("Connection Handle: %d", le16_to_cpu(evt->handle));
	print_sca(evt->sca);
}

struct subevent_data {
	uint8_t subevent;
	const char *str;
	void (*func) (const void *data, uint8_t size);
	uint8_t size;
	bool fixed;
};

static void print_subevent(const struct subevent_data *subevent_data,
					const void *data, uint8_t size)
{
	const char *subevent_color;

	if (subevent_data->func)
		subevent_color = COLOR_HCI_EVENT;
	else
		subevent_color = COLOR_HCI_EVENT_UNKNOWN;

	print_indent(6, subevent_color, "", subevent_data->str, COLOR_OFF,
					" (0x%2.2x)", subevent_data->subevent);

	if (!subevent_data->func) {
		packet_hexdump(data, size);
		return;
	}

	if (subevent_data->fixed) {
		if (size != subevent_data->size) {
			print_text(COLOR_ERROR, "invalid packet size");
			packet_hexdump(data, size);
			return;
		}
	} else {
		if (size < subevent_data->size) {
			print_text(COLOR_ERROR, "too short packet");
			packet_hexdump(data, size);
			return;
		}
	}

	subevent_data->func(data, size);
}

static const struct subevent_data le_meta_event_table[] = {
	{ 0x01, "LE Connection Complete",
				le_conn_complete_evt, 18, true },
	{ 0x02, "LE Advertising Report",
				le_adv_report_evt, 1, false },
	{ 0x03, "LE Connection Update Complete",
				le_conn_update_complete_evt, 9, true },
	{ 0x04, "LE Read Remote Used Features",
				le_remote_features_complete_evt, 11, true },
	{ 0x05, "LE Long Term Key Request",
				le_long_term_key_request_evt, 12, true },
	{ 0x06, "LE Remote Connection Parameter Request",
				le_conn_param_request_evt, 10, true },
	{ 0x07, "LE Data Length Change",
				le_data_length_change_evt, 10, true },
	{ 0x08, "LE Read Local P-256 Public Key Complete",
				le_read_local_pk256_complete_evt, 65, true },
	{ 0x09, "LE Generate DHKey Complete",
				le_generate_dhkey_complete_evt, 33, true },
	{ 0x0a, "LE Enhanced Connection Complete",
				le_enhanced_conn_complete_evt, 30, true },
	{ 0x0b, "LE Direct Advertising Report",
				le_direct_adv_report_evt, 1, false },
	{ 0x0c, "LE PHY Update Complete",
				le_phy_update_complete_evt, 5, true},
	{ 0x0d, "LE Extended Advertising Report",
				le_ext_adv_report_evt, 1, false},
	{ 0x0e, "LE Periodic Advertising Sync Established",
				le_per_adv_sync, 15, true },
	{ 0x0f, "LE Periodic Advertising Report",
				le_per_adv_report_evt, 7, false},
	{ 0x10, "LE Periodic Advertising Sync Lost",
				le_per_adv_sync_lost, 2, true},
	{ 0x11, "LE Scan Timeout" },
	{ 0x12, "LE Advertising Set Terminated",
				le_adv_set_term_evt, 5, true},
	{ 0x13, "LE Scan Request Received",
				le_scan_req_received_evt, 8, true},
	{ 0x14, "LE Channel Selection Algorithm",
				le_chan_select_alg_evt, 3, true},
	{ 0x17, "LE CTE Request Failed",
				le_cte_request_failed_evt, 3, true},
	{ 0x18, "LE Periodic Advertising Sync Transfer Received",
					le_per_adv_sync_trans_rec_evt, 19,
					true},
	{ BT_HCI_EVT_LE_CIS_ESTABLISHED,
				"LE Connected Isochronous Stream Established",
				le_cis_established_evt,
				sizeof(struct bt_hci_evt_le_cis_established),
				true },
	{ BT_HCI_EVT_LE_CIS_REQ, "LE Connected Isochronous Stream Request",
				le_req_cis_evt,
				sizeof(struct bt_hci_evt_le_cis_req),
				true },
	{ BT_HCI_EVT_LE_BIG_COMPLETE,
				"LE Broadcast Isochronous Group Complete",
				le_big_complete_evt,
				sizeof(struct bt_hci_evt_le_big_complete) },
	{ BT_HCI_EVT_LE_BIG_TERMINATE,
				"LE Broadcast Isochronous Group Terminate",
				le_big_terminate_evt,
				sizeof(struct bt_hci_evt_le_big_terminate) },
	{ BT_HCI_EVT_LE_BIG_SYNC_ESTABILISHED,
				"LE Broadcast Isochronous Group Sync "
				"Estabilished", le_big_sync_estabilished_evt,
				sizeof(struct bt_hci_evt_le_big_sync_lost) },
	{ BT_HCI_EVT_LE_BIG_SYNC_LOST,
				"LE Broadcast Isochronous Group Sync Lost",
				le_big_sync_lost_evt,
				sizeof(struct bt_hci_evt_le_big_sync_lost) },
	{ BT_HCI_EVT_LE_REQ_PEER_SCA_COMPLETE,
				"LE Request Peer SCA Complete",
				le_req_sca_complete_evt,
				sizeof(
				struct bt_hci_evt_le_req_peer_sca_complete)},
	{ }
};

static void le_meta_event_evt(const void *data, uint8_t size)
{
	uint8_t subevent = *((const uint8_t *) data);
	struct subevent_data unknown;
	const struct subevent_data *subevent_data = &unknown;
	int i;

	unknown.subevent = subevent;
	unknown.str = "Unknown";
	unknown.func = NULL;
	unknown.size = 0;
	unknown.fixed = true;

	for (i = 0; le_meta_event_table[i].str; i++) {
		if (le_meta_event_table[i].subevent == subevent) {
			subevent_data = &le_meta_event_table[i];
			break;
		}
	}

	print_subevent(subevent_data, data + 1, size - 1);
}

static void vendor_evt(const void *data, uint8_t size)
{
	uint8_t subevent = *((const uint8_t *) data);
	struct subevent_data vendor_data;
	char vendor_str[150];
	const struct vendor_evt *vnd = current_vendor_evt(subevent);

	if (vnd) {
		const char *str = current_vendor_str();

		if (str) {
			snprintf(vendor_str, sizeof(vendor_str),
						"%s %s", str, vnd->str);
			vendor_data.str = vendor_str;
		} else
			vendor_data.str = vnd->str;
		vendor_data.subevent = subevent;
		vendor_data.func = vnd->evt_func;
		vendor_data.size = vnd->evt_size;
		vendor_data.fixed = vnd->evt_fixed;

		print_subevent(&vendor_data, data + 1, size - 1);
	} else {
		uint16_t manufacturer;

		if (index_current < MAX_INDEX)
			manufacturer = index_list[index_current].manufacturer;
		else
			manufacturer = fallback_manufacturer;

		vendor_event(manufacturer, data, size);
	}
}

struct event_data {
	uint8_t event;
	const char *str;
	void (*func) (const void *data, uint8_t size);
	uint8_t size;
	bool fixed;
};

static const struct event_data event_table[] = {
	{ 0x01, "Inquiry Complete",
				inquiry_complete_evt, 1, true },
	{ 0x02, "Inquiry Result",
				inquiry_result_evt, 1, false },
	{ 0x03, "Connect Complete",
				conn_complete_evt, 11, true },
	{ 0x04, "Connect Request",
				conn_request_evt, 10, true },
	{ 0x05, "Disconnect Complete",
				disconnect_complete_evt, 4, true },
	{ 0x06, "Auth Complete",
				auth_complete_evt, 3, true },
	{ 0x07, "Remote Name Req Complete",
				remote_name_request_complete_evt, 255, true },
	{ 0x08, "Encryption Change",
				encrypt_change_evt, 4, true },
	{ 0x09, "Change Connection Link Key Complete",
				change_conn_link_key_complete_evt, 3, true },
	{ 0x0a, "Master Link Key Complete",
				master_link_key_complete_evt, 4, true },
	{ 0x0b, "Read Remote Supported Features",
				remote_features_complete_evt, 11, true },
	{ 0x0c, "Read Remote Version Complete",
				remote_version_complete_evt, 8, true },
	{ 0x0d, "QoS Setup Complete",
				qos_setup_complete_evt, 21, true },
	{ 0x0e, "Command Complete",
				cmd_complete_evt, 3, false },
	{ 0x0f, "Command Status",
				cmd_status_evt, 4, true },
	{ 0x10, "Hardware Error",
				hardware_error_evt, 1, true },
	{ 0x11, "Flush Occurred",
				flush_occurred_evt, 2, true },
	{ 0x12, "Role Change",
				role_change_evt, 8, true },
	{ 0x13, "Number of Completed Packets",
				num_completed_packets_evt, 1, false },
	{ 0x14, "Mode Change",
				mode_change_evt, 6, true },
	{ 0x15, "Return Link Keys",
				return_link_keys_evt, 1, false },
	{ 0x16, "PIN Code Request",
				pin_code_request_evt, 6, true },
	{ 0x17, "Link Key Request",
				link_key_request_evt, 6, true },
	{ 0x18, "Link Key Notification",
				link_key_notify_evt, 23, true },
	{ 0x19, "Loopback Command",
				loopback_command_evt, 3, false },
	{ 0x1a, "Data Buffer Overflow",
				data_buffer_overflow_evt, 1, true },
	{ 0x1b, "Max Slots Change",
				max_slots_change_evt, 3, true },
	{ 0x1c, "Read Clock Offset Complete",
				clock_offset_complete_evt, 5, true },
	{ 0x1d, "Connection Packet Type Changed",
				conn_pkt_type_changed_evt, 5, true },
	{ 0x1e, "QoS Violation",
				qos_violation_evt, 2, true },
	{ 0x1f, "Page Scan Mode Change",
				pscan_mode_change_evt, 7, true },
	{ 0x20, "Page Scan Repetition Mode Change",
				pscan_rep_mode_change_evt, 7, true },
	{ 0x21, "Flow Specification Complete",
				flow_spec_complete_evt, 22, true },
	{ 0x22, "Inquiry Result with RSSI",
				inquiry_result_with_rssi_evt, 1, false },
	{ 0x23, "Read Remote Extended Features",
				remote_ext_features_complete_evt, 13, true },
	{ 0x2c, "Synchronous Connect Complete",
				sync_conn_complete_evt, 17, true },
	{ 0x2d, "Synchronous Connect Changed",
				sync_conn_changed_evt, 9, true },
	{ 0x2e, "Sniff Subrating",
				sniff_subrating_evt, 11, true },
	{ 0x2f, "Extended Inquiry Result",
				ext_inquiry_result_evt, 1, false },
	{ 0x30, "Encryption Key Refresh Complete",
				encrypt_key_refresh_complete_evt, 3, true },
	{ 0x31, "IO Capability Request",
				io_capability_request_evt, 6, true },
	{ 0x32, "IO Capability Response",
				io_capability_response_evt, 9, true },
	{ 0x33, "User Confirmation Request",
				user_confirm_request_evt, 10, true },
	{ 0x34, "User Passkey Request",
				user_passkey_request_evt, 6, true },
	{ 0x35, "Remote OOB Data Request",
				remote_oob_data_request_evt, 6, true },
	{ 0x36, "Simple Pairing Complete",
				simple_pairing_complete_evt, 7, true },
	{ 0x38, "Link Supervision Timeout Changed",
				link_supv_timeout_changed_evt, 4, true },
	{ 0x39, "Enhanced Flush Complete",
				enhanced_flush_complete_evt, 2, true },
	{ 0x3b, "User Passkey Notification",
				user_passkey_notify_evt, 10, true },
	{ 0x3c, "Keypress Notification",
				keypress_notify_evt, 7, true },
	{ 0x3d, "Remote Host Supported Features",
				remote_host_features_notify_evt, 14, true },
	{ 0x3e, "LE Meta Event",
				le_meta_event_evt, 1, false },
	{ 0x40, "Physical Link Complete",
				phy_link_complete_evt, 2, true },
	{ 0x41, "Channel Selected",
				channel_selected_evt, 1, true },
	{ 0x42, "Disconnect Physical Link Complete",
				disconn_phy_link_complete_evt, 3, true },
	{ 0x43, "Physical Link Loss Early Warning",
				phy_link_loss_early_warning_evt, 2, true },
	{ 0x44, "Physical Link Recovery",
				phy_link_recovery_evt, 1, true },
	{ 0x45, "Logical Link Complete",
				logic_link_complete_evt, 5, true },
	{ 0x46, "Disconnect Logical Link Complete",
				disconn_logic_link_complete_evt, 4, true },
	{ 0x47, "Flow Specification Modify Complete",
				flow_spec_modify_complete_evt, 3, true },
	{ 0x48, "Number of Completed Data Blocks",
				num_completed_data_blocks_evt, 3, false },
	{ 0x49, "AMP Start Test" },
	{ 0x4a, "AMP Test End" },
	{ 0x4b, "AMP Receiver Report" },
	{ 0x4c, "Short Range Mode Change Complete",
				short_range_mode_change_evt, 3, true },
	{ 0x4d, "AMP Status Change",
				amp_status_change_evt, 2, true },
	{ 0x4e, "Triggered Clock Capture",
				triggered_clock_capture_evt, 9, true },
	{ 0x4f, "Synchronization Train Complete",
				sync_train_complete_evt, 1, true },
	{ 0x50, "Synchronization Train Received",
				sync_train_received_evt, 29, true },
	{ 0x51, "Connectionless Slave Broadcast Receive",
				slave_broadcast_receive_evt, 18, false },
	{ 0x52, "Connectionless Slave Broadcast Timeout",
				slave_broadcast_timeout_evt, 7, true },
	{ 0x53, "Truncated Page Complete",
				truncated_page_complete_evt, 7, true },
	{ 0x54, "Slave Page Response Timeout",
				slave_page_response_timeout_evt, 0, true },
	{ 0x55, "Connectionless Slave Broadcast Channel Map Change",
				slave_broadcast_channel_map_change_evt, 10, true },
	{ 0x56, "Inquiry Response Notification",
				inquiry_response_notify_evt, 4, true },
	{ 0x57, "Authenticated Payload Timeout Expired",
				auth_payload_timeout_expired_evt, 2, true },
	{ 0x58, "SAM Status Change" },
	{ 0xfe, "Testing" },
	{ 0xff, "Vendor", vendor_evt, 0, false },
	{ }
};

void packet_new_index(struct timeval *tv, uint16_t index, const char *label,
				uint8_t type, uint8_t bus, const char *name)
{
	char details[48];

	sprintf(details, "(%s,%s,%s)", hci_typetostr(type),
					hci_bustostr(bus), name);

	print_packet(tv, NULL, '=', index, NULL, COLOR_NEW_INDEX,
					"New Index", label, details);
}

void packet_del_index(struct timeval *tv, uint16_t index, const char *label)
{
	print_packet(tv, NULL, '=', index, NULL, COLOR_DEL_INDEX,
					"Delete Index", label, NULL);
}

void packet_open_index(struct timeval *tv, uint16_t index, const char *label)
{
	print_packet(tv, NULL, '=', index, NULL, COLOR_OPEN_INDEX,
					"Open Index", label, NULL);
}

void packet_close_index(struct timeval *tv, uint16_t index, const char *label)
{
	print_packet(tv, NULL, '=', index, NULL, COLOR_CLOSE_INDEX,
					"Close Index", label, NULL);
}

void packet_index_info(struct timeval *tv, uint16_t index, const char *label,
							uint16_t manufacturer)
{
	char details[128];

	sprintf(details, "(%s)", bt_compidtostr(manufacturer));

	print_packet(tv, NULL, '=', index, NULL, COLOR_INDEX_INFO,
					"Index Info", label, details);
}

void packet_vendor_diag(struct timeval *tv, uint16_t index,
					uint16_t manufacturer,
					const void *data, uint16_t size)
{
	char extra_str[16];

	sprintf(extra_str, "(len %d)", size);

	print_packet(tv, NULL, '=', index, NULL, COLOR_VENDOR_DIAG,
					"Vendor Diagnostic", NULL, extra_str);

	switch (manufacturer) {
	case 15:
		broadcom_lm_diag(data, size);
		break;
	default:
		packet_hexdump(data, size);
		break;
	}
}

void packet_system_note(struct timeval *tv, struct ucred *cred,
					uint16_t index, const void *message)
{
	print_packet(tv, cred, '=', index, NULL, COLOR_SYSTEM_NOTE,
					"Note", message, NULL);
}

struct monitor_l2cap_hdr {
	uint16_t cid;
	uint16_t psm;
};

static void packet_decode(struct timeval *tv, struct ucred *cred, char dir,
				uint16_t index, const char *color,
				const char *label, const void *data,
				uint16_t size)
{
	const struct monitor_l2cap_hdr *hdr = data;

	if (size < sizeof(*hdr)) {
		print_packet(tv, cred, '*', index, NULL, COLOR_ERROR,
			"Malformed User Data packet", NULL, NULL);
	}

	print_packet(tv, cred, dir, index, NULL, COLOR_HCI_ACLDATA, label,
				dir == '>' ? "User Data RX" : "User Data TX",
				NULL);

	/* Discard last byte since it just a filler */
	l2cap_frame(index, dir == '>', 0, hdr->cid, hdr->psm,
			data + sizeof(*hdr), size - (sizeof(*hdr) + 1));
}

void packet_user_logging(struct timeval *tv, struct ucred *cred,
					uint16_t index, uint8_t priority,
					const char *ident, const void *data,
					uint16_t size)
{
	char pid_str[140];
	const char *label;
	const char *color;

	if (priority > priority_level)
		return;

	switch (priority) {
	case BTSNOOP_PRIORITY_ERR:
		color = COLOR_ERROR;
		break;
	case BTSNOOP_PRIORITY_WARNING:
		color = COLOR_WARN;
		break;
	case BTSNOOP_PRIORITY_INFO:
		color = COLOR_INFO;
		break;
	case BTSNOOP_PRIORITY_DEBUG:
		color = COLOR_DEBUG;
		break;
	default:
		color = COLOR_WHITE_BG;
		break;
	}

	if (cred) {
		char *path = alloca(24);
		char line[128];
		FILE *fp;

		snprintf(path, 23, "/proc/%u/comm", cred->pid);

		fp = fopen(path, "re");
		if (fp) {
			if (fgets(line, sizeof(line), fp)) {
				line[strcspn(line, "\r\n")] = '\0';
				snprintf(pid_str, sizeof(pid_str), "%s[%u]",
							line, cred->pid);
			} else
				snprintf(pid_str, sizeof(pid_str), "%u",
								cred->pid);
			fclose(fp);
		} else
			snprintf(pid_str, sizeof(pid_str), "%u", cred->pid);

		label = pid_str;
        } else {
		if (ident)
			label = ident;
		else
			label = "Message";
	}

	if (ident[0] == '<' || ident[0] == '>') {
		packet_decode(tv, cred, ident[0], index, color,
				label == ident ? &ident[2] : label,
				data, size);
		return;
	}

	print_packet(tv, cred, '=', index, NULL, color, label, data, NULL);
}

void packet_hci_command(struct timeval *tv, struct ucred *cred, uint16_t index,
					const void *data, uint16_t size)
{
	const hci_command_hdr *hdr = data;
	uint16_t opcode = le16_to_cpu(hdr->opcode);
	uint16_t ogf = cmd_opcode_ogf(opcode);
	uint16_t ocf = cmd_opcode_ocf(opcode);
	struct opcode_data vendor_data;
	const struct opcode_data *opcode_data = NULL;
	const char *opcode_color, *opcode_str;
	char extra_str[25], vendor_str[150];
	int i;

	if (index >= MAX_INDEX) {
		print_field("Invalid index (%d).", index);
		return;
	}

	index_list[index].frame++;

	if (size < HCI_COMMAND_HDR_SIZE || size > BTSNOOP_MAX_PACKET_SIZE) {
		sprintf(extra_str, "(len %d)", size);
		print_packet(tv, cred, '*', index, NULL, COLOR_ERROR,
			"Malformed HCI Command packet", NULL, extra_str);
		return;
	}

	data += HCI_COMMAND_HDR_SIZE;
	size -= HCI_COMMAND_HDR_SIZE;

	for (i = 0; opcode_table[i].str; i++) {
		if (opcode_table[i].opcode == opcode) {
			opcode_data = &opcode_table[i];
			break;
		}
	}

	if (opcode_data) {
		if (opcode_data->cmd_func)
			opcode_color = COLOR_HCI_COMMAND;
		else
			opcode_color = COLOR_HCI_COMMAND_UNKNOWN;
		opcode_str = opcode_data->str;
	} else {
		if (ogf == 0x3f) {
			const struct vendor_ocf *vnd = current_vendor_ocf(ocf);

			if (vnd) {
				const char *str = current_vendor_str();

				if (str) {
					snprintf(vendor_str, sizeof(vendor_str),
							"%s %s", str, vnd->str);
					vendor_data.str = vendor_str;
				} else
					vendor_data.str = vnd->str;
				vendor_data.cmd_func = vnd->cmd_func;
				vendor_data.cmd_size = vnd->cmd_size;
				vendor_data.cmd_fixed = vnd->cmd_fixed;

				opcode_data = &vendor_data;

				if (opcode_data->cmd_func)
					opcode_color = COLOR_HCI_COMMAND;
				else
					opcode_color = COLOR_HCI_COMMAND_UNKNOWN;
				opcode_str = opcode_data->str;
			} else {
				opcode_color = COLOR_HCI_COMMAND;
				opcode_str = "Vendor";
			}
		} else {
			opcode_color = COLOR_HCI_COMMAND_UNKNOWN;
			opcode_str = "Unknown";
		}
	}

	sprintf(extra_str, "(0x%2.2x|0x%4.4x) plen %d", ogf, ocf, hdr->plen);

	print_packet(tv, cred, '<', index, NULL, opcode_color, "HCI Command",
							opcode_str, extra_str);

	if (!opcode_data || !opcode_data->cmd_func) {
		packet_hexdump(data, size);
		return;
	}

	if (size != hdr->plen) {
		print_text(COLOR_ERROR, "invalid packet size (%u != %u)", size,
								hdr->plen);
		packet_hexdump(data, size);
		return;
	}

	if (opcode_data->cmd_fixed) {
		if (hdr->plen != opcode_data->cmd_size) {
			print_text(COLOR_ERROR, "invalid packet size");
			packet_hexdump(data, size);
			return;
		}
	} else {
		if (hdr->plen < opcode_data->cmd_size) {
			print_text(COLOR_ERROR, "too short packet");
			packet_hexdump(data, size);
			return;
		}
	}

	opcode_data->cmd_func(data, hdr->plen);
}

void packet_hci_event(struct timeval *tv, struct ucred *cred, uint16_t index,
					const void *data, uint16_t size)
{
	const hci_event_hdr *hdr = data;
	const struct event_data *event_data = NULL;
	const char *event_color, *event_str;
	char extra_str[25];
	int i;

	if (index >= MAX_INDEX) {
		print_field("Invalid index (%d).", index);
		return;
	}


	index_list[index].frame++;

	if (size < HCI_EVENT_HDR_SIZE) {
		sprintf(extra_str, "(len %d)", size);
		print_packet(tv, cred, '*', index, NULL, COLOR_ERROR,
			"Malformed HCI Event packet", NULL, extra_str);
		packet_hexdump(data, size);
		return;
	}

	data += HCI_EVENT_HDR_SIZE;
	size -= HCI_EVENT_HDR_SIZE;

	for (i = 0; event_table[i].str; i++) {
		if (event_table[i].event == hdr->evt) {
			event_data = &event_table[i];
			break;
		}
	}

	if (event_data) {
		if (event_data->func)
			event_color = COLOR_HCI_EVENT;
		else
			event_color = COLOR_HCI_EVENT_UNKNOWN;
		event_str = event_data->str;
	} else {
		event_color = COLOR_HCI_EVENT_UNKNOWN;
		event_str = "Unknown";
	}

	sprintf(extra_str, "(0x%2.2x) plen %d", hdr->evt, hdr->plen);

	print_packet(tv, cred, '>', index, NULL, event_color, "HCI Event",
						event_str, extra_str);

	if (!event_data || !event_data->func) {
		packet_hexdump(data, size);
		return;
	}

	if (size != hdr->plen) {
		print_text(COLOR_ERROR, "invalid packet size (%u != %u)", size,
								hdr->plen);
		packet_hexdump(data, size);
		return;
	}

	if (event_data->fixed) {
		if (hdr->plen != event_data->size) {
			print_text(COLOR_ERROR, "invalid packet size");
			packet_hexdump(data, size);
			return;
		}
	} else {
		if (hdr->plen < event_data->size) {
			print_text(COLOR_ERROR, "too short packet");
			packet_hexdump(data, size);
			return;
		}
	}

	event_data->func(data, hdr->plen);
}

void packet_hci_acldata(struct timeval *tv, struct ucred *cred, uint16_t index,
				bool in, const void *data, uint16_t size)
{
	const struct bt_hci_acl_hdr *hdr = data;
	uint16_t handle = le16_to_cpu(hdr->handle);
	uint16_t dlen = le16_to_cpu(hdr->dlen);
	uint8_t flags = acl_flags(handle);
	char handle_str[16], extra_str[32];

	if (index >= MAX_INDEX) {
		print_field("Invalid index (%d).", index);
		return;
	}

	index_list[index].frame++;

	if (size < HCI_ACL_HDR_SIZE) {
		if (in)
			print_packet(tv, cred, '*', index, NULL, COLOR_ERROR,
				"Malformed ACL Data RX packet", NULL, NULL);
		else
			print_packet(tv, cred, '*', index, NULL, COLOR_ERROR,
				"Malformed ACL Data TX packet", NULL, NULL);
		packet_hexdump(data, size);
		return;
	}

	data += HCI_ACL_HDR_SIZE;
	size -= HCI_ACL_HDR_SIZE;

	sprintf(handle_str, "Handle %d", acl_handle(handle));
	sprintf(extra_str, "flags 0x%2.2x dlen %d", flags, dlen);

	print_packet(tv, cred, in ? '>' : '<', index, NULL, COLOR_HCI_ACLDATA,
				in ? "ACL Data RX" : "ACL Data TX",
						handle_str, extra_str);

	if (size != dlen) {
		print_text(COLOR_ERROR, "invalid packet size (%d != %d)",
								size, dlen);
		packet_hexdump(data, size);
		return;
	}

	if (filter_mask & PACKET_FILTER_SHOW_ACL_DATA)
		packet_hexdump(data, size);

	l2cap_packet(index, in, acl_handle(handle), flags, data, size);
}

void packet_hci_scodata(struct timeval *tv, struct ucred *cred, uint16_t index,
				bool in, const void *data, uint16_t size)
{
	const hci_sco_hdr *hdr = data;
	uint16_t handle = le16_to_cpu(hdr->handle);
	uint8_t flags = acl_flags(handle);
	char handle_str[16], extra_str[32];

	if (index >= MAX_INDEX) {
		print_field("Invalid index (%d).", index);
		return;
	}

	index_list[index].frame++;

	if (size < HCI_SCO_HDR_SIZE) {
		if (in)
			print_packet(tv, cred, '*', index, NULL, COLOR_ERROR,
				"Malformed SCO Data RX packet", NULL, NULL);
		else
			print_packet(tv, cred, '*', index, NULL, COLOR_ERROR,
				"Malformed SCO Data TX packet", NULL, NULL);
		packet_hexdump(data, size);
		return;
	}

	data += HCI_SCO_HDR_SIZE;
	size -= HCI_SCO_HDR_SIZE;

	sprintf(handle_str, "Handle %d", acl_handle(handle));
	sprintf(extra_str, "flags 0x%2.2x dlen %d", flags, hdr->dlen);

	print_packet(tv, cred, in ? '>' : '<', index, NULL, COLOR_HCI_SCODATA,
				in ? "SCO Data RX" : "SCO Data TX",
						handle_str, extra_str);

	if (size != hdr->dlen) {
		print_text(COLOR_ERROR, "invalid packet size (%d != %d)",
							size, hdr->dlen);
		packet_hexdump(data, size);
		return;
	}

	if (filter_mask & PACKET_FILTER_SHOW_SCO_DATA)
		packet_hexdump(data, size);
}

void packet_hci_isodata(struct timeval *tv, struct ucred *cred, uint16_t index,
				bool in, const void *data, uint16_t size)
{
	const struct bt_hci_iso_hdr *hdr = data;
	uint16_t handle = le16_to_cpu(hdr->handle);
	uint8_t flags = acl_flags(handle);
	char handle_str[16], extra_str[32];

	if (index >= MAX_INDEX) {
		print_field("Invalid index (%d).", index);
		return;
	}

	index_list[index].frame++;

	if (size < sizeof(*hdr)) {
		if (in)
			print_packet(tv, cred, '*', index, NULL, COLOR_ERROR,
				"Malformed ISO Data RX packet", NULL, NULL);
		else
			print_packet(tv, cred, '*', index, NULL, COLOR_ERROR,
				"Malformed ISO Data TX packet", NULL, NULL);
		packet_hexdump(data, size);
		return;
	}

	data += sizeof(*hdr);
	size -= sizeof(*hdr);

	sprintf(handle_str, "Handle %d", acl_handle(handle));
	sprintf(extra_str, "flags 0x%2.2x dlen %d", flags, hdr->dlen);

	print_packet(tv, cred, in ? '>' : '<', index, NULL, COLOR_HCI_SCODATA,
				in ? "ISO Data RX" : "ISO Data TX",
						handle_str, extra_str);

	if (size != hdr->dlen) {
		print_text(COLOR_ERROR, "invalid packet size (%d != %d)",
							size, hdr->dlen);
		packet_hexdump(data, size);
		return;
	}

	if (filter_mask & PACKET_FILTER_SHOW_SCO_DATA)
		packet_hexdump(data, size);
}

void packet_ctrl_open(struct timeval *tv, struct ucred *cred, uint16_t index,
					const void *data, uint16_t size)
{
	uint32_t cookie;
	uint16_t format;
	char channel[11];

	if (size < 6) {
		print_packet(tv, cred, '*', index, NULL, COLOR_ERROR,
				"Malformed Control Open packet", NULL, NULL);
		packet_hexdump(data, size);
		return;
	}

	cookie = get_le32(data);
	format = get_le16(data + 4);

	data += 6;
	size -= 6;

	sprintf(channel, "0x%4.4x", cookie);

	if ((format == CTRL_RAW || format == CTRL_USER || format == CTRL_MGMT)
								&& size >= 8) {
		uint8_t version;
		uint16_t revision;
		uint32_t flags;
		uint8_t ident_len;
		const char *comm;
		char details[48];
		const char *title;

		version = get_u8(data);
		revision = get_le16(data + 1);
		flags = get_le32(data + 3);
		ident_len = get_u8(data + 7);

		if ((8 + ident_len) > size) {
			print_packet(tv, cred, '*', index, NULL, COLOR_ERROR,
                                "Malformed Control Open packet", NULL, NULL);
			return;
		}

		data += 8;
		size -= 8;

		comm = ident_len > 0 ? data : "unknown";

		data += ident_len;
		size -= ident_len;

		assign_ctrl(cookie, format, comm);

		sprintf(details, "%sversion %u.%u",
				flags & 0x0001 ? "(privileged) " : "",
				version, revision);

		switch (format) {
		case CTRL_RAW:
			title = "RAW Open";
			break;
		case CTRL_USER:
			title = "USER Open";
			break;
		case CTRL_MGMT:
			title = "MGMT Open";
			break;
		default:
			title = "Control Open";
			break;
		}

		print_packet(tv, cred, '@', index, channel, COLOR_CTRL_OPEN,
						title, comm, details);
	} else {
		char label[7];

		assign_ctrl(cookie, format, NULL);

		sprintf(label, "0x%4.4x", format);

		print_packet(tv, cred, '@', index, channel, COLOR_CTRL_OPEN,
						"Control Open", label, NULL);
	}

	packet_hexdump(data, size);
}

void packet_ctrl_close(struct timeval *tv, struct ucred *cred, uint16_t index,
					const void *data, uint16_t size)
{
	uint32_t cookie;
	uint16_t format;
	char channel[11], label[22];
	const char *title;

	if (size < 4) {
		print_packet(tv, cred, '*', index, NULL, COLOR_ERROR,
				"Malformed Control Close packet", NULL, NULL);
		packet_hexdump(data, size);
		return;
	}

	cookie = get_le32(data);

	data += 4;
	size -= 4;

	sprintf(channel, "0x%4.4x", cookie);

	release_ctrl(cookie, &format, label);

	switch (format) {
	case CTRL_RAW:
		title = "RAW Close";
		break;
	case CTRL_USER:
		title = "USER Close";
		break;
	case CTRL_MGMT:
		title = "MGMT Close";
		break;
	default:
		sprintf(label, "0x%4.4x", format);
		title = "Control Close";
		break;
	}

	print_packet(tv, cred, '@', index, channel, COLOR_CTRL_CLOSE,
							title, label, NULL);

	packet_hexdump(data, size);
}

static const struct {
	uint8_t status;
	const char *str;
} mgmt_status_table[] = {
	{ 0x00, "Success"		},
	{ 0x01, "Unknown Command"	},
	{ 0x02, "Not Connected"		},
	{ 0x03, "Failed"		},
	{ 0x04, "Connect Failed"	},
	{ 0x05, "Authentication Failed"	},
	{ 0x06, "Not Paired"		},
	{ 0x07, "No Resources"		},
	{ 0x08, "Timeout"		},
	{ 0x09, "Already Connected"	},
	{ 0x0a, "Busy"			},
	{ 0x0b, "Rejected"		},
	{ 0x0c, "Not Supported"		},
	{ 0x0d, "Invalid Parameters"	},
	{ 0x0e, "Disconnected"		},
	{ 0x0f, "Not Powered"		},
	{ 0x10, "Cancelled"		},
	{ 0x11, "Invalid Index"		},
	{ 0x12, "RFKilled"		},
	{ 0x13, "Already Paired"	},
	{ 0x14, "Permission Denied"	},
	{ }
};

static void mgmt_print_status(uint8_t status)
{
	const char *str = "Unknown";
	const char *color_on, *color_off;
	bool unknown = true;
	int i;

	for (i = 0; mgmt_status_table[i].str; i++) {
		if (mgmt_status_table[i].status == status) {
			str = mgmt_status_table[i].str;
			unknown = false;
			break;
		}
	}

	if (use_color()) {
		if (status) {
			if (unknown)
				color_on = COLOR_UNKNOWN_ERROR;
			else
				color_on = COLOR_RED;
		} else
			color_on = COLOR_GREEN;
		color_off = COLOR_OFF;
	} else {
		color_on = "";
		color_off = "";
	}

	print_field("Status: %s%s%s (0x%2.2x)",
				color_on, str, color_off, status);
}

static void mgmt_print_address(const uint8_t *address, uint8_t type)
{
	switch (type) {
	case 0x00:
		print_addr_resolve("BR/EDR Address", address, 0x00, false);
		break;
	case 0x01:
		print_addr_resolve("LE Address", address, 0x00, false);
		break;
	case 0x02:
		print_addr_resolve("LE Address", address, 0x01, false);
		break;
	default:
		print_addr_resolve("Address", address, 0xff, false);
		break;
	}
}

static const struct bitfield_data mgmt_address_type_table[] = {
	{  0, "BR/EDR"		},
	{  1, "LE Public"	},
	{  2, "LE Random"	},
	{ }
};

static void mgmt_print_address_type(uint8_t type)
{
	uint8_t mask;

	print_field("Address type: 0x%2.2x", type);

	mask = print_bitfield(2, type, mgmt_address_type_table);
	if (mask)
		print_text(COLOR_UNKNOWN_ADDRESS_TYPE, "  Unknown address type"
							" (0x%2.2x)", mask);
}

static void mgmt_print_version(uint8_t version)
{
	packet_print_version("Version", version, NULL, 0x0000);
}

static void mgmt_print_manufacturer(uint16_t manufacturer)
{
	packet_print_company("Manufacturer", manufacturer);
}

static const struct bitfield_data mgmt_options_table[] = {
	{  0, "External configuration"			},
	{  1, "Bluetooth public address configuration"	},
	{ }
};

static void mgmt_print_options(const char *label, uint32_t options)
{
	uint32_t mask;

	print_field("%s: 0x%8.8x", label, options);

	mask = print_bitfield(2, options, mgmt_options_table);
	if (mask)
		print_text(COLOR_UNKNOWN_OPTIONS_BIT, "  Unknown options"
							" (0x%8.8x)", mask);
}

static const struct bitfield_data mgmt_settings_table[] = {
	{  0, "Powered"			},
	{  1, "Connectable"		},
	{  2, "Fast Connectable"	},
	{  3, "Discoverable"		},
	{  4, "Bondable"		},
	{  5, "Link Security"		},
	{  6, "Secure Simple Pairing"	},
	{  7, "BR/EDR"			},
	{  8, "High Speed"		},
	{  9, "Low Energy"		},
	{ 10, "Advertising"		},
	{ 11, "Secure Connections"	},
	{ 12, "Debug Keys"		},
	{ 13, "Privacy"			},
	{ 14, "Controller Configuration"},
	{ 15, "Static Address"		},
	{ 16, "PHY Configuration"	},
	{ 17, "Wideband Speech"		},
	{ }
};

static void mgmt_print_settings(const char *label, uint32_t settings)
{
	uint32_t mask;

	print_field("%s: 0x%8.8x", label, settings);

	mask = print_bitfield(2, settings, mgmt_settings_table);
	if (mask)
		print_text(COLOR_UNKNOWN_SETTINGS_BIT, "  Unknown settings"
							" (0x%8.8x)", mask);
}

static void mgmt_print_name(const void *data)
{
	print_field("Name: %s", (char *) data);
	print_field("Short name: %s", (char *) (data + 249));
}

static void mgmt_print_uuid(const void *data)
{
	const uint8_t *uuid = data;

	print_field("UUID: %8.8x-%4.4x-%4.4x-%4.4x-%8.8x%4.4x",
				get_le32(&uuid[12]), get_le16(&uuid[10]),
				get_le16(&uuid[8]), get_le16(&uuid[6]),
				get_le32(&uuid[2]), get_le16(&uuid[0]));
}

static void mgmt_print_io_capability(uint8_t capability)
{
	const char *str;

	switch (capability) {
	case 0x00:
		str = "DisplayOnly";
		break;
	case 0x01:
		str = "DisplayYesNo";
		break;
	case 0x02:
		str = "KeyboardOnly";
		break;
	case 0x03:
		str = "NoInputNoOutput";
		break;
	case 0x04:
		str = "KeyboardDisplay";
		break;
	default:
		str = "Reserved";
		break;
	}

	print_field("Capability: %s (0x%2.2x)", str, capability);
}

static const struct bitfield_data mgmt_device_flags_table[] = {
	{  0, "Confirm Name"	},
	{  1, "Legacy Pairing"	},
	{  2, "Not Connectable"	},
	{ }
};

static void mgmt_print_device_flags(uint32_t flags)
{
	uint32_t mask;

	print_field("Flags: 0x%8.8x", flags);

	mask = print_bitfield(2, flags, mgmt_device_flags_table);
	if (mask)
		print_text(COLOR_UNKNOWN_DEVICE_FLAG, "  Unknown device flag"
							" (0x%8.8x)", mask);
}

static void mgmt_print_device_action(uint8_t action)
{
	const char *str;

	switch (action) {
	case 0x00:
		str = "Background scan for device";
		break;
	case 0x01:
		str = "Allow incoming connection";
		break;
	case 0x02:
		str = "Auto-connect remote device";
		break;
	default:
		str = "Reserved";
		break;
	}

	print_field("Action: %s (0x%2.2x)", str, action);
}

static const struct bitfield_data mgmt_adv_flags_table[] = {
	{  0, "Switch into Connectable mode"		},
	{  1, "Advertise as Discoverable"		},
	{  2, "Advertise as Limited Discoverable"	},
	{  3, "Add Flags field to Advertising Data"	},
	{  4, "Add TX Power field to Advertising Data"	},
	{  5, "Add Appearance field to Scan Response"	},
	{  6, "Add Local Name in Scan Response"		},
	{  7, "Advertise in 1M on Secondary channel"	},
	{  8, "Advertise in 2M on Secondary channel"	},
	{  9, "Advertise in CODED on Secondary channel"	},
	{  12, "Use provided duration parameter"	},
	{  13, "Use provided timeout parameter"		},
	{  14, "Use provided interval parameters"	},
	{  15, "Use provided tx power parameter"	},
	{ }
};
#define MGMT_ADV_PARAM_DURATION		(1 << 12)
#define MGMT_ADV_PARAM_TIMEOUT		(1 << 13)
#define MGMT_ADV_PARAM_INTERVALS	(1 << 14)
#define MGMT_ADV_PARAM_TX_POWER		(1 << 15)

static void mgmt_print_adv_flags(uint32_t flags)
{
	uint32_t mask;

	print_field("Flags: 0x%8.8x", flags);

	mask = print_bitfield(2, flags, mgmt_adv_flags_table);
	if (mask)
		print_text(COLOR_UNKNOWN_ADV_FLAG, "  Unknown advertising flag"
							" (0x%8.8x)", mask);
}

static void mgmt_print_store_hint(uint8_t hint)
{
	const char *str;

	switch (hint) {
	case 0x00:
		str = "No";
		break;
	case 0x01:
		str = "Yes";
		break;
	default:
		str = "Reserved";
		break;
	}

	print_field("Store hint: %s (0x%2.2x)", str, hint);
}

static void mgmt_print_connection_parameter(const void *data)
{
	uint8_t address_type = get_u8(data + 6);
	uint16_t min_conn_interval = get_le16(data + 7);
	uint16_t max_conn_interval = get_le16(data + 9);
	uint16_t conn_latency = get_le16(data + 11);
	uint16_t supv_timeout = get_le16(data + 13);

	mgmt_print_address(data, address_type);
	print_field("Min connection interval: %u", min_conn_interval);
	print_field("Max connection interval: %u", max_conn_interval);
	print_conn_latency("Connection latency", conn_latency);
	print_field("Supervision timeout: %u", supv_timeout);
}

static void mgmt_print_link_key(const void *data)
{
	uint8_t address_type = get_u8(data + 6);
	uint8_t key_type = get_u8(data + 7);
	uint8_t pin_len = get_u8(data + 24);

	mgmt_print_address(data, address_type);
	print_key_type(key_type);
	print_link_key(data + 8);
	print_field("PIN length: %d", pin_len);
}

static void mgmt_print_long_term_key(const void *data)
{
	uint8_t address_type = get_u8(data + 6);
	uint8_t key_type = get_u8(data + 7);
	uint8_t master = get_u8(data + 8);
	uint8_t enc_size = get_u8(data + 9);
	const char *str;

	mgmt_print_address(data, address_type);

	switch (key_type) {
	case 0x00:
		str = "Unauthenticated legacy key";
		break;
	case 0x01:
		str = "Authenticated legacy key";
		break;
	case 0x02:
		str = "Unauthenticated key from P-256";
		break;
	case 0x03:
		str = "Authenticated key from P-256";
		break;
	case 0x04:
		str = "Debug key from P-256";
		break;
	default:
		str = "Reserved";
		break;
	}

	print_field("Key type: %s (0x%2.2x)", str, key_type);
	print_field("Master: 0x%2.2x", master);
	print_field("Encryption size: %u", enc_size);
	print_hex_field("Diversifier", data + 10, 2);
	print_hex_field("Randomizer", data + 12, 8);
	print_hex_field("Key", data + 20, 16);
}

static void mgmt_print_identity_resolving_key(const void *data)
{
	uint8_t address_type = get_u8(data + 6);

	mgmt_print_address(data, address_type);
	print_hex_field("Key", data + 7, 16);
}

static void mgmt_print_signature_resolving_key(const void *data)
{
	uint8_t address_type = get_u8(data + 6);
	uint8_t key_type = get_u8(data + 7);
	const char *str;

	mgmt_print_address(data, address_type);

	switch (key_type) {
	case 0x00:
		str = "Unauthenticated local CSRK";
		break;
	case 0x01:
		str = "Unauthenticated remote CSRK";
		break;
	case 0x02:
		str = "Authenticated local CSRK";
		break;
	case 0x03:
		str = "Authenticated remote CSRK";
		break;
	default:
		str = "Reserved";
		break;
	}

	print_field("Key type: %s (0x%2.2x)", str, key_type);
	print_hex_field("Key", data + 8, 16);
}

static void mgmt_print_oob_data(const void *data)
{
	print_hash_p192(data);
	print_randomizer_p192(data + 16);
	print_hash_p256(data + 32);
	print_randomizer_p256(data + 48);
}

static const struct bitfield_data mgmt_exp_feature_flags_table[] = {
	{  0, "Active"		},
	{  1, "Settings change"	},
	{ }
};

static void mgmt_print_exp_feature(const void *data)
{
	uint32_t flags = get_le32(data + 16);
	uint32_t mask;

	mgmt_print_uuid(data);
	print_field("Flags: 0x%8.8x", flags);

	mask = print_bitfield(2, flags, mgmt_exp_feature_flags_table);
	if (mask)
		print_text(COLOR_UNKNOWN_EXP_FEATURE_FLAG,
				"  Unknown feature flag (0x%8.8x)", mask);
}

static void mgmt_null_cmd(const void *data, uint16_t size)
{
}

static void mgmt_null_rsp(const void *data, uint16_t size)
{
}

static void mgmt_read_version_info_rsp(const void *data, uint16_t size)
{
	uint8_t version;
	uint16_t revision;

	version = get_u8(data);
	revision = get_le16(data + 1);

	print_field("Version: %u.%u", version, revision);
}

static void mgmt_print_commands(const void *data, uint16_t num);
static void mgmt_print_events(const void *data, uint16_t num);

static void mgmt_read_supported_commands_rsp(const void *data, uint16_t size)
{
	uint16_t num_commands = get_le16(data);
	uint16_t num_events = get_le16(data + 2);

	if (size - 4 != (num_commands * 2) + (num_events *2)) {
		packet_hexdump(data, size);
		return;
	}

	mgmt_print_commands(data + 4, num_commands);
	mgmt_print_events(data + 4 + num_commands * 2, num_events);
}

static void mgmt_read_index_list_rsp(const void *data, uint16_t size)
{
	uint16_t num_controllers = get_le16(data);
	int i;

	print_field("Controllers: %u", num_controllers);

	if (size - 2 != num_controllers * 2) {
		packet_hexdump(data + 2, size - 2);
		return;
	}

	for (i = 0; i < num_controllers; i++) {
		uint16_t index = get_le16(data + 2 + (i * 2));

		print_field("  hci%u", index);
	}
}

static void mgmt_read_controller_info_rsp(const void *data, uint16_t size)
{
	uint8_t version = get_u8(data + 6);
	uint16_t manufacturer = get_le16(data + 7);
	uint32_t supported_settings = get_le32(data + 9);
	uint32_t current_settings = get_le32(data + 13);

	print_addr_resolve("Address", data, 0x00, false);
	mgmt_print_version(version);
	mgmt_print_manufacturer(manufacturer);
	mgmt_print_settings("Supported settings", supported_settings);
	mgmt_print_settings("Current settings", current_settings);
	print_dev_class(data + 17);
	mgmt_print_name(data + 20);
}

static void mgmt_set_powered_cmd(const void *data, uint16_t size)
{
	uint8_t enable = get_u8(data);

	print_enable("Powered", enable);
}

static void mgmt_set_discoverable_cmd(const void *data, uint16_t size)
{
	uint8_t enable = get_u8(data);
	uint16_t timeout = get_le16(data + 1);
	const char *str;

	switch (enable) {
	case 0x00:
		str = "Disabled";
		break;
	case 0x01:
		str = "General";
		break;
	case 0x02:
		str = "Limited";
		break;
	default:
		str = "Reserved";
		break;
	}

	print_field("Discoverable: %s (0x%2.2x)", str, enable);
	print_field("Timeout: %u", timeout);
}

static void mgmt_set_connectable_cmd(const void *data, uint16_t size)
{
	uint8_t enable = get_u8(data);

	print_enable("Connectable", enable);
}

static void mgmt_set_fast_connectable_cmd(const void *data, uint16_t size)
{
	uint8_t enable = get_u8(data);

	print_enable("Fast Connectable", enable);
}

static void mgmt_set_bondable_cmd(const void *data, uint16_t size)
{
	uint8_t enable = get_u8(data);

	print_enable("Bondable", enable);
}

static void mgmt_set_link_security_cmd(const void *data, uint16_t size)
{
	uint8_t enable = get_u8(data);

	print_enable("Link Security", enable);
}

static void mgmt_set_secure_simple_pairing_cmd(const void *data, uint16_t size)
{
	uint8_t enable = get_u8(data);

	print_enable("Secure Simple Pairing", enable);
}

static void mgmt_set_high_speed_cmd(const void *data, uint16_t size)
{
	uint8_t enable = get_u8(data);

	print_enable("High Speed", enable);
}

static void mgmt_set_low_energy_cmd(const void *data, uint16_t size)
{
	uint8_t enable = get_u8(data);

	print_enable("Low Energy", enable);
}

static void mgmt_new_settings_rsp(const void *data, uint16_t size)
{
	uint32_t current_settings = get_le32(data);

	mgmt_print_settings("Current settings", current_settings);
}

static void mgmt_set_device_class_cmd(const void *data, uint16_t size)
{
	uint8_t major = get_u8(data);
	uint8_t minor = get_u8(data + 1);

	print_field("Major class: 0x%2.2x", major);
	print_field("Minor class: 0x%2.2x", minor);
}

static void mgmt_set_device_class_rsp(const void *data, uint16_t size)
{
	print_dev_class(data);
}

static void mgmt_set_local_name_cmd(const void *data, uint16_t size)
{
	mgmt_print_name(data);
}

static void mgmt_set_local_name_rsp(const void *data, uint16_t size)
{
	mgmt_print_name(data);
}

static void mgmt_add_uuid_cmd(const void *data, uint16_t size)
{
	uint8_t service_class = get_u8(data + 16);

	mgmt_print_uuid(data);
	print_field("Service class: 0x%2.2x", service_class);
}

static void mgmt_add_uuid_rsp(const void *data, uint16_t size)
{
	print_dev_class(data);
}

static void mgmt_remove_uuid_cmd(const void *data, uint16_t size)
{
	mgmt_print_uuid(data);
}

static void mgmt_remove_uuid_rsp(const void *data, uint16_t size)
{
	print_dev_class(data);
}

static void mgmt_load_link_keys_cmd(const void *data, uint16_t size)
{
	uint8_t debug_keys = get_u8(data);
	uint16_t num_keys = get_le16(data + 1);
	int i;

	print_enable("Debug keys", debug_keys);
	print_field("Keys: %u", num_keys);

	if (size - 3 != num_keys * 25) {
		packet_hexdump(data + 3, size - 3);
		return;
	}

	for (i = 0; i < num_keys; i++)
		mgmt_print_link_key(data + 3 + (i * 25));
}

static void mgmt_load_long_term_keys_cmd(const void *data, uint16_t size)
{
	uint16_t num_keys = get_le16(data);
	int i;

	print_field("Keys: %u", num_keys);

	if (size - 2 != num_keys * 36) {
		packet_hexdump(data + 2, size - 2);
		return;
	}

	for (i = 0; i < num_keys; i++)
		mgmt_print_long_term_key(data + 2 + (i * 36));
}

static void mgmt_disconnect_cmd(const void *data, uint16_t size)
{
	uint8_t address_type = get_u8(data + 6);

	mgmt_print_address(data, address_type);
}

static void mgmt_disconnect_rsp(const void *data, uint16_t size)
{
	uint8_t address_type = get_u8(data + 6);

	mgmt_print_address(data, address_type);
}

static void mgmt_get_connections_rsp(const void *data, uint16_t size)
{
	uint16_t num_connections = get_le16(data);
	int i;

	print_field("Connections: %u", num_connections);

	if (size - 2 != num_connections * 7) {
		packet_hexdump(data + 2, size - 2);
		return;
	}

	for (i = 0; i < num_connections; i++) {
		uint8_t address_type = get_u8(data + 2 + (i * 7) + 6);

		mgmt_print_address(data + 2 + (i * 7), address_type);
	}
}

static void mgmt_pin_code_reply_cmd(const void *data, uint16_t size)
{
	uint8_t address_type = get_u8(data + 6);
	uint8_t pin_len = get_u8(data + 7);

	mgmt_print_address(data, address_type);
	print_field("PIN length: %u", pin_len);
	print_hex_field("PIN code", data + 8, 16);
}

static void mgmt_pin_code_reply_rsp(const void *data, uint16_t size)
{
	uint8_t address_type = get_u8(data + 6);

	mgmt_print_address(data, address_type);
}

static void mgmt_pin_code_neg_reply_cmd(const void *data, uint16_t size)
{
	uint8_t address_type = get_u8(data + 6);

	mgmt_print_address(data, address_type);
}

static void mgmt_pin_code_neg_reply_rsp(const void *data, uint16_t size)
{
	uint8_t address_type = get_u8(data + 6);

	mgmt_print_address(data, address_type);
}

static void mgmt_set_io_capability_cmd(const void *data, uint16_t size)
{
	uint8_t capability = get_u8(data);

	mgmt_print_io_capability(capability);
}

static void mgmt_pair_device_cmd(const void *data, uint16_t size)
{
	uint8_t address_type = get_u8(data + 6);
	uint8_t capability = get_u8(data + 7);

	mgmt_print_address(data, address_type);
	mgmt_print_io_capability(capability);
}

static void mgmt_pair_device_rsp(const void *data, uint16_t size)
{
	uint8_t address_type = get_u8(data + 6);

	mgmt_print_address(data, address_type);
}

static void mgmt_cancel_pair_device_cmd(const void *data, uint16_t size)
{
	uint8_t address_type = get_u8(data + 6);

	mgmt_print_address(data, address_type);
}

static void mgmt_cancel_pair_device_rsp(const void *data, uint16_t size)
{
	uint8_t address_type = get_u8(data + 6);

	mgmt_print_address(data, address_type);
}

static void mgmt_unpair_device_cmd(const void *data, uint16_t size)
{
	uint8_t address_type = get_u8(data + 6);
	uint8_t disconnect = get_u8(data + 7);

	mgmt_print_address(data, address_type);
	print_enable("Disconnect", disconnect);
}

static void mgmt_unpair_device_rsp(const void *data, uint16_t size)
{
	uint8_t address_type = get_u8(data + 6);

	mgmt_print_address(data, address_type);
}

static void mgmt_user_confirmation_reply_cmd(const void *data, uint16_t size)
{
	uint8_t address_type = get_u8(data + 6);

	mgmt_print_address(data, address_type);
}

static void mgmt_user_confirmation_reply_rsp(const void *data, uint16_t size)
{
	uint8_t address_type = get_u8(data + 6);

	mgmt_print_address(data, address_type);
}

static void mgmt_user_confirmation_neg_reply_cmd(const void *data, uint16_t size)
{
	uint8_t address_type = get_u8(data + 6);

	mgmt_print_address(data, address_type);
}

static void mgmt_user_confirmation_neg_reply_rsp(const void *data, uint16_t size)
{
	uint8_t address_type = get_u8(data + 6);

	mgmt_print_address(data, address_type);
}

static void mgmt_user_passkey_reply_cmd(const void *data, uint16_t size)
{
	uint8_t address_type = get_u8(data + 6);
	uint32_t passkey = get_le32(data + 7);

	mgmt_print_address(data, address_type);
	print_field("Passkey: 0x%4.4x", passkey);
}

static void mgmt_user_passkey_reply_rsp(const void *data, uint16_t size)
{
	uint8_t address_type = get_u8(data + 6);

	mgmt_print_address(data, address_type);
}

static void mgmt_user_passkey_neg_reply_cmd(const void *data, uint16_t size)
{
	uint8_t address_type = get_u8(data + 6);

	mgmt_print_address(data, address_type);
}

static void mgmt_user_passkey_neg_reply_rsp(const void *data, uint16_t size)
{
	uint8_t address_type = get_u8(data + 6);

	mgmt_print_address(data, address_type);
}

static void mgmt_read_local_oob_data_rsp(const void *data, uint16_t size)
{
	mgmt_print_oob_data(data);
}

static void mgmt_add_remote_oob_data_cmd(const void *data, uint16_t size)
{
	uint8_t address_type = get_u8(data + 6);

	mgmt_print_address(data, address_type);
	mgmt_print_oob_data(data + 7);
}

static void mgmt_add_remote_oob_data_rsp(const void *data, uint16_t size)
{
	uint8_t address_type = get_u8(data + 6);

	mgmt_print_address(data, address_type);
}

static void mgmt_remove_remote_oob_data_cmd(const void *data, uint16_t size)
{
	uint8_t address_type = get_u8(data + 6);

	mgmt_print_address(data, address_type);
}

static void mgmt_remove_remote_oob_data_rsp(const void *data, uint16_t size)
{
	uint8_t address_type = get_u8(data + 6);

	mgmt_print_address(data, address_type);
}

static void mgmt_start_discovery_cmd(const void *data, uint16_t size)
{
	uint8_t type = get_u8(data);

	mgmt_print_address_type(type);
}

static void mgmt_start_discovery_rsp(const void *data, uint16_t size)
{
	uint8_t type = get_u8(data);

	mgmt_print_address_type(type);
}

static void mgmt_stop_discovery_cmd(const void *data, uint16_t size)
{
	uint8_t type = get_u8(data);

	mgmt_print_address_type(type);
}

static void mgmt_stop_discovery_rsp(const void *data, uint16_t size)
{
	uint8_t type = get_u8(data);

	mgmt_print_address_type(type);
}

static void mgmt_confirm_name_cmd(const void *data, uint16_t size)
{
	uint8_t address_type = get_u8(data + 6);
	uint8_t name_known = get_u8(data + 7);
	const char *str;

	mgmt_print_address(data, address_type);

	switch (name_known) {
	case 0x00:
		str = "No";
		break;
	case 0x01:
		str = "Yes";
		break;
	default:
		str = "Reserved";
		break;
	}

	print_field("Name known: %s (0x%2.2x)", str, name_known);
}

static void mgmt_confirm_name_rsp(const void *data, uint16_t size)
{
	uint8_t address_type = get_u8(data + 6);

	mgmt_print_address(data, address_type);
}

static void mgmt_block_device_cmd(const void *data, uint16_t size)
{
	uint8_t address_type = get_u8(data + 6);

	mgmt_print_address(data, address_type);
}

static void mgmt_block_device_rsp(const void *data, uint16_t size)
{
	uint8_t address_type = get_u8(data + 6);

	mgmt_print_address(data, address_type);
}

static void mgmt_unblock_device_cmd(const void *data, uint16_t size)
{
	uint8_t address_type = get_u8(data + 6);

	mgmt_print_address(data, address_type);
}

static void mgmt_unblock_device_rsp(const void *data, uint16_t size)
{
	uint8_t address_type = get_u8(data + 6);

	mgmt_print_address(data, address_type);
}

static void mgmt_set_device_id_cmd(const void *data, uint16_t size)
{
	print_device_id(data, size);
}

static void mgmt_set_advertising_cmd(const void *data, uint16_t size)
{
	uint8_t enable = get_u8(data);
	const char *str;

	switch (enable) {
	case 0x00:
		str = "Disabled";
		break;
	case 0x01:
		str = "Enabled";
		break;
	case 0x02:
		str = "Connectable";
		break;
	default:
		str = "Reserved";
		break;
	}

	print_field("Advertising: %s (0x%2.2x)", str, enable);
}

static void mgmt_set_bredr_cmd(const void *data, uint16_t size)
{
	uint8_t enable = get_u8(data);

	print_enable("BR/EDR", enable);
}

static void mgmt_set_static_address_cmd(const void *data, uint16_t size)
{
	print_addr_resolve("Address", data, 0x01, false);
}

static void mgmt_set_scan_parameters_cmd(const void *data, uint16_t size)
{
	uint16_t interval = get_le16(data);
	uint16_t window = get_le16(data + 2);

	print_field("Interval: %u (0x%2.2x)", interval, interval);
	print_field("Window: %u (0x%2.2x)", window, window);
}

static void mgmt_set_secure_connections_cmd(const void *data, uint16_t size)
{
	uint8_t enable = get_u8(data);
	const char *str;

	switch (enable) {
	case 0x00:
		str = "Disabled";
		break;
	case 0x01:
		str = "Enabled";
		break;
	case 0x02:
		str = "Only";
		break;
	default:
		str = "Reserved";
		break;
	}

	print_field("Secure Connections: %s (0x%2.2x)", str, enable);
}

static void mgmt_set_debug_keys_cmd(const void *data, uint16_t size)
{
	uint8_t enable = get_u8(data);
	const char *str;

	switch (enable) {
	case 0x00:
		str = "Disabled";
		break;
	case 0x01:
		str = "Enabled";
		break;
	case 0x02:
		str = "Generate";
		break;
	default:
		str = "Reserved";
		break;
	}

	print_field("Debug Keys: %s (0x%2.2x)", str, enable);
}

static void mgmt_set_privacy_cmd(const void *data, uint16_t size)
{
	uint8_t enable = get_u8(data);
	const char *str;

	switch (enable) {
	case 0x00:
		str = "Disabled";
		break;
	case 0x01:
		str = "Enabled";
		break;
	case 0x02:
		str = "Limited";
		break;
	default:
		str = "Reserved";
		break;
	}

	print_field("Privacy: %s (0x%2.2x)", str, enable);
	print_hex_field("Key", data + 1, 16);
}

static void mgmt_load_identity_resolving_keys_cmd(const void *data, uint16_t size)
{
	uint16_t num_keys = get_le16(data);
	int i;

	print_field("Keys: %u", num_keys);

	if (size - 2 != num_keys * 23) {
		packet_hexdump(data + 2, size - 2);
		return;
	}

	for (i = 0; i < num_keys; i++)
		mgmt_print_identity_resolving_key(data + 2 + (i * 23));
}

static void mgmt_get_connection_information_cmd(const void *data, uint16_t size)
{
	uint8_t address_type = get_u8(data + 6);

	mgmt_print_address(data, address_type);
}

static void mgmt_get_connection_information_rsp(const void *data, uint16_t size)
{
	uint8_t address_type = get_u8(data + 6);
	int8_t rssi = get_s8(data + 7);
	int8_t tx_power = get_s8(data + 8);
	int8_t max_tx_power = get_s8(data + 9);

	mgmt_print_address(data, address_type);
	print_rssi(rssi);
	print_power_level(tx_power, NULL);
	print_power_level(max_tx_power, "max");
}

static void mgmt_get_clock_information_cmd(const void *data, uint16_t size)
{
	uint8_t address_type = get_u8(data + 6);

	mgmt_print_address(data, address_type);
}

static void mgmt_get_clock_information_rsp(const void *data, uint16_t size)
{
	uint8_t address_type = get_u8(data + 6);
	uint32_t local_clock = get_le32(data + 7);
	uint32_t piconet_clock = get_le32(data + 11);
	uint16_t accuracy = get_le16(data + 15);

	mgmt_print_address(data, address_type);
	print_field("Local clock: 0x%8.8x", local_clock);
	print_field("Piconet clock: 0x%8.8x", piconet_clock);
	print_field("Accuracy: 0x%4.4x", accuracy);
}

static void mgmt_add_device_cmd(const void *data, uint16_t size)
{
	uint8_t address_type = get_u8(data + 6);
	uint8_t action = get_u8(data + 7);

	mgmt_print_address(data, address_type);
	mgmt_print_device_action(action);
}

static void mgmt_add_device_rsp(const void *data, uint16_t size)
{
	uint8_t address_type = get_u8(data + 6);

	mgmt_print_address(data, address_type);
}

static void mgmt_remove_device_cmd(const void *data, uint16_t size)
{
	uint8_t address_type = get_u8(data + 6);

	mgmt_print_address(data, address_type);
}

static void mgmt_remove_device_rsp(const void *data, uint16_t size)
{
	uint8_t address_type = get_u8(data + 6);

	mgmt_print_address(data, address_type);
}

static void mgmt_load_connection_parameters_cmd(const void *data, uint16_t size)
{
	uint16_t num_parameters = get_le16(data);
	int i;

	print_field("Parameters: %u", num_parameters);

	if (size - 2 != num_parameters * 15) {
		packet_hexdump(data + 2, size - 2);
		return;
	}

	for (i = 0; i < num_parameters; i++)
		mgmt_print_connection_parameter(data + 2 + (i * 15));
}

static void mgmt_read_unconf_index_list_rsp(const void *data, uint16_t size)
{
	uint16_t num_controllers = get_le16(data);
	int i;

	print_field("Controllers: %u", num_controllers);

	if (size - 2 != num_controllers * 2) {
		packet_hexdump(data + 2, size - 2);
		return;
	}

	for (i = 0; i < num_controllers; i++) {
		uint16_t index = get_le16(data + 2 + (i * 2));

		print_field("  hci%u", index);
	}
}

static void mgmt_read_controller_conf_info_rsp(const void *data, uint16_t size)
{
	uint16_t manufacturer = get_le16(data);
	uint32_t supported_options = get_le32(data + 2);
	uint32_t missing_options = get_le32(data + 6);

	mgmt_print_manufacturer(manufacturer);
	mgmt_print_options("Supported options", supported_options);
	mgmt_print_options("Missing options", missing_options);
}

static void mgmt_set_external_configuration_cmd(const void *data, uint16_t size)
{
	uint8_t enable = get_u8(data);

	print_enable("Configuration", enable);
}

static void mgmt_set_public_address_cmd(const void *data, uint16_t size)
{
	print_addr_resolve("Address", data, 0x00, false);
}

static void mgmt_new_options_rsp(const void *data, uint16_t size)
{
	uint32_t missing_options = get_le32(data);

	mgmt_print_options("Missing options", missing_options);
}

static void mgmt_start_service_discovery_cmd(const void *data, uint16_t size)
{
	uint8_t type = get_u8(data);
	int8_t rssi = get_s8(data + 1);
	uint16_t num_uuids = get_le16(data + 2);
	int i;

	mgmt_print_address_type(type);
	print_rssi(rssi);
	print_field("UUIDs: %u", num_uuids);

	if (size - 4 != num_uuids * 16) {
		packet_hexdump(data + 4, size - 4);
		return;
	}

	for (i = 0; i < num_uuids; i++)
		mgmt_print_uuid(data + 4 + (i * 16));
}

static void mgmt_start_service_discovery_rsp(const void *data, uint16_t size)
{
	uint8_t type = get_u8(data);

	mgmt_print_address_type(type);
}

static void mgmt_read_ext_index_list_rsp(const void *data, uint16_t size)
{
	uint16_t num_controllers = get_le16(data);
	int i;

	print_field("Controllers: %u", num_controllers);

	if (size - 2 != num_controllers * 4) {
		packet_hexdump(data + 2, size - 2);
		return;
	}

	for (i = 0; i < num_controllers; i++) {
		uint16_t index = get_le16(data + 2 + (i * 4));
		uint8_t type = get_u8(data + 4 + (i * 4));
		uint8_t bus = get_u8(data + 5 + (i * 4));
		const char *str;

		switch (type) {
		case 0x00:
			str = "Primary";
			break;
		case 0x01:
			str = "Unconfigured";
			break;
		case 0x02:
			str = "AMP";
			break;
		default:
			str = "Reserved";
			break;
		}

		print_field("  hci%u (%s,%s)", index, str, hci_bustostr(bus));
	}
}

static void mgmt_read_local_oob_ext_data_cmd(const void *data, uint16_t size)
{
	uint8_t type = get_u8(data);

	mgmt_print_address_type(type);
}

static void mgmt_read_local_oob_ext_data_rsp(const void *data, uint16_t size)
{
	uint8_t type = get_u8(data);
	uint16_t data_len = get_le16(data + 1);

	mgmt_print_address_type(type);
	print_field("Data length: %u", data_len);
	print_eir(data + 3, size - 3, true);
}

static void mgmt_read_advertising_features_rsp(const void *data, uint16_t size)
{
	uint32_t flags = get_le32(data);
	uint8_t adv_data_len = get_u8(data + 4);
	uint8_t scan_rsp_len = get_u8(data + 5);
	uint8_t max_instances = get_u8(data + 6);
	uint8_t num_instances = get_u8(data + 7);
	int i;

	mgmt_print_adv_flags(flags);
	print_field("Advertising data length: %u", adv_data_len);
	print_field("Scan response length: %u", scan_rsp_len);
	print_field("Max instances: %u", max_instances);
	print_field("Instances: %u", num_instances);

	if (size - 8 != num_instances) {
		packet_hexdump(data + 8, size - 8);
		return;
	}

	for (i = 0; i < num_instances; i++) {
		uint8_t instance = get_u8(data + 8 + i);

		print_field("  %u", instance);
	}
}

static void mgmt_add_advertising_cmd(const void *data, uint16_t size)
{
	uint8_t instance = get_u8(data);
	uint32_t flags = get_le32(data + 1);
	uint16_t duration = get_le16(data + 5);
	uint16_t timeout = get_le16(data + 7);
	uint8_t adv_data_len = get_u8(data + 9);
	uint8_t scan_rsp_len = get_u8(data + 10);

	print_field("Instance: %u", instance);
	mgmt_print_adv_flags(flags);
	print_field("Duration: %u", duration);
	print_field("Timeout: %u", timeout);
	print_field("Advertising data length: %u", adv_data_len);
	print_eir(data + 11, adv_data_len, false);
	print_field("Scan response length: %u", scan_rsp_len);
	print_eir(data + 11 + adv_data_len, scan_rsp_len, false);
}

static void mgmt_add_advertising_rsp(const void *data, uint16_t size)
{
	uint8_t instance = get_u8(data);

	print_field("Instance: %u", instance);
}

static void mgmt_remove_advertising_cmd(const void *data, uint16_t size)
{
	uint8_t instance = get_u8(data);

	print_field("Instance: %u", instance);
}

static void mgmt_remove_advertising_rsp(const void *data, uint16_t size)
{
	uint8_t instance = get_u8(data);

	print_field("Instance: %u", instance);
}

static void mgmt_get_advertising_size_info_cmd(const void *data, uint16_t size)
{
	uint8_t instance = get_u8(data);
	uint32_t flags = get_le32(data + 1);

	print_field("Instance: %u", instance);
	mgmt_print_adv_flags(flags);
}

static void mgmt_get_advertising_size_info_rsp(const void *data, uint16_t size)
{
	uint8_t instance = get_u8(data);
	uint32_t flags = get_le32(data + 1);
	uint8_t adv_data_len = get_u8(data + 5);
	uint8_t scan_rsp_len = get_u8(data + 6);

	print_field("Instance: %u", instance);
	mgmt_print_adv_flags(flags);
	print_field("Advertising data length: %u", adv_data_len);
	print_field("Scan response length: %u", scan_rsp_len);
}

static void mgmt_start_limited_discovery_cmd(const void *data, uint16_t size)
{
	uint8_t type = get_u8(data);

	mgmt_print_address_type(type);
}

static void mgmt_start_limited_discovery_rsp(const void *data, uint16_t size)
{
	uint8_t type = get_u8(data);

	mgmt_print_address_type(type);
}

static void mgmt_read_ext_controller_info_rsp(const void *data, uint16_t size)
{
	uint8_t version = get_u8(data + 6);
	uint16_t manufacturer = get_le16(data + 7);
	uint32_t supported_settings = get_le32(data + 9);
	uint32_t current_settings = get_le32(data + 13);
	uint16_t data_len = get_le16(data + 17);

	print_addr_resolve("Address", data, 0x00, false);
	mgmt_print_version(version);
	mgmt_print_manufacturer(manufacturer);
	mgmt_print_settings("Supported settings", supported_settings);
	mgmt_print_settings("Current settings", current_settings);
	print_field("Data length: %u", data_len);
	print_eir(data + 19, size - 19, false);
}

static void mgmt_set_apperance_cmd(const void *data, uint16_t size)
{
	uint16_t appearance = get_le16(data);

	print_appearance(appearance);
}

static const struct bitfield_data mgmt_phy_table[] = {
	{  0, "BR 1M 1SLOT"	},
	{  1, "BR 1M 3SLOT"	},
	{  2, "BR 1M 5SLOT"	},
	{  3, "EDR 2M 1SLOT"	},
	{  4, "EDR 2M 3SLOT"	},
	{  5, "EDR 2M 5SLOT"	},
	{  6, "EDR 3M 1SLOT"	},
	{  7, "EDR 3M 3SLOT"	},
	{  8, "EDR 3M 5SLOT"	},
	{  9, "LE 1M TX"	},
	{  10, "LE 1M RX"	},
	{  11, "LE 2M TX"	},
	{  12, "LE 2M RX"	},
	{  13, "LE CODED TX"	},
	{  14, "LE CODED RX"	},
	{ }
};

static void mgmt_print_phys(const char *label, uint16_t phys)
{
	uint16_t mask;

	print_field("%s: 0x%4.4x", label, phys);

	mask = print_bitfield(2, phys, mgmt_phy_table);
	if (mask)
		print_text(COLOR_UNKNOWN_PHY, "  Unknown PHYs"
							" (0x%8.8x)", mask);
}

static void mgmt_get_phy_rsp(const void *data, uint16_t size)
{
	uint32_t supported_phys = get_le32(data);
	uint32_t configurable_phys = get_le32(data + 4);
	uint32_t selected_phys = get_le32(data + 8);

	mgmt_print_phys("Supported PHYs", supported_phys);
	mgmt_print_phys("Configurable PHYs", configurable_phys);
	mgmt_print_phys("Selected PHYs", selected_phys);
}

static void mgmt_set_phy_cmd(const void *data, uint16_t size)
{
	uint32_t selected_phys = get_le32(data);

	mgmt_print_phys("Selected PHYs", selected_phys);
}

static void mgmt_read_exp_features_info_rsp(const void *data, uint16_t size)
{
	uint16_t num_features = get_le16(data);
	int i;

	print_field("Features: %u", num_features);

	if (size - 2 != num_features * 20) {
		packet_hexdump(data + 2, size - 2);
		return;
	}

	for (i = 0; i < num_features; i++)
		mgmt_print_exp_feature(data + 2 + (i * 20));
}

static void mgmt_set_exp_feature_cmd(const void *data, uint16_t size)
{
	uint8_t enable = get_u8(data + 16);

	mgmt_print_uuid(data);
	print_enable("Action", enable);
}

static void mgmt_set_exp_feature_rsp(const void *data, uint16_t size)
{
	mgmt_print_exp_feature(data);
}

static const struct bitfield_data mgmt_added_device_flags_table[] = {
	{ 0, "Remote Wakeup"	},
	{ }
};

static void mgmt_print_added_device_flags(char *label, uint32_t flags)
{
	uint32_t mask;

	print_field("%s: 0x%8.8x", label, flags);
	mask = print_bitfield(2, flags, mgmt_added_device_flags_table);
	if (mask)
		print_text(COLOR_UNKNOWN_ADDED_DEVICE_FLAG,
			   "  Unknown Flags (0x%8.8x)", mask);
}

static void mgmt_get_device_flags_cmd(const void *data, uint16_t size)
{
	uint8_t type = get_u8(data + 6);

	mgmt_print_address(data, type);
}

static void mgmt_get_device_flags_rsp(const void *data, uint16_t size)
{
	uint8_t type = get_u8(data + 6);
	uint32_t supported_flags = get_le32(data + 7);
	uint32_t current_flags = get_le32(data + 11);

	mgmt_print_address(data, type);
	mgmt_print_added_device_flags("Supported Flags", supported_flags);
	mgmt_print_added_device_flags("Current Flags", current_flags);
}

static void mgmt_set_device_flags_cmd(const void *data, uint16_t size)
{
	uint8_t type = get_u8(data + 6);
	uint32_t current_flags = get_le32(data + 7);

	mgmt_print_address(data, type);
	mgmt_print_added_device_flags("Current Flags", current_flags);
}

static void mgmt_set_device_flags_rsp(const void *data, uint16_t size)
{
	uint8_t type = get_u8(data + 6);

	mgmt_print_address(data, type);
}
static void mgmt_add_ext_adv_params_cmd(const void *data, uint16_t size)
{
	uint8_t instance = get_u8(data);
	uint32_t flags = get_le32(data + 1);
	uint16_t duration = get_le16(data + 5);
	uint16_t timeout = get_le16(data + 7);
	uint8_t *min_interval = (uint8_t *)(data + 9);
	uint8_t *max_interval = (uint8_t *)(data + 13);
	int8_t tx_power = get_s8(data + 17);

	print_field("Instance: %u", instance);
	mgmt_print_adv_flags(flags);
	print_field("Duration: %u", duration);
	print_field("Timeout: %u", timeout);
	print_ext_slot_625("Min advertising interval", min_interval);
	print_ext_slot_625("Max advertising interval", max_interval);
	print_power_level(tx_power, NULL);
}

static void mgmt_add_ext_adv_params_rsp(const void *data, uint16_t size)
{
	uint8_t instance = get_u8(data);
	int8_t tx_power = get_s8(data + 1);
	uint8_t max_adv_data_len = get_u8(data+2);
	uint8_t max_scan_rsp_len = get_u8(data+3);

	print_field("Instance: %u", instance);
	print_power_level(tx_power, NULL);
	print_field("Available adv data len: %u", max_adv_data_len);
	print_field("Available scan rsp data len: %u", max_scan_rsp_len);
}

static void mgmt_add_ext_adv_data_cmd(const void *data, uint16_t size)
{
	uint8_t instance = get_u8(data);
	uint8_t adv_data_len = get_u8(data + 1);
	uint8_t scan_rsp_len = get_u8(data + 2);

	print_field("Instance: %u", instance);
	print_field("Advertising data length: %u", adv_data_len);
	print_eir(data + 3, adv_data_len, false);
	print_field("Scan response length: %u", scan_rsp_len);
	print_eir(data + 3 + adv_data_len, scan_rsp_len, false);
}

static void mgmt_add_ext_adv_data_rsp(const void *data, uint16_t size)
{
	uint8_t instance = get_u8(data);

	print_field("Instance: %u", instance);
}

static const struct bitfield_data mgmt_adv_monitor_features_table[] = {
	{ 1, "OR Patterns"	},
	{ }
};

static void mgmt_print_adv_monitor_features(char *label, uint32_t flags)
{
	uint32_t mask;

	print_field("%s: 0x%8.8x", label, flags);
	mask = print_bitfield(2, flags, mgmt_adv_monitor_features_table);
	if (mask)
		print_text(COLOR_UNKNOWN_ADVMON_FEATURES,
			   "  Unknown Flags (0x%8.8x)", mask);
}

static void mgmt_print_adv_monitor_handles(const void *data, uint8_t len)
{
	uint8_t idx = 0;

	while (idx + 2 <= len) {
		print_field("  Handle: %d", get_le16(data + idx));
		idx += 2;
	}
}

static void mgmt_read_adv_monitor_features_rsp(const void *data, uint16_t size)
{
	uint32_t supported_features = get_le32(data);
	uint32_t enabled_features = get_le32(data + 4);
	uint16_t max_num_handles = get_le16(data + 8);
	uint8_t max_num_patterns = get_u8(data + 10);
	uint16_t num_handles = get_le16(data + 11);

	mgmt_print_adv_monitor_features("Supported Features",
							supported_features);
	mgmt_print_adv_monitor_features("Enabled Features",
							enabled_features);
	print_field("Max number of handles: %d", max_num_handles);
	print_field("Max number of patterns: %d", max_num_patterns);
	print_field("Number of handles: %d", num_handles);
	mgmt_print_adv_monitor_handles(data + 13, size - 13);
}

static void mgmt_print_adv_monitor_patterns(const void *data, uint8_t len)
{
	uint8_t data_idx = 0, pattern_idx = 1;

	/* Reference: struct mgmt_adv_pattern in lib/mgmt.h. */
	while (data_idx + 34 <= len) {
		uint8_t ad_type = get_u8(data);
		uint8_t offset = get_u8(data + 1);
		uint8_t length = get_u8(data + 2);

		print_field("  Pattern %d:", pattern_idx);
		print_field("    AD type: %d", ad_type);
		print_field("    Offset: %d", offset);
		print_field("    Length: %d", length);
		if (length <= 31)
			print_hex_field("    Value ", data + 3, length);
		else
			print_text(COLOR_ERROR, "    invalid length");

		pattern_idx += 1;
		data_idx += 34;
		data += 34;
	}
}

static void mgmt_add_adv_monitor_patterns_cmd(const void *data, uint16_t size)
{
	uint8_t pattern_count = get_u8(data);

	print_field("Number of patterns: %d", pattern_count);
	mgmt_print_adv_monitor_patterns(data + 1, size - 1);
}

static void mgmt_add_adv_monitor_patterns_rssi_cmd(const void *data,
								uint16_t size)
{
	int8_t high_rssi = get_s8(data);
	uint16_t high_rssi_timeout = get_le16(data + 1);
	int8_t low_rssi = get_s8(data + 3);
	uint16_t low_rssi_timeout = get_le16(data + 4);
	uint8_t sampling_period = get_u8(data + 6);
	uint8_t pattern_count = get_u8(data + 7);

	print_field("RSSI data:");
	print_field("  high threshold: %d dBm", high_rssi);
	print_field("  high timeout: %d seconds", high_rssi_timeout);
	print_field("  low threshold: %d dBm", low_rssi);
	print_field("  low timeout: %d seconds", low_rssi_timeout);

	if (sampling_period == 0)
		print_field("  sampling: propagate all (0x00)");
	else if (sampling_period == 0xff)
		print_field("  sampling: just once (0xFF)");
	else
		print_field("  sampling: every %d ms", 100 * sampling_period);

	print_field("Number of patterns: %d", pattern_count);
	mgmt_print_adv_monitor_patterns(data + 8, size - 8);
}

static void mgmt_add_adv_monitor_patterns_rsp(const void *data, uint16_t size)
{
	uint16_t handle = get_le16(data);

	print_field("Handle: %d", handle);
}

static void mgmt_remove_adv_monitor_patterns_cmd(const void *data,
								uint16_t size)
{
	uint16_t handle = get_le16(data);

	print_field("Handle: %d", handle);
}

static void mgmt_remove_adv_monitor_patterns_rsp(const void *data,
								uint16_t size)
{
	uint16_t handle = get_le16(data);

	print_field("Handle: %d", handle);
}

struct mgmt_data {
	uint16_t opcode;
	const char *str;
	void (*func) (const void *data, uint16_t size);
	uint16_t size;
	bool fixed;
	void (*rsp_func) (const void *data, uint16_t size);
	uint16_t rsp_size;
	bool rsp_fixed;
};

static const struct mgmt_data mgmt_command_table[] = {
	{ 0x0001, "Read Management Version Information",
				mgmt_null_cmd, 0, true,
				mgmt_read_version_info_rsp, 3, true },
	{ 0x0002, "Read Management Supported Commands",
				mgmt_null_cmd, 0, true,
				mgmt_read_supported_commands_rsp, 4, false },
	{ 0x0003, "Read Controller Index List",
				mgmt_null_cmd, 0, true,
				mgmt_read_index_list_rsp, 2, false },
	{ 0x0004, "Read Controller Information",
				mgmt_null_cmd, 0, true,
				mgmt_read_controller_info_rsp, 280, true },
	{ 0x0005, "Set Powered",
				mgmt_set_powered_cmd, 1, true,
				mgmt_new_settings_rsp, 4, true },
	{ 0x0006, "Set Discoverable",
				mgmt_set_discoverable_cmd, 3, true,
				mgmt_new_settings_rsp, 4, true },
	{ 0x0007, "Set Connectable",
				mgmt_set_connectable_cmd, 1, true,
				mgmt_new_settings_rsp, 4, true },
	{ 0x0008, "Set Fast Connectable",
				mgmt_set_fast_connectable_cmd, 1, true,
				mgmt_new_settings_rsp, 4, true },
	{ 0x0009, "Set Bondable",
				mgmt_set_bondable_cmd, 1, true,
				mgmt_new_settings_rsp, 4, true },
	{ 0x000a, "Set Link Security",
				mgmt_set_link_security_cmd, 1, true,
				mgmt_new_settings_rsp, 4, true },
	{ 0x000b, "Set Secure Simple Pairing",
				mgmt_set_secure_simple_pairing_cmd, 1, true,
				mgmt_new_settings_rsp, 4, true },
	{ 0x000c, "Set High Speed",
				mgmt_set_high_speed_cmd, 1, true,
				mgmt_new_settings_rsp, 4, true },
	{ 0x000d, "Set Low Energy",
				mgmt_set_low_energy_cmd, 1, true,
				mgmt_new_settings_rsp, 4, true },
	{ 0x000e, "Set Device Class",
				mgmt_set_device_class_cmd, 2, true,
				mgmt_set_device_class_rsp, 3, true },
	{ 0x000f, "Set Local Name",
				mgmt_set_local_name_cmd, 260, true,
				mgmt_set_local_name_rsp, 260, true },
	{ 0x0010, "Add UUID",
				mgmt_add_uuid_cmd, 17, true,
				mgmt_add_uuid_rsp, 3, true },
	{ 0x0011, "Remove UUID",
				mgmt_remove_uuid_cmd, 16, true,
				mgmt_remove_uuid_rsp, 3, true },
	{ 0x0012, "Load Link Keys",
				mgmt_load_link_keys_cmd, 3, false,
				mgmt_null_rsp, 0, true },
	{ 0x0013, "Load Long Term Keys",
				mgmt_load_long_term_keys_cmd, 2, false,
				mgmt_null_rsp, 0, true },
	{ 0x0014, "Disconnect",
				mgmt_disconnect_cmd, 7, true,
				mgmt_disconnect_rsp, 7, true },
	{ 0x0015, "Get Connections",
				mgmt_null_cmd, 0, true,
				mgmt_get_connections_rsp, 2, false },
	{ 0x0016, "PIN Code Reply",
				mgmt_pin_code_reply_cmd, 24, true,
				mgmt_pin_code_reply_rsp, 7, true },
	{ 0x0017, "PIN Code Negative Reply",
				mgmt_pin_code_neg_reply_cmd, 7, true,
				mgmt_pin_code_neg_reply_rsp, 7, true },
	{ 0x0018, "Set IO Capability",
				mgmt_set_io_capability_cmd, 1, true,
				mgmt_null_rsp, 0, true },
	{ 0x0019, "Pair Device",
				mgmt_pair_device_cmd, 8, true,
				mgmt_pair_device_rsp, 7, true },
	{ 0x001a, "Cancel Pair Device",
				mgmt_cancel_pair_device_cmd, 7, true,
				mgmt_cancel_pair_device_rsp, 7, true },
	{ 0x001b, "Unpair Device",
				mgmt_unpair_device_cmd, 8, true,
				mgmt_unpair_device_rsp, 7, true },
	{ 0x001c, "User Confirmation Reply",
				mgmt_user_confirmation_reply_cmd, 7, true,
				mgmt_user_confirmation_reply_rsp, 7, true },
	{ 0x001d, "User Confirmation Negative Reply",
				mgmt_user_confirmation_neg_reply_cmd, 7, true,
				mgmt_user_confirmation_neg_reply_rsp, 7, true },
	{ 0x001e, "User Passkey Reply",
				mgmt_user_passkey_reply_cmd, 11, true,
				mgmt_user_passkey_reply_rsp, 7, true },
	{ 0x001f, "User Passkey Negative Reply",
				mgmt_user_passkey_neg_reply_cmd, 7, true,
				mgmt_user_passkey_neg_reply_rsp, 7, true },
	{ 0x0020, "Read Local Out Of Band Data",
				mgmt_null_cmd, 0, true,
				mgmt_read_local_oob_data_rsp, 64, true },
	{ 0x0021, "Add Remote Out Of Band Data",
				mgmt_add_remote_oob_data_cmd, 71, true,
				mgmt_add_remote_oob_data_rsp, 7, true },
	{ 0x0022, "Remove Remote Out Of Band Data",
				mgmt_remove_remote_oob_data_cmd, 7, true,
				mgmt_remove_remote_oob_data_rsp, 7, true },
	{ 0x0023, "Start Discovery",
				mgmt_start_discovery_cmd, 1, true,
				mgmt_start_discovery_rsp, 1, true },
	{ 0x0024, "Stop Discovery",
				mgmt_stop_discovery_cmd, 1, true,
				mgmt_stop_discovery_rsp, 1, true },
	{ 0x0025, "Confirm Name",
				mgmt_confirm_name_cmd, 8, true,
				mgmt_confirm_name_rsp, 7, true },
	{ 0x0026, "Block Device",
				mgmt_block_device_cmd, 7, true,
				mgmt_block_device_rsp, 7, true },
	{ 0x0027, "Unblock Device",
				mgmt_unblock_device_cmd, 7, true,
				mgmt_unblock_device_rsp, 7, true },
	{ 0x0028, "Set Device ID",
				mgmt_set_device_id_cmd, 8, true,
				mgmt_null_rsp, 0, true },
	{ 0x0029, "Set Advertising",
				mgmt_set_advertising_cmd, 1, true,
				mgmt_new_settings_rsp, 4, true },
	{ 0x002a, "Set BR/EDR",
				mgmt_set_bredr_cmd, 1, true,
				mgmt_new_settings_rsp, 4, true },
	{ 0x002b, "Set Static Address",
				mgmt_set_static_address_cmd, 6, true,
				mgmt_new_settings_rsp, 4, true },
	{ 0x002c, "Set Scan Parameters",
				mgmt_set_scan_parameters_cmd, 4, true,
				mgmt_null_rsp, 0, true },
	{ 0x002d, "Set Secure Connections",
				mgmt_set_secure_connections_cmd, 1, true,
				mgmt_new_settings_rsp, 4, true },
	{ 0x002e, "Set Debug Keys",
				mgmt_set_debug_keys_cmd, 1, true,
				mgmt_new_settings_rsp, 4, true },
	{ 0x002f, "Set Privacy",
				mgmt_set_privacy_cmd, 17, true,
				mgmt_new_settings_rsp, 4, true },
	{ 0x0030, "Load Identity Resolving Keys",
				mgmt_load_identity_resolving_keys_cmd, 2, false,
				mgmt_null_rsp, 0, true },
	{ 0x0031, "Get Connection Information",
				mgmt_get_connection_information_cmd, 7, true,
				mgmt_get_connection_information_rsp, 10, true },
	{ 0x0032, "Get Clock Information",
				mgmt_get_clock_information_cmd, 7, true,
				mgmt_get_clock_information_rsp, 17, true },
	{ 0x0033, "Add Device",
				mgmt_add_device_cmd, 8, true,
				mgmt_add_device_rsp, 7, true },
	{ 0x0034, "Remove Device",
				mgmt_remove_device_cmd, 7, true,
				mgmt_remove_device_rsp, 7, true },
	{ 0x0035, "Load Connection Parameters",
				mgmt_load_connection_parameters_cmd, 2, false,
				mgmt_null_rsp, 0, true },
	{ 0x0036, "Read Unconfigured Controller Index List",
				mgmt_null_cmd, 0, true,
				mgmt_read_unconf_index_list_rsp, 2, false },
	{ 0x0037, "Read Controller Configuration Information",
				mgmt_null_cmd, 0, true,
				mgmt_read_controller_conf_info_rsp, 10, true },
	{ 0x0038, "Set External Configuration",
				mgmt_set_external_configuration_cmd, 1, true,
				mgmt_new_options_rsp, 4, true },
	{ 0x0039, "Set Public Address",
				mgmt_set_public_address_cmd, 6, true,
				mgmt_new_options_rsp, 4, true },
	{ 0x003a, "Start Service Discovery",
				mgmt_start_service_discovery_cmd, 3, false,
				mgmt_start_service_discovery_rsp, 1, true },
	{ 0x003b, "Read Local Out Of Band Extended Data",
				mgmt_read_local_oob_ext_data_cmd, 1, true,
				mgmt_read_local_oob_ext_data_rsp, 3, false },
	{ 0x003c, "Read Extended Controller Index List",
				mgmt_null_cmd, 0, true,
				mgmt_read_ext_index_list_rsp, 2, false },
	{ 0x003d, "Read Advertising Features",
				mgmt_null_cmd, 0, true,
				mgmt_read_advertising_features_rsp, 8, false },
	{ 0x003e, "Add Advertising",
				mgmt_add_advertising_cmd, 11, false,
				mgmt_add_advertising_rsp, 1, true },
	{ 0x003f, "Remove Advertising",
				mgmt_remove_advertising_cmd, 1, true,
				mgmt_remove_advertising_rsp, 1, true },
	{ 0x0040, "Get Advertising Size Information",
				mgmt_get_advertising_size_info_cmd, 5, true,
				mgmt_get_advertising_size_info_rsp, 7, true },
	{ 0x0041, "Start Limited Discovery",
				mgmt_start_limited_discovery_cmd, 1, true,
				mgmt_start_limited_discovery_rsp, 1, true },
	{ 0x0042, "Read Extended Controller Information",
				mgmt_null_cmd, 0, true,
				mgmt_read_ext_controller_info_rsp, 19, false },
	{ 0x0043, "Set Appearance",
				mgmt_set_apperance_cmd, 2, true,
				mgmt_null_rsp, 0, true },
	{ 0x0044, "Get PHY Configuration",
				mgmt_null_cmd, 0, true,
				mgmt_get_phy_rsp, 12, true },
	{ 0x0045, "Set PHY Configuration",
				mgmt_set_phy_cmd, 4, true,
				mgmt_null_rsp, 0, true },
	{ 0x0049, "Read Experimental Features Information",
				mgmt_null_cmd, 0, true,
				mgmt_read_exp_features_info_rsp, 2, false },
	{ 0x004a, "Set Experimental Feature",
				mgmt_set_exp_feature_cmd, 17, true,
				mgmt_set_exp_feature_rsp, 20, true },
	{ 0x004f, "Get Device Flags",
				mgmt_get_device_flags_cmd, 7, true,
				mgmt_get_device_flags_rsp, 15, true},
	{ 0x0050, "Set Device Flags",
				mgmt_set_device_flags_cmd, 11, true,
				mgmt_set_device_flags_rsp, 7, true},
	{ 0x0051, "Read Advertisement Monitor Features",
				mgmt_null_cmd, 0, true,
				mgmt_read_adv_monitor_features_rsp, 13, false},
	{ 0x0052, "Add Advertisement Monitor",
				mgmt_add_adv_monitor_patterns_cmd, 1, false,
				mgmt_add_adv_monitor_patterns_rsp, 2, true},
	{ 0x0053, "Remove Advertisement Monitor",
				mgmt_remove_adv_monitor_patterns_cmd, 2, true,
				mgmt_remove_adv_monitor_patterns_rsp, 2, true},
	{ 0x0054, "Add Ext Adv Params",
				mgmt_add_ext_adv_params_cmd, 18, false,
				mgmt_add_ext_adv_params_rsp, 4, true },
	{ 0x0055, "Add Ext Adv Data",
				mgmt_add_ext_adv_data_cmd, 3, false,
				mgmt_add_ext_adv_data_rsp, 1, true },
	{ 0x0056, "Add Advertisement Monitor With RSSI",
				mgmt_add_adv_monitor_patterns_rssi_cmd, 8,
									false,
				mgmt_add_adv_monitor_patterns_rsp, 2, true},
	{ }
};

static void mgmt_null_evt(const void *data, uint16_t size)
{
}

static void mgmt_command_complete_evt(const void *data, uint16_t size)
{
	uint16_t opcode;
	uint8_t status;
	const struct mgmt_data *mgmt_data = NULL;
	const char *mgmt_color, *mgmt_str;
	int i;

	opcode = get_le16(data);
	status = get_u8(data + 2);

	data += 3;
	size -= 3;

	for (i = 0; mgmt_command_table[i].str; i++) {
		if (mgmt_command_table[i].opcode == opcode) {
			mgmt_data = &mgmt_command_table[i];
			break;
		}
	}

	if (mgmt_data) {
		if (mgmt_data->rsp_func)
			mgmt_color = COLOR_CTRL_COMMAND;
		else
			mgmt_color = COLOR_CTRL_COMMAND_UNKNOWN;
		mgmt_str = mgmt_data->str;
	} else {
		mgmt_color = COLOR_CTRL_COMMAND_UNKNOWN;
		mgmt_str = "Unknown";
	}

	print_indent(6, mgmt_color, "", mgmt_str, COLOR_OFF,
					" (0x%4.4x) plen %u", opcode, size);

	mgmt_print_status(status);

	if (!mgmt_data || !mgmt_data->rsp_func) {
		packet_hexdump(data, size);
		return;
	}

	if (mgmt_data->rsp_fixed) {
		if (size != mgmt_data->rsp_size) {
			print_text(COLOR_ERROR, "invalid packet size");
			packet_hexdump(data, size);
			return;
		}
	} else {
		if (size < mgmt_data->rsp_size) {
			print_text(COLOR_ERROR, "too short packet");
			packet_hexdump(data, size);
			return;
		}
	}

	mgmt_data->rsp_func(data, size);
}

static void mgmt_command_status_evt(const void *data, uint16_t size)
{
	uint16_t opcode;
	uint8_t status;
	const struct mgmt_data *mgmt_data = NULL;
	const char *mgmt_color, *mgmt_str;
	int i;

	opcode = get_le16(data);
	status = get_u8(data + 2);

	for (i = 0; mgmt_command_table[i].str; i++) {
		if (mgmt_command_table[i].opcode == opcode) {
			mgmt_data = &mgmt_command_table[i];
			break;
		}
	}

	if (mgmt_data) {
		mgmt_color = COLOR_CTRL_COMMAND;
		mgmt_str = mgmt_data->str;
	} else {
		mgmt_color = COLOR_CTRL_COMMAND_UNKNOWN;
		mgmt_str = "Unknown";
	}

	print_indent(6, mgmt_color, "", mgmt_str, COLOR_OFF,
						" (0x%4.4x)", opcode);

	mgmt_print_status(status);
}

static void mgmt_controller_error_evt(const void *data, uint16_t size)
{
	uint8_t error = get_u8(data);

	print_field("Error: 0x%2.2x", error);
}

static void mgmt_new_settings_evt(const void *data, uint16_t size)
{
	uint32_t settings = get_le32(data);

	mgmt_print_settings("Current settings", settings);
}

static void mgmt_class_of_dev_changed_evt(const void *data, uint16_t size)
{
	print_dev_class(data);
}

static void mgmt_local_name_changed_evt(const void *data, uint16_t size)
{
	mgmt_print_name(data);
}

static void mgmt_new_link_key_evt(const void *data, uint16_t size)
{
	uint8_t store_hint = get_u8(data);

	mgmt_print_store_hint(store_hint);
	mgmt_print_link_key(data + 1);
}

static void mgmt_new_long_term_key_evt(const void *data, uint16_t size)
{
	uint8_t store_hint = get_u8(data);

	mgmt_print_store_hint(store_hint);
	mgmt_print_long_term_key(data + 1);
}

static void mgmt_device_connected_evt(const void *data, uint16_t size)
{
	uint8_t address_type = get_u8(data + 6);
	uint32_t flags = get_le32(data + 7);
	uint16_t data_len = get_le16(data + 11);

	mgmt_print_address(data, address_type);
	mgmt_print_device_flags(flags);
	print_field("Data length: %u", data_len);
	print_eir(data + 13, size - 13, false);
}

static void mgmt_device_disconnected_evt(const void *data, uint16_t size)
{
	uint8_t address_type = get_u8(data + 6);
	uint8_t reason = get_u8(data + 7);
	const char *str;

	mgmt_print_address(data, address_type);

	switch (reason) {
	case 0x00:
		str = "Unspecified";
		break;
	case 0x01:
		str = "Connection timeout";
		break;
	case 0x02:
		str = "Connection terminated by local host";
		break;
	case 0x03:
		str = "Connection terminated by remote host";
		break;
	case 0x04:
		str = "Connection terminated due to authentication failure";
		break;
	case 0x05:
		str = "Connection terminated by local host for suspend";
		break;
	default:
		str = "Reserved";
		break;
	}

	print_field("Reason: %s (0x%2.2x)", str, reason);
}

static void mgmt_connect_failed_evt(const void *data, uint16_t size)
{
	uint8_t address_type = get_u8(data + 6);
	uint8_t status = get_u8(data + 7);

	mgmt_print_address(data, address_type);
	mgmt_print_status(status);
}

static void mgmt_pin_code_request_evt(const void *data, uint16_t size)
{
	uint8_t address_type = get_u8(data + 6);
	uint8_t secure_pin = get_u8(data + 7);

	mgmt_print_address(data, address_type);
	print_field("Secure PIN: 0x%2.2x", secure_pin);
}

static void mgmt_user_confirmation_request_evt(const void *data, uint16_t size)
{
	uint8_t address_type = get_u8(data + 6);
	uint8_t confirm_hint = get_u8(data + 7);
	uint32_t value = get_le32(data + 8);

	mgmt_print_address(data, address_type);
	print_field("Confirm hint: 0x%2.2x", confirm_hint);
	print_field("Value: 0x%8.8x", value);
}

static void mgmt_user_passkey_request_evt(const void *data, uint16_t size)
{
	uint8_t address_type = get_u8(data + 6);

	mgmt_print_address(data, address_type);
}

static void mgmt_authentication_failed_evt(const void *data, uint16_t size)
{
	uint8_t address_type = get_u8(data + 6);
	uint8_t status = get_u8(data + 7);

	mgmt_print_address(data, address_type);
	mgmt_print_status(status);
}

static void mgmt_device_found_evt(const void *data, uint16_t size)
{
	uint8_t address_type = get_u8(data + 6);
	int8_t rssi = get_s8(data + 7);
	uint32_t flags = get_le32(data + 8);
	uint16_t data_len = get_le16(data + 12);

	mgmt_print_address(data, address_type);
	print_rssi(rssi);
	mgmt_print_device_flags(flags);
	print_field("Data length: %u", data_len);
	print_eir(data + 14, size - 14, false);
}

static void mgmt_discovering_evt(const void *data, uint16_t size)
{
	uint8_t type = get_u8(data);
	uint8_t enable = get_u8(data + 1);

	mgmt_print_address_type(type);
	print_enable("Discovery", enable);
}

static void mgmt_device_blocked_evt(const void *data, uint16_t size)
{
	uint8_t address_type = get_u8(data + 6);

	mgmt_print_address(data, address_type);
}

static void mgmt_device_unblocked_evt(const void *data, uint16_t size)
{
	uint8_t address_type = get_u8(data + 6);

	mgmt_print_address(data, address_type);
}

static void mgmt_device_unpaired_evt(const void *data, uint16_t size)
{
	uint8_t address_type = get_u8(data + 6);

	mgmt_print_address(data, address_type);
}

static void mgmt_passkey_notify_evt(const void *data, uint16_t size)
{
	uint8_t address_type = get_u8(data + 6);
	uint32_t passkey = get_le32(data + 7);
	uint8_t entered = get_u8(data + 11);

	mgmt_print_address(data, address_type);
	print_field("Passkey: 0x%8.8x", passkey);
	print_field("Entered: %u", entered);
}

static void mgmt_new_identity_resolving_key_evt(const void *data, uint16_t size)
{
	uint8_t store_hint = get_u8(data);

	mgmt_print_store_hint(store_hint);
	print_addr_resolve("Random address", data + 1, 0x01, false);
	mgmt_print_identity_resolving_key(data + 7);
}

static void mgmt_new_signature_resolving_key_evt(const void *data, uint16_t size)
{
	uint8_t store_hint = get_u8(data);

	mgmt_print_store_hint(store_hint);
	mgmt_print_signature_resolving_key(data + 1);
}

static void mgmt_device_added_evt(const void *data, uint16_t size)
{
	uint8_t address_type = get_u8(data + 6);
	uint8_t action = get_u8(data + 7);

	mgmt_print_address(data, address_type);
	mgmt_print_device_action(action);
}

static void mgmt_device_removed_evt(const void *data, uint16_t size)
{
	uint8_t address_type = get_u8(data + 6);

	mgmt_print_address(data, address_type);
}

static void mgmt_new_connection_parameter_evt(const void *data, uint16_t size)
{
	uint8_t store_hint = get_u8(data);

	mgmt_print_store_hint(store_hint);
	mgmt_print_connection_parameter(data + 1);
}

static void mgmt_new_conf_options_evt(const void *data, uint16_t size)
{
	uint32_t missing_options = get_le32(data);

	mgmt_print_options("Missing options", missing_options);
}

static void mgmt_ext_index_added_evt(const void *data, uint16_t size)
{
	uint8_t type = get_u8(data);
	uint8_t bus = get_u8(data + 1);

	print_field("type 0x%2.2x - bus 0x%2.2x", type, bus);
}

static void mgmt_ext_index_removed_evt(const void *data, uint16_t size)
{
	uint8_t type = get_u8(data);
	uint8_t bus = get_u8(data + 1);

	print_field("type 0x%2.2x - bus 0x%2.2x", type, bus);
}

static void mgmt_local_oob_ext_data_updated_evt(const void *data, uint16_t size)
{
	uint8_t type = get_u8(data);
	uint16_t data_len = get_le16(data + 1);

	mgmt_print_address_type(type);
	print_field("Data length: %u", data_len);
	print_eir(data + 3, size - 3, true);
}

static void mgmt_advertising_added_evt(const void *data, uint16_t size)
{
	uint8_t instance = get_u8(data);

	print_field("Instance: %u", instance);
}

static void mgmt_advertising_removed_evt(const void *data, uint16_t size)
{
	uint8_t instance = get_u8(data);

	print_field("Instance: %u", instance);
}

static void mgmt_ext_controller_info_changed_evt(const void *data, uint16_t size)
{
	uint16_t data_len = get_le16(data);

	print_field("Data length: %u", data_len);
	print_eir(data + 2, size - 2, false);
}

static void mgmt_phy_changed_evt(const void *data, uint16_t size)
{
	uint32_t selected_phys = get_le32(data);

	mgmt_print_phys("Selected PHYs", selected_phys);
}

static void mgmt_exp_feature_changed_evt(const void *data, uint16_t size)
{
	mgmt_print_exp_feature(data);
}

static void mgmt_device_flags_changed_evt(const void *data, uint16_t size)
{
	uint8_t type = get_u8(data + 6);
	uint32_t supported_flags = get_le32(data + 7);
	uint32_t current_flags = get_le32(data + 11);

	mgmt_print_address(data, type);
	mgmt_print_added_device_flags("Supported Flags", supported_flags);
	mgmt_print_added_device_flags("Current Flags", current_flags);
}

static void mgmt_adv_monitor_added_evt(const void *data, uint16_t size)
{
	uint16_t handle = get_le16(data);

	print_field("Handle: %d", handle);
}

static void mgmt_adv_monitor_removed_evt(const void *data, uint16_t size)
{
	uint16_t handle = get_le16(data);

	print_field("Handle: %d", handle);
}

static void mgmt_controller_suspend_evt(const void *data, uint16_t size)
{
	uint8_t state = get_u8(data);
	char *str;

	switch (state) {
	case 0x0:
		str = "Controller running (failed to suspend)";
		break;
	case 0x1:
		str = "Disconnected and not scanning";
		break;
	case 0x2:
		str = "Page scanning and/or passive scanning";
		break;
	default:
		str = "Unknown suspend state";
		break;
	}

	print_field("Suspend state: %s (%d)", str, state);
}

static void mgmt_controller_resume_evt(const void *data, uint16_t size)
{
	uint8_t addr_type = get_u8(data + 6);
	uint8_t wake_reason = get_u8(data + 7);
	char *str;

	switch (wake_reason) {
	case 0x0:
		str = "Resume from non-Bluetooth wake source";
		break;
	case 0x1:
		str = "Wake due to unexpected event";
		break;
	case 0x2:
		str = "Remote wake due to peer device connection";
		break;
	default:
		str = "Unknown wake reason";
		break;
	}

	print_field("Wake reason: %s (%d)", str, wake_reason);
	mgmt_print_address(data, addr_type);
}

static const struct mgmt_data mgmt_event_table[] = {
	{ 0x0001, "Command Complete",
			mgmt_command_complete_evt, 3, false },
	{ 0x0002, "Command Status",
			mgmt_command_status_evt, 3, true },
	{ 0x0003, "Controller Error",
			mgmt_controller_error_evt, 1, true },
	{ 0x0004, "Index Added",
			mgmt_null_evt, 0, true },
	{ 0x0005, "Index Removed",
			mgmt_null_evt, 0, true },
	{ 0x0006, "New Settings",
			mgmt_new_settings_evt, 4, true },
	{ 0x0007, "Class Of Device Changed",
			mgmt_class_of_dev_changed_evt, 3, true },
	{ 0x0008, "Local Name Changed",
			mgmt_local_name_changed_evt, 260, true },
	{ 0x0009, "New Link Key",
			mgmt_new_link_key_evt, 26, true },
	{ 0x000a, "New Long Term Key",
			mgmt_new_long_term_key_evt, 37, true },
	{ 0x000b, "Device Connected",
			mgmt_device_connected_evt, 13, false },
	{ 0x000c, "Device Disconnected",
			mgmt_device_disconnected_evt, 8, true },
	{ 0x000d, "Connect Failed",
			mgmt_connect_failed_evt, 8, true },
	{ 0x000e, "PIN Code Request",
			mgmt_pin_code_request_evt, 8, true },
	{ 0x000f, "User Confirmation Request",
			mgmt_user_confirmation_request_evt, 12, true },
	{ 0x0010, "User Passkey Request",
			mgmt_user_passkey_request_evt, 7, true },
	{ 0x0011, "Authentication Failed",
			mgmt_authentication_failed_evt, 8, true },
	{ 0x0012, "Device Found",
			mgmt_device_found_evt, 14, false },
	{ 0x0013, "Discovering",
			mgmt_discovering_evt, 2, true },
	{ 0x0014, "Device Blocked",
			mgmt_device_blocked_evt, 7, true },
	{ 0x0015, "Device Unblocked",
			mgmt_device_unblocked_evt, 7, true },
	{ 0x0016, "Device Unpaired",
			mgmt_device_unpaired_evt, 7, true },
	{ 0x0017, "Passkey Notify",
			mgmt_passkey_notify_evt, 12, true },
	{ 0x0018, "New Identity Resolving Key",
			mgmt_new_identity_resolving_key_evt, 30, true },
	{ 0x0019, "New Signature Resolving Key",
			mgmt_new_signature_resolving_key_evt, 25, true },
	{ 0x001a, "Device Added",
			mgmt_device_added_evt, 8, true },
	{ 0x001b, "Device Removed",
			mgmt_device_removed_evt, 7, true },
	{ 0x001c, "New Connection Parameter",
			mgmt_new_connection_parameter_evt, 16, true },
	{ 0x001d, "Unconfigured Index Added",
			mgmt_null_evt, 0, true },
	{ 0x001e, "Unconfigured Index Removed",
			mgmt_null_evt, 0, true },
	{ 0x001f, "New Configuration Options",
			mgmt_new_conf_options_evt, 4, true },
	{ 0x0020, "Extended Index Added",
			mgmt_ext_index_added_evt, 2, true },
	{ 0x0021, "Extended Index Removed",
			mgmt_ext_index_removed_evt, 2, true },
	{ 0x0022, "Local Out Of Band Extended Data Updated",
			mgmt_local_oob_ext_data_updated_evt, 3, false },
	{ 0x0023, "Advertising Added",
			mgmt_advertising_added_evt, 1, true },
	{ 0x0024, "Advertising Removed",
			mgmt_advertising_removed_evt, 1, true },
	{ 0x0025, "Extended Controller Information Changed",
			mgmt_ext_controller_info_changed_evt, 2, false },
	{ 0x0026, "PHY Configuration Changed",
			mgmt_phy_changed_evt, 4, true },
	{ 0x0027, "Experimental Feature Changed",
			mgmt_exp_feature_changed_evt, 20, true },
	{ 0x002a, "Device Flags Changed",
			mgmt_device_flags_changed_evt, 15, true },
	{ 0x002b, "Advertisement Monitor Added",
			mgmt_adv_monitor_added_evt, 2, true },
	{ 0x002c, "Advertisement Monitor Added",
			mgmt_adv_monitor_removed_evt, 2, true },
	{ 0x002d, "Controller Suspended",
			mgmt_controller_suspend_evt, 1, true },
	{ 0x002e, "Controller Resumed",
			mgmt_controller_resume_evt, 8, true },
	{ }
};

static void mgmt_print_commands(const void *data, uint16_t num)
{
	int i;

	print_field("Commands: %u", num);

	for (i = 0; i < num; i++) {
		uint16_t opcode = get_le16(data + (i * 2));
		const char *str = NULL;
		int n;

		for (n = 0; mgmt_command_table[n].str; n++) {
			if (mgmt_command_table[n].opcode == opcode) {
				str = mgmt_command_table[n].str;
				break;
			}
		}

		print_field("  %s (0x%4.4x)", str ?: "Reserved", opcode);
	}
}

static void mgmt_print_events(const void *data, uint16_t num)
{
	int i;

	print_field("Events: %u", num);

	for (i = 0; i < num; i++) {
		uint16_t opcode = get_le16(data + (i * 2));
		const char *str = NULL;
		int n;

		for (n = 0; mgmt_event_table[n].str; n++) {
			if (mgmt_event_table[n].opcode == opcode) {
				str = mgmt_event_table[n].str;
				break;
			}
		}

		print_field("  %s (0x%4.4x)", str ?: "Reserved", opcode);
	}
}

void packet_ctrl_command(struct timeval *tv, struct ucred *cred, uint16_t index,
					const void *data, uint16_t size)
{
	uint32_t cookie;
	uint16_t format, opcode;
	const struct mgmt_data *mgmt_data = NULL;
	const char *mgmt_color, *mgmt_str;
	char channel[11], extra_str[25];
	int i;

	if (size < 4) {
		print_packet(tv, cred, '*', index, NULL, COLOR_ERROR,
				"Malformed Control Command packet", NULL, NULL);
		packet_hexdump(data, size);
		return;
	}

	cookie = get_le32(data);

	data += 4;
	size -= 4;

	sprintf(channel, "0x%4.4x", cookie);

	format = get_format(cookie);

	if (format != CTRL_MGMT) {
		char label[7];

		sprintf(label, "0x%4.4x", format);

		print_packet(tv, cred, '@', index, channel, COLOR_CTRL_CLOSE,
						"Control Command", label, NULL);
		packet_hexdump(data, size);
		return;
	}

	if (size < 2) {
		print_packet(tv, cred, '*', index, NULL, COLOR_ERROR,
				"Malformed MGMT Command packet", NULL, NULL);
		packet_hexdump(data, size);
		return;
	}

	opcode = get_le16(data);

	data += 2;
	size -= 2;

	for (i = 0; mgmt_command_table[i].str; i++) {
		if (mgmt_command_table[i].opcode == opcode) {
			mgmt_data = &mgmt_command_table[i];
			break;
		}
	}

	if (mgmt_data) {
		if (mgmt_data->func)
			mgmt_color = COLOR_CTRL_COMMAND;
		else
			mgmt_color = COLOR_CTRL_COMMAND_UNKNOWN;
		mgmt_str = mgmt_data->str;
	} else {
		mgmt_color = COLOR_CTRL_COMMAND_UNKNOWN;
		mgmt_str = "Unknown";
	}

	sprintf(extra_str, "(0x%4.4x) plen %d", opcode, size);

	print_packet(tv, cred, '@', index, channel, mgmt_color,
					"MGMT Command", mgmt_str, extra_str);

	if (!mgmt_data || !mgmt_data->func) {
		packet_hexdump(data, size);
		return;
	}

	if (mgmt_data->fixed) {
		if (size != mgmt_data->size) {
			print_text(COLOR_ERROR, "invalid packet size");
			packet_hexdump(data, size);
			return;
		}
	} else {
		if (size < mgmt_data->size) {
			print_text(COLOR_ERROR, "too short packet");
			packet_hexdump(data, size);
			return;
		}
	}

	mgmt_data->func(data, size);
}

void packet_ctrl_event(struct timeval *tv, struct ucred *cred, uint16_t index,
					const void *data, uint16_t size)
{
	uint32_t cookie;
	uint16_t format, opcode;
	const struct mgmt_data *mgmt_data = NULL;
	const char *mgmt_color, *mgmt_str;
	char channel[11], extra_str[25];
	int i;

	if (size < 4) {
		print_packet(tv, cred, '*', index, NULL, COLOR_ERROR,
				"Malformed Control Event packet", NULL, NULL);
		packet_hexdump(data, size);
		return;
	}

	cookie = get_le32(data);

	data += 4;
	size -= 4;

	sprintf(channel, "0x%4.4x", cookie);

	format = get_format(cookie);

	if (format != CTRL_MGMT) {
		char label[7];

		sprintf(label, "0x%4.4x", format);

		print_packet(tv, cred, '@', index, channel, COLOR_CTRL_CLOSE,
						"Control Event", label, NULL);
		packet_hexdump(data, size);
		return;
	}

	if (size < 2) {
		print_packet(tv, cred, '*', index, NULL, COLOR_ERROR,
				"Malformed MGMT Event packet", NULL, NULL);
		packet_hexdump(data, size);
		return;
	}

	opcode = get_le16(data);

	data += 2;
	size -= 2;

	for (i = 0; mgmt_event_table[i].str; i++) {
		if (mgmt_event_table[i].opcode == opcode) {
			mgmt_data = &mgmt_event_table[i];
			break;
		}
	}

	if (mgmt_data) {
		if (mgmt_data->func)
			mgmt_color = COLOR_CTRL_EVENT;
		else
			mgmt_color = COLOR_CTRL_EVENT_UNKNOWN;
		mgmt_str = mgmt_data->str;
	} else {
		mgmt_color = COLOR_CTRL_EVENT_UNKNOWN;
		mgmt_str = "Unknown";
	}

	sprintf(extra_str, "(0x%4.4x) plen %d", opcode, size);

	print_packet(tv, cred, '@', index, channel, mgmt_color,
					"MGMT Event", mgmt_str, extra_str);

	if (!mgmt_data || !mgmt_data->func) {
		packet_hexdump(data, size);
		return;
	}

	if (mgmt_data->fixed) {
		if (size != mgmt_data->size) {
			print_text(COLOR_ERROR, "invalid packet size");
			packet_hexdump(data, size);
			return;
		}
	} else {
		if (size < mgmt_data->size) {
			print_text(COLOR_ERROR, "too short packet");
			packet_hexdump(data, size);
			return;
		}
	}

	mgmt_data->func(data, size);
}

void packet_todo(void)
{
	int i;

	printf("HCI commands with missing decodings:\n");

	for (i = 0; opcode_table[i].str; i++) {
		if (opcode_table[i].bit < 0)
			continue;

		if (opcode_table[i].cmd_func)
			continue;

		printf("\t%s\n", opcode_table[i].str);
	}

	printf("HCI events with missing decodings:\n");

	for (i = 0; event_table[i].str; i++) {
		if (event_table[i].func)
			continue;

		printf("\t%s\n", event_table[i].str);
	}

	for (i = 0; le_meta_event_table[i].str; i++) {
		if (le_meta_event_table[i].func)
			continue;

		printf("\t%s\n", le_meta_event_table[i].str);
	}
}

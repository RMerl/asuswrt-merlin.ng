/*
 *
 *  BlueZ - Bluetooth protocol stack for Linux
 *
 *  Copyright (C) 2011  André Dieb Martins <andre.dieb@gmail.com>
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
#include <unistd.h>
#include <stdlib.h>
#include <string.h>

#include "parser.h"

#define GATT_PRIM_SVC_UUID		0x2800
#define GATT_SND_SVC_UUID		0x2801
#define GATT_INCLUDE_UUID		0x2802
#define GATT_CHARAC_UUID		0x2803

#define GATT_CHARAC_DEVICE_NAME			0x2A00
#define GATT_CHARAC_APPEARANCE			0x2A01
#define GATT_CHARAC_PERIPHERAL_PRIV_FLAG	0x2A02
#define GATT_CHARAC_RECONNECTION_ADDRESS	0x2A03
#define GATT_CHARAC_PERIPHERAL_PREF_CONN	0x2A04
#define GATT_CHARAC_SERVICE_CHANGED		0x2A05

#define GATT_CHARAC_EXT_PROPER_UUID	0x2900
#define GATT_CHARAC_USER_DESC_UUID	0x2901
#define GATT_CLIENT_CHARAC_CFG_UUID	0x2902
#define GATT_SERVER_CHARAC_CFG_UUID	0x2903
#define GATT_CHARAC_FMT_UUID		0x2904
#define GATT_CHARAC_AGREG_FMT_UUID	0x2905



/* Attribute Protocol Opcodes */
#define ATT_OP_ERROR			0x01
#define ATT_OP_MTU_REQ			0x02
#define ATT_OP_MTU_RESP			0x03
#define ATT_OP_FIND_INFO_REQ		0x04
#define ATT_OP_FIND_INFO_RESP		0x05
#define ATT_OP_FIND_BY_TYPE_REQ		0x06
#define ATT_OP_FIND_BY_TYPE_RESP	0x07
#define ATT_OP_READ_BY_TYPE_REQ		0x08
#define ATT_OP_READ_BY_TYPE_RESP	0x09
#define ATT_OP_READ_REQ			0x0A
#define ATT_OP_READ_RESP		0x0B
#define ATT_OP_READ_BLOB_REQ		0x0C
#define ATT_OP_READ_BLOB_RESP		0x0D
#define ATT_OP_READ_MULTI_REQ		0x0E
#define ATT_OP_READ_MULTI_RESP		0x0F
#define ATT_OP_READ_BY_GROUP_REQ	0x10
#define ATT_OP_READ_BY_GROUP_RESP	0x11
#define ATT_OP_WRITE_REQ		0x12
#define ATT_OP_WRITE_RESP		0x13
#define ATT_OP_WRITE_CMD		0x52
#define ATT_OP_PREP_WRITE_REQ		0x16
#define ATT_OP_PREP_WRITE_RESP		0x17
#define ATT_OP_EXEC_WRITE_REQ		0x18
#define ATT_OP_EXEC_WRITE_RESP		0x19
#define ATT_OP_HANDLE_NOTIFY		0x1B
#define ATT_OP_HANDLE_IND		0x1D
#define ATT_OP_HANDLE_CNF		0x1E
#define ATT_OP_SIGNED_WRITE_CMD		0xD2

/* Error codes for Error response PDU */
#define ATT_ECODE_INVALID_HANDLE		0x01
#define ATT_ECODE_READ_NOT_PERM			0x02
#define ATT_ECODE_WRITE_NOT_PERM		0x03
#define ATT_ECODE_INVALID_PDU			0x04
#define ATT_ECODE_INSUFF_AUTHEN			0x05
#define ATT_ECODE_REQ_NOT_SUPP			0x06
#define ATT_ECODE_INVALID_OFFSET		0x07
#define ATT_ECODE_INSUFF_AUTHO			0x08
#define ATT_ECODE_PREP_QUEUE_FULL		0x09
#define ATT_ECODE_ATTR_NOT_FOUND		0x0A
#define ATT_ECODE_ATTR_NOT_LONG			0x0B
#define ATT_ECODE_INSUFF_ENCR_KEY_SIZE		0x0C
#define ATT_ECODE_INVAL_ATTR_VALUE_LEN		0x0D
#define ATT_ECODE_UNLIKELY			0x0E
#define ATT_ECODE_INSUFF_ENC			0x0F
#define ATT_ECODE_UNSUPP_GRP_TYPE		0x10
#define ATT_ECODE_INSUFF_RESOURCES		0x11
#define ATT_ECODE_IO				0xFF


/* Attribute Protocol Opcodes */
static const char *attop2str(uint8_t op)
{
	switch (op) {
	case ATT_OP_ERROR:
		return "Error";
	case ATT_OP_MTU_REQ:
		return "MTU req";
	case ATT_OP_MTU_RESP:
		return "MTU resp";
	case ATT_OP_FIND_INFO_REQ:
		return "Find Information req";
	case ATT_OP_FIND_INFO_RESP:
		return "Find Information resp";
	case ATT_OP_FIND_BY_TYPE_REQ:
		return "Find By Type req";
	case ATT_OP_FIND_BY_TYPE_RESP:
		return "Find By Type resp";
	case ATT_OP_READ_BY_TYPE_REQ:
		return "Read By Type req";
	case ATT_OP_READ_BY_TYPE_RESP:
		return "Read By Type resp";
	case ATT_OP_READ_REQ:
		return "Read req";
	case ATT_OP_READ_RESP:
		return "Read resp";
	case ATT_OP_READ_BLOB_REQ:
		return "Read Blob req";
	case ATT_OP_READ_BLOB_RESP:
		return "Read Blob resp";
	case ATT_OP_READ_MULTI_REQ:
		return "Read Multi req";
	case ATT_OP_READ_MULTI_RESP:
		return "Read Multi resp";
	case ATT_OP_READ_BY_GROUP_REQ:
		return "Read By Group req";
	case ATT_OP_READ_BY_GROUP_RESP:
		return "Read By Group resp";
	case ATT_OP_WRITE_REQ:
		return "Write req";
	case ATT_OP_WRITE_RESP:
		return "Write resp";
	case ATT_OP_WRITE_CMD:
		return "Write cmd";
	case ATT_OP_PREP_WRITE_REQ:
		return "Prepare Write req";
	case ATT_OP_PREP_WRITE_RESP:
		return "Prepare Write resp";
	case ATT_OP_EXEC_WRITE_REQ:
		return "Exec Write req";
	case ATT_OP_EXEC_WRITE_RESP:
		return "Exec Write resp";
	case ATT_OP_HANDLE_NOTIFY:
		return "Handle notify";
	case ATT_OP_HANDLE_IND:
		return "Handle indicate";
	case ATT_OP_HANDLE_CNF:
		return "Handle CNF";
	case ATT_OP_SIGNED_WRITE_CMD:
		return "Signed Write Cmd";
	default:
		return "Unknown";
	}
}

static const char * atterror2str(uint8_t err)
{
	switch (err) {
	case ATT_ECODE_INVALID_HANDLE:
		return "Invalid handle";
	case ATT_ECODE_READ_NOT_PERM:
		return "Read not permitted";
	case ATT_ECODE_WRITE_NOT_PERM:
		return "Write not permitted";
	case ATT_ECODE_INVALID_PDU:
		return "Invalid PDU";
	case ATT_ECODE_INSUFF_AUTHEN:
		return "Insufficient authentication";
	case ATT_ECODE_REQ_NOT_SUPP:
		return "Request not supported";
	case ATT_ECODE_INVALID_OFFSET:
		return "Invalid offset";
	case ATT_ECODE_INSUFF_AUTHO:
		return "Insufficient authorization";
	case ATT_ECODE_PREP_QUEUE_FULL:
		return "Prepare queue full";
	case ATT_ECODE_ATTR_NOT_FOUND:
		return "Attribute not found";
	case ATT_ECODE_ATTR_NOT_LONG:
		return "Attribute not long";
	case ATT_ECODE_INSUFF_ENCR_KEY_SIZE:
		return "Insufficient encryption key size";
	case ATT_ECODE_INVAL_ATTR_VALUE_LEN:
		return "Invalid attribute value length";
	case ATT_ECODE_UNLIKELY:
		return "Unlikely error";
	case ATT_ECODE_INSUFF_ENC:
		return "Insufficient encryption";
	case ATT_ECODE_UNSUPP_GRP_TYPE:
		return "Unsupported group type";
	case ATT_ECODE_INSUFF_RESOURCES:
		return "Insufficient resources";
	case ATT_ECODE_IO:
		return "Application Error";
	default:
		return "Reserved";
	}
}

static const char *uuid2str(uint16_t uuid)
{
	switch (uuid) {
	case GATT_PRIM_SVC_UUID:
		return "GATT Primary Service";
	case GATT_SND_SVC_UUID:
		return "GATT Secondary Service";
	case GATT_INCLUDE_UUID:
		return "GATT Include";
	case GATT_CHARAC_UUID:
		return "GATT Characteristic";
	case GATT_CHARAC_DEVICE_NAME:
		return "GATT(type) Device Name";
	case GATT_CHARAC_APPEARANCE:
		return "GATT(type) Appearance";
	case GATT_CHARAC_PERIPHERAL_PRIV_FLAG:
		return "GATT(type) Peripheral Privacy Flag";
	case GATT_CHARAC_RECONNECTION_ADDRESS:
		return "GATT(type) Characteristic Reconnection Address";
	case GATT_CHARAC_PERIPHERAL_PREF_CONN:
		return "GATT(type) Characteristic Preferred Connection Parameters";
	case GATT_CHARAC_SERVICE_CHANGED:
		return "GATT(type) Characteristic Service Changed";
	case GATT_CHARAC_EXT_PROPER_UUID:
		return "GATT(desc) Characteristic Extended Properties";
	case GATT_CHARAC_USER_DESC_UUID:
		return "GATT(desc) User Description";
	case GATT_CLIENT_CHARAC_CFG_UUID:
		return "GATT(desc) Client Characteristic Configuration";
	case GATT_SERVER_CHARAC_CFG_UUID:
		return "GATT(desc) Server Characteristic Configuration";
	case GATT_CHARAC_FMT_UUID:
		return "GATT(desc) Format";
	case GATT_CHARAC_AGREG_FMT_UUID:
		return "GATT(desc) Aggregate Format";
	default:
		return "Unknown";
	}
}

static void att_error_dump(int level, struct frame *frm)
{
	uint8_t op = p_get_u8(frm);
	uint16_t handle = btohs(htons(p_get_u16(frm)));
	uint8_t err = p_get_u8(frm);

	p_indent(level, frm);
	printf("Error: %s (%d)\n", atterror2str(err), err);

	p_indent(level, frm);
	printf("%s (0x%.2x) on handle 0x%4.4x\n", attop2str(op), op, handle);
}

static void att_mtu_req_dump(int level, struct frame *frm)
{
	uint16_t client_rx_mtu = btohs(htons(p_get_u16(frm)));

	p_indent(level, frm);
	printf("client rx mtu %d\n", client_rx_mtu);
}

static void att_mtu_resp_dump(int level, struct frame *frm)
{
	uint16_t server_rx_mtu = btohs(htons(p_get_u16(frm)));

	p_indent(level, frm);
	printf("server rx mtu %d\n", server_rx_mtu);
}

static void att_find_info_req_dump(int level, struct frame *frm)
{
	uint16_t start = btohs(htons(p_get_u16(frm)));
	uint16_t end = btohs(htons(p_get_u16(frm)));

	p_indent(level, frm);
	printf("start 0x%4.4x, end 0x%4.4x\n", start, end);
}

static void print_uuid128(struct frame *frm)
{
	uint8_t uuid[16];
	int i;

	for (i = 0; i < 16; i++)
		uuid[15 - i] = p_get_u8(frm);

	for (i = 0; i < 16; i++) {
		printf("%02x", uuid[i]);
		if (i == 3 || i == 5 || i == 7 || i == 9)
			printf("-");
	}
}

static void att_find_info_resp_dump(int level, struct frame *frm)
{
	uint8_t fmt = p_get_u8(frm);

	p_indent(level, frm);

	if (fmt == 0x01) {
		printf("format: uuid-16\n");

		while (frm->len > 0) {
			uint16_t handle = btohs(htons(p_get_u16(frm)));
			uint16_t uuid = btohs(htons(p_get_u16(frm)));
			p_indent(level + 1, frm);
			printf("handle 0x%4.4x, uuid 0x%4.4x (%s)\n", handle, uuid,
					uuid2str(uuid));
		}
	} else {
		printf("format: uuid-128\n");

		while (frm->len > 0) {
			uint16_t handle = btohs(htons(p_get_u16(frm)));

			p_indent(level + 1, frm);
			printf("handle 0x%4.4x, uuid ", handle);
			print_uuid128(frm);
			printf("\n");
		}
	}
}

static void att_find_by_type_req_dump(int level, struct frame *frm)
{
	uint16_t start = btohs(htons(p_get_u16(frm)));
	uint16_t end = btohs(htons(p_get_u16(frm)));
	uint16_t uuid = btohs(htons(p_get_u16(frm)));

	p_indent(level, frm);
	printf("start 0x%4.4x, end 0x%4.4x, uuid 0x%4.4x\n", start, end, uuid);

	p_indent(level, frm);
	printf("value");
	while (frm->len > 0)
		printf(" 0x%2.2x", p_get_u8(frm));
	printf("\n");
}

static void att_find_by_type_resp_dump(int level, struct frame *frm)
{
	while (frm->len > 0) {
		uint16_t uuid = btohs(htons(p_get_u16(frm)));
		uint16_t end = btohs(htons(p_get_u16(frm)));

		p_indent(level, frm);
		printf("Found attr 0x%4.4x, group end handle 0x%4.4x\n",
								uuid, end);
	}
}

static void att_read_by_type_req_dump(int level, struct frame *frm)
{
	uint16_t start = btohs(htons(p_get_u16(frm)));
	uint16_t end = btohs(htons(p_get_u16(frm)));

	p_indent(level, frm);
	printf("start 0x%4.4x, end 0x%4.4x\n", start, end);

	p_indent(level, frm);
	if (frm->len == 2) {
		printf("type-uuid 0x%4.4x\n", btohs(htons(p_get_u16(frm))));
	} else if (frm->len == 16) {
		printf("type-uuid ");
		print_uuid128(frm);
		printf("\n");
	} else {
		printf("malformed uuid (expected 2 or 16 octets)\n");
		p_indent(level, frm);
		raw_dump(level, frm);
	}
}

static void att_read_by_type_resp_dump(int level, struct frame *frm)
{
	uint8_t length = p_get_u8(frm);

	p_indent(level, frm);
	printf("length: %d\n", length);

	while (frm->len > 0) {
		uint16_t handle = btohs(htons(p_get_u16(frm)));
		int val_len = length - 2;
		int i;

		p_indent(level + 1, frm);
		printf("handle 0x%4.4x, value ", handle);
		for (i = 0; i < val_len; i++) {
			printf("0x%.2x ", p_get_u8(frm));
		}
		printf("\n");
	}
}

static void att_read_req_dump(int level, struct frame *frm)
{
	uint16_t handle = btohs(htons(p_get_u16(frm)));

	p_indent(level, frm);
	printf("handle 0x%4.4x\n", handle);
}

static void att_read_blob_req_dump(int level, struct frame *frm)
{
	uint16_t handle = btohs(htons(p_get_u16(frm)));
	uint16_t offset = btohs(htons(p_get_u16(frm)));

	p_indent(level, frm);
	printf("handle 0x%4.4x offset 0x%4.4x\n", handle, offset);
}

static void att_read_blob_resp_dump(int level, struct frame *frm)
{
	p_indent(level, frm);
	printf("value");

	while (frm->len > 0)
		printf(" 0x%2.2x", p_get_u8(frm));
	printf("\n");
}

static void att_read_multi_req_dump(int level, struct frame *frm)
{
	p_indent(level, frm);
	printf("Handles\n");

	while (frm->len > 0) {
		p_indent(level, frm);
		printf("handle 0x%4.4x\n", btohs(htons(p_get_u16(frm))));
	}
}

static void att_read_multi_resp_dump(int level, struct frame *frm)
{
	p_indent(level, frm);
	printf("values");

	while (frm->len > 0)
		printf(" 0x%2.2x", p_get_u8(frm));
	printf("\n");
}

static void att_read_by_group_resp_dump(int level, struct frame *frm)
{
	uint8_t length = p_get_u8(frm);

	while (frm->len > 0) {
		uint16_t attr_handle = btohs(htons(p_get_u16(frm)));
		uint16_t end_grp_handle = btohs(htons(p_get_u16(frm)));
		uint8_t remaining = length - 4;

		p_indent(level, frm);
		printf("attr handle 0x%4.4x, end group handle 0x%4.4x\n",
						attr_handle, end_grp_handle);

		p_indent(level, frm);
		printf("value");
		while (remaining > 0) {
			printf(" 0x%2.2x", p_get_u8(frm));
			remaining--;
		}
		printf("\n");
	}
}

static void att_write_req_dump(int level, struct frame *frm)
{
	uint16_t handle = btohs(htons(p_get_u16(frm)));

	p_indent(level, frm);
	printf("handle 0x%4.4x value ", handle);

	while (frm->len > 0)
		printf(" 0x%2.2x", p_get_u8(frm));
	printf("\n");
}

static void att_signed_write_dump(int level, struct frame *frm)
{
	uint16_t handle = btohs(htons(p_get_u16(frm)));
	int value_len = frm->len - 12; /* handle:2 already accounted, sig: 12 */

	p_indent(level, frm);
	printf("handle 0x%4.4x value ", handle);

	while (value_len--)
		printf(" 0x%2.2x", p_get_u8(frm));
	printf("\n");

	p_indent(level, frm);
	printf("auth signature ");
	while (frm->len > 0)
		printf(" 0x%2.2x", p_get_u8(frm));
	printf("\n");
}

static void att_prep_write_dump(int level, struct frame *frm)
{
	uint16_t handle = btohs(htons(p_get_u16(frm)));
	uint16_t val_offset = btohs(htons(p_get_u16(frm)));

	p_indent(level, frm);
	printf("attr handle 0x%4.4x, value offset 0x%4.4x\n", handle,
								val_offset);

	p_indent(level, frm);
	printf("part attr value ");
	while (frm->len > 0)
		printf(" 0x%2.2x", p_get_u8(frm));
	printf("\n");
}

static void att_exec_write_req_dump(int level, struct frame *frm)
{
	uint8_t flags = p_get_u8(frm);

	p_indent(level, frm);
	if (flags == 0x00)
		printf("cancel all prepared writes ");
	else
		printf("immediatelly write all pending prepared values ");

	printf("(0x%2.2x)\n", flags);
}

static void att_handle_notify_dump(int level, struct frame *frm)
{
	uint16_t handle = btohs(htons(p_get_u16(frm)));

	p_indent(level, frm);
	printf("handle 0x%4.4x\n", handle);

	p_indent(level, frm);
	printf("value ");
	while (frm->len > 0)
		printf("0x%.2x ", p_get_u8(frm));
	printf("\n");
}

void att_dump(int level, struct frame *frm)
{
	uint8_t op;

	op = p_get_u8(frm);

	p_indent(level, frm);
	printf("ATT: %s (0x%.2x)\n", attop2str(op), op);

	switch (op) {
		case ATT_OP_ERROR:
			att_error_dump(level + 1, frm);
			break;
		case ATT_OP_MTU_REQ:
			att_mtu_req_dump(level + 1, frm);
			break;
		case ATT_OP_MTU_RESP:
			att_mtu_resp_dump(level + 1, frm);
			break;
		case ATT_OP_FIND_INFO_REQ:
			att_find_info_req_dump(level + 1, frm);
			break;
		case ATT_OP_FIND_INFO_RESP:
			att_find_info_resp_dump(level + 1, frm);
			break;
		case ATT_OP_FIND_BY_TYPE_REQ:
			att_find_by_type_req_dump(level + 1, frm);
			break;
		case ATT_OP_FIND_BY_TYPE_RESP:
			att_find_by_type_resp_dump(level + 1, frm);
			break;
		case ATT_OP_READ_BY_TYPE_REQ:
		case ATT_OP_READ_BY_GROUP_REQ: /* exact same parsing */
			att_read_by_type_req_dump(level + 1, frm);
			break;
		case ATT_OP_READ_BY_TYPE_RESP:
			att_read_by_type_resp_dump(level + 1, frm);
			break;
		case ATT_OP_READ_REQ:
			att_read_req_dump(level + 1, frm);
			break;
		case ATT_OP_READ_RESP:
			raw_dump(level + 1, frm);
			break;
		case ATT_OP_READ_BLOB_REQ:
			att_read_blob_req_dump(level + 1, frm);
			break;
		case ATT_OP_READ_BLOB_RESP:
			att_read_blob_resp_dump(level + 1, frm);
			break;
		case ATT_OP_READ_MULTI_REQ:
			att_read_multi_req_dump(level + 1, frm);
			break;
		case ATT_OP_READ_MULTI_RESP:
			att_read_multi_resp_dump(level + 1, frm);
			break;
		case ATT_OP_READ_BY_GROUP_RESP:
			att_read_by_group_resp_dump(level + 1, frm);
			break;
		case ATT_OP_WRITE_REQ:
		case ATT_OP_WRITE_CMD:
			att_write_req_dump(level + 1, frm);
			break;
		case ATT_OP_SIGNED_WRITE_CMD:
			att_signed_write_dump(level + 1, frm);
			break;
		case ATT_OP_PREP_WRITE_REQ:
		case ATT_OP_PREP_WRITE_RESP:
			att_prep_write_dump(level + 1, frm);
			break;
		case ATT_OP_EXEC_WRITE_REQ:
			att_exec_write_req_dump(level + 1, frm);
			break;
		case ATT_OP_HANDLE_NOTIFY:
			att_handle_notify_dump(level + 1, frm);
			break;
		default:
			raw_dump(level, frm);
			break;
	}
}

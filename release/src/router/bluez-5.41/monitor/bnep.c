/*
 *
 *  BlueZ - Bluetooth protocol stack for Linux
 *
 *  Copyright (C) 2011-2014  Intel Corporation
 *  Copyright (C) 2002-2010  Marcel Holtmann <marcel@holtmann.org>
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
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <inttypes.h>

#include "lib/bluetooth.h"

#include "src/shared/util.h"
#include "bt.h"
#include "packet.h"
#include "display.h"
#include "l2cap.h"
#include "uuid.h"
#include "keys.h"
#include "sdp.h"
#include "bnep.h"

#define GET_PKT_TYPE(type) (type & 0x7f)
#define GET_EXTENSION(type) (type & 0x80)

/* BNEP Extension Type */
#define BNEP_EXTENSION_CONTROL		0x00

#define BNEP_CONTROL			0x01

uint16_t proto = 0x0000;

struct bnep_frame {
	uint8_t type;
	int extension;
	struct l2cap_frame l2cap_frame;
};

static bool get_macaddr(struct bnep_frame *bnep_frame, char *str)
{
	uint8_t addr[6];
	struct l2cap_frame *frame = &bnep_frame->l2cap_frame;
	int i;

	for (i = 0; i < 6; i++)
		if (!l2cap_frame_get_u8(frame, &addr[i]))
			return false;

	sprintf(str, "%02x:%02x:%02x:%02x:%02x:%02x",
		addr[0], addr[1], addr[2], addr[3], addr[4], addr[5]);

	return true;
}

static bool bnep_general(struct bnep_frame *bnep_frame,
					uint8_t indent,	int hdr_len)
{
	struct l2cap_frame *frame;
	char src_addr[20], dest_addr[20];

	if (!get_macaddr(bnep_frame, dest_addr))
		return false;

	if (!get_macaddr(bnep_frame, src_addr))
		return false;

	frame = &bnep_frame->l2cap_frame;

	if (!l2cap_frame_get_be16(frame, &proto))
		return false;

	print_field("%*cdst %s src %s [proto 0x%04x] ", indent,
					' ', dest_addr, src_addr, proto);

	return true;

}

static bool cmd_nt_understood(struct bnep_frame *bnep_frame, uint8_t indent)
{
	struct l2cap_frame *frame = &bnep_frame->l2cap_frame;
	uint8_t ptype;

	if (!l2cap_frame_get_u8(frame, &ptype))
		return false;

	print_field("%*cType: 0x%02x ", indent, ' ', ptype);

	return true;
}

static bool setup_conn_req(struct bnep_frame *bnep_frame, uint8_t indent)
{

	struct l2cap_frame *frame = &bnep_frame->l2cap_frame;
	uint8_t uuid_size;
	uint32_t src_uuid = 0, dst_uuid = 0;

	if (!l2cap_frame_get_u8(frame, &uuid_size))
		return false;

	print_field("%*cSize: 0x%02x ", indent, ' ', uuid_size);

	switch (uuid_size) {
	case 2:
		if (!l2cap_frame_get_be16(frame, (uint16_t *) &dst_uuid))
			return false;

		if (!l2cap_frame_get_be16(frame, (uint16_t *) &src_uuid))
			return false;
		break;
	case 4:
		if (!l2cap_frame_get_be32(frame, &dst_uuid))
			return false;

		if (!l2cap_frame_get_be32(frame, &src_uuid))
			return false;
		break;
	case 16:
		if (!l2cap_frame_get_be32(frame, &dst_uuid))
			return false;

		l2cap_frame_pull(frame, frame, 12);

		if (!l2cap_frame_get_be32(frame, &src_uuid))
			return false;

		l2cap_frame_pull(frame, frame, 12);
		break;
	default:
		l2cap_frame_pull(frame, frame, (uuid_size * 2));
		return true;
	}

	print_field("%*cDst: 0x%x(%s)", indent, ' ', dst_uuid,
						uuid32_to_str(dst_uuid));
	print_field("%*cSrc: 0x%x(%s)", indent, ' ', src_uuid,
						uuid32_to_str(src_uuid));
	return true;
}

static const char *value2str(uint16_t value)
{
	switch (value) {
	case 0x00:
		return "Operation Successful";
	case 0x01:
		return "Operation Failed - Invalid Dst Srv UUID";
	case 0x02:
		return "Operation Failed - Invalid Src Srv UUID";
	case 0x03:
		return "Operation Failed - Invalid Srv UUID size";
	case 0x04:
		return "Operation Failed - Conn not allowed";
	default:
		return "Unknown";
	}
}

static bool print_rsp_msg(struct bnep_frame *bnep_frame, uint8_t indent)
{
	struct l2cap_frame *frame = &bnep_frame->l2cap_frame;
	uint16_t rsp_msg;

	if (!l2cap_frame_get_be16(frame, &rsp_msg))
		return false;

	print_field("%*cRsp msg: %s(0x%04x) ", indent, ' ',
					value2str(rsp_msg), rsp_msg);

	return true;
}

static bool filter_nettype_req(struct bnep_frame *bnep_frame, uint8_t indent)
{
	struct l2cap_frame *frame = &bnep_frame->l2cap_frame;
	uint16_t length, start_range, end_range;
	int i;

	if (!l2cap_frame_get_be16(frame, &length))
		return false;

	print_field("%*cLength: 0x%04x", indent, ' ', length);

	for (i = 0; i < length / 4; i++) {

		if (!l2cap_frame_get_be16(frame, &start_range))
			return false;

		if (!l2cap_frame_get_be16(frame, &end_range))
			return false;

		print_field("%*c0x%04x - 0x%04x", indent, ' ',
						start_range, end_range);
	}

	return true;
}

static bool filter_multaddr_req(struct bnep_frame *bnep_frame, uint8_t indent)
{
	struct l2cap_frame *frame = &bnep_frame->l2cap_frame;
	uint16_t length;
	char start_addr[20], end_addr[20];
	int i;

	if (!l2cap_frame_get_be16(frame, &length))
		return false;

	print_field("%*cLength: 0x%04x", indent, ' ', length);

	for (i = 0; i < length / 12; i++) {

		if (!get_macaddr(bnep_frame, start_addr))
			return false;

		if (!get_macaddr(bnep_frame, end_addr))
			return false;

		print_field("%*c%s - %s", indent, ' ', start_addr, end_addr);
	}

	return true;
}

struct bnep_control_data {
	uint8_t type;
	const char *str;
	bool (*func) (struct bnep_frame *frame, uint8_t indent);
};

static const struct bnep_control_data bnep_control_table[] = {
	{ 0x00, "Command Not Understood",	cmd_nt_understood	},
	{ 0x01, "Setup Conn Req",		setup_conn_req		},
	{ 0x02, "Setup Conn Rsp",		print_rsp_msg		},
	{ 0x03, "Filter NetType Set",		filter_nettype_req	},
	{ 0x04, "Filter NetType Rsp",		print_rsp_msg		},
	{ 0x05, "Filter MultAddr Set",		filter_multaddr_req	},
	{ 0x06, "Filter MultAddr Rsp",		print_rsp_msg		},
	{ }
};

static bool bnep_control(struct bnep_frame *bnep_frame,
					uint8_t indent,	int hdr_len)
{
	uint8_t ctype;
	struct l2cap_frame *frame = &bnep_frame->l2cap_frame;
	const struct bnep_control_data *bnep_control_data = NULL;
	const char *type_str;
	int i;

	if (!l2cap_frame_get_u8(frame, &ctype))
		return false;

	for (i = 0; bnep_control_table[i].str; i++) {
		if (bnep_control_table[i].type == ctype) {
			bnep_control_data = &bnep_control_table[i];
			break;
		}
	}

	if (bnep_control_data)
		type_str = bnep_control_data->str;
	else
		type_str = "Unknown control type";

	print_field("%*c%s (0x%02x) ", indent, ' ', type_str, ctype);

	if (!bnep_control_data || !bnep_control_data->func) {
		packet_hexdump(frame->data, hdr_len - 1);
		l2cap_frame_pull(frame, frame, hdr_len - 1);
		goto done;
	}

	if (!bnep_control_data->func(bnep_frame, indent+2))
		return false;

done:
	return true;
}

static bool bnep_compressed(struct bnep_frame *bnep_frame,
					uint8_t indent,	int hdr_len)
{

	struct l2cap_frame *frame = &bnep_frame->l2cap_frame;

	if (!l2cap_frame_get_be16(frame, &proto))
		return false;

	print_field("%*c[proto 0x%04x] ", indent, ' ', proto);

	return true;
}

static bool bnep_src_only(struct bnep_frame *bnep_frame,
					uint8_t indent,	int hdr_len)
{

	struct l2cap_frame *frame;
	char src_addr[20];

	if (!get_macaddr(bnep_frame, src_addr))
		return false;

	frame = &bnep_frame->l2cap_frame;

	if (!l2cap_frame_get_be16(frame, &proto))
		return false;

	print_field("%*csrc %s [proto 0x%04x] ", indent,
					' ', src_addr, proto);

	return true;
}

static bool bnep_dst_only(struct bnep_frame *bnep_frame,
					uint8_t indent,	int hdr_len)
{

	struct l2cap_frame *frame;
	char dest_addr[20];

	if (!get_macaddr(bnep_frame, dest_addr))
		return false;

	frame = &bnep_frame->l2cap_frame;

	if (!l2cap_frame_get_be16(frame, &proto))
		return false;

	print_field("%*cdst %s [proto 0x%04x] ", indent,
					' ', dest_addr, proto);

	return true;
}

static bool bnep_eval_extension(struct bnep_frame *bnep_frame, uint8_t indent)
{
	struct l2cap_frame *frame = &bnep_frame->l2cap_frame;
	uint8_t type, length;
	int extension;

	if (!l2cap_frame_get_u8(frame, &type))
		return false;

	if (!l2cap_frame_get_u8(frame, &length))
		return false;

	extension = GET_EXTENSION(type);
	type = GET_PKT_TYPE(type);

	switch (type) {
	case BNEP_EXTENSION_CONTROL:
		print_field("%*cExt Control(0x%02x|%s) len 0x%02x", indent,
				' ', type, extension ? "1" : "0", length);
		if (!bnep_control(bnep_frame, indent+2, length))
			return false;
		break;

	default:
		print_field("%*cExt Unknown(0x%02x|%s) len 0x%02x", indent,
				' ', type, extension ? "1" : "0", length);
		packet_hexdump(frame->data, length);
		l2cap_frame_pull(frame, frame, length);
	}

	if (extension)
		if (!bnep_eval_extension(bnep_frame, indent))
			return false;

	return true;
}

struct bnep_data {
	uint8_t type;
	const char *str;
	bool (*func) (struct bnep_frame *frame, uint8_t indent, int hdr_len);
};

static const struct bnep_data bnep_table[] = {
	{ 0x00, "General Ethernet",		bnep_general	},
	{ 0x01, "Control",			bnep_control	},
	{ 0x02, "Compressed Ethernet",		bnep_compressed	},
	{ 0x03, "Compressed Ethernet SrcOnly",	bnep_src_only	},
	{ 0x04, "Compressed Ethernet DestOnly",	bnep_dst_only	},
	{ }
};

void bnep_packet(const struct l2cap_frame *frame)
{
	uint8_t type, indent = 1;
	struct bnep_frame bnep_frame;
	struct l2cap_frame *l2cap_frame;
	const struct bnep_data *bnep_data = NULL;
	const char *pdu_color, *pdu_str;
	int i;

	l2cap_frame_pull(&bnep_frame.l2cap_frame, frame, 0);
	l2cap_frame = &bnep_frame.l2cap_frame;

	if (!l2cap_frame_get_u8(l2cap_frame, &type))
		goto fail;

	bnep_frame.extension = GET_EXTENSION(type);
	bnep_frame.type = GET_PKT_TYPE(type);

	for (i = 0; bnep_table[i].str; i++) {
		if (bnep_table[i].type == bnep_frame.type) {
			bnep_data = &bnep_table[i];
			break;
		}
	}

	if (bnep_data) {
		if (bnep_data->func) {
			if (frame->in)
				pdu_color = COLOR_MAGENTA;
			else
				pdu_color = COLOR_BLUE;
		} else
			pdu_color = COLOR_WHITE_BG;
		pdu_str = bnep_data->str;
	} else {
		pdu_color = COLOR_WHITE_BG;
		pdu_str = "Unknown packet type";
	}

	print_indent(6, pdu_color, "BNEP: ", pdu_str, COLOR_OFF,
				" (0x%02x|%s)", bnep_frame.type,
				bnep_frame.extension ? "1" : "0");

	if (!bnep_data || !bnep_data->func) {
		packet_hexdump(l2cap_frame->data, l2cap_frame->size);
		return;
	}

	if (!bnep_data->func(&bnep_frame, indent, -1))
		goto fail;

	/* Extension info */
	if (bnep_frame.extension)
		if (!bnep_eval_extension(&bnep_frame, indent+2))
			goto fail;

	/* Control packet => No payload info */
	if (bnep_frame.type == BNEP_CONTROL)
		return;

	/* TODO: Handle BNEP IP packet */
	packet_hexdump(l2cap_frame->data, l2cap_frame->size);

	return;

fail:
	print_text(COLOR_ERROR, "frame too short");
	packet_hexdump(frame->data, frame->size);
}

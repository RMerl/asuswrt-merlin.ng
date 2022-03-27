/*
 *
 *  BlueZ - Bluetooth protocol stack for Linux
 *
 *  Copyright (C) 2004-2011  Marcel Holtmann <marcel@holtmann.org>
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

static char *opcode2str(uint8_t opcode)
{
	switch (opcode & 0x7f) {
	case 0x00:
		return "Connect";
	case 0x01:
		return "Disconnect";
	case 0x02:
		return "Put";
	case 0x03:
		return "Get";
	case 0x04:
		return "Reserved";
	case 0x05:
		return "SetPath";
	case 0x06:
		return "Action";
	case 0x07:
		return "Session";
	case 0x7f:
		return "Abort";
	case 0x10:
		return "Continue";
	case 0x20:
		return "Success";
	case 0x21:
		return "Created";
	case 0x22:
		return "Accepted";
	case 0x23:
		return "Non-authoritative information";
	case 0x24:
		return "No content";
	case 0x25:
		return "Reset content";
	case 0x26:
		return "Partial content";
	case 0x30:
		return "Multiple choices";
	case 0x31:
		return "Moved permanently";
	case 0x32:
		return "Moved temporarily";
	case 0x33:
		return "See other";
	case 0x34:
		return "Not modified";
	case 0x35:
		return "Use Proxy";
	case 0x40:
		return "Bad request";
	case 0x41:
		return "Unauthorized";
	case 0x42:
		return "Payment required";
	case 0x43:
		return "Forbidden";
	case 0x44:
		return "Not found";
	case 0x45:
		return "Method not allowed";
	case 0x46:
		return "Not acceptable";
	case 0x47:
		return "Proxy authentication required";
	case 0x48:
		return "Request timeout";
	case 0x49:
		return "Conflict";
	case 0x4a:
		return "Gone";
	case 0x4b:
		return "Length required";
	case 0x4c:
		return "Precondition failed";
	case 0x4d:
		return "Requested entity too large";
	case 0x4e:
		return "Requested URL too large";
	case 0x4f:
		return "Unsupported media type";
	case 0x50:
		return "Internal server error";
	case 0x51:
		return "Not implemented";
	case 0x52:
		return "Bad gateway";
	case 0x53:
		return "Service unavailable";
	case 0x54:
		return "Gateway timeout";
	case 0x55:
		return "HTTP version not supported";
	case 0x60:
		return "Database full";
	case 0x61:
		return "Database locked";
	default:
		return "Unknown";
	}
}

static char *hi2str(uint8_t hi)
{
	switch (hi & 0x3f) {
	case 0x00:
		return "Count";
	case 0x01:
		return "Name";
	case 0x02:
		return "Type";
	case 0x03:
		return "Length";
	case 0x04:
		return "Time";
	case 0x05:
		return "Description";
	case 0x06:
		return "Target";
	case 0x07:
		return "HTTP";
	case 0x08:
		return "Body";
	case 0x09:
		return "End of Body";
	case 0x0a:
		return "Who";
	case 0x0b:
		return "Connection ID";
	case 0x0c:
		return "App. Parameters";
	case 0x0d:
		return "Auth. Challenge";
	case 0x0e:
		return "Auth. Response";
	case 0x0f:
		return "Creator ID";
	case 0x10:
		return "WAN UUID";
	case 0x11:
		return "Object Class";
	case 0x12:
		return "Session Parameters";
	case 0x13:
		return "Session Sequence Number";
	case 0x14:
		return "Action ID";
	case 0x15:
		return "DestName";
	case 0x16:
		return "Permission";
	case 0x17:
		return "Single Response Mode";
	case 0x18:
		return "Single Response Mode Parameters";
	default:
		return "Unknown";
	}
}

static void parse_headers(int level, struct frame *frm)
{
	uint8_t hi, hv8;
	uint16_t len;
	uint32_t hv32;

	while (frm->len > 0) {
		hi = p_get_u8(frm);

		p_indent(level, frm);

		printf("%s (0x%02x)", hi2str(hi), hi);
		switch (hi & 0xc0) {
		case 0x00:	/* Unicode */
			if (frm->len < 2) {
				printf("\n");
				return;
			}

			len = p_get_u16(frm) - 3;
			printf(" = Unicode length %d\n", len);

			if (frm->len < len)
				return;

			raw_ndump(level, frm, len);
			frm->ptr += len;
			frm->len -= len;
			break;

		case 0x40:	/* Byte sequence */
			if (frm->len < 2) {
				printf("\n");
				return;
			}

			len = p_get_u16(frm) - 3;
			printf(" = Sequence length %d\n", len);

			if (frm->len < len)
				return;

			raw_ndump(level, frm, len);
			frm->ptr += len;
			frm->len -= len;
			break;

		case 0x80:	/* One byte */
			if (frm->len < 1) {
				printf("\n");
				return;
			}

			hv8 = p_get_u8(frm);
			printf(" = %d\n", hv8);
			break;

		case 0xc0:	/* Four bytes */
			if (frm->len < 4) {
				printf("\n");
				return;
			}

			hv32 = p_get_u32(frm);
			printf(" = %u\n", hv32);
			break;
		}
	}
}

void obex_dump(int level, struct frame *frm)
{
	uint8_t last_opcode, opcode, status;
	uint8_t version, flags, constants;
	uint16_t length, pktlen;

	frm = add_frame(frm);

	while (frm->len > 2) {
		opcode = p_get_u8(frm);
		length = p_get_u16(frm);
		status = opcode & 0x7f;

		if ((int) frm->len < length - 3) {
			frm->ptr -= 3;
			frm->len += 3;
			return;
		}

		p_indent(level, frm);

		last_opcode = get_opcode(frm->handle, frm->dlci);

		if (!(opcode & 0x70)) {
			printf("OBEX: %s cmd(%c): len %d",
					opcode2str(opcode),
					opcode & 0x80 ? 'f' : 'c', length);
			set_opcode(frm->handle, frm->dlci, opcode);
		} else {
			printf("OBEX: %s rsp(%c): status %x%02d len %d",
					opcode2str(last_opcode),
					opcode & 0x80 ? 'f' : 'c',
					status >> 4, status & 0xf, length);
			opcode = last_opcode;
		}

		if (get_status(frm->handle, frm->dlci) == 0x10)
			printf(" (continue)");

		set_status(frm->handle, frm->dlci, status);

		if (frm->len == 0) {
			printf("\n");
			break;
		}

		switch (opcode & 0x7f) {
		case 0x00:	/* Connect */
			if (frm->len < 4) {
				printf("\n");
				return;
			}

			version = p_get_u8(frm);
			flags   = p_get_u8(frm);
			pktlen  = p_get_u16(frm);
			printf(" version %d.%d flags %d mtu %d\n",
				version >> 4, version & 0xf, flags, pktlen);
			break;

		case 0x05:	/* SetPath */
			if (frm->len < 2) {
				printf("\n");
				return;
			}

			flags     = p_get_u8(frm);
			constants = p_get_u8(frm);
			printf(" flags %d constants %d\n", flags, constants);
			break;

		default:
			printf("\n");
			break;
		}

		if ((status & 0x70) && (parser.flags & DUMP_VERBOSE)) {
			p_indent(level, frm);
			printf("Status %x%02d = %s\n",
					status >> 4, status & 0xf,
							opcode2str(status));
		}

		parse_headers(level, frm);
	}
}

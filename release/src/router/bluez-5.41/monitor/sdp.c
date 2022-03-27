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
#include <inttypes.h>

#include "lib/bluetooth.h"

#include "src/shared/util.h"

#include "bt.h"
#include "packet.h"
#include "display.h"
#include "l2cap.h"
#include "uuid.h"
#include "sdp.h"

#define MAX_TID 16

struct tid_data {
	bool inuse;
	uint16_t tid;
	uint16_t channel;
	uint8_t cont[17];
};

static struct tid_data tid_list[MAX_TID];

static struct tid_data *get_tid(uint16_t tid, uint16_t channel)
{
	int i, n = -1;

	for (i = 0; i < MAX_TID; i++) {
		if (!tid_list[i].inuse) {
			if (n < 0)
				n = i;
			continue;
		}

		if (tid_list[i].tid == tid && tid_list[i].channel == channel)
			return &tid_list[i];
	}

	if (n < 0)
		return NULL;

	tid_list[n].inuse = true;
	tid_list[n].tid = tid;
	tid_list[n].channel = channel;

	return &tid_list[n];
}

static void clear_tid(struct tid_data *tid)
{
	if (tid)
		tid->inuse = false;
}

static void print_uint(uint8_t indent, const uint8_t *data, uint32_t size)
{
	switch (size) {
	case 1:
		print_field("%*c0x%2.2x", indent, ' ', data[0]);
		break;
	case 2:
		print_field("%*c0x%4.4x", indent, ' ', get_be16(data));
		break;
	case 4:
		print_field("%*c0x%8.8x", indent, ' ', get_be32(data));
		break;
	case 8:
		print_field("%*c0x%16.16" PRIx64, indent, ' ', get_be64(data));
		break;
	default:
		packet_hexdump(data, size);
		break;
	}
}

static void print_sint(uint8_t indent, const uint8_t *data, uint32_t size)
{
	packet_hexdump(data, size);
}

static void print_uuid(uint8_t indent, const uint8_t *data, uint32_t size)
{
	switch (size) {
	case 2:
		print_field("%*c%s (0x%4.4x)", indent, ' ',
			uuid16_to_str(get_be16(data)), get_be16(data));
		break;
	case 4:
		print_field("%*c%s (0x%8.8x)", indent, ' ',
			uuid32_to_str(get_be32(data)), get_be32(data));
		break;
	case 16:
		/* BASE_UUID = 00000000-0000-1000-8000-00805F9B34FB */
		print_field("%*c%8.8x-%4.4x-%4.4x-%4.4x-%4.4x%8.4x",
				indent, ' ',
				get_be32(data), get_be16(data + 4),
				get_be16(data + 6), get_be16(data + 8),
				get_be16(data + 10), get_be32(data + 12));
		if (get_be16(data + 4) == 0x0000 &&
				get_be16(data + 6) == 0x1000 &&
				get_be16(data + 8) == 0x8000 &&
				get_be16(data + 10) == 0x0080 &&
				get_be32(data + 12) == 0x5F9B34FB)
			print_field("%*c%s", indent, ' ',
				uuid32_to_str(get_be32(data)));
		break;
	default:
		packet_hexdump(data, size);
		break;
	}
}

static void print_string(uint8_t indent, const uint8_t *data, uint32_t size)
{
	char *str = alloca(size + 1);

	str[size] = '\0';
	strncpy(str, (const char *) data, size);

	print_field("%*c%s [len %d]", indent, ' ', str, size);
}

static void print_boolean(uint8_t indent, const uint8_t *data, uint32_t size)
{
	print_field("%*c%s", indent, ' ', data[0] ? "true" : "false");
}

#define SIZES(args...) ((uint8_t[]) { args, 0xff } )

static struct {
	uint8_t value;
	uint8_t *sizes;
	bool recurse;
	const char *str;
	void (*print) (uint8_t indent, const uint8_t *data, uint32_t size);
} type_table[] = {
	{ 0, SIZES(0),             false, "Nil"			},
	{ 1, SIZES(0, 1, 2, 3, 4), false, "Unsigned Integer",	print_uint },
	{ 2, SIZES(0, 1, 2, 3, 4), false, "Signed Integer",	print_sint },
	{ 3, SIZES(1, 2, 4),       false, "UUID",		print_uuid },
	{ 4, SIZES(5, 6, 7),       false, "String",		print_string },
	{ 5, SIZES(0),             false, "Boolean",		print_boolean },
	{ 6, SIZES(5, 6, 7),       true,  "Sequence"		},
	{ 7, SIZES(5, 6, 7),       true,  "Alternative"		},
	{ 8, SIZES(5, 6, 7),       false, "URL",		print_string },
	{ }
};

static struct {
	uint8_t index;
	uint8_t bits;
	uint8_t size;
	const char *str;
} size_table[] = {
	{ 0,  0,  1, "1 byte"	},
	{ 1,  0,  2, "2 bytes"	},
	{ 2,  0,  4, "4 bytes"	},
	{ 3,  0,  8, "8 bytes"	},
	{ 4,  0, 16, "16 bytes"	},
	{ 5,  8,  0, "8 bits"	},
	{ 6, 16,  0, "16 bits"	},
	{ 7, 32,  0, "32 bits"	},
	{ }
};

static bool valid_size(uint8_t size, uint8_t *sizes)
{
	int i;

	for (i = 0; sizes[i] != 0xff; i++) {
		if (sizes[i] == size)
			return true;
	}

	return false;
}

static uint8_t get_bits(const uint8_t *data, uint32_t size)
{
	int i;

	for (i = 0; size_table[i].str; i++) {
		if (size_table[i].index == (data[0] & 0x07))
			return size_table[i].bits;
	}

	return 0;
}

static uint32_t get_size(const uint8_t *data, uint32_t size)
{
	int i;

	for (i = 0; size_table[i].str; i++) {
		if (size_table[i].index == (data[0] & 0x07)) {
			switch (size_table[i].bits) {
			case 0:
				if ((data[0] & 0xf8) == 0)
					return 0;
				else
					return size_table[i].size;
			case 8:
				return data[1];
			case 16:
				return get_be16(data + 1);
			case 32:
				return get_be32(data + 1);
			default:
				return 0;
			}
		}
	}

	return 0;
}

static void decode_data_elements(uint32_t position, uint8_t indent,
				const uint8_t *data, uint32_t size,
				void (*print_func) (uint32_t, uint8_t, uint8_t,
						const uint8_t *, uint32_t))

{
	uint32_t datalen, elemlen, extrabits;
	int i;

	if (!size)
		return;

	extrabits = get_bits(data, size);

	if (size < 1 + (extrabits / 8)) {
		print_text(COLOR_ERROR, "data element descriptor too short");
		packet_hexdump(data, size);
		return;
	}

	datalen = get_size(data, size);

	if (size < 1 + (extrabits / 8) + datalen) {
		print_text(COLOR_ERROR, "data element size too short");
		packet_hexdump(data, size);
		return;
	}

	elemlen = 1 + (extrabits / 8) + datalen;

	for (i = 0; type_table[i].str; i++) {
		uint8_t type = (data[0] & 0xf8) >> 3;

		if (type_table[i].value != type)
			continue;

		if (print_func) {
			print_func(position, indent, type,
					data + 1 + (extrabits / 8), datalen);
			break;
		}

		print_field("%*c%s (%d) with %u byte%s [%u extra bits] len %u",
					indent, ' ', type_table[i].str, type,
					datalen, datalen == 1 ? "" : "s",
					extrabits, elemlen);
		if (!valid_size(data[0] & 0x07, type_table[i].sizes)) {
			print_text(COLOR_ERROR, "invalid data element size");
			packet_hexdump(data + 1 + (extrabits / 8), datalen);
			break;
		}

		if (type_table[i].recurse)
			decode_data_elements(0, indent + 2,
					data + 1 + (extrabits / 8), datalen,
								print_func);
		else if (type_table[i].print)
			type_table[i].print(indent + 2,
					data + 1 + (extrabits / 8), datalen);
		break;
	}

	data += elemlen;
	size -= elemlen;

	decode_data_elements(position + 1, indent, data, size, print_func);
}

static uint32_t get_bytes(const uint8_t *data, uint32_t size)
{
	switch (data[0] & 0x07) {
	case 5:
		return 2 + data[1];
	case 6:
		return 3 + get_be16(data + 1);
	case 7:
		return 5 + get_be32(data + 1);
	}

	return 0;
}

static struct {
	uint16_t id;
	const char *str;
} attribute_table[] = {
	{ 0x0000, "Service Record Handle"		},
	{ 0x0001, "Service Class ID List"		},
	{ 0x0002, "Service Record State"		},
	{ 0x0003, "Service ID"				},
	{ 0x0004, "Protocol Descriptor List"		},
	{ 0x0005, "Browse Group List"			},
	{ 0x0006, "Language Base Attribute ID List"	},
	{ 0x0007, "Service Info Time To Live"		},
	{ 0x0008, "Service Availability"		},
	{ 0x0009, "Bluetooth Profile Descriptor List"	},
	{ 0x000a, "Documentation URL"			},
	{ 0x000b, "Client Executable URL"		},
	{ 0x000c, "Icon URL"				},
	{ 0x000d, "Additional Protocol Descriptor List" },
	{ }
};

static void print_attr(uint32_t position, uint8_t indent, uint8_t type,
					const uint8_t *data, uint32_t size)
{
	int i;

	if ((position % 2) == 0) {
		uint16_t id = get_be16(data);
		const char *str = "Unknown";

		for (i = 0; attribute_table[i].str; i++) {
			if (attribute_table[i].id == id)
				str = attribute_table[i].str;
		}

		print_field("%*cAttribute: %s (0x%4.4x) [len %d]",
						indent, ' ', str, id, size);
		return;
	}

	for (i = 0; type_table[i].str; i++) {
		if (type_table[i].value != type)
			continue;

		if (type_table[i].recurse)
			decode_data_elements(0, indent + 2, data, size, NULL);
		else if (type_table[i].print)
			type_table[i].print(indent + 2, data, size);
		break;
	}
}

static void print_attr_list(uint32_t position, uint8_t indent, uint8_t type,
					const uint8_t *data, uint32_t size)
{
	print_field("%*cAttribute list: [len %d] {position %d}",
						indent, ' ', size, position);

	decode_data_elements(0, indent + 2, data, size, print_attr);
}

static void print_attr_lists(uint32_t position, uint8_t indent, uint8_t type,
					const uint8_t *data, uint32_t size)
{
	decode_data_elements(0, indent, data, size, print_attr_list);
}

static void print_continuation(const uint8_t *data, uint16_t size)
{
	if (data[0] != size - 1) {
		print_text(COLOR_ERROR, "invalid continuation state");
		packet_hexdump(data, size);
		return;
	}

	print_field("Continuation state: %d", data[0]);
	packet_hexdump(data + 1, size - 1);
}

static void store_continuation(struct tid_data *tid,
					const uint8_t *data, uint16_t size)
{
	memcpy(tid->cont, data, size);
	print_continuation(data, size);
}

#define MAX_CONT 8

struct cont_data {
	uint16_t channel;
	uint8_t cont[17];
	void *data;
	uint32_t size;
};

static struct cont_data cont_list[MAX_CONT];

static void handle_continuation(struct tid_data *tid, bool nested,
			uint16_t bytes, const uint8_t *data, uint16_t size)
{
	uint8_t *newdata;
	int i, n = -1;

	if (bytes + 1 > size) {
		print_text(COLOR_ERROR, "missing continuation state");
		return;
	}

	if (tid->cont[0] == 0x00 && data[bytes] == 0x00) {
		decode_data_elements(0, 2, data, bytes,
				nested ? print_attr_lists : print_attr_list);

		print_continuation(data + bytes, size - bytes);
		return;
	}

	for (i = 0; i < MAX_CONT; i++) {
		if (cont_list[i].cont[0] == 0x00) {
			if (n < 0)
				n = i;
			continue;
		}

		if (cont_list[i].channel != tid->channel)
			continue;

		if (cont_list[i].cont[0] != tid->cont[0])
			continue;

		if (!memcmp(cont_list[i].cont + 1,
					tid->cont + 1, tid->cont[0])) {
			n = i;
			break;
		}
	}

	print_continuation(data + bytes, size - bytes);

	if (n < 0)
		return;

	newdata = realloc(cont_list[n].data, cont_list[n].size + bytes);
	if (!newdata) {
		print_text(COLOR_ERROR, "failed buffer allocation");
		free(cont_list[n].data);
		cont_list[n].data = NULL;
		cont_list[n].size = 0;
		return;
	}

	cont_list[n].channel = tid->channel;
	cont_list[n].data = newdata;

	if (bytes > 0) {
		memcpy(cont_list[n].data + cont_list[n].size, data, bytes);
		cont_list[n].size += bytes;
	}

	if (data[bytes] == 0x00) {
		print_field("Combined attribute bytes: %d", cont_list[n].size);

		decode_data_elements(0, 2, cont_list[n].data, cont_list[n].size,
				nested ? print_attr_lists : print_attr_list);

		free(cont_list[n].data);
		cont_list[n].data = NULL;
		cont_list[n].size = 0;
	} else
		memcpy(cont_list[i].cont, data + bytes, data[bytes] + 1);
}

static uint16_t common_rsp(const struct l2cap_frame *frame,
						struct tid_data *tid)
{
	uint16_t bytes;

	if (frame->size < 2) {
		print_text(COLOR_ERROR, "invalid size");
		packet_hexdump(frame->data, frame->size);
		return 0;
	}

	bytes = get_be16(frame->data);
	print_field("Attribute bytes: %d", bytes);

	if (bytes > frame->size - 2) {
		print_text(COLOR_ERROR, "invalid attribute size");
		packet_hexdump(frame->data + 2, frame->size - 2);
		return 0;
	}

	return bytes;
}

static void error_rsp(const struct l2cap_frame *frame, struct tid_data *tid)
{
	uint16_t error;

	clear_tid(tid);

	if (frame->size < 2) {
		print_text(COLOR_ERROR, "invalid size");
		packet_hexdump(frame->data, frame->size);
		return;
	}

	error = get_be16(frame->data);

	print_field("Error code: 0x%2.2x", error);
}

static void service_req(const struct l2cap_frame *frame, struct tid_data *tid)
{
	uint32_t search_bytes;

	search_bytes = get_bytes(frame->data, frame->size);
	print_field("Search pattern: [len %d]", search_bytes);

	if (search_bytes + 2 > frame->size) {
		print_text(COLOR_ERROR, "invalid search list length");
		packet_hexdump(frame->data, frame->size);
		return;
	}

	decode_data_elements(0, 2, frame->data, search_bytes, NULL);

	print_field("Max record count: %d",
				get_be16(frame->data + search_bytes));

	print_continuation(frame->data + search_bytes + 2,
					frame->size - search_bytes - 2);
}

static void service_rsp(const struct l2cap_frame *frame, struct tid_data *tid)
{
	uint16_t count;
	int i;

	clear_tid(tid);

	if (frame->size < 4) {
		print_text(COLOR_ERROR, "invalid size");
		packet_hexdump(frame->data, frame->size);
		return;
	}

	count = get_be16(frame->data + 2);

	print_field("Total record count: %d", get_be16(frame->data));
	print_field("Current record count: %d", count);

	for (i = 0; i < count; i++)
		print_field("Record handle: 0x%4.4x",
				get_be32(frame->data + 4 + (i * 4)));

	print_continuation(frame->data + 4 + (count * 4),
					frame->size - 4 - (count * 4));
}

static void attr_req(const struct l2cap_frame *frame, struct tid_data *tid)
{
	uint32_t attr_bytes;

	if (frame->size < 6) {
		print_text(COLOR_ERROR, "invalid size");
		packet_hexdump(frame->data, frame->size);
		return;
	}

	print_field("Record handle: 0x%4.4x", get_be32(frame->data));
	print_field("Max attribute bytes: %d", get_be16(frame->data + 4));

	attr_bytes = get_bytes(frame->data + 6, frame->size - 6);
	print_field("Attribute list: [len %d]", attr_bytes);

	if (attr_bytes + 6 > frame->size) {
		print_text(COLOR_ERROR, "invalid attribute list length");
		packet_hexdump(frame->data, frame->size);
		return;
	}

	decode_data_elements(0, 2, frame->data + 6, attr_bytes, NULL);

	store_continuation(tid, frame->data + 6 + attr_bytes,
					frame->size - 6 - attr_bytes);
}

static void attr_rsp(const struct l2cap_frame *frame, struct tid_data *tid)
{
	uint16_t bytes;

	bytes = common_rsp(frame, tid);

	handle_continuation(tid, false, bytes,
					frame->data + 2, frame->size - 2);

	clear_tid(tid);
}

static void search_attr_req(const struct l2cap_frame *frame,
						struct tid_data *tid)
{
	uint32_t search_bytes, attr_bytes;

	search_bytes = get_bytes(frame->data, frame->size);
	print_field("Search pattern: [len %d]", search_bytes);

	if (search_bytes + 2 > frame->size) {
		print_text(COLOR_ERROR, "invalid search list length");
		packet_hexdump(frame->data, frame->size);
		return;
	}

	decode_data_elements(0, 2, frame->data, search_bytes, NULL);

	print_field("Max record count: %d",
				get_be16(frame->data + search_bytes));

	attr_bytes = get_bytes(frame->data + search_bytes + 2,
				frame->size - search_bytes - 2);
	print_field("Attribute list: [len %d]", attr_bytes);

	decode_data_elements(0, 2, frame->data + search_bytes + 2,
						attr_bytes, NULL);

	store_continuation(tid, frame->data + search_bytes + 2 + attr_bytes,
				frame->size - search_bytes - 2 - attr_bytes);
}

static void search_attr_rsp(const struct l2cap_frame *frame,
						struct tid_data *tid)
{
	uint16_t bytes;

	bytes = common_rsp(frame, tid);

	handle_continuation(tid, true, bytes, frame->data + 2, frame->size - 2);

	clear_tid(tid);
}

struct sdp_data {
	uint8_t pdu;
	const char *str;
	void (*func) (const struct l2cap_frame *frame, struct tid_data *tid);
};

static const struct sdp_data sdp_table[] = {
	{ 0x01, "Error Response",			error_rsp	},
	{ 0x02, "Service Search Request",		service_req	},
	{ 0x03, "Service Search Response",		service_rsp	},
	{ 0x04, "Service Attribute Request",		attr_req	},
	{ 0x05, "Service Attribute Response",		attr_rsp	},
	{ 0x06, "Service Search Attribute Request",	search_attr_req	},
	{ 0x07, "Service Search Attribute Response",	search_attr_rsp	},
	{ }
};

void sdp_packet(const struct l2cap_frame *frame)
{
	uint8_t pdu;
	uint16_t tid, plen;
	struct l2cap_frame sdp_frame;
	struct tid_data *tid_info;
	const struct sdp_data *sdp_data = NULL;
	const char *pdu_color, *pdu_str;
	int i;

	l2cap_frame_pull(&sdp_frame, frame, 0);

	if (!l2cap_frame_get_u8(&sdp_frame, &pdu) ||
				!l2cap_frame_get_be16(&sdp_frame, &tid) ||
				!l2cap_frame_get_be16(&sdp_frame, &plen)) {
		print_text(COLOR_ERROR, "frame too short");
		packet_hexdump(frame->data, frame->size);
		return;
	}

	if (sdp_frame.size != plen) {
		print_text(COLOR_ERROR, "invalid frame size");
		packet_hexdump(sdp_frame.data, sdp_frame.size);
		return;
	}

	for (i = 0; sdp_table[i].str; i++) {
		if (sdp_table[i].pdu == pdu) {
			sdp_data = &sdp_table[i];
			break;
		}
	}

	if (sdp_data) {
		if (sdp_data->func) {
			if (frame->in)
				pdu_color = COLOR_MAGENTA;
			else
				pdu_color = COLOR_BLUE;
		} else
			pdu_color = COLOR_WHITE_BG;
		pdu_str = sdp_data->str;
	} else {
		pdu_color = COLOR_WHITE_BG;
		pdu_str = "Unknown";
	}

	print_indent(6, pdu_color, "SDP: ", pdu_str, COLOR_OFF,
				" (0x%2.2x) tid %d len %d", pdu, tid, plen);

	tid_info = get_tid(tid, frame->chan);

	if (!sdp_data || !sdp_data->func || !tid_info) {
		packet_hexdump(sdp_frame.data, sdp_frame.size);
		return;
	}

	sdp_data->func(&sdp_frame, tid_info);
}

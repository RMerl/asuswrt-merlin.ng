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
#include <inttypes.h>

#include "src/shared/util.h"
#include "display.h"
#include "packet.h"
#include "crc.h"
#include "bt.h"
#include "ll.h"

#define COLOR_OPCODE		COLOR_MAGENTA
#define COLOR_OPCODE_UNKNOWN	COLOR_WHITE_BG

#define MAX_CHANNEL 16

struct channel_data {
	uint32_t access_addr;
	uint32_t crc_init;
};

static struct channel_data channel_list[MAX_CHANNEL];

static void set_crc_init(uint32_t access_addr, uint32_t crc_init)
{
	int i;

	for (i = 0; i < MAX_CHANNEL; i++) {
		if (channel_list[i].access_addr == 0x00000000 ||
				channel_list[i].access_addr == access_addr) {
			channel_list[i].access_addr = access_addr;
			channel_list[i].crc_init = crc_init;
			break;
		}
	}
}

static uint32_t get_crc_init(uint32_t access_addr)
{
	int i;

	for (i = 0; i < MAX_CHANNEL; i++) {
		if (channel_list[i].access_addr == access_addr)
			return channel_list[i].crc_init;
	}

	return 0x00000000;
}

static void advertising_packet(const void *data, uint8_t size)
{
	const uint8_t *ptr = data;
	uint8_t pdu_type, length, win_size, hop, sca;
	bool tx_add, rx_add;
	uint32_t access_addr, crc_init;
	uint16_t win_offset, interval, latency, timeout;
	const char *str;

	if (size < 2) {
		print_text(COLOR_ERROR, "packet too short");
		packet_hexdump(data, size);
		return;
	}

	pdu_type = ptr[0] & 0x0f;
	tx_add = !!(ptr[0] & 0x40);
	rx_add = !!(ptr[0] & 0x80);
	length = ptr[1] & 0x3f;

	switch (pdu_type) {
	case 0x00:
		str = "ADV_IND";
		break;
	case 0x01:
		str = "ADV_DIRECT_IND";
		break;
	case 0x02:
		str = "ADV_NONCONN_IND";
		break;
	case 0x03:
		str = "SCAN_REQ";
		break;
	case 0x04:
		str = "SCAN_RSP";
		break;
	case 0x05:
		str = "CONNECT_REQ";
		break;
	case 0x06:
		str = "ADV_SCAN_IND";
		break;
	default:
		str = "Reserved";
		break;
	}

	print_field("Type: %s (0x%2.2x)", str, pdu_type);
	print_field("TxAdd: %u", tx_add);
	print_field("RxAdd: %u", rx_add);
	print_field("Length: %u", length);

	if (length != size - 2) {
		print_text(COLOR_ERROR, "packet size mismatch");
		packet_hexdump(data + 2, size - 2);
		return;
	}

	switch (pdu_type) {
	case 0x00:	/* ADV_IND */
	case 0x02:	/* AVD_NONCONN_IND */
	case 0x06:	/* ADV_SCAN_IND */
	case 0x04:	/* SCAN_RSP */
		if (length < 6) {
			print_text(COLOR_ERROR, "payload too short");
			packet_hexdump(data + 2, length);
			return;
		}

		packet_print_addr("Advertiser address", data + 2, tx_add);
		packet_print_ad(data + 8, length - 6);
		break;

	case 0x01:	/* ADV_DIRECT_IND */
		if (length < 12) {
			print_text(COLOR_ERROR, "payload too short");
			packet_hexdump(data + 2, length);
			return;
		}

		packet_print_addr("Advertiser address", data + 2, tx_add);
		packet_print_addr("Inititator address", data + 8, rx_add);
		break;

	case 0x03:	/* SCAN_REQ */
		if (length < 12) {
			print_text(COLOR_ERROR, "payload too short");
			packet_hexdump(data + 2, length);
			return;
		}

		packet_print_addr("Scanner address", data + 2, tx_add);
		packet_print_addr("Advertiser address", data + 8, rx_add);
		break;

	case 0x05:	/* CONNECT_REQ */
		if (length < 34) {
			print_text(COLOR_ERROR, "payload too short");
			packet_hexdump(data + 2, length);
			return;
		}

		packet_print_addr("Inititator address", data + 2, tx_add);
		packet_print_addr("Advertiser address", data + 8, rx_add);

		access_addr = ptr[14] | ptr[15] << 8 |
					ptr[16] << 16 | ptr[17] << 24;
		crc_init = ptr[18] | ptr[19] << 8 | ptr[20] << 16;

		print_field("Access address: 0x%8.8x", access_addr);
		print_field("CRC init: 0x%6.6x", crc_init);

		set_crc_init(access_addr, crc24_bit_reverse(crc_init));

		win_size = ptr[21];
		win_offset = ptr[22] | ptr[23] << 8;
		interval = ptr[24] | ptr[25] << 8;
		latency = ptr[26] | ptr[27] << 8;
		timeout = ptr[28] | ptr[29] << 8;

		print_field("Transmit window size: %u", win_size);
		print_field("Transmit window offset: %u", win_offset);
		print_field("Connection interval: %u", interval);
		print_field("Connection slave latency: %u", latency);
		print_field("Connection supervision timeout: %u", timeout);

		packet_print_channel_map_ll(ptr + 30);

		hop = ptr[35] & 0x1f;
		sca = (ptr[35] & 0xe0) >> 5;

		switch (sca) {
		case 0:
			str = "251 ppm to 500 ppm";
			break;
		case 1:
			str = "151 ppm to 250 ppm";
			break;
		case 2:
			str = "101 ppm to 150ppm";
			break;
		case 3:
			str = "76 ppm to 100 ppm";
			break;
		case 4:
			str = "51 ppm to 75 ppm";
			break;
		case 5:
			str = "31 ppm to 50 ppm";
			break;
		case 6:
			str = "21 ppm to 30 ppm";
			break;
		case 7:
			str = "0 ppm to 20 ppm";
			break;
		default:
			str = "Invalid";
			break;
		}

		print_field("Hop increment: %u", hop);
		print_field("Sleep clock accuracy: %s (%u)", str, sca);
		break;

	default:
		packet_hexdump(data + 2, length);
		break;
	}
}

static void data_packet(const void *data, uint8_t size, bool padded)
{
	const uint8_t *ptr = data;
	uint8_t llid, length;
	bool nesn, sn, md;
	const char *str;

	if (size < 2) {
		print_text(COLOR_ERROR, "packet too short");
		packet_hexdump(data, size);
		return;
	}

	llid = ptr[0] & 0x03;
	nesn = !!(ptr[0] & 0x04);
	sn = !!(ptr[0] & 0x08);
	md = !!(ptr[0] & 0x10);
	length = ptr[1] & 0x1f;

	switch (llid) {
	case 0x01:
		if (length > 0)
			str = "Continuation fragement of L2CAP message";
		else
			str = "Empty message";
		break;
	case 0x02:
		str = "Start of L2CAP message";
		break;
	case 0x03:
		str = "Control";
		break;
	default:
		str = "Reserved";
		break;
	}

	print_field("LLID: %s (0x%2.2x)", str, llid);
	print_field("Next expected sequence number: %u", nesn);
	print_field("Sequence number: %u", sn);
	print_field("More data: %u", md);
	print_field("Length: %u", length);

	switch (llid) {
	case 0x03:
		llcp_packet(data + 2, size - 2, padded);
		break;

	default:
		packet_hexdump(data + 2, size - 2);
		break;
	}
}

void ll_packet(uint16_t frequency, const void *data, uint8_t size, bool padded)
{
	const struct bt_ll_hdr *hdr = data;
	uint8_t channel = (frequency - 2402) / 2;
	uint32_t access_addr;
	char access_str[12];
	const char *channel_label, *channel_color;
	const uint8_t *pdu_data;
	uint8_t pdu_len;
	uint32_t pdu_crc, crc, crc_init;

	if (size < sizeof(*hdr)) {
		print_text(COLOR_ERROR, "packet missing header");
		packet_hexdump(data, size);
		return;
	}

	if (size < sizeof(*hdr) + 3) {
		print_text(COLOR_ERROR, "packet missing checksum");
		packet_hexdump(data, size);
		return;
	}

	if (hdr->preamble != 0xaa && hdr->preamble != 0x55) {
		print_text(COLOR_ERROR, "invalid preamble");
		packet_hexdump(data, size);
		return;
	}

	access_addr = le32_to_cpu(hdr->access_addr);

	pdu_data = data + sizeof(*hdr);
	pdu_len = size - sizeof(*hdr) - 3;

	pdu_crc = pdu_data[pdu_len + 0] | (pdu_data[pdu_len + 1] << 8) |
						(pdu_data[pdu_len + 2] << 16);

	if (access_addr == 0x8e89bed6) {
		channel_label = "Advertising channel: ";
		channel_color = COLOR_MAGENTA;
	} else {
		channel_label = "Data channel: ";
		channel_color = COLOR_CYAN;
	}

	sprintf(access_str, "0x%8.8x", access_addr);

	print_indent(6, channel_color, channel_label, access_str, COLOR_OFF,
		" (channel %d) len %d crc 0x%6.6x", channel, pdu_len, pdu_crc);

	if (access_addr == 0x8e89bed6)
		crc_init = 0xaaaaaa;
	else
		crc_init = get_crc_init(access_addr);

	if (crc_init) {
		crc = crc24_calculate(crc_init, pdu_data, pdu_len);

		if (crc != pdu_crc) {
			print_text(COLOR_ERROR, "invalid checksum");
			packet_hexdump(pdu_data, pdu_len);
			return;
		}
	} else
		print_text(COLOR_ERROR, "unknown access address");

	if (access_addr == 0x8e89bed6)
		advertising_packet(pdu_data, pdu_len);
	else
		data_packet(pdu_data, pdu_len, padded);
}

static void null_pdu(const void *data, uint8_t size)
{
}

static void conn_update_req(const void *data, uint8_t size)
{
	const struct bt_ll_conn_update_req *pdu = data;

	print_field("Transmit window size: %u", pdu->win_size);
	print_field("Transmit window offset: %u", le16_to_cpu(pdu->win_offset));
	print_field("Connection interval: %u", le16_to_cpu(pdu->interval));
	print_field("Connection slave latency: %u", le16_to_cpu(pdu->latency));
	print_field("Connection supervision timeout: %u", le16_to_cpu(pdu->timeout));
	print_field("Connection instant: %u", le16_to_cpu(pdu->instant));
}

static void channel_map_req(const void *data, uint8_t size)
{
	const struct bt_ll_channel_map_req *pdu = data;

	packet_print_channel_map_ll(pdu->map);
	print_field("Connection instant: %u", le16_to_cpu(pdu->instant));
}

static void terminate_ind(const void *data, uint8_t size)
{
	const struct bt_ll_terminate_ind *pdu = data;

	packet_print_error("Error code", pdu->error);
}

static void enc_req(const void *data, uint8_t size)
{
	const struct bt_ll_enc_req *pdu = data;

	print_field("Rand: 0x%16.16" PRIx64, le64_to_cpu(pdu->rand));
	print_field("EDIV: 0x%4.4x", le16_to_cpu(pdu->ediv));
	print_field("SKD (master): 0x%16.16" PRIx64, le64_to_cpu(pdu->skd));
	print_field("IV (master): 0x%8.8x", le32_to_cpu(pdu->iv));
}

static void enc_rsp(const void *data, uint8_t size)
{
	const struct bt_ll_enc_rsp *pdu = data;

	print_field("SKD (slave): 0x%16.16" PRIx64, le64_to_cpu(pdu->skd));
	print_field("IV (slave): 0x%8.8x", le32_to_cpu(pdu->iv));
}

static const char *opcode_to_string(uint8_t opcode);

static void unknown_rsp(const void *data, uint8_t size)
{
	const struct bt_ll_unknown_rsp *pdu = data;

	print_field("Unknown type: %s (0x%2.2x)",
				opcode_to_string(pdu->type), pdu->type);
}

static void feature_req(const void *data, uint8_t size)
{
	const struct bt_ll_feature_req *pdu = data;

	packet_print_features_ll(pdu->features);
}

static void feature_rsp(const void *data, uint8_t size)
{
	const struct bt_ll_feature_rsp *pdu = data;

	packet_print_features_ll(pdu->features);
}

static void version_ind(const void *data, uint8_t size)
{
	const struct bt_ll_version_ind *pdu = data;

	packet_print_version("Version", pdu->version,
				"Subversion", le16_to_cpu(pdu->subversion));
	packet_print_company("Company", le16_to_cpu(pdu->company));
}

static void reject_ind(const void *data, uint8_t size)
{
	const struct bt_ll_reject_ind *pdu = data;

	packet_print_error("Error code", pdu->error);
}

static void slave_feature_req(const void *data, uint8_t size)
{
	const struct bt_ll_slave_feature_req *pdu = data;

	packet_print_features_ll(pdu->features);
}

static void reject_ind_ext(const void *data, uint8_t size)
{
	const struct bt_ll_reject_ind_ext *pdu = data;

	print_field("Reject opcode: %u (0x%2.2x)", pdu->opcode, pdu->opcode);
	packet_print_error("Error code", pdu->error);
}

struct llcp_data {
	uint8_t opcode;
	const char *str;
	void (*func) (const void *data, uint8_t size);
	uint8_t size;
	bool fixed;
};

static const struct llcp_data llcp_table[] = {
	{ 0x00, "LL_CONNECTION_UPDATE_REQ", conn_update_req,   11, true },
	{ 0x01, "LL_CHANNEL_MAP_REQ",       channel_map_req,    7, true },
	{ 0x02, "LL_TERMINATE_IND",         terminate_ind,      1, true },
	{ 0x03, "LL_ENC_REQ",               enc_req,           22, true },
	{ 0x04, "LL_ENC_RSP",               enc_rsp,           12, true },
	{ 0x05, "LL_START_ENC_REQ",         null_pdu,           0, true },
	{ 0x06, "LL_START_ENC_RSP",         null_pdu,           0, true },
	{ 0x07, "LL_UNKNOWN_RSP",           unknown_rsp,        1, true },
	{ 0x08, "LL_FEATURE_REQ",           feature_req,        8, true },
	{ 0x09, "LL_FEATURE_RSP",           feature_rsp,        8, true },
	{ 0x0a, "LL_PAUSE_ENC_REQ",         null_pdu,           0, true },
	{ 0x0b, "LL_PAUSE_ENC_RSP",         null_pdu,           0, true },
	{ 0x0c, "LL_VERSION_IND",           version_ind,        5, true },
	{ 0x0d, "LL_REJECT_IND",            reject_ind,         1, true },
	{ 0x0e, "LL_SLAVE_FEATURE_REQ",     slave_feature_req,  8, true },
	{ 0x0f, "LL_CONNECTION_PARAM_REQ",  NULL,              23, true },
	{ 0x10, "LL_CONNECTION_PARAM_RSP",  NULL,              23, true },
	{ 0x11, "LL_REJECT_IND_EXT",        reject_ind_ext,     2, true },
	{ 0x12, "LL_PING_REQ",              null_pdu,           0, true },
	{ 0x13, "LL_PING_RSP",              null_pdu,           0, true },
	{ 0x14, "LL_LENGTH_REQ",            NULL,               8, true },
	{ 0x15, "LL_LENGTH_RSP",            NULL,               8, true },
	{ }
};

static const char *opcode_to_string(uint8_t opcode)
{
	int i;

	for (i = 0; llcp_table[i].str; i++) {
		if (llcp_table[i].opcode == opcode)
			return llcp_table[i].str;
	}

	return "Unknown";
}

void llcp_packet(const void *data, uint8_t size, bool padded)
{
	uint8_t opcode = ((const uint8_t *) data)[0];
	const struct llcp_data *llcp_data = NULL;
	const char *opcode_color, *opcode_str;
	int i;

	for (i = 0; llcp_table[i].str; i++) {
		if (llcp_table[i].opcode == opcode) {
			llcp_data = &llcp_table[i];
			break;
		}
	}

	if (llcp_data) {
		if (llcp_data->func)
			opcode_color = COLOR_OPCODE;
		else
			opcode_color = COLOR_OPCODE_UNKNOWN;
		opcode_str = llcp_data->str;
	} else {
		opcode_color = COLOR_OPCODE_UNKNOWN;
		opcode_str = "Unknown";
	}

	print_indent(6, opcode_color, "", opcode_str, COLOR_OFF,
						" (0x%2.2x)", opcode);

	if (!llcp_data || !llcp_data->func) {
		packet_hexdump(data + 1, size - 1);
		return;
	}

	if (llcp_data->fixed && !padded) {
		if (size - 1 != llcp_data->size) {
			print_text(COLOR_ERROR, "invalid packet size");
			packet_hexdump(data + 1, size - 1);
			return;
		}
	} else {
		if (size - 1 < llcp_data->size) {
			print_text(COLOR_ERROR, "too short packet");
			packet_hexdump(data + 1, size - 1);
			return;
		}
	}

	llcp_data->func(data + 1, size - 1);
}

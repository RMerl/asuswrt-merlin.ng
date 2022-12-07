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
#include <inttypes.h>

#include "src/shared/util.h"
#include "display.h"
#include "packet.h"
#include "crc.h"
#include "bt.h"
#include "ll.h"

#define COLOR_OPCODE		COLOR_MAGENTA
#define COLOR_OPCODE_UNKNOWN	COLOR_WHITE_BG
#define COLOR_UNKNOWN_OPTIONS_BIT COLOR_WHITE_BG

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

static void length_req_rsp(const void *data, uint8_t size)
{
	const struct bt_ll_length *pdu = data;

	print_field("MaxRxOctets: %u", pdu->rx_len);
	print_field("MaxRxTime: %u", pdu->rx_time);
	print_field("MaxTxOctets: %u", pdu->tx_len);
	print_field("MaxtxTime: %u", pdu->tx_time);
}

static const struct bitfield_data le_phys[] = {
	{  0, "LE 1M"	},
	{  1, "LE 2M"	},
	{  2, "LE Coded"},
	{ }
};

static void phy_req_rsp(const void *data, uint8_t size)
{
	const struct bt_ll_phy *pdu = data;
	uint8_t mask;

	print_field("RX PHYs: 0x%2.2x", pdu->rx_phys);

	mask = print_bitfield(2, pdu->rx_phys, le_phys);
	if (mask)
		print_text(COLOR_UNKNOWN_OPTIONS_BIT, "  Reserved"
							" (0x%2.2x)", mask);
	print_field("TX PHYs: 0x%2.2x", pdu->tx_phys);

	mask = print_bitfield(2, pdu->tx_phys, le_phys);
	if (mask)
		print_text(COLOR_UNKNOWN_OPTIONS_BIT, "  Reserved"
							" (0x%2.2x)", mask);
}

static void phy_update_ind(const void *data, uint8_t size)
{
	const struct bt_ll_phy_update_ind *pdu = data;
	uint8_t mask;

	print_field("M_TO_S_PHY: 0x%2.2x", pdu->m_phy);

	mask = print_bitfield(2, pdu->m_phy, le_phys);
	if (mask)
		print_text(COLOR_UNKNOWN_OPTIONS_BIT, "  Reserved"
							" (0x%2.2x)", mask);

	print_field("S_TO_M_PHY: 0x%2.2x", pdu->s_phy);

	mask = print_bitfield(2, pdu->s_phy, le_phys);
	if (mask)
		print_text(COLOR_UNKNOWN_OPTIONS_BIT, "  Reserved"
							" (0x%2.2x)", mask);

	print_field("Instant: 0x%4.4x", pdu->instant);
}

static void min_used_channels(const void *data, uint8_t size)
{
	const struct bt_ll_min_used_channels *pdu = data;
	uint8_t mask;

	print_field("PHYS: 0x%2.2x", pdu->phys);

	mask = print_bitfield(2, pdu->phys, le_phys);
	if (mask)
		print_text(COLOR_UNKNOWN_OPTIONS_BIT, "  Reserved"
							" (0x%2.2x)", mask);

	print_field("MinUsedChannels: 0x%2.2x", pdu->min_channels);
}

static void cte_req(const void *data, uint8_t size)
{
	const struct bt_ll_cte_req *pdu = data;

	print_field("MinCTELenReq: 0x%2.2x", pdu->cte & 0xf8);
	print_field("CTETypeReq: 0x%2.2x", pdu->cte & 0x03);

	switch (pdu->cte & 0x03) {
	case 0x00:
		print_field("  AoA Constant Tone Extension");
		break;
	case 0x01:
		print_field("  AoD Constant Tone Extension with 1 μs slots");
		break;
	case 0x02:
		print_field("  AoD Constant Tone Extension with 2 μs slots");
		break;
	}
}

static void periodic_sync_ind(const void *data, uint8_t size)
{
	const struct bt_ll_periodic_sync_ind *pdu = data;
	uint8_t mask;

	print_field("ID: 0x%4.4x", pdu->id);
	print_field("SyncInfo:");
	packet_hexdump(pdu->info, sizeof(pdu->info));
	print_field("connEventCount: 0x%4.4x", pdu->event_count);
	print_field("lastPaEventCounter: 0x%4.4x", pdu->last_counter);
	print_field("SID: 0x%2.2x", pdu->adv_info & 0xf0);
	print_field("AType: %s", pdu->adv_info & 0x08 ? "random" : "public");
	print_field("SCA: 0x%2.2x", pdu->adv_info & 0x07);
	print_field("PHY: 0x%2.2x", pdu->phy);

	mask = print_bitfield(2, pdu->phy, le_phys);
	if (mask)
		print_text(COLOR_UNKNOWN_OPTIONS_BIT, "  Reserved"
							" (0x%2.2x)", mask);

	packet_print_addr("AdvA", pdu->adv_addr, pdu->adv_info & 0x08);
	print_field("syncConnEventCount: 0x%4.4x", pdu->sync_counter);
}

static void clock_acc_req_rsp(const void *data, uint8_t size)
{
	const struct bt_ll_clock_acc *pdu = data;

	print_field("SCA: 0x%2.2x", pdu->sca);
}

static void cis_req(const void *data, uint8_t size)
{
	const struct bt_ll_cis_req *cmd = data;
	uint32_t interval;
	uint8_t mask;

	print_field("CIG ID: 0x%2.2x", cmd->cig);
	print_field("CIS ID: 0x%2.2x", cmd->cis);
	print_field("Master to Slave PHY: 0x%2.2x", cmd->m_phy);

	mask = print_bitfield(2, cmd->m_phy, le_phys);
	if (mask)
		print_text(COLOR_UNKNOWN_OPTIONS_BIT, "  Reserved"
							" (0x%2.2x)", mask);

	print_field("Slave To Master PHY: 0x%2.2x", cmd->s_phy);

	mask = print_bitfield(2, cmd->s_phy, le_phys);
	if (mask)
		print_text(COLOR_UNKNOWN_OPTIONS_BIT, "  Reserved"
							" (0x%2.2x)", mask);

	print_field("Master to Slave Maximum SDU: %u", cmd->m_sdu);
	print_field("Slave to Master Maximum SDU: %u", cmd->s_sdu);

	memcpy(&interval, cmd->m_interval, sizeof(cmd->m_interval));
	print_field("Master to Slave Interval: 0x%6.6x", le32_to_cpu(interval));
	memcpy(&interval, cmd->s_interval, sizeof(cmd->s_interval));
	print_field("Slave to Master Interval: 0x%6.6x", le32_to_cpu(interval));

	print_field("Master to Slave Maximum PDU: %u", cmd->m_pdu);
	print_field("Slave to Master Maximum PDU: %u", cmd->s_pdu);

	print_field("Burst Number: %u us", cmd->bn);

	memcpy(&interval, cmd->sub_interval, sizeof(cmd->sub_interval));
	print_field("Sub-Interval: 0x%6.6x", le32_to_cpu(interval));

	print_field("Master to Slave Flush Timeout: %u", cmd->m_ft);
	print_field("Slave to Master Flush Timeout: %u", cmd->s_ft);

	print_field("ISO Interval: 0x%4.4x", le16_to_cpu(cmd->iso_interval));

	memcpy(&interval, cmd->offset_min, sizeof(cmd->offset_min));
	print_field("CIS Offset Minimum: 0x%6.6x", le32_to_cpu(interval));
	memcpy(&interval, cmd->offset_max, sizeof(cmd->offset_max));
	print_field("CIS Offset Maximum: 0x%6.6x", le32_to_cpu(interval));

	print_field("Connection Event Count: %u", cmd->conn_event_count);
}

static void cis_rsp(const void *data, uint8_t size)
{
	const struct bt_ll_cis_rsp *rsp = data;
	uint32_t interval;

	memcpy(&interval, rsp->offset_min, sizeof(rsp->offset_min));
	print_field("CIS Offset Minimum: 0x%6.6x", le32_to_cpu(interval));
	memcpy(&interval, rsp->offset_max, sizeof(rsp->offset_max));
	print_field("CIS Offset Maximum: 0x%6.6x", le32_to_cpu(interval));

	print_field("Connection Event Count: %u", rsp->conn_event_count);
}

static void cis_ind(const void *data, uint8_t size)
{
	const struct bt_ll_cis_ind *ind = data;
	uint32_t interval;

	print_field("CIS Access Address: 0x%4.4x", le32_to_cpu(ind->addr));
	memcpy(&interval, ind->cis_offset, sizeof(ind->cis_offset));
	print_field("CIS Offset: 0x%6.6x", le32_to_cpu(interval));

	memcpy(&interval, ind->cig_sync_delay, sizeof(ind->cig_sync_delay));
	print_field("CIG Synchronization Delay: 0x%6.6x",
					le32_to_cpu(interval));
	memcpy(&interval, ind->cis_sync_delay, sizeof(ind->cis_sync_delay));
	print_field("CIS Synchronization Delay: %u us",
					le32_to_cpu(interval));
	print_field("Connection Event Count: %u", ind->conn_event_count);
}

static void cis_term_ind(const void *data, uint8_t size)
{
	const struct bt_ll_cis_term_ind *ind = data;

	print_field("CIG ID: 0x%2.2x", ind->cig);
	print_field("CIS ID: 0x%2.2x", ind->cis);
	packet_print_error("Reason", ind->reason);
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
	{ 0x14, "LL_LENGTH_REQ",            length_req_rsp,     8, true },
	{ 0x15, "LL_LENGTH_RSP",            length_req_rsp,     8, true },
	{ 0x16, "LL_PHY_REQ",               phy_req_rsp,        2, true },
	{ 0x17, "LL_PHY_RSP",               phy_req_rsp,        2, true },
	{ 0x18, "LL_PHY_UPDATE_IND",        phy_update_ind,     4, true },
	{ 0x19, "LL_MIN_USED_CHANNELS_IND", min_used_channels,  2, true },
	{ 0x1a, "LL_CTE_REQ",               cte_req,            1, true },
	{ 0x1b, "LL_CTE_RSP",               null_pdu,           0, true },
	{ 0x1c, "LL_PERIODIC_SYNC_IND",     periodic_sync_ind, 34, true },
	{ 0x1d, "LL_CLOCK_ACCURACY_REQ",    clock_acc_req_rsp,  1, true },
	{ 0x1e, "LL_CLOCK_ACCURACY_RSP",    clock_acc_req_rsp,  1, true },
	{ BT_LL_CIS_REQ, "LL_CIS_REQ",      cis_req,
					sizeof(struct bt_ll_cis_req), true },
	{ BT_LL_CIS_RSP, "LL_CIS_RSP",      cis_rsp,
					sizeof(struct bt_ll_cis_rsp), true },
	{ BT_LL_CIS_IND, "LL_CIS_IND",      cis_ind,
					sizeof(struct bt_ll_cis_ind), true },
	{ BT_LL_CIS_TERMINATE_IND, "LL_CIS_TERMINATE_IND", cis_term_ind,
					sizeof(struct bt_ll_cis_term_ind),
					true },
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

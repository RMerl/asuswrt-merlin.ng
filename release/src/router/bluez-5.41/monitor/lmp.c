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
#include <string.h>

#include "src/shared/util.h"
#include "display.h"
#include "packet.h"
#include "bt.h"
#include "lmp.h"

#define COLOR_OPCODE		COLOR_MAGENTA
#define COLOR_OPCODE_UNKNOWN	COLOR_WHITE_BG

static const char *get_opcode_str(uint16_t opcode);

static void print_opcode(uint16_t opcode)
{
	const char *str;

	str = get_opcode_str(opcode);
	if (!str)
		str = "Unknown";

	if (opcode & 0xff00)
		print_field("Operation: %s (%u/%u)", str,
						opcode >> 8, opcode & 0xff);
	else
		print_field("Operation: %s (%u)", str, opcode);
}

static void name_req(const void *data, uint8_t size)
{
	const struct bt_lmp_name_req *pdu = data;

	print_field("Offset: %u", pdu->offset);
}

static void name_rsp(const void *data, uint8_t size)
{
	const struct bt_lmp_name_rsp *pdu = data;
	char str[15];

	memcpy(str, pdu->fragment, 14);
	str[14] = '\0';

	print_field("Offset: %u", pdu->offset);
	print_field("Length: %u", pdu->length);
	print_field("Fragment: %s", str);
}

static void accepted(const void *data, uint8_t size)
{
	const struct bt_lmp_accepted *pdu = data;

	print_opcode(pdu->opcode);
}

static void not_accepted(const void *data, uint8_t size)
{
	const struct bt_lmp_not_accepted *pdu = data;

	print_opcode(pdu->opcode);
	packet_print_error("Error code", pdu->error);
}

static void clkoffset_req(const void *data, uint8_t size)
{
}

static void clkoffset_rsp(const void *data, uint8_t size)
{
	const struct bt_lmp_clkoffset_rsp *pdu = data;

	print_field("Clock offset: 0x%4.4x", le16_to_cpu(pdu->offset));
}

static void detach(const void *data, uint8_t size)
{
	const struct bt_lmp_detach *pdu = data;

	packet_print_error("Error code", pdu->error);
}

static void au_rand(const void *data, uint8_t size)
{
	const struct bt_lmp_au_rand *pdu = data;

	packet_hexdump(pdu->number, 16);
}

static void sres(const void *data, uint8_t size)
{
	const struct bt_lmp_sres *pdu = data;

	packet_hexdump(pdu->response, 4);
}

static void encryption_mode_req(const void *data, uint8_t size)
{
	const struct bt_lmp_encryption_mode_req *pdu = data;
	const char *str;

	switch (pdu->mode) {
	case 0x00:
		str = "No encryption";
		break;
	case 0x01:
		str = "Encryption";
		break;
	case 0x02:
		str = "Encryption";
		break;
	default:
		str = "Reserved";
		break;
	}

	print_field("Mode: %s (%u)", str, pdu->mode);
}

static void encryption_key_size_req(const void *data, uint8_t size)
{
	const struct bt_lmp_encryption_key_size_req *pdu = data;

	print_field("Key size: %u", pdu->key_size);
}

static void start_encryption_req(const void *data, uint8_t size)
{
	const struct bt_lmp_start_encryption_req *pdu = data;

	packet_hexdump(pdu->number, 16);
}

static void stop_encryption_req(const void *data, uint8_t size)
{
}

static void switch_req(const void *data, uint8_t size)
{
	const struct bt_lmp_switch_req *pdu = data;

	print_field("Instant: 0x%8.8x", le32_to_cpu(pdu->instant));
}

static void unsniff_req(const void *data, uint8_t size)
{
}

static void max_power(const void *data, uint8_t size)
{
}

static void min_power(const void *data, uint8_t size)
{
}

static void auto_rate(const void *data, uint8_t size)
{
}

static void preferred_rate(const void *data, uint8_t size)
{
	const struct bt_lmp_preferred_rate *pdu = data;
	const char *str;

	str = (pdu->rate & 0x01) ? "do not use FEC" : "use FEC";

	print_field("Basic data rate: %s (0x%02x)", str, pdu->rate & 0x01);

	switch ((pdu->rate & 0x06) >> 1) {
	case 0:
		str = "No packet-size preference available";
		break;
	case 1:
		str = "use 1-slot packets";
		break;
	case 2:
		str = "use 3-slot packets";
		break;
	case 3:
		str = "use 5-slot packets";
		break;
	}

	print_field("Basic data rate: %s (0x%02x)", str, pdu->rate & 0x06);

	switch ((pdu->rate & 0x11) >> 3) {
	case 0:
		str = "use DM1 packets";
		break;
	case 1:
		str = "use 2 Mb/s packets";
		break;
	case 2:
		str = "use 3 MB/s packets";
		break;
	case 3:
		str = "reserved";
		break;
	}

	print_field("Enhanced data rate: %s (0x%2.2x)", str, pdu->rate & 0x11);

	switch ((pdu->rate & 0x60) >> 5) {
	case 0:
		str = "No packet-size preference available";
		break;
	case 1:
		str = "use 1-slot packets";
		break;
	case 2:
		str = "use 3-slot packets";
		break;
	case 3:
		str = "use 5-slot packets";
		break;
	}

	print_field("Enhanced data rate: %s (0x%2.2x)", str, pdu->rate & 0x60);
}

static void version_req(const void *data, uint8_t size)
{
	const struct bt_lmp_version_req *pdu = data;

	packet_print_version("Version", pdu->version,
				"Subversion", le16_to_cpu(pdu->subversion));
	packet_print_company("Company", le16_to_cpu(pdu->company));
}

static void version_res(const void *data, uint8_t size)
{
	const struct bt_lmp_version_res *pdu = data;

	packet_print_version("Version", pdu->version,
				"Subversion", le16_to_cpu(pdu->subversion));
	packet_print_company("Company", le16_to_cpu(pdu->company));
}

static void features_req(const void *data, uint8_t size)
{
	const struct bt_lmp_features_req *pdu = data;

	packet_print_features_lmp(pdu->features, 0x00);
}

static void features_res(const void *data, uint8_t size)
{
	const struct bt_lmp_features_res *pdu = data;

	packet_print_features_lmp(pdu->features, 0x00);
}

static void max_slot(const void *data, uint8_t size)
{
	const struct bt_lmp_max_slot *pdu = data;

	print_field("Slots: 0x%4.4x", pdu->slots);
}

static void max_slot_req(const void *data, uint8_t size)
{
	const struct bt_lmp_max_slot_req *pdu = data;

	print_field("Slots: 0x%4.4x", pdu->slots);
}

static void timing_accuracy_req(const void *data, uint8_t size)
{
}

static void timing_accuracy_res(const void *data, uint8_t size)
{
	const struct bt_lmp_timing_accuracy_res *pdu = data;

	print_field("Drift: %u ppm", pdu->drift);
	print_field("Jitter: %u usec", pdu->jitter);
}

static void setup_complete(const void *data, uint8_t size)
{
}

static void use_semi_permanent_key(const void *data, uint8_t size)
{
}

static void host_connection_req(const void *data, uint8_t size)
{
}

static void slot_offset(const void *data, uint8_t size)
{
	const struct bt_lmp_slot_offset *pdu = data;

	print_field("Offset: %u usec", le16_to_cpu(pdu->offset));
	packet_print_addr("Address", pdu->bdaddr, false);
}

static void page_scan_mode_req(const void *data, uint8_t size)
{
	const struct bt_lmp_page_scan_mode_req *pdu = data;
	const char *str;

	switch (pdu->scheme) {
	case 0x00:
		str = "Mandatory";
		break;
	default:
		str = "Reserved";
		break;
	}

	print_field("Paging scheme: %s (%u)", str, pdu->scheme);

	if (pdu->scheme == 0x00) {
		switch (pdu->settings) {
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
	} else
		str = "Reserved";

	print_field("Paging scheme settings: %s (%u)", str, pdu->settings);
}

static void test_activate(const void *data, uint8_t size)
{
}

static void encryption_key_size_mask_req(const void *data, uint8_t size)
{
}

static void set_afh(const void *data, uint8_t size)
{
	const struct bt_lmp_set_afh *pdu = data;
	const char *str;

	print_field("Instant: %u", le32_to_cpu(pdu->instant));

	switch (pdu->mode) {
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

	print_field("Mode: %s (0x%2.2x)", str, pdu->mode);
	packet_print_channel_map_lmp(pdu->map);
}

static void encapsulated_header(const void *data, uint8_t size)
{
	const struct bt_lmp_encapsulated_header *pdu = data;
	const char *str;

	print_field("Major type: %u", pdu->major);
	print_field("Minor type: %u", pdu->minor);

	if (pdu->major == 0x01) {
		switch (pdu->minor) {
		case 0x01:
			str = "P-192 Public Key";
			break;
		case 0x02:
			str = "P-256 Public Key";
			break;
		default:
			str = "Reserved";
			break;
		}

		print_field("  %s", str);
	}

	print_field("Length: %u", pdu->length);
}

static void encapsulated_payload(const void *data, uint8_t size)
{
	const struct bt_lmp_encapsulated_payload *pdu = data;

	packet_hexdump(pdu->data, 16);
}

static void simple_pairing_confirm(const void *data, uint8_t size)
{
	const struct bt_lmp_simple_pairing_confirm *pdu = data;

	packet_hexdump(pdu->value, 16);
}

static void simple_pairing_number(const void *data, uint8_t size)
{
	const struct bt_lmp_simple_pairing_number *pdu = data;

	packet_hexdump(pdu->value, 16);
}

static void dhkey_check(const void *data, uint8_t size)
{
	const struct bt_lmp_dhkey_check *pdu = data;

	packet_hexdump(pdu->value, 16);
}

static void accepted_ext(const void *data, uint8_t size)
{
	const struct bt_lmp_accepted_ext *pdu = data;
	uint16_t opcode;

	switch (pdu->escape) {
	case 127:
		opcode = LMP_ESC4(pdu->opcode);
		break;
	default:
		return;
	}

	print_opcode(opcode);
}

static void not_accepted_ext(const void *data, uint8_t size)
{
	const struct bt_lmp_not_accepted_ext *pdu = data;
	uint16_t opcode;

	switch (pdu->escape) {
	case 127:
		opcode = LMP_ESC4(pdu->opcode);
		break;
	default:
		return;
	}

	print_opcode(opcode);
	print_field("Error code: %u", pdu->error);
}

static void features_req_ext(const void *data, uint8_t size)
{
	const struct bt_lmp_features_req_ext *pdu = data;

	print_field("Features page: %u", pdu->page);
	print_field("Max supported page: %u", pdu->max_page);
	packet_print_features_lmp(pdu->features, pdu->page);
}

static void features_res_ext(const void *data, uint8_t size)
{
	const struct bt_lmp_features_res_ext *pdu = data;

	print_field("Features page: %u", pdu->page);
	print_field("Max supported page: %u", pdu->max_page);
	packet_print_features_lmp(pdu->features, pdu->page);
}

static void packet_type_table_req(const void *data, uint8_t size)
{
	const struct bt_lmp_packet_type_table_req *pdu = data;
	const char *str;

	switch (pdu->table) {
	case 0x00:
		str = "1 Mbps only";
		break;
	case 0x01:
		str = "2/3 Mbps";
		break;
	default:
		str = "Reserved";
		break;
	}

	print_field("Table: %s (0x%2.2x)", str, pdu->table);
}

static void channel_classification_req(const void *data, uint8_t size)
{
	const struct bt_lmp_channel_classification_req *pdu = data;
	const char *str;

	switch (pdu->mode) {
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

	print_field("Reporting mode: %s (0x%2.2x)", str, pdu->mode);
	print_field("Min interval: 0x%2.2x", pdu->min_interval);
	print_field("Max interval: 0x%2.2x", pdu->max_interval);
}

static void channel_classification(const void *data, uint8_t size)
{
	const struct bt_lmp_channel_classification *pdu = data;
	char str[21];
	int i;

	for (i = 0; i < 10; i++)
		sprintf(str + (i * 2), "%2.2x", pdu->classification[i]);

	print_field("Classification: 0x%s", str);
}

static void pause_encryption_req(const void *data, uint8_t size)
{
}

static void resume_encryption_req(const void *data, uint8_t size)
{
}

static void io_capability_req(const void *data, uint8_t size)
{
	const struct bt_lmp_io_capability_req *pdu = data;
	const char *str;

	packet_print_io_capability(pdu->capability);

	switch (pdu->oob_data) {
	case 0x00:
		str = "No authentication data received";
		break;
	case 0x01:
		str = "Authentication data received";
		break;
	default:
		str = "Reserved";
		break;
	}

	print_field("OOB data: %s (0x%2.2x)", str, pdu->oob_data);

	packet_print_io_authentication(pdu->authentication);
}

static void io_capability_res(const void *data, uint8_t size)
{
	const struct bt_lmp_io_capability_res *pdu = data;
	const char *str;

	packet_print_io_capability(pdu->capability);

	switch (pdu->oob_data) {
	case 0x00:
		str = "No authentication data received";
		break;
	case 0x01:
		str = "Authentication data received";
		break;
	default:
		str = "Reserved";
		break;
	}

	print_field("OOB data: %s (0x%2.2x)", str, pdu->oob_data);

	packet_print_io_authentication(pdu->authentication);
}

static void numeric_comparison_failed(const void *data, uint8_t size)
{
}

static void passkey_failed(const void *data, uint8_t size)
{
}

static void oob_failed(const void *data, uint8_t size)
{
}

static void power_control_req(const void *data, uint8_t size)
{
	const struct bt_lmp_power_control_req *pdu = data;
	const char *str;

	switch (pdu->request) {
	case 0x00:
		str = "Decrement power one step";
		break;
	case 0x01:
		str = "Increment power one step";
		break;
	case 0x02:
		str = "Increase to maximum power";
		break;
	default:
		str = "Reserved";
		break;
	}

	print_field("Request: %s (0x%2.2x)", str, pdu->request);
}

static void power_control_res(const void *data, uint8_t size)
{
	const struct bt_lmp_power_control_res *pdu = data;
	const char *str;

	print_field("Response: 0x%2.2x", pdu->response);

	switch (pdu->response & 0x03) {
	case 0x00:
		str = "Not supported";
		break;
	case 0x01:
		str = "Changed one step";
		break;
	case 0x02:
		str = "Max power";
		break;
	case 0x03:
		str = "Min power";
		break;
	default:
		str = "Reserved";
		break;
	}

	print_field("  GFSK: %s", str);

	switch ((pdu->response & 0x0c) >> 2) {
	case 0x00:
		str = "Not supported";
		break;
	case 0x01:
		str = "Changed one step";
		break;
	case 0x02:
		str = "Max power";
		break;
	case 0x03:
		str = "Min power";
		break;
	default:
		str = "Reserved";
		break;
	}

	print_field("  DQPSK: %s", str);

	switch ((pdu->response & 0x30) >> 4) {
	case 0x00:
		str = "Not supported";
		break;
	case 0x01:
		str = "Changed one step";
		break;
	case 0x02:
		str = "Max power";
		break;
	case 0x03:
		str = "Min power";
		break;
	default:
		str = "Reserved";
		break;
	}

	print_field("  8DPSK: %s", str);
}

static void ping_req(const void *data, uint8_t size)
{
}

static void ping_res(const void *data, uint8_t size)
{
}

struct lmp_data {
	uint16_t opcode;
	const char *str;
	void (*func) (const void *data, uint8_t size);
	uint8_t size;
	bool fixed;
};

static const struct lmp_data lmp_table[] = {
	{  1, "LMP_name_req", name_req, 1, true },
	{  2, "LMP_name_res", name_rsp, 16, true },
	{  3, "LMP_accepted", accepted, 1, true },
	{  4, "LMP_not_accepted", not_accepted, 2, true },
	{  5, "LMP_clkoffset_req", clkoffset_req, 0, true },
	{  6, "LMP_clkoffset_res", clkoffset_rsp, 2, true },
	{  7, "LMP_detach", detach, 1, true },
	{  8, "LMP_in_rand" },
	{  9, "LMP_comb_key" },
	{ 10, "LMP_unit_key" },
	{ 11, "LMP_au_rand", au_rand, 16, true },
	{ 12, "LMP_sres", sres, 4, true },
	{ 13, "LMP_temp_rand" },
	{ 14, "LMP_temp_key" },
	{ 15, "LMP_encryption_mode_req", encryption_mode_req, 1, true },
	{ 16, "LMP_encryption_key_size_req", encryption_key_size_req, 1, true },
	{ 17, "LMP_start_encryption_req", start_encryption_req, 16, true },
	{ 18, "LMP_stop_encryption_req", stop_encryption_req, 0, true },
	{ 19, "LMP_switch_req", switch_req, 4, true },
	{ 20, "LMP_hold" },
	{ 21, "LMP_hold_req" },
	{ 22, "LMP_sniff" },
	{ 23, "LMP_sniff_req" },
	{ 24, "LMP_unsniff_req", unsniff_req, 0, true },
	{ 25, "LMP_park_req" },
	{ 26, "LMP_park" },
	{ 27, "LMP_set_broadcast_scan_window" },
	{ 28, "LMP_modify_beacon" },
	{ 29, "LMP_unpark_BD_ADDR_req" },
	{ 30, "LMP_unpark_PM_ADDR_req" },
	{ 31, "LMP_incr_power_req" },
	{ 32, "LMP_decr_power_req" },
	{ 33, "LMP_max_power", max_power, 0, true },
	{ 34, "LMP_min_power", min_power, 0, true },
	{ 35, "LMP_auto_rate", auto_rate, 0, true },
	{ 36, "LMP_preferred_rate", preferred_rate, 1, true },
	{ 37, "LMP_version_req", version_req, 5, true },
	{ 38, "LMP_version_res", version_res, 5, true },
	{ 39, "LMP_features_req", features_req, 8, true },
	{ 40, "LMP_features_res", features_res, 8, true },
	{ 41, "LMP_quality_of_service" },
	{ 42, "LMP_quality_of_service_req" },
	{ 43, "LMP_SCO_link_req" },
	{ 44, "LMP_remove_SCO_link_req" },
	{ 45, "LMP_max_slot", max_slot, 1, true },
	{ 46, "LMP_max_slot_req", max_slot_req, 1, true },
	{ 47, "LMP_timing_accuracy_req", timing_accuracy_req, 0, true },
	{ 48, "LMP_timing_accuracy_res", timing_accuracy_res, 2, true },
	{ 49, "LMP_setup_complete", setup_complete, 0, true },
	{ 50, "LMP_use_semi_permanent_key", use_semi_permanent_key, 0, true },
	{ 51, "LMP_host_connection_req", host_connection_req, 0, true },
	{ 52, "LMP_slot_offset", slot_offset, 8, true },
	{ 53, "LMP_page_mode_req" },
	{ 54, "LMP_page_scan_mode_req", page_scan_mode_req, 2, true },
	{ 55, "LMP_supervision_timeout" },
	{ 56, "LMP_test_activate", test_activate, 0, true },
	{ 57, "LMP_test_control" },
	{ 58, "LMP_encryption_key_size_mask_req", encryption_key_size_mask_req, 0, true },
	{ 59, "LMP_encryption_key_size_mask_res" },
	{ 60, "LMP_set_AFH", set_afh, 15, true },
	{ 61, "LMP_encapsulated_header", encapsulated_header, 3, true },
	{ 62, "LMP_encapsulated_payload", encapsulated_payload, 16, true },
	{ 63, "LMP_simple_pairing_confirm", simple_pairing_confirm, 16, true },
	{ 64, "LMP_simple_pairing_number", simple_pairing_number, 16, true },
	{ 65, "LMP_DHkey_check", dhkey_check, 16, true },
	{ 66, "LMP_pause_encryption_aes_req" },
	{ LMP_ESC4(1),  "LMP_accepted_ext", accepted_ext, 2, true },
	{ LMP_ESC4(2),  "LMP_not_accepted_ext", not_accepted_ext, 3, true },
	{ LMP_ESC4(3),  "LMP_features_req_ext", features_req_ext, 10, true },
	{ LMP_ESC4(4),  "LMP_features_res_ext", features_res_ext, 10, true },
	{ LMP_ESC4(5),  "LMP_clk_adj" },
	{ LMP_ESC4(6),  "LMP_clk_adj_ack" },
	{ LMP_ESC4(7),  "LMP_clk_adj_req" },
	{ LMP_ESC4(11), "LMP_packet_type_table_req", packet_type_table_req, 1, true },
	{ LMP_ESC4(12), "LMP_eSCO_link_req" },
	{ LMP_ESC4(13), "LMP_remove_eSCO_link_req" },
	{ LMP_ESC4(16), "LMP_channel_classification_req", channel_classification_req, 5, true },
	{ LMP_ESC4(17), "LMP_channel_classification", channel_classification, 10, true },
	{ LMP_ESC4(21), "LMP_sniff_subrating_req" },
	{ LMP_ESC4(22), "LMP_sniff_subrating_res" },
	{ LMP_ESC4(23), "LMP_pause_encryption_req", pause_encryption_req, 0, true },
	{ LMP_ESC4(24), "LMP_resume_encryption_req", resume_encryption_req, 0, true },
	{ LMP_ESC4(25), "LMP_IO_capability_req", io_capability_req, 3, true },
	{ LMP_ESC4(26), "LMP_IO_capability_res", io_capability_res, 3, true },
	{ LMP_ESC4(27), "LMP_numeric_comparison_failed", numeric_comparison_failed, 0, true },
	{ LMP_ESC4(28), "LMP_passkey_failed", passkey_failed, 0, true },
	{ LMP_ESC4(29), "LMP_oob_failed", oob_failed, 0, true },
	{ LMP_ESC4(30), "LMP_keypress_notification" },
	{ LMP_ESC4(31), "LMP_power_control_req", power_control_req, 1, true },
	{ LMP_ESC4(32), "LMP_power_control_res", power_control_res, 1, true },
	{ LMP_ESC4(33), "LMP_ping_req", ping_req, 0, true },
	{ LMP_ESC4(34), "LMP_ping_res", ping_res, 0, true },
	{ }
};

static const char *get_opcode_str(uint16_t opcode)
{
	int i;

	for (i = 0; lmp_table[i].str; i++) {
		if (lmp_table[i].opcode == opcode)
			return lmp_table[i].str;
	}

	return NULL;
}

void lmp_packet(const void *data, uint8_t size, bool padded)
{
	const struct lmp_data *lmp_data = NULL;
	const char *opcode_color, *opcode_str;
	uint16_t opcode;
	uint8_t tid, off;
	const char *tid_str;
	int i;

	tid = ((const uint8_t *) data)[0] & 0x01;
	opcode = (((const uint8_t *) data)[0] & 0xfe) >> 1;

	tid_str = tid == 0x00 ? "Master" : "Slave";

	switch (opcode) {
	case 127:
		if (size < 2) {
			print_text(COLOR_ERROR, "extended opcode too short");
			packet_hexdump(data, size);
			return;
		}
		opcode = LMP_ESC4(((const uint8_t *) data)[1]);
		off = 2;
		break;
	case 126:
	case 125:
	case 124:
		return;
	default:
		off = 1;
		break;
	}

	for (i = 0; lmp_table[i].str; i++) {
		if (lmp_table[i].opcode == opcode) {
			lmp_data = &lmp_table[i];
			break;
		}
	}

	if (lmp_data) {
		if (lmp_data->func)
			opcode_color = COLOR_OPCODE;
		else
			opcode_color = COLOR_OPCODE_UNKNOWN;
		opcode_str = lmp_data->str;
	} else {
		opcode_color = COLOR_OPCODE_UNKNOWN;
		opcode_str = "Unknown";
	}

	if (opcode & 0xff00)
		print_indent(6, opcode_color, "", opcode_str, COLOR_OFF,
				" (%u/%u) %s transaction (%u)",
				opcode >> 8, opcode & 0xff, tid_str, tid);
	else
		print_indent(6, opcode_color, "", opcode_str, COLOR_OFF,
				" (%u) %s transaction (%d)",
				opcode, tid_str, tid);

	if (!lmp_data || !lmp_data->func) {
		packet_hexdump(data + off, size - off);
		return;
	}

	if (lmp_data->fixed && !padded) {
		if (size - off != lmp_data->size) {
			print_text(COLOR_ERROR, "invalid packet size");
			packet_hexdump(data + off, size - off);
			return;
		}
	} else {
		if (size - off < lmp_data->size) {
			print_text(COLOR_ERROR, "too short packet");
			packet_hexdump(data + off, size - off);
			return;
		}
	}

	lmp_data->func(data + off, size - off);
}

void lmp_todo(void)
{
	int i;

	printf("LMP operations with missing decodings:\n");

	for (i = 0; lmp_table[i].str; i++) {
		if (lmp_table[i].func)
			continue;

		printf("\t%s\n", lmp_table[i].str);
	}
}

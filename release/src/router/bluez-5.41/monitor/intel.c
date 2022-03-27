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

#include "lib/bluetooth.h"
#include "lib/hci.h"

#include "src/shared/util.h"
#include "display.h"
#include "packet.h"
#include "lmp.h"
#include "ll.h"
#include "vendor.h"
#include "intel.h"

#define COLOR_UNKNOWN_EVENT_MASK	COLOR_WHITE_BG
#define COLOR_UNKNOWN_SCAN_STATUS	COLOR_WHITE_BG

static void print_status(uint8_t status)
{
	packet_print_error("Status", status);
}

static void print_module(uint8_t module)
{
	const char *str;

	switch (module) {
	case 0x01:
		str = "BC";
		break;
	case 0x02:
		str = "HCI";
		break;
	case 0x03:
		str = "LLC";
		break;
	case 0x04:
		str = "OS";
		break;
	case 0x05:
		str = "LM";
		break;
	case 0x06:
		str = "SC";
		break;
	case 0x07:
		str = "SP";
		break;
	case 0x08:
		str = "OSAL";
		break;
	case 0x09:
		str = "LC";
		break;
	case 0x0a:
		str = "APP";
		break;
	case 0x0b:
		str = "TLD";
		break;
	case 0xf0:
		str = "Debug";
		break;
	default:
		str = "Reserved";
		break;
	}

	print_field("Module: %s (0x%2.2x)", str, module);
}

static void null_cmd(const void *data, uint8_t size)
{
}

static void status_rsp(const void *data, uint8_t size)
{
	uint8_t status = get_u8(data);

	print_status(status);
}

static void reset_cmd(const void *data, uint8_t size)
{
	uint8_t reset_type = get_u8(data);
	uint8_t patch_enable = get_u8(data + 1);
	uint8_t ddc_reload = get_u8(data + 2);
	uint8_t boot_option = get_u8(data + 3);
	uint32_t boot_addr = get_le32(data + 4);
	const char *str;

	switch (reset_type) {
	case 0x00:
		str = "Soft software reset";
		break;
	case 0x01:
		str = "Hard software reset";
		break;
	default:
		str = "Reserved";
		break;
	}

	print_field("Reset type: %s (0x%2.2x)", str, reset_type);

	switch (patch_enable) {
	case 0x00:
		str = "Do not enable";
		break;
	case 0x01:
		str = "Enable";
		break;
	default:
		str = "Reserved";
		break;
	}

	print_field("Patch vectors: %s (0x%2.2x)", str, patch_enable);

	switch (ddc_reload) {
	case 0x00:
		str = "Do not reload";
		break;
	case 0x01:
		str = "Reload from OTP";
		break;
	default:
		str = "Reserved";
		break;
	}

	print_field("DDC parameters: %s (0x%2.2x)", str, ddc_reload);

	switch (boot_option) {
	case 0x00:
		str = "Current image";
		break;
	case 0x01:
		str = "Specified address";
		break;
	default:
		str = "Reserved";
		break;
	}

	print_field("Boot option: %s (0x%2.2x)", str, boot_option);
	print_field("Boot address: 0x%8.8x", boot_addr);
}

static void read_version_rsp(const void *data, uint8_t size)
{
	uint8_t status = get_u8(data);
	uint8_t hw_platform = get_u8(data + 1);
	uint8_t hw_variant = get_u8(data + 2);
	uint8_t hw_revision = get_u8(data + 3);
	uint8_t fw_variant = get_u8(data + 4);
	uint8_t fw_revision = get_u8(data + 5);
	uint8_t fw_build_nn = get_u8(data + 6);
	uint8_t fw_build_cw = get_u8(data + 7);
	uint8_t fw_build_yy = get_u8(data + 8);
	uint8_t fw_patch = get_u8(data + 9);

	print_status(status);
	print_field("Hardware platform: 0x%2.2x", hw_platform);
	print_field("Hardware variant: 0x%2.2x", hw_variant);
	print_field("Hardware revision: %u.%u", hw_revision >> 4,
						hw_revision & 0x0f);
	print_field("Firmware variant: 0x%2.2x", fw_variant);
	print_field("Firmware revision: %u.%u", fw_revision >> 4,
						fw_revision & 0x0f);

	print_field("Firmware build: %u-%u.%u", fw_build_nn,
					fw_build_cw, 2000 + fw_build_yy);
	print_field("Firmware patch: %u", fw_patch);
}

static void secure_send_cmd(const void *data, uint8_t size)
{
	uint8_t type = get_u8(data);
	const char *str;

	switch (type) {
	case 0x00:
		str = "Init";
		break;
	case 0x01:
		str = "Data";
		break;
	case 0x02:
		str = "Sign";
		break;
	case 0x03:
		str = "PKey";
		break;
	default:
		str = "Reserved";
		break;
	}

	print_field("Type: %s fragment (0x%2.2x)", str, type);

	packet_hexdump(data + 1, size - 1);
}

static void manufacturer_mode_cmd(const void *data, uint8_t size)
{
	uint8_t mode = get_u8(data);
	uint8_t reset = get_u8(data + 1);
	const char *str;

	switch (mode) {
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

	print_field("Mode switch: %s (0x%2.2x)", str, mode);

	switch (reset) {
	case 0x00:
		str = "No reset";
		break;
	case 0x01:
		str = "Reset and deactivate patches";
		break;
	case 0x02:
		str = "Reset and activate patches";
		break;
	default:
		str = "Reserved";
		break;
	}

	print_field("Reset behavior: %s (0x%2.2x)", str, reset);
}

static void write_bd_data_cmd(const void *data, uint8_t size)
{
	uint8_t features[8];

	packet_print_addr("Address", data, false);
	packet_hexdump(data + 6, 6);

	memcpy(features, data + 12, 8);
	packet_print_features_lmp(features, 0);

	memcpy(features, data + 20, 1);
	memset(features + 1, 0, 7);
	packet_print_features_ll(features);

	packet_hexdump(data + 21, size - 21);
}

static void read_bd_data_rsp(const void *data, uint8_t size)
{
	uint8_t status = get_u8(data);

	print_status(status);
	packet_print_addr("Address", data + 1, false);
	packet_hexdump(data + 7, size - 7);
}

static void write_bd_address_cmd(const void *data, uint8_t size)
{
	packet_print_addr("Address", data, false);
}

static void act_deact_traces_cmd(const void *data, uint8_t size)
{
	uint8_t tx = get_u8(data);
	uint8_t tx_arq = get_u8(data + 1);
	uint8_t rx = get_u8(data + 2);

	print_field("Transmit traces: 0x%2.2x", tx);
	print_field("Transmit ARQ: 0x%2.2x", tx_arq);
	print_field("Receive traces: 0x%2.2x", rx);
}

static void stimulate_exception_cmd(const void *data, uint8_t size)
{
	uint8_t type = get_u8(data);
	const char *str;

	switch (type) {
	case 0x00:
		str = "Fatal Exception";
		break;
	case 0x01:
		str = "Debug Exception";
		break;
	default:
		str = "Reserved";
		break;
	}

	print_field("Type: %s (0x%2.2x)", str, type);
}

static const struct {
	uint8_t bit;
	const char *str;
} events_table[] = {
	{  0, "Bootup"			},
	{  1, "SCO Rejected via LMP"	},
	{  2, "PTT Switch Notification"	},
	{  7, "Scan Status"		},
	{  9, "Debug Exception"		},
	{ 10, "Fatal Exception"		},
	{ 11, "System Exception"	},
	{ 13, "LE Link Established"	},
	{ 14, "FW Trace String"		},
	{ }
};

static void set_event_mask_cmd(const void *data, uint8_t size)
{
	const uint8_t *events_array = data;
	uint64_t mask, events = 0;
	int i;

	for (i = 0; i < 8; i++)
		events |= ((uint64_t) events_array[i]) << (i * 8);

	print_field("Mask: 0x%16.16" PRIx64, events);

	mask = events;

	for (i = 0; events_table[i].str; i++) {
		if (events & (((uint64_t) 1) << events_table[i].bit)) {
			print_field("  %s", events_table[i].str);
			mask &= ~(((uint64_t) 1) << events_table[i].bit);
		}
	}

	if (mask)
		print_text(COLOR_UNKNOWN_EVENT_MASK, "  Unknown mask "
						"(0x%16.16" PRIx64 ")", mask);
}

static void ddc_config_write_cmd(const void *data, uint8_t size)
{
	while (size > 0) {
		uint8_t param_len = get_u8(data);
		uint16_t param_id = get_le16(data + 1);

		print_field("Identifier: 0x%4.4x", param_id);
		packet_hexdump(data + 2, param_len - 2);

		data += param_len + 1;
		size -= param_len + 1;
	}
}

static void ddc_config_write_rsp(const void *data, uint8_t size)
{
	uint8_t status = get_u8(data);
	uint16_t param_id = get_le16(data + 1);

	print_status(status);
	print_field("Identifier: 0x%4.4x", param_id);
}

static void memory_write_cmd(const void *data, uint8_t size)
{
	uint32_t addr = get_le32(data);
	uint8_t mode = get_u8(data + 4);
	uint8_t length = get_u8(data + 5);
	const char *str;

	print_field("Address: 0x%8.8x", addr);

	switch (mode) {
	case 0x00:
		str = "Byte access";
		break;
	case 0x01:
		str = "Half word access";
		break;
	case 0x02:
		str = "Word access";
		break;
	default:
		str = "Reserved";
		break;
	}

	print_field("Mode: %s (0x%2.2x)", str, mode);
	print_field("Length: %u", length);

	packet_hexdump(data + 6, size - 6);
}

static const struct vendor_ocf vendor_ocf_table[] = {
	{ 0x001, "Reset",
			reset_cmd, 8, true,
			status_rsp, 1, true },
	{ 0x002, "No Operation" },
	{ 0x005, "Read Version",
			null_cmd, 0, true,
			read_version_rsp, 10, true },
	{ 0x006, "Set UART Baudrate" },
	{ 0x007, "Enable LPM" },
	{ 0x008, "PCM Write Configuration" },
	{ 0x009, "Secure Send",
			secure_send_cmd, 1, false,
			status_rsp, 1, true },
	{ 0x00d, "Read Secure Boot Params",
			null_cmd, 0, true },
	{ 0x00e, "Write Secure Boot Params" },
	{ 0x00f, "Unlock" },
	{ 0x010, "Change UART Baudrate" },
	{ 0x011, "Manufacturer Mode",
			manufacturer_mode_cmd, 2, true,
			status_rsp, 1, true },
	{ 0x012, "Read Link RSSI" },
	{ 0x022, "Get Exception Info" },
	{ 0x024, "Clear Exception Info" },
	{ 0x02f, "Write BD Data",
			write_bd_data_cmd, 6, false },
	{ 0x030, "Read BD Data",
			null_cmd, 0, true,
			read_bd_data_rsp, 7, false },
	{ 0x031, "Write BD Address",
			write_bd_address_cmd, 6, true,
			status_rsp, 1, true },
	{ 0x032, "Flow Specification" },
	{ 0x034, "Read Secure ID" },
	{ 0x038, "Set Synchronous USB Interface Type" },
	{ 0x039, "Config Synchronous Interface" },
	{ 0x03f, "SW RF Kill",
			null_cmd, 0, true,
			status_rsp, 1, true },
	{ 0x043, "Activate Deactivate Traces",
			act_deact_traces_cmd, 3, true },
	{ 0x04d, "Stimulate Exception",
			stimulate_exception_cmd, 1, true,
			status_rsp, 1, true },
	{ 0x050, "Read HW Version" },
	{ 0x052, "Set Event Mask",
			set_event_mask_cmd, 8, true,
			status_rsp, 1, true },
	{ 0x053, "Config_Link_Controller" },
	{ 0x089, "DDC Write" },
	{ 0x08a, "DDC Read" },
	{ 0x08b, "DDC Config Write",
			ddc_config_write_cmd, 3, false,
			ddc_config_write_rsp, 3, true },
	{ 0x08c, "DDC Config Read" },
	{ 0x08d, "Memory Read" },
	{ 0x08e, "Memory Write",
			memory_write_cmd, 6, false,
			status_rsp, 1, true },
	{ }
};

const struct vendor_ocf *intel_vendor_ocf(uint16_t ocf)
{
	int i;

	for (i = 0; vendor_ocf_table[i].str; i++) {
		if (vendor_ocf_table[i].ocf == ocf)
			return &vendor_ocf_table[i];
	}

	return NULL;
}

static void startup_evt(const void *data, uint8_t size)
{
}

static void fatal_exception_evt(const void *data, uint8_t size)
{
	uint16_t line = get_le16(data);
	uint8_t module = get_u8(data + 2);
	uint8_t reason = get_u8(data + 3);

	print_field("Line: %u", line);
	print_module(module);
	print_field("Reason: 0x%2.2x", reason);
}

static void bootup_evt(const void *data, uint8_t size)
{
	uint8_t zero = get_u8(data);
	uint8_t num_packets = get_u8(data + 1);
	uint8_t source = get_u8(data + 2);
	uint8_t reset_type = get_u8(data + 3);
	uint8_t reset_reason = get_u8(data + 4);
	uint8_t ddc_status = get_u8(data + 5);
	const char *str;

	print_field("Zero: 0x%2.2x", zero);
	print_field("Number of packets: %d", num_packets);

	switch (source) {
	case 0x00:
		str = "Bootloader";
		break;
	case 0x01:
		str = "Operational firmware";
		break;
	case 0x02:
		str = "Self test firmware";
		break;
	default:
		str = "Reserved";
		break;
	}

	print_field("Source: %s (0x%2.2x)", str, source);

	switch (reset_type) {
	case 0x00:
		str = "Hardware reset";
		break;
	case 0x01:
		str = "Soft watchdog reset";
		break;
	case 0x02:
		str = "Soft software reset";
		break;
	case 0x03:
		str = "Hard watchdog reset";
		break;
	case 0x04:
		str = "Hard software reset";
		break;
	default:
		str = "Reserved";
		break;
	}

	print_field("Reset type: %s (0x%2.2x)", str, reset_type);

	switch (reset_reason) {
	case 0x00:
		str = "Power on";
		break;
	case 0x01:
		str = "Reset command";
		break;
	case 0x02:
		str = "Intel reset command";
		break;
	case 0x03:
		str = "Watchdog";
		break;
	case 0x04:
		str = "Fatal exception";
		break;
	case 0x05:
		str = "System exception";
		break;
	case 0xff:
		str = "Unknown";
		break;
	default:
		str = "Reserved";
		break;
	}

	print_field("Reset reason: %s (0x%2.2x)", str, reset_reason);

	switch (ddc_status) {
	case 0x00:
		str = "Firmware default";
		break;
	case 0x01:
		str = "Firmware default plus OTP";
		break;
	case 0x02:
		str = "Persistent RAM";
		break;
	case 0x03:
		str = "Not used";
		break;
	default:
		str = "Reserved";
		break;
	}

	print_field("DDC status: %s (0x%2.2x)", str, ddc_status);
}

static void default_bd_data_evt(const void *data, uint8_t size)
{
	uint8_t mem_status = get_u8(data);
	const char *str;

	switch (mem_status) {
	case 0x02:
		str = "Invalid manufacturing data";
		break;
	default:
		str = "Reserved";
		break;
	}

	print_field("Memory status: %s (0x%2.2x)", str, mem_status);
}

static void secure_send_commands_result_evt(const void *data, uint8_t size)
{
	uint8_t result = get_u8(data);
	uint16_t opcode = get_le16(data + 1);
	uint16_t ogf = cmd_opcode_ogf(opcode);
	uint16_t ocf = cmd_opcode_ocf(opcode);
	uint8_t status = get_u8(data + 3);
	const char *str;

	switch (result) {
	case 0x00:
		str = "Success";
		break;
	case 0x01:
		str = "General failure";
		break;
	case 0x02:
		str = "Hardware failure";
		break;
	case 0x03:
		str = "Signature verification failed";
		break;
	case 0x04:
		str = "Parsing error of command buffer";
		break;
	case 0x05:
		str = "Command execution failure";
		break;
	case 0x06:
		str = "Command parameters error";
		break;
	case 0x07:
		str = "Command missing";
		break;
	default:
		str = "Reserved";
		break;
	}

	print_field("Result: %s (0x%2.2x)", str, result);
	print_field("Opcode: 0x%4.4x (0x%2.2x|0x%4.4x)", opcode, ogf, ocf);
	print_status(status);
}

static void debug_exception_evt(const void *data, uint8_t size)
{
	uint16_t line = get_le16(data);
	uint8_t module = get_u8(data + 2);
	uint8_t reason = get_u8(data + 3);

	print_field("Line: %u", line);
	print_module(module);
	print_field("Reason: 0x%2.2x", reason);
}

static void le_link_established_evt(const void *data, uint8_t size)
{
	uint16_t handle = get_le16(data);
	uint32_t access_addr = get_le32(data + 10);

	print_field("Handle: %u", handle);

	packet_hexdump(data + 2, 8);

	print_field("Access address: 0x%8.8x", access_addr);

	packet_hexdump(data + 14, size - 14);
}

static void scan_status_evt(const void *data, uint8_t size)
{
	uint8_t enable = get_u8(data);

	print_field("Inquiry scan: %s",
				(enable & 0x01) ? "Enabled" : "Disabled");
	print_field("Page scan: %s",
				(enable & 0x02) ? "Enabled" : "Disabled");

	if (enable & 0xfc)
		print_text(COLOR_UNKNOWN_SCAN_STATUS,
				"  Unknown status (0x%2.2x)", enable & 0xfc);

}

static void act_deact_traces_complete_evt(const void *data, uint8_t size)
{
	uint8_t status = get_u8(data);

	print_status(status);
}

static void lmp_pdu_trace_evt(const void *data, uint8_t size)
{
	uint8_t type, len, id;
	uint16_t handle, count;
	uint32_t clock;
	const char *str;

	type = get_u8(data);
	handle = get_le16(data + 1);

	switch (type) {
	case 0x00:
		str = "RX LMP";
		break;
	case 0x01:
		str = "TX LMP";
		break;
	case 0x02:
		str = "ACK LMP";
		break;
	case 0x03:
		str = "RX LL";
		break;
	case 0x04:
		str = "TX LL";
		break;
	case 0x05:
		str = "ACK LL";
		break;
	default:
		str = "Unknown";
		break;
	}

	print_field("Type: %s (0x%2.2x)", str, type);
	print_field("Handle: %u", handle);

	switch (type) {
	case 0x00:
		len = size - 8;
		clock = get_le32(data + 4 + len);

		packet_hexdump(data + 3, 1);
		lmp_packet(data + 4, len, false);
		print_field("Clock: 0x%8.8x", clock);
		break;
	case 0x01:
		len = size - 9;
		clock = get_le32(data + 4 + len);
		id = get_u8(data + 4 + len + 4);

		packet_hexdump(data + 3, 1);
		lmp_packet(data + 4, len, false);
		print_field("Clock: 0x%8.8x", clock);
		print_field("ID: 0x%2.2x", id);
		break;
	case 0x02:
		clock = get_le32(data + 3);
		id = get_u8(data + 3 + 4);

		print_field("Clock: 0x%8.8x", clock);
		print_field("ID: 0x%2.2x", id);
		break;
	case 0x03:
		len = size - 8;
		count = get_le16(data + 3);

		print_field("Count: 0x%4.4x", count);
		packet_hexdump(data + 3 + 2 + 1, 2);
		llcp_packet(data + 8, len, false);
		break;
	case 0x04:
		len = size - 8;
		count = get_le16(data + 3);
		id = get_u8(data + 3 + 2);

		print_field("Count: 0x%4.4x", count);
		print_field("ID: 0x%2.2x", id);
		packet_hexdump(data + 3 + 2 + 1, 2);
		llcp_packet(data + 8, len, false);
		break;
	case 0x05:
		count = get_le16(data + 3);
		id = get_u8(data + 3 + 2);

		print_field("Count: 0x%4.4x", count);
		print_field("ID: 0x%2.2x", id);
		break;
	default:
		packet_hexdump(data + 3, size - 3);
		break;
	}
}

static void write_bd_data_complete_evt(const void *data, uint8_t size)
{
	uint8_t status = get_u8(data);

	print_status(status);
}

static void sco_rejected_via_lmp_evt(const void *data, uint8_t size)
{
	uint8_t reason = get_u8(data + 6);

	packet_print_addr("Address", data, false);
	packet_print_error("Reason", reason);
}

static void ptt_switch_notification_evt(const void *data, uint8_t size)
{
	uint16_t handle = get_le16(data);
	uint8_t table = get_u8(data + 2);
	const char *str;

	print_field("Handle: %u", handle);

	switch (table) {
	case 0x00:
		str = "Basic rate";
		break;
	case 0x01:
		str = "Enhanced data rate";
		break;
	default:
		str = "Reserved";
		break;
	}

	print_field("Packet type table: %s (0x%2.2x)", str, table);
}

static void system_exception_evt(const void *data, uint8_t size)
{
	uint8_t type = get_u8(data);
	const char *str;

	switch (type) {
	case 0x00:
		str = "No Exception";
		break;
	case 0x01:
		str = "Undefined Instruction";
		break;
	case 0x02:
		str = "Prefetch abort";
		break;
	case 0x03:
		str = "Data abort";
		break;
	default:
		str = "Reserved";
		break;
	}

	print_field("Type: %s (0x%2.2x)", str, type);

	packet_hexdump(data + 1, size - 1);
}

static const struct vendor_evt vendor_evt_table[] = {
	{ 0x00, "Startup",
			startup_evt, 0, true },
	{ 0x01, "Fatal Exception",
			fatal_exception_evt, 4, true },
	{ 0x02, "Bootup",
			bootup_evt, 6, true },
	{ 0x05, "Default BD Data",
			default_bd_data_evt, 1, true },
	{ 0x06, "Secure Send Commands Result",
			secure_send_commands_result_evt, 4, true },
	{ 0x08, "Debug Exception",
			debug_exception_evt, 4, true },
	{ 0x0f, "LE Link Established",
			le_link_established_evt, 26, true },
	{ 0x11, "Scan Status",
			scan_status_evt, 1, true },
	{ 0x16, "Activate Deactivate Traces Complete",
			act_deact_traces_complete_evt, 1, true },
	{ 0x17, "LMP PDU Trace",
			lmp_pdu_trace_evt, 3, false },
	{ 0x19, "Write BD Data Complete",
			write_bd_data_complete_evt, 1, true },
	{ 0x25, "SCO Rejected via LMP",
			sco_rejected_via_lmp_evt, 7, true },
	{ 0x26, "PTT Switch Notification",
			ptt_switch_notification_evt, 3, true },
	{ 0x29, "System Exception",
			system_exception_evt, 133, true },
	{ 0x2c, "FW Trace String" },
	{ 0x2e, "FW Trace Binary" },
	{ }
};

const struct vendor_evt *intel_vendor_evt(uint8_t evt)
{
	int i;

	for (i = 0; vendor_evt_table[i].str; i++) {
		if (vendor_evt_table[i].evt == evt)
			return &vendor_evt_table[i];
	}

	return NULL;
}

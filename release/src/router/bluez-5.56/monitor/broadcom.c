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
#include "lmp.h"
#include "ll.h"
#include "vendor.h"
#include "broadcom.h"

#define COLOR_UNKNOWN_FEATURE_BIT	COLOR_WHITE_BG

static void print_status(uint8_t status)
{
	packet_print_error("Status", status);
}

static void print_handle(uint16_t handle)
{
	packet_print_handle(handle);
}

static void print_rssi(int8_t rssi)
{
	packet_print_rssi(rssi);
}

static void print_sco_routing(uint8_t routing)
{
	const char *str;

	switch (routing) {
	case 0x00:
		str = "PCM";
		break;
	case 0x01:
		str = "Transport";
		break;
	case 0x02:
		str = "Codec";
		break;
	case 0x03:
		str = "I2S";
		break;
	default:
		str = "Reserved";
		break;
	}

	print_field("SCO routing: %s (0x%2.2x)", str, routing);
}

static void print_pcm_interface_rate(uint8_t rate)
{
	const char *str;

	switch (rate) {
	case 0x00:
		str = "128 KBps";
		break;
	case 0x01:
		str = "256 KBps";
		break;
	case 0x02:
		str = "512 KBps";
		break;
	case 0x03:
		str = "1024 KBps";
		break;
	case 0x04:
		str = "2048 KBps";
		break;
	default:
		str = "Reserved";
		break;
	}

	print_field("PCM interface rate: %s (0x%2.2x)", str, rate);
}

static void print_frame_type(uint8_t type)
{
	const char *str;

	switch (type) {
	case 0x00:
		str = "Short";
		break;
	case 0x01:
		str = "Long";
		break;
	default:
		str = "Reserved";
		break;
	}

	print_field("Frame type: %s (0x%2.2x)", str, type);
}

static void print_sync_mode(uint8_t mode)
{
	const char *str;

	switch (mode) {
	case 0x00:
		str = "Slave";
		break;
	case 0x01:
		str = "Master";
		break;
	default:
		str = "Reserved";
		break;
	}

	print_field("Sync mode: %s (0x%2.2x)", str, mode);
}

static void print_clock_mode(uint8_t mode)
{
	const char *str;

	switch (mode) {
	case 0x00:
		str = "Slave";
		break;
	case 0x01:
		str = "Master";
		break;
	default:
		str = "Reserved";
		break;
	}

	print_field("Clock mode: %s (0x%2.2x)", str, mode);
}

static void print_sleep_mode(uint8_t mode)
{
	const char *str;

	switch (mode) {
	case 0x00:
		str = "No sleep mode";
		break;
	case 0x01:
		str = "UART";
		break;
	case 0x02:
		str = "UART with messaging";
		break;
	case 0x03:
		str = "USB";
		break;
	case 0x04:
		str = "H4IBSS";
		break;
	case 0x05:
		str = "USB with Host wake";
		break;
	case 0x06:
		str = "SDIO";
		break;
	case 0x07:
		str = "UART CS-N";
		break;
	case 0x08:
		str = "SPI";
		break;
	case 0x09:
		str = "H5";
		break;
	case 0x0a:
		str = "H4DS";
		break;
	case 0x0c:
		str = "UART with BREAK";
		break;
	default:
		str = "Reserved";
		break;
	}

	print_field("Sleep mode: %s (0x%2.2x)", str, mode);
}

static void print_clock_setting(uint8_t clock)
{
	const char *str;

	switch (clock) {
	case 0x01:
		str = "48 Mhz";
		break;
	case 0x02:
		str = "24 Mhz";
		break;
	default:
		str = "Reserved";
		break;
	}

	print_field("UART clock: %s (0x%2.2x)", str, clock);
}

static void null_cmd(const void *data, uint8_t size)
{
}

static void status_rsp(const void *data, uint8_t size)
{
	uint8_t status = get_u8(data);

	print_status(status);
}

static void write_bd_addr_cmd(const void *data, uint8_t size)
{
	packet_print_addr("Address", data, false);
}

static void update_uart_baud_rate_cmd(const void *data, uint8_t size)
{
	uint16_t enc_rate = get_le16(data);
	uint32_t exp_rate = get_le32(data + 2);

	if (enc_rate == 0x0000)
		print_field("Encoded baud rate: Not used (0x0000)");
	else
		print_field("Encoded baud rate: 0x%4.4x", enc_rate);

	print_field("Explicit baud rate: %u Mbps", exp_rate);
}

static void write_sco_pcm_int_param_cmd(const void *data, uint8_t size)
{
	uint8_t routing = get_u8(data);
	uint8_t rate = get_u8(data + 1);
	uint8_t frame_type = get_u8(data + 2);
	uint8_t sync_mode = get_u8(data + 3);
	uint8_t clock_mode = get_u8(data + 4);

	print_sco_routing(routing);
	print_pcm_interface_rate(rate);
	print_frame_type(frame_type);
	print_sync_mode(sync_mode);
	print_clock_mode(clock_mode);
}

static void read_sco_pcm_int_param_rsp(const void *data, uint8_t size)
{
	uint8_t status = get_u8(data);
	uint8_t routing = get_u8(data + 1);
	uint8_t rate = get_u8(data + 2);
	uint8_t frame_type = get_u8(data + 3);
	uint8_t sync_mode = get_u8(data + 4);
	uint8_t clock_mode = get_u8(data + 5);

	print_status(status);
	print_sco_routing(routing);
	print_pcm_interface_rate(rate);
	print_frame_type(frame_type);
	print_sync_mode(sync_mode);
	print_clock_mode(clock_mode);
}

static void set_sleepmode_param_cmd(const void *data, uint8_t size)
{
	uint8_t mode = get_u8(data);

	print_sleep_mode(mode);

	packet_hexdump(data + 1, size - 1);
}

static void read_sleepmode_param_rsp(const void *data, uint8_t size)
{
	uint8_t status = get_u8(data);
	uint8_t mode = get_u8(data + 1);

	print_status(status);
	print_sleep_mode(mode);

	packet_hexdump(data + 2, size - 2);
}

static void enable_radio_cmd(const void *data, uint8_t size)
{
	uint8_t mode = get_u8(data);
	const char *str;

	switch (mode) {
	case 0x00:
		str = "Disable the radio";
		break;
	case 0x01:
		str = "Enable the radio";
		break;
	default:
		str = "Reserved";
		break;
	}

	print_field("Mode: %s (0x%2.2x)", str, mode);
}

static void enable_usb_hid_emulation_cmd(const void *data, uint8_t size)
{
	uint8_t enable = get_u8(data);
	const char *str;

	switch (enable) {
	case 0x00:
		str = "Bluetooth mode";
		break;
	case 0x01:
		str = "HID Mode";
		break;
	default:
		str = "Reserved";
		break;
	}

	print_field("Enable: %s (0x%2.2x)", str, enable);
}

static void read_uart_clock_setting_rsp(const void *data, uint8_t size)
{
	uint8_t status = get_u8(data);
	uint8_t clock = get_u8(data + 1);

	print_status(status);
	print_clock_setting(clock);
}

static void write_uart_clock_setting_cmd(const void *data, uint8_t size)
{
	uint8_t clock = get_u8(data);

	print_clock_setting(clock);
}

static void read_raw_rssi_cmd(const void *data, uint8_t size)
{
	uint16_t handle = get_le16(data);

	print_handle(handle);
}

static void read_raw_rssi_rsp(const void *data, uint8_t size)
{
	uint8_t status = get_u8(data);
	uint16_t handle = get_le16(data + 1);
	int8_t rssi = get_s8(data + 3);

	print_status(status);
	print_handle(handle);
	print_rssi(rssi);
}

static void write_ram_cmd(const void *data, uint8_t size)
{
	uint32_t addr = get_le32(data);

	print_field("Address: 0x%8.8x", addr);

	packet_hexdump(data + 4, size - 4);
}

static void read_ram_cmd(const void *data, uint8_t size)
{
	uint32_t addr = get_le32(data);
	uint8_t length = get_u8(data + 4);

	print_field("Address: 0x%8.8x", addr);
	print_field("Length: %u", length);
}

static void read_ram_rsp(const void *data, uint8_t size)
{
	uint8_t status = get_u8(data);

	print_status(status);

	packet_hexdump(data + 1, size - 1);
}

static void launch_ram_cmd(const void *data, uint8_t size)
{
	uint32_t addr = get_le32(data);

	print_field("Address: 0x%8.8x", addr);
}

static void read_vid_pid_rsp(const void *data, uint8_t size)
{
	uint8_t status = get_u8(data);
	uint16_t vid = get_le16(data + 1);
	uint16_t pid = get_le16(data + 3);

	print_status(status);
	print_field("Product: %4.4x:%4.4x", vid, pid);
}

static void write_high_priority_connection_cmd(const void *data, uint8_t size)
{
	uint16_t handle = get_le16(data);
	uint8_t priority = get_u8(data + 2);
	const char *str;

	print_handle(handle);

	switch (priority) {
	case 0x00:
		str = "Low";
		break;
	case 0x01:
		str = "High";
		break;
	default:
		str = "Reserved";
		break;
	}

	print_field("Priority: %s (0x%2.2x)", str, priority);
}

static const struct {
	uint8_t bit;
	const char *str;
} features_table[] = {
	{  0, "Multi-AV transport bandwidth reducer"	},
	{  1, "WBS SBC"					},
	{  2, "FW LC-PLC"				},
	{  3, "FM SBC internal stack"			},
	{ }
};

static void print_features(const uint8_t *features_array)
{
	uint64_t mask, features = 0;
	char str[41];
	int i;

	for (i = 0; i < 8; i++) {
		sprintf(str + (i * 5), " 0x%2.2x", features_array[i]);
		features |= ((uint64_t) features_array[i]) << (i * 8);
	}

	print_field("Features:%s", str);

	mask = features;

	for (i = 0; features_table[i].str; i++) {
		if (features & (((uint64_t) 1) << features_table[i].bit)) {
			print_field("  %s", features_table[i].str);
			mask &= ~(((uint64_t) 1) << features_table[i].bit);
		}
	}

	if (mask)
		print_text(COLOR_UNKNOWN_FEATURE_BIT, "  Unknown features "
						"(0x%16.16" PRIx64 ")", mask);
}

static void read_controller_features_rsp(const void *data, uint8_t size)
{
	uint8_t status = get_u8(data);

	print_status(status);
	print_features(data + 1);
}

static void read_verbose_version_info_rsp(const void *data, uint8_t size)
{
	uint8_t status = get_u8(data);
	uint8_t chip_id = get_u8(data + 1);
	uint8_t target_id = get_u8(data + 2);
	uint16_t build_base = get_le16(data + 3);
	uint16_t build_num = get_le16(data + 5);
	const char *str;

	print_status(status);
	print_field("Chip ID: %u (0x%2.2x)", chip_id, chip_id);

	switch (target_id) {
	case 254:
		str = "Invalid";
		break;
	case 255:
		str = "Undefined";
		break;
	default:
		str = "Reserved";
		break;
	}

	print_field("Build target: %s (%u)", str, target_id);
	print_field("Build baseline: %u (0x%4.4x)", build_base, build_base);
	print_field("Build number: %u (0x%4.4x)", build_num, build_num);
}

static void enable_wbs_cmd(const void *data, uint8_t size)
{
	uint8_t mode = get_u8(data);
	uint16_t codec = get_le16(data + 1);
	const char *str;

	switch (mode) {
	case 0x00:
		str = "Disable WBS";
		break;
	case 0x01:
		str = "Enable WBS";
		break;
	default:
		str = "Reserved";
		break;
	}

	print_field("Mode: %s (0x%2.2x)", str, mode);

	switch (codec) {
	case 0x0000:
		str = "None";
		break;
	case 0x0001:
		str = "CVSD";
		break;
	case 0x0002:
		str = "mSBC";
		break;
	default:
		str = "Reserved";
		break;
	}

	print_field("Codec: %s (0x%4.4x)", str, codec);
}

static const struct vendor_ocf vendor_ocf_table[] = {
	{ 0x001, "Write BD ADDR",
			write_bd_addr_cmd, 6, true,
			status_rsp, 1, true },
	{ 0x018, "Update UART Baud Rate",
			update_uart_baud_rate_cmd, 6, true,
			status_rsp, 1, true },
	{ 0x01c, "Write SCO PCM Int Param",
			write_sco_pcm_int_param_cmd, 5, true,
			status_rsp, 1, true },
	{ 0x01d, "Read SCO PCM Int Param",
			null_cmd, 0, true,
			read_sco_pcm_int_param_rsp, 6, true },
	{ 0x027, "Set Sleepmode Param",
			set_sleepmode_param_cmd, 12, true,
			status_rsp, 1, true },
	{ 0x028, "Read Sleepmode Param",
			null_cmd, 0, true,
			read_sleepmode_param_rsp, 13, true },
	{ 0x02e, "Download Minidriver",
			null_cmd, 0, true,
			status_rsp, 1, true },
	{ 0x034, "Enable Radio",
			enable_radio_cmd, 1, true,
			status_rsp, 1, true },
	{ 0x03b, "Enable USB HID Emulation",
			enable_usb_hid_emulation_cmd, 1, true,
			status_rsp, 1, true },
	{ 0x044, "Read UART Clock Setting",
			null_cmd, 0, true,
			read_uart_clock_setting_rsp, 1, true },
	{ 0x045, "Write UART Clock Setting",
			write_uart_clock_setting_cmd, 1, true,
			status_rsp, 1, true },
	{ 0x048, "Read Raw RSSI",
			read_raw_rssi_cmd, 2, true,
			read_raw_rssi_rsp, 4, true },
	{ 0x04c, "Write RAM",
			write_ram_cmd, 4, false,
			status_rsp, 1, true },
	{ 0x04d, "Read RAM",
			read_ram_cmd, 5, true,
			read_ram_rsp, 1, false },
	{ 0x04e, "Launch RAM",
			launch_ram_cmd, 4, true,
			status_rsp, 1, true },
	{ 0x05a, "Read VID PID",
			null_cmd, 0, true,
			read_vid_pid_rsp, 5, true },
	{ 0x057, "Write High Priority Connection",
			write_high_priority_connection_cmd, 3, true,
			status_rsp, 1, true },
	{ 0x06d, "Write I2SPCM Interface Param" },
	{ 0x06e, "Read Controller Features",
			null_cmd, 0, true,
			read_controller_features_rsp, 9, true },
	{ 0x079, "Read Verbose Config Version Info",
			null_cmd, 0, true,
			read_verbose_version_info_rsp, 7, true },
	{ 0x07e, "Enable WBS",
			enable_wbs_cmd, 3, true,
			status_rsp, 1, true },
	{ }
};

const struct vendor_ocf *broadcom_vendor_ocf(uint16_t ocf)
{
	int i;

	for (i = 0; vendor_ocf_table[i].str; i++) {
		if (vendor_ocf_table[i].ocf == ocf)
			return &vendor_ocf_table[i];
	}

	return NULL;
}

void broadcom_lm_diag(const void *data, uint8_t size)
{
	uint8_t type;
	uint32_t clock;
	const uint8_t *addr;
	const char *str;

	if (size != 63) {
		packet_hexdump(data, size);
		return;
	}

	type = *((uint8_t *) data);
	clock = get_be32(data + 1);

	switch (type) {
	case 0x00:
		str = "LMP sent";
		break;
	case 0x01:
		str = "LMP receive";
		break;
	case 0x80:
		str = "LL sent";
		break;
	case 0x81:
		str = "LL receive";
		break;
	default:
		str = "Unknown";
		break;
	}

	print_field("Type: %s (%u)", str, type);
	print_field("Clock: 0x%8.8x", clock);

	switch (type) {
	case 0x00:
		addr = data + 5;
		print_field("Address: --:--:%2.2X:%2.2X:%2.2X:%2.2X",
					addr[0], addr[1], addr[2], addr[3]);
		packet_hexdump(data + 9, 1);
		lmp_packet(data + 10, size - 10, true);
		break;
	case 0x01:
		addr = data + 5;
		print_field("Address: --:--:%2.2X:%2.2X:%2.2X:%2.2X",
					addr[0], addr[1], addr[2], addr[3]);
		packet_hexdump(data + 9, 4);
		lmp_packet(data + 13, size - 13, true);
		break;
	case 0x80:
	case 0x81:
		packet_hexdump(data + 5, 7);
		llcp_packet(data + 12, size - 12, true);
		break;
	default:
		packet_hexdump(data + 9, size - 9);
		break;
	}
}

static const struct vendor_evt vendor_evt_table[] = {
	{ }
};

const struct vendor_evt *broadcom_vendor_evt(uint8_t evt)
{
	int i;

	for (i = 0; vendor_evt_table[i].str; i++) {
		if (vendor_evt_table[i].evt == evt)
			return &vendor_evt_table[i];
	}

	return NULL;
}

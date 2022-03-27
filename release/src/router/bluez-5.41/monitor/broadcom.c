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

#include "src/shared/util.h"
#include "display.h"
#include "packet.h"
#include "lmp.h"
#include "ll.h"
#include "vendor.h"
#include "broadcom.h"

static void print_status(uint8_t status)
{
	packet_print_error("Status", status);
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

static void write_ram_cmd(const void *data, uint8_t size)
{
	uint32_t addr = get_le32(data);

	print_field("Address: 0x%8.8x", addr);

	packet_hexdump(data + 4, size - 4);
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

static const struct vendor_ocf vendor_ocf_table[] = {
	{ 0x001, "Write BD ADDR",
			write_bd_addr_cmd, 6, true,
			status_rsp, 1, true },
	{ 0x018, "Update UART Baud Rate" },
	{ 0x027, "Set Sleepmode Param" },
	{ 0x02e, "Download Minidriver",
			null_cmd, 0, true,
			status_rsp, 1, true },
	{ 0x03b, "Enable USB HID Emulation",
			enable_usb_hid_emulation_cmd, 1, true,
			status_rsp, 1, true },
	{ 0x045, "Write UART Clock Setting" },
	{ 0x04c, "Write RAM",
			write_ram_cmd, 4, false,
			status_rsp, 1, true },
	{ 0x04e, "Launch RAM",
			launch_ram_cmd, 4, true,
			status_rsp, 1, true },
	{ 0x05a, "Read VID PID",
			null_cmd, 0, true,
			read_vid_pid_rsp, 5, true },
	{ 0x079, "Read Verbose Config Version Info",
			null_cmd, 0, true,
			read_verbose_version_info_rsp, 7, true },
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

/*
 *
 *  BlueZ - Bluetooth protocol stack for Linux
 *
 *  Copyright (C) 2012  Intel Corporation. All rights reserved.
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

#include <ctype.h>
#include <stdio.h>
#include <fcntl.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <getopt.h>
#include <sys/stat.h>
#include <sys/param.h>

#include "monitor/bt.h"
#include "src/shared/mainloop.h"
#include "src/shared/util.h"
#include "src/shared/hci.h"

#define CMD_RESET		0xfc01
struct cmd_reset {
	uint8_t  reset_type;
	uint8_t  patch_enable;
	uint8_t  otp_ddc_reload;
	uint8_t  boot_option;
	uint32_t boot_addr;
} __attribute__ ((packed));

#define CMD_NO_OPERATION	0xfc02

#define CMD_READ_VERSION	0xfc05
struct rsp_read_version {
	uint8_t  status;
	uint8_t  hw_platform;
	uint8_t  hw_variant;
	uint8_t  hw_revision;
	uint8_t  fw_variant;
	uint8_t  fw_revision;
	uint8_t  fw_build_nn;
	uint8_t  fw_build_cw;
	uint8_t  fw_build_yy;
	uint8_t  fw_patch;
} __attribute__ ((packed));

#define CMD_READ_BOOT_PARAMS	0xfc0d
struct rsp_read_boot_params {
	uint8_t  status;
	uint8_t  otp_format;
	uint8_t  otp_content;
	uint8_t  otp_patch;
	uint16_t dev_revid;
	uint8_t  secure_boot;
	uint8_t  key_from_hdr;
	uint8_t  key_type;
	uint8_t  otp_lock;
	uint8_t  api_lock;
	uint8_t  debug_lock;
	uint8_t  otp_bdaddr[6];
	uint8_t  min_fw_build_nn;
	uint8_t  min_fw_build_cw;
	uint8_t  min_fw_build_yy;
	uint8_t  limited_cce;
	uint8_t  unlocked_state;
} __attribute__ ((packed));

#define CMD_WRITE_BOOT_PARAMS	0xfc0e
struct cmd_write_boot_params {
	uint32_t boot_addr;
	uint8_t  fw_build_nn;
	uint8_t  fw_build_cw;
	uint8_t  fw_build_yy;
} __attribute__ ((packed));

#define CMD_MANUFACTURER_MODE	0xfc11
struct cmd_manufacturer_mode {
	uint8_t  mode_switch;
	uint8_t  reset;
} __attribute__ ((packed));

#define CMD_WRITE_BD_DATA	0xfc2f
struct cmd_write_bd_data {
	uint8_t  bdaddr[6];
	uint8_t  reserved1[6];
	uint8_t  features[8];
	uint8_t  le_features;
	uint8_t  reserved2[32];
	uint8_t  lmp_version;
	uint8_t  reserved3[26];
} __attribute__ ((packed));

#define CMD_READ_BD_DATA	0xfc30
struct rsp_read_bd_data {
	uint8_t  status;
	uint8_t  bdaddr[6];
	uint8_t  reserved1[6];
	uint8_t  features[8];
	uint8_t  le_features;
	uint8_t  reserved2[32];
	uint8_t  lmp_version;
	uint8_t  reserved3[26];
} __attribute__ ((packed));

#define CMD_WRITE_BD_ADDRESS	0xfc31
struct cmd_write_bd_address {
	uint8_t  bdaddr[6];
} __attribute__ ((packed));

#define CMD_ACT_DEACT_TRACES	0xfc43
struct cmd_act_deact_traces {
	uint8_t  tx_trace;
	uint8_t  tx_arq;
	uint8_t  rx_trace;
} __attribute__ ((packed));

#define CMD_TRIGGER_EXCEPTION	0xfc4d
struct cmd_trigger_exception {
	uint8_t  type;
} __attribute__ ((packed));

#define CMD_MEMORY_WRITE	0xfc8e

static struct bt_hci *hci_dev;
static uint16_t hci_index = 0;

#define FIRMWARE_BASE_PATH "/lib/firmware"

static bool set_bdaddr = false;
static const char *set_bdaddr_value = NULL;
static bool get_bddata = false;
static bool load_firmware = false;
static const char *load_firmware_value = NULL;
static uint8_t *firmware_data = NULL;
static size_t firmware_size = 0;
static size_t firmware_offset = 0;
static bool check_firmware = false;
static const char *check_firmware_value = NULL;
uint8_t manufacturer_mode_reset = 0x00;
static bool use_manufacturer_mode = false;
static bool set_traces = false;
static bool set_exception = false;
static bool reset_on_exit = false;
static bool cold_boot = false;

static void reset_complete(const void *data, uint8_t size, void *user_data)
{
	uint8_t status = *((uint8_t *) data);

	if (status) {
		fprintf(stderr, "Failed to reset (0x%02x)\n", status);
		mainloop_quit();
		return;
	}

	mainloop_quit();
}

static void cold_boot_complete(const void *data, uint8_t size, void *user_data)
{
	uint8_t status = *((uint8_t *) data);

	if (status) {
		fprintf(stderr, "Failed to cold boot (0x%02x)\n", status);
		mainloop_quit();
		return;
	}

	if (reset_on_exit) {
		bt_hci_send(hci_dev, BT_HCI_CMD_RESET, NULL, 0,
						reset_complete, NULL, NULL);
		return;
	}

	mainloop_quit();
}

static void leave_manufacturer_mode_complete(const void *data, uint8_t size,
							void *user_data)
{
	uint8_t status = *((uint8_t *) data);

	if (status) {
		fprintf(stderr, "Failed to leave manufacturer mode (0x%02x)\n",
									status);
		mainloop_quit();
		return;
	}

	if (reset_on_exit) {
		bt_hci_send(hci_dev, BT_HCI_CMD_RESET, NULL, 0,
						reset_complete, NULL, NULL);
		return;
	}

	mainloop_quit();
}

static void shutdown_device(void)
{
	bt_hci_flush(hci_dev);

	free(firmware_data);

	if (use_manufacturer_mode) {
		struct cmd_manufacturer_mode cmd;

		cmd.mode_switch = 0x00;
		cmd.reset = manufacturer_mode_reset;

		bt_hci_send(hci_dev, CMD_MANUFACTURER_MODE, &cmd, sizeof(cmd),
				leave_manufacturer_mode_complete, NULL, NULL);
		return;
	}

	if (reset_on_exit) {
		bt_hci_send(hci_dev, BT_HCI_CMD_RESET, NULL, 0,
						reset_complete, NULL, NULL);
		return;
	}

	mainloop_quit();
}

static void write_bd_address_complete(const void *data, uint8_t size,
							void *user_data)
{
	uint8_t status = *((uint8_t *) data);

	if (status) {
		fprintf(stderr, "Failed to write address (0x%02x)\n", status);
		mainloop_quit();
		return;
	}

	shutdown_device();
}

static void read_bd_addr_complete(const void *data, uint8_t size,
							void *user_data)
{
	const struct bt_hci_rsp_read_bd_addr *rsp = data;
	struct cmd_write_bd_address cmd;

	if (rsp->status) {
		fprintf(stderr, "Failed to read address (0x%02x)\n",
							rsp->status);
		mainloop_quit();
		shutdown_device();
		return;
	}

	if (set_bdaddr_value) {
		fprintf(stderr, "Setting address is not supported\n");
		mainloop_quit();
		return;
	}

	printf("Controller Address\n");
	printf("\tOld BD_ADDR: %2.2X:%2.2X:%2.2X:%2.2X:%2.2X:%2.2X\n",
					rsp->bdaddr[5], rsp->bdaddr[4],
					rsp->bdaddr[3], rsp->bdaddr[2],
					rsp->bdaddr[1], rsp->bdaddr[0]);

	memcpy(cmd.bdaddr, rsp->bdaddr, 6);
	cmd.bdaddr[0] = (hci_index & 0xff);

	printf("\tNew BD_ADDR: %2.2X:%2.2X:%2.2X:%2.2X:%2.2X:%2.2X\n",
					cmd.bdaddr[5], cmd.bdaddr[4],
					cmd.bdaddr[3], cmd.bdaddr[2],
					cmd.bdaddr[1], cmd.bdaddr[0]);

	bt_hci_send(hci_dev, CMD_WRITE_BD_ADDRESS, &cmd, sizeof(cmd),
					write_bd_address_complete, NULL, NULL);
}

static void act_deact_traces_complete(const void *data, uint8_t size,
							void *user_data)
{
	uint8_t status = *((uint8_t *) data);

	if (status) {
		fprintf(stderr, "Failed to activate traces (0x%02x)\n", status);
		shutdown_device();
		return;
	}

	shutdown_device();
}

static void act_deact_traces(void)
{
	struct cmd_act_deact_traces cmd;

	cmd.tx_trace = 0x03;
	cmd.tx_arq = 0x03;
	cmd.rx_trace = 0x03;

	bt_hci_send(hci_dev, CMD_ACT_DEACT_TRACES, &cmd, sizeof(cmd),
					act_deact_traces_complete, NULL, NULL);
}

static void trigger_exception(void)
{
	struct cmd_trigger_exception cmd;

	cmd.type = 0x00;

	bt_hci_send(hci_dev, CMD_TRIGGER_EXCEPTION, &cmd, sizeof(cmd),
							NULL, NULL, NULL);

	shutdown_device();
}

static void write_bd_data_complete(const void *data, uint8_t size,
							void *user_data)
{
	uint8_t status = *((uint8_t *) data);

	if (status) {
		fprintf(stderr, "Failed to write data (0x%02x)\n", status);
		shutdown_device();
		return;
	}

	if (set_traces) {
		act_deact_traces();
		return;
	}

	shutdown_device();
}

static void read_bd_data_complete(const void *data, uint8_t size,
							void *user_data)
{
	const struct rsp_read_bd_data *rsp = data;

	if (rsp->status) {
		fprintf(stderr, "Failed to read data (0x%02x)\n", rsp->status);
		shutdown_device();
		return;
	}

	printf("Controller Data\n");
	printf("\tBD_ADDR: %2.2X:%2.2X:%2.2X:%2.2X:%2.2X:%2.2X\n",
					rsp->bdaddr[5], rsp->bdaddr[4],
					rsp->bdaddr[3], rsp->bdaddr[2],
					rsp->bdaddr[1], rsp->bdaddr[0]);

	printf("\tLMP Version: %u\n", rsp->lmp_version);
	printf("\tLMP Features: 0x%2.2x 0x%2.2x 0x%2.2x 0x%2.2x"
					" 0x%2.2x 0x%2.2x 0x%2.2x 0x%2.2x\n",
					rsp->features[0], rsp->features[1],
					rsp->features[2], rsp->features[3],
					rsp->features[4], rsp->features[5],
					rsp->features[6], rsp->features[7]);
	printf("\tLE Features: 0x%2.2x\n", rsp->le_features);

	if (set_bdaddr) {
		struct cmd_write_bd_data cmd;

		memcpy(cmd.bdaddr, rsp->bdaddr, 6);
		cmd.bdaddr[0] = (hci_index & 0xff);
		cmd.lmp_version = 0x07;
		memcpy(cmd.features, rsp->features, 8);
		cmd.le_features = rsp->le_features;
		cmd.le_features |= 0x1e;
		memcpy(cmd.reserved1, rsp->reserved1, sizeof(cmd.reserved1));
		memcpy(cmd.reserved2, rsp->reserved2, sizeof(cmd.reserved2));
		memcpy(cmd.reserved3, rsp->reserved3, sizeof(cmd.reserved3));

		bt_hci_send(hci_dev, CMD_WRITE_BD_DATA, &cmd, sizeof(cmd),
					write_bd_data_complete, NULL, NULL);
		return;
	}

	shutdown_device();
}

static void firmware_command_complete(const void *data, uint8_t size,
							void *user_data)
{
	uint8_t status = *((uint8_t *) data);

	if (status) {
		fprintf(stderr, "Failed to load firmware (0x%02x)\n", status);
		manufacturer_mode_reset = 0x01;
		shutdown_device();
		return;
	}

	if (firmware_offset >= firmware_size) {
		printf("Activating firmware\n");
		manufacturer_mode_reset = 0x02;
		shutdown_device();
		return;
	}

	if (firmware_data[firmware_offset] == 0x01) {
		uint16_t opcode;
		uint8_t dlen;

		opcode = firmware_data[firmware_offset + 2] << 8 |
					firmware_data[firmware_offset + 1];
		dlen = firmware_data[firmware_offset + 3];

		bt_hci_send(hci_dev, opcode, firmware_data +
						firmware_offset + 4, dlen,
					firmware_command_complete, NULL, NULL);

		firmware_offset += dlen + 4;

		if (firmware_data[firmware_offset] == 0x02) {
			dlen = firmware_data[firmware_offset + 2];
			firmware_offset += dlen + 3;
		}
	} else {
		fprintf(stderr, "Invalid packet in firmware\n");
		manufacturer_mode_reset = 0x01;
		shutdown_device();
	}

}

static void enter_manufacturer_mode_complete(const void *data, uint8_t size,
							void *user_data)
{
	uint8_t status = *((uint8_t *) data);

	if (status) {
		fprintf(stderr, "Failed to enter manufacturer mode (0x%02x)\n",
									status);
		mainloop_quit();
		return;
	}

	if (load_firmware) {
		uint8_t status = BT_HCI_ERR_SUCCESS;
		firmware_command_complete(&status, sizeof(status), NULL);
		return;
	}

	if (get_bddata || set_bdaddr) {
		bt_hci_send(hci_dev, CMD_READ_BD_DATA, NULL, 0,
					read_bd_data_complete, NULL, NULL);
		return;
	}

	if (set_traces) {
		act_deact_traces();
		return;
	}

	if (set_exception) {
		trigger_exception();
		return;
	}

	shutdown_device();
}

static void request_firmware(const char *path)
{
	unsigned int cmd_num = 0;
	unsigned int evt_num = 0;
	struct stat st;
	ssize_t len;
	int fd;

	fd = open(path, O_RDONLY);
	if (fd < 0) {
		fprintf(stderr, "Failed to open firmware %s\n", path);
		shutdown_device();
		return;
	}

	if (fstat(fd, &st) < 0) {
		fprintf(stderr, "Failed to get firmware size\n");
		close(fd);
		shutdown_device();
		return;
	}

	firmware_data = malloc(st.st_size);
	if (!firmware_data) {
		fprintf(stderr, "Failed to allocate firmware buffer\n");
		close(fd);
		shutdown_device();
		return;
	}

	len = read(fd, firmware_data, st.st_size);
	if (len < 0) {
		fprintf(stderr, "Failed to read firmware file\n");
		close(fd);
		shutdown_device();
		return;
	}

	close(fd);

	if (len < st.st_size) {
		fprintf(stderr, "Firmware size does not match buffer\n");
		shutdown_device();
		return;
	}

	firmware_size = len;

	if (firmware_data[0] == 0xff)
		firmware_offset = 1;

	while (firmware_offset < firmware_size) {
		uint16_t opcode;
		uint8_t evt, dlen;

		switch (firmware_data[firmware_offset]) {
		case 0x01:
			opcode = firmware_data[firmware_offset + 2] << 8 |
					firmware_data[firmware_offset + 1];
			dlen = firmware_data[firmware_offset + 3];

			if (opcode != CMD_MEMORY_WRITE)
				printf("Unexpected opcode 0x%02x\n", opcode);

			firmware_offset += dlen + 4;
			cmd_num++;
			break;

		case 0x02:
			evt = firmware_data[firmware_offset + 1];
			dlen = firmware_data[firmware_offset + 2];

			if (evt != BT_HCI_EVT_CMD_COMPLETE)
				printf("Unexpected event 0x%02x\n", evt);

			firmware_offset += dlen + 3;
			evt_num++;
			break;

		default:
			fprintf(stderr, "Invalid firmware file\n");
			shutdown_device();
			return;
		}
	}

	printf("Firmware with %u commands and %u events\n", cmd_num, evt_num);

	if (firmware_data[0] == 0xff)
		firmware_offset = 1;
}

static void read_boot_params_complete(const void *data, uint8_t size,
							void *user_data)
{
	const struct rsp_read_boot_params *rsp = data;

	if (rsp->status) {
		fprintf(stderr, "Failed to read boot params (0x%02x)\n",
							rsp->status);
		mainloop_quit();
		return;
	}

	if (size != sizeof(*rsp)) {
		fprintf(stderr, "Size mismatch for read boot params\n");
		mainloop_quit();
		return;
	}

	printf("Secure Boot Parameters\n");
	printf("\tOTP Format Version:\t%u\n", rsp->otp_format);
	printf("\tOTP Content Version:\t%u\n", rsp->otp_content);
	printf("\tOTP ROM Patch Version:\t%u\n", rsp->otp_patch);
	printf("\tDevice Revision ID:\t%u\n", le16_to_cpu(rsp->dev_revid));
	printf("\tSecure Boot Enable:\t%u\n", rsp->secure_boot);
	printf("\tTake Key From Header:\t%u\n", rsp->key_from_hdr);
	printf("\tRSA Key Type:\t\t%u\n", rsp->key_type);
	printf("\tOTP Lock:\t\t%u\n", rsp->otp_lock);
	printf("\tAPI Lock:\t\t%u\n", rsp->api_lock);
	printf("\tDebug Lock:\t\t%u\n", rsp->debug_lock);
	printf("\tMin FW Build Number:\t%u-%u.%u\n", rsp->min_fw_build_nn,
			rsp->min_fw_build_cw, 2000 + rsp->min_fw_build_yy);
	printf("\tLimited CCE to ISSC:\t%u\n", rsp->limited_cce);
	printf("\tUnlocked State:\t\t%u\n", rsp->unlocked_state);

	mainloop_quit();
}

static const struct {
	uint8_t val;
	const char *str;
} hw_variant_table[] = {
	{ 0x06, "iBT 1.1 (XG223)"	},
	{ 0x07, "iBT 2.0 (WP)"		},
	{ 0x08, "iBT 2.5 (StP)"		},
	{ 0x09, "iBT 1.5 (AG610)"	},
	{ 0x0a, "iBT 2.1 (AG620)"	},
	{ 0x0b, "iBT 3.0 (LnP)"		},
	{ 0x0c, "iBT 3.0 (WsP)"		},
	{ }
};

static const struct {
	uint8_t val;
	const char *str;
} fw_variant_table[] = {
	{ 0x01, "iBT 1.0 - iBT 2.5"	},
	{ 0x06, "iBT Bootloader"	},
	{ 0x23, "iBT 3.x Bluetooth FW"	},
	{ }
};

static void read_version_complete(const void *data, uint8_t size,
							void *user_data)
{
	const struct rsp_read_version *rsp = data;
	const char *str;
	int i;

	if (rsp->status) {
		fprintf(stderr, "Failed to read version (0x%02x)\n",
							rsp->status);
		mainloop_quit();
		return;
	}

	if (size != sizeof(*rsp)) {
		fprintf(stderr, "Size mismatch for read version response\n");
		mainloop_quit();
		return;
	}

	if (cold_boot) {
		struct cmd_reset cmd;

		cmd.reset_type = 0x01;
		cmd.patch_enable = 0x00;
		cmd.otp_ddc_reload = 0x01;
		cmd.boot_option = 0x00;
		cmd.boot_addr = cpu_to_le32(0x00000000);

		bt_hci_send(hci_dev, CMD_RESET, &cmd, sizeof(cmd),
					cold_boot_complete, NULL, NULL);
		return;
	}

	if (load_firmware) {
		if (load_firmware_value) {
			printf("Firmware: %s\n", load_firmware_value);
			request_firmware(load_firmware_value);
		} else {
			char fw_name[PATH_MAX];

			snprintf(fw_name, sizeof(fw_name),
				"%s/%s/ibt-hw-%x.%x.%x-fw-%x.%x.%x.%x.%x.bseq",
				FIRMWARE_BASE_PATH, "intel",
				rsp->hw_platform, rsp->hw_variant,
				rsp->hw_revision, rsp->fw_variant,
				rsp->fw_revision, rsp->fw_build_nn,
				rsp->fw_build_cw, rsp->fw_build_yy);

			printf("Firmware: %s\n", fw_name);
			printf("Patch level: %d\n", rsp->fw_patch);
			request_firmware(fw_name);
		}
	}

	if (use_manufacturer_mode) {
		struct cmd_manufacturer_mode cmd;

		cmd.mode_switch = 0x01;
		cmd.reset = 0x00;

		bt_hci_send(hci_dev, CMD_MANUFACTURER_MODE, &cmd, sizeof(cmd),
				enter_manufacturer_mode_complete, NULL, NULL);
		return;
	}

	if (set_bdaddr) {
		bt_hci_send(hci_dev, BT_HCI_CMD_READ_BD_ADDR, NULL, 0,
					read_bd_addr_complete, NULL, NULL);
		return;
	}

	printf("Controller Version Information\n");
	printf("\tHardware Platform:\t%u\n", rsp->hw_platform);

	str = "Reserved";

	for (i = 0; hw_variant_table[i].str; i++) {
		if (hw_variant_table[i].val == rsp->hw_variant) {
			str = hw_variant_table[i].str;
			break;
		}
	}

	printf("\tHardware Variant:\t%s (0x%02x)\n", str, rsp->hw_variant);
	printf("\tHardware Revision:\t%u.%u\n", rsp->hw_revision >> 4,
						rsp->hw_revision & 0x0f);

	str = "Reserved";

	for (i = 0; fw_variant_table[i].str; i++) {
		if (fw_variant_table[i].val == rsp->fw_variant) {
			str = fw_variant_table[i].str;
			break;
		}
	}

	printf("\tFirmware Variant:\t%s (0x%02x)\n", str, rsp->fw_variant);
	printf("\tFirmware Revision:\t%u.%u\n", rsp->fw_revision >> 4,
						rsp->fw_revision & 0x0f);
	printf("\tFirmware Build Number:\t%u-%u.%u\n", rsp->fw_build_nn,
				rsp->fw_build_cw, 2000 + rsp->fw_build_yy);
	printf("\tFirmware Patch Number:\t%u\n", rsp->fw_patch);

	if (rsp->hw_variant == 0x0b && rsp->fw_variant == 0x06) {
		bt_hci_send(hci_dev, CMD_READ_BOOT_PARAMS, NULL, 0,
					read_boot_params_complete, NULL, NULL);
		return;
	}

	mainloop_quit();
}

struct css_hdr {
	uint32_t module_type;
	uint32_t header_len;
	uint32_t header_version;
	uint32_t module_id;
	uint32_t module_vendor;
	uint32_t date;
	uint32_t size;
	uint32_t key_size;
	uint32_t modulus_size;
	uint32_t exponent_size;
	uint8_t  reserved[88];
} __attribute__ ((packed));

static void analyze_firmware(const char *path)
{
	unsigned int cmd_num = 0;
	struct css_hdr *css;
	struct stat st;
	ssize_t len;
	int fd;

	fd = open(path, O_RDONLY);
	if (fd < 0) {
		fprintf(stderr, "Failed to open firmware %s\n", path);
		return;
	}

	if (fstat(fd, &st) < 0) {
		fprintf(stderr, "Failed to get firmware size\n");
		close(fd);
		return;
	}

	firmware_data = malloc(st.st_size);
	if (!firmware_data) {
		fprintf(stderr, "Failed to allocate firmware buffer\n");
		close(fd);
		return;
	}

	len = read(fd, firmware_data, st.st_size);
	if (len < 0) {
		fprintf(stderr, "Failed to read firmware file\n");
		close(fd);
		goto done;
	}

	close(fd);

	if (len != st.st_size) {
		fprintf(stderr, "Failed to read complete firmware file\n");
		goto done;
	}

	if ((size_t) len < sizeof(*css)) {
		fprintf(stderr, "Firmware file is too short\n");
		goto done;
	}

	css = (void *) firmware_data;

	printf("Module type:\t%u\n", le32_to_cpu(css->module_type));
	printf("Header len:\t%u DWORDs / %u bytes\n",
				le32_to_cpu(css->header_len),
				le32_to_cpu(css->header_len) * 4);
	printf("Header version:\t%u.%u\n",
				le32_to_cpu(css->header_version) >> 16,
				le32_to_cpu(css->header_version) & 0xffff);
	printf("Module ID:\t%u\n", le32_to_cpu(css->module_id));
	printf("Module vendor:\t%u\n", le32_to_cpu(css->module_vendor));
	printf("Date:\t\t%u\n", le32_to_cpu(css->date));
	printf("Size:\t\t%u DWORDs / %u bytes\n", le32_to_cpu(css->size),
						le32_to_cpu(css->size) * 4);
	printf("Key size:\t%u DWORDs / %u bytes\n",
					le32_to_cpu(css->key_size),
					le32_to_cpu(css->key_size) * 4);
	printf("Modulus size:\t%u DWORDs / %u bytes\n",
					le32_to_cpu(css->modulus_size),
					le32_to_cpu(css->modulus_size) * 4);
	printf("Exponent size:\t%u DWORDs / %u bytes\n",
					le32_to_cpu(css->exponent_size),
					le32_to_cpu(css->exponent_size) * 4);
	printf("\n");


	if ((size_t) len != le32_to_cpu(css->size) * 4) {
		fprintf(stderr, "CSS.size does not match file length\n");
		goto done;
	}

	if (le32_to_cpu(css->header_len) != (sizeof(*css) / 4) +
					le32_to_cpu(css->key_size) +
					le32_to_cpu(css->modulus_size) +
					le32_to_cpu(css->exponent_size)) {
		fprintf(stderr, "CSS.headerLen does not match data sizes\n");
		goto done;
	}

	firmware_size = le32_to_cpu(css->size) * 4;
	firmware_offset = le32_to_cpu(css->header_len) * 4;

	while (firmware_offset < firmware_size) {
		uint16_t opcode;
		uint8_t dlen;

		opcode = get_le16(firmware_data + firmware_offset);
		dlen = firmware_data[firmware_offset + 2];

		switch (opcode) {
		case CMD_NO_OPERATION:
		case CMD_WRITE_BOOT_PARAMS:
		case CMD_MEMORY_WRITE:
			break;
		default:
			printf("Unexpected opcode 0x%02x\n", opcode);
			break;
		}

		firmware_offset += dlen + 3;
		cmd_num++;
	}

	printf("Firmware with %u commands\n", cmd_num);

done:
	free(firmware_data);
}

static void signal_callback(int signum, void *user_data)
{
	switch (signum) {
	case SIGINT:
	case SIGTERM:
		mainloop_quit();
		break;
	}
}

static void usage(void)
{
	printf("bluemoon - Bluemoon configuration utility\n"
		"Usage:\n");
	printf("\tbluemoon [options]\n");
	printf("Options:\n"
		"\t-A, --bdaddr [addr]    Set Bluetooth address\n"
		"\t-F, --firmware [file]  Load firmware\n"
		"\t-C, --check <file>     Check firmware image\n"
		"\t-R, --reset            Reset controller\n"
		"\t-B, --coldboot         Cold boot controller\n"
		"\t-E, --exception        Trigger exception\n"
		"\t-i, --index <num>      Use specified controller\n"
		"\t-h, --help             Show help options\n");
}

static const struct option main_options[] = {
	{ "bdaddr",   optional_argument, NULL, 'A' },
	{ "bddata",   no_argument,       NULL, 'D' },
	{ "firmware", optional_argument, NULL, 'F' },
	{ "check",    required_argument, NULL, 'C' },
	{ "traces",   no_argument,       NULL, 'T' },
	{ "reset",    no_argument,       NULL, 'R' },
	{ "coldboot", no_argument,       NULL, 'B' },
	{ "exception",no_argument,       NULL, 'E' },
	{ "index",    required_argument, NULL, 'i' },
	{ "raw",      no_argument,       NULL, 'r' },
	{ "version",  no_argument,       NULL, 'v' },
	{ "help",     no_argument,       NULL, 'h' },
	{ }
};

int main(int argc, char *argv[])
{
	const char *str;
	bool use_raw = false;
	sigset_t mask;
	int exit_status;

	for (;;) {
		int opt;

		opt = getopt_long(argc, argv, "A::DF::C:TRBEi:rvh",
						main_options, NULL);
		if (opt < 0)
			break;

		switch (opt) {
		case 'A':
			if (optarg)
				set_bdaddr_value = optarg;
			set_bdaddr = true;
			break;
		case 'D':
			use_manufacturer_mode = true;
			get_bddata = true;
			break;
		case 'F':
			use_manufacturer_mode = true;
			if (optarg)
				load_firmware_value = optarg;
			load_firmware = true;
			break;
		case 'C':
			check_firmware_value = optarg;
			check_firmware = true;
			break;
		case 'E':
			use_manufacturer_mode = true;
			set_exception = true;
			break;
		case 'T':
			use_manufacturer_mode = true;
			set_traces = true;
			break;
		case 'R':
			reset_on_exit = true;
			break;
		case 'B':
			cold_boot = true;
			break;
		case 'i':
			if (strlen(optarg) > 3 && !strncmp(optarg, "hci", 3))
				str = optarg + 3;
			else
				str = optarg;
			if (!isdigit(*str)) {
				usage();
				return EXIT_FAILURE;
			}
			hci_index = atoi(str);
			break;
		case 'r':
			use_raw = true;
			break;
		case 'v':
			printf("%s\n", VERSION);
			return EXIT_SUCCESS;
		case 'h':
			usage();
			return EXIT_SUCCESS;
		default:
			return EXIT_FAILURE;
		}
	}

	if (argc - optind > 0) {
		fprintf(stderr, "Invalid command line parameters\n");
		return EXIT_FAILURE;
	}

	mainloop_init();

	sigemptyset(&mask);
	sigaddset(&mask, SIGINT);
	sigaddset(&mask, SIGTERM);

	mainloop_set_signal(&mask, signal_callback, NULL, NULL);

	printf("Bluemoon configuration utility ver %s\n", VERSION);

	if (check_firmware) {
		analyze_firmware(check_firmware_value);
		return EXIT_SUCCESS;
	}

	if (use_raw) {
		hci_dev = bt_hci_new_raw_device(hci_index);
		if (!hci_dev) {
			fprintf(stderr, "Failed to open HCI raw device\n");
			return EXIT_FAILURE;
		}
	} else {
		hci_dev = bt_hci_new_user_channel(hci_index);
		if (!hci_dev) {
			fprintf(stderr, "Failed to open HCI user channel\n");
			return EXIT_FAILURE;
		}
	}

	bt_hci_send(hci_dev, CMD_READ_VERSION, NULL, 0,
					read_version_complete, NULL, NULL);

	exit_status = mainloop_run();

	bt_hci_unref(hci_dev);

	return exit_status;
}

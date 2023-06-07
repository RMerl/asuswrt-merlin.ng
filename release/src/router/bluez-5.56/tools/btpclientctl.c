// SPDX-License-Identifier: GPL-2.0-or-later
/*
 *
 *  BlueZ - Bluetooth protocol stack for Linux
 *
 *  Copyright (C) 2012  Intel Corporation. All rights reserved.
 *
 *
 */

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#define _GNU_SOURCE
#include <stdio.h>
#include <errno.h>
#include <unistd.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <poll.h>

#include "lib/bluetooth.h"

#include "src/shared/ad.h"
#include "src/shared/btp.h"
#include "src/shared/io.h"
#include "src/shared/mainloop.h"
#include "src/shared/queue.h"
#include "src/shared/shell.h"
#include "src/shared/util.h"

#define DEFAULT_SOCKET_PATH	"/tmp/bt-stack-tester"

#define PROMPT_ON	COLOR_BLUE "[btpclient]" COLOR_OFF "# "

#define EVT_OPCODE_BASE	0x80

#define DEFAULT_INDEX	0x00

struct ad_data {
	uint8_t data[25];
	uint8_t len;
};

struct name_func_entry {
	const char *name;
	void (*func)(void);
};

struct indexstr_data {
	int index;
	const char *str;
};

struct bitfield_data {
	uint32_t bit;
	const char *str;
};

struct opcode_data {
	uint8_t opcode;
	int bit;
	const char *str;

	void (*cmd_func)(const void *data, uint16_t size);
	uint16_t cmd_size;
	bool cmd_fixed;

	void (*rsp_func)(const void *data, uint16_t size);
	uint16_t rsp_size;
	bool rsp_fixed;

	void (*evt_func)(const void *data, uint16_t size);
	uint16_t evt_size;
	bool evt_fixed;
};

struct service_data {
	uint8_t id;
	int bit;
	const char *str;
	const struct opcode_data *opcode_table;
};

struct advertise_data {
	struct bt_ad *ad;
	struct bt_ad *scan;
};

struct client_data {
	int fd;

	/* Incoming buffer for response and event */
	uint8_t buf[512];
};

struct btpclientctl {
	int server_fd;
	char *socket_path;

	bool debug_enabled;
	bool enable_dump;

	bool client_active;
	struct client_data *client_data;

	/* Outgoing buffer for command */
	uint8_t buf[560];
	uint16_t buf_len;
};

static struct advertise_data *advertise_data;
static struct btpclientctl *btpclientctl;
static uint8_t bt_index = DEFAULT_INDEX;

static void hexdump_print(const char *str, void *user_data)
{
	bt_shell_printf("%s%s\n", (char *) user_data, str);
}

static bool parse_argument_on_off(int argc, char *argv[], uint8_t *val)
{
	if (!strcasecmp(argv[1], "on") || !strcasecmp(argv[1], "yes"))
		*val = 1;
	else if (!strcasecmp(argv[1], "off") || !strcasecmp(argv[1], "no"))
		*val = 0;
	else
		*val = atoi(argv[1]);
	return true;
}

static bool parse_argument_list(int argc, char *argv[], uint8_t *val,
					const struct indexstr_data *table)
{
	int i;

	for (i = 0; table[i].str; i++) {
		if (strcasecmp(argv[1], table[i].str) == 0) {
			*val = table[i].index;
			return true;
		}
	}

	bt_shell_printf("Invalid argument %s\n", argv[1]);

	return false;
}

static bool parse_argument_bitfield_list(int argc, char *argv[], uint32_t *val,
					const struct bitfield_data *table)
{
	int i;

	for (i = 0; table[i].str; i++) {
		if (strcasecmp(argv[0], table[i].str) == 0) {
			*val = table[i].bit;
			return true;
		}
	}

	bt_shell_printf("Invalid argument %s\n", argv[0]);

	return false;
}

static bool parse_argument_addr(int argc, char *argv[], uint8_t *addr_type,
							bdaddr_t *bdaddr)
{
	if (argc < 3) {
		bt_shell_printf("Invalid parameter\n");
		return false;
	}

	*addr_type = atoi(argv[1]);
	str2ba(argv[2], bdaddr);

	return true;
}

static char *argument_gen(const char *text, int state,
					const struct indexstr_data *list)
{
	static int index, len;
	const char *arg;

	if (!state) {
		index = 0;
		len = strlen(text);
	}

	while ((arg = list[index].str)) {
		index++;

		if (!strncasecmp(arg, text, len))
			return strdup(arg);
	}

	return NULL;
}

static char *argument_gen_bitfield(const char *text, int state,
					const struct bitfield_data *list)
{
	static int index, len;
	const char *arg;

	if (!state) {
		index = 0;
		len = strlen(text);
	}

	while ((arg = list[index].str)) {
		index++;

		if (!strncasecmp(arg, text, len))
			return strdup(arg);
	}

	return NULL;
}

static char *argument_gen_name_func_entry(const char *text, int state,
					const struct name_func_entry *list)
{
	static int index, len;
	const char *arg;

	if (!state) {
		index = 0;
		len = strlen(text);
	}

	while ((arg = list[index].name)) {
		index++;

		if (!strncasecmp(arg, text, len))
			return strdup(arg);
	}

	return NULL;
}

static const struct service_data service_table[];

static const struct service_data *find_service_data(uint8_t service_id)
{
	int i;

	for (i = 0; service_table[i].str; i++) {
		if (service_table[i].id == service_id)
			return &service_table[i];
	}

	return NULL;
}

static const struct opcode_data *find_opcode_data(uint8_t opcode,
						const struct opcode_data *table)
{
	int i;

	for (i = 0; table[i].str; i++) {
		if (table[i].opcode == opcode)
			return &table[i];
	}

	return NULL;
}

static const char *get_indexstr(int val, const struct indexstr_data *table)
{
	int i;

	for (i = 0; table[i].str; i++) {
		if (val == table[i].index)
			return table[i].str;
	}

	return "Unknown";
}

static uint32_t print_bitfield(uint32_t val, const struct bitfield_data *table,
							const char *prefix)
{
	uint32_t mask = val;
	int i;

	for (i = 0; table[i].str; i++) {
		if (val & (((uint32_t) 1) << table[i].bit)) {
			bt_shell_printf("%s%s (0x%4.4x)\n", prefix,
						table[i].str, table[i].bit);
			mask &= ~(((uint32_t) 1) << table[i].bit);
		}
	}

	return mask;
}

static void print_bdaddr(const bdaddr_t *address, uint8_t address_type)
{
	char addr[18];

	ba2str(address, addr);
	if (address_type == BTP_GAP_ADDR_PUBLIC)
		bt_shell_printf("\t%s (public)\n", addr);
	else if (address_type == BTP_GAP_ADDR_RANDOM)
		bt_shell_printf("\t%s (random)\n", addr);
	else
		bt_shell_printf("\t%s (unknown)\n", addr);
}

static void null_cmd(const void *data, uint16_t size)
{
	/* Empty */
}

static void null_rsp(const void *data, uint16_t size)
{
	/* Empty */
}

static void null_evt(const void *data, uint16_t size)
{
	/* Empty */
}

static const struct indexstr_data error_table[] = {
	{ 0x01, "Faile" },
	{ 0x02, "Unknown Command" },
	{ 0x03, "Not Ready" },
	{ 0x04, "Invalid Index" },
	{ }
};

static void print_error_rsp(const void *data, uint16_t size)
{
	uint8_t reason = ((uint8_t *)data)[0];

	bt_shell_printf(COLOR_RED "\tReason: %s (%d)\n" COLOR_OFF,
				get_indexstr(reason, error_table), reason);
}

static const char *service_to_str(uint8_t service_id)
{
	int i;

	for (i = 0; service_table[i].str; i++) {
		if (service_table[i].id == service_id)
			return service_table[i].str;
	}

	return "Unknown Service ID";
}

static const char *get_supported_service(int bit)
{
	int i;

	for (i = 0; service_table[i].str; i++) {
		if (service_table[i].bit == bit)
			return service_table[i].str;
	}

	return NULL;
}

static const char *get_supported_command(const struct opcode_data *table,
									int bit)
{
	int i;

	for (i = 0; table[i].str; i++) {
		if (table[i].bit == bit)
			return table[i].str;
	}
	return NULL;
}

static void print_btp_hdr(struct btp_hdr *btp_hdr, const char *type_str,
							const char *opcode_str)
{
	bt_shell_printf("%s: %s(%d) %s(0x%02x) INDEX(0x%02x)\n", type_str,
			service_to_str(btp_hdr->service), btp_hdr->service,
			opcode_str, btp_hdr->opcode, btp_hdr->index);
}

static const struct opcode_data opcode_table_core[];

static void print_core_read_supported_commands_rsp(const void *data,
								uint16_t size)
{
	uint8_t cmds;
	const char *str;
	int i, bit;

	cmds = ((uint8_t *)data)[0];

	for (i = 1; i < (int)(sizeof(cmds) * 8); i++) {
		bit = 0;
		bit = 1 << i;
		if (cmds & bit) {
			str = get_supported_command(opcode_table_core, i);
			if (str)
				bt_shell_printf("\t%s (Bit %d)\n", str, i);
			else
				bt_shell_printf("\tUNKNOWN (Bit %d)\n", i);
		}
	}
}

static void print_core_read_supported_services_rsp(const void *data,
								uint16_t size)
{
	uint8_t services;
	const char *str;
	int i, bit;

	services = ((uint8_t *)data)[0];

	for (i = 0; i < (int)(sizeof(services) * 8); i++) {
		bit = 1 << i;
		if (services & bit) {
			str = get_supported_service(i);
			if (str)
				bt_shell_printf("\t%s (Bit %d)\n", str, i);
			else
				bt_shell_printf("\tUNKNOWN (Bit %d)\n", i);
		}
	}
}

static void print_core_register_service_cmd(const void *data, uint16_t size)
{
	const struct btp_core_register_cp *cp = data;

	bt_shell_printf("\tService ID: %s(0x%02x)\n",
			service_to_str(cp->service_id), cp->service_id);
}

static void print_core_unregister_service_cmd(const void *data, uint16_t size)
{
	const struct btp_core_unregister_cp *cp = data;

	bt_shell_printf("\tService ID: %s(0x%02x)\n",
			service_to_str(cp->service_id), cp->service_id);
}

static const struct opcode_data opcode_table_core[] = {
	{ 0x00, 0, "Error",
			null_cmd, 0, true,
			print_error_rsp, 1, true },
	{ 0x01, 1, "Read Supported Commands",
			null_cmd, 0, true,
			print_core_read_supported_commands_rsp, 1, true },
	{ 0x02, 2, "Read Supported Services",
			null_cmd, 0, true,
			print_core_read_supported_services_rsp, 1, true },
	{ 0x03, 3, "Register Service",
			print_core_register_service_cmd, 1, true,
			null_rsp, 0, true },
	{ 0x04, 4, "Unregister Service",
			print_core_unregister_service_cmd, 1, true,
			null_rsp, 0, true },
	{ 0x80, -1, "IUT Ready",
			null_cmd, 0, true,
			null_rsp, 0, true,
			null_evt, 0, true },
	{ }
};

static const struct opcode_data opcode_table_gap[];

static void print_gap_read_supported_commands_rsp(const void *data,
								uint16_t size)
{
	uint16_t cmds;
	const char *str;
	int i;

	cmds = le16_to_cpu(((uint16_t *)data)[0]);

	for (i = 1; i < (int)(sizeof(cmds) * 8); i++) {
		if (cmds & (1 << i)) {
			str = get_supported_command(opcode_table_gap, i);
			if (str)
				bt_shell_printf("\t%s (Bit %d)\n", str, i);
			else
				bt_shell_printf("\tUNKNOWN (Bit %d)\n", i);
		}
	}
}

static void print_gap_read_controller_index_list_rsp(const void *data,
								uint16_t size)
{
	const struct btp_gap_read_index_rp *list = data;
	int i;

	for (i = 0; i < list->num; i++)
		bt_shell_printf("\tIndex: %d\n", list->indexes[i]);
}

static const struct bitfield_data gap_setting_table[] = {
	{ 0, "Powered" },
	{ 1, "Connectable" },
	{ 2, "Fast Connectable" },
	{ 3, "Discoverable" },
	{ 4, "Bondable" },
	{ 5, "Link Layer Security" },
	{ 6, "Secure Simple Pairing" },
	{ 7, "BR/EDR" },
	{ 8, "High Speed" },
	{ 9, "Low Energy" },
	{ 10, "Advertising" },
	{ 11, "Secure Connection" },
	{ 12, "Debug Keys" },
	{ 13, "Privacy" },
	{ 14, "Controller Configuration" },
	{ 15, "Static Address" },
	{ }
};

static void print_gap_settings(uint32_t val, const struct bitfield_data *table,
							const char *prefix)
{
	uint32_t mask;

	mask = print_bitfield(val, table, prefix);
	if (mask)
		bt_shell_printf("%sUnknown settings (0x%4.4x)\n", prefix, mask);
}

static void print_gap_read_controller_information_rsp(const void *data,
								uint16_t size)
{
	const struct btp_gap_read_info_rp *info = data;
	char addr[18];

	ba2str(&info->address, addr);
	bt_shell_printf("\tAddress: %s\n", addr);
	bt_shell_printf("\tSupported Settings\n");
	print_gap_settings(le32_to_cpu(info->supported_settings),
						gap_setting_table, "\t\t");
	bt_shell_printf("\tCurrent Settings\n");
	print_gap_settings(le32_to_cpu(info->current_settings),
						gap_setting_table, "\t\t");
	bt_shell_printf("\tClass: 0x%02x%02x%02x\n",
				info->cod[2], info->cod[1], info->cod[0]);
	bt_shell_printf("\tShort: %s\n", info->short_name);
	bt_shell_printf("\tName: %s\n", info->name);
}

static void print_gap_reset_rsp(const void *data, uint16_t size)
{
	const struct btp_gap_reset_rp *rp = data;

	print_gap_settings(le32_to_cpu(rp->current_settings),
						gap_setting_table, "\t");
}

static const struct indexstr_data on_off_table[] = {
	{ 0x00, "Off" },
	{ 0x01, "On" },
	{ }
};

static void print_gap_set_powered_cmd(const void *data, uint16_t size)
{
	const struct btp_gap_set_powered_cp *cp = data;

	bt_shell_printf("\tSet Power: %s (%d)\n",
				get_indexstr(cp->powered, on_off_table),
				cp->powered);
}

static void print_gap_set_powered_rsp(const void *data, uint16_t size)
{
	const struct btp_gap_set_powered_rp *rp = data;

	print_gap_settings(le32_to_cpu(rp->current_settings),
						gap_setting_table, "\t");
}

static void print_gap_set_connectable_cmd(const void *data, uint16_t size)
{
	const struct btp_gap_set_connectable_cp *cp = data;

	bt_shell_printf("\t Set Connectable: %s (%d)\n",
				get_indexstr(cp->connectable, on_off_table),
				cp->connectable);
}

static void print_gap_set_connectable_rsp(const void *data, uint16_t size)
{
	const struct btp_gap_set_connectable_rp *rp = data;

	print_gap_settings(le32_to_cpu(rp->current_settings),
						gap_setting_table, "\t");
}

static void print_gap_set_fast_connectable_cmd(const void *data, uint16_t size)
{
	const struct btp_gap_set_fast_connectable_cp *cp = data;

	bt_shell_printf("\t Set Fast Connectable: %s (%d)\n",
			get_indexstr(cp->fast_connectable, on_off_table),
			cp->fast_connectable);
}

static void print_gap_set_fast_connectable_rsp(const void *data, uint16_t size)
{
	const struct btp_gap_set_fast_connectable_rp *rp = data;

	print_gap_settings(le32_to_cpu(rp->current_settings),
						gap_setting_table, "\t");
}

static const struct indexstr_data gap_discoverable_table[] = {
	{ 0x00, "Off" },
	{ 0x01, "On" },
	{ 0x02, "Limited" },
	{ }
};

static void print_gap_set_discoverable_cmd(const void *data, uint16_t size)
{
	const struct btp_gap_set_discoverable_cp *cp = data;

	bt_shell_printf("\t Set Discoverable: %s (%d)\n",
			get_indexstr(cp->discoverable, gap_discoverable_table),
				cp->discoverable);
}

static void print_gap_set_discoverable_rsp(const void *data, uint16_t size)
{
	const struct btp_gap_set_discoverable_rp *rp = data;

	print_gap_settings(le32_to_cpu(rp->current_settings),
						gap_setting_table, "\t");
}

static void print_gap_set_bondable_cmd(const void *data, uint16_t size)
{
	const struct btp_gap_set_bondable_cp *cp = data;

	bt_shell_printf("\t Set Bondable: %s (%d)\n",
				get_indexstr(cp->bondable, on_off_table),
				cp->bondable);
}

static void print_gap_set_bondable_rsp(const void *data, uint16_t size)
{
	const struct btp_gap_set_bondable_rp *rp = data;

	print_gap_settings(le32_to_cpu(rp->current_settings),
						gap_setting_table, "\t");
}

const struct indexstr_data ad_type_table[] = {
	{ 0x01, "BT_AD_FLAGS" },
	{ 0x02, "BT_AD_UUID16_SOME" },
	{ 0x03, "BT_AD_UUID16_ALL" },
	{ 0x04, "BT_AD_UUID32_SOME" },
	{ 0x05, "BT_AD_UUID32_ALL" },
	{ 0x06, "BT_AD_UUID128_SOME" },
	{ 0x07, "BT_AD_UUID128_ALL" },
	{ 0x08, "BT_AD_NAME_SHORT" },
	{ 0x09, "BT_AD_NAME_COMPLETE" },
	{ 0x0a, "BT_AD_TX_POWER" },
	{ 0x0d, "BT_AD_CLASS_OF_DEV" },
	{ 0x0e, "BT_AD_SSP_HASH" },
	{ 0x0f, "BT_AD_SSP_RANDOMIZER" },
	{ 0x10, "BT_AD_DEVICE_ID" },
	{ 0x10, "BT_AD_SMP_TK" },
	{ 0x11, "BT_AD_SMP_OOB_FLAGS" },
	{ 0x12, "BT_AD_SLAVE_CONN_INTERVAL" },
	{ 0x14, "BT_AD_SOLICIT16" },
	{ 0x15, "BT_AD_SOLICIT128" },
	{ 0x16, "BT_AD_SERVICE_DATA16" },
	{ 0x17, "BT_AD_PUBLIC_ADDRESS" },
	{ 0x18, "BT_AD_RANDOM_ADDRESS" },
	{ 0x19, "BT_AD_GAP_APPEARANCE" },
	{ 0x1a, "BT_AD_ADVERTISING_INTERVAL" },
	{ 0x1b, "BT_AD_LE_DEVICE_ADDRESS" },
	{ 0x1c, "BT_AD_LE_ROLE" },
	{ 0x1d, "BT_AD_SSP_HASH_P256" },
	{ 0x1e, "BT_AD_SSP_RANDOMIZER_P256" },
	{ 0x1f, "BT_AD_SOLICIT32" },
	{ 0x20, "BT_AD_SERVICE_DATA32" },
	{ 0x21, "BT_AD_SERVICE_DATA128" },
	{ 0x22, "BT_AD_LE_SC_CONFIRM_VALUE" },
	{ 0x23, "BT_AD_LE_SC_RANDOM_VALUE" },
	{ 0x24, "BT_AD_URI" },
	{ 0x25, "BT_AD_INDOOR_POSITIONING" },
	{ 0x26, "BT_AD_TRANSPORT_DISCOVERY" },
	{ 0x27, "BT_AD_LE_SUPPORTED_FEATURES" },
	{ 0x28, "BT_AD_CHANNEL_MAP_UPDATE_IND" },
	{ 0x29, "BT_AD_MESH_PROV" },
	{ 0x2a, "BT_AD_MESH_DATA" },
	{ 0x2b, "BT_AD_MESH_BEACON" },
	{ 0x3d, "BT_AD_3D_INFO_DATA" },
	{ 0xff, "BT_AD_MANUFACTURER_DATA" }
};

struct ad_struct {
	uint8_t length;
	uint8_t type;
	uint8_t data[0];
};

static void print_ad_data(const uint8_t *data, size_t data_len)
{
	struct ad_struct *ad_struct;
	size_t count = 0;

	if (!data || !data_len) {
		bt_shell_printf("\tEmpty\n");
		return;
	}

	while (count < data_len) {
		ad_struct = (struct ad_struct *)(data + count);

		bt_shell_printf("Type: %s(0x%02x)\n",
				get_indexstr(ad_struct->type, ad_type_table),
				ad_struct->type);
		bt_shell_hexdump(ad_struct->data, ad_struct->length - 1);

		count += ad_struct->length + 1;
	}
}

static void print_gap_start_advertising_cmd(const void *data, uint16_t size)
{
	const struct btp_gap_start_adv_cp *cp = data;

	if (cp->adv_data_len) {
		bt_shell_printf("\tAdvertising Data:\n");
		print_ad_data(cp->data, cp->adv_data_len);
	}

	if (cp->scan_rsp_len) {
		bt_shell_printf("\tScan Response Data:\n");
		print_ad_data(cp->data + cp->adv_data_len, cp->scan_rsp_len);
	}
}

static void print_gap_start_advertising_rsp(const void *data, uint16_t size)
{
	const struct btp_gap_start_adv_rp *rp = data;

	print_gap_settings(le32_to_cpu(rp->current_settings),
						gap_setting_table, "\t");
}

static void print_gap_stop_advertising_rsp(const void *data, uint16_t size)
{
	const struct btp_gap_start_adv_rp *rp = data;

	print_gap_settings(le32_to_cpu(rp->current_settings),
						gap_setting_table, "\t");
}

static const struct bitfield_data gap_discovery_flags_table[] = {
	{ 0, "LE" },
	{ 1, "BREDE" },
	{ 2, "Limited" },
	{ 3, "Active" },
	{ 4, "Observation" },
	{ }
};

static void print_gap_start_discovery_cmd(const void *data, uint16_t size)
{
	const struct btp_gap_start_discovery_cp *cp = data;
	uint32_t mask;

	mask = print_bitfield(le32_to_cpu(cp->flags),
					gap_discovery_flags_table, "\t\t");
	if (mask)
		bt_shell_printf("\t\tUnknown flags (0x%4.4x)\n", mask);
}

static void print_gap_connect_cmd(const void *data, uint16_t size)
{
	const struct btp_gap_connect_cp *cp = data;

	print_bdaddr(&cp->address, cp->address_type);
}

static void print_gap_disconnect_cmd(const void *data, uint16_t size)
{
	const struct btp_gap_disconnect_cp *cp = data;

	print_bdaddr(&cp->address, cp->address_type);
}

static const struct indexstr_data gap_io_capa_table[] = {
	{ 0x00, "DisplayOnly" },
	{ 0x01, "DisplayYesNo" },
	{ 0x02, "KeyboardOnly" },
	{ 0x03, "NoInputOutput" },
	{ 0x04, "KeyboardDisplay" },
	{ }
};

static void print_gap_set_io_capa_cmd(const void *data, uint16_t size)
{
	const struct btp_gap_set_io_capa_cp *cp = data;

	bt_shell_printf("\tIO Capa: %s (%d)\n",
			get_indexstr(cp->capa, gap_io_capa_table), cp->capa);
}

static void print_gap_pair_cmd(const void *data, uint16_t size)
{
	const struct btp_gap_pair_cp *cp = data;

	print_bdaddr(&cp->address, cp->address_type);
}

static void print_gap_unpair_cmd(const void *data, uint16_t size)
{
	const struct btp_gap_unpair_cp *cp = data;

	print_bdaddr(&cp->address, cp->address_type);
}

static void print_gap_passkey_entry_response_cmd(const void *data,
								uint16_t size)
{
	const struct btp_gap_passkey_entry_rsp_cp *cp = data;

	print_bdaddr(&cp->address, cp->address_type);
	bt_shell_printf("\tPasskey: %d\n", le32_to_cpu(cp->passkey));
}

static void print_gap_passkey_confirmation_response_cmd(const void *data,
								uint16_t size)
{
	const struct btp_gap_passkey_confirm_rsp_cp *cp = data;

	print_bdaddr(&cp->address, cp->address_type);
	bt_shell_printf("\tMatch: %d\n", cp->match);
}

static void print_gap_new_settings_evt(const void *data, uint16_t size)
{
	const struct btp_new_settings_ev *ev = data;

	print_gap_settings(le32_to_cpu(ev->current_settings),
						gap_setting_table, "\t");
}

static void print_gap_eir(const uint8_t *eir, uint16_t eir_len,
							const char *prefix)
{
	char str[64];
	int i, n;

	if (eir_len == 0) {
		bt_shell_printf("%sEIR Data: Empty\n", prefix);
		return;
	}

	bt_shell_printf("%sEIR Data:\n", prefix);
	for (i = 0, n = 0; i < eir_len; i++) {
		n += sprintf(str + n, "%02x ", eir[i]);
		if ((i % 16) == 15) {
			str[n] = '\0';
			bt_shell_printf("\t%s%s\n", prefix, str);
			n = 0;
		}
	}
}

static const struct bitfield_data gap_device_found_flags_table[] = {
	{ 0, "RSSI Valid" },
	{ 1, "Adv_Data Included" },
	{ 2, "Scan_Rsp Included" },
	{ }
};

static void print_gap_device_found_evt(const void *data, uint16_t size)
{
	const struct btp_device_found_ev *ev = data;

	print_bdaddr(&ev->address, ev->address_type);
	bt_shell_printf("\tRSSI: %d\n", ev->rssi);
	bt_shell_printf("\tFlags:\n");
	print_bitfield(ev->flags, gap_device_found_flags_table, "\t\t");
	print_gap_eir(ev->eir, ev->eir_len, "\t");
}

static void print_gap_device_connected_evt(const void *data, uint16_t size)
{
	const struct btp_gap_device_connected_ev *ev = data;

	print_bdaddr(&ev->address, ev->address_type);
}

static void print_gap_device_disconnected_evt(const void *data, uint16_t size)
{
	const struct btp_gap_device_disconnected_ev *ev = data;

	print_bdaddr(&ev->address, ev->address_type);
}

static void print_gap_passkey_display_evt(const void *data, uint16_t size)
{
	const struct btp_gap_passkey_display_ev *ev = data;

	print_bdaddr(&ev->address, ev->address_type);
	bt_shell_printf("\tPasskey: %d\n", le32_to_cpu(ev->passkey));
}

static void print_gap_passkey_enter_request_evt(const void *data, uint16_t size)
{
	const struct btp_gap_passkey_req_ev *ev = data;

	print_bdaddr(&ev->address, ev->address_type);
}

static void print_gap_passkey_confirm_request_evt(const void *data,
								uint16_t size)
{
	const struct btp_gap_passkey_confirm_ev *ev = data;

	print_bdaddr(&ev->address, ev->address_type);
	bt_shell_printf("\tPasskey: %d\n", le32_to_cpu(ev->passkey));
}

static void print_gap_identity_resolved_evt(const void *data, uint16_t size)
{
	const struct btp_gap_identity_resolved_ev *ev = data;

	print_bdaddr(&ev->address, ev->address_type);
	bt_shell_printf("\tIdentity: ");
	print_bdaddr(&ev->identity_address, ev->identity_address_type);
}


static const struct opcode_data opcode_table_gap[] = {
	{ 0x00, 0, "Error",
			null_cmd, 0, true,
			print_error_rsp, 1, true },
	{ 0x01, 1, "Read Supported Commands",
			null_cmd, 0, true,
			print_gap_read_supported_commands_rsp, 2, true },
	{ 0x02, 2, "Read Controller Index List",
			null_cmd, 0, true,
			print_gap_read_controller_index_list_rsp, 2, false },
	{ 0x03, 3, "Read Controller Information",
			null_cmd, 0, true,
			print_gap_read_controller_information_rsp, 277, true },
	{ 0x04, 4, "Reset",
			null_cmd, 0, true,
			print_gap_reset_rsp, 4, true },
	{ 0x05, 5, "Set Powered",
			print_gap_set_powered_cmd, 1, true,
			print_gap_set_powered_rsp, 4, true },
	{ 0x06, 6, "Set Connectable",
			print_gap_set_connectable_cmd, 1, true,
			print_gap_set_connectable_rsp, 4, true },
	{ 0x07, 7, "Set Fast Connectable",
			print_gap_set_fast_connectable_cmd, 1, true,
			print_gap_set_fast_connectable_rsp, 4, true },
	{ 0x08, 8, "Set Discoverable",
			print_gap_set_discoverable_cmd, 1, true,
			print_gap_set_discoverable_rsp, 4, true },
	{ 0x09, 9, "Set Bondable",
			print_gap_set_bondable_cmd, 1, true,
			print_gap_set_bondable_rsp, 4, true },
	{ 0x0a, 10, "Starting Advertising",
			print_gap_start_advertising_cmd, 2, false,
			print_gap_start_advertising_rsp, 4, true },
	{ 0x0b, 11, "Stop Advertising",
			null_cmd, 0, true,
			print_gap_stop_advertising_rsp, 4, true },
	{ 0x0c, 12, "Start Discovery",
			print_gap_start_discovery_cmd, 1, true,
			null_rsp, 0, true },
	{ 0x0d, 13, "Stop Discovery",
			null_cmd, 0, true,
			null_rsp, 0, true },
	{ 0x0e, 14, "Connect",
			print_gap_connect_cmd, 7, true,
			null_rsp, 0, true },
	{ 0x0f, 15, "Disconnect",
			print_gap_disconnect_cmd, 7, true,
			null_rsp, 0, true },
	{ 0x10, 16, "Set I/O Capability",
			print_gap_set_io_capa_cmd, 1, true,
			null_rsp, 0, true },
	{ 0x11, 17, "Pair",
			print_gap_pair_cmd, 7, true,
			null_rsp, 0, true },
	{ 0x12, 18, "Unpair",
			print_gap_unpair_cmd, 7, true,
			null_rsp, 0, true },
	{ 0x13, 19, "Passkey Entry Response",
			print_gap_passkey_entry_response_cmd, 11, true,
			null_rsp, 0, true },
	{ 0x14, 20, "Passkey Confirmation Response",
			print_gap_passkey_confirmation_response_cmd, 8, true,
			null_rsp, 0, true },
	{ 0x80, -1, "New Settings",
			null_cmd, 0, true,
			null_rsp, 0, true,
			print_gap_new_settings_evt, 4, true },
	{ 0x81, -1, "Device Found",
			null_cmd, 0, true,
			null_rsp, 0, true,
			print_gap_device_found_evt, 11, false },
	{ 0x82, -1, "Device Connected",
			null_cmd, 0, true,
			null_rsp, 0, true,
			print_gap_device_connected_evt, 7, true },
	{ 0x83, -1, "Device Disconnected",
			null_cmd, 0, true,
			null_rsp, 0, true,
			print_gap_device_disconnected_evt, 7, true },
	{ 0x84, -1, "Passkey Display",
			null_cmd, 0, true,
			null_rsp, 0, true,
			print_gap_passkey_display_evt, 11, true },
	{ 0x85, -1, "Passkey Entry Request",
			null_cmd, 0, true,
			null_rsp, 0, true,
			print_gap_passkey_enter_request_evt, 7, true },
	{ 0x86, -1, "Passkey Confirm Request",
			null_cmd, 0, true,
			null_rsp, 0, true,
			print_gap_passkey_confirm_request_evt, 11, true },
	{ 0x87, -1, "Identity Resolved",
			null_cmd, 0, true,
			null_rsp, 0, true,
			print_gap_identity_resolved_evt, 14, true },
	{ }
};

static const struct service_data service_table[] = {
	{ 0x00, 0, "Core", opcode_table_core},
	{ 0x01, 1, "GAP", opcode_table_gap},
	{ }
};

static bool write_packet(int fd, const void *data, size_t size)
{
	while (size > 0) {
		ssize_t written;

		written = write(fd, data, size);
		if (written < 0) {
			if (errno == EAGAIN || errno == EINTR)
				continue;
			return false;
		}

		if (btpclientctl->enable_dump)
			util_hexdump('<', data, written, hexdump_print,
								"OUT: ");

		data += written;
		size -= written;
	}

	return true;
}

static void btp_print_cmd(struct btp_hdr *btp_hdr, void *data)
{
	const struct service_data *table;
	const struct opcode_data *opcode_data;

	table = find_service_data(btp_hdr->service);
	if (!table) {
		bt_shell_printf("Unknown Service: 0x%02x\n", btp_hdr->service);
		return;
	}

	opcode_data = find_opcode_data(btp_hdr->opcode, table->opcode_table);
	if (!opcode_data) {
		bt_shell_printf("Unknown Opcode: 0x%02x\n", btp_hdr->opcode);
		return;
	}

	print_btp_hdr(btp_hdr, "CMD", opcode_data->str);

	if (opcode_data->cmd_fixed) {
		if (btp_hdr->data_len != opcode_data->cmd_size) {
			bt_shell_printf("Invalid Parameter length %d\n",
							btp_hdr->data_len);
			return;
		}
	} else {
		if (btp_hdr->data_len < opcode_data->cmd_size) {
			bt_shell_printf("Invalid Parameter length %d\n",
							btp_hdr->data_len);
			return;
		}
	}

	opcode_data->cmd_func(data, btp_hdr->data_len);
}

static void btp_print_rsp(struct btp_hdr *btp_hdr, void *data)
{
	const struct service_data *table;
	const struct opcode_data *opcode_data;

	table = find_service_data(btp_hdr->service);
	if (!table) {
		bt_shell_printf("Unknown Service: 0x%02x\n", btp_hdr->service);
		return;
	}

	opcode_data = find_opcode_data(btp_hdr->opcode, table->opcode_table);
	if (!opcode_data) {
		bt_shell_printf("Unknown Opcode: 0x%02x\n", btp_hdr->opcode);
		return;
	}

	print_btp_hdr(btp_hdr, "RSP", opcode_data->str);

	if (opcode_data->rsp_fixed) {
		if (btp_hdr->data_len != opcode_data->rsp_size) {
			bt_shell_printf("Invalid Parameter length %d\n",
							btp_hdr->data_len);
			return;
		}
	} else {
		if (btp_hdr->data_len < opcode_data->rsp_size) {
			bt_shell_printf("Invalid Parameter length %d\n",
							btp_hdr->data_len);
			return;
		}
	}

	opcode_data->rsp_func(data, btp_hdr->data_len);
}

static void btp_print_evt(struct btp_hdr *btp_hdr, void *data)
{
	const struct service_data *table;
	const struct opcode_data *opcode_data;

	table = find_service_data(btp_hdr->service);
	if (!table) {
		bt_shell_printf("Unknown Service: 0x%02x\n", btp_hdr->service);
		return;
	}

	opcode_data = find_opcode_data(btp_hdr->opcode, table->opcode_table);
	if (!opcode_data) {
		bt_shell_printf("Unknown Opcode: 0x%02x\n", btp_hdr->opcode);
		return;
	}

	print_btp_hdr(btp_hdr, "EVT", opcode_data->str);

	if (opcode_data->evt_fixed) {
		if (btp_hdr->data_len != opcode_data->evt_size) {
			bt_shell_printf("Invalid Parameter length %d\n",
							btp_hdr->data_len);
			return;
		}
	} else {
		if (btp_hdr->data_len < opcode_data->evt_size) {
			bt_shell_printf("Invalid Parameter length %d\n",
							btp_hdr->data_len);
			return;
		}
	}

	opcode_data->evt_func(data, btp_hdr->data_len);
}

static bool send_cmd(uint8_t service_id, uint8_t opcode, uint8_t index,
					uint16_t data_len, void *data)
{
	struct btp_hdr *hdr;
	int client_fd;

	if (!btpclientctl->client_active) {
		bt_shell_printf("ERROR: Client is not active\n");
		return false;
	}

	hdr = (struct btp_hdr *)(btpclientctl->buf);

	hdr->service = service_id;
	hdr->opcode = opcode;
	hdr->index = index;
	hdr->data_len = cpu_to_le16(data_len);
	if (data)
		memcpy(hdr->data, data, data_len);

	btpclientctl->buf_len = sizeof(*hdr) + data_len;

	client_fd = btpclientctl->client_data->fd;

	btp_print_cmd(hdr, data_len ? hdr->data : NULL);

	if (!write_packet(client_fd, btpclientctl->buf,
						btpclientctl->buf_len)) {
		fprintf(stderr, "Failed to send command to client\n");
		mainloop_remove_fd(client_fd);
		return false;
	}

	return true;
}

static void client_read_destroy(void *user_data)
{
	struct client_data *client_data = user_data;

	close(client_data->fd);
	free(client_data);

	btpclientctl->client_active = false;

	bt_shell_printf("Client is disconnected\n");
}

static void client_read_callback(int fd, uint32_t events, void *user_data)
{
	struct client_data *client_data = user_data;
	struct btp_hdr *btp_hdr;
	uint8_t *data, *ptr;
	ssize_t len, pkt_len;

	if (events & (EPOLLERR | EPOLLHUP)) {
		fprintf(stderr, "Error from client connection\n");
		mainloop_remove_fd(client_data->fd);
		return;
	}

	if (events & EPOLLRDHUP) {
		fprintf(stderr, "Remote hangeup of cliient connection\n");
		mainloop_remove_fd(client_data->fd);
		return;
	}

	/* Read incoming packet */
	len = read(client_data->fd, client_data->buf, sizeof(client_data->buf));
	if (len < 0) {
		if (errno == EAGAIN || errno == EINTR)
			return;

		fprintf(stderr, "Read from client descriptor failed\n");
		mainloop_remove_fd(client_data->fd);
		return;
	}

	if (len < (ssize_t)sizeof(struct btp_hdr) - 1)
		return;

	ptr = client_data->buf;

	while (len) {
		btp_hdr = (struct btp_hdr *)ptr;

		pkt_len = sizeof(*btp_hdr) + btp_hdr->data_len;

		if (btpclientctl->enable_dump)
			util_hexdump('>', ptr, pkt_len, hexdump_print, "IN : ");

		if (btp_hdr->data_len)
			data = btp_hdr->data;
		else
			data = NULL;

		if (btp_hdr->opcode < EVT_OPCODE_BASE)
			btp_print_rsp(btp_hdr, data);
		else
			btp_print_evt(btp_hdr, data);

		ptr += pkt_len;
		len -= pkt_len;
	}
}

static struct client_data *setup_client(int client_fd)
{
	struct client_data *client_data;

	client_data = new0(struct client_data, 1);
	if (!client_data)
		return NULL;

	client_data->fd = client_fd;

	mainloop_add_fd(client_data->fd, EPOLLIN | EPOLLRDHUP,
			client_read_callback, client_data, client_read_destroy);

	return client_data;
}

static void server_callback(int fd, uint32_t events, void *user_data)
{
	union {
		struct sockaddr common;
		struct sockaddr_un sun;
		struct sockaddr_in sin;
	} addr;
	socklen_t len;
	int client_fd;
	struct client_data *client_data;
	struct btpclientctl *btpclientctl = user_data;

	if (events & (EPOLLERR | EPOLLHUP)) {
		mainloop_quit();
		return;
	}

	memset(&addr, 0, sizeof(addr));
	len = sizeof(addr);

	if (getsockname(fd, &addr.common, &len) < 0) {
		perror("Failed to get socket name");
		return;
	}

	client_fd = accept(fd, &addr.common, &len);
	if (client_fd < 0) {
		perror("Failed to accept client socket");
		return;
	}

	bt_shell_printf("Client is connected\n");

	/* Setup Client */
	client_data = setup_client(client_fd);
	if (!client_data)  {
		fprintf(stderr, "Failed to setup client\n");
		close(client_fd);
		return;
	}

	btpclientctl->client_data = client_data;
	btpclientctl->client_active = true;
}

static int open_socket(const char *path)
{
	struct sockaddr_un addr;
	size_t len;
	int fd;

	len = strlen(path);
	if (len > sizeof(addr.sun_path) - 1) {
		fprintf(stderr, "Socket path is too long\n");
		return -1;
	}

	unlink(path);

	fd = socket(AF_UNIX, SOCK_STREAM, 0);
	if (fd < 0) {
		perror("Failed to open Unix server socket");
		return -1;
	}

	memset(&addr, 0, sizeof(addr));
	addr.sun_family = AF_UNIX;
	strncpy(addr.sun_path, path, sizeof(addr.sun_path) - 1);

	if (bind(fd, (struct sockaddr *)&addr, sizeof(addr)) < 0) {
		perror("Failed to bind Unix server socket");
		goto error_close;
	}

	if (listen(fd, 1) < 0) {
		perror("Failed to listen Unix server socket");
		goto error_close;
	}

	bt_shell_printf("Waiting for client connection...\n");

	if (chmod(path, 0666) < 0) {
		perror("Failed to change Unix socket file mode");
		goto error_close;
	}

	return fd;

error_close:
	close(fd);
	return -1;
}

static void cmd_core_read_cmds(int argc, char **argv)
{
	if (!send_cmd(BTP_CORE_SERVICE, BTP_OP_CORE_READ_SUPPORTED_COMMANDS,
					BTP_INDEX_NON_CONTROLLER, 0, NULL))
		return bt_shell_noninteractive_quit(EXIT_FAILURE);

	return bt_shell_noninteractive_quit(EXIT_SUCCESS);
}

static void cmd_core_read_services(int argc, char **argv)
{
	if (!send_cmd(BTP_CORE_SERVICE, BTP_OP_CORE_READ_SUPPORTED_SERVICES,
					BTP_INDEX_NON_CONTROLLER, 0, NULL))
		return bt_shell_noninteractive_quit(EXIT_FAILURE);

	return bt_shell_noninteractive_quit(EXIT_SUCCESS);
}

static void cmd_core_register_service(int argc, char **argv)
{
	uint8_t service_id;

	service_id = atoi(argv[1]);
	if (service_id == 0) {
		bt_shell_printf("CORE service is already registered\n");
		return bt_shell_noninteractive_quit(EXIT_FAILURE);
	}

	if (!send_cmd(BTP_CORE_SERVICE, BTP_OP_CORE_REGISTER,
				BTP_INDEX_NON_CONTROLLER, 1, &service_id))
		return bt_shell_noninteractive_quit(EXIT_FAILURE);

	return bt_shell_noninteractive_quit(EXIT_SUCCESS);
}

static void cmd_core_unregister_service(int argc, char **argv)
{
	uint8_t service_id;

	service_id = atoi(argv[1]);
	if (service_id == 0) {
		bt_shell_printf("Cannot unregister CORE service\n");
		return bt_shell_noninteractive_quit(EXIT_FAILURE);
	}

	if (!send_cmd(BTP_CORE_SERVICE, BTP_OP_CORE_UNREGISTER,
				BTP_INDEX_NON_CONTROLLER, 1, &service_id))
		return bt_shell_noninteractive_quit(EXIT_FAILURE);

	return bt_shell_noninteractive_quit(EXIT_SUCCESS);
}

static void cmd_set_index(int argc, char **argv)
{
	uint8_t index;

	bt_shell_printf("Set Default Controller Index\n");

	index = atoi(argv[1]);
	if (index == bt_index) {
		bt_shell_printf("Controller index is already set\n");
		return bt_shell_noninteractive_quit(EXIT_SUCCESS);
	}

	bt_index = index;
	bt_shell_printf("Controller index is updated to 0x%02x\n", bt_index);

	return bt_shell_noninteractive_quit(EXIT_SUCCESS);
}

static void cmd_gap_read_cmds(int argc, char **argv)
{
	if (!send_cmd(BTP_GAP_SERVICE, BTP_OP_GAP_READ_SUPPORTED_COMMANDS,
					BTP_INDEX_NON_CONTROLLER, 0, NULL))
		return bt_shell_noninteractive_quit(EXIT_FAILURE);

	return bt_shell_noninteractive_quit(EXIT_SUCCESS);
}

static void cmd_gap_read_index(int argc, char **argv)
{
	if (!send_cmd(BTP_GAP_SERVICE, BTP_OP_GAP_READ_CONTROLLER_INDEX_LIST,
					BTP_INDEX_NON_CONTROLLER, 0, NULL))
		return bt_shell_noninteractive_quit(EXIT_FAILURE);

	return bt_shell_noninteractive_quit(EXIT_SUCCESS);
}

static void cmd_gap_read_info(int argc, char **argv)
{
	uint8_t index;

	index = atoi(argv[1]);

	if (!send_cmd(BTP_GAP_SERVICE, BTP_OP_GAP_READ_COTROLLER_INFO,
						index, 0, NULL))
		return bt_shell_noninteractive_quit(EXIT_FAILURE);

	return bt_shell_noninteractive_quit(EXIT_SUCCESS);
}

static void cmd_gap_reset(int argc, char **argv)
{
	if (!send_cmd(BTP_GAP_SERVICE, BTP_OP_GAP_RESET, bt_index, 0, NULL))
		return bt_shell_noninteractive_quit(EXIT_FAILURE);

	return bt_shell_noninteractive_quit(EXIT_SUCCESS);
}

static char *gap_on_off_gen(const char *text, int state)
{
	return argument_gen(text, state, on_off_table);
}

static void cmd_gap_power(int argc, char **argv)
{
	struct btp_gap_set_powered_cp cp;

	if (!parse_argument_on_off(argc, argv, &cp.powered))
		return bt_shell_noninteractive_quit(EXIT_FAILURE);

	if (!send_cmd(BTP_GAP_SERVICE, BTP_OP_GAP_SET_POWERED,
						bt_index, sizeof(cp), &cp))
		return bt_shell_noninteractive_quit(EXIT_FAILURE);

	return bt_shell_noninteractive_quit(EXIT_SUCCESS);
}

static void cmd_gap_connectable(int argc, char **argv)
{
	struct btp_gap_set_connectable_cp cp;

	if (!parse_argument_on_off(argc, argv, &cp.connectable))
		return bt_shell_noninteractive_quit(EXIT_FAILURE);

	if (!send_cmd(BTP_GAP_SERVICE, BTP_OP_GAP_SET_CONNECTABLE,
						bt_index, sizeof(cp), &cp))
		return bt_shell_noninteractive_quit(EXIT_FAILURE);

	return bt_shell_noninteractive_quit(EXIT_SUCCESS);
}

static void cmd_gap_fast_connectable(int argc, char **argv)
{
	struct btp_gap_set_fast_connectable_cp cp;

	if (!parse_argument_on_off(argc, argv, &cp.fast_connectable))
		return bt_shell_noninteractive_quit(EXIT_FAILURE);

	if (!send_cmd(BTP_GAP_SERVICE, BTP_OP_GAP_SET_FAST_CONNECTABLE,
						bt_index, sizeof(cp), &cp))
		return bt_shell_noninteractive_quit(EXIT_FAILURE);

	return bt_shell_noninteractive_quit(EXIT_SUCCESS);
}

static char *gap_discoverable_gen(const char *text, int state)
{
	return argument_gen(text, state, gap_discoverable_table);
}

static void cmd_gap_discoverable(int argc, char **argv)
{
	struct btp_gap_set_discoverable_cp cp;

	memset(&cp, 0, sizeof(cp));

	if (!parse_argument_list(argc, argv, &cp.discoverable,
						gap_discoverable_table))
		return bt_shell_noninteractive_quit(EXIT_FAILURE);

	if (!send_cmd(BTP_GAP_SERVICE, BTP_OP_GAP_SET_DISCOVERABLE,
						bt_index, sizeof(cp), &cp))
		return bt_shell_noninteractive_quit(EXIT_FAILURE);

	return bt_shell_noninteractive_quit(EXIT_SUCCESS);
}

static void cmd_gap_bondable(int argc, char **argv)
{
	struct btp_gap_set_bondable_cp cp;

	if (!parse_argument_on_off(argc, argv, &cp.bondable))
		return bt_shell_noninteractive_quit(EXIT_FAILURE);

	if (!send_cmd(BTP_GAP_SERVICE, BTP_OP_GAP_SET_BONDABLE,
						bt_index, sizeof(cp), &cp))
		return bt_shell_noninteractive_quit(EXIT_FAILURE);

	return bt_shell_noninteractive_quit(EXIT_SUCCESS);
}

/*
 * This function converts the advertise data in the format of BT Spec to the
 * format of BTP Spec.
 */
static void ad_convert_data(uint8_t *data, size_t len)
{
	struct ad_struct *ad_struct;
	size_t count = 0;
	uint8_t new_len;

	if (!data || !len)
		return;

	while (count < len) {
		ad_struct = (struct ad_struct *)(data + count);

		/* Swap AD_Type and AD_Len and update AD_Len */
		new_len = ad_struct->length - 1;
		ad_struct->length = ad_struct->type;
		ad_struct->type = new_len;

		count += new_len + 2;
	}
}

static void cmd_gap_start_adv(int argc, char **argv)
{
	struct btp_gap_start_adv_cp *cp;
	uint8_t *ad_data, *scan_data;
	size_t ad_len, scan_len;
	size_t total;
	int status = EXIT_SUCCESS;

	/* Generate advertise data */
	ad_data = bt_ad_generate(advertise_data->ad, &ad_len);
	scan_data = bt_ad_generate(advertise_data->scan, &scan_len);

	if (!ad_data && !scan_data) {
		bt_shell_printf("No Advertise or Scan data available\n");
		status = EXIT_FAILURE;
		goto exit_error_free_data;
	}

	total = ad_len + scan_len + 2;

	cp = (struct btp_gap_start_adv_cp *)malloc(total);
	if (!cp) {
		bt_shell_printf("Failed to allocated buffer\n");
		status = EXIT_FAILURE;
		goto exit_error_free_data;
	}
	memset(cp, 0, total);

	/*
	 * Convert Advertise Data in the format specified in BTP Spec.
	 * BTP uses { AD_Type, AD_Len, AD_Data } struct instead of the format
	 * defined in the BT spec and BlueZ, { AD_Len, AD_Type, AD_Data }.
	 *
	 * In order to comply it, AD_Type and AD_Len need to be swapped and
	 * AD_Len needs be updated to exclude the length of AD_Type.
	 */
	ad_convert_data(ad_data, ad_len);
	ad_convert_data(scan_data, scan_len);

	/* Copy Advertise data */
	cp->adv_data_len = ad_len;
	memcpy(cp->data, ad_data, ad_len);

	/* Copy Scan Response data */
	cp->scan_rsp_len = scan_len;
	memcpy(cp->data + ad_len, scan_data, scan_len);

	if (!send_cmd(BTP_GAP_SERVICE, BTP_OP_GAP_START_ADVERTISING,
						bt_index, total, cp)) {
		status = EXIT_FAILURE;
		goto exit_error_free_cp;
	}

exit_error_free_cp:
	free(cp);
exit_error_free_data:
	free(ad_data);
	free(scan_data);

	return bt_shell_noninteractive_quit(status);
}

static void cmd_gap_stop_adv(int argc, char **argv)
{
	if (!send_cmd(BTP_GAP_SERVICE, BTP_OP_GAP_STOP_ADVERTISING,
						bt_index, 0, NULL))
		return bt_shell_noninteractive_quit(EXIT_FAILURE);

	return bt_shell_noninteractive_quit(EXIT_SUCCESS);
}

static char *gap_start_disc_gen(const char *text, int state)
{
	return argument_gen_bitfield(text, state, gap_discovery_flags_table);
}

static void cmd_gap_start_disc(int argc, char **argv)
{
	struct btp_gap_start_discovery_cp cp;
	int i;
	uint32_t f;

	memset(&cp, 0, sizeof(cp));

	for (i = 1; i < argc; i++) {
		if (!parse_argument_bitfield_list(argc - i, &argv[i], &f,
						gap_discovery_flags_table))
			return bt_shell_noninteractive_quit(EXIT_FAILURE);
		cp.flags |= (1 << f);
	}

	if (!send_cmd(BTP_GAP_SERVICE, BTP_OP_GAP_START_DISCOVERY,
						bt_index, sizeof(cp), &cp))
		return bt_shell_noninteractive_quit(EXIT_FAILURE);

	return bt_shell_noninteractive_quit(EXIT_SUCCESS);
}

static void cmd_gap_stop_disc(int argc, char **argv)
{
	if (!send_cmd(BTP_GAP_SERVICE, BTP_OP_GAP_STOP_DISCOVERY,
						bt_index, 0, NULL))
		return bt_shell_noninteractive_quit(EXIT_FAILURE);

	return bt_shell_noninteractive_quit(EXIT_SUCCESS);
}

static void cmd_gap_connect(int argc, char **argv)
{
	struct btp_gap_connect_cp cp;

	memset(&cp, 0, sizeof(cp));

	if (!parse_argument_addr(argc, argv, &cp.address_type, &cp.address))
		return bt_shell_noninteractive_quit(EXIT_FAILURE);

	if (!send_cmd(BTP_GAP_SERVICE, BTP_OP_GAP_CONNECT,
						bt_index, sizeof(cp), &cp))
		return bt_shell_noninteractive_quit(EXIT_FAILURE);

	return bt_shell_noninteractive_quit(EXIT_SUCCESS);
}

static void cmd_gap_disconnect(int argc, char **argv)
{
	struct btp_gap_disconnect_cp cp;

	memset(&cp, 0, sizeof(cp));

	if (!parse_argument_addr(argc, argv, &cp.address_type, &cp.address))
		return bt_shell_noninteractive_quit(EXIT_FAILURE);

	if (!send_cmd(BTP_GAP_SERVICE, BTP_OP_GAP_DISCONNECT,
						bt_index, sizeof(cp), &cp))
		return bt_shell_noninteractive_quit(EXIT_FAILURE);

	return bt_shell_noninteractive_quit(EXIT_SUCCESS);
}

static char *gap_io_capa_gen(const char *text, int state)
{
	return argument_gen(text, state, gap_io_capa_table);
}

static void cmd_gap_set_io_capa(int argc, char **argv)
{
	struct btp_gap_set_io_capa_cp cp;

	memset(&cp, 0, sizeof(cp));

	if (argc != 2) {
		bt_shell_printf("Invalid parameter\n");
		return bt_shell_noninteractive_quit(EXIT_FAILURE);
	}

	if (!parse_argument_list(argc, argv, &cp.capa, gap_io_capa_table))
		return bt_shell_noninteractive_quit(EXIT_FAILURE);

	if (!send_cmd(BTP_GAP_SERVICE, BTP_OP_GAP_SET_IO_CAPA,
						bt_index, sizeof(cp), &cp))
		return bt_shell_noninteractive_quit(EXIT_FAILURE);

	return bt_shell_noninteractive_quit(EXIT_SUCCESS);
}

static void cmd_gap_pair(int argc, char **argv)
{
	struct btp_gap_pair_cp cp;

	memset(&cp, 0, sizeof(cp));

	if (!parse_argument_addr(argc, argv, &cp.address_type, &cp.address))
		return bt_shell_noninteractive_quit(EXIT_FAILURE);

	if (!send_cmd(BTP_GAP_SERVICE, BTP_OP_GAP_PAIR,
						bt_index, sizeof(cp), &cp))
		return bt_shell_noninteractive_quit(EXIT_FAILURE);

	return bt_shell_noninteractive_quit(EXIT_SUCCESS);
}

static void cmd_gap_unpair(int argc, char **argv)
{
	struct btp_gap_unpair_cp cp;

	memset(&cp, 0, sizeof(cp));

	if (!parse_argument_addr(argc, argv, &cp.address_type, &cp.address))
		return bt_shell_noninteractive_quit(EXIT_FAILURE);

	if (!send_cmd(BTP_GAP_SERVICE, BTP_OP_GAP_UNPAIR,
						bt_index, sizeof(cp), &cp))
		return bt_shell_noninteractive_quit(EXIT_FAILURE);

	return bt_shell_noninteractive_quit(EXIT_SUCCESS);
}

static bool ad_add_data(struct ad_data *data, int argc, char *argv[])
{
	char *endptr = NULL;
	int i;
	long value;

	memset(data, 0, sizeof(*data));

	for (i = 0; i < argc; i++) {
		endptr = NULL;

		value = strtol(argv[i], &endptr, 0);
		if (!endptr || *endptr != '\0' || value > UINT8_MAX) {
			bt_shell_printf("Invalid data index at %d\n", i);
			return false;
		}

		data->data[data->len] = value;
		data->len++;
	}

	return true;
}

static void cmd_ad_show(int argc, char **argv)
{
	uint8_t *data;
	size_t data_len;

	bt_shell_printf("Advertise Data:\n");
	data = bt_ad_generate(advertise_data->ad, &data_len);
	print_ad_data(data, data_len);

	bt_shell_printf("Scan Response Data:\n");
	data = bt_ad_generate(advertise_data->scan, &data_len);
	print_ad_data(data, data_len);

	return bt_shell_noninteractive_quit(EXIT_SUCCESS);
}

static void cmd_ad_name(int argc, char **argv)
{
	if (argc < 2) {
		cmd_ad_show(argc, argv);
		return bt_shell_noninteractive_quit(EXIT_SUCCESS);
	}

	if (!bt_ad_add_name(advertise_data->ad, argv[1])) {
		bt_shell_printf("Failed to add local name\n");
		return bt_shell_noninteractive_quit(EXIT_FAILURE);
	}

	cmd_ad_show(argc, argv);

	return bt_shell_noninteractive_quit(EXIT_SUCCESS);
}

static void cmd_ad_appearance(int argc, char **argv)
{
	long value;
	char *endptr = NULL;

	if (argc < 2) {
		cmd_ad_show(argc, argv);
		return bt_shell_noninteractive_quit(EXIT_SUCCESS);
	}

	value = strtol(argv[1], &endptr, 0);
	if (!endptr || *endptr != '\0' || value > UINT16_MAX) {
		bt_shell_printf("Invalid argument: %s\n", argv[1]);
		return bt_shell_noninteractive_quit(EXIT_FAILURE);
	}

	if (!bt_ad_add_appearance(advertise_data->ad, value)) {
		bt_shell_printf("Failed to add appearance\n");
		return bt_shell_noninteractive_quit(EXIT_FAILURE);
	}

	cmd_ad_show(argc, argv);

	return bt_shell_noninteractive_quit(EXIT_SUCCESS);
}

static void cmd_ad_uuids(int argc, char **argv)
{
	int i;

	if (argc < 2) {
		cmd_ad_show(argc, argv);
		return bt_shell_noninteractive_quit(EXIT_SUCCESS);
	}

	for (i = 1; i < argc; i++) {
		bt_uuid_t uuid;

		if (bt_string_to_uuid(&uuid, argv[i]) < 0) {
			bt_shell_printf("Invalid argument: %s\n", argv[i]);
			goto fail;
		}

		if (!bt_ad_add_service_uuid(advertise_data->ad, &uuid)) {
			bt_shell_printf("Failed to add service uuid\n");
			goto fail;
		}
	}

	cmd_ad_show(argc, argv);

	return bt_shell_noninteractive_quit(EXIT_SUCCESS);
fail:
	bt_ad_clear_service_uuid(advertise_data->ad);

	return bt_shell_noninteractive_quit(EXIT_FAILURE);
}

static void cmd_ad_manufacturer(int argc, char **argv)
{
	long value;
	char *endptr = NULL;
	struct ad_data data;

	if (argc < 2) {
		cmd_ad_show(argc, argv);
		return bt_shell_noninteractive_quit(EXIT_SUCCESS);
	}

	value = strtol(argv[1], &endptr, 0);
	if (!endptr || *endptr != '\0' || value > UINT16_MAX) {
		bt_shell_printf("Invalid manufacture id: %s\n", argv[1]);
		return bt_shell_noninteractive_quit(EXIT_FAILURE);
	}

	if (!ad_add_data(&data, argc-2, argv + 2))
		return bt_shell_noninteractive_quit(EXIT_FAILURE);

	if (!bt_ad_add_manufacturer_data(advertise_data->ad, value,
							data.data, data.len)) {
		bt_shell_printf("Failed to add manufacturer data\n");
		return bt_shell_noninteractive_quit(EXIT_FAILURE);
	}

	cmd_ad_show(argc, argv);

	return bt_shell_noninteractive_quit(EXIT_SUCCESS);
}

static void cmd_ad_solicit(int argc, char **argv)
{
	int i;

	if (argc < 2) {
		cmd_ad_show(argc, argv);
		return bt_shell_noninteractive_quit(EXIT_SUCCESS);
	}

	for (i = 1; i < argc; i++) {
		bt_uuid_t uuid;

		if (bt_string_to_uuid(&uuid, argv[i]) < 0) {
			bt_shell_printf("Invalid argument: %s\n", argv[i]);
			goto fail;
		}

		if (!bt_ad_add_solicit_uuid(advertise_data->ad, &uuid)) {
			bt_shell_printf("Failed to add service uuid\n");
			goto fail;
		}
	}

	cmd_ad_show(argc, argv);

	return bt_shell_noninteractive_quit(EXIT_SUCCESS);
fail:
	bt_ad_clear_solicit_uuid(advertise_data->ad);

	return bt_shell_noninteractive_quit(EXIT_FAILURE);
}

static void cmd_ad_service_data(int argc, char **argv)
{
	bt_uuid_t uuid;
	struct ad_data data;

	if (argc < 2) {
		cmd_ad_show(argc, argv);
		return bt_shell_noninteractive_quit(EXIT_SUCCESS);
	}

	if (bt_string_to_uuid(&uuid, argv[1]) < 0) {
		bt_shell_printf("Invalid argument: %s\n", argv[1]);
		goto fail;
	}

	if (!ad_add_data(&data, argc-2, argv + 2))
		return bt_shell_noninteractive_quit(EXIT_FAILURE);

	if (!bt_ad_add_service_data(advertise_data->ad, &uuid,
							data.data, data.len)) {
		bt_shell_printf("Failed to add service data\n");
		return bt_shell_noninteractive_quit(EXIT_FAILURE);
	}

	cmd_ad_show(argc, argv);

	return bt_shell_noninteractive_quit(EXIT_SUCCESS);
fail:
	bt_ad_clear_service_data(advertise_data->ad);

	return bt_shell_noninteractive_quit(EXIT_FAILURE);
}

static void cmd_ad_data(int argc, char **argv)
{
	long value;
	char *endptr = NULL;
	struct ad_data data;

	if (argc < 2) {
		cmd_ad_show(argc, argv);
		return bt_shell_noninteractive_quit(EXIT_SUCCESS);
	}

	value = strtol(argv[1], &endptr, 0);
	if (!endptr || *endptr != '\0' || value > UINT8_MAX) {
		bt_shell_printf("Invalid data type: %s\n", argv[1]);
		return bt_shell_noninteractive_quit(EXIT_FAILURE);
	}

	if (!ad_add_data(&data, argc-2, argv + 2))
		return bt_shell_noninteractive_quit(EXIT_FAILURE);

	if (!bt_ad_add_data(advertise_data->ad, value, data.data, data.len)) {
		bt_shell_printf("Failed to add data\n");
		return bt_shell_noninteractive_quit(EXIT_FAILURE);
	}

	cmd_ad_show(argc, argv);

	return bt_shell_noninteractive_quit(EXIT_SUCCESS);
}

static void cmd_scan_rsp_name(int argc, char **argv)
{
	if (argc < 2) {
		cmd_ad_show(argc, argv);
		return bt_shell_noninteractive_quit(EXIT_SUCCESS);
	}

	if (!bt_ad_add_name(advertise_data->scan, argv[1])) {
		bt_shell_printf("Failed to add local name\n");
		return bt_shell_noninteractive_quit(EXIT_FAILURE);
	}

	cmd_ad_show(argc, argv);

	return bt_shell_noninteractive_quit(EXIT_SUCCESS);
}

static void ad_clear_name(void)
{
	bt_ad_clear_name(advertise_data->ad);
}

static void ad_clear_appearance(void)
{
	bt_ad_clear_appearance(advertise_data->ad);
}

static void ad_clear_uuids(void)
{
	bt_ad_clear_service_uuid(advertise_data->ad);
}

static void ad_clear_manufacturer(void)
{
	bt_ad_clear_manufacturer_data(advertise_data->ad);
}

static void ad_clear_solicit(void)
{
	bt_ad_clear_solicit_uuid(advertise_data->ad);
}

static void ad_clear_service(void)
{
	bt_ad_clear_service_data(advertise_data->ad);
}

static void ad_clear_data(void)
{
	bt_ad_clear_data(advertise_data->ad);
}

static void ad_clear_scan_rsp(void)
{
	bt_ad_clear_name(advertise_data->scan);
}

static void ad_clear_all(void)
{
	ad_clear_name();
	ad_clear_appearance();
	ad_clear_uuids();
	ad_clear_manufacturer();
	ad_clear_solicit();
	ad_clear_service();
	ad_clear_data();
	ad_clear_scan_rsp();
}

static const struct name_func_entry ad_clear_entry_table[] = {
	{ "name",		ad_clear_name },
	{ "appearance",		ad_clear_appearance },
	{ "uuids",		ad_clear_uuids },
	{ "manufacturer",	ad_clear_manufacturer },
	{ "solicit",		ad_clear_solicit },
	{ "service",		ad_clear_service },
	{ "data",		ad_clear_data },
	{ "scan-rsp",		ad_clear_scan_rsp },
	{ "all",		ad_clear_all },
	{ }
};

static void cmd_ad_clear(int argc, char **argv)
{
	const struct name_func_entry *entry;

	if (argc < 2) {
		ad_clear_all();
		return bt_shell_noninteractive_quit(EXIT_SUCCESS);
	}

	for (entry = ad_clear_entry_table; entry && entry->name; entry++) {
		if (!strcmp(entry->name, argv[1])) {
			entry->func();
			goto done;
		}
	}

	bt_shell_printf("Invalid argument: %s\n", argv[1]);

	return bt_shell_noninteractive_quit(EXIT_FAILURE);
done:
	cmd_ad_show(argc, argv);

	return bt_shell_noninteractive_quit(EXIT_SUCCESS);
}

static char *ad_clear_gen(const char *text, int state)
{
	return argument_gen_name_func_entry(text, state, ad_clear_entry_table);
}

static const struct bt_shell_menu ad_menu = {
	.name = "ad",
	.desc = "Advertise Options Submenu",
	.entries = {
	{ "show",		NULL,
		cmd_ad_show,		"Show current saved advertise data" },
	{ "name",		"[name]",
		cmd_ad_name,		"Set/Get the local name" },
	{ "appearance",		"[value]",
		cmd_ad_appearance,	"Set/Get the appearance" },
	{ "uuids",		"[uuid ...]",
		cmd_ad_uuids,		"Set/Get advertise service uuids" },
	{ "manufacturer",	"[id] [data=xx xx ...]",
		cmd_ad_manufacturer,	"Set/Get manufacturer data" },
	{ "solicit",		"[uuid ...]",
		cmd_ad_solicit,	"Set/Get solicit uuids" },
	{ "service",		"[uuid] [data=xx xx ...]",
		cmd_ad_service_data,	"Set/Get service data" },
	{ "data",		"[type] [data=xx xx ...]",
		cmd_ad_data,		"Set/Get advertise data" },
	{ "scan-rsp",		"[name]",
		cmd_scan_rsp_name,	"Set/Get scan rsp data with name" },
	{ "clear",		"[all/name/appearance/uuids/...]",
		cmd_ad_clear,		"Clear advertise configuration",
					ad_clear_gen },
	{ } },
};

static const struct bt_shell_menu gap_menu = {
	.name = "gap",
	.desc = "GAP API Submenu",
	.entries = {
	{ "read-cmds",		NULL,
		cmd_gap_read_cmds,	"Show supported commands" },
	{ "list",		NULL,
		cmd_gap_read_index,	"Show index of controllers" },
	{ "read-info",		"<index>",
		cmd_gap_read_info,	"Read controller information" },
	{ "reset",		NULL,
		cmd_gap_reset,		"Reset controller and stack" },
	{ "power",		"<on/off>",
		cmd_gap_power,		"Set controller power",
					gap_on_off_gen },
	{ "connectable",	"<on/off>",
		cmd_gap_connectable,	"Set controller connectable",
					gap_on_off_gen },
	{ "fast-connectable",	"<on/off>",
		cmd_gap_fast_connectable, "Set controller fast connectable",
					gap_on_off_gen },
	{ "discoverable",	"<on/off/limited>",
		cmd_gap_discoverable,	"Set controller discoverable",
					gap_discoverable_gen },
	{ "bondable",		"<on/off>",
		cmd_gap_bondable,	"Set controller bondable",
					gap_on_off_gen },
	{ "start-adv",		NULL,
		cmd_gap_start_adv,	"Start Advertising" },
	{ "stop-adv",		NULL,
		cmd_gap_stop_adv,	"Stop Advertising" },
	{ "start-disc",		"<flag ...>",
		cmd_gap_start_disc,	"Start discovery",
					gap_start_disc_gen },
	{ "stop-disc",		NULL,
		cmd_gap_stop_disc,	"Stop discovery" },
	{ "connect",		"<type> <bdaddr>",
		cmd_gap_connect,	"Connect" },
	{ "disconnect",		"<type> <bdaddr>",
		cmd_gap_disconnect,	"Disconnect" },
	{ "set-capa",		"<io capability>",
		cmd_gap_set_io_capa,	"Set IO capability",
					gap_io_capa_gen },
	{ "pair",		"<type> <bdaddr>",
		cmd_gap_pair,		"Pair" },
	{ "unpair",		"<type> <bdaddr>",
		cmd_gap_unpair,		"Unpair" },
	{ } },
};

static const struct bt_shell_menu main_menu = {
	.name = "main",
	.entries =  {
	{ "read-cmds",		NULL,
		cmd_core_read_cmds,		"Read supported commands" },
	{ "read-services",	NULL,
		cmd_core_read_services,		"Read supported services" },
	{ "register",		"<service_id>",
		cmd_core_register_service,	"Register service" },
	{ "unregister",		"<service_id>",
		cmd_core_unregister_service,	"Unregister service" },
	{ "index",		"<index>",
		cmd_set_index,		"Set controller index. Default is 0" },
	{ } },
};

static const struct option main_options[] = {
	{ "socket",	required_argument, 0, 's' },
	{ "dump  ",	required_argument, 0, 'd' },
	{ 0, 0, 0, 0 }
};

static const char *socket_path_option;
static const char *dump_option;

static const char **optargs[] = {
	&socket_path_option,
	&dump_option,
};

static const char *help[] = {
	"Socket path to listen for BTP client\n",
	"Use \"on\" to enable hex dump\n"
};

static const struct bt_shell_opt opt = {
	.options = main_options,
	.optno = sizeof(main_options) / sizeof(struct option),
	.optstr = "s:d:",
	.optarg = optargs,
	.help = help,
};

int main(int argc, char *argv[])
{
	int status;
	int server_fd;

	bt_shell_init(argc, argv, &opt);
	bt_shell_set_menu(&main_menu);
	bt_shell_add_submenu(&ad_menu);
	bt_shell_add_submenu(&gap_menu);

	btpclientctl = new0(struct btpclientctl, 1);
	if (!btpclientctl) {
		bt_shell_printf("Failed to allocate btpclientctl\n");
		status = EXIT_FAILURE;
		goto error_exit;
	}

	if (socket_path_option)
		btpclientctl->socket_path = strdup(socket_path_option);
	else
		btpclientctl->socket_path = strdup(DEFAULT_SOCKET_PATH);

	if (dump_option && !strcasecmp(dump_option, "on"))
		btpclientctl->enable_dump = true;
	else
		btpclientctl->enable_dump = false;

	advertise_data = new0(struct advertise_data, 1);
	if (!advertise_data) {
		status = EXIT_FAILURE;
		goto error_free_clientctl;
	}

	advertise_data->ad = bt_ad_new();
	if (!advertise_data->ad) {
		status = EXIT_FAILURE;
		goto error_free_advertise_data;
	}

	advertise_data->scan = bt_ad_new();
	if (!advertise_data->scan) {
		status = EXIT_FAILURE;
		goto error_free_ad;
	}

	bt_shell_attach(fileno(stdin));

	server_fd = open_socket(btpclientctl->socket_path);
	if (server_fd < 0) {
		status = EXIT_FAILURE;
		goto error_free_scan;
	}

	btpclientctl->server_fd = server_fd;

	mainloop_add_fd(btpclientctl->server_fd, EPOLLIN, server_callback,
			btpclientctl, NULL);

	bt_shell_set_prompt(PROMPT_ON);

	status = bt_shell_run();

	close(btpclientctl->server_fd);
error_free_scan:
	bt_ad_unref(advertise_data->scan);
error_free_ad:
	bt_ad_unref(advertise_data->ad);
error_free_advertise_data:
	free(advertise_data);
error_free_clientctl:
	free(btpclientctl->socket_path);
	free(btpclientctl);
error_exit:
	return status;
}

/*
 *  BlueZ - Bluetooth protocol stack for Linux
 *
 *  Copyright (C) 2010  Nokia Corporation
 *  Copyright (C) 2010  Marcel Holtmann <marcel@holtmann.org>
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

#ifndef __packed
#define __packed __attribute__((packed))
#endif

#define MGMT_INDEX_NONE			0xFFFF

#define MGMT_STATUS_SUCCESS		0x00
#define MGMT_STATUS_UNKNOWN_COMMAND	0x01
#define MGMT_STATUS_NOT_CONNECTED	0x02
#define MGMT_STATUS_FAILED		0x03
#define MGMT_STATUS_CONNECT_FAILED	0x04
#define MGMT_STATUS_AUTH_FAILED		0x05
#define MGMT_STATUS_NOT_PAIRED		0x06
#define MGMT_STATUS_NO_RESOURCES	0x07
#define MGMT_STATUS_TIMEOUT		0x08
#define MGMT_STATUS_ALREADY_CONNECTED	0x09
#define MGMT_STATUS_BUSY		0x0a
#define MGMT_STATUS_REJECTED		0x0b
#define MGMT_STATUS_NOT_SUPPORTED	0x0c
#define MGMT_STATUS_INVALID_PARAMS	0x0d
#define MGMT_STATUS_DISCONNECTED	0x0e
#define MGMT_STATUS_NOT_POWERED		0x0f
#define MGMT_STATUS_CANCELLED		0x10
#define MGMT_STATUS_INVALID_INDEX	0x11
#define MGMT_STATUS_RFKILLED		0x12
#define MGMT_STATUS_ALREADY_PAIRED	0x13
#define MGMT_STATUS_PERMISSION_DENIED	0x14

struct mgmt_hdr {
	uint16_t opcode;
	uint16_t index;
	uint16_t len;
} __packed;
#define MGMT_HDR_SIZE	6

struct mgmt_addr_info {
	bdaddr_t bdaddr;
	uint8_t type;
} __packed;

#define MGMT_OP_READ_VERSION		0x0001
struct mgmt_rp_read_version {
	uint8_t version;
	uint16_t revision;
} __packed;

#define MGMT_OP_READ_COMMANDS		0x0002
struct mgmt_rp_read_commands {
	uint16_t num_commands;
	uint16_t num_events;
	uint16_t opcodes[0];
} __packed;

#define MGMT_OP_READ_INDEX_LIST		0x0003
struct mgmt_rp_read_index_list {
	uint16_t num_controllers;
	uint16_t index[0];
} __packed;

/* Reserve one extra byte for names in management messages so that they
 * are always guaranteed to be nul-terminated */
#define MGMT_MAX_NAME_LENGTH		(248 + 1)
#define MGMT_MAX_SHORT_NAME_LENGTH	(10 + 1)

#define MGMT_SETTING_POWERED		0x00000001
#define MGMT_SETTING_CONNECTABLE	0x00000002
#define MGMT_SETTING_FAST_CONNECTABLE	0x00000004
#define MGMT_SETTING_DISCOVERABLE	0x00000008
#define MGMT_SETTING_BONDABLE		0x00000010
#define MGMT_SETTING_LINK_SECURITY	0x00000020
#define MGMT_SETTING_SSP		0x00000040
#define MGMT_SETTING_BREDR		0x00000080
#define MGMT_SETTING_HS			0x00000100
#define MGMT_SETTING_LE			0x00000200
#define MGMT_SETTING_ADVERTISING	0x00000400
#define MGMT_SETTING_SECURE_CONN	0x00000800
#define MGMT_SETTING_DEBUG_KEYS		0x00001000
#define MGMT_SETTING_PRIVACY		0x00002000
#define MGMT_SETTING_CONFIGURATION	0x00004000
#define MGMT_SETTING_STATIC_ADDRESS	0x00008000

#define MGMT_OP_READ_INFO		0x0004
struct mgmt_rp_read_info {
	bdaddr_t bdaddr;
	uint8_t version;
	uint16_t manufacturer;
	uint32_t supported_settings;
	uint32_t current_settings;
	uint8_t dev_class[3];
	uint8_t name[MGMT_MAX_NAME_LENGTH];
	uint8_t short_name[MGMT_MAX_SHORT_NAME_LENGTH];
} __packed;

struct mgmt_mode {
	uint8_t val;
} __packed;

struct mgmt_cod {
	uint8_t val[3];
} __packed;

#define MGMT_OP_SET_POWERED		0x0005

#define MGMT_OP_SET_DISCOVERABLE	0x0006
struct mgmt_cp_set_discoverable {
	uint8_t val;
	uint16_t timeout;
} __packed;

#define MGMT_OP_SET_CONNECTABLE		0x0007

#define MGMT_OP_SET_FAST_CONNECTABLE	0x0008

#define MGMT_OP_SET_BONDABLE		0x0009

#define MGMT_OP_SET_LINK_SECURITY	0x000A

#define MGMT_OP_SET_SSP			0x000B

#define MGMT_OP_SET_HS			0x000C

#define MGMT_OP_SET_LE			0x000D

#define MGMT_OP_SET_DEV_CLASS		0x000E
struct mgmt_cp_set_dev_class {
	uint8_t major;
	uint8_t minor;
} __packed;

#define MGMT_OP_SET_LOCAL_NAME		0x000F
struct mgmt_cp_set_local_name {
	uint8_t name[MGMT_MAX_NAME_LENGTH];
	uint8_t short_name[MGMT_MAX_SHORT_NAME_LENGTH];
} __packed;

#define MGMT_OP_ADD_UUID		0x0010
struct mgmt_cp_add_uuid {
	uint8_t uuid[16];
	uint8_t svc_hint;
} __packed;

#define MGMT_OP_REMOVE_UUID		0x0011
struct mgmt_cp_remove_uuid {
	uint8_t uuid[16];
} __packed;

struct mgmt_link_key_info {
	struct mgmt_addr_info addr;
	uint8_t type;
	uint8_t val[16];
	uint8_t pin_len;
} __packed;

#define MGMT_OP_LOAD_LINK_KEYS		0x0012
struct mgmt_cp_load_link_keys {
	uint8_t debug_keys;
	uint16_t key_count;
	struct mgmt_link_key_info keys[0];
} __packed;

struct mgmt_ltk_info {
	struct mgmt_addr_info addr;
	uint8_t type;
	uint8_t master;
	uint8_t enc_size;
	uint16_t ediv;
	uint64_t rand;
	uint8_t val[16];
} __packed;

#define MGMT_OP_LOAD_LONG_TERM_KEYS	0x0013
struct mgmt_cp_load_long_term_keys {
	uint16_t key_count;
	struct mgmt_ltk_info keys[0];
} __packed;

#define MGMT_OP_DISCONNECT		0x0014
struct mgmt_cp_disconnect {
	struct mgmt_addr_info addr;
} __packed;
struct mgmt_rp_disconnect {
	struct mgmt_addr_info addr;
} __packed;

#define MGMT_OP_GET_CONNECTIONS		0x0015
struct mgmt_rp_get_connections {
	uint16_t conn_count;
	struct mgmt_addr_info addr[0];
} __packed;

#define MGMT_OP_PIN_CODE_REPLY		0x0016
struct mgmt_cp_pin_code_reply {
	struct mgmt_addr_info addr;
	uint8_t pin_len;
	uint8_t pin_code[16];
} __packed;

#define MGMT_OP_PIN_CODE_NEG_REPLY	0x0017
struct mgmt_cp_pin_code_neg_reply {
	struct mgmt_addr_info addr;
} __packed;

#define MGMT_OP_SET_IO_CAPABILITY	0x0018
struct mgmt_cp_set_io_capability {
	uint8_t io_capability;
} __packed;

#define MGMT_OP_PAIR_DEVICE		0x0019
struct mgmt_cp_pair_device {
	struct mgmt_addr_info addr;
	uint8_t io_cap;
} __packed;
struct mgmt_rp_pair_device {
	struct mgmt_addr_info addr;
} __packed;

#define MGMT_OP_CANCEL_PAIR_DEVICE	0x001A

#define MGMT_OP_UNPAIR_DEVICE		0x001B
struct mgmt_cp_unpair_device {
	struct mgmt_addr_info addr;
	uint8_t disconnect;
} __packed;
struct mgmt_rp_unpair_device {
	struct mgmt_addr_info addr;
} __packed;

#define MGMT_OP_USER_CONFIRM_REPLY	0x001C
struct mgmt_cp_user_confirm_reply {
	struct mgmt_addr_info addr;
} __packed;
struct mgmt_rp_user_confirm_reply {
	struct mgmt_addr_info addr;
} __packed;

#define MGMT_OP_USER_CONFIRM_NEG_REPLY	0x001D

#define MGMT_OP_USER_PASSKEY_REPLY	0x001E
struct mgmt_cp_user_passkey_reply {
	struct mgmt_addr_info addr;
	uint32_t passkey;
} __packed;
struct mgmt_rp_user_passkey_reply {
	struct mgmt_addr_info addr;
} __packed;

#define MGMT_OP_USER_PASSKEY_NEG_REPLY	0x001F
struct mgmt_cp_user_passkey_neg_reply {
	struct mgmt_addr_info addr;
} __packed;

#define MGMT_OP_READ_LOCAL_OOB_DATA	0x0020
struct mgmt_rp_read_local_oob_data {
	uint8_t hash192[16];
	uint8_t rand192[16];
	uint8_t hash256[16];
	uint8_t rand256[16];
} __packed;

#define MGMT_OP_ADD_REMOTE_OOB_DATA	0x0021
struct mgmt_cp_add_remote_oob_data {
	struct mgmt_addr_info addr;
	uint8_t hash192[16];
	uint8_t rand192[16];
	uint8_t hash256[16];
	uint8_t rand256[16];
} __packed;

#define MGMT_OP_REMOVE_REMOTE_OOB_DATA	0x0022
struct mgmt_cp_remove_remote_oob_data {
	struct mgmt_addr_info addr;
} __packed;

#define MGMT_OP_START_DISCOVERY		0x0023
struct mgmt_cp_start_discovery {
	uint8_t type;
} __packed;

#define MGMT_OP_STOP_DISCOVERY		0x0024
struct mgmt_cp_stop_discovery {
	uint8_t type;
} __packed;

#define MGMT_OP_CONFIRM_NAME		0x0025
struct mgmt_cp_confirm_name {
	struct mgmt_addr_info addr;
	uint8_t name_known;
} __packed;
struct mgmt_rp_confirm_name {
	struct mgmt_addr_info addr;
} __packed;

#define MGMT_OP_BLOCK_DEVICE		0x0026
struct mgmt_cp_block_device {
	struct mgmt_addr_info addr;
} __packed;

#define MGMT_OP_UNBLOCK_DEVICE		0x0027
struct mgmt_cp_unblock_device {
	struct mgmt_addr_info addr;
} __packed;

#define MGMT_OP_SET_DEVICE_ID		0x0028
struct mgmt_cp_set_device_id {
	uint16_t source;
	uint16_t vendor;
	uint16_t product;
	uint16_t version;
} __packed;

#define MGMT_OP_SET_ADVERTISING		0x0029

#define MGMT_OP_SET_BREDR		0x002A

#define MGMT_OP_SET_STATIC_ADDRESS	0x002B
struct mgmt_cp_set_static_address {
	bdaddr_t bdaddr;
} __packed;

#define MGMT_OP_SET_SCAN_PARAMS		0x002C
struct mgmt_cp_set_scan_params {
	uint16_t interval;
	uint16_t window;
} __packed;

#define MGMT_OP_SET_SECURE_CONN		0x002D

#define MGMT_OP_SET_DEBUG_KEYS		0x002E

struct mgmt_irk_info {
	struct mgmt_addr_info addr;
	uint8_t val[16];
} __packed;

#define MGMT_OP_SET_PRIVACY		0x002F
struct mgmt_cp_set_privacy {
	uint8_t privacy;
	uint8_t irk[16];
} __packed;

#define MGMT_OP_LOAD_IRKS		0x0030
struct mgmt_cp_load_irks {
	uint16_t irk_count;
	struct mgmt_irk_info irks[0];
} __packed;

#define MGMT_OP_GET_CONN_INFO		0x0031
struct mgmt_cp_get_conn_info {
	struct mgmt_addr_info addr;
} __packed;
struct mgmt_rp_get_conn_info {
	struct mgmt_addr_info addr;
	int8_t rssi;
	int8_t tx_power;
	int8_t max_tx_power;
} __packed;

#define MGMT_OP_GET_CLOCK_INFO		0x0032
struct mgmt_cp_get_clock_info {
	struct mgmt_addr_info addr;
} __packed;
struct mgmt_rp_get_clock_info {
	struct mgmt_addr_info addr;
	uint32_t  local_clock;
	uint32_t  piconet_clock;
	uint16_t  accuracy;
} __packed;

#define MGMT_OP_ADD_DEVICE		0x0033
struct mgmt_cp_add_device {
	struct mgmt_addr_info addr;
	uint8_t action;
} __packed;
struct mgmt_rp_add_device {
	struct mgmt_addr_info addr;
} __packed;

#define MGMT_OP_REMOVE_DEVICE		0x0034
struct mgmt_cp_remove_device {
	struct mgmt_addr_info addr;
} __packed;
struct mgmt_rp_remove_device {
	struct mgmt_addr_info addr;
} __packed;

struct mgmt_conn_param {
	struct mgmt_addr_info addr;
	uint16_t min_interval;
	uint16_t max_interval;
	uint16_t latency;
	uint16_t timeout;
} __packed;

#define MGMT_OP_LOAD_CONN_PARAM		0x0035
struct mgmt_cp_load_conn_param {
	uint16_t param_count;
	struct mgmt_conn_param params[0];
} __packed;

#define MGMT_OP_READ_UNCONF_INDEX_LIST	0x0036
struct mgmt_rp_read_unconf_index_list {
	uint16_t num_controllers;
	uint16_t index[0];
} __packed;

#define MGMT_OPTION_EXTERNAL_CONFIG	0x00000001
#define MGMT_OPTION_PUBLIC_ADDRESS	0x00000002

#define MGMT_OP_READ_CONFIG_INFO	0x0037
struct mgmt_rp_read_config_info {
	uint16_t manufacturer;
	uint32_t supported_options;
	uint32_t missing_options;
} __packed;

#define MGMT_OP_SET_EXTERNAL_CONFIG	0x0038
struct mgmt_cp_set_external_config {
	uint8_t config;
} __packed;

#define MGMT_OP_SET_PUBLIC_ADDRESS	0x0039
struct mgmt_cp_set_public_address {
	bdaddr_t bdaddr;
} __packed;

#define MGMT_OP_START_SERVICE_DISCOVERY		0x003A
struct mgmt_cp_start_service_discovery {
	uint8_t type;
	int8_t rssi;
	uint16_t uuid_count;
	uint8_t uuids[0][16];
} __packed;

#define MGMT_OP_READ_LOCAL_OOB_EXT_DATA	0x003B
struct mgmt_cp_read_local_oob_ext_data {
	uint8_t  type;
} __packed;
struct mgmt_rp_read_local_oob_ext_data {
	uint8_t  type;
	uint16_t eir_len;
	uint8_t  eir[0];
} __packed;

#define MGMT_OP_READ_EXT_INDEX_LIST	0x003C
struct mgmt_rp_read_ext_index_list {
	uint16_t num_controllers;
	struct {
		uint16_t index;
		uint8_t type;
		uint8_t bus;
	} entry[0];
} __packed;

#define MGMT_OP_READ_ADV_FEATURES	0x003D
struct mgmt_rp_read_adv_features {
	uint32_t supported_flags;
	uint8_t  max_adv_data_len;
	uint8_t  max_scan_rsp_len;
	uint8_t  max_instances;
	uint8_t  num_instances;
	uint8_t  instance[0];
} __packed;

#define MGMT_OP_ADD_ADVERTISING		0x003E
struct mgmt_cp_add_advertising {
	uint8_t  instance;
	uint32_t flags;
	uint16_t duration;
	uint16_t timeout;
	uint8_t  adv_data_len;
	uint8_t  scan_rsp_len;
	uint8_t  data[0];
} __packed;
struct mgmt_rp_add_advertising {
	uint8_t instance;
} __packed;

#define MGMT_ADV_FLAG_CONNECTABLE	(1 << 0)
#define MGMT_ADV_FLAG_DISCOV		(1 << 1)
#define MGMT_ADV_FLAG_LIMITED_DISCOV	(1 << 2)
#define MGMT_ADV_FLAG_MANAGED_FLAGS	(1 << 3)
#define MGMT_ADV_FLAG_TX_POWER		(1 << 4)
#define MGMT_ADV_FLAG_APPEARANCE	(1 << 5)
#define MGMT_ADV_FLAG_LOCAL_NAME	(1 << 6)

#define MGMT_OP_REMOVE_ADVERTISING	0x003F
struct mgmt_cp_remove_advertising {
	uint8_t instance;
} __packed;
#define MGMT_REMOVE_ADVERTISING_SIZE	1
struct mgmt_rp_remove_advertising {
	uint8_t instance;
} __packed;

#define MGMT_OP_GET_ADV_SIZE_INFO	0x0040
struct mgmt_cp_get_adv_size_info {
	uint8_t  instance;
	uint32_t flags;
} __packed;
#define MGMT_GET_ADV_SIZE_INFO_SIZE	5
struct mgmt_rp_get_adv_size_info {
	uint8_t  instance;
	uint32_t flags;
	uint8_t  max_adv_data_len;
	uint8_t  max_scan_rsp_len;
} __packed;

#define MGMT_OP_START_LIMITED_DISCOVERY	0x0041

#define MGMT_EV_CMD_COMPLETE		0x0001
struct mgmt_ev_cmd_complete {
	uint16_t opcode;
	uint8_t status;
	uint8_t data[0];
} __packed;

#define MGMT_EV_CMD_STATUS		0x0002
struct mgmt_ev_cmd_status {
	uint16_t opcode;
	uint8_t status;
} __packed;

#define MGMT_EV_CONTROLLER_ERROR	0x0003
struct mgmt_ev_controller_error {
	uint8_t error_code;
} __packed;

#define MGMT_EV_INDEX_ADDED		0x0004

#define MGMT_EV_INDEX_REMOVED		0x0005

#define MGMT_EV_NEW_SETTINGS		0x0006

#define MGMT_EV_CLASS_OF_DEV_CHANGED	0x0007
struct mgmt_ev_class_of_dev_changed {
	uint8_t dev_class[3];
} __packed;

#define MGMT_EV_LOCAL_NAME_CHANGED	0x0008
struct mgmt_ev_local_name_changed {
	uint8_t name[MGMT_MAX_NAME_LENGTH];
	uint8_t short_name[MGMT_MAX_SHORT_NAME_LENGTH];
} __packed;

#define MGMT_EV_NEW_LINK_KEY		0x0009
struct mgmt_ev_new_link_key {
	uint8_t store_hint;
	struct mgmt_link_key_info key;
} __packed;

#define MGMT_EV_NEW_LONG_TERM_KEY	0x000A
struct mgmt_ev_new_long_term_key {
	uint8_t store_hint;
	struct mgmt_ltk_info key;
} __packed;

#define MGMT_EV_DEVICE_CONNECTED	0x000B
struct mgmt_ev_device_connected {
	struct mgmt_addr_info addr;
	uint32_t flags;
	uint16_t eir_len;
	uint8_t eir[0];
} __packed;

#define MGMT_DEV_DISCONN_UNKNOWN	0x00
#define MGMT_DEV_DISCONN_TIMEOUT	0x01
#define MGMT_DEV_DISCONN_LOCAL_HOST	0x02
#define MGMT_DEV_DISCONN_REMOTE		0x03

#define MGMT_EV_DEVICE_DISCONNECTED	0x000C
struct mgmt_ev_device_disconnected {
	struct mgmt_addr_info addr;
	uint8_t reason;
} __packed;

#define MGMT_EV_CONNECT_FAILED		0x000D
struct mgmt_ev_connect_failed {
	struct mgmt_addr_info addr;
	uint8_t status;
} __packed;

#define MGMT_EV_PIN_CODE_REQUEST	0x000E
struct mgmt_ev_pin_code_request {
	struct mgmt_addr_info addr;
	uint8_t secure;
} __packed;

#define MGMT_EV_USER_CONFIRM_REQUEST	0x000F
struct mgmt_ev_user_confirm_request {
	struct mgmt_addr_info addr;
	uint8_t confirm_hint;
	uint32_t value;
} __packed;

#define MGMT_EV_USER_PASSKEY_REQUEST	0x0010
struct mgmt_ev_user_passkey_request {
	struct mgmt_addr_info addr;
} __packed;

#define MGMT_EV_AUTH_FAILED		0x0011
struct mgmt_ev_auth_failed {
	struct mgmt_addr_info addr;
	uint8_t status;
} __packed;

#define MGMT_DEV_FOUND_CONFIRM_NAME	0x01
#define MGMT_DEV_FOUND_LEGACY_PAIRING	0x02
#define MGMT_DEV_FOUND_NOT_CONNECTABLE	0x04

#define MGMT_EV_DEVICE_FOUND		0x0012
struct mgmt_ev_device_found {
	struct mgmt_addr_info addr;
	int8_t rssi;
	uint32_t flags;
	uint16_t eir_len;
	uint8_t eir[0];
} __packed;

#define MGMT_EV_DISCOVERING		0x0013
struct mgmt_ev_discovering {
	uint8_t type;
	uint8_t discovering;
} __packed;

#define MGMT_EV_DEVICE_BLOCKED		0x0014
struct mgmt_ev_device_blocked {
	struct mgmt_addr_info addr;
} __packed;

#define MGMT_EV_DEVICE_UNBLOCKED	0x0015
struct mgmt_ev_device_unblocked {
	struct mgmt_addr_info addr;
} __packed;

#define MGMT_EV_DEVICE_UNPAIRED		0x0016
struct mgmt_ev_device_unpaired {
	struct mgmt_addr_info addr;
} __packed;

#define MGMT_EV_PASSKEY_NOTIFY		0x0017
struct mgmt_ev_passkey_notify {
	struct mgmt_addr_info addr;
	uint32_t passkey;
	uint8_t entered;
} __packed;

#define MGMT_EV_NEW_IRK			0x0018
struct mgmt_ev_new_irk {
	uint8_t  store_hint;
	bdaddr_t rpa;
	struct mgmt_irk_info key;
} __packed;

struct mgmt_csrk_info {
	struct mgmt_addr_info addr;
	uint8_t type;
	uint8_t val[16];
} __packed;

#define MGMT_EV_NEW_CSRK		0x0019
struct mgmt_ev_new_csrk {
	uint8_t store_hint;
	struct mgmt_csrk_info key;
} __packed;

#define MGMT_EV_DEVICE_ADDED		0x001a
struct mgmt_ev_device_added {
	struct mgmt_addr_info addr;
	uint8_t action;
} __packed;

#define MGMT_EV_DEVICE_REMOVED		0x001b
struct mgmt_ev_device_removed {
	struct mgmt_addr_info addr;
} __packed;

#define MGMT_EV_NEW_CONN_PARAM		0x001c
struct mgmt_ev_new_conn_param {
	struct mgmt_addr_info addr;
	uint8_t store_hint;
	uint16_t min_interval;
	uint16_t max_interval;
	uint16_t latency;
	uint16_t timeout;
} __packed;

#define MGMT_EV_UNCONF_INDEX_ADDED	0x001d

#define MGMT_EV_UNCONF_INDEX_REMOVED	0x001e

#define MGMT_EV_NEW_CONFIG_OPTIONS	0x001f

#define MGMT_EV_EXT_INDEX_ADDED		0x0020
struct mgmt_ev_ext_index_added {
	uint8_t type;
	uint8_t bus;
} __packed;

#define MGMT_EV_EXT_INDEX_REMOVED	0x0021
struct mgmt_ev_ext_index_removed {
	uint8_t type;
	uint8_t bus;
} __packed;

#define MGMT_EV_LOCAL_OOB_DATA_UPDATED	0x0022
struct mgmt_ev_local_oob_data_updated {
	uint8_t  type;
	uint16_t eir_len;
	uint8_t  eir[0];
} __packed;

#define MGMT_EV_ADVERTISING_ADDED	0x0023
struct mgmt_ev_advertising_added {
	uint8_t instance;
} __packed;

#define MGMT_EV_ADVERTISING_REMOVED	0x0024
struct mgmt_ev_advertising_removed {
	uint8_t instance;
} __packed;

static const char *mgmt_op[] = {
	"<0x0000>",
	"Read Version",
	"Read Commands",
	"Read Index List",
	"Read Controller Info",
	"Set Powered",
	"Set Discoverable",
	"Set Connectable",
	"Set Fast Connectable",			/* 0x0008 */
	"Set Bondable",
	"Set Link Security",
	"Set Secure Simple Pairing",
	"Set High Speed",
	"Set Low Energy",
	"Set Dev Class",
	"Set Local Name",
	"Add UUID",					/* 0x0010 */
	"Remove UUID",
	"Load Link Keys",
	"Load Long Term Keys",
	"Disconnect",
	"Get Connections",
	"PIN Code Reply",
	"PIN Code Neg Reply",
	"Set IO Capability",				/* 0x0018 */
	"Pair Device",
	"Cancel Pair Device",
	"Unpair Device",
	"User Confirm Reply",
	"User Confirm Neg Reply",
	"User Passkey Reply",
	"User Passkey Neg Reply",
	"Read Local OOB Data",				/* 0x0020 */
	"Add Remote OOB Data",
	"Remove Remove OOB Data",
	"Start Discovery",
	"Stop Discovery",
	"Confirm Name",
	"Block Device",
	"Unblock Device",
	"Set Device ID",				/* 0x0028 */
	"Set Advertising",
	"Set BR/EDR",
	"Set Static Address",
	"Set Scan Parameters",
	"Set Secure Connections",
	"Set Debug Keys",
	"Set Privacy",
	"Load Identity Resolving Keys",			/* 0x0030 */
	"Get Connection Information",
	"Get Clock Information",
	"Add Device",
	"Remove Device",
	"Load Connection Parameters",
	"Read Unconfigured Index List",
	"Read Controller Configuration Information",
	"Set External Configuration",			/* 0x0038 */
	"Set Public Address",
	"Start Service Discovery",
	"Read Local Out Of Band Extended Data",
	"Read Extended Controller Index List",
	"Read Advertising Features",
	"Add Advertising",
	"Remove Advertising",
	"Get Advertising Size Information",		/* 0x0040 */
	"Start Limited Discovery",
};

static const char *mgmt_ev[] = {
	"<0x0000>",
	"Command Complete",
	"Command Status",
	"Controller Error",
	"Index Added",
	"Index Removed",
	"New Settings",
	"Class of Device Changed",
	"Local Name Changed",				/* 0x0008 */
	"New Link Key",
	"New Long Term Key",
	"Device Connected",
	"Device Disconnected",
	"Connect Failed",
	"PIN Code Request",
	"User Confirm Request",
	"User Passkey Request",				/* 0x0010 */
	"Authentication Failed",
	"Device Found",
	"Discovering",
	"Device Blocked",
	"Device Unblocked",
	"Device Unpaired",
	"Passkey Notify",
	"New Identity Resolving Key",			/* 0x0018 */
	"New Signature Resolving Key",
	"Device Added",
	"Device Removed",
	"New Connection Parameter",
	"Unconfigured Index Added",
	"Unconfigured Index Removed",
	"New Configuration Options",
	"Extended Index Added",				/* 0x0020 */
	"Extended Index Removed",
	"Local Out Of Band Extended Data Updated",
	"Advertising Added",
	"Advertising Removed",
};

static const char *mgmt_status[] = {
	"Success",
	"Unknown Command",
	"Not Connected",
	"Failed",
	"Connect Failed",
	"Authentication Failed",
	"Not Paired",
	"No Resources",
	"Timeout",
	"Already Connected",
	"Busy",
	"Rejected",
	"Not Supported",
	"Invalid Parameters",
	"Disconnected",
	"Not Powered",
	"Cancelled",
	"Invalid Index",
	"Blocked through rfkill",
	"Already Paired",
	"Permission Denied",
};

#ifndef NELEM
#define NELEM(x) (sizeof(x) / sizeof((x)[0]))
#endif

static inline const char *mgmt_opstr(uint16_t op)
{
	if (op >= NELEM(mgmt_op))
		return "<unknown opcode>";
	return mgmt_op[op];
}

static inline const char *mgmt_evstr(uint16_t ev)
{
	if (ev >= NELEM(mgmt_ev))
		return "<unknown event>";
	return mgmt_ev[ev];
}

static inline const char *mgmt_errstr(uint8_t status)
{
	if (status >= NELEM(mgmt_status))
		return "<unknown status>";
	return mgmt_status[status];
}

/* SPDX-License-Identifier: LGPL-2.1-or-later */
/*
 *
 *  BlueZ - Bluetooth protocol stack for Linux
 *
 *  Copyright (C) 2012-2017  Intel Corporation. All rights reserved.
 *
 *
 */

#include <stdbool.h>

#ifndef __packed
#define __packed __attribute__((packed))
#endif

#define BTP_INDEX_NON_CONTROLLER 0xff

#define BTP_ERROR_FAIL		0x01
#define BTP_ERROR_UNKNOWN_CMD	0x02
#define BTP_ERROR_NOT_READY	0x03
#define BTP_ERROR_INVALID_INDEX	0x04

#define BTP_CORE_SERVICE	0
#define BTP_GAP_SERVICE		1
#define BTP_GATT_SERVICE	2
#define BTP_L2CAP_SERVICE	3
#define BTP_MESH_NODE_SERVICE	4

struct btp_hdr {
	uint8_t service;
	uint8_t opcode;
	uint8_t index;
	uint16_t data_len;
	uint8_t data[0];
} __packed;

struct btp_error {
	uint8_t status;
} __packed;

#define BTP_OP_ERROR				0x00

#define BTP_OP_CORE_READ_SUPPORTED_COMMANDS	0x01

#define BTP_OP_CORE_READ_SUPPORTED_SERVICES	0x02

#define BTP_OP_CORE_REGISTER			0x03
struct btp_core_register_cp {
	uint8_t service_id;
} __packed;

#define BTP_OP_CORE_UNREGISTER			0x04
struct btp_core_unregister_cp {
	uint8_t service_id;
} __packed;

#define BTP_EV_CORE_READY			0x80

#define BTP_OP_GAP_READ_SUPPORTED_COMMANDS	0x01

#define BTP_OP_GAP_READ_CONTROLLER_INDEX_LIST	0x02
struct btp_gap_read_index_rp {
	uint8_t num;
	uint8_t indexes[0];
} __packed;

#define BTP_GAP_SETTING_POWERED			0x00000001
#define BTP_GAP_SETTING_CONNECTABLE		0x00000002
#define BTP_GAP_SETTING_FAST_CONNECTABLE	0x00000004
#define BTP_GAP_SETTING_DISCOVERABLE		0x00000008
#define BTP_GAP_SETTING_BONDABLE		0x00000010
#define BTP_GAP_SETTING_LL_SECURITY		0x00000020
#define BTP_GAP_SETTING_SSP			0x00000040
#define BTP_GAP_SETTING_BREDR			0x00000080
#define BTP_GAP_SETTING_HS			0x00000100
#define BTP_GAP_SETTING_LE			0x00000200
#define BTP_GAP_SETTING_ADVERTISING		0x00000400
#define BTP_GAP_SETTING_SC			0x00000800
#define BTP_GAP_SETTING_DEBUG_KEYS		0x00001000
#define BTP_GAP_SETTING_PRIVACY			0x00002000
#define BTP_GAP_SETTING_CONTROLLER_CONF		0x00004000
#define BTP_GAP_SETTING_STATIC_ADDRESS		0x00008000

#define BTP_OP_GAP_READ_COTROLLER_INFO		0x03
struct btp_gap_read_info_rp {
	bdaddr_t address;
	uint32_t supported_settings;
	uint32_t current_settings;
	uint8_t cod[3];
	uint8_t name[249];
	uint8_t short_name[11];
} __packed;

#define BTP_OP_GAP_RESET			0x04
struct btp_gap_reset_rp {
	uint32_t current_settings;
} __packed;

#define BTP_OP_GAP_SET_POWERED			0x05
struct btp_gap_set_powered_cp {
	uint8_t powered;
} __packed;

struct btp_gap_set_powered_rp {
	uint32_t current_settings;
} __packed;

#define BTP_OP_GAP_SET_CONNECTABLE		0x06
struct btp_gap_set_connectable_cp {
	uint8_t connectable;
} __packed;

struct btp_gap_set_connectable_rp {
	uint32_t current_settings;
} __packed;

#define BTP_OP_GAP_SET_FAST_CONNECTABLE		0x07
struct btp_gap_set_fast_connectable_cp {
	uint8_t fast_connectable;
} __packed;

struct btp_gap_set_fast_connectable_rp {
	uint32_t current_settings;
} __packed;

#define BTP_OP_GAP_SET_DISCOVERABLE		0x08
struct btp_gap_set_discoverable_cp {
	uint8_t discoverable;
} __packed;

struct btp_gap_set_discoverable_rp {
	uint32_t current_settings;
} __packed;

#define BTP_OP_GAP_SET_BONDABLE			0x09
struct btp_gap_set_bondable_cp {
	uint8_t bondable;
} __packed;

struct btp_gap_set_bondable_rp {
	uint32_t current_settings;
} __packed;

#define BTP_OP_GAP_START_ADVERTISING		0x0a
struct btp_gap_start_adv_cp {
	uint8_t adv_data_len;
	uint8_t scan_rsp_len;
	uint8_t data[0];
} __packed;

struct btp_gap_start_adv_rp {
	uint32_t current_settings;
} __packed;

#define BTP_OP_GAP_STOP_ADVERTISING		0x0b
struct btp_gap_stop_adv_rp {
	uint32_t current_settings;
} __packed;

#define BTP_GAP_DISCOVERY_FLAG_LE		0x01
#define BTP_GAP_DISCOVERY_FLAG_BREDR		0x02
#define BTP_GAP_DISCOVERY_FLAG_LIMITED		0x04
#define BTP_GAP_DISCOVERY_FLAG_ACTIVE		0x08
#define BTP_GAP_DISCOVERY_FLAG_OBSERVATION	0x10

#define BTP_OP_GAP_START_DISCOVERY		0x0c
struct btp_gap_start_discovery_cp {
	uint8_t flags;
} __packed;

#define BTP_OP_GAP_STOP_DISCOVERY		0x0d

#define BTP_GAP_ADDR_PUBLIC			0x00
#define BTP_GAP_ADDR_RANDOM			0x01

#define BTP_OP_GAP_CONNECT			0x0e
struct btp_gap_connect_cp {
	uint8_t address_type;
	bdaddr_t address;
} __packed;

#define BTP_OP_GAP_DISCONNECT			0x0f
struct btp_gap_disconnect_cp {
	uint8_t address_type;
	bdaddr_t address;
} __packed;

#define BTP_GAP_IOCAPA_DISPLAY_ONLY		0x00
#define BTP_GAP_IOCAPA_DISPLAY_YESNO		0x01
#define BTP_GAP_IOCAPA_KEYBOARD_ONLY		0x02
#define BTP_GAP_IOCAPA_NO_INPUT_NO_OUTPUT	0x03
#define BTP_GAP_IOCAPA_KEYBOARD_DISPLAY		0x04

#define BTP_OP_GAP_SET_IO_CAPA			0x10
struct btp_gap_set_io_capa_cp {
	uint8_t capa;
} __packed;

#define BTP_OP_GAP_PAIR				0x11
struct btp_gap_pair_cp {
	uint8_t address_type;
	bdaddr_t address;
} __packed;

#define BTP_OP_GAP_UNPAIR			0x12
struct btp_gap_unpair_cp {
	uint8_t address_type;
	bdaddr_t address;
} __packed;

#define BTP_OP_GAP_PASSKEY_ENTRY_RSP		0x13
struct btp_gap_passkey_entry_rsp_cp {
	uint8_t address_type;
	bdaddr_t address;
	uint32_t passkey;
} __packed;

#define BTP_OP_GAP_PASSKEY_CONFIRM_RSP		0x14
struct btp_gap_passkey_confirm_rsp_cp {
	uint8_t address_type;
	bdaddr_t address;
	uint8_t match;
} __packed;

#define BTP_EV_GAP_NEW_SETTINGS			0x80
struct btp_new_settings_ev {
	uint32_t current_settings;
} __packed;

#define BTP_EV_GAP_DEVICE_FOUND_FLAG_RSSI	0x01
#define BTP_EV_GAP_DEVICE_FOUND_FLAG_AD		0x02
#define BTP_EV_GAP_DEVICE_FOUND_FLAG_SR		0x04

#define BTP_EV_GAP_DEVICE_FOUND			0x81
struct btp_device_found_ev {
	uint8_t address_type;
	bdaddr_t address;
	int8_t rssi;
	uint8_t flags;
	uint16_t eir_len;
	uint8_t eir[0];
} __packed;

#define BTP_EV_GAP_DEVICE_CONNECTED		0x82
struct btp_gap_device_connected_ev {
	uint8_t address_type;
	bdaddr_t address;
	uint16_t connection_interval;
	uint16_t connection_latency;
	uint16_t supervision_timeout;
} __packed;

#define BTP_EV_GAP_DEVICE_DISCONNECTED		0x83
struct btp_gap_device_disconnected_ev {
	uint8_t address_type;
	bdaddr_t address;
} __packed;

#define BTP_EV_GAP_PASSKEY_DISPLAY		0x84
struct btp_gap_passkey_display_ev {
	uint8_t address_type;
	bdaddr_t address;
	uint32_t passkey;
} __packed;

#define BTP_EV_GAP_PASSKEY_REQUEST		0x85
struct btp_gap_passkey_req_ev {
	uint8_t address_type;
	bdaddr_t address;
} __packed;

#define BTP_EV_GAP_PASSKEY_CONFIRM		0x86
struct btp_gap_passkey_confirm_ev {
	uint8_t address_type;
	bdaddr_t address;
	uint32_t passkey;
} __packed;

#define BTP_EV_GAP_IDENTITY_RESOLVED		0x87
struct btp_gap_identity_resolved_ev {
	uint8_t address_type;
	bdaddr_t address;
	uint8_t identity_address_type;
	bdaddr_t identity_address;
} __packed;

struct btp;

typedef void (*btp_destroy_func_t)(void *user_data);
typedef void (*btp_disconnect_func_t)(struct btp *btp, void *user_data);
typedef void (*btp_cmd_func_t)(uint8_t index, const void *param,
					uint16_t length, void *user_data);

struct btp *btp_new(const char *path);
void btp_cleanup(struct btp *btp);

bool btp_set_disconnect_handler(struct btp *btp, btp_disconnect_func_t callback,
				void *user_data, btp_destroy_func_t destroy);

bool btp_send_error(struct btp *btp, uint8_t service, uint8_t index,
								uint8_t status);
bool btp_send(struct btp *btp, uint8_t service, uint8_t opcode, uint8_t index,
					uint16_t length, const void *param);

unsigned int btp_register(struct btp *btp, uint8_t service, uint8_t opcode,
				btp_cmd_func_t callback, void *user_data,
				btp_destroy_func_t destroy);
bool btp_unregister(struct btp *btp, unsigned int id);
void btp_unregister_service(struct btp *btp, uint8_t service);

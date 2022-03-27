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

#include <stdint.h>

struct bt_ll_hdr {
	uint8_t  preamble;
	uint32_t access_addr;
} __attribute__ ((packed));

#define BT_LL_CONN_UPDATE_REQ	0x00
struct bt_ll_conn_update_req {
	uint8_t  win_size;
	uint16_t win_offset;
	uint16_t interval;
	uint16_t latency;
	uint16_t timeout;
	uint16_t instant;
} __attribute__ ((packed));

#define BT_LL_CHANNEL_MAP_REQ	0x01
struct bt_ll_channel_map_req {
	uint8_t  map[5];
	uint16_t instant;
} __attribute__ ((packed));

#define BT_LL_TERMINATE_IND	0x02
struct bt_ll_terminate_ind {
	uint8_t  error;
} __attribute__ ((packed));

#define BT_LL_ENC_REQ		0x03
struct bt_ll_enc_req {
	uint64_t rand;
	uint16_t ediv;
	uint64_t skd;
	uint32_t iv;
} __attribute__ ((packed));

#define BT_LL_ENC_RSP		0x04
struct bt_ll_enc_rsp {
	uint64_t skd;
	uint32_t iv;
} __attribute__ ((packed));

#define BT_LL_START_ENC_REQ	0x05

#define BT_LL_START_ENC_RSP	0x06

#define BT_LL_UNKNOWN_RSP	0x07
struct bt_ll_unknown_rsp {
	uint8_t  type;
} __attribute__ ((packed));

#define BT_LL_FEATURE_REQ	0x08
struct bt_ll_feature_req {
	uint8_t  features[8];
} __attribute__ ((packed));

#define BT_LL_FEATURE_RSP	0x09
struct bt_ll_feature_rsp {
	uint8_t  features[8];
} __attribute__ ((packed));

#define BT_LL_PAUSE_ENC_REQ	0x0a

#define BT_LL_PAUSE_ENC_RSP	0x0b

#define BT_LL_VERSION_IND	0x0c
struct bt_ll_version_ind {
	uint8_t  version;
	uint16_t company;
	uint16_t subversion;
} __attribute__ ((packed));

#define BT_LL_REJECT_IND	0x0d
struct bt_ll_reject_ind {
	uint8_t  error;
} __attribute__ ((packed));

#define BT_LL_SLAVE_FEATURE_REQ	0x0e
struct bt_ll_slave_feature_req {
	uint8_t  features[8];
} __attribute__ ((packed));

#define BT_LL_CONN_PARAM_REQ	0x0f

#define BT_LL_CONN_PARAM_RSP	0x10

#define BT_LL_REJECT_IND_EXT	0x11
struct bt_ll_reject_ind_ext {
	uint8_t  opcode;
	uint8_t  error;
} __attribute__ ((packed));

#define BT_LL_PING_REQ		0x12

#define BT_LL_PING_RSP		0x13

#define BT_LL_LENGTH_REQ	0x14

#define BT_LL_LENGTH_RSP	0x15

#define LMP_ESC4(x) ((127 << 8) | (x))

#define BT_LMP_NAME_REQ			1
struct bt_lmp_name_req {
	uint8_t  offset;
} __attribute__ ((packed));

#define BT_LMP_NAME_RSP			2
struct bt_lmp_name_rsp {
	uint8_t  offset;
	uint8_t  length;
	uint8_t  fragment[14];
} __attribute__ ((packed));

#define BT_LMP_ACCEPTED			3
struct bt_lmp_accepted {
	uint8_t  opcode;
} __attribute__ ((packed));

#define BT_LMP_NOT_ACCEPTED		4
struct bt_lmp_not_accepted {
	uint8_t  opcode;
	uint8_t  error;
} __attribute__ ((packed));

#define BT_LMP_CLKOFFSET_REQ		5

#define BT_LMP_CLKOFFSET_RSP		6
struct bt_lmp_clkoffset_rsp {
	uint16_t offset;
} __attribute__ ((packed));

#define BT_LMP_DETACH			7
struct bt_lmp_detach {
	uint8_t  error;
} __attribute__ ((packed));

#define BT_LMP_AU_RAND			11
struct bt_lmp_au_rand {
	uint8_t  number[16];
} __attribute__ ((packed));

#define BT_LMP_SRES			12
struct bt_lmp_sres {
	uint8_t  response[4];
} __attribute__ ((packed));

#define BT_LMP_ENCRYPTION_MODE_REQ	15
struct bt_lmp_encryption_mode_req {
	uint8_t  mode;
} __attribute__ ((packed));

#define BT_LMP_ENCRYPTION_KEY_SIZE_REQ	16
struct bt_lmp_encryption_key_size_req {
	uint8_t  key_size;
} __attribute__ ((packed));

#define BT_LMP_START_ENCRYPTION_REQ	17
struct bt_lmp_start_encryption_req {
	uint8_t  number[16];
} __attribute__ ((packed));

#define BT_LMP_STOP_ENCRYPTION_REQ	18

#define BT_LMP_SWITCH_REQ		19
struct bt_lmp_switch_req {
	uint32_t instant;
} __attribute__ ((packed));

#define BT_LMP_UNSNIFF_REQ		24

#define BT_LMP_MAX_POWER		33

#define BT_LMP_MIN_POWER		34

#define BT_LMP_AUTO_RATE		35

#define BT_LMP_PREFERRED_RATE		36
struct bt_lmp_preferred_rate {
	uint8_t  rate;
} __attribute__ ((packed));

#define BT_LMP_VERSION_REQ		37
struct bt_lmp_version_req {
	uint8_t  version;
	uint16_t company;
	uint16_t subversion;
} __attribute__ ((packed));

#define BT_LMP_VERSION_RES		38
struct bt_lmp_version_res {
	uint8_t  version;
	uint16_t company;
	uint16_t subversion;
} __attribute__ ((packed));

#define BT_LMP_FEATURES_REQ		39
struct bt_lmp_features_req {
	uint8_t  features[8];
} __attribute__ ((packed));

#define BT_LMP_FEATURES_RES		40
struct bt_lmp_features_res {
	uint8_t  features[8];
} __attribute__ ((packed));

#define BT_LMP_MAX_SLOT			45
struct bt_lmp_max_slot {
	uint8_t  slots;
} __attribute__ ((packed));

#define BT_LMP_MAX_SLOT_REQ		46
struct bt_lmp_max_slot_req {
	uint8_t  slots;
} __attribute__ ((packed));

#define BT_LMP_TIMING_ACCURACY_REQ	47

#define BT_LMP_TIMING_ACCURACY_RES	48
struct bt_lmp_timing_accuracy_res {
	uint8_t  drift;
	uint8_t  jitter;
} __attribute__ ((packed));

#define BT_LMP_SETUP_COMPLETE		49

#define BT_LMP_USE_SEMI_PERMANENT_KEY	50

#define BT_LMP_HOST_CONNECTION_REQ	51

#define BT_LMP_SLOT_OFFSET		52
struct bt_lmp_slot_offset {
	uint16_t offset;
	uint8_t  bdaddr[6];
} __attribute__ ((packed));

#define BT_LMP_PAGE_SCAN_MODE_REQ	54
struct bt_lmp_page_scan_mode_req {
	uint8_t  scheme;
	uint8_t  settings;
} __attribute__ ((packed));

#define BT_LMP_TEST_ACTIVATE		56

#define BT_LMP_ENCRYPTION_KEY_SIZE_MASK_REQ	58

#define BT_LMP_SET_AFH			60
struct bt_lmp_set_afh {
	uint32_t instant;
	uint8_t  mode;
	uint8_t  map[10];
} __attribute__ ((packed));

#define BT_LMP_ENCAPSULATED_HEADER	61
struct bt_lmp_encapsulated_header {
	uint8_t  major;
	uint8_t  minor;
	uint8_t  length;
} __attribute__ ((packed));

#define BT_LMP_ENCAPSULATED_PAYLOAD	62
struct bt_lmp_encapsulated_payload {
	uint8_t  data[16];
} __attribute__ ((packed));

#define BT_LMP_SIMPLE_PAIRING_CONFIRM	63
struct bt_lmp_simple_pairing_confirm {
	uint8_t  value[16];
} __attribute__ ((packed));

#define BT_LMP_SIMPLE_PAIRING_NUMBER	64
struct bt_lmp_simple_pairing_number {
	uint8_t  value[16];
} __attribute__ ((packed));

#define BT_LMP_DHKEY_CHECK		65
struct bt_lmp_dhkey_check {
	uint8_t  value[16];
} __attribute__ ((packed));

#define BT_LMP_PAUSE_ENCRYPTION_AES_REQ	66

#define BT_LMP_ACCEPTED_EXT		LMP_ESC4(1)
struct bt_lmp_accepted_ext {
	uint8_t  escape;
	uint8_t  opcode;
} __attribute__ ((packed));

#define BT_LMP_NOT_ACCEPTED_EXT		LMP_ESC4(2)
struct bt_lmp_not_accepted_ext {
	uint8_t  escape;
	uint8_t  opcode;
	uint8_t  error;
} __attribute__ ((packed));

#define BT_LMP_FEATURES_REQ_EXT		LMP_ESC4(3)
struct bt_lmp_features_req_ext {
	uint8_t  page;
	uint8_t  max_page;
	uint8_t  features[8];
} __attribute__ ((packed));

#define BT_LMP_FEATURES_RES_EXT		LMP_ESC4(4)
struct bt_lmp_features_res_ext {
	uint8_t  page;
	uint8_t  max_page;
	uint8_t  features[8];
} __attribute__ ((packed));

#define BT_LMP_PACKET_TYPE_TABLE_REQ	LMP_ESC4(11)
struct bt_lmp_packet_type_table_req {
	uint8_t  table;
} __attribute__ ((packed));

#define BT_LMP_CHANNEL_CLASSIFICATION_REQ	LMP_ESC4(16)
struct bt_lmp_channel_classification_req {
	uint8_t  mode;
	uint16_t min_interval;
	uint16_t max_interval;
} __attribute__ ((packed));

#define BT_LMP_CHANNEL_CLASSIFICATION	LMP_ESC4(17)
struct bt_lmp_channel_classification {
	uint8_t  classification[10];
} __attribute__ ((packed));

#define BT_LMP_PAUSE_ENCRYPTION_REQ	LMP_ESC4(23)

#define BT_LMP_RESUME_ENCRYPTION_REQ	LMP_ESC4(24)

#define BT_LMP_IO_CAPABILITY_REQ	LMP_ESC4(25)
struct bt_lmp_io_capability_req {
	uint8_t  capability;
	uint8_t  oob_data;
	uint8_t  authentication;
} __attribute__ ((packed));

#define BT_LMP_IO_CAPABILITY_RES	LMP_ESC4(26)
struct bt_lmp_io_capability_res {
	uint8_t  capability;
	uint8_t  oob_data;
	uint8_t  authentication;
} __attribute__ ((packed));

#define BT_LMP_NUMERIC_COMPARISON_FAILED	LMP_ESC(27)

#define BT_LMP_PASSKEY_FAILED		LMP_ESC4(28)

#define BT_LMP_OOB_FAILED		LMP_ESC(29)

#define BT_LMP_POWER_CONTROL_REQ	LMP_ESC4(31)
struct bt_lmp_power_control_req {
	uint8_t  request;
} __attribute__ ((packed));

#define BT_LMP_POWER_CONTROL_RES	LMP_ESC4(32)
struct bt_lmp_power_control_res {
	uint8_t  response;
} __attribute__ ((packed));

#define BT_LMP_PING_REQ			LMP_ESC4(33)

#define BT_LMP_PING_RES			LMP_ESC4(34)

#define BT_H4_CMD_PKT	0x01
#define BT_H4_ACL_PKT	0x02
#define BT_H4_SCO_PKT	0x03
#define BT_H4_EVT_PKT	0x04

struct bt_hci_cmd_hdr {
	uint16_t opcode;
	uint8_t  plen;
} __attribute__ ((packed));

struct bt_hci_acl_hdr {
	uint16_t handle;
	uint16_t dlen;
} __attribute__ ((packed));

struct bt_hci_sco_hdr {
	uint16_t handle;
	uint8_t  dlen;
} __attribute__ ((packed));

struct bt_hci_evt_hdr {
	uint8_t  evt;
	uint8_t  plen;
} __attribute__ ((packed));

#define BT_HCI_CMD_NOP				0x0000

#define BT_HCI_CMD_INQUIRY			0x0401
struct bt_hci_cmd_inquiry {
	uint8_t  lap[3];
	uint8_t  length;
	uint8_t  num_resp;
} __attribute__ ((packed));

#define BT_HCI_CMD_INQUIRY_CANCEL		0x0402

#define BT_HCI_CMD_PERIODIC_INQUIRY		0x0403
struct bt_hci_cmd_periodic_inquiry {
	uint16_t max_period;
	uint16_t min_period;
	uint8_t  lap[3];
	uint8_t  length;
	uint8_t  num_resp;
} __attribute__ ((packed));

#define BT_HCI_CMD_EXIT_PERIODIC_INQUIRY	0x0404

#define BT_HCI_CMD_CREATE_CONN			0x0405
struct bt_hci_cmd_create_conn {
	uint8_t  bdaddr[6];
	uint16_t pkt_type;
	uint8_t  pscan_rep_mode;
	uint8_t  pscan_mode;
	uint16_t clock_offset;
	uint8_t  role_switch;
} __attribute__ ((packed));

#define BT_HCI_CMD_DISCONNECT			0x0406
struct bt_hci_cmd_disconnect {
	uint16_t handle;
	uint8_t  reason;
} __attribute__ ((packed));

#define BT_HCI_CMD_ADD_SCO_CONN			0x0407
struct bt_hci_cmd_add_sco_conn {
	uint16_t handle;
	uint16_t pkt_type;
} __attribute__ ((packed));

#define BT_HCI_CMD_CREATE_CONN_CANCEL		0x0408
struct bt_hci_cmd_create_conn_cancel {
	uint8_t  bdaddr[6];
} __attribute__ ((packed));

#define BT_HCI_CMD_ACCEPT_CONN_REQUEST		0x0409
struct bt_hci_cmd_accept_conn_request {
	uint8_t  bdaddr[6];
	uint8_t  role;
} __attribute__ ((packed));

#define BT_HCI_CMD_REJECT_CONN_REQUEST		0x040a
struct bt_hci_cmd_reject_conn_request {
	uint8_t  bdaddr[6];
	uint8_t  reason;
} __attribute__ ((packed));

#define BT_HCI_CMD_LINK_KEY_REQUEST_REPLY	0x040b
struct bt_hci_cmd_link_key_request_reply {
	uint8_t  bdaddr[6];
	uint8_t  link_key[16];
} __attribute__ ((packed));
struct bt_hci_rsp_link_key_request_reply {
	uint8_t  status;
	uint8_t  bdaddr[6];
} __attribute__ ((packed));

#define BT_HCI_CMD_LINK_KEY_REQUEST_NEG_REPLY	0x040c
struct bt_hci_cmd_link_key_request_neg_reply {
	uint8_t  bdaddr[6];
} __attribute__ ((packed));
struct bt_hci_rsp_link_key_request_neg_reply {
	uint8_t  status;
	uint8_t  bdaddr[6];
} __attribute__ ((packed));

#define BT_HCI_CMD_PIN_CODE_REQUEST_REPLY	0x040d
struct bt_hci_cmd_pin_code_request_reply {
	uint8_t  bdaddr[6];
	uint8_t  pin_len;
	uint8_t  pin_code[16];
} __attribute__ ((packed));

#define BT_HCI_CMD_PIN_CODE_REQUEST_NEG_REPLY	0x040e
struct bt_hci_cmd_pin_code_request_neg_reply {
	uint8_t  bdaddr[6];
} __attribute__ ((packed));
struct bt_hci_rsp_pin_code_request_neg_reply {
	uint8_t  status;
	uint8_t  bdaddr[6];
} __attribute__ ((packed));

#define BT_HCI_CMD_CHANGE_CONN_PKT_TYPE		0x040f
struct bt_hci_cmd_change_conn_pkt_type {
	uint16_t handle;
	uint16_t pkt_type;
} __attribute__ ((packed));

#define BT_HCI_CMD_AUTH_REQUESTED		0x0411
struct bt_hci_cmd_auth_requested {
	uint16_t handle;
} __attribute__ ((packed));

#define BT_HCI_CMD_SET_CONN_ENCRYPT		0x0413
struct bt_hci_cmd_set_conn_encrypt {
	uint16_t handle;
	uint8_t  encr_mode;
} __attribute__ ((packed));

#define BT_HCI_CMD_CHANGE_CONN_LINK_KEY		0x0415
struct bt_hci_cmd_change_conn_link_key {
	uint16_t handle;
} __attribute__ ((packed));

#define BT_HCI_CMD_MASTER_LINK_KEY		0x0417
struct bt_hci_cmd_master_link_key {
	uint8_t  key_flag;
} __attribute__ ((packed));

#define BT_HCI_CMD_REMOTE_NAME_REQUEST		0x0419
struct bt_hci_cmd_remote_name_request {
	uint8_t  bdaddr[6];
	uint8_t  pscan_rep_mode;
	uint8_t  pscan_mode;
	uint16_t clock_offset;
} __attribute__ ((packed));

#define BT_HCI_CMD_REMOTE_NAME_REQUEST_CANCEL	0x041a
struct bt_hci_cmd_remote_name_request_cancel {
	uint8_t  bdaddr[6];
} __attribute__ ((packed));
struct bt_hci_rsp_remote_name_request_cancel {
	uint8_t  status;
	uint8_t  bdaddr[6];
} __attribute__ ((packed));

#define BT_HCI_CMD_READ_REMOTE_FEATURES		0x041b
struct bt_hci_cmd_read_remote_features {
	uint16_t handle;
} __attribute__ ((packed));

#define BT_HCI_CMD_READ_REMOTE_EXT_FEATURES	0x041c
struct bt_hci_cmd_read_remote_ext_features {
	uint16_t handle;
	uint8_t  page;
} __attribute__ ((packed));

#define BT_HCI_CMD_READ_REMOTE_VERSION		0x041d
struct bt_hci_cmd_read_remote_version {
	uint16_t handle;
} __attribute__ ((packed));

#define BT_HCI_CMD_READ_CLOCK_OFFSET		0x041f
struct bt_hci_cmd_read_clock_offset {
	uint16_t handle;
} __attribute__ ((packed));

#define BT_HCI_CMD_READ_LMP_HANDLE		0x0420
struct bt_hci_cmd_read_lmp_handle {
	uint16_t  handle;
} __attribute__ ((packed));
struct bt_hci_rsp_read_lmp_handle {
	uint8_t  status;
	uint16_t handle;
	uint8_t  lmp_handle;
	uint32_t reserved;
} __attribute__ ((packed));

#define BT_HCI_CMD_SETUP_SYNC_CONN		0x0428
struct bt_hci_cmd_setup_sync_conn {
	uint16_t handle;
	uint32_t tx_bandwidth;
	uint32_t rx_bandwidth;
	uint16_t max_latency;
	uint16_t voice_setting;
	uint8_t  retrans_effort;
	uint16_t pkt_type;
} __attribute__ ((packed));

#define BT_HCI_CMD_ACCEPT_SYNC_CONN_REQUEST	0x0429
struct bt_hci_cmd_accept_sync_conn_request {
	uint8_t  bdaddr[6];
	uint32_t tx_bandwidth;
	uint32_t rx_bandwidth;
	uint16_t max_latency;
	uint16_t voice_setting;
	uint8_t  retrans_effort;
	uint16_t pkt_type;
} __attribute__ ((packed));

#define BT_HCI_CMD_REJECT_SYNC_CONN_REQUEST	0x042a
struct bt_hci_cmd_reject_sync_conn_request {
	uint8_t  bdaddr[6];
	uint8_t  reason;
} __attribute__ ((packed));

#define BT_HCI_CMD_IO_CAPABILITY_REQUEST_REPLY		0x042b
struct bt_hci_cmd_io_capability_request_reply {
	uint8_t  bdaddr[6];
	uint8_t  capability;
	uint8_t  oob_data;
	uint8_t  authentication;
} __attribute__ ((packed));
struct bt_hci_rsp_io_capability_request_reply {
	uint8_t  status;
	uint8_t  bdaddr[6];
} __attribute__ ((packed));

#define BT_HCI_CMD_USER_CONFIRM_REQUEST_REPLY		0x042c
struct bt_hci_cmd_user_confirm_request_reply {
	uint8_t  bdaddr[6];
} __attribute__ ((packed));
struct bt_hci_rsp_user_confirm_request_reply {
	uint8_t  status;
	uint8_t  bdaddr[6];
} __attribute__ ((packed));

#define BT_HCI_CMD_USER_CONFIRM_REQUEST_NEG_REPLY	0x042d
struct bt_hci_cmd_user_confirm_request_neg_reply {
	uint8_t  bdaddr[6];
} __attribute__ ((packed));
struct bt_hci_rsp_user_confirm_request_neg_reply {
	uint8_t  status;
	uint8_t  bdaddr[6];
} __attribute__ ((packed));

#define BT_HCI_CMD_USER_PASSKEY_REQUEST_REPLY		0x042e
struct bt_hci_cmd_user_passkey_request_reply {
	uint8_t  bdaddr[6];
	uint32_t passkey;
} __attribute__ ((packed));

#define BT_HCI_CMD_USER_PASSKEY_REQUEST_NEG_REPLY	0x042f
struct bt_hci_cmd_user_passkey_request_neg_reply {
	uint8_t  bdaddr[6];
} __attribute__ ((packed));

#define BT_HCI_CMD_REMOTE_OOB_DATA_REQUEST_REPLY	0x0430
struct bt_hci_cmd_remote_oob_data_request_reply {
	uint8_t  bdaddr[6];
	uint8_t  hash[16];
	uint8_t  randomizer[16];
} __attribute__ ((packed));

#define BT_HCI_CMD_REMOTE_OOB_DATA_REQUEST_NEG_REPLY	0x0433
struct bt_hci_cmd_remote_oob_data_request_neg_reply {
	uint8_t  bdaddr[6];
} __attribute__ ((packed));

#define BT_HCI_CMD_IO_CAPABILITY_REQUEST_NEG_REPLY	0x0434
struct bt_hci_cmd_io_capability_request_neg_reply {
	uint8_t  bdaddr[6];
	uint8_t  reason;
} __attribute__ ((packed));
struct bt_hci_rsp_io_capability_request_neg_reply {
	uint8_t  status;
	uint8_t  bdaddr[6];
} __attribute__ ((packed));

#define BT_HCI_CMD_CREATE_PHY_LINK		0x0435
struct bt_hci_cmd_create_phy_link {
	uint8_t  phy_handle;
	uint8_t  key_len;
	uint8_t  key_type;
} __attribute__ ((packed));

#define BT_HCI_CMD_ACCEPT_PHY_LINK		0x0436
struct bt_hci_cmd_accept_phy_link {
	uint8_t  phy_handle;
	uint8_t  key_len;
	uint8_t  key_type;
} __attribute__ ((packed));

#define BT_HCI_CMD_DISCONN_PHY_LINK		0x0437
struct bt_hci_cmd_disconn_phy_link {
	uint8_t  phy_handle;
	uint8_t  reason;
} __attribute__ ((packed));

#define BT_HCI_CMD_CREATE_LOGIC_LINK		0x0438
struct bt_hci_cmd_create_logic_link {
	uint8_t  phy_handle;
	uint8_t  tx_flow_spec[16];
	uint8_t  rx_flow_spec[16];
} __attribute__ ((packed));

#define BT_HCI_CMD_ACCEPT_LOGIC_LINK		0x0439
struct bt_hci_cmd_accept_logic_link {
	uint8_t  phy_handle;
	uint8_t  tx_flow_spec[16];
	uint8_t  rx_flow_spec[16];
} __attribute__ ((packed));

#define BT_HCI_CMD_DISCONN_LOGIC_LINK		0x043a
struct bt_hci_cmd_disconn_logic_link {
	uint16_t handle;
} __attribute__ ((packed));

#define BT_HCI_CMD_LOGIC_LINK_CANCEL		0x043b
struct bt_hci_cmd_logic_link_cancel {
	uint8_t  phy_handle;
	uint8_t  flow_spec;
} __attribute__ ((packed));
struct bt_hci_rsp_logic_link_cancel {
	uint8_t  status;
	uint8_t  phy_handle;
	uint8_t  flow_spec;
} __attribute__ ((packed));

#define BT_HCI_CMD_FLOW_SPEC_MODIFY		0x043c
struct bt_hci_cmd_flow_spec_modify {
	uint16_t handle;
	uint8_t  tx_flow_spec[16];
	uint8_t  rx_flow_spec[16];
} __attribute__ ((packed));

#define BT_HCI_CMD_ENHANCED_SETUP_SYNC_CONN	0x043d
struct bt_hci_cmd_enhanced_setup_sync_conn {
	uint16_t handle;
	uint32_t tx_bandwidth;
	uint32_t rx_bandwidth;
	uint8_t  tx_coding_format[5];
	uint8_t  rx_coding_format[5];
	uint16_t tx_codec_frame_size;
	uint16_t rx_codec_frame_size;
	uint32_t input_bandwidth;
	uint32_t output_bandwidth;
	uint8_t  input_coding_format[5];
	uint8_t  output_coding_format[5];
	uint16_t input_coded_data_size;
	uint16_t output_coded_data_size;
	uint8_t  input_pcm_data_format;
	uint8_t  output_pcm_data_format;
	uint8_t  input_pcm_msb_position;
	uint8_t  output_pcm_msb_position;
	uint8_t  input_data_path;
	uint8_t  output_data_path;
	uint8_t  input_unit_size;
	uint8_t  output_unit_size;
	uint16_t max_latency;
	uint16_t pkt_type;
	uint8_t  retrans_effort;
} __attribute__ ((packed));

#define BT_HCI_CMD_ENHANCED_ACCEPT_SYNC_CONN_REQUEST	0x043e
struct bt_hci_cmd_enhanced_accept_sync_conn_request {
	uint8_t  bdaddr[6];
	uint32_t tx_bandwidth;
	uint32_t rx_bandwidth;
	uint8_t  tx_coding_format[5];
	uint8_t  rx_coding_format[5];
	uint16_t tx_codec_frame_size;
	uint16_t rx_codec_frame_size;
	uint32_t input_bandwidth;
	uint32_t output_bandwidth;
	uint8_t  input_coding_format[5];
	uint8_t  output_coding_format[5];
	uint16_t input_coded_data_size;
	uint16_t output_coded_data_size;
	uint8_t  input_pcm_data_format;
	uint8_t  output_pcm_data_format;
	uint8_t  input_pcm_msb_position;
	uint8_t  output_pcm_msb_position;
	uint8_t  input_data_path;
	uint8_t  output_data_path;
	uint8_t  input_unit_size;
	uint8_t  output_unit_size;
	uint16_t max_latency;
	uint16_t pkt_type;
	uint8_t  retrans_effort;
} __attribute__ ((packed));

#define BT_HCI_CMD_TRUNCATED_PAGE		0x043f
struct bt_hci_cmd_truncated_page {
	uint8_t  bdaddr[6];
	uint8_t  pscan_rep_mode;
	uint16_t clock_offset;
} __attribute__ ((packed));

#define BT_HCI_CMD_TRUNCATED_PAGE_CANCEL	0x0440
struct bt_hci_cmd_truncated_page_cancel {
	uint8_t  bdaddr[6];
} __attribute__ ((packed));

#define BT_HCI_CMD_SET_SLAVE_BROADCAST		0x0441
struct bt_hci_cmd_set_slave_broadcast {
	uint8_t  enable;
	uint8_t  lt_addr;
	uint8_t  lpo_allowed;
	uint16_t pkt_type;
	uint16_t min_interval;
	uint16_t max_interval;
	uint16_t timeout;
} __attribute__ ((packed));
struct bt_hci_rsp_set_slave_broadcast {
	uint8_t  status;
	uint8_t  lt_addr;
	uint16_t interval;
} __attribute__ ((packed));

#define BT_HCI_CMD_SET_SLAVE_BROADCAST_RECEIVE	0x0442
struct bt_hci_cmd_set_slave_broadcast_receive {
	uint8_t  enable;
	uint8_t  bdaddr[6];
	uint8_t  lt_addr;
	uint16_t interval;
	uint32_t offset;
	uint32_t instant;
	uint16_t timeout;
	uint8_t  accuracy;
	uint8_t  skip;
	uint16_t pkt_type;
	uint8_t  map[10];
} __attribute__ ((packed));
struct bt_hci_rsp_set_slave_broadcast_receive {
	uint8_t  status;
	uint8_t  bdaddr[6];
	uint8_t  lt_addr;
} __attribute__ ((packed));

#define BT_HCI_CMD_START_SYNC_TRAIN		0x0443

#define BT_HCI_CMD_RECEIVE_SYNC_TRAIN		0x0444
struct bt_hci_cmd_receive_sync_train {
	uint8_t  bdaddr[6];
	uint16_t timeout;
	uint16_t window;
	uint16_t interval;
} __attribute__ ((packed));

#define BT_HCI_CMD_REMOTE_OOB_EXT_DATA_REQUEST_REPLY	0x0445
struct bt_hci_cmd_remote_oob_ext_data_request_reply {
	uint8_t  bdaddr[6];
	uint8_t  hash192[16];
	uint8_t  randomizer192[16];
	uint8_t  hash256[16];
	uint8_t  randomizer256[16];
} __attribute__ ((packed));

#define BT_HCI_CMD_HOLD_MODE			0x0801
struct bt_hci_cmd_hold_mode {
	uint16_t handle;
	uint16_t max_interval;
	uint16_t min_interval;
} __attribute__ ((packed));

#define BT_HCI_CMD_SNIFF_MODE			0x0803
struct bt_hci_cmd_sniff_mode {
	uint16_t handle;
	uint16_t max_interval;
	uint16_t min_interval;
	uint16_t attempt;
	uint16_t timeout;
} __attribute__ ((packed));

#define BT_HCI_CMD_EXIT_SNIFF_MODE		0x0804
struct bt_hci_cmd_exit_sniff_mode {
	uint16_t handle;
} __attribute__ ((packed));

#define BT_HCI_CMD_PARK_STATE			0x0805
struct bt_hci_cmd_park_state {
	uint16_t handle;
	uint16_t max_interval;
	uint16_t min_interval;
} __attribute__ ((packed));

#define BT_HCI_CMD_EXIT_PARK_STATE		0x0806
struct bt_hci_cmd_exit_park_state {
	uint16_t handle;
} __attribute__ ((packed));

#define BT_HCI_CMD_QOS_SETUP			0x0807
struct bt_hci_cmd_qos_setup {
	uint16_t handle;
	uint8_t  flags;
	uint8_t  service_type;
	uint32_t token_rate;
	uint32_t peak_bandwidth;
	uint32_t latency;
	uint32_t delay_variation;
} __attribute__ ((packed));

#define BT_HCI_CMD_ROLE_DISCOVERY		0x0809
struct bt_hci_cmd_role_discovery {
	uint16_t handle;
} __attribute__ ((packed));
struct bt_hci_rsp_role_discovery {
	uint8_t  status;
	uint16_t handle;
	uint8_t  role;
} __attribute__ ((packed));

#define BT_HCI_CMD_SWITCH_ROLE			0x080b
struct bt_hci_cmd_switch_role {
	uint8_t  bdaddr[6];
	uint8_t  role;
} __attribute__ ((packed));

#define BT_HCI_CMD_READ_LINK_POLICY		0x080c
struct bt_hci_cmd_read_link_policy {
	uint16_t handle;
} __attribute__ ((packed));
struct bt_hci_rsp_read_link_policy {
	uint8_t  status;
	uint16_t handle;
	uint16_t policy;
} __attribute__ ((packed));

#define BT_HCI_CMD_WRITE_LINK_POLICY		0x080d
struct bt_hci_cmd_write_link_policy {
	uint16_t handle;
	uint16_t policy;
} __attribute__ ((packed));
struct bt_hci_rsp_write_link_policy {
	uint8_t  status;
	uint16_t handle;
} __attribute__ ((packed));

#define BT_HCI_CMD_READ_DEFAULT_LINK_POLICY	0x080e
struct bt_hci_rsp_read_default_link_policy {
	uint8_t  status;
	uint16_t policy;
} __attribute__ ((packed));

#define BT_HCI_CMD_WRITE_DEFAULT_LINK_POLICY	0x080f
struct bt_hci_cmd_write_default_link_policy {
	uint16_t policy;
} __attribute__ ((packed));

#define BT_HCI_CMD_FLOW_SPEC			0x0810
struct bt_hci_cmd_flow_spec {
	uint16_t handle;
	uint8_t  flags;
	uint8_t  direction;
	uint8_t  service_type;
	uint32_t token_rate;
	uint32_t token_bucket_size;
	uint32_t peak_bandwidth;
	uint32_t access_latency;
} __attribute__ ((packed));

#define BT_HCI_CMD_SNIFF_SUBRATING		0x0811
struct bt_hci_cmd_sniff_subrating {
	uint16_t handle;
	uint16_t max_latency;
	uint16_t min_remote_timeout;
	uint16_t min_local_timeout;
} __attribute__ ((packed));
struct bt_hci_rsp_sniff_subrating {
	uint8_t  status;
	uint16_t handle;
} __attribute__ ((packed));

#define BT_HCI_CMD_SET_EVENT_MASK		0x0c01
struct bt_hci_cmd_set_event_mask {
	uint8_t  mask[8];
} __attribute__ ((packed));

#define BT_HCI_CMD_RESET			0x0c03

#define BT_HCI_CMD_SET_EVENT_FILTER		0x0c05
struct bt_hci_cmd_set_event_filter {
	uint8_t  type;
	uint8_t  cond_type;
	uint8_t  cond[0];
} __attribute__ ((packed));

#define BT_HCI_CMD_FLUSH			0x0c08
struct bt_hci_cmd_flush {
	uint16_t handle;
} __attribute__ ((packed));
struct bt_hci_rsp_flush {
	uint8_t  status;
	uint16_t handle;
} __attribute__ ((packed));

#define BT_HCI_CMD_READ_PIN_TYPE		0x0c09
struct bt_hci_rsp_read_pin_type {
	uint8_t  status;
	uint8_t  pin_type;
} __attribute__ ((packed));

#define BT_HCI_CMD_WRITE_PIN_TYPE		0x0c0a
struct bt_hci_cmd_write_pin_type {
	uint8_t  pin_type;
} __attribute__ ((packed));

#define BT_HCI_CMD_CREATE_NEW_UNIT_KEY		0x0c0b

#define BT_HCI_CMD_READ_STORED_LINK_KEY		0x0c0d
struct bt_hci_cmd_read_stored_link_key {
	uint8_t  bdaddr[6];
	uint8_t  read_all;
} __attribute__ ((packed));
struct bt_hci_rsp_read_stored_link_key {
	uint8_t  status;
	uint16_t max_num_keys;
	uint16_t num_keys;
} __attribute__ ((packed));

#define BT_HCI_CMD_WRITE_STORED_LINK_KEY	0x0c11
struct bt_hci_cmd_write_stored_link_key {
	uint8_t  num_keys;
} __attribute__ ((packed));
struct bt_hci_rsp_write_stored_link_key {
	uint8_t  status;
	uint8_t  num_keys;
} __attribute__ ((packed));

#define BT_HCI_CMD_DELETE_STORED_LINK_KEY	0x0c12
struct bt_hci_cmd_delete_stored_link_key {
	uint8_t  bdaddr[6];
	uint8_t  delete_all;
} __attribute__ ((packed));
struct bt_hci_rsp_delete_stored_link_key {
	uint8_t  status;
	uint16_t num_keys;
} __attribute__ ((packed));

#define BT_HCI_CMD_WRITE_LOCAL_NAME		0x0c13
struct bt_hci_cmd_write_local_name {
	uint8_t  name[248];
} __attribute__ ((packed));

#define BT_HCI_CMD_READ_LOCAL_NAME		0x0c14
struct bt_hci_rsp_read_local_name {
	uint8_t  status;
	uint8_t  name[248];
} __attribute__ ((packed));

#define BT_HCI_CMD_READ_CONN_ACCEPT_TIMEOUT	0x0c15
struct bt_hci_rsp_read_conn_accept_timeout {
	uint8_t  status;
	uint16_t timeout;
} __attribute__ ((packed));

#define BT_HCI_CMD_WRITE_CONN_ACCEPT_TIMEOUT	0x0c16
struct bt_hci_cmd_write_conn_accept_timeout {
	uint16_t timeout;
} __attribute__ ((packed));

#define BT_HCI_CMD_READ_PAGE_TIMEOUT		0x0c17
struct bt_hci_rsp_read_page_timeout {
	uint8_t  status;
	uint16_t timeout;
} __attribute__ ((packed));

#define BT_HCI_CMD_WRITE_PAGE_TIMEOUT		0x0c18
struct bt_hci_cmd_write_page_timeout {
	uint16_t timeout;
} __attribute__ ((packed));

#define BT_HCI_CMD_READ_SCAN_ENABLE		0x0c19
struct bt_hci_rsp_read_scan_enable {
	uint8_t  status;
	uint8_t  enable;
} __attribute__ ((packed));

#define BT_HCI_CMD_WRITE_SCAN_ENABLE		0x0c1a
struct bt_hci_cmd_write_scan_enable {
	uint8_t  enable;
} __attribute__ ((packed));

#define BT_HCI_CMD_READ_PAGE_SCAN_ACTIVITY	0x0c1b
struct bt_hci_rsp_read_page_scan_activity {
	uint8_t  status;
	uint16_t interval;
	uint16_t window;
} __attribute__ ((packed));

#define BT_HCI_CMD_WRITE_PAGE_SCAN_ACTIVITY	0x0c1c
struct bt_hci_cmd_write_page_scan_activity {
	uint16_t interval;
	uint16_t window;
} __attribute__ ((packed));

#define BT_HCI_CMD_READ_INQUIRY_SCAN_ACTIVITY	0x0c1d
struct bt_hci_rsp_read_inquiry_scan_activity {
	uint8_t  status;
	uint16_t interval;
	uint16_t window;
} __attribute__ ((packed));

#define BT_HCI_CMD_WRITE_INQUIRY_SCAN_ACTIVITY	0x0c1e
struct bt_hci_cmd_write_inquiry_scan_activity {
	uint16_t interval;
	uint16_t window;
} __attribute__ ((packed));

#define BT_HCI_CMD_READ_AUTH_ENABLE		0x0c1f
struct bt_hci_rsp_read_auth_enable {
	uint8_t  status;
	uint8_t  enable;
} __attribute__ ((packed));

#define BT_HCI_CMD_WRITE_AUTH_ENABLE		0x0c20
struct bt_hci_cmd_write_auth_enable {
	uint8_t  enable;
} __attribute__ ((packed));

#define BT_HCI_CMD_READ_ENCRYPT_MODE		0x0c21
struct bt_hci_rsp_read_encrypt_mode {
	uint8_t  status;
	uint8_t  mode;
} __attribute__ ((packed));

#define BT_HCI_CMD_WRITE_ENCRYPT_MODE		0x0c22
struct bt_hci_cmd_write_encrypt_mode {
	uint8_t  mode;
} __attribute__ ((packed));

#define BT_HCI_CMD_READ_CLASS_OF_DEV		0x0c23
struct bt_hci_rsp_read_class_of_dev {
	uint8_t  status;
	uint8_t  dev_class[3];
} __attribute__ ((packed));

#define BT_HCI_CMD_WRITE_CLASS_OF_DEV		0x0c24
struct bt_hci_cmd_write_class_of_dev {
	uint8_t  dev_class[3];
} __attribute__ ((packed));

#define BT_HCI_CMD_READ_VOICE_SETTING		0x0c25
struct bt_hci_rsp_read_voice_setting {
	uint8_t  status;
	uint16_t setting;
} __attribute__ ((packed));

#define BT_HCI_CMD_WRITE_VOICE_SETTING		0x0c26
struct bt_hci_cmd_write_voice_setting {
	uint16_t setting;
} __attribute__ ((packed));

#define BT_HCI_CMD_READ_AUTO_FLUSH_TIMEOUT	0x0c27
struct bt_hci_cmd_read_auto_flush_timeout {
	uint16_t handle;
} __attribute__ ((packed));
struct bt_hci_rsp_read_auto_flush_timeout {
	uint8_t  status;
	uint16_t handle;
	uint16_t timeout;
} __attribute__ ((packed));

#define BT_HCI_CMD_WRITE_AUTO_FLUSH_TIMEOUT	0x0c28
struct bt_hci_cmd_write_auto_flush_timeout {
	uint16_t handle;
	uint16_t timeout;
} __attribute__ ((packed));
struct bt_hci_rsp_write_auto_flush_timeout {
	uint8_t  status;
	uint16_t handle;
} __attribute__ ((packed));

#define BT_HCI_CMD_READ_NUM_BROADCAST_RETRANS	0x0c29
struct bt_hci_rsp_read_num_broadcast_retrans {
	uint8_t  status;
	uint8_t  num_retrans;
} __attribute__ ((packed));

#define BT_HCI_CMD_WRITE_NUM_BROADCAST_RETRANS	0x0c2a
struct bt_hci_cmd_write_num_broadcast_retrans {
	uint8_t  num_retrans;
} __attribute__ ((packed));

#define BT_HCI_CMD_READ_HOLD_MODE_ACTIVITY	0x0c2b
struct bt_hci_rsp_read_hold_mode_activity {
	uint8_t  status;
	uint8_t  activity;
} __attribute__ ((packed));

#define BT_HCI_CMD_WRITE_HOLD_MODE_ACTIVITY	0x0c2c
struct bt_hci_cmd_write_hold_mode_activity {
	uint8_t  activity;
} __attribute__ ((packed));

#define BT_HCI_CMD_READ_TX_POWER		0x0c2d
struct bt_hci_cmd_read_tx_power {
	uint16_t handle;
	uint8_t  type;
} __attribute__ ((packed));
struct bt_hci_rsp_read_tx_power {
	uint8_t  status;
	uint16_t handle;
	int8_t   level;
} __attribute__ ((packed));

#define BT_HCI_CMD_READ_SYNC_FLOW_CONTROL	0x0c2e
struct bt_hci_rsp_read_sync_flow_control {
	uint8_t  status;
	uint8_t  enable;
} __attribute__ ((packed));

#define BT_HCI_CMD_WRITE_SYNC_FLOW_CONTROL	0x0c2f
struct bt_hci_cmd_write_sync_flow_control {
	uint8_t  enable;
} __attribute__ ((packed));

#define BT_HCI_CMD_SET_HOST_FLOW_CONTROL	0x0c31
struct bt_hci_cmd_set_host_flow_control {
	uint8_t  enable;
} __attribute__ ((packed));

#define BT_HCI_CMD_HOST_BUFFER_SIZE		0x0c33
struct bt_hci_cmd_host_buffer_size {
	uint16_t acl_mtu;
	uint8_t  sco_mtu;
	uint16_t acl_max_pkt;
	uint16_t sco_max_pkt;
} __attribute__ ((packed));

#define BT_HCI_CMD_HOST_NUM_COMPLETED_PACKETS	0x0c35
struct bt_hci_cmd_host_num_completed_packets {
	uint8_t  num_handles;
	uint16_t handle;
	uint16_t count;
} __attribute__ ((packed));

#define BT_HCI_CMD_READ_LINK_SUPV_TIMEOUT	0x0c36
struct bt_hci_cmd_read_link_supv_timeout {
	uint16_t handle;
} __attribute__ ((packed));
struct bt_hci_rsp_read_link_supv_timeout {
	uint8_t  status;
	uint16_t handle;
	uint16_t timeout;
} __attribute__ ((packed));

#define BT_HCI_CMD_WRITE_LINK_SUPV_TIMEOUT	0x0c37
struct bt_hci_cmd_write_link_supv_timeout {
	uint16_t handle;
	uint16_t timeout;
} __attribute__ ((packed));
struct bt_hci_rsp_write_link_supv_timeout {
	uint8_t  status;
	uint16_t handle;
} __attribute__ ((packed));

#define BT_HCI_CMD_READ_NUM_SUPPORTED_IAC	0x0c38
struct bt_hci_rsp_read_num_supported_iac {
	uint8_t  status;
	uint8_t  num_iac;
} __attribute__ ((packed));

#define BT_HCI_CMD_READ_CURRENT_IAC_LAP		0x0c39
struct bt_hci_rsp_read_current_iac_lap {
	uint8_t  status;
	uint8_t  num_iac;
	uint8_t  iac_lap[0];
} __attribute__ ((packed));

#define BT_HCI_CMD_WRITE_CURRENT_IAC_LAP	0x0c3a
struct bt_hci_cmd_write_current_iac_lap {
	uint8_t  num_iac;
	uint8_t  iac_lap[0];
} __attribute__ ((packed));

#define BT_HCI_CMD_READ_PAGE_SCAN_PERIOD_MODE	0x0c3b
struct bt_hci_rsp_read_page_scan_period_mode {
	uint8_t  status;
	uint8_t  mode;
} __attribute__ ((packed));

#define BT_HCI_CMD_WRITE_PAGE_SCAN_PERIOD_MODE	0x0c3c
struct bt_hci_cmd_write_page_scan_period_mode {
	uint8_t  mode;
} __attribute__ ((packed));

#define BT_HCI_CMD_READ_PAGE_SCAN_MODE		0x0c3d
struct bt_hci_rsp_read_page_scan_mode {
	uint8_t  status;
	uint8_t  mode;
} __attribute__ ((packed));

#define BT_HCI_CMD_WRITE_PAGE_SCAN_MODE		0x0c3e
struct bt_hci_cmd_write_page_scan_mode {
	uint8_t  mode;
} __attribute__ ((packed));

#define BT_HCI_CMD_SET_AFH_HOST_CLASSIFICATION	0x0c3f
struct bt_hci_cmd_set_afh_host_classification {
	uint8_t  map[10];
} __attribute__ ((packed));

#define BT_HCI_CMD_READ_INQUIRY_SCAN_TYPE	0x0c42
struct bt_hci_rsp_read_inquiry_scan_type {
	uint8_t  status;
	uint8_t  type;
} __attribute__ ((packed));

#define BT_HCI_CMD_WRITE_INQUIRY_SCAN_TYPE	0x0c43
struct bt_hci_cmd_write_inquiry_scan_type {
	uint8_t type;
} __attribute__ ((packed));

#define BT_HCI_CMD_READ_INQUIRY_MODE		0x0c44
struct bt_hci_rsp_read_inquiry_mode {
	uint8_t  status;
	uint8_t  mode;
} __attribute__ ((packed));

#define BT_HCI_CMD_WRITE_INQUIRY_MODE		0x0c45
struct bt_hci_cmd_write_inquiry_mode {
	uint8_t  mode;
} __attribute__ ((packed));

#define BT_HCI_CMD_READ_PAGE_SCAN_TYPE		0x0c46
struct bt_hci_rsp_read_page_scan_type {
	uint8_t status;
	uint8_t type;
} __attribute__ ((packed));

#define BT_HCI_CMD_WRITE_PAGE_SCAN_TYPE		0x0c47
struct bt_hci_cmd_write_page_scan_type {
	uint8_t type;
} __attribute__ ((packed));

#define BT_HCI_CMD_READ_AFH_ASSESSMENT_MODE	0x0c48
struct bt_hci_rsp_read_afh_assessment_mode {
	uint8_t  status;
	uint8_t  mode;
} __attribute__ ((packed));

#define BT_HCI_CMD_WRITE_AFH_ASSESSMENT_MODE	0x0c49
struct bt_hci_cmd_write_afh_assessment_mode {
	uint8_t  mode;
} __attribute__ ((packed));

#define BT_HCI_CMD_READ_EXT_INQUIRY_RESPONSE	0x0c51
struct bt_hci_rsp_read_ext_inquiry_response {
	uint8_t  status;
	uint8_t  fec;
	uint8_t  data[240];
} __attribute__ ((packed));

#define BT_HCI_CMD_WRITE_EXT_INQUIRY_RESPONSE	0x0c52
struct bt_hci_cmd_write_ext_inquiry_response {
	uint8_t  fec;
	uint8_t  data[240];
} __attribute__ ((packed));

#define BT_HCI_CMD_REFRESH_ENCRYPT_KEY		0x0c53
struct bt_hci_cmd_refresh_encrypt_key {
	uint16_t handle;
} __attribute__ ((packed));

#define BT_HCI_CMD_READ_SIMPLE_PAIRING_MODE	0x0c55
struct bt_hci_rsp_read_simple_pairing_mode {
	uint8_t  status;
	uint8_t  mode;
} __attribute__ ((packed));

#define BT_HCI_CMD_WRITE_SIMPLE_PAIRING_MODE	0x0c56
struct bt_hci_cmd_write_simple_pairing_mode {
	uint8_t  mode;
} __attribute__ ((packed));

#define BT_HCI_CMD_READ_LOCAL_OOB_DATA		0x0c57
struct bt_hci_rsp_read_local_oob_data {
	uint8_t  status;
	uint8_t  hash[16];
	uint8_t  randomizer[16];
} __attribute__ ((packed));

#define BT_HCI_CMD_READ_INQUIRY_RESP_TX_POWER	0x0c58
struct bt_hci_rsp_read_inquiry_resp_tx_power {
	uint8_t  status;
	int8_t   level;
} __attribute__ ((packed));

#define BT_HCI_CMD_WRITE_INQUIRY_TX_POWER	0x0c59
struct bt_hci_cmd_write_inquiry_tx_power {
	int8_t   level;
} __attribute__ ((packed));

#define BT_HCI_CMD_READ_ERRONEOUS_REPORTING	0x0c5a
struct bt_hci_rsp_read_erroneous_reporting {
	uint8_t  status;
	uint8_t  mode;
} __attribute__ ((packed));

#define BT_HCI_CMD_WRITE_ERRONEOUS_REPORTING	0x0c5b
struct bt_hci_cmd_write_erroneous_reporting {
	uint8_t  mode;
} __attribute__ ((packed));

#define BT_HCI_CMD_ENHANCED_FLUSH		0x0c5f
struct bt_hci_cmd_enhanced_flush {
	uint16_t handle;
	uint8_t  type;
} __attribute__ ((packed));

#define BT_HCI_CMD_SEND_KEYPRESS_NOTIFY		0x0c60
struct bt_hci_cmd_send_keypress_notify {
	uint8_t  bdaddr[6];
	uint8_t  type;
} __attribute__ ((packed));
struct bt_hci_rsp_send_keypress_notify {
	uint8_t  status;
	uint8_t  bdaddr[6];
} __attribute__ ((packed));

#define BT_HCI_CMD_SET_EVENT_MASK_PAGE2		0x0c63
struct bt_hci_cmd_set_event_mask_page2 {
	uint8_t  mask[8];
} __attribute__ ((packed));

#define BT_HCI_CMD_READ_LOCATION_DATA		0x0c64
struct bt_hci_rsp_read_location_data {
	uint8_t  status;
	uint8_t  domain_aware;
	uint8_t  domain[2];
	uint8_t  domain_options;
	uint8_t  options;
} __attribute__ ((packed));

#define BT_HCI_CMD_WRITE_LOCATION_DATA		0x0c65
struct bt_hci_cmd_write_location_data {
	uint8_t  domain_aware;
	uint8_t  domain[2];
	uint8_t  domain_options;
	uint8_t  options;
} __attribute__ ((packed));

#define BT_HCI_CMD_READ_FLOW_CONTROL_MODE	0x0c66
struct bt_hci_rsp_read_flow_control_mode {
	uint8_t  status;
	uint8_t  mode;
} __attribute__ ((packed));

#define BT_HCI_CMD_WRITE_FLOW_CONTROL_MODE	0x0c67
struct bt_hci_cmd_write_flow_control_mode {
	uint8_t  mode;
} __attribute__ ((packed));

#define BT_HCI_CMD_READ_ENHANCED_TX_POWER	0x0c68
struct bt_hci_cmd_read_enhanced_tx_power {
	uint16_t handle;
	uint8_t  type;
} __attribute__ ((packed));
struct bt_hci_rsp_read_enhanced_tx_power {
	uint8_t  status;
	uint16_t handle;
	int8_t   level_gfsk;
	int8_t   level_dqpsk;
	int8_t   level_8dpsk;
} __attribute__ ((packed));

#define BT_HCI_CMD_SHORT_RANGE_MODE		0x0c6b
struct bt_hci_cmd_short_range_mode {
	uint8_t  phy_handle;
	uint8_t  mode;
} __attribute__ ((packed));

#define BT_HCI_CMD_READ_LE_HOST_SUPPORTED	0x0c6c
struct bt_hci_rsp_read_le_host_supported {
	uint8_t  status;
	uint8_t  supported;
	uint8_t  simultaneous;
} __attribute__ ((packed));

#define BT_HCI_CMD_WRITE_LE_HOST_SUPPORTED	0x0c6d
struct bt_hci_cmd_write_le_host_supported {
	uint8_t  supported;
	uint8_t  simultaneous;
} __attribute__ ((packed));

#define BT_HCI_CMD_SET_RESERVED_LT_ADDR		0x0c74
struct bt_hci_cmd_set_reserved_lt_addr {
	uint8_t  lt_addr;
} __attribute__ ((packed));
struct bt_hci_rsp_set_reserved_lt_addr {
	uint8_t  status;
	uint8_t  lt_addr;
} __attribute__ ((packed));

#define BT_HCI_CMD_DELETE_RESERVED_LT_ADDR	0x0c75
struct bt_hci_cmd_delete_reserved_lt_addr {
	uint8_t  lt_addr;
} __attribute__ ((packed));
struct bt_hci_rsp_delete_reserved_lt_addr {
	uint8_t  status;
	uint8_t  lt_addr;
} __attribute__ ((packed));

#define BT_HCI_CMD_SET_SLAVE_BROADCAST_DATA	0x0c76
struct bt_hci_cmd_set_slave_broadcast_data {
	uint8_t  lt_addr;
	uint8_t  fragment;
	uint8_t  length;
} __attribute__ ((packed));
struct bt_hci_rsp_set_slave_broadcast_data {
	uint8_t  status;
	uint8_t  lt_addr;
} __attribute__ ((packed));

#define BT_HCI_CMD_READ_SYNC_TRAIN_PARAMS	0x0c77
struct bt_hci_rsp_read_sync_train_params {
	uint8_t  status;
	uint16_t interval;
	uint32_t timeout;
	uint8_t  service_data;
} __attribute__ ((packed));

#define BT_HCI_CMD_WRITE_SYNC_TRAIN_PARAMS	0x0c78
struct bt_hci_cmd_write_sync_train_params {
	uint16_t min_interval;
	uint16_t max_interval;
	uint32_t timeout;
	uint8_t  service_data;
} __attribute__ ((packed));
struct bt_hci_rsp_write_sync_train_params {
	uint8_t  status;
	uint16_t interval;
} __attribute__ ((packed));

#define BT_HCI_CMD_READ_SECURE_CONN_SUPPORT	0x0c79
struct bt_hci_rsp_read_secure_conn_support {
	uint8_t  status;
	uint8_t  support;
} __attribute__ ((packed));

#define BT_HCI_CMD_WRITE_SECURE_CONN_SUPPORT	0x0c7a
struct bt_hci_cmd_write_secure_conn_support {
	uint8_t support;
} __attribute__ ((packed));

#define BT_HCI_CMD_READ_AUTH_PAYLOAD_TIMEOUT	0x0c7b
struct bt_hci_cmd_read_auth_payload_timeout {
	uint16_t handle;
} __attribute__ ((packed));
struct bt_hci_rsp_read_auth_payload_timeout {
	uint8_t  status;
	uint16_t handle;
	uint16_t timeout;
} __attribute__ ((packed));

#define BT_HCI_CMD_WRITE_AUTH_PAYLOAD_TIMEOUT	0x0c7c
struct bt_hci_cmd_write_auth_payload_timeout {
	uint16_t handle;
	uint16_t timeout;
} __attribute__ ((packed));
struct bt_hci_rsp_write_auth_payload_timeout {
	uint8_t  status;
	uint16_t handle;
} __attribute__ ((packed));

#define BT_HCI_CMD_READ_LOCAL_OOB_EXT_DATA	0x0c7d
struct bt_hci_rsp_read_local_oob_ext_data {
	uint8_t  status;
	uint8_t  hash192[16];
	uint8_t  randomizer192[16];
	uint8_t  hash256[16];
	uint8_t  randomizer256[16];
} __attribute__ ((packed));

#define BT_HCI_CMD_READ_EXT_PAGE_TIMEOUT	0x0c7e
struct bt_hci_rsp_read_ext_page_timeout {
	uint8_t  status;
	uint16_t timeout;
} __attribute__ ((packed));

#define BT_HCI_CMD_WRITE_EXT_PAGE_TIMEOUT	0x0c7f
struct bt_hci_cmd_write_ext_page_timeout {
	uint16_t timeout;
} __attribute__ ((packed));

#define BT_HCI_CMD_READ_EXT_INQUIRY_LENGTH	0x0c80
struct bt_hci_rsp_read_ext_inquiry_length {
	uint8_t  status;
	uint16_t interval;
} __attribute__ ((packed));

#define BT_HCI_CMD_WRITE_EXT_INQUIRY_LENGTH	0x0c81
struct bt_hci_cmd_write_ext_inquiry_length {
	uint16_t interval;
} __attribute__ ((packed));

#define BT_HCI_CMD_READ_LOCAL_VERSION		0x1001
struct bt_hci_rsp_read_local_version {
	uint8_t  status;
	uint8_t  hci_ver;
	uint16_t hci_rev;
	uint8_t  lmp_ver;
	uint16_t manufacturer;
	uint16_t lmp_subver;
} __attribute__ ((packed));

#define BT_HCI_CMD_READ_LOCAL_COMMANDS		0x1002
struct bt_hci_rsp_read_local_commands {
	uint8_t  status;
	uint8_t  commands[64];
} __attribute__ ((packed));

#define BT_HCI_CMD_READ_LOCAL_FEATURES		0x1003
struct bt_hci_rsp_read_local_features {
	uint8_t  status;
	uint8_t  features[8];
} __attribute__ ((packed));

#define BT_HCI_CMD_READ_LOCAL_EXT_FEATURES	0x1004
struct bt_hci_cmd_read_local_ext_features {
	uint8_t  page;
} __attribute__ ((packed));
struct bt_hci_rsp_read_local_ext_features {
	uint8_t  status;
	uint8_t  page;
	uint8_t  max_page;
	uint8_t  features[8];
} __attribute__ ((packed));

#define BT_HCI_CMD_READ_BUFFER_SIZE		0x1005
struct bt_hci_rsp_read_buffer_size {
	uint8_t  status;
	uint16_t acl_mtu;
	uint8_t  sco_mtu;
	uint16_t acl_max_pkt;
	uint16_t sco_max_pkt;
} __attribute__ ((packed));

#define BT_HCI_CMD_READ_COUNTRY_CODE		0x1007
struct bt_hci_rsp_read_country_code {
	uint8_t  status;
	uint8_t  code;
} __attribute__ ((packed));

#define BT_HCI_CMD_READ_BD_ADDR			0x1009
struct bt_hci_rsp_read_bd_addr {
	uint8_t  status;
	uint8_t  bdaddr[6];
} __attribute__ ((packed));

#define BT_HCI_CMD_READ_DATA_BLOCK_SIZE		0x100a
struct bt_hci_rsp_read_data_block_size {
	uint8_t  status;
	uint16_t max_acl_len;
	uint16_t block_len;
	uint16_t num_blocks;
} __attribute__ ((packed));

#define BT_HCI_CMD_READ_LOCAL_CODECS		0x100b
struct bt_hci_rsp_read_local_codecs {
	uint8_t  status;
	uint8_t  num_codecs;
	uint8_t  codec[0];
} __attribute__ ((packed));

#define BT_HCI_CMD_READ_FAILED_CONTACT_COUNTER	0x1401
struct bt_hci_cmd_read_failed_contact_counter {
	uint16_t handle;
} __attribute__ ((packed));
struct bt_hci_rsp_read_failed_contact_counter {
	uint8_t  status;
	uint16_t handle;
	uint16_t counter;
} __attribute__ ((packed));

#define BT_HCI_CMD_RESET_FAILED_CONTACT_COUNTER	0x1402
struct bt_hci_cmd_reset_failed_contact_counter {
	uint16_t handle;
} __attribute__ ((packed));
struct bt_hci_rsp_reset_failed_contact_counter {
	uint8_t  status;
	uint16_t handle;
} __attribute__ ((packed));

#define BT_HCI_CMD_READ_LINK_QUALITY		0x1403
struct bt_hci_cmd_read_link_quality {
	uint16_t handle;
} __attribute__ ((packed));
struct bt_hci_rsp_read_link_quality {
	uint8_t  status;
	uint16_t handle;
	uint8_t  link_quality;
} __attribute__ ((packed));

#define BT_HCI_CMD_READ_RSSI			0x1405
struct bt_hci_cmd_read_rssi {
	uint16_t handle;
} __attribute__ ((packed));
struct bt_hci_rsp_read_rssi {
	uint8_t  status;
	uint16_t handle;
	int8_t   rssi;
} __attribute__ ((packed));

#define BT_HCI_CMD_READ_AFH_CHANNEL_MAP		0x1406
struct bt_hci_cmd_read_afh_channel_map {
	uint16_t handle;
} __attribute__ ((packed));
struct bt_hci_rsp_read_afh_channel_map {
	uint8_t  status;
	uint16_t handle;
	uint8_t  mode;
	uint8_t  map[10];
} __attribute__ ((packed));

#define BT_HCI_CMD_READ_CLOCK			0x1407
struct bt_hci_cmd_read_clock {
	uint16_t handle;
	uint8_t  type;
} __attribute__ ((packed));
struct bt_hci_rsp_read_clock {
	uint8_t  status;
	uint16_t handle;
	uint32_t clock;
	uint16_t accuracy;
} __attribute__ ((packed));

#define BT_HCI_CMD_READ_ENCRYPT_KEY_SIZE	0x1408
struct bt_hci_cmd_read_encrypt_key_size {
	uint16_t handle;
} __attribute__ ((packed));
struct bt_hci_rsp_read_encrypt_key_size {
	uint8_t  status;
	uint16_t handle;
	uint8_t  key_size;
} __attribute__ ((packed));

#define BT_HCI_CMD_READ_LOCAL_AMP_INFO		0x1409
struct bt_hci_rsp_read_local_amp_info {
	uint8_t  status;
	uint8_t  amp_status;
	uint32_t total_bw;
	uint32_t max_bw;
	uint32_t min_latency;
	uint32_t max_pdu;
	uint8_t  amp_type;
	uint16_t pal_cap;
	uint16_t max_assoc_len;
	uint32_t max_flush_to;
	uint32_t be_flush_to;
} __attribute__ ((packed));

#define BT_HCI_CMD_READ_LOCAL_AMP_ASSOC		0x140a
struct bt_hci_cmd_read_local_amp_assoc {
	uint8_t  phy_handle;
	uint16_t len_so_far;
	uint16_t max_assoc_len;
} __attribute__ ((packed));
struct bt_hci_rsp_read_local_amp_assoc {
	uint8_t  status;
	uint8_t  phy_handle;
	uint16_t remain_assoc_len;
	uint8_t  assoc_fragment[248];
} __attribute__ ((packed));

#define BT_HCI_CMD_WRITE_REMOTE_AMP_ASSOC	0x140b
struct bt_hci_cmd_write_remote_amp_assoc {
	uint8_t  phy_handle;
	uint16_t len_so_far;
	uint16_t remain_assoc_len;
	uint8_t  assoc_fragment[248];
} __attribute__ ((packed));
struct bt_hci_rsp_write_remote_amp_assoc {
	uint8_t  status;
	uint8_t  phy_handle;
} __attribute__ ((packed));

#define BT_HCI_CMD_GET_MWS_TRANSPORT_CONFIG	0x140c
struct bt_hci_rsp_get_mws_transport_config {
	uint8_t  status;
	uint8_t  num_transports;
	uint8_t  transport[0];
} __attribute__ ((packed));

#define BT_HCI_CMD_SET_TRIGGERED_CLOCK_CAPTURE	0x140d
struct bt_hci_cmd_set_triggered_clock_capture {
	uint16_t handle;
	uint8_t  enable;
	uint8_t  type;
	uint8_t  lpo_allowed;
	uint8_t  num_filter;
} __attribute__ ((packed));

#define BT_HCI_CMD_READ_LOOPBACK_MODE		0x1801
struct bt_hci_rsp_read_loopback_mode {
	uint8_t  status;
	uint8_t  mode;
} __attribute__ ((packed));

#define BT_HCI_CMD_WRITE_LOOPBACK_MODE		0x1802
struct bt_hci_cmd_write_loopback_mode {
	uint8_t  mode;
} __attribute__ ((packed));

#define BT_HCI_CMD_ENABLE_DUT_MODE		0x1803

#define BT_HCI_CMD_WRITE_SSP_DEBUG_MODE		0x1804
struct bt_hci_cmd_write_ssp_debug_mode {
	uint8_t  mode;
} __attribute__ ((packed));

#define BT_HCI_CMD_LE_SET_EVENT_MASK		0x2001
struct bt_hci_cmd_le_set_event_mask {
	uint8_t  mask[8];
} __attribute__ ((packed));

#define BT_HCI_CMD_LE_READ_BUFFER_SIZE		0x2002
struct bt_hci_rsp_le_read_buffer_size {
	uint8_t  status;
        uint16_t le_mtu;
        uint8_t  le_max_pkt;
} __attribute__ ((packed));

#define BT_HCI_CMD_LE_READ_LOCAL_FEATURES	0x2003
struct bt_hci_rsp_le_read_local_features {
	uint8_t  status;
	uint8_t  features[8];
} __attribute__ ((packed));

#define BT_HCI_CMD_LE_SET_RANDOM_ADDRESS	0x2005
struct bt_hci_cmd_le_set_random_address {
	uint8_t  addr[6];
} __attribute__ ((packed));

#define BT_HCI_CMD_LE_SET_ADV_PARAMETERS	0x2006
struct bt_hci_cmd_le_set_adv_parameters {
	uint16_t min_interval;
	uint16_t max_interval;
	uint8_t  type;
	uint8_t  own_addr_type;
	uint8_t  direct_addr_type;
	uint8_t  direct_addr[6];
	uint8_t  channel_map;
	uint8_t  filter_policy;
} __attribute__ ((packed));

#define BT_HCI_CMD_LE_READ_ADV_TX_POWER		0x2007
struct bt_hci_rsp_le_read_adv_tx_power {
	uint8_t  status;
	int8_t   level;
} __attribute__ ((packed));

#define BT_HCI_CMD_LE_SET_ADV_DATA		0x2008
struct bt_hci_cmd_le_set_adv_data {
	uint8_t  len;
	uint8_t  data[31];
} __attribute__ ((packed));

#define BT_HCI_CMD_LE_SET_SCAN_RSP_DATA		0x2009
struct bt_hci_cmd_le_set_scan_rsp_data {
	uint8_t  len;
	uint8_t  data[31];
} __attribute__ ((packed));

#define BT_HCI_CMD_LE_SET_ADV_ENABLE		0x200a
struct bt_hci_cmd_le_set_adv_enable {
	uint8_t  enable;
} __attribute__ ((packed));

#define BT_HCI_CMD_LE_SET_SCAN_PARAMETERS	0x200b
struct bt_hci_cmd_le_set_scan_parameters {
	uint8_t  type;
	uint16_t interval;
	uint16_t window;
	uint8_t  own_addr_type;
	uint8_t  filter_policy;
} __attribute__ ((packed));

#define BT_HCI_CMD_LE_SET_SCAN_ENABLE		0x200c
struct bt_hci_cmd_le_set_scan_enable {
	uint8_t  enable;
	uint8_t  filter_dup;
} __attribute__ ((packed));

#define BT_HCI_CMD_LE_CREATE_CONN		0x200d
struct bt_hci_cmd_le_create_conn {
	uint16_t scan_interval;
	uint16_t scan_window;
	uint8_t  filter_policy;
	uint8_t  peer_addr_type;
	uint8_t  peer_addr[6];
	uint8_t  own_addr_type;
	uint16_t min_interval;
	uint16_t max_interval;
	uint16_t latency;
	uint16_t supv_timeout;
	uint16_t min_length;
	uint16_t max_length;
} __attribute__ ((packed));

#define BT_HCI_CMD_LE_CREATE_CONN_CANCEL	0x200e

#define BT_HCI_CMD_LE_READ_WHITE_LIST_SIZE	0x200f
struct bt_hci_rsp_le_read_white_list_size {
	uint8_t  status;
	uint8_t  size;
} __attribute__ ((packed));

#define BT_HCI_CMD_LE_CLEAR_WHITE_LIST		0x2010

#define BT_HCI_CMD_LE_ADD_TO_WHITE_LIST		0x2011
struct bt_hci_cmd_le_add_to_white_list {
	uint8_t  addr_type;
	uint8_t  addr[6];
} __attribute__ ((packed));

#define BT_HCI_CMD_LE_REMOVE_FROM_WHITE_LIST	0x2012
struct bt_hci_cmd_le_remove_from_white_list {
	uint8_t  addr_type;
	uint8_t  addr[6];
} __attribute__ ((packed));

#define BT_HCI_CMD_LE_CONN_UPDATE		0x2013
struct bt_hci_cmd_le_conn_update {
	uint16_t handle;
	uint16_t min_interval;
	uint16_t max_interval;
	uint16_t latency;
	uint16_t supv_timeout;
	uint16_t min_length;
	uint16_t max_length;
} __attribute__ ((packed));

#define BT_HCI_CMD_LE_SET_HOST_CLASSIFICATION	0x2014
struct bt_hci_cmd_le_set_host_classification {
	uint8_t  map[5];
} __attribute__ ((packed));

#define BT_HCI_CMD_LE_READ_CHANNEL_MAP		0x2015
struct bt_hci_cmd_le_read_channel_map {
	uint16_t handle;
} __attribute__ ((packed));
struct bt_hci_rsp_le_read_channel_map {
	uint8_t  status;
	uint16_t handle;
	uint8_t  map[5];
} __attribute__ ((packed));

#define BT_HCI_CMD_LE_READ_REMOTE_FEATURES	0x2016
struct bt_hci_cmd_le_read_remote_features {
	uint16_t handle;
} __attribute__ ((packed));

#define BT_HCI_CMD_LE_ENCRYPT			0x2017
struct bt_hci_cmd_le_encrypt {
	uint8_t  key[16];
	uint8_t  plaintext[16];
} __attribute__ ((packed));
struct bt_hci_rsp_le_encrypt {
	uint8_t  status;
	uint8_t  data[16];
} __attribute__ ((packed));

#define BT_HCI_CMD_LE_RAND			0x2018
struct bt_hci_rsp_le_rand {
	uint8_t  status;
	uint64_t number;
} __attribute__ ((packed));

#define BT_HCI_CMD_LE_START_ENCRYPT		0x2019
struct bt_hci_cmd_le_start_encrypt {
	uint16_t handle;
	uint64_t rand;
	uint16_t ediv;
	uint8_t  ltk[16];
} __attribute__ ((packed));

#define BT_HCI_CMD_LE_LTK_REQ_REPLY		0x201a
struct bt_hci_cmd_le_ltk_req_reply {
	uint16_t handle;
	uint8_t  ltk[16];
} __attribute__ ((packed));
struct bt_hci_rsp_le_ltk_req_reply {
	uint8_t  status;
	uint16_t handle;
} __attribute__ ((packed));

#define BT_HCI_CMD_LE_LTK_REQ_NEG_REPLY		0x201b
struct bt_hci_cmd_le_ltk_req_neg_reply {
	uint16_t handle;
} __attribute__ ((packed));
struct bt_hci_rsp_le_ltk_req_neg_reply {
	uint8_t  status;
	uint16_t handle;
} __attribute__ ((packed));

#define BT_HCI_CMD_LE_READ_SUPPORTED_STATES	0x201c
struct bt_hci_rsp_le_read_supported_states {
	uint8_t  status;
	uint8_t  states[8];
} __attribute__ ((packed));

#define BT_HCI_CMD_LE_RECEIVER_TEST		0x201d
struct bt_hci_cmd_le_receiver_test {
	uint8_t  frequency;
} __attribute__ ((packed));

#define BT_HCI_CMD_LE_TRANSMITTER_TEST		0x201e
struct bt_hci_cmd_le_transmitter_test {
	uint8_t  frequency;
	uint8_t  data_len;
	uint8_t  payload;
} __attribute__ ((packed));

#define BT_HCI_CMD_LE_TEST_END			0x201f
struct bt_hci_rsp_le_test_end {
	uint8_t  status;
	uint16_t num_packets;
} __attribute__ ((packed));

#define BT_HCI_CMD_LE_CONN_PARAM_REQ_REPLY	0x2020
struct bt_hci_cmd_le_conn_param_req_reply {
	uint16_t handle;
	uint16_t min_interval;
	uint16_t max_interval;
	uint16_t latency;
	uint16_t supv_timeout;
	uint16_t min_length;
	uint16_t max_length;
} __attribute__ ((packed));
struct bt_hci_rsp_le_conn_param_req_reply {
	uint8_t  status;
	uint16_t handle;
} __attribute__ ((packed));

#define BT_HCI_CMD_LE_CONN_PARAM_REQ_NEG_REPLY	0x2021
struct bt_hci_cmd_le_conn_param_req_neg_reply {
	uint16_t handle;
	uint8_t  reason;
} __attribute__ ((packed));
struct bt_hci_rsp_le_conn_param_req_neg_reply {
	uint8_t  status;
	uint16_t handle;
} __attribute__ ((packed));

#define BT_HCI_CMD_LE_SET_DATA_LENGTH		0x2022
struct bt_hci_cmd_le_set_data_length {
	uint16_t handle;
	uint16_t tx_len;
	uint16_t tx_time;
} __attribute__ ((packed));
struct bt_hci_rsp_le_set_data_length {
	uint8_t  status;
	uint16_t handle;
} __attribute__ ((packed));

#define BT_HCI_CMD_LE_READ_DEFAULT_DATA_LENGTH	0x2023
struct bt_hci_rsp_le_read_default_data_length {
	uint8_t  status;
	uint16_t tx_len;
	uint16_t tx_time;
} __attribute__ ((packed));

#define BT_HCI_CMD_LE_WRITE_DEFAULT_DATA_LENGTH	0x2024
struct bt_hci_cmd_le_write_default_data_length {
	uint16_t tx_len;
	uint16_t tx_time;
} __attribute__ ((packed));

#define BT_HCI_CMD_LE_READ_LOCAL_PK256		0x2025

#define BT_HCI_CMD_LE_GENERATE_DHKEY		0x2026
struct bt_hci_cmd_le_generate_dhkey {
	uint8_t  remote_pk256[64];
} __attribute__ ((packed));

#define BT_HCI_CMD_LE_ADD_TO_RESOLV_LIST	0x2027
struct bt_hci_cmd_le_add_to_resolv_list {
	uint8_t  addr_type;
	uint8_t  addr[6];
	uint8_t  peer_irk[16];
	uint8_t  local_irk[16];
} __attribute__ ((packed));

#define BT_HCI_CMD_LE_REMOVE_FROM_RESOLV_LIST	0x2028
struct bt_hci_cmd_le_remove_from_resolv_list {
	uint8_t  addr_type;
	uint8_t  addr[6];
} __attribute__ ((packed));

#define BT_HCI_CMD_LE_CLEAR_RESOLV_LIST		0x2029

#define BT_HCI_CMD_LE_READ_RESOLV_LIST_SIZE	0x202a
struct bt_hci_rsp_le_read_resolv_list_size {
	uint8_t  status;
	uint8_t  size;
} __attribute__ ((packed));

#define BT_HCI_CMD_LE_READ_PEER_RESOLV_ADDR	0x202b
struct bt_hci_cmd_le_read_peer_resolv_addr {
	uint8_t  addr_type;
	uint8_t  addr[6];
} __attribute__ ((packed));
struct bt_hci_rsp_le_read_peer_resolv_addr {
	uint8_t  status;
	uint8_t  addr[6];
} __attribute__ ((packed));

#define BT_HCI_CMD_LE_READ_LOCAL_RESOLV_ADDR	0x202c
struct bt_hci_cmd_le_read_local_resolv_addr {
	uint8_t  addr_type;
	uint8_t  addr[6];
} __attribute__ ((packed));
struct bt_hci_rsp_le_read_local_resolv_addr {
	uint8_t  status;
	uint8_t  addr[6];
} __attribute__ ((packed));

#define BT_HCI_CMD_LE_SET_RESOLV_ENABLE		0x202d
struct bt_hci_cmd_le_set_resolv_enable {
	uint8_t  enable;
} __attribute__ ((packed));

#define BT_HCI_CMD_LE_SET_RESOLV_TIMEOUT	0x202e
struct bt_hci_cmd_le_set_resolv_timeout {
	uint16_t timeout;
} __attribute__ ((packed));

#define BT_HCI_CMD_LE_READ_MAX_DATA_LENGTH	0x202f
struct bt_hci_rsp_le_read_max_data_length {
	uint8_t  status;
	uint16_t max_tx_len;
	uint16_t max_tx_time;
	uint16_t max_rx_len;
	uint16_t max_rx_time;
} __attribute__ ((packed));

#define BT_HCI_EVT_INQUIRY_COMPLETE		0x01
struct bt_hci_evt_inquiry_complete {
	uint8_t  status;
} __attribute__ ((packed));

#define BT_HCI_EVT_INQUIRY_RESULT		0x02
struct bt_hci_evt_inquiry_result {
	uint8_t  num_resp;
	uint8_t  bdaddr[6];
	uint8_t  pscan_rep_mode;
	uint8_t  pscan_period_mode;
	uint8_t  pscan_mode;
	uint8_t  dev_class[3];
	uint16_t clock_offset;
} __attribute__ ((packed));

#define BT_HCI_EVT_CONN_COMPLETE		0x03
struct bt_hci_evt_conn_complete {
	uint8_t  status;
	uint16_t handle;
	uint8_t  bdaddr[6];
	uint8_t  link_type;
	uint8_t  encr_mode;
} __attribute__ ((packed));

#define BT_HCI_EVT_CONN_REQUEST			0x04
struct bt_hci_evt_conn_request {
	uint8_t  bdaddr[6];
	uint8_t  dev_class[3];
	uint8_t  link_type;
} __attribute__ ((packed));

#define BT_HCI_EVT_DISCONNECT_COMPLETE		0x05
struct bt_hci_evt_disconnect_complete {
	uint8_t  status;
	uint16_t handle;
	uint8_t  reason;
} __attribute__ ((packed));

#define BT_HCI_EVT_AUTH_COMPLETE		0x06
struct bt_hci_evt_auth_complete {
	uint8_t  status;
	uint16_t handle;
} __attribute__ ((packed));

#define BT_HCI_EVT_REMOTE_NAME_REQUEST_COMPLETE	0x07
struct bt_hci_evt_remote_name_request_complete {
	uint8_t  status;
	uint8_t  bdaddr[6];
	uint8_t  name[248];
} __attribute__ ((packed));

#define BT_HCI_EVT_ENCRYPT_CHANGE		0x08
struct bt_hci_evt_encrypt_change {
	uint8_t  status;
	uint16_t handle;
	uint8_t  encr_mode;
} __attribute__ ((packed));

#define BT_HCI_EVT_CHANGE_CONN_LINK_KEY_COMPLETE 0x09
struct bt_hci_evt_change_conn_link_key_complete {
	uint8_t  status;
	uint16_t handle;
} __attribute__ ((packed));

#define BT_HCI_EVT_MASTER_LINK_KEY_COMPLETE	0x0a
struct bt_hci_evt_master_link_key_complete {
	uint8_t  status;
	uint16_t handle;
	uint8_t  key_flag;
} __attribute__ ((packed));

#define BT_HCI_EVT_REMOTE_FEATURES_COMPLETE	0x0b
struct bt_hci_evt_remote_features_complete {
	uint8_t  status;
	uint16_t handle;
	uint8_t  features[8];
} __attribute__ ((packed));

#define BT_HCI_EVT_REMOTE_VERSION_COMPLETE	0x0c
struct bt_hci_evt_remote_version_complete {
	uint8_t  status;
	uint16_t handle;
	uint8_t  lmp_ver;
	uint16_t manufacturer;
	uint16_t lmp_subver;
} __attribute__ ((packed));

#define BT_HCI_EVT_QOS_SETUP_COMPLETE		0x0d
struct bt_hci_evt_qos_setup_complete {
	uint8_t  status;
	uint16_t handle;
	uint8_t  flags;
	uint8_t  service_type;
	uint32_t token_rate;
	uint32_t peak_bandwidth;
	uint32_t latency;
	uint32_t delay_variation;
} __attribute__ ((packed));

#define BT_HCI_EVT_CMD_COMPLETE			0x0e
struct bt_hci_evt_cmd_complete {
	uint8_t  ncmd;
	uint16_t opcode;
} __attribute__ ((packed));

#define BT_HCI_EVT_CMD_STATUS			0x0f
struct bt_hci_evt_cmd_status {
	uint8_t  status;
	uint8_t  ncmd;
	uint16_t opcode;
} __attribute__ ((packed));

#define BT_HCI_EVT_HARDWARE_ERROR		0x10
struct bt_hci_evt_hardware_error {
	uint8_t  code;
} __attribute__ ((packed));

#define BT_HCI_EVT_FLUSH_OCCURRED		0x11
struct bt_hci_evt_flush_occurred {
	uint16_t handle;
} __attribute__ ((packed));

#define BT_HCI_EVT_ROLE_CHANGE			0x12
struct bt_hci_evt_role_change {
	uint8_t  status;
	uint8_t  bdaddr[6];
	uint8_t  role;
} __attribute__ ((packed));

#define BT_HCI_EVT_NUM_COMPLETED_PACKETS	0x13
struct bt_hci_evt_num_completed_packets {
	uint8_t  num_handles;
	uint16_t handle;
	uint16_t count;
} __attribute__ ((packed));

#define BT_HCI_EVT_MODE_CHANGE			0x14
struct bt_hci_evt_mode_change {
	uint8_t  status;
	uint16_t handle;
	uint8_t  mode;
	uint16_t interval;
} __attribute__ ((packed));

#define BT_HCI_EVT_RETURN_LINK_KEYS		0x15
struct bt_hci_evt_return_link_keys {
	uint8_t  num_keys;
	uint8_t  keys[0];
} __attribute__ ((packed));

#define BT_HCI_EVT_PIN_CODE_REQUEST		0x16
struct bt_hci_evt_pin_code_request {
	uint8_t  bdaddr[6];
} __attribute__ ((packed));

#define BT_HCI_EVT_LINK_KEY_REQUEST		0x17
struct bt_hci_evt_link_key_request {
	uint8_t  bdaddr[6];
} __attribute__ ((packed));

#define BT_HCI_EVT_LINK_KEY_NOTIFY		0x18
struct bt_hci_evt_link_key_notify {
	uint8_t  bdaddr[6];
	uint8_t  link_key[16];
	uint8_t  key_type;
} __attribute__ ((packed));

#define BT_HCI_EVT_LOOPBACK_COMMAND		0x19

#define BT_HCI_EVT_DATA_BUFFER_OVERFLOW		0x1a
struct bt_hci_evt_data_buffer_overflow {
	uint8_t  link_type;
} __attribute__ ((packed));

#define BT_HCI_EVT_MAX_SLOTS_CHANGE		0x1b
struct bt_hci_evt_max_slots_change {
	uint16_t handle;
	uint8_t  max_slots;
} __attribute__ ((packed));

#define BT_HCI_EVT_CLOCK_OFFSET_COMPLETE	0x1c
struct bt_hci_evt_clock_offset_complete {
	uint8_t  status;
	uint16_t handle;
	uint16_t clock_offset;
} __attribute__ ((packed));

#define BT_HCI_EVT_CONN_PKT_TYPE_CHANGED	0x1d
struct bt_hci_evt_conn_pkt_type_changed {
	uint8_t  status;
	uint16_t handle;
	uint16_t pkt_type;
} __attribute__ ((packed));

#define BT_HCI_EVT_QOS_VIOLATION		0x1e
struct bt_hci_evt_qos_violation {
	uint16_t handle;
} __attribute__ ((packed));

#define BT_HCI_EVT_PSCAN_MODE_CHANGE		0x1f
struct bt_hci_evt_pscan_mode_change {
	uint8_t  bdaddr[6];
	uint8_t  pscan_mode;
} __attribute__ ((packed));

#define BT_HCI_EVT_PSCAN_REP_MODE_CHANGE	0x20
struct bt_hci_evt_pscan_rep_mode_change {
	uint8_t  bdaddr[6];
	uint8_t  pscan_rep_mode;
} __attribute__ ((packed));

#define BT_HCI_EVT_FLOW_SPEC_COMPLETE		0x21
struct bt_hci_evt_flow_spec_complete {
	uint8_t  status;
	uint16_t handle;
	uint8_t  flags;
	uint8_t  direction;
	uint8_t  service_type;
	uint32_t token_rate;
	uint32_t token_bucket_size;
	uint32_t peak_bandwidth;
	uint32_t access_latency;
} __attribute__ ((packed));

#define BT_HCI_EVT_INQUIRY_RESULT_WITH_RSSI	0x22
struct bt_hci_evt_inquiry_result_with_rssi {
	uint8_t  num_resp;
	uint8_t  bdaddr[6];
	uint8_t  pscan_rep_mode;
	uint8_t  pscan_period_mode;
	uint8_t  dev_class[3];
	uint16_t clock_offset;
	int8_t   rssi;
} __attribute__ ((packed));

#define BT_HCI_EVT_REMOTE_EXT_FEATURES_COMPLETE	0x23
struct bt_hci_evt_remote_ext_features_complete {
	uint8_t  status;
	uint16_t handle;
	uint8_t  page;
	uint8_t  max_page;
	uint8_t  features[8];
} __attribute__ ((packed));

#define BT_HCI_EVT_SYNC_CONN_COMPLETE		0x2c
struct bt_hci_evt_sync_conn_complete {
	uint8_t  status;
	uint16_t handle;
	uint8_t  bdaddr[6];
	uint8_t  link_type;
	uint8_t  tx_interval;
	uint8_t  retrans_window;
	uint16_t rx_pkt_len;
	uint16_t tx_pkt_len;
	uint8_t  air_mode;
} __attribute__ ((packed));

#define BT_HCI_EVT_SYNC_CONN_CHANGED		0x2d
struct bt_hci_evt_sync_conn_changed {
	uint8_t  status;
	uint16_t handle;
	uint8_t  tx_interval;
	uint8_t  retrans_window;
	uint16_t rx_pkt_len;
	uint16_t tx_pkt_len;
} __attribute__ ((packed));

#define BT_HCI_EVT_SNIFF_SUBRATING		0x2e
struct bt_hci_evt_sniff_subrating {
	uint8_t  status;
	uint16_t handle;
	uint16_t max_tx_latency;
	uint16_t max_rx_latency;
	uint16_t min_remote_timeout;
	uint16_t min_local_timeout;
} __attribute__ ((packed));

#define BT_HCI_EVT_EXT_INQUIRY_RESULT		0x2f
struct bt_hci_evt_ext_inquiry_result {
	uint8_t  num_resp;
	uint8_t  bdaddr[6];
	uint8_t  pscan_rep_mode;
	uint8_t  pscan_period_mode;
	uint8_t  dev_class[3];
	uint16_t clock_offset;
	int8_t   rssi;
	uint8_t  data[240];
} __attribute__ ((packed));

#define BT_HCI_EVT_ENCRYPT_KEY_REFRESH_COMPLETE	0x30
struct bt_hci_evt_encrypt_key_refresh_complete {
	uint8_t  status;
	uint16_t handle;
} __attribute__ ((packed));

#define BT_HCI_EVT_IO_CAPABILITY_REQUEST	0x31
struct bt_hci_evt_io_capability_request {
	uint8_t  bdaddr[6];
} __attribute__ ((packed));

#define BT_HCI_EVT_IO_CAPABILITY_RESPONSE	0x32
struct bt_hci_evt_io_capability_response {
	uint8_t  bdaddr[6];
	uint8_t  capability;
	uint8_t  oob_data;
	uint8_t  authentication;
} __attribute__ ((packed));

#define BT_HCI_EVT_USER_CONFIRM_REQUEST		0x33
struct bt_hci_evt_user_confirm_request {
	uint8_t  bdaddr[6];
	uint32_t passkey;
} __attribute__ ((packed));

#define BT_HCI_EVT_USER_PASSKEY_REQUEST		0x34
struct bt_hci_evt_user_passkey_request {
	uint8_t  bdaddr[6];
} __attribute__ ((packed));

#define BT_HCI_EVT_REMOTE_OOB_DATA_REQUEST	0x35
struct bt_hci_evt_remote_oob_data_request {
	uint8_t  bdaddr[6];
} __attribute__ ((packed));

#define BT_HCI_EVT_SIMPLE_PAIRING_COMPLETE	0x36
struct bt_hci_evt_simple_pairing_complete {
	uint8_t  status;
	uint8_t  bdaddr[6];
} __attribute__ ((packed));

#define BT_HCI_EVT_LINK_SUPV_TIMEOUT_CHANGED	0x38
struct bt_hci_evt_link_supv_timeout_changed {
	uint16_t handle;
	uint16_t timeout;
} __attribute__ ((packed));

#define BT_HCI_EVT_ENHANCED_FLUSH_COMPLETE	0x39
struct bt_hci_evt_enhanced_flush_complete {
	uint16_t handle;
} __attribute__ ((packed));

#define BT_HCI_EVT_USER_PASSKEY_NOTIFY		0x3b
struct bt_hci_evt_user_passkey_notify {
	uint8_t  bdaddr[6];
	uint32_t passkey;
} __attribute__ ((packed));

#define BT_HCI_EVT_KEYPRESS_NOTIFY		0x3c
struct bt_hci_evt_keypress_notify {
	uint8_t  bdaddr[6];
	uint8_t  type;
} __attribute__ ((packed));

#define BT_HCI_EVT_REMOTE_HOST_FEATURES_NOTIFY	0x3d
struct bt_hci_evt_remote_host_features_notify {
	uint8_t  bdaddr[6];
	uint8_t  features[8];
} __attribute__ ((packed));

#define BT_HCI_EVT_LE_META_EVENT		0x3e

#define BT_HCI_EVT_PHY_LINK_COMPLETE		0x40
struct bt_hci_evt_phy_link_complete {
	uint8_t  status;
	uint8_t  phy_handle;
} __attribute__ ((packed));

#define BT_HCI_EVT_CHANNEL_SELECTED		0x41
struct bt_hci_evt_channel_selected {
	uint8_t  phy_handle;
} __attribute__ ((packed));

#define BT_HCI_EVT_DISCONN_PHY_LINK_COMPLETE	0x42
struct bt_hci_evt_disconn_phy_link_complete {
	uint8_t  status;
	uint8_t  phy_handle;
	uint8_t  reason;
} __attribute__ ((packed));

#define BT_HCI_EVT_PHY_LINK_LOSS_EARLY_WARNING	0x43
struct bt_hci_evt_phy_link_loss_early_warning {
	uint8_t  phy_handle;
	uint8_t  reason;
} __attribute__ ((packed));

#define BT_HCI_EVT_PHY_LINK_RECOVERY		0x44
struct bt_hci_evt_phy_link_recovery {
	uint8_t  phy_handle;
} __attribute__ ((packed));

#define BT_HCI_EVT_LOGIC_LINK_COMPLETE		0x45
struct bt_hci_evt_logic_link_complete {
	uint8_t  status;
	uint16_t handle;
	uint8_t  phy_handle;
	uint8_t  flow_spec;
} __attribute__ ((packed));

#define BT_HCI_EVT_DISCONN_LOGIC_LINK_COMPLETE	0x46
struct bt_hci_evt_disconn_logic_link_complete {
	uint8_t  status;
	uint16_t handle;
	uint8_t  reason;
} __attribute__ ((packed));

#define BT_HCI_EVT_FLOW_SPEC_MODIFY_COMPLETE	0x47
struct bt_hci_evt_flow_spec_modify_complete {
	uint8_t  status;
	uint16_t handle;
} __attribute__ ((packed));

#define BT_HCI_EVT_NUM_COMPLETED_DATA_BLOCKS	0x48
struct bt_hci_evt_num_completed_data_blocks {
	uint16_t total_num_blocks;
	uint8_t  num_handles;
	uint16_t handle;
	uint16_t num_packets;
	uint16_t num_blocks;
} __attribute__ ((packed));

#define BT_HCI_EVT_SHORT_RANGE_MODE_CHANGE	0x4c
struct bt_hci_evt_short_range_mode_change {
	uint8_t  status;
	uint8_t  phy_handle;
	uint8_t  mode;
} __attribute__ ((packed));

#define BT_HCI_EVT_AMP_STATUS_CHANGE		0x4d
struct bt_hci_evt_amp_status_change {
	uint8_t  status;
	uint8_t  amp_status;
} __attribute__ ((packed));

#define BT_HCI_EVT_TRIGGERED_CLOCK_CAPTURE	0x4e
struct bt_hci_evt_triggered_clock_capture {
	uint16_t handle;
	uint8_t  type;
	uint32_t clock;
	uint16_t clock_offset;
} __attribute__ ((packed));

#define BT_HCI_EVT_SYNC_TRAIN_COMPLETE		0x4f
struct bt_hci_evt_sync_train_complete {
	uint8_t  status;
} __attribute__ ((packed));

#define BT_HCI_EVT_SYNC_TRAIN_RECEIVED		0x50
struct bt_hci_evt_sync_train_received {
	uint8_t  status;
	uint8_t  bdaddr[6];
	uint32_t offset;
	uint8_t  map[10];
	uint8_t  lt_addr;
	uint32_t instant;
	uint16_t interval;
	uint8_t  service_data;
} __attribute__ ((packed));

#define BT_HCI_EVT_SLAVE_BROADCAST_RECEIVE	0x51
struct bt_hci_evt_slave_broadcast_receive {
	uint8_t  bdaddr[6];
	uint8_t  lt_addr;
	uint32_t clock;
	uint32_t offset;
	uint8_t  status;
	uint8_t  fragment;
	uint8_t  length;
} __attribute__ ((packed));

#define BT_HCI_EVT_SLAVE_BROADCAST_TIMEOUT	0x52
struct bt_hci_evt_slave_broadcast_timeout {
	uint8_t  bdaddr[6];
	uint8_t  lt_addr;
} __attribute__ ((packed));

#define BT_HCI_EVT_TRUNCATED_PAGE_COMPLETE	0x53
struct bt_hci_evt_truncated_page_complete {
	uint8_t  status;
	uint8_t  bdaddr[6];
} __attribute__ ((packed));

#define BT_HCI_EVT_SLAVE_PAGE_RESPONSE_TIMEOUT	0x54

#define BT_HCI_EVT_SLAVE_BROADCAST_CHANNEL_MAP_CHANGE	0x55
struct bt_hci_evt_slave_broadcast_channel_map_change {
	uint8_t  map[10];
} __attribute__ ((packed));

#define BT_HCI_EVT_INQUIRY_RESPONSE_NOTIFY	0x56
struct bt_hci_evt_inquiry_response_notify {
	uint8_t  lap[3];
	int8_t   rssi;
} __attribute__ ((packed));

#define BT_HCI_EVT_AUTH_PAYLOAD_TIMEOUT_EXPIRED	0x57
struct bt_hci_evt_auth_payload_timeout_expired {
	uint16_t handle;
} __attribute__ ((packed));

#define BT_HCI_EVT_LE_CONN_COMPLETE		0x01
struct bt_hci_evt_le_conn_complete {
	uint8_t  status;
	uint16_t handle;
	uint8_t  role;
	uint8_t  peer_addr_type;
	uint8_t  peer_addr[6];
	uint16_t interval;
	uint16_t latency;
	uint16_t supv_timeout;
	uint8_t  clock_accuracy;
} __attribute__ ((packed));

#define BT_HCI_EVT_LE_ADV_REPORT		0x02
struct bt_hci_evt_le_adv_report {
	uint8_t  num_reports;
	uint8_t  event_type;
	uint8_t  addr_type;
	uint8_t  addr[6];
	uint8_t  data_len;
	uint8_t  data[0];
} __attribute__ ((packed));

#define BT_HCI_EVT_LE_CONN_UPDATE_COMPLETE	0x03
struct bt_hci_evt_le_conn_update_complete {
	uint8_t  status;
	uint16_t handle;
	uint16_t interval;
	uint16_t latency;
	uint16_t supv_timeout;
} __attribute__ ((packed));

#define BT_HCI_EVT_LE_REMOTE_FEATURES_COMPLETE	0x04
struct bt_hci_evt_le_remote_features_complete {
	uint8_t  status;
	uint16_t handle;
	uint8_t  features[8];
} __attribute__ ((packed));

#define BT_HCI_EVT_LE_LONG_TERM_KEY_REQUEST	0x05
struct bt_hci_evt_le_long_term_key_request {
	uint16_t handle;
	uint64_t rand;
	uint16_t ediv;
} __attribute__ ((packed));

#define BT_HCI_EVT_LE_CONN_PARAM_REQUEST	0x06
struct bt_hci_evt_le_conn_param_request {
	uint16_t handle;
	uint16_t min_interval;
	uint16_t max_interval;
	uint16_t latency;
	uint16_t supv_timeout;
} __attribute__ ((packed));

#define BT_HCI_EVT_LE_DATA_LENGTH_CHANGE	0x07
struct bt_hci_evt_le_data_length_change {
	uint16_t handle;
	uint16_t max_tx_len;
	uint16_t max_tx_time;
	uint16_t max_rx_len;
	uint16_t max_rx_time;
} __attribute__ ((packed));

#define BT_HCI_EVT_LE_READ_LOCAL_PK256_COMPLETE	0x08
struct bt_hci_evt_le_read_local_pk256_complete {
	uint8_t  status;
	uint8_t  local_pk256[64];
} __attribute__ ((packed));

#define BT_HCI_EVT_LE_GENERATE_DHKEY_COMPLETE	0x09
struct bt_hci_evt_le_generate_dhkey_complete {
	uint8_t  status;
	uint8_t  dhkey[32];
} __attribute__ ((packed));

#define BT_HCI_EVT_LE_ENHANCED_CONN_COMPLETE	0x0a
struct bt_hci_evt_le_enhanced_conn_complete {
	uint8_t  status;
	uint16_t handle;
	uint8_t  role;
	uint8_t  peer_addr_type;
	uint8_t  peer_addr[6];
	uint8_t  local_rpa[6];
	uint8_t  peer_rpa[6];
	uint16_t interval;
	uint16_t latency;
	uint16_t supv_timeout;
	uint8_t  clock_accuracy;
} __attribute__ ((packed));

#define BT_HCI_EVT_LE_DIRECT_ADV_REPORT		0x0b
struct bt_hci_evt_le_direct_adv_report {
	uint8_t  num_reports;
	uint8_t  event_type;
	uint8_t  addr_type;
	uint8_t  addr[6];
	uint8_t  direct_addr_type;
	uint8_t  direct_addr[6];
	int8_t   rssi;
} __attribute__ ((packed));

#define BT_HCI_ERR_SUCCESS			0x00
#define BT_HCI_ERR_UNKNOWN_COMMAND		0x01
#define BT_HCI_ERR_UNKNOWN_CONN_ID		0x02
#define BT_HCI_ERR_HARDWARE_FAILURE		0x03
#define BT_HCI_ERR_PAGE_TIMEOUT			0x04
#define BT_HCI_ERR_AUTH_FAILURE			0x05
#define BT_HCI_ERR_PIN_OR_KEY_MISSING		0x06
#define BT_HCI_ERR_MEM_CAPACITY_EXCEEDED	0x07
#define BT_HCI_ERR_COMMAND_DISALLOWED		0x0c
#define BT_HCI_ERR_UNSUPPORTED_FEATURE		0x11
#define BT_HCI_ERR_INVALID_PARAMETERS		0x12
#define BT_HCI_ERR_UNSPECIFIED_ERROR		0x1f
#define BT_HCI_ERR_CONN_FAILED_TO_ESTABLISH	0x3e

struct bt_l2cap_hdr {
	uint16_t len;
	uint16_t cid;
} __attribute__ ((packed));

struct bt_l2cap_hdr_sig {
	uint8_t  code;
	uint8_t  ident;
	uint16_t len;
} __attribute__ ((packed));

#define BT_L2CAP_PDU_CMD_REJECT		0x01
struct bt_l2cap_pdu_cmd_reject {
	uint16_t reason;
} __attribute__ ((packed));

#define BT_L2CAP_PDU_CONN_REQ		0x02
struct bt_l2cap_pdu_conn_req {
	uint16_t psm;
	uint16_t scid;
} __attribute__ ((packed));

#define BT_L2CAP_PDU_CONN_RSP		0x03
struct bt_l2cap_pdu_conn_rsp {
	uint16_t dcid;
	uint16_t scid;
	uint16_t result;
	uint16_t status;
} __attribute__ ((packed));

#define BT_L2CAP_PDU_CONFIG_REQ		0x04
struct bt_l2cap_pdu_config_req {
	uint16_t dcid;
	uint16_t flags;
} __attribute__ ((packed));

#define BT_L2CAP_PDU_CONFIG_RSP		0x05
struct bt_l2cap_pdu_config_rsp {
	uint16_t scid;
	uint16_t flags;
	uint16_t result;
} __attribute__ ((packed));

#define BT_L2CAP_PDU_DISCONN_REQ	0x06
struct bt_l2cap_pdu_disconn_req {
	uint16_t dcid;
	uint16_t scid;
} __attribute__ ((packed));

#define BT_L2CAP_PDU_DISCONN_RSP	0x07
struct bt_l2cap_pdu_disconn_rsp {
	uint16_t dcid;
	uint16_t scid;
} __attribute__ ((packed));

#define BT_L2CAP_PDU_ECHO_REQ		0x08

#define BT_L2CAP_PDU_ECHO_RSP		0x09

#define BT_L2CAP_PDU_INFO_REQ		0x0a
struct bt_l2cap_pdu_info_req {
	uint16_t type;
} __attribute__ ((packed));

#define BT_L2CAP_PDU_INFO_RSP		0x0b
struct bt_l2cap_pdu_info_rsp {
	uint16_t type;
	uint16_t result;
	uint8_t  data[0];
} __attribute__ ((packed));

#define BT_L2CAP_PDU_CREATE_CHAN_REQ	0x0c
struct bt_l2cap_pdu_create_chan_req {
	uint16_t psm;
	uint16_t scid;
	uint8_t  ctrlid;
} __attribute__ ((packed));

#define BT_L2CAP_PDU_CREATE_CHAN_RSP	0x0d
struct bt_l2cap_pdu_create_chan_rsp {
	uint16_t dcid;
	uint16_t scid;
	uint16_t result;
	uint16_t status;
} __attribute__ ((packed));

#define BT_L2CAP_PDU_MOVE_CHAN_REQ	0x0e
struct bt_l2cap_pdu_move_chan_req {
	uint16_t icid;
	uint8_t  ctrlid;
} __attribute__ ((packed));

#define BT_L2CAP_PDU_MOVE_CHAN_RSP	0x0f
struct bt_l2cap_pdu_move_chan_rsp {
	uint16_t icid;
	uint16_t result;
} __attribute__ ((packed));

#define BT_L2CAP_PDU_MOVE_CHAN_CFM	0x10
struct bt_l2cap_pdu_move_chan_cfm {
	uint16_t icid;
	uint16_t result;
} __attribute__ ((packed));

#define BT_L2CAP_PDU_MOVE_CHAN_CFM_RSP	0x11
struct bt_l2cap_pdu_move_chan_cfm_rsp {
	uint16_t icid;
} __attribute__ ((packed));

#define BT_L2CAP_PDU_CONN_PARAM_REQ	0x12
struct bt_l2cap_pdu_conn_param_req {
	uint16_t min_interval;
	uint16_t max_interval;
	uint16_t latency;
	uint16_t timeout;
} __attribute__ ((packed));

#define BT_L2CAP_PDU_CONN_PARAM_RSP	0x13
struct bt_l2cap_pdu_conn_param_rsp {
	uint16_t result;
} __attribute__ ((packed));

#define BT_L2CAP_PDU_LE_CONN_REQ	0x14
struct bt_l2cap_pdu_le_conn_req {
	uint16_t psm;
	uint16_t scid;
	uint16_t mtu;
	uint16_t mps;
	uint16_t credits;
} __attribute__ ((packed));

#define BT_L2CAP_PDU_LE_CONN_RSP	0x15
struct bt_l2cap_pdu_le_conn_rsp {
	uint16_t dcid;
	uint16_t mtu;
	uint16_t mps;
	uint16_t credits;
	uint16_t result;
} __attribute__ ((packed));

#define BT_L2CAP_PDU_LE_FLOWCTL_CREDS	0x16
struct bt_l2cap_pdu_le_flowctl_creds {
	uint16_t cid;
	uint16_t credits;
} __attribute__ ((packed));

struct bt_l2cap_hdr_connless {
	uint16_t psm;
} __attribute__ ((packed));

struct bt_l2cap_hdr_amp {
	uint8_t  code;
	uint8_t  ident;
	uint16_t len;
} __attribute__ ((packed));

#define BT_L2CAP_AMP_CMD_REJECT		0x01
struct bt_l2cap_amp_cmd_reject {
	uint16_t reason;
} __attribute__ ((packed));

#define BT_L2CAP_AMP_DISCOVER_REQ	0x02
struct bt_l2cap_amp_discover_req {
	uint16_t size;
	uint16_t features;
} __attribute__ ((packed));

#define BT_L2CAP_AMP_DISCOVER_RSP	0x03
struct bt_l2cap_amp_discover_rsp {
	uint16_t size;
	uint16_t features;
} __attribute__ ((packed));

#define BT_L2CAP_AMP_CHANGE_NOTIFY	0x04

#define BT_L2CAP_AMP_CHANGE_RESPONSE	0x05

#define BT_L2CAP_AMP_GET_INFO_REQ	0x06
struct bt_l2cap_amp_get_info_req {
	uint8_t  ctrlid;
} __attribute__ ((packed));

#define BT_L2CAP_AMP_GET_INFO_RSP	0x07
struct bt_l2cap_amp_get_info_rsp {
	uint8_t  ctrlid;
	uint8_t  status;
	uint32_t total_bw;
	uint32_t max_bw;
	uint32_t min_latency;
	uint16_t pal_cap;
	uint16_t max_assoc_len;
} __attribute__ ((packed));

#define BT_L2CAP_AMP_GET_ASSOC_REQ	0x08
struct bt_l2cap_amp_get_assoc_req {
	uint8_t  ctrlid;
} __attribute__ ((packed));

#define BT_L2CAP_AMP_GET_ASSOC_RSP	0x09
struct bt_l2cap_amp_get_assoc_rsp {
	uint8_t  ctrlid;
	uint8_t  status;
} __attribute__ ((packed));

#define BT_L2CAP_AMP_CREATE_PHY_LINK_REQ	0x0a
struct bt_l2cap_amp_create_phy_link_req {
	uint8_t  local_ctrlid;
	uint8_t  remote_ctrlid;
} __attribute__ ((packed));

#define BT_L2CAP_AMP_CREATE_PHY_LINK_RSP	0x0b
struct bt_l2cap_amp_create_phy_link_rsp {
	uint8_t  local_ctrlid;
	uint8_t  remote_ctrlid;
	uint8_t  status;
} __attribute__ ((packed));

#define BT_L2CAP_AMP_DISCONN_PHY_LINK_REQ	0x0c
struct bt_l2cap_amp_disconn_phy_link_req {
	uint8_t  local_ctrlid;
	uint8_t  remote_ctrlid;
} __attribute__ ((packed));

#define BT_L2CAP_AMP_DISCONN_PHY_LINK_RSP	0x0d
struct bt_l2cap_amp_disconn_phy_link_rsp {
	uint8_t  local_ctrlid;
	uint8_t  remote_ctrlid;
	uint8_t  status;
} __attribute__ ((packed));

struct bt_l2cap_hdr_att {
	uint8_t  code;
} __attribute__ ((packed));

#define BT_L2CAP_ATT_ERROR_RESPONSE		0x01
struct bt_l2cap_att_error_response {
	uint8_t  request;
	uint16_t handle;
	uint8_t  error;
} __attribute__ ((packed));

#define BT_L2CAP_ATT_EXCHANGE_MTU_REQ		0x02
struct bt_l2cap_att_exchange_mtu_req {
	uint16_t mtu;
} __attribute__ ((packed));

#define BT_L2CAP_ATT_EXCHANGE_MTU_RSP		0x03
struct bt_l2cap_att_exchange_mtu_rsp {
	uint16_t mtu;
} __attribute__ ((packed));

#define BT_L2CAP_ATT_READ_TYPE_REQ		0x08
struct bt_l2cap_att_read_type_req {
	uint16_t start_handle;
	uint16_t end_handle;
} __attribute__ ((packed));

#define BT_L2CAP_ATT_READ_TYPE_RSP		0x09
struct bt_l2cap_att_read_type_rsp {
	uint8_t  length;
} __attribute__ ((packed));

#define BT_L2CAP_ATT_READ_REQ			0x0a
struct bt_l2cap_att_read_req {
	uint16_t handle;
} __attribute__ ((packed));

#define BT_L2CAP_ATT_READ_RSP			0x0b

#define BT_L2CAP_ATT_READ_GROUP_TYPE_REQ	0x10
struct bt_l2cap_att_read_group_type_req {
	uint16_t start_handle;
	uint16_t end_handle;
} __attribute__ ((packed));

#define BT_L2CAP_ATT_READ_GROUP_TYPE_RSP	0x11
struct bt_l2cap_att_read_group_type_rsp {
	uint8_t  length;
} __attribute__ ((packed));

#define BT_L2CAP_ATT_HANDLE_VALUE_NOTIFY	0x1b
struct bt_l2cap_att_handle_value_notify {
	uint16_t handle;
} __attribute__ ((packed));

#define BT_L2CAP_ATT_HANDLE_VALUE_IND		0x1d
struct bt_l2cap_att_handle_value_ind {
	uint16_t handle;
} __attribute__ ((packed));

#define BT_L2CAP_ATT_HANDLE_VALUE_CONF		0x1e

struct bt_l2cap_hdr_smp {
	uint8_t  code;
} __attribute__ ((packed));

#define BT_L2CAP_SMP_PAIRING_REQUEST	0x01
struct bt_l2cap_smp_pairing_request {
	uint8_t  io_capa;
	uint8_t  oob_data;
	uint8_t  auth_req;
	uint8_t  max_key_size;
	uint8_t  init_key_dist;
	uint8_t  resp_key_dist;
} __attribute__ ((packed));

#define BT_L2CAP_SMP_PAIRING_RESPONSE	0x02
struct bt_l2cap_smp_pairing_response {
	uint8_t  io_capa;
	uint8_t  oob_data;
	uint8_t  auth_req;
	uint8_t  max_key_size;
	uint8_t  init_key_dist;
	uint8_t  resp_key_dist;
} __attribute__ ((packed));

#define BT_L2CAP_SMP_PAIRING_CONFIRM	0x03
struct bt_l2cap_smp_pairing_confirm {
	uint8_t  value[16];
} __attribute__ ((packed));

#define BT_L2CAP_SMP_PAIRING_RANDOM	0x04
struct bt_l2cap_smp_pairing_random {
	uint8_t  value[16];
} __attribute__ ((packed));

#define BT_L2CAP_SMP_PAIRING_FAILED	0x05
struct bt_l2cap_smp_pairing_failed {
	uint8_t  reason;
} __attribute__ ((packed));

#define BT_L2CAP_SMP_ENCRYPT_INFO	0x06
struct bt_l2cap_smp_encrypt_info {
	uint8_t  ltk[16];
} __attribute__ ((packed));

#define BT_L2CAP_SMP_MASTER_IDENT	0x07
struct bt_l2cap_smp_master_ident {
	uint16_t ediv;
	uint64_t rand;
} __attribute__ ((packed));

#define BT_L2CAP_SMP_IDENT_INFO		0x08
struct bt_l2cap_smp_ident_info {
	uint8_t  irk[16];
} __attribute__ ((packed));

#define BT_L2CAP_SMP_IDENT_ADDR_INFO	0x09
struct bt_l2cap_smp_ident_addr_info {
	uint8_t  addr_type;
	uint8_t  addr[6];
} __attribute__ ((packed));

#define BT_L2CAP_SMP_SIGNING_INFO	0x0a
struct bt_l2cap_smp_signing_info {
	uint8_t  csrk[16];
} __attribute__ ((packed));

#define BT_L2CAP_SMP_SECURITY_REQUEST	0x0b
struct bt_l2cap_smp_security_request {
	uint8_t  auth_req;
} __attribute__ ((packed));

#define BT_L2CAP_SMP_PUBLIC_KEY		0x0c
struct bt_l2cap_smp_public_key {
	uint8_t  x[32];
	uint8_t  y[32];
} __attribute__ ((packed));

#define BT_L2CAP_SMP_DHKEY_CHECK	0x0d
struct bt_l2cap_smp_dhkey_check {
	uint8_t  e[16];
} __attribute__ ((packed));

#define BT_L2CAP_SMP_KEYPRESS_NOTIFY	0x0e
struct bt_l2cap_smp_keypress_notify {
	uint8_t  type;
} __attribute__ ((packed));

struct bt_sdp_hdr {
	uint8_t  pdu;
	uint16_t tid;
	uint16_t plen;
} __attribute__ ((packed));

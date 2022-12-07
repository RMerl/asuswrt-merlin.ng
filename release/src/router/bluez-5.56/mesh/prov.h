/* SPDX-License-Identifier: LGPL-2.1-or-later */
/*
 *
 *  BlueZ - Bluetooth protocol stack for Linux
 *
 *  Copyright (C) 2018  Intel Corporation. All rights reserved.
 *
 *
 */

#ifndef __packed
#define __packed __attribute__((packed))
#endif

struct mesh_net;
struct mesh_dev;

enum mesh_trans {
	MESH_TRANS_IDLE,
	MESH_TRANS_TX,
	MESH_TRANS_RX,
};

enum mesh_bearer {
	MESH_BEARER_IDLE,
	MESH_BEARER_ADV,
};

enum mesh_prov_mode {
	MESH_PROV_MODE_NONE,
	MESH_PROV_MODE_INITIATOR,
	MESH_PROV_MODE_GATT_ACCEPTOR,
	MESH_PROV_MODE_ADV_ACCEPTOR,
	MESH_PROV_MODE_GATT_CLIENT,
	MESH_PROV_MODE_MESH_SERVER,
	MESH_PROV_MODE_MESH_CLIENT,
	MESH_PROV_MODE_MESH_GATT_CLIENT,
};

struct mesh_prov;

typedef void (*prov_trans_tx_t)(void *trans_data, void *data, uint16_t len);
typedef void (*mesh_prov_open_func_t)(void *user_data, prov_trans_tx_t trans_tx,
					void *trans_data, uint8_t trans_type);

typedef void (*mesh_prov_close_func_t)(void *user_data, uint8_t reason);
typedef void (*mesh_prov_send_func_t)(bool success, struct mesh_prov *prov);
typedef void (*mesh_prov_ack_func_t)(void *user_data, uint8_t msg_num);
typedef void (*mesh_prov_receive_func_t)(void *user_data, const uint8_t *data,
								uint16_t size);


struct prov_invite {
	uint8_t attention;
} __packed;

struct prov_invite_msg {
	uint8_t opcode;
	struct prov_invite invite;
} __packed;

struct prov_start {
	uint8_t algorithm;
	uint8_t pub_key;
	uint8_t auth_method;
	uint8_t auth_action;
	uint8_t auth_size;
} __packed;

struct prov_caps_msg {
	uint8_t opcode;
	struct mesh_net_prov_caps caps;
} __packed;

struct prov_start_msg {
	uint8_t opcode;
	struct prov_start start;
} __packed;

struct prov_pub_key_msg {
	uint8_t opcode;
	uint8_t pub_key[64];
} __packed;

struct prov_conf_msg {
	uint8_t opcode;
	uint8_t conf[16];
} __packed;

struct prov_rand_msg {
	uint8_t opcode;
	uint8_t rand[16];
} __packed;

struct prov_data {
	uint8_t net_key[16];
	uint16_t net_idx;
	uint8_t flags;
	uint32_t iv_index;
	uint16_t primary;
} __packed;

struct prov_data_msg {
	uint8_t opcode;
	struct prov_data data;
	uint64_t mic;
} __packed;

struct prov_fail_msg {
	uint8_t opcode;
	uint8_t reason;
} __packed;

struct conf_input {
	struct prov_invite		invite;
	struct mesh_net_prov_caps	caps;
	struct prov_start		start;
	uint8_t				prv_pub_key[64];
	uint8_t				dev_pub_key[64];
} __packed;

struct mesh_prov {
	int ref_count;
	struct mesh_dev *dev;
	struct mesh_net *net;
	enum mesh_prov_mode mode;
	enum mesh_trans trans;
	enum mesh_bearer bearer;
	uint8_t uuid[16];
	uint8_t caps[12];

	uint32_t conn_id;
	uint16_t net_idx;
	uint16_t remote;
	uint16_t addr;
	uint16_t expected_len;
	uint16_t packet_len;
	uint8_t local_msg_num;
	uint8_t peer_msg_num;
	uint8_t last_peer_msg_num;
	uint8_t got_segs;
	uint8_t expected_segs;
	uint8_t expected_fcs;
	uint8_t packet_buf[80];
	uint8_t peer_buf[80];
	struct timeval tx_start;
	struct l_timeout *tx_timeout;

	/* Provisioning credentials and crypto material */
	struct conf_input conf_inputs;
	uint8_t dev_key[16];
	uint8_t conf_salt[16];
	uint8_t s_key[16];
	uint8_t s_nonce[13];
	uint8_t conf_key[16];
	uint8_t conf[16];
	uint8_t r_conf[16];
	uint8_t rand_auth[32];
	uint8_t prov_salt[16];
	uint8_t secret[32];
	uint8_t r_public[64];
	uint8_t l_public[64];
	/* End Provisioning credentials and crypto material */

	mesh_prov_open_func_t open_callback;
	mesh_prov_close_func_t close_callback;
	mesh_prov_receive_func_t receive_callback;
	void *receive_data;
	mesh_prov_send_func_t send_callback;
	void *send_data;
};

struct mesh_prov *mesh_prov_new(struct mesh_net *net, uint16_t remote);

struct mesh_prov *mesh_prov_ref(struct mesh_prov *prov);
void mesh_prov_unref(struct mesh_prov *prov);

bool mesh_prov_gatt_client(struct mesh_prov *prov, struct mesh_dev *dev,
					uint8_t uuid[16],
					mesh_prov_open_func_t open_callback,
					mesh_prov_close_func_t close_callback,
					mesh_prov_receive_func_t recv_callback,
					void *user_data);

bool mesh_prov_listen(struct mesh_net *net, uint8_t uuid[16], uint8_t caps[12],
					mesh_prov_open_func_t open_callback,
					mesh_prov_close_func_t close_callback,
					mesh_prov_receive_func_t recv_callback,
					void *user_data);

bool mesh_prov_connect(struct mesh_prov *prov, struct mesh_dev *dev,
					uint16_t net_idx, uint8_t uuid[16],
					mesh_prov_open_func_t open_callback,
					mesh_prov_close_func_t close_callback,
					mesh_prov_receive_func_t recv_callback,
					void *user_data);

unsigned int mesh_prov_send(struct mesh_prov *prov,
					const void *data, uint16_t size,
					mesh_prov_send_func_t send_callback,
					void *user_data);
bool mesh_prov_cancel(struct mesh_prov *prov, unsigned int id);

bool mesh_prov_close(struct mesh_prov *prov, uint8_t reason);
void mesh_prov_set_addr(struct mesh_prov *prov, uint16_t addr);
uint16_t mesh_prov_get_addr(struct mesh_prov *prov);
void mesh_prov_set_idx(struct mesh_prov *prov, uint16_t net_idx);
uint16_t mesh_prov_get_idx(struct mesh_prov *prov);

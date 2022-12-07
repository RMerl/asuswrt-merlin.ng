/* SPDX-License-Identifier: LGPL-2.1-or-later */
/*
 *
 *  BlueZ - Bluetooth protocol stack for Linux
 *
 *  Copyright (C) 2017  Intel Corporation. All rights reserved.
 *
 *
 */

struct mesh_node;

#define ACTION_ADD		1
#define ACTION_UPDATE		2
#define ACTION_DELETE		3

struct prov_svc_data {
	uint16_t oob;
	uint8_t dev_uuid[16];
};

struct mesh_node_composition {
	bool relay;
	bool proxy;
	bool lpn;
	bool friend;
	uint16_t cid;
	uint16_t pid;
	uint16_t vid;
	uint16_t crpl;
};

struct mesh_publication {
	uint16_t app_idx;
	union {
		uint16_t addr16;
		uint8_t va_128[16];
	} u;
	uint8_t ttl;
	uint8_t credential;
	uint8_t period;
	uint8_t retransmit;
};

typedef bool (*node_model_recv_callback)(uint16_t src, uint8_t *data,
						uint16_t len, void *user_data);
typedef int (*node_model_bind_callback)(uint16_t app_idx, int action);
typedef void (*node_model_pub_callback)(struct mesh_publication *pub);
typedef void (*node_model_sub_callback)(uint16_t sub_addr, int action);

struct mesh_model_ops {
	node_model_recv_callback recv;
	node_model_bind_callback bind;
	node_model_pub_callback pub;
	node_model_sub_callback sub;
};

struct mesh_node *node_find_by_addr(uint16_t addr);
struct mesh_node *node_find_by_uuid(uint8_t uuid[16]);
struct mesh_node *node_create_new(struct prov_svc_data *prov);
struct mesh_node *node_new(void);
void node_free(struct mesh_node *node);
bool node_is_provisioned(struct mesh_node *node);
void *node_get_prov(struct mesh_node *node);
void node_set_prov(struct mesh_node *node, void *prov);
bool node_app_key_add(struct mesh_node *node, uint16_t idx);
bool node_net_key_add(struct mesh_node *node, uint16_t index);
bool node_app_key_delete(struct mesh_node *node, uint16_t net_idx,
				uint16_t idx);
bool node_net_key_delete(struct mesh_node *node, uint16_t index);
void node_set_primary(struct mesh_node *node, uint16_t unicast);
uint16_t node_get_primary(struct mesh_node *node);
uint16_t node_get_primary_net_idx(struct mesh_node *node);
void node_set_device_key(struct mesh_node *node, uint8_t *key);
uint8_t *node_get_device_key(struct mesh_node *node);
void node_set_num_elements(struct mesh_node *node, uint8_t num_ele);
uint8_t node_get_num_elements(struct mesh_node *node);
bool node_parse_composition(struct mesh_node *node, uint8_t *buf, uint16_t len);
GList *node_get_net_keys(struct mesh_node *node);
GList *node_get_app_keys(struct mesh_node *node);
void node_cleanup(void);

bool node_set_local_node(struct mesh_node *node);
struct mesh_node *node_get_local_node(void);
void node_local_data_handler(uint16_t src, uint32_t dst,
				uint32_t iv_index, uint32_t seq_num,
				uint16_t app_idx, uint8_t *data, uint16_t len);

bool node_local_model_register(uint8_t element_idx, uint16_t model_id,
				struct mesh_model_ops *ops, void *user_data);
bool node_local_vendor_model_register(uint8_t element_idx, uint32_t model_id,
				struct mesh_model_ops *ops, void *user_data);

bool node_set_element(struct mesh_node *node, uint8_t ele_idx);
bool node_set_model(struct mesh_node *node, uint8_t ele_idx, uint32_t id);
struct mesh_node_composition *node_get_composition(struct mesh_node *node);
bool node_set_composition(struct mesh_node *node,
				struct mesh_node_composition *comp);
bool node_add_binding(struct mesh_node *node, uint8_t ele_idx,
			uint32_t model_id, uint16_t app_idx);
bool node_add_subscription(struct mesh_node *node, uint8_t ele_idx,
			   uint32_t model_id, uint16_t addr);
uint8_t node_get_default_ttl(struct mesh_node *node);
bool node_set_default_ttl(struct mesh_node *node, uint8_t ttl);
bool node_set_sequence_number(struct mesh_node *node, uint32_t seq);
uint32_t node_get_sequence_number(struct mesh_node *node);
bool node_set_iv_index(struct mesh_node *node, uint32_t iv_index);
uint32_t node_get_iv_index(struct mesh_node *node);
bool node_model_pub_set(struct mesh_node *node, uint8_t ele, uint32_t model_id,
						struct mesh_publication *pub);
struct mesh_publication *node_model_pub_get(struct mesh_node *node, uint8_t ele,
							uint32_t model_id);

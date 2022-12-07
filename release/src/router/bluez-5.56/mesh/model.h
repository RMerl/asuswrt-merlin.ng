/* SPDX-License-Identifier: LGPL-2.1-or-later */
/*
 *
 *  BlueZ - Bluetooth protocol stack for Linux
 *
 *  Copyright (C) 2018-2020  Intel Corporation. All rights reserved.
 *
 *
 */

struct mesh_model;

#define MAX_MODEL_BINDINGS	10
#define MAX_MODEL_SUBS		10

#define ACTION_ADD	1
#define ACTION_UPDATE	2
#define ACTION_DELETE	3

/* For internal representation of SIG defined models */
#define SIG_VENDOR	0xFFFF

#define IS_VENDOR(x)	((x) < ((uint32_t)(SIG_VENDOR) << 16))
#define SET_ID(v, m)	((((uint32_t) (v)) << 16) | (m))
#define MODEL_ID(x)	((x) & ~VENDOR_ID_MASK)
#define VENDOR_ID(x)	((x) >> 16)

struct mesh_virtual;

struct mesh_model_pub {
	struct mesh_virtual *virt;
	uint16_t addr;
	uint16_t idx;
	struct {
		uint16_t interval;
		uint8_t cnt;
	} rtx;
	uint8_t ttl;
	uint8_t credential;
	uint8_t period;
};

typedef void (*mesh_model_unregister)(void *user_data);
typedef bool (*mesh_model_recv_cb)(uint16_t src, uint16_t unicast,
					uint16_t app_idx, uint16_t net_idx,
					const uint8_t *data, uint16_t len,
					const void *user_data);
typedef int (*mesh_model_bind_cb)(uint16_t app_idx, int action);
typedef int (*mesh_model_pub_cb)(struct mesh_model_pub *pub);
typedef int (*mesh_model_sub_cb)(uint16_t sub_addr, int action);

struct mesh_model_ops {
	mesh_model_unregister unregister;
	mesh_model_recv_cb recv;
	mesh_model_bind_cb bind;
	mesh_model_pub_cb pub;
	mesh_model_sub_cb sub;
};

bool mesh_model_add(struct mesh_node *node, struct l_queue *mods,
			uint32_t id, struct l_dbus_message_iter *opts);
void mesh_model_free(void *data);
bool mesh_model_register(struct mesh_node *node, uint8_t ele_idx,
			uint32_t id, const struct mesh_model_ops *cbs,
							void *user_data);
bool mesh_model_add_from_storage(struct mesh_node *node, uint8_t ele_idx,
				struct l_queue *mods, struct l_queue *db_mods);
void mesh_model_convert_to_storage(struct l_queue *db_mods,
							struct l_queue *mods);
struct mesh_model_pub *mesh_model_pub_get(struct mesh_node *node,
						uint16_t ele_addr,
						uint32_t id, int *status);
int mesh_model_pub_set(struct mesh_node *node, uint16_t ele_addr, uint32_t id,
			const uint8_t *addr, uint16_t idx, bool cred_flag,
			uint8_t ttl, uint8_t period, uint8_t rtx_cnt,
			uint16_t rtx_interval, bool is_virt, uint16_t *dst);

int mesh_model_binding_add(struct mesh_node *node, uint16_t ele_addr,
						uint32_t id, uint16_t idx);
int mesh_model_binding_del(struct mesh_node *node, uint16_t ele_addr,
						uint32_t id, uint16_t idx);
int mesh_model_get_bindings(struct mesh_node *node, uint16_t ele_addr,
				uint32_t id, uint8_t *buf, uint16_t buf_sz,
								uint16_t *len);
int mesh_model_sub_add(struct mesh_node *node, uint16_t ele_addr, uint32_t id,
								uint16_t grp);
int mesh_model_virt_sub_add(struct mesh_node *node, uint16_t ele_addr,
					uint32_t id, const uint8_t *label,
					uint16_t *addr);
int mesh_model_sub_del(struct mesh_node *node, uint16_t ele_addr, uint32_t id,
								uint16_t grp);
int mesh_model_virt_sub_del(struct mesh_node *node, uint16_t ele_addr,
					uint32_t id, const uint8_t *label,
					uint16_t *addr);
int mesh_model_sub_del_all(struct mesh_node *node, uint16_t ele_addr,
								uint32_t id);
int mesh_model_sub_ovrt(struct mesh_node *node, uint16_t ele_addr, uint32_t id,
								uint16_t addr);
int mesh_model_virt_sub_ovrt(struct mesh_node *node, uint16_t ele_addr,
					uint32_t id, const uint8_t *label,
					uint16_t *addr);
int mesh_model_sub_get(struct mesh_node *node, uint16_t ele_addr, uint32_t id,
			uint8_t *buf, uint16_t buf_size, uint16_t *size);
uint16_t mesh_model_cfg_blk(uint8_t *pkt);
bool mesh_model_send(struct mesh_node *node, uint16_t src, uint16_t dst,
			uint16_t app_idx, uint16_t net_idx, uint8_t ttl,
			bool segmented, uint16_t len, const void *data);
int mesh_model_publish(struct mesh_node *node, uint32_t id, uint16_t src,
			bool segmented, uint16_t len, const void *data);
bool mesh_model_rx(struct mesh_node *node, bool szmict, uint32_t seq0,
			uint32_t iv_index, uint16_t net_idx, uint16_t src,
			uint16_t dst, uint8_t key_aid, const uint8_t *data,
								uint16_t size);
void mesh_model_app_key_delete(struct mesh_node *node, uint16_t ele_idx,
				struct l_queue *models, uint16_t app_idx);
uint16_t mesh_model_opcode_set(uint32_t opcode, uint8_t *buf);
bool mesh_model_opcode_get(const uint8_t *buf, uint16_t size, uint32_t *opcode,
								uint16_t *n);
void mesh_model_build_config(void *model, void *msg_builder);
void mesh_model_update_opts(struct mesh_node *node, uint8_t ele_idx,
				struct l_queue *curr, struct l_queue *updated);
uint16_t mesh_model_generate_composition(struct l_queue *mods, uint16_t buf_sz,
								uint8_t *buf);
void mesh_model_init(void);
void mesh_model_cleanup(void);

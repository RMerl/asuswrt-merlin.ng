/* SPDX-License-Identifier: LGPL-2.1-or-later */
/*
 *
 *  BlueZ - Bluetooth protocol stack for Linux
 *
 *  Copyright (C) 2018-2019  Intel Corporation. All rights reserved.
 *
 *
 */

struct mesh_net;
struct mesh_node;
struct mesh_io;
struct mesh_agent;
struct mesh_config;
struct mesh_config_node;

typedef void (*node_ready_func_t) (void *user_data, int status,
							struct mesh_node *node);

typedef void (*node_join_ready_func_t) (struct mesh_node *node,
						struct mesh_agent *agent);

void node_remove(struct mesh_node *node);
void node_join(const char *app_root, const char *sender, const uint8_t *uuid,
						node_join_ready_func_t cb);
uint8_t *node_uuid_get(struct mesh_node *node);
struct mesh_net *node_get_net(struct mesh_node *node);
struct mesh_node *node_find_by_addr(uint16_t addr);
struct mesh_node *node_find_by_uuid(uint8_t uuid[16]);
struct mesh_node *node_find_by_token(uint64_t token);
bool node_is_provisioner(struct mesh_node *node);
bool node_is_busy(struct mesh_node *node);
void node_app_key_delete(struct mesh_node *node, uint16_t net_idx,
							uint16_t app_idx);
uint16_t node_get_primary(struct mesh_node *node);
uint16_t node_get_primary_net_idx(struct mesh_node *node);
void node_set_token(struct mesh_node *node, uint8_t token[8]);
const uint8_t *node_get_token(struct mesh_node *node);
const uint8_t *node_get_device_key(struct mesh_node *node);
void node_set_num_elements(struct mesh_node *node, uint8_t num_ele);
uint8_t node_get_num_elements(struct mesh_node *node);
uint8_t node_default_ttl_get(struct mesh_node *node);
bool node_default_ttl_set(struct mesh_node *node, uint8_t ttl);
bool node_set_sequence_number(struct mesh_node *node, uint32_t seq);
uint32_t node_get_sequence_number(struct mesh_node *node);
int node_get_element_idx(struct mesh_node *node, uint16_t ele_addr);
struct l_queue *node_get_element_models(struct mesh_node *node,
							uint8_t ele_idx);
uint16_t node_get_crpl(struct mesh_node *node);
bool node_init_from_storage(struct mesh_node *node, const uint8_t uuid[16],
					struct mesh_config_node *db_node);
const uint8_t *node_get_comp(struct mesh_node *node, uint8_t page_num,
								uint16_t *len);
bool node_replace_comp(struct mesh_node *node, uint8_t retire, uint8_t with);
uint8_t node_lpn_mode_get(struct mesh_node *node);
bool node_relay_mode_set(struct mesh_node *node, bool enable, uint8_t cnt,
							uint16_t interval);
uint8_t node_relay_mode_get(struct mesh_node *node, uint8_t *cnt,
							uint16_t *interval);
bool node_proxy_mode_set(struct mesh_node *node, bool enable);
uint8_t node_proxy_mode_get(struct mesh_node *node);
bool node_beacon_mode_set(struct mesh_node *node, bool enable);
uint8_t node_beacon_mode_get(struct mesh_node *node);
bool node_friend_mode_set(struct mesh_node *node, bool enable);
uint8_t node_friend_mode_get(struct mesh_node *node);
const char *node_get_element_path(struct mesh_node *node, uint8_t ele_idx);
const char *node_get_owner(struct mesh_node *node);
const char *node_get_app_path(struct mesh_node *node);
bool node_add_pending_local(struct mesh_node *node, void *info);
void node_attach_io_all(struct mesh_io *io);
void node_attach_io(struct mesh_node *node, struct mesh_io *io);
void node_attach(const char *app_root, const char *sender, uint64_t token,
					node_ready_func_t cb, void *user_data);
void node_build_attach_reply(struct mesh_node *node,
						struct l_dbus_message *reply);
void node_create(const char *app_root, const char *sender, const uint8_t *uuid,
					node_ready_func_t cb, void *user_data);
void node_import(const char *app_root, const char *sender, const uint8_t *uuid,
			const uint8_t dev_key[16], const uint8_t net_key[16],
			uint16_t net_idx, bool kr, bool ivu,
			uint32_t iv_index, uint16_t unicast,
			node_ready_func_t cb, void *user_data);
bool node_dbus_init(struct l_dbus *bus);
void node_cleanup_all(void);
struct mesh_config *node_config_get(struct mesh_node *node);
struct mesh_agent *node_get_agent(struct mesh_node *node);
const char *node_get_storage_dir(struct mesh_node *node);
bool node_load_from_storage(const char *storage_dir);
void node_finalize_new_node(struct mesh_node *node, struct mesh_io *io);
void node_property_changed(struct mesh_node *node, const char *property);

/* SPDX-License-Identifier: LGPL-2.1-or-later */
/*
 *
 *  BlueZ - Bluetooth protocol stack for Linux
 *
 *  Copyright (C) 2019  Intel Corporation. All rights reserved.
 *
 *
 */

#include "mesh/mesh-config.h"

struct mesh_group;

bool mesh_db_create(const char *fname, const uint8_t token[8],
							const char *name);
bool mesh_db_load(const char *fname);

bool mesh_db_get_token(uint8_t token[8]);
bool mesh_db_set_iv_index(uint32_t ivi);
uint32_t mesh_db_get_iv_index(void);
bool mesh_db_net_key_add(uint16_t idx);
bool mesh_db_net_key_del(uint16_t idx);
bool mesh_db_net_key_phase_set(uint16_t net_idx, uint8_t phase);
bool mesh_db_app_key_add(uint16_t net_idx, uint16_t app_idx);
bool mesh_db_app_key_del(uint16_t app_idx);
bool mesh_db_set_addr_range(uint16_t low, uint16_t high);
bool mesh_db_get_addr_range(uint16_t *low, uint16_t *high);

bool mesh_db_add_node(uint8_t uuid[16], uint8_t num_els, uint16_t unicast,
							uint16_t net_idx);
bool mesh_db_del_node(uint16_t unicast);
bool mesh_db_node_set_composition(uint16_t unicast, uint8_t *data,
								uint16_t len);
bool mesh_db_add_provisioner(const char *name, uint8_t uuid[16],
				uint16_t unicast_low, uint16_t unicast_high,
				uint16_t group_low, uint16_t group_high);
bool mesh_db_node_set_net_transmit(uint16_t unicast, uint8_t cnt,
							uint16_t interval);
bool mesh_db_node_net_key_add(uint16_t unicast, uint16_t idx);
bool mesh_db_node_net_key_del(uint16_t unicast, uint16_t idx);
bool mesh_db_node_app_key_add(uint16_t unicast, uint16_t idx);
bool mesh_db_node_app_key_del(uint16_t unicast, uint16_t idx);
bool mesh_db_node_ttl_set(uint16_t unicast, uint8_t ttl);
bool mesh_db_node_write_mode(uint16_t unicast, const char *keyword, int value);
bool mesh_db_node_model_binding_add(uint16_t unicast, uint8_t ele, bool vendor,
					uint32_t mod_id, uint16_t app_idx);
bool mesh_db_node_model_binding_del(uint16_t unicast, uint8_t ele, bool vendor,
					uint32_t mod_id, uint16_t app_idx);
struct l_queue *mesh_db_load_groups(void);
bool mesh_db_add_group(struct mesh_group *grp);
bool mesh_db_add_blacklisted_addr(uint16_t unicast, uint32_t iv_index);
bool mesh_db_clear_blacklisted(uint32_t iv_index);

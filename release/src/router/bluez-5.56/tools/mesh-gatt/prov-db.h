/* SPDX-License-Identifier: LGPL-2.1-or-later */
/*
 *
 *  BlueZ - Bluetooth protocol stack for Linux
 *
 *  Copyright (C) 2017  Intel Corporation. All rights reserved.
 *
 *
 */

bool prov_db_show(const char *filename);
bool prov_db_read(const char *filename);
bool prov_db_read_local_node(const char *filename, bool provisioner);
bool prov_db_add_new_node(struct mesh_node *node);
bool prov_db_add_node_composition(struct mesh_node *node, uint8_t *data,
								uint16_t len);
bool prov_db_node_keys(struct mesh_node *node, GList *idxs, const char *desc);
bool prov_db_add_binding(struct mesh_node *node, uint8_t ele_idx,
			uint32_t model_id, uint16_t app_idx);
bool prov_db_add_subscription(struct mesh_node *node, uint8_t ele_idx,
			      uint32_t model_id, uint16_t addr);
bool prov_db_node_set_ttl(struct mesh_node *node, uint8_t ttl);
bool prov_db_node_set_iv_seq(struct mesh_node *node, uint32_t iv, uint32_t seq);
bool prov_db_local_set_iv_index(uint32_t iv_index, bool update, bool prov);
bool prov_db_local_set_seq_num(uint32_t seq_num);
bool prov_db_node_set_model_pub(struct mesh_node *node, uint8_t ele_idx,
							uint32_t model_id,
						struct mesh_publication *pub);
void prov_db_print_node_composition(struct mesh_node *node);

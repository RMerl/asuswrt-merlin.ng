/* SPDX-License-Identifier: LGPL-2.1-or-later */
/*
 *
 *  BlueZ - Bluetooth protocol stack for Linux
 *
 *  Copyright (C) 2017  Intel Corporation. All rights reserved.
 *
 *
 */

#include "gdbus/gdbus.h"

typedef void (*net_mesh_session_open_callback)(int status);

uint32_t net_get_iv_index(bool *iv_update);
bool net_get_key(uint16_t net_idx, uint8_t *key);
bool net_get_flags(uint16_t net_idx, uint8_t *out_flags);
void net_set_iv_index(uint32_t index, bool update);
uint32_t get_sequence_number(void);
void set_sequence_number(uint32_t seq_number);
uint16_t net_validate_proxy_beacon(const uint8_t *proxy_beacon);
bool net_add_address_pool(uint16_t min, uint16_t max);
uint16_t net_obtain_address(uint8_t num_elements);
bool net_reserve_address_range(uint16_t base, uint8_t num_elements);
void net_release_address(uint16_t addr, uint8_t num_elements);
bool net_session_open(GDBusProxy *data_in, bool provisioner,
					net_mesh_session_open_callback cb);
void net_session_close(GDBusProxy *data_in);

bool net_data_ready(uint8_t *ptr, uint8_t len);
bool net_access_layer_send(uint8_t ttl, uint16_t src, uint32_t dst,
				uint16_t app_idx, uint8_t *buf, uint16_t len);
bool net_ctl_msg_send(uint8_t ttl, uint16_t src, uint16_t dst,
					uint8_t *buf, uint16_t len);
bool net_set_default_ttl(uint8_t ttl);
uint8_t net_get_default_ttl(void);
bool net_set_seq_num(uint32_t seq_num);
uint32_t net_get_seq_num(void);
void net_dest_ref(uint16_t dst);
void net_dest_unref(uint16_t dst);
bool net_register_unicast(uint16_t unicast, uint8_t count);
bool net_register_group(uint16_t group_addr);
uint32_t net_register_virtual(uint8_t buf[16]);
bool mesh_model_recv(uint16_t app_idx, uint16_t src, uint32_t dst,
						uint8_t *payload, uint16_t len);

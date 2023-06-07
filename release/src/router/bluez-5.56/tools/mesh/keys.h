/* SPDX-License-Identifier: LGPL-2.1-or-later */
/*
 *
 *  BlueZ - Bluetooth protocol stack for Linux
 *
 *  Copyright (C) 2019  Intel Corporation. All rights reserved.
 *
 *
 */

void keys_add_net_key(uint16_t net_idx);
void keys_del_net_key(uint16_t net_idx);
void keys_set_net_key_phase(uint16_t net_idx, uint8_t phase, bool save);
bool keys_get_net_key_phase(uint16_t net_idx, uint8_t *phase);
void keys_add_app_key(uint16_t net_idx, uint16_t app_idx);
void keys_del_app_key(uint16_t app_idx);
uint16_t keys_get_bound_key(uint16_t app_idx);
bool keys_subnet_exists(uint16_t idx);
void keys_print_keys(void);

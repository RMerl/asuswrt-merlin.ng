/* SPDX-License-Identifier: LGPL-2.1-or-later */
/*
 *
 *  BlueZ - Bluetooth protocol stack for Linux
 *
 *  Copyright (C) 2015  Intel Corporation. All rights reserved.
 *
 *
 */

#include <stdint.h>

void gatt_set_static_address(uint8_t addr[6]);
void gatt_set_device_name(uint8_t name[20], uint8_t len);

void gatt_server_start(void);
void gatt_server_stop(void);

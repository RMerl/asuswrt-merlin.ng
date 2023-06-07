/* SPDX-License-Identifier: LGPL-2.1-or-later */
/*
 *
 *  BlueZ - Bluetooth protocol stack for Linux
 *
 *  Copyright (C) 2017  Intel Corporation. All rights reserved.
 *
 *
 */

#define GENERIC_ONOFF_SERVER_MODEL_ID	0x1000
#define GENERIC_ONOFF_CLIENT_MODEL_ID	0x1001

#define OP_GENERIC_ONOFF_GET			0x8201
#define OP_GENERIC_ONOFF_SET			0x8202
#define OP_GENERIC_ONOFF_SET_UNACK		0x8203
#define OP_GENERIC_ONOFF_STATUS			0x8204

void onoff_set_node(const char *args);
bool onoff_client_init(uint8_t ele);

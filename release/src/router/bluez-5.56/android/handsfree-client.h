/* SPDX-License-Identifier: LGPL-2.1-or-later */
/*
 *
 *  BlueZ - Bluetooth protocol stack for Linux
 *
 *  Copyright (C) 2014  Intel Corporation. All rights reserved.
 *
 *
 */

bool bt_hf_client_register(struct ipc *ipc, const bdaddr_t *addr);
void bt_hf_client_unregister(void);

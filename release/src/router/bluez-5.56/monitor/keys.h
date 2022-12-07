/* SPDX-License-Identifier: LGPL-2.1-or-later */
/*
 *
 *  BlueZ - Bluetooth protocol stack for Linux
 *
 *  Copyright (C) 2011-2014  Intel Corporation
 *  Copyright (C) 2002-2010  Marcel Holtmann <marcel@holtmann.org>
 *
 *
 */

#include <stdint.h>
#include <stdbool.h>

void keys_setup(void);
void keys_cleanup(void);

void keys_update_identity_key(const uint8_t key[16]);
void keys_update_identity_addr(const uint8_t addr[6], uint8_t addr_type);

bool keys_resolve_identity(const uint8_t addr[6], uint8_t ident[6],
							uint8_t *ident_type);

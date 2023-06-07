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

void ellisys_enable(const char *server, uint16_t port);

void ellisys_inject_hci(struct timeval *tv, uint16_t index, uint16_t opcode,
					const void *data, uint16_t size);

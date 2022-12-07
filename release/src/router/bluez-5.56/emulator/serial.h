/* SPDX-License-Identifier: LGPL-2.1-or-later */
/*
 *
 *  BlueZ - Bluetooth protocol stack for Linux
 *
 *  Copyright (C) 2011-2014  Intel Corporation
 *  Copyright (C) 2004-2010  Marcel Holtmann <marcel@holtmann.org>
 *
 *
 */

#include <stdint.h>

enum serial_type {
	SERIAL_TYPE_BREDRLE,
	SERIAL_TYPE_BREDR,
	SERIAL_TYPE_LE,
	SERIAL_TYPE_AMP,
};

struct serial;

struct serial *serial_open(enum serial_type type);
void serial_close(struct serial *serial);

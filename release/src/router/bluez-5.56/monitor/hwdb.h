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

bool hwdb_get_vendor_model(const char *modalias, char **vendor, char **model);
bool hwdb_get_company(const uint8_t *bdaddr, char **company);

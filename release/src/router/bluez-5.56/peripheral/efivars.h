/* SPDX-License-Identifier: LGPL-2.1-or-later */
/*
 *
 *  BlueZ - Bluetooth protocol stack for Linux
 *
 *  Copyright (C) 2015  Intel Corporation. All rights reserved.
 *
 *
 */

#define EFIVARS_NON_VOLATILE			0x00000001
#define EFIVARS_BOOTSERVICE_ACCESS		0x00000002
#define EFIVARS_RUNTIME_ACCESS			0x00000004
#define EFIVARS_HARDWARE_ERROR_RECORD		0x00000008
#define EFIVARS_AUTHENTICATED_WRITE_ACCESS	0x00000010

int efivars_read(const char *name, uint32_t *attributes,
					void *data, size_t size);
int efivars_write(const char *name, uint32_t attributes,
					const void *data, size_t size);

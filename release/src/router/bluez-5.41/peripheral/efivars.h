/*
 *
 *  BlueZ - Bluetooth protocol stack for Linux
 *
 *  Copyright (C) 2015  Intel Corporation. All rights reserved.
 *
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Lesser General Public
 *  License as published by the Free Software Foundation; either
 *  version 2.1 of the License, or (at your option) any later version.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public
 *  License along with this library; if not, write to the Free Software
 *  Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
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

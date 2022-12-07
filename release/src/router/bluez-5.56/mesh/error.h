/* SPDX-License-Identifier: LGPL-2.1-or-later */
/*
 *
 *  BlueZ - Bluetooth protocol stack for Linux
 *
 *  Copyright (C) 2018-2019  Intel Corporation. All rights reserved.
 *
 *
 */

/*
 * Important: Changes in this table must be reflected in the
 * the entries of error_table[] in dbus.c
 */
enum mesh_error {
	MESH_ERROR_NONE,
	MESH_ERROR_FAILED,
	MESH_ERROR_NOT_AUTHORIZED,
	MESH_ERROR_NOT_FOUND,
	MESH_ERROR_INVALID_ARGS,
	MESH_ERROR_IN_PROGRESS,
	MESH_ERROR_BUSY,
	MESH_ERROR_ALREADY_EXISTS,
	MESH_ERROR_DOES_NOT_EXIST,
	MESH_ERROR_CANCELED,
	MESH_ERROR_NOT_IMPLEMENTED,
};

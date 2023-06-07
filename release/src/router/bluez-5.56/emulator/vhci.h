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

enum vhci_type {
	VHCI_TYPE_BREDRLE,
	VHCI_TYPE_BREDR,
	VHCI_TYPE_LE,
	VHCI_TYPE_AMP,
};

struct vhci;

typedef void (*vhci_debug_func_t)(const char *str, void *user_data);
typedef void (*vhci_destroy_func_t)(void *user_data);
bool vhci_set_debug(struct vhci *vhci, vhci_debug_func_t callback,
			void *user_data, vhci_destroy_func_t destroy);

struct vhci *vhci_open(enum vhci_type type);
void vhci_close(struct vhci *vhci);

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

struct vendor_ocf {
	uint16_t ocf;
	const char *str;
	void (*cmd_func) (const void *data, uint8_t size);
	uint8_t cmd_size;
	bool cmd_fixed;
	void (*rsp_func) (const void *data, uint8_t size);
	uint8_t rsp_size;
	bool rsp_fixed;
};

struct vendor_evt {
	uint8_t evt;
	const char *str;
	void (*evt_func) (const void *data, uint8_t size);
	uint8_t evt_size;
	bool evt_fixed;
};

void vendor_event(uint16_t manufacturer, const void *data, uint8_t size);

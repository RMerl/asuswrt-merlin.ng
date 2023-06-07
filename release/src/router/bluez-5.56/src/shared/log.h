/* SPDX-License-Identifier: LGPL-2.1-or-later */
/*
 *
 *  BlueZ - Bluetooth protocol stack for Linux
 *
 *  Copyright (C) 2018  Intel Corporation. All rights reserved.
 *
 *
 */

int bt_log_open(void);
int bt_log_sendmsg(uint16_t index, const char *label, int level,
					struct iovec *io, size_t io_len);
int bt_log_vprintf(uint16_t index, const char *label, int level,
					const char *format, va_list ap);
int bt_log_printf(uint16_t index, const char *label, int level,
					const char *format, ...);
void bt_log_close(void);

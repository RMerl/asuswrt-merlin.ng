/* SPDX-License-Identifier: GPL-2.0-or-later */
/*
 *
 *  BlueZ - Bluetooth protocol stack for Linux
 *
 *  Copyright (C) 2004-2010  Marcel Holtmann <marcel@holtmann.org>
 *
 *
 */

#include <stdint.h>

void btd_backtrace_init(void);
void btd_backtrace(uint16_t index);

void btd_assertion_message_expr(const char *file, int line,
					const char *func, const char *expr);

#define btd_assert(expr) do { \
	if (expr) ; else \
		btd_assertion_message_expr(__FILE__, __LINE__, __func__, #expr); \
	} while (0)

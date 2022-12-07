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

void info(const char *format, ...) __attribute__((format(printf, 1, 2)));

void btd_log(uint16_t index, int priority, const char *format, ...)
					__attribute__((format(printf, 3, 4)));

void btd_error(uint16_t index, const char *format, ...)
					__attribute__((format(printf, 2, 3)));
void btd_warn(uint16_t index, const char *format, ...)
					__attribute__((format(printf, 2, 3)));
void btd_info(uint16_t index, const char *format, ...)
					__attribute__((format(printf, 2, 3)));
void btd_debug(uint16_t index, const char *format, ...)
					__attribute__((format(printf, 2, 3)));

void __btd_log_init(const char *debug, int detach);
void __btd_log_cleanup(void);
void __btd_toggle_debug(void);

struct btd_debug_desc {
	const char *file;
#define BTD_DEBUG_FLAG_DEFAULT (0)
#define BTD_DEBUG_FLAG_PRINT   (1 << 0)
	unsigned int flags;
} __attribute__((aligned(8)));

void __btd_enable_debug(struct btd_debug_desc *start,
					struct btd_debug_desc *stop);

/**
 * DBG:
 * @fmt: format string
 * @arg...: list of arguments
 *
 * Simple macro around btd_debug() which also include the function
 * name it is called in.
 */
#define DBG_IDX(idx, fmt, arg...) do { \
	static struct btd_debug_desc __btd_debug_desc \
	__attribute__((used, section("__debug"), aligned(8))) = { \
		.file = __FILE__, .flags = BTD_DEBUG_FLAG_DEFAULT, \
	}; \
	if (__btd_debug_desc.flags & BTD_DEBUG_FLAG_PRINT) \
		btd_debug(idx, "%s:%s() " fmt, __FILE__, __func__ , ## arg); \
} while (0)

#define DBG(fmt, arg...) DBG_IDX(0xffff, fmt, ## arg)
#define error(fmt, arg...) \
	btd_error(0xffff, "%s:%s() " fmt, __FILE__, __func__, ## arg)
#define warn(fmt, arg...) \
	btd_warn(0xffff, "%s:%s() " fmt, __FILE__, __func__, ## arg)

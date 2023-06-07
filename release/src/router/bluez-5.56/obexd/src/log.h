/* SPDX-License-Identifier: GPL-2.0-or-later */
/*
 *
 *  OBEX Server
 *
 *  Copyright (C) 2007-2010  Marcel Holtmann <marcel@holtmann.org>
 *
 *
 */

void info(const char *format, ...) __attribute__((format(printf, 1, 2)));
void error(const char *format, ...) __attribute__((format(printf, 1, 2)));

void obex_debug(const char *format, ...) __attribute__((format(printf, 1, 2)));

void __obex_log_init(const char *debug, int detach);
void __obex_log_cleanup(void);
void __obex_log_enable_debug(void);

struct obex_debug_desc {
	const char *name;
	const char *file;
#define OBEX_DEBUG_FLAG_DEFAULT (0)
#define OBEX_DEBUG_FLAG_PRINT   (1 << 0)
	unsigned int flags;
} __attribute__((aligned(8)));

/**
 * DBG:
 * @fmt: format string
 * @arg...: list of arguments
 *
 * Simple macro around debug() which also include the function
 * name it is called in.
 */
#define DBG(fmt, arg...) do { \
	static struct obex_debug_desc __obex_debug_desc \
	__attribute__((used, section("__debug"), aligned(8))) = { \
		.file = __FILE__, .flags = OBEX_DEBUG_FLAG_DEFAULT, \
	}; \
	if (__obex_debug_desc.flags & OBEX_DEBUG_FLAG_PRINT) \
		obex_debug("%s:%s() " fmt,  __FILE__, __func__ , ## arg); \
} while (0)

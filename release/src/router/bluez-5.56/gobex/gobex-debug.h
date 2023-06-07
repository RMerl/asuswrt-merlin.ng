/* SPDX-License-Identifier: GPL-2.0-or-later */
/*
 *  OBEX library with GLib integration
 *
 *  Copyright (C) 2011  Intel Corporation. All rights reserved.
 *
 */

#ifndef __GOBEX_DEBUG_H
#define __GOBEX_DEBUG_H

#include <glib.h>
#include <stdio.h>
#include <ctype.h>

#define G_OBEX_DEBUG_NONE	1
#define G_OBEX_DEBUG_ERROR	(1 << 1)
#define G_OBEX_DEBUG_COMMAND	(1 << 2)
#define G_OBEX_DEBUG_TRANSFER	(1 << 3)
#define G_OBEX_DEBUG_HEADER	(1 << 4)
#define G_OBEX_DEBUG_PACKET	(1 << 5)
#define G_OBEX_DEBUG_DATA	(1 << 6)
#define G_OBEX_DEBUG_APPARAM	(1 << 7)

extern guint gobex_debug;

#define g_obex_debug(level, format, ...) \
	if (gobex_debug & level) \
		g_log("gobex", G_LOG_LEVEL_DEBUG, "%s:%s() " format, __FILE__, \
						__func__, ## __VA_ARGS__)

static inline void g_obex_dump(guint level, const char *prefix,
					const void *buf, gsize len)
{
	const guint8 *data = buf;
	int n = 0;

	if (!(gobex_debug & level))
		return;

	while (len > 0) {
		int i, size;

		printf("%s %04x:", prefix, n);

		size = len > 16 ? 16 : len;

		for (i = 0; i < size; i++)
			printf("%02x%s", data[i], (i + 1) % 8 ? " " : "  ");

		for (; i < 16; i++)
			printf("  %s", (i + 1) % 8 ? " " : "  ");

		for (i = 0; i < size; i++)
			printf("%1c", isprint(data[i]) ? data[i] : '.');

		printf("\n");

		data += size;
		len -= size;
		n += size;
	}
}

#endif /* __GOBEX_DEBUG_H */

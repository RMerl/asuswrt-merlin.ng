// SPDX-License-Identifier: GPL-2.0-or-later
/*
 *
 *  BlueZ - Bluetooth protocol stack for Linux
 *
 *  Copyright (C) 2004-2010  Marcel Holtmann <marcel@holtmann.org>
 *
 *
 */

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#define _GNU_SOURCE
#include <stdio.h>
#include <errno.h>
#include <syslog.h>
#include <stdarg.h>
#include <stdint.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/socket.h>

#include <glib.h>

#include "lib/bluetooth.h"
#include "lib/hci.h"

#include "src/shared/util.h"
#include "src/shared/log.h"
#include "log.h"

#define LOG_IDENT "bluetoothd"

static void monitor_log(uint16_t index, int priority,
					const char *format, va_list ap)
{
	bt_log_vprintf(index, LOG_IDENT, priority, format, ap);
}

void info(const char *format, ...)
{
	va_list ap;

	va_start(ap, format);
	vsyslog(LOG_INFO, format, ap);
	va_end(ap);

	va_start(ap, format);
	monitor_log(HCI_DEV_NONE, LOG_INFO, format, ap);
	va_end(ap);
}

void btd_log(uint16_t index, int priority, const char *format, ...)
{
	va_list ap;

	va_start(ap, format);
	vsyslog(priority, format, ap);
	va_end(ap);

	va_start(ap, format);
	monitor_log(index, priority, format, ap);
	va_end(ap);
}

void btd_error(uint16_t index, const char *format, ...)
{
	va_list ap;

	va_start(ap, format);
	vsyslog(LOG_ERR, format, ap);
	va_end(ap);

	va_start(ap, format);
	monitor_log(index, LOG_ERR, format, ap);
	va_end(ap);
}

void btd_warn(uint16_t index, const char *format, ...)
{
	va_list ap;

	va_start(ap, format);
	vsyslog(LOG_WARNING, format, ap);
	va_end(ap);

	va_start(ap, format);
	monitor_log(index, LOG_WARNING, format, ap);
	va_end(ap);
}

void btd_info(uint16_t index, const char *format, ...)
{
	va_list ap;

	va_start(ap, format);
	vsyslog(LOG_INFO, format, ap);
	va_end(ap);

	va_start(ap, format);
	monitor_log(index, LOG_INFO, format, ap);
	va_end(ap);
}

void btd_debug(uint16_t index, const char *format, ...)
{
	va_list ap;

	va_start(ap, format);
	vsyslog(LOG_DEBUG, format, ap);
	va_end(ap);

	va_start(ap, format);
	monitor_log(index, LOG_DEBUG, format, ap);
	va_end(ap);
}

extern struct btd_debug_desc __start___debug[];
extern struct btd_debug_desc __stop___debug[];

static char **enabled = NULL;

static gboolean is_enabled(struct btd_debug_desc *desc)
{
	int i;

	if (enabled == NULL)
		return 0;

	for (i = 0; enabled[i] != NULL; i++)
		if (desc->file != NULL && g_pattern_match_simple(enabled[i],
							desc->file) == TRUE)
			return 1;

	return 0;
}

void __btd_enable_debug(struct btd_debug_desc *start,
					struct btd_debug_desc *stop)
{
	struct btd_debug_desc *desc;

	if (start == NULL || stop == NULL)
		return;

	for (desc = start; desc < stop; desc++) {
		if (is_enabled(desc))
			desc->flags |= BTD_DEBUG_FLAG_PRINT;
	}
}

void __btd_toggle_debug(void)
{
	struct btd_debug_desc *desc;

	for (desc = __start___debug; desc < __stop___debug; desc++)
		desc->flags |= BTD_DEBUG_FLAG_PRINT;
}

void __btd_log_init(const char *debug, int detach)
{
	int option = LOG_NDELAY | LOG_PID;

	if (debug != NULL)
		enabled = g_strsplit_set(debug, ":, ", 0);

	__btd_enable_debug(__start___debug, __stop___debug);

	bt_log_open();

	if (!detach)
		option |= LOG_PERROR;

	openlog(LOG_IDENT, option, LOG_DAEMON);

	info("Bluetooth daemon %s", VERSION);
}

void __btd_log_cleanup(void)
{
	closelog();

	bt_log_close();

	g_strfreev(enabled);
}

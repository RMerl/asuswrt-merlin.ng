/*
 *
 *  BlueZ - Bluetooth protocol stack for Linux
 *
 *  Copyright (C) 2004-2010  Marcel Holtmann <marcel@holtmann.org>
 *
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
 *
 */

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

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
#include "log.h"

#define LOG_IDENT "bluetoothd"
#define LOG_IDENT_LEN sizeof(LOG_IDENT)

struct log_hdr {
	uint16_t opcode;
	uint16_t index;
	uint16_t len;
	uint8_t  priority;
	uint8_t  ident_len;
} __attribute__((packed));

static int logging_fd = -1;

static void logging_open(void)
{
	struct sockaddr_hci addr;
	int fd;

	if (logging_fd >= 0)
		return;

	fd = socket(PF_BLUETOOTH, SOCK_RAW, BTPROTO_HCI);
	if (fd < 0)
		return;

	memset(&addr, 0, sizeof(addr));
	addr.hci_family = AF_BLUETOOTH;
	addr.hci_dev = HCI_DEV_NONE;
	addr.hci_channel = HCI_CHANNEL_LOGGING;

	if (bind(fd, (struct sockaddr *) &addr, sizeof(addr)) < 0) {
		close(fd);
		return;
	}

	logging_fd = fd;
}

static void logging_close(void)
{
	if (logging_fd >= 0) {
		close(logging_fd);
		logging_fd = -1;
	}
}

static void logging_log(uint16_t index, int priority,
					const char *format, va_list ap)
{
	struct log_hdr hdr;
	struct msghdr msg;
	struct iovec iov[3];
	uint16_t len;
	char *str;

	if (vasprintf(&str, format, ap) < 0)
		return;

	len = strlen(str) + 1;

	hdr.opcode = cpu_to_le16(0x0000);
	hdr.index = cpu_to_le16(index);
	hdr.len = cpu_to_le16(2 + LOG_IDENT_LEN + len);
	hdr.priority = priority;
	hdr.ident_len = LOG_IDENT_LEN;

	iov[0].iov_base = &hdr;
	iov[0].iov_len = sizeof(hdr);

	iov[1].iov_base = LOG_IDENT;
	iov[1].iov_len = LOG_IDENT_LEN;

	iov[2].iov_base = str;
	iov[2].iov_len = len;

	memset(&msg, 0, sizeof(msg));
	msg.msg_iov = iov;
	msg.msg_iovlen = 3;

	if (sendmsg(logging_fd, &msg, 0) < 0) {
		if (errno != ENODEV) {
			close(logging_fd);
			logging_fd = -1;
		}
	}

	free(str);
}

void error(const char *format, ...)
{
	va_list ap;

	va_start(ap, format);
	vsyslog(LOG_ERR, format, ap);
	va_end(ap);

	if (logging_fd < 0)
		return;

	va_start(ap, format);
	logging_log(HCI_DEV_NONE, LOG_ERR, format, ap);
	va_end(ap);
}

void warn(const char *format, ...)
{
	va_list ap;

	va_start(ap, format);
	vsyslog(LOG_WARNING, format, ap);
	va_end(ap);

	if (logging_fd < 0)
		return;

	va_start(ap, format);
	logging_log(HCI_DEV_NONE, LOG_WARNING, format, ap);
	va_end(ap);
}

void info(const char *format, ...)
{
	va_list ap;

	va_start(ap, format);
	vsyslog(LOG_INFO, format, ap);
	va_end(ap);

	if (logging_fd < 0)
		return;

	va_start(ap, format);
	logging_log(HCI_DEV_NONE, LOG_INFO, format, ap);
	va_end(ap);
}

void btd_log(uint16_t index, int priority, const char *format, ...)
{
	va_list ap;

	va_start(ap, format);
	vsyslog(priority, format, ap);
	va_end(ap);

	if (logging_fd < 0)
		return;

	va_start(ap, format);
	logging_log(index, priority, format, ap);
	va_end(ap);
}

void btd_error(uint16_t index, const char *format, ...)
{
	va_list ap;

	va_start(ap, format);
	vsyslog(LOG_ERR, format, ap);
	va_end(ap);

	if (logging_fd < 0)
		return;

	va_start(ap, format);
	logging_log(index, LOG_ERR, format, ap);
	va_end(ap);
}

void btd_warn(uint16_t index, const char *format, ...)
{
	va_list ap;

	va_start(ap, format);
	vsyslog(LOG_WARNING, format, ap);
	va_end(ap);

	if (logging_fd < 0)
		return;

	va_start(ap, format);
	logging_log(index, LOG_WARNING, format, ap);
	va_end(ap);
}

void btd_info(uint16_t index, const char *format, ...)
{
	va_list ap;

	va_start(ap, format);
	vsyslog(LOG_INFO, format, ap);
	va_end(ap);

	if (logging_fd < 0)
		return;

	va_start(ap, format);
	logging_log(index, LOG_INFO, format, ap);
	va_end(ap);
}

void btd_debug(uint16_t index, const char *format, ...)
{
	va_list ap;

	va_start(ap, format);
	vsyslog(LOG_DEBUG, format, ap);
	va_end(ap);

	if (logging_fd < 0)
		return;

	va_start(ap, format);
	logging_log(index, LOG_DEBUG, format, ap);
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

	logging_open();

	if (!detach)
		option |= LOG_PERROR;

	openlog(LOG_IDENT, option, LOG_DAEMON);

	info("Bluetooth daemon %s", VERSION);
}

void __btd_log_cleanup(void)
{
	closelog();

	logging_close();

	g_strfreev(enabled);
}

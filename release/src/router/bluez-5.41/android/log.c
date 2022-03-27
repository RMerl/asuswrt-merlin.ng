/*
 *
 *  BlueZ - Bluetooth protocol stack for Linux
 *
 *  Copyright (C) 2014  Intel Corporation. All rights reserved.
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

#include <fcntl.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <unistd.h>
#include <stdbool.h>
#include <time.h>
#include <sys/uio.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/un.h>

#include "src/log.h"

#define LOG_TAG "bluetoothd"

#define LOG_DEBUG 3
#define LOG_INFO 4
#define LOG_WARN 5
#define LOG_ERR 6

#define LOG_ID_SYSTEM 3

struct logd_header {
	uint8_t id;
	uint16_t pid; /* Android logd expects only 2 bytes for PID */
	uint32_t sec;
	uint32_t nsec;
} __attribute__ ((packed));

static int log_fd = -1;
static bool legacy_log = false;

static void android_log(unsigned char level, const char *fmt, va_list ap)
{
	struct logd_header header;
	struct iovec vec[4];
	int cnt = 0;
	char *msg;
	static pid_t pid = 0;

	if (log_fd < 0)
		return;

	/* no need to call getpid all the time since we don't fork */
	if (!pid)
		pid = getpid();

	if (vasprintf(&msg, fmt, ap) < 0)
		return;

	if (!legacy_log) {
		struct timespec ts;

		clock_gettime(CLOCK_REALTIME, &ts);

		header.id = LOG_ID_SYSTEM;
		header.pid = pid;
		header.sec = ts.tv_sec;
		header.nsec = ts.tv_nsec;

		vec[0].iov_base = &header;
		vec[0].iov_len = sizeof(header);

		cnt += 1;
	}

	vec[cnt + 0].iov_base = &level;
	vec[cnt + 0].iov_len = sizeof(level);
	vec[cnt + 1].iov_base = LOG_TAG;
	vec[cnt + 1].iov_len = sizeof(LOG_TAG);
	vec[cnt + 2].iov_base  = msg;
	vec[cnt + 2].iov_len  = strlen(msg) + 1;

	cnt += 3;

	writev(log_fd, vec, cnt);

	free(msg);
}

void info(const char *format, ...)
{
	va_list ap;

	va_start(ap, format);

	android_log(LOG_INFO, format, ap);

	va_end(ap);
}

void warn(const char *format, ...)
{
	va_list ap;

	va_start(ap, format);

	android_log(LOG_WARN, format, ap);

	va_end(ap);
}

void error(const char *format, ...)
{
	va_list ap;

	va_start(ap, format);

	android_log(LOG_ERR, format, ap);

	va_end(ap);
}

void btd_debug(uint16_t index, const char *format, ...)
{
	va_list ap;

	va_start(ap, format);

	android_log(LOG_DEBUG, format, ap);

	va_end(ap);
}

static bool init_legacy_log(void)
{
	log_fd = open("/dev/log/system", O_WRONLY);
	if (log_fd < 0)
		return false;

	legacy_log = true;

	return true;
}

static bool init_logd(void)
{
	struct sockaddr_un addr;

	log_fd = socket(PF_UNIX, SOCK_DGRAM | SOCK_CLOEXEC, 0);
	if (log_fd < 0)
		return false;

	if (fcntl(log_fd, F_SETFL, O_NONBLOCK) < 0)
		goto failed;

	memset(&addr, 0, sizeof(addr));
	addr.sun_family = AF_UNIX;
	strcpy(addr.sun_path, "/dev/socket/logdw");

	if (connect(log_fd, (struct sockaddr *)&addr, sizeof(addr)) < 0)
		goto failed;

	return true;

failed:
	close(log_fd);
	log_fd = -1;

	return false;
}

extern struct btd_debug_desc __start___debug[];
extern struct btd_debug_desc __stop___debug[];

void __btd_log_init(const char *debug, int detach)
{
	if (!init_logd() && !init_legacy_log())
		return;

	if (debug) {
		struct btd_debug_desc *desc;

		for (desc = __start___debug; desc < __stop___debug; desc++)
			desc->flags |= BTD_DEBUG_FLAG_PRINT;
	}

	info("Bluetooth daemon %s", VERSION);
}

void __btd_log_cleanup(void)
{
	if (log_fd < 0)
		return;

	close(log_fd);
	log_fd = -1;
}

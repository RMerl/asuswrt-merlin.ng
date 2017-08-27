/*
 * ulog - simple logging functions
 *
 * Copyright (C) 2015 Jo-Philipp Wich <jow@openwrt.org>
 *
 * Permission to use, copy, modify, and/or distribute this software for any
 * purpose with or without fee is hereby granted, provided that the above
 * copyright notice and this permission notice appear in all copies.
 *
 * THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES
 * WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF
 * MERCHANTABILITY AND FITNESS. IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR
 * ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES
 * WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN AN
 * ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF
 * OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.
 */

#include "ulog.h"

#include <stdio.h>
#include <stdarg.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>

static int _ulog_channels = -1;
static int _ulog_facility = -1;
static int _ulog_threshold = LOG_DEBUG;
static int _ulog_initialized = 0;
static const char *_ulog_ident = NULL;

static const char *ulog_default_ident(void)
{
	FILE *self;
	static char line[64];
	char *p = NULL;
	char *sbuf;

	if ((self = fopen("/proc/self/status", "r")) != NULL) {
		while (fgets(line, sizeof(line), self)) {
			if (!strncmp(line, "Name:", 5)) {
				strtok_r(line, "\t\n", &sbuf);
				p = strtok_r(NULL, "\t\n", &sbuf);
				break;
			}
		}
		fclose(self);
	}

	return p;
}

static void ulog_defaults(void)
{
	char *env;

	if (_ulog_initialized)
		return;

	env = getenv("PREINIT");

	if (_ulog_channels < 0) {
		if (env && !strcmp(env, "1"))
			_ulog_channels = ULOG_KMSG;
		else if (isatty(1))
			_ulog_channels = ULOG_STDIO;
		else
			_ulog_channels = ULOG_SYSLOG;
	}

	if (_ulog_facility < 0) {
		if (env && !strcmp(env, "1"))
			_ulog_facility = LOG_DAEMON;
		else if (isatty(1))
			_ulog_facility = LOG_USER;
		else
			_ulog_facility = LOG_DAEMON;
	}

	if (_ulog_ident == NULL && _ulog_channels != ULOG_STDIO)
		_ulog_ident = ulog_default_ident();

	if (_ulog_channels & ULOG_SYSLOG)
		openlog(_ulog_ident, 0, _ulog_facility);

	_ulog_initialized = 1;
}

static void ulog_kmsg(int priority, const char *fmt, va_list ap)
{
	FILE *kmsg;

	if ((kmsg = fopen("/dev/kmsg", "r+")) != NULL) {
		fprintf(kmsg, "<%u>", priority);

		if (_ulog_ident)
			fprintf(kmsg, "%s: ", _ulog_ident);

		vfprintf(kmsg, fmt, ap);
		fclose(kmsg);
	}
}

static void ulog_stdio(int priority, const char *fmt, va_list ap)
{
	FILE *out = stderr;

	if (_ulog_ident)
		fprintf(out, "%s: ", _ulog_ident);

	vfprintf(out, fmt, ap);
}

static void ulog_syslog(int priority, const char *fmt, va_list ap)
{
	vsyslog(priority, fmt, ap);
}

void ulog_open(int channels, int facility, const char *ident)
{
	ulog_close();

	_ulog_channels = channels;
	_ulog_facility = facility;
	_ulog_ident = ident;
}

void ulog_close(void)
{
	if (!_ulog_initialized)
		return;

	if (_ulog_channels & ULOG_SYSLOG)
		closelog();

	_ulog_initialized = 0;
}

void ulog_threshold(int threshold)
{
	_ulog_threshold = threshold;
}

void ulog(int priority, const char *fmt, ...)
{
	va_list ap;

	if (priority > _ulog_threshold)
		return;

	ulog_defaults();

	if (_ulog_channels & ULOG_KMSG)
	{
		va_start(ap, fmt);
		ulog_kmsg(priority, fmt, ap);
		va_end(ap);
	}

	if (_ulog_channels & ULOG_STDIO)
	{
		va_start(ap, fmt);
		ulog_stdio(priority, fmt, ap);
		va_end(ap);
	}

	if (_ulog_channels & ULOG_SYSLOG)
	{
		va_start(ap, fmt);
		ulog_syslog(priority, fmt, ap);
		va_end(ap);
	}
}

/*
	Copyright (C) Slava Astashonok <sla@0n.ru>

	This program is free software; you can redistribute it and/or
	modify it under the terms of the GNU General Public License.

	$Id: my_log.c,v 1.3.2.2 2004/02/02 08:06:24 sla Exp $
*/

#include <common.h>

#include <syslog.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>

#include <my_log.h>

static char *my_log_indent;
static unsigned my_log_min_level;
static unsigned my_log_flags;
static char *my_log_names[] = {
	"EMERG", "ALERT", "CRIT", "ERR", "WARNING", "NOTICE", "INFO", "DEBUG"
};

void my_log_open(char *indent, unsigned min_level, unsigned flags)
{
	my_log_indent = indent;
	my_log_min_level = min_level;
	my_log_flags = flags;
	//openlog(0, LOG_PID, MY_LOG_SYSLOG_FACILITY);
	openlog(my_log_indent, 0, MY_LOG_SYSLOG_FACILITY);
}

void my_log_close(void)
{
	closelog();
}

void my_log(unsigned level, const char *format, ...)
{
	va_list args;
	char msg[256];
	char msg_prefix[64];

	if (level <= my_log_min_level) {
		va_start(args, format);
		vsnprintf(msg, sizeof(msg), format, args);
		snprintf(msg_prefix, sizeof(msg_prefix), "[%s]: ", my_log_names[level]);

		if (my_log_flags & MY_LOG_SYSLOG)
			syslog(level, "%s%s", msg_prefix, msg);

		if (my_log_flags & MY_LOG_STDOUT)
			fprintf(stdout, "%s%s\n", msg_prefix, msg);
	}
}

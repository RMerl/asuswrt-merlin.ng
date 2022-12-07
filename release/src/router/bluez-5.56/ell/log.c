/*
 *
 *  Embedded Linux library
 *
 *  Copyright (C) 2011-2014  Intel Corporation. All rights reserved.
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Lesser General Public
 *  License as published by the Free Software Foundation; either
 *  version 2.1 of the License, or (at your option) any later version.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public
 *  License along with this library; if not, write to the Free Software
 *  Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
 *
 */

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#define _GNU_SOURCE
#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <stdarg.h>
#include <string.h>
#include <fnmatch.h>
#include <sys/socket.h>
#include <sys/un.h>

#include "queue.h"
#include "log.h"
#include "private.h"

struct debug_section {
	struct l_debug_desc *start;
	struct l_debug_desc *end;
};

struct l_queue *debug_sections;

/**
 * SECTION:log
 * @short_description: Logging framework
 *
 * Logging framework
 */

/**
 * l_debug_desc:
 *
 * Debug descriptor.
 */

static void log_null(int priority, const char *file, const char *line,
			const char *func, const char *format, va_list ap)
{
}

static l_log_func_t log_func = log_null;
static const char *log_ident = "";
static int log_fd = -1;
static unsigned long log_pid;

static inline void close_log(void)
{
	if (log_fd > 0) {
		close(log_fd);
		log_fd = -1;
	}
}

static int open_log(const char *path)
{
	struct sockaddr_un addr;

	log_fd = socket(PF_UNIX, SOCK_DGRAM | SOCK_CLOEXEC, 0);
	if (log_fd < 0)
		return -1;

	memset(&addr, 0, sizeof(addr));
	addr.sun_family = AF_UNIX;
	strncpy(addr.sun_path, path, sizeof(addr.sun_path) - 1);

	if (connect(log_fd, (struct sockaddr *) &addr, sizeof(addr)) < 0) {
		close_log();
		return -1;
	}

	return 0;
}

/**
 * l_log_set_ident:
 * @ident: string identifier
 *
 * Sets the log identifier string.
 **/
LIB_EXPORT void l_log_set_ident(const char *ident)
{
	log_ident = ident;
}

/**
 * l_log_set_handler:
 * @function: log handler function
 *
 * Sets the log handler function.
 **/
LIB_EXPORT void l_log_set_handler(l_log_func_t function)
{
	L_DEBUG_SYMBOL(__debug_intern, "");

	close_log();

	if (!function) {
		log_func = log_null;
		return;
	}

	log_func = function;
}

/**
 * l_log_set_null:
 *
 * Disable logging.
 **/
LIB_EXPORT void l_log_set_null(void)
{
	close_log();

	log_func = log_null;
}

__attribute__((format(printf, 5, 0)))
static void log_stderr(int priority, const char *file, const char *line,
			const char *func, const char *format, va_list ap)
{
	vfprintf(stderr, format, ap);
}

/**
 * l_log_set_stderr:
 *
 * Enable logging to stderr.
 **/
LIB_EXPORT void l_log_set_stderr(void)
{
	close_log();

	log_func = log_stderr;
}

__attribute__((format(printf, 5, 0)))
static void log_syslog(int priority, const char *file, const char *line,
			const char *func, const char *format, va_list ap)
{
	struct msghdr msg;
	struct iovec iov[2];
	char hdr[64], *str;
	int hdr_len, str_len;

	str_len = vasprintf(&str, format, ap);
	if (str_len < 0)
		return;

	hdr_len = snprintf(hdr, sizeof(hdr), "<%i>%s[%lu]: ", priority,
					log_ident, (unsigned long) log_pid);

	iov[0].iov_base = hdr;
	iov[0].iov_len  = hdr_len;
	iov[1].iov_base = str;
	iov[1].iov_len  = str_len;

	memset(&msg, 0, sizeof(msg));
	msg.msg_iov = iov;
	msg.msg_iovlen = 2;

	sendmsg(log_fd, &msg, 0);

	free(str);
}

/**
 * l_log_set_syslog:
 *
 * Enable logging to syslog.
 **/
LIB_EXPORT void l_log_set_syslog(void)
{
	close_log();

	if (open_log("/dev/log") < 0) {
		log_func = log_null;
		return;
	}

	log_pid = getpid();

	log_func = log_syslog;
}

__attribute__((format(printf, 5, 0)))
static void log_journal(int priority, const char *file, const char *line,
			const char *func, const char *format, va_list ap)
{
	struct msghdr msg;
	struct iovec iov[12];
	char prio[16], *str;
	int prio_len, str_len;

	str_len = vasprintf(&str, format, ap);
	if (str_len < 0)
		return;

	prio_len = snprintf(prio, sizeof(prio), "PRIORITY=%u\n", priority);

	iov[0].iov_base = "MESSAGE=";
	iov[0].iov_len  = 8;
	iov[1].iov_base = str;
	iov[1].iov_len  = str_len;
	iov[2].iov_base = prio;
	iov[2].iov_len  = prio_len;
	iov[3].iov_base = "CODE_FILE=";
	iov[3].iov_len  = 10;
	iov[4].iov_base = (char *) file;
	iov[4].iov_len  = strlen(file);
	iov[5].iov_base = "\n";
	iov[5].iov_len  = 1;
	iov[6].iov_base = "CODE_LINE=";
	iov[6].iov_len  = 10;
	iov[7].iov_base = (char *) line;
	iov[7].iov_len  = strlen(line);
	iov[8].iov_base = "\n";
	iov[8].iov_len  = 1;
	iov[9].iov_base = "CODE_FUNC=";
	iov[9].iov_len  = 10;
	iov[10].iov_base = (char *) func;
	iov[10].iov_len  = strlen(func);
	iov[11].iov_base = "\n";
	iov[11].iov_len  = 1;

	memset(&msg, 0, sizeof(msg));
	msg.msg_iov = iov;
	msg.msg_iovlen = 12;

	sendmsg(log_fd, &msg, 0);

	free(str);
}

/**
 * l_log_set_journal:
 *
 * Enable logging to journal.
 **/
LIB_EXPORT void l_log_set_journal(void)
{
	close_log();

	if (open_log("/run/systemd/journal/socket") < 0) {
		log_func = log_null;
		return;
	}

	log_pid = getpid();

	log_func = log_journal;
}

/**
 * l_log_with_location:
 * @priority: priority level
 * @file: source file
 * @line: source line
 * @func: source function
 * @format: format string
 * @...: format arguments
 *
 * Log information.
 **/
LIB_EXPORT void l_log_with_location(int priority,
				const char *file, const char *line,
				const char *func, const char *format, ...)
{
	va_list ap;

	va_start(ap, format);
	log_func(priority, file, line, func, format, ap);
	va_end(ap);
}

/**
 * l_error:
 * @format: format string
 * @...: format arguments
 *
 **/

/**
 * l_warn:
 * @format: format string
 * @...: format arguments
 *
 **/

/**
 * l_info:
 * @format: format string
 * @...: format arguments
 *
 **/

/**
 * l_debug:
 * @format: format string
 * @...: format arguments
 **/

static const char *debug_pattern;

void debug_enable(struct l_debug_desc *start, struct l_debug_desc *stop)
{
	struct l_debug_desc *desc;
	char *pattern_copy;

	if (!debug_pattern)
		return;

	pattern_copy = strdupa(debug_pattern);

	while (pattern_copy) {
		char *str = strsep(&pattern_copy, ":,");
		if (!str)
			break;

		for (desc = start; desc < stop; desc++) {
			if (!fnmatch(str, desc->file, 0))
				desc->flags |= L_DEBUG_FLAG_PRINT;
			if (!fnmatch(str, desc->func, 0))
				desc->flags |= L_DEBUG_FLAG_PRINT;
		}
	}
}

void debug_disable(struct l_debug_desc *start, struct l_debug_desc *stop)
{
	struct l_debug_desc *desc;

	for (desc = start; desc < stop; desc++)
		desc->flags &= ~L_DEBUG_FLAG_PRINT;
}

/**
 * l_debug_add_section:
 * @start: start of the debug section
 * @stop: stop of the debug section
 *
 * Add information about a debug section.  This is used by shared libraries
 * to tell ell about their debug section start & stopping points.  This is used
 * to make l_debug statements work across all shared libraries that might be
 * linked into the executable
 */
LIB_EXPORT void l_debug_add_section(struct l_debug_desc *start,
					struct l_debug_desc *end)
{
	const struct l_queue_entry *entry;
	struct debug_section *new_section;

	if (!debug_sections) {
		debug_sections = l_queue_new();
		goto add;
	}

	for (entry = l_queue_get_entries(debug_sections); entry;
					entry = entry->next) {
		const struct debug_section *section = entry->data;

		if (section->start == start && section->end == end)
			return;
	}

add:
	new_section = l_new(struct debug_section, 1);
	new_section->start = start;
	new_section->end = end;

	l_queue_push_head(debug_sections, new_section);
}

/**
 * l_debug_enable_full:
 * @pattern: debug pattern
 * @start: start of the debug section
 * @stop: end of the debug section
 *
 * Enable debug sections based on @pattern.
 **/
LIB_EXPORT void l_debug_enable_full(const char *pattern,
					struct l_debug_desc *start,
					struct l_debug_desc *end)
{
	const struct l_queue_entry *entry;

	if (!pattern)
		return;

	debug_pattern = pattern;

	l_debug_add_section(start, end);

	for (entry = l_queue_get_entries(debug_sections); entry;
					entry = entry->next) {
		const struct debug_section *section = entry->data;

		debug_enable(section->start, section->end);
	}
}

/**
 * l_debug_disable:
 *
 * Disable all debug sections.
 **/
LIB_EXPORT void l_debug_disable(void)
{
	const struct l_queue_entry *entry;

	for (entry = l_queue_get_entries(debug_sections); entry;
					entry = entry->next) {
		const struct debug_section *section = entry->data;

		debug_disable(section->start, section->end);
	}

	debug_pattern = NULL;
}

__attribute__((constructor)) static void register_debug_section()
{
	extern struct l_debug_desc __start___ell_debug[];
	extern struct l_debug_desc __stop___ell_debug[];

	l_debug_add_section(__start___ell_debug, __stop___ell_debug);
}

__attribute__((destructor(65535))) static void free_debug_sections()
{
	l_queue_destroy(debug_sections, l_free);
}

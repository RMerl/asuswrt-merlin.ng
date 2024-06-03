/*
 * wpa_supplicant/hostapd / Debug prints
 * Copyright (c) 2002-2013, Jouni Malinen <j@w1.fi>
 *
 * This software may be distributed under the terms of the BSD license.
 * See README for more details.
 */
/*
 * ***************************************************************************
 * *  Mediatek Inc.
 * * 4F, No. 2 Technology 5th Rd.
 * * Science-based Industrial Park
 * * Hsin-chu, Taiwan, R.O.C.
 * *
 * * (c) Copyright 2002-2011, Mediatek, Inc.
 * *
 * * All rights reserved. Mediatek's source code is an unpublished work and the
 * * use of a copyright notice does not imply otherwise. This source code
 * * contains confidential trade secret material of Ralink Tech. Any attemp
 * * or participation in deciphering, decoding, reverse engineering or in any
 * * way altering the source code is stricitly prohibited, unless the prior
 * * written consent of Mediatek, Inc. is obtained.
 * ***************************************************************************
 *
 *  Module Name:
 *  Debug Prints
 *
 *  Abstract:
 *  Debug Prints
 *
 *  Revision History:
 *  Who         When          What
 *  --------    ----------    -----------------------------------------
 *  Neelansh.M   2018/05/02     Derived from wpa_supplicant/hostpad / Debug Prints
 * */

#include "includes.h"
#include "common.h"

#ifdef CONFIG_DEBUG_SYSLOG
#include <syslog.h>

int mapd_debug_syslog = 0;
#endif /* CONFIG_DEBUG_SYSLOG */

int mapd_debug_level = MSG_ERROR;
int mapd_debug_timestamp = 0;

#ifndef CONFIG_NO_STDOUT_DEBUG

#ifdef CONFIG_DEBUG_FILE
static FILE *out_file = NULL;
#endif /* CONFIG_DEBUG_FILE */


void mapd_debug_print_timestamp(void)
{
	struct os_time tv;

	if (!mapd_debug_timestamp)
		return;

	os_get_time(&tv);
#ifdef CONFIG_DEBUG_FILE
	if (out_file) {
		fprintf(out_file, "%ld.%06u: ", (long) tv.sec,
			(unsigned int) tv.usec);
	} else
#endif /* CONFIG_DEBUG_FILE */
	printf("%ld.%06u: ", (long) tv.sec, (unsigned int) tv.usec);
}


#ifdef CONFIG_DEBUG_SYSLOG
#ifndef LOG_HOSTAPD
#define LOG_HOSTAPD LOG_DAEMON
#endif /* LOG_HOSTAPD */

void mapd_debug_open_syslog(void)
{
	openlog("mapd", LOG_PID | LOG_NDELAY, LOG_HOSTAPD);
	mapd_debug_syslog++;
}


void mapd_debug_close_syslog(void)
{
	if (mapd_debug_syslog)
		closelog();
}


static int syslog_priority(int level)
{
	switch (level) {
	case MSG_MSGDUMP:
	case MSG_DEBUG:
		return LOG_DEBUG;
	case MSG_INFO:
		return LOG_NOTICE;
	case MSG_WARNING:
		return LOG_WARNING;
	case MSG_ERROR:
		return LOG_ERR;
	}
	return LOG_INFO;
}
#endif /* CONFIG_DEBUG_SYSLOG */


#ifdef CONFIG_DEBUG_LINUX_TRACING

int mapd_debug_open_linux_tracing(void)
{
	int mounts, trace_fd;
	char buf[4096] = {};
	ssize_t buflen;
	char *line, *tmp1, *path = NULL;

	mounts = open("/proc/mounts", O_RDONLY);
	if (mounts < 0) {
		printf("no /proc/mounts\n");
		return -1;
	}

	buflen = read(mounts, buf, sizeof(buf) - 1);
	close(mounts);
	if (buflen < 0) {
		printf("failed to read /proc/mounts\n");
		return -1;
	}

	line = strtok_r(buf, "\n", &tmp1);
	while (line) {
		char *tmp2, *tmp_path, *fstype;
		/* "<dev> <mountpoint> <fs type> ..." */
		strtok_r(line, " ", &tmp2);
		tmp_path = strtok_r(NULL, " ", &tmp2);
		fstype = strtok_r(NULL, " ", &tmp2);
		if (fstype && strcmp(fstype, "debugfs") == 0) {
			path = tmp_path;
			break;
		}

		line = strtok_r(NULL, "\n", &tmp1);
	}

	if (path == NULL) {
		printf("debugfs mountpoint not found\n");
		return -1;
	}

	snprintf(buf, sizeof(buf) - 1, "%s/tracing/trace_marker", path);

	trace_fd = open(buf, O_WRONLY);
	if (trace_fd < 0) {
		printf("failed to open trace_marker file\n");
		return -1;
	}
	mapd_debug_tracing_file = fdopen(trace_fd, "w");
	if (mapd_debug_tracing_file == NULL) {
		close(trace_fd);
		printf("failed to fdopen()\n");
		return -1;
	}

	return 0;
}


void mapd_debug_close_linux_tracing(void)
{
	if (mapd_debug_tracing_file == NULL)
		return;
	fclose(mapd_debug_tracing_file);
	mapd_debug_tracing_file = NULL;
}

#endif /* CONFIG_DEBUG_LINUX_TRACING */


/**
 * mapd_printf - conditional printf
 * @level: priority level (MSG_*) of the message
 * @fmt: printf format string, followed by optional arguments
 *
 * This function is used to print conditional debugging and error messages. The
 * output may be directed to stdout, stderr, and/or syslog based on
 * configuration.
 *
 * Note: New line '\n' is added to the end of the text when printing to stdout.
 */
void  mapd_printf_debug(const char *func, int line, int level, const char *fmt, ...)
{
	va_list ap;

	va_start(ap, fmt);
#ifdef CONFIG_DEBUG_SYSLOG
	if (mapd_debug_syslog) {
		vsyslog(syslog_priority(level), fmt, ap);
	}
#endif /* CONFIG_DEBUG_SYSLOG */
	if (level >= mapd_debug_level) {
		mapd_debug_print_timestamp();
#ifdef CONFIG_DEBUG_FILE
		if (out_file) {
			vfprintf(out_file, fmt, ap);
			fprintf(out_file, "\n");
		} else {
#endif /* CONFIG_DEBUG_FILE */
		printf("[mapd][%s][%d]", func, line);
		vprintf(fmt, ap);
		printf("\n");
#ifdef CONFIG_DEBUG_FILE
		}
#endif /* CONFIG_DEBUG_FILE */
	}
	va_end(ap);

#ifdef CONFIG_DEBUG_LINUX_TRACING
	if (mapd_debug_tracing_file != NULL) {
		va_start(ap, fmt);
		fprintf(mapd_debug_tracing_file, WPAS_TRACE_PFX, level);
		vfprintf(mapd_debug_tracing_file, fmt, ap);
		fprintf(mapd_debug_tracing_file, "\n");
		fflush(mapd_debug_tracing_file);
		va_end(ap);
	}
#endif /* CONFIG_DEBUG_LINUX_TRACING */
}



/**
 * mapd_printf_raw- conditional printf
 * @level: priority level (MSG_*) of the message
 * @fmt: printf format string, followed by optional arguments
 *
 * This function is used to print conditional debugging and error messages. The
 * output may be directed to stdout, stderr, and/or syslog based on
 * configuration.
 *
 * Note: this prints the string without function-name, line number and \n
 */
void  mapd_printf_raw(const char *func, int line,int level, const char *fmt, ...)
{
	va_list ap;

	va_start(ap, fmt);
#ifdef CONFIG_DEBUG_SYSLOG
	if (mapd_debug_syslog) {
		vsyslog(syslog_priority(level), fmt, ap);
	}
#endif /* CONFIG_DEBUG_SYSLOG */
	if (level >= mapd_debug_level) {
		mapd_debug_print_timestamp();
#ifdef CONFIG_DEBUG_FILE
		if (out_file) {
			vfprintf(out_file, fmt, ap);
			fprintf(out_file, "\n");
		} else {
#endif /* CONFIG_DEBUG_FILE */
		vprintf(fmt, ap);
#ifdef CONFIG_DEBUG_FILE
		}
#endif /* CONFIG_DEBUG_FILE */
	}
	va_end(ap);

#ifdef CONFIG_DEBUG_LINUX_TRACING
	if (mapd_debug_tracing_file != NULL) {
		va_start(ap, fmt);
		fprintf(mapd_debug_tracing_file, WPAS_TRACE_PFX, level);
		vfprintf(mapd_debug_tracing_file, fmt, ap);
		fprintf(mapd_debug_tracing_file, "\n");
		fflush(mapd_debug_tracing_file);
		va_end(ap);
	}
#endif /* CONFIG_DEBUG_LINUX_TRACING */
}


static void _mapd_hexdump(int level, const char *title, const u8 *buf,
			 size_t len, int show)
{
	size_t i;

#ifdef CONFIG_DEBUG_LINUX_TRACING
	if (mapd_debug_tracing_file != NULL) {
		fprintf(mapd_debug_tracing_file,
			WPAS_TRACE_PFX "%s - hexdump(len=%lu):",
			level, title, (unsigned long) len);
		if (buf == NULL) {
			fprintf(mapd_debug_tracing_file, " [NULL]\n");
		} else if (!show) {
			fprintf(mapd_debug_tracing_file, " [REMOVED]\n");
		} else {
			for (i = 0; i < len; i++)
				fprintf(mapd_debug_tracing_file,
					" %02x", buf[i]);
		}
		fflush(mapd_debug_tracing_file);
	}
#endif /* CONFIG_DEBUG_LINUX_TRACING */

	if (level < mapd_debug_level)
		return;
#ifdef CONFIG_DEBUG_SYSLOG
	if (mapd_debug_syslog) {
		const char *display;
		char *strbuf = NULL;

		if (buf == NULL) {
			display = " [NULL]";
		} else if (len == 0) {
			display = "";
		} else if (show && len) {
			strbuf = os_malloc(1 + 3 * len);
			if (strbuf == NULL) {
				mapd_printf(MSG_ERROR, "mapd_hexdump: Failed to "
					   "allocate message buffer");
				return;
			}

			for (i = 0; i < len; i++)
				os_snprintf(&strbuf[i * 3], 4, " %02x",
					    buf[i]);

			display = strbuf;
		} else {
			display = " [REMOVED]";
		}

		syslog(syslog_priority(level), "%s - hexdump(len=%lu):%s",
		       title, (unsigned long) len, display);
		bin_clear_free(strbuf, 1 + 3 * len);
		return;
	}
#endif /* CONFIG_DEBUG_SYSLOG */
	mapd_debug_print_timestamp();
#ifdef CONFIG_DEBUG_FILE
	if (out_file) {
		fprintf(out_file, "%s - hexdump(len=%lu):",
			title, (unsigned long) len);
		if (buf == NULL) {
			fprintf(out_file, " [NULL]");
		} else if (show) {
			for (i = 0; i < len; i++)
				fprintf(out_file, " %02x", buf[i]);
		} else {
			fprintf(out_file, " [REMOVED]");
		}
		fprintf(out_file, "\n");
	} else {
#endif /* CONFIG_DEBUG_FILE */
	printf("%s - hexdump(len=%lu):", title, (unsigned long) len);
	if (buf == NULL) {
		printf(" [NULL]");
	} else if (show) {
		for (i = 0; i < len; i++)
			printf(" %02x", buf[i]);
	} else {
		printf(" [REMOVED]");
	}
	printf("\n");
#ifdef CONFIG_DEBUG_FILE
	}
#endif /* CONFIG_DEBUG_FILE */
}

void mapd_hexdump(int level, const char *title, const void *buf, size_t len)
{
	_mapd_hexdump(level, title, buf, len, 1);
}


static void _mapd_hexdump_ascii(int level, const char *title, const void *buf,
			       size_t len, int show)
{
	size_t i, llen;
	const u8 *pos = buf;
	const size_t line_len = 16;

#ifdef CONFIG_DEBUG_LINUX_TRACING
	if (mapd_debug_tracing_file != NULL) {
		fprintf(mapd_debug_tracing_file,
			WPAS_TRACE_PFX "%s - hexdump_ascii(len=%lu):",
			level, title, (unsigned long) len);
		if (buf == NULL) {
			fprintf(mapd_debug_tracing_file, " [NULL]\n");
		} else if (!show) {
			fprintf(mapd_debug_tracing_file, " [REMOVED]\n");
		} else {
			/* can do ascii processing in userspace */
			for (i = 0; i < len; i++)
				fprintf(mapd_debug_tracing_file,
					" %02x", pos[i]);
		}
		fflush(mapd_debug_tracing_file);
	}
#endif /* CONFIG_DEBUG_LINUX_TRACING */

	if (level < mapd_debug_level)
		return;
	mapd_debug_print_timestamp();
#ifdef CONFIG_DEBUG_FILE
	if (out_file) {
		if (!show) {
			fprintf(out_file,
				"%s - hexdump_ascii(len=%lu): [REMOVED]\n",
				title, (unsigned long) len);
			return;
		}
		if (buf == NULL) {
			fprintf(out_file,
				"%s - hexdump_ascii(len=%lu): [NULL]\n",
				title, (unsigned long) len);
			return;
		}
		fprintf(out_file, "%s - hexdump_ascii(len=%lu):\n",
			title, (unsigned long) len);
		while (len) {
			llen = len > line_len ? line_len : len;
			fprintf(out_file, "    ");
			for (i = 0; i < llen; i++)
				fprintf(out_file, " %02x", pos[i]);
			for (i = llen; i < line_len; i++)
				fprintf(out_file, "   ");
			fprintf(out_file, "   ");
			for (i = 0; i < llen; i++) {
				if (isprint(pos[i]))
					fprintf(out_file, "%c", pos[i]);
				else
					fprintf(out_file, "_");
			}
			for (i = llen; i < line_len; i++)
				fprintf(out_file, " ");
			fprintf(out_file, "\n");
			pos += llen;
			len -= llen;
		}
	} else {
#endif /* CONFIG_DEBUG_FILE */
	if (!show) {
		printf("%s - hexdump_ascii(len=%lu): [REMOVED]\n",
		       title, (unsigned long) len);
		return;
	}
	if (buf == NULL) {
		printf("%s - hexdump_ascii(len=%lu): [NULL]\n",
		       title, (unsigned long) len);
		return;
	}
	printf("%s - hexdump_ascii(len=%lu):\n", title, (unsigned long) len);
	while (len) {
		llen = len > line_len ? line_len : len;
		printf("    ");
		for (i = 0; i < llen; i++)
			printf(" %02x", pos[i]);
		for (i = llen; i < line_len; i++)
			printf("   ");
		printf("   ");
		for (i = 0; i < llen; i++) {
			if (isprint(pos[i]))
				printf("%c", pos[i]);
			else
				printf("_");
		}
		for (i = llen; i < line_len; i++)
			printf(" ");
		printf("\n");
		pos += llen;
		len -= llen;
	}
#ifdef CONFIG_DEBUG_FILE
	}
#endif /* CONFIG_DEBUG_FILE */
}


void mapd_hexdump_ascii(int level, const char *title, const void *buf,
		       size_t len)
{
	_mapd_hexdump_ascii(level, title, buf, len, 1);
}

#ifdef CONFIG_DEBUG_FILE
static char *last_path = NULL;
#endif /* CONFIG_DEBUG_FILE */

int mapd_debug_open_file(const char *path)
{
#ifdef CONFIG_DEBUG_FILE
	if (!path)
		return 0;

	if (last_path == NULL || os_strcmp(last_path, path) != 0) {
		/* Save our path to enable re-open */
		os_free(last_path);
		last_path = os_strdup(path);
	}

	out_file = fopen(path, "a");
	if (out_file == NULL) {
		mapd_printf(MSG_ERROR, "mapd_debug_open_file: Failed to open "
			   "output file, using standard output");
		return -1;
	}
#ifndef _WIN32
	setvbuf(out_file, NULL, _IOLBF, 0);
#endif /* _WIN32 */
#else /* CONFIG_DEBUG_FILE */
	(void)path;
#endif /* CONFIG_DEBUG_FILE */
	return 0;
}


void mapd_debug_close_file(void)
{
#ifdef CONFIG_DEBUG_FILE
	if (!out_file)
		return;
	fclose(out_file);
	out_file = NULL;
	os_free(last_path);
	last_path = NULL;
#endif /* CONFIG_DEBUG_FILE */
}


void mapd_debug_setup_stdout(void)
{
#ifndef _WIN32
	setvbuf(stdout, NULL, _IOLBF, 0);
#endif /* _WIN32 */
}

#endif /* CONFIG_NO_STDOUT_DEBUG */

const char * debug_level_str(int level)
{
	switch (level) {
	case MSG_EXCESSIVE:
		return "EXCESSIVE";
	case MSG_MSGDUMP:
		return "MSGDUMP";
	case MSG_DEBUG:
		return "DEBUG";
	case MSG_INFO:
		return "INFO";
	case MSG_WARNING:
		return "WARNING";
	case MSG_ERROR:
		return "ERROR";
	default:
		return "?";
	}
}


int str_to_debug_level(const char *s)
{
	if (os_strcasecmp(s, "EXCESSIVE") == 0)
		return MSG_EXCESSIVE;
	if (os_strcasecmp(s, "MSGDUMP") == 0)
		return MSG_MSGDUMP;
	if (os_strcasecmp(s, "DEBUG") == 0)
		return MSG_DEBUG;
	if (os_strcasecmp(s, "INFO") == 0)
		return MSG_INFO;
	if (os_strcasecmp(s, "WARNING") == 0)
		return MSG_WARNING;
	if (os_strcasecmp(s, "ERROR") == 0)
		return MSG_ERROR;
	return -1;
}

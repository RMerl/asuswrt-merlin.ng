/*
 * log.c	Logging module.
 *
 * Version:	$Id$
 *
 *   This program is free software; you can redistribute it and/or modify
 *   it under the terms of the GNU General Public License as published by
 *   the Free Software Foundation; either version 2 of the License, or
 *   (at your option) any later version.
 *
 *   This program is distributed in the hope that it will be useful,
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *   GNU General Public License for more details.
 *
 *   You should have received a copy of the GNU General Public License
 *   along with this program; if not, write to the Free Software
 *   Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA 02110-1301, USA
 *
 * Copyright 2001,2006  The FreeRADIUS server project
 * Copyright 2000  Miquel van Smoorenburg <miquels@cistron.nl>
 * Copyright 2000  Alan DeKok <aland@ox.org>
 * Copyright 2001  Chad Miller <cmiller@surfsouth.com>
 */

RCSID("$Id$")

#include <freeradius-devel/radiusd.h>

#ifdef HAVE_SYS_STAT_H
#include <sys/stat.h>
#endif

#ifdef HAVE_SYSLOG_H
#	include <syslog.h>
#endif

/*
 * Logging facility names
 */
static const FR_NAME_NUMBER levels[] = {
	{ ": Debug: ",		L_DBG		},
	{ ": Auth: ",		L_AUTH		},
	{ ": Proxy: ",		L_PROXY		},
	{ ": Info: ",		L_INFO		},
	{ ": Acct: ",		L_ACCT		},
	{ ": Error: ",		L_ERR		},
	{ ": WARNING: ",	L_DBG_WARN	},
	{ ": ERROR: ",		L_DBG_ERR	},
	{ ": WARNING: ",	L_DBG_WARN2	},
	{ ": ERROR: ",		L_DBG_ERR2	},
	{ NULL, 0 }
};

#define VTC_RED		"\x1b[31m"
#define VTC_YELLOW      "\x1b[33m"
#define VTC_BOLD	"\x1b[1m"
#define VTC_RESET	"\x1b[0m"

static const FR_NAME_NUMBER colours[] = {
	{ "",			L_DBG		},
	{ VTC_BOLD,		L_AUTH		},
	{ VTC_BOLD,		L_PROXY		},
	{ VTC_BOLD,		L_INFO		},
	{ VTC_BOLD,		L_ACCT		},
	{ VTC_RED,		L_ERR		},
	{ VTC_BOLD VTC_RED,	L_DBG_ERR	},
	{ VTC_BOLD VTC_YELLOW,	L_DBG_WARN	},
	{ VTC_BOLD VTC_RED,	L_DBG_ERR2	},
	{ VTC_BOLD VTC_YELLOW,	L_DBG_WARN2	},
	{ NULL, 0 }
};

/*
 *	Syslog facility table.
 */
const FR_NAME_NUMBER syslog_str2fac[] = {
#ifdef LOG_KERN
	{ "kern",		LOG_KERN	},
#endif
#ifdef LOG_USER
	{ "user",		LOG_USER	},
#endif
#ifdef LOG_MAIL
	{ "mail",		LOG_MAIL	},
#endif
#ifdef LOG_DAEMON
	{ "daemon",		LOG_DAEMON	},
#endif
#ifdef LOG_AUTH
	{ "auth",		LOG_AUTH	},
#endif
#ifdef LOG_LPR
	{ "lpr",		LOG_LPR		},
#endif
#ifdef LOG_NEWS
	{ "news",		LOG_NEWS	},
#endif
#ifdef LOG_UUCP
	{ "uucp",		LOG_UUCP	},
#endif
#ifdef LOG_CRON
	{ "cron",		LOG_CRON	},
#endif
#ifdef LOG_AUTHPRIV
	{ "authpriv",		LOG_AUTHPRIV	},
#endif
#ifdef LOG_FTP
	{ "ftp",		LOG_FTP		},
#endif
#ifdef LOG_LOCAL0
	{ "local0",		LOG_LOCAL0	},
#endif
#ifdef LOG_LOCAL1
	{ "local1",		LOG_LOCAL1	},
#endif
#ifdef LOG_LOCAL2
	{ "local2",		LOG_LOCAL2	},
#endif
#ifdef LOG_LOCAL3
	{ "local3",		LOG_LOCAL3	},
#endif
#ifdef LOG_LOCAL4
	{ "local4",		LOG_LOCAL4	},
#endif
#ifdef LOG_LOCAL5
	{ "local5",		LOG_LOCAL5	},
#endif
#ifdef LOG_LOCAL6
	{ "local6",		LOG_LOCAL6	},
#endif
#ifdef LOG_LOCAL7
	{ "local7",		LOG_LOCAL7	},
#endif
	{ NULL,			-1		}
};

const FR_NAME_NUMBER log_str2dst[] = {
	{ "null",		L_DST_NULL	},
	{ "files",		L_DST_FILES	},
	{ "syslog",		L_DST_SYSLOG	},
	{ "stdout",		L_DST_STDOUT	},
	{ "stderr",		L_DST_STDERR	},
	{ NULL,			L_DST_NUM_DEST	}
};

int log_dates_utc = 0;


fr_log_t default_log = {
	.colourise = true,
	.fd = STDOUT_FILENO,
	.dest = L_DST_STDOUT,
	.file = NULL,
	.debug_file = NULL,
};

/*
 *	Log the message to the logfile. Include the severity and
 *	a time stamp.
 */
DIAG_OFF(format-nonliteral)
int vradlog(log_type_t type, char const *fmt, va_list ap)
{
	unsigned char *p;
	char buffer[8192];
	char *unsan;
	size_t len;
	int colourise = default_log.colourise;

	/*
	 *	NOT debugging, and trying to log debug messages.
	 *
	 *	Throw the message away.
	 */
	if (!debug_flag && ((type & L_DBG) != 0)) {
		return 0;
	}

	/*
	 *	If we don't want any messages, then
	 *	throw them away.
	 */
	if (default_log.dest == L_DST_NULL) {
		return 0;
	}

	buffer[0] = '\0';
	len = 0;

	if (colourise) {
		len += strlcpy(buffer + len, fr_int2str(colours, type, ""),
			       sizeof(buffer) - len) ;
		if (len == 0) colourise = false;
	}

	/*
	 *	Mark the point where we treat the buffer as unsanitized.
	 */
	unsan = buffer + len;

	/*
	 *	Don't print timestamps to syslog, it does that for us.
	 *	Don't print timestamps for low levels of debugging.
	 *
	 *	Print timestamps for non-debugging, and for high levels
	 *	of debugging.
	 */
	if ((default_log.dest != L_DST_SYSLOG) &&
	    (debug_flag != 1) && (debug_flag != 2)) {
		time_t timeval;

		timeval = time(NULL);
		CTIME_R(&timeval, buffer + len, sizeof(buffer) - len - 1);

		len = strlen(buffer);

		len += strlcpy(buffer + len,
			       fr_int2str(levels, type, ": "),
			       sizeof(buffer) - len);
	}

	switch (type) {
	case L_DBG_WARN:
		len += strlcpy(buffer + len, "WARNING: ", sizeof(buffer) - len);
		break;

	case L_DBG_ERR:
		len += strlcpy(buffer + len, "ERROR: ", sizeof(buffer) - len);
		break;

	default:
		break;
	}

	if (len < sizeof(buffer)) {
		len += vsnprintf(buffer + len, sizeof(buffer) - len - 1, fmt, ap);
	}

	/*
	 *	Filter out characters not in Latin-1.
	 */
	for (p = (unsigned char *)unsan; *p != '\0'; p++) {
		if (*p == '\r' || *p == '\n')
			*p = ' ';
		else if (*p == '\t') continue;
		else if (*p < 32 || (*p >= 128 && *p <= 160))
			*p = '?';
	}

	if (colourise && (len < sizeof(buffer))) {
		len += strlcpy(buffer + len, VTC_RESET, sizeof(buffer) - len);
	}

	if (len < (sizeof(buffer) - 2)) {
		buffer[len]	= '\n';
		buffer[len + 1] = '\0';
	} else {
		buffer[sizeof(buffer) - 2] = '\n';
		buffer[sizeof(buffer) - 1] = '\0';
	}

	switch (default_log.dest) {

#ifdef HAVE_SYSLOG_H
	case L_DST_SYSLOG:
		switch(type) {
			case L_DBG:
			case L_WARN:
			case L_DBG_WARN:
			case L_DBG_WARN2:
			case L_DBG_ERR:
			case L_DBG_ERR2:
				type = LOG_DEBUG;
				break;
			case L_AUTH:
			case L_PROXY:
			case L_ACCT:
				type = LOG_NOTICE;
				break;
			case L_INFO:
				type = LOG_INFO;
				break;
			case L_ERR:
				type = LOG_ERR;
				break;
		}
		syslog(type, "%s", buffer);
		break;
#endif

	case L_DST_FILES:
	case L_DST_STDOUT:
	case L_DST_STDERR:
		return write(default_log.fd, buffer, strlen(buffer));

	default:
	case L_DST_NULL:	/* should have been caught above */
		break;
	}

	return 0;
}
DIAG_ON(format-nonliteral)

int radlog(log_type_t type, char const *msg, ...)
{
	va_list ap;
	int r;

	va_start(ap, msg);
	r = vradlog(type, msg, ap);
	va_end(ap);

	return r;
}

/*
 *      Dump a whole list of attributes to DEBUG2
 */
void vp_listdebug(VALUE_PAIR *vp)
{
	vp_cursor_t cursor;
	char tmpPair[70];
	for (vp = paircursor(&cursor, &vp);
	     vp;
	     vp = pairnext(&cursor)) {
		vp_prints(tmpPair, sizeof(tmpPair), vp);
		DEBUG2("     %s", tmpPair);
	}
}

inline bool radlog_debug_enabled(log_type_t type, log_debug_t lvl, REQUEST *request)
{
	/*
	 *	It's a debug class message, not this doesn't mean it's a debug type message.
	 *
	 *	For example it could be a RIDEBUG message, which would be an informational message,
	 *	instead of an RDEBUG message which would be a debug debug message.
	 *
	 *	There is log function, but the request debug level isn't high enough.
	 *	OR, we're in debug mode, and the global debug level isn't high enough,
	 *	then don't log the message.
	 */
	if ((type & L_DBG) &&
	    ((request && request->radlog && (lvl > request->options)) ||
	     ((debug_flag != 0) && (lvl > debug_flag)))) {
	 	return false;
	}

	return true;
}

void radlog_request(log_type_t type, log_debug_t lvl, REQUEST *request, char const *msg, ...)
{
	size_t len = 0;
	char const *filename = default_log.file;
	FILE *fp = NULL;
	va_list ap;
	char buffer[8192];
	char *p;
	char const *extra = "";

	va_start(ap, msg);

	/*
	 *	Debug messages get treated specially.
	 */
	if ((type & L_DBG) != 0) {

		if (!radlog_debug_enabled(type, lvl, request)) {
			va_end(ap);
			return;
		}

		/*
		 *	Use the debug output file, if specified,
		 *	otherwise leave it as the default log file.
		 */
#ifdef WITH_COMMAND_SOCKET
		filename = default_log.debug_file;
		if (!filename)
#endif

		filename = default_log.file;
	}

	if (request && filename) {
		radlog_func_t rl = request->radlog;

		request->radlog = NULL;

		/*
		 *	This is SLOW!  Doing it for every log message
		 *	in every request is NOT recommended!
		 */

		 /* FIXME: escape chars! */
		if (radius_xlat(buffer, sizeof(buffer), request, filename, NULL, NULL) < 0) {
			va_end(ap);
			return;
		}
		request->radlog = rl;

		p = strrchr(buffer, FR_DIR_SEP);
		if (p) {
			*p = '\0';
			if (rad_mkdir(buffer, S_IRWXU) < 0) {
				ERROR("Failed creating %s: %s", buffer, strerror(errno));
				va_end(ap);
				return;
			}
			*p = FR_DIR_SEP;
		}

		fp = fopen(buffer, "a");
	}

	/*
	 *	Print timestamps to the file.
	 */
	if (fp) {
		time_t timeval;
		timeval = time(NULL);

#ifdef HAVE_GMTIME_R
		if (log_dates_utc) {
			struct tm utc;
			gmtime_r(&timeval, &utc);
			ASCTIME_R(&utc, buffer, sizeof(buffer) - 1);
		} else
#endif
		{
			CTIME_R(&timeval, buffer, sizeof(buffer) - 1);
		}
		len = strlen(buffer);
		p = strrchr(buffer, '\n');
		if (p) {
			p[0] = ' ';
			p[1] = '\0';
		}

		len += strlcpy(buffer + len,
			       fr_int2str(levels, type, ": "),
		 	       sizeof(buffer) - len);

		if (len >= sizeof(buffer)) goto finish;
	}

	if (request && request->module[0]) {
		len = snprintf(buffer + len, sizeof(buffer) - len, "%s : ",
			       request->module);

		if (len >= sizeof(buffer)) goto finish;
	}

	vsnprintf(buffer + len, sizeof(buffer) - len, msg, ap);

	finish:
	switch (type) {
	case L_DBG_WARN:
		extra = "WARNING: ";
		type = L_DBG_WARN2;
		break;

	case L_DBG_ERR:
		extra = "ERROR: ";
		type = L_DBG_ERR2;
		break;
	default:
		break;
	}

	if (!fp) {
		if (request) {
			radlog(type, "(%u) %s%s", request->number, extra, buffer);
		} else {
			radlog(type, "%s%s", extra, buffer);
		}
	} else {
		if (request) fprintf(fp, "(%u) ", request->number);
		fputs(buffer, fp);
		fputc('\n', fp);
		fclose(fp);
	}

	va_end(ap);
}

void log_talloc(char const *msg)
{
	INFO("%s", msg);
}

void log_talloc_report(TALLOC_CTX *ctx)
{
	FILE *fd;
	char const *null_ctx = NULL;
	int i = 0;

	if (ctx) {
		null_ctx = talloc_get_name(NULL);
	}

	fd = fdopen(default_log.fd, "w");
	if (!fd) {
		ERROR("Couldn't write memory report, fdopen failed: %s", strerror(errno));

		return;
	}

	if (!ctx) {
		talloc_report_full(NULL, fd);
	} else {
		do {
			INFO("Context level %i", i++);

			talloc_report_full(ctx, fd);
		} while ((ctx = talloc_parent(ctx)) && (talloc_get_name(ctx) != null_ctx));  /* Stop before we hit NULL ctx */
	}

	fclose(fd);
}


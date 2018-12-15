/* imkmsg driver for Linux /dev/kmsg structured logging
 *
 * This contains Linux-specific functionality to read /dev/kmsg
 * For a general overview, see head comment in imkmsg.c.
 * This is heavily based on imklog bsd.c file.
 *
 * Copyright 2008-2014 Adiscon GmbH
 *
 * This file is part of rsyslog.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *       http://www.apache.org/licenses/LICENSE-2.0
 *       -or-
 *       see COPYING.ASL20 in the source distribution
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */
#ifdef HAVE_CONFIG_H
#	include "config.h"
#endif
#include <stdlib.h>
#include <time.h>
#include <unistd.h>
#include <fcntl.h>
#include <errno.h>
#include <string.h>
#include <ctype.h>
#include <sys/klog.h>
#include <sys/sysinfo.h>
#include <sys/time.h>
#include <json.h>

#include "rsyslog.h"
#include "srUtils.h"
#include "debug.h"
#include "imkmsg.h"

/* globals */
static int	fklog = -1;	/* kernel log fd */

#ifndef _PATH_KLOG
#	define _PATH_KLOG "/dev/kmsg"
#endif

/* submit a message to imkmsg Syslog() API. In this function, we parse
 * necessary information from kernel log line, and make json string
 * from the rest.
 */
static void
submitSyslog(uchar *buf)
{
	long offs = 0;
	struct timeval tv;
	struct sysinfo info;
	unsigned long int timestamp = 0;
	char name[1024];
	char value[1024];
	char msg[1024];
	syslog_pri_t priority = 0;
	long int sequnum = 0;
	struct json_object *json = NULL, *jval;

	/* create new json object */
	json = json_object_new_object();

	/* get priority */
	for (; isdigit(*buf); buf++) {
		priority = (priority * 10) + (*buf - '0');
	}
	buf++;

	/* get messages sequence number and add it to json */
	for (; isdigit(*buf); buf++) {
		sequnum = (sequnum * 10) + (*buf - '0');
	}
	buf++; /* skip , */
	jval = json_object_new_int(sequnum);
	json_object_object_add(json, "sequnum", jval);

	/* get timestamp */
	for (; isdigit(*buf); buf++) {
		timestamp = (timestamp * 10) + (*buf - '0');
	}

	while (*buf != ';') {
		buf++; /* skip everything till the first ; */
	}
	buf++; /* skip ; */

	/* get message */
	offs = 0;
	for (; *buf != '\n' && *buf != '\0'; buf++, offs++) {
		msg[offs] = *buf;
	}
	msg[offs] = '\0';
	jval = json_object_new_string((char*)msg);
	json_object_object_add(json, "msg", jval);

	if (*buf != '\0') /* message has appended properties, skip \n */
		buf++;

	while (*buf) {
		/* get name of the property */
		buf++; /* skip ' ' */
		offs = 0;
		for (; *buf != '=' && *buf != ' '; buf++, offs++) {
			name[offs] = *buf;
		}
		name[offs] = '\0';
		buf++; /* skip = or ' ' */;

		offs = 0;
		for (; *buf != '\n' && *buf != '\0'; buf++, offs++) {
			value[offs] = *buf;
		}
		value[offs] = '\0';
		if (*buf != '\0') {
			buf++; /* another property, skip \n */
		}

		jval = json_object_new_string((char*)value);
		json_object_object_add(json, name, jval);
	}

	/* calculate timestamp */
	sysinfo(&info);
	gettimeofday(&tv, NULL);

	/* get boot time */
	tv.tv_sec -= info.uptime;

	tv.tv_sec += timestamp / 1000000;
	tv.tv_usec += timestamp % 1000000;

	while (tv.tv_usec < 0) {
		tv.tv_sec--;
		tv.tv_usec += 1000000;
	}

	while (tv.tv_usec >= 1000000) {
		tv.tv_sec++;
		tv.tv_usec -= 1000000;
	}

	Syslog(priority, (uchar *)msg, &tv, json);
}


/* open the kernel log - will be called inside the willRun() imkmsg entry point
 */
rsRetVal
klogWillRunPrePrivDrop(modConfData_t __attribute__((unused)) *pModConf)
{
	char errmsg[2048];
	DEFiRet;

	fklog = open(_PATH_KLOG, O_RDONLY, 0);
	if (fklog < 0) {
		imkmsgLogIntMsg(LOG_ERR, "imkmsg: cannot open kernel log (%s): %s.",
			_PATH_KLOG, rs_strerror_r(errno, errmsg, sizeof(errmsg)));
		ABORT_FINALIZE(RS_RET_ERR_OPEN_KLOG);
	}

finalize_it:
	RETiRet;
}

/* make sure the kernel log is readable after dropping privileges
 */
rsRetVal
klogWillRunPostPrivDrop(modConfData_t __attribute__((unused)) *pModConf)
{
	char errmsg[2048];
	int r;
	DEFiRet;

	/* this normally returns EINVAL */
	/* on an OpenVZ VM, we get EPERM */
	r = read(fklog, NULL, 0);
	if (r < 0 && errno != EINVAL) {
		imkmsgLogIntMsg(LOG_ERR, "imkmsg: cannot open kernel log (%s): %s.",
			_PATH_KLOG, rs_strerror_r(errno, errmsg, sizeof(errmsg)));
		fklog = -1;
		ABORT_FINALIZE(RS_RET_ERR_OPEN_KLOG);
	}

finalize_it:
	RETiRet;
}

/* Read kernel log while data are available, each read() reads one
 * record of printk buffer.
 */
static void
readkmsg(void)
{
	int i;
	uchar pRcv[8192+1];
	char errmsg[2048];

	for (;;) {
		dbgprintf("imkmsg waiting for kernel log line\n");

		/* every read() from the opened device node receives one record of the printk buffer */
		i = read(fklog, pRcv, 8192);

		if (i > 0) {
			/* successful read of message of nonzero length */
			pRcv[i] = '\0';
		} else if (i == -EPIPE) {
			imkmsgLogIntMsg(LOG_WARNING,
					"imkmsg: some messages in circular buffer got overwritten");
			continue;
		} else {
			/* something went wrong - error or zero length message */
			if (i < 0 && errno != EINTR && errno != EAGAIN) {
				/* error occured */
				imkmsgLogIntMsg(LOG_ERR,
				       "imkmsg: error reading kernel log - shutting down: %s",
					rs_strerror_r(errno, errmsg, sizeof(errmsg)));
				fklog = -1;
			}
			break;
		}

		submitSyslog(pRcv);
	}
}


/* to be called in the module's AfterRun entry point
 * rgerhards, 2008-04-09
 */
rsRetVal klogAfterRun(modConfData_t *pModConf)
{
	DEFiRet;
	if(fklog != -1)
		close(fklog);
	/* Turn on logging of messages to console, but only if a log level was speficied */
	if(pModConf->console_log_level != -1)
		klogctl(7, NULL, 0);
	RETiRet;
}


/* to be called in the module's WillRun entry point, this is the main
 * "message pull" mechanism.
 * rgerhards, 2008-04-09
 */
rsRetVal klogLogKMsg(modConfData_t __attribute__((unused)) *pModConf)
{
	DEFiRet;
	readkmsg();
	RETiRet;
}


/* provide the (system-specific) default facility for internal messages
 * rgerhards, 2008-04-14
 */
int
klogFacilIntMsg(void)
{
	return LOG_SYSLOG;
}


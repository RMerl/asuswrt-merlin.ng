/**
 * main rsyslog file with GPLv3 content.
 *
 * *********************** NOTE ************************
 * * Do no longer patch this file. If there is hard    *
 * * need to, talk to Rainer as to how we can make any *
 * * patch be licensed under ASL 2.0.                  *
 * * THIS FILE WILL GO AWAY. The new main file is      *
 * * rsyslogd.c.                                       *
 * *****************************************************
 *
 * Please visit the rsyslog project at
 * http://www.rsyslog.com
 * to learn more about it and discuss any questions you may have.
 *
 * rsyslog had initially been forked from the sysklogd project.
 * I would like to express my thanks to the developers of the sysklogd
 * package - without it, I would have had a much harder start...
 *
 * Please note that while rsyslog started from the sysklogd code base,
 * it nowadays has almost nothing left in common with it. Allmost all
 * parts of the code have been rewritten.
 *
 * This Project was intiated and is maintained by
 * Rainer Gerhards <rgerhards@hq.adiscon.com>.
 *
 * rsyslog - An Enhanced syslogd Replacement.
 * Copyright 2003-2016 Rainer Gerhards and Adiscon GmbH.
 *
 * This file is part of rsyslog.
 *
 * Rsyslog is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * Rsyslog is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with Rsyslog.  If not, see <http://www.gnu.org/licenses/>.
 *
 * A copy of the GPL can be found in the file "COPYING" in this distribution.
 */
#include "config.h"
#include "rsyslog.h"

#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <stddef.h>
#include <ctype.h>
#include <limits.h>
#include <string.h>
#include <stdarg.h>
#include <time.h>
#include <assert.h>
#include <errno.h>

#ifdef OS_SOLARIS
#	include <fcntl.h>
#	include <stropts.h>
#	include <sys/termios.h>
#	include <sys/types.h>
#else
#	include <libgen.h>
#endif

#include <sys/ioctl.h>
#include <sys/wait.h>
#include <sys/file.h>
#include <sys/resource.h>
#include <grp.h>

#ifdef HAVE_SYS_TIMESPEC_H
# include <sys/timespec.h>
#endif

#ifdef HAVE_SYS_STAT_H
#	include <sys/stat.h>
#endif

#include <signal.h>

#ifdef HAVE_PATHS_H
#include <paths.h>
#endif

#include "srUtils.h"
#include "stringbuf.h"
#include "syslogd-types.h"
#include "template.h"
#include "outchannel.h"
#include "syslogd.h"

#include "msg.h"
#include "iminternal.h"
#include "threads.h"
#include "parser.h"
#include "unicode-helper.h"
#include "dnscache.h"
#include "ratelimit.h"

/* forward defintions from rsyslogd.c (ASL 2.0 code) */
extern ratelimit_t *internalMsg_ratelimiter;
extern uchar *ConfFile;
extern ratelimit_t *dflt_ratelimiter;
extern void rsyslogd_usage(void);
extern rsRetVal rsyslogdInit(void);
extern void rsyslogd_destructAllActions(void);
extern void rsyslogd_sigttin_handler();
extern int forkRsyslog(void);
void rsyslogd_submitErrMsg(const int severity, const int iErr, const uchar *msg);
rsRetVal rsyslogd_InitGlobalClasses(void);
rsRetVal rsyslogd_InitStdRatelimiters(void);
rsRetVal rsyslogdInit(void);
void rsyslogdDebugSwitch();
void rsyslogdDoDie(int sig);


#define LIST_DELIMITER	':'		/* delimiter between two hosts */
/* rgerhards, 2005-10-24: crunch_list is called only during option processing. So
 * it is never called once rsyslogd is running. This code
 * contains some exits, but they are considered safe because they only happen
 * during startup. Anyhow, when we review the code here, we might want to
 * reconsider the exit()s.
 * Note: this stems back to sysklogd, so we cannot put it under ASL 2.0. But
 * we may want to check if the code inside the BSD sources is exactly the same
 * (remember that sysklogd forked the BSD sources). If so, the BSD license applies
 * and permits us to move to ASL 2.0 (but we need to check the fine details).
 * Probably it is best just to rewrite this code.
 */
char **syslogd_crunch_list(char *list);
char **syslogd_crunch_list(char *list)
{
	int count, i;
	char *p, *q;
	char **result = NULL;

	p = list;

	/* strip off trailing delimiters */
	while (p[strlen(p)-1] == LIST_DELIMITER) {
		p[strlen(p)-1] = '\0';
	}
	/* cut off leading delimiters */
	while (p[0] == LIST_DELIMITER) {
		p++;
	}

	/* count delimiters to calculate elements */
	for (count=i=0; p[i]; i++)
		if (p[i] == LIST_DELIMITER) count++;

	if ((result = (char **)MALLOC(sizeof(char *) * (count+2))) == NULL) {
		printf ("Sorry, can't get enough memory, exiting.\n");
		exit(0); /* safe exit, because only called during startup */
	}

	/*
	 * We now can assume that the first and last
	 * characters are different from any delimiters,
	 * so we don't have to care about this.
	 */
	count = 0;
	while ((q=strchr(p, LIST_DELIMITER))) {
		result[count] = (char *) MALLOC(q - p + 1);
		if (result[count] == NULL) {
			printf ("Sorry, can't get enough memory, exiting.\n");
			exit(0); /* safe exit, because only called during startup */
		}
		strncpy(result[count], p, q - p);
		result[count][q - p] = '\0';
		p = q; p++;
		count++;
	}
	if ((result[count] = \
	     (char *)MALLOC(strlen(p) + 1)) == NULL) {
		printf ("Sorry, can't get enough memory, exiting.\n");
		exit(0); /* safe exit, because only called during startup */
	}
	strcpy(result[count],p);
	result[++count] = NULL;

	return result;
}


#ifndef HAVE_SETSID
/* stems back to sysklogd in whole */
void untty(void)
{
	int i;
	pid_t pid;

	if(!Debug) {
		/* Peng Haitao <penght@cn.fujitsu.com> contribution */
		pid = getpid();
		if (setpgid(pid, pid) < 0) {
			perror("setpgid");
			exit(1);
		}
		/* end Peng Haitao <penght@cn.fujitsu.com> contribution */

		i = open(_PATH_TTY, O_RDWR|O_CLOEXEC);
		if (i >= 0) {
#			if !defined(__hpux)
				(void) ioctl(i, (int) TIOCNOTTY, NULL);
#			else
				/* TODO: we need to implement something for HP UX! -- rgerhards, 2008-03-04 */
				/* actually, HP UX should have setsid, so the code directly above should
				 * trigger. So the actual question is why it doesn't do that...
				 */
#			endif
			close(i);
		}
	}
}
#endif

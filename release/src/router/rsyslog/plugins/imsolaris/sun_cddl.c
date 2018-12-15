/*
 * CDDL HEADER START
 *
 * The contents of this file are subject to the terms of the
 * Common Development and Distribution License (the "License").
 * You may not use this file except in compliance with the License.
 *
 * You can obtain a copy of the license at usr/src/OPENSOLARIS.LICENSE
 * or http://www.opensolaris.org/os/licensing.
 * See the License for the specific language governing permissions
 * and limitations under the License.
 *
 * When distributing Covered Code, include this CDDL HEADER in each
 * file and include the License file at usr/src/OPENSOLARIS.LICENSE.
 * If applicable, add the following below this CDDL HEADER, with the
 * fields enclosed by brackets "[]" replaced with your own identifying
 * information: Portions Copyright [yyyy] [name of copyright owner]
 *
 * CDDL HEADER END
 */
/* Portions Copyright 2010-2014 by Rainer Gerhards and Adiscon
 */
/*
 * Copyright 2008 Sun Microsystems, Inc.  All rights reserved.
 * Use is subject to license terms.
 */

/*
 *      Copyright (c) 1984, 1986, 1987, 1988, 1989 AT&T
 *      All Rights Reserved
 */

/*
 * University Copyright- Copyright (c) 1982, 1986, 1988
 * The Regents of the University of California
 * All Rights Reserved
 *
 * University Acknowledgment- Portions of this document are derived from
 * software developed by the University of California, Berkeley, and its
 * contributors.
 */
#include "config.h"
#include <unistd.h>
#include <errno.h>
#include <sys/types.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <strings.h>
#include <pthread.h>
#include <fcntl.h>
#include <stropts.h>
#include <assert.h>

#include <sys/param.h>
#include <sys/strlog.h>
#include <sys/stat.h>
#include <sys/time.h>
#include <sys/poll.h>
#include <door.h>
#include <sys/door.h>

#include "rsyslog.h"
#include "srUtils.h"
#include "debug.h"
#include "imsolaris.h"

#define	DOORFILE		"/var/run/syslog_door"
#define	RELATIVE_DOORFILE	"../var/run/syslog_door"
#define	OLD_DOORFILE		"/etc/.syslog_door"

/* Buffer to allocate for error messages: */
#define ERRMSG_LEN 1024

/* Max number of door server threads for syslogd. Since door is used
 * to check the health of syslogd, we don't need large number of
 * server threads.
 */
#define	MAX_DOOR_SERVER_THR	3


struct pollfd sun_Pfd;		/* Pollfd for local log device */

static int		DoorFd = -1;
static int		DoorCreated = 0;
static char		*DoorFileName = DOORFILE;

/* for managing door server threads */
static pthread_mutex_t door_server_cnt_lock = PTHREAD_MUTEX_INITIALIZER;
static uint_t door_server_cnt = 0;
static pthread_attr_t door_thr_attr;

/* the 'server' function that we export via the door. It does
 * nothing but return.
 */
/*ARGSUSED*/
static void
server(	void __attribute__((unused)) *cookie,
	char __attribute__((unused)) *argp,
	size_t __attribute__((unused)) arg_size,
	door_desc_t __attribute__((unused)) *dp,
	__attribute__((unused)) uint_t n          )
{
	(void) door_return(NULL, 0, NULL, 0);
	/* NOTREACHED */
}

/*ARGSUSED*/
static void *
create_door_thr(void __attribute__((unused)) *arg)
{
	(void) pthread_setcancelstate(PTHREAD_CANCEL_DISABLE, NULL);
	(void) door_return(NULL, 0, NULL, 0);

	/* If there is an error in door_return(), it will return here and
	 * the thread will exit. Hence we need to decrement door_server_cnt.
	 */
	(void) pthread_mutex_lock(&door_server_cnt_lock);
	door_server_cnt--;
	(void) pthread_mutex_unlock(&door_server_cnt_lock);
	return (NULL);
}

/*
 * Manage door server thread pool.
 */
/*ARGSUSED*/
static void
door_server_pool(door_info_t __attribute__((unused)) *dip)
{
	(void) pthread_mutex_lock(&door_server_cnt_lock);
	if (door_server_cnt <= MAX_DOOR_SERVER_THR &&
	    pthread_create(NULL, &door_thr_attr, create_door_thr, NULL) == 0) {
		door_server_cnt++;
		(void) pthread_mutex_unlock(&door_server_cnt_lock);
		return;
	}

	(void) pthread_mutex_unlock(&door_server_cnt_lock);
}

void
sun_delete_doorfiles(void)
{
	struct stat sb;
	int err;
	char line[ERRMSG_LEN+1];

	if (lstat(DoorFileName, &sb) == 0 && !S_ISDIR(sb.st_mode)) {
		if (unlink(DoorFileName) < 0) {
			err = errno;
			(void) snprintf(line, sizeof (line),
			    "unlink() of %s failed - fatal", DoorFileName);
			imsolaris_logerror(err, line);
			DBGPRINTF("delete_doorfiles: error: %s, "
			    "errno=%d\n", line, err);
			exit(1);
		}

		DBGPRINTF("delete_doorfiles: deleted %s\n", DoorFileName);
	}

	if (strcmp(DoorFileName, DOORFILE) == 0) {
		if (lstat(OLD_DOORFILE, &sb) == 0 && !S_ISDIR(sb.st_mode)) {
			if (unlink(OLD_DOORFILE) < 0) {
				err = errno;
				(void) snprintf(line, sizeof (line),
				    "unlink() of %s failed", OLD_DOORFILE);
				DBGPRINTF("delete_doorfiles: %s\n", line);

				if (err != EROFS) {
					errno = err;
					(void) strlcat(line, " - fatal",
					    sizeof (line));
					imsolaris_logerror(err, line);
					DBGPRINTF("delete_doorfiles: "
					    "error: %s, errno=%d\n",
					    line, err);
					exit(1);
				}

				DBGPRINTF("delete_doorfiles: unlink() "
				    "failure OK on RO file system\n");
			}

			DBGPRINTF("delete_doorfiles: deleted %s\n",
			    OLD_DOORFILE);
		}
	}

	if (DoorFd != -1) {
		(void) door_revoke(DoorFd);
	}

	DBGPRINTF("delete_doorfiles: revoked door: DoorFd=%d\n",
	    DoorFd);
}


/* Create the door file.  If the filesystem
 * containing /etc is writable, create symlinks /etc/.syslog_door
 * to them.  On systems that do not support /var/run, create
 * /etc/.syslog_door directly.
 */
void
sun_open_door(void)
{
	struct stat buf;
	door_info_t info;
	char line[ERRMSG_LEN+1];
	int err;

	/* first see if another instance of imsolaris OR another
	 * syslogd is running by trying a door call - if it succeeds,
	 * there is already one active.
	 */

	if (!DoorCreated) {
		int door;

		if ((door = open(DoorFileName, O_RDONLY)) >= 0) {
			DBGPRINTF("open_door: %s opened "
			    "successfully\n", DoorFileName);

			if (door_info(door, &info) >= 0) {
				DBGPRINTF("open_door: "
				    "door_info:info.di_target = %ld\n",
				    info.di_target);

				if (info.di_target > 0) {
					(void) sprintf(line, "syslogd pid %ld"
					    " already running. Cannot "
					    "start another syslogd pid %ld",
					    info.di_target, getpid());
					DBGPRINTF("open_door: error: "
					    "%s\n", line);
					imsolaris_logerror(0, line);
					exit(1);
				}
			}

			(void) close(door);
		} else {
			if (lstat(DoorFileName, &buf) < 0) {
				err = errno;

				DBGPRINTF("open_door: lstat() of %s "
				    "failed, errno=%d\n",
				    DoorFileName, err);

				if ((door = creat(DoorFileName, 0644)) < 0) {
					err = errno;
					(void) snprintf(line, sizeof (line),
					    "creat() of %s failed - fatal",
					    DoorFileName);
					DBGPRINTF("open_door: error: %s, "
					    "errno=%d\n", line,
					    err);
					imsolaris_logerror(err, line);
					sun_delete_doorfiles();
					exit(1);
				}

				(void) fchmod(door,
				    S_IRUSR|S_IWUSR|S_IRGRP|S_IROTH);

				DBGPRINTF("open_door: creat() of %s "
				    "succeeded\n",
				    DoorFileName);

				(void) close(door);
			}
		}

		if (strcmp(DoorFileName, DOORFILE) == 0) {
			if (lstat(OLD_DOORFILE, &buf) == 0) {
				DBGPRINTF("open_door: lstat() of %s "
				    "succeeded\n", OLD_DOORFILE);

				if (S_ISDIR(buf.st_mode)) {
					(void) snprintf(line, sizeof (line),
					    "%s is a directory - fatal",
					    OLD_DOORFILE);
					DBGPRINTF("open_door: error: "
					    "%s\n", line);
					imsolaris_logerror(0, line);
					sun_delete_doorfiles();
					exit(1);
				}

				DBGPRINTF("open_door: %s is not a "
				    "directory\n", OLD_DOORFILE);
				if (unlink(OLD_DOORFILE) < 0) {
					err = errno;
					(void) snprintf(line, sizeof (line),
					    "unlink() of %s failed",
					    OLD_DOORFILE);
					DBGPRINTF("open_door: %s\n",
					    line);

					if (err != EROFS) {
						DBGPRINTF("open_door: "
						    "error: %s, "
						    "errno=%d\n",
						    line, err);
						(void) strcat(line, " - fatal");
						imsolaris_logerror(err, line);
						sun_delete_doorfiles();
						exit(1);
					}

					DBGPRINTF("open_door: unlink "
					    "failure OK on RO file "
					    "system\n");
				}
			} else {
				DBGPRINTF("open_door: file %s doesn't "
				    "exist\n", OLD_DOORFILE);
			}

			if (symlink(RELATIVE_DOORFILE, OLD_DOORFILE) < 0) {
				err = errno;
				(void) snprintf(line, sizeof (line),
				    "symlink %s -> %s failed", OLD_DOORFILE,
				    RELATIVE_DOORFILE);
				DBGPRINTF("open_door: %s\n",
				    line);

				if (err != EROFS) {
					DBGPRINTF("open_door: error: %s, "
					    "errno=%d\n", line,
					    err);
					(void) strcat(line, " - fatal");
					imsolaris_logerror(err, line);
					sun_delete_doorfiles();
					exit(1);
				}

				DBGPRINTF("open_door: symlink failure OK "
				    "on RO file system\n");
			} else {
				DBGPRINTF("open_door: symlink %s -> %s "
				    "succeeded\n",
				    OLD_DOORFILE, RELATIVE_DOORFILE);
			}
		}

		if ((DoorFd = door_create(server, 0,
		    DOOR_REFUSE_DESC)) < 0) {
		    //???? DOOR_NO_CANEL requires newer libs??? DOOR_REFUSE_DESC | DOOR_NO_CANCEL)) < 0) {
			err = errno;
			(void) sprintf(line, "door_create() failed - fatal");
			DBGPRINTF("open_door: error: %s, errno=%d\n",
			    line, err);
			imsolaris_logerror(err, line);
			sun_delete_doorfiles();
			exit(1);
		}
		//???? (void) door_setparam(DoorFd, DOOR_PARAM_DATA_MAX, 0);
		DBGPRINTF("open_door: door_create() succeeded, "
		    "DoorFd=%d\n", DoorFd);

		DoorCreated = 1;
	}

	(void) fdetach(DoorFileName);	/* just in case... */

	(void) door_server_create(door_server_pool);

	if (fattach(DoorFd, DoorFileName) < 0) {
		err = errno;
		(void) snprintf(line, sizeof (line), "fattach() of fd"
		    " %d to %s failed - fatal", DoorFd, DoorFileName);
		DBGPRINTF("open_door: error: %s, errno=%d\n",
		    line, err);
		imsolaris_logerror(err, line);
		sun_delete_doorfiles();
		exit(1);
	}

	DBGPRINTF("open_door: attached server() to %s\n",
	    DoorFileName);

}


/* Attempts to open the local log device
 * and return a file descriptor.
 */
rsRetVal
sun_openklog(char *name)
{
	DEFiRet;
	int fd;
	struct strioctl str;
	char errBuf[1024];

	if((fd = open(name, O_RDONLY)) < 0) {
		rs_strerror_r(errno, errBuf, sizeof(errBuf));
		DBGPRINTF("imsolaris:openklog: cannot open %s: %s\n",
		    name, errBuf);
		ABORT_FINALIZE(RS_RET_ERR_OPEN_KLOG);
	}
	str.ic_cmd = I_CONSLOG;
	str.ic_timout = 0;
	str.ic_len = 0;
	str.ic_dp = NULL;
	if (ioctl(fd, I_STR, &str) < 0) {
		rs_strerror_r(errno, errBuf, sizeof(errBuf));
		DBGPRINTF("imsolaris:openklog: cannot register to log "
		    "console messages: %s\n", errBuf);
		ABORT_FINALIZE(RS_RET_ERR_AQ_CONLOG);
	}
	sun_Pfd.fd = fd;
	sun_Pfd.events = POLLIN;
	DBGPRINTF("imsolaris/openklog: opened '%s' as fd %d.\n", name, fd);

finalize_it:
	RETiRet;
}

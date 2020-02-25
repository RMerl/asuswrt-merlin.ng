/*	Updated by troglobit for libite/finit/uftpd projects 2016/07/04 */
/*	$OpenBSD: pidfile.c,v 1.11 2015/06/03 02:24:36 millert Exp $	*/
/*	$NetBSD: pidfile.c,v 1.4 2001/02/19 22:43:42 cgd Exp $	*/

/*-
 * Copyright (c) 1999 The NetBSD Foundation, Inc.
 * All rights reserved.
 *
 * This code is derived from software contributed to The NetBSD Foundation
 * by Jason R. Thorpe.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY THE NETBSD FOUNDATION, INC. AND CONTRIBUTORS
 * ``AS IS'' AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED
 * TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR
 * PURPOSE ARE DISCLAIMED.  IN NO EVENT SHALL THE FOUNDATION OR CONTRIBUTORS
 * BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 * CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 * SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
 * CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 * POSSIBILITY OF SUCH DAMAGE.
 */

#include <sys/stat.h>		/* utimensat() */
#include <sys/time.h>		/* utimensat() on *BSD */
#include <sys/types.h>
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#ifndef HAVE_UTIMENSAT
int utimensat(int dirfd, const char *pathname, const struct timespec ts[2], int flags);
#endif

static char *pidfile_path = NULL;
static pid_t pidfile_pid  = 0;

static void pidfile_cleanup(void);

const  char *__pidfile_path = RUNSTATEDIR;
const  char *__pidfile_name = NULL;
extern char *prognm;

int
pidfile(const char *basename)
{
	int save_errno;
	int atexit_already;
	pid_t pid;
	FILE *f;

	if (basename == NULL)
		basename = prognm;

	pid = getpid();
	atexit_already = 0;

	if (pidfile_path != NULL) {
		if (!access(pidfile_path, R_OK) && pid == pidfile_pid) {
			utimensat(0, pidfile_path, NULL, 0);
			return (0);
		}
		free(pidfile_path);
		pidfile_path = NULL;
		__pidfile_name = NULL;
		atexit_already = 1;
	}

	if (basename[0] != '/') {
		if (asprintf(&pidfile_path, "%s/%s.pid", __pidfile_path, basename) == -1)
			return (-1);
	} else {
		if (asprintf(&pidfile_path, "%s", basename) == -1)
			return (-1);
	}

	if ((f = fopen(pidfile_path, "w")) == NULL) {
		save_errno = errno;
		free(pidfile_path);
		pidfile_path = NULL;
		errno = save_errno;
		return (-1);
	}

	if (fprintf(f, "%ld\n", (long)pid) <= 0 || fflush(f) != 0) {
		save_errno = errno;
		(void) fclose(f);
		(void) unlink(pidfile_path);
		free(pidfile_path);
		pidfile_path = NULL;
		errno = save_errno;
		return (-1);
	}
	(void) fclose(f);
	__pidfile_name = pidfile_path;

	/*
	 * LITE extension, no need to set up another atexit() handler
	 * if user only called us to update the mtime of the PID file
	 */
	if (atexit_already)
		return (0);

	pidfile_pid = pid;
	if (atexit(pidfile_cleanup) < 0) {
		save_errno = errno;
		(void) unlink(pidfile_path);
		free(pidfile_path);
		pidfile_path = NULL;
		pidfile_pid = 0;
		errno = save_errno;
		return (-1);
	}

	return (0);
}

static void
pidfile_cleanup(void)
{
	if (pidfile_path != NULL && pidfile_pid == getpid()) {
		(void) unlink(pidfile_path);
		free(pidfile_path);
		pidfile_path = NULL;
	}
}

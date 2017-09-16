/*
 * Copyright (C) 1984 Sun Microsystems, Inc.
 * Modified by Jeffrey A. Uphoff, 1995, 1997-1999.
 * Modified by Olaf Kirch, 1996.
 *
 * NSM for Linux.
 */

/* 
 * Copyright (c) 2009, Sun Microsystems, Inc.
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 * - Redistributions of source code must retain the above copyright notice,
 *   this list of conditions and the following disclaimer.
 * - Redistributions in binary form must reproduce the above copyright notice,
 *   this list of conditions and the following disclaimer in the documentation
 *   and/or other materials provided with the distribution.
 * - Neither the name of Sun Microsystems, Inc. nor the names of its
 *   contributors may be used to endorse or promote products derived
 *   from this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE
 * LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 * CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 * SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
 * CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 * POSSIBILITY OF SUCH DAMAGE.
 */

/* 
 * This has been modified for my own evil purposes to prevent deadlocks
 * when two hosts start NSM's simultaneously and try to notify each
 * other (which mainly occurs during testing), or to stop and smell the
 * roses when I have callbacks due.
 * --Jeff Uphoff.
 */

/* 
 * This is the RPC server side idle loop.
 * Wait for input, call server program.
 */

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include <errno.h>
#include <time.h>
#include "statd.h"
#include "notlist.h"

static int	svc_stop = 0;

/*
 * This is the global notify list onto which all SM_NOTIFY and CALLBACK
 * requests are put.
 */
notify_list *	notify = NULL;

/*
 * Jump-off function.
 */
void
my_svc_exit(void)
{
	svc_stop = 1;
}


/*
 * The heart of the server.  A crib from libc for the most part...
 */
void
my_svc_run(int sockfd)
{
	FD_SET_TYPE	readfds;
	int             selret;
	time_t		now;

	svc_stop = 0;

	for (;;) {
		if (svc_stop)
			return;

		/* Ah, there are some notifications to be processed */
		while (notify && NL_WHEN(notify) <= time(&now)) {
			process_notify_list();
		}

		readfds = SVC_FDSET;
		/* Set notify sockfd for waiting for reply */
		FD_SET(sockfd, &readfds);
		if (notify) {
			struct timeval	tv;

			tv.tv_sec  = NL_WHEN(notify) - now;
			tv.tv_usec = 0;
			xlog(D_GENERAL, "Waiting for reply... (timeo %d)",
							tv.tv_sec);
			selret = select(FD_SETSIZE, &readfds,
				(void *) 0, (void *) 0, &tv);
		} else {
			xlog(D_GENERAL, "Waiting for client connections");
			selret = select(FD_SETSIZE, &readfds,
				(void *) 0, (void *) 0, (struct timeval *) 0);
		}

		switch (selret) {
		case -1:
			if (errno == EINTR || errno == ECONNREFUSED
			 || errno == ENETUNREACH || errno == EHOSTUNREACH)
				continue;
			xlog(L_ERROR, "my_svc_run() - select: %m");
			return;

		case 0:
			/* A notify/callback timed out. */
			continue;

		default:
			selret -= process_reply(&readfds);
			if (selret) {
				FD_CLR(sockfd, &readfds);
				svc_getreqset(&readfds);
			}
		}
	}
}

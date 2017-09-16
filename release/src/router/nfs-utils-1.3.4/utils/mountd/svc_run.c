/*
 * Copyright (C) 1984 Sun Microsystems, Inc.
 * Based on svc_run.c from statd which claimed:
 * Modified by Jeffrey A. Uphoff, 1995, 1997-1999.
 * Modified by Olaf Kirch, 1996.
 *
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
 * Allow svc_run to listen to other file descriptors as well
 */

/* 
 * This is the RPC server side idle loop.
 * Wait for input, call server program.
 */

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include <sys/types.h>
#include <rpc/rpc.h>
#include "xlog.h"
#include <errno.h>
#include <time.h>

#ifdef HAVE_LIBTIRPC
#include <rpc/rpc_com.h>
#endif

void cache_set_fds(fd_set *fdset);
int cache_process_req(fd_set *readfds);

#if defined(__GLIBC__) && LONG_MAX != INT_MAX
/* bug in glibc 2.3.6 and earlier, we need
 * our own svc_getreqset
 */
static void
my_svc_getreqset (fd_set *readfds)
{
	fd_mask mask;
	fd_mask *maskp;
	int setsize;
	int sock;
	int bit;

	setsize = _rpc_dtablesize ();
	if (setsize > FD_SETSIZE)
		setsize = FD_SETSIZE;
	maskp = readfds->fds_bits;
	for (sock = 0; sock < setsize; sock += NFDBITS)
		for (mask = *maskp++;
		     (bit = ffsl (mask));
		     mask ^= (1L << (bit - 1)))
			svc_getreq_common (sock + bit - 1);
}
#define svc_getreqset my_svc_getreqset

#endif

/*
 * The heart of the server.  A crib from libc for the most part...
 */
void
my_svc_run(void)
{
	fd_set	readfds;
	int	selret;

	for (;;) {

		readfds = svc_fdset;
		cache_set_fds(&readfds);

		selret = select(FD_SETSIZE, &readfds,
				(void *) 0, (void *) 0, (struct timeval *) 0);


		switch (selret) {
		case -1:
			if (errno == EINTR || errno == ECONNREFUSED
			 || errno == ENETUNREACH || errno == EHOSTUNREACH)
				continue;
			xlog(L_ERROR, "my_svc_run() - select: %m");
			return;

		default:
			selret -= cache_process_req(&readfds);
			if (selret)
				svc_getreqset(&readfds);
		}
	}
}

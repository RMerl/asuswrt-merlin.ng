/*
  gssd.c

  Copyright (c) 2000 The Regents of the University of Michigan.
  All rights reserved.

  Copyright (c) 2000 Dug Song <dugsong@UMICH.EDU>.
  Copyright (c) 2002 Andy Adamson <andros@UMICH.EDU>.
  Copyright (c) 2002 Marius Aamodt Eriksen <marius@UMICH.EDU>.
  Copyright (c) 2002 J. Bruce Fields <bfields@UMICH.EDU>.
  All rights reserved, all wrongs reversed.

  Redistribution and use in source and binary forms, with or without
  modification, are permitted provided that the following conditions
  are met:

  1. Redistributions of source code must retain the above copyright
     notice, this list of conditions and the following disclaimer.
  2. Redistributions in binary form must reproduce the above copyright
     notice, this list of conditions and the following disclaimer in the
     documentation and/or other materials provided with the distribution.
  3. Neither the name of the University nor the names of its
     contributors may be used to endorse or promote products derived
     from this software without specific prior written permission.

  THIS SOFTWARE IS PROVIDED ``AS IS'' AND ANY EXPRESS OR IMPLIED
  WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF
  MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
  DISCLAIMED. IN NO EVENT SHALL THE REGENTS OR CONTRIBUTORS BE LIABLE
  FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
  CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
  SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR
  BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF
  LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING
  NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
  SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

*/

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif	/* HAVE_CONFIG_H */

#include <sys/param.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/socket.h>
#include <rpc/rpc.h>
#include <fcntl.h>
#include <errno.h>


#include <unistd.h>
#include <err.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <signal.h>
#include <nfsidmap.h>
#include "nfslib.h"
#include "svcgssd.h"
#include "gss_util.h"
#include "err_util.h"

void
sig_die(int signal)
{
	/* destroy krb5 machine creds */
	printerr(1, "exiting on signal %d\n", signal);
	exit(0);
}

void
sig_hup(int signal)
{
	/* don't exit on SIGHUP */
	printerr(1, "Received SIGHUP(%d)... Ignoring.\n", signal);
	return;
}

static void
usage(char *progname)
{
	fprintf(stderr, "usage: %s [-n] [-f] [-v] [-r] [-i] [-p principal]\n",
		progname);
	exit(1);
}

int
main(int argc, char *argv[])
{
	int get_creds = 1;
	int fg = 0;
	int verbosity = 0;
	int rpc_verbosity = 0;
	int idmap_verbosity = 0;
	int opt, status;
	extern char *optarg;
	char *progname;
	char *principal = NULL;

	while ((opt = getopt(argc, argv, "fivrnp:")) != -1) {
		switch (opt) {
			case 'f':
				fg = 1;
				break;
			case 'i':
				idmap_verbosity++;
				break;
			case 'n':
				get_creds = 0;
				break;
			case 'v':
				verbosity++;
				break;
			case 'r':
				rpc_verbosity++;
				break;
			case 'p':
				principal = optarg;
				break;
			default:
				usage(argv[0]);
				break;
		}
	}

	if ((progname = strrchr(argv[0], '/')))
		progname++;
	else
		progname = argv[0];

	initerr(progname, verbosity, fg);
#ifdef HAVE_AUTHGSS_SET_DEBUG_LEVEL
	if (verbosity && rpc_verbosity == 0)
		rpc_verbosity = verbosity;
	authgss_set_debug_level(rpc_verbosity);
#elif HAVE_LIBTIRPC_SET_DEBUG
        /*
	 * Only set the libtirpc debug level if explicitly requested via -r...
	 * svcgssd is chatty enough as it is.
	 */
        if (rpc_verbosity > 0)
                libtirpc_set_debug(progname, rpc_verbosity, fg);
#else
	if (rpc_verbosity > 0)
		printerr(0, "Warning: rpcsec_gss library does not "
			    "support setting debug level\n");
#endif
#ifdef HAVE_NFS4_SET_DEBUG
		if (verbosity && idmap_verbosity == 0)
			idmap_verbosity = verbosity;
        nfs4_set_debug(idmap_verbosity, NULL);
#else
	if (idmap_verbosity > 0)
		printerr(0, "Warning: your nfsidmap library does not "
			    "support setting debug level\n");
#endif

	if (gssd_check_mechs() != 0) {
		printerr(0, "ERROR: Problem with gssapi library\n");
		exit(1);
	}

	daemon_init(fg);

	signal(SIGINT, sig_die);
	signal(SIGTERM, sig_die);
	signal(SIGHUP, sig_hup);

	if (get_creds) {
		if (principal)
			status = gssd_acquire_cred(principal, 
				((const gss_OID)GSS_C_NT_USER_NAME));
		else
			status = gssd_acquire_cred(GSSD_SERVICE_NAME, 
				(const gss_OID)GSS_C_NT_HOSTBASED_SERVICE);
		if (status == FALSE) {
			printerr(0, "unable to obtain root (machine) credentials\n");
			printerr(0, "do you have a keytab entry for "
				"nfs/<your.host>@<YOUR.REALM> in "
				"/etc/krb5.keytab?\n");
			exit(1);
		}
	} else {
		status = gssd_acquire_cred(NULL,
			(const gss_OID)GSS_C_NT_HOSTBASED_SERVICE);
		if (status == FALSE) {
			printerr(0, "unable to obtain nameless credentials\n");
			exit(1);
		}
	}

	daemon_ready();

	nfs4_init_name_mapping(NULL); /* XXX: should only do this once */
	gssd_run();
	printerr(0, "gssd_run returned!\n");
	abort();
}

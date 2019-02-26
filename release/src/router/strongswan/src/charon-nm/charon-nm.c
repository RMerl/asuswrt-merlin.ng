/*
 * Copyright (C) 2012 Tobias Brunner
 * HSR Hochschule fuer Technik Rapperswil
 *
 * This program is free software; you can redistribute it and/or modify it
 * under the terms of the GNU General Public License as published by the
 * Free Software Foundation; either version 2 of the License, or (at your
 * option) any later version.  See <http://www.fsf.org/copyleft/gpl.txt>.
 *
 * This program is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY
 * or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License
 * for more details.
 */

#include <stdio.h>
#include <syslog.h>
#include <signal.h>
#include <sys/types.h>
#include <unistd.h>
#include <errno.h>

#include <daemon.h>

#include <library.h>
#include <utils/backtrace.h>
#include <threading/thread.h>

#include <nm/nm_backend.h>

/**
 * Default user and group
 */
#ifndef IPSEC_USER
#define IPSEC_USER NULL
#endif

#ifndef IPSEC_GROUP
#define IPSEC_GROUP NULL
#endif

/**
 * Hook in library for debugging messages
 */
extern void (*dbg) (debug_t group, level_t level, char *fmt, ...);

/**
 * Simple logging hook for library logs, using syslog output
 */
static void dbg_syslog(debug_t group, level_t level, char *fmt, ...)
{
	if (level <= 1)
	{
		char buffer[8192], groupstr[4];
		va_list args;

		va_start(args, fmt);
		/* write in memory buffer first */
		vsnprintf(buffer, sizeof(buffer), fmt, args);
		/* cache group name */
		snprintf(groupstr, sizeof(groupstr), "%N", debug_names, group);
		syslog(LOG_DAEMON|LOG_INFO, "00[%s] %s", groupstr, buffer);
		va_end(args);
	}
}

/**
 * Run the daemon and handle unix signals
 */
static void run()
{
	sigset_t set;

	/* handle SIGINT and SIGTERM in this handler */
	sigemptyset(&set);
	sigaddset(&set, SIGINT);
	sigaddset(&set, SIGTERM);
	sigprocmask(SIG_BLOCK, &set, NULL);

	while (TRUE)
	{
		int sig;

		sig = sigwaitinfo(&set, NULL);
		if (sig == -1)
		{
			if (errno == EINTR)
			{	/* ignore signals we didn't wait for */
				continue;
			}
			DBG1(DBG_DMN, "waiting for signal failed: %s", strerror(errno));
			return;
		}
		switch (sig)
		{
			case SIGINT:
			{
				DBG1(DBG_DMN, "signal of type SIGINT received. Shutting down");
				charon->bus->alert(charon->bus, ALERT_SHUTDOWN_SIGNAL, sig);
				return;
			}
			case SIGTERM:
			{
				DBG1(DBG_DMN, "signal of type SIGTERM received. Shutting down");
				charon->bus->alert(charon->bus, ALERT_SHUTDOWN_SIGNAL, sig);
				return;
			}
		}
	}
}

/**
 * Handle SIGSEGV/SIGILL signals raised by threads
 */
static void segv_handler(int signal)
{
	backtrace_t *backtrace;

	DBG1(DBG_DMN, "thread %u received %d", thread_current_id(), signal);
	backtrace = backtrace_create(2);
	backtrace->log(backtrace, stderr, TRUE);
	backtrace->destroy(backtrace);

	DBG1(DBG_DMN, "killing ourself, received critical signal");
	abort();
}

/**
 * Lookup UID and GID
 */
static bool lookup_uid_gid()
{
	char *name;

	name = lib->settings->get_str(lib->settings, "charon-nm.user",
								  IPSEC_USER);
	if (name && !lib->caps->resolve_uid(lib->caps, name))
	{
		return FALSE;
	}
	name = lib->settings->get_str(lib->settings, "charon-nm.group",
								  IPSEC_GROUP);
	if (name && !lib->caps->resolve_gid(lib->caps, name))
	{
		return FALSE;
	}
	return TRUE;
}

/**
 * Main function, starts NetworkManager backend.
 */
int main(int argc, char *argv[])
{
	struct sigaction action;
	int status = SS_RC_INITIALIZATION_FAILED;

	/* logging for library during initialization, as we have no bus yet */
	dbg = dbg_syslog;

	/* LD causes a crash probably due to Glib */
	setenv("LEAK_DETECTIVE_DISABLE", "1", 1);

	/* initialize library */
	if (!library_init(NULL, "charon-nm"))
	{
		library_deinit();
		exit(SS_RC_LIBSTRONGSWAN_INTEGRITY);
	}

	if (lib->integrity &&
		!lib->integrity->check_file(lib->integrity, "charon-nm", argv[0]))
	{
		dbg_syslog(DBG_DMN, 1, "integrity check of charon-nm failed");
		library_deinit();
		exit(SS_RC_DAEMON_INTEGRITY);
	}

	if (!libcharon_init())
	{
		dbg_syslog(DBG_DMN, 1, "initialization failed - aborting charon-nm");
		goto deinit;
	}

	if (!lookup_uid_gid())
	{
		dbg_syslog(DBG_DMN, 1, "invalid uid/gid - aborting charon-nm");
		goto deinit;
	}

	/* make sure we log to the DAEMON facility by default */
	lib->settings->set_int(lib->settings, "charon-nm.syslog.daemon.default",
		lib->settings->get_int(lib->settings,
							   "charon-nm.syslog.daemon.default", 1));
	charon->load_loggers(charon);

	/* use random ports to avoid conflicts with regular charon */
	lib->settings->set_int(lib->settings, "charon-nm.port", 0);
	lib->settings->set_int(lib->settings, "charon-nm.port_nat_t", 0);

	DBG1(DBG_DMN, "Starting charon NetworkManager backend (strongSwan "VERSION")");
	if (lib->integrity)
	{
		DBG1(DBG_DMN, "integrity tests enabled:");
		DBG1(DBG_DMN, "lib    'libstrongswan': passed file and segment integrity tests");
		DBG1(DBG_DMN, "lib    'libcharon': passed file and segment integrity tests");
		DBG1(DBG_DMN, "daemon 'charon-nm': passed file integrity test");
	}

	/* register NM backend to be loaded with plugins */
	nm_backend_register();

	/* initialize daemon */
	if (!charon->initialize(charon,
			lib->settings->get_str(lib->settings, "charon-nm.load", PLUGINS)))
	{
		DBG1(DBG_DMN, "initialization failed - aborting charon-nm");
		goto deinit;
	}
	lib->plugins->status(lib->plugins, LEVEL_CTRL);

	if (!lib->caps->drop(lib->caps))
	{
		DBG1(DBG_DMN, "capability dropping failed - aborting charon-nm");
		goto deinit;
	}

	/* add handler for SEGV and ILL,
	 * INT and TERM are handled by sigwaitinfo() in run() */
	action.sa_handler = segv_handler;
	action.sa_flags = 0;
	sigemptyset(&action.sa_mask);
	sigaddset(&action.sa_mask, SIGINT);
	sigaddset(&action.sa_mask, SIGTERM);
	sigaction(SIGSEGV, &action, NULL);
	sigaction(SIGILL, &action, NULL);
	sigaction(SIGBUS, &action, NULL);
	action.sa_handler = SIG_IGN;
	sigaction(SIGPIPE, &action, NULL);

	pthread_sigmask(SIG_SETMASK, &action.sa_mask, NULL);

	/* start daemon (i.e. the threads in the thread-pool) */
	charon->start(charon);

	/* main thread goes to run loop */
	run();

	status = 0;

deinit:
	libcharon_deinit();
	library_deinit();
	return status;
}


/*
 * Copyright (C) 2013 Martin Willi
 * Copyright (C) 2013 revosec AG
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

#include <sys/types.h>
#include <sys/utsname.h>
#include <unistd.h>
#include <stdio.h>
#include <signal.h>
#include <pthread.h>
#include <errno.h>

#include <library.h>
#include <daemon.h>
#include <threading/thread.h>
#include <utils/backtrace.h>

#include "xpc_dispatch.h"

/**
 * XPC dispatcher class
 */
static xpc_dispatch_t *dispatcher;

/**
 * atexit() cleanup for dispatcher
 */
void dispatcher_cleanup()
{
	DESTROY_IF(dispatcher);
}

/**
 * hook in library for debugging messages
 */
extern void (*dbg) (debug_t group, level_t level, char *fmt, ...);

/**
 * Logging hook for library logs, using stderr output
 */
static void dbg_stderr(debug_t group, level_t level, char *fmt, ...)
{
	va_list args;

	if (level <= 1)
	{
		va_start(args, fmt);
		fprintf(stderr, "00[%N] ", debug_names, group);
		vfprintf(stderr, fmt, args);
		fprintf(stderr, "\n");
		va_end(args);
	}
}

/**
 * Run the daemon and handle unix signals
 */
static int run()
{
	sigset_t set;

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
			return 1;
		}
		switch (sig)
		{
			case SIGINT:
				DBG1(DBG_DMN, "signal of type SIGINT received. Shutting down");
				charon->bus->alert(charon->bus, ALERT_SHUTDOWN_SIGNAL, sig);
				return 0;
			case SIGTERM:
				DBG1(DBG_DMN, "signal of type SIGTERM received. Shutting down");
				charon->bus->alert(charon->bus, ALERT_SHUTDOWN_SIGNAL, sig);
				return 0;
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
	backtrace->log(backtrace, NULL, TRUE);
	backtrace->destroy(backtrace);

	DBG1(DBG_DMN, "killing ourself, received critical signal");
	abort();
}

/**
 * PF_ROUTE for some reason does not send an event for the first address
 * installed after a fresh boot. Fix this by installing a fake tun address
 * just to remove it afterwards.
 */
static void fixup_pf_route()
{
	tun_device_t *tun;
	host_t *host;

	tun = tun_device_create(NULL);
	if (tun)
	{
		if (tun->up(tun))
		{
			host = host_create_from_string("127.0.0.99", 0);
			tun->set_address(tun, host, 32);
			host->destroy(host);
		}
		tun->destroy(tun);
	}
}

/**
 * Main function, starts the daemon.
 */
int main(int argc, char *argv[])
{
	struct sigaction action;
	struct utsname utsname;
	level_t levels[DBG_MAX];
	int group;

	dbg = dbg_stderr;
	atexit(library_deinit);
	if (!library_init(NULL, "charon-xpc"))
	{
		exit(SS_RC_LIBSTRONGSWAN_INTEGRITY);
	}
	if (lib->integrity)
	{
		if (!lib->integrity->check_file(lib->integrity, "charon-xpc", argv[0]))
		{
			exit(SS_RC_DAEMON_INTEGRITY);
		}
	}
	atexit(libcharon_deinit);
	if (!libcharon_init())
	{
		exit(SS_RC_INITIALIZATION_FAILED);
	}
	for (group = 0; group < DBG_MAX; group++)
	{
		levels[group] = LEVEL_CTRL;
	}
	charon->set_default_loggers(charon, levels, TRUE);
	charon->load_loggers(charon);

	lib->settings->set_default_str(lib->settings, "charon-xpc.port", "0");
	lib->settings->set_default_str(lib->settings, "charon-xpc.port_nat_t", "0");
	lib->settings->set_default_str(lib->settings,
								"charon-xpc.close_ike_on_child_failure", "yes");
	lib->settings->set_default_str(lib->settings,
								"charon-xpc.plugins.osx-attr.append", "no");
	if (!charon->initialize(charon,
			lib->settings->get_str(lib->settings, "charon-xpc.load",
				"nonce pkcs1 openssl keychain ctr ccm gcm kernel-libipsec "
				"kernel-pfroute socket-default eap-identity eap-mschapv2 "
				"eap-md5 xauth-generic osx-attr")))
	{
		exit(SS_RC_INITIALIZATION_FAILED);
	}

	if (uname(&utsname) != 0)
	{
		memset(&utsname, 0, sizeof(utsname));
	}
	DBG1(DBG_DMN, "Starting charon-xpc IKE daemon (strongSwan %s, %s %s, %s)",
		 VERSION, utsname.sysname, utsname.release, utsname.machine);

	/* add handler for SEGV and ILL,
	 * INT, TERM and HUP are handled by sigwaitinfo() in run() */
	action.sa_handler = segv_handler;
	action.sa_flags = 0;
	sigemptyset(&action.sa_mask);
	sigaddset(&action.sa_mask, SIGINT);
	sigaddset(&action.sa_mask, SIGTERM);
	sigaddset(&action.sa_mask, SIGHUP);
	sigaction(SIGSEGV, &action, NULL);
	sigaction(SIGILL, &action, NULL);
	sigaction(SIGBUS, &action, NULL);
	action.sa_handler = SIG_IGN;
	sigaction(SIGPIPE, &action, NULL);

	pthread_sigmask(SIG_SETMASK, &action.sa_mask, NULL);

	dispatcher = xpc_dispatch_create();
	if (!dispatcher)
	{
		exit(SS_RC_INITIALIZATION_FAILED);
	}
	atexit(dispatcher_cleanup);

	charon->start(charon);
	fixup_pf_route();
	return run();
}

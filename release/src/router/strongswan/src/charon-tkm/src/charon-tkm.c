/*
 * Copyright (C) 2012-2017 Tobias Brunner
 * Copyright (C) 2012 Reto Buerki
 * Copyright (C) 2012 Adrian-Ken Rueegsegger
 *
 * Copyright (C) secunet Security Networks AG
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

#define _GNU_SOURCE

#include <stdio.h>
#include <syslog.h>
#include <signal.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>
#include <libgen.h>
#include <fcntl.h>
#include <errno.h>

#include <daemon.h>
#include <library.h>
#include <utils/backtrace.h>
#include <threading/thread.h>
#include <sa/keymat.h>
#include <credentials/credential_manager.h>

#include "tkm.h"
#include "tkm_nonceg.h"
#include "tkm_diffie_hellman.h"
#include "tkm_keymat.h"
#include "tkm_listener.h"
#include "tkm_kernel_ipsec.h"
#include "tkm_public_key.h"
#include "tkm_cred.h"
#include "tkm_encoder.h"

/**
 * TKM bus listener for IKE authorize events.
 */
static tkm_listener_t *listener;

/**
 * Name of the daemon
 */
static char *dmn_name;

/**
 * PID file, in which charon-tkm stores its process id
 */
static char *pidfile_name = NULL;

/**
 * Global reference to PID file (required to truncate, if undeletable)
 */
static FILE *pidfile = NULL;

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
		char buffer[8192];
		va_list args;

		va_start(args, fmt);
		/* write in memory buffer first */
		vsnprintf(buffer, sizeof(buffer), fmt, args);
		syslog(LOG_DAEMON|LOG_INFO, "00[%s] %s", debug_names->names[group],
				buffer);
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
			case SIGTERM:
			{
				DBG1(DBG_DMN, "%s received, shutting down",
					 sig == SIGINT ? "SIGINT" : "SIGTERM");
				charon->bus->alert(charon->bus, ALERT_SHUTDOWN_SIGNAL, sig);
				return;
			}
		}
	}
}

#ifndef DISABLE_SIGNAL_HANDLER
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
#endif /* DISABLE_SIGNAL_HANDLER */

/**
 * Lookup UID and GID
 */
static bool lookup_uid_gid()
{
#ifdef IPSEC_USER
	if (!lib->caps->resolve_uid(lib->caps, IPSEC_USER))
	{
		return FALSE;
	}
#endif
#ifdef IPSEC_GROUP
	if (!lib->caps->resolve_gid(lib->caps, IPSEC_GROUP))
	{
		return FALSE;
	}
#endif
	return TRUE;
}

/**
 * Check/create PID file, return TRUE if already running
 */
static bool check_pidfile()
{
	struct stat stb;

	if (stat(pidfile_name, &stb) == 0)
	{
		pidfile = fopen(pidfile_name, "r");
		if (pidfile)
		{
			char buf[64];
			pid_t pid = 0;

			memset(buf, 0, sizeof(buf));
			if (fread(buf, 1, sizeof(buf), pidfile))
			{
				buf[sizeof(buf) - 1] = '\0';
				pid = atoi(buf);
			}
			fclose(pidfile);
			pidfile = NULL;
			if (pid && pid != getpid() && kill(pid, 0) == 0)
			{
				DBG1(DBG_DMN, "%s already running ('%s' exists)", dmn_name,
					 pidfile_name);
				return TRUE;
			}
		}
		DBG1(DBG_DMN, "removing pidfile '%s', process not running", pidfile_name);
		unlink(pidfile_name);
	}

	/* create new pidfile */
	pidfile = fopen(pidfile_name, "w");
	if (pidfile)
	{
		int fd;

		fd = fileno(pidfile);
		if (fd == -1)
		{
			DBG1(DBG_DMN, "unable to determine fd for '%s'", pidfile_name);
			return TRUE;
		}
		if (fcntl(fd, F_SETFD, FD_CLOEXEC) == -1)
		{
			DBG1(DBG_LIB, "setting FD_CLOEXEC for '%s' failed: %s",
				 pidfile_name, strerror(errno));
		}
		ignore_result(fchown(fd,
							 lib->caps->get_uid(lib->caps),
							 lib->caps->get_gid(lib->caps)));
		fprintf(pidfile, "%d\n", getpid());
		fflush(pidfile);
		return FALSE;
	}
	else
	{
		DBG1(DBG_DMN, "unable to create pidfile '%s'", pidfile_name);
		return TRUE;
	}
}

/**
 * Delete/truncate the PID file
 */
static void unlink_pidfile()
{
	/* because unlinking the PID file may fail, we truncate it to ensure the
	 * daemon can be properly restarted.  one probable cause for this is the
	 * combination of not running as root and the effective user lacking
	 * permissions on the parent dir(s) of the PID file */
	if (pidfile)
	{
		ignore_result(ftruncate(fileno(pidfile), 0));
		fclose(pidfile);
		unlink(pidfile_name);
	}
}

/**
 * Main function, starts TKM backend.
 */
int main(int argc, char *argv[])
{
	if (argc > 0 && strlen(argv[0]) > 0)
	{
		dmn_name = basename(argv[0]);
	}
	else
	{
		dmn_name = "charon-tkm";
	}

	/* TKM credential set */
	tkm_cred_t *creds;

	struct sigaction action;
	int status = SS_RC_INITIALIZATION_FAILED;

	/* logging for library during initialization, as we have no bus yet */
	dbg = dbg_syslog;

	/* initialize library */
	if (!library_init(NULL, dmn_name))
	{
		library_deinit();
		exit(status);
	}

	if (!libcharon_init())
	{
		dbg_syslog(DBG_DMN, 1, "initialization failed - aborting %s", dmn_name);
		goto deinit;
	}

	if (!lookup_uid_gid())
	{
		dbg_syslog(DBG_DMN, 1, "invalid uid/gid - aborting %s", dmn_name);
		goto deinit;
	}

	/* the authorize hook currently does not support RFC 7427 signature auth */
	lib->settings->set_bool(lib->settings, "%s.signature_authentication", FALSE,
							dmn_name);

	/* make sure we log to the DAEMON facility by default */
	lib->settings->set_int(lib->settings, "%s.syslog.daemon.default",
			lib->settings->get_int(lib->settings, "%s.syslog.daemon.default", 1,
								   dmn_name), dmn_name);
	charon->load_loggers(charon);

	DBG1(DBG_DMN, "Starting charon with TKM backend (strongSwan "VERSION")");

	/* register TKM specific plugins */
	static plugin_feature_t features[] = {
		PLUGIN_REGISTER(NONCE_GEN, tkm_nonceg_create),
			PLUGIN_PROVIDE(NONCE_GEN),
		PLUGIN_REGISTER(PUBKEY, tkm_public_key_load, TRUE),
			PLUGIN_PROVIDE(PUBKEY, KEY_RSA),
			PLUGIN_PROVIDE(PUBKEY_VERIFY, SIGN_RSA_EMSA_PKCS1_SHA1),
			PLUGIN_PROVIDE(PUBKEY_VERIFY, SIGN_RSA_EMSA_PKCS1_SHA2_256),
		PLUGIN_CALLBACK(kernel_ipsec_register, tkm_kernel_ipsec_create),
			PLUGIN_PROVIDE(CUSTOM, "kernel-ipsec"),
	};
	lib->plugins->add_static_features(lib->plugins, "tkm-backend", features,
			countof(features), TRUE, NULL, NULL);

	if (!register_dh_mapping())
	{
		DBG1(DBG_DMN, "no DH group mapping defined - aborting %s", dmn_name);
		goto deinit;
	}

	if (!register_ca_mapping())
	{
		DBG1(DBG_DMN, "no CA certificate ID mapping defined - aborting %s", dmn_name);
		goto deinit;
	}

	/* register TKM keymat variant */
	keymat_register_constructor(IKEV2, (keymat_constructor_t)tkm_keymat_create);

	/* register TKM credential encoder */
	lib->encoding->add_encoder(lib->encoding, tkm_encoder_encode);

	/* initialize daemon */
	if (!charon->initialize(charon, PLUGINS))
	{
		DBG1(DBG_DMN, "initialization failed - aborting %s", dmn_name);
		goto deinit;
	}
	lib->plugins->status(lib->plugins, LEVEL_CTRL);

	/* set global pidfile name depending on daemon name */
	if (asprintf(&pidfile_name, IPSEC_PIDDIR"/%s.pid", dmn_name) < 0)
	{
		DBG1(DBG_DMN, "unable to set pidfile name - aborting %s", dmn_name);
		goto deinit;
	};

	if (check_pidfile())
	{
		goto deinit;
	}

	if (!lib->caps->drop(lib->caps))
	{
		DBG1(DBG_DMN, "capability dropping failed - aborting %s", dmn_name);
		goto deinit;
	}

	/* initialize TKM client */
	if (!tkm_init())
	{
		DBG1(DBG_DMN, "init of TKM client failed - aborting %s", dmn_name);
		goto deinit;
	}

	/* register TKM authorization hook */
	listener = tkm_listener_create();
	charon->bus->add_listener(charon->bus, &listener->listener);

	/* register TKM credential set */
	creds = tkm_cred_create();
	lib->credmgr->add_set(lib->credmgr, (credential_set_t*)creds);

	/* add handler for fatal signals,
	 * INT and TERM are handled by sigwaitinfo() in run() */
	action.sa_flags = 0;
	sigemptyset(&action.sa_mask);
	sigaddset(&action.sa_mask, SIGINT);
	sigaddset(&action.sa_mask, SIGTERM);

	/* optionally let the external system handle fatal signals */
#ifndef DISABLE_SIGNAL_HANDLER
	action.sa_handler = segv_handler;
	sigaction(SIGSEGV, &action, NULL);
	sigaction(SIGILL, &action, NULL);
	sigaction(SIGBUS, &action, NULL);
#endif /* DISABLE_SIGNAL_HANDLER */

	action.sa_handler = SIG_IGN;
	sigaction(SIGPIPE, &action, NULL);

	pthread_sigmask(SIG_SETMASK, &action.sa_mask, NULL);

	/* start daemon (i.e. the threads in the thread-pool) */
	charon->start(charon);

	/* main thread goes to run loop */
	run();

	status = 0;
	charon->bus->remove_listener(charon->bus, &listener->listener);
	listener->destroy(listener);
	creds->destroy(creds);
	lib->encoding->remove_encoder(lib->encoding, tkm_encoder_encode);

deinit:
	destroy_dh_mapping();
	destroy_ca_mapping();
	libcharon_deinit();
	tkm_deinit();
	unlink_pidfile();
	free(pidfile_name);
	library_deinit();
	return status;
}

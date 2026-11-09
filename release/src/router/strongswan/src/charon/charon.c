/*
 * Copyright (C) 2006-2017 Tobias Brunner
 * Copyright (C) 2005-2009 Martin Willi
 * Copyright (C) 2006 Daniel Roethlisberger
 * Copyright (C) 2005 Jan Hutter
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

#include <stdio.h>
#include <signal.h>
#include <pthread.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/utsname.h>
#include <unistd.h>
#include <getopt.h>
#include <fcntl.h>
#include <errno.h>

#include <daemon.h>

#include <library.h>
#include <utils/backtrace.h>
#include <threading/thread.h>

#ifdef ANDROID
#include <private/android_filesystem_config.h> /* for AID_VPN */
#endif

/**
 * PID file, in which charon stores its process id
 */
#define PID_FILE IPSEC_PIDDIR "/charon.pid"

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
 * Global reference to PID file (required to truncate, if undeletable)
 */
static FILE *pidfile = NULL;

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
static void run()
{
	sigset_t set;

	/* handle SIGINT, SIGHUP and SIGTERM in this handler */
	sigemptyset(&set);
	sigaddset(&set, SIGINT);
	sigaddset(&set, SIGHUP);
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
			case SIGHUP:
			{
				DBG1(DBG_DMN, "signal of type SIGHUP received. Reloading "
					 "configuration");
				if (lib->settings->load_files(lib->settings, lib->conf, FALSE))
				{
					charon->load_loggers(charon);
					lib->plugins->reload(lib->plugins, NULL);
				}
				else
				{
					DBG1(DBG_DMN, "reloading config failed, keeping old");
				}
				break;
			}
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

/**
 * lookup UID and GID
 */
static bool lookup_uid_gid()
{
	char *name;

	name = lib->settings->get_str(lib->settings, "charon.user", IPSEC_USER);
	if (name && !lib->caps->resolve_uid(lib->caps, name))
	{
		return FALSE;
	}
	name = lib->settings->get_str(lib->settings, "charon.group", IPSEC_GROUP);
	if (name && !lib->caps->resolve_gid(lib->caps, name))
	{
		return FALSE;
	}
#ifdef ANDROID
	lib->caps->set_uid(lib->caps, AID_VPN);
#endif
	return TRUE;
}

/**
 * Handle SIGSEGV/SIGILL signals raised by threads
 */
#ifndef DISABLE_SIGNAL_HANDLER
static void segv_handler(int signal)
{
	backtrace_t *backtrace;

	DBG1(DBG_DMN, "thread %u received %d", thread_current_id(), signal);
	backtrace = backtrace_create(2);
	backtrace->log(backtrace, NULL, TRUE);
	backtrace->log(backtrace, stderr, TRUE);
	backtrace->destroy(backtrace);

	DBG1(DBG_DMN, "killing ourself, received critical signal");
	abort();
}
#endif /* DISABLE_SIGNAL_HANDLER */

/**
 * Check/create PID file, return TRUE if already running
 */
static bool check_pidfile()
{
	struct stat stb;

	if (stat(PID_FILE, &stb) == 0)
	{
		pidfile = fopen(PID_FILE, "r");
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
				DBG1(DBG_DMN, "charon already running ('"PID_FILE"' exists)");
				return TRUE;
			}
		}
		DBG1(DBG_DMN, "removing pidfile '"PID_FILE"', process not running");
		unlink(PID_FILE);
	}

	/* create new pidfile */
	pidfile = fopen(PID_FILE, "w");
	if (pidfile)
	{
		int fd;

		fd = fileno(pidfile);
		if (fd == -1)
		{
			DBG1(DBG_DMN, "unable to determine fd for '"PID_FILE"'");
			return TRUE;
		}
		if (fcntl(fd, F_SETFD, FD_CLOEXEC) == -1)
		{
			DBG1(DBG_LIB, "setting FD_CLOEXEC for '"PID_FILE"' failed: %s",
				 strerror(errno));
		}
		/* Only change owner of the pidfile if we have CAP_CHOWN. Otherwise,
		 * attempt to change group of pidfile to group under which charon
		 * runs after dropping caps. This requires the user that charon
		 * starts as to:
		 * a) Have write access to the socket dir.
		 * b) Belong to the group that charon will run under after dropping
		 *    caps. */
		if (lib->caps->check(lib->caps, CAP_CHOWN))
		{
			ignore_result(fchown(fd,
								 lib->caps->get_uid(lib->caps),
								 lib->caps->get_gid(lib->caps)));
		}
		else
		{
			ignore_result(fchown(fd, -1,
								 lib->caps->get_gid(lib->caps)));
		}
		fprintf(pidfile, "%d\n", getpid());
		fflush(pidfile);
		return FALSE;
	}
	else
	{
		DBG1(DBG_DMN, "unable to create pidfile '"PID_FILE"'");
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
		unlink(PID_FILE);
	}
}

/**
 * print command line usage and exit
 */
static void usage(const char *msg)
{
	if (msg != NULL && *msg != '\0')
	{
		fprintf(stderr, "%s\n", msg);
	}
	fprintf(stderr, "Usage: charon\n"
					"         [--help]\n"
					"         [--version]\n"
					"         [--use-syslog]\n"
					"         [--debug-<type> <level>]\n"
					"           <type>:  log context type (dmn|mgr|ike|chd|job|cfg|knl|net|asn|enc|tnc|imc|imv|pts|tls|esp|lib)\n"
					"           <level>: log verbosity (-1 = silent, 0 = audit, 1 = control,\n"
					"                                    2 = controlmore, 3 = raw, 4 = private)\n"
					"\n"
		   );
}

/**
 * Main function, starts the daemon.
 */
int main(int argc, char *argv[])
{
	struct sigaction action;
	int group, status = SS_RC_INITIALIZATION_FAILED;
	struct utsname utsname;
	level_t levels[DBG_MAX];
	bool use_syslog = FALSE;

	/* logging for library during initialization, as we have no bus yet */
	dbg = dbg_stderr;

	/* initialize library */
	if (!library_init(NULL, "charon"))
	{
		library_deinit();
		exit(SS_RC_LIBSTRONGSWAN_INTEGRITY);
	}

	if (lib->integrity &&
		!lib->integrity->check_file(lib->integrity, "charon", argv[0]))
	{
		dbg_stderr(DBG_DMN, 1, "integrity check of charon failed");
		library_deinit();
		exit(SS_RC_DAEMON_INTEGRITY);
	}

	if (!libcharon_init())
	{
		dbg_stderr(DBG_DMN, 1, "initialization failed - aborting charon");
		goto deinit;
	}

	/* use CTRL loglevel for default */
	for (group = 0; group < DBG_MAX; group++)
	{
		levels[group] = LEVEL_CTRL;
	}

	/* handle arguments */
	for (;;)
	{
		struct option long_opts[] = {
			{ "help", no_argument, NULL, 'h' },
			{ "version", no_argument, NULL, 'v' },
			{ "use-syslog", no_argument, NULL, 'l' },
			/* TODO: handle "debug-all" */
			{ "debug-dmn", required_argument, &group, DBG_DMN },
			{ "debug-mgr", required_argument, &group, DBG_MGR },
			{ "debug-ike", required_argument, &group, DBG_IKE },
			{ "debug-chd", required_argument, &group, DBG_CHD },
			{ "debug-job", required_argument, &group, DBG_JOB },
			{ "debug-cfg", required_argument, &group, DBG_CFG },
			{ "debug-knl", required_argument, &group, DBG_KNL },
			{ "debug-net", required_argument, &group, DBG_NET },
			{ "debug-asn", required_argument, &group, DBG_ASN },
			{ "debug-enc", required_argument, &group, DBG_ENC },
			{ "debug-tnc", required_argument, &group, DBG_TNC },
			{ "debug-imc", required_argument, &group, DBG_IMC },
			{ "debug-imv", required_argument, &group, DBG_IMV },
			{ "debug-pts", required_argument, &group, DBG_PTS },
			{ "debug-tls", required_argument, &group, DBG_TLS },
			{ "debug-esp", required_argument, &group, DBG_ESP },
			{ "debug-lib", required_argument, &group, DBG_LIB },
			{ 0,0,0,0 }
		};

		int c = getopt_long(argc, argv, "", long_opts, NULL);
		switch (c)
		{
			case EOF:
				break;
			case 'h':
				usage(NULL);
				status = 0;
				goto deinit;
			case 'v':
				printf("strongSwan %s\n", VERSION);
				status = 0;
				goto deinit;
			case 'l':
				use_syslog = TRUE;
				continue;
			case 0:
				/* option is in group */
				levels[group] = atoi(optarg);
				continue;
			default:
				usage("");
				status = 1;
				goto deinit;
		}
		break;
	}

	if (!lookup_uid_gid())
	{
		dbg_stderr(DBG_DMN, 1, "invalid uid/gid - aborting charon");
		goto deinit;
	}

	charon->set_default_loggers(charon, levels, !use_syslog);
	charon->load_loggers(charon);

	if (uname(&utsname) != 0)
	{
		memset(&utsname, 0, sizeof(utsname));
	}
	DBG1(DBG_DMN, "Starting IKE charon daemon (strongSwan "VERSION", %s %s, %s)",
		  utsname.sysname, utsname.release, utsname.machine);
	if (lib->integrity)
	{
		DBG1(DBG_DMN, "integrity tests enabled:");
		DBG1(DBG_DMN, "lib    'libstrongswan': passed file and segment integrity tests");
		DBG1(DBG_DMN, "lib    'libcharon': passed file and segment integrity tests");
		DBG1(DBG_DMN, "daemon 'charon': passed file integrity test");
	}

	/* initialize daemon */
	if (!charon->initialize(charon,
				lib->settings->get_str(lib->settings, "charon.load", PLUGINS)))
	{
		DBG1(DBG_DMN, "initialization failed - aborting charon");
		goto deinit;
	}
	lib->plugins->status(lib->plugins, LEVEL_CTRL);

	if (check_pidfile())
	{
		goto deinit;
	}

	if (!lib->caps->drop(lib->caps))
	{
		DBG1(DBG_DMN, "capability dropping failed - aborting charon");
		goto deinit;
	}

	/* add handler for fatal signals,
	 * INT, TERM and HUP are handled by sigwaitinfo() in run() */
	action.sa_flags = 0;
	sigemptyset(&action.sa_mask);
	sigaddset(&action.sa_mask, SIGINT);
	sigaddset(&action.sa_mask, SIGTERM);
	sigaddset(&action.sa_mask, SIGHUP);

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

deinit:
	libcharon_deinit();
	unlink_pidfile();
	library_deinit();
	return status;
}

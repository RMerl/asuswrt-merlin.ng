/* strongSwan IPsec starter
 * Copyright (C) 2001-2002 Mathieu Lafon - Arkoon Network Security
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

#include <sys/select.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <sys/stat.h>
#include <stdlib.h>
#include <stdio.h>
#include <signal.h>
#include <syslog.h>
#include <unistd.h>
#include <sys/time.h>
#include <time.h>
#include <string.h>
#include <errno.h>
#include <fcntl.h>
#include <pwd.h>
#include <grp.h>
#include <pthread.h>

#include <library.h>
#include <hydra.h>
#include <utils/backtrace.h>
#include <threading/thread.h>
#include <utils/debug.h>

#include "confread.h"
#include "files.h"
#include "starterstroke.h"
#include "invokecharon.h"
#include "netkey.h"
#include "klips.h"
#include "cmp.h"

#ifndef LOG_AUTHPRIV
#define LOG_AUTHPRIV LOG_AUTH
#endif

#define CHARON_RESTART_DELAY 5

static const char* cmd_default = IPSEC_DIR "/charon";
static const char* pid_file_default = IPSEC_PIDDIR "/charon.pid";
static const char* starter_pid_file_default = IPSEC_PIDDIR "/starter.pid";

char *daemon_name = NULL;
char *cmd = NULL;
char *pid_file = NULL;
char *starter_pid_file = NULL;

static char *config_file = NULL;

/* logging */
static bool log_to_stderr = TRUE;
static bool log_to_syslog = TRUE;
static level_t current_loglevel = 1;

/**
 * logging function for scepclient
 */
static void starter_dbg(debug_t group, level_t level, char *fmt, ...)
{
	char buffer[8192];
	char *current = buffer, *next;
	va_list args;

	if (level <= current_loglevel)
	{
		if (log_to_stderr)
		{
			va_start(args, fmt);
			vfprintf(stderr, fmt, args);
			va_end(args);
			fprintf(stderr, "\n");
		}
		if (log_to_syslog)
		{
			/* write in memory buffer first */
			va_start(args, fmt);
			vsnprintf(buffer, sizeof(buffer), fmt, args);
			va_end(args);

			/* do a syslog with every line */
			while (current)
			{
				next = strchr(current, '\n');
				if (next)
				{
					*(next++) = '\0';
				}
				syslog(LOG_INFO, "%s\n", current);
				current = next;
			}
		}
	}
}

/**
 * Initialize logging to stderr/syslog
 */
static void init_log(const char *program)
{
	dbg = starter_dbg;

	if (log_to_stderr)
	{
		setbuf(stderr, NULL);
	}
	if (log_to_syslog)
	{
		openlog(program, LOG_CONS | LOG_NDELAY | LOG_PID, LOG_AUTHPRIV);
	}
}

/**
 * Deinitialize logging to syslog
 */
static void close_log()
{
	if (log_to_syslog)
	{
		closelog();
	}
}

/**
 * Return codes defined by Linux Standard Base Core Specification 3.1
 * in section 20.2. Init Script Actions
 */
#define LSB_RC_SUCCESS               0   /* success                          */
#define LSB_RC_FAILURE               1   /* generic or unspecified error     */
#define LSB_RC_INVALID_ARGUMENT      2   /* invalid or excess argument(s)    */
#define LSB_RC_NOT_IMPLEMENTED       3   /* unimplemented feature (reload)   */
#define LSB_RC_NOT_ALLOWED           4   /* user had insufficient privilege  */
#define LSB_RC_NOT_INSTALLED         5   /* program is not installed         */
#define LSB_RC_NOT_CONFIGURED        6   /* program is not configured        */
#define LSB_RC_NOT_RUNNING           7   /* program is not running           */

#define FLAG_ACTION_START_PLUTO   0x01
#define FLAG_ACTION_UPDATE        0x02
#define FLAG_ACTION_RELOAD        0x04
#define FLAG_ACTION_QUIT          0x08
#define FLAG_ACTION_LISTEN        0x10
#define FLAG_ACTION_START_CHARON  0x20

static unsigned int _action_ = 0;

/**
 * Handle signals in the main thread
 */
static void signal_handler(int signal)
{
	switch (signal)
	{
		case SIGCHLD:
		{
			int status, exit_status = 0;
			pid_t pid;
			char *name = NULL;

			while ((pid = waitpid(-1, &status, WNOHANG)) > 0)
			{
				if (pid == starter_charon_pid())
				{
					if (asprintf(&name, " (%s)", daemon_name) < 0)
					{
						 name = NULL;
					}
				}
				if (WIFSIGNALED(status))
				{
					DBG2(DBG_APP, "child %d%s has been killed by sig %d\n",
						 pid, name?name:"", WTERMSIG(status));
				}
				else if (WIFSTOPPED(status))
				{
					DBG2(DBG_APP, "child %d%s has been stopped by sig %d\n",
						 pid, name?name:"", WSTOPSIG(status));
				}
				else if (WIFEXITED(status))
				{
					exit_status =  WEXITSTATUS(status);
					if (exit_status >= SS_RC_FIRST && exit_status <= SS_RC_LAST)
					{
						_action_ =  FLAG_ACTION_QUIT;
					}
					DBG2(DBG_APP, "child %d%s has quit (exit code %d)\n",
						 pid, name?name:"", exit_status);
				}
				else
				{
					DBG2(DBG_APP, "child %d%s has quit", pid, name?name:"");
				}
				if (pid == starter_charon_pid())
				{
					starter_charon_sigchild(pid, exit_status);
				}
			}

			if (name)
			{
				free(name);
			}
		}
		break;

		case SIGALRM:
			_action_ |= FLAG_ACTION_START_CHARON;
			break;

		case SIGHUP:
			_action_ |= FLAG_ACTION_UPDATE;
			break;

		case SIGTERM:
		case SIGQUIT:
		case SIGINT:
			_action_ |= FLAG_ACTION_QUIT;
			break;

		case SIGUSR1:
			_action_ |= FLAG_ACTION_RELOAD;
			_action_ |= FLAG_ACTION_UPDATE;
			break;

		default:
			DBG1(DBG_APP, "fsig(): unknown signal %d -- investigate", signal);
			break;
	}
}

/**
 * Handle fatal signals raised by threads
 */
static void fatal_signal_handler(int signal)
{
	backtrace_t *backtrace;

	DBG1(DBG_APP, "thread %u received %d", thread_current_id(), signal);
	backtrace = backtrace_create(2);
	backtrace->log(backtrace, stderr, TRUE);
	backtrace->destroy(backtrace);

	DBG1(DBG_APP, "killing ourself, received critical signal");
	abort();
}

#ifdef GENERATE_SELFCERT
static void generate_selfcert()
{
	const char *secrets_file;
	struct stat stb;

	secrets_file = lib->settings->get_str(lib->settings,
							"charon.plugins.stroke.secrets_file", SECRETS_FILE);

	/* if ipsec.secrets file is missing then generate RSA default key pair */
	if (stat(secrets_file, &stb) != 0)
	{
		mode_t oldmask;
		FILE *f;
		uid_t uid = 0;
		gid_t gid = 0;

#ifdef IPSEC_GROUP
		{
			char buf[1024];
			struct group group, *grp;

			if (getgrnam_r(IPSEC_GROUP, &group, buf, sizeof(buf), &grp) == 0 &&	grp)
			{
				gid = grp->gr_gid;
			}
		}
#endif
#ifdef IPSEC_USER
		{
			char buf[1024];
			struct passwd passwd, *pwp;

			if (getpwnam_r(IPSEC_USER, &passwd, buf, sizeof(buf), &pwp) == 0 &&	pwp)
			{
				uid = pwp->pw_uid;
			}
		}
#endif
		ignore_result(setegid(gid));
		ignore_result(seteuid(uid));
		ignore_result(system(IPSEC_SCRIPT " scepclient --out pkcs1 --out cert-self --quiet"));
		ignore_result(seteuid(0));
		ignore_result(setegid(0));

		/* ipsec.secrets is root readable only */
		oldmask = umask(0066);

		f = fopen(secrets_file, "w");
		if (f)
		{
			fprintf(f, "# /etc/ipsec.secrets - strongSwan IPsec secrets file\n");
			fprintf(f, "\n");
			fprintf(f, ": RSA myKey.der\n");
			fclose(f);
		}
		ignore_result(chown(secrets_file, uid, gid));
		umask(oldmask);
	}
}
#endif /* GENERATE_SELFCERT */

static bool check_pid(char *pid_file)
{
	struct stat stb;
	FILE *pidfile;

	if (stat(pid_file, &stb) == 0)
	{
		pidfile = fopen(pid_file, "r");
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
			if (pid && kill(pid, 0) == 0)
			{	/* such a process is running */
				return TRUE;
			}
		}
		DBG1(DBG_APP, "removing pidfile '%s', process not running", pid_file);
		unlink(pid_file);
	}
	return FALSE;
}

/* Set daemon name and adjust command and pid filenames accordingly */
static bool set_daemon_name()
{
	if (!daemon_name)
	{
		daemon_name = "charon";
	}

	if (asprintf(&cmd, IPSEC_DIR"/%s", daemon_name) < 0)
	{
		 cmd = (char*)cmd_default;
	}

	if (asprintf(&pid_file, IPSEC_PIDDIR"/%s.pid", daemon_name) < 0)
	{
		 pid_file = (char*)pid_file_default;
	}

	if (asprintf(&starter_pid_file, IPSEC_PIDDIR"/starter.%s.pid",
				 daemon_name) < 0)
	{
		 starter_pid_file = (char*)starter_pid_file_default;
	}

	return TRUE;
}

static void cleanup()
{
	if (cmd != cmd_default)
	{
		free(cmd);
	}

	if (pid_file != pid_file_default)
	{
		free(pid_file);
	}

	if (starter_pid_file != starter_pid_file_default)
	{
		free(starter_pid_file);
	}
}

static void usage(char *name)
{
	fprintf(stderr, "Usage: starter [--nofork] [--auto-update <sec>]\n"
			"               [--debug|--debug-more|--debug-all|--nolog]\n"
			"               [--attach-gdb] [--daemon <name>]\n"
			"               [--conf <path to ipsec.conf>]\n");
	exit(LSB_RC_INVALID_ARGUMENT);
}

int main (int argc, char **argv)
{
	starter_config_t *cfg = NULL;
	starter_config_t *new_cfg;
	starter_conn_t *conn, *conn2;
	starter_ca_t *ca, *ca2;

	struct sigaction action;
	struct stat stb;

	int i;
	int id = 1;
	struct timespec ts;
	unsigned long auto_update = 0;
	time_t last_reload;
	bool no_fork = FALSE;
	bool attach_gdb = FALSE;
	bool load_warning = FALSE;
	bool conftest = FALSE;

	library_init(NULL, "starter");
	atexit(library_deinit);

	libhydra_init();
	atexit(libhydra_deinit);

	/* parse command line */
	for (i = 1; i < argc; i++)
	{
		if (streq(argv[i], "--debug"))
		{
			current_loglevel = 2;
		}
		else if (streq(argv[i], "--debug-more"))
		{
			current_loglevel = 3;
		}
		else if (streq(argv[i], "--debug-all"))
		{
			current_loglevel = 4;
		}
		else if (streq(argv[i], "--nolog"))
		{
			current_loglevel = 0;
		}
		else if (streq(argv[i], "--nofork"))
		{
			no_fork = TRUE;
		}
		else if (streq(argv[i], "--attach-gdb"))
		{
			no_fork = TRUE;
			attach_gdb = TRUE;
		}
		else if (streq(argv[i], "--auto-update") && i+1 < argc)
		{
			auto_update = atoi(argv[++i]);
			if (!auto_update)
				usage(argv[0]);
		}
		else if (streq(argv[i], "--daemon") && i+1 < argc)
		{
			daemon_name = argv[++i];
		}
		else if (streq(argv[i], "--conf") && i+1 < argc)
		{
			config_file = argv[++i];
		}
		else if (streq(argv[i], "--conftest"))
		{
			conftest = TRUE;
		}
		else
		{
			usage(argv[0]);
		}
	}

	if (!set_daemon_name())
	{
		DBG1(DBG_APP, "unable to set daemon name");
		exit(LSB_RC_FAILURE);
	}
	if (!config_file)
	{
		config_file = lib->settings->get_str(lib->settings,
											 "starter.config_file", CONFIG_FILE);
	}

	init_log("ipsec_starter");

	if (conftest)
	{
		int status = LSB_RC_SUCCESS;

		cfg = confread_load(config_file);
		if (cfg == NULL || cfg->err > 0)
		{
			DBG1(DBG_APP, "config invalid!");
			status = LSB_RC_INVALID_ARGUMENT;
		}
		else
		{
			DBG1(DBG_APP, "config OK");
		}
		if (cfg)
		{
			confread_free(cfg);
		}
		cleanup();
		exit(status);
	}

	DBG1(DBG_APP, "Starting %sSwan "VERSION" IPsec [starter]...",
		lib->settings->get_bool(lib->settings,
			"charon.i_dont_care_about_security_and_use_aggressive_mode_psk",
				FALSE) ? "weak" : "strong");

#ifdef LOAD_WARNING
	load_warning = TRUE;
#endif

	if (lib->settings->get_bool(lib->settings, "starter.load_warning", load_warning))
	{
		if (lib->settings->get_str(lib->settings, "charon.load", NULL))
		{
			DBG1(DBG_APP, "!! Your strongswan.conf contains manual plugin load options for charon.");
			DBG1(DBG_APP, "!! This is recommended for experts only, see");
			DBG1(DBG_APP, "!! http://wiki.strongswan.org/projects/strongswan/wiki/PluginLoad");
		}
	}

	/* verify that we can start */
	if (getuid() != 0)
	{
		DBG1(DBG_APP, "permission denied (must be superuser)");
		cleanup();
		exit(LSB_RC_NOT_ALLOWED);
	}

	if (check_pid(pid_file))
	{
		DBG1(DBG_APP, "%s is already running (%s exists) -- skipping daemon start",
			 daemon_name, pid_file);
	}
	else
	{
		_action_ |= FLAG_ACTION_START_CHARON;
	}
	if (stat(DEV_RANDOM, &stb) != 0)
	{
		DBG1(DBG_APP, "unable to start strongSwan IPsec -- no %s!", DEV_RANDOM);
		cleanup();
		exit(LSB_RC_FAILURE);
	}

	if (stat(DEV_URANDOM, &stb)!= 0)
	{
		DBG1(DBG_APP, "unable to start strongSwan IPsec -- no %s!", DEV_URANDOM);
		cleanup();
		exit(LSB_RC_FAILURE);
	}

	cfg = confread_load(config_file);
	if (cfg == NULL || cfg->err > 0)
	{
		DBG1(DBG_APP, "unable to start strongSwan -- fatal errors in config");
		if (cfg)
		{
			confread_free(cfg);
		}
		cleanup();
		exit(LSB_RC_INVALID_ARGUMENT);
	}

	/* determine if we have a native netkey IPsec stack */
	if (!starter_netkey_init())
	{
		DBG1(DBG_APP, "no netkey IPsec stack detected");
		if (!starter_klips_init())
		{
			DBG1(DBG_APP, "no KLIPS IPsec stack detected");
			DBG1(DBG_APP, "no known IPsec stack detected, ignoring!");
		}
	}

	last_reload = time_monotonic(NULL);

	if (check_pid(starter_pid_file))
	{
		DBG1(DBG_APP, "starter is already running (%s exists) -- no fork done",
			 starter_pid_file);
		confread_free(cfg);
		cleanup();
		exit(LSB_RC_SUCCESS);
	}

#ifdef GENERATE_SELFCERT
	generate_selfcert();
#endif

	/* fork if we're not debugging stuff */
	if (!no_fork)
	{
		log_to_stderr = FALSE;

		switch (fork())
		{
			case 0:
			{
				int fnull;

				close_log();

				fnull = open("/dev/null", O_RDWR);
				if (fnull >= 0)
				{
					dup2(fnull, STDIN_FILENO);
					dup2(fnull, STDOUT_FILENO);
					dup2(fnull, STDERR_FILENO);
					close(fnull);
				}

				setsid();
				init_log("ipsec_starter");
			}
			break;
			case -1:
				DBG1(DBG_APP, "can't fork: %s", strerror(errno));
				break;
			default:
				confread_free(cfg);
				cleanup();
				exit(LSB_RC_SUCCESS);
		}
	}

	/* save pid file in /var/run/starter[.daemon_name].pid */
	{
		FILE *fd = fopen(starter_pid_file, "w");

		if (fd)
		{
			fprintf(fd, "%u\n", getpid());
			fclose(fd);
		}
	}

	/* we handle these signals only in pselect() */
	memset(&action, 0, sizeof(action));
	sigemptyset(&action.sa_mask);
	sigaddset(&action.sa_mask, SIGHUP);
	sigaddset(&action.sa_mask, SIGINT);
	sigaddset(&action.sa_mask, SIGTERM);
	sigaddset(&action.sa_mask, SIGQUIT);
	sigaddset(&action.sa_mask, SIGALRM);
	sigaddset(&action.sa_mask, SIGUSR1);
	pthread_sigmask(SIG_SETMASK, &action.sa_mask, NULL);

	/* install a handler for fatal signals */
	action.sa_handler = fatal_signal_handler;
	sigaction(SIGSEGV, &action, NULL);
	sigaction(SIGILL, &action, NULL);
	sigaction(SIGBUS, &action, NULL);
	action.sa_handler = SIG_IGN;
	sigaction(SIGPIPE, &action, NULL);

	/* install main signal handler */
	action.sa_handler = signal_handler;
	sigaction(SIGHUP, &action, NULL);
	sigaction(SIGINT, &action, NULL);
	sigaction(SIGTERM, &action, NULL);
	sigaction(SIGQUIT, &action, NULL);
	sigaction(SIGALRM, &action, NULL);
	sigaction(SIGUSR1, &action, NULL);
	/* this is not blocked above as we want to receive it asynchronously */
	sigaction(SIGCHLD, &action, NULL);

	/* empty mask for pselect() call below */
	sigemptyset(&action.sa_mask);

	for (;;)
	{
		/*
		 * Stop charon (if started) and exit
		 */
		if (_action_ & FLAG_ACTION_QUIT)
		{
			if (starter_charon_pid())
			{
				starter_stop_charon();
			}
			starter_netkey_cleanup();
			confread_free(cfg);
			unlink(starter_pid_file);
			cleanup();
			DBG1(DBG_APP, "ipsec starter stopped");
			close_log();
			exit(LSB_RC_SUCCESS);
		}

		/*
		 * Delete all connections. Will be added below
		 */
		if (_action_ & FLAG_ACTION_RELOAD)
		{
			if (starter_charon_pid())
			{
				for (conn = cfg->conn_first; conn; conn = conn->next)
				{
					if (conn->state == STATE_ADDED)
					{
						if (starter_charon_pid())
						{
							if (conn->startup == STARTUP_ROUTE)
							{
								starter_stroke_unroute_conn(conn);
							}
							starter_stroke_del_conn(conn);
						}
						conn->state = STATE_TO_ADD;
					}
				}
				for (ca = cfg->ca_first; ca; ca = ca->next)
				{
					if (ca->state == STATE_ADDED)
					{
						if (starter_charon_pid())
						{
							starter_stroke_del_ca(ca);
						}
						ca->state = STATE_TO_ADD;
					}
				}
			}
			_action_ &= ~FLAG_ACTION_RELOAD;
		}

		/*
		 * Update configuration
		 */
		if (_action_ & FLAG_ACTION_UPDATE)
		{
			DBG2(DBG_APP, "Reloading config...");
			new_cfg = confread_load(config_file);

			if (new_cfg && (new_cfg->err == 0))
			{
				/* Switch to new config. New conn will be loaded below */

				/* Look for new connections that are already loaded */
				for (conn = cfg->conn_first; conn; conn = conn->next)
				{
					if (conn->state == STATE_ADDED)
					{
						for (conn2 = new_cfg->conn_first; conn2; conn2 = conn2->next)
						{
							if (conn2->state == STATE_TO_ADD && starter_cmp_conn(conn, conn2))
							{
								conn->state = STATE_REPLACED;
								conn2->state = STATE_ADDED;
								conn2->id = conn->id;
								break;
							}
						}
					}
				}

				/* Remove conn sections that have become unused */
				for (conn = cfg->conn_first; conn; conn = conn->next)
				{
					if (conn->state == STATE_ADDED)
					{
						if (starter_charon_pid())
						{
							if (conn->startup == STARTUP_ROUTE)
							{
								starter_stroke_unroute_conn(conn);
							}
							starter_stroke_del_conn(conn);
						}
					}
				}

				/* Look for new ca sections that are already loaded */
				for (ca = cfg->ca_first; ca; ca = ca->next)
				{
					if (ca->state == STATE_ADDED)
					{
						for (ca2 = new_cfg->ca_first; ca2; ca2 = ca2->next)
						{
							if (ca2->state == STATE_TO_ADD && starter_cmp_ca(ca, ca2))
							{
								ca->state = STATE_REPLACED;
								ca2->state = STATE_ADDED;
								break;
							}
						}
					}
				}

				/* Remove ca sections that have become unused */
				for (ca = cfg->ca_first; ca; ca = ca->next)
				{
					if (ca->state == STATE_ADDED)
					{
						if (starter_charon_pid())
						{
							starter_stroke_del_ca(ca);
						}
					}
				}
				confread_free(cfg);
				cfg = new_cfg;
			}
			else
			{
				DBG1(DBG_APP, "can't reload config file due to errors -- keeping old one");
				if (new_cfg)
				{
					confread_free(new_cfg);
				}
			}
			_action_ &= ~FLAG_ACTION_UPDATE;
			last_reload = time_monotonic(NULL);
		}

		/*
		 * Start daemon
		 */
		if (_action_ & FLAG_ACTION_START_CHARON)
		{
			if (cfg->setup.charonstart && !starter_charon_pid())
			{
				DBG2(DBG_APP, "Attempting to start %s...", daemon_name);
				if (starter_start_charon(cfg, no_fork, attach_gdb))
				{
					/* schedule next try */
					alarm(CHARON_RESTART_DELAY);
				}
				starter_stroke_configure(cfg);
			}
			_action_ &= ~FLAG_ACTION_START_CHARON;

			for (ca = cfg->ca_first; ca; ca = ca->next)
			{
				if (ca->state == STATE_ADDED)
				{
					ca->state = STATE_TO_ADD;
				}
			}

			for (conn = cfg->conn_first; conn; conn = conn->next)
			{
				if (conn->state == STATE_ADDED)
				{
					conn->state = STATE_TO_ADD;
				}
			}
		}

		/*
		 * Add stale conn and ca sections
		 */
		if (starter_charon_pid())
		{
			for (ca = cfg->ca_first; ca; ca = ca->next)
			{
				if (ca->state == STATE_TO_ADD)
				{
					if (starter_charon_pid())
					{
						starter_stroke_add_ca(ca);
					}
					ca->state = STATE_ADDED;
				}
			}

			for (conn = cfg->conn_first; conn; conn = conn->next)
			{
				if (conn->state == STATE_TO_ADD)
				{
					if (conn->id == 0)
					{
						/* affect new unique id */
						conn->id = id++;
					}
					if (starter_charon_pid())
					{
						starter_stroke_add_conn(cfg, conn);
					}
					conn->state = STATE_ADDED;

					if (conn->startup == STARTUP_START)
					{
						if (starter_charon_pid())
						{
							starter_stroke_initiate_conn(conn);
						}
					}
					else if (conn->startup == STARTUP_ROUTE)
					{
						if (starter_charon_pid())
						{
							starter_stroke_route_conn(conn);
						}
					}
				}
			}
		}

		/*
		 * If auto_update activated, when to stop select
		 */
		if (auto_update)
		{
			time_t now = time_monotonic(NULL);

			ts.tv_sec = (now < last_reload + auto_update) ?
						(last_reload + auto_update - now) : 0;
			ts.tv_nsec = 0;
		}

		/*
		 * Wait for something to happen
		 */
		if (!_action_ &&
			pselect(0, NULL, NULL, NULL, auto_update ? &ts : NULL,
					&action.sa_mask) == 0)
		{
			/* timeout -> auto_update */
			_action_ |= FLAG_ACTION_UPDATE;
		}
	}
	exit(LSB_RC_SUCCESS);
}

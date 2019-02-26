/*
 * Copyright (C) 2006-2018 Tobias Brunner
 * Copyright (C) 2005-2014 Martin Willi
 * Copyright (C) 2006 Daniel Roethlisberger
 * Copyright (C) 2005 Jan Hutter
 * HSR Hochschule fuer Technik Rapperswil
 * Copyright (C) 2014 revosec AG
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

#include <signal.h>
#include <stdio.h>
#include <pthread.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/utsname.h>
#include <unistd.h>
#include <errno.h>

/* won't make sense from our logging hook */
#define SD_JOURNAL_SUPPRESS_LOCATION
#include <systemd/sd-daemon.h>
#include <systemd/sd-journal.h>

#include <daemon.h>

#include <library.h>
#include <utils/backtrace.h>
#include <threading/thread.h>
#include <threading/rwlock.h>

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

typedef struct journal_logger_t journal_logger_t;

/**
 * Logger implementation using systemd-journal
 */
struct journal_logger_t {

	/**
	 * Public interface
	 */
	custom_logger_t public;

	/**
	 * Configured loglevels
	 */
	level_t levels[DBG_MAX];

	/**
	 * Lock for levels
	 */
	rwlock_t *lock;
};

METHOD(logger_t, vlog, void,
	journal_logger_t *this, debug_t group, level_t level, int thread,
	ike_sa_t *ike_sa, const char *fmt, va_list args)
{
	char buf[4096], *msg = buf;
	ssize_t len;
	va_list copy;

	va_copy(copy, args);
	len = vsnprintf(msg, sizeof(buf), fmt, copy);
	va_end(copy);

	if (len >= sizeof(buf))
	{
		len++;
		msg = malloc(len);
		va_copy(copy, args);
		len = vsnprintf(msg, len, fmt, copy);
		va_end(copy);
	}
	if (len > 0)
	{
		char unique[64] = "", name[256] = "";
		int priority;

		if (ike_sa)
		{
			snprintf(unique, sizeof(unique), "IKE_SA_UNIQUE_ID=%u",
					 ike_sa->get_unique_id(ike_sa));
			if (ike_sa->get_peer_cfg(ike_sa))
			{
				snprintf(name, sizeof(name), "IKE_SA_NAME=%s",
						 ike_sa->get_name(ike_sa));
			}
		}
		switch (level)
		{
			case LEVEL_AUDIT:
				priority = LOG_NOTICE;
				break;
			case LEVEL_CTRL:
				priority = LOG_INFO;
				break;
			default:
				priority = LOG_DEBUG;
				break;
		}
		sd_journal_send(
			"MESSAGE=%s", msg,
			"MESSAGE_ID=57d2708c-d607-43bd-8c39-66bf%.8x",
				chunk_hash_static(chunk_from_str((char*)fmt)),
			"PRIORITY=%d", priority,
			"GROUP=%N", debug_names, group,
			"LEVEL=%d", level,
			"THREAD=%d", thread,
			unique[0] ? unique : NULL,
			name[0] ? name : NULL,
			NULL);
	}
	if (msg != buf)
	{
		free(msg);
	}
}

METHOD(logger_t, get_level, level_t,
	journal_logger_t *this, debug_t group)
{
	level_t level;

	this->lock->read_lock(this->lock);
	level = this->levels[group];
	this->lock->unlock(this->lock);

	return level;
}

METHOD(custom_logger_t, set_level, void,
	journal_logger_t *this, debug_t group, level_t level)
{
	this->lock->write_lock(this->lock);
	this->levels[group] = level;
	this->lock->unlock(this->lock);
}

METHOD(custom_logger_t, logger_destroy, void,
	journal_logger_t *this)
{
	this->lock->destroy(this->lock);
	free(this);
}

static custom_logger_t *journal_logger_create(const char *name)
{
	journal_logger_t *this;

	INIT(this,
		.public = {
			.logger = {
				.vlog = _vlog,
				.get_level = _get_level,
			},
			.set_level = _set_level,
			.destroy = _logger_destroy,
		},
		.lock = rwlock_create(RWLOCK_TYPE_DEFAULT),
	);
	return &this->public;
}

/**
 * Run the daemon and handle unix signals
 */
static int run()
{
	sigset_t set;

	sigemptyset(&set);
	sigaddset(&set, SIGHUP);
	sigaddset(&set, SIGTERM);
	sigprocmask(SIG_BLOCK, &set, NULL);

	sd_notify(0, "READY=1\n");

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
			return SS_RC_INITIALIZATION_FAILED;
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
			case SIGTERM:
			{
				DBG1(DBG_DMN, "SIGTERM received, shutting down");
				charon->bus->alert(charon->bus, ALERT_SHUTDOWN_SIGNAL, sig);
				return 0;
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

	name = lib->settings->get_str(lib->settings, "%s.user", IPSEC_USER,
								  lib->ns);
	if (name && !lib->caps->resolve_uid(lib->caps, name))
	{
		return FALSE;
	}
	name = lib->settings->get_str(lib->settings, "%s.group", IPSEC_GROUP,
								  lib->ns);
	if (name && !lib->caps->resolve_gid(lib->caps, name))
	{
		return FALSE;
	}
	return TRUE;
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
	backtrace->log(backtrace, stderr, TRUE);
	backtrace->destroy(backtrace);

	DBG1(DBG_DMN, "killing ourself, received critical signal");
	abort();
}

/**
 * Add namespace alias
 */
static void __attribute__ ((constructor))register_namespace()
{
	/* inherit settings from charon */
	library_add_namespace("charon");
}

/**
 * Register journal logger
 */
static void __attribute__ ((constructor))register_logger()
{
	register_custom_logger("journal", journal_logger_create);
}

/**
 * Main function, starts the daemon.
 */
int main(int argc, char *argv[])
{
	struct sigaction action;
	struct utsname utsname;
	int status = SS_RC_INITIALIZATION_FAILED;

	dbg = dbg_stderr;

	if (uname(&utsname) != 0)
	{
		memset(&utsname, 0, sizeof(utsname));
	}

	sd_notifyf(0, "STATUS=Starting charon-systemd, strongSwan %s, %s %s, %s",
			   VERSION, utsname.sysname, utsname.release, utsname.machine);

	atexit(library_deinit);
	if (!library_init(NULL, "charon-systemd"))
	{
		sd_notifyf(0, "STATUS=libstrongswan initialization failed");
		return SS_RC_INITIALIZATION_FAILED;
	}
	if (lib->integrity &&
		!lib->integrity->check_file(lib->integrity, "charon-systemd", argv[0]))
	{
		sd_notifyf(0, "STATUS=integrity check of charon-systemd failed");
		return SS_RC_INITIALIZATION_FAILED;
	}
	if (!libcharon_init())
	{
		sd_notifyf(0, "STATUS=libcharon initialization failed");
		goto error;
	}
	if (!lookup_uid_gid())
	{
		sd_notifyf(0, "STATUS=unknown uid/gid");
		goto error;
	}
	/* we registered the journal logger as custom logger, which gets its
	 * settings from <ns>.customlog.journal, let it fallback to <ns>.journal */
	lib->settings->add_fallback(lib->settings, "%s.customlog.journal",
								"%s.journal", lib->ns);
	/* load the journal logger by default */
	lib->settings->set_default_str(lib->settings, "%s.journal.default", "1",
								   lib->ns);

	charon->load_loggers(charon);

	if (!charon->initialize(charon,
			lib->settings->get_str(lib->settings, "%s.load", PLUGINS, lib->ns)))
	{
		sd_notifyf(0, "STATUS=charon initialization failed");
		goto error;
	}
	lib->plugins->status(lib->plugins, LEVEL_CTRL);

	if (!lib->caps->drop(lib->caps))
	{
		sd_notifyf(0, "STATUS=dropping capabilities failed");
		goto error;
	}

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

	charon->start(charon);

	sd_notifyf(0, "STATUS=charon-systemd running, strongSwan %s, %s %s, %s",
			   VERSION, utsname.sysname, utsname.release, utsname.machine);

	status = run();

error:
	libcharon_deinit();
	return status;
}

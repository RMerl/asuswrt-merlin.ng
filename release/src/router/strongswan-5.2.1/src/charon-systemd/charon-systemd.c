/*
 * Copyright (C) 2006-2012 Tobias Brunner
 * Copyright (C) 2005-2014 Martin Willi
 * Copyright (C) 2006 Daniel Roethlisberger
 * Copyright (C) 2005 Jan Hutter
 * Hochschule fuer Technik Rapperswil
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

#include <hydra.h>
#include <daemon.h>

#include <library.h>
#include <utils/backtrace.h>
#include <threading/thread.h>
#include <threading/rwlock.h>

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
	 * Implements logger_t
	 */
	logger_t logger;

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

/**
 * Reload journal logger configuration
 */
CALLBACK(journal_reload, bool,
	journal_logger_t **journal)
{
	journal_logger_t *this = *journal;
	debug_t group;
	level_t def;

	def = lib->settings->get_int(lib->settings, "%s.journal.default", 1, lib->ns);

	this->lock->write_lock(this->lock);
	for (group = 0; group < DBG_MAX; group++)
	{
		this->levels[group] =
			lib->settings->get_int(lib->settings,
				"%s.journal.%N", def, lib->ns, debug_lower_names, group);
	}
	this->lock->unlock(this->lock);

	charon->bus->add_logger(charon->bus, &this->logger);

	return TRUE;
}

/**
 * Initialize/deinitialize journal logger
 */
static bool journal_register(void *plugin, plugin_feature_t *feature,
							 bool reg, journal_logger_t **logger)
{
	journal_logger_t *this;

	if (reg)
	{
		INIT(this,
			.logger = {
				.vlog = _vlog,
				.get_level = _get_level,
			},
			.lock = rwlock_create(RWLOCK_TYPE_DEFAULT),
		);

		journal_reload(&this);

		*logger = this;
		return TRUE;
	}
	else
	{
		this = *logger;

		charon->bus->remove_logger(charon->bus, &this->logger);

		this->lock->destroy(this->lock);
		free(this);

		return TRUE;
	}
}

/**
 * Run the daemon and handle unix signals
 */
static int run()
{
	sigset_t set;

	sigemptyset(&set);
	sigaddset(&set, SIGTERM);
	sigprocmask(SIG_BLOCK, &set, NULL);

	sd_notify(0, "READY=1\n");

	while (TRUE)
	{
		int sig, error;

		error = sigwait(&set, &sig);
		if (error)
		{
			DBG1(DBG_DMN, "waiting for signal failed: %s", strerror(error));
			return SS_RC_INITIALIZATION_FAILED;
		}
		switch (sig)
		{
			case SIGTERM:
			{
				DBG1(DBG_DMN, "SIGTERM received, shutting down");
				charon->bus->alert(charon->bus, ALERT_SHUTDOWN_SIGNAL, sig);
				return 0;
			}
			default:
			{
				DBG1(DBG_DMN, "unknown signal %d received. Ignored", sig);
				break;
			}
		}
	}
}

/**
 * lookup UID and GID
 */
static bool lookup_uid_gid()
{
#ifdef IPSEC_USER
	if (!lib->caps->resolve_uid(lib->caps, IPSEC_USER))
	{
		return FALSE;
	}
#endif /* IPSEC_USER */
#ifdef IPSEC_GROUP
	if (!lib->caps->resolve_gid(lib->caps, IPSEC_GROUP))
	{
		return FALSE;
	}
#endif /* IPSEC_GROUP */
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
 * The journal logger instance
 */
static journal_logger_t *journal;

/**
 * Journal static features
 */
static plugin_feature_t features[] = {
	PLUGIN_CALLBACK((plugin_feature_callback_t)journal_register, &journal),
		PLUGIN_PROVIDE(CUSTOM, "systemd-journal"),
};

/**
 * Main function, starts the daemon.
 */
int main(int argc, char *argv[])
{
	struct sigaction action;
	struct utsname utsname;

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
	atexit(libhydra_deinit);
	if (!libhydra_init())
	{
		sd_notifyf(0, "STATUS=libhydra initialization failed");
		return SS_RC_INITIALIZATION_FAILED;
	}
	atexit(libcharon_deinit);
	if (!libcharon_init())
	{
		sd_notifyf(0, "STATUS=libcharon initialization failed");
		return SS_RC_INITIALIZATION_FAILED;
	}
	if (!lookup_uid_gid())
	{
		sd_notifyf(0, "STATUS=unknown uid/gid");
		return SS_RC_INITIALIZATION_FAILED;
	}
	charon->load_loggers(charon, NULL, FALSE);

	lib->plugins->add_static_features(lib->plugins, lib->ns, features,
							countof(features), TRUE, journal_reload, &journal);

	if (!charon->initialize(charon, PLUGINS))
	{
		sd_notifyf(0, "STATUS=charon initialization failed");
		return SS_RC_INITIALIZATION_FAILED;
	}
	lib->plugins->status(lib->plugins, LEVEL_CTRL);

	if (!lib->caps->drop(lib->caps))
	{
		sd_notifyf(0, "STATUS=dropping capabilities failed");
		return SS_RC_INITIALIZATION_FAILED;
	}

	/* add handler for SEGV and ILL,
	 * INT, TERM and HUP are handled by sigwait() in run() */
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

	return run();
}

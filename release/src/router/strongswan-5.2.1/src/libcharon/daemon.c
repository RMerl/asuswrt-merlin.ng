/*
 * Copyright (C) 2006-2012 Tobias Brunner
 * Copyright (C) 2005-2009 Martin Willi
 * Copyright (C) 2006 Daniel Roethlisberger
 * Copyright (C) 2005 Jan Hutter
 * Hochschule fuer Technik Rapperswil
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
#include <sys/types.h>
#include <unistd.h>
#include <time.h>
#include <errno.h>

#ifdef HAVE_SYSLOG
#include <syslog.h>
#endif

#include "daemon.h"

#include <library.h>
#include <bus/listeners/sys_logger.h>
#include <bus/listeners/file_logger.h>
#include <config/proposal.h>
#include <plugins/plugin_feature.h>
#include <kernel/kernel_handler.h>
#include <processing/jobs/start_action_job.h>
#include <threading/mutex.h>

#ifndef LOG_AUTHPRIV /* not defined on OpenSolaris */
#define LOG_AUTHPRIV LOG_AUTH
#endif

typedef struct private_daemon_t private_daemon_t;

/**
 * Private additions to daemon_t, contains threads and internal functions.
 */
struct private_daemon_t {
	/**
	 * Public members of daemon_t.
	 */
	daemon_t public;

	/**
	 * Handler for kernel events
	 */
	kernel_handler_t *kernel_handler;

	/**
	 * A list of installed loggers (as logger_entry_t*)
	 */
	linked_list_t *loggers;

	/**
	 * Identifier used for syslog (in the openlog call)
	 */
	char *syslog_identifier;

	/**
	 * Mutex for configured loggers
	 */
	mutex_t *mutex;

	/**
	 * Integrity check failed?
	 */
	bool integrity_failed;

	/**
	 * Number of times we have been initialized
	 */
	refcount_t ref;
};

/**
 * One and only instance of the daemon.
 */
daemon_t *charon;

/**
 * hook in library for debugging messages
 */
extern void (*dbg) (debug_t group, level_t level, char *fmt, ...);

/**
 * we store the previous debug function so we can reset it
 */
static void (*dbg_old) (debug_t group, level_t level, char *fmt, ...);

/**
 * Logging hook for library logs, spreads debug message over bus
 */
static void dbg_bus(debug_t group, level_t level, char *fmt, ...)
{
	va_list args;

	va_start(args, fmt);
	charon->bus->vlog(charon->bus, group, level, fmt, args);
	va_end(args);
}

/**
 * Some metadata about configured loggers
 */
typedef struct {
	/**
	 * Target of the logger (syslog facility or filename)
	 */
	char *target;

	/**
	 * TRUE if this is a file logger
	 */
	bool file;

	/**
	 * The actual logger
	 */
	union {
		sys_logger_t *sys;
		file_logger_t *file;
	} logger;

} logger_entry_t;

/**
 * Destroy a logger entry
 */
static void logger_entry_destroy(logger_entry_t *this)
{
	if (this->file)
	{
		DESTROY_IF(this->logger.file);
	}
	else
	{
		DESTROY_IF(this->logger.sys);
	}
	free(this->target);
	free(this);
}

/**
 * Unregister and destroy a logger entry
 */
static void logger_entry_unregister_destroy(logger_entry_t *this)
{
	if (this->file)
	{
		charon->bus->remove_logger(charon->bus, &this->logger.file->logger);
	}
	else
	{
		charon->bus->remove_logger(charon->bus, &this->logger.sys->logger);
	}
	logger_entry_destroy(this);
}

/**
 * Match a logger entry by target and whether it is a file or syslog logger
 */
static bool logger_entry_match(logger_entry_t *this, char *target, bool *file)
{
	return this->file == *file && streq(this->target, target);
}

/**
 * Handle configured syslog identifier
 *
 * mutex must be locked when calling this function
 */
static void handle_syslog_identifier(private_daemon_t *this)
{
#ifdef HAVE_SYSLOG
	char *identifier;

	identifier = lib->settings->get_str(lib->settings, "%s.syslog.identifier",
										NULL, lib->ns);
	if (identifier)
	{	/* set identifier, which is prepended to each log line */
		if (!this->syslog_identifier ||
			!streq(identifier, this->syslog_identifier))
		{
			closelog();
			this->syslog_identifier = identifier;
			openlog(this->syslog_identifier, 0, 0);
		}
	}
	else if (this->syslog_identifier)
	{
		closelog();
		this->syslog_identifier = NULL;
	}
#endif /* HAVE_SYSLOG */
}

/**
 * Convert the given string into a syslog facility, returns -1 if the facility
 * is not supported
 */
static int get_syslog_facility(char *facility)
{
#ifdef HAVE_SYSLOG
	if (streq(facility, "daemon"))
	{
		return LOG_DAEMON;
	}
	else if (streq(facility, "auth"))
	{
		return LOG_AUTHPRIV;
	}
#endif /* HAVE_SYSLOG */
	return -1;
}

/**
 * Returns an existing or newly created logger entry (if found, it is removed
 * from the given linked list of existing loggers)
 */
static logger_entry_t *get_logger_entry(char *target, bool is_file_logger,
										linked_list_t *existing)
{
	logger_entry_t *entry;

	if (existing->find_first(existing, (void*)logger_entry_match,
							(void**)&entry, target, &is_file_logger) != SUCCESS)
	{
		INIT(entry,
			.target = strdup(target),
			.file = is_file_logger,
		);
		if (is_file_logger)
		{
			entry->logger.file = file_logger_create(target);
		}
#ifdef HAVE_SYSLOG
		else
		{
			entry->logger.sys = sys_logger_create(get_syslog_facility(target));
		}
#endif /* HAVE_SYSLOG */
	}
	else
	{
		existing->remove(existing, entry, NULL);
	}
	return entry;
}

/**
 * Create or reuse a syslog logger
 */
static sys_logger_t *add_sys_logger(private_daemon_t *this, char *facility,
									linked_list_t *current_loggers)
{
	logger_entry_t *entry;

	entry = get_logger_entry(facility, FALSE, current_loggers);
	this->loggers->insert_last(this->loggers, entry);
	return entry->logger.sys;
}

/**
 * Create or reuse a file logger
 */
static file_logger_t *add_file_logger(private_daemon_t *this, char *filename,
									  linked_list_t *current_loggers)
{
	logger_entry_t *entry;

	entry = get_logger_entry(filename, TRUE, current_loggers);
	this->loggers->insert_last(this->loggers, entry);
	return entry->logger.file;
}

/**
 * Load the given syslog logger configured in strongswan.conf
 */
static void load_sys_logger(private_daemon_t *this, char *facility,
							linked_list_t *current_loggers)
{
	sys_logger_t *sys_logger;
	debug_t group;
	level_t def;

	if (get_syslog_facility(facility) == -1)
	{
		return;
	}

	sys_logger = add_sys_logger(this, facility, current_loggers);
	sys_logger->set_options(sys_logger,
				lib->settings->get_bool(lib->settings, "%s.syslog.%s.ike_name",
										FALSE, lib->ns, facility));

	def = lib->settings->get_int(lib->settings, "%s.syslog.%s.default", 1,
								 lib->ns, facility);
	for (group = 0; group < DBG_MAX; group++)
	{
		sys_logger->set_level(sys_logger, group,
				lib->settings->get_int(lib->settings, "%s.syslog.%s.%N", def,
							lib->ns, facility, debug_lower_names, group));
	}
	charon->bus->add_logger(charon->bus, &sys_logger->logger);
}

/**
 * Load the given file logger configured in strongswan.conf
 */
static void load_file_logger(private_daemon_t *this, char *filename,
							 linked_list_t *current_loggers)
{
	file_logger_t *file_logger;
	debug_t group;
	level_t def;
	bool ike_name, flush_line, append;
	char *time_format;

	time_format = lib->settings->get_str(lib->settings,
						"%s.filelog.%s.time_format", NULL, lib->ns, filename);
	ike_name = lib->settings->get_bool(lib->settings,
						"%s.filelog.%s.ike_name", FALSE, lib->ns, filename);
	flush_line = lib->settings->get_bool(lib->settings,
						"%s.filelog.%s.flush_line", FALSE, lib->ns, filename);
	append = lib->settings->get_bool(lib->settings,
						"%s.filelog.%s.append", TRUE, lib->ns, filename);

	file_logger = add_file_logger(this, filename, current_loggers);
	file_logger->set_options(file_logger, time_format, ike_name);
	file_logger->open(file_logger, flush_line, append);

	def = lib->settings->get_int(lib->settings, "%s.filelog.%s.default", 1,
								 lib->ns, filename);
	for (group = 0; group < DBG_MAX; group++)
	{
		file_logger->set_level(file_logger, group,
				lib->settings->get_int(lib->settings, "%s.filelog.%s.%N", def,
							lib->ns, filename, debug_lower_names, group));
	}
	charon->bus->add_logger(charon->bus, &file_logger->logger);
}

METHOD(daemon_t, load_loggers, void,
	private_daemon_t *this, level_t levels[DBG_MAX], bool to_stderr)
{
	enumerator_t *enumerator;
	linked_list_t *current_loggers;
	char *target;

	this->mutex->lock(this->mutex);
	handle_syslog_identifier(this);
	current_loggers = this->loggers;
	this->loggers = linked_list_create();
	enumerator = lib->settings->create_section_enumerator(lib->settings,
														"%s.syslog", lib->ns);
	while (enumerator->enumerate(enumerator, &target))
	{
		load_sys_logger(this, target, current_loggers);
	}
	enumerator->destroy(enumerator);

	enumerator = lib->settings->create_section_enumerator(lib->settings,
														"%s.filelog", lib->ns);
	while (enumerator->enumerate(enumerator, &target))
	{
		load_file_logger(this, target, current_loggers);
	}
	enumerator->destroy(enumerator);

	if (!this->loggers->get_count(this->loggers) && levels)
	{	/* setup legacy style default loggers configured via command-line */
		file_logger_t *file_logger;
		sys_logger_t *sys_logger;
		debug_t group;

		sys_logger = add_sys_logger(this, "daemon", current_loggers);
		file_logger = add_file_logger(this, "stdout", current_loggers);
		file_logger->open(file_logger, FALSE, FALSE);

		for (group = 0; group < DBG_MAX; group++)
		{
			if (sys_logger)
			{
				sys_logger->set_level(sys_logger, group, levels[group]);
			}
			if (to_stderr)
			{
				file_logger->set_level(file_logger, group, levels[group]);
			}
		}
		if (sys_logger)
		{
			charon->bus->add_logger(charon->bus, &sys_logger->logger);
		}
		charon->bus->add_logger(charon->bus, &file_logger->logger);

		sys_logger = add_sys_logger(this, "auth", current_loggers);
		if (sys_logger)
		{
			sys_logger->set_level(sys_logger, DBG_ANY, LEVEL_AUDIT);
			charon->bus->add_logger(charon->bus, &sys_logger->logger);
		}
	}
	/* unregister and destroy any unused remaining loggers */
	current_loggers->destroy_function(current_loggers,
									 (void*)logger_entry_unregister_destroy);
	this->mutex->unlock(this->mutex);
}

METHOD(daemon_t, set_level, void,
	private_daemon_t *this, debug_t group, level_t level)
{
	enumerator_t *enumerator;
	logger_entry_t *entry;

	/* we set the loglevel on ALL sys- and file-loggers */
	this->mutex->lock(this->mutex);
	enumerator = this->loggers->create_enumerator(this->loggers);
	while (enumerator->enumerate(enumerator, &entry))
	{
		if (entry->file)
		{
			entry->logger.file->set_level(entry->logger.file, group, level);
			charon->bus->add_logger(charon->bus, &entry->logger.file->logger);
		}
		else
		{
			entry->logger.sys->set_level(entry->logger.sys, group, level);
			charon->bus->add_logger(charon->bus, &entry->logger.sys->logger);
		}
	}
	enumerator->destroy(enumerator);
	this->mutex->unlock(this->mutex);
}

/**
 * Clean up all daemon resources
 */
static void destroy(private_daemon_t *this)
{
	/* terminate all idle threads */
	lib->processor->set_threads(lib->processor, 0);
	/* make sure nobody waits for a DNS query */
	lib->hosts->flush(lib->hosts);
	/* close all IKE_SAs */
	if (this->public.ike_sa_manager)
	{
		this->public.ike_sa_manager->flush(this->public.ike_sa_manager);
	}
	if (this->public.traps)
	{
		this->public.traps->flush(this->public.traps);
	}
	if (this->public.sender)
	{
		this->public.sender->flush(this->public.sender);
	}

	/* cancel all threads and wait for their termination */
	lib->processor->cancel(lib->processor);

#ifdef ME
	DESTROY_IF(this->public.connect_manager);
	DESTROY_IF(this->public.mediation_manager);
#endif /* ME */
	/* make sure the cache is clear before unloading plugins */
	lib->credmgr->flush_cache(lib->credmgr, CERT_ANY);
	lib->plugins->unload(lib->plugins);
	DESTROY_IF(this->kernel_handler);
	DESTROY_IF(this->public.traps);
	DESTROY_IF(this->public.shunts);
	DESTROY_IF(this->public.ike_sa_manager);
	DESTROY_IF(this->public.controller);
	DESTROY_IF(this->public.eap);
	DESTROY_IF(this->public.xauth);
	DESTROY_IF(this->public.backends);
	DESTROY_IF(this->public.socket);

	/* rehook library logging, shutdown logging */
	dbg = dbg_old;
	DESTROY_IF(this->public.bus);
	this->loggers->destroy_function(this->loggers, (void*)logger_entry_destroy);
	this->mutex->destroy(this->mutex);
	free(this);
}

/**
 * Run a set of configured scripts
 */
static void run_scripts(private_daemon_t *this, char *verb)
{
	enumerator_t *enumerator;
	char *key, *value, *pos, buf[1024];
	FILE *cmd;

	enumerator = lib->settings->create_key_value_enumerator(lib->settings,
												"%s.%s-scripts", lib->ns, verb);
	while (enumerator->enumerate(enumerator, &key, &value))
	{
		DBG1(DBG_DMN, "executing %s script '%s' (%s):", verb, key, value);
		cmd = popen(value, "r");
		if (!cmd)
		{
			DBG1(DBG_DMN, "executing %s script '%s' (%s) failed: %s",
				 verb, key, value, strerror(errno));
			continue;
		}
		while (TRUE)
		{
			if (!fgets(buf, sizeof(buf), cmd))
			{
				if (ferror(cmd))
				{
					DBG1(DBG_DMN, "reading from %s script '%s' (%s) failed",
						 verb, key, value);
				}
				break;
			}
			else
			{
				pos = buf + strlen(buf);
				if (pos > buf && pos[-1] == '\n')
				{
					pos[-1] = '\0';
				}
				DBG1(DBG_DMN, "%s: %s", key, buf);
			}
		}
		pclose(cmd);
	}
	enumerator->destroy(enumerator);
}

METHOD(daemon_t, start, void,
	   private_daemon_t *this)
{
	/* start the engine, go multithreaded */
	lib->processor->set_threads(lib->processor,
						lib->settings->get_int(lib->settings, "%s.threads",
											   DEFAULT_THREADS, lib->ns));

	run_scripts(this, "start");
}


/**
 * Initialize/deinitialize sender and receiver
 */
static bool sender_receiver_cb(void *plugin, plugin_feature_t *feature,
							   bool reg, private_daemon_t *this)
{
	if (reg)
	{
		this->public.receiver = receiver_create();
		if (!this->public.receiver)
		{
			return FALSE;
		}
		this->public.sender = sender_create();
	}
	else
	{
		DESTROY_IF(this->public.receiver);
		DESTROY_IF(this->public.sender);
	}
	return TRUE;
}

METHOD(daemon_t, initialize, bool,
	private_daemon_t *this, char *plugins)
{
	plugin_feature_t features[] = {
		PLUGIN_PROVIDE(CUSTOM, "libcharon"),
			PLUGIN_DEPENDS(NONCE_GEN),
			PLUGIN_DEPENDS(CUSTOM, "libcharon-receiver"),
			PLUGIN_DEPENDS(CUSTOM, "kernel-ipsec"),
			PLUGIN_DEPENDS(CUSTOM, "kernel-net"),
		PLUGIN_CALLBACK((plugin_feature_callback_t)sender_receiver_cb, this),
			PLUGIN_PROVIDE(CUSTOM, "libcharon-receiver"),
				PLUGIN_DEPENDS(HASHER, HASH_SHA1),
				PLUGIN_DEPENDS(RNG, RNG_STRONG),
				PLUGIN_DEPENDS(CUSTOM, "socket"),
	};
	lib->plugins->add_static_features(lib->plugins, lib->ns, features,
									  countof(features), TRUE, NULL, NULL);

	/* load plugins, further infrastructure may need it */
	if (!lib->plugins->load(lib->plugins, plugins))
	{
		return FALSE;
	}

	this->public.ike_sa_manager = ike_sa_manager_create();
	if (this->public.ike_sa_manager == NULL)
	{
		return FALSE;
	}

	/* Queue start_action job */
	lib->processor->queue_job(lib->processor, (job_t*)start_action_job_create());

#ifdef ME
	this->public.connect_manager = connect_manager_create();
	if (this->public.connect_manager == NULL)
	{
		return FALSE;
	}
	this->public.mediation_manager = mediation_manager_create();
#endif /* ME */

	return TRUE;
}

/**
 * Create the daemon.
 */
private_daemon_t *daemon_create()
{
	private_daemon_t *this;

	INIT(this,
		.public = {
			.initialize = _initialize,
			.start = _start,
			.load_loggers = _load_loggers,
			.set_level = _set_level,
			.bus = bus_create(),
		},
		.loggers = linked_list_create(),
		.mutex = mutex_create(MUTEX_TYPE_DEFAULT),
		.ref = 1,
	);
	charon = &this->public;
	this->public.controller = controller_create();
	this->public.eap = eap_manager_create();
	this->public.xauth = xauth_manager_create();
	this->public.backends = backend_manager_create();
	this->public.socket = socket_manager_create();
	this->public.traps = trap_manager_create();
	this->public.shunts = shunt_manager_create();
	this->kernel_handler = kernel_handler_create();

	return this;
}

/**
 * Described in header.
 */
void libcharon_deinit()
{
	private_daemon_t *this = (private_daemon_t*)charon;

	if (!this || !ref_put(&this->ref))
	{	/* have more users */
		return;
	}

	run_scripts(this, "stop");

	destroy(this);
	charon = NULL;
}

/**
 * Described in header.
 */
bool libcharon_init()
{
	private_daemon_t *this;

	if (charon)
	{	/* already initialized, increase refcount */
		this = (private_daemon_t*)charon;
		ref_get(&this->ref);
		return !this->integrity_failed;
	}

	this = daemon_create();

	/* for uncritical pseudo random numbers */
	srandom(time(NULL) + getpid());

	/* set up hook to log dbg message in library via charons message bus */
	dbg_old = dbg;
	dbg = dbg_bus;

	lib->printf_hook->add_handler(lib->printf_hook, 'P',
								  proposal_printf_hook,
								  PRINTF_HOOK_ARGTYPE_POINTER,
								  PRINTF_HOOK_ARGTYPE_END);

	if (lib->integrity &&
		!lib->integrity->check(lib->integrity, "libcharon", libcharon_init))
	{
		dbg(DBG_DMN, 1, "integrity check of libcharon failed");
		this->integrity_failed = TRUE;
	}
	return !this->integrity_failed;
}

/*
 * Copyright (C) 2006-2017 Tobias Brunner
 * Copyright (C) 2005-2009 Martin Willi
 * Copyright (C) 2006 Daniel Roethlisberger
 * Copyright (C) 2005 Jan Hutter
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

/*
 * Copyright (C) 2016 secunet Security Networks AG
 * Copyright (C) 2016 Thomas Egerer
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
 * THE SOFTWARE.
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
#include <collections/array.h>
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
	 * Cached log levels for default loggers
	 */
	level_t *levels;

	/**
	 * Whether to log to stdout/err by default
	 */
	bool to_stderr;

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
 * Register plugins if built statically
 */
#ifdef STATIC_PLUGIN_CONSTRUCTORS
#include "plugin_constructors.c"
#endif

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
 * Data for registered custom loggers
 */
typedef struct {
	/**
	 * Name of the custom logger (also used for loglevel configuration)
	 */
	char *name;

	/**
	 * Constructor to be called for custom logger creation
	 */
	custom_logger_constructor_t constructor;

} custom_logger_entry_t;

#define MAX_CUSTOM_LOGGERS 10

/**
 * Static array for logger registration using __attribute__((constructor))
 */
static custom_logger_entry_t custom_loggers[MAX_CUSTOM_LOGGERS];
static int custom_logger_count;

/**
 * Described in header
 */
void register_custom_logger(char *name,
							custom_logger_constructor_t constructor)
{
	if (custom_logger_count < MAX_CUSTOM_LOGGERS - 1)
	{
		custom_loggers[custom_logger_count].name = name;
		custom_loggers[custom_logger_count].constructor = constructor;
		custom_logger_count++;
	}
	else
	{
		fprintf(stderr, "failed to register custom logger, please increase "
				"MAX_CUSTOM_LOGGERS");
	}
}

/**
 * Types of supported loggers
 */
typedef enum {
	/**
	 * Syslog logger instance
	 */
	SYS_LOGGER,

	/**
	 * File logger instance
	 */
	FILE_LOGGER,

	/**
	 * Custom logger instance
	 */
	CUSTOM_LOGGER,

} logger_type_t;

/**
 * Some metadata about configured loggers
 */
typedef struct {
	/**
	 * Target of the logger (syslog facility or filename)
	 */
	char *target;

	/**
	 * Type of logger
	 */
	logger_type_t type;

	/**
	 * The actual logger
	 */
	union {
		sys_logger_t *sys;
		file_logger_t *file;
		custom_logger_t *custom;
	} logger;

} logger_entry_t;

/**
 * Destroy a logger entry
 */
static void logger_entry_destroy(logger_entry_t *this)
{
	switch (this->type)
	{
		case FILE_LOGGER:
			DESTROY_IF(this->logger.file);
			break;
		case SYS_LOGGER:
			DESTROY_IF(this->logger.sys);
			break;
		case CUSTOM_LOGGER:
			DESTROY_IF(this->logger.custom);
			break;
	}
	free(this->target);
	free(this);
}

/**
 * Unregister and destroy a logger entry
 */
static void logger_entry_unregister_destroy(logger_entry_t *this)
{
	switch (this->type)
	{
		case FILE_LOGGER:
			charon->bus->remove_logger(charon->bus, &this->logger.file->logger);
			break;
		case SYS_LOGGER:
			charon->bus->remove_logger(charon->bus, &this->logger.sys->logger);
			break;
		case CUSTOM_LOGGER:
			charon->bus->remove_logger(charon->bus,
									   &this->logger.custom->logger);
			break;
	}
	logger_entry_destroy(this);
}

CALLBACK(logger_entry_match, bool,
	logger_entry_t *this, va_list args)
{
	logger_type_t type;
	char *target;

	VA_ARGS_VGET(args, target, type);
	return this->type == type && streq(this->target, target);
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
static logger_entry_t *get_logger_entry(char *target, logger_type_t type,
										linked_list_t *existing,
										custom_logger_constructor_t constructor)
{
	logger_entry_t *entry;

	if (!existing->find_first(existing, logger_entry_match, (void**)&entry,
							  target, type))
	{
		INIT(entry,
			.target = strdup(target),
			.type = type,
		);
		switch (type)
		{
			case FILE_LOGGER:
				entry->logger.file = file_logger_create(target);
				break;
			case SYS_LOGGER:
#ifdef HAVE_SYSLOG
				entry->logger.sys = sys_logger_create(
												get_syslog_facility(target));
				break;
#else
				free(entry);
				return NULL;
#endif /* HAVE_SYSLOG */
			case CUSTOM_LOGGER:
				if (constructor)
				{
					entry->logger.custom = constructor(target);
				}
				if (!entry->logger.custom)
				{
					free(entry);
					return NULL;
				}
				break;
		}
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

	entry = get_logger_entry(facility, SYS_LOGGER, current_loggers, NULL);
	if (entry)
	{
		this->loggers->insert_last(this->loggers, entry);
	}
	return entry ? entry->logger.sys : NULL;
}

/**
 * Create or reuse a file logger
 */
static file_logger_t *add_file_logger(private_daemon_t *this, char *filename,
									  linked_list_t *current_loggers)
{
	logger_entry_t *entry;

	entry = get_logger_entry(filename, FILE_LOGGER, current_loggers, NULL);
	if (entry)
	{
		this->loggers->insert_last(this->loggers, entry);
	}
	return entry ? entry->logger.file : NULL;
}

 /**
 * Create or reuse a custom logger
 */
static custom_logger_t *add_custom_logger(private_daemon_t *this,
										  custom_logger_entry_t *custom,
										  linked_list_t *current_loggers)
{
	logger_entry_t *entry;

	entry = get_logger_entry(custom->name, CUSTOM_LOGGER, current_loggers,
							 custom->constructor);
	if (entry)
	{
		this->loggers->insert_last(this->loggers, entry);
	}
	return entry ? entry->logger.custom : NULL;
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
	if (!sys_logger)
	{
		return;
	}

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
static void load_file_logger(private_daemon_t *this, char *section,
							 linked_list_t *current_loggers)
{
	file_logger_t *file_logger;
	debug_t group;
	level_t def;
	bool add_ms, ike_name, flush_line, append;
	char *time_format, *filename;

	time_format = lib->settings->get_str(lib->settings,
						"%s.filelog.%s.time_format", NULL, lib->ns, section);
	add_ms = lib->settings->get_bool(lib->settings,
						"%s.filelog.%s.time_add_ms", FALSE, lib->ns, section);
	ike_name = lib->settings->get_bool(lib->settings,
						"%s.filelog.%s.ike_name", FALSE, lib->ns, section);
	flush_line = lib->settings->get_bool(lib->settings,
						"%s.filelog.%s.flush_line", FALSE, lib->ns, section);
	append = lib->settings->get_bool(lib->settings,
						"%s.filelog.%s.append", TRUE, lib->ns, section);
	filename = lib->settings->get_str(lib->settings,
						"%s.filelog.%s.path", section, lib->ns, section);

	file_logger = add_file_logger(this, filename, current_loggers);
	if (!file_logger)
	{
		return;
	}

	file_logger->set_options(file_logger, time_format, add_ms, ike_name);
	file_logger->open(file_logger, flush_line, append);

	def = lib->settings->get_int(lib->settings, "%s.filelog.%s.default", 1,
								 lib->ns, section);
	for (group = 0; group < DBG_MAX; group++)
	{
		file_logger->set_level(file_logger, group,
				lib->settings->get_int(lib->settings, "%s.filelog.%s.%N", def,
							lib->ns, section, debug_lower_names, group));
	}
	charon->bus->add_logger(charon->bus, &file_logger->logger);
}

/**
 * Load the given custom logger configured in strongswan.conf
 */
static void load_custom_logger(private_daemon_t *this,
							   custom_logger_entry_t *entry,
							   linked_list_t *current_loggers)
{
	custom_logger_t *custom_logger;
	debug_t group;
	level_t def;

	custom_logger = add_custom_logger(this, entry, current_loggers);
	if (!custom_logger)
	{
		return;
	}

	def = lib->settings->get_int(lib->settings, "%s.customlog.%s.default", 1,
								 lib->ns, entry->name);
	for (group = 0; group < DBG_MAX; group++)
	{
		custom_logger->set_level(custom_logger, group,
				lib->settings->get_int(lib->settings, "%s.customlog.%s.%N", def,
							lib->ns, entry->name, debug_lower_names, group));
	}
	if (custom_logger->reload)
	{
		custom_logger->reload(custom_logger);
	}
	charon->bus->add_logger(charon->bus, &custom_logger->logger);
}

METHOD(daemon_t, load_loggers, void,
	private_daemon_t *this)
{
	enumerator_t *enumerator;
	linked_list_t *current_loggers;
	char *target;
	int i;

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

	for (i = 0; i < custom_logger_count; ++i)
	{
		load_custom_logger(this, &custom_loggers[i], current_loggers);
	}

	if (!this->loggers->get_count(this->loggers) && this->levels)
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
				sys_logger->set_level(sys_logger, group, this->levels[group]);
			}
			if (this->to_stderr)
			{
				file_logger->set_level(file_logger, group, this->levels[group]);
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

METHOD(daemon_t, set_default_loggers, void,
	private_daemon_t *this, level_t levels[DBG_MAX], bool to_stderr)
{
	debug_t group;

	this->mutex->lock(this->mutex);
	if (!levels)
	{
		free(this->levels);
		this->levels = NULL;
	}
	else
	{
		if (!this->levels)
		{
			this->levels = calloc(sizeof(level_t), DBG_MAX);
		}
		for (group = 0; group < DBG_MAX; group++)
		{
			this->levels[group] = levels[group];
		}
		this->to_stderr = to_stderr;
	}
	this->mutex->unlock(this->mutex);
}

METHOD(daemon_t, set_level, void,
	private_daemon_t *this, debug_t group, level_t level)
{
	enumerator_t *enumerator;
	logger_entry_t *entry;

	/* we set the loglevel on ALL loggers */
	this->mutex->lock(this->mutex);
	enumerator = this->loggers->create_enumerator(this->loggers);
	while (enumerator->enumerate(enumerator, &entry))
	{
		switch (entry->type)
		{
			case FILE_LOGGER:
				entry->logger.file->set_level(entry->logger.file, group, level);
				charon->bus->add_logger(charon->bus,
										&entry->logger.file->logger);
				break;
			case SYS_LOGGER:
				entry->logger.sys->set_level(entry->logger.sys, group, level);
				charon->bus->add_logger(charon->bus,
										&entry->logger.sys->logger);
				break;
			case CUSTOM_LOGGER:
				entry->logger.custom->set_level(entry->logger.custom, group,
												level);
				charon->bus->add_logger(charon->bus,
										&entry->logger.sys->logger);
				break;
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
	if (this->public.shunts)
	{
		this->public.shunts->flush(this->public.shunts);
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
	/* make sure the cache and scheduler are clear before unloading plugins */
	lib->credmgr->flush_cache(lib->credmgr, CERT_ANY);
	lib->scheduler->flush(lib->scheduler);
	lib->plugins->unload(lib->plugins);
	DESTROY_IF(this->public.attributes);
	DESTROY_IF(this->kernel_handler);
	DESTROY_IF(this->public.traps);
	DESTROY_IF(this->public.shunts);
	DESTROY_IF(this->public.redirect);
	DESTROY_IF(this->public.controller);
	DESTROY_IF(this->public.eap);
	DESTROY_IF(this->public.xauth);
	DESTROY_IF(this->public.backends);
	DESTROY_IF(this->public.socket);
	DESTROY_IF(this->public.kernel);

	/* rehook library logging, shutdown logging */
	dbg = dbg_old;
	DESTROY_IF(this->public.bus);
	this->loggers->destroy_function(this->loggers, (void*)logger_entry_destroy);
	this->mutex->destroy(this->mutex);
	free(this->levels);
	free(this);
}

/**
 * Run a set of configured scripts
 */
static void run_scripts(private_daemon_t *this, char *verb)
{
	struct {
		char *name;
		char *path;
	} *script;
	array_t *scripts = NULL;
	enumerator_t *enumerator;
	char *key, *value, *pos, buf[1024];
	FILE *cmd;

	/* copy the scripts so we don't hold any locks while executing them */
	enumerator = lib->settings->create_key_value_enumerator(lib->settings,
												"%s.%s-scripts", lib->ns, verb);
	while (enumerator->enumerate(enumerator, &key, &value))
	{
		INIT(script,
			.name = key,
			.path = value,
		);
		array_insert_create(&scripts, ARRAY_TAIL, script);
	}
	enumerator->destroy(enumerator);

	enumerator = array_create_enumerator(scripts);
	while (enumerator->enumerate(enumerator, &script))
	{
		DBG1(DBG_DMN, "executing %s script '%s' (%s)", verb, script->name,
			 script->path);
		cmd = popen(script->path, "r");
		if (!cmd)
		{
			DBG1(DBG_DMN, "executing %s script '%s' (%s) failed: %s",
				 verb, script->name, script->path, strerror(errno));
		}
		else
		{
			while (TRUE)
			{
				if (!fgets(buf, sizeof(buf), cmd))
				{
					if (ferror(cmd))
					{
						DBG1(DBG_DMN, "reading from %s script '%s' (%s) failed",
							 verb, script->name, script->path);
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
					DBG1(DBG_DMN, "%s: %s", script->name, buf);
				}
			}
			pclose(cmd);
		}
		free(script);
	}
	enumerator->destroy(enumerator);
	array_destroy(scripts);
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

/**
 * Initialize/deinitialize IKE_SA/CHILD_SA managers
 */
static bool sa_managers_cb(void *plugin, plugin_feature_t *feature,
						   bool reg, private_daemon_t *this)
{
	if (reg)
	{
		this->public.ike_sa_manager = ike_sa_manager_create();
		if (!this->public.ike_sa_manager)
		{
			return FALSE;
		}
		this->public.child_sa_manager = child_sa_manager_create();
	}
	else
	{
		DESTROY_IF(this->public.ike_sa_manager);
		DESTROY_IF(this->public.child_sa_manager);
	}
	return TRUE;
}

METHOD(daemon_t, initialize, bool,
	private_daemon_t *this, char *plugins)
{
	plugin_feature_t features[] = {
		PLUGIN_PROVIDE(CUSTOM, "libcharon"),
			PLUGIN_DEPENDS(NONCE_GEN),
			PLUGIN_DEPENDS(CUSTOM, "libcharon-sa-managers"),
			PLUGIN_DEPENDS(CUSTOM, "libcharon-receiver"),
			PLUGIN_DEPENDS(CUSTOM, "kernel-ipsec"),
			PLUGIN_DEPENDS(CUSTOM, "kernel-net"),
		PLUGIN_CALLBACK((plugin_feature_callback_t)sender_receiver_cb, this),
			PLUGIN_PROVIDE(CUSTOM, "libcharon-receiver"),
				PLUGIN_DEPENDS(HASHER, HASH_SHA1),
				PLUGIN_DEPENDS(RNG, RNG_STRONG),
				PLUGIN_DEPENDS(CUSTOM, "socket"),
		PLUGIN_CALLBACK((plugin_feature_callback_t)sa_managers_cb, this),
			PLUGIN_PROVIDE(CUSTOM, "libcharon-sa-managers"),
				PLUGIN_DEPENDS(HASHER, HASH_SHA1),
				PLUGIN_DEPENDS(RNG, RNG_WEAK),
	};
	lib->plugins->add_static_features(lib->plugins, lib->ns, features,
									  countof(features), TRUE, NULL, NULL);

	/* load plugins, further infrastructure may need it */
	if (!lib->plugins->load(lib->plugins, plugins))
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
			.set_default_loggers = _set_default_loggers,
			.set_level = _set_level,
			.bus = bus_create(),
		},
		.loggers = linked_list_create(),
		.mutex = mutex_create(MUTEX_TYPE_DEFAULT),
		.ref = 1,
	);
	charon = &this->public;
	this->public.kernel = kernel_interface_create();
	this->public.attributes = attribute_manager_create();
	this->public.controller = controller_create();
	this->public.eap = eap_manager_create();
	this->public.xauth = xauth_manager_create();
	this->public.backends = backend_manager_create();
	this->public.socket = socket_manager_create();
	this->public.traps = trap_manager_create();
	this->public.shunts = shunt_manager_create();
	this->public.redirect = redirect_manager_create();
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

	if (lib->integrity &&
		!lib->integrity->check(lib->integrity, "libcharon", libcharon_init))
	{
		dbg(DBG_DMN, 1, "integrity check of libcharon failed");
		this->integrity_failed = TRUE;
	}
	return !this->integrity_failed;
}

/*
 * Copyright (C) 2010 Martin Willi
 * Copyright (C) 2010 revosec AG
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
#include <unistd.h>
#include <stdio.h>
#include <errno.h>
#include <signal.h>
#include <getopt.h>
#include <dlfcn.h>
#include <libgen.h>

#include "conftest.h"
#include "config.h"
#include "hooks/hook.h"

#include <bus/listeners/file_logger.h>
#include <threading/thread.h>
#include <credentials/certificates/x509.h>

/**
 * Conftest globals struct
 */
conftest_t *conftest;

/**
 * Print usage information
 */
static void usage(FILE *out)
{
	fprintf(out, "Usage:\n");
	fprintf(out, "  --help           show usage information\n");
	fprintf(out, "  --version        show conftest version\n");
	fprintf(out, "  --suite <file>   global testsuite configuration "
									 "(default: ./suite.conf)\n");
	fprintf(out, "  --test <file>    test specific configuration\n");
}

/**
 * Handle SIGSEGV/SIGILL signals raised by threads
 */
static void segv_handler(int signal)
{
	fprintf(stderr, "thread %u received %d\n", thread_current_id(), signal);
	abort();
}

/**
 * Load suite and test specific configurations
 */
static bool load_configs(char *suite_file, char *test_file)
{
	if (!test_file)
	{
		fprintf(stderr, "Missing test configuration file.\n");
		return FALSE;
	}
	if (access(suite_file, R_OK) != 0)
	{
		fprintf(stderr, "Reading suite configuration file '%s' failed: %s.\n",
				suite_file, strerror(errno));
		return FALSE;
	}
	if (access(test_file, R_OK) != 0)
	{
		fprintf(stderr, "Reading test configuration file '%s' failed: %s.\n",
				test_file, strerror(errno));
		return FALSE;
	}
	conftest->test = settings_create(suite_file);
	conftest->test->load_files(conftest->test, test_file, TRUE);
	conftest->suite_dir = path_dirname(suite_file);
	return TRUE;
}

/**
 * Load trusted/untrusted certificates
 */
static bool load_cert(settings_t *settings, bool trusted)
{
	enumerator_t *enumerator;
	char *key, *value;

	enumerator = settings->create_key_value_enumerator(settings,
								trusted ? "certs.trusted" : "certs.untrusted");
	while (enumerator->enumerate(enumerator, &key, &value))
	{
		certificate_t *cert = NULL;

		if (strncaseeq(key, "x509", strlen("x509")))
		{
			cert = lib->creds->create(lib->creds, CRED_CERTIFICATE,
							CERT_X509, BUILD_FROM_FILE, value, BUILD_END);
		}
		else if (strncaseeq(key, "crl", strlen("crl")))
		{
			cert = lib->creds->create(lib->creds, CRED_CERTIFICATE,
							CERT_X509_CRL, BUILD_FROM_FILE, value, BUILD_END);
		}
		else
		{
			fprintf(stderr, "certificate type '%s' not supported\n", key);
			enumerator->destroy(enumerator);
			return FALSE;
		}
		if (!cert)
		{
			fprintf(stderr, "loading %strusted certificate '%s' from '%s' "
					"failed\n", trusted ? "" : "un", key, value);
			enumerator->destroy(enumerator);
			return FALSE;
		}
		conftest->creds->add_cert(conftest->creds, trusted, cert);
	}
	enumerator->destroy(enumerator);
	return TRUE;
}

/**
 * Load certificates from the confiuguration file
 */
static bool load_certs(settings_t *settings, char *dir)
{
	char wd[PATH_MAX];

	if (getcwd(wd, sizeof(wd)) == NULL)
	{
		fprintf(stderr, "getting cwd failed: %s\n", strerror(errno));
		return FALSE;
	}
	if (chdir(dir) != 0)
	{
		fprintf(stderr, "opening directory '%s' failed: %s\n",
				dir, strerror(errno));
		return FALSE;
	}

	if (!load_cert(settings, TRUE) ||
		!load_cert(settings, FALSE))
	{
		return FALSE;
	}

	if (chdir(wd) != 0)
	{
		fprintf(stderr, "opening directory '%s' failed: %s\n",
				wd, strerror(errno));
		return FALSE;
	}
	return TRUE;
}

/**
 * Load private keys from the confiuguration file
 */
static bool load_keys(settings_t *settings, char *dir)
{
	enumerator_t *enumerator;
	char *type, *value, wd[PATH_MAX];
	private_key_t *key;
	key_type_t key_type;

	if (getcwd(wd, sizeof(wd)) == NULL)
	{
		fprintf(stderr, "getting cwd failed: %s\n", strerror(errno));
		return FALSE;
	}
	if (chdir(dir) != 0)
	{
		fprintf(stderr, "opening directory '%s' failed: %s\n",
				dir, strerror(errno));
		return FALSE;
	}

	enumerator = settings->create_key_value_enumerator(settings, "keys");
	while (enumerator->enumerate(enumerator, &type, &value))
	{
		if (strncaseeq(type, "ecdsa", strlen("ecdsa")))
		{
			key_type = KEY_ECDSA;
		}
		else if (strncaseeq(type, "rsa", strlen("rsa")))
		{
			key_type = KEY_RSA;
		}
		else
		{
			fprintf(stderr, "unknown key type: '%s'\n", type);
			enumerator->destroy(enumerator);
			return FALSE;
		}
		key = lib->creds->create(lib->creds, CRED_PRIVATE_KEY, key_type,
								 BUILD_FROM_FILE, value, BUILD_END);
		if (!key)
		{
			fprintf(stderr, "loading %s key from '%s' failed\n", type, value);
			enumerator->destroy(enumerator);
			return FALSE;
		}
		conftest->creds->add_key(conftest->creds, key);
	}
	enumerator->destroy(enumerator);

	if (chdir(wd) != 0)
	{
		fprintf(stderr, "opening directory '%s' failed: %s\n",
				wd, strerror(errno));
		return FALSE;
	}
	return TRUE;
}

/**
 * Load certificate distribution points
 */
static void load_cdps(settings_t *settings)
{
	enumerator_t *enumerator;
	identification_t *id;
	char *ca, *uri, *section;
	certificate_type_t type;
	x509_t *x509;

	enumerator = settings->create_section_enumerator(settings, "cdps");
	while (enumerator->enumerate(enumerator, &section))
	{
		if (strncaseeq(section, "crl", strlen("crl")))
		{
			type = CERT_X509_CRL;
		}
		else if (strncaseeq(section, "ocsp", strlen("ocsp")))
		{
			type = CERT_X509_OCSP_RESPONSE;
		}
		else
		{
			fprintf(stderr, "unknown cdp type '%s', ignored\n", section);
			continue;
		}

		uri = settings->get_str(settings, "cdps.%s.uri", NULL, section);
		ca = settings->get_str(settings, "cdps.%s.ca", NULL, section);
		if (!ca || !uri)
		{
			fprintf(stderr, "cdp '%s' misses ca/uri, ignored\n", section);
			continue;
		}
		x509 = lib->creds->create(lib->creds, CRED_CERTIFICATE,
							CERT_X509, BUILD_FROM_FILE, ca, BUILD_END);
		if (!x509)
		{
			fprintf(stderr, "loading cdp '%s' ca failed, ignored\n", section);
			continue;
		}
		id = identification_create_from_encoding(ID_KEY_ID,
									x509->get_subjectKeyIdentifier(x509));
		conftest->creds->add_cdp(conftest->creds, type, id, uri);
		DESTROY_IF((certificate_t*)x509);
		id->destroy(id);
	}
	enumerator->destroy(enumerator);
}

/**
 * Load configured hooks
 */
static bool load_hooks()
{
	enumerator_t *enumerator;
	char *name, *pos, buf[64];
	hook_t *(*create)(char*);
	hook_t *hook;

	enumerator = conftest->test->create_section_enumerator(conftest->test,
														   "hooks");
	while (enumerator->enumerate(enumerator, &name))
	{
		pos = strchr(name, '-');
		if (pos)
		{
			snprintf(buf, sizeof(buf), "%.*s_hook_create", (int)(pos - name),
					 name);
		}
		else
		{
			snprintf(buf, sizeof(buf), "%s_hook_create", name);
		}
		create = dlsym(RTLD_DEFAULT, buf);
		if (create)
		{
			hook = create(name);
			if (hook)
			{
				conftest->hooks->insert_last(conftest->hooks, hook);
				charon->bus->add_listener(charon->bus, &hook->listener);
			}
		}
		else
		{
			fprintf(stderr, "dlsym() for hook '%s' failed: %s\n", name, dlerror());
			enumerator->destroy(enumerator);
			return FALSE;
		}
	}
	enumerator->destroy(enumerator);
	return TRUE;
}

/**
 * atexit() cleanup handler
 */
static void cleanup()
{
	file_logger_t *logger;
	hook_t *hook;

	DESTROY_IF(conftest->test);
	lib->credmgr->remove_set(lib->credmgr, &conftest->creds->set);
	conftest->creds->destroy(conftest->creds);
	DESTROY_IF(conftest->actions);
	while (conftest->hooks->remove_last(conftest->hooks,
										(void**)&hook) == SUCCESS)
	{
		charon->bus->remove_listener(charon->bus, &hook->listener);
		hook->destroy(hook);
	}
	conftest->hooks->destroy(conftest->hooks);
	if (conftest->config)
	{
		if (charon->backends)
		{
			charon->backends->remove_backend(charon->backends,
											 &conftest->config->backend);
		}
		conftest->config->destroy(conftest->config);
	}
	while (conftest->loggers->remove_last(conftest->loggers,
										  (void**)&logger) == SUCCESS)
	{
		charon->bus->remove_logger(charon->bus, &logger->logger);
		logger->destroy(logger);
	}
	conftest->loggers->destroy(conftest->loggers);
	free(conftest->suite_dir);
	free(conftest);
	libcharon_deinit();
	library_deinit();
}

/**
 * Load log levels for a logger from section
 */
static void load_log_levels(file_logger_t *logger, char *section)
{
	debug_t group;
	level_t  def;

	def = conftest->test->get_int(conftest->test, "log.%s.default", 1, section);
	for (group = 0; group < DBG_MAX; group++)
	{
		logger->set_level(logger, group,
					conftest->test->get_int(conftest->test, "log.%s.%N", def,
											section, debug_lower_names, group));
	}
}

/**
 * Load logger options for a logger from section
 */
static void load_logger_options(file_logger_t *logger, char *section)
{
	char *time_format;
	bool add_ms, ike_name;

	time_format = conftest->test->get_str(conftest->test,
					"log.%s.time_format", NULL, section);
	add_ms = conftest->test->get_bool(conftest->test,
					"log.%s.time_add_ms", FALSE, section);
	ike_name = conftest->test->get_bool(conftest->test,
					"log.%s.ike_name", FALSE, section);

	logger->set_options(logger, time_format, add_ms, ike_name);
}

/**
 * Load logger configuration
 */
static void load_loggers(file_logger_t *logger)
{
	enumerator_t *enumerator;
	char *section;

	load_log_levels(logger, "stdout");
	load_logger_options(logger, "stdout");
	/* Re-add the logger to propagate configuration changes to the
	 * logging system */
	charon->bus->add_logger(charon->bus, &logger->logger);

	enumerator = conftest->test->create_section_enumerator(conftest->test, "log");
	while (enumerator->enumerate(enumerator, &section))
	{
		if (!streq(section, "stdout"))
		{
			logger = file_logger_create(section);
			load_logger_options(logger, section);
			logger->open(logger, FALSE, FALSE);
			load_log_levels(logger, section);
			charon->bus->add_logger(charon->bus, &logger->logger);
			conftest->loggers->insert_last(conftest->loggers, logger);
		}
	}
	enumerator->destroy(enumerator);
}

/**
 * Main function, starts the conftest daemon.
 */
int main(int argc, char *argv[])
{
	struct sigaction action;
	int status = 0;
	sigset_t set;
	int sig;
	char *suite_file = "suite.conf", *test_file = NULL, *preload, *plugins;
	file_logger_t *logger;

	if (!library_init(NULL, "conftest"))
	{
		library_deinit();
		return SS_RC_LIBSTRONGSWAN_INTEGRITY;
	}
	if (!libcharon_init())
	{
		libcharon_deinit();
		library_deinit();
		return SS_RC_INITIALIZATION_FAILED;
	}

	INIT(conftest,
		.creds = mem_cred_create(),
		.config = config_create(),
		.hooks = linked_list_create(),
		.loggers = linked_list_create(),
	);
	lib->credmgr->add_set(lib->credmgr, &conftest->creds->set);

	logger = file_logger_create("stdout");
	logger->set_options(logger, NULL, FALSE, FALSE);
	logger->open(logger, FALSE, FALSE);
	logger->set_level(logger, DBG_ANY, LEVEL_CTRL);
	charon->bus->add_logger(charon->bus, &logger->logger);
	conftest->loggers->insert_last(conftest->loggers, logger);

	atexit(cleanup);

	while (TRUE)
	{
		struct option long_opts[] = {
			{ "help", no_argument, NULL, 'h' },
			{ "version", no_argument, NULL, 'v' },
			{ "suite", required_argument, NULL, 's' },
			{ "test", required_argument, NULL, 't' },
			{ 0,0,0,0 }
		};
		switch (getopt_long(argc, argv, "", long_opts, NULL))
		{
			case EOF:
				break;
			case 'h':
				usage(stdout);
				return 0;
			case 'v':
				printf("strongSwan %s conftest\n", VERSION);
				return 0;
			case 's':
				suite_file = optarg;
				continue;
			case 't':
				test_file = optarg;
				continue;
			default:
				usage(stderr);
				return 1;
		}
		break;
	}

	if (!load_configs(suite_file, test_file))
	{
		return 1;
	}
	load_loggers(logger);

	preload = conftest->test->get_str(conftest->test, "preload", "");
	if (asprintf(&plugins, "%s %s", preload, PLUGINS) < 0)
	{
		return 1;
	}
	if (!charon->initialize(charon, plugins))
	{
		free(plugins);
		return 1;
	}
	lib->plugins->status(lib->plugins, LEVEL_CTRL);
	free(plugins);

	if (!load_certs(conftest->test, conftest->suite_dir))
	{
		return 1;
	}
	if (!load_keys(conftest->test, conftest->suite_dir))
	{
		return 1;
	}
	load_cdps(conftest->test);
	if (!load_hooks())
	{
		return 1;
	}
	charon->backends->add_backend(charon->backends, &conftest->config->backend);
	conftest->config->load(conftest->config, conftest->test);
	conftest->actions = actions_create();

	/* set up thread specific handlers */
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

	/* start thread pool */
	charon->start(charon);

	/* handle SIGINT/SIGTERM in main thread */
	sigemptyset(&set);
	sigaddset(&set, SIGINT);
	sigaddset(&set, SIGHUP);
	sigaddset(&set, SIGTERM);
	sigprocmask(SIG_BLOCK, &set, NULL);

	while ((sig = sigwaitinfo(&set, NULL)) != -1 || errno == EINTR)
	{
		switch (sig)
		{
			case SIGINT:
			case SIGTERM:
				fprintf(stderr, "\nshutting down...\n");
				break;
			default:
				continue;
		}
		break;
	}
	return status;
}

/*
 * Copyright (C) 2010-2014 Tobias Brunner
 * Copyright (C) 2008 Martin Willi
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

/**
 * @defgroup libstrongswan libstrongswan
 *
 * @defgroup asn1 asn1
 * @ingroup libstrongswan
 *
 * @defgroup bio bio
 * @ingroup libstrongswan
 *
 * @defgroup collections collections
 * @ingroup libstrongswan
 *
 * @defgroup credentials credentials
 * @ingroup libstrongswan
 *
 * @defgroup keys keys
 * @ingroup credentials
 *
 * @defgroup certificates certificates
 * @ingroup credentials
 *
 * @defgroup containers containers
 * @ingroup credentials
 *
 * @defgroup sets sets
 * @ingroup credentials
 *
 * @defgroup crypto crypto
 * @ingroup libstrongswan
 *
 * @defgroup database database
 * @ingroup libstrongswan
 *
 * @defgroup fetcher fetcher
 * @ingroup libstrongswan
 *
 * @defgroup resolver resolver
 * @ingroup libstrongswan
 *
 * @defgroup ipsec ipsec
 * @ingroup libstrongswan
 *
 * @defgroup networking networking
 * @ingroup libstrongswan
 *
 * @defgroup streams streams
 * @ingroup networking
 *
 * @defgroup plugins plugins
 * @ingroup libstrongswan
 *
 * @defgroup processing processing
 * @ingroup libstrongswan
 *
 * @defgroup jobs jobs
 * @ingroup processing
 *
 * @defgroup selectors selectors
 * @ingroup libstrongswan
 *
 * @defgroup threading threading
 * @ingroup libstrongswan
 *
 * @defgroup utils utils
 * @ingroup libstrongswan
 */

/**
 * @defgroup library library
 * @{ @ingroup libstrongswan
 */

#ifndef LIBRARY_H_
#define LIBRARY_H_

#ifndef CONFIG_H_INCLUDED
# error config.h not included, pass "-include [...]/config.h" to gcc
#endif

/* make sure we include printf_hook.h and utils.h first */
#include "utils/printf_hook/printf_hook.h"
#include "utils/utils.h"
#include "networking/host_resolver.h"
#include "networking/streams/stream_manager.h"
#include "processing/processor.h"
#include "processing/scheduler.h"
#include "processing/watcher.h"
#include "crypto/crypto_factory.h"
#include "crypto/proposal/proposal_keywords.h"
#include "fetcher/fetcher_manager.h"
#include "resolver/resolver_manager.h"
#include "database/database_factory.h"
#include "credentials/credential_factory.h"
#include "credentials/credential_manager.h"
#include "credentials/cred_encoding.h"
#include "utils/chunk.h"
#include "utils/capabilities.h"
#include "utils/integrity_checker.h"
#include "utils/leak_detective.h"
#include "plugins/plugin_loader.h"
#include "settings/settings.h"

typedef struct library_t library_t;

/**
 * Libstrongswan library context, contains library relevant globals.
 */
struct library_t {

	/**
	 * Get an arbitrary object registered by name.
	 *
	 * @param name		name of the object to get
	 * @return			object, NULL if none found
	 */
	void* (*get)(library_t *this, char *name);

	/**
	 * (Un-)Register an arbitrary object using the given name.
	 *
	 * @param name		name to register object under
	 * @param object	object to register, NULL to unregister
	 * @return			TRUE if registered, FALSE if name already taken
	 */
	bool (*set)(library_t *this, char *name, void *object);

	/**
	 * Namespace used for settings etc. (i.e. the name of the binary that uses
	 * the library)
	 */
	const char *ns;

	/**
	 * Main configuration file passed to library_init(), the default, or NULL
	 */
	char *conf;

	/**
	 * Printf hook registering facility
	 */
	printf_hook_t *printf_hook;

	/**
	 * Proposal keywords registry
	 */
	proposal_keywords_t *proposal;

	/**
	 * POSIX capability dropping
	 */
	capabilities_t *caps;

	/**
	 * crypto algorithm registry and factory
	 */
	crypto_factory_t *crypto;

	/**
	 * credential constructor registry and factory
	 */
	credential_factory_t *creds;

	/**
	 * Manager for the credential set backends
	 */
	credential_manager_t *credmgr;

	/**
	 * Credential encoding registry and factory
	 */
	cred_encoding_t *encoding;

	/**
	 * URL fetching facility
	 */
	fetcher_manager_t *fetcher;

	/**
	 * Manager for DNS resolvers
	 */
	 resolver_manager_t *resolver;

	/**
	 * database construction factory
	 */
	database_factory_t *db;

	/**
	 * plugin loading facility
	 */
	plugin_loader_t *plugins;

	/**
	 * process jobs using a thread pool
	 */
	processor_t *processor;

	/**
	 * schedule jobs
	 */
	scheduler_t *scheduler;

	/**
	 * File descriptor monitoring
	 */
	watcher_t *watcher;

	/**
	 * Streams and Services
	 */
	stream_manager_t *streams;

	/**
	 * resolve hosts by DNS name
	 */
	host_resolver_t *hosts;

	/**
	 * various settings loaded from settings file
	 */
	settings_t *settings;

	/**
	 * integrity checker to verify code integrity
	 */
	integrity_checker_t *integrity;

	/**
	 * Leak detective, if built and enabled
	 */
	leak_detective_t *leak_detective;
};

/**
 * Initialize library, creates "lib" instance.
 *
 * library_init() may be called multiple times in a single process, but each
 * caller must call library_deinit() for each call to library_init().
 *
 * The settings and namespace arguments are only used on the first call.
 *
 * @param settings		file to read settings from, may be NULL for default
 * @param namespace		name of the binary that uses the library, determines
 *						the first section name when reading config options.
 *						Defaults to libstrongswan if NULL.
 * @return				FALSE if integrity check failed
 */
bool library_init(char *settings, const char *namespace);

/**
 * Deinitialize library, destroys "lib" instance.
 */
void library_deinit();

/**
 * Library instance, set after library_init() and before library_deinit() calls.
 */
extern library_t *lib;

#endif /** LIBRARY_H_ @}*/

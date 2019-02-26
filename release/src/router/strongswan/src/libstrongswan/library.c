/*
 * Copyright (C) 2009-2018 Tobias Brunner
 * Copyright (C) 2008 Martin Willi
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

#include "library.h"

#include <stdlib.h>

#include <utils/debug.h>
#include <threading/thread.h>
#include <utils/identification.h>
#include <networking/host.h>
#include <collections/array.h>
#include <collections/hashtable.h>
#include <utils/backtrace.h>
#include <selectors/traffic_selector.h>
#include <crypto/proposal/proposal.h>

#define CHECKSUM_LIBRARY IPSEC_LIB_DIR"/libchecksum.so"

#ifndef STRONGSWAN_CONF
#define STRONGSWAN_CONF NULL
#endif

typedef struct private_library_t private_library_t;

/**
 * private data of library
 */
struct private_library_t {

	/**
	 * public functions
	 */
	library_t public;

	/**
	 * Hashtable with registered objects (name => object)
	 */
	hashtable_t *objects;

	/**
	 * Integrity check failed?
	 */
	bool init_failed;

#ifdef LEAK_DETECTIVE
	/**
	 * Where to write leak detective output to
	 */
	FILE *ld_out;
#endif

	/**
	 * Number of times we have been initialized
	 */
	refcount_t ref;
};

#define MAX_NAMESPACES 5

/**
 * Additional namespaces registered using __atrribute__((constructor))
 */
static char *namespaces[MAX_NAMESPACES];
static int ns_count;

/**
 * Described in header
 */
void library_add_namespace(char *ns)
{
	if (ns_count < MAX_NAMESPACES - 1)
	{
		namespaces[ns_count] = ns;
		ns_count++;
	}
	else
	{
		fprintf(stderr, "failed to register additional namespace alias, please "
				"increase MAX_NAMESPACES");
	}
}

/**
 * Register plugins if built statically
 */
#ifdef STATIC_PLUGIN_CONSTRUCTORS
#include "plugin_constructors.c"
#endif

/**
 * library instance
 */
library_t *lib = NULL;

#ifdef LEAK_DETECTIVE
/**
 * Default leak report callback
 */
CALLBACK(report_leaks, void,
	private_library_t *this, int count, size_t bytes, backtrace_t *bt,
	bool detailed)
{
	fprintf(this->ld_out, "%zu bytes total, %d allocations, %zu bytes average:\n",
			bytes, count, bytes / count);
	bt->log(bt, this->ld_out, detailed);
}

/**
 * Default leak report summary callback
 */
CALLBACK(sum_leaks, void,
	private_library_t *this, int count, size_t bytes, int whitelisted)
{
	switch (count)
	{
		case 0:
			fprintf(this->ld_out, "No leaks detected");
			break;
		case 1:
			fprintf(this->ld_out, "One leak detected");
			break;
		default:
			fprintf(this->ld_out, "%d leaks detected, %zu bytes", count, bytes);
			break;
	}
	fprintf(this->ld_out, ", %d suppressed by whitelist\n", whitelisted);
}
#endif /* LEAK_DETECTIVE */

/**
 * Deinitialize library
 */
void library_deinit()
{
	private_library_t *this = (private_library_t*)lib;
	bool detailed;

	if (!this || !ref_put(&this->ref))
	{	/* have more users */
		return;
	}

	detailed = lib->settings->get_bool(lib->settings,
								"%s.leak_detective.detailed", TRUE, lib->ns);

	/* make sure the cache is clear before unloading plugins */
	lib->credmgr->flush_cache(lib->credmgr, CERT_ANY);

	this->public.streams->destroy(this->public.streams);
	this->public.watcher->destroy(this->public.watcher);
	this->public.scheduler->destroy(this->public.scheduler);
	this->public.processor->destroy(this->public.processor);
	this->public.plugins->destroy(this->public.plugins);
	this->public.hosts->destroy(this->public.hosts);
	this->public.settings->destroy(this->public.settings);
	this->public.credmgr->destroy(this->public.credmgr);
	this->public.creds->destroy(this->public.creds);
	this->public.encoding->destroy(this->public.encoding);
	this->public.crypto->destroy(this->public.crypto);
	this->public.caps->destroy(this->public.caps);
	this->public.proposal->destroy(this->public.proposal);
	this->public.fetcher->destroy(this->public.fetcher);
	this->public.resolver->destroy(this->public.resolver);
	this->public.db->destroy(this->public.db);
	this->public.printf_hook->destroy(this->public.printf_hook);
	this->objects->destroy(this->objects);
	if (this->public.integrity)
	{
		this->public.integrity->destroy(this->public.integrity);
	}

	if (lib->leak_detective)
	{
		lib->leak_detective->report(lib->leak_detective, detailed);
		lib->leak_detective->destroy(lib->leak_detective);
		lib->leak_detective = NULL;
	}
#ifdef LEAK_DETECTIVE
	if (this->ld_out && this->ld_out != stderr)
	{
		fclose(this->ld_out);
	}
#endif /* LEAK_DETECTIVE */

	backtrace_deinit();
	arrays_deinit();
	utils_deinit();
	threads_deinit();

	free(this->public.conf);
	free((void*)this->public.ns);
	free(this);
	lib = NULL;
}

METHOD(library_t, get, void*,
	private_library_t *this, char *name)
{
	return this->objects->get(this->objects, name);
}

METHOD(library_t, set, bool,
	private_library_t *this, char *name, void *object)
{
	if (object)
	{
		if (this->objects->get(this->objects, name))
		{
			return FALSE;
		}
		this->objects->put(this->objects, name, object);
		return TRUE;
	}
	return this->objects->remove(this->objects, name) != NULL;
}

/**
 * Hashtable hash function
 */
static u_int hash(char *key)
{
	return chunk_hash(chunk_create(key, strlen(key)));
}

/**
 * Hashtable equals function
 */
static bool equals(char *a, char *b)
{
	return streq(a, b);
}

/**
 * Number of words we write and memwipe() in memwipe check
 */
#define MEMWIPE_WIPE_WORDS 16

#ifndef NO_CHECK_MEMWIPE

/**
 * Write magic to memory, and try to clear it with memwipe()
 */
__attribute__((noinline))
static void do_magic(int *magic, int **out)
{
	int buf[MEMWIPE_WIPE_WORDS], i;

	*out = buf;
	for (i = 0; i < countof(buf); i++)
	{
		buf[i] = *magic;
	}
	/* passing buf to dbg should make sure the compiler can't optimize out buf.
	 * we use directly dbg(3), as DBG3() might be stripped with DEBUG_LEVEL. */
	dbg(DBG_LIB, 3, "memwipe() pre: %b", buf, sizeof(buf));
	memwipe(buf, sizeof(buf));
}

/**
 * Check if memwipe works as expected
 */
static bool check_memwipe()
{
	int magic = 0xCAFEBABE, *buf, i;

	do_magic(&magic, &buf);

	for (i = 0; i < MEMWIPE_WIPE_WORDS; i++)
	{
		if (buf[i] == magic)
		{
			DBG1(DBG_LIB, "memwipe() check failed: stackdir: %b",
				 buf, MEMWIPE_WIPE_WORDS * sizeof(int));
			return FALSE;
		}
	}
	return TRUE;
}

#endif

/*
 * see header file
 */
bool library_init(char *settings, const char *namespace)
{
	private_library_t *this;
	printf_hook_t *pfh;
	int i;

	if (lib)
	{	/* already initialized, increase refcount */
		this = (private_library_t*)lib;
		ref_get(&this->ref);
		return !this->init_failed;
	}

	chunk_hash_seed();

	INIT(this,
		.public = {
			.get = _get,
			.set = _set,
			.ns = strdup(namespace ?: "libstrongswan"),
			.conf = strdupnull(settings ?: (getenv("STRONGSWAN_CONF") ?: STRONGSWAN_CONF)),
		},
		.ref = 1,
	);
	lib = &this->public;

	threads_init();
	utils_init();
	arrays_init();
	backtrace_init();

#ifdef LEAK_DETECTIVE
	{
		FILE *out = NULL;
		char *log;

		log = getenv("LEAK_DETECTIVE_LOG");
		if (log)
		{
			out = fopen(log, "a");
		}
		this->ld_out = out ?: stderr;
	}
	lib->leak_detective = leak_detective_create();
	if (lib->leak_detective)
	{
		lib->leak_detective->set_report_cb(lib->leak_detective,
										   report_leaks, sum_leaks, this);
	}
#endif /* LEAK_DETECTIVE */

	pfh = printf_hook_create();
	this->public.printf_hook = pfh;

	pfh->add_handler(pfh, 'b', mem_printf_hook,
					 PRINTF_HOOK_ARGTYPE_POINTER, PRINTF_HOOK_ARGTYPE_INT,
					 PRINTF_HOOK_ARGTYPE_END);
	pfh->add_handler(pfh, 'B', chunk_printf_hook,
					 PRINTF_HOOK_ARGTYPE_POINTER, PRINTF_HOOK_ARGTYPE_END);
	pfh->add_handler(pfh, 'H', host_printf_hook,
					 PRINTF_HOOK_ARGTYPE_POINTER, PRINTF_HOOK_ARGTYPE_END);
	pfh->add_handler(pfh, 'N', enum_printf_hook,
					 PRINTF_HOOK_ARGTYPE_POINTER, PRINTF_HOOK_ARGTYPE_INT,
					 PRINTF_HOOK_ARGTYPE_END);
	pfh->add_handler(pfh, 'T', time_printf_hook,
					 PRINTF_HOOK_ARGTYPE_POINTER, PRINTF_HOOK_ARGTYPE_INT,
					 PRINTF_HOOK_ARGTYPE_END);
	pfh->add_handler(pfh, 'V', time_delta_printf_hook,
					 PRINTF_HOOK_ARGTYPE_POINTER, PRINTF_HOOK_ARGTYPE_POINTER,
					 PRINTF_HOOK_ARGTYPE_END);
	pfh->add_handler(pfh, 'Y', identification_printf_hook,
					 PRINTF_HOOK_ARGTYPE_POINTER, PRINTF_HOOK_ARGTYPE_END);
	pfh->add_handler(pfh, 'R', traffic_selector_printf_hook,
					 PRINTF_HOOK_ARGTYPE_POINTER, PRINTF_HOOK_ARGTYPE_END);
	pfh->add_handler(pfh, 'P', proposal_printf_hook,
					 PRINTF_HOOK_ARGTYPE_POINTER, PRINTF_HOOK_ARGTYPE_END);

	this->objects = hashtable_create((hashtable_hash_t)hash,
									 (hashtable_equals_t)equals, 4);

	this->public.settings = settings_create(NULL);
	if (!this->public.settings->load_files(this->public.settings,
										   this->public.conf, FALSE))
	{
		DBG1(DBG_LIB, "abort initialization due to invalid configuration");
		this->init_failed = TRUE;
	}

	/* add registered aliases */
	for (i = 0; i < ns_count; ++i)
	{
		lib->settings->add_fallback(lib->settings, lib->ns, namespaces[i]);
	}
	/* all namespace settings may fall back to libstrongswan */
	lib->settings->add_fallback(lib->settings, lib->ns, "libstrongswan");

	this->public.hosts = host_resolver_create();
	this->public.proposal = proposal_keywords_create();
	this->public.caps = capabilities_create();
	this->public.crypto = crypto_factory_create();
	this->public.creds = credential_factory_create();
	this->public.credmgr = credential_manager_create();
	this->public.encoding = cred_encoding_create();
	this->public.fetcher = fetcher_manager_create();
	this->public.resolver = resolver_manager_create();
	this->public.db = database_factory_create();
	this->public.processor = processor_create();
	this->public.scheduler = scheduler_create();
	this->public.watcher = watcher_create();
	this->public.streams = stream_manager_create();
	this->public.plugins = plugin_loader_create();

#ifndef NO_CHECK_MEMWIPE
	if (!check_memwipe())
	{
		return FALSE;
	}
#endif

	if (lib->settings->get_bool(lib->settings,
								"%s.integrity_test", FALSE, lib->ns))
	{
#ifdef INTEGRITY_TEST
		this->public.integrity = integrity_checker_create(CHECKSUM_LIBRARY);
		if (!lib->integrity->check(lib->integrity, "libstrongswan", library_init))
		{
			DBG1(DBG_LIB, "integrity check of libstrongswan failed");
			this->init_failed = TRUE;
		}
#else /* !INTEGRITY_TEST */
		DBG1(DBG_LIB, "integrity test enabled, but not supported");
		this->init_failed = TRUE;
#endif /* INTEGRITY_TEST */
	}

	diffie_hellman_init();

	return !this->init_failed;
}

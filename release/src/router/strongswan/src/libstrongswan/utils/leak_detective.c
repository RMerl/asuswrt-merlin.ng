/*
 * Copyright (C) 2013-2018 Tobias Brunner
 * Copyright (C) 2006-2013 Martin Willi
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

#define _GNU_SOURCE
#include <stddef.h>
#include <string.h>
#include <stdio.h>
#include <signal.h>
#include <unistd.h>
#include <locale.h>
#ifdef HAVE_DLADDR
#include <dlfcn.h>
#endif
#include <time.h>
#include <errno.h>

#ifdef __APPLE__
#include <sys/mman.h>
#include <malloc/malloc.h>
/* overload some of our types clashing with mach */
#define host_t strongswan_host_t
#define processor_t strongswan_processor_t
#define thread_t strongswan_thread_t
#endif /* __APPLE__ */

#include "leak_detective.h"

#include <library.h>
#include <utils/utils.h>
#include <utils/debug.h>
#include <utils/backtrace.h>
#include <collections/hashtable.h>
#include <threading/thread_value.h>
#include <threading/spinlock.h>

typedef struct private_leak_detective_t private_leak_detective_t;

/**
 * private data of leak_detective
 */
struct private_leak_detective_t {

	/**
	 * public functions
	 */
	leak_detective_t public;

	/**
	 * Registered report() function
	 */
	leak_detective_report_cb_t report_cb;

	/**
	 * Registered report() summary function
	 */
	leak_detective_summary_cb_t report_scb;

	/**
	 * Registered user data for callbacks
	 */
	void *report_data;
};

/**
 * Magic value which helps to detect memory corruption. Yummy!
 */
#define MEMORY_HEADER_MAGIC 0x7ac0be11

/**
 * Magic written to tail of allocation
 */
#define MEMORY_TAIL_MAGIC 0xcafebabe

/**
 * Pattern which is filled in memory before freeing it
 */
#define MEMORY_FREE_PATTERN 0xFF

/**
 * Pattern which is filled in newly allocated memory
 */
#define MEMORY_ALLOC_PATTERN 0xEE

typedef struct memory_header_t memory_header_t;
typedef struct memory_tail_t memory_tail_t;

/**
 * Header which is prepended to each allocated memory block
 */
struct memory_header_t {

	/**
	 * Pointer to previous entry in linked list
	 */
	memory_header_t *previous;

	/**
	 * Pointer to next entry in linked list
	 */
	memory_header_t *next;

	/**
	 * backtrace taken during (re-)allocation
	 */
	backtrace_t *backtrace;

	/**
	 * Padding to make sizeof(memory_header_t) == 32
	 */
	uint32_t padding[sizeof(void*) == sizeof(uint32_t) ? 3 : 0];

	/**
	 * Number of bytes following after the header
	 */
	uint32_t bytes;

	/**
	 * magic bytes to detect bad free or heap underflow, MEMORY_HEADER_MAGIC
	 */
	uint32_t magic;

}__attribute__((__packed__));

/**
 * tail appended to each allocated memory block
 */
struct memory_tail_t {

	/**
	 * Magic bytes to detect heap overflow, MEMORY_TAIL_MAGIC
	 */
	uint32_t magic;

}__attribute__((__packed__));

/**
 * first mem header is just a dummy to chain
 * the others on it...
 */
static memory_header_t first_header = {
	.magic = MEMORY_HEADER_MAGIC,
};

/**
 * Spinlock to access header linked list
 */
static spinlock_t *lock;

/**
 * Is leak detection currently enabled?
 */
static bool enabled;

/**
 * Whether to report calls to free() with memory not allocated by us
 */
static bool ignore_unknown;

/**
 * Is leak detection disabled for the current thread?
 */
static thread_value_t *thread_disabled;

/**
 * Installs the malloc hooks, enables leak detection
 */
static void enable_leak_detective()
{
	enabled = TRUE;
}

/**
 * Uninstalls the malloc hooks, disables leak detection
 */
static void disable_leak_detective()
{
	enabled = FALSE;
}

/**
 * Enable/Disable leak detective for the current thread
 *
 * @return Previous value
 */
static bool enable_thread(bool enable)
{
	bool before;

	before = thread_disabled->get(thread_disabled) == NULL;
	thread_disabled->set(thread_disabled, enable ? NULL : (void*)TRUE);
	return before;
}

/**
 * Add a header to the beginning of the list
 */
static void add_hdr(memory_header_t *hdr)
{
	lock->lock(lock);
	hdr->next = first_header.next;
	if (hdr->next)
	{
		hdr->next->previous = hdr;
	}
	hdr->previous = &first_header;
	first_header.next = hdr;
	lock->unlock(lock);
}

/**
 * Remove a header from the list
 */
static void remove_hdr(memory_header_t *hdr)
{
	lock->lock(lock);
	if (hdr->next)
	{
		hdr->next->previous = hdr->previous;
	}
	hdr->previous->next = hdr->next;
	lock->unlock(lock);
}

/**
 * Check if a header is in the list
 */
static bool has_hdr(memory_header_t *hdr)
{
	memory_header_t *current;
	bool found = FALSE;

	lock->lock(lock);
	for (current = &first_header; current != NULL; current = current->next)
	{
		if (current == hdr)
		{
			found = TRUE;
			break;
		}
	}
	lock->unlock(lock);

	return found;
}

#ifdef __APPLE__

/**
 * Copy of original default zone, with functions we call in hooks
 */
static malloc_zone_t original;

/**
 * Call original malloc()
 */
static void* real_malloc(size_t size)
{
	return original.malloc(malloc_default_zone(), size);
}

/**
 * Call original free()
 */
static void real_free(void *ptr)
{
	original.free(malloc_default_zone(), ptr);
}

/**
 * Call original realloc()
 */
static void* real_realloc(void *ptr, size_t size)
{
	return original.realloc(malloc_default_zone(), ptr, size);
}

/**
 * Hook definition: static function with _hook suffix, takes additional zone
 */
#define HOOK(ret, name, ...) \
	static ret name ## _hook(malloc_zone_t *_z, __VA_ARGS__)

/**
 * forward declaration of hooks
 */
HOOK(void*, malloc, size_t bytes);
HOOK(void*, calloc, size_t nmemb, size_t size);
HOOK(void*, valloc, size_t size);
HOOK(void, free, void *ptr);
HOOK(void*, realloc, void *old, size_t bytes);

/**
 * malloc zone size(), must consider the memory header prepended
 */
HOOK(size_t, size, const void *ptr)
{
	bool before;
	size_t size;

	if (enabled)
	{
		before = enable_thread(FALSE);
		if (before)
		{
			ptr -= sizeof(memory_header_t);
		}
	}
	size = original.size(malloc_default_zone(), ptr);
	if (enabled)
	{
		enable_thread(before);
	}
	return size;
}

/**
 * Version of malloc zones we currently support
 */
#define MALLOC_ZONE_VERSION 8 /* Snow Leopard */

/**
 * Hook-in our malloc functions into the default zone
 */
static bool register_hooks()
{
	static bool once = FALSE;
	malloc_zone_t *zone;
	void *page;

	if (once)
	{
		return TRUE;
	}
	once = TRUE;

	zone = malloc_default_zone();
	if (zone->version != MALLOC_ZONE_VERSION)
	{
		DBG1(DBG_CFG, "malloc zone version %d unsupported (requiring %d)",
			 zone->version, MALLOC_ZONE_VERSION);
		return FALSE;
	}

	original = *zone;

	page = (void*)((uintptr_t)zone / getpagesize() * getpagesize());
	if (mprotect(page, getpagesize(), PROT_WRITE | PROT_READ) != 0)
	{
		DBG1(DBG_CFG, "malloc zone unprotection failed: %s", strerror(errno));
		return FALSE;
	}

	zone->size = size_hook;
	zone->malloc = malloc_hook;
	zone->calloc = calloc_hook;
	zone->valloc = valloc_hook;
	zone->free = free_hook;
	zone->realloc = realloc_hook;

	/* those other functions can be NULLed out to not use them */
	zone->batch_malloc = NULL;
	zone->batch_free = NULL;
	zone->memalign = NULL;
	zone->free_definite_size = NULL;

	return TRUE;
}

#else /* !__APPLE__ */

/**
 * dlsym() might do a malloc(), but we can't do one before we get the malloc()
 * function pointer. Use this minimalistic malloc implementation instead.
 */
static void* malloc_for_dlsym(size_t size)
{
	static char buf[1024] = {};
	static size_t used = 0;
	char *ptr;

	/* roundup to a multiple of 32 */
	size = (size - 1) / 32 * 32 + 32;

	if (used + size > sizeof(buf))
	{
		return NULL;
	}
	ptr = buf + used;
	used += size;
	return ptr;
}

/**
 * Lookup a malloc function, while disabling wrappers
 */
static void* get_malloc_fn(char *name)
{
	bool before = FALSE;
	void *fn;

	if (enabled)
	{
		before = enable_thread(FALSE);
	}
	fn = dlsym(RTLD_NEXT, name);
	if (enabled)
	{
		enable_thread(before);
	}
	return fn;
}

/**
 * Call original malloc()
 */
static void* real_malloc(size_t size)
{
	static void* (*fn)(size_t size);
	static int recursive = 0;

	if (!fn)
	{
		/* checking recursiveness should actually be thread-specific. But as
		 * it is very likely that the first allocation is done before we go
		 * multi-threaded, we keep it simple. */
		if (recursive)
		{
			return malloc_for_dlsym(size);
		}
		recursive++;
		fn = get_malloc_fn("malloc");
		recursive--;
	}
	return fn(size);
}

/**
 * Call original free()
 */
static void real_free(void *ptr)
{
	static void (*fn)(void *ptr);

	if (!fn)
	{
		fn = get_malloc_fn("free");
	}
	return fn(ptr);
}

/**
 * Call original realloc()
 */
static void* real_realloc(void *ptr, size_t size)
{
	static void* (*fn)(void *ptr, size_t size);

	if (!fn)
	{
		fn = get_malloc_fn("realloc");
	}
	return fn(ptr, size);
}

/**
 * Hook definition: plain function overloading existing malloc calls
 */
#define HOOK(ret, name, ...) ret name(__VA_ARGS__)

/**
 * Hook initialization when not using hooks, resolve functions.
 */
static bool register_hooks()
{
	void *buf = real_malloc(8);
	buf = real_realloc(buf, 16);
	real_free(buf);
	return TRUE;
}

#endif /* !__APPLE__ */

/**
 * Leak report white list
 *
 * List of functions using static allocation buffers or should be suppressed
 * otherwise on leak report.
 */
static char *whitelist[] = {
	/* backtraces, including own */
	"backtrace_create",
	"strerror_safe",
	/* pthread stuff */
	"pthread_create",
	"pthread_setspecific",
	"__pthread_setspecific",
	/* glibc functions */
	"inet_ntoa",
	"strerror",
	"getprotobyname",
	"getprotobynumber",
	"getservbyport",
	"getservbyname",
	"gethostbyname",
	"gethostbyname2",
	"gethostbyname_r",
	"gethostbyname2_r",
	"getnetbyname",
	"getpwnam_r",
	"getgrnam_r",
	"register_printf_function",
	"register_printf_specifier",
	"syslog",
	"vsyslog",
	"__syslog_chk",
	"__vsyslog_chk",
	"__fprintf_chk",
	"getaddrinfo",
	"setlocale",
	"getpass",
	"getpwent_r",
	"setpwent",
	"endpwent",
	"getspnam_r",
	"getpwuid_r",
	"initgroups",
	"tzset",
	"_IO_file_doallocate",
	/* ignore dlopen, as we do not dlclose to get proper leak reports */
	"dlopen",
	"dlerror",
	"dlclose",
	"dlsym",
	/* mysql functions */
	"mysql_init_character_set",
	"init_client_errs",
	"my_thread_init",
	/* fastcgi library */
	"FCGX_Init",
	/* libxml */
	"xmlInitCharEncodingHandlers",
	"xmlInitParser",
	"xmlInitParserCtxt",
	/* libcurl */
	"Curl_client_write",
	/* libsoup */
	"soup_message_headers_append",
	"soup_message_headers_clear",
	"soup_message_headers_get_list",
	"soup_message_headers_get_one",
	"soup_session_abort",
	"soup_session_get_type",
	/* libldap */
	"ldap_int_initialize",
	/* ClearSilver */
	"nerr_init",
	/* libgcrypt */
	"gcrypt_plugin_create",
	"gcry_control",
	"gcry_check_version",
	"gcry_randomize",
	"gcry_create_nonce",
	/* OpenSSL: These are needed for unit-tests only, the openssl plugin
	 * does properly clean up any memory during destroy(). */
	"ECDSA_do_sign_ex",
	"ECDSA_verify",
	"RSA_new_method",
	/* OpenSSL 1.1.0 does not cleanup anymore until the library is unloaded */
	"OPENSSL_init_crypto",
	"CRYPTO_THREAD_lock_new",
	"ERR_add_error_data",
	"ERR_set_mark",
	"ENGINE_load_builtin_engines",
	"OPENSSL_load_builtin_modules",
	"CONF_modules_load_file",
	"CONF_module_add",
	"RAND_DRBG_bytes",
	"RAND_DRBG_generate",
	"RAND_DRBG_get0_master",
	"RAND_DRBG_get0_private",
	"RAND_DRBG_get0_public",
	/* OpenSSL libssl */
	"SSL_COMP_get_compression_methods",
	/* NSPR */
	"PR_CallOnce",
	/* libapr */
	"apr_pool_create_ex",
	/* glib */
	"g_output_stream_write",
	"g_resolver_lookup_by_name",
	"g_signal_connect_data",
	"g_socket_connection_factory_lookup_type",
	"g_type_init_with_debug_flags",
	"g_type_register_static",
	"g_type_class_ref",
	"g_type_create_instance",
	"g_type_add_interface_static",
	"g_type_interface_add_prerequisite",
	"g_private_set",
	"g_queue_pop_tail",
	/* libgpg */
	"gpg_err_init",
	/* gnutls */
	"gnutls_global_init",
	/* Ada runtime */
	"system__tasking__initialize",
	"system__tasking__initialization__abort_defer",
	"system__tasking__stages__create_task",
	/* in case external threads call into our code */
	"thread_current_id",
	/* FHH IMCs and IMVs */
	"TNC_IMC_NotifyConnectionChange",
	"TNC_IMV_NotifyConnectionChange",
	/* Botan */
	"botan_public_key_load",
	"botan_privkey_create_ecdsa",
	"botan_privkey_create_ecdh",
	"botan_privkey_load_ecdh",
	"botan_privkey_load",
};

/**
 * Some functions are hard to whitelist, as they don't use a symbol directly.
 * Use some static initialization to suppress them on leak reports
 */
static void init_static_allocations()
{
	struct tm tm;
	time_t t = 0;

	tzset();
	gmtime_r(&t, &tm);
	localtime_r(&t, &tm);
}

/**
 * Hashtable hash function
 */
static u_int hash(backtrace_t *key)
{
	enumerator_t *enumerator;
	void *addr;
	u_int hash = 0;

	enumerator = key->create_frame_enumerator(key);
	while (enumerator->enumerate(enumerator, &addr))
	{
		hash = chunk_hash_inc(chunk_from_thing(addr), hash);
	}
	enumerator->destroy(enumerator);

	return hash;
}

/**
 * Hashtable equals function
 */
static bool equals(backtrace_t *a, backtrace_t *b)
{
	return a->equals(a, b);
}

/**
 * Summarize and print backtraces
 */
static int print_traces(private_leak_detective_t *this,
						leak_detective_report_cb_t cb, void *user,
						int thresh, int thresh_count,
						bool detailed, int *whitelisted, size_t *sum)
{
	int leaks = 0;
	memory_header_t *hdr;
	enumerator_t *enumerator;
	hashtable_t *entries, *ignored = NULL;
	backtrace_t *bt;
	struct {
		/** associated backtrace */
		backtrace_t *backtrace;
		/** total size of all allocations */
		size_t bytes;
		/** number of allocations */
		u_int count;
	} *entry;
	bool before;

	before = enable_thread(FALSE);

	entries = hashtable_create((hashtable_hash_t)hash,
							   (hashtable_equals_t)equals, 1024);
	if (whitelisted)
	{
		ignored = hashtable_create((hashtable_hash_t)hash,
								   (hashtable_equals_t)equals, 1024);
	}

	lock->lock(lock);
	for (hdr = first_header.next; hdr != NULL; hdr = hdr->next)
	{
		if (whitelisted)
		{
			bt = ignored->get(ignored, hdr->backtrace);
			if (!bt)
			{
				if (hdr->backtrace->contains_function(hdr->backtrace, whitelist,
													  countof(whitelist)))
				{
					bt = hdr->backtrace;
					ignored->put(ignored, bt, bt);
				}
			}
			if (bt)
			{
				(*whitelisted)++;
				continue;
			}
		}
		entry = entries->get(entries, hdr->backtrace);
		if (entry)
		{
			entry->bytes += hdr->bytes;
			entry->count++;
		}
		else
		{
			INIT(entry,
				.backtrace = hdr->backtrace->clone(hdr->backtrace),
				.bytes = hdr->bytes,
				.count = 1,
			);
			entries->put(entries, entry->backtrace, entry);
		}
		if (sum)
		{
			*sum += hdr->bytes;
		}
		leaks++;
	}
	lock->unlock(lock);
	DESTROY_IF(ignored);

	enumerator = entries->create_enumerator(entries);
	while (enumerator->enumerate(enumerator, NULL, &entry))
	{
		if (cb)
		{
			if (!thresh || entry->bytes >= thresh)
			{
				if (!thresh_count || entry->count >= thresh_count)
				{
					cb(user, entry->count, entry->bytes, entry->backtrace,
					   detailed);
				}
			}
		}
		entry->backtrace->destroy(entry->backtrace);
		free(entry);
	}
	enumerator->destroy(enumerator);
	entries->destroy(entries);

	enable_thread(before);
	return leaks;
}

METHOD(leak_detective_t, report, void,
	private_leak_detective_t *this, bool detailed)
{
	if (lib->leak_detective)
	{
		int leaks, whitelisted = 0;
		size_t sum = 0;

		leaks = print_traces(this, this->report_cb, this->report_data,
							 0, 0, detailed, &whitelisted, &sum);
		if (this->report_scb)
		{
			this->report_scb(this->report_data, leaks, sum, whitelisted);
		}
	}
}

METHOD(leak_detective_t, set_report_cb, void,
	private_leak_detective_t *this, leak_detective_report_cb_t cb,
	leak_detective_summary_cb_t scb, void *user)
{
	this->report_cb = cb;
	this->report_scb = scb;
	this->report_data = user;
}

METHOD(leak_detective_t, leaks, int,
	private_leak_detective_t *this)
{
	int whitelisted = 0;

	return print_traces(this, NULL, NULL, 0, 0, FALSE, &whitelisted, NULL);
}

METHOD(leak_detective_t, set_state, bool,
	private_leak_detective_t *this, bool enable)
{
	return enable_thread(enable);
}

METHOD(leak_detective_t, usage, void,
	private_leak_detective_t *this, leak_detective_report_cb_t cb,
	leak_detective_summary_cb_t scb, void *user)
{
	bool detailed;
	int thresh, thresh_count, leaks, whitelisted = 0;
	size_t sum = 0;

	thresh = lib->settings->get_int(lib->settings,
						"%s.leak_detective.usage_threshold", 10240, lib->ns);
	thresh_count = lib->settings->get_int(lib->settings,
						"%s.leak_detective.usage_threshold_count", 0, lib->ns);
	detailed = lib->settings->get_bool(lib->settings,
						"%s.leak_detective.detailed", TRUE, lib->ns);

	leaks = print_traces(this, cb, user, thresh, thresh_count,
						 detailed, &whitelisted, &sum);
	if (scb)
	{
		scb(user, leaks, sum, whitelisted);
	}
}

/**
 * Wrapped malloc() function
 */
HOOK(void*, malloc, size_t bytes)
{
	memory_header_t *hdr;
	memory_tail_t *tail;
	bool before;

	if (!enabled || thread_disabled->get(thread_disabled))
	{
		return real_malloc(bytes);
	}

	hdr = real_malloc(sizeof(memory_header_t) + bytes + sizeof(memory_tail_t));
	tail = ((void*)hdr) + bytes + sizeof(memory_header_t);
	/* set to something which causes crashes */
	memset(hdr, MEMORY_ALLOC_PATTERN,
		   sizeof(memory_header_t) + bytes + sizeof(memory_tail_t));

	before = enable_thread(FALSE);
	hdr->backtrace = backtrace_create(2);
	enable_thread(before);

	hdr->magic = MEMORY_HEADER_MAGIC;
	hdr->bytes = bytes;
	tail->magic = MEMORY_TAIL_MAGIC;

	add_hdr(hdr);

	return hdr + 1;
}

/**
 * Wrapped calloc() function
 */
HOOK(void*, calloc, size_t nmemb, size_t size)
{
	void *ptr;
	volatile size_t total;

	total = nmemb * size;
	ptr = malloc(total);
	memset(ptr, 0, total);

	return ptr;
}

/**
 * Wrapped valloc(), TODO: currently not supported
 */
HOOK(void*, valloc, size_t size)
{
	DBG1(DBG_LIB, "valloc() used, but leak-detective hook missing");
	return NULL;
}

/**
 * Wrapped free() function
 */
HOOK(void, free, void *ptr)
{
	memory_header_t *hdr;
	memory_tail_t *tail;
	backtrace_t *backtrace;
	bool before;

	if (!enabled || thread_disabled->get(thread_disabled))
	{
		/* after deinitialization we might have to free stuff we allocated
		 * while we were enabled */
		if (!first_header.magic && ptr)
		{
			hdr = ptr - sizeof(memory_header_t);
			tail = ptr + hdr->bytes;
			if (hdr->magic == MEMORY_HEADER_MAGIC &&
				tail->magic == MEMORY_TAIL_MAGIC)
			{
				ptr = hdr;
			}
		}
		real_free(ptr);
		return;
	}
	/* allow freeing of NULL */
	if (!ptr)
	{
		return;
	}
	hdr = ptr - sizeof(memory_header_t);
	tail = ptr + hdr->bytes;

	before = enable_thread(FALSE);
	if (hdr->magic != MEMORY_HEADER_MAGIC ||
		tail->magic != MEMORY_TAIL_MAGIC)
	{
		bool bt = TRUE;

		/* check if memory appears to be allocated by our hooks */
		if (has_hdr(hdr))
		{
			fprintf(stderr, "freeing corrupted memory (%p): "
					"%u bytes, header magic 0x%x, tail magic 0x%x:\n",
					ptr, hdr->bytes, hdr->magic, tail->magic);
			remove_hdr(hdr);

			if (hdr->magic == MEMORY_HEADER_MAGIC)
			{	/* only access the old backtrace if header magic is valid */
				hdr->backtrace->log(hdr->backtrace, stderr, TRUE);
				hdr->backtrace->destroy(hdr->backtrace);
			}
			else
			{
				fprintf(stderr, " header magic invalid, ignore backtrace of "
						"allocation\n");
			}
		}
		else
		{
			/* just free this block of unknown memory */
			hdr = ptr;

			if (ignore_unknown)
			{
				bt = FALSE;
			}
			else
			{
				fprintf(stderr, "freeing unknown memory (%p):\n", ptr);
			}
		}
		if (bt)
		{
			backtrace = backtrace_create(2);
			backtrace->log(backtrace, stderr, TRUE);
			backtrace->destroy(backtrace);
		}
	}
	else
	{
		remove_hdr(hdr);

		hdr->backtrace->destroy(hdr->backtrace);

		/* set mem to something remarkable */
		memset(hdr, MEMORY_FREE_PATTERN,
			   sizeof(memory_header_t) + hdr->bytes + sizeof(memory_tail_t));
	}
	real_free(hdr);
	enable_thread(before);
}

/**
 * Wrapped realloc() function
 */
HOOK(void*, realloc, void *old, size_t bytes)
{
	memory_header_t *hdr;
	memory_tail_t *tail;
	backtrace_t *backtrace;
	bool before, have_backtrace = TRUE;

	if (!enabled || thread_disabled->get(thread_disabled))
	{
		return real_realloc(old, bytes);
	}
	/* allow reallocation of NULL */
	if (!old)
	{
		return malloc(bytes);
	}
	/* handle zero size as a free() */
	if (!bytes)
	{
		free(old);
		return NULL;
	}

	hdr = old - sizeof(memory_header_t);
	tail = old + hdr->bytes;

	before = enable_thread(FALSE);
	if (hdr->magic != MEMORY_HEADER_MAGIC ||
		tail->magic != MEMORY_TAIL_MAGIC)
	{
		bool bt = TRUE;

		/* check if memory appears to be allocated by our hooks */
		if (has_hdr(hdr))
		{
			fprintf(stderr, "reallocating corrupted memory (%p, %u bytes): "
					"%zu bytes, header magic 0x%x, tail magic 0x%x:\n",
					old, hdr->bytes, bytes, hdr->magic, tail->magic);
			remove_hdr(hdr);

			if (hdr->magic == MEMORY_HEADER_MAGIC)
			{	/* only access header fields (backtrace, bytes) if header magic
				 * is still valid */
				hdr->backtrace->log(hdr->backtrace, stderr, TRUE);
				memset(&tail->magic, MEMORY_ALLOC_PATTERN, sizeof(tail->magic));
			}
			else
			{
				fprintf(stderr, " header magic invalid, ignore backtrace of "
						"allocation\n");
				have_backtrace = FALSE;
				hdr->magic = MEMORY_HEADER_MAGIC;
			}
		}
		else
		{
			/* adopt this block of unknown memory */
			hdr = old;
			have_backtrace = FALSE;

			if (ignore_unknown)
			{
				bt = FALSE;
			}
			else
			{
				fprintf(stderr, "reallocating unknown memory (%p): %zu bytes:\n",
						old, bytes);
			}
		}
		if (bt)
		{
			backtrace = backtrace_create(2);
			backtrace->log(backtrace, stderr, TRUE);
			backtrace->destroy(backtrace);
		}
	}
	else
	{
		remove_hdr(hdr);
		/* clear tail magic, allocate, set tail magic */
		memset(&tail->magic, MEMORY_ALLOC_PATTERN, sizeof(tail->magic));
	}

	hdr = real_realloc(hdr,
					   sizeof(memory_header_t) + bytes + sizeof(memory_tail_t));
	tail = ((void*)hdr) + bytes + sizeof(memory_header_t);
	tail->magic = MEMORY_TAIL_MAGIC;

	/* update statistics */
	hdr->bytes = bytes;

	if (have_backtrace)
	{
		hdr->backtrace->destroy(hdr->backtrace);
	}
	hdr->backtrace = backtrace_create(2);
	enable_thread(before);

	add_hdr(hdr);

	return hdr + 1;
}

METHOD(leak_detective_t, destroy, void,
	private_leak_detective_t *this)
{
	disable_leak_detective();
	lock->destroy(lock);
	thread_disabled->destroy(thread_disabled);
	free(this);
	first_header.magic = 0;
	first_header.next = NULL;
}

/*
 * see header file
 */
leak_detective_t *leak_detective_create()
{
	private_leak_detective_t *this;

	INIT(this,
		.public = {
			.report = _report,
			.set_report_cb = _set_report_cb,
			.usage = _usage,
			.leaks = _leaks,
			.set_state = _set_state,
			.destroy = _destroy,
		},
	);

	if (getenv("LEAK_DETECTIVE_DISABLE") != NULL)
	{
		free(this);
		return NULL;
	}
	ignore_unknown = getenv("LEAK_DETECTIVE_IGNORE_UNKNOWN") != NULL;

	lock = spinlock_create();
	thread_disabled = thread_value_create(NULL);

	init_static_allocations();

	if (register_hooks())
	{
		enable_leak_detective();
	}
	return &this->public;
}

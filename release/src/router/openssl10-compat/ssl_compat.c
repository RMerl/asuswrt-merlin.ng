#define _GNU_SOURCE
#include <dlfcn.h>
#include <stdio.h>
#include <stdlib.h>

#ifndef LIBCRYPTO_REAL_PATH
#define LIBCRYPTO_REAL_PATH "/usr/lib/libcrypto.so.3"
#endif
#ifndef LIBSSL_REAL_PATH
#define LIBSSL_REAL_PATH "/usr/lib/libssl.so.3"
#endif

static __attribute__((noreturn)) void compat_fatal(const char *what, const char *detail)
{
	fprintf(stderr, "openssl10-compat: %s: %s\n", what,
		detail != NULL ? detail : "unknown error");
	_Exit(127);
}

static const char *compat_getenv(const char *name)
{
#if defined(__GLIBC__)
	return secure_getenv(name);
#else
	(void)name;
	compat_fatal("unsupported libc environment security", "secure_getenv unavailable");
	return NULL;
#endif
}

static void reject_unsafe_openssl_env(void)
{
	static const char *const env_names[] = {
		"OPENSSL_CONF",
		"OPENSSL_CONF_INCLUDE",
		"OPENSSL_MODULES",
		"OPENSSL_ENGINES",
		"LD_PRELOAD",
		"LD_AUDIT",
		"LD_LIBRARY_PATH",
		"LD_ORIGIN_PATH",
		"LD_DEBUG",
		"LD_DEBUG_OUTPUT",
		"LD_PROFILE",
		"LD_ASSUME_KERNEL",
		"LD_HWCAP_MASK",
		"LD_DYNAMIC_WEAK",
	};
	size_t i;

	for (i = 0; i < sizeof(env_names) / sizeof(env_names[0]); ++i) {
		const char *value = compat_getenv(env_names[i]);
		if (value != NULL && *value != '\0')
			compat_fatal("refusing unsafe loader/OpenSSL runtime override from environment",
				env_names[i]);
	}
}

static void validate_ssl_backend(void)
{
	typedef unsigned long (*version_fn_t)(void);
	void *crypto_handle;
	void *ssl_handle;
	version_fn_t crypto_version;
	version_fn_t ssl_version;
	const char *err;

	reject_unsafe_openssl_env();
	crypto_handle = dlopen(LIBCRYPTO_REAL_PATH, RTLD_NOW | RTLD_LOCAL);
	if (crypto_handle == NULL)
		compat_fatal("dlopen(libcrypto.so.3) failed", dlerror());
	ssl_handle = dlopen(LIBSSL_REAL_PATH, RTLD_NOW | RTLD_LOCAL);
	if (ssl_handle == NULL)
		compat_fatal("dlopen(libssl.so.3) failed", dlerror());

	dlerror();
	crypto_version = (version_fn_t)dlsym(crypto_handle, "OpenSSL_version_num");
	err = dlerror();
	if (err != NULL || crypto_version == NULL || (crypto_version() >> 28) != 3)
		compat_fatal("unexpected libcrypto.so.3 backend", err);
	dlerror();
	ssl_version = (version_fn_t)dlsym(ssl_handle, "OpenSSL_version_num");
	err = dlerror();
	if (err != NULL || ssl_version == NULL || (ssl_version() >> 28) != 3)
		compat_fatal("unexpected libssl.so.3 backend", err);
	if ((void *)crypto_version != (void *)ssl_version)
		compat_fatal("libssl.so.3 resolved against an unexpected libcrypto.so.3 instance",
			LIBCRYPTO_REAL_PATH);

	dlclose(ssl_handle);
	dlclose(crypto_handle);
}

/* Disabled only by the dedicated concurrency test variant. */
#ifndef COMPAT_DISABLE_CONSTRUCTOR
static void __attribute__((constructor)) preload_ssl_compat(void)
{
	validate_ssl_backend();
}
#endif

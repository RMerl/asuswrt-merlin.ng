#define _GNU_SOURCE
#include <dlfcn.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* Keep the OpenSSL 3 declarations out of the legacy public ABI. */
#define CRYPTO_malloc CRYPTO_malloc__openssl3_decl
#define CRYPTO_free CRYPTO_free__openssl3_decl

#include <openssl/aes.h>
#include <openssl/bio.h>
#include <openssl/crypto.h>
#include <openssl/engine.h>
#include <openssl/err.h>
#include <openssl/evp.h>
#include <openssl/hmac.h>
#include <openssl/md5.h>
#include <openssl/pem.h>
#include <openssl/rsa.h>
#include <openssl/sha.h>
#include <openssl/x509.h>

#undef EVP_CIPHER_CTX_init
#undef EVP_CIPHER_CTX_cleanup
#undef EVP_MD_CTX_init
#undef EVP_MD_CTX_cleanup
#undef OPENSSL_add_all_algorithms_noconf
#undef CRYPTO_malloc
#undef CRYPTO_free
#undef BIO_set_flags

#ifndef LIBCRYPTO_REAL_PATH
#define LIBCRYPTO_REAL_PATH "/usr/lib/libcrypto.so.3"
#endif
#define COMPAT_EXPORT __attribute__((visibility("default")))

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

/* HND rc exports this fixed system-only search path to every service. */
static int trusted_hnd_library_path(const char *value)
{
	static const char *const trusted_dirs[] = {
		"/lib",
		"/usr/lib",
		"/lib/aarch64",
	};
	const char *component;

	if (value == NULL || *value == '\0')
		return 1;

	for (component = value;;) {
		const char *end = component;
		size_t length;
		size_t i;

		while (*end != '\0' && *end != ':')
			++end;
		length = (size_t)(end - component);
		if (length == 0)
			return 0;

		for (i = 0; i < sizeof(trusted_dirs) / sizeof(trusted_dirs[0]); ++i) {
			if (strlen(trusted_dirs[i]) == length &&
			    memcmp(component, trusted_dirs[i], length) == 0)
				break;
		}
		if (i == sizeof(trusted_dirs) / sizeof(trusted_dirs[0]))
			return 0;
		if (*end == '\0')
			return 1;
		component = end + 1;
	}
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
		"LD_ORIGIN_PATH",
		"LD_DEBUG",
		"LD_DEBUG_OUTPUT",
		"LD_PROFILE",
		"LD_ASSUME_KERNEL",
		"LD_HWCAP_MASK",
		"LD_DYNAMIC_WEAK",
	};
	const char *library_path;
	size_t i;

	library_path = compat_getenv("LD_LIBRARY_PATH");
	if (!trusted_hnd_library_path(library_path))
		compat_fatal("refusing unsafe loader search path from environment",
			"LD_LIBRARY_PATH");

	for (i = 0; i < sizeof(env_names) / sizeof(env_names[0]); ++i) {
		const char *value = compat_getenv(env_names[i]);
		if (value != NULL && *value != '\0')
			compat_fatal("refusing unsafe loader/OpenSSL runtime override from environment",
				env_names[i]);
	}
}

static void *volatile crypto_handle_cache;

static void validate_crypto_handle(void *handle)
{
	typedef unsigned long (*version_fn_t)(void);
	version_fn_t version_fn;
	const char *err;

	dlerror();
	version_fn = (version_fn_t)dlsym(handle, "OpenSSL_version_num");
	err = dlerror();
	if (err != NULL || version_fn == NULL)
		compat_fatal("missing libcrypto.so.3 version probe", err);
	if ((version_fn() >> 28) != 3)
		compat_fatal("unexpected libcrypto.so.3 major version", "expected OpenSSL 3.x");
}

static void *crypto_handle(void)
{
	void *loaded;
	void *expected;

	loaded = __atomic_load_n(&crypto_handle_cache, __ATOMIC_ACQUIRE);
	if (loaded != NULL)
		return loaded;

	reject_unsafe_openssl_env();
	loaded = dlopen(LIBCRYPTO_REAL_PATH, RTLD_NOW | RTLD_LOCAL);
	if (loaded == NULL)
		compat_fatal("dlopen(libcrypto.so.3) failed", dlerror());
	validate_crypto_handle(loaded);

	expected = NULL;
	if (!__atomic_compare_exchange_n(&crypto_handle_cache, &expected, loaded, 0,
					 __ATOMIC_RELEASE, __ATOMIC_ACQUIRE)) {
		dlclose(loaded);
		loaded = expected;
	}
	return loaded;
}

static void *crypto_sym(void **slot, const char *name)
{
	void *sym;
	const char *err;

	sym = __atomic_load_n(slot, __ATOMIC_ACQUIRE);
	if (sym != NULL)
		return sym;
	dlerror();
	sym = dlsym(crypto_handle(), name);
	err = dlerror();
	if (err != NULL || sym == NULL)
		compat_fatal("missing libcrypto.so.3 symbol", name);

	{
		void *expected = NULL;
		if (!__atomic_compare_exchange_n(slot, &expected, sym, 0,
						 __ATOMIC_RELEASE, __ATOMIC_ACQUIRE))
			sym = expected;
	}
	return sym;
}

static void *evp_cipher_ctx_new_sym;
static void *evp_cipher_ctx_free_sym;
static void *evp_cipher_ctx_reset_sym;
static void *evp_cipher_final_ex_sym;
static void *evp_cipher_init_ex_sym;
static void *evp_cipher_update_sym;
static void *evp_digest_final_ex_sym;
static void *evp_digest_init_ex_sym;
static void *evp_digest_update_sym;
static void *evp_md_ctx_new_sym;
static void *evp_md_ctx_free_sym;
static void *evp_md_ctx_reset_sym;
static void *hmac_ctx_new_sym;
static void *hmac_ctx_free_sym;
static void *hmac_final_sym;
static void *hmac_init_ex_sym;
static void *hmac_update_sym;
static void *openssl_init_crypto_sym;
static void *bio_ctrl_sym;
static void *bio_f_base64_sym;
static void *bio_free_sym;
static void *bio_free_all_sym;
static void *bio_new_sym;
static void *bio_new_file_sym;
static void *bio_new_mem_buf_sym;
static void *bio_push_sym;
static void *bio_read_sym;
static void *bio_s_file_sym;
static void *bio_s_mem_sym;
static void *bio_set_flags_sym;
static void *bio_write_sym;
static void *crypto_free_sym;
static void *crypto_malloc_sym;
static void *err_get_error_sym;
static void *err_reason_error_string_sym;
static void *evp_pkey_free_sym;
static void *evp_pkey_get1_rsa_sym;
static void *evp_aes_128_cbc_sym;
static void *evp_aes_256_cbc_sym;
static void *evp_md5_sym;
static void *evp_sha1_sym;
static void *pem_read_rsaprivatekey_sym;
static void *pem_read_rsa_pubkey_sym;
static void *pem_read_bio_x509_sym;
static void *rsa_free_sym;
static void *rsa_private_decrypt_sym;
static void *rsa_public_decrypt_sym;
static void *rsa_public_encrypt_sym;
static void *rsa_size_sym;
static void *x509_free_sym;
static void *x509_get_pubkey_sym;
static void *aes_cbc_encrypt_sym;
static void *aes_set_decrypt_key_sym;
static void *md5_one_shot_sym;
static void *sha256_one_shot_sym;

enum compat_context_kind {
	COMPAT_CIPHER_CONTEXT,
	COMPAT_MD_CONTEXT,
	COMPAT_HMAC_CONTEXT,
};

struct compat_context_entry {
	void *legacy;
	void *real;
	enum compat_context_kind kind;
	struct compat_context_entry *next;
};

static struct compat_context_entry *compat_context_entries;
static volatile unsigned char compat_context_lock;

static void compat_lock_acquire(void)
{
	while (__atomic_test_and_set(&compat_context_lock, __ATOMIC_ACQUIRE))
		;
}

static void compat_lock_release(void)
{
	__atomic_clear(&compat_context_lock, __ATOMIC_RELEASE);
}

static struct compat_context_entry *compat_find_locked(enum compat_context_kind kind,
	void *legacy)
{
	struct compat_context_entry *entry;

	for (entry = compat_context_entries; entry != NULL; entry = entry->next) {
		if (entry->kind == kind && entry->legacy == legacy)
			return entry;
	}
	return NULL;
}

static void compat_context_add(enum compat_context_kind kind, void *legacy, void *real,
	void (*free_fn)(void *))
{
	struct compat_context_entry *entry;

	entry = calloc(1, sizeof(*entry));
	if (entry == NULL) {
		free_fn(real);
		compat_fatal("unable to allocate compatibility context entry", NULL);
	}
	entry->kind = kind;
	entry->legacy = legacy;
	entry->real = real;

	compat_lock_acquire();
	if (compat_find_locked(kind, legacy) != NULL) {
		compat_lock_release();
		free(entry);
		free_fn(real);
		compat_fatal("duplicate legacy context initialization", NULL);
	}
	entry->next = compat_context_entries;
	compat_context_entries = entry;
	compat_lock_release();
}

static void *compat_context_remove(enum compat_context_kind kind, void *legacy)
{
	struct compat_context_entry **link;
	struct compat_context_entry *entry;
	void *real = NULL;

	compat_lock_acquire();
	for (link = &compat_context_entries; *link != NULL; link = &(*link)->next) {
		entry = *link;
		if (entry->kind == kind && entry->legacy == legacy) {
			*link = entry->next;
			real = entry->real;
			free(entry);
			break;
		}
	}
	compat_lock_release();
	return real;
}

static void *compat_context_real_locked(enum compat_context_kind kind, void *legacy)
{
	struct compat_context_entry *entry = compat_find_locked(kind, legacy);

	/*
	 * OpenSSL 1.0.x exposed these context layouts while OpenSSL 3 keeps
	 * them opaque.  A legacy address is only an identity token; it must
	 * never be passed to an OpenSSL 3 EVP/HMAC operation.
	 */
	return entry != NULL ? entry->real : NULL;
}

static void compat_cipher_init(EVP_CIPHER_CTX *legacy)
{
	typedef EVP_CIPHER_CTX *(*new_fn_t)(void);
	EVP_CIPHER_CTX *real;

	if (legacy == NULL)
		return;
	real = ((new_fn_t)crypto_sym(&evp_cipher_ctx_new_sym,
		"EVP_CIPHER_CTX_new"))();
	if (real == NULL)
		compat_fatal("unable to allocate EVP cipher context", NULL);
	compat_context_add(COMPAT_CIPHER_CONTEXT, legacy, real,
		(void (*)(void *))crypto_sym(&evp_cipher_ctx_free_sym, "EVP_CIPHER_CTX_free"));
}

static int compat_cipher_cleanup(EVP_CIPHER_CTX *legacy)
{
	typedef int (*reset_fn_t)(EVP_CIPHER_CTX *);
	typedef void (*free_fn_t)(EVP_CIPHER_CTX *);
	EVP_CIPHER_CTX *real;
	int ret;

	if (legacy == NULL)
		return 0;
	real = compat_context_remove(COMPAT_CIPHER_CONTEXT, legacy);
	if (real == NULL)
		return 1;
	ret = ((reset_fn_t)crypto_sym(&evp_cipher_ctx_reset_sym,
		"EVP_CIPHER_CTX_reset"))(real);
	((free_fn_t)crypto_sym(&evp_cipher_ctx_free_sym, "EVP_CIPHER_CTX_free"))(real);
	return ret;
}

static void compat_md_init(EVP_MD_CTX *legacy)
{
	typedef EVP_MD_CTX *(*new_fn_t)(void);
	EVP_MD_CTX *real;

	if (legacy == NULL)
		return;
	real = ((new_fn_t)crypto_sym(&evp_md_ctx_new_sym, "EVP_MD_CTX_new"))();
	if (real == NULL)
		compat_fatal("unable to allocate EVP digest context", NULL);
	compat_context_add(COMPAT_MD_CONTEXT, legacy, real,
		(void (*)(void *))crypto_sym(&evp_md_ctx_free_sym, "EVP_MD_CTX_free"));
}

static int compat_md_cleanup(EVP_MD_CTX *legacy)
{
	typedef int (*reset_fn_t)(EVP_MD_CTX *);
	typedef void (*free_fn_t)(EVP_MD_CTX *);
	EVP_MD_CTX *real;
	int ret;

	if (legacy == NULL)
		return 0;
	real = compat_context_remove(COMPAT_MD_CONTEXT, legacy);
	if (real == NULL)
		return 1;
	ret = ((reset_fn_t)crypto_sym(&evp_md_ctx_reset_sym,
		"EVP_MD_CTX_reset"))(real);
	((free_fn_t)crypto_sym(&evp_md_ctx_free_sym, "EVP_MD_CTX_free"))(real);
	return ret;
}

static void compat_hmac_init(HMAC_CTX *legacy)
{
	typedef HMAC_CTX *(*new_fn_t)(void);
	HMAC_CTX *real;

	if (legacy == NULL)
		return;
	real = ((new_fn_t)crypto_sym(&hmac_ctx_new_sym, "HMAC_CTX_new"))();
	if (real == NULL)
		compat_fatal("unable to allocate HMAC context", NULL);
	compat_context_add(COMPAT_HMAC_CONTEXT, legacy, real,
		(void (*)(void *))crypto_sym(&hmac_ctx_free_sym, "HMAC_CTX_free"));
}

static void compat_hmac_cleanup(HMAC_CTX *legacy)
{
	typedef void (*free_fn_t)(HMAC_CTX *);
	HMAC_CTX *real;

	if (legacy == NULL)
		return;
	real = compat_context_remove(COMPAT_HMAC_CONTEXT, legacy);
	if (real != NULL)
		((free_fn_t)crypto_sym(&hmac_ctx_free_sym, "HMAC_CTX_free"))(real);
}

/* Disabled only by the dedicated concurrency test variant. */
#ifndef COMPAT_DISABLE_CONSTRUCTOR
static void __attribute__((constructor)) preload_crypto_compat(void)
{
	(void)crypto_handle();
	(void)crypto_sym(&evp_cipher_ctx_new_sym, "EVP_CIPHER_CTX_new");
	(void)crypto_sym(&evp_cipher_ctx_free_sym, "EVP_CIPHER_CTX_free");
	(void)crypto_sym(&evp_cipher_ctx_reset_sym, "EVP_CIPHER_CTX_reset");
	(void)crypto_sym(&evp_cipher_final_ex_sym, "EVP_CipherFinal_ex");
	(void)crypto_sym(&evp_cipher_init_ex_sym, "EVP_CipherInit_ex");
	(void)crypto_sym(&evp_cipher_update_sym, "EVP_CipherUpdate");
	(void)crypto_sym(&evp_digest_final_ex_sym, "EVP_DigestFinal_ex");
	(void)crypto_sym(&evp_digest_init_ex_sym, "EVP_DigestInit_ex");
	(void)crypto_sym(&evp_digest_update_sym, "EVP_DigestUpdate");
	(void)crypto_sym(&evp_md_ctx_new_sym, "EVP_MD_CTX_new");
	(void)crypto_sym(&evp_md_ctx_free_sym, "EVP_MD_CTX_free");
	(void)crypto_sym(&evp_md_ctx_reset_sym, "EVP_MD_CTX_reset");
	(void)crypto_sym(&hmac_ctx_new_sym, "HMAC_CTX_new");
	(void)crypto_sym(&hmac_ctx_free_sym, "HMAC_CTX_free");
	(void)crypto_sym(&hmac_final_sym, "HMAC_Final");
	(void)crypto_sym(&hmac_init_ex_sym, "HMAC_Init_ex");
	(void)crypto_sym(&hmac_update_sym, "HMAC_Update");
	(void)crypto_sym(&openssl_init_crypto_sym, "OPENSSL_init_crypto");
	(void)crypto_sym(&bio_ctrl_sym, "BIO_ctrl");
	(void)crypto_sym(&bio_f_base64_sym, "BIO_f_base64");
	(void)crypto_sym(&bio_free_sym, "BIO_free");
	(void)crypto_sym(&bio_free_all_sym, "BIO_free_all");
	(void)crypto_sym(&bio_new_sym, "BIO_new");
	(void)crypto_sym(&bio_new_file_sym, "BIO_new_file");
	(void)crypto_sym(&bio_new_mem_buf_sym, "BIO_new_mem_buf");
	(void)crypto_sym(&bio_push_sym, "BIO_push");
	(void)crypto_sym(&bio_read_sym, "BIO_read");
	(void)crypto_sym(&bio_s_file_sym, "BIO_s_file");
	(void)crypto_sym(&bio_s_mem_sym, "BIO_s_mem");
	(void)crypto_sym(&bio_set_flags_sym, "BIO_set_flags");
	(void)crypto_sym(&bio_write_sym, "BIO_write");
	(void)crypto_sym(&crypto_free_sym, "CRYPTO_free");
	(void)crypto_sym(&crypto_malloc_sym, "CRYPTO_malloc");
	(void)crypto_sym(&err_get_error_sym, "ERR_get_error");
	(void)crypto_sym(&err_reason_error_string_sym, "ERR_reason_error_string");
	(void)crypto_sym(&evp_pkey_free_sym, "EVP_PKEY_free");
	(void)crypto_sym(&evp_pkey_get1_rsa_sym, "EVP_PKEY_get1_RSA");
	(void)crypto_sym(&evp_aes_128_cbc_sym, "EVP_aes_128_cbc");
	(void)crypto_sym(&evp_aes_256_cbc_sym, "EVP_aes_256_cbc");
	(void)crypto_sym(&evp_md5_sym, "EVP_md5");
	(void)crypto_sym(&evp_sha1_sym, "EVP_sha1");
	(void)crypto_sym(&pem_read_rsaprivatekey_sym, "PEM_read_RSAPrivateKey");
	(void)crypto_sym(&pem_read_rsa_pubkey_sym, "PEM_read_RSA_PUBKEY");
	(void)crypto_sym(&pem_read_bio_x509_sym, "PEM_read_bio_X509");
	(void)crypto_sym(&rsa_free_sym, "RSA_free");
	(void)crypto_sym(&rsa_private_decrypt_sym, "RSA_private_decrypt");
	(void)crypto_sym(&rsa_public_decrypt_sym, "RSA_public_decrypt");
	(void)crypto_sym(&rsa_public_encrypt_sym, "RSA_public_encrypt");
	(void)crypto_sym(&rsa_size_sym, "RSA_size");
	(void)crypto_sym(&x509_free_sym, "X509_free");
	(void)crypto_sym(&x509_get_pubkey_sym, "X509_get_pubkey");
	(void)crypto_sym(&aes_cbc_encrypt_sym, "AES_cbc_encrypt");
	(void)crypto_sym(&aes_set_decrypt_key_sym, "AES_set_decrypt_key");
	(void)crypto_sym(&md5_one_shot_sym, "MD5");
	(void)crypto_sym(&sha256_one_shot_sym, "SHA256");
}
#endif

COMPAT_EXPORT void EVP_CIPHER_CTX_init(EVP_CIPHER_CTX *legacy)
{
	compat_cipher_init(legacy);
}

COMPAT_EXPORT int EVP_CIPHER_CTX_cleanup(EVP_CIPHER_CTX *legacy)
{
	return compat_cipher_cleanup(legacy);
}

COMPAT_EXPORT int EVP_CipherInit_ex(EVP_CIPHER_CTX *legacy, const EVP_CIPHER *type,
	ENGINE *impl, const unsigned char *key, const unsigned char *iv, int enc)
{
	typedef int (*fn_t)(EVP_CIPHER_CTX *, const EVP_CIPHER *, ENGINE *,
		const unsigned char *, const unsigned char *, int);
	EVP_CIPHER_CTX *real;
	int ret;

	/* A legacy ENGINE object is not ABI-compatible with OpenSSL 3. */
	if (impl != NULL)
		return 0;
	compat_lock_acquire();
	real = compat_context_real_locked(COMPAT_CIPHER_CONTEXT, legacy);
	if (real == NULL) {
		compat_lock_release();
		return 0;
	}
	ret = ((fn_t)crypto_sym(&evp_cipher_init_ex_sym, "EVP_CipherInit_ex"))(
		real, type, NULL, key, iv, enc);
	compat_lock_release();
	return ret;
}

COMPAT_EXPORT int EVP_CipherUpdate(EVP_CIPHER_CTX *legacy, unsigned char *out,
	int *outl, const unsigned char *in, int inl)
{
	typedef int (*fn_t)(EVP_CIPHER_CTX *, unsigned char *, int *, const unsigned char *, int);
	EVP_CIPHER_CTX *real;
	int ret;

	compat_lock_acquire();
	real = compat_context_real_locked(COMPAT_CIPHER_CONTEXT, legacy);
	if (real == NULL) {
		compat_lock_release();
		return 0;
	}
	ret = ((fn_t)crypto_sym(&evp_cipher_update_sym, "EVP_CipherUpdate"))(
		real, out, outl, in, inl);
	compat_lock_release();
	return ret;
}

COMPAT_EXPORT int EVP_CipherFinal_ex(EVP_CIPHER_CTX *legacy, unsigned char *out,
	int *outl)
{
	typedef int (*fn_t)(EVP_CIPHER_CTX *, unsigned char *, int *);
	EVP_CIPHER_CTX *real;
	int ret;

	compat_lock_acquire();
	real = compat_context_real_locked(COMPAT_CIPHER_CONTEXT, legacy);
	if (real == NULL) {
		compat_lock_release();
		return 0;
	}
	ret = ((fn_t)crypto_sym(&evp_cipher_final_ex_sym, "EVP_CipherFinal_ex"))(
		real, out, outl);
	compat_lock_release();
	return ret;
}

COMPAT_EXPORT void EVP_MD_CTX_init(EVP_MD_CTX *legacy)
{
	compat_md_init(legacy);
}

COMPAT_EXPORT int EVP_MD_CTX_cleanup(EVP_MD_CTX *legacy)
{
	return compat_md_cleanup(legacy);
}

COMPAT_EXPORT int EVP_DigestInit_ex(EVP_MD_CTX *legacy, const EVP_MD *type,
	ENGINE *impl)
{
	typedef int (*fn_t)(EVP_MD_CTX *, const EVP_MD *, ENGINE *);
	EVP_MD_CTX *real;
	int ret;

	/* A legacy ENGINE object is not ABI-compatible with OpenSSL 3. */
	if (impl != NULL)
		return 0;
	compat_lock_acquire();
	real = compat_context_real_locked(COMPAT_MD_CONTEXT, legacy);
	if (real == NULL) {
		compat_lock_release();
		return 0;
	}
	ret = ((fn_t)crypto_sym(&evp_digest_init_ex_sym, "EVP_DigestInit_ex"))(
		real, type, NULL);
	compat_lock_release();
	return ret;
}

COMPAT_EXPORT int EVP_DigestUpdate(EVP_MD_CTX *legacy, const void *data, size_t len)
{
	typedef int (*fn_t)(EVP_MD_CTX *, const void *, size_t);
	EVP_MD_CTX *real;
	int ret;

	compat_lock_acquire();
	real = compat_context_real_locked(COMPAT_MD_CONTEXT, legacy);
	if (real == NULL) {
		compat_lock_release();
		return 0;
	}
	ret = ((fn_t)crypto_sym(&evp_digest_update_sym, "EVP_DigestUpdate"))(
		real, data, len);
	compat_lock_release();
	return ret;
}

COMPAT_EXPORT int EVP_DigestFinal_ex(EVP_MD_CTX *legacy, unsigned char *md,
	unsigned int *len)
{
	typedef int (*fn_t)(EVP_MD_CTX *, unsigned char *, unsigned int *);
	EVP_MD_CTX *real;
	int ret;

	compat_lock_acquire();
	real = compat_context_real_locked(COMPAT_MD_CONTEXT, legacy);
	if (real == NULL) {
		compat_lock_release();
		return 0;
	}
	ret = ((fn_t)crypto_sym(&evp_digest_final_ex_sym, "EVP_DigestFinal_ex"))(
		real, md, len);
	compat_lock_release();
	return ret;
}

COMPAT_EXPORT void HMAC_CTX_init(HMAC_CTX *legacy)
{
	compat_hmac_init(legacy);
}

COMPAT_EXPORT void HMAC_CTX_cleanup(HMAC_CTX *legacy)
{
	compat_hmac_cleanup(legacy);
}

COMPAT_EXPORT int HMAC_Init_ex(HMAC_CTX *legacy, const void *key, int len,
	const EVP_MD *md, ENGINE *impl)
{
	typedef int (*fn_t)(HMAC_CTX *, const void *, int, const EVP_MD *, ENGINE *);
	int ret;

	/* A legacy ENGINE object is not ABI-compatible with OpenSSL 3. */
	if (impl != NULL)
		return 0;
	compat_lock_acquire();
	if (compat_find_locked(COMPAT_HMAC_CONTEXT, legacy) == NULL) {
		compat_lock_release();
		return 0;
	}
	ret = ((fn_t)crypto_sym(&hmac_init_ex_sym, "HMAC_Init_ex"))(
		compat_context_real_locked(COMPAT_HMAC_CONTEXT, legacy), key, len, md, NULL);
	compat_lock_release();
	return ret;
}

/* OpenSSL 1.0.2 and OpenSSL 3 both define this length as size_t. */
COMPAT_EXPORT int HMAC_Update(HMAC_CTX *legacy, const unsigned char *data, size_t len)
{
	typedef int (*fn_t)(HMAC_CTX *, const unsigned char *, size_t);
	int ret;

	compat_lock_acquire();
	if (compat_find_locked(COMPAT_HMAC_CONTEXT, legacy) == NULL) {
		compat_lock_release();
		return 0;
	}
	ret = ((fn_t)crypto_sym(&hmac_update_sym, "HMAC_Update"))(
		compat_context_real_locked(COMPAT_HMAC_CONTEXT, legacy), data, len);
	compat_lock_release();
	return ret;
}

COMPAT_EXPORT int HMAC_Final(HMAC_CTX *legacy, unsigned char *md, unsigned int *len)
{
	typedef int (*fn_t)(HMAC_CTX *, unsigned char *, unsigned int *);
	int ret;

	compat_lock_acquire();
	if (compat_find_locked(COMPAT_HMAC_CONTEXT, legacy) == NULL) {
		compat_lock_release();
		return 0;
	}
	ret = ((fn_t)crypto_sym(&hmac_final_sym, "HMAC_Final"))(
		compat_context_real_locked(COMPAT_HMAC_CONTEXT, legacy), md, len);
	compat_lock_release();
	return ret;
}

COMPAT_EXPORT void OPENSSL_add_all_algorithms_noconf(void)
{
	typedef int (*fn_t)(uint64_t, const OPENSSL_INIT_SETTINGS *);

	(void)((fn_t)crypto_sym(&openssl_init_crypto_sym, "OPENSSL_init_crypto"))(
		OPENSSL_INIT_ADD_ALL_CIPHERS | OPENSSL_INIT_ADD_ALL_DIGESTS, NULL);
}

/*
 * The remainder of the RT-BE58_GO HNS import set has an unchanged OpenSSL
 * 3.x ABI.  Resolve every entry through the verified absolute backend rather
 * than relying on a generic libcrypto.so.3 DT_NEEDED dependency.
 */
#define WRAP0(ret, name, slot) \
	COMPAT_EXPORT ret name(void) { typedef ret (*fn_t)(void); return ((fn_t)crypto_sym(&(slot), #name))(); }
#define WRAP1(ret, name, slot, t1, a1) \
	COMPAT_EXPORT ret name(t1 a1) { typedef ret (*fn_t)(t1); return ((fn_t)crypto_sym(&(slot), #name))(a1); }
#define WRAP2(ret, name, slot, t1, a1, t2, a2) \
	COMPAT_EXPORT ret name(t1 a1, t2 a2) { typedef ret (*fn_t)(t1, t2); return ((fn_t)crypto_sym(&(slot), #name))(a1, a2); }
#define WRAP3(ret, name, slot, t1, a1, t2, a2, t3, a3) \
	COMPAT_EXPORT ret name(t1 a1, t2 a2, t3 a3) { typedef ret (*fn_t)(t1, t2, t3); return ((fn_t)crypto_sym(&(slot), #name))(a1, a2, a3); }
#define WRAP4(ret, name, slot, t1, a1, t2, a2, t3, a3, t4, a4) \
	COMPAT_EXPORT ret name(t1 a1, t2 a2, t3 a3, t4 a4) { typedef ret (*fn_t)(t1, t2, t3, t4); return ((fn_t)crypto_sym(&(slot), #name))(a1, a2, a3, a4); }
#define WRAP5(ret, name, slot, t1, a1, t2, a2, t3, a3, t4, a4, t5, a5) \
	COMPAT_EXPORT ret name(t1 a1, t2 a2, t3 a3, t4 a4, t5 a5) { typedef ret (*fn_t)(t1, t2, t3, t4, t5); return ((fn_t)crypto_sym(&(slot), #name))(a1, a2, a3, a4, a5); }
#define WRAP6(ret, name, slot, t1, a1, t2, a2, t3, a3, t4, a4, t5, a5, t6, a6) \
	COMPAT_EXPORT ret name(t1 a1, t2 a2, t3 a3, t4 a4, t5 a5, t6 a6) { typedef ret (*fn_t)(t1, t2, t3, t4, t5, t6); return ((fn_t)crypto_sym(&(slot), #name))(a1, a2, a3, a4, a5, a6); }
#define WRAPV1(name, slot, t1, a1) \
	COMPAT_EXPORT void name(t1 a1) { typedef void (*fn_t)(t1); ((fn_t)crypto_sym(&(slot), #name))(a1); }

WRAP6(void, AES_cbc_encrypt, aes_cbc_encrypt_sym, const unsigned char *, in, unsigned char *, out, size_t, length, const AES_KEY *, key, unsigned char *, ivec, const int, enc)
WRAP3(int, AES_set_decrypt_key, aes_set_decrypt_key_sym, const unsigned char *, user_key, const int, bits, AES_KEY *, key)
WRAP4(long, BIO_ctrl, bio_ctrl_sym, BIO *, bio, int, cmd, long, larg, void *, parg)
WRAP0(const BIO_METHOD *, BIO_f_base64, bio_f_base64_sym)
WRAP1(int, BIO_free, bio_free_sym, BIO *, bio)
WRAPV1(BIO_free_all, bio_free_all_sym, BIO *, bio)
WRAP1(BIO *, BIO_new, bio_new_sym, const BIO_METHOD *, type)
WRAP2(BIO *, BIO_new_file, bio_new_file_sym, const char *, filename, const char *, mode)
WRAP2(BIO *, BIO_new_mem_buf, bio_new_mem_buf_sym, const void *, buf, int, len)
WRAP2(BIO *, BIO_push, bio_push_sym, BIO *, bio, BIO *, append)
WRAP3(int, BIO_read, bio_read_sym, BIO *, bio, void *, data, int, dlen)
WRAP0(const BIO_METHOD *, BIO_s_file, bio_s_file_sym)
WRAP0(const BIO_METHOD *, BIO_s_mem, bio_s_mem_sym)
WRAP2(void, BIO_set_flags, bio_set_flags_sym, BIO *, bio, int, flags)
WRAP3(int, BIO_write, bio_write_sym, BIO *, bio, const void *, data, int, dlen)
/* OpenSSL 1.0.2 exposes a one-argument free and int-sized allocation. */
COMPAT_EXPORT void CRYPTO_free(void *ptr)
{
	typedef void (*fn_t)(void *, const char *, int);

	((fn_t)crypto_sym(&crypto_free_sym, "CRYPTO_free"))(ptr, NULL, 0);
}

COMPAT_EXPORT void *CRYPTO_malloc(int num, const char *file, int line)
{
	typedef void *(*fn_t)(size_t, const char *, int);

	if (num < 0)
		return NULL;
	return ((fn_t)crypto_sym(&crypto_malloc_sym, "CRYPTO_malloc"))(
		(size_t)num, file, line);
}
WRAP0(unsigned long, ERR_get_error, err_get_error_sym)
WRAP1(const char *, ERR_reason_error_string, err_reason_error_string_sym, unsigned long, e)
WRAPV1(EVP_PKEY_free, evp_pkey_free_sym, EVP_PKEY *, pkey)
WRAP1(RSA *, EVP_PKEY_get1_RSA, evp_pkey_get1_rsa_sym, EVP_PKEY *, pkey)
WRAP0(const EVP_CIPHER *, EVP_aes_128_cbc, evp_aes_128_cbc_sym)
WRAP0(const EVP_CIPHER *, EVP_aes_256_cbc, evp_aes_256_cbc_sym)
WRAP0(const EVP_MD *, EVP_md5, evp_md5_sym)
WRAP0(const EVP_MD *, EVP_sha1, evp_sha1_sym)
WRAP3(unsigned char *, MD5, md5_one_shot_sym, const unsigned char *, data, size_t, len, unsigned char *, md)
WRAP4(RSA *, PEM_read_RSAPrivateKey, pem_read_rsaprivatekey_sym, FILE *, fp, RSA **, x, pem_password_cb *, cb, void *, u)
WRAP4(RSA *, PEM_read_RSA_PUBKEY, pem_read_rsa_pubkey_sym, FILE *, fp, RSA **, x, pem_password_cb *, cb, void *, u)
WRAP4(X509 *, PEM_read_bio_X509, pem_read_bio_x509_sym, BIO *, bp, X509 **, x, pem_password_cb *, cb, void *, u)
WRAPV1(RSA_free, rsa_free_sym, RSA *, rsa)
WRAP5(int, RSA_private_decrypt, rsa_private_decrypt_sym, int, flen, const unsigned char *, from, unsigned char *, to, RSA *, rsa, int, padding)
WRAP5(int, RSA_public_decrypt, rsa_public_decrypt_sym, int, flen, const unsigned char *, from, unsigned char *, to, RSA *, rsa, int, padding)
WRAP5(int, RSA_public_encrypt, rsa_public_encrypt_sym, int, flen, const unsigned char *, from, unsigned char *, to, RSA *, rsa, int, padding)
WRAP1(int, RSA_size, rsa_size_sym, const RSA *, rsa)
WRAP3(unsigned char *, SHA256, sha256_one_shot_sym, const unsigned char *, data, size_t, len, unsigned char *, md)
WRAPV1(X509_free, x509_free_sym, X509 *, x509)
WRAP1(EVP_PKEY *, X509_get_pubkey, x509_get_pubkey_sym, X509 *, x509)

#define _GNU_SOURCE
#include <dlfcn.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

#define SSL_CTX_set_options SSL_CTX_set_options__openssl3_decl
#define SSL_set_options SSL_set_options__openssl3_decl
#include <openssl/crypto.h>
#include <openssl/ssl.h>
#include <openssl/x509.h>
#undef SSL_CTX_set_options
#undef SSL_set_options

#ifdef SSL_get_peer_certificate
#undef SSL_get_peer_certificate
#endif

#if defined(__arm__) || defined(__thumb__)
_Static_assert(sizeof(unsigned long) == 4,
	       "OpenSSL 1.1 compatibility shim expects 32-bit unsigned long on ARM");
#endif

static __attribute__((noreturn)) void compat_fatal(const char *what, const char *detail)
{
	fprintf(stderr, "openssl11-compat: %s: %s\n",
		what, detail != NULL ? detail : "unknown error");
	_Exit(127);
}

#define LIBCRYPTO_REAL_PATH "/usr/lib/libcrypto.so.3"
#define LIBSSL_REAL_PATH "/usr/lib/libssl.so.3"

static void *volatile ssl_handle_cache;

static void *ssl_ctx_free_sym;
static void *ssl_ctx_new_sym;
static void *ssl_ctx_check_private_key_sym;
static void *ssl_ctx_ctrl_sym;
static void *ssl_ctx_set_options_sym;
static void *ssl_ctx_set_cipher_list_sym;
static void *ssl_ctx_set_verify_sym;
static void *ssl_ctx_use_privatekey_file_sym;
static void *ssl_ctx_use_certificate_file_sym;
static void *ssl_ctx_use_certificate_chain_file_sym;
static void *ssl_accept_sym;
static void *ssl_connect_sym;
static void *ssl_ctrl_sym;
static void *ssl_free_sym;
static void *ssl_get_error_sym;
static void *ssl_get_verify_result_sym;
static void *ssl_pending_sym;
static void *ssl_new_sym;
static void *ssl_read_sym;
static void *ssl_set_fd_sym;
static void *ssl_set_verify_sym;
static void *ssl_shutdown_sym;
static void *ssl_write_sym;
static void *tls_client_method_sym;
static void *tls_server_method_sym;
static void *tls_method_sym;
static void *dtls_method_sym;
static void *dtls_server_method_sym;
static void *tlsv1_method_sym;
static void *tlsv1_server_method_sym;
static void *openssl_init_ssl_sym;
static void *ssl_get1_peer_certificate_sym;
static void *ssl_cipher_get_id_sym;
static void *ssl_cipher_get_name_sym;
static void *ssl_ctx_load_verify_locations_sym;
static void *ssl_ctx_set_cookie_generate_cb_sym;
static void *ssl_ctx_set_cookie_verify_cb_sym;
static void *ssl_ctx_set_default_passwd_cb_sym;
static void *ssl_ctx_set_default_passwd_cb_userdata_sym;
static void *ssl_ctx_set_verify_depth_sym;
static void *ssl_do_handshake_sym;
static void *ssl_get_certificate_sym;
static void *ssl_get_ciphers_sym;
static void *ssl_get_current_cipher_sym;
static void *ssl_get_ex_data_sym;
static void *ssl_get_ex_data_x509_store_ctx_idx_sym;
static void *ssl_is_init_finished_sym;
static void *ssl_renegotiate_sym;
static void *ssl_renegotiate_pending_sym;
static void *ssl_set_accept_state_sym;
static void *ssl_set_bio_sym;
static void *ssl_set_cipher_list_sym;
static void *ssl_set_connect_state_sym;
static void *ssl_set_ex_data_sym;
static void *ssl_set_options_sym;

static const char *compat_getenv(const char *name)
{
#if defined(__GLIBC__)
	return secure_getenv(name);
#else
	return getenv(name);
#endif
}

static void reject_unsafe_openssl_env(void)
{
	static const char *const env_names[] = {
		"OPENSSL_CONF",
		"OPENSSL_CONF_INCLUDE",
		"OPENSSL_MODULES",
		"OPENSSL_ENGINES",
	};
	size_t i;

	for (i = 0; i < (sizeof(env_names) / sizeof(env_names[0])); ++i) {
		const char *value = compat_getenv(env_names[i]);
		if (value != NULL && *value != '\0')
			compat_fatal("refusing OpenSSL runtime override from environment", env_names[i]);
	}
}

static void *ssl_sym(void **slot, const char *name);

static void validate_ssl_handle(void *ssl_handle, void *crypto_handle)
{
	typedef unsigned long (*version_fn_t)(void);
	version_fn_t version_fn;
	version_fn_t crypto_version_fn;
	version_fn_t ssl_version_fn;
	const char *err;
	unsigned long version;

	dlerror();
	version_fn = (version_fn_t)dlsym(ssl_handle, "OpenSSL_version_num");
	err = dlerror();
	if (err != NULL || version_fn == NULL)
		compat_fatal("missing libssl.so.3 version probe", err);

	version = version_fn();
	if ((version >> 28) != 3)
		compat_fatal("unexpected libssl.so.3 major version", "expected OpenSSL 3.x");

	dlerror();
	crypto_version_fn = (version_fn_t)dlsym(crypto_handle, "OpenSSL_version_num");
	err = dlerror();
	if (err != NULL || crypto_version_fn == NULL)
		compat_fatal("missing libcrypto.so.3 version probe", err);

	dlerror();
	ssl_version_fn = (version_fn_t)dlsym(ssl_handle, "OpenSSL_version_num");
	err = dlerror();
	if (err != NULL || ssl_version_fn == NULL)
		compat_fatal("missing libcrypto dependency probe from libssl.so.3", err);

	if ((void *)crypto_version_fn != (void *)ssl_version_fn)
		compat_fatal("libssl.so.3 resolved against an unexpected libcrypto.so.3 instance",
			     LIBCRYPTO_REAL_PATH);
}

static void *ssl_handle(void)
{
	void *loaded;
	void *expected;
	void *crypto_handle;

	loaded = __atomic_load_n(&ssl_handle_cache, __ATOMIC_ACQUIRE);
	if (loaded != NULL)
		return loaded;

	reject_unsafe_openssl_env();

	crypto_handle = dlopen(LIBCRYPTO_REAL_PATH, RTLD_NOW | RTLD_LOCAL);
	if (crypto_handle == NULL)
		compat_fatal("dlopen(libcrypto.so.3) failed", dlerror());

	loaded = dlopen(LIBSSL_REAL_PATH, RTLD_NOW | RTLD_LOCAL);
	if (loaded == NULL)
		compat_fatal("dlopen(libssl.so.3) failed", dlerror());

	validate_ssl_handle(loaded, crypto_handle);
	/* libssl.so.3 retains its dependency reference; release our validation
	 * reference before publishing or discarding the SSL handle. */
	dlclose(crypto_handle);
	crypto_handle = NULL;

	expected = NULL;
	if (!__atomic_compare_exchange_n(&ssl_handle_cache, &expected, loaded, 0,
					 __ATOMIC_RELEASE, __ATOMIC_ACQUIRE)) {
		dlclose(loaded);
		loaded = expected;
	}

	return loaded;
}

static void *ssl_sym(void **slot, const char *name)
{
	void *sym;
	const char *err;

	sym = __atomic_load_n(slot, __ATOMIC_ACQUIRE);
	if (sym != NULL)
		return sym;

	dlerror();
	sym = dlsym(ssl_handle(), name);
	err = dlerror();
	if (err != NULL || sym == NULL)
		compat_fatal("missing libssl.so.3 symbol", name);

	{
		void *expected = NULL;
		if (!__atomic_compare_exchange_n(slot, &expected, sym, 0,
						 __ATOMIC_RELEASE, __ATOMIC_ACQUIRE))
			sym = expected;
	}

	return sym;
}

static void preload_ssl_symbols(void)
{
	(void)ssl_handle();
	(void)ssl_sym(&ssl_ctx_free_sym, "SSL_CTX_free");
	(void)ssl_sym(&ssl_ctx_new_sym, "SSL_CTX_new");
	(void)ssl_sym(&ssl_ctx_check_private_key_sym, "SSL_CTX_check_private_key");
	(void)ssl_sym(&ssl_ctx_ctrl_sym, "SSL_CTX_ctrl");
	(void)ssl_sym(&ssl_ctx_set_options_sym, "SSL_CTX_set_options");
	(void)ssl_sym(&ssl_ctx_set_cipher_list_sym, "SSL_CTX_set_cipher_list");
	(void)ssl_sym(&ssl_ctx_set_verify_sym, "SSL_CTX_set_verify");
	(void)ssl_sym(&ssl_ctx_use_privatekey_file_sym, "SSL_CTX_use_PrivateKey_file");
	(void)ssl_sym(&ssl_ctx_use_certificate_file_sym, "SSL_CTX_use_certificate_file");
	(void)ssl_sym(&ssl_ctx_use_certificate_chain_file_sym, "SSL_CTX_use_certificate_chain_file");
	(void)ssl_sym(&ssl_accept_sym, "SSL_accept");
	(void)ssl_sym(&ssl_connect_sym, "SSL_connect");
	(void)ssl_sym(&ssl_ctrl_sym, "SSL_ctrl");
	(void)ssl_sym(&ssl_free_sym, "SSL_free");
	(void)ssl_sym(&ssl_get_error_sym, "SSL_get_error");
	(void)ssl_sym(&ssl_get_verify_result_sym, "SSL_get_verify_result");
	(void)ssl_sym(&ssl_pending_sym, "SSL_pending");
	(void)ssl_sym(&ssl_new_sym, "SSL_new");
	(void)ssl_sym(&ssl_read_sym, "SSL_read");
	(void)ssl_sym(&ssl_set_fd_sym, "SSL_set_fd");
	(void)ssl_sym(&ssl_set_verify_sym, "SSL_set_verify");
	(void)ssl_sym(&ssl_shutdown_sym, "SSL_shutdown");
	(void)ssl_sym(&ssl_write_sym, "SSL_write");
	(void)ssl_sym(&tls_client_method_sym, "TLS_client_method");
	(void)ssl_sym(&tls_server_method_sym, "TLS_server_method");
	(void)ssl_sym(&tls_method_sym, "TLS_method");
	(void)ssl_sym(&dtls_method_sym, "DTLS_method");
	(void)ssl_sym(&dtls_server_method_sym, "DTLS_server_method");
	(void)ssl_sym(&tlsv1_method_sym, "TLSv1_method");
	(void)ssl_sym(&tlsv1_server_method_sym, "TLSv1_server_method");
	(void)ssl_sym(&openssl_init_ssl_sym, "OPENSSL_init_ssl");
	(void)ssl_sym(&ssl_get1_peer_certificate_sym, "SSL_get1_peer_certificate");
	(void)ssl_sym(&ssl_cipher_get_id_sym, "SSL_CIPHER_get_id");
	(void)ssl_sym(&ssl_cipher_get_name_sym, "SSL_CIPHER_get_name");
	(void)ssl_sym(&ssl_ctx_load_verify_locations_sym, "SSL_CTX_load_verify_locations");
	(void)ssl_sym(&ssl_ctx_set_cookie_generate_cb_sym, "SSL_CTX_set_cookie_generate_cb");
	(void)ssl_sym(&ssl_ctx_set_cookie_verify_cb_sym, "SSL_CTX_set_cookie_verify_cb");
	(void)ssl_sym(&ssl_ctx_set_default_passwd_cb_sym, "SSL_CTX_set_default_passwd_cb");
	(void)ssl_sym(&ssl_ctx_set_default_passwd_cb_userdata_sym, "SSL_CTX_set_default_passwd_cb_userdata");
	(void)ssl_sym(&ssl_ctx_set_verify_depth_sym, "SSL_CTX_set_verify_depth");
	(void)ssl_sym(&ssl_do_handshake_sym, "SSL_do_handshake");
	(void)ssl_sym(&ssl_get_certificate_sym, "SSL_get_certificate");
	(void)ssl_sym(&ssl_get_ciphers_sym, "SSL_get_ciphers");
	(void)ssl_sym(&ssl_get_current_cipher_sym, "SSL_get_current_cipher");
	(void)ssl_sym(&ssl_get_ex_data_sym, "SSL_get_ex_data");
	(void)ssl_sym(&ssl_get_ex_data_x509_store_ctx_idx_sym, "SSL_get_ex_data_X509_STORE_CTX_idx");
	(void)ssl_sym(&ssl_is_init_finished_sym, "SSL_is_init_finished");
	(void)ssl_sym(&ssl_renegotiate_sym, "SSL_renegotiate");
	(void)ssl_sym(&ssl_renegotiate_pending_sym, "SSL_renegotiate_pending");
	(void)ssl_sym(&ssl_set_accept_state_sym, "SSL_set_accept_state");
	(void)ssl_sym(&ssl_set_bio_sym, "SSL_set_bio");
	(void)ssl_sym(&ssl_set_cipher_list_sym, "SSL_set_cipher_list");
	(void)ssl_sym(&ssl_set_connect_state_sym, "SSL_set_connect_state");
	(void)ssl_sym(&ssl_set_ex_data_sym, "SSL_set_ex_data");
	(void)ssl_sym(&ssl_set_options_sym, "SSL_set_options");
}

static void __attribute__((constructor)) preload_ssl_compat(void)
{
	preload_ssl_symbols();
}

#define COMPAT_EXPORT __attribute__((visibility("default")))

#define WRAP0(ret, name, slot) \
	COMPAT_EXPORT ret name(void) { typedef ret (*fn_t)(void); return ((fn_t)ssl_sym(&(slot), #name))(); }
#define WRAP1(ret, name, slot, t1, a1) \
	COMPAT_EXPORT ret name(t1 a1) { typedef ret (*fn_t)(t1); return ((fn_t)ssl_sym(&(slot), #name))(a1); }
#define WRAP2(ret, name, slot, t1, a1, t2, a2) \
	COMPAT_EXPORT ret name(t1 a1, t2 a2) { typedef ret (*fn_t)(t1, t2); return ((fn_t)ssl_sym(&(slot), #name))(a1, a2); }
#define WRAP3(ret, name, slot, t1, a1, t2, a2, t3, a3) \
	COMPAT_EXPORT ret name(t1 a1, t2 a2, t3 a3) { typedef ret (*fn_t)(t1, t2, t3); return ((fn_t)ssl_sym(&(slot), #name))(a1, a2, a3); }
#define WRAP4(ret, name, slot, t1, a1, t2, a2, t3, a3, t4, a4) \
	COMPAT_EXPORT ret name(t1 a1, t2 a2, t3 a3, t4 a4) { typedef ret (*fn_t)(t1, t2, t3, t4); return ((fn_t)ssl_sym(&(slot), #name))(a1, a2, a3, a4); }
#define WRAPV1(name, slot, t1, a1) \
	COMPAT_EXPORT void name(t1 a1) { typedef void (*fn_t)(t1); ((fn_t)ssl_sym(&(slot), #name))(a1); }
#define WRAPV3(name, slot, t1, a1, t2, a2, t3, a3) \
	COMPAT_EXPORT void name(t1 a1, t2 a2, t3 a3) { typedef void (*fn_t)(t1, t2, t3); ((fn_t)ssl_sym(&(slot), #name))(a1, a2, a3); }

WRAPV1(SSL_CTX_free, ssl_ctx_free_sym, SSL_CTX *, ctx)
WRAP1(SSL_CTX *, SSL_CTX_new, ssl_ctx_new_sym, const SSL_METHOD *, method)
WRAP1(int, SSL_CTX_check_private_key, ssl_ctx_check_private_key_sym, const SSL_CTX *, ctx)
WRAP4(long, SSL_CTX_ctrl, ssl_ctx_ctrl_sym, SSL_CTX *, ctx, int, cmd, long, larg, void *, parg)
WRAP2(int, SSL_CTX_set_cipher_list, ssl_ctx_set_cipher_list_sym, SSL_CTX *, ctx, const char *, str)
WRAPV3(SSL_CTX_set_verify, ssl_ctx_set_verify_sym, SSL_CTX *, ctx, int, mode, SSL_verify_cb, callback)
WRAP3(int, SSL_CTX_use_PrivateKey_file, ssl_ctx_use_privatekey_file_sym, SSL_CTX *, ctx, const char *, file, int, type)
WRAP3(int, SSL_CTX_use_certificate_file, ssl_ctx_use_certificate_file_sym, SSL_CTX *, ctx, const char *, file, int, type)
WRAP2(int, SSL_CTX_use_certificate_chain_file, ssl_ctx_use_certificate_chain_file_sym, SSL_CTX *, ctx, const char *, file)
WRAP1(int, SSL_accept, ssl_accept_sym, SSL *, ssl)
WRAP1(int, SSL_connect, ssl_connect_sym, SSL *, ssl)
WRAP4(long, SSL_ctrl, ssl_ctrl_sym, SSL *, ssl, int, cmd, long, larg, void *, parg)
WRAPV1(SSL_free, ssl_free_sym, SSL *, ssl)
WRAP2(int, SSL_get_error, ssl_get_error_sym, const SSL *, ssl, int, ret)
WRAP1(long, SSL_get_verify_result, ssl_get_verify_result_sym, const SSL *, ssl)
WRAP1(int, SSL_pending, ssl_pending_sym, const SSL *, ssl)
WRAP1(SSL *, SSL_new, ssl_new_sym, SSL_CTX *, ctx)
WRAP3(int, SSL_read, ssl_read_sym, SSL *, ssl, void *, buf, int, num)
WRAP2(int, SSL_set_fd, ssl_set_fd_sym, SSL *, ssl, int, fd)
WRAPV3(SSL_set_verify, ssl_set_verify_sym, SSL *, ssl, int, mode, SSL_verify_cb, callback)
WRAP1(int, SSL_shutdown, ssl_shutdown_sym, SSL *, ssl)
WRAP3(int, SSL_write, ssl_write_sym, SSL *, ssl, const void *, buf, int, num)
WRAP0(const SSL_METHOD *, TLS_client_method, tls_client_method_sym)
WRAP0(const SSL_METHOD *, TLS_server_method, tls_server_method_sym)
WRAP0(const SSL_METHOD *, TLS_method, tls_method_sym)
WRAP0(const SSL_METHOD *, DTLS_method, dtls_method_sym)
WRAP0(const SSL_METHOD *, DTLS_server_method, dtls_server_method_sym)
WRAP0(const SSL_METHOD *, TLSv1_method, tlsv1_method_sym)
WRAP0(const SSL_METHOD *, TLSv1_server_method, tlsv1_server_method_sym)

COMPAT_EXPORT unsigned long SSL_CTX_set_options(SSL_CTX *ctx, unsigned long op)
{
	typedef uint64_t (*fn_t)(SSL_CTX *, uint64_t);
	uint64_t ret;

	ret = ((fn_t)ssl_sym(&ssl_ctx_set_options_sym, "SSL_CTX_set_options"))(ctx, (uint64_t)op);
	return (unsigned long)ret;
}

COMPAT_EXPORT int OPENSSL_init_ssl(uint64_t opts, const OPENSSL_INIT_SETTINGS *settings)
{
	typedef int (*fn_t)(uint64_t, const OPENSSL_INIT_SETTINGS *);

	return ((fn_t)ssl_sym(&openssl_init_ssl_sym, "OPENSSL_init_ssl"))(opts, settings);
}

COMPAT_EXPORT X509 *SSL_get_peer_certificate(const SSL *ssl)
{
	typedef X509 *(*fn_t)(const SSL *);

	return ((fn_t)ssl_sym(&ssl_get1_peer_certificate_sym, "SSL_get1_peer_certificate"))(ssl);
}

WRAP1(uint32_t, SSL_CIPHER_get_id, ssl_cipher_get_id_sym, const SSL_CIPHER *, c)
WRAP1(const char *, SSL_CIPHER_get_name, ssl_cipher_get_name_sym, const SSL_CIPHER *, c)
WRAP3(int, SSL_CTX_load_verify_locations, ssl_ctx_load_verify_locations_sym, SSL_CTX *, ctx, const char *, ca_file, const char *, ca_dir)
WRAP2(void, SSL_CTX_set_default_passwd_cb, ssl_ctx_set_default_passwd_cb_sym, SSL_CTX *, ctx, pem_password_cb *, cb)
WRAP2(void, SSL_CTX_set_default_passwd_cb_userdata, ssl_ctx_set_default_passwd_cb_userdata_sym, SSL_CTX *, ctx, void *, u)
WRAP2(void, SSL_CTX_set_verify_depth, ssl_ctx_set_verify_depth_sym, SSL_CTX *, ctx, int, depth)
WRAP1(int, SSL_do_handshake, ssl_do_handshake_sym, SSL *, ssl)
WRAP1(X509 *, SSL_get_certificate, ssl_get_certificate_sym, const SSL *, ssl)
WRAP1(STACK_OF(SSL_CIPHER) *, SSL_get_ciphers, ssl_get_ciphers_sym, const SSL *, ssl)
WRAP1(const SSL_CIPHER *, SSL_get_current_cipher, ssl_get_current_cipher_sym, const SSL *, ssl)
WRAP2(void *, SSL_get_ex_data, ssl_get_ex_data_sym, const SSL *, ssl, int, idx)
WRAP0(int, SSL_get_ex_data_X509_STORE_CTX_idx, ssl_get_ex_data_x509_store_ctx_idx_sym)
WRAP1(int, SSL_is_init_finished, ssl_is_init_finished_sym, const SSL *, ssl)
WRAP1(int, SSL_renegotiate, ssl_renegotiate_sym, SSL *, ssl)
WRAP1(int, SSL_renegotiate_pending, ssl_renegotiate_pending_sym, const SSL *, ssl)
WRAPV1(SSL_set_accept_state, ssl_set_accept_state_sym, SSL *, ssl)
WRAP3(void, SSL_set_bio, ssl_set_bio_sym, SSL *, ssl, BIO *, rbio, BIO *, wbio)
WRAP2(int, SSL_set_cipher_list, ssl_set_cipher_list_sym, SSL *, ssl, const char *, str)
WRAPV1(SSL_set_connect_state, ssl_set_connect_state_sym, SSL *, ssl)
WRAP3(int, SSL_set_ex_data, ssl_set_ex_data_sym, SSL *, ssl, int, idx, void *, data)

COMPAT_EXPORT void SSL_CTX_set_cookie_generate_cb(SSL_CTX *ctx,
	int (*cb)(SSL *, unsigned char *, unsigned int *))
{
	typedef void (*fn_t)(SSL_CTX *, int (*)(SSL *, unsigned char *, unsigned int *));
	((fn_t)ssl_sym(&ssl_ctx_set_cookie_generate_cb_sym, "SSL_CTX_set_cookie_generate_cb"))(ctx, cb);
}

COMPAT_EXPORT void SSL_CTX_set_cookie_verify_cb(SSL_CTX *ctx,
	int (*cb)(SSL *, const unsigned char *, unsigned int))
{
	typedef void (*fn_t)(SSL_CTX *, int (*)(SSL *, const unsigned char *, unsigned int));
	((fn_t)ssl_sym(&ssl_ctx_set_cookie_verify_cb_sym, "SSL_CTX_set_cookie_verify_cb"))(ctx, cb);
}

COMPAT_EXPORT unsigned long SSL_set_options(SSL *ssl, unsigned long options)
{
	typedef uint64_t (*fn_t)(SSL *, uint64_t);
	uint64_t ret;

	ret = ((fn_t)ssl_sym(&ssl_set_options_sym, "SSL_set_options"))(ssl,
		(uint64_t)options);
	return (unsigned long)ret;
}

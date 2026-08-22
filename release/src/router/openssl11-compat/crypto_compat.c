#define _GNU_SOURCE
#include <dlfcn.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

#define CRYPTO_get_ex_new_index CRYPTO_get_ex_new_index__openssl3_decl
#include <openssl/bio.h>
#include <openssl/bn.h>
#include <openssl/crypto.h>
#include <openssl/dsa.h>
#include <openssl/ec.h>
#include <openssl/err.h>
#include <openssl/engine.h>
#include <openssl/evp.h>
#include <openssl/hmac.h>
#include <openssl/md5.h>
#include <openssl/pem.h>
#include <openssl/rand.h>
#include <openssl/rsa.h>
#include <openssl/sha.h>
#include <openssl/x509.h>
#undef CRYPTO_get_ex_new_index

#ifdef EVP_CIPHER_CTX_block_size
#undef EVP_CIPHER_CTX_block_size
#endif
#ifdef EVP_PKEY_id
#undef EVP_PKEY_id
#endif
#ifdef EVP_CIPHER_CTX_init
#undef EVP_CIPHER_CTX_init
#endif
#ifdef EVP_CIPHER_CTX_cleanup
#undef EVP_CIPHER_CTX_cleanup
#endif
#ifdef EVP_MD_CTX_init
#undef EVP_MD_CTX_init
#endif
#ifdef EVP_MD_CTX_cleanup
#undef EVP_MD_CTX_cleanup
#endif
#ifdef OPENSSL_add_all_algorithms_noconf
#undef OPENSSL_add_all_algorithms_noconf
#endif
#ifdef CRYPTO_malloc
#undef CRYPTO_malloc
#endif
#ifdef CRYPTO_free
#undef CRYPTO_free
#endif

/*
 * Report a stable OpenSSL 1.1.1 compatibility version to legacy callers.
 * The shim forwards to OpenSSL 3.5 internally, but callers loaded against
 * libcrypto.so.1.1 should not see a 3.x version number.
 */
#define COMPAT_OPENSSL_VERSION_NUM 0x1010100fUL
#define LIBCRYPTO_REAL_PATH "/usr/lib/libcrypto.so.3"

static __attribute__((noreturn)) void compat_fatal(const char *what, const char *detail)
{
	fprintf(stderr, "openssl11-compat: %s: %s\n",
		what, detail != NULL ? detail : "unknown error");
	_Exit(127);
}

static void *volatile crypto_handle_cache;

static void *bio_free_sym;
static void *bio_new_mem_buf_sym;
static void *bn_cmp_sym;
static void *dsa_free_sym;
static void *ec_key_free_sym;
static void *ec_key_new_by_curve_name_sym;
static void *ec_key_get0_group_sym;
static void *ec_key_get0_public_key_sym;
static void *ec_point_cmp_sym;
static void *evp_cipher_ctx_free_sym;
static void *evp_cipher_ctx_new_sym;
static void *evp_decryptfinal_ex_sym;
static void *evp_decryptinit_ex_sym;
static void *evp_decryptupdate_sym;
static void *evp_encryptfinal_ex_sym;
static void *evp_encryptinit_ex_sym;
static void *evp_encryptupdate_sym;
static void *evp_pkey_free_sym;
static void *evp_pkey_get1_dsa_sym;
static void *evp_pkey_get1_ec_key_sym;
static void *evp_pkey_get1_rsa_sym;
static void *evp_aes_256_ecb_sym;
static void *err_print_errors_fp_sym;
static void *md5_final_sym;
static void *md5_init_sym;
static void *md5_update_sym;
static void *pem_read_privatekey_sym;
static void *pem_read_rsaprivatekey_sym;
static void *pem_read_rsa_pubkey_sym;
static void *pem_read_x509_sym;
static void *pem_read_bio_rsaprivatekey_sym;
static void *pem_read_bio_rsa_pubkey_sym;
static void *rand_bytes_sym;
static void *rsa_free_sym;
static void *rsa_new_sym;
static void *rsa_get0_n_sym;
static void *rsa_private_decrypt_sym;
static void *rsa_private_encrypt_sym;
static void *rsa_public_decrypt_sym;
static void *rsa_public_encrypt_sym;
static void *rsa_size_sym;
static void *sha256_final_sym;
static void *sha256_init_sym;
static void *sha256_update_sym;
static void *x509_name_oneline_sym;
static void *x509_free_sym;
static void *x509_get_issuer_name_sym;
static void *x509_get_pubkey_sym;
static void *x509_get_subject_name_sym;
static void *x509_getm_notafter_sym;
static void *dsa_get0_pub_key_sym;
static void *d2i_privatekey_fp_sym;
static void *d2i_x509_fp_sym;
static void *evp_cipher_ctx_get_block_size_sym;
static void *evp_pkey_get_id_sym;
static void *bio_ctrl_sym;
static void *bio_f_base64_sym;
static void *bio_free_all_sym;
static void *bio_new_sym;
static void *bio_new_fp_sym;
static void *bio_new_file_sym;
static void *bio_push_sym;
static void *bio_read_sym;
static void *bio_s_file_sym;
static void *bio_s_mem_sym;
static void *bio_set_flags_sym;
static void *bio_write_sym;
static void *asn1_string_data_sym;
static void *asn1_string_get0_data_sym;
static void *asn1_string_length_sym;
static void *asn1_string_to_utf8_sym;
static void *crypto_free_sym;
static void *crypto_get_ex_new_index_sym;
static void *crypto_malloc_sym;
static void *evp_cipher_ctx_set_padding_sym;
static void *evp_cipher_ctx_reset_sym;
static void *evp_md_ctx_reset_sym;
static void *evp_cipher_final_ex_sym;
static void *evp_cipher_init_ex_sym;
static void *evp_cipher_update_sym;
static void *evp_digest_final_ex_sym;
static void *evp_digest_init_ex_sym;
static void *evp_digest_update_sym;
static void *evp_aes_128_cbc_sym;
static void *evp_aes_256_cbc_sym;
static void *evp_md5_sym;
static void *err_clear_error_sym;
static void *err_error_string_n_sym;
static void *err_get_error_sym;
static void *err_load_bio_strings_sym;
static void *err_reason_error_string_sym;
static void *evp_sha1_sym;
static void *evp_sha256_sym;
static void *md5_one_shot_sym;
static void *openssl_cleanse_sym;
static void *openssl_version_sym;
static void *hmac_sym;
static void *hmac_ctx_new_sym;
static void *hmac_ctx_free_sym;
static void *hmac_init_ex_sym;
static void *hmac_update_sym;
static void *hmac_final_sym;
static void *openssl_die_sym;
static void *openssl_init_crypto_sym;
static void *openssl_sk_num_sym;
static void *openssl_sk_value_sym;
static void *rsa_verify_sym;
static void *sha256_one_shot_sym;
static void *sha512_final_sym;
static void *sha512_init_sym;
static void *sha512_update_sym;
static void *x509_store_ctx_get_error_sym;
static void *x509_store_ctx_get_ex_data_sym;
static void *x509_digest_sym;
static void *x509_get_ext_d2i_sym;
static void *x509_get_serialnumber_sym;
static void *x509_get_version_sym;
static void *x509_getm_notbefore_sym;
static void *openssl_add_all_algorithms_noconf_sym;
static void *pem_read_bio_x509_sym;

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

static void *crypto_sym(void **slot, const char *name);

static void validate_crypto_handle(void *handle)
{
	typedef unsigned long (*version_fn_t)(void);
	version_fn_t version_fn;
	const char *err;
	unsigned long version;

	dlerror();
	version_fn = (version_fn_t)dlsym(handle, "OpenSSL_version_num");
	err = dlerror();
	if (err != NULL || version_fn == NULL)
		compat_fatal("missing libcrypto.so.3 version probe", err);

	version = version_fn();
	if ((version >> 28) != 3)
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

static void preload_crypto_symbols(void)
{
	(void)crypto_handle();
	(void)crypto_sym(&bio_free_sym, "BIO_free");
	(void)crypto_sym(&bio_new_mem_buf_sym, "BIO_new_mem_buf");
	(void)crypto_sym(&bn_cmp_sym, "BN_cmp");
	(void)crypto_sym(&dsa_free_sym, "DSA_free");
	(void)crypto_sym(&ec_key_free_sym, "EC_KEY_free");
	(void)crypto_sym(&ec_key_new_by_curve_name_sym, "EC_KEY_new_by_curve_name");
	(void)crypto_sym(&ec_key_get0_group_sym, "EC_KEY_get0_group");
	(void)crypto_sym(&ec_key_get0_public_key_sym, "EC_KEY_get0_public_key");
	(void)crypto_sym(&ec_point_cmp_sym, "EC_POINT_cmp");
	(void)crypto_sym(&evp_cipher_ctx_free_sym, "EVP_CIPHER_CTX_free");
	(void)crypto_sym(&evp_cipher_ctx_new_sym, "EVP_CIPHER_CTX_new");
	(void)crypto_sym(&evp_decryptfinal_ex_sym, "EVP_DecryptFinal_ex");
	(void)crypto_sym(&evp_decryptinit_ex_sym, "EVP_DecryptInit_ex");
	(void)crypto_sym(&evp_decryptupdate_sym, "EVP_DecryptUpdate");
	(void)crypto_sym(&evp_encryptfinal_ex_sym, "EVP_EncryptFinal_ex");
	(void)crypto_sym(&evp_encryptinit_ex_sym, "EVP_EncryptInit_ex");
	(void)crypto_sym(&evp_encryptupdate_sym, "EVP_EncryptUpdate");
	(void)crypto_sym(&evp_pkey_free_sym, "EVP_PKEY_free");
	(void)crypto_sym(&evp_pkey_get1_dsa_sym, "EVP_PKEY_get1_DSA");
	(void)crypto_sym(&evp_pkey_get1_ec_key_sym, "EVP_PKEY_get1_EC_KEY");
	(void)crypto_sym(&evp_pkey_get1_rsa_sym, "EVP_PKEY_get1_RSA");
	(void)crypto_sym(&evp_aes_256_ecb_sym, "EVP_aes_256_ecb");
	(void)crypto_sym(&err_print_errors_fp_sym, "ERR_print_errors_fp");
	(void)crypto_sym(&md5_final_sym, "MD5_Final");
	(void)crypto_sym(&md5_init_sym, "MD5_Init");
	(void)crypto_sym(&md5_update_sym, "MD5_Update");
	(void)crypto_sym(&pem_read_privatekey_sym, "PEM_read_PrivateKey");
	(void)crypto_sym(&pem_read_rsaprivatekey_sym, "PEM_read_RSAPrivateKey");
	(void)crypto_sym(&pem_read_rsa_pubkey_sym, "PEM_read_RSA_PUBKEY");
	(void)crypto_sym(&pem_read_x509_sym, "PEM_read_X509");
	(void)crypto_sym(&pem_read_bio_rsaprivatekey_sym, "PEM_read_bio_RSAPrivateKey");
	(void)crypto_sym(&pem_read_bio_rsa_pubkey_sym, "PEM_read_bio_RSA_PUBKEY");
	(void)crypto_sym(&rand_bytes_sym, "RAND_bytes");
	(void)crypto_sym(&rsa_free_sym, "RSA_free");
	(void)crypto_sym(&rsa_new_sym, "RSA_new");
	(void)crypto_sym(&rsa_get0_n_sym, "RSA_get0_n");
	(void)crypto_sym(&rsa_private_decrypt_sym, "RSA_private_decrypt");
	(void)crypto_sym(&rsa_private_encrypt_sym, "RSA_private_encrypt");
	(void)crypto_sym(&rsa_public_decrypt_sym, "RSA_public_decrypt");
	(void)crypto_sym(&rsa_public_encrypt_sym, "RSA_public_encrypt");
	(void)crypto_sym(&rsa_size_sym, "RSA_size");
	(void)crypto_sym(&sha256_final_sym, "SHA256_Final");
	(void)crypto_sym(&sha256_init_sym, "SHA256_Init");
	(void)crypto_sym(&sha256_update_sym, "SHA256_Update");
	(void)crypto_sym(&x509_name_oneline_sym, "X509_NAME_oneline");
	(void)crypto_sym(&x509_free_sym, "X509_free");
	(void)crypto_sym(&x509_get_issuer_name_sym, "X509_get_issuer_name");
	(void)crypto_sym(&x509_get_pubkey_sym, "X509_get_pubkey");
	(void)crypto_sym(&x509_get_subject_name_sym, "X509_get_subject_name");
	(void)crypto_sym(&x509_getm_notafter_sym, "X509_getm_notAfter");
	(void)crypto_sym(&dsa_get0_pub_key_sym, "DSA_get0_pub_key");
	(void)crypto_sym(&d2i_privatekey_fp_sym, "d2i_PrivateKey_fp");
	(void)crypto_sym(&d2i_x509_fp_sym, "d2i_X509_fp");
	(void)crypto_sym(&evp_cipher_ctx_get_block_size_sym, "EVP_CIPHER_CTX_get_block_size");
	(void)crypto_sym(&evp_pkey_get_id_sym, "EVP_PKEY_get_id");
	(void)crypto_sym(&bio_ctrl_sym, "BIO_ctrl");
	(void)crypto_sym(&bio_f_base64_sym, "BIO_f_base64");
	(void)crypto_sym(&bio_free_all_sym, "BIO_free_all");
	(void)crypto_sym(&bio_new_sym, "BIO_new");
	(void)crypto_sym(&bio_new_fp_sym, "BIO_new_fp");
	(void)crypto_sym(&bio_new_file_sym, "BIO_new_file");
	(void)crypto_sym(&bio_push_sym, "BIO_push");
	(void)crypto_sym(&bio_read_sym, "BIO_read");
	(void)crypto_sym(&bio_s_file_sym, "BIO_s_file");
	(void)crypto_sym(&bio_s_mem_sym, "BIO_s_mem");
	(void)crypto_sym(&bio_set_flags_sym, "BIO_set_flags");
	(void)crypto_sym(&bio_write_sym, "BIO_write");
	(void)crypto_sym(&asn1_string_data_sym, "ASN1_STRING_data");
	(void)crypto_sym(&asn1_string_get0_data_sym, "ASN1_STRING_get0_data");
	(void)crypto_sym(&asn1_string_length_sym, "ASN1_STRING_length");
	(void)crypto_sym(&asn1_string_to_utf8_sym, "ASN1_STRING_to_UTF8");
	(void)crypto_sym(&crypto_free_sym, "CRYPTO_free");
	(void)crypto_sym(&crypto_get_ex_new_index_sym, "CRYPTO_get_ex_new_index");
	(void)crypto_sym(&crypto_malloc_sym, "CRYPTO_malloc");
	(void)crypto_sym(&hmac_sym, "HMAC");
	(void)crypto_sym(&hmac_ctx_new_sym, "HMAC_CTX_new");
	(void)crypto_sym(&hmac_ctx_free_sym, "HMAC_CTX_free");
	(void)crypto_sym(&hmac_init_ex_sym, "HMAC_Init_ex");
	(void)crypto_sym(&hmac_update_sym, "HMAC_Update");
	(void)crypto_sym(&hmac_final_sym, "HMAC_Final");
	(void)crypto_sym(&evp_cipher_ctx_set_padding_sym, "EVP_CIPHER_CTX_set_padding");
	(void)crypto_sym(&evp_cipher_ctx_reset_sym, "EVP_CIPHER_CTX_reset");
	(void)crypto_sym(&evp_md_ctx_reset_sym, "EVP_MD_CTX_reset");
	(void)crypto_sym(&evp_cipher_final_ex_sym, "EVP_CipherFinal_ex");
	(void)crypto_sym(&evp_cipher_init_ex_sym, "EVP_CipherInit_ex");
	(void)crypto_sym(&evp_cipher_update_sym, "EVP_CipherUpdate");
	(void)crypto_sym(&evp_digest_final_ex_sym, "EVP_DigestFinal_ex");
	(void)crypto_sym(&evp_digest_init_ex_sym, "EVP_DigestInit_ex");
	(void)crypto_sym(&evp_digest_update_sym, "EVP_DigestUpdate");
	(void)crypto_sym(&err_clear_error_sym, "ERR_clear_error");
	(void)crypto_sym(&err_error_string_n_sym, "ERR_error_string_n");
	(void)crypto_sym(&err_get_error_sym, "ERR_get_error");
	(void)crypto_sym(&err_load_bio_strings_sym, "ERR_load_BIO_strings");
	(void)crypto_sym(&err_reason_error_string_sym, "ERR_reason_error_string");
	(void)crypto_sym(&openssl_version_sym, "OpenSSL_version");
	(void)crypto_sym(&evp_aes_128_cbc_sym, "EVP_aes_128_cbc");
	(void)crypto_sym(&evp_aes_256_cbc_sym, "EVP_aes_256_cbc");
	(void)crypto_sym(&evp_md5_sym, "EVP_md5");
	(void)crypto_sym(&evp_sha1_sym, "EVP_sha1");
	(void)crypto_sym(&evp_sha256_sym, "EVP_sha256");
	(void)crypto_sym(&md5_one_shot_sym, "MD5");
	(void)crypto_sym(&openssl_cleanse_sym, "OPENSSL_cleanse");
	(void)crypto_sym(&openssl_die_sym, "OPENSSL_die");
	(void)crypto_sym(&openssl_init_crypto_sym, "OPENSSL_init_crypto");
	(void)crypto_sym(&openssl_sk_num_sym, "OPENSSL_sk_num");
	(void)crypto_sym(&openssl_sk_value_sym, "OPENSSL_sk_value");
	(void)crypto_sym(&rsa_verify_sym, "RSA_verify");
	(void)crypto_sym(&sha256_one_shot_sym, "SHA256");
	(void)crypto_sym(&sha512_final_sym, "SHA512_Final");
	(void)crypto_sym(&sha512_init_sym, "SHA512_Init");
	(void)crypto_sym(&sha512_update_sym, "SHA512_Update");
	(void)crypto_sym(&x509_store_ctx_get_error_sym, "X509_STORE_CTX_get_error");
	(void)crypto_sym(&x509_store_ctx_get_ex_data_sym, "X509_STORE_CTX_get_ex_data");
	(void)crypto_sym(&x509_digest_sym, "X509_digest");
	(void)crypto_sym(&x509_get_ext_d2i_sym, "X509_get_ext_d2i");
	(void)crypto_sym(&x509_get_serialnumber_sym, "X509_get_serialNumber");
	(void)crypto_sym(&x509_get_version_sym, "X509_get_version");
	(void)crypto_sym(&x509_getm_notbefore_sym, "X509_getm_notBefore");
	(void)crypto_sym(&openssl_add_all_algorithms_noconf_sym, "OPENSSL_init_crypto");
	(void)crypto_sym(&pem_read_bio_x509_sym, "PEM_read_bio_X509");
}

static void __attribute__((constructor)) preload_crypto_compat(void)
{
	preload_crypto_symbols();
}

#define COMPAT_EXPORT __attribute__((visibility("default")))

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
#define WRAP7(ret, name, slot, t1, a1, t2, a2, t3, a3, t4, a4, t5, a5, t6, a6, t7, a7) \
	COMPAT_EXPORT ret name(t1 a1, t2 a2, t3 a3, t4 a4, t5 a5, t6 a6, t7 a7) { typedef ret (*fn_t)(t1, t2, t3, t4, t5, t6, t7); return ((fn_t)crypto_sym(&(slot), #name))(a1, a2, a3, a4, a5, a6, a7); }
#define WRAPV1(name, slot, t1, a1) \
	COMPAT_EXPORT void name(t1 a1) { typedef void (*fn_t)(t1); ((fn_t)crypto_sym(&(slot), #name))(a1); }

/*
 * OpenSSL 1.0 exposed HMAC_CTX as a public, stack-allocatable structure;
 * OpenSSL 3 keeps it opaque.  Keep a private map from the legacy address to
 * a real OpenSSL 3 context instead of casting the incompatible layouts.
 */
struct compat_hmac_entry {
	HMAC_CTX *legacy;
	HMAC_CTX *real;
	struct compat_hmac_entry *next;
};

static struct compat_hmac_entry *compat_hmac_entries;
static volatile unsigned char compat_hmac_lock;

static void compat_hmac_lock_acquire(void)
{
	while (__atomic_test_and_set(&compat_hmac_lock, __ATOMIC_ACQUIRE))
		;
}

static void compat_hmac_lock_release(void)
{
	__atomic_clear(&compat_hmac_lock, __ATOMIC_RELEASE);
}

static HMAC_CTX *compat_hmac_lookup(HMAC_CTX *legacy)
{
	struct compat_hmac_entry *entry;

	compat_hmac_lock_acquire();
	for (entry = compat_hmac_entries; entry != NULL; entry = entry->next) {
		if (entry->legacy == legacy) {
			HMAC_CTX *real = entry->real;
			compat_hmac_lock_release();
			return real;
		}
	}
	compat_hmac_lock_release();
	return NULL;
}

COMPAT_EXPORT void HMAC_CTX_init(HMAC_CTX *legacy)
{
	struct compat_hmac_entry *entry;
	HMAC_CTX *real;

	if (legacy == NULL)
		return;
	real = ((HMAC_CTX *(*)(void))crypto_sym(&hmac_ctx_new_sym, "HMAC_CTX_new"))();
	if (real == NULL)
		compat_fatal("unable to allocate HMAC compatibility context", NULL);
	entry = calloc(1, sizeof(*entry));
	if (entry == NULL) {
		((void (*)(HMAC_CTX *))crypto_sym(&hmac_ctx_free_sym, "HMAC_CTX_free"))(real);
		compat_fatal("unable to allocate HMAC compatibility entry", NULL);
	}
	entry->legacy = legacy;
	entry->real = real;
	compat_hmac_lock_acquire();
	entry->next = compat_hmac_entries;
	compat_hmac_entries = entry;
	compat_hmac_lock_release();
}

COMPAT_EXPORT void HMAC_CTX_cleanup(HMAC_CTX *legacy)
{
	struct compat_hmac_entry **link;
	struct compat_hmac_entry *entry = NULL;

	if (legacy == NULL)
		return;
	compat_hmac_lock_acquire();
	for (link = &compat_hmac_entries; *link != NULL; link = &(*link)->next) {
		if ((*link)->legacy == legacy) {
			entry = *link;
			*link = entry->next;
			break;
		}
	}
	compat_hmac_lock_release();
	if (entry != NULL) {
		((void (*)(HMAC_CTX *))crypto_sym(&hmac_ctx_free_sym, "HMAC_CTX_free"))(entry->real);
		free(entry);
	}
}

COMPAT_EXPORT int HMAC_Init_ex(HMAC_CTX *legacy, const void *key, int len,
	const EVP_MD *md, ENGINE *impl)
{
	HMAC_CTX *real = compat_hmac_lookup(legacy);
	if (real == NULL)
		return 0;
	return ((int (*)(HMAC_CTX *, const void *, int, const EVP_MD *, ENGINE *))
		crypto_sym(&hmac_init_ex_sym, "HMAC_Init_ex"))(real, key, len, md, impl);
}

COMPAT_EXPORT int HMAC_Update(HMAC_CTX *legacy, const unsigned char *data, size_t len)
{
	HMAC_CTX *real = compat_hmac_lookup(legacy);
	if (real == NULL)
		return 0;
	return ((int (*)(HMAC_CTX *, const unsigned char *, size_t))
		crypto_sym(&hmac_update_sym, "HMAC_Update"))(real, data, len);
}

COMPAT_EXPORT int HMAC_Final(HMAC_CTX *legacy, unsigned char *md, unsigned int *len)
{
	HMAC_CTX *real = compat_hmac_lookup(legacy);
	if (real == NULL)
		return 0;
	return ((int (*)(HMAC_CTX *, unsigned char *, unsigned int *))
		crypto_sym(&hmac_final_sym, "HMAC_Final"))(real, md, len);
}

/*
 * The inspected vendor callers pass dup_func == NULL.  Keep the legacy
 * function signature at the shim boundary, but do not carry an unused
 * callback adapter into the production shim.  If a future proprietary
 * caller supplies a duplication callback, fail closed rather than passing
 * the OpenSSL 1.1 callback through the incompatible OpenSSL 3 ABI.
 */
typedef int (*compat_legacy_ex_dup_fn)(CRYPTO_EX_DATA *to,
	const CRYPTO_EX_DATA *from, void *from_d, int idx, long argl, void *argp);

COMPAT_EXPORT int CRYPTO_get_ex_new_index(int class_index, long argl, void *argp,
	CRYPTO_EX_new *new_func, compat_legacy_ex_dup_fn dup_func,
	CRYPTO_EX_free *free_func)
{
	typedef int (*backend_dup_fn)(CRYPTO_EX_DATA *, const CRYPTO_EX_DATA *,
		void **, int, long, void *);
	typedef int (*backend_fn)(int, long, void *, CRYPTO_EX_new *,
		backend_dup_fn, CRYPTO_EX_free *);

	if (dup_func != NULL)
		compat_fatal("unsupported CRYPTO_EX_dup callback", "legacy callback ABI");

	return ((backend_fn)crypto_sym(&crypto_get_ex_new_index_sym,
		"CRYPTO_get_ex_new_index"))(class_index, argl, argp, new_func,
		NULL, free_func);
}

WRAP1(int, BIO_free, bio_free_sym, BIO *, bio)
WRAP2(BIO *, BIO_new_mem_buf, bio_new_mem_buf_sym, const void *, buf, int, len)
WRAP2(int, BN_cmp, bn_cmp_sym, const BIGNUM *, a, const BIGNUM *, b)
WRAPV1(DSA_free, dsa_free_sym, DSA *, dsa)
WRAPV1(EC_KEY_free, ec_key_free_sym, EC_KEY *, key)
WRAP1(EC_KEY *, EC_KEY_new_by_curve_name, ec_key_new_by_curve_name_sym, int, nid)
WRAP1(const EC_GROUP *, EC_KEY_get0_group, ec_key_get0_group_sym, const EC_KEY *, key)
WRAP1(const EC_POINT *, EC_KEY_get0_public_key, ec_key_get0_public_key_sym, const EC_KEY *, key)
WRAP4(int, EC_POINT_cmp, ec_point_cmp_sym, const EC_GROUP *, group, const EC_POINT *, a, const EC_POINT *, b, BN_CTX *, ctx)
WRAPV1(EVP_CIPHER_CTX_free, evp_cipher_ctx_free_sym, EVP_CIPHER_CTX *, ctx)
WRAP0(EVP_CIPHER_CTX *, EVP_CIPHER_CTX_new, evp_cipher_ctx_new_sym)
WRAP3(int, EVP_DecryptFinal_ex, evp_decryptfinal_ex_sym, EVP_CIPHER_CTX *, ctx, unsigned char *, out, int *, outl)
WRAP5(int, EVP_DecryptInit_ex, evp_decryptinit_ex_sym, EVP_CIPHER_CTX *, ctx, const EVP_CIPHER *, type, ENGINE *, impl, const unsigned char *, key, const unsigned char *, iv)
WRAP5(int, EVP_DecryptUpdate, evp_decryptupdate_sym, EVP_CIPHER_CTX *, ctx, unsigned char *, out, int *, outl, const unsigned char *, in, int, inl)
WRAP3(int, EVP_EncryptFinal_ex, evp_encryptfinal_ex_sym, EVP_CIPHER_CTX *, ctx, unsigned char *, out, int *, outl)
WRAP5(int, EVP_EncryptInit_ex, evp_encryptinit_ex_sym, EVP_CIPHER_CTX *, ctx, const EVP_CIPHER *, type, ENGINE *, impl, const unsigned char *, key, const unsigned char *, iv)
WRAP5(int, EVP_EncryptUpdate, evp_encryptupdate_sym, EVP_CIPHER_CTX *, ctx, unsigned char *, out, int *, outl, const unsigned char *, in, int, inl)
WRAPV1(EVP_PKEY_free, evp_pkey_free_sym, EVP_PKEY *, pkey)
WRAP1(DSA *, EVP_PKEY_get1_DSA, evp_pkey_get1_dsa_sym, EVP_PKEY *, pkey)
WRAP1(EC_KEY *, EVP_PKEY_get1_EC_KEY, evp_pkey_get1_ec_key_sym, EVP_PKEY *, pkey)
WRAP1(RSA *, EVP_PKEY_get1_RSA, evp_pkey_get1_rsa_sym, EVP_PKEY *, pkey)
WRAP0(const EVP_CIPHER *, EVP_aes_256_ecb, evp_aes_256_ecb_sym)
WRAPV1(ERR_print_errors_fp, err_print_errors_fp_sym, FILE *, fp)
WRAP2(int, MD5_Final, md5_final_sym, unsigned char *, md, MD5_CTX *, c)
WRAP1(int, MD5_Init, md5_init_sym, MD5_CTX *, c)
WRAP3(int, MD5_Update, md5_update_sym, MD5_CTX *, c, const void *, data, size_t, len)
WRAP4(EVP_PKEY *, PEM_read_PrivateKey, pem_read_privatekey_sym, FILE *, fp, EVP_PKEY **, x, pem_password_cb *, cb, void *, u)
WRAP4(RSA *, PEM_read_RSAPrivateKey, pem_read_rsaprivatekey_sym, FILE *, fp, RSA **, x, pem_password_cb *, cb, void *, u)
WRAP4(RSA *, PEM_read_RSA_PUBKEY, pem_read_rsa_pubkey_sym, FILE *, fp, RSA **, x, pem_password_cb *, cb, void *, u)
WRAP4(X509 *, PEM_read_X509, pem_read_x509_sym, FILE *, fp, X509 **, x, pem_password_cb *, cb, void *, u)
WRAP4(RSA *, PEM_read_bio_RSAPrivateKey, pem_read_bio_rsaprivatekey_sym, BIO *, bp, RSA **, x, pem_password_cb *, cb, void *, u)
WRAP4(RSA *, PEM_read_bio_RSA_PUBKEY, pem_read_bio_rsa_pubkey_sym, BIO *, bp, RSA **, x, pem_password_cb *, cb, void *, u)
WRAP2(int, RAND_bytes, rand_bytes_sym, unsigned char *, buf, int, num)
WRAPV1(RSA_free, rsa_free_sym, RSA *, rsa)
WRAP0(RSA *, RSA_new, rsa_new_sym)
WRAP5(int, RSA_private_decrypt, rsa_private_decrypt_sym, int, flen, const unsigned char *, from, unsigned char *, to, RSA *, rsa, int, padding)
WRAP5(int, RSA_private_encrypt, rsa_private_encrypt_sym, int, flen, const unsigned char *, from, unsigned char *, to, RSA *, rsa, int, padding)
WRAP5(int, RSA_public_decrypt, rsa_public_decrypt_sym, int, flen, const unsigned char *, from, unsigned char *, to, RSA *, rsa, int, padding)
WRAP5(int, RSA_public_encrypt, rsa_public_encrypt_sym, int, flen, const unsigned char *, from, unsigned char *, to, RSA *, rsa, int, padding)
WRAP1(int, RSA_size, rsa_size_sym, const RSA *, rsa)
WRAP2(int, SHA256_Final, sha256_final_sym, unsigned char *, md, SHA256_CTX *, c)
WRAP1(int, SHA256_Init, sha256_init_sym, SHA256_CTX *, c)
WRAP3(int, SHA256_Update, sha256_update_sym, SHA256_CTX *, c, const void *, data, size_t, len)
WRAP3(char *, X509_NAME_oneline, x509_name_oneline_sym, const X509_NAME *, name, char *, buf, int, size)
WRAPV1(X509_free, x509_free_sym, X509 *, x509)
WRAP1(X509_NAME *, X509_get_issuer_name, x509_get_issuer_name_sym, const X509 *, x509)
WRAP1(EVP_PKEY *, X509_get_pubkey, x509_get_pubkey_sym, X509 *, x509)
WRAP1(X509_NAME *, X509_get_subject_name, x509_get_subject_name_sym, const X509 *, x509)
WRAP1(ASN1_TIME *, X509_getm_notAfter, x509_getm_notafter_sym, const X509 *, x509)
WRAP2(EVP_PKEY *, d2i_PrivateKey_fp, d2i_privatekey_fp_sym, FILE *, fp, EVP_PKEY **, a)
WRAP2(X509 *, d2i_X509_fp, d2i_x509_fp_sym, FILE *, fp, X509 **, a)

static const BIGNUM *compat_RSA_get0_n_impl(const RSA *rsa)
{
	typedef const BIGNUM *(*fn_t)(const RSA *);

	return ((fn_t)crypto_sym(&rsa_get0_n_sym, "RSA_get0_n"))(rsa);
}

static const BIGNUM *compat_DSA_get0_pub_key_impl(const DSA *dsa)
{
	typedef const BIGNUM *(*fn_t)(const DSA *);

	return ((fn_t)crypto_sym(&dsa_get0_pub_key_sym, "DSA_get0_pub_key"))(dsa);
}

COMPAT_EXPORT const BIGNUM *RSA_get0_n_v110(const RSA *rsa)
{
	return compat_RSA_get0_n_impl(rsa);
}

COMPAT_EXPORT const BIGNUM *RSA_get0_n_v111(const RSA *rsa)
{
	return compat_RSA_get0_n_impl(rsa);
}

COMPAT_EXPORT const BIGNUM *DSA_get0_pub_key_v110(const DSA *dsa)
{
	return compat_DSA_get0_pub_key_impl(dsa);
}

COMPAT_EXPORT const BIGNUM *DSA_get0_pub_key_v111(const DSA *dsa)
{
	return compat_DSA_get0_pub_key_impl(dsa);
}

__asm__(".symver RSA_get0_n_v110,RSA_get0_n@OPENSSL_1_1_0");
__asm__(".symver RSA_get0_n_v111,RSA_get0_n@@OPENSSL_1_1_1");
__asm__(".symver DSA_get0_pub_key_v110,DSA_get0_pub_key@OPENSSL_1_1_0");
__asm__(".symver DSA_get0_pub_key_v111,DSA_get0_pub_key@@OPENSSL_1_1_1");

COMPAT_EXPORT unsigned long OpenSSL_version_num(void)
{
	return COMPAT_OPENSSL_VERSION_NUM;
}

COMPAT_EXPORT int EVP_CIPHER_CTX_block_size(const EVP_CIPHER_CTX *ctx)
{
	typedef int (*fn_t)(const EVP_CIPHER_CTX *);

	return ((fn_t)crypto_sym(&evp_cipher_ctx_get_block_size_sym,
				 "EVP_CIPHER_CTX_get_block_size"))(ctx);
}

COMPAT_EXPORT int EVP_PKEY_id(const EVP_PKEY *pkey)
{
	typedef int (*fn_t)(const EVP_PKEY *);

	return ((fn_t)crypto_sym(&evp_pkey_get_id_sym, "EVP_PKEY_get_id"))(pkey);
}

WRAP4(long, BIO_ctrl, bio_ctrl_sym, BIO *, b, int, cmd, long, larg, void *, parg)
WRAP0(const BIO_METHOD *, BIO_f_base64, bio_f_base64_sym)
WRAPV1(BIO_free_all, bio_free_all_sym, BIO *, bio)
WRAP1(BIO *, BIO_new, bio_new_sym, const BIO_METHOD *, type)
WRAP2(BIO *, BIO_new_fp, bio_new_fp_sym, FILE *, stream, int, close_flag)
WRAP2(BIO *, BIO_new_file, bio_new_file_sym, const char *, filename, const char *, mode)
WRAP2(BIO *, BIO_push, bio_push_sym, BIO *, b, BIO *, append)
WRAP3(int, BIO_read, bio_read_sym, BIO *, b, void *, data, int, dlen)
WRAP0(const BIO_METHOD *, BIO_s_file, bio_s_file_sym)
WRAP0(const BIO_METHOD *, BIO_s_mem, bio_s_mem_sym)
WRAP2(void, BIO_set_flags, bio_set_flags_sym, BIO *, b, int, flags)
WRAP3(int, BIO_write, bio_write_sym, BIO *, b, const void *, data, int, dlen)
WRAP1(unsigned char *, ASN1_STRING_data, asn1_string_data_sym, ASN1_STRING *, x)
WRAP1(const unsigned char *, ASN1_STRING_get0_data, asn1_string_get0_data_sym, const ASN1_STRING *, x)
WRAP1(int, ASN1_STRING_length, asn1_string_length_sym, const ASN1_STRING *, x)
WRAP2(int, ASN1_STRING_to_UTF8, asn1_string_to_utf8_sym, unsigned char **, out, const ASN1_STRING *, in)
WRAP3(void, CRYPTO_free, crypto_free_sym, void *, ptr, const char *, file, int, line)
WRAP3(void *, CRYPTO_malloc, crypto_malloc_sym, size_t, num, const char *, file, int, line)
WRAP0(void, ERR_clear_error, err_clear_error_sym)
WRAP3(void, ERR_error_string_n, err_error_string_n_sym, unsigned long, e, char *, buf, size_t, len)
WRAP0(unsigned long, ERR_get_error, err_get_error_sym)
WRAP0(int, ERR_load_BIO_strings, err_load_bio_strings_sym)
WRAP1(const char *, ERR_reason_error_string, err_reason_error_string_sym, unsigned long, e)
WRAP2(int, EVP_CIPHER_CTX_set_padding, evp_cipher_ctx_set_padding_sym, EVP_CIPHER_CTX *, c, int, pad)
WRAP3(int, EVP_CipherFinal_ex, evp_cipher_final_ex_sym, EVP_CIPHER_CTX *, ctx, unsigned char *, out, int *, outl)
WRAP6(int, EVP_CipherInit_ex, evp_cipher_init_ex_sym, EVP_CIPHER_CTX *, ctx, const EVP_CIPHER *, type, ENGINE *, impl, const unsigned char *, key, const unsigned char *, iv, int, enc)
WRAP5(int, EVP_CipherUpdate, evp_cipher_update_sym, EVP_CIPHER_CTX *, ctx, unsigned char *, out, int *, outl, const unsigned char *, in, int, inl)
WRAP3(int, EVP_DigestFinal_ex, evp_digest_final_ex_sym, EVP_MD_CTX *, ctx, unsigned char *, md, unsigned int *, s)
WRAP3(int, EVP_DigestInit_ex, evp_digest_init_ex_sym, EVP_MD_CTX *, ctx, const EVP_MD *, type, ENGINE *, impl)
WRAP3(int, EVP_DigestUpdate, evp_digest_update_sym, EVP_MD_CTX *, ctx, const void *, d, size_t, cnt)
WRAP0(const EVP_CIPHER *, EVP_aes_128_cbc, evp_aes_128_cbc_sym)
WRAP0(const EVP_CIPHER *, EVP_aes_256_cbc, evp_aes_256_cbc_sym)
WRAP0(const EVP_MD *, EVP_md5, evp_md5_sym)
WRAP0(const EVP_MD *, EVP_sha1, evp_sha1_sym)
WRAP0(const EVP_MD *, EVP_sha256, evp_sha256_sym)
WRAP1(const char *, OpenSSL_version, openssl_version_sym, int, type)
WRAP7(unsigned char *, HMAC, hmac_sym, const EVP_MD *, evp_md, const void *, key, int, key_len, const unsigned char *, data, size_t, data_len, unsigned char *, md, unsigned int *, md_len)
WRAP3(unsigned char *, MD5, md5_one_shot_sym, const unsigned char *, data, size_t, len, unsigned char *, md)
WRAP2(void, OPENSSL_cleanse, openssl_cleanse_sym, void *, ptr, size_t, len)
WRAP2(int, OPENSSL_init_crypto, openssl_init_crypto_sym, uint64_t, opts, const OPENSSL_INIT_SETTINGS *, settings)
WRAP1(int, OPENSSL_sk_num, openssl_sk_num_sym, const OPENSSL_STACK *, st)
WRAP2(void *, OPENSSL_sk_value, openssl_sk_value_sym, const OPENSSL_STACK *, st, int, i)
WRAP6(int, RSA_verify, rsa_verify_sym, int, type, const unsigned char *, m, unsigned int, m_length, const unsigned char *, sigbuf, unsigned int, siglen, RSA *, rsa)
WRAP3(unsigned char *, SHA256, sha256_one_shot_sym, const unsigned char *, data, size_t, len, unsigned char *, md)
WRAP2(int, SHA512_Final, sha512_final_sym, unsigned char *, md, SHA512_CTX *, c)
WRAP1(int, SHA512_Init, sha512_init_sym, SHA512_CTX *, c)
WRAP3(int, SHA512_Update, sha512_update_sym, SHA512_CTX *, c, const void *, data, size_t, len)
WRAP1(int, X509_STORE_CTX_get_error, x509_store_ctx_get_error_sym, const X509_STORE_CTX *, ctx)
WRAP2(void *, X509_STORE_CTX_get_ex_data, x509_store_ctx_get_ex_data_sym, const X509_STORE_CTX *, ctx, int, idx)
WRAP4(int, X509_digest, x509_digest_sym, const X509 *, data, const EVP_MD *, type, unsigned char *, md, unsigned int *, len)
WRAP4(void *, X509_get_ext_d2i, x509_get_ext_d2i_sym, const X509 *, x, int, nid, int *, crit, int *, idx)
WRAP1(ASN1_INTEGER *, X509_get_serialNumber, x509_get_serialnumber_sym, X509 *, x)
WRAP1(long, X509_get_version, x509_get_version_sym, const X509 *, x)
WRAP1(ASN1_TIME *, X509_getm_notBefore, x509_getm_notbefore_sym, const X509 *, x)
WRAPV1(EVP_CIPHER_CTX_init, evp_cipher_ctx_reset_sym, EVP_CIPHER_CTX *, ctx)
WRAP1(int, EVP_CIPHER_CTX_cleanup, evp_cipher_ctx_reset_sym, EVP_CIPHER_CTX *, ctx)
WRAPV1(EVP_MD_CTX_init, evp_md_ctx_reset_sym, EVP_MD_CTX *, ctx)
WRAP1(int, EVP_MD_CTX_cleanup, evp_md_ctx_reset_sym, EVP_MD_CTX *, ctx)
WRAP4(X509 *, PEM_read_bio_X509, pem_read_bio_x509_sym, BIO *, bp, X509 **, x, pem_password_cb *, cb, void *, u)

COMPAT_EXPORT void OPENSSL_add_all_algorithms_noconf(void)
{
	typedef int (*fn_t)(uint64_t, const OPENSSL_INIT_SETTINGS *);
	(void)((fn_t)crypto_sym(&openssl_add_all_algorithms_noconf_sym,
				"OPENSSL_init_crypto"))(OPENSSL_INIT_ADD_ALL_CIPHERS |
							OPENSSL_INIT_ADD_ALL_DIGESTS, NULL);
}

COMPAT_EXPORT void OPENSSL_die(const char *assertion, const char *file, int line)
{
	typedef void (*fn_t)(const char *, const char *, int);
	((fn_t)crypto_sym(&openssl_die_sym, "OPENSSL_die"))(assertion, file, line);
	__builtin_unreachable();
}

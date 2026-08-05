#define _GNU_SOURCE
#include <dlfcn.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

#include <openssl/bio.h>
#include <openssl/bn.h>
#include <openssl/crypto.h>
#include <openssl/dsa.h>
#include <openssl/ec.h>
#include <openssl/err.h>
#include <openssl/engine.h>
#include <openssl/evp.h>
#include <openssl/md5.h>
#include <openssl/pem.h>
#include <openssl/rand.h>
#include <openssl/rsa.h>
#include <openssl/sha.h>
#include <openssl/x509.h>

#ifdef EVP_CIPHER_CTX_block_size
#undef EVP_CIPHER_CTX_block_size
#endif
#ifdef EVP_PKEY_id
#undef EVP_PKEY_id
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
#define WRAPV1(name, slot, t1, a1) \
	COMPAT_EXPORT void name(t1 a1) { typedef void (*fn_t)(t1); ((fn_t)crypto_sym(&(slot), #name))(a1); }

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

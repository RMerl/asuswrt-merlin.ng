#ifndef NETTLE_TESTUTILS_H_INCLUDED
#define NETTLE_TESTUTILS_H_INCLUDED

/* config.h should usually be first in each .c file. This is an
   exception, include it here to reduce clutter in the test cases. */
#if HAVE_CONFIG_H
# include "config.h"
#endif

#include <stdarg.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include "nettle-types.h"
#include "version.h"

#if WITH_HOGWEED
# include "rsa.h"
# include "dsa-compat.h"
# include "ecc-curve.h"
# include "ecc.h"
# include "ecc-internal.h"
# include "ecdsa.h"
# include "gmp-glue.h"
# if NETTLE_USE_MINI_GMP
#  include "knuth-lfib.h"
# endif

/* Undo dsa-compat name mangling */
#undef dsa_generate_keypair
#define dsa_generate_keypair nettle_dsa_generate_keypair
#endif /* WITH_HOGWEED */

#include "nettle-meta.h"

/* Forward declare */
struct nettle_aead;

#ifdef __cplusplus
extern "C" {
#endif

void
die(const char *format, ...) PRINTF_STYLE (1, 2) NORETURN;

void *
xalloc(size_t size);

struct tstring {
  struct tstring *next;
  size_t length;
  uint8_t data[1];
};

struct tstring *
tstring_alloc (size_t length);

void
tstring_clear(void);

struct tstring *
tstring_data(size_t length, const uint8_t *data);

struct tstring *
tstring_hex(const char *hex);

void
tstring_print_hex(const struct tstring *s);

/* Decodes a NUL-terminated hex string. */

void
print_hex(size_t length, const uint8_t *data);

/* The main program */
void
test_main(void);

extern int verbose;

/* Test functions deallocate their inputs when finished.*/
void
test_cipher(const struct nettle_cipher *cipher,
	    const struct tstring *key,
	    const struct tstring *cleartext,
	    const struct tstring *ciphertext);

void
test_cipher_cbc(const struct nettle_cipher *cipher,
		const struct tstring *key,
		const struct tstring *cleartext,
		const struct tstring *ciphertext,
		const struct tstring *iv);

void
test_cipher_cfb(const struct nettle_cipher *cipher,
		const struct tstring *key,
		const struct tstring *cleartext,
		const struct tstring *ciphertext,
		const struct tstring *iv);

void
test_cipher_cfb8(const struct nettle_cipher *cipher,
		 const struct tstring *key,
		 const struct tstring *cleartext,
		 const struct tstring *ciphertext,
		 const struct tstring *iv);

void
test_cipher_ctr(const struct nettle_cipher *cipher,
		const struct tstring *key,
		const struct tstring *cleartext,
		const struct tstring *ciphertext,
		const struct tstring *iv);

void
test_cipher_stream(const struct nettle_cipher *cipher,
		   const struct tstring *key,
		   const struct tstring *cleartext,
		   const struct tstring *ciphertext);

void
test_aead(const struct nettle_aead *aead,
	  nettle_hash_update_func *set_nonce,
	  const struct tstring *key,
	  const struct tstring *authtext,
	  const struct tstring *cleartext,
	  const struct tstring *ciphertext,
	  const struct tstring *nonce,
	  const struct tstring *digest);

void
test_hash(const struct nettle_hash *hash,
	  const struct tstring *msg,
	  const struct tstring *digest);

void
test_hash_large(const struct nettle_hash *hash,
		size_t count, size_t length,
		uint8_t c,
		const struct tstring *digest);

void
test_mac(const struct nettle_mac *mac,
	 const struct tstring *key,
	 const struct tstring *msg,
	 const struct tstring *digest);

void
test_armor(const struct nettle_armor *armor,
           size_t data_length,
           const uint8_t *data,
           const char *ascii);

#if WITH_HOGWEED

#if NETTLE_USE_MINI_GMP
typedef struct knuth_lfib_ctx gmp_randstate_t[1];

void gmp_randinit_default (struct knuth_lfib_ctx *ctx);
#define gmp_randclear(state)
void mpz_urandomb (mpz_t r, struct knuth_lfib_ctx *ctx, mp_bitcnt_t bits);
/* This is cheating */
#define mpz_rrandomb mpz_urandomb

#endif /* NETTLE_USE_MINI_GMP */

void
mpn_out_str (FILE *f, int base, const mp_limb_t *xp, mp_size_t xn);

mp_limb_t *
xalloc_limbs (mp_size_t n);

void
write_mpn (FILE *f, int base, const mp_limb_t *xp, mp_size_t n);

void
test_rsa_set_key_1(struct rsa_public_key *pub,
		   struct rsa_private_key *key);

void
test_rsa_md5(struct rsa_public_key *pub,
	     struct rsa_private_key *key,
	     mpz_t expected);

void
test_rsa_sha1(struct rsa_public_key *pub,
	      struct rsa_private_key *key,
	      mpz_t expected);

void
test_rsa_sha256(struct rsa_public_key *pub,
		struct rsa_private_key *key,
		mpz_t expected);

void
test_rsa_sha512(struct rsa_public_key *pub,
		struct rsa_private_key *key,
		mpz_t expected);

void
test_rsa_key(struct rsa_public_key *pub,
	     struct rsa_private_key *key);

void
test_dsa160(const struct dsa_public_key *pub,
	    const struct dsa_private_key *key,
	    const struct dsa_signature *expected);

void
test_dsa256(const struct dsa_public_key *pub,
	    const struct dsa_private_key *key,
	    const struct dsa_signature *expected);

#if 0
void
test_dsa_sign(const struct dsa_public_key *pub,
	      const struct dsa_private_key *key,
	      const struct nettle_hash *hash,
	      const struct dsa_signature *expected);
#endif

void
test_dsa_verify(const struct dsa_params *params,
		const mpz_t pub,
		const struct nettle_hash *hash,
		struct tstring *msg,
		const struct dsa_signature *ref);

void
test_dsa_key(const struct dsa_params *params,
	     const mpz_t pub,
	     const mpz_t key,
	     unsigned q_size);

extern const struct ecc_curve * const ecc_curves[];

struct ecc_ref_point
{
  const char *x;
  const char *y;
};

void
test_ecc_point (const struct ecc_curve *ecc,
		const struct ecc_ref_point *ref,
		const mp_limb_t *p);

void
test_ecc_mul_a (unsigned curve, unsigned n, const mp_limb_t *p);

void
test_ecc_mul_h (unsigned curve, unsigned n, const mp_limb_t *p);

/* Checks that p == g (affine coordinates) */
void
test_ecc_ga (unsigned curve, const mp_limb_t *p);

/* Gets the curve generator, with coordinates in redc form, if
   appropriate, and with an appended z = 1 coordinate. */
void
test_ecc_get_g (unsigned curve, mp_limb_t *rp);

/* Variant with only two coordinates, and no redc. */
void
test_ecc_get_ga (unsigned curve, mp_limb_t *rp);

#endif /* WITH_HOGWEED */

/* String literal of type unsigned char. The GNUC version is safer. */
#if __GNUC__
#define US(s) ({ static const unsigned char us_s[] = s; us_s; })
#else
#define US(s) ((const uint8_t *) (s))
#endif
  
/* LDATA needs to handle NUL characters. */
#define LLENGTH(x) (sizeof(x) - 1)
#define LDATA(x) LLENGTH(x), US(x)
#define LDUP(x) strlen(x), strdup(x)

#define SHEX(x) (tstring_hex(x))
#define SDATA(x) ((const struct tstring *)tstring_data(LLENGTH(x), US(x)))
#define H(x) (SHEX(x)->data)

#define MEMEQ(length, a, b) (!memcmp((a), (b), (length)))

#define FAIL() abort()
#define SKIP() exit(77)

#define ASSERT(x) do {							\
    if (!(x))								\
      {									\
	fprintf(stderr, "Assert failed: %s:%d: %s\n", \
		__FILE__, __LINE__, #x);					\
	FAIL();								\
      }									\
  } while(0)

#ifdef __cplusplus
}
#endif

#endif /* NETTLE_TESTUTILS_H_INCLUDED */

/* crypto-openssl.c is Copyright (c) 2019 Vladislav Grishenko <themiron@mail.ru>
                   and Copyright (c) 2012-2018 Simon Kelley

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; version 2 dated June, 1991, or
   (at your option) version 3 dated 29 June, 2007.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

#include "dnsmasq.h"

#if defined(HAVE_DNSSEC) && defined(HAVE_OPENSSL)

#include <openssl/opensslv.h>
#include <openssl/crypto.h>
#include <openssl/evp.h>
#include <openssl/rsa.h>
#include <openssl/ecdsa.h>
#include <openssl/x509.h>
#include <openssl/err.h>

#if !defined(OPENSSL_NO_DSA) && defined(HAVE_DSA)
#include <openssl/dsa.h>
#else
#undef HAVE_DSA
#endif

#if !defined(OPENSSL_NO_ENGINE) && !defined(NO_GOST)
#include <openssl/engine.h>
#define HAVE_GOST
static ENGINE *gost_engine = NULL;
static char *gost_hash = NULL;
#endif

#if OPENSSL_VERSION_NUMBER >= 0x10101000L
#define HAVE_EDDSA
#endif

#if OPENSSL_VERSION_NUMBER < 0x10100000L
static void BN_set(BIGNUM **bp, BIGNUM *b)
{
  if (b != NULL) {
    BN_free(*bp);
    *bp = b;
  }
}

static int RSA_set0_key(RSA *r, BIGNUM *n, BIGNUM *e, BIGNUM *d)
{
  BN_set(&r->n, n);
  BN_set(&r->e, e);
  BN_set(&r->d, d);
  return 1;
}

#ifdef HAVE_DSA
static int DSA_set0_pqg(DSA *d, BIGNUM *p, BIGNUM *q, BIGNUM *g)
{
  BN_set(&d->p, p);
  BN_set(&d->q, q);
  BN_set(&d->g, g);
  return 1;
}

static int DSA_set0_key(DSA *d, BIGNUM *pub_key, BIGNUM *priv_key)
{
  BN_set(&d->pub_key, pub_key);
  BN_set(&d->priv_key, priv_key);
  return 1;
}

static int DSA_SIG_set0(DSA_SIG *sig, BIGNUM *r, BIGNUM *s)
{
  BN_set(&sig->r, r);
  BN_set(&sig->s, s);
  return 1;
}
#endif

static int ECDSA_SIG_set0(ECDSA_SIG *sig, BIGNUM *r, BIGNUM *s)
{
  BN_set(&sig->r, r);
  BN_set(&sig->s, s);
  return 1;
}
#endif

#ifdef HAVE_EDDSA
/* Implement a "hash-function" to the nettle API, which simply returns
   the input data, concatenated into a single, statically maintained, buffer.

   Used for the EdDSA sigs, which operate on the whole message, rather
   than a digest. */

struct null_hash_digest
{
  uint8_t *buff;
  size_t len;
};

struct null_hash_ctx
{
  size_t len;
};

static size_t null_hash_buff_sz = 0;
static uint8_t *null_hash_buff = NULL;
#define BUFF_INCR 128

static int null_hash_init(void **ctxp, unsigned char **digestp)
{
  static struct null_hash_ctx ctx;
  static struct null_hash_digest digest;

  ctx.len = 0;

  *ctxp = &ctx;
  *digestp = (void *)&digest;

  return 1;
}

static void null_hash_update(void *ctxv, size_t length, const unsigned char *src)
{
  struct null_hash_ctx *ctx = ctxv;
  size_t new_len = ctx->len + length;

  if (new_len > null_hash_buff_sz)
    {
      uint8_t *new;

      if (!(new = whine_malloc(new_len + BUFF_INCR)))
	return;

      if (null_hash_buff)
	{
	  if (ctx->len != 0)
	    memcpy(new, null_hash_buff, ctx->len);
	  free(null_hash_buff);
	}

      null_hash_buff_sz = new_len + BUFF_INCR;
      null_hash_buff = new;
    }

  memcpy(null_hash_buff + ctx->len, src, length);
  ctx->len += length;
}

static void null_hash_digest(void *ctx, size_t length, unsigned char *dst)
{
  (void)length;

  ((struct null_hash_digest *)dst)->buff = null_hash_buff;
  ((struct null_hash_digest *)dst)->len = ((struct null_hash_ctx *)ctx)->len;
}

static size_t null_hash_length()
{
  return sizeof(struct null_hash_digest);
}

static struct {
  char *name;
} null_hash = {
  "null_hash"
};
#endif

/* Find pointer to correct hash function in openssl library */
const void *hash_find(char *name)
{
  if (!name)
    return NULL;

#ifdef HAVE_EDDSA
  /* We provide a "null" hash which returns the input data as digest. */
  if (strcmp(null_hash.name, name) == 0)
    return &null_hash;
#endif

  return EVP_get_digestbyname(name);
}

int hash_init(const void *hash, void **ctxp, unsigned char **digestp)
{
  const EVP_MD *md = (const EVP_MD *)hash;
  static EVP_MD_CTX *mdctx = NULL;
  static unsigned char *digest = NULL;
  static int digest_sz = 0;

  void *new;

#ifdef HAVE_EDDSA
  if (hash == &null_hash)
      return null_hash_init(ctxp, digestp);
#endif

  if (mdctx)
    EVP_MD_CTX_init(mdctx);
  else if (!(mdctx = EVP_MD_CTX_create()))
    return 0;

  if (EVP_DigestInit_ex(mdctx, md, NULL) == 0)
    return 0;

  if (digest_sz < EVP_MD_size(md))
    {
      if (!(new = whine_malloc(EVP_MD_size(md))))
	return 0;
      if (digest)
	free(digest);
      digest = new;
      digest_sz = EVP_MD_size(md);
    }

  *ctxp = mdctx;
  *digestp = digest;

  return 1;
}

void hash_update(const void *hash, void *ctx, size_t length, const unsigned char *src)
{
  EVP_MD_CTX *mdctx = (EVP_MD_CTX *)ctx;

#ifdef HAVE_EDDSA
  if (hash == &null_hash)
    return null_hash_update(ctx, length, src);
#else
  (void)hash;
#endif

  EVP_DigestUpdate(mdctx, src, length);
}

void hash_digest(const void *hash, void *ctx, size_t length, unsigned char *dst)
{
  EVP_MD_CTX *mdctx = (EVP_MD_CTX *)ctx;

  (void)length;

#ifdef HAVE_EDDSA
  if (hash == &null_hash)
    return null_hash_digest(ctx, length, dst);
#else
  (void)hash;
#endif

  EVP_DigestFinal_ex(mdctx, dst, NULL);
}

size_t hash_length(const void *hash)
{
  const EVP_MD *md = (const EVP_MD *)hash;

#ifdef HAVE_EDDSA
  if (hash == &null_hash)
    return null_hash_length();
#endif

  return EVP_MD_size(md);
}

static int dnsmasq_rsa_verify(struct blockdata *key_data, unsigned int key_len, unsigned char *sig, size_t sig_len,
			      unsigned char *digest, size_t digest_len, int algo)
{
  static RSA *rsa = NULL;
  BIGNUM *E, *N;

  unsigned char *p;
  size_t exp_len;
  int r;

  switch (algo)
    {
    case 1: r = NID_md5; break;
    case 5: r = NID_sha1; break;
    case 7: r = NID_sha1; break;
    case 8: r = NID_sha256; break;
    case 10: r = NID_sha512; break;
    default:
      return 0;
    }

  if (!rsa && !(rsa = RSA_new()))
    return 0;

  if ((key_len < 3) || !(p = blockdata_retrieve(key_data, key_len, NULL)))
    return 0;

  key_len--;
  if ((exp_len = *p++) == 0)
    {
      GETSHORT(exp_len, p);
      key_len -= 2;
    }

  if (exp_len >= key_len)
    return 0;

  E = BN_bin2bn(p, exp_len, NULL);
  N = BN_bin2bn(p + exp_len, key_len - exp_len, NULL);
  if (!E || !N)
    goto err;
  if (!RSA_set0_key(rsa, N, E, NULL))
    goto err;

  r = RSA_verify(r, digest, digest_len, sig, sig_len, rsa);

  RSA_free(rsa), rsa = NULL;

  return (r > 0);

err:
  BN_free(E);
  BN_free(N);

  return 0;
}

#ifdef HAVE_DSA
static int dnsmasq_dsa_verify(struct blockdata *key_data, unsigned int key_len, unsigned char *sig, size_t sig_len,
			      unsigned char *digest, size_t digest_len, int algo)
{
  static DSA *dsa = NULL;
  static DSA_SIG *dsasig = NULL;
  BIGNUM *Q, *P, *G, *Y, *R, *S;

  unsigned char *p;
  unsigned int t, l;
  int r;

  (void)algo;

  if (!dsa && !(dsa = DSA_new()))
    return 0;

  if (!dsasig && !(dsasig = DSA_SIG_new()))
    return 0;

  if ((sig_len < 41) || !(p = blockdata_retrieve(key_data, key_len, NULL)))
    return 0;

  t = *p++;
  l = 64 + (t*8);

  if (key_len < (213 + (t * 24)))
    return 0;

  Q = BN_bin2bn(p, 20, NULL); p += 20;
  P = BN_bin2bn(p, l, NULL); p += l;
  G = BN_bin2bn(p, l, NULL); p += l;
  Y = BN_bin2bn(p, l, NULL);
  R = BN_bin2bn(sig + 1, 20, NULL);
  S = BN_bin2bn(sig + 21, 20, NULL);
  if (!Q || !P || !G || !Y || !R || !Y || !DSA_set0_pqg(dsa, P, Q, G))
    goto err;
  if (!DSA_set0_key(dsa, Y, NULL))
    goto err_y;
  if (!DSA_SIG_set0(dsasig, R, S))
    goto err_rs;

  r = DSA_do_verify(digest, digest_len, dsasig, dsa);

  DSA_free(dsa), dsa = NULL;

  return (r > 0);

err:
  BN_free(Q);
  BN_free(P);
  BN_free(G);
err_y:
  BN_free(Y);
err_rs:
  BN_free(R);
  BN_free(S);

  return 0;
}
#endif

static int dnsmasq_ecdsa_verify(struct blockdata *key_data, unsigned int key_len,
				unsigned char *sig, size_t sig_len,
				unsigned char *digest, size_t digest_len, int algo)
{
  static EC_KEY *eckey = NULL;
  static ECDSA_SIG *ecsig = NULL;
  BIGNUM *R, *S;

  unsigned char keybuf[256 + 2];
  const unsigned char *p = keybuf;
  unsigned int t;
  int r;

  switch (algo)
    {
    case 13:
      if (!eckey && !(eckey = EC_KEY_new_by_curve_name(NID_X9_62_prime256v1)))
        return 0;
      t = 32;
      break;

    case 14:
      if (!eckey && !(eckey = EC_KEY_new_by_curve_name(NID_secp384r1)))
        return 0;
      t = 48;
      break;

    default:
      return 0;
    }

  if (!ecsig && !(ecsig = ECDSA_SIG_new()))
    return 0;

  if (sig_len != 2*t || key_len != 2*t ||
      !blockdata_retrieve(key_data, key_len, keybuf + 1))
    return 0;

  keybuf[0] = POINT_CONVERSION_UNCOMPRESSED;
  if (!o2i_ECPublicKey(&eckey, &p, (int)key_len + 1))
    return 0;

  R = BN_bin2bn(sig, t, NULL);
  S = BN_bin2bn(sig + t, t, NULL);
  if (!R || !S || !ECDSA_SIG_set0(ecsig, R, S))
    goto err;

  r = ECDSA_do_verify(digest, digest_len, ecsig, eckey);

  EC_KEY_free(eckey), eckey = NULL;

  return (r > 0);

err:
  BN_free(R);
  BN_free(S);

  return 0;
}

#ifdef HAVE_GOST
static int dnsmasq_gost_init(void)
{
  ENGINE *engine;

  if (!EVP_PKEY_asn1_find_str(NULL, SN_id_GostR3410_2001, -1))
    {
      if (!(engine = ENGINE_by_id("gost")))
	{
	  ENGINE_load_builtin_engines();
	  ENGINE_load_dynamic();
	  if (!(engine = ENGINE_by_id("gost")))
	    return 0;
	}

      if (ENGINE_set_default(engine, ENGINE_METHOD_ALL) <= 0 ||
	  !EVP_PKEY_asn1_find_str(&engine, SN_id_GostR3410_2001, -1))
	{
	  ENGINE_finish(engine);
	  ENGINE_free(engine);
	  return 0;
	}

      gost_engine = engine;
    }

  gost_hash = SN_id_GostR3411_94;

  return 1;
}

static int dnsmasq_gost_verify(struct blockdata *key_data, unsigned int key_len,
			       unsigned char *sig, size_t sig_len,
			       unsigned char *digest, size_t digest_len, int algo)
{
  static const unsigned char hdr[37] = {
    0x30, 0x63, 0x30, 0x1c, 0x06, 0x06, 0x2a, 0x85, 0x03, 0x02, 0x02, 0x13, 0x30,
    0x12, 0x06, 0x07, 0x2a, 0x85, 0x03, 0x02, 0x02, 0x23, 0x01, 0x06, 0x07, 0x2a,
    0x85, 0x03, 0x02, 0x02, 0x1e, 0x01, 0x03, 0x43, 0x00, 0x04, 0x40 };
  static EVP_PKEY *pkey = NULL;
  static EVP_PKEY_CTX *pkctx = NULL;

  unsigned char keybuf[sizeof(hdr) + 64];
  const unsigned char *p = keybuf;
  int r;

  if (key_len != 64 || sig_len != 64)
    return 0;

  memcpy(keybuf, hdr, sizeof(hdr));
  if (!blockdata_retrieve(key_data, key_len, keybuf + sizeof(hdr)) ||
      !d2i_PUBKEY(&pkey, &p, (int)key_len + sizeof(hdr)))
    return 0;

  if (!pkctx && !(pkctx = EVP_PKEY_CTX_new(pkey, NULL)))
    return 0;

  if (EVP_PKEY_verify_init(pkctx) <= 0)
    return 0;

  r = EVP_PKEY_verify(pkctx, sig, sig_len, digest, digest_len);

  EVP_PKEY_CTX_free(pkctx), pkctx = NULL;

  return (r > 0);
}
#endif

#ifdef HAVE_EDDSA
static int dnsmasq_eddsa_verify(struct blockdata *key_data, unsigned int key_len,
				unsigned char *sig, size_t sig_len,
				unsigned char *digest, size_t digest_len, int algo)
{
  static const unsigned char ed25519_hdr[] = { 0x30, 0x2a, 0x30, 0x05, 0x06, 0x03, 0x2b, 0x65, 0x70, 0x03, 0x21, 0x00 };
  static const unsigned char ed448_hdr[] = { 0x30, 0x43, 0x30, 0x05, 0x06, 0x03, 0x2b, 0x65, 0x71, 0x03, 0x3a, 0x00 };
  static EVP_MD_CTX *mdctx = NULL;
  static EVP_PKEY *pkey = NULL;

  unsigned char keybuf[256];
  const unsigned char *p = keybuf;
  int r, hdr_len;

#define ED25519_KEY_SIZE 32
#define ED25519_SIGNATURE_SIZE 64
#define ED448_KEY_SIZE 57
#define ED448_SIGNATURE_SIZE 114

  if (digest_len != sizeof(struct null_hash_digest))
    return 0;

  switch (algo)
    {
    case 15:
      if (key_len != ED25519_KEY_SIZE ||
	  sig_len != ED25519_SIGNATURE_SIZE)
	return 0;
      hdr_len = sizeof(ed25519_hdr);
      memcpy(keybuf, ed25519_hdr, hdr_len);
      break;

    case 16:
      if (key_len != ED448_KEY_SIZE ||
	  sig_len != ED448_SIGNATURE_SIZE)
	return 0;
      hdr_len = sizeof(ed448_hdr);
      memcpy(keybuf, ed448_hdr, hdr_len);
      break;

    default:
      return 0;
    }

  if (!blockdata_retrieve(key_data, key_len, keybuf + hdr_len) ||
      !d2i_PUBKEY(&pkey, &p, (int)key_len + hdr_len))
    return 0;

  if (mdctx)
    EVP_MD_CTX_init(mdctx);
  else if (!(mdctx = EVP_MD_CTX_create()))
    return 0;

  if (EVP_DigestVerifyInit(mdctx, NULL, NULL, NULL, pkey) <= 0)
    return 0;

  r = EVP_DigestVerify(mdctx, sig, sig_len,
		       ((struct null_hash_digest *)digest)->buff,
		       ((struct null_hash_digest *)digest)->len);

  return (r > 0);
}
#endif

static int (*verify_func(int algo))(struct blockdata *key_data, unsigned int key_len, unsigned char *sig, size_t sig_len,
			     unsigned char *digest, size_t digest_len, int algo)
{
  /* Enure at runtime that we have support for this digest */
  if (!hash_find(algo_digest_name(algo)))
    return NULL;

  /* This switch defines which sig algorithms we support. */
  switch (algo)
    {
    case 1: case 5: case 7: case 8: case 10:
      return dnsmasq_rsa_verify;

#ifdef HAVE_DSA
    case 3: case 6:
      return dnsmasq_dsa_verify;
#endif

#ifdef HAVE_GOST
    case 12:
      return dnsmasq_gost_verify;
#endif

    case 13: case 14:
      return dnsmasq_ecdsa_verify;

#ifdef HAVE_EDDSA
    case 15: case 16:
      return dnsmasq_eddsa_verify;
#endif
    }

  return NULL;
}

int verify(struct blockdata *key_data, unsigned int key_len, unsigned char *sig, size_t sig_len,
	   unsigned char *digest, size_t digest_len, int algo)
{

  int (*func)(struct blockdata *key_data, unsigned int key_len, unsigned char *sig, size_t sig_len,
	      unsigned char *digest, size_t digest_len, int algo);

  func = verify_func(algo);

  if (!func)
    return 0;

  return (*func)(key_data, key_len, sig, sig_len, digest, digest_len, algo);
}

/* Note the ds_digest_name(), algo_digest_name() and nsec3_digest_name()
   define which algo numbers we support. If algo_digest_name() returns
   non-NULL for an algorithm number, we assume that algorithm is
   supported by verify(). */

/* http://www.iana.org/assignments/ds-rr-types/ds-rr-types.xhtml */
char *ds_digest_name(int digest)
{
  switch (digest)
    {
    case 1: return SN_sha1;
    case 2: return SN_sha256;
#ifdef HAVE_GOST
    case 3: return gost_hash;
#endif
    case 4: return SN_sha384;
    default: return NULL;
    }
}

/* http://www.iana.org/assignments/dns-sec-alg-numbers/dns-sec-alg-numbers.xhtml */
char *algo_digest_name(int algo)
{
  switch (algo)
    {
    case 1: return NULL;          /* RSA/MD5 - Must Not Implement.  RFC 6944 para 2.3. */
    case 2: return NULL;          /* Diffie-Hellman */
#ifdef HAVE_DSA
    case 3: return SN_sha1;       /* DSA/SHA1 - Must Not Implement. RFC 8624 para 1.3. */
    case 6: return SN_sha1;       /* DSA-NSEC3-SHA1 - Must Not Implement. RFC 8624 para 1.3. */
#endif
    case 5: return SN_sha1;       /* RSA/SHA1 */
    case 7: return SN_sha1;       /* RSASHA1-NSEC3-SHA1 */
    case 8: return SN_sha256;     /* RSA/SHA-256 */
    case 10: return SN_sha512;    /* RSA/SHA-512 */
#ifdef HAVE_GOST
    case 12: return gost_hash;    /* ECC-GOST */
#endif
    case 13: return SN_sha256;    /* ECDSAP256SHA256 */
    case 14: return SN_sha384;    /* ECDSAP384SHA384 */
#ifdef HAVE_EDDSA
    case 15: return "null_hash";  /* ED25519 */
    case 16: return "null_hash";  /* ED448 */
#endif
    default: return NULL;
    }
}

/* http://www.iana.org/assignments/dnssec-nsec3-parameters/dnssec-nsec3-parameters.xhtml */
char *nsec3_digest_name(int digest)
{
  switch (digest)
    {
    case 1: return SN_sha1;
    default: return NULL;
    }
}

#if OPENSSL_VERSION_NUMBER >= 0x10100000L
static void *dnsmasq_malloc(size_t size, const char *file, int line)
#else
static void *dnsmasq_malloc(size_t size)
#endif
{
  void *ret = malloc(size);

#if OPENSSL_VERSION_NUMBER >= 0x10100000L
  (void)file;
  (void)line;
#endif

  if (!ret)
    my_syslog(LOG_ERR, _("failed to allocate %d bytes"), (int) size);

  return ret;
}

#if OPENSSL_VERSION_NUMBER >= 0x10100000L
static void *dnsmasq_realloc(void *p, size_t size, const char *file, int line)
#else
static void *dnsmasq_realloc(void *p, size_t size)
#endif
{
  void *ret = realloc(p, size);

#if OPENSSL_VERSION_NUMBER >= 0x10100000L
  (void)file;
  (void)line;
#endif

  if (!ret)
    my_syslog(LOG_ERR, _("failed to allocate %d bytes"), (int) size);

  return ret;
}

#if OPENSSL_VERSION_NUMBER >= 0x10100000L
static void dnsmasq_free(void *str, const char *file, int line)
#else
static void dnsmasq_free(void *str)
#endif
{
#if OPENSSL_VERSION_NUMBER >= 0x10100000L
  (void)file;
  (void)line;
#endif

  free(str);
}

void crypto_init(void)
{
  CRYPTO_set_mem_functions(dnsmasq_malloc,
			   dnsmasq_realloc,
			   dnsmasq_free);
  OPENSSL_add_all_algorithms_conf();

#ifdef HAVE_GOST
  dnsmasq_gost_init();
#endif
}

#endif

/* dnsmasq is Copyright (c) 2000-2018 Simon Kelley

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

#ifdef HAVE_DNSSEC

#include <nettle/rsa.h>
#include <nettle/dsa.h>
#include <nettle/ecdsa.h>
#include <nettle/ecc-curve.h>
#include <nettle/eddsa.h>
#include <nettle/nettle-meta.h>
#include <nettle/bignum.h>

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

static void null_hash_init(void *ctx)
{
  ((struct null_hash_ctx *)ctx)->len = 0;
}

static void null_hash_update(void *ctxv, size_t length, const uint8_t *src)
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
 

static void null_hash_digest(void *ctx, size_t length, uint8_t *dst)
{
  (void)length;
  
  ((struct null_hash_digest *)dst)->buff = null_hash_buff;
  ((struct null_hash_digest *)dst)->len = ((struct null_hash_ctx *)ctx)->len;
}

static struct nettle_hash null_hash = {
  "null_hash",
  sizeof(struct null_hash_ctx),
  sizeof(struct null_hash_digest),
  0,
  (nettle_hash_init_func *) null_hash_init,
  (nettle_hash_update_func *) null_hash_update,
  (nettle_hash_digest_func *) null_hash_digest
};

/* Find pointer to correct hash function in nettle library */
const struct nettle_hash *hash_find(char *name)
{
  if (!name)
    return NULL;
  
  /* We provide a "null" hash which returns the input data as digest. */
  if (strcmp(null_hash.name, name) == 0)
    return &null_hash;

  /* libnettle >= 3.4 provides nettle_lookup_hash() which avoids nasty ABI
     incompatibilities if sizeof(nettle_hashes) changes between library
     versions. It also #defines nettle_hashes, so use that to tell
     if we have the new facilities. */
  
#ifdef nettle_hashes
  return nettle_lookup_hash(name);
#else
  {
    int i;

    for (i = 0; nettle_hashes[i]; i++)
      if (strcmp(nettle_hashes[i]->name, name) == 0)
	return nettle_hashes[i];
  }
  
  return NULL;
#endif
}

/* expand ctx and digest memory allocations if necessary and init hash function */
int hash_init(const struct nettle_hash *hash, void **ctxp, unsigned char **digestp)
{
  static void *ctx = NULL;
  static unsigned char *digest = NULL;
  static unsigned int ctx_sz = 0;
  static unsigned int digest_sz = 0;

  void *new;

  if (ctx_sz < hash->context_size)
    {
      if (!(new = whine_malloc(hash->context_size)))
	return 0;
      if (ctx)
	free(ctx);
      ctx = new;
      ctx_sz = hash->context_size;
    }
  
  if (digest_sz < hash->digest_size)
    {
      if (!(new = whine_malloc(hash->digest_size)))
	return 0;
      if (digest)
	free(digest);
      digest = new;
      digest_sz = hash->digest_size;
    }

  *ctxp = ctx;
  *digestp = digest;

  hash->init(ctx);

  return 1;
}
  
static int dnsmasq_rsa_verify(struct blockdata *key_data, unsigned int key_len, unsigned char *sig, size_t sig_len,
			      unsigned char *digest, size_t digest_len, int algo)
{
  unsigned char *p;
  size_t exp_len;
  
  static struct rsa_public_key *key = NULL;
  static mpz_t sig_mpz;

  (void)digest_len;
  
  if (key == NULL)
    {
      if (!(key = whine_malloc(sizeof(struct rsa_public_key))))
	return 0;
      
      nettle_rsa_public_key_init(key);
      mpz_init(sig_mpz);
    }
  
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
  
  key->size =  key_len - exp_len;
  mpz_import(key->e, exp_len, 1, 1, 0, 0, p);
  mpz_import(key->n, key->size, 1, 1, 0, 0, p + exp_len);

  mpz_import(sig_mpz, sig_len, 1, 1, 0, 0, sig);
  
  switch (algo)
    {
    case 1:
      return nettle_rsa_md5_verify_digest(key, digest, sig_mpz);
    case 5: case 7:
      return nettle_rsa_sha1_verify_digest(key, digest, sig_mpz);
    case 8:
      return nettle_rsa_sha256_verify_digest(key, digest, sig_mpz);
    case 10:
      return nettle_rsa_sha512_verify_digest(key, digest, sig_mpz);
    }

  return 0;
}  

static int dnsmasq_dsa_verify(struct blockdata *key_data, unsigned int key_len, unsigned char *sig, size_t sig_len,
			      unsigned char *digest, size_t digest_len, int algo)
{
  unsigned char *p;
  unsigned int t;

  static mpz_t y;
  static struct dsa_params *params = NULL;
  static struct dsa_signature *sig_struct;
  
  (void)digest_len;

  if (params == NULL)
    {
      if (!(sig_struct = whine_malloc(sizeof(struct dsa_signature))) || 
	  !(params = whine_malloc(sizeof(struct dsa_params)))) 
	return 0;
      
      mpz_init(y);
      nettle_dsa_params_init(params);
      nettle_dsa_signature_init(sig_struct);
    }
  
  if ((sig_len < 41) || !(p = blockdata_retrieve(key_data, key_len, NULL)))
    return 0;
  
  t = *p++;
  
  if (key_len < (213 + (t * 24)))
    return 0;
  
  mpz_import(params->q, 20, 1, 1, 0, 0, p); p += 20;
  mpz_import(params->p, 64 + (t*8), 1, 1, 0, 0, p); p += 64 + (t*8);
  mpz_import(params->g, 64 + (t*8), 1, 1, 0, 0, p); p += 64 + (t*8);
  mpz_import(y, 64 + (t*8), 1, 1, 0, 0, p); p += 64 + (t*8);
  
  mpz_import(sig_struct->r, 20, 1, 1, 0, 0, sig+1);
  mpz_import(sig_struct->s, 20, 1, 1, 0, 0, sig+21);
  
  (void)algo;
  
  return nettle_dsa_verify(params, y, digest_len, digest, sig_struct);
} 
 
static int dnsmasq_ecdsa_verify(struct blockdata *key_data, unsigned int key_len, 
				unsigned char *sig, size_t sig_len,
				unsigned char *digest, size_t digest_len, int algo)
{
  unsigned char *p;
  unsigned int t;
  struct ecc_point *key;

  static struct ecc_point *key_256 = NULL, *key_384 = NULL;
  static mpz_t x, y;
  static struct dsa_signature *sig_struct;
  
  if (!sig_struct)
    {
      if (!(sig_struct = whine_malloc(sizeof(struct dsa_signature))))
	return 0;
      
      nettle_dsa_signature_init(sig_struct);
      mpz_init(x);
      mpz_init(y);
    }
  
  switch (algo)
    {
    case 13:
      if (!key_256)
	{
	  if (!(key_256 = whine_malloc(sizeof(struct ecc_point))))
	    return 0;
	  
	  nettle_ecc_point_init(key_256, &nettle_secp_256r1);
	}
      
      key = key_256;
      t = 32;
      break;
      
    case 14:
      if (!key_384)
	{
	  if (!(key_384 = whine_malloc(sizeof(struct ecc_point))))
	    return 0;
	  
	  nettle_ecc_point_init(key_384, &nettle_secp_384r1);
	}
      
      key = key_384;
      t = 48;
      break;
        
    default:
      return 0;
    }
  
  if (sig_len != 2*t || key_len != 2*t ||
      !(p = blockdata_retrieve(key_data, key_len, NULL)))
    return 0;
  
  mpz_import(x, t , 1, 1, 0, 0, p);
  mpz_import(y, t , 1, 1, 0, 0, p + t);

  if (!ecc_point_set(key, x, y))
    return 0;
  
  mpz_import(sig_struct->r, t, 1, 1, 0, 0, sig);
  mpz_import(sig_struct->s, t, 1, 1, 0, 0, sig + t);
  
  return nettle_ecdsa_verify(key, digest_len, digest, sig_struct);
}

static int dnsmasq_eddsa_verify(struct blockdata *key_data, unsigned int key_len, 
				unsigned char *sig, size_t sig_len,
				unsigned char *digest, size_t digest_len, int algo)
{
  unsigned char *p;
   
  if (key_len != ED25519_KEY_SIZE ||
      sig_len != ED25519_SIGNATURE_SIZE ||
      digest_len != sizeof(struct null_hash_digest) ||
      !(p = blockdata_retrieve(key_data, key_len, NULL)))
    return 0;
  
  /* The "digest" returned by the null_hash function is simply a struct null_hash_digest
     which has a pointer to the actual data and a length, because the buffer
     may need to be extended during "hashing". */
  
  switch (algo)
    {
    case 15:
      return ed25519_sha512_verify(p,
				   ((struct null_hash_digest *)digest)->len,
				   ((struct null_hash_digest *)digest)->buff,
				   sig);
    case 16:
      /* Ed448 when available */
      return 0;
    }

  return 0;
}

static int (*verify_func(int algo))(struct blockdata *key_data, unsigned int key_len, unsigned char *sig, size_t sig_len,
			     unsigned char *digest, size_t digest_len, int algo)
{
    
  /* Enure at runtime that we have support for this digest */
  if (!hash_find(algo_digest_name(algo)))
    return NULL;
  
  /* This switch defines which sig algorithms we support, can't introspect Nettle for that. */
  switch (algo)
    {
    case 1: case 5: case 7: case 8: case 10:
      return dnsmasq_rsa_verify;
      
    case 3: case 6: 
      return dnsmasq_dsa_verify;
    
    case 13: case 14:
      return dnsmasq_ecdsa_verify;

    case 15: case 16:
      return dnsmasq_eddsa_verify;
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
    case 1: return "sha1";
    case 2: return "sha256";
    case 3: return "gosthash94";
    case 4: return "sha384";
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
    case 3: return "sha1";        /* DSA/SHA1 */ 
    case 5: return "sha1";        /* RSA/SHA1 */
    case 6: return "sha1";        /* DSA-NSEC3-SHA1 */
    case 7: return "sha1";        /* RSASHA1-NSEC3-SHA1 */
    case 8: return "sha256";      /* RSA/SHA-256 */
    case 10: return "sha512";     /* RSA/SHA-512 */
    case 12: return NULL;         /* ECC-GOST */
    case 13: return "sha256";     /* ECDSAP256SHA256 */
    case 14: return "sha384";     /* ECDSAP384SHA384 */ 	
    case 15: return "null_hash";  /* ED25519 */
    case 16: return NULL;         /* ED448 */
    default: return NULL;
    }
}
  
/* http://www.iana.org/assignments/dnssec-nsec3-parameters/dnssec-nsec3-parameters.xhtml */
char *nsec3_digest_name(int digest)
{
  switch (digest)
    {
    case 1: return "sha1";
    default: return NULL;
    }
}

#endif

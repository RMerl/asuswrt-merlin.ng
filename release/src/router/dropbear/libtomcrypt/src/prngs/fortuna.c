/* LibTomCrypt, modular cryptographic library -- Tom St Denis
 *
 * LibTomCrypt is a library that provides various cryptographic
 * algorithms in a highly modular and flexible manner.
 *
 * The library is free for all purposes without any express
 * guarantee it works.
 */
#include "tomcrypt.h"

/**
  @file fortuna.c
  Fortuna PRNG, Tom St Denis
*/

/* Implementation of Fortuna by Tom St Denis

We deviate slightly here for reasons of simplicity [and to fit in the API].  First all "sources"
in the AddEntropy function are fixed to 0.  Second since no reliable timer is provided
we reseed automatically when len(pool0) >= 64 or every LTC_FORTUNA_WD calls to the read function */

#ifdef LTC_FORTUNA

/* requries LTC_SHA256 and AES  */
#if !(defined(LTC_RIJNDAEL) && defined(LTC_SHA256))
   #error LTC_FORTUNA requires LTC_SHA256 and LTC_RIJNDAEL (AES)
#endif

#ifndef LTC_FORTUNA_POOLS
   #warning LTC_FORTUNA_POOLS was not previously defined (old headers?)
   #define LTC_FORTUNA_POOLS 32
#endif

#if LTC_FORTUNA_POOLS < 4 || LTC_FORTUNA_POOLS > 32
   #error LTC_FORTUNA_POOLS must be in [4..32]
#endif

const struct ltc_prng_descriptor fortuna_desc = {
    "fortuna",
    (32 * LTC_FORTUNA_POOLS), /* default: 1024 */
    &fortuna_start,
    &fortuna_add_entropy,
    &fortuna_ready,
    &fortuna_read,
    &fortuna_done,
    &fortuna_export,
    &fortuna_import,
    &fortuna_test
};

/* update the IV */
static void _fortuna_update_iv(prng_state *prng)
{
   int            x;
   unsigned char *IV;
   /* update IV */
   IV = prng->fortuna.IV;
   for (x = 0; x < 16; x++) {
      IV[x] = (IV[x] + 1) & 255;
      if (IV[x] != 0) break;
   }
}

/* reseed the PRNG */
static int _fortuna_reseed(prng_state *prng)
{
   unsigned char tmp[MAXBLOCKSIZE];
   hash_state    md;
   ulong64       reset_cnt;
   int           err, x;


   /* new K == LTC_SHA256(K || s) where s == LTC_SHA256(P0) || LTC_SHA256(P1) ... */
   sha256_init(&md);
   if ((err = sha256_process(&md, prng->fortuna.K, 32)) != CRYPT_OK) {
      sha256_done(&md, tmp);
      return err;
   }

   reset_cnt = prng->fortuna.reset_cnt + 1;

   for (x = 0; x < LTC_FORTUNA_POOLS; x++) {
       if (x == 0 || ((reset_cnt >> (x-1)) & 1) == 0) {
          /* terminate this hash */
          if ((err = sha256_done(&prng->fortuna.pool[x], tmp)) != CRYPT_OK) {
             sha256_done(&md, tmp);
             return err;
          }
          /* add it to the string */
          if ((err = sha256_process(&md, tmp, 32)) != CRYPT_OK) {
             sha256_done(&md, tmp);
             return err;
          }
          /* reset this pool */
          if ((err = sha256_init(&prng->fortuna.pool[x])) != CRYPT_OK) {
             sha256_done(&md, tmp);
             return err;
          }
       } else {
          break;
       }
   }

   /* finish key */
   if ((err = sha256_done(&md, prng->fortuna.K)) != CRYPT_OK) {
      return err;
   }
   if ((err = rijndael_setup(prng->fortuna.K, 32, 0, &prng->fortuna.skey)) != CRYPT_OK) {
      return err;
   }
   _fortuna_update_iv(prng);

   /* reset/update internals */
   prng->fortuna.pool0_len = 0;
   prng->fortuna.wd        = 0;
   prng->fortuna.reset_cnt = reset_cnt;


#ifdef LTC_CLEAN_STACK
   zeromem(&md, sizeof(md));
   zeromem(tmp, sizeof(tmp));
#endif

   return CRYPT_OK;
}

/**
  "Update Seed File"-compliant update of K

  @param in       The PRNG state
  @param inlen    Size of the state
  @param prng     The PRNG to import
  @return CRYPT_OK if successful
*/
static int _fortuna_update_seed(const unsigned char *in, unsigned long inlen, prng_state *prng)
{
   int           err;
   unsigned char tmp[MAXBLOCKSIZE];
   hash_state    md;

   LTC_MUTEX_LOCK(&prng->lock);
   /* new K = LTC_SHA256(K || in) */
   sha256_init(&md);
   if ((err = sha256_process(&md, prng->fortuna.K, 32)) != CRYPT_OK) {
      sha256_done(&md, tmp);
      goto LBL_UNLOCK;
   }
   if ((err = sha256_process(&md, in, inlen)) != CRYPT_OK) {
      sha256_done(&md, tmp);
      goto LBL_UNLOCK;
   }
   /* finish key */
   if ((err = sha256_done(&md, prng->fortuna.K)) != CRYPT_OK) {
      goto LBL_UNLOCK;
   }
   _fortuna_update_iv(prng);

LBL_UNLOCK:
   LTC_MUTEX_UNLOCK(&prng->lock);
#ifdef LTC_CLEAN_STACK
   zeromem(&md, sizeof(md));
#endif

   return err;
}

/**
  Start the PRNG
  @param prng     [out] The PRNG state to initialize
  @return CRYPT_OK if successful
*/
int fortuna_start(prng_state *prng)
{
   int err, x, y;
   unsigned char tmp[MAXBLOCKSIZE];

   LTC_ARGCHK(prng != NULL);
   prng->ready = 0;

   /* initialize the pools */
   for (x = 0; x < LTC_FORTUNA_POOLS; x++) {
       if ((err = sha256_init(&prng->fortuna.pool[x])) != CRYPT_OK) {
          for (y = 0; y < x; y++) {
              sha256_done(&prng->fortuna.pool[y], tmp);
          }
          return err;
       }
   }
   prng->fortuna.pool_idx = prng->fortuna.pool0_len = prng->fortuna.wd = 0;
   prng->fortuna.reset_cnt = 0;

   /* reset bufs */
   zeromem(prng->fortuna.K, 32);
   if ((err = rijndael_setup(prng->fortuna.K, 32, 0, &prng->fortuna.skey)) != CRYPT_OK) {
      for (x = 0; x < LTC_FORTUNA_POOLS; x++) {
          sha256_done(&prng->fortuna.pool[x], tmp);
      }
      return err;
   }
   zeromem(prng->fortuna.IV, 16);

   LTC_MUTEX_INIT(&prng->lock)

   return CRYPT_OK;
}

/**
  Add entropy to the PRNG state
  @param in       The data to add
  @param inlen    Length of the data to add
  @param prng     PRNG state to update
  @return CRYPT_OK if successful
*/
int fortuna_add_entropy(const unsigned char *in, unsigned long inlen, prng_state *prng)
{
   unsigned char tmp[2];
   int           err;

   LTC_ARGCHK(prng != NULL);
   LTC_ARGCHK(in != NULL);
   LTC_ARGCHK(inlen > 0);

   /* ensure inlen <= 32 */
   if (inlen > 32) {
      inlen = 32;
   }

   /* add s || length(in) || in to pool[pool_idx] */
   tmp[0] = 0;
   tmp[1] = (unsigned char)inlen;

   LTC_MUTEX_LOCK(&prng->lock);
   if ((err = sha256_process(&prng->fortuna.pool[prng->fortuna.pool_idx], tmp, 2)) != CRYPT_OK) {
      goto LBL_UNLOCK;
   }
   if ((err = sha256_process(&prng->fortuna.pool[prng->fortuna.pool_idx], in, inlen)) != CRYPT_OK) {
      goto LBL_UNLOCK;
   }
   if (prng->fortuna.pool_idx == 0) {
      prng->fortuna.pool0_len += inlen;
   }
   if (++(prng->fortuna.pool_idx) == LTC_FORTUNA_POOLS) {
      prng->fortuna.pool_idx = 0;
   }
   err = CRYPT_OK; /* success */

LBL_UNLOCK:
   LTC_MUTEX_UNLOCK(&prng->lock);
   return err;
}

/**
  Make the PRNG ready to read from
  @param prng   The PRNG to make active
  @return CRYPT_OK if successful
*/
int fortuna_ready(prng_state *prng)
{
   int err;
   LTC_ARGCHK(prng != NULL);

   LTC_MUTEX_LOCK(&prng->lock);
   err = _fortuna_reseed(prng);
   prng->ready = (err == CRYPT_OK) ? 1 : 0;

   LTC_MUTEX_UNLOCK(&prng->lock);
   return err;
}

/**
  Read from the PRNG
  @param out      Destination
  @param outlen   Length of output
  @param prng     The active PRNG to read from
  @return Number of octets read
*/
unsigned long fortuna_read(unsigned char *out, unsigned long outlen, prng_state *prng)
{
   unsigned char tmp[16];
   unsigned long tlen = 0;

   if (outlen == 0 || prng == NULL || out == NULL) return 0;

   LTC_MUTEX_LOCK(&prng->lock);

   if (!prng->ready) {
      goto LBL_UNLOCK;
   }

   /* do we have to reseed? */
   if ((++prng->fortuna.wd == LTC_FORTUNA_WD) && (prng->fortuna.pool0_len >= 64)) {
      if (_fortuna_reseed(prng) != CRYPT_OK) {
         goto LBL_UNLOCK;
      }
   }

   /* ensure that one reseed happened before allowing to read */
   if (prng->fortuna.reset_cnt == 0) {
      goto LBL_UNLOCK;
   }

   /* now generate the blocks required */
   tlen = outlen;

   /* handle whole blocks without the extra XMEMCPY */
   while (outlen >= 16) {
      /* encrypt the IV and store it */
      rijndael_ecb_encrypt(prng->fortuna.IV, out, &prng->fortuna.skey);
      out += 16;
      outlen -= 16;
      _fortuna_update_iv(prng);
   }

   /* left over bytes? */
   if (outlen > 0) {
      rijndael_ecb_encrypt(prng->fortuna.IV, tmp, &prng->fortuna.skey);
      XMEMCPY(out, tmp, outlen);
      _fortuna_update_iv(prng);
   }

   /* generate new key */
   rijndael_ecb_encrypt(prng->fortuna.IV, prng->fortuna.K   , &prng->fortuna.skey);
   _fortuna_update_iv(prng);

   rijndael_ecb_encrypt(prng->fortuna.IV, prng->fortuna.K+16, &prng->fortuna.skey);
   _fortuna_update_iv(prng);

   if (rijndael_setup(prng->fortuna.K, 32, 0, &prng->fortuna.skey) != CRYPT_OK) {
      tlen = 0;
   }

LBL_UNLOCK:
#ifdef LTC_CLEAN_STACK
   zeromem(tmp, sizeof(tmp));
#endif
   LTC_MUTEX_UNLOCK(&prng->lock);
   return tlen;
}

/**
  Terminate the PRNG
  @param prng   The PRNG to terminate
  @return CRYPT_OK if successful
*/
int fortuna_done(prng_state *prng)
{
   int           err, x;
   unsigned char tmp[32];

   LTC_ARGCHK(prng != NULL);

   LTC_MUTEX_LOCK(&prng->lock);
   prng->ready = 0;

   /* terminate all the hashes */
   for (x = 0; x < LTC_FORTUNA_POOLS; x++) {
       if ((err = sha256_done(&(prng->fortuna.pool[x]), tmp)) != CRYPT_OK) {
          goto LBL_UNLOCK;
       }
   }
   /* call cipher done when we invent one ;-) */
   err = CRYPT_OK; /* success */

LBL_UNLOCK:
#ifdef LTC_CLEAN_STACK
   zeromem(tmp, sizeof(tmp));
#endif
   LTC_MUTEX_UNLOCK(&prng->lock);
   LTC_MUTEX_DESTROY(&prng->lock);
   return err;
}

/**
  Export the PRNG state
  @param out       [out] Destination
  @param outlen    [in/out] Max size and resulting size of the state
  @param prng      The PRNG to export
  @return CRYPT_OK if successful
*/
int fortuna_export(unsigned char *out, unsigned long *outlen, prng_state *prng)
{
   int         x, err;
   hash_state *md;
   unsigned long len = fortuna_desc.export_size;

   LTC_ARGCHK(out    != NULL);
   LTC_ARGCHK(outlen != NULL);
   LTC_ARGCHK(prng   != NULL);

   LTC_MUTEX_LOCK(&prng->lock);

   if (!prng->ready) {
      err = CRYPT_ERROR;
      goto LBL_UNLOCK;
   }

   /* we'll write bytes for s&g's */
   if (*outlen < len) {
      *outlen = len;
      err = CRYPT_BUFFER_OVERFLOW;
      goto LBL_UNLOCK;
   }

   md = XMALLOC(sizeof(hash_state));
   if (md == NULL) {
      err = CRYPT_MEM;
      goto LBL_UNLOCK;
   }

   /* to emit the state we copy each pool, terminate it then hash it again so
    * an attacker who sees the state can't determine the current state of the PRNG
    */
   for (x = 0; x < LTC_FORTUNA_POOLS; x++) {
      /* copy the PRNG */
      XMEMCPY(md, &(prng->fortuna.pool[x]), sizeof(*md));

      /* terminate it */
      if ((err = sha256_done(md, out+x*32)) != CRYPT_OK) {
         goto LBL_ERR;
      }

      /* now hash it */
      if ((err = sha256_init(md)) != CRYPT_OK) {
         goto LBL_ERR;
      }
      if ((err = sha256_process(md, out+x*32, 32)) != CRYPT_OK) {
         goto LBL_ERR;
      }
      if ((err = sha256_done(md, out+x*32)) != CRYPT_OK) {
         goto LBL_ERR;
      }
   }
   *outlen = len;
   err = CRYPT_OK;

LBL_ERR:
#ifdef LTC_CLEAN_STACK
   zeromem(md, sizeof(*md));
#endif
   XFREE(md);
LBL_UNLOCK:
   LTC_MUTEX_UNLOCK(&prng->lock);
   return err;
}

/**
  Import a PRNG state
  @param in       The PRNG state
  @param inlen    Size of the state
  @param prng     The PRNG to import
  @return CRYPT_OK if successful
*/
int fortuna_import(const unsigned char *in, unsigned long inlen, prng_state *prng)
{
   int           err;

   LTC_ARGCHK(in    != NULL);
   LTC_ARGCHK(prng  != NULL);

   if (inlen < (unsigned long)fortuna_desc.export_size) {
      return CRYPT_INVALID_ARG;
   }

   if ((err = fortuna_start(prng)) != CRYPT_OK) {
      return err;
   }

   if ((err = _fortuna_update_seed(in, inlen, prng)) != CRYPT_OK) {
      return err;
   }

   return err;
}

/**
  PRNG self-test
  @return CRYPT_OK if successful, CRYPT_NOP if self-testing has been disabled
*/
int fortuna_test(void)
{
#ifndef LTC_TEST
   return CRYPT_NOP;
#else
   int err;

   if ((err = sha256_test()) != CRYPT_OK) {
      return err;
   }
   return rijndael_test();
#endif
}

#endif


/* ref:         $Format:%D$ */
/* git commit:  $Format:%H$ */
/* commit time: $Format:%ai$ */

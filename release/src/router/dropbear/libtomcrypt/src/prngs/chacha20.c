/* LibTomCrypt, modular cryptographic library -- Tom St Denis
 *
 * LibTomCrypt is a library that provides various cryptographic
 * algorithms in a highly modular and flexible manner.
 *
 * The library is free for all purposes without any express
 * guarantee it works.
 */

 /* the idea of re-keying loosely follows the approach used in:
  * http://bxr.su/OpenBSD/lib/libc/crypt/arc4random.c
  */

#include "tomcrypt.h"

#ifdef LTC_CHACHA20_PRNG

const struct ltc_prng_descriptor chacha20_prng_desc =
{
   "chacha20",
   40,
   &chacha20_prng_start,
   &chacha20_prng_add_entropy,
   &chacha20_prng_ready,
   &chacha20_prng_read,
   &chacha20_prng_done,
   &chacha20_prng_export,
   &chacha20_prng_import,
   &chacha20_prng_test
};

/**
  Start the PRNG
  @param prng The PRNG state to initialize
  @return CRYPT_OK if successful
*/
int chacha20_prng_start(prng_state *prng)
{
   LTC_ARGCHK(prng != NULL);
   prng->ready = 0;
   XMEMSET(&prng->chacha.ent, 0, sizeof(prng->chacha.ent));
   prng->chacha.idx = 0;
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
int chacha20_prng_add_entropy(const unsigned char *in, unsigned long inlen, prng_state *prng)
{
   unsigned char buf[40];
   unsigned long i;
   int err;

   LTC_ARGCHK(prng != NULL);
   LTC_ARGCHK(in != NULL);
   LTC_ARGCHK(inlen > 0);

   LTC_MUTEX_LOCK(&prng->lock);
   if (prng->ready) {
      /* chacha20_prng_ready() was already called, do "rekey" operation */
      if ((err = chacha_keystream(&prng->chacha.s, buf, sizeof(buf))) != CRYPT_OK) goto LBL_UNLOCK;
      for(i = 0; i < inlen; i++) buf[i % sizeof(buf)] ^= in[i];
      /* key 32 bytes, 20 rounds */
      if ((err = chacha_setup(&prng->chacha.s, buf, 32, 20)) != CRYPT_OK)      goto LBL_UNLOCK;
      /* iv 8 bytes */
      if ((err = chacha_ivctr64(&prng->chacha.s, buf + 32, 8, 0)) != CRYPT_OK) goto LBL_UNLOCK;
      /* clear KEY + IV */
      zeromem(buf, sizeof(buf));
   }
   else {
      /* chacha20_prng_ready() was not called yet, add entropy to ent buffer */
      while (inlen--) prng->chacha.ent[prng->chacha.idx++ % sizeof(prng->chacha.ent)] ^= *in++;
   }
   err = CRYPT_OK;
LBL_UNLOCK:
   LTC_MUTEX_UNLOCK(&prng->lock);
   return err;
}

/**
  Make the PRNG ready to read from
  @param prng   The PRNG to make active
  @return CRYPT_OK if successful
*/
int chacha20_prng_ready(prng_state *prng)
{
   int err;

   LTC_ARGCHK(prng != NULL);

   LTC_MUTEX_LOCK(&prng->lock);
   if (prng->ready)                                                    { err = CRYPT_OK; goto LBL_UNLOCK; }
   /* key 32 bytes, 20 rounds */
   if ((err = chacha_setup(&prng->chacha.s, prng->chacha.ent, 32, 20)) != CRYPT_OK)      goto LBL_UNLOCK;
   /* iv 8 bytes */
   if ((err = chacha_ivctr64(&prng->chacha.s, prng->chacha.ent + 32, 8, 0)) != CRYPT_OK) goto LBL_UNLOCK;
   XMEMSET(&prng->chacha.ent, 0, sizeof(prng->chacha.ent));
   prng->chacha.idx = 0;
   prng->ready = 1;
LBL_UNLOCK:
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
unsigned long chacha20_prng_read(unsigned char *out, unsigned long outlen, prng_state *prng)
{
   if (outlen == 0 || prng == NULL || out == NULL) return 0;
   LTC_MUTEX_LOCK(&prng->lock);
   if (!prng->ready) { outlen = 0; goto LBL_UNLOCK; }
   if (chacha_keystream(&prng->chacha.s, out, outlen) != CRYPT_OK) outlen = 0;
LBL_UNLOCK:
   LTC_MUTEX_UNLOCK(&prng->lock);
   return outlen;
}

/**
  Terminate the PRNG
  @param prng   The PRNG to terminate
  @return CRYPT_OK if successful
*/
int chacha20_prng_done(prng_state *prng)
{
   int err;
   LTC_ARGCHK(prng != NULL);
   LTC_MUTEX_LOCK(&prng->lock);
   prng->ready = 0;
   err = chacha_done(&prng->chacha.s);
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
int chacha20_prng_export(unsigned char *out, unsigned long *outlen, prng_state *prng)
{
   unsigned long len = chacha20_prng_desc.export_size;

   LTC_ARGCHK(prng   != NULL);
   LTC_ARGCHK(out    != NULL);
   LTC_ARGCHK(outlen != NULL);

   if (*outlen < len) {
      *outlen = len;
      return CRYPT_BUFFER_OVERFLOW;
   }

   if (chacha20_prng_read(out, len, prng) != len) {
      return CRYPT_ERROR_READPRNG;
   }

   *outlen = len;
   return CRYPT_OK;
}

/**
  Import a PRNG state
  @param in       The PRNG state
  @param inlen    Size of the state
  @param prng     The PRNG to import
  @return CRYPT_OK if successful
*/
int chacha20_prng_import(const unsigned char *in, unsigned long inlen, prng_state *prng)
{
   int err;

   LTC_ARGCHK(prng != NULL);
   LTC_ARGCHK(in   != NULL);
   if (inlen < (unsigned long)chacha20_prng_desc.export_size) return CRYPT_INVALID_ARG;

   if ((err = chacha20_prng_start(prng)) != CRYPT_OK)                  return err;
   if ((err = chacha20_prng_add_entropy(in, inlen, prng)) != CRYPT_OK) return err;
   return CRYPT_OK;
}

/**
  PRNG self-test
  @return CRYPT_OK if successful, CRYPT_NOP if self-testing has been disabled
*/
int chacha20_prng_test(void)
{
#ifndef LTC_TEST
   return CRYPT_NOP;
#else
   prng_state st;
   unsigned char en[] = { 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08, 0x09, 0x0a,
                          0x0b, 0x0c, 0x0d, 0x0e, 0x0f, 0x10, 0x11, 0x12, 0x13, 0x14,
                          0x15, 0x16, 0x17, 0x18, 0x19, 0x1a, 0x1b, 0x1c, 0x1d, 0x1e,
                          0x1f, 0x20, 0x21, 0x22, 0x23, 0x24, 0x25, 0x26, 0x27, 0x28,
                          0x29, 0x2a, 0x2b, 0x2c, 0x2d, 0x2e, 0x2f, 0x30, 0x31, 0x32 };
   unsigned char dmp[300];
   unsigned long dmplen = sizeof(dmp);
   unsigned char out[500];
   unsigned char t1[] = { 0x59, 0xB2, 0x26, 0x95, 0x2B, 0x01, 0x8F, 0x05, 0xBE, 0xD8 };
   unsigned char t2[] = { 0x47, 0xC9, 0x0D, 0x03, 0xE4, 0x75, 0x34, 0x27, 0xBD, 0xDE };
   unsigned char t3[] = { 0xBC, 0xFA, 0xEF, 0x59, 0x37, 0x7F, 0x1A, 0x91, 0x1A, 0xA6 };
   int err;

   if ((err = chacha20_prng_start(&st)) != CRYPT_OK)                       return err;
   /* add entropy to uninitialized prng */
   if ((err = chacha20_prng_add_entropy(en, sizeof(en), &st)) != CRYPT_OK) return err;
   if ((err = chacha20_prng_ready(&st)) != CRYPT_OK)                       return err;
   if (chacha20_prng_read(out, 10, &st) != 10)                             return CRYPT_ERROR_READPRNG; /* 10 bytes for testing */
   if (compare_testvector(out, 10, t1, sizeof(t1), "CHACHA-PRNG", 1))      return CRYPT_FAIL_TESTVECTOR;
   if (chacha20_prng_read(out, 500, &st) != 500)                           return CRYPT_ERROR_READPRNG; /* skip 500 bytes */
   /* add entropy to already initialized prng */
   if ((err = chacha20_prng_add_entropy(en, sizeof(en), &st)) != CRYPT_OK) return err;
   if (chacha20_prng_read(out, 500, &st) != 500)                           return CRYPT_ERROR_READPRNG; /* skip 500 bytes */
   if ((err = chacha20_prng_export(dmp, &dmplen, &st)) != CRYPT_OK)        return err;
   if (chacha20_prng_read(out, 500, &st) != 500)                           return CRYPT_ERROR_READPRNG; /* skip 500 bytes */
   if (chacha20_prng_read(out, 10, &st) != 10)                             return CRYPT_ERROR_READPRNG; /* 10 bytes for testing */
   if (compare_testvector(out, 10, t2, sizeof(t2), "CHACHA-PRNG", 2))      return CRYPT_FAIL_TESTVECTOR;
   if ((err = chacha20_prng_done(&st)) != CRYPT_OK)                        return err;
   if ((err = chacha20_prng_import(dmp, dmplen, &st)) != CRYPT_OK)         return err;
   if ((err = chacha20_prng_ready(&st)) != CRYPT_OK)                       return err;
   if (chacha20_prng_read(out, 500, &st) != 500)                           return CRYPT_ERROR_READPRNG; /* skip 500 bytes */
   if (chacha20_prng_read(out, 10, &st) != 10)                             return CRYPT_ERROR_READPRNG; /* 10 bytes for testing */
   if (compare_testvector(out, 10, t3, sizeof(t3), "CHACHA-PRNG", 3))      return CRYPT_FAIL_TESTVECTOR;
   if ((err = chacha20_prng_done(&st)) != CRYPT_OK)                        return err;

   return CRYPT_OK;
#endif
}

#endif

/* ref:         $Format:%D$ */
/* git commit:  $Format:%H$ */
/* commit time: $Format:%ai$ */

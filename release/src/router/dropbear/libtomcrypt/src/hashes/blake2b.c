/* LibTomCrypt, modular cryptographic library -- Tom St Denis
 *
 * LibTomCrypt is a library that provides various cryptographic
 * algorithms in a highly modular and flexible manner.
 *
 * The library is free for all purposes without any express
 * guarantee it works.
 */

/*
   BLAKE2 reference source code package - reference C implementations

   Copyright 2012, Samuel Neves <sneves@dei.uc.pt>.  You may use this under the
   terms of the CC0, the OpenSSL Licence, or the Apache Public License 2.0, at
   your option.  The terms of these licenses can be found at:

   - CC0 1.0 Universal : http://creativecommons.org/publicdomain/zero/1.0
   - OpenSSL license   : https://www.openssl.org/source/license.html
   - Apache 2.0        : http://www.apache.org/licenses/LICENSE-2.0

   More information about the BLAKE2 hash function can be found at
   https://blake2.net.
*/
/* see also https://www.ietf.org/rfc/rfc7693.txt */

#include "tomcrypt.h"

#ifdef LTC_BLAKE2B

enum blake2b_constant {
   BLAKE2B_BLOCKBYTES = 128,
   BLAKE2B_OUTBYTES = 64,
   BLAKE2B_KEYBYTES = 64,
   BLAKE2B_SALTBYTES = 16,
   BLAKE2B_PERSONALBYTES = 16,
   BLAKE2B_PARAM_SIZE = 64
};

/* param offsets */
enum {
   O_DIGEST_LENGTH = 0,
   O_KEY_LENGTH = 1,
   O_FANOUT = 2,
   O_DEPTH = 3,
   O_LEAF_LENGTH = 4,
   O_NODE_OFFSET = 8,
   O_XOF_LENGTH = 12,
   O_NODE_DEPTH = 16,
   O_INNER_LENGTH = 17,
   O_RESERVED = 18,
   O_SALT = 32,
   O_PERSONAL = 48
};

/*
struct blake2b_param {
   unsigned char digest_length;
   unsigned char key_length;
   unsigned char fanout;
   unsigned char depth;
   ulong32 leaf_length;
   ulong32 node_offset;
   ulong32 xof_length;
   unsigned char node_depth;
   unsigned char inner_length;
   unsigned char reserved[14];
   unsigned char salt[BLAKE2B_SALTBYTES];
   unsigned char personal[BLAKE2B_PERSONALBYTES];
};
*/

const struct ltc_hash_descriptor blake2b_160_desc =
{
    "blake2b-160",
    25,
    20,
    128,
    { 1, 3, 6, 1, 4, 1, 1722, 12, 2, 1, 5 },
    11,
    &blake2b_160_init,
    &blake2b_process,
    &blake2b_done,
    &blake2b_160_test,
    NULL
};

const struct ltc_hash_descriptor blake2b_256_desc =
{
    "blake2b-256",
    26,
    32,
    128,
    { 1, 3, 6, 1, 4, 1, 1722, 12, 2, 1, 8 },
    11,
    &blake2b_256_init,
    &blake2b_process,
    &blake2b_done,
    &blake2b_256_test,
    NULL
};

const struct ltc_hash_descriptor blake2b_384_desc =
{
    "blake2b-384",
    27,
    48,
    128,
    { 1, 3, 6, 1, 4, 1, 1722, 12, 2, 1, 12 },
    11,
    &blake2b_384_init,
    &blake2b_process,
    &blake2b_done,
    &blake2b_384_test,
    NULL
};

const struct ltc_hash_descriptor blake2b_512_desc =
{
    "blake2b-512",
    28,
    64,
    128,
    { 1, 3, 6, 1, 4, 1, 1722, 12, 2, 1, 16 },
    11,
    &blake2b_512_init,
    &blake2b_process,
    &blake2b_done,
    &blake2b_512_test,
    NULL
};

static const ulong64 blake2b_IV[8] =
{
  CONST64(0x6a09e667f3bcc908), CONST64(0xbb67ae8584caa73b),
  CONST64(0x3c6ef372fe94f82b), CONST64(0xa54ff53a5f1d36f1),
  CONST64(0x510e527fade682d1), CONST64(0x9b05688c2b3e6c1f),
  CONST64(0x1f83d9abfb41bd6b), CONST64(0x5be0cd19137e2179)
};

static const unsigned char blake2b_sigma[12][16] =
{
  {  0,  1,  2,  3,  4,  5,  6,  7,  8,  9, 10, 11, 12, 13, 14, 15 } ,
  { 14, 10,  4,  8,  9, 15, 13,  6,  1, 12,  0,  2, 11,  7,  5,  3 } ,
  { 11,  8, 12,  0,  5,  2, 15, 13, 10, 14,  3,  6,  7,  1,  9,  4 } ,
  {  7,  9,  3,  1, 13, 12, 11, 14,  2,  6,  5, 10,  4,  0, 15,  8 } ,
  {  9,  0,  5,  7,  2,  4, 10, 15, 14,  1, 11, 12,  6,  8,  3, 13 } ,
  {  2, 12,  6, 10,  0, 11,  8,  3,  4, 13,  7,  5, 15, 14,  1,  9 } ,
  { 12,  5,  1, 15, 14, 13,  4, 10,  0,  7,  6,  3,  9,  2,  8, 11 } ,
  { 13, 11,  7, 14, 12,  1,  3,  9,  5,  0, 15,  4,  8,  6,  2, 10 } ,
  {  6, 15, 14,  9, 11,  3,  0,  8, 12,  2, 13,  7,  1,  4, 10,  5 } ,
  { 10,  2,  8,  4,  7,  6,  1,  5, 15, 11,  9, 14,  3, 12, 13 , 0 } ,
  {  0,  1,  2,  3,  4,  5,  6,  7,  8,  9, 10, 11, 12, 13, 14, 15 } ,
  { 14, 10,  4,  8,  9, 15, 13,  6,  1, 12,  0,  2, 11,  7,  5,  3 }
};

static void blake2b_set_lastnode(hash_state *md) { md->blake2b.f[1] = CONST64(0xffffffffffffffff); }

/* Some helper functions, not necessarily useful */
static int blake2b_is_lastblock(const hash_state *md) { return md->blake2b.f[0] != 0; }

static void blake2b_set_lastblock(hash_state *md)
{
   if (md->blake2b.last_node)
      blake2b_set_lastnode(md);

   md->blake2b.f[0] = CONST64(0xffffffffffffffff);
}

static void blake2b_increment_counter(hash_state *md, ulong64 inc)
{
   md->blake2b.t[0] += inc;
   if (md->blake2b.t[0] < inc) md->blake2b.t[1]++;
}

static void blake2b_init0(hash_state *md)
{
   unsigned long i;
   XMEMSET(&md->blake2b, 0, sizeof(md->blake2b));

   for (i = 0; i < 8; ++i)
      md->blake2b.h[i] = blake2b_IV[i];
}

/* init xors IV with input parameter block */
static int blake2b_init_param(hash_state *md, const unsigned char *P)
{
   unsigned long i;

   blake2b_init0(md);

   /* IV XOR ParamBlock */
   for (i = 0; i < 8; ++i) {
      ulong64 tmp;
      LOAD64L(tmp, P + i * 8);
      md->blake2b.h[i] ^= tmp;
   }

   md->blake2b.outlen = P[O_DIGEST_LENGTH];
   return CRYPT_OK;
}

int blake2b_init(hash_state *md, unsigned long outlen, const unsigned char *key, unsigned long keylen)
{
   unsigned char P[BLAKE2B_PARAM_SIZE];
   int err;

   LTC_ARGCHK(md != NULL);

   if ((!outlen) || (outlen > BLAKE2B_OUTBYTES))
      return CRYPT_INVALID_ARG;

   if ((key && !keylen) || (keylen && !key) || (keylen > BLAKE2B_KEYBYTES))
      return CRYPT_INVALID_ARG;

   XMEMSET(P, 0, sizeof(P));

   P[O_DIGEST_LENGTH] = (unsigned char)outlen;
   P[O_KEY_LENGTH] = (unsigned char)keylen;
   P[O_FANOUT] = 1;
   P[O_DEPTH] = 1;

   err = blake2b_init_param(md, P);
   if (err != CRYPT_OK) return err;

   if (key) {
      unsigned char block[BLAKE2B_BLOCKBYTES];

      XMEMSET(block, 0, BLAKE2B_BLOCKBYTES);
      XMEMCPY(block, key, keylen);
      blake2b_process(md, block, BLAKE2B_BLOCKBYTES);

#ifdef LTC_CLEAN_STACK
      zeromem(block, sizeof(block));
#endif
   }

   return CRYPT_OK;
}

int blake2b_160_init(hash_state *md) { return blake2b_init(md, 20, NULL, 0); }

int blake2b_256_init(hash_state *md) { return blake2b_init(md, 32, NULL, 0); }

int blake2b_384_init(hash_state *md) { return blake2b_init(md, 48, NULL, 0); }

int blake2b_512_init(hash_state *md) { return blake2b_init(md, 64, NULL, 0); }

#define G(r, i, a, b, c, d)                                                                                            \
   do {                                                                                                                \
      a = a + b + m[blake2b_sigma[r][2 * i + 0]];                                                                      \
      d = ROR64(d ^ a, 32);                                                                                            \
      c = c + d;                                                                                                       \
      b = ROR64(b ^ c, 24);                                                                                            \
      a = a + b + m[blake2b_sigma[r][2 * i + 1]];                                                                      \
      d = ROR64(d ^ a, 16);                                                                                            \
      c = c + d;                                                                                                       \
      b = ROR64(b ^ c, 63);                                                                                            \
   } while (0)

#define ROUND(r)                                                                                                       \
   do {                                                                                                                \
      G(r, 0, v[0], v[4], v[8], v[12]);                                                                                \
      G(r, 1, v[1], v[5], v[9], v[13]);                                                                                \
      G(r, 2, v[2], v[6], v[10], v[14]);                                                                               \
      G(r, 3, v[3], v[7], v[11], v[15]);                                                                               \
      G(r, 4, v[0], v[5], v[10], v[15]);                                                                               \
      G(r, 5, v[1], v[6], v[11], v[12]);                                                                               \
      G(r, 6, v[2], v[7], v[8], v[13]);                                                                                \
      G(r, 7, v[3], v[4], v[9], v[14]);                                                                                \
   } while (0)

#ifdef LTC_CLEAN_STACK
static int _blake2b_compress(hash_state *md, const unsigned char *buf)
#else
static int blake2b_compress(hash_state *md, const unsigned char *buf)
#endif
{
   ulong64 m[16];
   ulong64 v[16];
   unsigned long i;

   for (i = 0; i < 16; ++i) {
      LOAD64L(m[i], buf + i * sizeof(m[i]));
   }

   for (i = 0; i < 8; ++i) {
      v[i] = md->blake2b.h[i];
   }

   v[8] = blake2b_IV[0];
   v[9] = blake2b_IV[1];
   v[10] = blake2b_IV[2];
   v[11] = blake2b_IV[3];
   v[12] = blake2b_IV[4] ^ md->blake2b.t[0];
   v[13] = blake2b_IV[5] ^ md->blake2b.t[1];
   v[14] = blake2b_IV[6] ^ md->blake2b.f[0];
   v[15] = blake2b_IV[7] ^ md->blake2b.f[1];

   ROUND(0);
   ROUND(1);
   ROUND(2);
   ROUND(3);
   ROUND(4);
   ROUND(5);
   ROUND(6);
   ROUND(7);
   ROUND(8);
   ROUND(9);
   ROUND(10);
   ROUND(11);

   for (i = 0; i < 8; ++i) {
      md->blake2b.h[i] = md->blake2b.h[i] ^ v[i] ^ v[i + 8];
   }
   return CRYPT_OK;
}

#undef G
#undef ROUND

#ifdef LTC_CLEAN_STACK
static int blake2b_compress(hash_state *md, const unsigned char *buf)
{
   int err;
   err = _blake2b_compress(md, buf);
   burn_stack(sizeof(ulong64) * 32 + sizeof(unsigned long));
   return err;
}
#endif

int blake2b_process(hash_state *md, const unsigned char *in, unsigned long inlen)
{
   LTC_ARGCHK(md != NULL);
   LTC_ARGCHK(in != NULL);

   if (md->blake2b.curlen > sizeof(md->blake2b.buf)) {
      return CRYPT_INVALID_ARG;
   }

   if (inlen > 0) {
      unsigned long left = md->blake2b.curlen;
      unsigned long fill = BLAKE2B_BLOCKBYTES - left;
      if (inlen > fill) {
         md->blake2b.curlen = 0;
         XMEMCPY(md->blake2b.buf + (left % sizeof(md->blake2b.buf)), in, fill); /* Fill buffer */
         blake2b_increment_counter(md, BLAKE2B_BLOCKBYTES);
         blake2b_compress(md, md->blake2b.buf); /* Compress */
         in += fill;
         inlen -= fill;
         while (inlen > BLAKE2B_BLOCKBYTES) {
            blake2b_increment_counter(md, BLAKE2B_BLOCKBYTES);
            blake2b_compress(md, in);
            in += BLAKE2B_BLOCKBYTES;
            inlen -= BLAKE2B_BLOCKBYTES;
         }
      }
      XMEMCPY(md->blake2b.buf + md->blake2b.curlen, in, inlen);
      md->blake2b.curlen += inlen;
   }
   return CRYPT_OK;
}

int blake2b_done(hash_state *md, unsigned char *out)
{
   unsigned char buffer[BLAKE2B_OUTBYTES] = { 0 };
   unsigned long i;

   LTC_ARGCHK(md != NULL);
   LTC_ARGCHK(out != NULL);

   /* if(md->blakebs.outlen != outlen) return CRYPT_INVALID_ARG; */

   if (blake2b_is_lastblock(md))
      return CRYPT_ERROR;

   blake2b_increment_counter(md, md->blake2b.curlen);
   blake2b_set_lastblock(md);
   XMEMSET(md->blake2b.buf + md->blake2b.curlen, 0, BLAKE2B_BLOCKBYTES - md->blake2b.curlen); /* Padding */
   blake2b_compress(md, md->blake2b.buf);

   for (i = 0; i < 8; ++i) /* Output full hash to temp buffer */
      STORE64L(md->blake2b.h[i], buffer + i * 8);

   XMEMCPY(out, buffer, md->blake2b.outlen);
   zeromem(md, sizeof(hash_state));
#ifdef LTC_CLEAN_STACK
   zeromem(buffer, sizeof(buffer));
#endif
   return CRYPT_OK;
}

/**
  Self-test the hash
  @return CRYPT_OK if successful, CRYPT_NOP if self-tests have been disabled
*/
int blake2b_512_test(void)
{
#ifndef LTC_TEST
   return CRYPT_NOP;
#else
   static const struct {
      const char *msg;
      unsigned char hash[64];
  } tests[] = {
    { "",
      { 0x78, 0x6a, 0x02, 0xf7, 0x42, 0x01, 0x59, 0x03,
        0xc6, 0xc6, 0xfd, 0x85, 0x25, 0x52, 0xd2, 0x72,
        0x91, 0x2f, 0x47, 0x40, 0xe1, 0x58, 0x47, 0x61,
        0x8a, 0x86, 0xe2, 0x17, 0xf7, 0x1f, 0x54, 0x19,
        0xd2, 0x5e, 0x10, 0x31, 0xaf, 0xee, 0x58, 0x53,
        0x13, 0x89, 0x64, 0x44, 0x93, 0x4e, 0xb0, 0x4b,
        0x90, 0x3a, 0x68, 0x5b, 0x14, 0x48, 0xb7, 0x55,
        0xd5, 0x6f, 0x70, 0x1a, 0xfe, 0x9b, 0xe2, 0xce } },
    { "abc",
      { 0xba, 0x80, 0xa5, 0x3f, 0x98, 0x1c, 0x4d, 0x0d,
        0x6a, 0x27, 0x97, 0xb6, 0x9f, 0x12, 0xf6, 0xe9,
        0x4c, 0x21, 0x2f, 0x14, 0x68, 0x5a, 0xc4, 0xb7,
        0x4b, 0x12, 0xbb, 0x6f, 0xdb, 0xff, 0xa2, 0xd1,
        0x7d, 0x87, 0xc5, 0x39, 0x2a, 0xab, 0x79, 0x2d,
        0xc2, 0x52, 0xd5, 0xde, 0x45, 0x33, 0xcc, 0x95,
        0x18, 0xd3, 0x8a, 0xa8, 0xdb, 0xf1, 0x92, 0x5a,
        0xb9, 0x23, 0x86, 0xed, 0xd4, 0x00, 0x99, 0x23 } },

    { NULL, { 0 } }
  };

   int i;
   unsigned char tmp[64];
   hash_state md;

   for (i = 0; tests[i].msg != NULL; i++) {
      blake2b_512_init(&md);
      blake2b_process(&md, (unsigned char *)tests[i].msg, (unsigned long)strlen(tests[i].msg));
      blake2b_done(&md, tmp);
      if (compare_testvector(tmp, sizeof(tmp), tests[i].hash, sizeof(tests[i].hash), "BLAKE2B_512", i)) {
         return CRYPT_FAIL_TESTVECTOR;
      }
   }
   return CRYPT_OK;
#endif
}

/**
  Self-test the hash
  @return CRYPT_OK if successful, CRYPT_NOP if self-tests have been disabled
*/
int blake2b_384_test(void)
{
#ifndef LTC_TEST
   return CRYPT_NOP;
#else
   static const struct {
      const char *msg;
      unsigned char hash[48];
  } tests[] = {
    { "",
      { 0xb3, 0x28, 0x11, 0x42, 0x33, 0x77, 0xf5, 0x2d,
        0x78, 0x62, 0x28, 0x6e, 0xe1, 0xa7, 0x2e, 0xe5,
        0x40, 0x52, 0x43, 0x80, 0xfd, 0xa1, 0x72, 0x4a,
        0x6f, 0x25, 0xd7, 0x97, 0x8c, 0x6f, 0xd3, 0x24,
        0x4a, 0x6c, 0xaf, 0x04, 0x98, 0x81, 0x26, 0x73,
        0xc5, 0xe0, 0x5e, 0xf5, 0x83, 0x82, 0x51, 0x00 } },
    { "abc",
      { 0x6f, 0x56, 0xa8, 0x2c, 0x8e, 0x7e, 0xf5, 0x26,
        0xdf, 0xe1, 0x82, 0xeb, 0x52, 0x12, 0xf7, 0xdb,
        0x9d, 0xf1, 0x31, 0x7e, 0x57, 0x81, 0x5d, 0xbd,
        0xa4, 0x60, 0x83, 0xfc, 0x30, 0xf5, 0x4e, 0xe6,
        0xc6, 0x6b, 0xa8, 0x3b, 0xe6, 0x4b, 0x30, 0x2d,
        0x7c, 0xba, 0x6c, 0xe1, 0x5b, 0xb5, 0x56, 0xf4 } },

    { NULL, { 0 } }
  };

   int i;
   unsigned char tmp[48];
   hash_state md;

   for (i = 0; tests[i].msg != NULL; i++) {
      blake2b_384_init(&md);
      blake2b_process(&md, (unsigned char *)tests[i].msg, (unsigned long)strlen(tests[i].msg));
      blake2b_done(&md, tmp);
      if (compare_testvector(tmp, sizeof(tmp), tests[i].hash, sizeof(tests[i].hash), "BLAKE2B_384", i)) {
         return CRYPT_FAIL_TESTVECTOR;
      }
   }
   return CRYPT_OK;
#endif
}

/**
  Self-test the hash
  @return CRYPT_OK if successful, CRYPT_NOP if self-tests have been disabled
*/
int blake2b_256_test(void)
{
#ifndef LTC_TEST
   return CRYPT_NOP;
#else
   static const struct {
      const char *msg;
      unsigned char hash[32];
  } tests[] = {
    { "",
      { 0x0e, 0x57, 0x51, 0xc0, 0x26, 0xe5, 0x43, 0xb2,
        0xe8, 0xab, 0x2e, 0xb0, 0x60, 0x99, 0xda, 0xa1,
        0xd1, 0xe5, 0xdf, 0x47, 0x77, 0x8f, 0x77, 0x87,
        0xfa, 0xab, 0x45, 0xcd, 0xf1, 0x2f, 0xe3, 0xa8 } },
    { "abc",
      { 0xbd, 0xdd, 0x81, 0x3c, 0x63, 0x42, 0x39, 0x72,
        0x31, 0x71, 0xef, 0x3f, 0xee, 0x98, 0x57, 0x9b,
        0x94, 0x96, 0x4e, 0x3b, 0xb1, 0xcb, 0x3e, 0x42,
        0x72, 0x62, 0xc8, 0xc0, 0x68, 0xd5, 0x23, 0x19 } },
    { "12345678901234567890123456789012345678901234567890"
      "12345678901234567890123456789012345678901234567890"
      "12345678901234567890123456789012345678901234567890"
      "12345678901234567890123456789012345678901234567890"
      "12345678901234567890123456789012345678901234567890"
      "12345678901234567890123456789012345678901234567890",
      { 0x0f, 0x6e, 0x01, 0x8d, 0x38, 0xd6, 0x3f, 0x08,
        0x4d, 0x58, 0xe3, 0x0c, 0x90, 0xfb, 0xa2, 0x41,
        0x5f, 0xca, 0x17, 0xfa, 0x66, 0x26, 0x49, 0xf3,
        0x8a, 0x30, 0x41, 0x7c, 0x57, 0xcd, 0xa8, 0x14 } },

    { NULL, { 0 } }
  };

   int i;
   unsigned char tmp[32];
   hash_state md;

   for (i = 0; tests[i].msg != NULL; i++) {
      blake2b_256_init(&md);
      blake2b_process(&md, (unsigned char *)tests[i].msg, (unsigned long)strlen(tests[i].msg));
      blake2b_done(&md, tmp);
      if (compare_testvector(tmp, sizeof(tmp), tests[i].hash, sizeof(tests[i].hash), "BLAKE2B_256", i)) {
         return CRYPT_FAIL_TESTVECTOR;
      }
   }
   return CRYPT_OK;
#endif
}

/**
  Self-test the hash
  @return CRYPT_OK if successful, CRYPT_NOP if self-tests have been disabled
*/
int blake2b_160_test(void)
{
#ifndef LTC_TEST
   return CRYPT_NOP;
#else
   static const struct {
      const char *msg;
      unsigned char hash[20];
  } tests[] = {
    { "",
      { 0x33, 0x45, 0x52, 0x4a, 0xbf, 0x6b, 0xbe, 0x18,
        0x09, 0x44, 0x92, 0x24, 0xb5, 0x97, 0x2c, 0x41,
        0x79, 0x0b, 0x6c, 0xf2 } },
    { "abc",
      { 0x38, 0x42, 0x64, 0xf6, 0x76, 0xf3, 0x95, 0x36,
        0x84, 0x05, 0x23, 0xf2, 0x84, 0x92, 0x1c, 0xdc,
        0x68, 0xb6, 0x84, 0x6b } },

    { NULL, { 0 } }
  };

   int i;
   unsigned char tmp[20];
   hash_state md;

   for (i = 0; tests[i].msg != NULL; i++) {
      blake2b_160_init(&md);
      blake2b_process(&md, (unsigned char *)tests[i].msg, (unsigned long)strlen(tests[i].msg));
      blake2b_done(&md, tmp);
      if (compare_testvector(tmp, sizeof(tmp), tests[i].hash, sizeof(tests[i].hash), "BLAKE2B_160", i)) {
         return CRYPT_FAIL_TESTVECTOR;
      }
   }
   return CRYPT_OK;
#endif
}

#endif

/* ref:         $Format:%D$ */
/* git commit:  $Format:%H$ */
/* commit time: $Format:%ai$ */

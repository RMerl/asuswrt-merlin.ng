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

#ifdef LTC_BLAKE2S

enum blake2s_constant {
   BLAKE2S_BLOCKBYTES = 64,
   BLAKE2S_OUTBYTES = 32,
   BLAKE2S_KEYBYTES = 32,
   BLAKE2S_SALTBYTES = 8,
   BLAKE2S_PERSONALBYTES = 8,
   BLAKE2S_PARAM_SIZE = 32
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
   O_NODE_DEPTH = 14,
   O_INNER_LENGTH = 15,
   O_SALT = 16,
   O_PERSONAL = 24
};

/*
struct blake2s_param {
   unsigned char digest_length;
   unsigned char key_length;
   unsigned char fanout;
   unsigned char depth;
   ulong32 leaf_length;
   ulong32 node_offset;
   ushort16 xof_length;
   unsigned char node_depth;
   unsigned char inner_length;
   unsigned char salt[BLAKE2S_SALTBYTES];
   unsigned char personal[BLAKE2S_PERSONALBYTES];
};
*/

const struct ltc_hash_descriptor blake2s_128_desc =
{
    "blake2s-128",
    21,
    16,
    64,
    { 1, 3, 6, 1, 4, 1, 1722, 12, 2, 2, 4 },
    11,
    &blake2s_128_init,
    &blake2s_process,
    &blake2s_done,
    &blake2s_128_test,
    NULL
};

const struct ltc_hash_descriptor blake2s_160_desc =
{
    "blake2s-160",
    22,
    20,
    64,
    { 1, 3, 6, 1, 4, 1, 1722, 12, 2, 2, 5 },
    11,
    &blake2s_160_init,
    &blake2s_process,
    &blake2s_done,
    &blake2s_160_test,
    NULL
};

const struct ltc_hash_descriptor blake2s_224_desc =
{
    "blake2s-224",
    23,
    28,
    64,
    { 1, 3, 6, 1, 4, 1, 1722, 12, 2, 2, 7 },
    11,
    &blake2s_224_init,
    &blake2s_process,
    &blake2s_done,
    &blake2s_224_test,
    NULL
};

const struct ltc_hash_descriptor blake2s_256_desc =
{
    "blake2s-256",
    24,
    32,
    64,
    { 1, 3, 6, 1, 4, 1, 1722, 12, 2, 2, 8 },
    11,
    &blake2s_256_init,
    &blake2s_process,
    &blake2s_done,
    &blake2s_256_test,
    NULL
};

static const ulong32 blake2s_IV[8] = {
    0x6A09E667UL, 0xBB67AE85UL, 0x3C6EF372UL, 0xA54FF53AUL,
    0x510E527FUL, 0x9B05688CUL, 0x1F83D9ABUL, 0x5BE0CD19UL
};

static const unsigned char blake2s_sigma[10][16] = {
    { 0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15 },
    { 14, 10, 4, 8, 9, 15, 13, 6, 1, 12, 0, 2, 11, 7, 5, 3 },
    { 11, 8, 12, 0, 5, 2, 15, 13, 10, 14, 3, 6, 7, 1, 9, 4 },
    { 7, 9, 3, 1, 13, 12, 11, 14, 2, 6, 5, 10, 4, 0, 15, 8 },
    { 9, 0, 5, 7, 2, 4, 10, 15, 14, 1, 11, 12, 6, 8, 3, 13 },
    { 2, 12, 6, 10, 0, 11, 8, 3, 4, 13, 7, 5, 15, 14, 1, 9 },
    { 12, 5, 1, 15, 14, 13, 4, 10, 0, 7, 6, 3, 9, 2, 8, 11 },
    { 13, 11, 7, 14, 12, 1, 3, 9, 5, 0, 15, 4, 8, 6, 2, 10 },
    { 6, 15, 14, 9, 11, 3, 0, 8, 12, 2, 13, 7, 1, 4, 10, 5 },
    { 10, 2, 8, 4, 7, 6, 1, 5, 15, 11, 9, 14, 3, 12, 13, 0 },
};

static void blake2s_set_lastnode(hash_state *md) { md->blake2s.f[1] = 0xffffffffUL; }

/* Some helper functions, not necessarily useful */
static int blake2s_is_lastblock(const hash_state *md) { return md->blake2s.f[0] != 0; }

static void blake2s_set_lastblock(hash_state *md)
{
   if (md->blake2s.last_node)
      blake2s_set_lastnode(md);

   md->blake2s.f[0] = 0xffffffffUL;
}

static void blake2s_increment_counter(hash_state *md, const ulong32 inc)
{
   md->blake2s.t[0] += inc;
   if (md->blake2s.t[0] < inc) md->blake2s.t[1]++;
}

static int blake2s_init0(hash_state *md)
{
   int i;
   XMEMSET(&md->blake2s, 0, sizeof(struct blake2s_state));

   for (i = 0; i < 8; ++i)
      md->blake2s.h[i] = blake2s_IV[i];

   return CRYPT_OK;
}

/* init2 xors IV with input parameter block */
static int blake2s_init_param(hash_state *md, const unsigned char *P)
{
   unsigned long i;

   blake2s_init0(md);

   /* IV XOR ParamBlock */
   for (i = 0; i < 8; ++i) {
      ulong32 tmp;
      LOAD32L(tmp, P + i * 4);
      md->blake2s.h[i] ^= tmp;
   }

   md->blake2s.outlen = P[O_DIGEST_LENGTH];
   return CRYPT_OK;
}

int blake2s_init(hash_state *md, unsigned long outlen, const unsigned char *key, unsigned long keylen)
{
   unsigned char P[BLAKE2S_PARAM_SIZE];
   int err;

   LTC_ARGCHK(md != NULL);

   if ((!outlen) || (outlen > BLAKE2S_OUTBYTES))
      return CRYPT_INVALID_ARG;

   if ((key && !keylen) || (keylen && !key) || (keylen > BLAKE2S_KEYBYTES))
      return CRYPT_INVALID_ARG;

   XMEMSET(P, 0, sizeof(P));

   P[O_DIGEST_LENGTH] = (unsigned char)outlen;
   P[O_KEY_LENGTH] = (unsigned char)keylen;
   P[O_FANOUT] = 1;
   P[O_DEPTH] = 1;

   err = blake2s_init_param(md, P);
   if (err != CRYPT_OK) return err;

   if (key) {
      unsigned char block[BLAKE2S_BLOCKBYTES];

      XMEMSET(block, 0, BLAKE2S_BLOCKBYTES);
      XMEMCPY(block, key, keylen);
      blake2s_process(md, block, BLAKE2S_BLOCKBYTES);

#ifdef LTC_CLEAN_STACK
      zeromem(block, sizeof(block));
#endif
   }
   return CRYPT_OK;
}

int blake2s_128_init(hash_state *md) { return blake2s_init(md, 16, NULL, 0); }

int blake2s_160_init(hash_state *md) { return blake2s_init(md, 20, NULL, 0); }

int blake2s_224_init(hash_state *md) { return blake2s_init(md, 28, NULL, 0); }

int blake2s_256_init(hash_state *md) { return blake2s_init(md, 32, NULL, 0); }

#define G(r, i, a, b, c, d)                                                                                            \
   do {                                                                                                                \
      a = a + b + m[blake2s_sigma[r][2 * i + 0]];                                                                      \
      d = ROR(d ^ a, 16);                                                                                              \
      c = c + d;                                                                                                       \
      b = ROR(b ^ c, 12);                                                                                              \
      a = a + b + m[blake2s_sigma[r][2 * i + 1]];                                                                      \
      d = ROR(d ^ a, 8);                                                                                               \
      c = c + d;                                                                                                       \
      b = ROR(b ^ c, 7);                                                                                               \
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
static int _blake2s_compress(hash_state *md, const unsigned char *buf)
#else
static int blake2s_compress(hash_state *md, const unsigned char *buf)
#endif
{
   unsigned long i;
   ulong32 m[16];
   ulong32 v[16];

   for (i = 0; i < 16; ++i) {
      LOAD32L(m[i], buf + i * sizeof(m[i]));
   }

   for (i = 0; i < 8; ++i)
      v[i] = md->blake2s.h[i];

   v[8] = blake2s_IV[0];
   v[9] = blake2s_IV[1];
   v[10] = blake2s_IV[2];
   v[11] = blake2s_IV[3];
   v[12] = md->blake2s.t[0] ^ blake2s_IV[4];
   v[13] = md->blake2s.t[1] ^ blake2s_IV[5];
   v[14] = md->blake2s.f[0] ^ blake2s_IV[6];
   v[15] = md->blake2s.f[1] ^ blake2s_IV[7];

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

   for (i = 0; i < 8; ++i)
      md->blake2s.h[i] = md->blake2s.h[i] ^ v[i] ^ v[i + 8];

   return CRYPT_OK;
}
#undef G
#undef ROUND

#ifdef LTC_CLEAN_STACK
static int blake2s_compress(hash_state *md, const unsigned char *buf)
{
   int err;
   err = _blake2s_compress(md, buf);
   burn_stack(sizeof(ulong32) * (32) + sizeof(unsigned long));
   return err;
}
#endif

int blake2s_process(hash_state *md, const unsigned char *in, unsigned long inlen)
{
   LTC_ARGCHK(md != NULL);
   LTC_ARGCHK(in != NULL);

   if (md->blake2s.curlen > sizeof(md->blake2s.buf)) {
      return CRYPT_INVALID_ARG;
   }

   if (inlen > 0) {
      unsigned long left = md->blake2s.curlen;
      unsigned long fill = BLAKE2S_BLOCKBYTES - left;
      if (inlen > fill) {
         md->blake2s.curlen = 0;
         XMEMCPY(md->blake2s.buf + (left % sizeof(md->blake2s.buf)), in, fill); /* Fill buffer */
         blake2s_increment_counter(md, BLAKE2S_BLOCKBYTES);
         blake2s_compress(md, md->blake2s.buf); /* Compress */
         in += fill;
         inlen -= fill;
         while (inlen > BLAKE2S_BLOCKBYTES) {
            blake2s_increment_counter(md, BLAKE2S_BLOCKBYTES);
            blake2s_compress(md, in);
            in += BLAKE2S_BLOCKBYTES;
            inlen -= BLAKE2S_BLOCKBYTES;
         }
      }
      XMEMCPY(md->blake2s.buf + md->blake2s.curlen, in, inlen);
      md->blake2s.curlen += inlen;
   }
   return CRYPT_OK;
}

int blake2s_done(hash_state *md, unsigned char *out)
{
   unsigned char buffer[BLAKE2S_OUTBYTES] = { 0 };
   unsigned long i;

   LTC_ARGCHK(md != NULL);
   LTC_ARGCHK(out != NULL);

   /* if(md->blake2s.outlen != outlen) return CRYPT_INVALID_ARG; */

   if (blake2s_is_lastblock(md))
      return CRYPT_ERROR;

   blake2s_increment_counter(md, md->blake2s.curlen);
   blake2s_set_lastblock(md);
   XMEMSET(md->blake2s.buf + md->blake2s.curlen, 0, BLAKE2S_BLOCKBYTES - md->blake2s.curlen); /* Padding */
   blake2s_compress(md, md->blake2s.buf);

   for (i = 0; i < 8; ++i) /* Output full hash to temp buffer */
      STORE32L(md->blake2s.h[i], buffer + i * 4);

   XMEMCPY(out, buffer, md->blake2s.outlen);
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
int blake2s_256_test(void)
{
#ifndef LTC_TEST
   return CRYPT_NOP;
#else
   static const struct {
      const char *msg;
      unsigned char hash[32];
  } tests[] = {
    { "",
      { 0x69, 0x21, 0x7a, 0x30, 0x79, 0x90, 0x80, 0x94,
        0xe1, 0x11, 0x21, 0xd0, 0x42, 0x35, 0x4a, 0x7c,
        0x1f, 0x55, 0xb6, 0x48, 0x2c, 0xa1, 0xa5, 0x1e,
        0x1b, 0x25, 0x0d, 0xfd, 0x1e, 0xd0, 0xee, 0xf9 } },
    { "abc",
      { 0x50, 0x8c, 0x5e, 0x8c, 0x32, 0x7c, 0x14, 0xe2,
        0xe1, 0xa7, 0x2b, 0xa3, 0x4e, 0xeb, 0x45, 0x2f,
        0x37, 0x45, 0x8b, 0x20, 0x9e, 0xd6, 0x3a, 0x29,
        0x4d, 0x99, 0x9b, 0x4c, 0x86, 0x67, 0x59, 0x82 } },
    { "12345678901234567890123456789012345678901234567890"
      "12345678901234567890123456789012345678901234567890"
      "12345678901234567890123456789012345678901234567890"
      "12345678901234567890123456789012345678901234567890"
      "12345678901234567890123456789012345678901234567890"
      "12345678901234567890123456789012345678901234567890",
      { 0xa3, 0x78, 0x8b, 0x5b, 0x59, 0xee, 0xe4, 0x41,
        0x95, 0x23, 0x58, 0x00, 0xa4, 0xf9, 0xfa, 0x41,
        0x86, 0x0c, 0x7b, 0x1c, 0x35, 0xa2, 0x42, 0x70,
        0x50, 0x80, 0x79, 0x56, 0xe3, 0xbe, 0x31, 0x74 } },

    { NULL, { 0 } }
  };

   int i;
   unsigned char tmp[32];
   hash_state md;

   for (i = 0; tests[i].msg != NULL; i++) {
      blake2s_256_init(&md);
      blake2s_process(&md, (unsigned char *)tests[i].msg, (unsigned long)strlen(tests[i].msg));
      blake2s_done(&md, tmp);
      if (compare_testvector(tmp, sizeof(tmp), tests[i].hash, sizeof(tests[i].hash), "BLAKE2S_256", i)) {
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
int blake2s_224_test(void)
{
#ifndef LTC_TEST
   return CRYPT_NOP;
#else
   static const struct {
      const char *msg;
      unsigned char hash[28];
  } tests[] = {
    { "",
      { 0x1f, 0xa1, 0x29, 0x1e, 0x65, 0x24, 0x8b, 0x37,
        0xb3, 0x43, 0x34, 0x75, 0xb2, 0xa0, 0xdd, 0x63,
        0xd5, 0x4a, 0x11, 0xec, 0xc4, 0xe3, 0xe0, 0x34,
        0xe7, 0xbc, 0x1e, 0xf4 } },
    { "abc",
      { 0x0b, 0x03, 0x3f, 0xc2, 0x26, 0xdf, 0x7a, 0xbd,
        0xe2, 0x9f, 0x67, 0xa0, 0x5d, 0x3d, 0xc6, 0x2c,
        0xf2, 0x71, 0xef, 0x3d, 0xfe, 0xa4, 0xd3, 0x87,
        0x40, 0x7f, 0xbd, 0x55 } },

    { NULL, { 0 } }
  };

   int i;
   unsigned char tmp[28];
   hash_state md;

   for (i = 0; tests[i].msg != NULL; i++) {
      blake2s_224_init(&md);
      blake2s_process(&md, (unsigned char *)tests[i].msg, (unsigned long)strlen(tests[i].msg));
      blake2s_done(&md, tmp);
      if (compare_testvector(tmp, sizeof(tmp), tests[i].hash, sizeof(tests[i].hash), "BLAKE2S_224", i)) {
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
int blake2s_160_test(void)
{
#ifndef LTC_TEST
   return CRYPT_NOP;
#else
   static const struct {
      const char *msg;
      unsigned char hash[20];
  } tests[] = {
    { "",
      { 0x35, 0x4c, 0x9c, 0x33, 0xf7, 0x35, 0x96, 0x24,
        0x18, 0xbd, 0xac, 0xb9, 0x47, 0x98, 0x73, 0x42,
        0x9c, 0x34, 0x91, 0x6f} },
    { "abc",
      { 0x5a, 0xe3, 0xb9, 0x9b, 0xe2, 0x9b, 0x01, 0x83,
        0x4c, 0x3b, 0x50, 0x85, 0x21, 0xed, 0xe6, 0x04,
        0x38, 0xf8, 0xde, 0x17 } },

    { NULL, { 0 } }
  };

   int i;
   unsigned char tmp[20];
   hash_state md;

   for (i = 0; tests[i].msg != NULL; i++) {
      blake2s_160_init(&md);
      blake2s_process(&md, (unsigned char *)tests[i].msg, (unsigned long)strlen(tests[i].msg));
      blake2s_done(&md, tmp);
      if (compare_testvector(tmp, sizeof(tmp), tests[i].hash, sizeof(tests[i].hash), "BLAKE2S_160", i)) {
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
int blake2s_128_test(void)
{
#ifndef LTC_TEST
   return CRYPT_NOP;
#else
   static const struct {
      const char *msg;
      unsigned char hash[16];
  } tests[] = {
    { "",
      { 0x64, 0x55, 0x0d, 0x6f, 0xfe, 0x2c, 0x0a, 0x01,
        0xa1, 0x4a, 0xba, 0x1e, 0xad, 0xe0, 0x20, 0x0c } },
    { "abc",
      { 0xaa, 0x49, 0x38, 0x11, 0x9b, 0x1d, 0xc7, 0xb8,
        0x7c, 0xba, 0xd0, 0xff, 0xd2, 0x00, 0xd0, 0xae } },

    { NULL, { 0 } }
  };

   int i;
   unsigned char tmp[16];
   hash_state md;

   for (i = 0; tests[i].msg != NULL; i++) {
      blake2s_128_init(&md);
      blake2s_process(&md, (unsigned char *)tests[i].msg, (unsigned long)strlen(tests[i].msg));
      blake2s_done(&md, tmp);
      if (compare_testvector(tmp, sizeof(tmp), tests[i].hash, sizeof(tests[i].hash), "BLAKE2S_128", i)) {
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

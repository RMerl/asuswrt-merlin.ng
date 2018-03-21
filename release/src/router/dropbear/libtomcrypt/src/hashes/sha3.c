/* LibTomCrypt, modular cryptographic library -- Tom St Denis
 *
 * LibTomCrypt is a library that provides various cryptographic
 * algorithms in a highly modular and flexible manner.
 *
 * The library is free for all purposes without any express
 * guarantee it works.
 */

/* based on https://github.com/brainhub/SHA3IUF (public domain) */

#include "tomcrypt.h"

#ifdef LTC_SHA3

const struct ltc_hash_descriptor sha3_224_desc =
{
   "sha3-224",                  /* name of hash */
   17,                          /* internal ID */
   28,                          /* Size of digest in octets */
   144,                         /* Input block size in octets */
   { 2,16,840,1,101,3,4,2,7 },  /* ASN.1 OID */
   9,                           /* Length OID */
   &sha3_224_init,
   &sha3_process,
   &sha3_done,
   &sha3_224_test,
   NULL
};

const struct ltc_hash_descriptor sha3_256_desc =
{
   "sha3-256",                  /* name of hash */
   18,                          /* internal ID */
   32,                          /* Size of digest in octets */
   136,                         /* Input block size in octets */
   { 2,16,840,1,101,3,4,2,8 },  /* ASN.1 OID */
   9,                           /* Length OID */
   &sha3_256_init,
   &sha3_process,
   &sha3_done,
   &sha3_256_test,
   NULL
};

const struct ltc_hash_descriptor sha3_384_desc =
{
   "sha3-384",                  /* name of hash */
   19,                          /* internal ID */
   48,                          /* Size of digest in octets */
   104,                         /* Input block size in octets */
   { 2,16,840,1,101,3,4,2,9 },  /* ASN.1 OID */
   9,                           /* Length OID */
   &sha3_384_init,
   &sha3_process,
   &sha3_done,
   &sha3_384_test,
   NULL
};

const struct ltc_hash_descriptor sha3_512_desc =
{
   "sha3-512",                  /* name of hash */
   20,                          /* internal ID */
   64,                          /* Size of digest in octets */
   72,                          /* Input block size in octets */
   { 2,16,840,1,101,3,4,2,10 }, /* ASN.1 OID */
   9,                           /* Length OID */
   &sha3_512_init,
   &sha3_process,
   &sha3_done,
   &sha3_512_test,
   NULL
};

#define SHA3_KECCAK_SPONGE_WORDS 25 /* 1600 bits > 200 bytes > 25 x ulong64 */
#define SHA3_KECCAK_ROUNDS 24

static const ulong64 keccakf_rndc[24] = {
   CONST64(0x0000000000000001), CONST64(0x0000000000008082),
   CONST64(0x800000000000808a), CONST64(0x8000000080008000),
   CONST64(0x000000000000808b), CONST64(0x0000000080000001),
   CONST64(0x8000000080008081), CONST64(0x8000000000008009),
   CONST64(0x000000000000008a), CONST64(0x0000000000000088),
   CONST64(0x0000000080008009), CONST64(0x000000008000000a),
   CONST64(0x000000008000808b), CONST64(0x800000000000008b),
   CONST64(0x8000000000008089), CONST64(0x8000000000008003),
   CONST64(0x8000000000008002), CONST64(0x8000000000000080),
   CONST64(0x000000000000800a), CONST64(0x800000008000000a),
   CONST64(0x8000000080008081), CONST64(0x8000000000008080),
   CONST64(0x0000000080000001), CONST64(0x8000000080008008)
};

static const unsigned keccakf_rotc[24] = {
   1, 3, 6, 10, 15, 21, 28, 36, 45, 55, 2, 14, 27, 41, 56, 8, 25, 43, 62, 18, 39, 61, 20, 44
};

static const unsigned keccakf_piln[24] = {
   10, 7, 11, 17, 18, 3, 5, 16, 8, 21, 24, 4, 15, 23, 19, 13, 12, 2, 20, 14, 22, 9, 6, 1
};

static void keccakf(ulong64 s[25])
{
   int i, j, round;
   ulong64 t, bc[5];

   for(round = 0; round < SHA3_KECCAK_ROUNDS; round++) {
      /* Theta */
      for(i = 0; i < 5; i++)
         bc[i] = s[i] ^ s[i + 5] ^ s[i + 10] ^ s[i + 15] ^ s[i + 20];

      for(i = 0; i < 5; i++) {
         t = bc[(i + 4) % 5] ^ ROL64(bc[(i + 1) % 5], 1);
         for(j = 0; j < 25; j += 5)
            s[j + i] ^= t;
      }
      /* Rho Pi */
      t = s[1];
      for(i = 0; i < 24; i++) {
         j = keccakf_piln[i];
         bc[0] = s[j];
         s[j] = ROL64(t, keccakf_rotc[i]);
         t = bc[0];
      }
      /* Chi */
      for(j = 0; j < 25; j += 5) {
         for(i = 0; i < 5; i++)
            bc[i] = s[j + i];
         for(i = 0; i < 5; i++)
            s[j + i] ^= (~bc[(i + 1) % 5]) & bc[(i + 2) % 5];
      }
      /* Iota */
      s[0] ^= keccakf_rndc[round];
   }
}

/* Public Inteface */

int sha3_224_init(hash_state *md)
{
   LTC_ARGCHK(md != NULL);
   XMEMSET(&md->sha3, 0, sizeof(md->sha3));
   md->sha3.capacity_words = 2 * 224 / (8 * sizeof(ulong64));
   return CRYPT_OK;
}

int sha3_256_init(hash_state *md)
{
   LTC_ARGCHK(md != NULL);
   XMEMSET(&md->sha3, 0, sizeof(md->sha3));
   md->sha3.capacity_words = 2 * 256 / (8 * sizeof(ulong64));
   return CRYPT_OK;
}

int sha3_384_init(hash_state *md)
{
   LTC_ARGCHK(md != NULL);
   XMEMSET(&md->sha3, 0, sizeof(md->sha3));
   md->sha3.capacity_words = 2 * 384 / (8 * sizeof(ulong64));
   return CRYPT_OK;
}

int sha3_512_init(hash_state *md)
{
   LTC_ARGCHK(md != NULL);
   XMEMSET(&md->sha3, 0, sizeof(md->sha3));
   md->sha3.capacity_words = 2 * 512 / (8 * sizeof(ulong64));
   return CRYPT_OK;
}

int sha3_shake_init(hash_state *md, int num)
{
   LTC_ARGCHK(md != NULL);
   if (num != 128 && num != 256) return CRYPT_INVALID_ARG;
   XMEMSET(&md->sha3, 0, sizeof(md->sha3));
   md->sha3.capacity_words = (unsigned short)(2 * num / (8 * sizeof(ulong64)));
   return CRYPT_OK;
}

int sha3_process(hash_state *md, const unsigned char *in, unsigned long inlen)
{
   /* 0...7 -- how much is needed to have a word */
   unsigned old_tail = (8 - md->sha3.byte_index) & 7;

   unsigned long words;
   unsigned tail;
   unsigned long i;

   if (inlen == 0) return CRYPT_OK; /* nothing to do */
   LTC_ARGCHK(md != NULL);
   LTC_ARGCHK(in != NULL);

   if(inlen < old_tail) {       /* have no complete word or haven't started the word yet */
      while (inlen--) md->sha3.saved |= (ulong64) (*(in++)) << ((md->sha3.byte_index++) * 8);
      return CRYPT_OK;
   }

   if(old_tail) {               /* will have one word to process */
      inlen -= old_tail;
      while (old_tail--) md->sha3.saved |= (ulong64) (*(in++)) << ((md->sha3.byte_index++) * 8);
      /* now ready to add saved to the sponge */
      md->sha3.s[md->sha3.word_index] ^= md->sha3.saved;
      md->sha3.byte_index = 0;
      md->sha3.saved = 0;
      if(++md->sha3.word_index == (SHA3_KECCAK_SPONGE_WORDS - md->sha3.capacity_words)) {
         keccakf(md->sha3.s);
         md->sha3.word_index = 0;
      }
   }

   /* now work in full words directly from input */
   words = inlen / sizeof(ulong64);
   tail = inlen - words * sizeof(ulong64);

   for(i = 0; i < words; i++, in += sizeof(ulong64)) {
      ulong64 t;
      LOAD64L(t, in);
      md->sha3.s[md->sha3.word_index] ^= t;
      if(++md->sha3.word_index == (SHA3_KECCAK_SPONGE_WORDS - md->sha3.capacity_words)) {
         keccakf(md->sha3.s);
         md->sha3.word_index = 0;
      }
   }

   /* finally, save the partial word */
   while (tail--) {
      md->sha3.saved |= (ulong64) (*(in++)) << ((md->sha3.byte_index++) * 8);
   }
   return CRYPT_OK;
}

int sha3_done(hash_state *md, unsigned char *hash)
{
   unsigned i;

   LTC_ARGCHK(md   != NULL);
   LTC_ARGCHK(hash != NULL);

   md->sha3.s[md->sha3.word_index] ^= (md->sha3.saved ^ (CONST64(0x06) << (md->sha3.byte_index * 8)));
   md->sha3.s[SHA3_KECCAK_SPONGE_WORDS - md->sha3.capacity_words - 1] ^= CONST64(0x8000000000000000);
   keccakf(md->sha3.s);

   /* store sha3.s[] as little-endian bytes into sha3.sb */
   for(i = 0; i < SHA3_KECCAK_SPONGE_WORDS; i++) {
      STORE64L(md->sha3.s[i], md->sha3.sb + i * 8);
   }

   XMEMCPY(hash, md->sha3.sb, md->sha3.capacity_words * 4);
   return CRYPT_OK;
}

int sha3_shake_done(hash_state *md, unsigned char *out, unsigned long outlen)
{
   /* IMPORTANT NOTE: sha3_shake_done can be called many times */
   unsigned long idx;
   unsigned i;

   if (outlen == 0) return CRYPT_OK; /* nothing to do */
   LTC_ARGCHK(md  != NULL);
   LTC_ARGCHK(out != NULL);

   if (!md->sha3.xof_flag) {
      /* shake_xof operation must be done only once */
      md->sha3.s[md->sha3.word_index] ^= (md->sha3.saved ^ (CONST64(0x1F) << (md->sha3.byte_index * 8)));
      md->sha3.s[SHA3_KECCAK_SPONGE_WORDS - md->sha3.capacity_words - 1] ^= CONST64(0x8000000000000000);
      keccakf(md->sha3.s);
      /* store sha3.s[] as little-endian bytes into sha3.sb */
      for(i = 0; i < SHA3_KECCAK_SPONGE_WORDS; i++) {
         STORE64L(md->sha3.s[i], md->sha3.sb + i * 8);
      }
      md->sha3.byte_index = 0;
      md->sha3.xof_flag = 1;
   }

   for (idx = 0; idx < outlen; idx++) {
      if(md->sha3.byte_index >= (SHA3_KECCAK_SPONGE_WORDS - md->sha3.capacity_words) * 8) {
         keccakf(md->sha3.s);
         /* store sha3.s[] as little-endian bytes into sha3.sb */
         for(i = 0; i < SHA3_KECCAK_SPONGE_WORDS; i++) {
            STORE64L(md->sha3.s[i], md->sha3.sb + i * 8);
         }
         md->sha3.byte_index = 0;
      }
      out[idx] = md->sha3.sb[md->sha3.byte_index++];
   }
   return CRYPT_OK;
}

int sha3_shake_memory(int num, const unsigned char *in, unsigned long inlen, unsigned char *out, unsigned long *outlen)
{
   hash_state md;
   int err;
   LTC_ARGCHK(in  != NULL);
   LTC_ARGCHK(out != NULL);
   LTC_ARGCHK(outlen != NULL);
   if ((err = sha3_shake_init(&md, num))          != CRYPT_OK) return err;
   if ((err = sha3_shake_process(&md, in, inlen)) != CRYPT_OK) return err;
   if ((err = sha3_shake_done(&md, out, *outlen)) != CRYPT_OK) return err;
   return CRYPT_OK;
}

#endif

/* ref:         $Format:%D$ */
/* git commit:  $Format:%H$ */
/* commit time: $Format:%ai$ */

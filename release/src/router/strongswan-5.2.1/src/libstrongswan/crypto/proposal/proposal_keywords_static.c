/* C code produced by gperf version 3.0.4 */
/* Command-line: /usr/bin/gperf -N proposal_get_token_static -m 10 -C -G -c -t -D  */
/* Computed positions: -k'1,5-7,10,15,$' */

#if !((' ' == 32) && ('!' == 33) && ('"' == 34) && ('#' == 35) \
      && ('%' == 37) && ('&' == 38) && ('\'' == 39) && ('(' == 40) \
      && (')' == 41) && ('*' == 42) && ('+' == 43) && (',' == 44) \
      && ('-' == 45) && ('.' == 46) && ('/' == 47) && ('0' == 48) \
      && ('1' == 49) && ('2' == 50) && ('3' == 51) && ('4' == 52) \
      && ('5' == 53) && ('6' == 54) && ('7' == 55) && ('8' == 56) \
      && ('9' == 57) && (':' == 58) && (';' == 59) && ('<' == 60) \
      && ('=' == 61) && ('>' == 62) && ('?' == 63) && ('A' == 65) \
      && ('B' == 66) && ('C' == 67) && ('D' == 68) && ('E' == 69) \
      && ('F' == 70) && ('G' == 71) && ('H' == 72) && ('I' == 73) \
      && ('J' == 74) && ('K' == 75) && ('L' == 76) && ('M' == 77) \
      && ('N' == 78) && ('O' == 79) && ('P' == 80) && ('Q' == 81) \
      && ('R' == 82) && ('S' == 83) && ('T' == 84) && ('U' == 85) \
      && ('V' == 86) && ('W' == 87) && ('X' == 88) && ('Y' == 89) \
      && ('Z' == 90) && ('[' == 91) && ('\\' == 92) && (']' == 93) \
      && ('^' == 94) && ('_' == 95) && ('a' == 97) && ('b' == 98) \
      && ('c' == 99) && ('d' == 100) && ('e' == 101) && ('f' == 102) \
      && ('g' == 103) && ('h' == 104) && ('i' == 105) && ('j' == 106) \
      && ('k' == 107) && ('l' == 108) && ('m' == 109) && ('n' == 110) \
      && ('o' == 111) && ('p' == 112) && ('q' == 113) && ('r' == 114) \
      && ('s' == 115) && ('t' == 116) && ('u' == 117) && ('v' == 118) \
      && ('w' == 119) && ('x' == 120) && ('y' == 121) && ('z' == 122) \
      && ('{' == 123) && ('|' == 124) && ('}' == 125) && ('~' == 126))
/* The character set is not based on ISO-646.  */
error "gperf generated tables don't work with this execution character set. Please report a bug to <bug-gnu-gperf@gnu.org>."
#endif


/*
 * Copyright (C) 2009-2013 Andreas Steffen
 * HSR Hochschule fuer Technik Rapperswil, Switzerland
 *
 * This program is free software; you can redistribute it and/or modify it
 * under the terms of the GNU General Public License as published by the
 * Free Software Foundation; either version 2 of the License, or (at your
 * option) any later version.  See <http://www.fsf.org/copyleft/gpl.txt>.
 *
 * This program is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY
 * or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License
 * for more details.
 */

#include <string.h>

#include <crypto/transform.h>
#include <crypto/crypters/crypter.h>
#include <crypto/signers/signer.h>
#include <crypto/diffie_hellman.h>

struct proposal_token {
	char             *name;
	transform_type_t  type;
	u_int16_t         algorithm;
	u_int16_t         keysize;
};

#define TOTAL_KEYWORDS 138
#define MIN_WORD_LENGTH 3
#define MAX_WORD_LENGTH 17
#define MIN_HASH_VALUE 20
#define MAX_HASH_VALUE 295
/* maximum key range = 276, duplicates = 0 */

#ifdef __GNUC__
__inline
#else
#ifdef __cplusplus
inline
#endif
#endif
static unsigned int
hash (str, len)
     register const char *str;
     register unsigned int len;
{
  static const unsigned short asso_values[] =
    {
      296, 296, 296, 296, 296, 296, 296, 296, 296, 296,
      296, 296, 296, 296, 296, 296, 296, 296, 296, 296,
      296, 296, 296, 296, 296, 296, 296, 296, 296, 296,
      296, 296, 296, 296, 296, 296, 296, 296, 296, 296,
      296, 296, 296, 296, 296, 296, 296, 296,  47,   6,
       15,   8,  64,  24,  12,  14,   7,   5, 296, 296,
      296, 296, 296, 296, 296, 296, 296, 296, 296, 296,
      296, 296, 296, 296, 296, 296, 296, 296, 296, 296,
      296, 296, 296, 296, 296, 296, 296, 296, 296, 296,
      296, 296, 296, 296, 296, 120, 296,   9,   5,  22,
       48, 114,  28,  76,   6,   5, 296, 296,   5,  20,
        7,  14,  82,   7,  81,  98,  10,  86, 296, 296,
        5, 296, 296, 296, 296, 296, 296, 296, 296, 296,
      296, 296, 296, 296, 296, 296, 296, 296, 296, 296,
      296, 296, 296, 296, 296, 296, 296, 296, 296, 296,
      296, 296, 296, 296, 296, 296, 296, 296, 296, 296,
      296, 296, 296, 296, 296, 296, 296, 296, 296, 296,
      296, 296, 296, 296, 296, 296, 296, 296, 296, 296,
      296, 296, 296, 296, 296, 296, 296, 296, 296, 296,
      296, 296, 296, 296, 296, 296, 296, 296, 296, 296,
      296, 296, 296, 296, 296, 296, 296, 296, 296, 296,
      296, 296, 296, 296, 296, 296, 296, 296, 296, 296,
      296, 296, 296, 296, 296, 296, 296, 296, 296, 296,
      296, 296, 296, 296, 296, 296, 296, 296, 296, 296,
      296, 296, 296, 296, 296, 296, 296, 296, 296, 296,
      296, 296, 296, 296, 296, 296, 296
    };
  register int hval = len;

  switch (hval)
    {
      default:
        hval += asso_values[(unsigned char)str[14]];
      /*FALLTHROUGH*/
      case 14:
      case 13:
      case 12:
      case 11:
      case 10:
        hval += asso_values[(unsigned char)str[9]];
      /*FALLTHROUGH*/
      case 9:
      case 8:
      case 7:
        hval += asso_values[(unsigned char)str[6]];
      /*FALLTHROUGH*/
      case 6:
        hval += asso_values[(unsigned char)str[5]];
      /*FALLTHROUGH*/
      case 5:
        hval += asso_values[(unsigned char)str[4]];
      /*FALLTHROUGH*/
      case 4:
      case 3:
      case 2:
      case 1:
        hval += asso_values[(unsigned char)str[0]+1];
        break;
    }
  return hval + asso_values[(unsigned char)str[len - 1]];
}

static const struct proposal_token wordlist[] =
  {
    {"sha1",             INTEGRITY_ALGORITHM,  AUTH_HMAC_SHA1_96,         0},
    {"sha",              INTEGRITY_ALGORITHM,  AUTH_HMAC_SHA1_96,         0},
    {"null",             ENCRYPTION_ALGORITHM, ENCR_NULL,                 0},
    {"noesn",            EXTENDED_SEQUENCE_NUMBERS, NO_EXT_SEQ_NUMBERS,   0},
    {"md5",              INTEGRITY_ALGORITHM,  AUTH_HMAC_MD5_96,          0},
    {"esn",              EXTENDED_SEQUENCE_NUMBERS, EXT_SEQ_NUMBERS,      0},
    {"aes128",           ENCRYPTION_ALGORITHM, ENCR_AES_CBC,            128},
    {"prfsha1",          PSEUDO_RANDOM_FUNCTION, PRF_HMAC_SHA1,           0},
    {"aes192",           ENCRYPTION_ALGORITHM, ENCR_AES_CBC,            192},
    {"modp8192",         DIFFIE_HELLMAN_GROUP, MODP_8192_BIT,             0},
    {"md5_128",          INTEGRITY_ALGORITHM,  AUTH_HMAC_MD5_128,         0},
    {"sha512",           INTEGRITY_ALGORITHM,  AUTH_HMAC_SHA2_512_256,    0},
    {"modp768",          DIFFIE_HELLMAN_GROUP, MODP_768_BIT,              0},
    {"ntru128",          DIFFIE_HELLMAN_GROUP, NTRU_128_BIT,              0},
    {"prfsha256",        PSEUDO_RANDOM_FUNCTION, PRF_HMAC_SHA2_256,       0},
    {"aes256",           ENCRYPTION_ALGORITHM, ENCR_AES_CBC,            256},
    {"ecp521",           DIFFIE_HELLMAN_GROUP, ECP_521_BIT,               0},
    {"ntru192",          DIFFIE_HELLMAN_GROUP, NTRU_192_BIT,              0},
    {"ntru112",          DIFFIE_HELLMAN_GROUP, NTRU_112_BIT,              0},
    {"sha256",           INTEGRITY_ALGORITHM,  AUTH_HMAC_SHA2_256_128,    0},
    {"modp1536",         DIFFIE_HELLMAN_GROUP, MODP_1536_BIT,             0},
    {"ecp192",           DIFFIE_HELLMAN_GROUP, ECP_192_BIT,               0},
    {"prfsha512",        PSEUDO_RANDOM_FUNCTION, PRF_HMAC_SHA2_512,       0},
    {"aes192ccm8",       ENCRYPTION_ALGORITHM, ENCR_AES_CCM_ICV8,       192},
    {"aes192ccm128",     ENCRYPTION_ALGORITHM, ENCR_AES_CCM_ICV16,      192},
    {"aes128ccm8",       ENCRYPTION_ALGORITHM, ENCR_AES_CCM_ICV8,       128},
    {"aes128ccm128",     ENCRYPTION_ALGORITHM, ENCR_AES_CCM_ICV16,      128},
    {"aes192ccm96",      ENCRYPTION_ALGORITHM, ENCR_AES_CCM_ICV12,      192},
    {"aes192ccm16",      ENCRYPTION_ALGORITHM, ENCR_AES_CCM_ICV16,      192},
    {"aes128ccm96",      ENCRYPTION_ALGORITHM, ENCR_AES_CCM_ICV12,      128},
    {"aes128ccm16",      ENCRYPTION_ALGORITHM, ENCR_AES_CCM_ICV16,      128},
    {"aes192ccm12",      ENCRYPTION_ALGORITHM, ENCR_AES_CCM_ICV12,      192},
    {"camellia",         ENCRYPTION_ALGORITHM, ENCR_CAMELLIA_CBC,       128},
    {"aes128ccm12",      ENCRYPTION_ALGORITHM, ENCR_AES_CCM_ICV12,      128},
    {"ecp256",           DIFFIE_HELLMAN_GROUP, ECP_256_BIT,               0},
    {"aesxcbc",          INTEGRITY_ALGORITHM,  AUTH_AES_XCBC_96,          0},
    {"ntru256",          DIFFIE_HELLMAN_GROUP, NTRU_256_BIT,              0},
    {"aescmac",          INTEGRITY_ALGORITHM,  AUTH_AES_CMAC_96,          0},
    {"aes256ccm8",       ENCRYPTION_ALGORITHM, ENCR_AES_CCM_ICV8,       256},
    {"aes256ccm128",     ENCRYPTION_ALGORITHM, ENCR_AES_CCM_ICV16,      256},
    {"cast128",          ENCRYPTION_ALGORITHM, ENCR_CAST,               128},
    {"aes256ccm96",      ENCRYPTION_ALGORITHM, ENCR_AES_CCM_ICV12,      256},
    {"aes256ccm16",      ENCRYPTION_ALGORITHM, ENCR_AES_CCM_ICV16,      256},
    {"camellia192",      ENCRYPTION_ALGORITHM, ENCR_CAMELLIA_CBC,       192},
    {"aes256ccm12",      ENCRYPTION_ALGORITHM, ENCR_AES_CCM_ICV12,      256},
    {"camellia128",      ENCRYPTION_ALGORITHM, ENCR_CAMELLIA_CBC,       128},
    {"camellia192ccm8",  ENCRYPTION_ALGORITHM, ENCR_CAMELLIA_CCM_ICV8,  192},
    {"camellia192ccm128",ENCRYPTION_ALGORITHM, ENCR_CAMELLIA_CCM_ICV16, 192},
    {"modp3072",         DIFFIE_HELLMAN_GROUP, MODP_3072_BIT,             0},
    {"camellia192ccm96", ENCRYPTION_ALGORITHM, ENCR_CAMELLIA_CCM_ICV12, 192},
    {"camellia192ccm16", ENCRYPTION_ALGORITHM, ENCR_CAMELLIA_CCM_ICV16, 192},
    {"prfsha384",        PSEUDO_RANDOM_FUNCTION, PRF_HMAC_SHA2_384,       0},
    {"camellia192ccm12", ENCRYPTION_ALGORITHM, ENCR_CAMELLIA_CCM_ICV12, 192},
    {"aes",              ENCRYPTION_ALGORITHM, ENCR_AES_CBC,            128},
    {"camellia128ccm8",  ENCRYPTION_ALGORITHM, ENCR_CAMELLIA_CCM_ICV8,  128},
    {"camellia128ccm128",ENCRYPTION_ALGORITHM, ENCR_CAMELLIA_CCM_ICV16, 128},
    {"prfmd5",           PSEUDO_RANDOM_FUNCTION, PRF_HMAC_MD5,            0},
    {"camellia256",      ENCRYPTION_ALGORITHM, ENCR_CAMELLIA_CBC,       256},
    {"camellia128ccm96", ENCRYPTION_ALGORITHM, ENCR_CAMELLIA_CCM_ICV12, 128},
    {"camellia128ccm16", ENCRYPTION_ALGORITHM, ENCR_CAMELLIA_CCM_ICV16, 128},
    {"camellia128ccm12", ENCRYPTION_ALGORITHM, ENCR_CAMELLIA_CCM_ICV12, 128},
    {"camellia256ccm8",  ENCRYPTION_ALGORITHM, ENCR_CAMELLIA_CCM_ICV8,  256},
    {"camellia256ccm128",ENCRYPTION_ALGORITHM, ENCR_CAMELLIA_CCM_ICV16, 256},
    {"modpnull",         DIFFIE_HELLMAN_GROUP, MODP_NULL,                 0},
    {"camelliaxcbc",     INTEGRITY_ALGORITHM,  AUTH_CAMELLIA_XCBC_96,     0},
    {"camellia256ccm96", ENCRYPTION_ALGORITHM, ENCR_CAMELLIA_CCM_ICV12, 256},
    {"camellia256ccm16", ENCRYPTION_ALGORITHM, ENCR_CAMELLIA_CCM_ICV16, 256},
    {"camellia256ccm12", ENCRYPTION_ALGORITHM, ENCR_CAMELLIA_CCM_ICV12, 256},
    {"aes192gcm8",       ENCRYPTION_ALGORITHM, ENCR_AES_GCM_ICV8,       192},
    {"aes192gcm128",     ENCRYPTION_ALGORITHM, ENCR_AES_GCM_ICV16,      192},
    {"aes128gcm8",       ENCRYPTION_ALGORITHM, ENCR_AES_GCM_ICV8,       128},
    {"aes128gcm128",     ENCRYPTION_ALGORITHM, ENCR_AES_GCM_ICV16,      128},
    {"aes192gcm96",      ENCRYPTION_ALGORITHM, ENCR_AES_GCM_ICV12,      192},
    {"aes192gcm16",      ENCRYPTION_ALGORITHM, ENCR_AES_GCM_ICV16,      192},
    {"aes128gcm96",      ENCRYPTION_ALGORITHM, ENCR_AES_GCM_ICV12,      128},
    {"aes128gcm16",      ENCRYPTION_ALGORITHM, ENCR_AES_GCM_ICV16,      128},
    {"aes192gcm12",      ENCRYPTION_ALGORITHM, ENCR_AES_GCM_ICV12,      192},
    {"aes192ccm64",      ENCRYPTION_ALGORITHM, ENCR_AES_CCM_ICV8,       192},
    {"aes128gcm12",      ENCRYPTION_ALGORITHM, ENCR_AES_GCM_ICV12,      128},
    {"aes128ccm64",      ENCRYPTION_ALGORITHM, ENCR_AES_CCM_ICV8,       128},
    {"aes192ctr",        ENCRYPTION_ALGORITHM, ENCR_AES_CTR,            192},
    {"aes128ctr",        ENCRYPTION_ALGORITHM, ENCR_AES_CTR,            128},
    {"modp1024s160",     DIFFIE_HELLMAN_GROUP, MODP_1024_160,             0},
    {"aes256gcm8",       ENCRYPTION_ALGORITHM, ENCR_AES_GCM_ICV8,       256},
    {"aes256gcm128",     ENCRYPTION_ALGORITHM, ENCR_AES_GCM_ICV16,      256},
    {"modp4096",         DIFFIE_HELLMAN_GROUP, MODP_4096_BIT,             0},
    {"ecp512bp",         DIFFIE_HELLMAN_GROUP, ECP_512_BP,                0},
    {"aes256gcm96",      ENCRYPTION_ALGORITHM, ENCR_AES_GCM_ICV12,      256},
    {"aes256gcm16",      ENCRYPTION_ALGORITHM, ENCR_AES_GCM_ICV16,      256},
    {"modp1024",         DIFFIE_HELLMAN_GROUP, MODP_1024_BIT,             0},
    {"modp2048",         DIFFIE_HELLMAN_GROUP, MODP_2048_BIT,             0},
    {"aes256gcm12",      ENCRYPTION_ALGORITHM, ENCR_AES_GCM_ICV12,      256},
    {"aes256ccm64",      ENCRYPTION_ALGORITHM, ENCR_AES_CCM_ICV8,       256},
    {"sha384",           INTEGRITY_ALGORITHM,  AUTH_HMAC_SHA2_384_192,    0},
    {"aes256ctr",        ENCRYPTION_ALGORITHM, ENCR_AES_CTR,            256},
    {"aes192gmac",       ENCRYPTION_ALGORITHM, ENCR_NULL_AUTH_AES_GMAC, 192},
    {"aes128gmac",       ENCRYPTION_ALGORITHM, ENCR_NULL_AUTH_AES_GMAC, 128},
    {"serpent",          ENCRYPTION_ALGORITHM, ENCR_SERPENT_CBC,        128},
    {"ecp256bp",         DIFFIE_HELLMAN_GROUP, ECP_256_BP,                0},
    {"camellia192ccm64", ENCRYPTION_ALGORITHM, ENCR_CAMELLIA_CCM_ICV8,  192},
    {"modp6144",         DIFFIE_HELLMAN_GROUP, MODP_6144_BIT,             0},
    {"camellia192ctr",   ENCRYPTION_ALGORITHM, ENCR_CAMELLIA_CTR,       192},
    {"serpent128",       ENCRYPTION_ALGORITHM, ENCR_SERPENT_CBC,        128},
    {"3des",             ENCRYPTION_ALGORITHM, ENCR_3DES,                 0},
    {"blowfish",         ENCRYPTION_ALGORITHM, ENCR_BLOWFISH,           128},
    {"ecp384",           DIFFIE_HELLMAN_GROUP, ECP_384_BIT,               0},
    {"camellia128ccm64", ENCRYPTION_ALGORITHM, ENCR_CAMELLIA_CCM_ICV8,  128},
    {"aes256gmac",       ENCRYPTION_ALGORITHM, ENCR_NULL_AUTH_AES_GMAC, 256},
    {"modp2048s256",     DIFFIE_HELLMAN_GROUP, MODP_2048_256,             0},
    {"camellia128ctr",   ENCRYPTION_ALGORITHM, ENCR_CAMELLIA_CTR,       128},
    {"serpent256",       ENCRYPTION_ALGORITHM, ENCR_SERPENT_CBC,        256},
    {"ecp224",           DIFFIE_HELLMAN_GROUP, ECP_224_BIT,               0},
    {"camellia256ccm64", ENCRYPTION_ALGORITHM, ENCR_CAMELLIA_CCM_ICV8,  256},
    {"serpent192",       ENCRYPTION_ALGORITHM, ENCR_SERPENT_CBC,        192},
    {"camellia256ctr",   ENCRYPTION_ALGORITHM, ENCR_CAMELLIA_CTR,       256},
    {"sha2_512",         INTEGRITY_ALGORITHM,  AUTH_HMAC_SHA2_512_256,    0},
    {"blowfish192",      ENCRYPTION_ALGORITHM, ENCR_BLOWFISH,           192},
    {"blowfish128",      ENCRYPTION_ALGORITHM, ENCR_BLOWFISH,           128},
    {"sha256_96",        INTEGRITY_ALGORITHM,  AUTH_HMAC_SHA2_256_96,     0},
    {"aes192gcm64",      ENCRYPTION_ALGORITHM, ENCR_AES_GCM_ICV8,       192},
    {"sha2_256",         INTEGRITY_ALGORITHM,  AUTH_HMAC_SHA2_256_128,    0},
    {"aes128gcm64",      ENCRYPTION_ALGORITHM, ENCR_AES_GCM_ICV8,       128},
    {"ecp384bp",         DIFFIE_HELLMAN_GROUP, ECP_384_BP,                0},
    {"sha2_256_96",      INTEGRITY_ALGORITHM,  AUTH_HMAC_SHA2_256_96,     0},
    {"blowfish256",      ENCRYPTION_ALGORITHM, ENCR_BLOWFISH,           256},
    {"ecp224bp",         DIFFIE_HELLMAN_GROUP, ECP_224_BP,                0},
    {"sha1_160",         INTEGRITY_ALGORITHM,  AUTH_HMAC_SHA1_160,        0},
    {"aes256gcm64",      ENCRYPTION_ALGORITHM, ENCR_AES_GCM_ICV8,       256},
    {"twofish",          ENCRYPTION_ALGORITHM, ENCR_TWOFISH_CBC,        128},
    {"prfcamelliaxcbc",  PSEUDO_RANDOM_FUNCTION, PRF_CAMELLIA128_XCBC,    0},
    {"des",              ENCRYPTION_ALGORITHM, ENCR_DES,                  0},
    {"sha2_384",         INTEGRITY_ALGORITHM,  AUTH_HMAC_SHA2_384_192,    0},
    {"twofish128",       ENCRYPTION_ALGORITHM, ENCR_TWOFISH_CBC,        128},
    {"modp2048s224",     DIFFIE_HELLMAN_GROUP, MODP_2048_224,             0},
    {"twofish256",       ENCRYPTION_ALGORITHM, ENCR_TWOFISH_CBC,        256},
    {"twofish192",       ENCRYPTION_ALGORITHM, ENCR_TWOFISH_CBC,        192},
    {"prfaesxcbc",       PSEUDO_RANDOM_FUNCTION, PRF_AES128_XCBC,         0},
    {"prfaescmac",       PSEUDO_RANDOM_FUNCTION, PRF_AES128_CMAC,         0}
  };

static const short lookup[] =
  {
     -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,
     -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,
      0,  -1,   1,   2,  -1,  -1,  -1,  -1,  -1,  -1,
     -1,  -1,  -1,   3,   4,  -1,  -1,  -1,   5,  -1,
      6,   7,  -1,  -1,  -1,  -1,   8,  -1,   9,  10,
     -1,  -1,  11,  -1,  12,  -1,  13,  -1,  14,  15,
     -1,  16,  17,  18,  19,  20,  -1,  -1,  -1,  21,
     22,  23,  24,  25,  26,  27,  28,  29,  30,  31,
     32,  33,  34,  35,  36,  37,  -1,  38,  39,  -1,
     40,  41,  42,  -1,  43,  44,  45,  46,  47,  48,
     -1,  49,  50,  51,  -1,  52,  53,  54,  55,  56,
     57,  58,  59,  -1,  -1,  60,  61,  62,  63,  64,
     65,  66,  -1,  -1,  67,  68,  69,  70,  71,  72,
     73,  74,  75,  76,  77,  78,  79,  80,  -1,  81,
     82,  83,  84,  85,  86,  87,  88,  89,  90,  91,
     92,  93,  -1,  94,  -1,  95,  -1,  96,  97,  98,
     99, 100,  -1, 101,  -1, 102, 103, 104,  -1, 105,
    106, 107, 108, 109,  -1, 110,  -1, 111,  -1, 112,
     -1, 113, 114, 115, 116,  -1, 117, 118, 119, 120,
    121,  -1,  -1,  -1, 122,  -1,  -1, 123,  -1,  -1,
    124,  -1, 125, 126, 127,  -1,  -1,  -1, 128,  -1,
     -1,  -1,  -1,  -1, 129, 130,  -1, 131,  -1, 132,
     -1,  -1,  -1,  -1, 133,  -1,  -1,  -1,  -1, 134,
     -1,  -1,  -1,  -1,  -1, 135,  -1,  -1,  -1,  -1,
     -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,
     -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,
     -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,
     -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1, 136,  -1,
     -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,
     -1,  -1,  -1,  -1,  -1, 137
  };

#ifdef __GNUC__
__inline
#if defined __GNUC_STDC_INLINE__ || defined __GNUC_GNU_INLINE__
__attribute__ ((__gnu_inline__))
#endif
#endif
const struct proposal_token *
proposal_get_token_static (str, len)
     register const char *str;
     register unsigned int len;
{
  if (len <= MAX_WORD_LENGTH && len >= MIN_WORD_LENGTH)
    {
      register int key = hash (str, len);

      if (key <= MAX_HASH_VALUE && key >= 0)
        {
          register int index = lookup[key];

          if (index >= 0)
            {
              register const char *s = wordlist[index].name;

              if (*str == *s && !strncmp (str + 1, s + 1, len - 1) && s[len] == '\0')
                return &wordlist[index];
            }
        }
    }
  return 0;
}

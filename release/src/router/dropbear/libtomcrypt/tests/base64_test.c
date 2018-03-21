/* LibTomCrypt, modular cryptographic library -- Tom St Denis
 *
 * LibTomCrypt is a library that provides various cryptographic
 * algorithms in a highly modular and flexible manner.
 *
 * The library is free for all purposes without any express
 * guarantee it works.
 */
#include  <tomcrypt_test.h>

#if defined(LTC_BASE64) || defined(LTC_BASE64_URL)
int base64_test(void)
{
   unsigned char in[64], out[256], tmp[64];
   unsigned long x, l1, l2, slen1;

   const unsigned char special_case[] = {
         0xbe, 0xe8, 0x92, 0x3c, 0xa2, 0x25, 0xf0, 0xf8,
         0x91, 0xe4, 0xef, 0xab, 0x0b, 0x8c, 0xfd, 0xff,
         0x14, 0xd0, 0x29, 0x9d, 0x00 };

#if defined(LTC_BASE64)
   /*
    TEST CASES SOURCE:

    Network Working Group                                       S. Josefsson
    Request for Comments: 4648                                           SJD
    Obsoletes: 3548                                             October 2006
    Category: Standards Track
    */
   const struct {
     const char* s;
     const char* b64;
   } cases[] = {
       {"", ""              },
       {"f", "Zg=="         },
       {"fo", "Zm8="        },
       {"foo", "Zm9v"       },
       {"foob", "Zm9vYg=="  },
       {"fooba", "Zm9vYmE=" },
       {"foobar", "Zm9vYmFy"},
       {(char*)special_case,"vuiSPKIl8PiR5O+rC4z9/xTQKZ0="}
   };
#endif

#ifdef LTC_BASE64_URL
   const struct {
      const char* s;
      int is_strict;
   } url_cases[] = {
         {"vuiSPKIl8PiR5O-rC4z9_xTQKZ0", 0},
         {"vuiSPKIl8PiR5O-rC4z9_xTQKZ0=", 1},
         {"vuiS*PKIl8P*iR5O-rC4*z9_xTQKZ0", 0},
         {"vuiS*PKIl8P*iR5O-rC4*z9_xTQKZ0=", 0},
         {"vuiS*PKIl8P*iR5O-rC4*z9_xTQKZ0==", 0},
         {"vuiS*PKIl8P*iR5O-rC4*z9_xTQKZ0===", 0},
         {"vuiS*PKIl8P*iR5O-rC4*z9_xTQKZ0====", 0},
         {"vuiS*=PKIl8P*iR5O-rC4*z9_xTQKZ0=", 0},
         {"vuiS*==PKIl8P*iR5O-rC4*z9_xTQKZ0=", 0},
         {"vuiS*===PKIl8P*iR5O-rC4*z9_xTQKZ0=", 0},
   };

   for (x = 0; x < sizeof(url_cases)/sizeof(url_cases[0]); ++x) {
       slen1 = strlen(url_cases[x].s);
       l1 = sizeof(out);
       if(url_cases[x].is_strict)
          DO(base64url_strict_decode((unsigned char*)url_cases[x].s, slen1, out, &l1));
       else
          DO(base64url_decode((unsigned char*)url_cases[x].s, slen1, out, &l1));
       if (compare_testvector(out, l1, special_case, sizeof(special_case) - 1, "base64url decode", x)) {
           return 1;
       }
       if(x < 2) {
          l2 = sizeof(tmp);
          if(x == 0)
             DO(base64url_encode(out, l1, tmp, &l2));
          else
             DO(base64url_strict_encode(out, l1, tmp, &l2));
          if (compare_testvector(tmp, l2, url_cases[x].s, strlen(url_cases[x].s), "base64url encode", x)) {
              return 1;
          }
       }
   }

   DO(base64url_strict_decode((unsigned char*)url_cases[4].s, slen1, out, &l1) == CRYPT_INVALID_PACKET ? CRYPT_OK : CRYPT_INVALID_PACKET);
#endif

#if defined(LTC_BASE64)
   for (x = 0; x < sizeof(cases)/sizeof(cases[0]); ++x) {
       memset(out, 0, sizeof(out));
       memset(tmp, 0, sizeof(tmp));
       slen1 = strlen(cases[x].s);
       l1 = sizeof(out);
       DO(base64_encode((unsigned char*)cases[x].s, slen1, out, &l1));
       l2 = sizeof(tmp);
       DO(base64_strict_decode(out, l1, tmp, &l2));
       if (compare_testvector(out, l1, cases[x].b64, strlen(cases[x].b64), "base64 encode", x) ||
             compare_testvector(tmp, l2, cases[x].s, slen1, "base64 decode", x)) {
           return 1;
       }
   }

   for  (x = 0; x < 64; x++) {
       yarrow_read(in, x, &yarrow_prng);
       l1 = sizeof(out);
       DO(base64_encode(in, x, out, &l1));
       l2 = sizeof(tmp);
       DO(base64_decode(out, l1, tmp, &l2));
       if (compare_testvector(tmp, x, in, x, "random base64", x)) {
           return 1;
       }
   }

   x--;
   memmove(&out[11], &out[10], l1 - 10);
   out[10] = '=';
   l1++;
   l2 = sizeof(tmp);
   DO(base64_decode(out, l1, tmp, &l2));
   if (compare_testvector(tmp, l2, in, l2, "relaxed base64 decoding", -1)) {
       print_hex("input ", out, l1);
       return 1;
   }
   l2 = sizeof(tmp);
   DO(base64_strict_decode(out, l1, tmp, &l2) == CRYPT_INVALID_PACKET ? CRYPT_OK : CRYPT_INVALID_PACKET);
#endif

   return 0;
}
#endif

/* ref:         $Format:%D$ */
/* git commit:  $Format:%H$ */
/* commit time: $Format:%ai$ */

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
  @file der_decode_generalizedtime.c
  ASN.1 DER, decode a GeneralizedTime, Steffen Jaeckel
  Based on der_decode_utctime.c
*/

#ifdef LTC_DER

static int _char_to_int(unsigned char x)
{
   switch (x)  {
      case '0': return 0;
      case '1': return 1;
      case '2': return 2;
      case '3': return 3;
      case '4': return 4;
      case '5': return 5;
      case '6': return 6;
      case '7': return 7;
      case '8': return 8;
      case '9': return 9;
      default:  return 100;
   }
}

#define DECODE_V(y, max) do {\
   y  = _char_to_int(buf[x])*10 + _char_to_int(buf[x+1]); \
   if (y >= max) return CRYPT_INVALID_PACKET;           \
   x += 2; \
} while(0)

#define DECODE_V4(y, max) do {\
   y  = _char_to_int(buf[x])*1000 + _char_to_int(buf[x+1])*100 + _char_to_int(buf[x+2])*10 + _char_to_int(buf[x+3]); \
   if (y >= max) return CRYPT_INVALID_PACKET; \
   x += 4; \
} while(0)

/**
  Decodes a Generalized time structure in DER format (reads all 6 valid encoding formats)
  @param in     Input buffer
  @param inlen  Length of input buffer in octets
  @param out    [out] Destination of Generalized time structure
  @return CRYPT_OK   if successful
*/
int der_decode_generalizedtime(const unsigned char *in, unsigned long *inlen,
                               ltc_generalizedtime *out)
{
   unsigned char buf[32];
   unsigned long x;
   int           y;

   LTC_ARGCHK(in    != NULL);
   LTC_ARGCHK(inlen != NULL);
   LTC_ARGCHK(out   != NULL);

   /* check header */
   if (*inlen < 2UL || (in[1] >= sizeof(buf)) || ((in[1] + 2UL) > *inlen)) {
      return CRYPT_INVALID_PACKET;
   }

   /* decode the string */
   for (x = 0; x < in[1]; x++) {
       y = der_ia5_value_decode(in[x+2]);
       if (y == -1) {
          return CRYPT_INVALID_PACKET;
       }
       if (!((y >= '0' && y <= '9')
            || y == 'Z' || y == '.'
            || y == '+' || y == '-')) {
          return CRYPT_INVALID_PACKET;
       }
       buf[x] = y;
   }
   *inlen = 2 + x;

   if (x < 15) {
      return CRYPT_INVALID_PACKET;
   }

   /* possible encodings are
YYYYMMDDhhmmssZ
YYYYMMDDhhmmss+hh'mm'
YYYYMMDDhhmmss-hh'mm'
YYYYMMDDhhmmss.fsZ
YYYYMMDDhhmmss.fs+hh'mm'
YYYYMMDDhhmmss.fs-hh'mm'

    So let's do a trivial decode upto [including] ss
   */

    x = 0;
    DECODE_V4(out->YYYY, 10000);
    DECODE_V(out->MM, 13);
    DECODE_V(out->DD, 32);
    DECODE_V(out->hh, 24);
    DECODE_V(out->mm, 60);
    DECODE_V(out->ss, 60);

    /* clear fractional seconds info */
    out->fs = 0;

    /* now is it Z or . */
    if (buf[x] == 'Z') {
       return CRYPT_OK;
    } else if (buf[x] == '.') {
       x++;
       while (buf[x] >= '0' && buf[x] <= '9') {
          unsigned fs = out->fs;
          if (x >= sizeof(buf)) return CRYPT_INVALID_PACKET;
          out->fs *= 10;
          out->fs += _char_to_int(buf[x]);
          if (fs > out->fs) return CRYPT_OVERFLOW;
          x++;
       }
    }

    /* now is it Z, +, - */
    if (buf[x] == 'Z') {
       return CRYPT_OK;
    } else if (buf[x] == '+' || buf[x] == '-') {
       out->off_dir = (buf[x++] == '+') ? 0 : 1;
       DECODE_V(out->off_hh, 24);
       DECODE_V(out->off_mm, 60);
       return CRYPT_OK;
    } else {
       return CRYPT_INVALID_PACKET;
    }
}

#endif

/* ref:         $Format:%D$ */
/* git commit:  $Format:%H$ */
/* commit time: $Format:%ai$ */

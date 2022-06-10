/* Copyright (c) 2001, Matej Pfajfar.
 * Copyright (c) 2001-2004, Roger Dingledine.
 * Copyright (c) 2004-2006, Roger Dingledine, Nick Mathewson.
 * Copyright (c) 2007-2021, The Tor Project, Inc. */
/* See LICENSE for licensing information */

/**
 * \file binascii.c
 *
 * \brief Miscellaneous functions for encoding and decoding various things
 *   in base{16,32,64}.
 */

#include "orconfig.h"

#include "lib/encoding/binascii.h"
#include "lib/log/log.h"
#include "lib/log/util_bug.h"
#include "lib/cc/torint.h"
#include "lib/string/compat_ctype.h"
#include "lib/intmath/muldiv.h"
#include "lib/malloc/malloc.h"

#include <stddef.h>
#include <string.h>
#include <stdlib.h>

/** Return a pointer to a NUL-terminated hexadecimal string encoding
 * the first <b>fromlen</b> bytes of <b>from</b>. (fromlen must be \<= 32.) The
 * result does not need to be deallocated, but repeated calls to
 * hex_str will trash old results.
 */
const char *
hex_str(const char *from, size_t fromlen)
{
  static char buf[65];
  if (fromlen>(sizeof(buf)-1)/2)
    fromlen = (sizeof(buf)-1)/2;
  base16_encode(buf,sizeof(buf),from,fromlen);
  return buf;
}

/* Return the base32 encoded size in bytes using the source length srclen.
 *
 * (WATCH OUT: This API counts the terminating NUL byte, but
 * base64_encode_size does not.)
 */
size_t
base32_encoded_size(size_t srclen)
{
  size_t enclen;
  tor_assert(srclen < SIZE_T_CEILING / 8);
  enclen = BASE32_NOPAD_BUFSIZE(srclen);
  tor_assert(enclen < INT_MAX && enclen > srclen);
  return enclen;
}

/** Implements base32 encoding as in RFC 4648. */
void
base32_encode(char *dest, size_t destlen, const char *src, size_t srclen)
{
  unsigned int i, v, u;
  size_t nbits = srclen * 8;
  size_t bit;

  /* We need enough space for the encoded data and the extra NUL byte. */
  tor_assert(base32_encoded_size(srclen) <= destlen);
  tor_assert(destlen < SIZE_T_CEILING);

  /* Make sure we leave no uninitialized data in the destination buffer. */
  memset(dest, 0, destlen);

  for (i=0,bit=0; bit < nbits; ++i, bit+=5) {
    /* set v to the 16-bit value starting at src[bits/8], 0-padded. */
    size_t idx = bit / 8;
    v = ((uint8_t)src[idx]) << 8;
    if (idx+1 < srclen)
      v += (uint8_t)src[idx+1];
    /* set u to the 5-bit value at the bit'th bit of buf. */
    u = (v >> (11-(bit%8))) & 0x1F;
    dest[i] = BASE32_CHARS[u];
  }
  dest[i] = '\0';
}

/** Implements base32 decoding as in RFC 4648.
 * Return the number of bytes decoded if successful; -1 otherwise.
 */
int
base32_decode(char *dest, size_t destlen, const char *src, size_t srclen)
{
  /* XXXX we might want to rewrite this along the lines of base64_decode, if
   * it ever shows up in the profile. */
  unsigned int i;
  size_t nbits, j, bit;
  char *tmp;
  nbits = ((srclen * 5) / 8) * 8;

  tor_assert(srclen < SIZE_T_CEILING / 5);
  tor_assert((nbits/8) <= destlen); /* We need enough space. */
  tor_assert(destlen < SIZE_T_CEILING);

  /* Make sure we leave no uninitialized data in the destination buffer. */
  memset(dest, 0, destlen);

  /* Convert base32 encoded chars to the 5-bit values that they represent. */
  tmp = tor_malloc_zero(srclen);
  for (j = 0; j < srclen; ++j) {
    if (src[j] > 0x60 && src[j] < 0x7B) tmp[j] = src[j] - 0x61;
    else if (src[j] > 0x31 && src[j] < 0x38) tmp[j] = src[j] - 0x18;
    else if (src[j] > 0x40 && src[j] < 0x5B) tmp[j] = src[j] - 0x41;
    else {
      log_warn(LD_GENERAL, "illegal character in base32 encoded string");
      tor_free(tmp);
      return -1;
    }
  }

  /* Assemble result byte-wise by applying five possible cases. */
  for (i = 0, bit = 0; bit < nbits; ++i, bit += 8) {
    switch (bit % 40) {
    case 0:
      dest[i] = (((uint8_t)tmp[(bit/5)]) << 3) +
                (((uint8_t)tmp[(bit/5)+1]) >> 2);
      break;
    case 8:
      dest[i] = (((uint8_t)tmp[(bit/5)]) << 6) +
                (((uint8_t)tmp[(bit/5)+1]) << 1) +
                (((uint8_t)tmp[(bit/5)+2]) >> 4);
      break;
    case 16:
      dest[i] = (((uint8_t)tmp[(bit/5)]) << 4) +
                (((uint8_t)tmp[(bit/5)+1]) >> 1);
      break;
    case 24:
      dest[i] = (((uint8_t)tmp[(bit/5)]) << 7) +
                (((uint8_t)tmp[(bit/5)+1]) << 2) +
                (((uint8_t)tmp[(bit/5)+2]) >> 3);
      break;
    case 32:
      dest[i] = (((uint8_t)tmp[(bit/5)]) << 5) +
                ((uint8_t)tmp[(bit/5)+1]);
      break;
    }
  }

  memset(tmp, 0, srclen); /* on the heap, this should be safe */
  tor_free(tmp);
  tmp = NULL;
  return i;
}

#define BASE64_OPENSSL_LINELEN 64

/** Return the Base64 encoded size of <b>srclen</b> bytes of data in
 * bytes.
 *
 * (WATCH OUT: This API <em>does not</em> count the terminating NUL byte,
 * but base32_encoded_size does.)
 *
 * If <b>flags</b>&amp;BASE64_ENCODE_MULTILINE is true, return the size
 * of the encoded output as multiline output (64 character, `\n' terminated
 * lines).
 */
size_t
base64_encode_size(size_t srclen, int flags)
{
  size_t enclen;

  /* Use INT_MAX for overflow checking because base64_encode() returns int. */
  tor_assert(srclen < INT_MAX);
  tor_assert(CEIL_DIV(srclen, 3) < INT_MAX / 4);

  enclen = BASE64_LEN(srclen);
  if (flags & BASE64_ENCODE_MULTILINE)
    enclen += CEIL_DIV(enclen, BASE64_OPENSSL_LINELEN);

  tor_assert(enclen < INT_MAX && (enclen == 0 || enclen > srclen));
  return enclen;
}

/** Return an upper bound on the number of bytes that might be needed to hold
 * the data from decoding the base64 string <b>srclen</b>.  This is only an
 * upper bound, since some part of the base64 string might be padding or
 * space. */
size_t
base64_decode_maxsize(size_t srclen)
{
  tor_assert(srclen < INT_MAX / 3);

  return CEIL_DIV(srclen * 3, 4);
}

/** Internal table mapping 6 bit values to the Base64 alphabet. */
static const char base64_encode_table[64] = {
  'A', 'B', 'C', 'D', 'E', 'F', 'G', 'H',
  'I', 'J', 'K', 'L', 'M', 'N', 'O', 'P',
  'Q', 'R', 'S', 'T', 'U', 'V', 'W', 'X',
  'Y', 'Z', 'a', 'b', 'c', 'd', 'e', 'f',
  'g', 'h', 'i', 'j', 'k', 'l', 'm', 'n',
  'o', 'p', 'q', 'r', 's', 't', 'u', 'v',
  'w', 'x', 'y', 'z', '0', '1', '2', '3',
  '4', '5', '6', '7', '8', '9', '+', '/'
};

/** Base64 encode <b>srclen</b> bytes of data from <b>src</b>.  Write
 * the result into <b>dest</b>, if it will fit within <b>destlen</b>
 * bytes. Return the number of bytes written on success; -1 if
 * destlen is too short, or other failure.
 *
 * If <b>flags</b>&amp;BASE64_ENCODE_MULTILINE is true, return encoded
 * output in multiline format (64 character, `\n' terminated lines).
 */
int
base64_encode(char *dest, size_t destlen, const char *src, size_t srclen,
              int flags)
{
  const unsigned char *usrc = (unsigned char *)src;
  const unsigned char *eous = usrc + srclen;
  char *d = dest;
  uint32_t n = 0;
  size_t linelen = 0;
  size_t enclen;
  int n_idx = 0;

  if (!src || !dest)
    return -1;

  /* Ensure that there is sufficient space, including the NUL. */
  enclen = base64_encode_size(srclen, flags);
  if (destlen < enclen + 1)
    return -1;
  if (destlen > SIZE_T_CEILING)
    return -1;
  if (enclen > INT_MAX)
    return -1;

  /* Make sure we leave no uninitialized data in the destination buffer. */
  memset(dest, 0, destlen);

  /* XXX/Yawning: If this ends up being too slow, this can be sped up
   * by separating the multiline format case and the normal case, and
   * processing 48 bytes of input at a time when newlines are desired.
   */
#define ENCODE_CHAR(ch) \
  STMT_BEGIN                                                    \
    *d++ = ch;                                                  \
    if (flags & BASE64_ENCODE_MULTILINE) {                      \
      if (++linelen % BASE64_OPENSSL_LINELEN == 0) {            \
        linelen = 0;                                            \
        *d++ = '\n';                                            \
      }                                                         \
    }                                                           \
  STMT_END

#define ENCODE_N(idx) \
  ENCODE_CHAR(base64_encode_table[(n >> ((3 - idx) * 6)) & 0x3f])

#define ENCODE_PAD() ENCODE_CHAR('=')

  /* Iterate over all the bytes in src.  Each one will add 8 bits to the
   * value we're encoding.  Accumulate bits in <b>n</b>, and whenever we
   * have 24 bits, batch them into 4 bytes and flush those bytes to dest.
   */
  for ( ; usrc < eous; ++usrc) {
    n = (n << 8) | *usrc;
    if ((++n_idx) == 3) {
      ENCODE_N(0);
      ENCODE_N(1);
      ENCODE_N(2);
      ENCODE_N(3);
      n_idx = 0;
      n = 0;
    }
  }
  switch (n_idx) {
  case 0:
    /* 0 leftover bits, no padding to add. */
    break;
  case 1:
    /* 8 leftover bits, pad to 12 bits, write the 2 6-bit values followed
     * by 2 padding characters.
     */
    n <<= 4;
    ENCODE_N(2);
    ENCODE_N(3);
    ENCODE_PAD();
    ENCODE_PAD();
    break;
  case 2:
    /* 16 leftover bits, pad to 18 bits, write the 3 6-bit values followed
     * by 1 padding character.
     */
    n <<= 2;
    ENCODE_N(1);
    ENCODE_N(2);
    ENCODE_N(3);
    ENCODE_PAD();
    break;
  // LCOV_EXCL_START -- we can't reach this point, because we enforce
  // 0 <= ncov_idx < 3 in the loop above.
  default:
    /* Something went catastrophically wrong. */
    tor_fragile_assert();
    return -1;
  // LCOV_EXCL_STOP
  }

#undef ENCODE_N
#undef ENCODE_PAD
#undef ENCODE_CHAR

  /* Multiline output always includes at least one newline. */
  if (flags & BASE64_ENCODE_MULTILINE && linelen != 0)
    *d++ = '\n';

  tor_assert(d - dest == (ptrdiff_t)enclen);

  *d++ = '\0'; /* NUL terminate the output. */

  return (int) enclen;
}

/** As base64_encode, but do not add any internal spaces, and remove external
 * padding from the output stream.
 * dest must be at least base64_encode_size(srclen, 0), including space for
 * the removed external padding. */
int
base64_encode_nopad(char *dest, size_t destlen,
                    const uint8_t *src, size_t srclen)
{
  int n = base64_encode(dest, destlen, (const char*) src, srclen, 0);
  if (n <= 0)
    return n;
  tor_assert((size_t)n < destlen && dest[n] == 0);
  char *in, *out;
  in = out = dest;
  while (*in) {
    if (*in == '=' || *in == '\n') {
      ++in;
    } else {
      *out++ = *in++;
    }
  }
  *out = 0;

  tor_assert(out - dest <= INT_MAX);

  return (int)(out - dest);
}

#undef BASE64_OPENSSL_LINELEN

/** @{ */
/** Special values used for the base64_decode_table */
#define X 255
#define SP 64
#define PAD 65
/** @} */
/** Internal table mapping byte values to what they represent in base64.
 * Numbers 0..63 are 6-bit integers.  SPs are spaces, and should be
 * skipped.  Xs are invalid and must not appear in base64. PAD indicates
 * end-of-string. */
static const uint8_t base64_decode_table[256] = {
  X, X, X, X, X, X, X, X, X, SP, SP, SP, X, SP, X, X, /* */
  X, X, X, X, X, X, X, X, X, X, X, X, X, X, X, X,
  SP, X, X, X, X, X, X, X, X, X, X, 62, X, X, X, 63,
  52, 53, 54, 55, 56, 57, 58, 59, 60, 61, X, X, X, PAD, X, X,
  X, 0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14,
  15, 16, 17, 18, 19, 20, 21, 22, 23, 24, 25, X, X, X, X, X,
  X, 26, 27, 28, 29, 30, 31, 32, 33, 34, 35, 36, 37, 38, 39, 40,
  41, 42, 43, 44, 45, 46, 47, 48, 49, 50, 51, X, X, X, X, X,
  X, X, X, X, X, X, X, X, X, X, X, X, X, X, X, X,
  X, X, X, X, X, X, X, X, X, X, X, X, X, X, X, X,
  X, X, X, X, X, X, X, X, X, X, X, X, X, X, X, X,
  X, X, X, X, X, X, X, X, X, X, X, X, X, X, X, X,
  X, X, X, X, X, X, X, X, X, X, X, X, X, X, X, X,
  X, X, X, X, X, X, X, X, X, X, X, X, X, X, X, X,
  X, X, X, X, X, X, X, X, X, X, X, X, X, X, X, X,
  X, X, X, X, X, X, X, X, X, X, X, X, X, X, X, X,
};

/** Base64 decode <b>srclen</b> bytes of data from <b>src</b>.  Write
 * the result into <b>dest</b>, if it will fit within <b>destlen</b>
 * bytes.  Return the number of bytes written on success; -1 if
 * destlen is too short, or other failure.
 *
 * NOTE 1: destlen is checked conservatively, as though srclen contained no
 * spaces or padding.
 *
 * NOTE 2: This implementation does not check for the correct number of
 * padding "=" characters at the end of the string, and does not check
 * for internal padding characters.
 */
int
base64_decode(char *dest, size_t destlen, const char *src, size_t srclen)
{
  const char *eos = src+srclen;
  uint32_t n=0;
  int n_idx=0;
  size_t di = 0;

  if (destlen > INT_MAX)
    return -1;

  /* Make sure we leave no uninitialized data in the destination buffer. */
  memset(dest, 0, destlen);

  /* Iterate over all the bytes in src.  Each one will add 0 or 6 bits to the
   * value we're decoding.  Accumulate bits in <b>n</b>, and whenever we have
   * 24 bits, batch them into 3 bytes and flush those bytes to dest.
   */
  for ( ; src < eos; ++src) {
    unsigned char c = (unsigned char) *src;
    uint8_t v = base64_decode_table[c];
    switch (v) {
      case X:
        /* This character isn't allowed in base64. */
        return -1;
      case SP:
        /* This character is whitespace, and has no effect. */
        continue;
      case PAD:
        /* We've hit an = character: the data is over. */
        goto end_of_loop;
      default:
        /* We have an actual 6-bit value.  Append it to the bits in n. */
        n = (n<<6) | v;
        if ((++n_idx) == 4) {
          /* We've accumulated 24 bits in n. Flush them. */
          if (destlen < 3 || di > destlen - 3)
            return -1;
          dest[di++] = (n>>16);
          dest[di++] = (n>>8) & 0xff;
          dest[di++] = (n) & 0xff;
          n_idx = 0;
          n = 0;
        }
    }
  }
 end_of_loop:
  /* If we have leftover bits, we need to cope. */
  switch (n_idx) {
    case 0:
    default:
      /* No leftover bits.  We win. */
      break;
    case 1:
      /* 6 leftover bits. That's invalid; we can't form a byte out of that. */
      return -1;
    case 2:
      /* 12 leftover bits: The last 4 are padding and the first 8 are data. */
      if (destlen < 1 || di > destlen - 1)
        return -1;
      dest[di++] = n >> 4;
      break;
    case 3:
      /* 18 leftover bits: The last 2 are padding and the first 16 are data. */
      if (destlen < 2 || di > destlen - 2)
        return -1;
      dest[di++] = n >> 10;
      dest[di++] = n >> 2;
  }

  tor_assert(di <= destlen);

  return (int)di;
}
#undef X
#undef SP
#undef PAD

/** Encode the <b>srclen</b> bytes at <b>src</b> in a NUL-terminated,
 * uppercase hexadecimal string; store it in the <b>destlen</b>-byte buffer
 * <b>dest</b>.
 */
void
base16_encode(char *dest, size_t destlen, const char *src, size_t srclen)
{
  const char *end;
  char *cp;

  tor_assert(srclen < SIZE_T_CEILING / 2 - 1);
  tor_assert(destlen >= BASE16_BUFSIZE(srclen));
  tor_assert(destlen < SIZE_T_CEILING);

  /* Make sure we leave no uninitialized data in the destination buffer. */
  memset(dest, 0, destlen);

  cp = dest;
  end = src+srclen;
  while (src<end) {
    *cp++ = "0123456789ABCDEF"[ (*(const uint8_t*)src) >> 4 ];
    *cp++ = "0123456789ABCDEF"[ (*(const uint8_t*)src) & 0xf ];
    ++src;
  }
  *cp = '\0';
}

/** Given a hexadecimal string of <b>srclen</b> bytes in <b>src</b>, decode
 * it and store the result in the <b>destlen</b>-byte buffer at <b>dest</b>.
 * Return the number of bytes decoded on success, -1 on failure. If
 * <b>destlen</b> is greater than INT_MAX or less than half of
 * <b>srclen</b>, -1 is returned. */
int
base16_decode(char *dest, size_t destlen, const char *src, size_t srclen)
{
  const char *end;
  char *dest_orig = dest;
  int v1,v2;

  if ((srclen % 2) != 0)
    return -1;
  if (destlen < srclen/2 || destlen > INT_MAX)
    return -1;

  /* Make sure we leave no uninitialized data in the destination buffer. */
  memset(dest, 0, destlen);

  end = src+srclen;
  while (src<end) {
    v1 = hex_decode_digit(*src);
    v2 = hex_decode_digit(*(src+1));
    if (v1<0||v2<0)
      return -1;
    *(uint8_t*)dest = (v1<<4)|v2;
    ++dest;
    src+=2;
  }

  tor_assert((dest-dest_orig) <= (ptrdiff_t) destlen);

  return (int) (dest-dest_orig);
}

/* bufhelp.h  -  Some buffer manipulation helpers
 * Copyright (C) 2012-2017 Jussi Kivilinna <jussi.kivilinna@iki.fi>
 *
 * This file is part of Libgcrypt.
 *
 * Libgcrypt is free software; you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License as
 * published by the Free Software Foundation; either version 2.1 of
 * the License, or (at your option) any later version.
 *
 * Libgcrypt is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this program; if not, see <http://www.gnu.org/licenses/>.
 */
#ifndef GCRYPT_BUFHELP_H
#define GCRYPT_BUFHELP_H


#include "g10lib.h"
#include "bithelp.h"


#undef BUFHELP_UNALIGNED_ACCESS
#if defined(HAVE_GCC_ATTRIBUTE_PACKED) && \
    defined(HAVE_GCC_ATTRIBUTE_ALIGNED) && \
    defined(HAVE_GCC_ATTRIBUTE_MAY_ALIAS)
/* Compiler is supports attributes needed for automatically issuing unaligned
   memory access instructions.
 */
# define BUFHELP_UNALIGNED_ACCESS 1
#endif


#ifndef BUFHELP_UNALIGNED_ACCESS

/* Functions for loading and storing unaligned u32 values of different
   endianness.  */
static inline u32 buf_get_be32(const void *_buf)
{
  const byte *in = _buf;
  return ((u32)in[0] << 24) | ((u32)in[1] << 16) | \
         ((u32)in[2] << 8) | (u32)in[3];
}

static inline u32 buf_get_le32(const void *_buf)
{
  const byte *in = _buf;
  return ((u32)in[3] << 24) | ((u32)in[2] << 16) | \
         ((u32)in[1] << 8) | (u32)in[0];
}

static inline void buf_put_be32(void *_buf, u32 val)
{
  byte *out = _buf;
  out[0] = val >> 24;
  out[1] = val >> 16;
  out[2] = val >> 8;
  out[3] = val;
}

static inline void buf_put_le32(void *_buf, u32 val)
{
  byte *out = _buf;
  out[3] = val >> 24;
  out[2] = val >> 16;
  out[1] = val >> 8;
  out[0] = val;
}


/* Functions for loading and storing unaligned u64 values of different
   endianness.  */
static inline u64 buf_get_be64(const void *_buf)
{
  const byte *in = _buf;
  return ((u64)in[0] << 56) | ((u64)in[1] << 48) | \
         ((u64)in[2] << 40) | ((u64)in[3] << 32) | \
         ((u64)in[4] << 24) | ((u64)in[5] << 16) | \
         ((u64)in[6] << 8) | (u64)in[7];
}

static inline u64 buf_get_le64(const void *_buf)
{
  const byte *in = _buf;
  return ((u64)in[7] << 56) | ((u64)in[6] << 48) | \
         ((u64)in[5] << 40) | ((u64)in[4] << 32) | \
         ((u64)in[3] << 24) | ((u64)in[2] << 16) | \
         ((u64)in[1] << 8) | (u64)in[0];
}

static inline void buf_put_be64(void *_buf, u64 val)
{
  byte *out = _buf;
  out[0] = val >> 56;
  out[1] = val >> 48;
  out[2] = val >> 40;
  out[3] = val >> 32;
  out[4] = val >> 24;
  out[5] = val >> 16;
  out[6] = val >> 8;
  out[7] = val;
}

static inline void buf_put_le64(void *_buf, u64 val)
{
  byte *out = _buf;
  out[7] = val >> 56;
  out[6] = val >> 48;
  out[5] = val >> 40;
  out[4] = val >> 32;
  out[3] = val >> 24;
  out[2] = val >> 16;
  out[1] = val >> 8;
  out[0] = val;
}

#else /*BUFHELP_UNALIGNED_ACCESS*/

typedef struct bufhelp_u32_s
{
  u32 a;
} __attribute__((packed, aligned(1), may_alias)) bufhelp_u32_t;

/* Functions for loading and storing unaligned u32 values of different
   endianness.  */
static inline u32 buf_get_be32(const void *_buf)
{
  return be_bswap32(((const bufhelp_u32_t *)_buf)->a);
}

static inline u32 buf_get_le32(const void *_buf)
{
  return le_bswap32(((const bufhelp_u32_t *)_buf)->a);
}

static inline void buf_put_be32(void *_buf, u32 val)
{
  bufhelp_u32_t *out = _buf;
  out->a = be_bswap32(val);
}

static inline void buf_put_le32(void *_buf, u32 val)
{
  bufhelp_u32_t *out = _buf;
  out->a = le_bswap32(val);
}


typedef struct bufhelp_u64_s
{
  u64 a;
} __attribute__((packed, aligned(1), may_alias)) bufhelp_u64_t;

/* Functions for loading and storing unaligned u64 values of different
   endianness.  */
static inline u64 buf_get_be64(const void *_buf)
{
  return be_bswap64(((const bufhelp_u64_t *)_buf)->a);
}

static inline u64 buf_get_le64(const void *_buf)
{
  return le_bswap64(((const bufhelp_u64_t *)_buf)->a);
}

static inline void buf_put_be64(void *_buf, u64 val)
{
  bufhelp_u64_t *out = _buf;
  out->a = be_bswap64(val);
}

static inline void buf_put_le64(void *_buf, u64 val)
{
  bufhelp_u64_t *out = _buf;
  out->a = le_bswap64(val);
}

#endif /*BUFHELP_UNALIGNED_ACCESS*/


/* Host-endian get/put macros */
#ifdef WORDS_BIGENDIAN
# define buf_get_he32 buf_get_be32
# define buf_put_he32 buf_put_be32
# define buf_get_he64 buf_get_be64
# define buf_put_he64 buf_put_be64
#else
# define buf_get_he32 buf_get_le32
# define buf_put_he32 buf_put_le32
# define buf_get_he64 buf_get_le64
# define buf_put_he64 buf_put_le64
#endif



/* Optimized function for small buffer copying */
static inline void
buf_cpy(void *_dst, const void *_src, size_t len)
{
  byte *dst = _dst;
  const byte *src = _src;

#if __GNUC__ >= 4
  if (!__builtin_constant_p (len))
    {
      if (UNLIKELY(len == 0))
	return;
      memcpy(_dst, _src, len);
      return;
    }
#endif

  while (len >= sizeof(u64))
    {
      buf_put_he64(dst, buf_get_he64(src));
      dst += sizeof(u64);
      src += sizeof(u64);
      len -= sizeof(u64);
    }

  if (len >= sizeof(u32))
    {
      buf_put_he32(dst, buf_get_he32(src));
      dst += sizeof(u32);
      src += sizeof(u32);
      len -= sizeof(u32);
    }

  /* Handle tail.  */
  for (; len; len--)
    *dst++ = *src++;
}


/* Optimized function for buffer xoring */
static inline void
buf_xor(void *_dst, const void *_src1, const void *_src2, size_t len)
{
  byte *dst = _dst;
  const byte *src1 = _src1;
  const byte *src2 = _src2;

  while (len >= sizeof(u64))
    {
      buf_put_he64(dst, buf_get_he64(src1) ^ buf_get_he64(src2));
      dst += sizeof(u64);
      src1 += sizeof(u64);
      src2 += sizeof(u64);
      len -= sizeof(u64);
    }

  if (len > sizeof(u32))
    {
      buf_put_he32(dst, buf_get_he32(src1) ^ buf_get_he32(src2));
      dst += sizeof(u32);
      src1 += sizeof(u32);
      src2 += sizeof(u32);
      len -= sizeof(u32);
    }

  /* Handle tail.  */
  for (; len; len--)
    *dst++ = *src1++ ^ *src2++;
}


/* Optimized function for buffer xoring with two destination buffers.  Used
   mainly by CFB mode encryption.  */
static inline void
buf_xor_2dst(void *_dst1, void *_dst2, const void *_src, size_t len)
{
  byte *dst1 = _dst1;
  byte *dst2 = _dst2;
  const byte *src = _src;

  while (len >= sizeof(u64))
    {
      u64 temp = buf_get_he64(dst2) ^ buf_get_he64(src);
      buf_put_he64(dst2, temp);
      buf_put_he64(dst1, temp);
      dst2 += sizeof(u64);
      dst1 += sizeof(u64);
      src += sizeof(u64);
      len -= sizeof(u64);
    }

  if (len >= sizeof(u32))
    {
      u32 temp = buf_get_he32(dst2) ^ buf_get_he32(src);
      buf_put_he32(dst2, temp);
      buf_put_he32(dst1, temp);
      dst2 += sizeof(u32);
      dst1 += sizeof(u32);
      src += sizeof(u32);
      len -= sizeof(u32);
    }

  /* Handle tail.  */
  for (; len; len--)
    *dst1++ = (*dst2++ ^= *src++);
}


/* Optimized function for combined buffer xoring and copying.  Used by mainly
   CBC mode decryption.  */
static inline void
buf_xor_n_copy_2(void *_dst_xor, const void *_src_xor, void *_srcdst_cpy,
		 const void *_src_cpy, size_t len)
{
  byte *dst_xor = _dst_xor;
  byte *srcdst_cpy = _srcdst_cpy;
  const byte *src_xor = _src_xor;
  const byte *src_cpy = _src_cpy;

  while (len >= sizeof(u64))
    {
      u64 temp = buf_get_he64(src_cpy);
      buf_put_he64(dst_xor, buf_get_he64(srcdst_cpy) ^ buf_get_he64(src_xor));
      buf_put_he64(srcdst_cpy, temp);
      dst_xor += sizeof(u64);
      srcdst_cpy += sizeof(u64);
      src_xor += sizeof(u64);
      src_cpy += sizeof(u64);
      len -= sizeof(u64);
    }

  if (len >= sizeof(u32))
    {
      u32 temp = buf_get_he32(src_cpy);
      buf_put_he32(dst_xor, buf_get_he32(srcdst_cpy) ^ buf_get_he32(src_xor));
      buf_put_he32(srcdst_cpy, temp);
      dst_xor += sizeof(u32);
      srcdst_cpy += sizeof(u32);
      src_xor += sizeof(u32);
      src_cpy += sizeof(u32);
      len -= sizeof(u32);
    }

  /* Handle tail.  */
  for (; len; len--)
    {
      byte temp = *src_cpy++;
      *dst_xor++ = *srcdst_cpy ^ *src_xor++;
      *srcdst_cpy++ = temp;
    }
}


/* Optimized function for combined buffer xoring and copying.  Used by mainly
   CFB mode decryption.  */
static inline void
buf_xor_n_copy(void *_dst_xor, void *_srcdst_cpy, const void *_src, size_t len)
{
  buf_xor_n_copy_2(_dst_xor, _src, _srcdst_cpy, _src, len);
}


/* Constant-time compare of two buffers.  Returns 1 if buffers are equal,
   and 0 if buffers differ.  */
static inline int
buf_eq_const(const void *_a, const void *_b, size_t len)
{
  const byte *a = _a;
  const byte *b = _b;
  int ab, ba;
  size_t i;

  /* Constant-time compare. */
  for (i = 0, ab = 0, ba = 0; i < len; i++)
    {
      /* If a[i] != b[i], either ab or ba will be negative. */
      ab |= a[i] - b[i];
      ba |= b[i] - a[i];
    }

  /* 'ab | ba' is negative when buffers are not equal. */
  return (ab | ba) >= 0;
}


#endif /*GCRYPT_BUFHELP_H*/

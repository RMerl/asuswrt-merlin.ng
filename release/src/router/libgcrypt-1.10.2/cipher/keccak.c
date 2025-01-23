/* keccak.c - SHA3 hash functions
 * Copyright (C) 2015  g10 Code GmbH
 *
 * This file is part of Libgcrypt.
 *
 * Libgcrypt is free software; you can redistribute it and/or modify
 * it under the terms of the GNU Lesser general Public License as
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


#include <config.h>
#include <string.h>
#include "g10lib.h"
#include "bithelp.h"
#include "bufhelp.h"
#include "cipher.h"
#include "hash-common.h"



/* USE_64BIT indicates whether to use 64-bit generic implementation.
 * USE_32BIT indicates whether to use 32-bit generic implementation. */
#undef USE_64BIT
#if defined(__x86_64__) || SIZEOF_UNSIGNED_LONG == 8
# define USE_64BIT 1
#else
# define USE_32BIT 1
#endif


/* USE_64BIT_BMI2 indicates whether to compile with 64-bit Intel BMI2 code. */
#undef USE_64BIT_BMI2
#if defined(USE_64BIT) && defined(HAVE_GCC_INLINE_ASM_BMI2) && \
    defined(HAVE_CPU_ARCH_X86)
# define USE_64BIT_BMI2 1
#endif


/* USE_64BIT_SHLD indicates whether to compile with 64-bit Intel SHLD code. */
#undef USE_64BIT_SHLD
#if defined(USE_64BIT) && defined (__GNUC__) && defined(__x86_64__) && \
    defined(HAVE_CPU_ARCH_X86)
# define USE_64BIT_SHLD 1
#endif


/* USE_32BIT_BMI2 indicates whether to compile with 32-bit Intel BMI2 code. */
#undef USE_32BIT_BMI2
#if defined(USE_32BIT) && defined(HAVE_GCC_INLINE_ASM_BMI2) && \
    defined(HAVE_CPU_ARCH_X86)
# define USE_32BIT_BMI2 1
#endif


/* USE_64BIT_ARM_NEON indicates whether to enable 64-bit ARM/NEON assembly
 * code. */
#undef USE_64BIT_ARM_NEON
#ifdef ENABLE_NEON_SUPPORT
# if defined(HAVE_ARM_ARCH_V6) && defined(__ARMEL__) \
     && defined(HAVE_COMPATIBLE_GCC_ARM_PLATFORM_AS) \
     && defined(HAVE_GCC_INLINE_ASM_NEON)
#  define USE_64BIT_ARM_NEON 1
# endif
#endif /*ENABLE_NEON_SUPPORT*/


/* USE_S390X_CRYPTO indicates whether to enable zSeries code. */
#undef USE_S390X_CRYPTO
#if defined(HAVE_GCC_INLINE_ASM_S390X)
# define USE_S390X_CRYPTO 1
#endif /* USE_S390X_CRYPTO */


#if defined(USE_64BIT) || defined(USE_64BIT_ARM_NEON)
# define NEED_COMMON64 1
#endif

#ifdef USE_32BIT
# define NEED_COMMON32BI 1
#endif


#define SHA3_DELIMITED_SUFFIX 0x06
#define SHAKE_DELIMITED_SUFFIX 0x1F


typedef struct
{
  union {
#ifdef NEED_COMMON64
    u64 state64[25];
#endif
#ifdef NEED_COMMON32BI
    u32 state32bi[50];
#endif
  } u;
} KECCAK_STATE;


typedef struct
{
  unsigned int (*permute)(KECCAK_STATE *hd);
  unsigned int (*absorb)(KECCAK_STATE *hd, int pos, const byte *lanes,
			 size_t nlanes, int blocklanes);
  unsigned int (*extract) (KECCAK_STATE *hd, unsigned int pos, byte *outbuf,
			   unsigned int outlen);
} keccak_ops_t;


typedef struct KECCAK_CONTEXT_S
{
  KECCAK_STATE state;
  unsigned int outlen;
  unsigned int blocksize;
  unsigned int count;
  unsigned int suffix;
  const keccak_ops_t *ops;
#ifdef USE_S390X_CRYPTO
  unsigned int kimd_func;
  unsigned int buf_pos;
  byte buf[1344 / 8]; /* SHAKE128 requires biggest buffer, 1344 bits. */
#endif
} KECCAK_CONTEXT;



#ifdef NEED_COMMON64

const u64 _gcry_keccak_round_consts_64bit[24 + 1] =
{
  U64_C(0x0000000000000001), U64_C(0x0000000000008082),
  U64_C(0x800000000000808A), U64_C(0x8000000080008000),
  U64_C(0x000000000000808B), U64_C(0x0000000080000001),
  U64_C(0x8000000080008081), U64_C(0x8000000000008009),
  U64_C(0x000000000000008A), U64_C(0x0000000000000088),
  U64_C(0x0000000080008009), U64_C(0x000000008000000A),
  U64_C(0x000000008000808B), U64_C(0x800000000000008B),
  U64_C(0x8000000000008089), U64_C(0x8000000000008003),
  U64_C(0x8000000000008002), U64_C(0x8000000000000080),
  U64_C(0x000000000000800A), U64_C(0x800000008000000A),
  U64_C(0x8000000080008081), U64_C(0x8000000000008080),
  U64_C(0x0000000080000001), U64_C(0x8000000080008008),
  U64_C(0xFFFFFFFFFFFFFFFF)
};

static unsigned int
keccak_extract64(KECCAK_STATE *hd, unsigned int pos, byte *outbuf,
		 unsigned int outlen)
{
  unsigned int i;

  /* NOTE: when pos == 0, hd and outbuf may point to same memory (SHA-3). */

  for (i = pos; i < pos + outlen / 8 + !!(outlen % 8); i++)
    {
      u64 tmp = hd->u.state64[i];
      buf_put_le64(outbuf, tmp);
      outbuf += 8;
    }

  return 0;
}

#endif /* NEED_COMMON64 */


#ifdef NEED_COMMON32BI

static const u32 round_consts_32bit[2 * 24] =
{
  0x00000001UL, 0x00000000UL, 0x00000000UL, 0x00000089UL,
  0x00000000UL, 0x8000008bUL, 0x00000000UL, 0x80008080UL,
  0x00000001UL, 0x0000008bUL, 0x00000001UL, 0x00008000UL,
  0x00000001UL, 0x80008088UL, 0x00000001UL, 0x80000082UL,
  0x00000000UL, 0x0000000bUL, 0x00000000UL, 0x0000000aUL,
  0x00000001UL, 0x00008082UL, 0x00000000UL, 0x00008003UL,
  0x00000001UL, 0x0000808bUL, 0x00000001UL, 0x8000000bUL,
  0x00000001UL, 0x8000008aUL, 0x00000001UL, 0x80000081UL,
  0x00000000UL, 0x80000081UL, 0x00000000UL, 0x80000008UL,
  0x00000000UL, 0x00000083UL, 0x00000000UL, 0x80008003UL,
  0x00000001UL, 0x80008088UL, 0x00000000UL, 0x80000088UL,
  0x00000001UL, 0x00008000UL, 0x00000000UL, 0x80008082UL
};

static unsigned int
keccak_extract32bi(KECCAK_STATE *hd, unsigned int pos, byte *outbuf,
		   unsigned int outlen)
{
  unsigned int i;
  u32 x0;
  u32 x1;
  u32 t;

  /* NOTE: when pos == 0, hd and outbuf may point to same memory (SHA-3). */

  for (i = pos; i < pos + outlen / 8 + !!(outlen % 8); i++)
    {
      x0 = hd->u.state32bi[i * 2 + 0];
      x1 = hd->u.state32bi[i * 2 + 1];

      t = (x0 & 0x0000FFFFUL) + (x1 << 16);
      x1 = (x0 >> 16) + (x1 & 0xFFFF0000UL);
      x0 = t;
      t = (x0 ^ (x0 >> 8)) & 0x0000FF00UL; x0 = x0 ^ t ^ (t << 8);
      t = (x0 ^ (x0 >> 4)) & 0x00F000F0UL; x0 = x0 ^ t ^ (t << 4);
      t = (x0 ^ (x0 >> 2)) & 0x0C0C0C0CUL; x0 = x0 ^ t ^ (t << 2);
      t = (x0 ^ (x0 >> 1)) & 0x22222222UL; x0 = x0 ^ t ^ (t << 1);
      t = (x1 ^ (x1 >> 8)) & 0x0000FF00UL; x1 = x1 ^ t ^ (t << 8);
      t = (x1 ^ (x1 >> 4)) & 0x00F000F0UL; x1 = x1 ^ t ^ (t << 4);
      t = (x1 ^ (x1 >> 2)) & 0x0C0C0C0CUL; x1 = x1 ^ t ^ (t << 2);
      t = (x1 ^ (x1 >> 1)) & 0x22222222UL; x1 = x1 ^ t ^ (t << 1);

      buf_put_le32(&outbuf[0], x0);
      buf_put_le32(&outbuf[4], x1);
      outbuf += 8;
    }

  return 0;
}

static inline void
keccak_absorb_lane32bi(u32 *lane, u32 x0, u32 x1)
{
  u32 t;

  t = (x0 ^ (x0 >> 1)) & 0x22222222UL; x0 = x0 ^ t ^ (t << 1);
  t = (x0 ^ (x0 >> 2)) & 0x0C0C0C0CUL; x0 = x0 ^ t ^ (t << 2);
  t = (x0 ^ (x0 >> 4)) & 0x00F000F0UL; x0 = x0 ^ t ^ (t << 4);
  t = (x0 ^ (x0 >> 8)) & 0x0000FF00UL; x0 = x0 ^ t ^ (t << 8);
  t = (x1 ^ (x1 >> 1)) & 0x22222222UL; x1 = x1 ^ t ^ (t << 1);
  t = (x1 ^ (x1 >> 2)) & 0x0C0C0C0CUL; x1 = x1 ^ t ^ (t << 2);
  t = (x1 ^ (x1 >> 4)) & 0x00F000F0UL; x1 = x1 ^ t ^ (t << 4);
  t = (x1 ^ (x1 >> 8)) & 0x0000FF00UL; x1 = x1 ^ t ^ (t << 8);
  lane[0] ^= (x0 & 0x0000FFFFUL) + (x1 << 16);
  lane[1] ^= (x0 >> 16) + (x1 & 0xFFFF0000UL);
}

#endif /* NEED_COMMON32BI */


/* Construct generic 64-bit implementation. */
#ifdef USE_64BIT

#if __GNUC__ >= 4 && defined(__x86_64__)

static inline void absorb_lanes64_8(u64 *dst, const byte *in)
{
  asm ("movdqu 0*16(%[dst]), %%xmm0\n\t"
       "movdqu 0*16(%[in]), %%xmm4\n\t"
       "movdqu 1*16(%[dst]), %%xmm1\n\t"
       "movdqu 1*16(%[in]), %%xmm5\n\t"
       "movdqu 2*16(%[dst]), %%xmm2\n\t"
       "movdqu 3*16(%[dst]), %%xmm3\n\t"
       "pxor %%xmm4, %%xmm0\n\t"
       "pxor %%xmm5, %%xmm1\n\t"
       "movdqu 2*16(%[in]), %%xmm4\n\t"
       "movdqu 3*16(%[in]), %%xmm5\n\t"
       "movdqu %%xmm0, 0*16(%[dst])\n\t"
       "pxor %%xmm4, %%xmm2\n\t"
       "movdqu %%xmm1, 1*16(%[dst])\n\t"
       "pxor %%xmm5, %%xmm3\n\t"
       "movdqu %%xmm2, 2*16(%[dst])\n\t"
       "movdqu %%xmm3, 3*16(%[dst])\n\t"
       :
       : [dst] "r" (dst), [in] "r" (in)
       : "xmm0", "xmm1", "xmm2", "xmm3", "xmm4", "xmm5", "memory");
}

static inline void absorb_lanes64_4(u64 *dst, const byte *in)
{
  asm ("movdqu 0*16(%[dst]), %%xmm0\n\t"
       "movdqu 0*16(%[in]), %%xmm4\n\t"
       "movdqu 1*16(%[dst]), %%xmm1\n\t"
       "movdqu 1*16(%[in]), %%xmm5\n\t"
       "pxor %%xmm4, %%xmm0\n\t"
       "pxor %%xmm5, %%xmm1\n\t"
       "movdqu %%xmm0, 0*16(%[dst])\n\t"
       "movdqu %%xmm1, 1*16(%[dst])\n\t"
       :
       : [dst] "r" (dst), [in] "r" (in)
       : "xmm0", "xmm1", "xmm4", "xmm5", "memory");
}

static inline void absorb_lanes64_2(u64 *dst, const byte *in)
{
  asm ("movdqu 0*16(%[dst]), %%xmm0\n\t"
       "movdqu 0*16(%[in]), %%xmm4\n\t"
       "pxor %%xmm4, %%xmm0\n\t"
       "movdqu %%xmm0, 0*16(%[dst])\n\t"
       :
       : [dst] "r" (dst), [in] "r" (in)
       : "xmm0", "xmm4", "memory");
}

#else /* __x86_64__ */

static inline void absorb_lanes64_8(u64 *dst, const byte *in)
{
  dst[0] ^= buf_get_le64(in + 8 * 0);
  dst[1] ^= buf_get_le64(in + 8 * 1);
  dst[2] ^= buf_get_le64(in + 8 * 2);
  dst[3] ^= buf_get_le64(in + 8 * 3);
  dst[4] ^= buf_get_le64(in + 8 * 4);
  dst[5] ^= buf_get_le64(in + 8 * 5);
  dst[6] ^= buf_get_le64(in + 8 * 6);
  dst[7] ^= buf_get_le64(in + 8 * 7);
}

static inline void absorb_lanes64_4(u64 *dst, const byte *in)
{
  dst[0] ^= buf_get_le64(in + 8 * 0);
  dst[1] ^= buf_get_le64(in + 8 * 1);
  dst[2] ^= buf_get_le64(in + 8 * 2);
  dst[3] ^= buf_get_le64(in + 8 * 3);
}

static inline void absorb_lanes64_2(u64 *dst, const byte *in)
{
  dst[0] ^= buf_get_le64(in + 8 * 0);
  dst[1] ^= buf_get_le64(in + 8 * 1);
}

#endif /* !__x86_64__ */

static inline void absorb_lanes64_1(u64 *dst, const byte *in)
{
  dst[0] ^= buf_get_le64(in + 8 * 0);
}


# define ANDN64(x, y) (~(x) & (y))
# define ROL64(x, n) (((x) << ((unsigned int)n & 63)) | \
		      ((x) >> ((64 - (unsigned int)(n)) & 63)))

# define KECCAK_F1600_PERMUTE_FUNC_NAME keccak_f1600_state_permute64
# define KECCAK_F1600_ABSORB_FUNC_NAME keccak_absorb_lanes64
# include "keccak_permute_64.h"

# undef ANDN64
# undef ROL64
# undef KECCAK_F1600_PERMUTE_FUNC_NAME
# undef KECCAK_F1600_ABSORB_FUNC_NAME

static const keccak_ops_t keccak_generic64_ops =
{
  .permute = keccak_f1600_state_permute64,
  .absorb = keccak_absorb_lanes64,
  .extract = keccak_extract64,
};

#endif /* USE_64BIT */


/* Construct 64-bit Intel SHLD implementation. */
#ifdef USE_64BIT_SHLD

# define ANDN64(x, y) (~(x) & (y))
# define ROL64(x, n) ({ \
			u64 tmp = (x); \
			asm ("shldq %1, %0, %0" \
			     : "+r" (tmp) \
			     : "J" ((n) & 63) \
			     : "cc"); \
			tmp; })

# define KECCAK_F1600_PERMUTE_FUNC_NAME keccak_f1600_state_permute64_shld
# define KECCAK_F1600_ABSORB_FUNC_NAME keccak_absorb_lanes64_shld
# include "keccak_permute_64.h"

# undef ANDN64
# undef ROL64
# undef KECCAK_F1600_PERMUTE_FUNC_NAME
# undef KECCAK_F1600_ABSORB_FUNC_NAME

static const keccak_ops_t keccak_shld_64_ops =
{
  .permute = keccak_f1600_state_permute64_shld,
  .absorb = keccak_absorb_lanes64_shld,
  .extract = keccak_extract64,
};

#endif /* USE_64BIT_SHLD */


/* Construct 64-bit Intel BMI2 implementation. */
#ifdef USE_64BIT_BMI2

# define ANDN64(x, y) ({ \
			u64 tmp; \
			asm ("andnq %2, %1, %0" \
			     : "=r" (tmp) \
			     : "r0" (x), "rm" (y)); \
			tmp; })

# define ROL64(x, n) ({ \
			u64 tmp; \
			asm ("rorxq %2, %1, %0" \
			     : "=r" (tmp) \
			     : "rm0" (x), "J" (64 - ((n) & 63))); \
			tmp; })

# define KECCAK_F1600_PERMUTE_FUNC_NAME keccak_f1600_state_permute64_bmi2
# define KECCAK_F1600_ABSORB_FUNC_NAME keccak_absorb_lanes64_bmi2
# include "keccak_permute_64.h"

# undef ANDN64
# undef ROL64
# undef KECCAK_F1600_PERMUTE_FUNC_NAME
# undef KECCAK_F1600_ABSORB_FUNC_NAME

static const keccak_ops_t keccak_bmi2_64_ops =
{
  .permute = keccak_f1600_state_permute64_bmi2,
  .absorb = keccak_absorb_lanes64_bmi2,
  .extract = keccak_extract64,
};

#endif /* USE_64BIT_BMI2 */


/* 64-bit ARMv7/NEON implementation. */
#ifdef USE_64BIT_ARM_NEON

unsigned int _gcry_keccak_permute_armv7_neon(u64 *state);
unsigned int _gcry_keccak_absorb_lanes64_armv7_neon(u64 *state, int pos,
						    const byte *lanes,
						    size_t nlanes,
						    int blocklanes);

static unsigned int keccak_permute64_armv7_neon(KECCAK_STATE *hd)
{
  return _gcry_keccak_permute_armv7_neon(hd->u.state64);
}

static unsigned int
keccak_absorb_lanes64_armv7_neon(KECCAK_STATE *hd, int pos, const byte *lanes,
				 size_t nlanes, int blocklanes)
{
  if (blocklanes < 0)
    {
      /* blocklanes == -1, permutationless absorb from keccak_final. */

      while (nlanes)
	{
	  hd->u.state64[pos] ^= buf_get_le64(lanes);
	  lanes += 8;
	  nlanes--;
	}

      return 0;
    }
  else
    {
      return _gcry_keccak_absorb_lanes64_armv7_neon(hd->u.state64, pos, lanes,
						    nlanes, blocklanes);
    }
}

static const keccak_ops_t keccak_armv7_neon_64_ops =
{
  .permute = keccak_permute64_armv7_neon,
  .absorb = keccak_absorb_lanes64_armv7_neon,
  .extract = keccak_extract64,
};

#endif /* USE_64BIT_ARM_NEON */


/* Construct generic 32-bit implementation. */
#ifdef USE_32BIT

# define ANDN32(x, y) (~(x) & (y))
# define ROL32(x, n) (((x) << ((unsigned int)n & 31)) | \
		      ((x) >> ((32 - (unsigned int)(n)) & 31)))

# define KECCAK_F1600_PERMUTE_FUNC_NAME keccak_f1600_state_permute32bi
# include "keccak_permute_32.h"

# undef ANDN32
# undef ROL32
# undef KECCAK_F1600_PERMUTE_FUNC_NAME

static unsigned int
keccak_absorb_lanes32bi(KECCAK_STATE *hd, int pos, const byte *lanes,
		        size_t nlanes, int blocklanes)
{
  unsigned int burn = 0;

  while (nlanes)
    {
      keccak_absorb_lane32bi(&hd->u.state32bi[pos * 2],
			     buf_get_le32(lanes + 0),
			     buf_get_le32(lanes + 4));
      lanes += 8;
      nlanes--;

      if (++pos == blocklanes)
	{
	  burn = keccak_f1600_state_permute32bi(hd);
	  pos = 0;
	}
    }

  return burn;
}

static const keccak_ops_t keccak_generic32bi_ops =
{
  .permute = keccak_f1600_state_permute32bi,
  .absorb = keccak_absorb_lanes32bi,
  .extract = keccak_extract32bi,
};

#endif /* USE_32BIT */


/* Construct 32-bit Intel BMI2 implementation. */
#ifdef USE_32BIT_BMI2

# define ANDN32(x, y) ({ \
			u32 tmp; \
			asm ("andnl %2, %1, %0" \
			     : "=r" (tmp) \
			     : "r0" (x), "rm" (y)); \
			tmp; })

# define ROL32(x, n) ({ \
			u32 tmp; \
			asm ("rorxl %2, %1, %0" \
			     : "=r" (tmp) \
			     : "rm0" (x), "J" (32 - ((n) & 31))); \
			tmp; })

# define KECCAK_F1600_PERMUTE_FUNC_NAME keccak_f1600_state_permute32bi_bmi2
# include "keccak_permute_32.h"

# undef ANDN32
# undef ROL32
# undef KECCAK_F1600_PERMUTE_FUNC_NAME

static inline u32 pext(u32 x, u32 mask)
{
  u32 tmp;
  asm ("pextl %2, %1, %0" : "=r" (tmp) : "r0" (x), "rm" (mask));
  return tmp;
}

static inline u32 pdep(u32 x, u32 mask)
{
  u32 tmp;
  asm ("pdepl %2, %1, %0" : "=r" (tmp) : "r0" (x), "rm" (mask));
  return tmp;
}

static inline void
keccak_absorb_lane32bi_bmi2(u32 *lane, u32 x0, u32 x1)
{
  x0 = pdep(pext(x0, 0x55555555), 0x0000ffff) | (pext(x0, 0xaaaaaaaa) << 16);
  x1 = pdep(pext(x1, 0x55555555), 0x0000ffff) | (pext(x1, 0xaaaaaaaa) << 16);

  lane[0] ^= (x0 & 0x0000FFFFUL) + (x1 << 16);
  lane[1] ^= (x0 >> 16) + (x1 & 0xFFFF0000UL);
}

static unsigned int
keccak_absorb_lanes32bi_bmi2(KECCAK_STATE *hd, int pos, const byte *lanes,
		             size_t nlanes, int blocklanes)
{
  unsigned int burn = 0;

  while (nlanes)
    {
      keccak_absorb_lane32bi_bmi2(&hd->u.state32bi[pos * 2],
			          buf_get_le32(lanes + 0),
			          buf_get_le32(lanes + 4));
      lanes += 8;
      nlanes--;

      if (++pos == blocklanes)
	{
	  burn = keccak_f1600_state_permute32bi_bmi2(hd);
	  pos = 0;
	}
    }

  return burn;
}

static unsigned int
keccak_extract32bi_bmi2(KECCAK_STATE *hd, unsigned int pos, byte *outbuf,
			unsigned int outlen)
{
  unsigned int i;
  u32 x0;
  u32 x1;
  u32 t;

  /* NOTE: when pos == 0, hd and outbuf may point to same memory (SHA-3). */

  for (i = pos; i < pos + outlen / 8 + !!(outlen % 8); i++)
    {
      x0 = hd->u.state32bi[i * 2 + 0];
      x1 = hd->u.state32bi[i * 2 + 1];

      t = (x0 & 0x0000FFFFUL) + (x1 << 16);
      x1 = (x0 >> 16) + (x1 & 0xFFFF0000UL);
      x0 = t;

      x0 = pdep(pext(x0, 0xffff0001), 0xaaaaaaab) | pdep(x0 >> 1, 0x55555554);
      x1 = pdep(pext(x1, 0xffff0001), 0xaaaaaaab) | pdep(x1 >> 1, 0x55555554);

      buf_put_le32(&outbuf[0], x0);
      buf_put_le32(&outbuf[4], x1);
      outbuf += 8;
    }

  return 0;
}

static const keccak_ops_t keccak_bmi2_32bi_ops =
{
  .permute = keccak_f1600_state_permute32bi_bmi2,
  .absorb = keccak_absorb_lanes32bi_bmi2,
  .extract = keccak_extract32bi_bmi2,
};

#endif /* USE_32BIT_BMI2 */


#ifdef USE_S390X_CRYPTO
#include "asm-inline-s390x.h"

static inline void
keccak_bwrite_s390x (void *context, const byte *in, size_t inlen)
{
  KECCAK_CONTEXT *ctx = context;

  /* Write full-blocks. */
  kimd_execute (ctx->kimd_func, &ctx->state, in, inlen);
  return;
}

static inline void
keccak_final_s390x (void *context)
{
  KECCAK_CONTEXT *ctx = context;

  if (ctx->suffix == SHA3_DELIMITED_SUFFIX)
    {
      klmd_execute (ctx->kimd_func, &ctx->state, ctx->buf, ctx->count);
    }
  else
    {
      klmd_shake_execute (ctx->kimd_func, &ctx->state, NULL, 0, ctx->buf,
			  ctx->count);
      ctx->count = 0;
      ctx->buf_pos = 0;
    }

  return;
}

static inline void
keccak_bextract_s390x (void *context, byte *out, size_t outlen)
{
  KECCAK_CONTEXT *ctx = context;

  /* Extract full-blocks. */
  klmd_shake_execute (ctx->kimd_func | KLMD_PADDING_STATE, &ctx->state,
		      out, outlen, NULL, 0);
  return;
}

static void
keccak_write_s390x (void *context, const byte *inbuf, size_t inlen)
{
  KECCAK_CONTEXT *hd = context;
  const size_t blocksize = hd->blocksize;
  size_t inblocks;
  size_t copylen;

  while (hd->count)
    {
      if (hd->count == blocksize)  /* Flush the buffer. */
	{
	  keccak_bwrite_s390x (hd, hd->buf, blocksize);
	  hd->count = 0;
	}
      else
	{
	  copylen = inlen;
	  if (copylen > blocksize - hd->count)
	    copylen = blocksize - hd->count;

	  if (copylen == 0)
	    break;

	  buf_cpy (&hd->buf[hd->count], inbuf, copylen);
	  hd->count += copylen;
	  inbuf += copylen;
	  inlen -= copylen;
	}
    }

  if (inlen == 0)
    return;

  if (inlen >= blocksize)
    {
      inblocks = inlen / blocksize;
      keccak_bwrite_s390x (hd, inbuf, inblocks * blocksize);
      hd->count = 0;
      inlen -= inblocks * blocksize;
      inbuf += inblocks * blocksize;
    }

  if (inlen)
    {
      buf_cpy (hd->buf, inbuf, inlen);
      hd->count = inlen;
    }
}

static void
keccak_extract_s390x (void *context, void *outbuf_arg, size_t outlen)
{
  KECCAK_CONTEXT *hd = context;
  const size_t blocksize = hd->blocksize;
  byte *outbuf = outbuf_arg;

  while (outlen)
    {
      gcry_assert(hd->count == 0 || hd->buf_pos < hd->count);

      if (hd->buf_pos < hd->count && outlen)
	{
	  size_t copylen = hd->count - hd->buf_pos;

	  if (copylen > outlen)
	    copylen = outlen;

	  buf_cpy (outbuf, &hd->buf[hd->buf_pos], copylen);

	  outbuf += copylen;
	  outlen -= copylen;
	  hd->buf_pos += copylen;
	}

      if (hd->buf_pos == hd->count)
	{
	  hd->buf_pos = 0;
	  hd->count = 0;
	}

      if (outlen == 0)
	return;

      if (outlen >= blocksize)
	{
	  size_t outblocks = outlen / blocksize;

	  keccak_bextract_s390x (context, outbuf, outblocks * blocksize);

	  outlen -= outblocks * blocksize;
	  outbuf += outblocks * blocksize;

	  if (outlen == 0)
	    return;
	}

      keccak_bextract_s390x (context, hd->buf, blocksize);
      hd->count = blocksize;
    }
}
#endif /* USE_S390X_CRYPTO */


static void
keccak_write (void *context, const void *inbuf_arg, size_t inlen)
{
  KECCAK_CONTEXT *ctx = context;
  const size_t bsize = ctx->blocksize;
  const size_t blocklanes = bsize / 8;
  const byte *inbuf = inbuf_arg;
  unsigned int nburn, burn = 0;
  unsigned int count, i;
  unsigned int pos;
  size_t nlanes;

#ifdef USE_S390X_CRYPTO
  if (ctx->kimd_func)
    {
      keccak_write_s390x (context, inbuf, inlen);
      return;
    }
#endif

  count = ctx->count;

  if (inlen && (count % 8))
    {
      byte lane[8] = { 0, };

      /* Complete absorbing partial input lane. */

      pos = count / 8;

      for (i = count % 8; inlen && i < 8; i++)
	{
	  lane[i] = *inbuf++;
	  inlen--;
	  count++;
	}

      if (count == bsize)
	count = 0;

      nburn = ctx->ops->absorb(&ctx->state, pos, lane, 1,
			       (count % 8) ? -1 : blocklanes);
      burn = nburn > burn ? nburn : burn;
    }

  /* Absorb full input lanes. */

  pos = count / 8;
  nlanes = inlen / 8;
  if (nlanes > 0)
    {
      nburn = ctx->ops->absorb(&ctx->state, pos, inbuf, nlanes, blocklanes);
      burn = nburn > burn ? nburn : burn;
      inlen -= nlanes * 8;
      inbuf += nlanes * 8;
      count = ((size_t) count + nlanes * 8) % bsize;
    }

  if (inlen)
    {
      byte lane[8] = { 0, };

      /* Absorb remaining partial input lane. */

      pos = count / 8;

      for (i = count % 8; inlen && i < 8; i++)
	{
	  lane[i] = *inbuf++;
	  inlen--;
	  count++;
	}

      nburn = ctx->ops->absorb(&ctx->state, pos, lane, 1, -1);
      burn = nburn > burn ? nburn : burn;

      gcry_assert(count < bsize);
    }

  ctx->count = count;

  if (burn)
    _gcry_burn_stack (burn);
}


static void
keccak_init (int algo, void *context, unsigned int flags)
{
  KECCAK_CONTEXT *ctx = context;
  KECCAK_STATE *hd = &ctx->state;
  unsigned int features = _gcry_get_hw_features ();

  (void)flags;
  (void)features;

  memset (hd, 0, sizeof *hd);

  ctx->count = 0;

  /* Select generic implementation. */
#ifdef USE_64BIT
  ctx->ops = &keccak_generic64_ops;
#elif defined USE_32BIT
  ctx->ops = &keccak_generic32bi_ops;
#endif

  /* Select optimized implementation based in hw features. */
  if (0) {}
#ifdef USE_64BIT_ARM_NEON
  else if (features & HWF_ARM_NEON)
    ctx->ops = &keccak_armv7_neon_64_ops;
#endif
#ifdef USE_64BIT_BMI2
  else if (features & HWF_INTEL_BMI2)
    ctx->ops = &keccak_bmi2_64_ops;
#endif
#ifdef USE_32BIT_BMI2
  else if (features & HWF_INTEL_BMI2)
    ctx->ops = &keccak_bmi2_32bi_ops;
#endif
#ifdef USE_64BIT_SHLD
  else if (features & HWF_INTEL_FAST_SHLD)
    ctx->ops = &keccak_shld_64_ops;
#endif

  /* Set input block size, in Keccak terms this is called 'rate'. */

  switch (algo)
    {
    case GCRY_MD_SHA3_224:
      ctx->suffix = SHA3_DELIMITED_SUFFIX;
      ctx->blocksize = 1152 / 8;
      ctx->outlen = 224 / 8;
      break;
    case GCRY_MD_SHA3_256:
      ctx->suffix = SHA3_DELIMITED_SUFFIX;
      ctx->blocksize = 1088 / 8;
      ctx->outlen = 256 / 8;
      break;
    case GCRY_MD_SHA3_384:
      ctx->suffix = SHA3_DELIMITED_SUFFIX;
      ctx->blocksize = 832 / 8;
      ctx->outlen = 384 / 8;
      break;
    case GCRY_MD_SHA3_512:
      ctx->suffix = SHA3_DELIMITED_SUFFIX;
      ctx->blocksize = 576 / 8;
      ctx->outlen = 512 / 8;
      break;
    case GCRY_MD_SHAKE128:
      ctx->suffix = SHAKE_DELIMITED_SUFFIX;
      ctx->blocksize = 1344 / 8;
      ctx->outlen = 0;
      break;
    case GCRY_MD_SHAKE256:
      ctx->suffix = SHAKE_DELIMITED_SUFFIX;
      ctx->blocksize = 1088 / 8;
      ctx->outlen = 0;
      break;
    default:
      BUG();
    }

#ifdef USE_S390X_CRYPTO
  ctx->kimd_func = 0;
  if ((features & HWF_S390X_MSA) != 0)
    {
      unsigned int kimd_func = 0;

      switch (algo)
	{
	case GCRY_MD_SHA3_224:
	  kimd_func = KMID_FUNCTION_SHA3_224;
	  break;
	case GCRY_MD_SHA3_256:
	  kimd_func = KMID_FUNCTION_SHA3_256;
	  break;
	case GCRY_MD_SHA3_384:
	  kimd_func = KMID_FUNCTION_SHA3_384;
	  break;
	case GCRY_MD_SHA3_512:
	  kimd_func = KMID_FUNCTION_SHA3_512;
	  break;
	case GCRY_MD_SHAKE128:
	  kimd_func = KMID_FUNCTION_SHAKE128;
	  break;
	case GCRY_MD_SHAKE256:
	  kimd_func = KMID_FUNCTION_SHAKE256;
	  break;
	}

      if ((kimd_query () & km_function_to_mask (kimd_func)) &&
	  (klmd_query () & km_function_to_mask (kimd_func)))
	{
	  ctx->kimd_func = kimd_func;
	}
    }
#endif
}

static void
sha3_224_init (void *context, unsigned int flags)
{
  keccak_init (GCRY_MD_SHA3_224, context, flags);
}

static void
sha3_256_init (void *context, unsigned int flags)
{
  keccak_init (GCRY_MD_SHA3_256, context, flags);
}

static void
sha3_384_init (void *context, unsigned int flags)
{
  keccak_init (GCRY_MD_SHA3_384, context, flags);
}

static void
sha3_512_init (void *context, unsigned int flags)
{
  keccak_init (GCRY_MD_SHA3_512, context, flags);
}

static void
shake128_init (void *context, unsigned int flags)
{
  keccak_init (GCRY_MD_SHAKE128, context, flags);
}

static void
shake256_init (void *context, unsigned int flags)
{
  keccak_init (GCRY_MD_SHAKE256, context, flags);
}

/* The routine final terminates the computation and
 * returns the digest.
 * The handle is prepared for a new cycle, but adding bytes to the
 * handle will the destroy the returned buffer.
 * Returns: 64 bytes representing the digest.  When used for sha384,
 * we take the leftmost 48 of those bytes.
 */
static void
keccak_final (void *context)
{
  KECCAK_CONTEXT *ctx = context;
  KECCAK_STATE *hd = &ctx->state;
  const size_t bsize = ctx->blocksize;
  const byte suffix = ctx->suffix;
  unsigned int nburn, burn = 0;
  unsigned int lastbytes;
  byte lane[8];

#ifdef USE_S390X_CRYPTO
  if (ctx->kimd_func)
    {
      keccak_final_s390x (context);
      return;
    }
#endif

  lastbytes = ctx->count;

  /* Do the padding and switch to the squeezing phase */

  /* Absorb the last few bits and add the first bit of padding (which
     coincides with the delimiter in delimited suffix) */
  buf_put_le64(lane, (u64)suffix << ((lastbytes % 8) * 8));
  nburn = ctx->ops->absorb(&ctx->state, lastbytes / 8, lane, 1, -1);
  burn = nburn > burn ? nburn : burn;

  /* Add the second bit of padding. */
  buf_put_le64(lane, (u64)0x80 << (((bsize - 1) % 8) * 8));
  nburn = ctx->ops->absorb(&ctx->state, (bsize - 1) / 8, lane, 1, -1);
  burn = nburn > burn ? nburn : burn;

  if (suffix == SHA3_DELIMITED_SUFFIX)
    {
      /* Switch to the squeezing phase. */
      nburn = ctx->ops->permute(hd);
      burn = nburn > burn ? nburn : burn;

      /* Squeeze out the SHA3 digest. */
      nburn = ctx->ops->extract(hd, 0, (void *)hd, ctx->outlen);
      burn = nburn > burn ? nburn : burn;
    }
  else
    {
      /* Output for SHAKE can now be read with md_extract(). */

      ctx->count = 0;
    }

  wipememory(lane, sizeof(lane));
  if (burn)
    _gcry_burn_stack (burn);
}


static byte *
keccak_read (void *context)
{
  KECCAK_CONTEXT *ctx = (KECCAK_CONTEXT *) context;
  KECCAK_STATE *hd = &ctx->state;
  return (byte *)&hd->u;
}


static void
keccak_extract (void *context, void *out, size_t outlen)
{
  KECCAK_CONTEXT *ctx = context;
  KECCAK_STATE *hd = &ctx->state;
  const size_t bsize = ctx->blocksize;
  unsigned int nburn, burn = 0;
  byte *outbuf = out;
  unsigned int nlanes;
  unsigned int nleft;
  unsigned int count;
  unsigned int i;
  byte lane[8];

#ifdef USE_S390X_CRYPTO
  if (ctx->kimd_func)
    {
      keccak_extract_s390x (context, out, outlen);
      return;
    }
#endif

  count = ctx->count;

  while (count && outlen && (outlen < 8 || count % 8))
    {
      /* Extract partial lane. */
      nburn = ctx->ops->extract(hd, count / 8, lane, 8);
      burn = nburn > burn ? nburn : burn;

      for (i = count % 8; outlen && i < 8; i++)
	{
	  *outbuf++ = lane[i];
	  outlen--;
	  count++;
	}

      gcry_assert(count <= bsize);

      if (count == bsize)
	count = 0;
    }

  if (outlen >= 8 && count)
    {
      /* Extract tail of partial block. */
      nlanes = outlen / 8;
      nleft = (bsize - count) / 8;
      nlanes = nlanes < nleft ? nlanes : nleft;

      nburn = ctx->ops->extract(hd, count / 8, outbuf, nlanes * 8);
      burn = nburn > burn ? nburn : burn;
      outlen -= nlanes * 8;
      outbuf += nlanes * 8;
      count += nlanes * 8;

      gcry_assert(count <= bsize);

      if (count == bsize)
	count = 0;
    }

  while (outlen >= bsize)
    {
      gcry_assert(count == 0);

      /* Squeeze more. */
      nburn = ctx->ops->permute(hd);
      burn = nburn > burn ? nburn : burn;

      /* Extract full block. */
      nburn = ctx->ops->extract(hd, 0, outbuf, bsize);
      burn = nburn > burn ? nburn : burn;

      outlen -= bsize;
      outbuf += bsize;
    }

  if (outlen)
    {
      gcry_assert(outlen < bsize);

      if (count == 0)
	{
	  /* Squeeze more. */
	  nburn = ctx->ops->permute(hd);
	  burn = nburn > burn ? nburn : burn;
	}

      if (outlen >= 8)
	{
	  /* Extract head of partial block. */
	  nlanes = outlen / 8;
	  nburn = ctx->ops->extract(hd, count / 8, outbuf, nlanes * 8);
	  burn = nburn > burn ? nburn : burn;
	  outlen -= nlanes * 8;
	  outbuf += nlanes * 8;
	  count += nlanes * 8;

	  gcry_assert(count < bsize);
	}

      if (outlen)
	{
	  /* Extract head of partial lane. */
	  nburn = ctx->ops->extract(hd, count / 8, lane, 8);
	  burn = nburn > burn ? nburn : burn;

	  for (i = count % 8; outlen && i < 8; i++)
	    {
	      *outbuf++ = lane[i];
	      outlen--;
	      count++;
	    }

	  gcry_assert(count < bsize);
	}
    }

  ctx->count = count;

  if (burn)
    _gcry_burn_stack (burn);
}


/* Variant of the above shortcut function using multiple buffers.  */
static void
_gcry_sha3_hash_buffers (void *outbuf, size_t nbytes, const gcry_buffer_t *iov,
			 int iovcnt, const gcry_md_spec_t *spec)
{
  KECCAK_CONTEXT hd;

  spec->init (&hd, 0);
  for (;iovcnt > 0; iov++, iovcnt--)
    keccak_write (&hd, (const char*)iov[0].data + iov[0].off, iov[0].len);
  keccak_final (&hd);
  if (spec->mdlen > 0)
    memcpy (outbuf, keccak_read (&hd), spec->mdlen);
  else
    keccak_extract (&hd, outbuf, nbytes);
}


static void
_gcry_sha3_224_hash_buffers (void *outbuf, size_t nbytes,
			     const gcry_buffer_t *iov, int iovcnt)
{
  _gcry_sha3_hash_buffers (outbuf, nbytes, iov, iovcnt,
			   &_gcry_digest_spec_sha3_224);
}

static void
_gcry_sha3_256_hash_buffers (void *outbuf, size_t nbytes,
			     const gcry_buffer_t *iov, int iovcnt)
{
  _gcry_sha3_hash_buffers (outbuf, nbytes, iov, iovcnt,
			   &_gcry_digest_spec_sha3_256);
}

static void
_gcry_sha3_384_hash_buffers (void *outbuf, size_t nbytes,
			     const gcry_buffer_t *iov, int iovcnt)
{
  _gcry_sha3_hash_buffers (outbuf, nbytes, iov, iovcnt,
			   &_gcry_digest_spec_sha3_384);
}

static void
_gcry_sha3_512_hash_buffers (void *outbuf, size_t nbytes,
			     const gcry_buffer_t *iov, int iovcnt)
{
  _gcry_sha3_hash_buffers (outbuf, nbytes, iov, iovcnt,
			   &_gcry_digest_spec_sha3_512);
}

static void
_gcry_shake128_hash_buffers (void *outbuf, size_t nbytes,
			     const gcry_buffer_t *iov, int iovcnt)
{
  _gcry_sha3_hash_buffers (outbuf, nbytes, iov, iovcnt,
			   &_gcry_digest_spec_shake128);
}

static void
_gcry_shake256_hash_buffers (void *outbuf, size_t nbytes,
			     const gcry_buffer_t *iov, int iovcnt)
{
  _gcry_sha3_hash_buffers (outbuf, nbytes, iov, iovcnt,
			   &_gcry_digest_spec_shake256);
}


/*
     Self-test section.
 */


static gpg_err_code_t
selftests_keccak (int algo, int extended, selftest_report_func_t report)
{
  const char *what;
  const char *errtxt;
  const char *short_hash;
  const char *long_hash;
  const char *one_million_a_hash;
  int hash_len;

  switch (algo)
  {
    default:
      BUG();

    case GCRY_MD_SHA3_224:
      short_hash =
	"\xe6\x42\x82\x4c\x3f\x8c\xf2\x4a\xd0\x92\x34\xee\x7d\x3c\x76\x6f"
	"\xc9\xa3\xa5\x16\x8d\x0c\x94\xad\x73\xb4\x6f\xdf";
      long_hash =
	"\x54\x3e\x68\x68\xe1\x66\x6c\x1a\x64\x36\x30\xdf\x77\x36\x7a\xe5"
	"\xa6\x2a\x85\x07\x0a\x51\xc1\x4c\xbf\x66\x5c\xbc";
      one_million_a_hash =
	"\xd6\x93\x35\xb9\x33\x25\x19\x2e\x51\x6a\x91\x2e\x6d\x19\xa1\x5c"
	"\xb5\x1c\x6e\xd5\xc1\x52\x43\xe7\xa7\xfd\x65\x3c";
      hash_len = 28;
      break;

    case GCRY_MD_SHA3_256:
      short_hash =
	"\x3a\x98\x5d\xa7\x4f\xe2\x25\xb2\x04\x5c\x17\x2d\x6b\xd3\x90\xbd"
	"\x85\x5f\x08\x6e\x3e\x9d\x52\x5b\x46\xbf\xe2\x45\x11\x43\x15\x32";
      long_hash =
	"\x91\x6f\x60\x61\xfe\x87\x97\x41\xca\x64\x69\xb4\x39\x71\xdf\xdb"
	"\x28\xb1\xa3\x2d\xc3\x6c\xb3\x25\x4e\x81\x2b\xe2\x7a\xad\x1d\x18";
      one_million_a_hash =
	"\x5c\x88\x75\xae\x47\x4a\x36\x34\xba\x4f\xd5\x5e\xc8\x5b\xff\xd6"
	"\x61\xf3\x2a\xca\x75\xc6\xd6\x99\xd0\xcd\xcb\x6c\x11\x58\x91\xc1";
      hash_len = 32;
      break;

    case GCRY_MD_SHA3_384:
      short_hash =
	"\xec\x01\x49\x82\x88\x51\x6f\xc9\x26\x45\x9f\x58\xe2\xc6\xad\x8d"
	"\xf9\xb4\x73\xcb\x0f\xc0\x8c\x25\x96\xda\x7c\xf0\xe4\x9b\xe4\xb2"
	"\x98\xd8\x8c\xea\x92\x7a\xc7\xf5\x39\xf1\xed\xf2\x28\x37\x6d\x25";
      long_hash =
	"\x79\x40\x7d\x3b\x59\x16\xb5\x9c\x3e\x30\xb0\x98\x22\x97\x47\x91"
	"\xc3\x13\xfb\x9e\xcc\x84\x9e\x40\x6f\x23\x59\x2d\x04\xf6\x25\xdc"
	"\x8c\x70\x9b\x98\xb4\x3b\x38\x52\xb3\x37\x21\x61\x79\xaa\x7f\xc7";
      one_million_a_hash =
	"\xee\xe9\xe2\x4d\x78\xc1\x85\x53\x37\x98\x34\x51\xdf\x97\xc8\xad"
	"\x9e\xed\xf2\x56\xc6\x33\x4f\x8e\x94\x8d\x25\x2d\x5e\x0e\x76\x84"
	"\x7a\xa0\x77\x4d\xdb\x90\xa8\x42\x19\x0d\x2c\x55\x8b\x4b\x83\x40";
      hash_len = 48;
      break;

    case GCRY_MD_SHA3_512:
      short_hash =
	"\xb7\x51\x85\x0b\x1a\x57\x16\x8a\x56\x93\xcd\x92\x4b\x6b\x09\x6e"
	"\x08\xf6\x21\x82\x74\x44\xf7\x0d\x88\x4f\x5d\x02\x40\xd2\x71\x2e"
	"\x10\xe1\x16\xe9\x19\x2a\xf3\xc9\x1a\x7e\xc5\x76\x47\xe3\x93\x40"
	"\x57\x34\x0b\x4c\xf4\x08\xd5\xa5\x65\x92\xf8\x27\x4e\xec\x53\xf0";
      long_hash =
	"\xaf\xeb\xb2\xef\x54\x2e\x65\x79\xc5\x0c\xad\x06\xd2\xe5\x78\xf9"
	"\xf8\xdd\x68\x81\xd7\xdc\x82\x4d\x26\x36\x0f\xee\xbf\x18\xa4\xfa"
	"\x73\xe3\x26\x11\x22\x94\x8e\xfc\xfd\x49\x2e\x74\xe8\x2e\x21\x89"
	"\xed\x0f\xb4\x40\xd1\x87\xf3\x82\x27\x0c\xb4\x55\xf2\x1d\xd1\x85";
      one_million_a_hash =
	"\x3c\x3a\x87\x6d\xa1\x40\x34\xab\x60\x62\x7c\x07\x7b\xb9\x8f\x7e"
	"\x12\x0a\x2a\x53\x70\x21\x2d\xff\xb3\x38\x5a\x18\xd4\xf3\x88\x59"
	"\xed\x31\x1d\x0a\x9d\x51\x41\xce\x9c\xc5\xc6\x6e\xe6\x89\xb2\x66"
	"\xa8\xaa\x18\xac\xe8\x28\x2a\x0e\x0d\xb5\x96\xc9\x0b\x0a\x7b\x87";
      hash_len = 64;
      break;

    case GCRY_MD_SHAKE128:
      short_hash =
	"\x58\x81\x09\x2d\xd8\x18\xbf\x5c\xf8\xa3\xdd\xb7\x93\xfb\xcb\xa7"
	"\x40\x97\xd5\xc5\x26\xa6\xd3\x5f\x97\xb8\x33\x51\x94\x0f\x2c\xc8";
      long_hash =
	"\x7b\x6d\xf6\xff\x18\x11\x73\xb6\xd7\x89\x8d\x7f\xf6\x3f\xb0\x7b"
	"\x7c\x23\x7d\xaf\x47\x1a\x5a\xe5\x60\x2a\xdb\xcc\xef\x9c\xcf\x4b";
      one_million_a_hash =
	"\x9d\x22\x2c\x79\xc4\xff\x9d\x09\x2c\xf6\xca\x86\x14\x3a\xa4\x11"
	"\xe3\x69\x97\x38\x08\xef\x97\x09\x32\x55\x82\x6c\x55\x72\xef\x58";
      hash_len = 32;
      break;

    case GCRY_MD_SHAKE256:
      short_hash =
	"\x48\x33\x66\x60\x13\x60\xa8\x77\x1c\x68\x63\x08\x0c\xc4\x11\x4d"
	"\x8d\xb4\x45\x30\xf8\xf1\xe1\xee\x4f\x94\xea\x37\xe7\x8b\x57\x39";
      long_hash =
	"\x98\xbe\x04\x51\x6c\x04\xcc\x73\x59\x3f\xef\x3e\xd0\x35\x2e\xa9"
	"\xf6\x44\x39\x42\xd6\x95\x0e\x29\xa3\x72\xa6\x81\xc3\xde\xaf\x45";
      one_million_a_hash =
	"\x35\x78\xa7\xa4\xca\x91\x37\x56\x9c\xdf\x76\xed\x61\x7d\x31\xbb"
	"\x99\x4f\xca\x9c\x1b\xbf\x8b\x18\x40\x13\xde\x82\x34\xdf\xd1\x3a";
      hash_len = 32;
      break;
  }

  what = "short string";
  errtxt = _gcry_hash_selftest_check_one (algo, 0, "abc", 3, short_hash,
					  hash_len);
  if (errtxt)
    goto failed;

  if (extended)
    {
      what = "long string";
      errtxt = _gcry_hash_selftest_check_one
	(algo, 0,
	"abcdefghbcdefghicdefghijdefghijkefghijklfghijklmghijklmn"
	"hijklmnoijklmnopjklmnopqklmnopqrlmnopqrsmnopqrstnopqrstu", 112,
	long_hash, hash_len);
      if (errtxt)
	goto failed;

      what = "one million \"a\"";
      errtxt = _gcry_hash_selftest_check_one (algo, 1, NULL, 0,
					      one_million_a_hash, hash_len);
      if (errtxt)
	goto failed;
    }

  return 0; /* Succeeded. */

failed:
  if (report)
    report ("digest", algo, what, errtxt);
  return GPG_ERR_SELFTEST_FAILED;
}


/* Run a full self-test for ALGO and return 0 on success.  */
static gpg_err_code_t
run_selftests (int algo, int extended, selftest_report_func_t report)
{
  gpg_err_code_t ec;

  switch (algo)
    {
    case GCRY_MD_SHA3_224:
    case GCRY_MD_SHA3_256:
    case GCRY_MD_SHA3_384:
    case GCRY_MD_SHA3_512:
    case GCRY_MD_SHAKE128:
    case GCRY_MD_SHAKE256:
      ec = selftests_keccak (algo, extended, report);
      break;
    default:
      ec = GPG_ERR_DIGEST_ALGO;
      break;
    }

  return ec;
}




static const byte sha3_224_asn[] = { 0x30 };
static const gcry_md_oid_spec_t oid_spec_sha3_224[] =
  {
    { "2.16.840.1.101.3.4.2.7" },
    /* PKCS#1 sha3_224WithRSAEncryption */
    { "?" },
    { NULL }
  };
static const byte sha3_256_asn[] = { 0x30 };
static const gcry_md_oid_spec_t oid_spec_sha3_256[] =
  {
    { "2.16.840.1.101.3.4.2.8" },
    /* PKCS#1 sha3_256WithRSAEncryption */
    { "?" },
    { NULL }
  };
static const byte sha3_384_asn[] = { 0x30 };
static const gcry_md_oid_spec_t oid_spec_sha3_384[] =
  {
    { "2.16.840.1.101.3.4.2.9" },
    /* PKCS#1 sha3_384WithRSAEncryption */
    { "?" },
    { NULL }
  };
static const byte sha3_512_asn[] = { 0x30 };
static const gcry_md_oid_spec_t oid_spec_sha3_512[] =
  {
    { "2.16.840.1.101.3.4.2.10" },
    /* PKCS#1 sha3_512WithRSAEncryption */
    { "?" },
    { NULL }
  };
static const byte shake128_asn[] = { 0x30 };
static const gcry_md_oid_spec_t oid_spec_shake128[] =
  {
    { "2.16.840.1.101.3.4.2.11" },
    /* PKCS#1 shake128WithRSAEncryption */
    { "?" },
    { NULL }
  };
static const byte shake256_asn[] = { 0x30 };
static const gcry_md_oid_spec_t oid_spec_shake256[] =
  {
    { "2.16.840.1.101.3.4.2.12" },
    /* PKCS#1 shake256WithRSAEncryption */
    { "?" },
    { NULL }
  };

const gcry_md_spec_t _gcry_digest_spec_sha3_224 =
  {
    GCRY_MD_SHA3_224, {0, 1},
    "SHA3-224", sha3_224_asn, DIM (sha3_224_asn), oid_spec_sha3_224, 28,
    sha3_224_init, keccak_write, keccak_final, keccak_read, NULL,
    _gcry_sha3_224_hash_buffers,
    sizeof (KECCAK_CONTEXT),
    run_selftests
  };
const gcry_md_spec_t _gcry_digest_spec_sha3_256 =
  {
    GCRY_MD_SHA3_256, {0, 1},
    "SHA3-256", sha3_256_asn, DIM (sha3_256_asn), oid_spec_sha3_256, 32,
    sha3_256_init, keccak_write, keccak_final, keccak_read, NULL,
    _gcry_sha3_256_hash_buffers,
    sizeof (KECCAK_CONTEXT),
    run_selftests
  };
const gcry_md_spec_t _gcry_digest_spec_sha3_384 =
  {
    GCRY_MD_SHA3_384, {0, 1},
    "SHA3-384", sha3_384_asn, DIM (sha3_384_asn), oid_spec_sha3_384, 48,
    sha3_384_init, keccak_write, keccak_final, keccak_read, NULL,
    _gcry_sha3_384_hash_buffers,
    sizeof (KECCAK_CONTEXT),
    run_selftests
  };
const gcry_md_spec_t _gcry_digest_spec_sha3_512 =
  {
    GCRY_MD_SHA3_512, {0, 1},
    "SHA3-512", sha3_512_asn, DIM (sha3_512_asn), oid_spec_sha3_512, 64,
    sha3_512_init, keccak_write, keccak_final, keccak_read, NULL,
    _gcry_sha3_512_hash_buffers,
    sizeof (KECCAK_CONTEXT),
    run_selftests
  };
const gcry_md_spec_t _gcry_digest_spec_shake128 =
  {
    GCRY_MD_SHAKE128, {0, 1},
    "SHAKE128", shake128_asn, DIM (shake128_asn), oid_spec_shake128, 0,
    shake128_init, keccak_write, keccak_final, NULL, keccak_extract,
    _gcry_shake128_hash_buffers,
    sizeof (KECCAK_CONTEXT),
    run_selftests
  };
const gcry_md_spec_t _gcry_digest_spec_shake256 =
  {
    GCRY_MD_SHAKE256, {0, 1},
    "SHAKE256", shake256_asn, DIM (shake256_asn), oid_spec_shake256, 0,
    shake256_init, keccak_write, keccak_final, NULL, keccak_extract,
    _gcry_shake256_hash_buffers,
    sizeof (KECCAK_CONTEXT),
    run_selftests
  };

/* crc-armv8-aarch64-ce.S - ARMv8/CE PMULL accelerated CRC implementation
 * Copyright (C) 2019 Jussi Kivilinna <jussi.kivilinna@iki.fi>
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

#include "asm-common-aarch64.h"

#if defined(__AARCH64EL__) && \
    defined(HAVE_COMPATIBLE_GCC_AARCH64_PLATFORM_AS) && \
    defined(HAVE_GCC_INLINE_ASM_AARCH64_CRYPTO)

.cpu generic+simd+crypto

.text


/* Structure of crc32_consts_s */

#define consts_k(idx)    ((idx) * 8)
#define consts_my_p(idx) (consts_k(6) + (idx) * 8)

/* Constants */

.align 6
.Lcrc32_constants:
.Lcrc32_partial_fold_input_mask:
  .byte 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00
  .byte 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00
.Lcrc32_refl_shuf_shift:
  .byte 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff
  .byte 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff
  .byte 0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07
  .byte 0x08, 0x09, 0x0a, 0x0b, 0x0c, 0x0d, 0x0e, 0x0f
.Lcrc32_shuf_shift:
  .byte 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff
  .byte 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff
.Lcrc32_bswap_shuf:
  .byte 0x0f, 0x0e, 0x0d, 0x0c, 0x0b, 0x0a, 0x09, 0x08
  .byte 0x07, 0x06, 0x05, 0x04, 0x03, 0x02, 0x01, 0x00
  .byte 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff
  .byte 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff


/*
 * void _gcry_crc32r_armv8_ce_bulk (u32 *pcrc, const byte *inbuf, size_t inlen,
 *                                  const struct crc32_consts_s *consts);
 */
.align 3
.globl _gcry_crc32r_armv8_ce_bulk
ELF(.type  _gcry_crc32r_armv8_ce_bulk,%function;)
_gcry_crc32r_armv8_ce_bulk:
  /* input:
   *    x0: pcrc
   *    x1: inbuf
   *    x2: inlen
   *    x3: consts
   */
  CFI_STARTPROC()

  GET_DATA_POINTER(x7, .Lcrc32_constants)
  add x9, x3, #consts_k(5 - 1)
  cmp x2, #128

  b.lo .Lcrc32r_fold_by_one_setup

  eor v4.16b, v4.16b, v4.16b
  add x4, x3, #consts_k(1 - 1)
  ld1 {v4.s}[0], [x0]             /* load pcrc */
  ld1 {v0.16b-v3.16b}, [x1], #64  /* load 64 bytes of input */
  sub x2, x2, #64
  ld1 {v6.16b}, [x4]
  eor v0.16b, v0.16b, v4.16b

  add x4, x3, #consts_k(3 - 1)
  add x5, x3, #consts_my_p(0)

.Lcrc32r_fold_by_four:

  /* Fold by 4. */
  ld1 {v16.16b-v19.16b}, [x1], #64 /* load 64 bytes of input */
  sub x2, x2, #64
  pmull v20.1q, v0.1d, v6.1d
  pmull v21.1q, v1.1d, v6.1d
  pmull v22.1q, v2.1d, v6.1d
  pmull v23.1q, v3.1d, v6.1d
  cmp x2, #64
  pmull2 v24.1q, v0.2d, v6.2d
  pmull2 v25.1q, v1.2d, v6.2d
  pmull2 v26.1q, v2.2d, v6.2d
  pmull2 v27.1q, v3.2d, v6.2d
  eor v0.16b, v20.16b, v16.16b
  eor v1.16b, v21.16b, v17.16b
  eor v2.16b, v22.16b, v18.16b
  eor v3.16b, v23.16b, v19.16b
  eor v0.16b, v0.16b, v24.16b
  eor v1.16b, v1.16b, v25.16b
  eor v2.16b, v2.16b, v26.16b
  eor v3.16b, v3.16b, v27.16b
  b.hs .Lcrc32r_fold_by_four

  ld1 {v6.16b}, [x4]
  ld1 {v5.16b}, [x5]

  cmp x2, #16

  /* Fold 4 to 1. */

  pmull v16.1q, v0.1d, v6.1d
  pmull2 v4.1q, v0.2d, v6.2d
  eor v0.16b, v16.16b, v1.16b
  eor v0.16b, v0.16b, v4.16b

  pmull v16.1q, v0.1d, v6.1d
  pmull2 v4.1q, v0.2d, v6.2d
  eor v0.16b, v16.16b, v2.16b
  eor v0.16b, v0.16b, v4.16b

  pmull v16.1q, v0.1d, v6.1d
  pmull2 v4.1q, v0.2d, v6.2d
  eor v0.16b, v16.16b, v3.16b
  eor v0.16b, v0.16b, v4.16b

  b.lo .Lcrc32r_fold_by_one_done
  b .Lcrc32r_fold_by_one

.Lcrc32r_fold_by_one_setup:

  eor v1.16b, v1.16b, v1.16b
  add x4, x3, #consts_k(3 - 1)
  add x5, x3, #consts_my_p(0)
  sub x2, x2, #16
  ld1 {v1.s}[0], [x0]             /* load pcrc */
  ld1 {v0.16b}, [x1], #16         /* load 16 bytes of input */
  cmp x2, #16
  ld1 {v6.16b}, [x4]              /* load k3k4 */
  ld1 {v5.16b}, [x5]              /* load my_p */
  eor v0.16b, v0.16b, v1.16b
  b.lo .Lcrc32r_fold_by_one_done

.Lcrc32r_fold_by_one:
  sub x2, x2, #16
  ld1 {v2.16b}, [x1], #16         /* load 16 bytes of input */
  pmull v3.1q, v0.1d, v6.1d
  pmull2 v1.1q, v0.2d, v6.2d
  cmp x2, #16
  eor v0.16b, v3.16b, v2.16b
  eor v0.16b, v0.16b, v1.16b

  b.hs .Lcrc32r_fold_by_one

.Lcrc32r_fold_by_one_done:

  cmp x2, #0
  b.eq .Lcrc32r_final_fold

  /* Partial fold. */

  add x4, x7, #.Lcrc32_refl_shuf_shift - .Lcrc32_constants
  add x5, x7, #.Lcrc32_refl_shuf_shift - .Lcrc32_constants + 16
  add x6, x7, #.Lcrc32_partial_fold_input_mask - .Lcrc32_constants
  sub x8, x2, #16
  add x4, x4, x2
  add x5, x5, x2
  add x6, x6, x2
  add x8, x1, x8

  /* Load last input and add padding zeros. */
  ld1 {v4.16b}, [x4]
  eor x2, x2, x2
  ld1 {v3.16b}, [x5]
  ld1 {v2.16b}, [x6]
  tbl v30.16b, {v0.16b}, v4.16b
  ld1 {v4.16b}, [x8]
  tbl v1.16b, {v0.16b}, v3.16b

  pmull v0.1q, v30.1d, v6.1d
  and v2.16b, v2.16b, v4.16b
  pmull2 v31.1q, v30.2d, v6.2d
  orr v2.16b, v2.16b, v1.16b
  eor v0.16b, v0.16b, v31.16b
  eor v0.16b, v0.16b, v2.16b

.Lcrc32r_final_fold:

  /* Final fold. */

  eor v2.16b, v2.16b, v2.16b      /* zero reg */
  ld1 {v7.16b}, [x9]

  /* reduce 128-bits to 96-bits */
  ext v6.16b, v6.16b, v6.16b, #8  /* swap high and low parts */
  mov v1.16b, v0.16b
  pmull v0.1q, v0.1d, v6.1d
  ext v6.16b, v5.16b, v5.16b, #8  /* swap high and low parts */
  ext v1.16b, v1.16b, v2.16b, #8  /* high to low, high zeroed */
  eor v3.16b, v0.16b, v1.16b

  /* reduce 96-bits to 64-bits */
  eor v1.16b, v1.16b, v1.16b
  ext v0.16b, v3.16b, v2.16b, #4  /* [00][00][x2][x1] */
  mov v1.s[0], v3.s[0]            /* [00][00][00][x0] */
  eor v3.16b, v3.16b, v3.16b
  pmull v1.1q, v1.1d, v7.1d       /* [00][00][xx][xx] */
  eor v0.16b, v0.16b, v1.16b      /* top 64-bit are zero */

  /* barrett reduction */
  mov v3.s[1], v0.s[0]            /* [00][00][x1][00] */
  ext v0.16b, v2.16b, v0.16b, #12 /* [??][x1][??][00] */
  pmull v1.1q, v3.1d, v5.1d       /* [00][xx][xx][00] */
  pmull v1.1q, v1.1d, v6.1d       /* [00][xx][xx][00] */
  eor v0.16b, v0.16b, v1.16b

  /* store CRC */
  st1 {v0.s}[2], [x0]

  ret_spec_stop
  CFI_ENDPROC()
ELF(.size _gcry_crc32r_armv8_ce_bulk,.-_gcry_crc32r_armv8_ce_bulk;)

/*
 * void _gcry_crc32r_armv8_ce_reduction_4 (u32 *pcrc, u32 data, u32 crc,
 *                                         const struct crc32_consts_s *consts);
 */
.align 3
.globl _gcry_crc32r_armv8_ce_reduction_4
ELF(.type  _gcry_crc32r_armv8_ce_reduction_4,%function;)
_gcry_crc32r_armv8_ce_reduction_4:
  /* input:
   *    w0: data
   *    w1: crc
   *    x2: crc32 constants
   */
  CFI_STARTPROC()

  eor v0.16b, v0.16b, v0.16b
  add x2, x2, #consts_my_p(0)
  eor v1.16b, v1.16b, v1.16b
  ld1 {v5.16b}, [x2]

  mov v0.s[0], w0
  pmull v0.1q, v0.1d, v5.1d     /* [00][00][xx][xx] */
  mov v1.s[1], w1
  mov v0.s[2], v0.s[0]          /* [00][x0][x1][x0] */
  pmull2 v0.1q, v0.2d, v5.2d    /* [00][00][xx][xx] */
  eor v0.16b, v0.16b, v1.16b

  mov w0, v0.s[1]

  ret_spec_stop
  CFI_ENDPROC()
ELF(.size _gcry_crc32r_armv8_ce_reduction_4,.-_gcry_crc32r_armv8_ce_reduction_4;)

/*
 * void _gcry_crc32_armv8_ce_bulk (u32 *pcrc, const byte *inbuf, size_t inlen,
 *                                 const struct crc32_consts_s *consts);
 */
.align 3
.globl _gcry_crc32_armv8_ce_bulk
ELF(.type  _gcry_crc32_armv8_ce_bulk,%function;)
_gcry_crc32_armv8_ce_bulk:
  /* input:
   *    x0: pcrc
   *    x1: inbuf
   *    x2: inlen
   *    x3: consts
   */
  CFI_STARTPROC()

  GET_DATA_POINTER(x7, .Lcrc32_constants)
  add x4, x7, #.Lcrc32_bswap_shuf - .Lcrc32_constants
  cmp x2, #128
  ld1 {v7.16b}, [x4]

  b.lo .Lcrc32_fold_by_one_setup

  eor v4.16b, v4.16b, v4.16b
  add x4, x3, #consts_k(1 - 1)
  ld1 {v4.s}[0], [x0]            /* load pcrc */
  ld1 {v0.16b-v3.16b}, [x1], #64 /* load 64 bytes of input */
  sub x2, x2, #64
  ld1 {v6.16b}, [x4]
  eor v0.16b, v0.16b, v4.16b
  ext v4.16b, v6.16b, v6.16b, #8
  tbl v0.16b, { v0.16b }, v7.16b /* byte swap */
  tbl v1.16b, { v1.16b }, v7.16b /* byte swap */
  tbl v2.16b, { v2.16b }, v7.16b /* byte swap */
  tbl v3.16b, { v3.16b }, v7.16b /* byte swap */

  add x4, x3, #consts_k(3 - 1)
  add x5, x3, #consts_my_p(0)

.Lcrc32_fold_by_four:

  /* Fold by 4. */
  ld1 {v16.16b-v19.16b}, [x1], #64 /* load 64 bytes of input */
  sub x2, x2, #64
  tbl v16.16b, { v16.16b }, v7.16b /* byte swap */
  tbl v17.16b, { v17.16b }, v7.16b /* byte swap */
  tbl v18.16b, { v18.16b }, v7.16b /* byte swap */
  tbl v19.16b, { v19.16b }, v7.16b /* byte swap */
  cmp x2, #64
  pmull2 v20.1q, v0.2d, v4.2d
  pmull2 v21.1q, v1.2d, v4.2d
  pmull2 v22.1q, v2.2d, v4.2d
  pmull2 v23.1q, v3.2d, v4.2d
  pmull v24.1q, v0.1d, v4.1d
  pmull v25.1q, v1.1d, v4.1d
  pmull v26.1q, v2.1d, v4.1d
  pmull v27.1q, v3.1d, v4.1d
  eor v0.16b, v20.16b, v16.16b
  eor v1.16b, v21.16b, v17.16b
  eor v2.16b, v22.16b, v18.16b
  eor v3.16b, v23.16b, v19.16b
  eor v0.16b, v0.16b, v24.16b
  eor v1.16b, v1.16b, v25.16b
  eor v2.16b, v2.16b, v26.16b
  eor v3.16b, v3.16b, v27.16b
  b.hs .Lcrc32_fold_by_four

  ld1 {v6.16b}, [x4]
  ld1 {v5.16b}, [x5]
  ext v6.16b, v6.16b, v6.16b, #8
  ext v5.16b, v5.16b, v5.16b, #8

  cmp x2, #16

  /* Fold 4 to 1. */

  pmull2 v16.1q, v0.2d, v6.2d
  pmull v4.1q, v0.1d, v6.1d
  eor v0.16b, v16.16b, v1.16b
  eor v0.16b, v0.16b, v4.16b

  pmull2 v16.1q, v0.2d, v6.2d
  pmull v4.1q, v0.1d, v6.1d
  eor v0.16b, v16.16b, v2.16b
  eor v0.16b, v0.16b, v4.16b

  pmull2 v16.1q, v0.2d, v6.2d
  pmull v4.1q, v0.1d, v6.1d
  eor v0.16b, v16.16b, v3.16b
  eor v0.16b, v0.16b, v4.16b

  b.lo .Lcrc32_fold_by_one_done
  b .Lcrc32_fold_by_one

.Lcrc32_fold_by_one_setup:

  eor v1.16b, v1.16b, v1.16b
  add x4, x3, #consts_k(3 - 1)
  add x5, x3, #consts_my_p(0)
  ld1 {v1.s}[0], [x0]            /* load pcrc */
  sub x2, x2, #16
  ld1 {v0.16b}, [x1], #16        /* load 16 bytes of input */
  ld1 {v6.16b}, [x4]             /* load k3k4 */
  ld1 {v5.16b}, [x5]             /* load my_p */
  eor v0.16b, v0.16b, v1.16b
  cmp x2, #16
  ext v6.16b, v6.16b, v6.16b, #8 /* swap high and low parts */
  ext v5.16b, v5.16b, v5.16b, #8 /* swap high and low parts */
  tbl v0.16b, { v0.16b }, v7.16b /* byte swap */
  b.lo .Lcrc32_fold_by_one_done

.Lcrc32_fold_by_one:
  sub x2, x2, #16
  ld1 {v2.16b}, [x1], #16        /* load 16 bytes of input */
  pmull2 v3.1q, v0.2d, v6.2d
  tbl v2.16b, { v2.16b }, v7.16b /* byte swap */
  pmull v1.1q, v0.1d, v6.1d
  cmp x2, #16
  eor v0.16b, v3.16b, v2.16b
  eor v0.16b, v0.16b, v1.16b

  b.hs .Lcrc32_fold_by_one

.Lcrc32_fold_by_one_done:

  cmp x2, #0
  b.eq .Lcrc32_final_fold

  /* Partial fold. */

  add x4, x7, #.Lcrc32_refl_shuf_shift - .Lcrc32_constants + 32
  add x5, x7, #.Lcrc32_shuf_shift - .Lcrc32_constants + 16
  add x6, x7, #.Lcrc32_partial_fold_input_mask - .Lcrc32_constants
  sub x8, x2, #16
  sub x4, x4, x2
  add x5, x5, x2
  add x6, x6, x2
  add x8, x1, x8

  /* Load last input and add padding zeros. */
  ld1 {v4.16b}, [x4]
  eor x2, x2, x2
  ld1 {v3.16b}, [x5]
  ld1 {v2.16b}, [x6]
  tbl v30.16b, {v0.16b}, v4.16b
  ld1 {v4.16b}, [x8]
  tbl v1.16b, {v0.16b}, v3.16b
  and v2.16b, v2.16b, v4.16b

  pmull2 v0.1q, v30.2d, v6.2d
  orr v2.16b, v2.16b, v1.16b
  pmull v1.1q, v30.1d, v6.1d
  tbl v2.16b, {v2.16b}, v7.16b   /* byte swap */
  eor v0.16b, v0.16b, v1.16b
  eor v0.16b, v0.16b, v2.16b

.Lcrc32_final_fold:

  /* Final fold. */

  eor v2.16b, v2.16b, v2.16b     /* zero reg */

  /* reduce 128-bits to 96-bits */
  add x4, x3, #consts_k(4)
  ext v3.16b, v6.16b, v6.16b, #8 /* swap high and low parts */
  eor v6.16b, v6.16b, v6.16b
  mov v1.16b, v0.16b
  pmull2 v0.1q, v0.2d, v3.2d
  ld1 {v6.d}[1], [x4]            /* load k4 */
  ext v1.16b, v2.16b, v1.16b, #8 /* low to high, low zeroed */
  eor v3.16b, v0.16b, v1.16b     /* bottom 32-bit are zero */

  /* reduce 96-bits to 64-bits */
  eor v0.16b, v0.16b, v0.16b
  eor v1.16b, v1.16b, v1.16b
  mov v0.s[1], v3.s[1]           /* [00][00][x1][00] */
  mov v1.s[2], v3.s[3]           /* [00][x3][00][00] */
  mov v0.s[2], v3.s[2]           /* [00][x2][x1][00] */
  eor v3.16b, v3.16b, v3.16b
  pmull2 v1.1q, v1.2d, v6.2d     /* [00][xx][xx][00] */
  eor v0.16b, v0.16b, v1.16b     /* top and bottom 32-bit are zero */

  /* barrett reduction */
  mov v3.s[0], v0.s[1]           /* [00][00][00][x1] */
  pmull2 v0.1q, v0.2d, v5.2d     /* [00][xx][xx][xx] */
  ext v0.16b, v0.16b, v2.16b, #4 /* [00][00][xx][xx] */
  pmull v0.1q, v0.1d, v5.1d
  eor v0.16b, v0.16b, v3.16b

  /* store CRC in input endian */
  rev32 v0.8b, v0.8b             /* byte swap */
  st1 {v0.s}[0], [x0]

  ret_spec_stop
  CFI_ENDPROC()
ELF(.size _gcry_crc32_armv8_ce_bulk,.-_gcry_crc32_armv8_ce_bulk;)

/*
 * void _gcry_crc32_armv8_ce_reduction_4 (u32 *pcrc, u32 data, u32 crc,
 *                                        const struct crc32_consts_s *consts);
 */
.align 3
.globl _gcry_crc32_armv8_ce_reduction_4
ELF(.type  _gcry_crc32_armv8_ce_reduction_4,%function;)
_gcry_crc32_armv8_ce_reduction_4:
  /* input:
   *    w0: data
   *    w1: crc
   *    x2: crc32 constants
   */
  CFI_STARTPROC()

  eor v0.16b, v0.16b, v0.16b
  add x2, x2, #consts_my_p(0)
  eor v1.16b, v1.16b, v1.16b
  ld1 {v5.16b}, [x2]

  mov v0.s[1], w0
  pmull v0.1q, v0.1d, v5.1d     /* [00][xx][xx][00] */
  mov v1.s[0], w1
  pmull2 v0.1q, v0.2d, v5.2d    /* [00][00][xx][xx] */
  eor v0.16b, v0.16b, v1.16b

  rev32 v0.8b, v0.8b            /* Return in input endian */
  mov w0, v0.s[0]

  ret_spec_stop
  CFI_ENDPROC()
ELF(.size _gcry_crc32_armv8_ce_reduction_4,.-_gcry_crc32_armv8_ce_reduction_4;)

#endif

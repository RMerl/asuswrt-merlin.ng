/* Copyright (c) 2025, The Tor Project, Inc. */
/* See LICENSE for licensing information */

/**
 * \file polyval.h
 * \brief Implementation for polyval universal hash function.
 *
 * Polyval, which was first defined for AES-GCM-SIV, is a
 * universal hash based on multiplication in GF(2^128).
 * Unlike the more familiar GHASH, it is defined to work on
 * little-endian inputs, and so is more straightforward and efficient
 * on little-endian architectures.
 *
 * In Tor we use it as part of the Counter Galois Onion relay
 * encryption format.
 **/

/* Implementation notes:
 *
 * Our implementation is based on the GHASH code from BearSSL
 * by Thomas Pornin.  There are three implementations:
 *
 * pclmul.c -- An x86-only version, based on the CLMUL instructions
 * introduced for westlake processors in 2010.
 *
 * ctmul64.c -- A portable contant-time implementation for 64-bit
 * processors.
 *
 * ctmul.c -- A portable constant-time implementation for 32-bit
 * processors with an efficient 32x32->64 multiply instruction.
 *
 * Note that the "ctmul" implementations are only constant-time
 * if the corresponding CPU multiply and rotate instructions are
 * also constant-time.  But that's an architectural assumption we
 * make in Tor.
 */

#include "ext/polyval/polyval.h"
#include "lib/cc/compat_compiler.h"

#include <string.h>

#ifdef PV_USE_PCLMUL_DETECT
#include <cpuid.h>
#endif

typedef pv_u128_ u128;

/* ========
 * We declare these functions, to help implement polyval.
 *
 * They have different definitions depending on our representation
 * of 128-bit integers.
 */
#if 0
/**
 * Read a u128-bit little-endian integer from 'bytes',
 * which may not be aligned.
 */
static inline u128 u128_from_bytes(const uint8_t *bytes);
/**
 * Store a u128-bit little-endian integer to 'bytes_out',
 * which may not be aligned.
 */
static inline void u128_to_bytes(u128, uint8_t *bytes_out);
/**
 * XOR a u128 into the y field of a polyval_t struct.
 *
 * (Within the polyval struct, perform "y ^= v").
 */
static inline void pv_xor_y(polyval_t *, u128 v);

/* ========
 * The function which we expect our backend to implement.
 */
/**
 * Within the polyval struct, perform "y *= h".
 *
 * (This is a carryless multiply in the Polyval galois field)
 */
static void pv_mul_y_h(polyval_t *);h
#endif

/* =====
 * Endianness conversion for big-endian platforms
 */
#ifdef WORDS_BIGENDIAN
#ifdef __GNUC__
#define bswap64(x) __builtin_bswap64(x)
#define bswap32(x) __builtin_bswap32(x)
#else
/* The (compiler should optimize these into a decent bswap instruction) */
static inline uint64_t
bswap64(uint64_t value)
{
  return
    ((value & 0xFF00000000000000) >> 56) |
    ((value & 0x00FF000000000000) >> 40) |
    ((value & 0x0000FF0000000000) >> 24) |
    ((value & 0x000000FF00000000) >> 8) |
    ((value & 0x00000000FF000000) << 8) |
    ((value & 0x0000000000FF0000) << 24) |
    ((value & 0x000000000000FF00) << 40) |
    ((value & 0x00000000000000FF) << 56);
}

static inline uint32_t
bswap32(uint32_t value)
{
  return
    ((value & 0xFF000000) >> 24) |
    ((value & 0x00FF0000) >> 8) |
    ((value & 0x0000FF00) << 8) |
    ((value & 0x000000FF) << 24);
}
#endif
#endif

#ifdef WORDS_BIGENDIAN
#define convert_byte_order64(x) bswap64(x)
#define convert_byte_order32(x) bswap32(x)
#else
#define convert_byte_order64(x) (x)
#define convert_byte_order32(x) (x)
#endif

#if defined PV_USE_PCLMUL_UNCONDITIONAL
#define PCLMUL_MEMBER(v) (v)
#define PV_USE_PCLMUL

#elif defined PV_USE_PCLMUL_DETECT
#define PCLMUL_MEMBER(v) (v).u128x1
#define CTMUL64_MEMBER(v) (v).u64x2
#define PV_USE_PCLMUL
#define PV_USE_CTMUL64

#elif defined PV_USE_CTMUL64
#define CTMUL64_MEMBER(v) (v)
#endif

#ifdef PV_USE_PCLMUL
#include "ext/polyval/pclmul.c"

DISABLE_GCC_WARNING("-Waggregate-return")
static inline u128
u128_from_bytes_pclmul(const uint8_t *bytes)
{
  u128 r;
  PCLMUL_MEMBER(r) = _mm_loadu_si128((const __m128i*)bytes);
  return r;
}
ENABLE_GCC_WARNING("-Waggregate-return")

static inline void
u128_to_bytes_pclmul(u128 val, uint8_t *bytes_out)
{
  _mm_storeu_si128((__m128i*)bytes_out, PCLMUL_MEMBER(val));
}
static inline void
pv_xor_y_pclmul(polyval_t *pv, u128 v)
{
  PCLMUL_MEMBER(pv->y) = _mm_xor_si128(PCLMUL_MEMBER(pv->y),
                                       PCLMUL_MEMBER(v));
}
#endif

#if defined(PV_USE_CTMUL64)
#include "ext/polyval/ctmul64.c"

DISABLE_GCC_WARNING("-Waggregate-return")
static inline u128
u128_from_bytes_ctmul64(const uint8_t *bytes)
{
  u128 r;
  memcpy(&CTMUL64_MEMBER(r).lo, bytes, 8);
  memcpy(&CTMUL64_MEMBER(r).hi, bytes + 8, 8);
  CTMUL64_MEMBER(r).lo = convert_byte_order64(CTMUL64_MEMBER(r).lo);
  CTMUL64_MEMBER(r).hi = convert_byte_order64(CTMUL64_MEMBER(r).hi);
  return r;
}
ENABLE_GCC_WARNING("-Waggregate-return")

static inline void
u128_to_bytes_ctmul64(u128 val, uint8_t *bytes_out)
{
  uint64_t lo = convert_byte_order64(CTMUL64_MEMBER(val).lo);
  uint64_t hi = convert_byte_order64(CTMUL64_MEMBER(val).hi);
  memcpy(bytes_out, &lo, 8);
  memcpy(bytes_out + 8, &hi, 8);
}
static inline void
pv_xor_y_ctmul64(polyval_t *pv, u128 val)
{
  CTMUL64_MEMBER(pv->y).lo ^= CTMUL64_MEMBER(val).lo;
  CTMUL64_MEMBER(pv->y).hi ^= CTMUL64_MEMBER(val).hi;
}
#endif

#if defined(PV_USE_CTMUL)
#include "ext/polyval/ctmul.c"

DISABLE_GCC_WARNING("-Waggregate-return")
static inline u128
u128_from_bytes_ctmul(const uint8_t *bytes)
{
  u128 r;
  memcpy(&r.v, bytes, 16);
  for (int i = 0; i < 4; ++i) {
    r.v[i] = convert_byte_order32(r.v[i]);
  }
  return r;
}
ENABLE_GCC_WARNING("-Waggregate-return")

static inline void
u128_to_bytes_ctmul(u128 val, uint8_t *bytes_out)
{
  uint32_t v[4];
  for (int i = 0; i < 4; ++i) {
    v[i] = convert_byte_order32(val.v[i]);
  }
  memcpy(bytes_out, v, 16);
}
static inline void
pv_xor_y_ctmul(polyval_t *pv, u128 val)
{
  for (int i = 0; i < 4; ++i) {
    pv->y.v[i] ^= val.v[i];
  }
}
#endif

struct expanded_key_none {};
static inline void add_multiple_none(polyval_t *pv,
                                     const uint8_t *input,
                                     const struct expanded_key_none *expanded)
{
  (void) pv;
  (void) input;
  (void) expanded;
}
static inline void expand_key_none(const polyval_t *inp,
                                   struct expanded_key_none *out)
{
  (void) inp;
  (void) out;
}

/* Kludge: a special value to use for block_stride when we don't support
 * processing multiple blocks at once.  Previously we used 0, but that
 * caused warnings with some comparisons. */
#define BLOCK_STRIDE_NONE  0xffff

DISABLE_GCC_WARNING("-Waggregate-return")
#define PV_DECLARE(prefix,                                              \
                   st,                                                  \
                   u128_from_bytes,                                     \
                   u128_to_bytes,                                       \
                   pv_xor_y,                                            \
                   pv_mul_y_h,                                          \
                   block_stride,                                        \
                   expanded_key_tp, expand_fn, add_multiple_fn)         \
  st void                                                               \
  prefix ## polyval_key_init(polyval_key_t *pvk, const uint8_t *key)    \
  {                                                                     \
    pvk->h = u128_from_bytes(key);                                      \
  }                                                                     \
  st void                                                               \
  prefix ## polyval_init(polyval_t *pv, const uint8_t *key)             \
  {                                                                     \
    polyval_key_init(&pv->key, key);                                    \
    memset(&pv->y, 0, sizeof(u128));                                    \
  }                                                                     \
  st void                                                               \
  prefix ## polyval_init_from_key(polyval_t *pv, const polyval_key_t *key) \
  {                                                                     \
    memcpy(&pv->key, key, sizeof(polyval_key_t));                       \
    memset(&pv->y, 0, sizeof(u128));                                    \
  }                                                                     \
  st void                                                               \
  prefix ## polyval_add_block(polyval_t *pv, const uint8_t *block)      \
  {                                                                     \
    u128 b = u128_from_bytes(block);                                    \
    pv_xor_y(pv, b);                                                    \
    pv_mul_y_h(pv);                                                     \
  }                                                                     \
  st void                                                               \
  prefix ## polyval_add_zpad(polyval_t *pv, const uint8_t *data, size_t n) \
  {                                                                     \
    /* since block_stride is a constant, this should get optimized */   \
    if ((block_stride != BLOCK_STRIDE_NONE)                             \
        && n >= (block_stride) * 16) {                                  \
      expanded_key_tp expanded_key;                                     \
      expand_fn(pv, &expanded_key);                                     \
      while (n >= (block_stride) * 16) {                                \
        add_multiple_fn(pv, data, &expanded_key);                       \
        n -= block_stride*16;                                           \
        data += block_stride * 16;                                      \
      }                                                                 \
    }                                                                   \
    while (n > 16) {                                                    \
      polyval_add_block(pv, data);                                      \
      data += 16;                                                       \
      n -= 16;                                                          \
    }                                                                   \
    if (n) {                                                            \
      uint8_t block[16];                                                \
      memset(&block, 0, sizeof(block));                                 \
      memcpy(block, data, n);                                           \
      polyval_add_block(pv, block);                                     \
    }                                                                   \
  }                                                                     \
  st void                                                               \
  prefix ## polyval_get_tag(const polyval_t *pv, uint8_t *tag_out)      \
  {                                                                     \
    u128_to_bytes(pv->y, tag_out);                                      \
  }                                                                     \
  st void                                                               \
  prefix ## polyval_reset(polyval_t *pv)                                \
  {                                                                     \
    memset(&pv->y, 0, sizeof(u128));                                    \
  }
ENABLE_GCC_WARNING("-Waggregate-return")

#ifdef PV_USE_PCLMUL_DETECT
/* We use a boolean to distinguish whether to use the PCLMUL instructions,
 * but instead we could use function pointers.  It's probably worth
 * benchmarking, though it's unlikely to make a measurable difference.
 */
static bool use_pclmul = false;

/* Declare _both_ variations of our code, statically,
 * with different prefixes. */
DISABLE_GCC_WARNING("-Waggregate-return")
PV_DECLARE(pclmul_, static,
           u128_from_bytes_pclmul,
           u128_to_bytes_pclmul,
           pv_xor_y_pclmul,
           pv_mul_y_h_pclmul,
           PV_BLOCK_STRIDE,
           pv_expanded_key_t,
           expand_key_pclmul,
           pv_add_multiple_pclmul)

PV_DECLARE(ctmul64_, static,
           u128_from_bytes_ctmul64,
           u128_to_bytes_ctmul64,
           pv_xor_y_ctmul64,
           pv_mul_y_h_ctmul64,
           BLOCK_STRIDE_NONE,
           struct expanded_key_none,
           expand_key_none,
           add_multiple_none)
ENABLE_GCC_WARNING("-Waggregate-return")

void
polyval_key_init(polyval_key_t *pv, const uint8_t *key)
{
  if (use_pclmul)
    pclmul_polyval_key_init(pv, key);
  else
    ctmul64_polyval_key_init(pv, key);
}
void
polyval_init(polyval_t *pv, const uint8_t *key)
{
  if (use_pclmul)
    pclmul_polyval_init(pv, key);
  else
    ctmul64_polyval_init(pv, key);
}
void
polyval_init_from_key(polyval_t *pv, const polyval_key_t *key)
{
  if (use_pclmul)
    pclmul_polyval_init_from_key(pv, key);
  else
    ctmul64_polyval_init_from_key(pv, key);
}
void
polyval_add_block(polyval_t *pv, const uint8_t *block)
{
  if (use_pclmul)
    pclmul_polyval_add_block(pv, block);
  else
    ctmul64_polyval_add_block(pv, block);
}
void
polyval_add_zpad(polyval_t *pv, const uint8_t *data, size_t n)
{
  if (use_pclmul)
    pclmul_polyval_add_zpad(pv, data, n);
  else
    ctmul64_polyval_add_zpad(pv, data, n);
}
void
polyval_get_tag(const polyval_t *pv, uint8_t *tag_out)
{
  if (use_pclmul)
    pclmul_polyval_get_tag(pv, tag_out);
  else
    ctmul64_polyval_get_tag(pv, tag_out);
}
void
polyval_reset(polyval_t *pv)
{
  if (use_pclmul)
    pclmul_polyval_reset(pv);
  else
    ctmul64_polyval_reset(pv);
}

#elif defined(PV_USE_PCLMUL)
PV_DECLARE(, ,
           u128_from_bytes_pclmul,
           u128_to_bytes_pclmul,
           pv_xor_y_pclmul,
           pv_mul_y_h_pclmul,
           PV_BLOCK_STRIDE,
           pv_expanded_key_t,
           expand_key_pclmul,
           pv_add_multiple_pclmul)
#elif defined(PV_USE_CTMUL64)
PV_DECLARE(, ,
           u128_from_bytes_ctmul64,
           u128_to_bytes_ctmul64,
           pv_xor_y_ctmul64,
           pv_mul_y_h_ctmul64,
           BLOCK_STRIDE_NONE,
           struct expanded_key_none,
           expand_key_none,
           add_multiple_none)

#elif defined(PV_USE_CTMUL)
DISABLE_GCC_WARNING("-Waggregate-return")
PV_DECLARE(, , u128_from_bytes_ctmul,
           u128_to_bytes_ctmul,
           pv_xor_y_ctmul,
           pv_mul_y_h_ctmul,
           BLOCK_STRIDE_NONE,
           struct expanded_key_none,
           expand_key_none,
           add_multiple_none)
ENABLE_GCC_WARNING("-Waggregate-return")
#endif

#ifdef PV_USE_PCLMUL_DETECT
void
polyval_detect_implementation(void)
{
  unsigned int eax, ebx, ecx, edx;
  use_pclmul = false;
  if (__get_cpuid(1, &eax, &ebx, &ecx, &edx)) {
    if (0 != (ecx & bit_PCLMUL)) {
      use_pclmul = true;
    }
  }
}
#else
void
polyval_detect_implementation(void)
{
}
#endif

#ifdef POLYVAL_USE_EXPANDED_KEYS

#ifdef PV_USE_PCLMUL_DETECT
#define SHOULD_EXPAND() (use_pclmul)
#else
#define SHOULD_EXPAND() (1)
#endif

void
polyvalx_init(polyvalx_t *pvx, const uint8_t *key)
{
  polyval_init(&pvx->pv, key);
  if (SHOULD_EXPAND()) {
    expand_key_pclmul(&pvx->pv, &pvx->expanded);
  }
}
void
polyvalx_init_from_key(polyvalx_t *pvx, const polyval_key_t *key)
{
  polyval_init_from_key(&pvx->pv, key);
  if (SHOULD_EXPAND()) {
    expand_key_pclmul(&pvx->pv, &pvx->expanded);
  }
}
void
polyvalx_add_block(polyvalx_t *pvx, const uint8_t *block)
{
  polyval_add_block(&pvx->pv, block);
}
void
polyvalx_add_zpad(polyvalx_t *pvx, const uint8_t *data, size_t n)
{
  if (SHOULD_EXPAND() && n >= PV_BLOCK_STRIDE * 16) {
    while (n > PV_BLOCK_STRIDE * 16) {
      pv_add_multiple_pclmul(&pvx->pv, data, &pvx->expanded);
      data += PV_BLOCK_STRIDE * 16;
      n -= PV_BLOCK_STRIDE * 16;
    }
  }
  while (n > 16) {
    polyval_add_block(&pvx->pv, data);
    data += 16;
    n -= 16;
  }
  if (n) {
    uint8_t block[16];
    memset(&block, 0, sizeof(block));
    memcpy(block, data, n);
    polyval_add_block(&pvx->pv, block);
  }
}
void
polyvalx_get_tag(const polyvalx_t *pvx, uint8_t *tag_out)
{
  polyval_get_tag(&pvx->pv, tag_out);
}
void polyvalx_reset(polyvalx_t *pvx)
{
  polyval_reset(&pvx->pv);
}
#endif

#if 0
#include <stdio.h>
int
main(int c, char **v)
{
  // From RFC 8452 appendix A
  uint8_t key[16] =
    { 0x25, 0x62, 0x93, 0x47, 0x58, 0x92, 0x42, 0x76,
      0x1d, 0x31, 0xf8, 0x26, 0xba, 0x4b, 0x75, 0x7b  };
  uint8_t block1[16] =
    { 0x4f, 0x4f, 0x95, 0x66, 0x8c, 0x83, 0xdf, 0xb6,
      0x40, 0x17, 0x62, 0xbb, 0x2d, 0x01, 0xa2, 0x62 };
  uint8_t block2[16] = {
    0xd1, 0xa2, 0x4d, 0xdd, 0x27, 0x21, 0xd0, 0x06,
    0xbb, 0xe4, 0x5f, 0x20, 0xd3, 0xc9, 0xf3, 0x62 };
  uint8_t expect_result[16] = {
    0xf7, 0xa3, 0xb4, 0x7b, 0x84, 0x61, 0x19, 0xfa,
    0xe5, 0xb7, 0x86, 0x6c, 0xf5, 0xe5, 0xb7, 0x7e };

  polyval_t pv;
  uint8_t tag[16];
  polyval_init(&pv, key);
  polyval_add_block(&pv, block1);
  polyval_add_block(&pv, block2);
  polyval_get_tag(&pv, tag);
  if (!memcmp(expect_result, tag, 16)) {
    puts("OK");
    return 0;
  }  else {
    puts("NOPE");
    return 1;
  }
}
#endif

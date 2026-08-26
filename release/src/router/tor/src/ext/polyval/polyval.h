/* Copyright (c) 2025, The Tor Project, Inc. */
/* See LICENSE for licensing information */

/**
 * \file polyval.h
 * \brief APIs for polyval universal hash function.
 **/

#ifndef TOR_POLYVAL_H
#define TOR_POLYVAL_H

#include "orconfig.h"
#include "lib/cc/torint.h"

/* Decide which implementation to use. */
#if defined(__amd64__) || defined(__amd64) || defined(__x86_64__) \
  || defined(_M_X64) || defined(_M_IX86) || defined(__i486)       \
  || defined(__i386__)
#define PV_INTEL_ARCH
#endif

#if defined(PV_INTEL_ARCH) && defined(__PCLMUL__)
/* We're building for an architecture that always has the intel
 * intrinsics for carryless multiply.
 * No need for runtime detection.
 */
#define PV_USE_PCLMUL_UNCONDITIONAL
#define PCLMUL_ANY

#elif defined(PV_INTEL_ARCH) && SIZEOF_VOID_P >= 8
/* We _might_ have PCLMUL, or we might not.
 * We need to detect it at runtime.
 */
#define PV_USE_PCLMUL_DETECT
#define PCLMUL_ANY

#elif SIZEOF_VOID_P >= 8
/* It's a 64-bit architecture; use the generic 64-bit constant-time
 * implementation.
 */
#define PV_USE_CTMUL64
#elif SIZEOF_VOID_P == 4
/* It's a 64-bit architecture; use the generic 32-bit constant-time
 * implementation.
 */
#define PV_USE_CTMUL
#else
#error "sizeof(void*) is implausibly weird."
#endif

#ifdef PCLMUL_ANY
#include <emmintrin.h>

#define POLYVAL_USE_EXPANDED_KEYS
#endif

/**
 * Declare a 128 bit integer type.
 # The exact representation will depend on which implementation we've chosen.
 */
#if defined(PV_USE_PCLMUL_UNCONDITIONAL)
typedef __m128i pv_u128_;
#elif defined(PV_USE_PCLMUL_DETECT)
typedef union pv_u128_ {
  __m128i u128x1;
  struct {
    uint64_t lo;
    uint64_t hi;
  } u64x2;
} pv_u128_;
#elif defined(PV_USE_CTMUL64)
typedef struct pv_u128_ {
  uint64_t lo;
  uint64_t hi;
} pv_u128_;
#elif defined(PV_USE_CTMUL)
typedef struct pv_u128_ {
  uint32_t v[4];
} pv_u128_;
#endif

/** A key for a polyval hash, plus any precomputed key material. */
typedef struct polyval_key_t {
  pv_u128_ h;
} polyval_key_t;

/**
 * State for an instance of the polyval hash.
 **/
typedef struct polyval_t {
  /** The key used for this instance of polyval. */
  polyval_key_t key;
  /** The accumulator */
  pv_u128_ y;
} polyval_t;

/**
 * Length of a polyval key, in bytes.
 */
#define POLYVAL_KEY_LEN 16
/**
 * Length of a polyval block, in bytes.
 */
#define POLYVAL_BLOCK_LEN 16
/**
 * Length of a polyval tag (output), in bytes.
 */
#define POLYVAL_TAG_LEN 16

/** Do any necessary precomputation from a polyval key,
 * and store it.
 */
void polyval_key_init(polyval_key_t *, const uint8_t *key);
/**
 * Initialize a polyval instance with a given key.
 */
void polyval_init(polyval_t *, const uint8_t *key);
/**
 * Initialize a polyval instance with a preconstructed key.
 */
void polyval_init_from_key(polyval_t *, const polyval_key_t *key);
/**
 * Update a polyval instance with a new 16-byte block.
 */
void polyval_add_block(polyval_t *, const uint8_t *block);
/**
 * Update a polyval instance with 'n' bytes from 'data'.
 * If 'n' is not evenly divisible by 16, pad it at the end with zeros.
 *
 * NOTE: This is not a general-purpose padding construction;
 * it can be insecure if your are using it in context where the input length
 * is variable.
 */
void polyval_add_zpad(polyval_t *, const uint8_t *data, size_t n);
/**
 * Copy the 16-byte tag from a polyval instance into 'tag_out'
 */
void polyval_get_tag(const polyval_t *, uint8_t *tag_out);
/**
 * Reset a polyval instance to its original state,
 * retaining its key.
 */
void polyval_reset(polyval_t *);

/** If a faster-than-default polyval implementation is available, use it. */
void polyval_detect_implementation(void);

#ifdef POLYVAL_USE_EXPANDED_KEYS
/* These variations are as for polyval_\*, but they use pre-expanded keys.
 * They're appropriate when you know a key is likely to get used more than once
 * on a large input.
 */

/** How many blocks to handle at once with an expanded key */
#define PV_BLOCK_STRIDE 8
typedef struct pv_expanded_key_t {
  // powers of h in reverse order, down to 2.
  // (in other words, contains
  // h^PCLMUL_BLOCK_STRIDE .. H^2)
  __m128i k[PV_BLOCK_STRIDE-1];
} pv_expanded_key_t;
typedef struct polyvalx_t {
  polyval_t pv;
  pv_expanded_key_t expanded;
} polyvalx_t;

void polyvalx_init(polyvalx_t *, const uint8_t *key);
void polyvalx_init_from_key(polyvalx_t *, const polyval_key_t *key);
void polyvalx_add_block(polyvalx_t *, const uint8_t *block);
void polyvalx_add_zpad(polyvalx_t *, const uint8_t *data, size_t n);
void polyvalx_get_tag(const polyvalx_t *, uint8_t *tag_out);
void polyvalx_reset(polyvalx_t *);

#else
#define polyvalx_t polyval_t
#define polyvalx_key_init polyval_key_init
#define polyvalx_init polyval_init
#define polyvalx_init_from_key polyval_init_from_key
#define polyvalx_add_block polyval_add_block
#define polyvalx_add_zpad polyval_add_zpad
#define polyvalx_get_tag polyval_get_tag
#define polyvalx_reset polyval_reset
#endif

#endif

/* Copyright (c) 2001, Matej Pfajfar.
 * Copyright (c) 2001-2004, Roger Dingledine.
 * Copyright (c) 2004-2006, Roger Dingledine, Nick Mathewson.
 * Copyright (c) 2007-2019, The Tor Project, Inc. */
/* See LICENSE for licensing information */

/**
 * \file crypto_rand.h
 *
 * \brief Common functions for using (pseudo-)random number generators.
 **/

#ifndef TOR_CRYPTO_RAND_H
#define TOR_CRYPTO_RAND_H

#include "lib/cc/compat_compiler.h"
#include "lib/cc/torint.h"
#include "lib/testsupport/testsupport.h"

/* random numbers */
int crypto_seed_rng(void) ATTR_WUR;
MOCK_DECL(void,crypto_rand,(char *to, size_t n));
void crypto_rand_unmocked(char *to, size_t n);
void crypto_strongest_rand(uint8_t *out, size_t out_len);
MOCK_DECL(void,crypto_strongest_rand_,(uint8_t *out, size_t out_len));
int crypto_rand_int(unsigned int max);
int crypto_rand_int_range(unsigned int min, unsigned int max);
uint64_t crypto_rand_uint64_range(uint64_t min, uint64_t max);
time_t crypto_rand_time_range(time_t min, time_t max);
uint64_t crypto_rand_uint64(uint64_t max);
double crypto_rand_double(void);
struct tor_weak_rng_t;
void crypto_seed_weak_rng(struct tor_weak_rng_t *rng);

char *crypto_random_hostname(int min_rand_len, int max_rand_len,
                             const char *prefix, const char *suffix);

struct smartlist_t;
void *smartlist_choose(const struct smartlist_t *sl);
void smartlist_shuffle(struct smartlist_t *sl);
int crypto_force_rand_ssleay(void);

#ifdef CRYPTO_RAND_PRIVATE

STATIC int crypto_strongest_rand_raw(uint8_t *out, size_t out_len);

#ifdef TOR_UNIT_TESTS
extern int break_strongest_rng_syscall;
extern int break_strongest_rng_fallback;
#endif
#endif /* defined(CRYPTO_RAND_PRIVATE) */

#endif /* !defined(TOR_CRYPTO_RAND_H) */

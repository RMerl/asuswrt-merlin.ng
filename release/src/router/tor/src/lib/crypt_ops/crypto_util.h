/* Copyright (c) 2001, Matej Pfajfar.
 * Copyright (c) 2001-2004, Roger Dingledine.
 * Copyright (c) 2004-2006, Roger Dingledine, Nick Mathewson.
 * Copyright (c) 2007-2019, The Tor Project, Inc. */
/* See LICENSE for licensing information */

/**
 * \file crypto_util.h
 *
 * \brief Common functions for cryptographic routines.
 **/

#ifndef TOR_CRYPTO_UTIL_H
#define TOR_CRYPTO_UTIL_H

#include "lib/cc/torint.h"

/** OpenSSL-based utility functions. */
void memwipe(void *mem, uint8_t byte, size_t sz);

#endif /* !defined(TOR_CRYPTO_UTIL_H) */

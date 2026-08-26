/* Copyright (c) 2003, Roger Dingledine
 * Copyright (c) 2004-2006, Roger Dingledine, Nick Mathewson.
 * Copyright (c) 2007-2021, The Tor Project, Inc. */
/* See LICENSE for licensing information */

/* Implements a minimal interface to counter-mode AES. */

#ifndef TOR_AES_H
#define TOR_AES_H

/**
 * \file aes.h
 * \brief Headers for aes.c
 */

#include "lib/cc/torint.h"
#include "lib/malloc/malloc.h"
#include "lib/testsupport/testsupport.h"

typedef struct aes_cnt_cipher_t aes_cnt_cipher_t;

aes_cnt_cipher_t* aes_new_cipher(const uint8_t *key, const uint8_t *iv,
                                 int key_bits);
void aes_cipher_set_iv_aligned(aes_cnt_cipher_t *cipher_, const uint8_t *iv);
void aes_cipher_set_key(aes_cnt_cipher_t *cipher_,
                        const uint8_t *key, int key_bits);
void aes_cipher_free_(aes_cnt_cipher_t *cipher);
#define aes_cipher_free(cipher) \
  FREE_AND_NULL(aes_cnt_cipher_t, aes_cipher_free_, (cipher))
void aes_crypt_inplace(aes_cnt_cipher_t *cipher, char *data, size_t len);

#ifdef USE_AES_RAW
typedef struct aes_raw_t aes_raw_t;

aes_raw_t *aes_raw_new(const uint8_t *key, int key_bits, bool encrypt);
void aes_raw_set_key(aes_raw_t **cipher, const uint8_t *key,
                     int key_bits, bool encrypt);
void aes_raw_free_(aes_raw_t *cipher);
#define aes_raw_free(cipher) \
  FREE_AND_NULL(aes_raw_t, aes_raw_free_, (cipher))
void aes_raw_encrypt(const aes_raw_t *cipher, uint8_t *block);
void aes_raw_decrypt(const aes_raw_t *cipher, uint8_t *block);
#endif

#endif /* !defined(TOR_AES_H) */

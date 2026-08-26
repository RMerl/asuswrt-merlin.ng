/* Copyright (c) 2001 Matej Pfajfar.
 * Copyright (c) 2001-2004, Roger Dingledine.
 * Copyright (c) 2004-2006, Roger Dingledine, Nick Mathewson.
 * Copyright (c) 2007-2025, The Tor Project, Inc. */
/* See LICENSE for licensing information */

/**
 * \file relay_crypto_cgo.c
 * \brief Implementation for counter galois onion encryption.
 **/

#define RELAY_CRYPTO_CGO_PRIVATE
#define USE_AES_RAW

#include "orconfig.h"
#include "core/or/or.h"
#include "lib/crypt_ops/aes.h"
#include "ext/polyval/polyval.h"
#include "lib/crypt_ops/crypto_util.h"
#include "lib/log/util_bug.h"
#include "lib/arch/bytes.h"
#include "ext/polyval/polyval.h"
#include "core/crypto/relay_crypto_cgo.h"
#include "core/crypto/relay_crypto.h"
#include "core/or/cell_st.h"

#if 0
// XXXX debugging.
#include "lib/encoding/binascii.h"
#include <stdio.h>
#endif

#include <string.h>

static int
cgo_et_keylen(int aesbits)
{
  return (aesbits / 8) + POLYVAL_KEY_LEN;
}

/** Initialize an instance of the tweakable block cipher,
 * using an 'aesbits'-bit AES key.
 *
 * The total key material used from 'key' will be
 * (aesbits / 8) + 16.
 *
 * This will be initialized for encryption or decryption depending
 * on the value of 'encrypt'
 */
STATIC int
cgo_et_init(cgo_et_t *et, int aesbits, bool encrypt,
            const uint8_t *key)
{
  size_t aes_key_bytes = aesbits / 8;
  et->kb = aes_raw_new(key, aesbits, encrypt);
  if (et->kb == NULL)
    return -1;
  polyvalx_init(&et->ku, key + aes_key_bytes);
  return 0;
}
/** Replace the key on an existing, already initialized cgo_et_t.
 *
 * Does fewer allocations than a clear+init. */
STATIC void
cgo_et_set_key(cgo_et_t *et, int aesbits, bool encrypt,
               const uint8_t *key)
{
  size_t aes_key_bytes = aesbits / 8;
  aes_raw_set_key(&et->kb, key, aesbits, encrypt);
  polyvalx_init(&et->ku, key + aes_key_bytes);
}

/** Helper: Compute polyval(KU, H | CMD | X_R). */
static inline void
compute_et_mask(polyvalx_t *pvk, const et_tweak_t tweak, uint8_t *t_out)
{
  // block 0: tweak.h
  // block 1: one byte of command, first 15 bytes of x_r
  // block 2...: remainder of x_r, zero-padded.
  polyvalx_reset(pvk);
  uint8_t block1[16];
  block1[0] = tweak.uiv.cmd;
  memcpy(block1+1, tweak.x_r, 15);
  polyvalx_add_block(pvk, tweak.uiv.h);
  polyvalx_add_block(pvk, block1);
  polyvalx_add_zpad(pvk, tweak.x_r + 15, ET_TWEAK_LEN_X_R - 15);
  polyvalx_get_tag(pvk, t_out);
}
/** XOR the 16 byte block from inp into out. */
static void
xor_block(uint8_t *out, const uint8_t *inp)
{
  for (int i = 0; i < 16; ++i)
    out[i] ^= inp[i];
}

/**
 * Encrypt the 16-byte block in 'block'.
 */
STATIC void
cgo_et_encrypt(cgo_et_t *et, const et_tweak_t tweak,
               uint8_t *block)
{
  uint8_t mask[16];
  compute_et_mask(&et->ku, tweak, mask);
  xor_block(block, mask);
  aes_raw_encrypt(et->kb, block);
  xor_block(block, mask);
}
/**
 * Decrypt the 16-byte b lock in 'block'
 */
STATIC void
cgo_et_decrypt(cgo_et_t *et, const et_tweak_t tweak,
               uint8_t *block)
{
  uint8_t mask[16];
  compute_et_mask(&et->ku, tweak, mask);
  xor_block(block, mask);
  aes_raw_decrypt(et->kb, block);
  xor_block(block, mask);
}
/**
 * Release any storage held in 'et'.
 *
 * This _doesn't_ wipe 'et'; that's done from a higher-level function.
 */
STATIC void
cgo_et_clear(cgo_et_t *et)
{
  aes_raw_free(et->kb);
}

static int
cgo_prf_keylen(int aesbits)
{
  return (aesbits / 8) + POLYVAL_KEY_LEN;
}

/**
 * Initialize a psedorandom function from a given key.
 * Uses an internal 'aesbits'-bit AES key.
 *
 * The total key material used from 'key' will be
 * (aesbits / 8) + 16.
 */
STATIC int
cgo_prf_init(cgo_prf_t *prf, int aesbits,
             const uint8_t *key)
{
  const uint8_t iv[16] = {0};
  size_t aes_key_bytes = aesbits / 8;
  memset(prf,0, sizeof(*prf));
  prf->k = aes_new_cipher(key, iv, aesbits);
  polyval_key_init(&prf->b, key + aes_key_bytes);
  return 0;
}
/** Replace the key on an existing cgo_prf_t.
 *
 * Does fewer allocations than a clear+init. */
STATIC void
cgo_prf_set_key(cgo_prf_t *prf, int aesbits,
                const uint8_t *key)
{
  size_t aes_key_bytes = aesbits / 8;
  aes_cipher_set_key(prf->k, key, aesbits);
  polyval_key_init(&prf->b, key + aes_key_bytes);
}
/**
 * Compute the PRF's results on 'input', for position t=0,
 * XOR it into 'data'.
 *
 * 'input' must be PRF_INPUT_LEN bytes long.
 *
 * 'data' must be PRF_T0_DATA_LEN bytes long.
 */
STATIC void
cgo_prf_xor_t0(cgo_prf_t *prf, const uint8_t *input,
               uint8_t *data)
{
  uint8_t hash[16];
  polyval_t pv;
  polyval_init_from_key(&pv, &prf->b);
  polyval_add_block(&pv, input);
  polyval_get_tag(&pv, hash);
  hash[15] &= 0xC0; // Clear the low six bits.

  aes_cipher_set_iv_aligned(prf->k, hash);
  aes_crypt_inplace(prf->k, (char*) data, PRF_T0_DATA_LEN);

  // Re-align the cipher.
  //
  // This approach is faster than EVP_CIPHER_set_num!
  const int ns = 16 - (PRF_T0_DATA_LEN & 0xf);
  // We're not using the hash for anything, so it's okay to overwrite
  aes_crypt_inplace(prf->k, (char*)hash,  ns);
}
/**
 * Generate 'n' bytes of the PRF's results on 'input', for position t=1,
 * and store them into 'buf'.
 *
 * 'input' must be PRF_INPUT_LEN bytes long.
 */
STATIC void
cgo_prf_gen_t1(cgo_prf_t *prf, const uint8_t *input,
               uint8_t *buf, size_t n)
{
  #define T1_OFFSET 31
  uint8_t hash[16];
  polyval_t pv;
  polyval_init_from_key(&pv, &prf->b);
  polyval_add_block(&pv, input);
  polyval_get_tag(&pv, hash);
  hash[15] &= 0xC0; // Clear the low six bits.
  hash[15] += T1_OFFSET; // Can't overflow!

  memset(buf, 0, n);
  aes_cipher_set_iv_aligned(prf->k, hash);
  aes_crypt_inplace(prf->k, (char*)buf, n);

  // Re-align the cipher.
  size_t ns = 16-(n&0x0f);
  if (ns) {
    // We're not using the hash for anything, so it's okay to overwrite
    aes_crypt_inplace(prf->k, (char*) hash, ns);
  }
}
/**
 * Release any storage held in 'prf'.
 *
 * This _doesn't_ wipe 'prf'; that's done from a higher-level function.
 */
STATIC void
cgo_prf_clear(cgo_prf_t *prf)
{
  aes_cipher_free(prf->k);
}

static int
cgo_uiv_keylen(int aesbits)
{
  return cgo_et_keylen(aesbits) + cgo_prf_keylen(aesbits);
}

/**
 * Initialize the 'uiv' wide-block cipher, using 'aesbits'-bit
 * AES keys internally.
 *
 * Initializes for encryption or decryption depending on the value of
 * 'encrypt'.
 *
 * The total key material used from 'key' will be
 * (aesbits / 8) * 2 + 32.
 */
STATIC int
cgo_uiv_init(cgo_uiv_t *uiv, int aesbits, bool encrypt,
             const uint8_t *key)
{
  size_t aes_key_bytes = aesbits / 8;
  if (cgo_et_init(&uiv->j, aesbits, encrypt, key) < 0)
    return -1;
  if (cgo_prf_init(&uiv->s, aesbits, key + aes_key_bytes + POLYVAL_KEY_LEN)<0)
    return -1;
#ifdef TOR_UNIT_TESTS
  /* Testing only: copy the keys so we can test UIV_UPDATE function. */
  size_t total_key_len = aes_key_bytes * 2 + POLYVAL_KEY_LEN * 2;
  tor_assert(total_key_len <= sizeof(uiv->uiv_keys_));
  memset(uiv->uiv_keys_, 0, sizeof(uiv->uiv_keys_));
  memcpy(uiv->uiv_keys_, key, total_key_len);
#endif
  return 0;
}
/**
 * Encrypt 'cell_body', with the provided tweak.
 *
 * The cell body must be UIV_BLOCK_LEN bytes long.
 */
STATIC void
cgo_uiv_encrypt(cgo_uiv_t *uiv, const uiv_tweak_t tweak, uint8_t *cell_body)
{
  uint8_t *X_L = cell_body;
  uint8_t *X_R = cell_body + 16;

  const et_tweak_t et_tweak = {
    .uiv = tweak,
    .x_r = X_R,
  };
  cgo_et_encrypt(&uiv->j, et_tweak, X_L);
  cgo_prf_xor_t0(&uiv->s, X_L, X_R);
}
/**
 * Decrypt 'cell_body', with the provided tweak.
 *
 * The cell body must be UIV_BLOCK_LEN bytes long.
 */
STATIC void
cgo_uiv_decrypt(cgo_uiv_t *uiv, const uiv_tweak_t tweak, uint8_t *cell_body)
{
  uint8_t *X_L = cell_body;
  uint8_t *X_R = cell_body + 16;

  const et_tweak_t et_tweak = {
    .uiv = tweak,
    .x_r = X_R,
  };
  cgo_prf_xor_t0(&uiv->s, X_L, X_R);
  cgo_et_decrypt(&uiv->j, et_tweak, X_L);
}
/**
 * Irreversibly ransform the keys of this UIV+, and the provided nonce,
 * using the nonce as input.
 *
 * The nonce must be 16 bytes long.
 */
STATIC void
cgo_uiv_update(cgo_uiv_t *uiv, int aesbits, bool encrypt, uint8_t *nonce)
{
  size_t aes_bytes = aesbits / 8;
  size_t single_key_len = aes_bytes + POLYVAL_KEY_LEN;
  size_t total_key_len = single_key_len * 2 + 16;
  // Note: We could store this on the stack, but stack-protector
  // wouldn't like that.
  uint8_t *new_keys = tor_malloc(total_key_len);

  cgo_prf_gen_t1(&uiv->s, nonce, new_keys, total_key_len);

  cgo_et_set_key(&uiv->j, aesbits, encrypt, new_keys);
  cgo_prf_set_key(&uiv->s, aesbits, new_keys + single_key_len);

  memcpy(nonce, new_keys + single_key_len * 2, 16);

#ifdef TOR_UNIT_TESTS
  /* Testing only: copy the keys so we can test UIV_UPDATE function. */
  memset(uiv->uiv_keys_, 0, sizeof(uiv->uiv_keys_));
  memcpy(uiv->uiv_keys_, new_keys, total_key_len);
#endif

  // This is key material, so we should really discard it.
  memwipe(new_keys, 0, total_key_len);
  tor_free(new_keys);
}
/**
 * Release any storage held in 'prf'.
 *
 * This _doesn't_ wipe 'prf'; that's done from a higher-level function.
 */
STATIC void
cgo_uiv_clear(cgo_uiv_t *uiv)
{
  cgo_et_clear(&uiv->j);
  cgo_prf_clear(&uiv->s);
}

/* ====================
 * High level counter galois onion implementations.
 */

/**
 * Return the total number of bytes needed to initialize a cgo_crypt_t.
 */
size_t
cgo_key_material_len(int aesbits)
{
  tor_assert(aesbits == 128 || aesbits == 192 || aesbits == 256);
  size_t r = (cgo_uiv_keylen(aesbits) + SENDME_TAG_LEN_CGO);
  tor_assert(r * 2 <= MAX_RELAY_KEY_MATERIAL_LEN);
  return r;
}

/**
 * Instantiate a CGO authenticated encryption object from the provided
 * 'keylen' bytes in 'keys'.
 *
 * 'keylen' must equal 'cgo_key_material_len(aesbits)'.
 *
 * The client and relay must have two cgo_crypt_t objects each:
 * one for the forward direction, and one for the reverse direction.
 */
cgo_crypt_t *
cgo_crypt_new(cgo_mode_t mode, int aesbits, const uint8_t *keys, size_t keylen)
{
  tor_assert(keylen == cgo_key_material_len(aesbits));
  const uint8_t *end_of_keys = keys + keylen;
  // Relays encrypt; clients decrypt.
  // Don't reverse this: UIV+ is only non-malleable for _encryption_.
  bool encrypt = (mode == CGO_MODE_RELAY_BACKWARD ||
                  mode == CGO_MODE_RELAY_FORWARD);
  int r;

  cgo_crypt_t *cgo = tor_malloc_zero(sizeof(cgo_crypt_t));
  r = cgo_uiv_init(&cgo->uiv, aesbits, encrypt, keys);
  tor_assert(r == 0);
  keys += cgo_uiv_keylen(aesbits);
  memcpy(cgo->nonce, keys, SENDME_TAG_LEN_CGO);
  keys += SENDME_TAG_LEN_CGO;
  tor_assert(keys == end_of_keys);

  cgo->aes_bytes = aesbits / 8;

  return cgo;
}
/**
 * Clean up 'cgo' and free it.
 */
void
cgo_crypt_free_(cgo_crypt_t *cgo)
{
  if (!cgo)
    return;
  cgo_uiv_clear(&cgo->uiv);
  memwipe(cgo, 0, sizeof(cgo_crypt_t));
  tor_free(cgo);
}

/**
 * Internal: Run the UIV Update operation on our UIV+ instance.
 */
static void
cgo_crypt_update(cgo_crypt_t *cgo, cgo_mode_t mode)
{
  bool encrypt = (mode == CGO_MODE_RELAY_BACKWARD ||
                  mode == CGO_MODE_RELAY_FORWARD);
  cgo_uiv_update(&cgo->uiv, cgo->aes_bytes * 8, encrypt, cgo->nonce);
}

/**
 * Forward CGO encryption operation at a relay:
 * process an outbound cell from the client.
 *
 * If the cell is for this relay, set *'recognized_tag_out'
 * to point to a SENDME_TAG_LEN_CGO value that should be used
 * if we want to acknowledge this cell with an authenticated SENDME.
 *
 * The value of 'recognized_tag_out' will become invalid
 * as soon as any change is made to this 'cgo' object,
 * or to the cell; if you need it, you should copy it immediately.
 *
 * If the cell is not for this relay, set *'recognized_tag_out' to NULL.
 */
void
cgo_crypt_relay_forward(cgo_crypt_t *cgo, cell_t *cell,
                        const uint8_t **recognized_tag_out)
{
  uiv_tweak_t h = {
    .h = cgo->tprime,
    .cmd = cell->command,
  };
  memcpy(cgo->last_tag_relay_fwd, cell->payload, SENDME_TAG_LEN_CGO);
  cgo_uiv_encrypt(&cgo->uiv, h, cell->payload);
  memcpy(cgo->tprime, cell->payload, SENDME_TAG_LEN_CGO);
  if (tor_memeq(cell->payload, cgo->nonce, SENDME_TAG_LEN_CGO)) {
    cgo_crypt_update(cgo, CGO_MODE_RELAY_FORWARD);
    *recognized_tag_out = cgo->last_tag_relay_fwd;
  } else {
    *recognized_tag_out = NULL;
  }
}

/**
 * Backward CGO encryption operation at a relay:
 * process an inbound cell from another relay, for the client.
 */
void
cgo_crypt_relay_backward(cgo_crypt_t *cgo, cell_t *cell)
{
  uiv_tweak_t h = {
    .h = cgo->tprime,
    .cmd = cell->command,
  };
  cgo_uiv_encrypt(&cgo->uiv, h, cell->payload);
  memcpy(cgo->tprime, cell->payload, SENDME_TAG_LEN_CGO);
}

/**
 * Backward CGO encryption operation at a relay:
 * encrypt an inbound message that we are originating, for the client.
 *
 * The provided cell must have its command value set,
 * and should have the first SENDME_TAG_LEN_CGO bytes of its payload unused.
 *
 * Set '*tag_out' to a value that we should expect
 * if we want an authenticated SENDME for this cell.
 *
 * The value of 'recognized_tag_out' will become invalid
 * as soon as any change is made to this 'cgo' object,
 * or to the cell; if you need it, you should copy it immediately.
 */
void
cgo_crypt_relay_originate(cgo_crypt_t *cgo, cell_t *cell,
                          const uint8_t **tag_out)
{
  uiv_tweak_t h = {
    .h = cgo->tprime,
    .cmd = cell->command,
  };
  memcpy(cell->payload, cgo->nonce, SENDME_TAG_LEN_CGO);
  cgo_uiv_encrypt(&cgo->uiv, h, cell->payload);
  memcpy(&cgo->tprime, cell->payload, SENDME_TAG_LEN_CGO);
  memcpy(&cgo->nonce, cell->payload, SENDME_TAG_LEN_CGO);
  if (tag_out) {
    // tor_assert(tor_memeq(cgo->tprime, cell->payload, SENDME_TAG_LEN_CGO));
    *tag_out = cgo->tprime;
  }
  cgo_crypt_update(cgo, CGO_MODE_RELAY_BACKWARD);
}

/**
 * Forward CGO encryption at a client:
 * process a cell for a non-destination hop.
 **/
void
cgo_crypt_client_forward(cgo_crypt_t *cgo, cell_t *cell)
{
  uint8_t tprime_new[SENDME_TAG_LEN_CGO];
  memcpy(tprime_new, cell->payload, SENDME_TAG_LEN_CGO);
  uiv_tweak_t h = {
    .h = cgo->tprime,
    .cmd = cell->command,
  };
  cgo_uiv_decrypt(&cgo->uiv, h, cell->payload);
  memcpy(cgo->tprime, tprime_new, SENDME_TAG_LEN_CGO);
}

/**
 * Forward CGO encryption at a client:
 * originate a cell for a given target hop.
 *
 * The provided cell must have its command value set,
 * and should have the first SENDME_TAG_LEN_CGO bytes of its payload unused.
 *
 * Set '*tag_out' to a value that we should expect
 * if we want an authenticated SENDME for this cell.
 *
 * The value of 'recognized_tag_out' will become invalid
 * as soon as any change is made to this 'cgo' object,
 * or to the cell; if you need it, you should copy it immediately.
 */
void
cgo_crypt_client_originate(cgo_crypt_t *cgo, cell_t *cell,
                           const uint8_t **tag_out)
{
  memcpy(cell->payload, cgo->nonce, SENDME_TAG_LEN_CGO);
  cgo_crypt_client_forward(cgo, cell);
  cgo_crypt_update(cgo, CGO_MODE_CLIENT_FORWARD);
  *tag_out = cell->payload;
}

/**
 * Backward CGO encryption operation at a rclient.
 * process an inbound cell from a relay.
 *
 * If the cell originated from this this relay, set *'recognized_tag_out'
 * to point to a SENDME_TAG_LEN_CGO value that should be used
 * if we want to acknowledge this cell with an authenticated SENDME.
 *
 * The value of 'recognized_tag_out' will become invalid
 * as soon as any change is made to this 'cgo' object,
 * or to the cell; if you need it, you should copy it immediately.
 *
 * If the cell is not from this relay, set *'recognized_tag_out' to NULL.
 */
void
cgo_crypt_client_backward(cgo_crypt_t *cgo, cell_t *cell,
                          const uint8_t **recognized_tag_out)
{
  uiv_tweak_t h = {
    .h = cgo->tprime,
    .cmd = cell->command,
  };
  uint8_t t_orig[SENDME_TAG_LEN_CGO];
  memcpy(t_orig, cell->payload, SENDME_TAG_LEN_CGO);

  cgo_uiv_decrypt(&cgo->uiv, h, cell->payload);
  memcpy(cgo->tprime, t_orig, SENDME_TAG_LEN_CGO);
  if (tor_memeq(cell->payload, cgo->nonce, SENDME_TAG_LEN_CGO)) {
    memcpy(cgo->nonce, t_orig, SENDME_TAG_LEN_CGO);
    cgo_crypt_update(cgo, CGO_MODE_CLIENT_BACKWARD);
    // tor_assert(tor_memeq(cgo->tprime, t_orig, SENDME_TAG_LEN_CGO));
    *recognized_tag_out = cgo->tprime;
  } else {
    *recognized_tag_out = NULL;
  }
}

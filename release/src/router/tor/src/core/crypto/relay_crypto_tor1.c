/* Copyright (c) 2001 Matej Pfajfar.
 * Copyright (c) 2001-2004, Roger Dingledine.
 * Copyright (c) 2004-2006, Roger Dingledine, Nick Mathewson.
 * Copyright (c) 2007-2021, The Tor Project, Inc. */
/* See LICENSE for licensing information */

/**
 * @file relay_crypto_tor1.c
 * @brief Implementation for legacy (tor1) relay cell encryption.
 **/

#include "core/or/or.h"
#include "lib/crypt_ops/crypto_cipher.h"
#include "lib/crypt_ops/crypto_util.h"
#include "core/crypto/hs_ntor.h" // for HS_NTOR_KEY_EXPANSION_KDF_OUT_LEN
#include "core/crypto/relay_crypto_tor1.h"
#include "lib/cc/ctassert.h"

#include "core/or/cell_st.h"
#include "core/crypto/relay_crypto_st.h"

/* Offset of digest within relay cell body for v0 cells. */
#define V0_DIGEST_OFFSET 5
#define V0_DIGEST_LEN 4
#define V0_RECOGNIZED_OFFSET 1

/** Update digest from the payload of cell. Assign integrity part to
 * cell.  Record full 20-byte digest in `buf`.
 */
static void
tor1_set_digest_v0(crypto_digest_t *digest, cell_t *cell, uint8_t *buf)
{
  crypto_digest_add_bytes(digest, (char*)cell->payload, CELL_PAYLOAD_SIZE);
  crypto_digest_get_digest(digest, (char*)buf, DIGEST_LEN);
//  log_fn(LOG_DEBUG,"Putting digest of %u %u %u %u into relay cell.",
//    integrity[0], integrity[1], integrity[2], integrity[3]);
  memcpy(cell->payload + V0_DIGEST_OFFSET, buf, V0_DIGEST_LEN);
}

/** Does the digest for this circuit indicate that this cell is for us?
 *
 * Update digest from the payload of cell (with the integrity part set
 * to 0). If the integrity part is valid,
 * return 1 and save the full digest in the 20-byte buffer `buf`,
 * else restore digest
 * and cell to their original state and return 0.
 */
static int
tor1_relay_digest_matches_v0(crypto_digest_t *digest, cell_t *cell,
                             uint8_t *buf)
{
  uint32_t received_integrity, calculated_integrity;
  uint8_t calculated_digest[DIGEST_LEN];
  crypto_digest_checkpoint_t backup_digest;

  CTASSERT(sizeof(uint32_t) == V0_DIGEST_LEN);

  crypto_digest_checkpoint(&backup_digest, digest);

  memcpy(&received_integrity, cell->payload + V0_DIGEST_OFFSET, V0_DIGEST_LEN);
  memset(cell->payload + V0_DIGEST_OFFSET, 0, V0_DIGEST_LEN);

//  log_fn(LOG_DEBUG,"Reading digest of %u %u %u %u from relay cell.",
//    received_integrity[0], received_integrity[1],
//    received_integrity[2], received_integrity[3]);

  crypto_digest_add_bytes(digest, (char*) cell->payload, CELL_PAYLOAD_SIZE);
  crypto_digest_get_digest(digest, (char*) calculated_digest, DIGEST_LEN);
  calculated_integrity = get_uint32(calculated_digest);

  int rv = 1;

  if (calculated_integrity != received_integrity) {
//    log_fn(LOG_INFO,"Recognized=0 but bad digest. Not recognizing.");
// (%d vs %d).", received_integrity, calculated_integrity);
    /* restore digest to its old form */
    crypto_digest_restore(digest, &backup_digest);
    /* restore the relay header */
    memcpy(cell->payload + V0_DIGEST_OFFSET, &received_integrity,
           V0_DIGEST_LEN);
    rv = 0;
  } else {
    memcpy(buf, calculated_digest, DIGEST_LEN);
  }

  memwipe(&backup_digest, 0, sizeof(backup_digest));
  return rv;
}

static inline bool
relay_cell_is_recognized_v0(const cell_t *cell)
{
  return get_uint16(cell->payload + V0_RECOGNIZED_OFFSET) == 0;
}

/** Apply <b>cipher</b> to CELL_PAYLOAD_SIZE bytes of <b>in</b>
 * (in place).
 *
 * Note that we use the same operation for encrypting and for decrypting.
 */
static void
tor1_crypt_one_payload(crypto_cipher_t *cipher, uint8_t *in)
{
  crypto_cipher_crypt_inplace(cipher, (char*) in, CELL_PAYLOAD_SIZE);
}

/** Encrypt and authenticate `cell` using the cryptographic
 * material in `tor1`.
 *
 * This method should be used for the first encryption performed
 * by the client - that is, the one corresponding to the exit node.
 */
void
tor1_crypt_client_originate(tor1_crypt_t *tor1,
                            cell_t *cell)
{
  tor1_set_digest_v0(tor1->f_digest, cell, tor1->sendme_digest);
  tor1_crypt_one_payload(tor1->f_crypto, cell->payload);
}

/** Encrypt and authenticate `cell`, using the cryptographic
 * material in `tor1`.
 *
 * This method should be used by relays when originating cells toward the
 * client.
 */
void
tor1_crypt_relay_originate(tor1_crypt_t *tor1,
                           cell_t *cell)
{
  tor1_set_digest_v0(tor1->b_digest, cell, tor1->sendme_digest);
  tor1_crypt_one_payload(tor1->b_crypto, cell->payload);
}

/** Encrypt `cell` using the cryptographic material in `tor1`.
 *
 * This method should be used by clients for cryptographic layers
 * that are _not_ the final recipient of the cell. */
void
tor1_crypt_client_forward(tor1_crypt_t *tor1, cell_t *cell)
{
  tor1_crypt_one_payload(tor1->f_crypto, cell->payload);
}

/** Encrypt `cell` using the cryptographic material in `tor1`.
 *
 * This method should be used by relays on cells that are moving
 * toward the client. */
void
tor1_crypt_relay_backward(tor1_crypt_t *tor1, cell_t *cell)
{
  tor1_crypt_one_payload(tor1->b_crypto, cell->payload);
}

/** Decrypt `cell` using the cryptographic material in `tor1`.
 *
 * Return `true` when we are the destination for this cell.
 *
 * This method should be used by relays on cells
 * that are moving away from the client. */
bool
tor1_crypt_relay_forward(tor1_crypt_t *tor1, cell_t *cell)
{
  tor1_crypt_one_payload(tor1->f_crypto, cell->payload);
  if (relay_cell_is_recognized_v0(cell)) {
    if (tor1_relay_digest_matches_v0(tor1->f_digest, cell,
                                     tor1->sendme_digest)) {
      return true;
    }
  }
  return false;
}

/** Decrypt `cell` using  the cryptographic material in `tor1`.
 *
 * Return `true` when this cell is recognized and authenticated
 * as coming from the relay that also holds this cryptographic material.
 *
 * This method should be used by clients on incoming cells. */
bool
tor1_crypt_client_backward(tor1_crypt_t *tor1, cell_t *cell)
{
  tor1_crypt_one_payload(tor1->b_crypto, cell->payload);

  if (relay_cell_is_recognized_v0(cell)) {
    if (tor1_relay_digest_matches_v0(tor1->b_digest, cell,
                                     tor1->sendme_digest)) {
      return true;
    }
  }
  return false;
}

/** Return the number of bytes that tor1_crypt_init expects. */
size_t
tor1_key_material_len(bool is_hs)
{
  if (is_hs)
    return HS_NTOR_KEY_EXPANSION_KDF_OUT_LEN;
  else
    return CPATH_KEY_MATERIAL_LEN;
}

/** Initialize <b>crypto</b> from the key material in key_data.
 *
 * If <b>is_hs_v3</b> is set, this cpath will be used for next gen hidden
 * service circuits and <b>key_data</b> must be
 * HS_NTOR_KEY_EXPANSION_KDF_OUT_LEN bytes in length.
 *
 * If <b>is_hs_v3</b> is not set, key_data must contain CPATH_KEY_MATERIAL_LEN
 * bytes, which are used as follows:
 *   - 20 to initialize f_digest
 *   - 20 to initialize b_digest
 *   - 16 to key f_crypto
 *   - 16 to key b_crypto
 *
 * (If 'reverse' is true, then f_XX and b_XX are swapped.)
 *
 * Return 0 if init was successful, else -1 if it failed.
 */
int
tor1_crypt_init(tor1_crypt_t *crypto,
                 const char *key_data, size_t key_data_len,
                 int reverse, int is_hs_v3)
{
  crypto_digest_t *tmp_digest;
  crypto_cipher_t *tmp_crypto;
  size_t digest_len = 0;
  size_t cipher_key_len = 0;

  tor_assert(crypto);
  tor_assert(key_data);
  tor_assert(!(crypto->f_crypto || crypto->b_crypto ||
             crypto->f_digest || crypto->b_digest));

  /* Basic key size validation */
  if (is_hs_v3 && BUG(key_data_len != HS_NTOR_KEY_EXPANSION_KDF_OUT_LEN)) {
    goto err;
  } else if (!is_hs_v3 && BUG(key_data_len != CPATH_KEY_MATERIAL_LEN)) {
    goto err;
  }

  /* If we are using this crypto for next gen onion services use SHA3-256,
     otherwise use good ol' SHA1 */
  if (is_hs_v3) {
    digest_len = DIGEST256_LEN;
    cipher_key_len = CIPHER256_KEY_LEN;
    crypto->f_digest = crypto_digest256_new(DIGEST_SHA3_256);
    crypto->b_digest = crypto_digest256_new(DIGEST_SHA3_256);
  } else {
    digest_len = DIGEST_LEN;
    cipher_key_len = CIPHER_KEY_LEN;
    crypto->f_digest = crypto_digest_new();
    crypto->b_digest = crypto_digest_new();
  }

  tor_assert(digest_len != 0);
  tor_assert(cipher_key_len != 0);
  const int cipher_key_bits = (int) cipher_key_len * 8;

  crypto_digest_add_bytes(crypto->f_digest, key_data, digest_len);
  crypto_digest_add_bytes(crypto->b_digest, key_data+digest_len, digest_len);

  crypto->f_crypto = crypto_cipher_new_with_bits(key_data+(2*digest_len),
                                                cipher_key_bits);
  if (!crypto->f_crypto) {
    log_warn(LD_BUG,"Forward cipher initialization failed.");
    goto err;
  }

  crypto->b_crypto = crypto_cipher_new_with_bits(
                                        key_data+(2*digest_len)+cipher_key_len,
                                        cipher_key_bits);
  if (!crypto->b_crypto) {
    log_warn(LD_BUG,"Backward cipher initialization failed.");
    goto err;
  }

  if (reverse) {
    tmp_digest = crypto->f_digest;
    crypto->f_digest = crypto->b_digest;
    crypto->b_digest = tmp_digest;
    tmp_crypto = crypto->f_crypto;
    crypto->f_crypto = crypto->b_crypto;
    crypto->b_crypto = tmp_crypto;
  }

  return 0;
 err:
  tor1_crypt_clear(crypto);
  return -1;
}

/** Assert that <b>crypto</b> is valid and set. */
void
tor1_crypt_assert_ok(const tor1_crypt_t *crypto)
{
  tor_assert(crypto->f_crypto);
  tor_assert(crypto->b_crypto);
  tor_assert(crypto->f_digest);
  tor_assert(crypto->b_digest);
}

void
tor1_crypt_clear(tor1_crypt_t *crypto)
{
  if (BUG(!crypto))
    return;
  crypto_cipher_free(crypto->f_crypto);
  crypto_cipher_free(crypto->b_crypto);
  crypto_digest_free(crypto->f_digest);
  crypto_digest_free(crypto->b_digest);
}

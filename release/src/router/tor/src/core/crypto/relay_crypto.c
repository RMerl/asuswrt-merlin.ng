/* Copyright (c) 2001 Matej Pfajfar.
 * Copyright (c) 2001-2004, Roger Dingledine.
 * Copyright (c) 2004-2006, Roger Dingledine, Nick Mathewson.
 * Copyright (c) 2007-2021, The Tor Project, Inc. */
/* See LICENSE for licensing information */

/**
 * @file relay_crypto.h
 * @brief Header for relay_crypto.c
 **/

#include "core/or/or.h"
#include "core/or/circuitlist.h"
#include "core/or/crypt_path.h"
#include "app/config/config.h"
#include "lib/crypt_ops/crypto_cipher.h"
#include "lib/crypt_ops/crypto_util.h"
#include "core/crypto/hs_ntor.h" // for HS_NTOR_KEY_EXPANSION_KDF_OUT_LEN
#include "core/or/relay.h"
#include "core/crypto/relay_crypto.h"
#include "core/or/sendme.h"

#include "core/or/cell_st.h"
#include "core/or/or_circuit_st.h"
#include "core/or/origin_circuit_st.h"

/** Update digest from the payload of cell. Assign integrity part to
 * cell.
 */
void
relay_set_digest(crypto_digest_t *digest, cell_t *cell)
{
  char integrity[4];
  relay_header_t rh;

  crypto_digest_add_bytes(digest, (char*)cell->payload, CELL_PAYLOAD_SIZE);
  crypto_digest_get_digest(digest, integrity, 4);
//  log_fn(LOG_DEBUG,"Putting digest of %u %u %u %u into relay cell.",
//    integrity[0], integrity[1], integrity[2], integrity[3]);
  relay_header_unpack(&rh, cell->payload);
  memcpy(rh.integrity, integrity, 4);
  relay_header_pack(cell->payload, &rh);
}

/** Does the digest for this circuit indicate that this cell is for us?
 *
 * Update digest from the payload of cell (with the integrity part set
 * to 0). If the integrity part is valid, return 1, else restore digest
 * and cell to their original state and return 0.
 */
static int
relay_digest_matches(crypto_digest_t *digest, cell_t *cell)
{
  uint32_t received_integrity, calculated_integrity;
  relay_header_t rh;
  crypto_digest_checkpoint_t backup_digest;

  crypto_digest_checkpoint(&backup_digest, digest);

  relay_header_unpack(&rh, cell->payload);
  memcpy(&received_integrity, rh.integrity, 4);
  memset(rh.integrity, 0, 4);
  relay_header_pack(cell->payload, &rh);

//  log_fn(LOG_DEBUG,"Reading digest of %u %u %u %u from relay cell.",
//    received_integrity[0], received_integrity[1],
//    received_integrity[2], received_integrity[3]);

  crypto_digest_add_bytes(digest, (char*) cell->payload, CELL_PAYLOAD_SIZE);
  crypto_digest_get_digest(digest, (char*) &calculated_integrity, 4);

  int rv = 1;

  if (calculated_integrity != received_integrity) {
//    log_fn(LOG_INFO,"Recognized=0 but bad digest. Not recognizing.");
// (%d vs %d).", received_integrity, calculated_integrity);
    /* restore digest to its old form */
    crypto_digest_restore(digest, &backup_digest);
    /* restore the relay header */
    memcpy(rh.integrity, &received_integrity, 4);
    relay_header_pack(cell->payload, &rh);
    rv = 0;
  }

  memwipe(&backup_digest, 0, sizeof(backup_digest));
  return rv;
}

/** Apply <b>cipher</b> to CELL_PAYLOAD_SIZE bytes of <b>in</b>
 * (in place).
 *
 * Note that we use the same operation for encrypting and for decrypting.
 */
void
relay_crypt_one_payload(crypto_cipher_t *cipher, uint8_t *in)
{
  crypto_cipher_crypt_inplace(cipher, (char*) in, CELL_PAYLOAD_SIZE);
}

/** Return the sendme_digest within the <b>crypto</b> object. */
uint8_t *
relay_crypto_get_sendme_digest(relay_crypto_t *crypto)
{
  tor_assert(crypto);
  return crypto->sendme_digest;
}

/** Record the cell digest, indicated by is_foward_digest or not, as the
 * SENDME cell digest. */
void
relay_crypto_record_sendme_digest(relay_crypto_t *crypto,
                                  bool is_foward_digest)
{
  struct crypto_digest_t *digest;

  tor_assert(crypto);

  digest = crypto->b_digest;
  if (is_foward_digest) {
    digest = crypto->f_digest;
  }

  crypto_digest_get_digest(digest, (char *) crypto->sendme_digest,
                           sizeof(crypto->sendme_digest));
}

/** Do the appropriate en/decryptions for <b>cell</b> arriving on
 * <b>circ</b> in direction <b>cell_direction</b>.
 *
 * If cell_direction == CELL_DIRECTION_IN:
 *   - If we're at the origin (we're the OP), for hops 1..N,
 *     decrypt cell. If recognized, stop.
 *   - Else (we're not the OP), encrypt one hop. Cell is not recognized.
 *
 * If cell_direction == CELL_DIRECTION_OUT:
 *   - decrypt one hop. Check if recognized.
 *
 * If cell is recognized, set *recognized to 1, and set
 * *layer_hint to the hop that recognized it.
 *
 * Return -1 to indicate that we should mark the circuit for close,
 * else return 0.
 */
int
relay_decrypt_cell(circuit_t *circ, cell_t *cell,
                   cell_direction_t cell_direction,
                   crypt_path_t **layer_hint, char *recognized)
{
  relay_header_t rh;

  tor_assert(circ);
  tor_assert(cell);
  tor_assert(recognized);
  tor_assert(cell_direction == CELL_DIRECTION_IN ||
             cell_direction == CELL_DIRECTION_OUT);

  if (cell_direction == CELL_DIRECTION_IN) {
    if (CIRCUIT_IS_ORIGIN(circ)) { /* We're at the beginning of the circuit.
                                    * We'll want to do layered decrypts. */
      crypt_path_t *thishop, *cpath = TO_ORIGIN_CIRCUIT(circ)->cpath;
      thishop = cpath;
      if (thishop->state != CPATH_STATE_OPEN) {
        log_fn(LOG_PROTOCOL_WARN, LD_PROTOCOL,
               "Relay cell before first created cell? Closing.");
        return -1;
      }
      do { /* Remember: cpath is in forward order, that is, first hop first. */
        tor_assert(thishop);

        /* decrypt one layer */
        cpath_crypt_cell(thishop, cell->payload, true);

        relay_header_unpack(&rh, cell->payload);
        if (rh.recognized == 0) {
          /* it's possibly recognized. have to check digest to be sure. */
          if (relay_digest_matches(cpath_get_incoming_digest(thishop), cell)) {
            *recognized = 1;
            *layer_hint = thishop;
            return 0;
          }
        }

        thishop = thishop->next;
      } while (thishop != cpath && thishop->state == CPATH_STATE_OPEN);
      log_fn(LOG_PROTOCOL_WARN, LD_OR,
             "Incoming cell at client not recognized. Closing.");
      return -1;
    } else {
      relay_crypto_t *crypto = &TO_OR_CIRCUIT(circ)->crypto;
      /* We're in the middle. Encrypt one layer. */
      relay_crypt_one_payload(crypto->b_crypto, cell->payload);
    }
  } else /* cell_direction == CELL_DIRECTION_OUT */ {
    /* We're in the middle. Decrypt one layer. */
    relay_crypto_t *crypto = &TO_OR_CIRCUIT(circ)->crypto;

    relay_crypt_one_payload(crypto->f_crypto, cell->payload);

    relay_header_unpack(&rh, cell->payload);
    if (rh.recognized == 0) {
      /* it's possibly recognized. have to check digest to be sure. */
      if (relay_digest_matches(crypto->f_digest, cell)) {
        *recognized = 1;
        return 0;
      }
    }
  }
  return 0;
}

/**
 * Encrypt a cell <b>cell</b> that we are creating, and sending outbound on
 * <b>circ</b> until the hop corresponding to <b>layer_hint</b>.
 *
 * The integrity field and recognized field of <b>cell</b>'s relay headers
 * must be set to zero.
 */
void
relay_encrypt_cell_outbound(cell_t *cell,
                            origin_circuit_t *circ,
                            crypt_path_t *layer_hint)
{
  crypt_path_t *thishop; /* counter for repeated crypts */
  cpath_set_cell_forward_digest(layer_hint, cell);

  /* Record cell digest as the SENDME digest if need be. */
  sendme_record_sending_cell_digest(TO_CIRCUIT(circ), layer_hint);

  thishop = layer_hint;
  /* moving from farthest to nearest hop */
  do {
    tor_assert(thishop);
    log_debug(LD_OR,"encrypting a layer of the relay cell.");
    cpath_crypt_cell(thishop, cell->payload, false);

    thishop = thishop->prev;
  } while (thishop != circ->cpath->prev);
}

/**
 * Encrypt a cell <b>cell</b> that we are creating, and sending on
 * <b>circuit</b> to the origin.
 *
 * The integrity field and recognized field of <b>cell</b>'s relay headers
 * must be set to zero.
 */
void
relay_encrypt_cell_inbound(cell_t *cell,
                           or_circuit_t *or_circ)
{
  relay_set_digest(or_circ->crypto.b_digest, cell);

  /* Record cell digest as the SENDME digest if need be. */
  sendme_record_sending_cell_digest(TO_CIRCUIT(or_circ), NULL);

  /* encrypt one layer */
  relay_crypt_one_payload(or_circ->crypto.b_crypto, cell->payload);
}

/**
 * Release all storage held inside <b>crypto</b>, but do not free
 * <b>crypto</b> itself: it lives inside another object.
 */
void
relay_crypto_clear(relay_crypto_t *crypto)
{
  if (BUG(!crypto))
    return;
  crypto_cipher_free(crypto->f_crypto);
  crypto_cipher_free(crypto->b_crypto);
  crypto_digest_free(crypto->f_digest);
  crypto_digest_free(crypto->b_digest);
}

/** Initialize <b>crypto</b> from the key material in key_data.
 *
 * If <b>is_hs_v3</b> is set, this cpath will be used for next gen hidden
 * service circuits and <b>key_data</b> must be at least
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
relay_crypto_init(relay_crypto_t *crypto,
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
  relay_crypto_clear(crypto);
  return -1;
}

/** Assert that <b>crypto</b> is valid and set. */
void
relay_crypto_assert_ok(const relay_crypto_t *crypto)
{
  tor_assert(crypto->f_crypto);
  tor_assert(crypto->b_crypto);
  tor_assert(crypto->f_digest);
  tor_assert(crypto->b_digest);
}

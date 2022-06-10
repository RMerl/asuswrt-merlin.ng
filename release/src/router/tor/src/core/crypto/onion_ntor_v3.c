/* Copyright (c) 2001 Matej Pfajfar.
 * Copyright (c) 2001-2004, Roger Dingledine.
 * Copyright (c) 2004-2006, Roger Dingledine, Nick Mathewson.
 * Copyright (c) 2007-2021, The Tor Project, Inc. */
/* See LICENSE for licensing information */

/**
 * @file onion_ntor_v3.c
 * @brief Implements the version 3 ntor handshake as first specified in
 * proposal 332.
 *
 * The v3 ntor handshake differs from the earlier versions (ntor and hs-ntor)
 * primarily in that it allows the client to send an authenticated encrypted
 * message as part of its onion skin, and allows the relay to send and
 * encrypted authenticated reply as part of its response.
 *
 * It also takes a "verification string" -- the handshake cannot succeed
 * unless both parties use the same value for their verification stream.
 **/

#define ONION_NTOR_V3_PRIVATE

#include "orconfig.h"
#include "core/crypto/onion_ntor_v3.h"

#include "lib/arch/bytes.h"
#include "lib/crypt_ops/crypto_digest.h"
#include "lib/crypt_ops/crypto_rand.h"
#include "lib/crypt_ops/crypto_util.h"
#include "lib/ctime/di_ops.h"
#include "lib/log/util_bug.h"

#include <string.h>

/* Parameters used to keep the outputs of this handshake from colliding with
 * others.  These are defined in the specification. */
#define PROTOID "ntor3-curve25519-sha3_256-1"
#define TWEAK(A) (PROTOID ":" A)

#define T_MSGKDF TWEAK("kdf_phase1")
#define T_MSGMAC TWEAK("msg_mac")
#define T_KEY_SEED TWEAK("key_seed")
#define T_VERIFY TWEAK("verify")
#define T_FINAL TWEAK("kdf_final")
#define T_AUTH TWEAK("auth_final")

/**
 * Add @a len bytes of @a data as input to the provided @a xof.
 *
 * (This is provided just for abbreviation).
 **/
#define xof_add(xof, data, len) crypto_xof_add_bytes((xof), (data), (len))
/**
 * Add @a len bytes of @a data as input to the provided @a xof,
 * prefixed with an encoding of the length.
 *
 * This is equivalent to ENCAP(data) in the spec.
 **/
static void
xof_add_encap(crypto_xof_t *xof, const uint8_t *data, size_t len)
{
  uint64_t len64 = tor_htonll(len);
  xof_add(xof, (uint8_t *)(&len64), 8);
  xof_add(xof, data, len);
}
/**
 * Add an encapsulated tweak to the provided xof.
 **/
#define xof_add_tweak(d, s) xof_add_encap((d), (const uint8_t *)(s), strlen(s))

/**
 * Add @a len bytes of @a data to the provided @a digest.
 *
 * This is provided as an abbreviation, and to get the types right.
 **/
static void
d_add(crypto_digest_t *digest, const uint8_t *data, size_t len)
{
  crypto_digest_add_bytes(digest, (const char *)data, len);
}
/**
 * Add @a len bytes of @a data to the provided @a digest, prefixed
 * with the encoded length.
 *
 * This is equivalent to ENCAP(data) from the spec.
 **/
static void
d_add_encap(crypto_digest_t *digest, const uint8_t *data, size_t len)
{
  uint64_t len64 = tor_htonll(len);
  d_add(digest, (const uint8_t *)(&len64), 8);
  d_add(digest, data, len);
}
/**
 * Add an encapsulated tweak to the provided digest.
 **/
#define d_add_tweak(d, s) d_add_encap((d), (const uint8_t *)(s), strlen(s))

/**
 * Helper: copy @a len bytes of @a data onto *@a ptr, and advance @a ptr
 * forward by @a len bytes.
 *
 * Asserts that @a ptr will not be advanced beyond @a endptr.
 **/
static void
push(uint8_t **ptr, const uint8_t *endptr, const uint8_t *data, size_t len)
{
  size_t remaining = endptr - *ptr;
  tor_assert(len <= remaining);
  memcpy(*ptr, data, len);
  *ptr += len;
}

/**
 * Helper: Drop storage held by @a state, after wiping it.
 **/
void
ntor3_handshake_state_free_(ntor3_handshake_state_t *state)
{
  if (!state)
    return;

  memwipe(state, 0, sizeof(*state));
  tor_free(state);
}

/**
 * Perform a client-side v3 ntor handshake with a given relay.
 *
 * As inputs this function takes the relay's Ed25519 identity (@a relay_id),
 * the relay's current ntor onion key (@a relay_key), a verification string
 * (@a verification_len bytes at @a verification), and a message to send
 * as part of the handshake (@a message_len bytes at @a message).
 *
 * The message will be encrypted and authenticated to the relay, but will not
 * receive the same forward secrecy as the rest of the handshake.  We should
 * not put any super-confidential data in it.
 *
 * The handshake will only succeed if the relay uses the same verification
 * string as we are using.
 *
 * As outputs, this function returns 0 on success and -1 on failure.  On
 * success, it sets @a onion_skin_out and @a onion_skin_len_out to a newly
 * allocated handshake message that the client can send as part of its CREATE2
 * or EXTEND2 cell.  It also sets it sets @a handshake_state_out to a newly
 * allocated handshake state object; the client needs to use this object to
 * process the relay's eventual reply.
 **/
int
onion_skin_ntor3_create(const ed25519_public_key_t *relay_id,
                        const curve25519_public_key_t *relay_key,
                        const uint8_t *verification,
                        const size_t verification_len,
                        const uint8_t *message,
                        const size_t message_len,
                        ntor3_handshake_state_t **handshake_state_out,
                        uint8_t **onion_skin_out,
                        size_t *onion_skin_len_out)
{
  curve25519_keypair_t client_keypair;
  if (curve25519_keypair_generate(&client_keypair, 0) < 0) {
    return -1;
  }
  int r = onion_skin_ntor3_create_nokeygen(
                                   &client_keypair,
                                   relay_id,
                                   relay_key,
                                   verification,
                                   verification_len,
                                   message,
                                   message_len,
                                   handshake_state_out,
                                   onion_skin_out,
                                   onion_skin_len_out);
  memwipe(&client_keypair, 0, sizeof(client_keypair));
  return r;
}

/**
 * Like onion_skin_ntor3_create, but do not generate a new ephemeral keypair.
 * Instead, take the ephemeral keypair (x,X) from @a client_keypair.
 *
 * (Having a separate function for this lets us test the code for correct
 * behavior.)
 **/
STATIC int
onion_skin_ntor3_create_nokeygen(
                        const curve25519_keypair_t *client_keypair,
                        const ed25519_public_key_t *relay_id,
                        const curve25519_public_key_t *relay_key,
                        const uint8_t *verification,
                        const size_t verification_len,
                        const uint8_t *message,
                        const size_t message_len,
                        ntor3_handshake_state_t **handshake_state_out,
                        uint8_t **onion_skin_out,
                        size_t *onion_skin_len_out)
{
  *handshake_state_out = NULL;
  *onion_skin_out = NULL;
  *onion_skin_len_out = 0;

  // Set up the handshake state object.
  *handshake_state_out = tor_malloc_zero(sizeof(ntor3_handshake_state_t));
  memcpy(&(*handshake_state_out)->client_keypair, client_keypair,
         sizeof(*client_keypair));
  memcpy(&(*handshake_state_out)->relay_id, relay_id, sizeof(*relay_id));
  memcpy(&(*handshake_state_out)->relay_key, relay_key, sizeof(*relay_key));

  // Perform the first DH handshake.
  curve25519_handshake((*handshake_state_out)->bx,
                       &client_keypair->seckey, relay_key);
  if (safe_mem_is_zero((*handshake_state_out)->bx, CURVE25519_OUTPUT_LEN)) {
    // Okay to return early here, since our behavior here doesn't
    // cause a visible timing sidechannel.
    return -1;
  }

  // Compute phase1_keys.
  uint8_t enc_key[CIPHER256_KEY_LEN];
  uint8_t mac_key[DIGEST256_LEN];
  {
    crypto_xof_t *xof = crypto_xof_new();
    // secret_input_phase1 = Bx | ID | X | B | PROTOID | ENCAP(VER)
    xof_add_tweak(xof, T_MSGKDF);
    xof_add(xof, (*handshake_state_out)->bx, CURVE25519_OUTPUT_LEN);
    xof_add(xof, relay_id->pubkey, ED25519_PUBKEY_LEN);
    xof_add(xof, client_keypair->pubkey.public_key, CURVE25519_PUBKEY_LEN);
    xof_add(xof, relay_key->public_key, CURVE25519_PUBKEY_LEN);
    xof_add(xof, (const uint8_t *)PROTOID, strlen(PROTOID));
    xof_add_encap(xof, verification, verification_len);
    crypto_xof_squeeze_bytes(xof, enc_key, sizeof(enc_key));
    crypto_xof_squeeze_bytes(xof, mac_key, sizeof(mac_key));
    crypto_xof_free(xof);
  }

  // Compute encrypted message.
  uint8_t *encrypted_message = tor_memdup(message, message_len);
  {
    crypto_cipher_t *c =
      crypto_cipher_new_with_bits((const char *)enc_key, 256);
    crypto_cipher_crypt_inplace(c, (char *)encrypted_message, message_len);
    crypto_cipher_free(c);
  }

  // Compute the MAC value.
  {
    crypto_digest_t *m = crypto_digest256_new(DIGEST_SHA3_256);
    d_add_tweak(m, T_MSGMAC);
    d_add_encap(m, mac_key, sizeof(mac_key));
    d_add(m, relay_id->pubkey, ED25519_PUBKEY_LEN);
    d_add(m, relay_key->public_key, CURVE25519_PUBKEY_LEN);
    d_add(m, client_keypair->pubkey.public_key, CURVE25519_PUBKEY_LEN);
    d_add(m, encrypted_message, message_len);
    crypto_digest_get_digest(m,
                             (char *)(*handshake_state_out)->msg_mac,
                             DIGEST256_LEN);
    crypto_digest_free(m);
  }

  // Build the onionskin.
  *onion_skin_len_out = (ED25519_PUBKEY_LEN + CURVE25519_PUBKEY_LEN*2 +
                          DIGEST256_LEN + message_len);
  *onion_skin_out = tor_malloc(*onion_skin_len_out);
  {
    uint8_t *ptr = *onion_skin_out, *end = ptr + *onion_skin_len_out;

    push(&ptr, end, relay_id->pubkey, ED25519_PUBKEY_LEN);
    push(&ptr, end, relay_key->public_key, CURVE25519_PUBKEY_LEN);
    push(&ptr, end, client_keypair->pubkey.public_key, CURVE25519_PUBKEY_LEN);
    push(&ptr, end, encrypted_message, message_len);
    push(&ptr, end, (*handshake_state_out)->msg_mac, DIGEST256_LEN);
    tor_assert(ptr == end);
  }

  memwipe(&enc_key, 0, sizeof(enc_key));
  memwipe(&mac_key, 0, sizeof(mac_key));
  memwipe(encrypted_message, 0, message_len);
  tor_free(encrypted_message);

  return 0;
}

/**
 * Complete a client-side v3 ntor handshake.
 *
 * Takes a @a handshake_state returned earlier by `onion_skin_ntor3_create()`,
 * and the relay's reply to that handshake (@a reply_len bytes at @a
 * handshake_reply).  Also takes a verification string (@a verification_len
 * bytes at @a verification).
 *
 * Returns 0 on success and -1 on failure. On success, generates @a key_len
 * bytes of key material into the provided @a keys_out buffer, and sets @a
 * message_out to the message that the relay sent in reply to our message (and
 * sets @a message_out_len to that message's length).
 **/
int
onion_ntor3_client_handshake(const ntor3_handshake_state_t *handshake_state,
                             const uint8_t *handshake_reply,
                             size_t reply_len,
                             const uint8_t *verification,
                             size_t verification_len,
                             uint8_t *keys_out,
                             size_t keys_out_len,
                             uint8_t **message_out,
                             size_t *message_len_out)
{
  *message_out = NULL;
  *message_len_out = 0;

  int problems = 0;

  // Parse the relay's message.
  curve25519_public_key_t relay_Y;
  uint8_t relay_auth[DIGEST256_LEN];
  size_t encrypted_msg_len;
  const uint8_t *encrypted_msg;
  {
    if (reply_len < CURVE25519_PUBKEY_LEN + DIGEST256_LEN) {
      // Okay to return early here, since the message is completely
      // ill-formed, so we can't leak anything.
      ++problems;
      goto done;
    }
    encrypted_msg_len = reply_len - (CURVE25519_PUBKEY_LEN + DIGEST256_LEN);

    memcpy(&relay_Y.public_key, handshake_reply, CURVE25519_PUBKEY_LEN);
    handshake_reply += CURVE25519_PUBKEY_LEN;
    memcpy(&relay_auth, handshake_reply, DIGEST256_LEN);
    handshake_reply += DIGEST256_LEN;
    encrypted_msg = handshake_reply;
  }

  // Finish the second diffie hellman handshake.
  uint8_t yx[CURVE25519_OUTPUT_LEN];
  curve25519_handshake(yx, &handshake_state->client_keypair.seckey, &relay_Y);
  problems |= safe_mem_is_zero(yx, sizeof(yx));

  // Compute two tweaked hashes of secret_input.
  uint8_t key_seed[DIGEST256_LEN], verify[DIGEST256_LEN];
  {
    crypto_digest_t *ks = crypto_digest256_new(DIGEST_SHA3_256);
    crypto_digest_t *v = crypto_digest256_new(DIGEST_SHA3_256);
    d_add_tweak(ks, T_KEY_SEED);
    d_add_tweak(v, T_VERIFY);
#define ADD2(s,len) STMT_BEGIN { \
      d_add(ks, (s),(len)); d_add(v, (s), (len));       \
    } STMT_END
#define ADD2_ENCAP(s,len) STMT_BEGIN { \
        d_add_encap(ks, (s),(len)); d_add_encap(v, (s), (len)); \
      } STMT_END

    ADD2(yx, sizeof(yx));
    ADD2(handshake_state->bx, sizeof(handshake_state->bx));
    ADD2(handshake_state->relay_id.pubkey, ED25519_PUBKEY_LEN);
    ADD2(handshake_state->relay_key.public_key, CURVE25519_PUBKEY_LEN);
    ADD2(handshake_state->client_keypair.pubkey.public_key,
         CURVE25519_PUBKEY_LEN);
    ADD2(relay_Y.public_key, CURVE25519_PUBKEY_LEN);
    ADD2((const uint8_t *)PROTOID, strlen(PROTOID));
    ADD2_ENCAP(verification, verification_len);

    crypto_digest_get_digest(ks, (char*) key_seed, DIGEST256_LEN);
    crypto_digest_get_digest(v, (char*) verify, DIGEST256_LEN);
    crypto_digest_free(ks);
    crypto_digest_free(v);
  }

  // compute expected auth value.
  uint8_t auth_computed[DIGEST256_LEN];
  {
    crypto_digest_t *d = crypto_digest256_new(DIGEST_SHA3_256);
    d_add_tweak(d, T_AUTH);
    d_add(d, verify, sizeof(verify));
    d_add(d, handshake_state->relay_id.pubkey, ED25519_PUBKEY_LEN);
    d_add(d, handshake_state->relay_key.public_key, CURVE25519_PUBKEY_LEN);
    d_add(d, relay_Y.public_key, CURVE25519_PUBKEY_LEN);
    d_add(d, handshake_state->client_keypair.pubkey.public_key,
          CURVE25519_PUBKEY_LEN);
    d_add(d, handshake_state->msg_mac, DIGEST256_LEN);
    d_add_encap(d, encrypted_msg, encrypted_msg_len);
    d_add(d, (const uint8_t*)PROTOID, strlen(PROTOID));
    d_add(d, (const uint8_t*)"Server", strlen("Server"));
    crypto_digest_get_digest(d, (char *)auth_computed, DIGEST256_LEN);
    crypto_digest_free(d);
  }

  // Check authentication value.
  problems |= tor_memneq(auth_computed, relay_auth, DIGEST256_LEN);

  // Compute keystream, decrypt message, and return.
  *message_out = tor_malloc(encrypted_msg_len);
  *message_len_out = encrypted_msg_len;
  uint8_t enc_key[CIPHER256_KEY_LEN];
  {
    crypto_xof_t *xof = crypto_xof_new();
    xof_add_tweak(xof, T_FINAL);
    xof_add(xof, key_seed, sizeof(key_seed));
    crypto_xof_squeeze_bytes(xof, enc_key, sizeof(enc_key));
    crypto_xof_squeeze_bytes(xof, (uint8_t *)keys_out, keys_out_len);
    crypto_xof_free(xof);

    crypto_cipher_t *c =
      crypto_cipher_new_with_bits((const char *)enc_key, 256);
    crypto_cipher_decrypt(c, (char *)*message_out,
                          (const char *)encrypted_msg, encrypted_msg_len);
    crypto_cipher_free(c);
  }

 done:
  memwipe(&relay_Y, 0, sizeof(relay_Y));
  memwipe(&relay_auth, 0, sizeof(relay_auth));
  memwipe(&yx, 0, sizeof(yx));
  memwipe(key_seed, 0, sizeof(key_seed));
  memwipe(verify, 0, sizeof(verify));
  memwipe(enc_key, 0, sizeof(enc_key));
  if (problems) {
    if (*message_out) {
      memwipe(*message_out, 0, *message_len_out);
      tor_free(*message_out); // Sets it to NULL.
    }
    *message_len_out = 0;
    crypto_rand((char*)keys_out, keys_out_len); // In case bad code uses it.
    return -1;
  }

  return 0;
}

/**
 * Wipe a server handshake state, and release the storage it holds.
 **/
void
ntor3_server_handshake_state_free_(ntor3_server_handshake_state_t *state)
{
  if (state == NULL)
    return;

  memwipe(state, 0, sizeof(ntor3_server_handshake_state_t));
  tor_free(state);
}

/**
 * As a relay, start handling a client's v3 ntor handshake.
 *
 * This function performs the _first half_ of the handshake, up to the point
 * where the client's message is decoded.  After calling it, the relay should
 * decide how and whether to reply to the client's message, compose its reply,
 * and call `onion_skin_ntor3_server_handshake_part2`.
 *
 * It takes as input a map of the relay's known onion keys in @a private_keys,
 * along with a fake @a junk_key to use if there is a complete mismatch.  It
 * takes the relay's ed25519 identity in @a my_id, along with the client's
 * handshake message (@a client_handshake_len bytes in @a client_handshake),
 * and a verification string (@a verification_len bytes in @a verification).
 *
 * Return 0 on success, and -1 on failure.  On success, sets @a
 * client_message_out to a newly allocated string holding the plaintext of the
 * message that the client sent as part of its handshake, and @a
 * client_message_out_len to its length. Also sets @a state_out to a newly
 * allocated state object holding the intermediate computation for this
 * handshake.
 **/
int
onion_skin_ntor3_server_handshake_part1(
                const di_digest256_map_t *private_keys,
                const curve25519_keypair_t *junk_key,
                const ed25519_public_key_t *my_id,
                const uint8_t *client_handshake,
                size_t client_handshake_len,
                const uint8_t *verification,
                size_t verification_len,
                uint8_t **client_message_out,
                size_t *client_message_len_out,
                ntor3_server_handshake_state_t **state_out)
{
  *client_message_out = NULL;
  *client_message_len_out = 0;
  *state_out = NULL;

  int problems = 0;

  // Initialize state.
  (*state_out) = tor_malloc_zero(sizeof(ntor3_server_handshake_state_t));
  memcpy(&(*state_out)->my_id, my_id, sizeof(*my_id));

  const uint8_t *wanted_id; // [ED25519_PUBKEY_LEN]
  const uint8_t *wanted_key; // [CURVE25519_PUBKEY_LEN]
  const uint8_t *encrypted_message;
  size_t encrypted_message_len;
  // Unpack the client handshake.
  {
    const uint8_t *ptr = client_handshake;
    const uint8_t *end = ptr + client_handshake_len;

    if (client_handshake_len <
        ED25519_PUBKEY_LEN + CURVE25519_PUBKEY_LEN * 2 + DIGEST256_LEN) {
      // Okay to end early; the client knows this is unparseable already.
      ++problems;
      goto done;
    }
    wanted_id = ptr;
    ptr += ED25519_PUBKEY_LEN;
    wanted_key = ptr;
    ptr += CURVE25519_PUBKEY_LEN;
    memcpy((*state_out)->client_key.public_key, ptr, CURVE25519_PUBKEY_LEN);
    ptr += CURVE25519_PUBKEY_LEN;
    size_t remaining = (end-ptr);
    if (BUG(remaining < DIGEST256_LEN)) {
      // Okay to end early; this is a bug.
      ++problems;
      goto done;
    }
    encrypted_message = ptr;
    encrypted_message_len = remaining - DIGEST256_LEN;
    ptr += encrypted_message_len;
    remaining = (end-ptr);
    tor_assert(remaining == DIGEST256_LEN);
    memcpy((*state_out)->msg_mac, ptr, DIGEST256_LEN);
  }

  // Check the identity.
  problems |= tor_memneq(my_id->pubkey, wanted_id, ED25519_PUBKEY_LEN);

  // Find the correct keypair.
  const curve25519_keypair_t *keypair =
    dimap_search(private_keys, wanted_key, (void *)junk_key);
  tor_assert(keypair);
  memcpy(&(*state_out)->my_key, &keypair->pubkey,
         sizeof(curve25519_public_key_t));

  // Do the first diffie hellman handshake.
  curve25519_handshake((*state_out)->xb,
                       &keypair->seckey, &(*state_out)->client_key);
  problems |= safe_mem_is_zero((*state_out)->xb, CURVE25519_OUTPUT_LEN);

  // Derive the encryption and mac keys
  uint8_t enc_key[CIPHER256_KEY_LEN], mac_key[DIGEST256_LEN];
  {
    crypto_xof_t *xof = crypto_xof_new();
    xof_add_tweak(xof, T_MSGKDF);
    xof_add(xof, (*state_out)->xb, CURVE25519_OUTPUT_LEN);
    xof_add(xof, wanted_id, ED25519_PUBKEY_LEN);
    xof_add(xof, (*state_out)->client_key.public_key, CURVE25519_PUBKEY_LEN);
    xof_add(xof, keypair->pubkey.public_key, CURVE25519_PUBKEY_LEN);
    xof_add(xof, (const uint8_t *)PROTOID, strlen(PROTOID));
    xof_add_encap(xof, verification, verification_len);
    crypto_xof_squeeze_bytes(xof, enc_key, sizeof(enc_key));
    crypto_xof_squeeze_bytes(xof, mac_key, sizeof(mac_key));
    crypto_xof_free(xof);
  }

  // Check the MAC.
  uint8_t computed_mac[DIGEST256_LEN];
  {
    crypto_digest_t *d = crypto_digest256_new(DIGEST_SHA3_256);
    d_add_tweak(d, T_MSGMAC);
    d_add_encap(d, mac_key, sizeof(mac_key));
    d_add(d, my_id->pubkey, ED25519_PUBKEY_LEN);
    d_add(d, keypair->pubkey.public_key, CURVE25519_PUBKEY_LEN);
    d_add(d, (*state_out)->client_key.public_key, CURVE25519_PUBKEY_LEN);
    d_add(d, encrypted_message, encrypted_message_len);
    crypto_digest_get_digest(d, (char *)computed_mac, DIGEST256_LEN);
    crypto_digest_free(d);
  }

  problems |= tor_memneq((*state_out)->msg_mac, computed_mac, DIGEST256_LEN);

  // Decrypt the message.
  *client_message_out = tor_malloc(encrypted_message_len);
  *client_message_len_out = encrypted_message_len;
  {
    crypto_cipher_t *c =
      crypto_cipher_new_with_bits((const char *)enc_key, 256);
    crypto_cipher_decrypt(c, (char *)*client_message_out,
                          (const char *)encrypted_message,
                          encrypted_message_len);
    crypto_cipher_free(c);
  }

 done:
  memwipe(enc_key, 0, sizeof(enc_key));
  memwipe(mac_key, 0, sizeof(mac_key));
  memwipe(computed_mac, 0, sizeof(computed_mac));
  if (problems) {
    if (*client_message_out) {
      memwipe(*client_message_out, 0, *client_message_len_out);
      tor_free(*client_message_out); // Sets it to NULL.
    }
    *client_message_len_out = 0;
    ntor3_server_handshake_state_free(*state_out);
    return -1;
  }

  return 0;
}

/**
 * Finish the relay side of an ntor v3 handshake.
 *
 * The relay calls this function after it has decided to respond to the
 * client's original encrypted message.  This function receives the relay's
 * message in @a server_message and its length in @a server_message_len, and
 * completes the handshake.
 *
 * Returns 0 on success and -1 on failure. On success, stores the newly
 * allocated handshake for the relay to send in @a handshake_out, and its
 * length in @a handshake_len_out.  Stores @a keys_out_len bytes of generated
 * keys in the provided buffer at @a keys_out.
 **/
int
onion_skin_ntor3_server_handshake_part2(
                const ntor3_server_handshake_state_t *state,
                const uint8_t *verification,
                size_t verification_len,
                const uint8_t *server_message,
                size_t server_message_len,
                uint8_t **handshake_out,
                size_t *handshake_len_out,
                uint8_t *keys_out,
                size_t keys_out_len)
{
  curve25519_keypair_t relay_keypair;
  if (curve25519_keypair_generate(&relay_keypair, 0) < 0) {
    return -1;
  }
  int r = onion_skin_ntor3_server_handshake_part2_nokeygen(
              &relay_keypair,
              state,
              verification,
              verification_len,
              server_message,
              server_message_len,
              handshake_out,
              handshake_len_out,
              keys_out,
              keys_out_len);
  memwipe(&relay_keypair, 0, sizeof(relay_keypair));
  return r;
}

/**
 * Like `onion_skin_ntor3_server_handshake_part2`, but do not generate
 * an ephemeral (y,Y) keypair.
 *
 * Instead, this function takes that keypair as @a relay_keypair_y.
 *
 * (Having a separate function for this lets us test the code for correct
 * behavior.)
 **/
STATIC int
onion_skin_ntor3_server_handshake_part2_nokeygen(
                const curve25519_keypair_t *relay_keypair_y,
                const ntor3_server_handshake_state_t *state,
                const uint8_t *verification,
                size_t verification_len,
                const uint8_t *server_message,
                size_t server_message_len,
                uint8_t **handshake_out,
                size_t *handshake_len_out,
                uint8_t *keys_out,
                size_t keys_out_len)
{
  *handshake_out = NULL;
  *handshake_len_out = 0;

  int problems = 0;

  // Second diffie-hellman handshake.
  uint8_t xy[CURVE25519_OUTPUT_LEN];
  curve25519_handshake(xy, &relay_keypair_y->seckey, &state->client_key);
  problems |= safe_mem_is_zero(xy, sizeof(xy));

  // Compute two tweaked hashes of secret_input.
  uint8_t key_seed[DIGEST256_LEN], verify[DIGEST256_LEN];
  {
    crypto_digest_t *ks = crypto_digest256_new(DIGEST_SHA3_256);
    crypto_digest_t *v = crypto_digest256_new(DIGEST_SHA3_256);
    d_add_tweak(ks, T_KEY_SEED);
    d_add_tweak(v, T_VERIFY);
    ADD2(xy, sizeof(xy));
    ADD2(state->xb, sizeof(state->xb));
    ADD2(state->my_id.pubkey, ED25519_PUBKEY_LEN);
    ADD2(state->my_key.public_key, CURVE25519_PUBKEY_LEN);
    ADD2(state->client_key.public_key, CURVE25519_PUBKEY_LEN);
    ADD2(relay_keypair_y->pubkey.public_key, CURVE25519_PUBKEY_LEN);
    ADD2((const uint8_t *)PROTOID, strlen(PROTOID));
    ADD2_ENCAP(verification, verification_len);
    crypto_digest_get_digest(ks, (char*) key_seed, DIGEST256_LEN);
    crypto_digest_get_digest(v, (char*) verify, DIGEST256_LEN);
    crypto_digest_free(ks);
    crypto_digest_free(v);
  }

  // Compute enc_key and keystream.
  uint8_t enc_key[CIPHER256_KEY_LEN];
  {
    crypto_xof_t *xof = crypto_xof_new();
    xof_add_tweak(xof, T_FINAL);
    xof_add(xof, key_seed, sizeof(key_seed));
    crypto_xof_squeeze_bytes(xof, enc_key, sizeof(enc_key));
    crypto_xof_squeeze_bytes(xof, keys_out, keys_out_len);
    crypto_xof_free(xof);
  }

  // Encrypt message.
  uint8_t *encrypted_message = tor_memdup(server_message, server_message_len);
  {
    crypto_cipher_t *c =
      crypto_cipher_new_with_bits((const char *)enc_key, 256);
    crypto_cipher_crypt_inplace(
                c, (char *)encrypted_message, server_message_len);
    crypto_cipher_free(c);
  }

  // Compute AUTH digest.
  uint8_t auth[DIGEST256_LEN];
  {
    crypto_digest_t *d = crypto_digest256_new(DIGEST_SHA3_256);
    d_add_tweak(d, T_AUTH);
    d_add(d, verify, sizeof(verify));
    d_add(d, state->my_id.pubkey, ED25519_PUBKEY_LEN);
    d_add(d, state->my_key.public_key, CURVE25519_PUBKEY_LEN);
    d_add(d, relay_keypair_y->pubkey.public_key, CURVE25519_PUBKEY_LEN);
    d_add(d, state->client_key.public_key, CURVE25519_PUBKEY_LEN);
    d_add(d, state->msg_mac, DIGEST256_LEN);
    d_add_encap(d, encrypted_message, server_message_len);
    d_add(d, (const uint8_t*)PROTOID, strlen(PROTOID));
    d_add(d, (const uint8_t*)"Server", strlen("Server"));
    crypto_digest_get_digest(d, (char *)auth, DIGEST256_LEN);
    crypto_digest_free(d);
  }

  // Compose the reply.
  *handshake_len_out = CURVE25519_PUBKEY_LEN + DIGEST256_LEN +
    server_message_len;
  *handshake_out = tor_malloc(*handshake_len_out);
  uint8_t *ptr = *handshake_out, *end = ptr + *handshake_len_out;
  push(&ptr, end, relay_keypair_y->pubkey.public_key, CURVE25519_PUBKEY_LEN);
  push(&ptr, end, auth, sizeof(auth));
  push(&ptr, end, encrypted_message, server_message_len);
  tor_assert(ptr == end);

  // Clean up and return.
  memwipe(xy, 0, sizeof(xy));
  memwipe(key_seed, 0, sizeof(key_seed));
  memwipe(verify, 0, sizeof(verify));
  memwipe(enc_key, 0, sizeof(enc_key));
  memwipe(encrypted_message, 0, server_message_len);
  tor_free(encrypted_message);

  if (problems) {
    memwipe(*handshake_out, 0, *handshake_len_out);
    tor_free(*handshake_out); // Sets it to NULL.
    *handshake_len_out = 0;
    crypto_rand((char*)keys_out, keys_out_len); // In case bad code uses it.
    return -1;
  }
  return 0;
}

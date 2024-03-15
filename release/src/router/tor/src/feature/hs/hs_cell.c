/* Copyright (c) 2017-2021, The Tor Project, Inc. */
/* See LICENSE for licensing information */

/**
 * \file hs_cell.c
 * \brief Hidden service API for cell creation and handling.
 **/

#include "core/or/or.h"
#include "app/config/config.h"
#include "lib/crypt_ops/crypto_util.h"
#include "feature/hs_common/replaycache.h"

#include "feature/hs/hs_cell.h"
#include "feature/hs/hs_ob.h"
#include "core/crypto/hs_ntor.h"
#include "core/or/congestion_control_common.h"

#include "core/or/origin_circuit_st.h"

/* Trunnel. */
#include "trunnel/congestion_control.h"
#include "trunnel/ed25519_cert.h"
#include "trunnel/extension.h"
#include "trunnel/hs/cell_establish_intro.h"
#include "trunnel/hs/cell_introduce1.h"
#include "trunnel/hs/cell_rendezvous.h"

/** Compute the MAC of an INTRODUCE cell in mac_out. The encoded_cell param is
 * the cell content up to the ENCRYPTED section of length encoded_cell_len.
 * The encrypted param is the start of the ENCRYPTED section of length
 * encrypted_len. The mac_key is the key needed for the computation of the MAC
 * derived from the ntor handshake of length mac_key_len.
 *
 * The length mac_out_len must be at least DIGEST256_LEN. */
static void
compute_introduce_mac(const uint8_t *encoded_cell, size_t encoded_cell_len,
                      const uint8_t *encrypted, size_t encrypted_len,
                      const uint8_t *mac_key, size_t mac_key_len,
                      uint8_t *mac_out, size_t mac_out_len)
{
  size_t offset = 0;
  size_t mac_msg_len;
  uint8_t mac_msg[RELAY_PAYLOAD_SIZE] = {0};

  tor_assert(encoded_cell);
  tor_assert(encrypted);
  tor_assert(mac_key);
  tor_assert(mac_out);
  tor_assert(mac_out_len >= DIGEST256_LEN);

  /* Compute the size of the message which is basically the entire cell until
   * the MAC field of course. */
  mac_msg_len = encoded_cell_len + (encrypted_len - DIGEST256_LEN);
  tor_assert(mac_msg_len <= sizeof(mac_msg));

  /* First, put the encoded cell in the msg. */
  memcpy(mac_msg, encoded_cell, encoded_cell_len);
  offset += encoded_cell_len;
  /* Second, put the CLIENT_PK + ENCRYPTED_DATA but omit the MAC field (which
   * is junk at this point). */
  memcpy(mac_msg + offset, encrypted, (encrypted_len - DIGEST256_LEN));
  offset += (encrypted_len - DIGEST256_LEN);
  tor_assert(offset == mac_msg_len);

  crypto_mac_sha3_256(mac_out, mac_out_len,
                      mac_key, mac_key_len,
                      mac_msg, mac_msg_len);
  memwipe(mac_msg, 0, sizeof(mac_msg));
}

/**
 * From a set of keys, a list of subcredentials, and the ENCRYPTED section of
 * an INTRODUCE2 cell, return an array of newly allocated intro cell keys
 * structures.  Finally, the client public key is copied in client_pk. On
 * error, return NULL.
 **/
static hs_ntor_intro_cell_keys_t *
get_introduce2_key_material(const ed25519_public_key_t *auth_key,
                            const curve25519_keypair_t *enc_key,
                            size_t n_subcredentials,
                            const hs_subcredential_t *subcredentials,
                            const uint8_t *encrypted_section,
                            curve25519_public_key_t *client_pk)
{
  hs_ntor_intro_cell_keys_t *keys;

  tor_assert(auth_key);
  tor_assert(enc_key);
  tor_assert(n_subcredentials > 0);
  tor_assert(subcredentials);
  tor_assert(encrypted_section);
  tor_assert(client_pk);

  keys = tor_calloc(n_subcredentials, sizeof(hs_ntor_intro_cell_keys_t));

  /* First bytes of the ENCRYPTED section are the client public key. */
  memcpy(client_pk->public_key, encrypted_section, CURVE25519_PUBKEY_LEN);

  if (hs_ntor_service_get_introduce1_keys_multi(auth_key, enc_key, client_pk,
                                                n_subcredentials,
                                                subcredentials, keys) < 0) {
    /* Don't rely on the caller to wipe this on error. */
    memwipe(client_pk, 0, sizeof(curve25519_public_key_t));
    tor_free(keys);
    keys = NULL;
  }
  return keys;
}

/** Using the given encryption key, decrypt the encrypted_section of length
 * encrypted_section_len of an INTRODUCE2 cell and return a newly allocated
 * buffer containing the decrypted data. On decryption failure, NULL is
 * returned. */
static uint8_t *
decrypt_introduce2(const uint8_t *enc_key, const uint8_t *encrypted_section,
                   size_t encrypted_section_len)
{
  uint8_t *decrypted = NULL;
  crypto_cipher_t *cipher = NULL;

  tor_assert(enc_key);
  tor_assert(encrypted_section);

  /* Decrypt ENCRYPTED section. */
  cipher = crypto_cipher_new_with_bits((char *) enc_key,
                                       CURVE25519_PUBKEY_LEN * 8);
  tor_assert(cipher);

  /* This is symmetric encryption so can't be bigger than the encrypted
   * section length. */
  decrypted = tor_malloc_zero(encrypted_section_len);
  if (crypto_cipher_decrypt(cipher, (char *) decrypted,
                            (const char *) encrypted_section,
                            encrypted_section_len) < 0) {
    tor_free(decrypted);
    decrypted = NULL;
    goto done;
  }

 done:
  crypto_cipher_free(cipher);
  return decrypted;
}

/** Given a pointer to the decrypted data of the ENCRYPTED section of an
 * INTRODUCE2 cell of length decrypted_len, parse and validate the cell
 * content. Return a newly allocated cell structure or NULL on error. The
 * circuit and service object are only used for logging purposes. */
static trn_cell_introduce_encrypted_t *
parse_introduce2_encrypted(const uint8_t *decrypted_data,
                           size_t decrypted_len, const origin_circuit_t *circ,
                           const hs_service_t *service)
{
  trn_cell_introduce_encrypted_t *enc_cell = NULL;

  tor_assert(decrypted_data);
  tor_assert(circ);
  tor_assert(service);

  if (trn_cell_introduce_encrypted_parse(&enc_cell, decrypted_data,
                                         decrypted_len) < 0) {
    log_info(LD_REND, "Unable to parse the decrypted ENCRYPTED section of "
                      "the INTRODUCE2 cell on circuit %u for service %s",
             TO_CIRCUIT(circ)->n_circ_id,
             safe_str_client(service->onion_address));
    goto err;
  }

  if (trn_cell_introduce_encrypted_get_onion_key_type(enc_cell) !=
      TRUNNEL_HS_INTRO_ONION_KEY_TYPE_NTOR) {
    log_info(LD_REND, "INTRODUCE2 onion key type is invalid. Got %u but "
                      "expected %u on circuit %u for service %s",
             trn_cell_introduce_encrypted_get_onion_key_type(enc_cell),
             TRUNNEL_HS_INTRO_ONION_KEY_TYPE_NTOR,
             TO_CIRCUIT(circ)->n_circ_id,
             safe_str_client(service->onion_address));
    goto err;
  }

  if (trn_cell_introduce_encrypted_getlen_onion_key(enc_cell) !=
      CURVE25519_PUBKEY_LEN) {
    log_info(LD_REND, "INTRODUCE2 onion key length is invalid. Got %u but "
                      "expected %d on circuit %u for service %s",
             (unsigned)trn_cell_introduce_encrypted_getlen_onion_key(enc_cell),
             CURVE25519_PUBKEY_LEN, TO_CIRCUIT(circ)->n_circ_id,
             safe_str_client(service->onion_address));
    goto err;
  }
  /* XXX: Validate NSPEC field as well. */

  return enc_cell;
 err:
  trn_cell_introduce_encrypted_free(enc_cell);
  return NULL;
}

/** Parse an INTRODUCE2 cell from payload of size payload_len for the given
 * service and circuit which are used only for logging purposes. The resulting
 * parsed cell is put in cell_ptr_out.
 *
 * Return 0 on success else a negative value and cell_ptr_out is untouched. */
static int
parse_introduce2_cell(const hs_service_t *service,
                      const origin_circuit_t *circ, const uint8_t *payload,
                      size_t payload_len,
                      trn_cell_introduce1_t **cell_ptr_out)
{
  trn_cell_introduce1_t *cell = NULL;

  tor_assert(service);
  tor_assert(circ);
  tor_assert(payload);
  tor_assert(cell_ptr_out);

  /* Parse the cell so we can start cell validation. */
  if (trn_cell_introduce1_parse(&cell, payload, payload_len) < 0) {
    log_info(LD_PROTOCOL, "Unable to parse INTRODUCE2 cell on circuit %u "
                          "for service %s",
             TO_CIRCUIT(circ)->n_circ_id,
             safe_str_client(service->onion_address));
    goto err;
  }

  /* Success. */
  *cell_ptr_out = cell;
  return 0;
 err:
  return -1;
}

/** Set the onion public key onion_pk in cell, the encrypted section of an
 * INTRODUCE1 cell. */
static void
introduce1_set_encrypted_onion_key(trn_cell_introduce_encrypted_t *cell,
                                   const uint8_t *onion_pk)
{
  tor_assert(cell);
  tor_assert(onion_pk);
  /* There is only one possible key type for a non legacy cell. */
  trn_cell_introduce_encrypted_set_onion_key_type(cell,
                                   TRUNNEL_HS_INTRO_ONION_KEY_TYPE_NTOR);
  trn_cell_introduce_encrypted_set_onion_key_len(cell, CURVE25519_PUBKEY_LEN);
  trn_cell_introduce_encrypted_setlen_onion_key(cell, CURVE25519_PUBKEY_LEN);
  memcpy(trn_cell_introduce_encrypted_getarray_onion_key(cell), onion_pk,
         trn_cell_introduce_encrypted_getlen_onion_key(cell));
}

/** Set the link specifiers in lspecs in cell, the encrypted section of an
 * INTRODUCE1 cell. */
static void
introduce1_set_encrypted_link_spec(trn_cell_introduce_encrypted_t *cell,
                                   const smartlist_t *lspecs)
{
  tor_assert(cell);
  tor_assert(lspecs);
  tor_assert(smartlist_len(lspecs) > 0);
  tor_assert(smartlist_len(lspecs) <= UINT8_MAX);

  uint8_t lspecs_num = (uint8_t) smartlist_len(lspecs);
  trn_cell_introduce_encrypted_set_nspec(cell, lspecs_num);
  /* We aren't duplicating the link specifiers object here which means that
   * the ownership goes to the trn_cell_introduce_encrypted_t cell and those
   * object will be freed when the cell is. */
  SMARTLIST_FOREACH(lspecs, link_specifier_t *, ls,
                    trn_cell_introduce_encrypted_add_nspecs(cell, ls));
}

/** Set padding in the enc_cell only if needed that is the total length of both
 * sections are below the minimum required for an INTRODUCE1 cell. */
static void
introduce1_set_encrypted_padding(const trn_cell_introduce1_t *cell,
                                 trn_cell_introduce_encrypted_t *enc_cell)
{
  tor_assert(cell);
  tor_assert(enc_cell);
  /* This is the length we expect to have once encoded of the whole cell. */
  ssize_t full_len = trn_cell_introduce1_encoded_len(cell) +
                     trn_cell_introduce_encrypted_encoded_len(enc_cell);
  tor_assert(full_len > 0);
  if (full_len < HS_CELL_INTRODUCE1_MIN_SIZE) {
    size_t padding = HS_CELL_INTRODUCE1_MIN_SIZE - full_len;
    trn_cell_introduce_encrypted_setlen_pad(enc_cell, padding);
    memset(trn_cell_introduce_encrypted_getarray_pad(enc_cell), 0,
           trn_cell_introduce_encrypted_getlen_pad(enc_cell));
  }
}

/** Encrypt the ENCRYPTED payload and encode it in the cell using the enc_cell
 * and the INTRODUCE1 data.
 *
 * This can't fail but it is very important that the caller sets every field
 * in data so the computation of the INTRODUCE1 keys doesn't fail. */
static void
introduce1_encrypt_and_encode(trn_cell_introduce1_t *cell,
                              const trn_cell_introduce_encrypted_t *enc_cell,
                              const hs_cell_introduce1_data_t *data)
{
  size_t offset = 0;
  ssize_t encrypted_len;
  ssize_t encoded_cell_len, encoded_enc_cell_len;
  uint8_t encoded_cell[RELAY_PAYLOAD_SIZE] = {0};
  uint8_t encoded_enc_cell[RELAY_PAYLOAD_SIZE] = {0};
  uint8_t *encrypted = NULL;
  uint8_t mac[DIGEST256_LEN];
  crypto_cipher_t *cipher = NULL;
  hs_ntor_intro_cell_keys_t keys;

  tor_assert(cell);
  tor_assert(enc_cell);
  tor_assert(data);

  /* Encode the cells up to now of what we have to we can perform the MAC
   * computation on it. */
  encoded_cell_len = trn_cell_introduce1_encode(encoded_cell,
                                                sizeof(encoded_cell), cell);
  /* We have a much more serious issue if this isn't true. */
  tor_assert(encoded_cell_len > 0);

  encoded_enc_cell_len =
    trn_cell_introduce_encrypted_encode(encoded_enc_cell,
                                        sizeof(encoded_enc_cell), enc_cell);
  /* We have a much more serious issue if this isn't true. */
  tor_assert(encoded_enc_cell_len > 0);

  /* Get the key material for the encryption. */
  if (hs_ntor_client_get_introduce1_keys(data->auth_pk, data->enc_pk,
                                         data->client_kp,
                                         data->subcredential, &keys) < 0) {
    tor_assert_unreached();
  }

  /* Prepare cipher with the encryption key just computed. */
  cipher = crypto_cipher_new_with_bits((const char *) keys.enc_key,
                                       sizeof(keys.enc_key) * 8);
  tor_assert(cipher);

  /* Compute the length of the ENCRYPTED section which is the CLIENT_PK,
   * ENCRYPTED_DATA and MAC length. */
  encrypted_len = sizeof(data->client_kp->pubkey) + encoded_enc_cell_len +
                  sizeof(mac);
  tor_assert(encrypted_len < RELAY_PAYLOAD_SIZE);
  encrypted = tor_malloc_zero(encrypted_len);

  /* Put the CLIENT_PK first. */
  memcpy(encrypted, data->client_kp->pubkey.public_key,
         sizeof(data->client_kp->pubkey.public_key));
  offset += sizeof(data->client_kp->pubkey.public_key);
  /* Then encrypt and set the ENCRYPTED_DATA. This can't fail. */
  crypto_cipher_encrypt(cipher, (char *) encrypted + offset,
                        (const char *) encoded_enc_cell, encoded_enc_cell_len);
  crypto_cipher_free(cipher);
  offset += encoded_enc_cell_len;
  /* Compute MAC from the above and put it in the buffer. This function will
   * make the adjustment to the encrypted_len to omit the MAC length. */
  compute_introduce_mac(encoded_cell, encoded_cell_len,
                        encrypted, encrypted_len,
                        keys.mac_key, sizeof(keys.mac_key),
                        mac, sizeof(mac));
  memcpy(encrypted + offset, mac, sizeof(mac));
  offset += sizeof(mac);
  tor_assert(offset == (size_t) encrypted_len);

  /* Set the ENCRYPTED section in the cell. */
  trn_cell_introduce1_setlen_encrypted(cell, encrypted_len);
  memcpy(trn_cell_introduce1_getarray_encrypted(cell),
         encrypted, encrypted_len);

  /* Cleanup. */
  memwipe(&keys, 0, sizeof(keys));
  memwipe(mac, 0, sizeof(mac));
  memwipe(encrypted, 0, sizeof(encrypted_len));
  memwipe(encoded_enc_cell, 0, sizeof(encoded_enc_cell));
  tor_free(encrypted);
}

/** Build the PoW cell extension and put it in the given extensions object.
 * Return 0 on success, -1 on failure. */
static int
build_introduce_pow_extension(const hs_pow_solution_t *pow_solution,
                              trn_extension_t *extensions)
{
  ssize_t ret;
  size_t pow_ext_encoded_len;
  uint8_t *field_array;
  trn_extension_field_t *field = NULL;
  trn_cell_extension_pow_t *pow_ext = NULL;

  tor_assert(pow_solution);
  tor_assert(extensions);

  /* We are creating a cell extension field of type PoW solution. */
  field = trn_extension_field_new();
  trn_extension_field_set_field_type(field, TRUNNEL_EXT_TYPE_POW);

  /* Build PoW extension field. */
  pow_ext = trn_cell_extension_pow_new();

  /* Copy PoW solution values into PoW extension cell. */

  /* Equi-X base scheme */
  trn_cell_extension_pow_set_pow_version(pow_ext, TRUNNEL_POW_VERSION_EQUIX);

  memcpy(trn_cell_extension_pow_getarray_pow_nonce(pow_ext),
         &pow_solution->nonce, TRUNNEL_POW_NONCE_LEN);

  trn_cell_extension_pow_set_pow_effort(pow_ext, pow_solution->effort);

  memcpy(trn_cell_extension_pow_getarray_pow_seed(pow_ext),
         pow_solution->seed_head, TRUNNEL_POW_SEED_HEAD_LEN);
  memcpy(trn_cell_extension_pow_getarray_pow_solution(pow_ext),
         pow_solution->equix_solution, TRUNNEL_POW_SOLUTION_LEN);

  /* Set the field with the encoded PoW extension. */
  ret = trn_cell_extension_pow_encoded_len(pow_ext);
  if (BUG(ret <= 0)) {
    goto err;
  }
  pow_ext_encoded_len = ret;

  /* Set length field and the field array size length. */
  trn_extension_field_set_field_len(field, pow_ext_encoded_len);
  trn_extension_field_setlen_field(field, pow_ext_encoded_len);
  /* Encode the PoW extension into the cell extension field. */
  field_array = trn_extension_field_getarray_field(field);
  ret = trn_cell_extension_pow_encode(field_array,
                 trn_extension_field_getlen_field(field), pow_ext);
  if (BUG(ret <= 0)) {
    goto err;
  }
  tor_assert(ret == (ssize_t)pow_ext_encoded_len);

  /* Finally, encode field into the cell extension. */
  trn_extension_add_fields(extensions, field);

  /* We've just add an extension field to the cell extensions so increment the
   * total number. */
  trn_extension_set_num(extensions, trn_extension_get_num(extensions) + 1);

  /* Cleanup. PoW extension has been encoded at this point. */
  trn_cell_extension_pow_free(pow_ext);

  return 0;

err:
  trn_extension_field_free(field);
  trn_cell_extension_pow_free(pow_ext);
  return -1;
}

/** Build and set the INTRODUCE congestion control extension in the given
 * extensions. */
static void
build_introduce_cc_extension(trn_extension_t *extensions)
{
  trn_extension_field_t *field = NULL;

  /* Build CC request extension. */
  field = trn_extension_field_new();
  trn_extension_field_set_field_type(field,
                                     TRUNNEL_EXT_TYPE_CC_REQUEST);

  /* No payload indicating a request to use congestion control. */
  trn_extension_field_set_field_len(field, 0);

  /* Build final extension. */
  trn_extension_add_fields(extensions, field);
  trn_extension_set_num(extensions, trn_extension_get_num(extensions) + 1);
}

/** Using the INTRODUCE1 data, setup the ENCRYPTED section in cell. This means
 * set it, encrypt it and encode it. */
static void
introduce1_set_encrypted(trn_cell_introduce1_t *cell,
                         const hs_cell_introduce1_data_t *data)
{
  trn_cell_introduce_encrypted_t *enc_cell;
  trn_extension_t *ext;

  tor_assert(cell);
  tor_assert(data);

  enc_cell = trn_cell_introduce_encrypted_new();
  tor_assert(enc_cell);

  /* Setup extension(s) if any. */
  ext = trn_extension_new();
  tor_assert(ext);
  /* Build congestion control extension if enabled. */
  if (data->cc_enabled) {
    build_introduce_cc_extension(ext);
  }
  /* Build PoW extension if present. */
  if (data->pow_solution) {
    build_introduce_pow_extension(data->pow_solution, ext);
  }
  trn_cell_introduce_encrypted_set_extensions(enc_cell, ext);

  /* Set the rendezvous cookie. */
  memcpy(trn_cell_introduce_encrypted_getarray_rend_cookie(enc_cell),
         data->rendezvous_cookie, REND_COOKIE_LEN);

  /* Set the onion public key. */
  introduce1_set_encrypted_onion_key(enc_cell, data->onion_pk->public_key);

  /* Set the link specifiers. */
  introduce1_set_encrypted_link_spec(enc_cell, data->link_specifiers);

  /* Set padding. */
  introduce1_set_encrypted_padding(cell, enc_cell);

  /* Encrypt and encode it in the cell. */
  introduce1_encrypt_and_encode(cell, enc_cell, data);

  /* Cleanup. */
  trn_cell_introduce_encrypted_free(enc_cell);
}

/** Set the authentication key in the INTRODUCE1 cell from the given data. */
static void
introduce1_set_auth_key(trn_cell_introduce1_t *cell,
                        const hs_cell_introduce1_data_t *data)
{
  tor_assert(cell);
  tor_assert(data);
  /* There is only one possible type for a non legacy cell. */
  trn_cell_introduce1_set_auth_key_type(cell,
                                   TRUNNEL_HS_INTRO_AUTH_KEY_TYPE_ED25519);
  trn_cell_introduce1_set_auth_key_len(cell, ED25519_PUBKEY_LEN);
  trn_cell_introduce1_setlen_auth_key(cell, ED25519_PUBKEY_LEN);
  memcpy(trn_cell_introduce1_getarray_auth_key(cell),
         data->auth_pk->pubkey, trn_cell_introduce1_getlen_auth_key(cell));
}

/** Build and add to the given DoS cell extension the given parameter type and
 * value. */
static void
build_establish_intro_dos_param(trn_cell_extension_dos_t *dos_ext,
                                uint8_t param_type, uint64_t param_value)
{
  trn_cell_extension_dos_param_t *dos_param =
    trn_cell_extension_dos_param_new();

  /* Extra safety. We should never send an unknown parameter type. */
  tor_assert(param_type == TRUNNEL_DOS_PARAM_TYPE_INTRO2_RATE_PER_SEC ||
             param_type == TRUNNEL_DOS_PARAM_TYPE_INTRO2_BURST_PER_SEC);

  trn_cell_extension_dos_param_set_type(dos_param, param_type);
  trn_cell_extension_dos_param_set_value(dos_param, param_value);
  trn_cell_extension_dos_add_params(dos_ext, dos_param);

  /* Not freeing the trunnel object because it is now owned by dos_ext. */
}

/** Build the DoS defense cell extension and put it in the given extensions
 * object. Return 0 on success, -1 on failure.  (Right now, failure is only
 * possible if there is a bug.) */
static int
build_establish_intro_dos_extension(const hs_service_config_t *service_config,
                                    trn_extension_t *extensions)
{
  ssize_t ret;
  size_t dos_ext_encoded_len;
  uint8_t *field_array;
  trn_extension_field_t *field = NULL;
  trn_cell_extension_dos_t *dos_ext = NULL;

  tor_assert(service_config);
  tor_assert(extensions);

  /* We are creating a cell extension field of the type DoS. */
  field = trn_extension_field_new();
  trn_extension_field_set_field_type(field,
                                          TRUNNEL_CELL_EXTENSION_TYPE_DOS);

  /* Build DoS extension field. We will put in two parameters. */
  dos_ext = trn_cell_extension_dos_new();
  trn_cell_extension_dos_set_n_params(dos_ext, 2);

  /* Build DoS parameter INTRO2 rate per second. */
  build_establish_intro_dos_param(dos_ext,
                                  TRUNNEL_DOS_PARAM_TYPE_INTRO2_RATE_PER_SEC,
                                  service_config->intro_dos_rate_per_sec);
  /* Build DoS parameter INTRO2 burst per second. */
  build_establish_intro_dos_param(dos_ext,
                                  TRUNNEL_DOS_PARAM_TYPE_INTRO2_BURST_PER_SEC,
                                  service_config->intro_dos_burst_per_sec);

  /* Set the field with the encoded DoS extension. */
  ret = trn_cell_extension_dos_encoded_len(dos_ext);
  if (BUG(ret <= 0)) {
    goto err;
  }
  dos_ext_encoded_len = ret;
  /* Set length field and the field array size length. */
  trn_extension_field_set_field_len(field, dos_ext_encoded_len);
  trn_extension_field_setlen_field(field, dos_ext_encoded_len);
  /* Encode the DoS extension into the cell extension field. */
  field_array = trn_extension_field_getarray_field(field);
  ret = trn_cell_extension_dos_encode(field_array,
                 trn_extension_field_getlen_field(field), dos_ext);
  if (BUG(ret <= 0)) {
    goto err;
  }
  tor_assert(ret == (ssize_t) dos_ext_encoded_len);

  /* Finally, encode field into the cell extension. */
  trn_extension_add_fields(extensions, field);

  /* We've just add an extension field to the cell extensions so increment the
   * total number. */
  trn_extension_set_num(extensions, trn_extension_get_num(extensions) + 1);

  /* Cleanup. DoS extension has been encoded at this point. */
  trn_cell_extension_dos_free(dos_ext);

  return 0;

 err:
  trn_extension_field_free(field);
  trn_cell_extension_dos_free(dos_ext);
  return -1;
}

/* ========== */
/* Public API */
/* ========== */

/** Allocate and build all the ESTABLISH_INTRO cell extension. The given
 * extensions pointer is always set to a valid cell extension object. */
STATIC trn_extension_t *
build_establish_intro_extensions(const hs_service_config_t *service_config,
                                 const hs_service_intro_point_t *ip)
{
  int ret;
  trn_extension_t *extensions;

  tor_assert(service_config);
  tor_assert(ip);

  extensions = trn_extension_new();
  trn_extension_set_num(extensions, 0);

  /* If the defense has been enabled service side (by the operator with a
   * torrc option) and the intro point does support it. */
  if (service_config->has_dos_defense_enabled &&
      ip->support_intro2_dos_defense) {
    /* This function takes care to increment the number of extensions. */
    ret = build_establish_intro_dos_extension(service_config, extensions);
    if (ret < 0) {
      /* Return no extensions on error. */
      goto end;
    }
  }

 end:
  return extensions;
}

/** Build an ESTABLISH_INTRO cell with the given circuit nonce and intro point
 * object. The encoded cell is put in cell_out that MUST at least be of the
 * size of RELAY_PAYLOAD_SIZE. Return the encoded cell length on success else
 * a negative value and cell_out is untouched. */
ssize_t
hs_cell_build_establish_intro(const char *circ_nonce,
                              const hs_service_config_t *service_config,
                              const hs_service_intro_point_t *ip,
                              uint8_t *cell_out)
{
  ssize_t cell_len = -1;
  uint16_t sig_len = ED25519_SIG_LEN;
  trn_cell_establish_intro_t *cell = NULL;
  trn_extension_t *extensions;

  tor_assert(circ_nonce);
  tor_assert(service_config);
  tor_assert(ip);

  /* Build the extensions, if any. */
  extensions = build_establish_intro_extensions(service_config, ip);

  /* Set extension data. None used here. */
  cell = trn_cell_establish_intro_new();
  trn_cell_establish_intro_set_extensions(cell, extensions);
  /* Set signature size. Array is then allocated in the cell. We need to do
   * this early so we can use trunnel API to get the signature length. */
  trn_cell_establish_intro_set_sig_len(cell, sig_len);
  trn_cell_establish_intro_setlen_sig(cell, sig_len);

  /* Set AUTH_KEY_TYPE: 2 means ed25519 */
  trn_cell_establish_intro_set_auth_key_type(cell,
                                    TRUNNEL_HS_INTRO_AUTH_KEY_TYPE_ED25519);

  /* Set AUTH_KEY and AUTH_KEY_LEN field. Must also set byte-length of
   * AUTH_KEY to match */
  {
    uint16_t auth_key_len = ED25519_PUBKEY_LEN;
    trn_cell_establish_intro_set_auth_key_len(cell, auth_key_len);
    trn_cell_establish_intro_setlen_auth_key(cell, auth_key_len);
    /* We do this call _after_ setting the length because it's reallocated at
     * that point only. */
    uint8_t *auth_key_ptr = trn_cell_establish_intro_getarray_auth_key(cell);
    memcpy(auth_key_ptr, ip->auth_key_kp.pubkey.pubkey, auth_key_len);
  }

  /* Calculate HANDSHAKE_AUTH field (MAC). */
  {
    ssize_t tmp_cell_enc_len = 0;
    ssize_t tmp_cell_mac_offset =
      sig_len + sizeof(cell->sig_len) +
      trn_cell_establish_intro_getlen_handshake_mac(cell);
    uint8_t tmp_cell_enc[RELAY_PAYLOAD_SIZE] = {0};
    uint8_t mac[TRUNNEL_SHA3_256_LEN], *handshake_ptr;

    /* We first encode the current fields we have in the cell so we can
     * compute the MAC using the raw bytes. */
    tmp_cell_enc_len = trn_cell_establish_intro_encode(tmp_cell_enc,
                                                       sizeof(tmp_cell_enc),
                                                       cell);
    if (BUG(tmp_cell_enc_len < 0)) {
      goto done;
    }
    /* Sanity check. */
    tor_assert(tmp_cell_enc_len > tmp_cell_mac_offset);

    /* Circuit nonce is always DIGEST_LEN according to tor-spec.txt. */
    crypto_mac_sha3_256(mac, sizeof(mac),
                        (uint8_t *) circ_nonce, DIGEST_LEN,
                        tmp_cell_enc, tmp_cell_enc_len - tmp_cell_mac_offset);
    handshake_ptr = trn_cell_establish_intro_getarray_handshake_mac(cell);
    memcpy(handshake_ptr, mac, sizeof(mac));

    memwipe(mac, 0, sizeof(mac));
    memwipe(tmp_cell_enc, 0, sizeof(tmp_cell_enc));
  }

  /* Calculate the cell signature SIG. */
  {
    ssize_t tmp_cell_enc_len = 0;
    ssize_t tmp_cell_sig_offset = (sig_len + sizeof(cell->sig_len));
    uint8_t tmp_cell_enc[RELAY_PAYLOAD_SIZE] = {0}, *sig_ptr;
    ed25519_signature_t sig;

    /* We first encode the current fields we have in the cell so we can
     * compute the signature from the raw bytes of the cell. */
    tmp_cell_enc_len = trn_cell_establish_intro_encode(tmp_cell_enc,
                                                       sizeof(tmp_cell_enc),
                                                       cell);
    if (BUG(tmp_cell_enc_len < 0)) {
      goto done;
    }

    if (ed25519_sign_prefixed(&sig, tmp_cell_enc,
                              tmp_cell_enc_len - tmp_cell_sig_offset,
                              ESTABLISH_INTRO_SIG_PREFIX, &ip->auth_key_kp)) {
      log_warn(LD_BUG, "Unable to make signature for ESTABLISH_INTRO cell.");
      goto done;
    }
    /* Copy the signature into the cell. */
    sig_ptr = trn_cell_establish_intro_getarray_sig(cell);
    memcpy(sig_ptr, sig.sig, sig_len);

    memwipe(tmp_cell_enc, 0, sizeof(tmp_cell_enc));
  }

  /* Encode the cell. Can't be bigger than a standard cell. */
  cell_len = trn_cell_establish_intro_encode(cell_out, RELAY_PAYLOAD_SIZE,
                                             cell);

 done:
  trn_cell_establish_intro_free(cell);
  return cell_len;
}

/** Parse the INTRO_ESTABLISHED cell in the payload of size payload_len. If we
 * are successful at parsing it, return the length of the parsed cell else a
 * negative value on error. */
ssize_t
hs_cell_parse_intro_established(const uint8_t *payload, size_t payload_len)
{
  ssize_t ret;
  trn_cell_intro_established_t *cell = NULL;

  tor_assert(payload);

  /* Try to parse the payload into a cell making sure we do actually have a
   * valid cell. */
  ret = trn_cell_intro_established_parse(&cell, payload, payload_len);
  if (ret >= 0) {
    /* On success, we do not keep the cell, we just notify the caller that it
     * was successfully parsed. */
    trn_cell_intro_established_free(cell);
  }
  return ret;
}

/** Parse the cell PoW solution extension. Return 0 on success and data
 * structure is updated with the PoW effort. Return -1 on any kind of error
 * including if PoW couldn't be verified. */
static int
handle_introduce2_encrypted_cell_pow_extension(const hs_service_t *service,
                                const hs_service_intro_point_t *ip,
                                const trn_extension_field_t *field,
                                hs_cell_introduce2_data_t *data)
{
  int ret = -1;
  trn_cell_extension_pow_t *pow = NULL;
  hs_pow_solution_t sol;

  tor_assert(field);
  tor_assert(ip);

  if (!service->state.pow_state) {
    log_info(LD_REND, "Unsolicited PoW solution in INTRODUCE2 request.");
    goto end;
  }

  if (trn_cell_extension_pow_parse(&pow,
               trn_extension_field_getconstarray_field(field),
               trn_extension_field_getlen_field(field)) < 0) {
    goto end;
  }

  /* There is only one version supported at the moment so validate we at least
   * have that. */
  if (trn_cell_extension_pow_get_pow_version(pow) !=
      TRUNNEL_POW_VERSION_EQUIX) {
    log_debug(LD_REND, "Unsupported PoW version. Malformed INTRODUCE2");
    goto end;
  }

  /* Effort E */
  sol.effort = trn_cell_extension_pow_get_pow_effort(pow);
  /* Seed C */
  memcpy(sol.seed_head, trn_cell_extension_pow_getconstarray_pow_seed(pow),
         HS_POW_SEED_HEAD_LEN);
  /* Nonce N */
  memcpy(sol.nonce, trn_cell_extension_pow_getconstarray_pow_nonce(pow),
         HS_POW_NONCE_LEN);
  /* Solution S */
  memcpy(sol.equix_solution,
         trn_cell_extension_pow_getconstarray_pow_solution(pow),
         HS_POW_EQX_SOL_LEN);

  if (hs_pow_verify(&ip->blinded_id, service->state.pow_state, &sol)) {
    log_info(LD_REND, "PoW INTRODUCE2 request failed to verify.");
    goto end;
  }

  log_info(LD_REND, "PoW INTRODUCE2 request successfully verified.");
  data->rdv_data.pow_effort = sol.effort;

  /* Successfully parsed and verified the PoW solution */
  ret = 0;

 end:
  trn_cell_extension_pow_free(pow);
  return ret;
}

/** For the encrypted INTRO2 cell in <b>encrypted_section</b>, use the crypto
 * material in <b>data</b> to compute the right ntor keys. Also validate the
 * INTRO2 MAC to ensure that the keys are the right ones.
 *
 * Return NULL on failure to either produce the key material or on MAC
 * validation. Else return a newly allocated intro keys object. */
static hs_ntor_intro_cell_keys_t *
get_introduce2_keys_and_verify_mac(hs_cell_introduce2_data_t *data,
                                   const uint8_t *encrypted_section,
                                   size_t encrypted_section_len)
{
  hs_ntor_intro_cell_keys_t *intro_keys = NULL;
  hs_ntor_intro_cell_keys_t *intro_keys_result = NULL;

  /* Build the key material out of the key material found in the cell. */
  intro_keys = get_introduce2_key_material(data->auth_pk, data->enc_kp,
                                           data->n_subcredentials,
                                           data->subcredentials,
                                           encrypted_section,
                                           &data->rdv_data.client_pk);
  if (intro_keys == NULL) {
    log_info(LD_REND, "Invalid INTRODUCE2 encrypted data. Unable to "
             "compute key material");
    return NULL;
  }

  /* Make sure we are not about to underflow. */
  if (BUG(encrypted_section_len < DIGEST256_LEN)) {
    return NULL;
  }

  /* Validate MAC from the cell and our computed key material. The MAC field
   * in the cell is at the end of the encrypted section. */
  intro_keys_result = tor_malloc_zero(sizeof(*intro_keys_result));
  for (unsigned i = 0; i < data->n_subcredentials; ++i) {
    uint8_t mac[DIGEST256_LEN];

    /* The MAC field is at the very end of the ENCRYPTED section. */
    size_t mac_offset = encrypted_section_len - sizeof(mac);
    /* Compute the MAC. Use the entire encoded payload with a length up to the
     * ENCRYPTED section. */
    compute_introduce_mac(data->payload,
                          data->payload_len - encrypted_section_len,
                          encrypted_section, encrypted_section_len,
                          intro_keys[i].mac_key,
                          sizeof(intro_keys[i].mac_key),
                          mac, sizeof(mac));
    /* Time-invariant conditional copy: if the MAC is what we expected, then
     * set intro_keys_result to intro_keys[i]. Otherwise, don't: but don't
     * leak which one it was! */
    bool equal = tor_memeq(mac, encrypted_section + mac_offset, sizeof(mac));
    memcpy_if_true_timei(equal, intro_keys_result, &intro_keys[i],
                         sizeof(*intro_keys_result));
  }

  /* We no longer need intro_keys. */
  memwipe(intro_keys, 0,
          sizeof(hs_ntor_intro_cell_keys_t) * data->n_subcredentials);
  tor_free(intro_keys);

  if (safe_mem_is_zero(intro_keys_result, sizeof(*intro_keys_result))) {
    log_info(LD_REND, "Invalid MAC validation for INTRODUCE2 cell");
    tor_free(intro_keys_result); /* sets intro_keys_result to NULL */
  }

  return intro_keys_result;
}

/** Parse the given INTRODUCE cell extension. Update the data object
 * accordingly depending on the extension. Return 0 if it validated
 * correctly, or return -1 if it is malformed (for example because it
 * includes a PoW that doesn't verify). */
static int
parse_introduce_cell_extension(const hs_service_t *service,
                               const hs_service_intro_point_t *ip,
                               hs_cell_introduce2_data_t *data,
                               const trn_extension_field_t *field)
{
  int ret = 0;
  trn_extension_field_cc_t *cc_field = NULL;

  tor_assert(data);
  tor_assert(field);

  switch (trn_extension_field_get_field_type(field)) {
  case TRUNNEL_EXT_TYPE_CC_REQUEST:
    /* CC requests, enable it. */
    data->rdv_data.cc_enabled = 1;
    data->pv.protocols_known = 1;
    data->pv.supports_congestion_control = data->rdv_data.cc_enabled;
    break;
  case TRUNNEL_EXT_TYPE_POW:
    /* PoW request. If successful, the effort is put in the data. */
    if (handle_introduce2_encrypted_cell_pow_extension(service, ip,
                                                       field, data) < 0) {
      log_fn(LOG_PROTOCOL_WARN, LD_REND, "Invalid PoW cell extension.");
      ret = -1;
    }
    break;
  default:
    break;
  }

  trn_extension_field_cc_free(cc_field);
  return ret;
}

/** Parse the INTRODUCE2 cell using data which contains everything we need to
 * do so and contains the destination buffers of information we extract and
 * compute from the cell. Return 0 on success else a negative value. The
 * service and circ are only used for logging purposes. */
ssize_t
hs_cell_parse_introduce2(hs_cell_introduce2_data_t *data,
                         const origin_circuit_t *circ,
                         const hs_service_t *service,
                         const hs_service_intro_point_t *ip)
{
  int ret = -1;
  time_t elapsed;
  uint8_t *decrypted = NULL;
  size_t encrypted_section_len;
  const uint8_t *encrypted_section;
  trn_cell_introduce1_t *cell = NULL;
  trn_cell_introduce_encrypted_t *enc_cell = NULL;
  hs_ntor_intro_cell_keys_t *intro_keys = NULL;

  tor_assert(data);
  tor_assert(circ);
  tor_assert(service);

  /* Parse the cell into a decoded data structure pointed by cell_ptr. */
  if (parse_introduce2_cell(service, circ, data->payload, data->payload_len,
                            &cell) < 0) {
    goto done;
  }

  log_info(LD_REND, "Received a decodable INTRODUCE2 cell on circuit %u "
                    "for service %s. Decoding encrypted section...",
           TO_CIRCUIT(circ)->n_circ_id,
           safe_str_client(service->onion_address));

  encrypted_section = trn_cell_introduce1_getconstarray_encrypted(cell);
  encrypted_section_len = trn_cell_introduce1_getlen_encrypted(cell);

  /* Encrypted section must at least contain the CLIENT_PK and MAC which is
   * defined in section 3.3.2 of the specification. */
  if (encrypted_section_len < (CURVE25519_PUBKEY_LEN + DIGEST256_LEN)) {
    log_info(LD_REND, "Invalid INTRODUCE2 encrypted section length "
                      "for service %s. Dropping cell.",
             safe_str_client(service->onion_address));
    goto done;
  }

  /* Check our replay cache for this introduction point. */
  if (replaycache_add_test_and_elapsed(data->replay_cache, encrypted_section,
                                       encrypted_section_len, &elapsed)) {
    log_warn(LD_REND, "Possible replay detected! An INTRODUCE2 cell with the "
                      "same ENCRYPTED section was seen %ld seconds ago. "
                      "Dropping cell.", (long int) elapsed);
    goto done;
  }

  /* First bytes of the ENCRYPTED section are the client public key (they are
   * guaranteed to exist because of the length check above). We are gonna use
   * the client public key to compute the ntor keys and decrypt the payload:
   */
  memcpy(&data->rdv_data.client_pk.public_key, encrypted_section,
         CURVE25519_PUBKEY_LEN);

  /* Get the right INTRODUCE2 ntor keys and verify the cell MAC */
  intro_keys = get_introduce2_keys_and_verify_mac(data, encrypted_section,
                                                  encrypted_section_len);
  if (!intro_keys) {
    log_warn(LD_REND, "Could not get valid INTRO2 keys on circuit %u "
             "for service %s", TO_CIRCUIT(circ)->n_circ_id,
             safe_str_client(service->onion_address));
    goto done;
  }

  {
    /* The ENCRYPTED_DATA section starts just after the CLIENT_PK. */
    const uint8_t *encrypted_data =
      encrypted_section + sizeof(data->rdv_data.client_pk);
    /* It's symmetric encryption so it's correct to use the ENCRYPTED length
     * for decryption. Computes the length of ENCRYPTED_DATA meaning removing
     * the CLIENT_PK and MAC length. */
    size_t encrypted_data_len =
      encrypted_section_len -
      (sizeof(data->rdv_data.client_pk) + DIGEST256_LEN);

    /* This decrypts the ENCRYPTED_DATA section of the cell. */
    decrypted = decrypt_introduce2(intro_keys->enc_key,
                                   encrypted_data, encrypted_data_len);
    if (decrypted == NULL) {
      log_info(LD_REND, "Unable to decrypt the ENCRYPTED section of an "
                        "INTRODUCE2 cell on circuit %u for service %s",
               TO_CIRCUIT(circ)->n_circ_id,
               safe_str_client(service->onion_address));
      goto done;
    }

    /* Parse this blob into an encrypted cell structure so we can then extract
     * the data we need out of it. */
    enc_cell = parse_introduce2_encrypted(decrypted, encrypted_data_len,
                                          circ, service);
    memwipe(decrypted, 0, encrypted_data_len);
    if (enc_cell == NULL) {
      goto done;
    }
  }

  /* XXX: Implement client authorization checks. */

  /* Extract onion key and rendezvous cookie from the cell used for the
   * rendezvous point circuit e2e encryption. */
  memcpy(data->rdv_data.onion_pk.public_key,
         trn_cell_introduce_encrypted_getconstarray_onion_key(enc_cell),
         CURVE25519_PUBKEY_LEN);
  memcpy(data->rdv_data.rendezvous_cookie,
         trn_cell_introduce_encrypted_getconstarray_rend_cookie(enc_cell),
         sizeof(data->rdv_data.rendezvous_cookie));

  /* Extract rendezvous link specifiers. */
  for (size_t idx = 0;
       idx < trn_cell_introduce_encrypted_get_nspec(enc_cell); idx++) {
    link_specifier_t *lspec =
      trn_cell_introduce_encrypted_get_nspecs(enc_cell, idx);
    if (BUG(!lspec)) {
      goto done;
    }
    link_specifier_t *lspec_dup = link_specifier_dup(lspec);
    if (BUG(!lspec_dup)) {
      goto done;
    }
    smartlist_add(data->rdv_data.link_specifiers, lspec_dup);
  }

  /* Extract any extensions. */
  const trn_extension_t *extensions =
    trn_cell_introduce_encrypted_get_extensions(enc_cell);
  if (extensions != NULL) {
    for (size_t idx = 0; idx < trn_extension_get_num(extensions); idx++) {
      const trn_extension_field_t *field =
        trn_extension_getconst_fields(extensions, idx);
      if (BUG(field == NULL)) {
        /* The number of extensions should match the number of fields. */
        break;
      }
      if (parse_introduce_cell_extension(service, ip, data, field) < 0) {
        goto done;
      }
    }
  }

  /* If the client asked for congestion control, but we don't support it,
   * that's a failure. It should not have asked, based on our descriptor. */
  if (data->rdv_data.cc_enabled && !congestion_control_enabled()) {
    goto done;
  }

  /* Success. */
  ret = 0;
  log_info(LD_REND,
           "Valid INTRODUCE2 cell. Willing to launch rendezvous circuit.");

 done:
  if (intro_keys) {
    memwipe(intro_keys, 0, sizeof(hs_ntor_intro_cell_keys_t));
    tor_free(intro_keys);
  }
  tor_free(decrypted);
  trn_cell_introduce_encrypted_free(enc_cell);
  trn_cell_introduce1_free(cell);
  return ret;
}

/** Build a RENDEZVOUS1 cell with the given rendezvous cookie and handshake
 * info. The encoded cell is put in cell_out and the length of the data is
 * returned. This can't fail. */
ssize_t
hs_cell_build_rendezvous1(const uint8_t *rendezvous_cookie,
                          size_t rendezvous_cookie_len,
                          const uint8_t *rendezvous_handshake_info,
                          size_t rendezvous_handshake_info_len,
                          uint8_t *cell_out)
{
  ssize_t cell_len;
  trn_cell_rendezvous1_t *cell;

  tor_assert(rendezvous_cookie);
  tor_assert(rendezvous_handshake_info);
  tor_assert(cell_out);

  cell = trn_cell_rendezvous1_new();
  /* Set the RENDEZVOUS_COOKIE. */
  memcpy(trn_cell_rendezvous1_getarray_rendezvous_cookie(cell),
         rendezvous_cookie, rendezvous_cookie_len);
  /* Set the HANDSHAKE_INFO. */
  trn_cell_rendezvous1_setlen_handshake_info(cell,
                                            rendezvous_handshake_info_len);
  memcpy(trn_cell_rendezvous1_getarray_handshake_info(cell),
         rendezvous_handshake_info, rendezvous_handshake_info_len);
  /* Encoding. */
  cell_len = trn_cell_rendezvous1_encode(cell_out, RELAY_PAYLOAD_SIZE, cell);
  tor_assert(cell_len > 0);

  trn_cell_rendezvous1_free(cell);
  return cell_len;
}

/** Build an INTRODUCE1 cell from the given data. The encoded cell is put in
 * cell_out which must be of at least size RELAY_PAYLOAD_SIZE. On success, the
 * encoded length is returned else a negative value and the content of
 * cell_out should be ignored. */
ssize_t
hs_cell_build_introduce1(const hs_cell_introduce1_data_t *data,
                         uint8_t *cell_out)
{
  ssize_t cell_len;
  trn_cell_introduce1_t *cell;
  trn_extension_t *ext;

  tor_assert(data);
  tor_assert(cell_out);

  cell = trn_cell_introduce1_new();
  tor_assert(cell);

  /* Set extension data. None are used. */
  ext = trn_extension_new();
  tor_assert(ext);
  trn_extension_set_num(ext, 0);
  trn_cell_introduce1_set_extensions(cell, ext);

  /* Set the authentication key. */
  introduce1_set_auth_key(cell, data);

  /* Set the encrypted section. This will set, encrypt and encode the
   * ENCRYPTED section in the cell. After this, we'll be ready to encode. */
  introduce1_set_encrypted(cell, data);

  /* Final encoding. */
  cell_len = trn_cell_introduce1_encode(cell_out, RELAY_PAYLOAD_SIZE, cell);

  trn_cell_introduce1_free(cell);
  return cell_len;
}

/** Build an ESTABLISH_RENDEZVOUS cell from the given rendezvous_cookie. The
 * encoded cell is put in cell_out which must be of at least
 * RELAY_PAYLOAD_SIZE. On success, the encoded length is returned and the
 * caller should clear up the content of the cell.
 *
 * This function can't fail. */
ssize_t
hs_cell_build_establish_rendezvous(const uint8_t *rendezvous_cookie,
                                   uint8_t *cell_out)
{
  tor_assert(rendezvous_cookie);
  tor_assert(cell_out);

  memcpy(cell_out, rendezvous_cookie, HS_REND_COOKIE_LEN);
  return HS_REND_COOKIE_LEN;
}

/** Handle an INTRODUCE_ACK cell encoded in payload of length payload_len.
 * Return the status code on success else a negative value if the cell as not
 * decodable. */
int
hs_cell_parse_introduce_ack(const uint8_t *payload, size_t payload_len)
{
  int ret = -1;
  trn_cell_introduce_ack_t *cell = NULL;

  tor_assert(payload);

  if (trn_cell_introduce_ack_parse(&cell, payload, payload_len) < 0) {
    log_info(LD_REND, "Invalid INTRODUCE_ACK cell. Unable to parse it.");
    goto end;
  }

  ret = trn_cell_introduce_ack_get_status(cell);

 end:
  trn_cell_introduce_ack_free(cell);
  return ret;
}

/** Handle a RENDEZVOUS2 cell encoded in payload of length payload_len. On
 * success, handshake_info contains the data in the HANDSHAKE_INFO field, and
 * 0 is returned. On error, a negative value is returned. */
int
hs_cell_parse_rendezvous2(const uint8_t *payload, size_t payload_len,
                          uint8_t *handshake_info, size_t handshake_info_len)
{
  int ret = -1;
  trn_cell_rendezvous2_t *cell = NULL;

  tor_assert(payload);
  tor_assert(handshake_info);

  if (trn_cell_rendezvous2_parse(&cell, payload, payload_len) < 0) {
    log_info(LD_REND, "Invalid RENDEZVOUS2 cell. Unable to parse it.");
    goto end;
  }

  /* Static size, we should never have an issue with this else we messed up
   * our code flow. */
  tor_assert(trn_cell_rendezvous2_getlen_handshake_info(cell) ==
             handshake_info_len);
  memcpy(handshake_info,
         trn_cell_rendezvous2_getconstarray_handshake_info(cell),
         handshake_info_len);
  ret = 0;

 end:
  trn_cell_rendezvous2_free(cell);
  return ret;
}

/** Clear the given INTRODUCE1 data structure data. */
void
hs_cell_introduce1_data_clear(hs_cell_introduce1_data_t *data)
{
  if (data == NULL) {
    return;
  }
  /* Object in this list have been moved to the cell object when building it
   * so they've been freed earlier. We do that in order to avoid duplicating
   * them leading to more memory and CPU time being used for nothing. */
  smartlist_free(data->link_specifiers);
  /* The data object has no ownership of any members. */
  memwipe(data, 0, sizeof(hs_cell_introduce1_data_t));
}

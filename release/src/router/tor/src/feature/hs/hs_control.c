/* Copyright (c) 2017-2020, The Tor Project, Inc. */
/* See LICENSE for licensing information */

/**
 * \file hs_control.c
 * \brief Contains control port event related code.
 **/

#include "core/or/or.h"
#include "feature/control/control_events.h"
#include "lib/crypt_ops/crypto_format.h"
#include "lib/crypt_ops/crypto_util.h"
#include "feature/hs/hs_client.h"
#include "feature/hs/hs_common.h"
#include "feature/hs/hs_control.h"
#include "feature/hs/hs_descriptor.h"
#include "feature/hs/hs_service.h"
#include "feature/nodelist/nodelist.h"

#include "feature/nodelist/node_st.h"
#include "feature/nodelist/routerstatus_st.h"

/** Send on the control port the "HS_DESC REQUESTED [...]" event.
 *
 * The onion_pk is the onion service public key, base64_blinded_pk is the
 * base64 encoded blinded key for the service and hsdir_rs is the routerstatus
 * object of the HSDir that this request is for. */
void
hs_control_desc_event_requested(const ed25519_public_key_t *onion_pk,
                                const char *base64_blinded_pk,
                                const routerstatus_t *hsdir_rs)
{
  char onion_address[HS_SERVICE_ADDR_LEN_BASE32 + 1];
  const uint8_t *hsdir_index;
  const node_t *hsdir_node;

  tor_assert(onion_pk);
  tor_assert(base64_blinded_pk);
  tor_assert(hsdir_rs);

  hs_build_address(onion_pk, HS_VERSION_THREE, onion_address);

  /* Get the node from the routerstatus object to get the HSDir index used for
   * this request. We can't have a routerstatus entry without a node and we
   * can't pick a node without an hsdir_index. */
  hsdir_node = node_get_by_id(hsdir_rs->identity_digest);
  tor_assert(hsdir_node);
  /* This is a fetch event. */
  hsdir_index = hsdir_node->hsdir_index.fetch;

  /* Trigger the event. */
  control_event_hs_descriptor_requested(onion_address, REND_NO_AUTH,
                                        hsdir_rs->identity_digest,
                                        base64_blinded_pk,
                                        hex_str((const char *) hsdir_index,
                                                DIGEST256_LEN));
  memwipe(onion_address, 0, sizeof(onion_address));
}

/** Send on the control port the "HS_DESC FAILED [...]" event.
 *
 * Using a directory connection identifier, the HSDir identity digest and a
 * reason for the failure. None can be NULL. */
void
hs_control_desc_event_failed(const hs_ident_dir_conn_t *ident,
                             const char *hsdir_id_digest,
                             const char *reason)
{
  char onion_address[HS_SERVICE_ADDR_LEN_BASE32 + 1];
  char base64_blinded_pk[ED25519_BASE64_LEN + 1];

  tor_assert(ident);
  tor_assert(hsdir_id_digest);
  tor_assert(reason);

  /* Build onion address and encoded blinded key. */
  ed25519_public_to_base64(base64_blinded_pk, &ident->blinded_pk);
  hs_build_address(&ident->identity_pk, HS_VERSION_THREE, onion_address);

  control_event_hsv3_descriptor_failed(onion_address, base64_blinded_pk,
                                       hsdir_id_digest, reason);
}

/** Send on the control port the "HS_DESC RECEIVED [...]" event.
 *
 * Using a directory connection identifier and the HSDir identity digest.
 * None can be NULL. */
void
hs_control_desc_event_received(const hs_ident_dir_conn_t *ident,
                               const char *hsdir_id_digest)
{
  char onion_address[HS_SERVICE_ADDR_LEN_BASE32 + 1];
  char base64_blinded_pk[ED25519_BASE64_LEN + 1];

  tor_assert(ident);
  tor_assert(hsdir_id_digest);

  /* Build onion address and encoded blinded key. */
  ed25519_public_to_base64(base64_blinded_pk, &ident->blinded_pk);
  hs_build_address(&ident->identity_pk, HS_VERSION_THREE, onion_address);

  control_event_hsv3_descriptor_received(onion_address, base64_blinded_pk,
                                         hsdir_id_digest);
}

/** Send on the control port the "HS_DESC CREATED [...]" event.
 *
 * Using the onion address of the descriptor's service and the blinded public
 * key of the descriptor as a descriptor ID. None can be NULL. */
void
hs_control_desc_event_created(const char *onion_address,
                              const ed25519_public_key_t *blinded_pk)
{
  char base64_blinded_pk[ED25519_BASE64_LEN + 1];

  tor_assert(onion_address);
  tor_assert(blinded_pk);

  /* Build base64 encoded blinded key. */
  ed25519_public_to_base64(base64_blinded_pk, blinded_pk);

  /* Version 3 doesn't use the replica number in its descriptor ID computation
   * so we pass negative value so the control port subsystem can ignore it. */
  control_event_hs_descriptor_created(onion_address, base64_blinded_pk, -1);
}

/** Send on the control port the "HS_DESC UPLOAD [...]" event.
 *
 * Using the onion address of the descriptor's service, the HSDir identity
 * digest, the blinded public key of the descriptor as a descriptor ID and the
 * HSDir index for this particular request. None can be NULL. */
void
hs_control_desc_event_upload(const char *onion_address,
                             const char *hsdir_id_digest,
                             const ed25519_public_key_t *blinded_pk,
                             const uint8_t *hsdir_index)
{
  char base64_blinded_pk[ED25519_BASE64_LEN + 1];

  tor_assert(onion_address);
  tor_assert(hsdir_id_digest);
  tor_assert(blinded_pk);
  tor_assert(hsdir_index);

  /* Build base64 encoded blinded key. */
  ed25519_public_to_base64(base64_blinded_pk, blinded_pk);

  control_event_hs_descriptor_upload(onion_address, hsdir_id_digest,
                                     base64_blinded_pk,
                                     hex_str((const char *) hsdir_index,
                                             DIGEST256_LEN));
}

/** Send on the control port the "HS_DESC UPLOADED [...]" event.
 *
 * Using the directory connection identifier and the HSDir identity digest.
 * None can be NULL. */
void
hs_control_desc_event_uploaded(const hs_ident_dir_conn_t *ident,
                               const char *hsdir_id_digest)
{
  char onion_address[HS_SERVICE_ADDR_LEN_BASE32 + 1];

  tor_assert(ident);
  tor_assert(hsdir_id_digest);

  hs_build_address(&ident->identity_pk, HS_VERSION_THREE, onion_address);

  control_event_hs_descriptor_uploaded(hsdir_id_digest, onion_address);
}

/** Send on the control port the "HS_DESC_CONTENT [...]" event.
 *
 * Using the directory connection identifier, the HSDir identity digest and
 * the body of the descriptor (as it was received from the directory). None
 * can be NULL. */
void
hs_control_desc_event_content(const hs_ident_dir_conn_t *ident,
                              const char *hsdir_id_digest,
                              const char *body)
{
  char onion_address[HS_SERVICE_ADDR_LEN_BASE32 + 1];
  char base64_blinded_pk[ED25519_BASE64_LEN + 1];

  tor_assert(ident);
  tor_assert(hsdir_id_digest);

  /* Build onion address and encoded blinded key. */
  ed25519_public_to_base64(base64_blinded_pk, &ident->blinded_pk);
  hs_build_address(&ident->identity_pk, HS_VERSION_THREE, onion_address);

  control_event_hs_descriptor_content(onion_address, base64_blinded_pk,
                                      hsdir_id_digest, body);
}

/** Handle the "HSPOST [...]" command. The body is an encoded descriptor for
 * the given onion_address. The descriptor will be uploaded to each directory
 * in hsdirs_rs. If NULL, the responsible directories for the current time
 * period will be selected.
 *
 * Return -1 on if the descriptor plaintext section is not decodable. Else, 0
 * on success. */
int
hs_control_hspost_command(const char *body, const char *onion_address,
                          const smartlist_t *hsdirs_rs)
{
  int ret = -1;
  ed25519_public_key_t identity_pk;
  hs_desc_plaintext_data_t plaintext;
  smartlist_t *hsdirs = NULL;

  tor_assert(body);
  tor_assert(onion_address);

  /* This can't fail because we require the caller to pass us a valid onion
   * address that has passed hs_address_is_valid(). */
  if (BUG(hs_parse_address(onion_address, &identity_pk, NULL, NULL) < 0)) {
    goto done; // LCOV_EXCL_LINE
  }

  /* Only decode the plaintext part which is what the directory will do to
   * validate before caching. */
  if (hs_desc_decode_plaintext(body, &plaintext) < 0) {
    goto done;
  }

  /* No HSDir(s) given, we'll compute what the current ones should be. */
  if (hsdirs_rs == NULL) {
    hsdirs = smartlist_new();
    hs_get_responsible_hsdirs(&plaintext.blinded_pubkey,
                              hs_get_time_period_num(0),
                              0, /* Always the current descriptor which uses
                                  * the first hsdir index. */
                              0, /* It is for storing on a directory. */
                              hsdirs);
    hsdirs_rs = hsdirs;
  }

  SMARTLIST_FOREACH_BEGIN(hsdirs_rs, const routerstatus_t *, rs) {
    hs_service_upload_desc_to_dir(body, plaintext.version, &identity_pk,
                                  &plaintext.blinded_pubkey, rs);
  } SMARTLIST_FOREACH_END(rs);
  ret = 0;

 done:
  /* We don't have ownership of the objects in this list. */
  smartlist_free(hsdirs);
  return ret;
}

/** With a given <b>onion_identity_pk</b>, fetch its descriptor, optionally
 * using the list of directory servers given in <b>hsdirs</b>, or a random
 * server if it is NULL. This function calls hs_client_launch_v3_desc_fetch().
 */
void
hs_control_hsfetch_command(const ed25519_public_key_t *onion_identity_pk,
                           const smartlist_t *hsdirs)
{
  tor_assert(onion_identity_pk);

  hs_client_launch_v3_desc_fetch(onion_identity_pk, hsdirs);
}

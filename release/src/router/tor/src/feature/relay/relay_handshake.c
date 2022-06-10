/* Copyright (c) 2001 Matej Pfajfar.
 * Copyright (c) 2001-2004, Roger Dingledine.
 * Copyright (c) 2004-2006, Roger Dingledine, Nick Mathewson.
 * Copyright (c) 2007-2021, The Tor Project, Inc. */
/* See LICENSE for licensing information */

/**
 * @file relay_handshake.c
 * @brief Functions to implement the relay-only parts of our
 *    connection handshake.
 *
 * Some parts of our TLS link handshake are only done by relays (including
 * bridges).  Specifically, only relays need to send CERTS cells; only
 * relays need to send or receive AUTHCHALLENGE cells, and only relays need to
 * send or receive AUTHENTICATE cells.
 **/

#include "orconfig.h"
#include "core/or/or.h"
#include "feature/relay/relay_handshake.h"

#include "app/config/config.h"
#include "core/or/connection_or.h"
#include "lib/crypt_ops/crypto_rand.h"
#include "trunnel/link_handshake.h"
#include "feature/relay/routerkeys.h"
#include "feature/nodelist/torcert.h"

#include "core/or/or_connection_st.h"
#include "core/or/or_handshake_certs_st.h"
#include "core/or/or_handshake_state_st.h"
#include "core/or/var_cell_st.h"

#include "lib/tls/tortls.h"
#include "lib/tls/x509.h"

/** Helper used to add an encoded certs to a cert cell */
static void
add_certs_cell_cert_helper(certs_cell_t *certs_cell,
                           uint8_t cert_type,
                           const uint8_t *cert_encoded,
                           size_t cert_len)
{
  tor_assert(cert_len <= UINT16_MAX);
  certs_cell_cert_t *ccc = certs_cell_cert_new();
  ccc->cert_type = cert_type;
  ccc->cert_len = cert_len;
  certs_cell_cert_setlen_body(ccc, cert_len);
  memcpy(certs_cell_cert_getarray_body(ccc), cert_encoded, cert_len);

  certs_cell_add_certs(certs_cell, ccc);
}

/** Add an encoded X509 cert (stored as <b>cert_len</b> bytes at
 * <b>cert_encoded</b>) to the trunnel certs_cell_t object that we are
 * building in <b>certs_cell</b>.  Set its type field to <b>cert_type</b>.
 * (If <b>cert</b> is NULL, take no action.) */
static void
add_x509_cert(certs_cell_t *certs_cell,
              uint8_t cert_type,
              const tor_x509_cert_t *cert)
{
  if (NULL == cert)
    return;

  const uint8_t *cert_encoded = NULL;
  size_t cert_len;
  tor_x509_cert_get_der(cert, &cert_encoded, &cert_len);

  add_certs_cell_cert_helper(certs_cell, cert_type, cert_encoded, cert_len);
}

/** Add an Ed25519 cert from <b>cert</b> to the trunnel certs_cell_t object
 * that we are building in <b>certs_cell</b>.  Set its type field to
 * <b>cert_type</b>. (If <b>cert</b> is NULL, take no action.) */
static void
add_ed25519_cert(certs_cell_t *certs_cell,
                 uint8_t cert_type,
                 const tor_cert_t *cert)
{
  if (NULL == cert)
    return;

  add_certs_cell_cert_helper(certs_cell, cert_type,
                             cert->encoded, cert->encoded_len);
}

#ifdef TOR_UNIT_TESTS
int certs_cell_ed25519_disabled_for_testing = 0;
#else
#define certs_cell_ed25519_disabled_for_testing 0
#endif

/** Send a CERTS cell on the connection <b>conn</b>.  Return 0 on success, -1
 * on failure. */
int
connection_or_send_certs_cell(or_connection_t *conn)
{
  const tor_x509_cert_t *global_link_cert = NULL, *id_cert = NULL;
  tor_x509_cert_t *own_link_cert = NULL;
  var_cell_t *cell;

  certs_cell_t *certs_cell = NULL;

  tor_assert(conn->base_.state == OR_CONN_STATE_OR_HANDSHAKING_V3);

  if (! conn->handshake_state)
    return -1;

  const int conn_in_server_mode = ! conn->handshake_state->started_here;

  /* Get the encoded values of the X509 certificates */
  if (tor_tls_get_my_certs(conn_in_server_mode,
                           &global_link_cert, &id_cert) < 0)
    return -1;

  if (conn_in_server_mode) {
    own_link_cert = tor_tls_get_own_cert(conn->tls);
  }
  tor_assert(id_cert);

  certs_cell = certs_cell_new();

  /* Start adding certs.  First the link cert or auth1024 cert. */
  if (conn_in_server_mode) {
    tor_assert_nonfatal(own_link_cert);
    add_x509_cert(certs_cell,
                  OR_CERT_TYPE_TLS_LINK, own_link_cert);
  } else {
    tor_assert(global_link_cert);
    add_x509_cert(certs_cell,
                  OR_CERT_TYPE_AUTH_1024, global_link_cert);
  }

  /* Next the RSA->RSA ID cert */
  add_x509_cert(certs_cell,
                OR_CERT_TYPE_ID_1024, id_cert);

  /* Next the Ed25519 certs */
  add_ed25519_cert(certs_cell,
                   CERTTYPE_ED_ID_SIGN,
                   get_master_signing_key_cert());
  if (conn_in_server_mode) {
    tor_assert_nonfatal(conn->handshake_state->own_link_cert ||
                        certs_cell_ed25519_disabled_for_testing);
    add_ed25519_cert(certs_cell,
                     CERTTYPE_ED_SIGN_LINK,
                     conn->handshake_state->own_link_cert);
  } else {
    add_ed25519_cert(certs_cell,
                     CERTTYPE_ED_SIGN_AUTH,
                     get_current_auth_key_cert());
  }

  /* And finally the crosscert. */
  {
    const uint8_t *crosscert=NULL;
    size_t crosscert_len;
    get_master_rsa_crosscert(&crosscert, &crosscert_len);
    if (crosscert) {
      add_certs_cell_cert_helper(certs_cell,
                               CERTTYPE_RSA1024_ID_EDID,
                               crosscert, crosscert_len);
    }
  }

  /* We've added all the certs; make the cell. */
  certs_cell->n_certs = certs_cell_getlen_certs(certs_cell);

  ssize_t alloc_len = certs_cell_encoded_len(certs_cell);
  tor_assert(alloc_len >= 0 && alloc_len <= UINT16_MAX);
  cell = var_cell_new(alloc_len);
  cell->command = CELL_CERTS;
  ssize_t enc_len = certs_cell_encode(cell->payload, alloc_len, certs_cell);
  tor_assert(enc_len > 0 && enc_len <= alloc_len);
  cell->payload_len = enc_len;

  connection_or_write_var_cell_to_buf(cell, conn);
  var_cell_free(cell);
  certs_cell_free(certs_cell);
  tor_x509_cert_free(own_link_cert);

  return 0;
}

#ifdef TOR_UNIT_TESTS
int testing__connection_or_pretend_TLSSECRET_is_supported = 0;
#else
#define testing__connection_or_pretend_TLSSECRET_is_supported 0
#endif

/** Return true iff <b>challenge_type</b> is an AUTHCHALLENGE type that
 * we can send and receive. */
int
authchallenge_type_is_supported(uint16_t challenge_type)
{
  switch (challenge_type) {
     case AUTHTYPE_RSA_SHA256_TLSSECRET:
#ifdef HAVE_WORKING_TOR_TLS_GET_TLSSECRETS
       return 1;
#else
       return testing__connection_or_pretend_TLSSECRET_is_supported;
#endif
     case AUTHTYPE_ED25519_SHA256_RFC5705:
       return 1;
     case AUTHTYPE_RSA_SHA256_RFC5705:
     default:
       return 0;
  }
}

/** Return true iff <b>challenge_type_a</b> is one that we would rather
 * use than <b>challenge_type_b</b>. */
int
authchallenge_type_is_better(uint16_t challenge_type_a,
                             uint16_t challenge_type_b)
{
  /* Any supported type is better than an unsupported one;
   * all unsupported types are equally bad. */
  if (!authchallenge_type_is_supported(challenge_type_a))
    return 0;
  if (!authchallenge_type_is_supported(challenge_type_b))
    return 1;
  /* It happens that types are superior in numerically ascending order.
   * If that ever changes, this must change too. */
  return (challenge_type_a > challenge_type_b);
}

/** Send an AUTH_CHALLENGE cell on the connection <b>conn</b>. Return 0
 * on success, -1 on failure. */
int
connection_or_send_auth_challenge_cell(or_connection_t *conn)
{
  var_cell_t *cell = NULL;
  int r = -1;
  tor_assert(conn->base_.state == OR_CONN_STATE_OR_HANDSHAKING_V3);

  if (! conn->handshake_state)
    return -1;

  auth_challenge_cell_t *ac = auth_challenge_cell_new();

  tor_assert(sizeof(ac->challenge) == 32);
  crypto_rand((char*)ac->challenge, sizeof(ac->challenge));

  if (authchallenge_type_is_supported(AUTHTYPE_RSA_SHA256_TLSSECRET))
    auth_challenge_cell_add_methods(ac, AUTHTYPE_RSA_SHA256_TLSSECRET);
  /* Disabled, because everything that supports this method also supports
   * the much-superior ED25519_SHA256_RFC5705 */
  /* auth_challenge_cell_add_methods(ac, AUTHTYPE_RSA_SHA256_RFC5705); */
  if (authchallenge_type_is_supported(AUTHTYPE_ED25519_SHA256_RFC5705))
    auth_challenge_cell_add_methods(ac, AUTHTYPE_ED25519_SHA256_RFC5705);
  auth_challenge_cell_set_n_methods(ac,
                                    auth_challenge_cell_getlen_methods(ac));

  cell = var_cell_new(auth_challenge_cell_encoded_len(ac));
  ssize_t len = auth_challenge_cell_encode(cell->payload, cell->payload_len,
                                           ac);
  if (len != cell->payload_len) {
    /* LCOV_EXCL_START */
    log_warn(LD_BUG, "Encoded auth challenge cell length not as expected");
    goto done;
    /* LCOV_EXCL_STOP */
  }
  cell->command = CELL_AUTH_CHALLENGE;

  connection_or_write_var_cell_to_buf(cell, conn);
  r = 0;

 done:
  var_cell_free(cell);
  auth_challenge_cell_free(ac);

  return r;
}

/** Compute the main body of an AUTHENTICATE cell that a client can use
 * to authenticate itself on a v3 handshake for <b>conn</b>.  Return it
 * in a var_cell_t.
 *
 * If <b>server</b> is true, only calculate the first
 * V3_AUTH_FIXED_PART_LEN bytes -- the part of the authenticator that's
 * determined by the rest of the handshake, and which match the provided value
 * exactly.
 *
 * If <b>server</b> is false and <b>signing_key</b> is NULL, calculate the
 * first V3_AUTH_BODY_LEN bytes of the authenticator (that is, everything
 * that should be signed), but don't actually sign it.
 *
 * If <b>server</b> is false and <b>signing_key</b> is provided, calculate the
 * entire authenticator, signed with <b>signing_key</b>.
 *
 * Return the length of the cell body on success, and -1 on failure.
 */
var_cell_t *
connection_or_compute_authenticate_cell_body(or_connection_t *conn,
                                             const int authtype,
                                             crypto_pk_t *signing_key,
                                      const ed25519_keypair_t *ed_signing_key,
                                      int server)
{
  auth1_t *auth = NULL;
  auth_ctx_t *ctx = auth_ctx_new();
  var_cell_t *result = NULL;
  int old_tlssecrets_algorithm = 0;
  const char *authtype_str = NULL;

  int is_ed = 0;

  /* assert state is reasonable XXXX */
  switch (authtype) {
  case AUTHTYPE_RSA_SHA256_TLSSECRET:
    authtype_str = "AUTH0001";
    old_tlssecrets_algorithm = 1;
    break;
  case AUTHTYPE_RSA_SHA256_RFC5705:
    authtype_str = "AUTH0002";
    break;
  case AUTHTYPE_ED25519_SHA256_RFC5705:
    authtype_str = "AUTH0003";
    is_ed = 1;
    break;
  default:
    tor_assert(0);
    break;
  }

  auth = auth1_new();
  ctx->is_ed = is_ed;

  /* Type: 8 bytes. */
  memcpy(auth1_getarray_type(auth), authtype_str, 8);

  {
    const tor_x509_cert_t *id_cert=NULL;
    const common_digests_t *my_digests, *their_digests;
    const uint8_t *my_id, *their_id, *client_id, *server_id;
    if (tor_tls_get_my_certs(server, NULL, &id_cert))
      goto err;
    my_digests = tor_x509_cert_get_id_digests(id_cert);
    their_digests =
      tor_x509_cert_get_id_digests(conn->handshake_state->certs->id_cert);
    tor_assert(my_digests);
    tor_assert(their_digests);
    my_id = (uint8_t*)my_digests->d[DIGEST_SHA256];
    their_id = (uint8_t*)their_digests->d[DIGEST_SHA256];

    client_id = server ? their_id : my_id;
    server_id = server ? my_id : their_id;

    /* Client ID digest: 32 octets. */
    memcpy(auth->cid, client_id, 32);

    /* Server ID digest: 32 octets. */
    memcpy(auth->sid, server_id, 32);
  }

  if (is_ed) {
    const ed25519_public_key_t *my_ed_id, *their_ed_id;
    if (!conn->handshake_state->certs->ed_id_sign) {
      log_warn(LD_OR, "Ed authenticate without Ed ID cert from peer.");
      goto err;
    }
    my_ed_id = get_master_identity_key();
    their_ed_id = &conn->handshake_state->certs->ed_id_sign->signing_key;

    const uint8_t *cid_ed = (server ? their_ed_id : my_ed_id)->pubkey;
    const uint8_t *sid_ed = (server ? my_ed_id : their_ed_id)->pubkey;

    memcpy(auth->u1_cid_ed, cid_ed, ED25519_PUBKEY_LEN);
    memcpy(auth->u1_sid_ed, sid_ed, ED25519_PUBKEY_LEN);
  }

  {
    crypto_digest_t *server_d, *client_d;
    if (server) {
      server_d = conn->handshake_state->digest_sent;
      client_d = conn->handshake_state->digest_received;
    } else {
      client_d = conn->handshake_state->digest_sent;
      server_d = conn->handshake_state->digest_received;
    }

    /* Server log digest : 32 octets */
    crypto_digest_get_digest(server_d, (char*)auth->slog, 32);

    /* Client log digest : 32 octets */
    crypto_digest_get_digest(client_d, (char*)auth->clog, 32);
  }

  {
    /* Digest of cert used on TLS link : 32 octets. */
    tor_x509_cert_t *cert = NULL;
    if (server) {
      cert = tor_tls_get_own_cert(conn->tls);
    } else {
      cert = tor_tls_get_peer_cert(conn->tls);
    }
    if (!cert) {
      log_warn(LD_OR, "Unable to find cert when making %s data.",
               authtype_str);
      goto err;
    }

    memcpy(auth->scert,
           tor_x509_cert_get_cert_digests(cert)->d[DIGEST_SHA256], 32);

    tor_x509_cert_free(cert);
  }

  /* HMAC of clientrandom and serverrandom using master key : 32 octets */
  if (old_tlssecrets_algorithm) {
    if (tor_tls_get_tlssecrets(conn->tls, auth->tlssecrets) < 0) {
      log_fn(LOG_PROTOCOL_WARN, LD_OR, "Somebody asked us for an older TLS "
         "authentication method (AUTHTYPE_RSA_SHA256_TLSSECRET) "
         "which we don't support.");
    }
  } else {
    char label[128];
    tor_snprintf(label, sizeof(label),
                 "EXPORTER FOR TOR TLS CLIENT BINDING %s", authtype_str);
    int r = tor_tls_export_key_material(conn->tls, auth->tlssecrets,
                                        auth->cid, sizeof(auth->cid),
                                        label);
    if (r < 0) {
      if (r != -2)
        log_warn(LD_BUG, "TLS key export failed for unknown reason.");
      // If r == -2, this was openssl bug 7712.
      goto err;
    }
  }

  /* 8 octets were reserved for the current time, but we're trying to get out
   * of the habit of sending time around willynilly.  Fortunately, nothing
   * checks it.  That's followed by 16 bytes of nonce. */
  crypto_rand((char*)auth->rand, 24);

  ssize_t maxlen = auth1_encoded_len(auth, ctx);
  if (ed_signing_key && is_ed) {
    maxlen += ED25519_SIG_LEN;
  } else if (signing_key && !is_ed) {
    maxlen += crypto_pk_keysize(signing_key);
  }

  const int AUTH_CELL_HEADER_LEN = 4; /* 2 bytes of type, 2 bytes of length */
  result = var_cell_new(AUTH_CELL_HEADER_LEN + maxlen);
  uint8_t *const out = result->payload + AUTH_CELL_HEADER_LEN;
  const size_t outlen = maxlen;
  ssize_t len;

  result->command = CELL_AUTHENTICATE;
  set_uint16(result->payload, htons(authtype));

  if ((len = auth1_encode(out, outlen, auth, ctx)) < 0) {
    /* LCOV_EXCL_START */
    log_warn(LD_BUG, "Unable to encode signed part of AUTH1 data.");
    goto err;
    /* LCOV_EXCL_STOP */
  }

  if (server) {
    auth1_t *tmp = NULL;
    ssize_t len2 = auth1_parse(&tmp, out, len, ctx);
    if (!tmp) {
      /* LCOV_EXCL_START */
      log_warn(LD_BUG, "Unable to parse signed part of AUTH1 data that "
               "we just encoded");
      goto err;
      /* LCOV_EXCL_STOP */
    }
    result->payload_len = (tmp->end_of_signed - result->payload);

    auth1_free(tmp);
    if (len2 != len) {
      /* LCOV_EXCL_START */
      log_warn(LD_BUG, "Mismatched length when re-parsing AUTH1 data.");
      goto err;
      /* LCOV_EXCL_STOP */
    }
    goto done;
  }

  if (ed_signing_key && is_ed) {
    ed25519_signature_t sig;
    if (ed25519_sign(&sig, out, len, ed_signing_key) < 0) {
      /* LCOV_EXCL_START */
      log_warn(LD_BUG, "Unable to sign ed25519 authentication data");
      goto err;
      /* LCOV_EXCL_STOP */
    }
    auth1_setlen_sig(auth, ED25519_SIG_LEN);
    memcpy(auth1_getarray_sig(auth), sig.sig, ED25519_SIG_LEN);

  } else if (signing_key && !is_ed) {
    auth1_setlen_sig(auth, crypto_pk_keysize(signing_key));

    char d[32];
    crypto_digest256(d, (char*)out, len, DIGEST_SHA256);
    int siglen = crypto_pk_private_sign(signing_key,
                                    (char*)auth1_getarray_sig(auth),
                                    auth1_getlen_sig(auth),
                                    d, 32);
    if (siglen < 0) {
      log_warn(LD_OR, "Unable to sign AUTH1 data.");
      goto err;
    }

    auth1_setlen_sig(auth, siglen);
  }

  len = auth1_encode(out, outlen, auth, ctx);
  if (len < 0) {
    /* LCOV_EXCL_START */
    log_warn(LD_BUG, "Unable to encode signed AUTH1 data.");
    goto err;
    /* LCOV_EXCL_STOP */
  }
  tor_assert(len + AUTH_CELL_HEADER_LEN <= result->payload_len);
  result->payload_len = len + AUTH_CELL_HEADER_LEN;
  set_uint16(result->payload+2, htons(len));

  goto done;

 err:
  var_cell_free(result);
  result = NULL;
 done:
  auth1_free(auth);
  auth_ctx_free(ctx);
  return result;
}

/** Send an AUTHENTICATE cell on the connection <b>conn</b>.  Return 0 on
 * success, -1 on failure */
MOCK_IMPL(int,
connection_or_send_authenticate_cell,(or_connection_t *conn, int authtype))
{
  var_cell_t *cell;
  crypto_pk_t *pk = tor_tls_get_my_client_auth_key();
  /* XXXX make sure we're actually supposed to send this! */

  if (!pk) {
    log_warn(LD_BUG, "Can't compute authenticate cell: no client auth key");
    return -1;
  }
  if (! authchallenge_type_is_supported(authtype)) {
    log_warn(LD_BUG, "Tried to send authenticate cell with unknown "
             "authentication type %d", authtype);
    return -1;
  }

  cell = connection_or_compute_authenticate_cell_body(conn,
                                                 authtype,
                                                 pk,
                                                 get_current_auth_keypair(),
                                                 0 /* not server */);
  if (! cell) {
    log_fn(LOG_PROTOCOL_WARN, LD_NET, "Unable to compute authenticate cell!");
    return -1;
  }
  connection_or_write_var_cell_to_buf(cell, conn);
  var_cell_free(cell);

  return 0;
}

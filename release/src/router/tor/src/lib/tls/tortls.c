/* Copyright (c) 2003, Roger Dingledine.
 * Copyright (c) 2004-2006, Roger Dingledine, Nick Mathewson.
 * Copyright (c) 2007-2021, The Tor Project, Inc. */
/* See LICENSE for licensing information */

/**
 * @file tortls.c
 * @brief Shared functionality for our TLS backends.
 **/

#define TORTLS_PRIVATE
#define TOR_X509_PRIVATE
#include "lib/tls/x509.h"
#include "lib/tls/x509_internal.h"
#include "lib/tls/tortls_sys.h"
#include "lib/tls/tortls.h"
#include "lib/tls/tortls_st.h"
#include "lib/tls/tortls_internal.h"
#include "lib/log/util_bug.h"
#include "lib/intmath/cmp.h"
#include "lib/crypt_ops/crypto_rsa.h"
#include "lib/crypt_ops/crypto_rand.h"
#include "lib/net/socket.h"
#include "lib/subsys/subsys.h"

#ifdef _WIN32
  #include <winsock2.h>
  #include <ws2tcpip.h>
#endif

#include <time.h>

/** Global TLS contexts. We keep them here because nobody else needs
 * to touch them.
 *
 * @{ */
STATIC tor_tls_context_t *server_tls_context = NULL;
STATIC tor_tls_context_t *client_tls_context = NULL;
/**@}*/

/**
 * Return the appropriate TLS context.
 */
tor_tls_context_t *
tor_tls_context_get(int is_server)
{
  return is_server ? server_tls_context : client_tls_context;
}

/** Convert an errno (or a WSAerrno on windows) into a TOR_TLS_* error
 * code. */
int
tor_errno_to_tls_error(int e)
{
  switch (e) {
    case SOCK_ERRNO(ECONNRESET): // most common
      return TOR_TLS_ERROR_CONNRESET;
    case SOCK_ERRNO(ETIMEDOUT):
      return TOR_TLS_ERROR_TIMEOUT;
    case SOCK_ERRNO(EHOSTUNREACH):
    case SOCK_ERRNO(ENETUNREACH):
      return TOR_TLS_ERROR_NO_ROUTE;
    case SOCK_ERRNO(ECONNREFUSED):
      return TOR_TLS_ERROR_CONNREFUSED; // least common
    default:
      return TOR_TLS_ERROR_MISC;
  }
}

/** Set *<b>link_cert_out</b> and *<b>id_cert_out</b> to the link certificate
 * and ID certificate that we're currently using for our V3 in-protocol
 * handshake's certificate chain.  If <b>server</b> is true, provide the certs
 * that we use in server mode (auth, ID); otherwise, provide the certs that we
 * use in client mode. (link, ID) */
int
tor_tls_get_my_certs(int server,
                     const tor_x509_cert_t **link_cert_out,
                     const tor_x509_cert_t **id_cert_out)
{
  tor_tls_context_t *ctx = tor_tls_context_get(server);
  int rv = -1;
  const tor_x509_cert_t *link_cert = NULL;
  const tor_x509_cert_t *id_cert = NULL;
  if (ctx) {
    rv = 0;
    link_cert = server ? ctx->my_link_cert : ctx->my_auth_cert;
    id_cert = ctx->my_id_cert;
  }
  if (link_cert_out)
    *link_cert_out = link_cert;
  if (id_cert_out)
    *id_cert_out = id_cert;
  return rv;
}

/**
 * Return the authentication key that we use to authenticate ourselves as a
 * client in the V3 in-protocol handshake.
 */
crypto_pk_t *
tor_tls_get_my_client_auth_key(void)
{
  tor_tls_context_t *context = tor_tls_context_get(0);
  if (! context)
    return NULL;
  return context->auth_key;
}

/** Increase the reference count of <b>ctx</b>. */
void
tor_tls_context_incref(tor_tls_context_t *ctx)
{
  ++ctx->refcnt;
}

/** Remove a reference to <b>ctx</b>, and free it if it has no more
 * references. */
void
tor_tls_context_decref(tor_tls_context_t *ctx)
{
  tor_assert(ctx);
  if (--ctx->refcnt == 0) {
    tor_tls_context_impl_free(ctx->ctx);
    tor_x509_cert_free(ctx->my_link_cert);
    tor_x509_cert_free(ctx->my_id_cert);
    tor_x509_cert_free(ctx->my_auth_cert);
    crypto_pk_free(ctx->link_key);
    crypto_pk_free(ctx->auth_key);
    /* LCOV_EXCL_BR_START since ctx will never be NULL here */
    tor_free(ctx);
    /* LCOV_EXCL_BR_STOP */
  }
}

/** Free all global TLS structures. */
void
tor_tls_free_all(void)
{
  check_no_tls_errors();

  if (server_tls_context) {
    tor_tls_context_t *ctx = server_tls_context;
    server_tls_context = NULL;
    tor_tls_context_decref(ctx);
  }
  if (client_tls_context) {
    tor_tls_context_t *ctx = client_tls_context;
    client_tls_context = NULL;
    tor_tls_context_decref(ctx);
  }
}

/** Given a TOR_TLS_* error code, return a string equivalent. */
const char *
tor_tls_err_to_string(int err)
{
  if (err >= 0)
    return "[Not an error.]";
  switch (err) {
    case TOR_TLS_ERROR_MISC: return "misc error";
    case TOR_TLS_ERROR_IO: return "unexpected close";
    case TOR_TLS_ERROR_CONNREFUSED: return "connection refused";
    case TOR_TLS_ERROR_CONNRESET: return "connection reset";
    case TOR_TLS_ERROR_NO_ROUTE: return "host unreachable";
    case TOR_TLS_ERROR_TIMEOUT: return "connection timed out";
    case TOR_TLS_CLOSE: return "closed";
    case TOR_TLS_WANTREAD: return "want to read";
    case TOR_TLS_WANTWRITE: return "want to write";
    default: return "(unknown error code)";
  }
}

/** Create new global client and server TLS contexts.
 *
 * If <b>server_identity</b> is NULL, this will not generate a server
 * TLS context. If TOR_TLS_CTX_IS_PUBLIC_SERVER is set in <b>flags</b>, use
 * the same TLS context for incoming and outgoing connections, and
 * ignore <b>client_identity</b>. If one of TOR_TLS_CTX_USE_ECDHE_P{224,256}
 * is set in <b>flags</b>, use that ECDHE group if possible; otherwise use
 * the default ECDHE group. */
int
tor_tls_context_init(unsigned flags,
                     crypto_pk_t *client_identity,
                     crypto_pk_t *server_identity,
                     unsigned int key_lifetime)
{
  int rv1 = 0;
  int rv2 = 0;
  const int is_public_server = flags & TOR_TLS_CTX_IS_PUBLIC_SERVER;
  check_no_tls_errors();

  if (is_public_server) {
    tor_tls_context_t *new_ctx;
    tor_tls_context_t *old_ctx;

    tor_assert(server_identity != NULL);

    rv1 = tor_tls_context_init_one(&server_tls_context,
                                   server_identity,
                                   key_lifetime, flags, 0);

    if (rv1 >= 0) {
      new_ctx = server_tls_context;
      tor_tls_context_incref(new_ctx);
      old_ctx = client_tls_context;
      client_tls_context = new_ctx;

      if (old_ctx != NULL) {
        tor_tls_context_decref(old_ctx);
      }
    } else {
      tls_log_errors(NULL, LOG_WARN, LD_CRYPTO,
                     "constructing a TLS context");
    }
  } else {
    if (server_identity != NULL) {
      rv1 = tor_tls_context_init_one(&server_tls_context,
                                     server_identity,
                                     key_lifetime,
                                     flags,
                                     0);
      if (rv1 < 0)
        tls_log_errors(NULL, LOG_WARN, LD_CRYPTO,
                       "constructing a server TLS context");
    } else {
      tor_tls_context_t *old_ctx = server_tls_context;
      server_tls_context = NULL;

      if (old_ctx != NULL) {
        tor_tls_context_decref(old_ctx);
      }
    }

    rv2 = tor_tls_context_init_one(&client_tls_context,
                                   client_identity,
                                   key_lifetime,
                                   flags,
                                   1);
    if (rv2 < 0)
        tls_log_errors(NULL, LOG_WARN, LD_CRYPTO,
                       "constructing a client TLS context");
  }

  return MIN(rv1, rv2);
}

/** Create a new global TLS context.
 *
 * You can call this function multiple times.  Each time you call it,
 * it generates new certificates; all new connections will use
 * the new SSL context.
 */
int
tor_tls_context_init_one(tor_tls_context_t **ppcontext,
                         crypto_pk_t *identity,
                         unsigned int key_lifetime,
                         unsigned int flags,
                         int is_client)
{
  tor_tls_context_t *new_ctx = tor_tls_context_new(identity,
                                                   key_lifetime,
                                                   flags,
                                                   is_client);
  tor_tls_context_t *old_ctx = *ppcontext;

  if (new_ctx != NULL) {
    *ppcontext = new_ctx;

    /* Free the old context if one existed. */
    if (old_ctx != NULL) {
      /* This is safe even if there are open connections: we reference-
       * count tor_tls_context_t objects. */
      tor_tls_context_decref(old_ctx);
    }
  }

  return ((new_ctx != NULL) ? 0 : -1);
}

/** Size of the RSA key to use for our TLS link keys */
#define RSA_LINK_KEY_BITS 2048

/** How long do identity certificates live? (sec) */
#define IDENTITY_CERT_LIFETIME  (365*24*60*60)

/**
 * Initialize the certificates and keys for a TLS context <b>result</b>
 *
 * Other arguments as for tor_tls_context_new().
 */
int
tor_tls_context_init_certificates(tor_tls_context_t *result,
                                  crypto_pk_t *identity,
                                  unsigned key_lifetime,
                                  unsigned flags)
{
  (void)flags;
  int rv = -1;
  char *nickname = NULL, *nn2 = NULL;
  crypto_pk_t *rsa = NULL, *rsa_auth = NULL;
  tor_x509_cert_impl_t *cert = NULL, *idcert = NULL, *authcert = NULL;

  nickname = crypto_random_hostname(8, 20, "www.", ".net");

#ifdef DISABLE_V3_LINKPROTO_SERVERSIDE
  nn2 = crypto_random_hostname(8, 20, "www.", ".net");
#else
  nn2 = crypto_random_hostname(8, 20, "www.", ".com");
#endif

  /* Generate short-term RSA key for use with TLS. */
  if (!(rsa = crypto_pk_new()))
    goto error;
  if (crypto_pk_generate_key_with_bits(rsa, RSA_LINK_KEY_BITS)<0)
    goto error;

  /* Generate short-term RSA key for use in the in-protocol ("v3")
   * authentication handshake. */
  if (!(rsa_auth = crypto_pk_new()))
    goto error;
  if (crypto_pk_generate_key(rsa_auth)<0)
    goto error;

  /* Create a link certificate signed by identity key. */
  cert = tor_tls_create_certificate(rsa, identity, nickname, nn2,
                                    key_lifetime);
  /* Create self-signed certificate for identity key. */
  idcert = tor_tls_create_certificate(identity, identity, nn2, nn2,
                                      IDENTITY_CERT_LIFETIME);
  /* Create an authentication certificate signed by identity key. */
  authcert = tor_tls_create_certificate(rsa_auth, identity, nickname, nn2,
                                        key_lifetime);
  if (!cert || !idcert || !authcert) {
    log_warn(LD_CRYPTO, "Error creating certificate");
    goto error;
  }

  result->my_link_cert = tor_x509_cert_new(cert);
  cert = NULL;
  result->my_id_cert = tor_x509_cert_new(idcert);
  idcert = NULL;
  result->my_auth_cert = tor_x509_cert_new(authcert);
  authcert = NULL;
  if (!result->my_link_cert || !result->my_id_cert || !result->my_auth_cert)
    goto error;
  result->link_key = rsa;
  rsa = NULL;
  result->auth_key = rsa_auth;
  rsa_auth = NULL;

  rv = 0;
 error:

  tor_free(nickname);
  tor_free(nn2);

  tor_x509_cert_impl_free(cert);
  tor_x509_cert_impl_free(idcert);
  tor_x509_cert_impl_free(authcert);
  crypto_pk_free(rsa);
  crypto_pk_free(rsa_auth);

  return rv;
}
/** Make future log messages about <b>tls</b> display the address
 * <b>address</b>.
 */
void
tor_tls_set_logged_address(tor_tls_t *tls, const char *address)
{
  tor_assert(tls);
  tor_free(tls->address);
  tls->address = tor_strdup(address);
}

/** Return whether this tls initiated the connect (client) or
 * received it (server). */
int
tor_tls_is_server(tor_tls_t *tls)
{
  tor_assert(tls);
  return tls->isServer;
}

/** Release resources associated with a TLS object.  Does not close the
 * underlying file descriptor.
 */
void
tor_tls_free_(tor_tls_t *tls)
{
  if (!tls)
    return;
  tor_assert(tls->ssl);
  {
    size_t r,w;
    tor_tls_get_n_raw_bytes(tls,&r,&w); /* ensure written_by_tls is updated */
  }
  tor_tls_impl_free(tls->ssl);
  tls->ssl = NULL;
#ifdef ENABLE_OPENSSL
  tls->negotiated_callback = NULL;
#endif
  if (tls->context)
    tor_tls_context_decref(tls->context);
  tor_free(tls->address);
  tls->magic = 0x99999999;
  tor_free(tls);
}

/** If the provided tls connection is authenticated and has a
 * certificate chain that is currently valid and signed, then set
 * *<b>identity_key</b> to the identity certificate's key and return
 * 0.  Else, return -1 and log complaints with log-level <b>severity</b>.
 */
int
tor_tls_verify(int severity, tor_tls_t *tls, crypto_pk_t **identity)
{
  tor_x509_cert_impl_t *cert = NULL, *id_cert = NULL;
  tor_x509_cert_t *peer_x509 = NULL, *id_x509 = NULL;
  tor_assert(tls);
  tor_assert(identity);
  int rv = -1;

  try_to_extract_certs_from_tls(severity, tls, &cert, &id_cert);
  if (!cert)
    goto done;
  if (!id_cert) {
    log_fn(severity,LD_PROTOCOL,"No distinct identity certificate found");
    goto done;
  }
  peer_x509 = tor_x509_cert_new(cert);
  id_x509 = tor_x509_cert_new(id_cert);
  cert = id_cert = NULL; /* Prevent double-free */

  if (! tor_tls_cert_is_valid(severity, peer_x509, id_x509, time(NULL), 0)) {
    goto done;
  }

  *identity = tor_tls_cert_get_key(id_x509);
  rv = 0;

 done:
  tor_x509_cert_impl_free(cert);
  tor_x509_cert_impl_free(id_cert);
  tor_x509_cert_free(peer_x509);
  tor_x509_cert_free(id_x509);

  return rv;
}

static void
subsys_tortls_shutdown(void)
{
  tor_tls_free_all();
}

const subsys_fns_t sys_tortls = {
  .name = "tortls",
  SUBSYS_DECLARE_LOCATION(),
  .level = -50,
  .shutdown = subsys_tortls_shutdown
};

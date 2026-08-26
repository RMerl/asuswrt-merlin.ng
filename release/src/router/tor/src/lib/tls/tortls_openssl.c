/* Copyright (c) 2003, Roger Dingledine.
 * Copyright (c) 2004-2006, Roger Dingledine, Nick Mathewson.
 * Copyright (c) 2007-2021, The Tor Project, Inc. */
/* See LICENSE for licensing information */

/**
 * \file tortls.c
 * \brief Wrapper functions to present a consistent interface to
 * TLS, SSL, and X.509 functions from OpenSSL.
 **/

/* (Unlike other tor functions, these
 * are prefixed with tor_ in order to avoid conflicting with OpenSSL
 * functions and variables.)
 */

#include "orconfig.h"

#define TORTLS_PRIVATE
#define TORTLS_OPENSSL_PRIVATE
#define TOR_X509_PRIVATE

#ifdef _WIN32
  /* We need to include these here, or else the dtls1.h header will include
   * <winsock.h> and mess things up, in at least some openssl versions. */
  #include <winsock2.h>
  #include <ws2tcpip.h>
#endif /* defined(_WIN32) */

#include "lib/crypt_ops/crypto_cipher.h"
#include "lib/crypt_ops/crypto_rand.h"
#include "lib/crypt_ops/crypto_dh.h"
#include "lib/crypt_ops/crypto_util.h"
#include "lib/crypt_ops/compat_openssl.h"
#include "lib/tls/x509.h"
#include "lib/tls/x509_internal.h"

/* Some versions of OpenSSL declare SSL_get_selected_srtp_profile twice in
 * srtp.h. Suppress the GCC warning so we can build with -Wredundant-decl. */
DISABLE_GCC_WARNING("-Wredundant-decls")

#include <openssl/opensslv.h>

#ifdef OPENSSL_NO_EC
#error "We require OpenSSL with ECC support"
#endif

#include <openssl/ssl.h>
#include <openssl/ssl3.h>
#include <openssl/err.h>
#include <openssl/tls1.h>
#include <openssl/asn1.h>
#include <openssl/bio.h>
#include <openssl/bn.h>
#include <openssl/rsa.h>

ENABLE_GCC_WARNING("-Wredundant-decls")

#include "lib/tls/tortls.h"
#include "lib/tls/tortls_st.h"
#include "lib/tls/tortls_internal.h"
#include "lib/log/log.h"
#include "lib/log/util_bug.h"
#include "lib/container/smartlist.h"
#include "lib/string/compat_string.h"
#include "lib/string/printf.h"
#include "lib/net/socket.h"
#include "lib/intmath/cmp.h"
#include "lib/ctime/di_ops.h"
#include "lib/encoding/time_fmt.h"

#include <stdlib.h>
#include <string.h>

#include "lib/arch/bytes.h"

/* Copied from or.h */
#define LEGAL_NICKNAME_CHARACTERS \
  "abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789"

#define ADDR(tls) (((tls) && (tls)->address) ? tls->address : "peer")

/** Set to true iff openssl bug 7712 has been detected. */
static int openssl_bug_7712_is_present = 0;

/** The ex_data index in which we store a pointer to an SSL object's
 * corresponding tor_tls_t object. */
STATIC int tor_tls_object_ex_data_index = -1;

/** Helper: Allocate tor_tls_object_ex_data_index. */
void
tor_tls_allocate_tor_tls_object_ex_data_index(void)
{
  if (tor_tls_object_ex_data_index == -1) {
    tor_tls_object_ex_data_index =
      SSL_get_ex_new_index(0, NULL, NULL, NULL, NULL);
    tor_assert(tor_tls_object_ex_data_index != -1);
  }
}

/** Helper: given a SSL* pointer, return the tor_tls_t object using that
 * pointer. */
tor_tls_t *
tor_tls_get_by_ssl(const SSL *ssl)
{
  tor_tls_t *result = SSL_get_ex_data(ssl, tor_tls_object_ex_data_index);
  if (result)
    tor_assert(result->magic == TOR_TLS_MAGIC);
  return result;
}

/** True iff tor_tls_init() has been called. */
static int tls_library_is_initialized = 0;

/* Module-internal error codes. */
#define TOR_TLS_SYSCALL_    (MIN_TOR_TLS_ERROR_VAL_ - 2)
#define TOR_TLS_ZERORETURN_ (MIN_TOR_TLS_ERROR_VAL_ - 1)

/** Write a description of the current state of <b>tls</b> into the
 * <b>sz</b>-byte buffer at <b>buf</b>. */
void
tor_tls_get_state_description(tor_tls_t *tls, char *buf, size_t sz)
{
  const char *ssl_state;
  const char *tortls_state;

  if (PREDICT_UNLIKELY(!tls || !tls->ssl)) {
    strlcpy(buf, "(No SSL object)", sz);
    return;
  }

  ssl_state = SSL_state_string_long(tls->ssl);
  switch (tls->state) {
#define CASE(st) case TOR_TLS_ST_##st: tortls_state = " in "#st ; break
    CASE(HANDSHAKE);
    CASE(OPEN);
    CASE(GOTCLOSE);
    CASE(SENTCLOSE);
    CASE(CLOSED);
    CASE(RENEGOTIATE);
#undef CASE
  case TOR_TLS_ST_BUFFEREVENT:
    tortls_state = "";
    break;
  default:
    tortls_state = " in unknown TLS state";
    break;
  }

  tor_snprintf(buf, sz, "%s%s", ssl_state, tortls_state);
}

/** Log a single error <b>err</b> as returned by ERR_get_error(), which was
 * received while performing an operation <b>doing</b> on <b>tls</b>.  Log
 * the message at <b>severity</b>, in log domain <b>domain</b>. */
void
tor_tls_log_one_error(tor_tls_t *tls, unsigned long err,
                  int severity, int domain, const char *doing)
{
  const char *state = NULL, *addr;
  const char *msg, *lib, *func;

  state = (tls && tls->ssl)?SSL_state_string_long(tls->ssl):"---";

  addr = tls ? tls->address : NULL;

  /* Some errors are known-benign, meaning they are the fault of the other
   * side of the connection. The caller doesn't know this, so override the
   * priority for those cases. */
  switch (ERR_GET_REASON(err)) {
    case SSL_R_HTTP_REQUEST:
    case SSL_R_HTTPS_PROXY_REQUEST:
    case SSL_R_RECORD_LENGTH_MISMATCH:
    case SSL_R_UNKNOWN_PROTOCOL:
    case SSL_R_UNSUPPORTED_PROTOCOL:
      severity = LOG_INFO;
      break;
    default:
      break;
  }

  msg = (const char*)ERR_reason_error_string(err);
  lib = (const char*)ERR_lib_error_string(err);
  func = (const char*)ERR_func_error_string(err);
  if (!msg) msg = "(null)";
  if (!lib) lib = "(null)";
  if (!func) func = "(null)";
  if (doing) {
    tor_log(severity, domain, "TLS error while %s%s%s: %s (in %s:%s:%s)",
        doing, addr?" with ":"", addr?addr:"",
        msg, lib, func, state);
  } else {
    tor_log(severity, domain, "TLS error%s%s: %s (in %s:%s:%s)",
        addr?" with ":"", addr?addr:"",
        msg, lib, func, state);
  }
}

/** Log all pending tls errors at level <b>severity</b> in log domain
 * <b>domain</b>.  Use <b>doing</b> to describe our current activities.
 */
void
tls_log_errors(tor_tls_t *tls, int severity, int domain, const char *doing)
{
  unsigned long err;

  while ((err = ERR_get_error()) != 0) {
    if (tls)
      tls->last_error = err;
    tor_tls_log_one_error(tls, err, severity, domain, doing);
  }
}

/**
 * Return a string representing more detail about the last error received
 * on TLS.
 *
 * May return null if no error was found.
 **/
const char *
tor_tls_get_last_error_msg(const tor_tls_t *tls)
{
  IF_BUG_ONCE(!tls) {
    return NULL;
  }
  if (tls->last_error == 0) {
    return NULL;
  }
  return (const char*)ERR_reason_error_string(tls->last_error);
}

#define CATCH_SYSCALL 1
#define CATCH_ZERO    2

/** Given a TLS object and the result of an SSL_* call, use
 * SSL_get_error to determine whether an error has occurred, and if so
 * which one.  Return one of TOR_TLS_{DONE|WANTREAD|WANTWRITE|ERROR}.
 * If extra&CATCH_SYSCALL is true, return TOR_TLS_SYSCALL_ instead of
 * reporting syscall errors.  If extra&CATCH_ZERO is true, return
 * TOR_TLS_ZERORETURN_ instead of reporting zero-return errors.
 *
 * If an error has occurred, log it at level <b>severity</b> and describe the
 * current action as <b>doing</b>.
 */
int
tor_tls_get_error(tor_tls_t *tls, int r, int extra,
                  const char *doing, int severity, int domain)
{
  int err = SSL_get_error(tls->ssl, r);
  int tor_error = TOR_TLS_ERROR_MISC;
  switch (err) {
    case SSL_ERROR_NONE:
      return TOR_TLS_DONE;
    case SSL_ERROR_WANT_READ:
      return TOR_TLS_WANTREAD;
    case SSL_ERROR_WANT_WRITE:
      return TOR_TLS_WANTWRITE;
    case SSL_ERROR_SYSCALL:
      if (extra&CATCH_SYSCALL)
        return TOR_TLS_SYSCALL_;
      if (r == 0) {
        tor_log(severity, LD_NET, "TLS error: unexpected close while %s (%s)",
            doing, SSL_state_string_long(tls->ssl));
        tor_error = TOR_TLS_ERROR_IO;
      } else {
        int e = tor_socket_errno(tls->socket);
        tor_log(severity, LD_NET,
            "TLS error: <syscall error while %s> (errno=%d: %s; state=%s)",
            doing, e, tor_socket_strerror(e),
            SSL_state_string_long(tls->ssl));
        tor_error = tor_errno_to_tls_error(e);
      }
      tls_log_errors(tls, severity, domain, doing);
      return tor_error;
    case SSL_ERROR_ZERO_RETURN:
      if (extra&CATCH_ZERO)
        return TOR_TLS_ZERORETURN_;
      tor_log(severity, LD_NET, "TLS connection closed while %s in state %s",
          doing, SSL_state_string_long(tls->ssl));
      tls_log_errors(tls, severity, domain, doing);
      return TOR_TLS_CLOSE;
    default:
      tls_log_errors(tls, severity, domain, doing);
      return TOR_TLS_ERROR_MISC;
  }
}

/** Initialize OpenSSL, unless it has already been initialized.
 */
void
tor_tls_init(void)
{
  check_no_tls_errors();

  if (!tls_library_is_initialized) {
    OPENSSL_init_ssl(OPENSSL_INIT_LOAD_SSL_STRINGS, NULL);

    tor_tls_allocate_tor_tls_object_ex_data_index();

    tls_library_is_initialized = 1;
  }
}

/** We need to give OpenSSL a callback to verify certificates. This is
 * it: We always accept peer certs and complete the handshake.  We
 * don't validate them until later.
 */
int
always_accept_verify_cb(int preverify_ok,
                        X509_STORE_CTX *x509_ctx)
{
  (void) preverify_ok;
  (void) x509_ctx;
  return 1;
}

/** List of ciphers that servers should select from when using TLS 1.2 */
static const char UNRESTRICTED_TLS1_2_SERVER_CIPHER_LIST[] =
  /* This list is autogenerated with the gen_server_ciphers.py script;
   * don't hand-edit it. */
#ifdef TLS1_TXT_ECDHE_RSA_WITH_AES_256_GCM_SHA384
       TLS1_TXT_ECDHE_RSA_WITH_AES_256_GCM_SHA384 ":"
#endif
#ifdef TLS1_TXT_ECDHE_RSA_WITH_AES_128_GCM_SHA256
       TLS1_TXT_ECDHE_RSA_WITH_AES_128_GCM_SHA256 ":"
#endif
#ifdef TLS1_TXT_ECDHE_RSA_WITH_AES_256_SHA384
       TLS1_TXT_ECDHE_RSA_WITH_AES_256_SHA384 ":"
#endif
#ifdef TLS1_TXT_ECDHE_RSA_WITH_AES_128_SHA256
       TLS1_TXT_ECDHE_RSA_WITH_AES_128_SHA256 ":"
#endif
#ifdef TLS1_TXT_ECDHE_RSA_WITH_AES_256_CBC_SHA
       TLS1_TXT_ECDHE_RSA_WITH_AES_256_CBC_SHA ":"
#endif
#ifdef TLS1_TXT_ECDHE_RSA_WITH_AES_128_CBC_SHA
       TLS1_TXT_ECDHE_RSA_WITH_AES_128_CBC_SHA ":"
#endif
#ifdef TLS1_TXT_DHE_RSA_WITH_AES_256_GCM_SHA384
       TLS1_TXT_DHE_RSA_WITH_AES_256_GCM_SHA384 ":"
#endif
#ifdef TLS1_TXT_DHE_RSA_WITH_AES_128_GCM_SHA256
       TLS1_TXT_DHE_RSA_WITH_AES_128_GCM_SHA256 ":"
#endif
#ifdef TLS1_TXT_DHE_RSA_WITH_AES_256_CCM
       TLS1_TXT_DHE_RSA_WITH_AES_256_CCM ":"
#endif
#ifdef TLS1_TXT_DHE_RSA_WITH_AES_128_CCM
       TLS1_TXT_DHE_RSA_WITH_AES_128_CCM ":"
#endif
#ifdef TLS1_TXT_DHE_RSA_WITH_AES_256_SHA256
       TLS1_TXT_DHE_RSA_WITH_AES_256_SHA256 ":"
#endif
#ifdef TLS1_TXT_DHE_RSA_WITH_AES_128_SHA256
       TLS1_TXT_DHE_RSA_WITH_AES_128_SHA256 ":"
#endif
       /* Required */
       TLS1_TXT_DHE_RSA_WITH_AES_256_SHA ":"
       /* Required */
       TLS1_TXT_DHE_RSA_WITH_AES_128_SHA ":"
#ifdef TLS1_TXT_ECDHE_RSA_WITH_CHACHA20_POLY1305
       TLS1_TXT_ECDHE_RSA_WITH_CHACHA20_POLY1305 ":"
#endif
#ifdef TLS1_TXT_DHE_RSA_WITH_CHACHA20_POLY1305
       TLS1_TXT_DHE_RSA_WITH_CHACHA20_POLY1305
#endif
  ;

/* Note: to set up your own private testing network with link crypto
 * disabled, set your Tors' cipher list to
 * (SSL3_TXT_RSA_NULL_SHA).  If you do this, you won't be able to communicate
 * with any of the "real" Tors, though. */

#define CIPHER(id, name) name ":"
#define XCIPHER(id, name)
/** List of ciphers that clients should advertise, omitting items that
 * our OpenSSL doesn't know about. */
static const char CLIENT_CIPHER_LIST[] =
#ifndef COCCI
#include "lib/tls/ciphers.inc"
#endif
  /* Tell it not to use SSLv2 ciphers, so that it can select an SSLv3 version
   * of any cipher we say. */
  "!SSLv2"
  ;
static char CLIENT_CIPHER_LIST_TLSv13[] =
#ifndef COCCI
#include "lib/tls/ciphers_v13.inc"
#endif
  ""
  ;
#undef CIPHER
#undef XCIPHER

/** Return true iff the other side of <b>tls</b> has authenticated to us, and
 * the key certified in <b>cert</b> is the same as the key they used to do it.
 */
MOCK_IMPL(int,
tor_tls_cert_matches_key,(const tor_tls_t *tls, const tor_x509_cert_t *cert))
{
  tor_x509_cert_t *peer = tor_tls_get_peer_cert((tor_tls_t *)tls);
  if (!peer)
    return 0;

  X509 *peercert = peer->cert;
  EVP_PKEY *link_key = NULL, *cert_key = NULL;
  int result;

  link_key = X509_get_pubkey(peercert);
  cert_key = X509_get_pubkey(cert->cert);

  result = link_key && cert_key && EVP_PKEY_cmp(cert_key, link_key) == 1;

  tor_x509_cert_free(peer);
  if (link_key)
    EVP_PKEY_free(link_key);
  if (cert_key)
    EVP_PKEY_free(cert_key);

  return result;
}

void
tor_tls_context_impl_free_(struct ssl_ctx_st *ctx)
{
  if (!ctx)
    return;
  SSL_CTX_free(ctx);
}

/** The group we should use for ecdhe when none was selected. */
#define  NID_tor_default_ecdhe_group NID_X9_62_prime256v1

/** Create a new TLS context for use with Tor TLS handshakes.
 * <b>identity</b> should be set to the identity key used to sign the
 * certificate.
 */
tor_tls_context_t *
tor_tls_context_new(crypto_pk_t *identity, unsigned int key_lifetime,
                    unsigned flags, int is_client)
{
  EVP_PKEY *pkey = NULL;
  tor_tls_context_t *result = NULL;

  tor_tls_init();

  result = tor_malloc_zero(sizeof(tor_tls_context_t));
  result->refcnt = 1;

  if (! is_client) {
    if (tor_tls_context_init_certificates(result, identity, key_lifetime,
                                          flags) < 0) {
      goto error;
    }
  }

  /* Tell OpenSSL to use TLS 1.2 or later. */
  if (!(result->ctx = SSL_CTX_new(TLS_method())))
    goto error;
  if (!SSL_CTX_set_min_proto_version(result->ctx, TLS1_2_VERSION))
    goto error;

#ifdef HAVE_SSL_CTX_SET_SECURITY_LEVEL
  /* Level 1 re-enables RSA1024 and DH1024 for compatibility with old tors */
  SSL_CTX_set_security_level(result->ctx, 1);
#endif

  /* Prefer the server's ordering of ciphers: the client's ordering has
  * historically been chosen for fingerprinting resistance. */
  SSL_CTX_set_options(result->ctx, SSL_OP_CIPHER_SERVER_PREFERENCE);

  /* Disable TLS tickets if they're supported.  We never want to use them;
   * using them can make our perfect forward secrecy a little worse, *and*
   * create an opportunity to fingerprint us (since it's unusual to use them
   * with TLS sessions turned off).
   *
   * In 0.2.4, clients advertise support for them though, to avoid a TLS
   * distinguishability vector.  This can give us worse PFS, though, if we
   * get a server that doesn't set SSL_OP_NO_TICKET.  With luck, there will
   * be few such servers by the time 0.2.4 is more stable.
   */
#ifdef SSL_OP_NO_TICKET
  if (! is_client) {
    SSL_CTX_set_options(result->ctx, SSL_OP_NO_TICKET);
  }
#endif

  SSL_CTX_set_options(result->ctx, SSL_OP_SINGLE_DH_USE);
  SSL_CTX_set_options(result->ctx, SSL_OP_SINGLE_ECDH_USE);

#ifdef SSL_OP_NO_SESSION_RESUMPTION_ON_RENEGOTIATION
  SSL_CTX_set_options(result->ctx,
                      SSL_OP_NO_SESSION_RESUMPTION_ON_RENEGOTIATION);
#endif
#ifdef SSL_OP_NO_RENEGOTIATION
  SSL_CTX_set_options(result->ctx, SSL_OP_NO_RENEGOTIATION);
#endif
#ifdef SSL_OP_NO_CLIENT_RENEGOTIATION
  SSL_CTX_set_options(result->ctx, SSL_OP_NO_CLIENT_RENEGOTIATION);
#endif

  /* Don't actually allow compression; it uses RAM and time, it makes TLS
   * vulnerable to CRIME-style attacks, and most of the data we transmit over
   * TLS is encrypted (and therefore uncompressible) anyway. */
#ifdef SSL_OP_NO_COMPRESSION
  SSL_CTX_set_options(result->ctx, SSL_OP_NO_COMPRESSION);
#endif

#ifdef SSL_MODE_RELEASE_BUFFERS
  SSL_CTX_set_mode(result->ctx, SSL_MODE_RELEASE_BUFFERS);
#endif
  if (! is_client) {
    if (result->my_link_cert &&
        !SSL_CTX_use_certificate(result->ctx,
                                 result->my_link_cert->cert)) {
      goto error;
    }
    // Here we would once add my_id_cert too via X509_STORE_add_cert.
    //
    // We no longer do that, since we no longer send multiple certs;
    // that was part of the obsolete v1 handshake.
  }
  SSL_CTX_set_session_cache_mode(result->ctx, SSL_SESS_CACHE_OFF);
  if (!is_client) {
    tor_assert(result->link_key);
    if (!(pkey = crypto_pk_get_openssl_evp_pkey_(result->link_key,1)))
      goto error;
    if (!SSL_CTX_use_PrivateKey(result->ctx, pkey))
      goto error;
    EVP_PKEY_free(pkey);
    pkey = NULL;
    if (!SSL_CTX_check_private_key(result->ctx))
      goto error;
  }

  {
    DH *dh = crypto_dh_new_openssl_tls();
    tor_assert(dh);
    SSL_CTX_set_tmp_dh(result->ctx, dh);
    DH_free(dh);
  }

  {
    // We'd like to say something like:
    //    "?X25519MLKEM768:P-256:P-224"
    // to mean that we prefer X25519MLKEM768 if it is present;
    // but we do insist on the presence of  P-256 and P-224.
    //
    // Unfortunately, we support back to OpenSSL 3.0, which did not provide
    // any syntax for saying "don't worry if this group isn't supported."
    // Instead, we have to make this preference list of preference lists.
    static const struct {
      // Minimal version with which to try this syntax.
      // We have to restrict, since older versions of openssl
      // can misunderstand-but nonetheless accept!-syntaxes
      // supported by newer versions.  See #41058 for one example.
      long min_version;
      const char *groups;
    } group_lists[] = {
      // We do use the ? syntax here, since every version of OpenSSL
      // that supports ML-KEM also supports the ? syntax.
      // We also use the * and / syntaxes:
      //   '*' indicates that the client should send these keyshares.
      //   "/" separates tuples of groups that are "comparably secure".
      //
      // Note that we tell the client to send a P-256 keyshare, since until
      // this commit, our servers didn't accept X25519.
      //
      // Also note that until the upstream LibreSSL bug from tor#41134 gets
      // fixed, the order of groups common between each preference list must
      // be the same. We can't prefer P-256 in one, and X25519 in another.
      {
        OPENSSL_V_SERIES(3,5,0),
        "?*X25519MLKEM768 / ?SecP256r1MLKEM768 / *P-256:?X25519:P-224"
      },
      { 0, "P-256:X25519:P-224" },
      { 0, "P-256:P-224" },
    };
    bool success = false;
    long our_version = tor_OpenSSL_version_num();
    for (unsigned j = 0; j < ARRAY_LENGTH(group_lists); ++j) {
      const char *list = group_lists[j].groups;
      if (group_lists[j].min_version > our_version) {
        log_info(LD_NET, "Not trying groups %s because of OpenSSL version.",
                 list);
        continue;
      }
      int r = (int) SSL_CTX_set1_groups_list(result->ctx, list);
      if (r == 1) {
        static bool have_logged_already = false;
        if (!have_logged_already) {
          /* say it only once at startup, since the answer won't change */
          log_notice(LD_NET, "Set list of supported TLS groups to: %s", list);
          have_logged_already = true;
        }
        success = true;
        break;
      }
      log_info(LD_NET, "Group list %s wasn't accepted", list);
    }
    if (! success) {
      log_warn(LD_NET, "No lists of TLS groups were supported. "
               "Using library defaults");
    }
  }

  if (is_client) {
    SSL_CTX_set_verify(result->ctx, SSL_VERIFY_PEER,
                       always_accept_verify_cb);
  } else {
    /* Don't send a certificate request at all if we're not a client. */
    SSL_CTX_set_verify(result->ctx, SSL_VERIFY_NONE, NULL);
  }
  /* let us realloc bufs that we're writing from */
  SSL_CTX_set_mode(result->ctx, SSL_MODE_ACCEPT_MOVING_WRITE_BUFFER);

#ifdef SSL_OP_TLSEXT_PADDING
  /* Adds a padding extension to ensure the ClientHello size is never between
   * 256 and 511 bytes in length. */
  SSL_CTX_set_options(result->ctx, SSL_OP_TLSEXT_PADDING);
#endif

  return result;

 error:
  tls_log_errors(NULL, LOG_WARN, LD_NET, "creating TLS context");
  if (pkey)
    EVP_PKEY_free(pkey);
  tor_tls_context_decref(result);
  return NULL;
}

/** Invoked when a TLS state changes: log the change at severity 'debug' */
void
tor_tls_debug_state_callback(const SSL *ssl, int type, int val)
{
  /* LCOV_EXCL_START since this depends on whether debug is captured or not */
  log_debug(LD_HANDSHAKE, "SSL %p is now in state %s [type=%d,val=%d].",
            ssl, SSL_state_string_long(ssl), type, val);
  /* LCOV_EXCL_STOP */
}

/** Create a new TLS object from a file descriptor, and a flag to
 * determine whether it is functioning as a server.
 */
tor_tls_t *
tor_tls_new(tor_socket_t sock, int isServer)
{
  BIO *bio = NULL;
  tor_tls_t *result = tor_malloc_zero(sizeof(tor_tls_t));
  tor_tls_context_t *context = tor_tls_context_get(isServer);
  result->magic = TOR_TLS_MAGIC;

  check_no_tls_errors();
  tor_assert(context); /* make sure somebody made it first */
  if (!(result->ssl = SSL_new(context->ctx))) {
    tls_log_errors(NULL, LOG_WARN, LD_NET, "creating SSL object");
    tor_free(result);
    goto err;
  }

#ifdef SSL_set_tlsext_host_name
  /* Browsers use the TLS hostname extension, so we should too. */
  if (!isServer) {
    char *fake_hostname = crypto_random_hostname(4,25, "www.",".com");
    SSL_set_tlsext_host_name(result->ssl, fake_hostname);
    tor_free(fake_hostname);
  }
#endif /* defined(SSL_set_tlsext_host_name) */

#ifdef SSL_CTRL_SET_MAX_PROTO_VERSION
  if (openssl_bug_7712_is_present) {
    /* We can't actually use TLS 1.3 until this bug is fixed. */
    SSL_set_max_proto_version(result->ssl, TLS1_2_VERSION);
  }
#endif /* defined(SSL_CTRL_SET_MAX_PROTO_VERSION) */

  /* Contrary to SSL_set_cipher_list(), TLSv1.3 SSL_set_ciphersuites() does NOT
   * accept the final ':' so we have to strip it out. */
  size_t TLSv13len = strlen(CLIENT_CIPHER_LIST_TLSv13);
  if (TLSv13len && CLIENT_CIPHER_LIST_TLSv13[TLSv13len - 1] == ':') {
    CLIENT_CIPHER_LIST_TLSv13[TLSv13len - 1] = '\0';
  }

  const bool tls12_ciphers_ok = SSL_set_cipher_list(
      result->ssl,
      isServer ? UNRESTRICTED_TLS1_2_SERVER_CIPHER_LIST : CLIENT_CIPHER_LIST);

  bool tls13_ciphers_ok = true;
#ifdef HAVE_SSL_SET_CIPHERSUITES
  if (!isServer) {
    tls13_ciphers_ok =
      SSL_set_ciphersuites(result->ssl, CLIENT_CIPHER_LIST_TLSv13);
  }
#endif

  if (!tls12_ciphers_ok || !tls13_ciphers_ok) {
    tls_log_errors(NULL, LOG_WARN, LD_NET, "setting ciphers");
#ifdef SSL_set_tlsext_host_name
    SSL_set_tlsext_host_name(result->ssl, NULL);
#endif
    SSL_free(result->ssl);
    tor_free(result);
    goto err;
  }

  result->socket = sock;
  bio = BIO_new_socket(sock, BIO_CLOSE);
  if (! bio) {
    tls_log_errors(NULL, LOG_WARN, LD_NET, "opening BIO");
#ifdef SSL_set_tlsext_host_name
    SSL_set_tlsext_host_name(result->ssl, NULL);
#endif
    SSL_free(result->ssl);
    tor_free(result);
    goto err;
  }
  {
    int set_worked =
      SSL_set_ex_data(result->ssl, tor_tls_object_ex_data_index, result);
    if (!set_worked) {
      log_warn(LD_BUG,
               "Couldn't set the tls for an SSL*; connection will fail");
    }
  }
  SSL_set_bio(result->ssl, bio, bio);
  tor_tls_context_incref(context);
  result->context = context;
  result->state = TOR_TLS_ST_HANDSHAKE;
  result->isServer = isServer;
  result->wantwrite_n = 0;
  result->last_write_count = (unsigned long) BIO_number_written(bio);
  result->last_read_count = (unsigned long) BIO_number_read(bio);
  if (result->last_write_count || result->last_read_count) {
    log_warn(LD_NET, "Newly created BIO has read count %lu, write count %lu",
             result->last_read_count, result->last_write_count);
  }

  SSL_set_info_callback(result->ssl, tor_tls_debug_state_callback);

  goto done;
 err:
  result = NULL;
 done:
  /* Not expected to get called. */
  tls_log_errors(NULL, LOG_WARN, LD_NET, "creating tor_tls_t object");
  return result;
}

/**
 * Tell the TLS library that the underlying socket for <b>tls</b> has been
 * closed, and the library should not attempt to free that socket itself.
 */
void
tor_tls_release_socket(tor_tls_t *tls)
{
  if (! tls)
    return;

  BIO *rbio, *wbio;
  rbio = SSL_get_rbio(tls->ssl);
  wbio = SSL_get_wbio(tls->ssl);

  if (rbio) {
    (void) BIO_set_close(rbio, BIO_NOCLOSE);
  }
  if (wbio && wbio != rbio) {
    (void) BIO_set_close(wbio, BIO_NOCLOSE);
  }
}

void
tor_tls_impl_free_(tor_tls_impl_t *ssl)
{
  if (!ssl)
    return;

#ifdef SSL_set_tlsext_host_name
  SSL_set_tlsext_host_name(ssl, NULL);
#endif
  SSL_free(ssl);
}

/** Underlying function for TLS reading.  Reads up to <b>len</b>
 * characters from <b>tls</b> into <b>cp</b>.  On success, returns the
 * number of characters read.  On failure, returns TOR_TLS_ERROR,
 * TOR_TLS_CLOSE, TOR_TLS_WANTREAD, or TOR_TLS_WANTWRITE.
 */
MOCK_IMPL(int,
tor_tls_read,(tor_tls_t *tls, char *cp, size_t len))
{
  int r, err;
  tor_assert(tls);
  tor_assert(tls->ssl);
  tor_assert(tls->state == TOR_TLS_ST_OPEN);
  tor_assert(len<INT_MAX);
  r = SSL_read(tls->ssl, cp, (int)len);
  if (r > 0) {
    return r;
  }
  err = tor_tls_get_error(tls, r, CATCH_ZERO, "reading", LOG_DEBUG, LD_NET);
  if (err == TOR_TLS_ZERORETURN_ || err == TOR_TLS_CLOSE) {
    log_debug(LD_NET,"read returned r=%d; TLS is closed",r);
    tls->state = TOR_TLS_ST_CLOSED;
    return TOR_TLS_CLOSE;
  } else {
    tor_assert(err != TOR_TLS_DONE);
    log_debug(LD_NET,"read returned r=%d, err=%d",r,err);
    return err;
  }
}

/** Total number of bytes that we've used TLS to send.  Used to track TLS
 * overhead. */
STATIC uint64_t total_bytes_written_over_tls = 0;
/** Total number of bytes that TLS has put on the network for us. Used to
 * track TLS overhead. */
STATIC uint64_t total_bytes_written_by_tls = 0;

/** Underlying function for TLS writing.  Write up to <b>n</b>
 * characters from <b>cp</b> onto <b>tls</b>.  On success, returns the
 * number of characters written.  On failure, returns TOR_TLS_ERROR,
 * TOR_TLS_WANTREAD, or TOR_TLS_WANTWRITE.
 */
int
tor_tls_write(tor_tls_t *tls, const char *cp, size_t n)
{
  int r, err;
  tor_assert(tls);
  tor_assert(tls->ssl);
  tor_assert(tls->state == TOR_TLS_ST_OPEN);
  tor_assert(n < INT_MAX);
  if (n == 0)
    return 0;
  if (tls->wantwrite_n) {
    /* if WANTWRITE last time, we must use the _same_ n as before */
    tor_assert(n >= tls->wantwrite_n);
    log_debug(LD_NET,"resuming pending-write, (%d to flush, reusing %d)",
              (int)n, (int)tls->wantwrite_n);
    n = tls->wantwrite_n;
    tls->wantwrite_n = 0;
  }
  r = SSL_write(tls->ssl, cp, (int)n);
  err = tor_tls_get_error(tls, r, 0, "writing", LOG_INFO, LD_NET);
  if (err == TOR_TLS_DONE) {
    total_bytes_written_over_tls += r;
    return r;
  }
  if (err == TOR_TLS_WANTWRITE || err == TOR_TLS_WANTREAD) {
    tls->wantwrite_n = n;
  }
  return err;
}

/** Perform initial handshake on <b>tls</b>.  When finished, returns
 * TOR_TLS_DONE.  On failure, returns TOR_TLS_ERROR, TOR_TLS_WANTREAD,
 * or TOR_TLS_WANTWRITE.
 */
int
tor_tls_handshake(tor_tls_t *tls)
{
  int r;
  tor_assert(tls);
  tor_assert(tls->ssl);
  tor_assert(tls->state == TOR_TLS_ST_HANDSHAKE);

  check_no_tls_errors();

  OSSL_HANDSHAKE_STATE oldstate = SSL_get_state(tls->ssl);

  if (tls->isServer) {
    log_debug(LD_HANDSHAKE, "About to call SSL_accept on %p (%s)", tls,
              SSL_state_string_long(tls->ssl));
    r = SSL_accept(tls->ssl);
  } else {
    log_debug(LD_HANDSHAKE, "About to call SSL_connect on %p (%s)", tls,
              SSL_state_string_long(tls->ssl));
    r = SSL_connect(tls->ssl);
  }

  OSSL_HANDSHAKE_STATE newstate = SSL_get_state(tls->ssl);

  if (oldstate != newstate)
    log_debug(LD_HANDSHAKE, "After call, %p was in state %s",
              tls, SSL_state_string_long(tls->ssl));

  r = tor_tls_get_error(tls,r,0, "handshaking", LOG_INFO, LD_HANDSHAKE);
  if (ERR_peek_error() != 0) {
    tls_log_errors(tls, tls->isServer ? LOG_INFO : LOG_WARN, LD_HANDSHAKE,
                   "handshaking");
    return TOR_TLS_ERROR_MISC;
  }
  if (r == TOR_TLS_DONE) {
    tls->state = TOR_TLS_ST_OPEN;
  }
  return r;
}

/** Return true iff this TLS connection is authenticated.
 */
int
tor_tls_peer_has_cert(tor_tls_t *tls)
{
  X509 *cert;
  cert = SSL_get_peer_certificate(tls->ssl);
  tls_log_errors(tls, LOG_WARN, LD_HANDSHAKE, "getting peer certificate");
  if (!cert)
    return 0;
  X509_free(cert);
  return 1;
}

/** Return a newly allocated copy of the peer certificate, or NULL if there
 * isn't one. */
MOCK_IMPL(tor_x509_cert_t *,
tor_tls_get_peer_cert,(tor_tls_t *tls))
{
  X509 *cert;
  cert = SSL_get_peer_certificate(tls->ssl);
  tls_log_errors(tls, LOG_WARN, LD_HANDSHAKE, "getting peer certificate");
  if (!cert)
    return NULL;
  return tor_x509_cert_new(cert);
}

/** Return a newly allocated copy of the cerficate we used on the connection,
 * or NULL if somehow we didn't use one. */
MOCK_IMPL(tor_x509_cert_t *,
tor_tls_get_own_cert,(tor_tls_t *tls))
{
  X509 *cert = SSL_get_certificate(tls->ssl);
  tls_log_errors(tls, LOG_WARN, LD_HANDSHAKE,
                 "getting own-connection certificate");
  if (!cert)
    return NULL;
  /* Fun inconsistency: SSL_get_peer_certificate increments the reference
   * count, but SSL_get_certificate does not. */
  X509 *duplicate = X509_dup(cert);
  if (BUG(duplicate == NULL))
    return NULL;
  return tor_x509_cert_new(duplicate);
}

/** Return the number of bytes available for reading from <b>tls</b>.
 */
int
tor_tls_get_pending_bytes(tor_tls_t *tls)
{
  tor_assert(tls);
  return SSL_pending(tls->ssl);
}

/** If <b>tls</b> requires that the next write be of a particular size,
 * return that size.  Otherwise, return 0. */
size_t
tor_tls_get_forced_write_size(tor_tls_t *tls)
{
  return tls->wantwrite_n;
}

/** Sets n_read and n_written to the number of bytes read and written,
 * respectively, on the raw socket used by <b>tls</b> since the last time this
 * function was called on <b>tls</b>. */
void
tor_tls_get_n_raw_bytes(tor_tls_t *tls, size_t *n_read, size_t *n_written)
{
  BIO *wbio, *tmpbio;
  unsigned long r, w;
  r = (unsigned long) BIO_number_read(SSL_get_rbio(tls->ssl));
  /* We want the number of bytes actually for real written.  Unfortunately,
   * sometimes OpenSSL replaces the wbio on tls->ssl with a buffering bio,
   * which makes the answer turn out wrong.  Let's cope with that.  Note
   * that this approach will fail if we ever replace tls->ssl's BIOs with
   * buffering bios for reasons of our own.  As an alternative, we could
   * save the original BIO for  tls->ssl in the tor_tls_t structure, but
   * that would be tempting fate. */
  wbio = SSL_get_wbio(tls->ssl);
  if (BIO_method_type(wbio) == BIO_TYPE_BUFFER &&
        (tmpbio = BIO_next(wbio)) != NULL)
    wbio = tmpbio;
  w = (unsigned long) BIO_number_written(wbio);

  /* We are ok with letting these unsigned ints go "negative" here:
   * If we wrapped around, this should still give us the right answer, unless
   * we wrapped around by more than ULONG_MAX since the last time we called
   * this function.
   */
  *n_read = (size_t)(r - tls->last_read_count);
  *n_written = (size_t)(w - tls->last_write_count);
  if (*n_read > INT_MAX || *n_written > INT_MAX) {
    log_warn(LD_BUG, "Preposterously large value in tor_tls_get_n_raw_bytes. "
             "r=%lu, last_read=%lu, w=%lu, last_written=%lu",
             r, tls->last_read_count, w, tls->last_write_count);
  }
  total_bytes_written_by_tls += *n_written;
  tls->last_read_count = r;
  tls->last_write_count = w;
}

/** Return a ratio of the bytes that TLS has sent to the bytes that we've told
 * it to send. Used to track whether our TLS records are getting too tiny. */
MOCK_IMPL(double,
tls_get_write_overhead_ratio,(void))
{
  if (total_bytes_written_over_tls == 0)
    return 1.0;

  return ((double)total_bytes_written_by_tls) /
    ((double)total_bytes_written_over_tls);
}

/** Implement check_no_tls_errors: If there are any pending OpenSSL
 * errors, log an error message. */
void
check_no_tls_errors_(const char *fname, int line)
{
  if (ERR_peek_error() == 0)
    return;
  log_warn(LD_CRYPTO, "Unhandled OpenSSL errors found at %s:%d: ",
      tor_fix_source_file(fname), line);
  tls_log_errors(NULL, LOG_WARN, LD_NET, NULL);
}

/** Using the RFC5705 key material exporting construction, and the
 * provided <b>context</b> (<b>context_len</b> bytes long) and
 * <b>label</b> (a NUL-terminated string), compute a 32-byte secret in
 * <b>secrets_out</b> that only the parties to this TLS session can
 * compute.  Return 0 on success; -1 on failure; and -2 on failure
 * caused by OpenSSL bug 7712.
 */
MOCK_IMPL(int,
tor_tls_export_key_material,(tor_tls_t *tls, uint8_t *secrets_out,
                             const uint8_t *context,
                             size_t context_len,
                             const char *label))
{
  tor_assert(tls);
  tor_assert(tls->ssl);

  int r = SSL_export_keying_material(tls->ssl,
                                     secrets_out, DIGEST256_LEN,
                                     label, strlen(label),
                                     context, context_len, 1);

  if (r != 1) {
    int severity = openssl_bug_7712_is_present ? LOG_WARN : LOG_DEBUG;
    tls_log_errors(tls, severity, LD_NET, "exporting keying material");
  }

#ifdef TLS1_3_VERSION
  if (r != 1 &&
      strlen(label) > 12 &&
      SSL_version(tls->ssl) >= TLS1_3_VERSION) {

    if (! openssl_bug_7712_is_present) {
      /* We might have run into OpenSSL issue 7712, which caused OpenSSL
       * 1.1.1a to not handle long labels.  Let's test to see if we have.
       */
      r = SSL_export_keying_material(tls->ssl, secrets_out, DIGEST256_LEN,
                                     "short", 5, context, context_len, 1);
      if (r == 1) {
        /* A short label succeeds, but a long label fails. This was openssl
         * issue 7712. */
        openssl_bug_7712_is_present = 1;
        log_warn(LD_GENERAL, "Detected OpenSSL bug 7712: disabling TLS 1.3 on "
                 "future connections.");
      }
    }
    if (openssl_bug_7712_is_present)
      return -2;
    else
      return -1;
  }
#endif /* defined(TLS1_3_VERSION) */

  return (r == 1) ? 0 : -1;
}

/** Examine the amount of memory used and available for buffers in <b>tls</b>.
 * Set *<b>rbuf_capacity</b> to the amount of storage allocated for the read
 * buffer and *<b>rbuf_bytes</b> to the amount actually used.
 * Set *<b>wbuf_capacity</b> to the amount of storage allocated for the write
 * buffer and *<b>wbuf_bytes</b> to the amount actually used.
 *
 * Return 0 on success, -1 on failure.*/
int
tor_tls_get_buffer_sizes(tor_tls_t *tls,
                         size_t *rbuf_capacity, size_t *rbuf_bytes,
                         size_t *wbuf_capacity, size_t *wbuf_bytes)
{
  (void)tls;
  (void)rbuf_capacity;
  (void)rbuf_bytes;
  (void)wbuf_capacity;
  (void)wbuf_bytes;

  return -1;
}

/** Check whether the ECC group requested is supported by the current OpenSSL
 * library instance.  Return 1 if the group is supported, and 0 if not.
 */
int
evaluate_ecgroup_for_tls(const char *ecgroup)
{
  EC_KEY *ec_key;
  int nid;
  int ret;

  if (!ecgroup)
    nid = NID_tor_default_ecdhe_group;
  else if (!strcasecmp(ecgroup, "P256"))
    nid = NID_X9_62_prime256v1;
  else if (!strcasecmp(ecgroup, "P224"))
    nid = NID_secp224r1;
  else
    return 0;

  ec_key = EC_KEY_new_by_curve_name(nid);
  ret = (ec_key != NULL);
  EC_KEY_free(ec_key);

  return ret;
}

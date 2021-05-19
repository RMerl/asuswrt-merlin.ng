/* Copyright (c) 2004-2006, Roger Dingledine, Nick Mathewson.
 * Copyright (c) 2007-2020, The Tor Project, Inc. */
/* See LICENSE for licensing information */

/**
 * \file control_auth.c
 * \brief Authentication for Tor's control-socket interface.
 **/

#include "core/or/or.h"
#include "app/config/config.h"
#include "core/mainloop/connection.h"
#include "feature/control/control.h"
#include "feature/control/control_cmd.h"
#include "feature/control/control_auth.h"
#include "feature/control/control_cmd_args_st.h"
#include "feature/control/control_connection_st.h"
#include "feature/control/control_proto.h"
#include "lib/crypt_ops/crypto_rand.h"
#include "lib/crypt_ops/crypto_util.h"
#include "lib/encoding/confline.h"
#include "lib/encoding/kvline.h"
#include "lib/encoding/qstring.h"

#include "lib/crypt_ops/crypto_s2k.h"

/** If we're using cookie-type authentication, how long should our cookies be?
 */
#define AUTHENTICATION_COOKIE_LEN 32

/** If true, we've set authentication_cookie to a secret code and
 * stored it to disk. */
static int authentication_cookie_is_set = 0;
/** If authentication_cookie_is_set, a secret cookie that we've stored to disk
 * and which we're using to authenticate controllers.  (If the controller can
 * read it off disk, it has permission to connect.) */
static uint8_t *authentication_cookie = NULL;

#define SAFECOOKIE_SERVER_TO_CONTROLLER_CONSTANT \
  "Tor safe cookie authentication server-to-controller hash"
#define SAFECOOKIE_CONTROLLER_TO_SERVER_CONSTANT \
  "Tor safe cookie authentication controller-to-server hash"
#define SAFECOOKIE_SERVER_NONCE_LEN DIGEST256_LEN

/** Helper: Return a newly allocated string containing a path to the
 * file where we store our authentication cookie. */
char *
get_controller_cookie_file_name(void)
{
  const or_options_t *options = get_options();
  if (options->CookieAuthFile && strlen(options->CookieAuthFile)) {
    return tor_strdup(options->CookieAuthFile);
  } else {
    return get_datadir_fname("control_auth_cookie");
  }
}

/* Initialize the cookie-based authentication system of the
 * ControlPort. If <b>enabled</b> is 0, then disable the cookie
 * authentication system.  */
int
init_control_cookie_authentication(int enabled)
{
  char *fname = NULL;
  int retval;

  if (!enabled) {
    authentication_cookie_is_set = 0;
    return 0;
  }

  fname = get_controller_cookie_file_name();
  retval = init_cookie_authentication(fname, "", /* no header */
                                      AUTHENTICATION_COOKIE_LEN,
                                   get_options()->CookieAuthFileGroupReadable,
                                      &authentication_cookie,
                                      &authentication_cookie_is_set);
  tor_free(fname);
  return retval;
}

/** Decode the hashed, base64'd passwords stored in <b>passwords</b>.
 * Return a smartlist of acceptable passwords (unterminated strings of
 * length S2K_RFC2440_SPECIFIER_LEN+DIGEST_LEN) on success, or NULL on
 * failure.
 */
smartlist_t *
decode_hashed_passwords(config_line_t *passwords)
{
  char decoded[64];
  config_line_t *cl;
  smartlist_t *sl = smartlist_new();

  tor_assert(passwords);

  for (cl = passwords; cl; cl = cl->next) {
    const char *hashed = cl->value;

    if (!strcmpstart(hashed, "16:")) {
      if (base16_decode(decoded, sizeof(decoded), hashed+3, strlen(hashed+3))
                        != S2K_RFC2440_SPECIFIER_LEN + DIGEST_LEN
          || strlen(hashed+3) != (S2K_RFC2440_SPECIFIER_LEN+DIGEST_LEN)*2) {
        goto err;
      }
    } else {
        if (base64_decode(decoded, sizeof(decoded), hashed, strlen(hashed))
            != S2K_RFC2440_SPECIFIER_LEN+DIGEST_LEN) {
          goto err;
        }
    }
    smartlist_add(sl,
                  tor_memdup(decoded, S2K_RFC2440_SPECIFIER_LEN+DIGEST_LEN));
  }

  return sl;

 err:
  SMARTLIST_FOREACH(sl, char*, cp, tor_free(cp));
  smartlist_free(sl);
  return NULL;
}

const control_cmd_syntax_t authchallenge_syntax = {
   .min_args = 1,
   .max_args = 1,
   .accept_keywords=true,
   .kvline_flags=KV_OMIT_KEYS|KV_QUOTED_QSTRING,
   .store_raw_body=true
};

/** Called when we get an AUTHCHALLENGE command. */
int
handle_control_authchallenge(control_connection_t *conn,
                             const control_cmd_args_t *args)
{
  char *client_nonce;
  size_t client_nonce_len;
  char server_hash[DIGEST256_LEN];
  char server_hash_encoded[HEX_DIGEST256_LEN+1];
  char server_nonce[SAFECOOKIE_SERVER_NONCE_LEN];
  char server_nonce_encoded[(2*SAFECOOKIE_SERVER_NONCE_LEN) + 1];

  if (strcasecmp(smartlist_get(args->args, 0), "SAFECOOKIE")) {
    control_write_endreply(conn, 513,
                           "AUTHCHALLENGE only supports SAFECOOKIE "
                           "authentication");
    goto fail;
  }
  if (!authentication_cookie_is_set) {
    control_write_endreply(conn, 515, "Cookie authentication is disabled");
    goto fail;
  }
  if (args->kwargs == NULL || args->kwargs->next != NULL) {
    control_write_endreply(conn, 512,
                           "Wrong number of arguments for AUTHCHALLENGE");
    goto fail;
  }
  if (strcmp(args->kwargs->key, "")) {
    control_write_endreply(conn, 512,
                           "AUTHCHALLENGE does not accept keyword "
                           "arguments.");
    goto fail;
  }

  bool contains_quote = strchr(args->raw_body, '\"');
  if (contains_quote) {
    /* The nonce was quoted */
    client_nonce = tor_strdup(args->kwargs->value);
    client_nonce_len = strlen(client_nonce);
  } else {
    /* The nonce was should be in hex. */
    const char *hex_nonce = args->kwargs->value;
    client_nonce_len = strlen(hex_nonce) / 2;
    client_nonce = tor_malloc(client_nonce_len);
    if (base16_decode(client_nonce, client_nonce_len, hex_nonce,
                      strlen(hex_nonce)) != (int)client_nonce_len) {
      control_write_endreply(conn, 513, "Invalid base16 client nonce");
      tor_free(client_nonce);
      goto fail;
    }
  }

  crypto_rand(server_nonce, SAFECOOKIE_SERVER_NONCE_LEN);

  /* Now compute and send the server-to-controller response, and the
   * server's nonce. */
  tor_assert(authentication_cookie != NULL);

  {
    size_t tmp_len = (AUTHENTICATION_COOKIE_LEN +
                      client_nonce_len +
                      SAFECOOKIE_SERVER_NONCE_LEN);
    char *tmp = tor_malloc_zero(tmp_len);
    char *client_hash = tor_malloc_zero(DIGEST256_LEN);
    memcpy(tmp, authentication_cookie, AUTHENTICATION_COOKIE_LEN);
    memcpy(tmp + AUTHENTICATION_COOKIE_LEN, client_nonce, client_nonce_len);
    memcpy(tmp + AUTHENTICATION_COOKIE_LEN + client_nonce_len,
           server_nonce, SAFECOOKIE_SERVER_NONCE_LEN);

    crypto_hmac_sha256(server_hash,
                       SAFECOOKIE_SERVER_TO_CONTROLLER_CONSTANT,
                       strlen(SAFECOOKIE_SERVER_TO_CONTROLLER_CONSTANT),
                       tmp,
                       tmp_len);

    crypto_hmac_sha256(client_hash,
                       SAFECOOKIE_CONTROLLER_TO_SERVER_CONSTANT,
                       strlen(SAFECOOKIE_CONTROLLER_TO_SERVER_CONSTANT),
                       tmp,
                       tmp_len);

    conn->safecookie_client_hash = client_hash;

    tor_free(tmp);
  }

  base16_encode(server_hash_encoded, sizeof(server_hash_encoded),
                server_hash, sizeof(server_hash));
  base16_encode(server_nonce_encoded, sizeof(server_nonce_encoded),
                server_nonce, sizeof(server_nonce));

  control_printf_endreply(conn, 250,
                          "AUTHCHALLENGE SERVERHASH=%s SERVERNONCE=%s",
                          server_hash_encoded,
                          server_nonce_encoded);

  tor_free(client_nonce);
  return 0;
 fail:
  connection_mark_for_close(TO_CONN(conn));
  return -1;
}

const control_cmd_syntax_t authenticate_syntax = {
   .max_args = 0,
   .accept_keywords=true,
   .kvline_flags=KV_OMIT_KEYS|KV_QUOTED_QSTRING,
   .store_raw_body=true
};

/** Called when we get an AUTHENTICATE message.  Check whether the
 * authentication is valid, and if so, update the connection's state to
 * OPEN.  Reply with DONE or ERROR.
 */
int
handle_control_authenticate(control_connection_t *conn,
                            const control_cmd_args_t *args)
{
  bool used_quoted_string = false;
  const or_options_t *options = get_options();
  const char *errstr = "Unknown error";
  char *password;
  size_t password_len;
  int bad_cookie=0, bad_password=0;
  smartlist_t *sl = NULL;

  if (args->kwargs == NULL) {
    password = tor_strdup("");
    password_len = 0;
  } else if (args->kwargs->next) {
    control_write_endreply(conn, 512, "Too many arguments to AUTHENTICATE.");
    connection_mark_for_close(TO_CONN(conn));
    return 0;
  } else if (strcmp(args->kwargs->key, "")) {
    control_write_endreply(conn, 512,
                           "AUTHENTICATE does not accept keyword arguments.");
    connection_mark_for_close(TO_CONN(conn));
    return 0;
  } else if (strchr(args->raw_body, '\"')) {
    used_quoted_string = true;
    password = tor_strdup(args->kwargs->value);
    password_len = strlen(password);
  } else {
    const char *hex_passwd = args->kwargs->value;
    password_len = strlen(hex_passwd) / 2;
    password = tor_malloc(password_len+1);
    if (base16_decode(password, password_len+1, hex_passwd, strlen(hex_passwd))
                      != (int) password_len) {
      control_write_endreply(conn, 551,
            "Invalid hexadecimal encoding.  Maybe you tried a plain text "
            "password?  If so, the standard requires that you put it in "
            "double quotes.");
      connection_mark_for_close(TO_CONN(conn));
      tor_free(password);
      return 0;
    }
  }

  if (conn->safecookie_client_hash != NULL) {
    /* The controller has chosen safe cookie authentication; the only
     * acceptable authentication value is the controller-to-server
     * response. */

    tor_assert(authentication_cookie_is_set);

    if (password_len != DIGEST256_LEN) {
      log_warn(LD_CONTROL,
               "Got safe cookie authentication response with wrong length "
               "(%d)", (int)password_len);
      errstr = "Wrong length for safe cookie response.";
      goto err;
    }

    if (tor_memneq(conn->safecookie_client_hash, password, DIGEST256_LEN)) {
      log_warn(LD_CONTROL,
               "Got incorrect safe cookie authentication response");
      errstr = "Safe cookie response did not match expected value.";
      goto err;
    }

    tor_free(conn->safecookie_client_hash);
    goto ok;
  }

  if (!options->CookieAuthentication && !options->HashedControlPassword &&
      !options->HashedControlSessionPassword) {
    /* if Tor doesn't demand any stronger authentication, then
     * the controller can get in with anything. */
    goto ok;
  }

  if (options->CookieAuthentication) {
    int also_password = options->HashedControlPassword != NULL ||
      options->HashedControlSessionPassword != NULL;
    if (password_len != AUTHENTICATION_COOKIE_LEN) {
      if (!also_password) {
        log_warn(LD_CONTROL, "Got authentication cookie with wrong length "
                 "(%d)", (int)password_len);
        errstr = "Wrong length on authentication cookie.";
        goto err;
      }
      bad_cookie = 1;
    } else if (tor_memneq(authentication_cookie, password, password_len)) {
      if (!also_password) {
        log_warn(LD_CONTROL, "Got mismatched authentication cookie");
        errstr = "Authentication cookie did not match expected value.";
        goto err;
      }
      bad_cookie = 1;
    } else {
      goto ok;
    }
  }

  if (options->HashedControlPassword ||
      options->HashedControlSessionPassword) {
    int bad = 0;
    smartlist_t *sl_tmp;
    char received[DIGEST_LEN];
    int also_cookie = options->CookieAuthentication;
    sl = smartlist_new();
    if (options->HashedControlPassword) {
      sl_tmp = decode_hashed_passwords(options->HashedControlPassword);
      if (!sl_tmp)
        bad = 1;
      else {
        smartlist_add_all(sl, sl_tmp);
        smartlist_free(sl_tmp);
      }
    }
    if (options->HashedControlSessionPassword) {
      sl_tmp = decode_hashed_passwords(options->HashedControlSessionPassword);
      if (!sl_tmp)
        bad = 1;
      else {
        smartlist_add_all(sl, sl_tmp);
        smartlist_free(sl_tmp);
      }
    }
    if (bad) {
      if (!also_cookie) {
        log_warn(LD_BUG,
                 "Couldn't decode HashedControlPassword: invalid base16");
        errstr="Couldn't decode HashedControlPassword value in configuration.";
        goto err;
      }
      bad_password = 1;
      SMARTLIST_FOREACH(sl, char *, str, tor_free(str));
      smartlist_free(sl);
      sl = NULL;
    } else {
      SMARTLIST_FOREACH(sl, char *, expected,
      {
        secret_to_key_rfc2440(received,DIGEST_LEN,
                              password,password_len,expected);
        if (tor_memeq(expected + S2K_RFC2440_SPECIFIER_LEN,
                      received, DIGEST_LEN))
          goto ok;
      });
      SMARTLIST_FOREACH(sl, char *, str, tor_free(str));
      smartlist_free(sl);
      sl = NULL;

      if (used_quoted_string)
        errstr = "Password did not match HashedControlPassword value from "
          "configuration";
      else
        errstr = "Password did not match HashedControlPassword value from "
          "configuration. Maybe you tried a plain text password? "
          "If so, the standard requires that you put it in double quotes.";
      bad_password = 1;
      if (!also_cookie)
        goto err;
    }
  }

  /** We only get here if both kinds of authentication failed. */
  tor_assert(bad_password && bad_cookie);
  log_warn(LD_CONTROL, "Bad password or authentication cookie on controller.");
  errstr = "Password did not match HashedControlPassword *or* authentication "
    "cookie.";

 err:
  tor_free(password);
  control_printf_endreply(conn, 515, "Authentication failed: %s", errstr);
  connection_mark_for_close(TO_CONN(conn));
  if (sl) { /* clean up */
    SMARTLIST_FOREACH(sl, char *, str, tor_free(str));
    smartlist_free(sl);
  }
  return 0;
 ok:
  log_info(LD_CONTROL, "Authenticated control connection ("TOR_SOCKET_T_FORMAT
           ")", conn->base_.s);
  send_control_done(conn);
  conn->base_.state = CONTROL_CONN_STATE_OPEN;
  tor_free(password);
  if (sl) { /* clean up */
    SMARTLIST_FOREACH(sl, char *, str, tor_free(str));
    smartlist_free(sl);
  }
  return 0;
}

void
control_auth_free_all(void)
{
  if (authentication_cookie) /* Free the auth cookie */
    tor_free(authentication_cookie);
  authentication_cookie_is_set = 0;
}

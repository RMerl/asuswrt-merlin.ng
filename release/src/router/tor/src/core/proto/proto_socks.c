/* Copyright (c) 2001 Matej Pfajfar.
 * Copyright (c) 2001-2004, Roger Dingledine.
 * Copyright (c) 2004-2006, Roger Dingledine, Nick Mathewson.
 * Copyright (c) 2007-2020, The Tor Project, Inc. */
/* See LICENSE for licensing information */

/**
 * @file proto_socks.c
 * @brief Implementations for SOCKS4 and SOCKS5 protocols.
 **/

#include "core/or/or.h"
#include "feature/client/addressmap.h"
#include "lib/buf/buffers.h"
#include "core/mainloop/connection.h"
#include "feature/control/control_events.h"
#include "app/config/config.h"
#include "lib/crypt_ops/crypto_util.h"
#include "feature/relay/ext_orport.h"
#include "core/proto/proto_socks.h"
#include "core/or/reasons.h"

#include "core/or/socks_request_st.h"

#include "trunnel/socks5.h"

#define SOCKS_VER_5 0x05 /* First octet of non-auth SOCKS5 messages */
#define SOCKS_VER_4 0x04 /*                SOCKS4 messages */
#define SOCKS_AUTH  0x01 /*                SOCKS5 auth messages */

typedef enum {
  SOCKS_RESULT_INVALID       = -1, /* Message invalid. */
  SOCKS_RESULT_TRUNCATED     =  0, /* Message incomplete/truncated. */
  SOCKS_RESULT_DONE          =  1, /* OK, we're done. */
  SOCKS_RESULT_MORE_EXPECTED =  2, /* OK, more messages expected. */
} socks_result_t;

static void socks_request_set_socks5_error(socks_request_t *req,
                              socks5_reply_status_t reason);

static socks_result_t parse_socks(const char *data,
                                  size_t datalen,
                                  socks_request_t *req,
                                  int log_sockstype,
                                  int safe_socks,
                                  size_t *drain_out);
static int parse_socks_client(const uint8_t *data, size_t datalen,
                              int state, char **reason,
                              ssize_t *drain_out);
/**
 * Wait this many seconds before warning the user about using SOCKS unsafely
 * again. */
#define SOCKS_WARN_INTERVAL 5

/** Warn that the user application has made an unsafe socks request using
 * protocol <b>socks_protocol</b> on port <b>port</b>.  Don't warn more than
 * once per SOCKS_WARN_INTERVAL, unless <b>safe_socks</b> is set. */
static void
log_unsafe_socks_warning(int socks_protocol, const char *address,
                         uint16_t port, int safe_socks)
{
  static ratelim_t socks_ratelim = RATELIM_INIT(SOCKS_WARN_INTERVAL);

  if (safe_socks) {
    log_fn_ratelim(&socks_ratelim, LOG_WARN, LD_APP,
             "Your application (using socks%d to port %d) is giving "
             "Tor only an IP address. Applications that do DNS resolves "
             "themselves may leak information. Consider using Socks4A "
             "(e.g. via privoxy or socat) instead. For more information, "
             "please see https://2019.www.torproject.org/docs/faq.html.en"
             "#WarningsAboutSOCKSandDNSInformationLeaks.%s",
             socks_protocol,
             (int)port,
             safe_socks ? " Rejecting." : "");
  }
  control_event_client_status(LOG_WARN,
                              "DANGEROUS_SOCKS PROTOCOL=SOCKS%d ADDRESS=%s:%d",
                              socks_protocol, address, (int)port);
}

/** Do not attempt to parse socks messages longer than this.  This value is
 * actually significantly higher than the longest possible socks message. */
#define MAX_SOCKS_MESSAGE_LEN 512

/** Return a new socks_request_t. */
socks_request_t *
socks_request_new(void)
{
  return tor_malloc_zero(sizeof(socks_request_t));
}

/** Free all storage held in the socks_request_t <b>req</b>. */
void
socks_request_free_(socks_request_t *req)
{
  if (!req)
    return;
  if (req->username) {
    memwipe(req->username, 0x10, req->usernamelen);
    tor_free(req->username);
  }
  if (req->password) {
    memwipe(req->password, 0x04, req->passwordlen);
    tor_free(req->password);
  }
  memwipe(req, 0xCC, sizeof(socks_request_t));
  tor_free(req);
}

/**
 * Parse a single SOCKS4 request from buffer <b>raw_data</b> of length
 * <b>datalen</b> and update relevant fields of <b>req</b>. If SOCKS4a
 * request is detected, set <b>*is_socks4a</b> to true. Set <b>*drain_out</b>
 * to number of bytes we parsed so far.
 *
 * Return SOCKS_RESULT_DONE if parsing succeeded, SOCKS_RESULT_INVALID if
 * parsing failed because of invalid input or SOCKS_RESULT_TRUNCATED if it
 * failed due to incomplete (truncated) input.
 */
static socks_result_t
parse_socks4_request(const uint8_t *raw_data, socks_request_t *req,
                     size_t datalen, int *is_socks4a, size_t *drain_out)
{
  // http://ss5.sourceforge.net/socks4.protocol.txt
  // http://ss5.sourceforge.net/socks4A.protocol.txt
  socks_result_t res = SOCKS_RESULT_DONE;
  tor_addr_t destaddr;

  tor_assert(is_socks4a);
  tor_assert(drain_out);

  *is_socks4a = 0;
  *drain_out = 0;

  req->socks_version = SOCKS_VER_4;

  socks4_client_request_t *trunnel_req;

  ssize_t parsed =
  socks4_client_request_parse(&trunnel_req, raw_data, datalen);

  if (parsed == -1) {
    log_warn(LD_APP, "socks4: parsing failed - invalid request.");
    res = SOCKS_RESULT_INVALID;
    goto end;
  } else if (parsed == -2) {
    res = SOCKS_RESULT_TRUNCATED;
    if (datalen >= MAX_SOCKS_MESSAGE_LEN) {
      log_warn(LD_APP, "socks4: parsing failed - invalid request.");
      res = SOCKS_RESULT_INVALID;
    }
    goto end;
  }

  tor_assert(parsed >= 0);
  *drain_out = (size_t)parsed;

  uint8_t command = socks4_client_request_get_command(trunnel_req);
  req->command = command;

  req->port = socks4_client_request_get_port(trunnel_req);
  uint32_t dest_ip = socks4_client_request_get_addr(trunnel_req);

  if ((!req->port && req->command != SOCKS_COMMAND_RESOLVE) ||
      dest_ip == 0) {
    log_warn(LD_APP, "socks4: Port or DestIP is zero. Rejecting.");
    res = SOCKS_RESULT_INVALID;
    goto end;
  }

  *is_socks4a = (dest_ip >> 8) == 0;

  const char *username = socks4_client_request_get_username(trunnel_req);
  const size_t usernamelen = username ? strlen(username) : 0;
  if (username && usernamelen) {
    if (usernamelen > MAX_SOCKS_MESSAGE_LEN) {
      log_warn(LD_APP, "Socks4 user name too long; rejecting.");
      res = SOCKS_RESULT_INVALID;
      goto end;
    }

    tor_free(req->username);
    req->got_auth = 1;
    req->username = tor_strdup(username);
    req->usernamelen = usernamelen;
  }

  if (*is_socks4a) {
    // We cannot rely on trunnel here, as we want to detect if
    // we have abnormally long hostname field.
    const char *hostname = (char *)raw_data + SOCKS4_NETWORK_LEN +
     usernamelen + 1;
    size_t hostname_len = (char *)raw_data + datalen - hostname;

    if (hostname_len <= sizeof(req->address)) {
      const char *trunnel_hostname =
      socks4_client_request_get_socks4a_addr_hostname(trunnel_req);

      if (trunnel_hostname)
        strlcpy(req->address, trunnel_hostname, sizeof(req->address));
    } else {
      log_warn(LD_APP, "socks4: Destaddr too long. Rejecting.");
      res = SOCKS_RESULT_INVALID;
      goto end;
    }
  } else {
    tor_addr_from_ipv4h(&destaddr, dest_ip);

    if (!tor_addr_to_str(req->address, &destaddr,
                         MAX_SOCKS_ADDR_LEN, 0)) {
      res = SOCKS_RESULT_INVALID;
      goto end;
    }
  }

  end:
  socks4_client_request_free(trunnel_req);

  return res;
}

/**
 * Validate SOCKS4/4a related fields in <b>req</b>. Expect SOCKS4a
 * if <b>is_socks4a</b> is true. If <b>log_sockstype</b> is true,
 * log a notice about possible DNS leaks on local system. If
 * <b>safe_socks</b> is true, reject insecure usage of SOCKS
 * protocol.
 *
 * Return SOCKS_RESULT_DONE if validation passed or
 * SOCKS_RESULT_INVALID if it failed.
 */
static socks_result_t
process_socks4_request(const socks_request_t *req, int is_socks4a,
                       int log_sockstype, int safe_socks)
{
  if (is_socks4a && !addressmap_have_mapping(req->address, 0)) {
    log_unsafe_socks_warning(4, req->address, req->port, safe_socks);

    if (safe_socks)
      return SOCKS_RESULT_INVALID;
  }

  if (req->command != SOCKS_COMMAND_CONNECT &&
      req->command != SOCKS_COMMAND_RESOLVE) {
    /* not a connect or resolve? we don't support it. (No resolve_ptr with
     * socks4.) */
    log_warn(LD_APP, "socks4: command %d not recognized. Rejecting.",
             req->command);
    return SOCKS_RESULT_INVALID;
  }

  if (is_socks4a) {
    if (log_sockstype)
      log_notice(LD_APP,
                 "Your application (using socks4a to port %d) instructed "
                 "Tor to take care of the DNS resolution itself if "
                 "necessary. This is good.", req->port);
  }

  if (!string_is_valid_dest(req->address)) {
    log_warn(LD_PROTOCOL,
             "Your application (using socks4 to port %d) gave Tor "
             "a malformed hostname: %s. Rejecting the connection.",
             req->port, escaped_safe_str_client(req->address));
     return SOCKS_RESULT_INVALID;
  }

  return SOCKS_RESULT_DONE;
}

/** Parse a single SOCKS5 version identifier/method selection message
 * from buffer <b>raw_data</b> (of length <b>datalen</b>). Update
 * relevant fields of <b>req</b> (if any). Set <b>*have_user_pass</b> to
 * true if username/password method is found. Set <b>*have_no_auth</b>
 * if no-auth method is found. Set <b>*drain_out</b> to number of bytes
 * we parsed so far.
 *
 * Return SOCKS_RESULT_DONE if parsing succeeded, SOCKS_RESULT_INVALID if
 * parsing failed because of invalid input or SOCKS_RESULT_TRUNCATED if it
 * failed due to incomplete (truncated) input.
 */
static socks_result_t
parse_socks5_methods_request(const uint8_t *raw_data, socks_request_t *req,
                             size_t datalen, int *have_user_pass,
                             int *have_no_auth, size_t *drain_out)
{
  socks_result_t res = SOCKS_RESULT_DONE;
  socks5_client_version_t *trunnel_req;

  ssize_t parsed = socks5_client_version_parse(&trunnel_req, raw_data,
                                               datalen);

  (void)req;

  tor_assert(have_no_auth);
  tor_assert(have_user_pass);
  tor_assert(drain_out);

  *drain_out = 0;

  if (parsed == -1) {
    log_warn(LD_APP, "socks5: parsing failed - invalid version "
                     "id/method selection message.");
    res = SOCKS_RESULT_INVALID;
    goto end;
  } else if (parsed == -2) {
    res = SOCKS_RESULT_TRUNCATED;
    if (datalen > MAX_SOCKS_MESSAGE_LEN) {
      log_warn(LD_APP, "socks5: parsing failed - invalid version "
                       "id/method selection message.");
      res = SOCKS_RESULT_INVALID;
    }
    goto end;
  }

  tor_assert(parsed >= 0);
  *drain_out = (size_t)parsed;

  size_t n_methods = (size_t)socks5_client_version_get_n_methods(trunnel_req);
  if (n_methods == 0) {
    res = SOCKS_RESULT_INVALID;
    goto end;
  }

  *have_no_auth = 0;
  *have_user_pass = 0;

  for (size_t i = 0; i < n_methods; i++) {
    uint8_t method = socks5_client_version_get_methods(trunnel_req,
                                                       i);

    if (method == SOCKS_USER_PASS) {
      *have_user_pass = 1;
    } else if (method == SOCKS_NO_AUTH) {
      *have_no_auth = 1;
    }
  }

  end:
  socks5_client_version_free(trunnel_req);

  return res;
}

/**
 * Validate and respond to version identifier/method selection message
 * we parsed in parse_socks5_methods_request (corresponding to <b>req</b>
 * and having user/pass method if <b>have_user_pass</b> is true, no-auth
 * method if <b>have_no_auth</b> is true). Set <b>req->reply</b> to
 * an appropriate response (in SOCKS5 wire format).
 *
 * On success, return SOCKS_RESULT_DONE. On failure, return
 * SOCKS_RESULT_INVALID.
 */
static socks_result_t
process_socks5_methods_request(socks_request_t *req, int have_user_pass,
                               int have_no_auth)
{
  socks_result_t res = SOCKS_RESULT_DONE;
  socks5_server_method_t *trunnel_resp = socks5_server_method_new();
  tor_assert(trunnel_resp);

  socks5_server_method_set_version(trunnel_resp, SOCKS_VER_5);

  if (have_user_pass && !(have_no_auth && req->socks_prefer_no_auth)) {
    req->auth_type = SOCKS_USER_PASS;
    socks5_server_method_set_method(trunnel_resp, SOCKS_USER_PASS);

    req->socks_version = SOCKS_VER_5;
    // FIXME: come up with better way to remember
    // that we negotiated auth

    log_debug(LD_APP,"socks5: accepted method 2 (username/password)");
  } else if (have_no_auth) {
    req->auth_type = SOCKS_NO_AUTH;
    socks5_server_method_set_method(trunnel_resp, SOCKS_NO_AUTH);

    req->socks_version = SOCKS_VER_5;

    log_debug(LD_APP,"socks5: accepted method 0 (no authentication)");
  } else {
    log_warn(LD_APP,
             "socks5: offered methods don't include 'no auth' or "
             "username/password. Rejecting.");
    socks5_server_method_set_method(trunnel_resp, 0xFF); // reject all
    res = SOCKS_RESULT_INVALID;
  }

  const char *errmsg = socks5_server_method_check(trunnel_resp);
  if (errmsg) {
    log_warn(LD_APP, "socks5: method selection validation failed: %s",
             errmsg);
    res = SOCKS_RESULT_INVALID;
  } else {
    ssize_t encoded =
    socks5_server_method_encode(req->reply, sizeof(req->reply),
                                trunnel_resp);

    if (encoded < 0) {
      log_warn(LD_APP, "socks5: method selection encoding failed");
      res = SOCKS_RESULT_INVALID;
    } else {
      req->replylen = (size_t)encoded;
    }
  }

  socks5_server_method_free(trunnel_resp);
  return res;
}

/**
 * Parse SOCKS5/RFC1929 username/password request from buffer
 * <b>raw_data</b> of length <b>datalen</b> and update relevant
 * fields of <b>req</b>. Set <b>*drain_out</b> to number of bytes
 * we parsed so far.
 *
 * Return SOCKS_RESULT_DONE if parsing succeeded, SOCKS_RESULT_INVALID if
 * parsing failed because of invalid input or SOCKS_RESULT_TRUNCATED if it
 * failed due to incomplete (truncated) input.
 */
static socks_result_t
parse_socks5_userpass_auth(const uint8_t *raw_data, socks_request_t *req,
                           size_t datalen, size_t *drain_out)
{
  socks_result_t res = SOCKS_RESULT_DONE;
  socks5_client_userpass_auth_t *trunnel_req = NULL;
  ssize_t parsed = socks5_client_userpass_auth_parse(&trunnel_req, raw_data,
                                                     datalen);
  tor_assert(drain_out);
  *drain_out = 0;

  if (parsed == -1) {
    log_warn(LD_APP, "socks5: parsing failed - invalid user/pass "
                     "authentication message.");
    res = SOCKS_RESULT_INVALID;
    goto end;
  } else if (parsed == -2) {
    res = SOCKS_RESULT_TRUNCATED;
    goto end;
  }

  tor_assert(parsed >= 0);
  *drain_out = (size_t)parsed;

  uint8_t usernamelen =
   socks5_client_userpass_auth_get_username_len(trunnel_req);
  uint8_t passwordlen =
   socks5_client_userpass_auth_get_passwd_len(trunnel_req);
  const char *username =
   socks5_client_userpass_auth_getconstarray_username(trunnel_req);
  const char *password =
   socks5_client_userpass_auth_getconstarray_passwd(trunnel_req);

  if (usernamelen && username) {
    tor_free(req->username);
    req->username = tor_memdup_nulterm(username, usernamelen);
    req->usernamelen = usernamelen;
  }

  if (passwordlen && password) {
    tor_free(req->password);
    req->password = tor_memdup_nulterm(password, passwordlen);
    req->passwordlen = passwordlen;
  }

  /**
   * Yes, we allow username and/or password to be empty. Yes, that does
   * violate RFC 1929. However, some client software can send a username/
   * password message with these fields being empty and we want to allow them
   * to be used with Tor.
   */
  req->got_auth = 1;

  end:
  socks5_client_userpass_auth_free(trunnel_req);
  return res;
}

/**
 * Validate and respond to SOCKS5 username/password request we
 * parsed in parse_socks5_userpass_auth (corresponding to <b>req</b>.
 * Set <b>req->reply</b> to appropriate response. Return
 * SOCKS_RESULT_DONE on success or SOCKS_RESULT_INVALID on failure.
 */
static socks_result_t
process_socks5_userpass_auth(socks_request_t *req)
{
  socks_result_t res = SOCKS_RESULT_DONE;
  socks5_server_userpass_auth_t *trunnel_resp =
    socks5_server_userpass_auth_new();
  tor_assert(trunnel_resp);

  if (req->socks_version != SOCKS_VER_5) {
    res = SOCKS_RESULT_INVALID;
    goto end;
  }

  if (req->auth_type != SOCKS_USER_PASS &&
      req->auth_type != SOCKS_NO_AUTH) {
    res = SOCKS_RESULT_INVALID;
    goto end;
  }

  socks5_server_userpass_auth_set_version(trunnel_resp, SOCKS_AUTH);
  socks5_server_userpass_auth_set_status(trunnel_resp, 0); // auth OK

  const char *errmsg = socks5_server_userpass_auth_check(trunnel_resp);
  if (errmsg) {
    log_warn(LD_APP, "socks5: server userpass auth validation failed: %s",
             errmsg);
    res = SOCKS_RESULT_INVALID;
    goto end;
  }

  ssize_t encoded = socks5_server_userpass_auth_encode(req->reply,
                                                       sizeof(req->reply),
                                                       trunnel_resp);

  if (encoded < 0) {
    log_warn(LD_APP, "socks5: server userpass auth encoding failed");
    res = SOCKS_RESULT_INVALID;
    goto end;
  }

  req->replylen = (size_t)encoded;

  end:
  socks5_server_userpass_auth_free(trunnel_resp);
  return res;
}

/**
 * Parse a single SOCKS5 client request (RFC 1928 section 4) from buffer
 * <b>raw_data</b> of length <b>datalen</b> and update relevant field of
 * <b>req</b>. Set <b>*drain_out</b> to number of bytes we parsed so far.
 *
 * Return SOCKS_RESULT_DONE if parsing succeeded, SOCKS_RESULT_INVALID if
 * parsing failed because of invalid input or SOCKS_RESULT_TRUNCATED if it
 * failed due to incomplete (truncated) input.
 */
static socks_result_t
parse_socks5_client_request(const uint8_t *raw_data, socks_request_t *req,
                            size_t datalen, size_t *drain_out)
{
  socks_result_t res = SOCKS_RESULT_DONE;
  tor_addr_t destaddr;
  socks5_client_request_t *trunnel_req = NULL;
  ssize_t parsed =
   socks5_client_request_parse(&trunnel_req, raw_data, datalen);
  if (parsed == -1) {
    log_warn(LD_APP, "socks5: parsing failed - invalid client request");
    res = SOCKS_RESULT_INVALID;
    socks_request_set_socks5_error(req, SOCKS5_GENERAL_ERROR);
    goto end;
  } else if (parsed == -2) {
    res = SOCKS_RESULT_TRUNCATED;
    goto end;
  }

  tor_assert(parsed >= 0);
  *drain_out = (size_t)parsed;

  if (socks5_client_request_get_version(trunnel_req) != 5) {
    res = SOCKS_RESULT_INVALID;
    socks_request_set_socks5_error(req, SOCKS5_GENERAL_ERROR);
    goto end;
  }

  req->command = socks5_client_request_get_command(trunnel_req);

  req->port = socks5_client_request_get_dest_port(trunnel_req);

  uint8_t atype = socks5_client_request_get_atype(trunnel_req);
  req->socks5_atyp = atype;

  switch (atype) {
    case 1: {
      uint32_t ipv4 = socks5_client_request_get_dest_addr_ipv4(trunnel_req);
      tor_addr_from_ipv4h(&destaddr, ipv4);

      tor_addr_to_str(req->address, &destaddr, sizeof(req->address), 1);
    } break;
    case 3: {
      const struct domainname_st *dns_name =
        socks5_client_request_getconst_dest_addr_domainname(trunnel_req);

      const char *hostname = domainname_getconstarray_name(dns_name);

      strlcpy(req->address, hostname, sizeof(req->address));
    } break;
    case 4: {
      const uint8_t *ipv6 =
          socks5_client_request_getarray_dest_addr_ipv6(trunnel_req);
      tor_addr_from_ipv6_bytes(&destaddr, ipv6);

      tor_addr_to_str(req->address, &destaddr, sizeof(req->address), 1);
    } break;
    default: {
      socks_request_set_socks5_error(req, SOCKS5_ADDRESS_TYPE_NOT_SUPPORTED);
      res = -1;
    } break;
  }

  end:
  socks5_client_request_free(trunnel_req);
  return res;
}

/**
 * Validate and respond to SOCKS5 request we parsed in
 * parse_socks5_client_request (corresponding to <b>req</b>.
 * Write appropriate response to <b>req->reply</b> (in
 * SOCKS5 wire format). If <b>log_sockstype</b> is true, log a
 * notice about possible DNS leaks on local system. If
 * <b>safe_socks</b> is true, disallow insecure usage of SOCKS
 * protocol. Return SOCKS_RESULT_DONE on success or
 * SOCKS_RESULT_INVALID on failure.
 */
static socks_result_t
process_socks5_client_request(socks_request_t *req,
                              int log_sockstype,
                              int safe_socks)
{
  socks_result_t res = SOCKS_RESULT_DONE;
  tor_addr_t tmpaddr;

  if (req->command != SOCKS_COMMAND_CONNECT &&
      req->command != SOCKS_COMMAND_RESOLVE &&
      req->command != SOCKS_COMMAND_RESOLVE_PTR) {
    socks_request_set_socks5_error(req,SOCKS5_COMMAND_NOT_SUPPORTED);
    res = SOCKS_RESULT_INVALID;
    goto end;
  }

  if (req->command == SOCKS_COMMAND_RESOLVE_PTR &&
      tor_addr_parse(&tmpaddr, req->address) < 0) {
    socks_request_set_socks5_error(req, SOCKS5_ADDRESS_TYPE_NOT_SUPPORTED);
    log_warn(LD_APP, "socks5 received RESOLVE_PTR command with "
                     "a malformed address. Rejecting.");

    res = SOCKS_RESULT_INVALID;
    goto end;
  }

  if (!string_is_valid_dest(req->address)) {
    socks_request_set_socks5_error(req, SOCKS5_GENERAL_ERROR);

    log_warn(LD_PROTOCOL,
             "Your application (using socks5 to port %d) gave Tor "
             "a malformed hostname: %s. Rejecting the connection.",
             req->port, escaped_safe_str_client(req->address));

    res = SOCKS_RESULT_INVALID;
    goto end;
  }

  if (req->socks5_atyp == 1 || req->socks5_atyp == 4) {
    if (req->command != SOCKS_COMMAND_RESOLVE_PTR &&
        !addressmap_have_mapping(req->address,0)) {
      log_unsafe_socks_warning(5, req->address, req->port, safe_socks);
      if (safe_socks) {
        socks_request_set_socks5_error(req, SOCKS5_NOT_ALLOWED);
        res = SOCKS_RESULT_INVALID;
        goto end;
      }
    }
  }

  if (log_sockstype)
    log_notice(LD_APP,
              "Your application (using socks5 to port %d) instructed "
              "Tor to take care of the DNS resolution itself if "
              "necessary. This is good.", req->port);

  end:
  return res;
}

/**
 * Handle (parse, validate, process, respond) a single SOCKS
 * message in buffer <b>raw_data</b> of length <b>datalen</b>.
 * Update relevant fields of <b>req</b>. If <b>log_sockstype</b>
 * is true, log a warning about possible DNS leaks on local
 * system. If <b>safe_socks</b> is true, disallow insecure
 * usage of SOCKS protocol. Set <b>*drain_out</b> to number
 * of bytes in <b>raw_data</b> that we processed so far and
 * that can be safely drained from buffer.
 *
 * Return:
 *  - SOCKS_RESULT_DONE if succeeded and not expecting further
 *    messages from client.
 *  - SOCKS_RESULT_INVALID if any of the steps failed due to
 *    request being invalid or unexpected given current state.
 *  - SOCKS_RESULT_TRUNCATED if we do not found an expected
 *    SOCKS message in its entirety (more stuff has to arrive
 *    from client).
 *  - SOCKS_RESULT_MORE_EXPECTED if we handled current message
 *    successfully, but we expect more messages from the
 *    client.
 */
static socks_result_t
handle_socks_message(const uint8_t *raw_data, size_t datalen,
                     socks_request_t *req, int log_sockstype,
                     int safe_socks, size_t *drain_out)
{
  socks_result_t res = SOCKS_RESULT_DONE;

  uint8_t socks_version = raw_data[0];

  if (socks_version == SOCKS_AUTH)
    socks_version = SOCKS_VER_5; // SOCKS5 username/pass subnegotiation

  if (socks_version == SOCKS_VER_4) {
    if (datalen < SOCKS4_NETWORK_LEN) {
      res = 0;
      goto end;
    }

    int is_socks4a = 0;
    res = parse_socks4_request((const uint8_t *)raw_data, req, datalen,
                               &is_socks4a, drain_out);

    if (res != SOCKS_RESULT_DONE) {
      goto end;
    }

    res = process_socks4_request(req, is_socks4a,log_sockstype,
                                 safe_socks);

    if (res != SOCKS_RESULT_DONE) {
      goto end;
    }

    goto end;
  } else if (socks_version == SOCKS_VER_5) {
    if (datalen < 2) { /* version and another byte */
      res = 0;
      goto end;
    }
    /* RFC1929 SOCKS5 username/password subnegotiation. */
    if (!req->got_auth && (raw_data[0] == 1 ||
        req->auth_type == SOCKS_USER_PASS)) {
      res = parse_socks5_userpass_auth(raw_data, req, datalen,
                                       drain_out);

      if (res != SOCKS_RESULT_DONE) {
        goto end;
      }

      res = process_socks5_userpass_auth(req);
      if (res != SOCKS_RESULT_DONE) {
        goto end;
      }

      res = SOCKS_RESULT_MORE_EXPECTED;
      goto end;
    } else if (req->socks_version != SOCKS_VER_5) {
      int have_user_pass=0, have_no_auth=0;
      res = parse_socks5_methods_request(raw_data, req, datalen,
                                         &have_user_pass,
                                         &have_no_auth,
                                         drain_out);

      if (res != SOCKS_RESULT_DONE) {
        goto end;
      }

      res = process_socks5_methods_request(req, have_user_pass,
                                           have_no_auth);

      if (res != SOCKS_RESULT_DONE) {
        goto end;
      }

      res = SOCKS_RESULT_MORE_EXPECTED;
      goto end;
    } else {
      res = parse_socks5_client_request(raw_data, req,
                                        datalen, drain_out);
      if (BUG(res == SOCKS_RESULT_INVALID && req->replylen == 0)) {
        socks_request_set_socks5_error(req, SOCKS5_GENERAL_ERROR);
      }
      if (res != SOCKS_RESULT_DONE) {
        goto end;
      }

      res = process_socks5_client_request(req, log_sockstype,
                                          safe_socks);

      if (res != SOCKS_RESULT_DONE) {
        goto end;
      }
    }
  } else {
    *drain_out = datalen;
    res = SOCKS_RESULT_INVALID;
  }

  end:
  return res;
}

/** There is a (possibly incomplete) socks handshake on <b>buf</b>, of one
 * of the forms
 *  - socks4: "socksheader username\\0"
 *  - socks4a: "socksheader username\\0 destaddr\\0"
 *  - socks5 phase one: "version #methods methods"
 *  - socks5 phase two: "version command 0 addresstype..."
 * If it's a complete and valid handshake, and destaddr fits in
 *   MAX_SOCKS_ADDR_LEN bytes, then pull the handshake off the buf,
 *   assign to <b>req</b>, and return 1.
 *
 * If it's invalid or too big, return -1.
 *
 * Else it's not all there yet, leave buf alone and return 0.
 *
 * If you want to specify the socks reply, write it into <b>req->reply</b>
 *   and set <b>req->replylen</b>, else leave <b>req->replylen</b> alone.
 *
 * If <b>log_sockstype</b> is non-zero, then do a notice-level log of whether
 * the connection is possibly leaking DNS requests locally or not.
 *
 * If <b>safe_socks</b> is true, then reject unsafe socks protocols.
 *
 * If returning 0 or -1, <b>req->address</b> and <b>req->port</b> are
 * undefined.
 */
int
fetch_from_buf_socks(buf_t *buf, socks_request_t *req,
                     int log_sockstype, int safe_socks)
{
  int res = 0;
  size_t datalen = buf_datalen(buf);
  size_t n_drain;
  const char *head = NULL;
  socks_result_t socks_res;
  size_t n_pullup;

  if (buf_datalen(buf) < 2) { /* version and another byte */
    res = 0;
    goto end;
  }

  do {
    n_drain = 0;
    n_pullup = MIN(MAX_SOCKS_MESSAGE_LEN, buf_datalen(buf));
    buf_pullup(buf, n_pullup, &head, &datalen);
    tor_assert(head && datalen >= 2);

    socks_res = parse_socks(head, datalen, req, log_sockstype,
                            safe_socks, &n_drain);

    if (socks_res == SOCKS_RESULT_INVALID)
      buf_clear(buf);
    else if (socks_res != SOCKS_RESULT_TRUNCATED && n_drain > 0)
      buf_drain(buf, n_drain);

    switch (socks_res) {
      case SOCKS_RESULT_INVALID:
        res = -1;
        break;
      case SOCKS_RESULT_DONE:
        res = 1;
        break;
      case SOCKS_RESULT_TRUNCATED:
        if (datalen == n_pullup)
          return 0;
        FALLTHROUGH;
      case SOCKS_RESULT_MORE_EXPECTED:
        res = 0;
        break;
    }
  } while (res == 0 && head && buf_datalen(buf) >= 2);

  end:
  return res;
}

/** Create a SOCKS5 reply message with <b>reason</b> in its REP field and
 * have Tor send it as error response to <b>req</b>.
 */
static void
socks_request_set_socks5_error(socks_request_t *req,
                  socks5_reply_status_t reason)
{
  socks5_server_reply_t *trunnel_resp = socks5_server_reply_new();
  tor_assert(trunnel_resp);

  socks5_server_reply_set_version(trunnel_resp, SOCKS_VER_5);
  socks5_server_reply_set_reply(trunnel_resp, reason);
  socks5_server_reply_set_atype(trunnel_resp, 0x01);

  const char *errmsg = socks5_server_reply_check(trunnel_resp);
  if (errmsg) {
    log_warn(LD_APP, "socks5: reply validation failed: %s",
             errmsg);
    goto end;
  }

  ssize_t encoded = socks5_server_reply_encode(req->reply,
                                               sizeof(req->reply),
                                               trunnel_resp);
  if (encoded < 0) {
    log_warn(LD_APP, "socks5: reply encoding failed: %d",
             (int)encoded);
  } else {
    req->replylen = (size_t)encoded;
  }

  end:
  socks5_server_reply_free(trunnel_resp);
}

static const char SOCKS_PROXY_IS_NOT_AN_HTTP_PROXY_MSG[] =
  "HTTP/1.0 501 Tor is not an HTTP Proxy\r\n"
  "Content-Type: text/html; charset=iso-8859-1\r\n\r\n"
  "<html>\n"
  "<head>\n"
  "<title>This is a SOCKS Proxy, Not An HTTP Proxy</title>\n"
  "</head>\n"
  "<body>\n"
  "<h1>This is a SOCKs proxy, not an HTTP proxy.</h1>\n"
  "<p>\n"
  "It appears you have configured your web browser to use this Tor port as\n"
  "an HTTP proxy.\n"
  "</p><p>\n"
  "This is not correct: This port is configured as a SOCKS proxy, not\n"
  "an HTTP proxy. If you need an HTTP proxy tunnel, use the HTTPTunnelPort\n"
  "configuration option in place of, or in addition to, SOCKSPort.\n"
  "Please configure your client accordingly.\n"
  "</p>\n"
  "<p>\n"
  "See <a href=\"https://www.torproject.org/documentation.html\">"
  "https://www.torproject.org/documentation.html</a> for more "
  "information.\n"
  "</p>\n"
  "</body>\n"
  "</html>\n";

/** Implementation helper to implement fetch_from_*_socks.  Instead of looking
 * at a buffer's contents, we look at the <b>datalen</b> bytes of data in
 * <b>data</b>. Instead of removing data from the buffer, we set
 * <b>drain_out</b> to the amount of data that should be removed (or -1 if the
 * buffer should be cleared).  Instead of pulling more data into the first
 * chunk of the buffer, we set *<b>want_length_out</b> to the number of bytes
 * we'd like to see in the input buffer, if they're available. */
static int
parse_socks(const char *data, size_t datalen, socks_request_t *req,
            int log_sockstype, int safe_socks, size_t *drain_out)
{
  uint8_t first_octet;

  if (datalen < 2) {
    /* We always need at least 2 bytes. */
    return 0;
  }

  first_octet = get_uint8(data);

  if (first_octet == SOCKS_VER_5 || first_octet == SOCKS_VER_4 ||
      first_octet == SOCKS_AUTH) { // XXX: RFC 1929
    return handle_socks_message((const uint8_t *)data, datalen, req,
                                log_sockstype, safe_socks, drain_out);
  }

  switch (first_octet) { /* which version of socks? */
    case 'G': /* get */
    case 'H': /* head */
    case 'P': /* put/post */
    case 'C': /* connect */
      strlcpy((char*)req->reply, SOCKS_PROXY_IS_NOT_AN_HTTP_PROXY_MSG,
              MAX_SOCKS_REPLY_LEN);
      req->replylen = strlen((char*)req->reply)+1;
      FALLTHROUGH;
    default: /* version is not socks4 or socks5 */
      log_warn(LD_APP,
               "Socks version %d not recognized. (This port is not an "
               "HTTP proxy; did you want to use HTTPTunnelPort?)",
               *(data));
      {
        /* Tell the controller the first 8 bytes. */
        char *tmp = tor_strndup(data, datalen < 8 ? datalen : 8);
        control_event_client_status(LOG_WARN,
                                    "SOCKS_UNKNOWN_PROTOCOL DATA=\"%s\"",
                                    escaped(tmp));
        tor_free(tmp);
      }
      return -1;
  }

  tor_assert_unreached();
  return -1;
}

/** Inspect a reply from SOCKS server stored in <b>buf</b> according
 * to <b>state</b>, removing the protocol data upon success. Return 0 on
 * incomplete response, 1 on success and -1 on error, in which case
 * <b>reason</b> is set to a descriptive message (free() when finished
 * with it).
 *
 * As a special case, 2 is returned when user/pass is required
 * during SOCKS5 handshake and user/pass is configured.
 */
int
fetch_from_buf_socks_client(buf_t *buf, int state, char **reason)
{
  ssize_t drain = 0;
  int r;
  const char *head = NULL;
  size_t datalen = 0;

  if (buf_datalen(buf) < 2)
    return 0;

  buf_pullup(buf, MAX_SOCKS_MESSAGE_LEN, &head, &datalen);
  tor_assert(head && datalen >= 2);

  r = parse_socks_client((uint8_t*)head, datalen,
                         state, reason, &drain);
  if (drain > 0)
    buf_drain(buf, drain);
  else if (drain < 0)
    buf_clear(buf);

  return r;
}

/** Implementation logic for fetch_from_*_socks_client. */
static int
parse_socks_client(const uint8_t *data, size_t datalen,
                   int state, char **reason,
                   ssize_t *drain_out)
{
  unsigned int addrlen;
  *drain_out = 0;
  if (datalen < 2)
    return 0;

  switch (state) {
    case PROXY_SOCKS4_WANT_CONNECT_OK:
      /* Wait for the complete response */
      if (datalen < 8)
        return 0;

      if (data[1] != 0x5a) {
        *reason = tor_strdup(socks4_response_code_to_string(data[1]));
        return -1;
      }

      /* Success */
      *drain_out = 8;
      return 1;

    case PROXY_SOCKS5_WANT_AUTH_METHOD_NONE:
      /* we don't have any credentials */
      if (data[1] != 0x00) {
        *reason = tor_strdup("server doesn't support any of our "
                             "available authentication methods");
        return -1;
      }

      log_info(LD_NET, "SOCKS 5 client: continuing without authentication");
      *drain_out = -1;
      return 1;

    case PROXY_SOCKS5_WANT_AUTH_METHOD_RFC1929:
      /* we have a username and password. return 1 if we can proceed without
       * providing authentication, or 2 otherwise. */
      switch (data[1]) {
        case 0x00:
          log_info(LD_NET, "SOCKS 5 client: we have auth details but server "
                            "doesn't require authentication.");
          *drain_out = -1;
          return 1;
        case 0x02:
          log_info(LD_NET, "SOCKS 5 client: need authentication.");
          *drain_out = -1;
          return 2;
        default:
          /* This wasn't supposed to be exhaustive; there are other
           * authentication methods too. */
          ;
      }

      *reason = tor_strdup("server doesn't support any of our available "
                           "authentication methods");
      return -1;

    case PROXY_SOCKS5_WANT_AUTH_RFC1929_OK:
      /* handle server reply to rfc1929 authentication */
      if (data[1] != 0x00) {
        *reason = tor_strdup("authentication failed");
        return -1;
      }

      log_info(LD_NET, "SOCKS 5 client: authentication successful.");
      *drain_out = -1;
      return 1;

    case PROXY_SOCKS5_WANT_CONNECT_OK:
      /* response is variable length. BND.ADDR, etc, isn't needed
       * (don't bother with buf_pullup()), but make sure to eat all
       * the data used */

      /* wait for address type field to arrive */
      if (datalen < 4)
        return 0;

      switch (data[3]) {
        case 0x01: /* ip4 */
          addrlen = 4;
          break;
        case 0x04: /* ip6 */
          addrlen = 16;
          break;
        case 0x03: /* fqdn (can this happen here?) */
          if (datalen < 5)
            return 0;
          addrlen = 1 + data[4];
          break;
        default:
          *reason = tor_strdup("invalid response to connect request");
          return -1;
      }

      /* wait for address and port */
      if (datalen < 6 + addrlen)
        return 0;

      if (data[1] != 0x00) {
        *reason = tor_strdup(socks5_response_code_to_string(data[1]));
        return -1;
      }

      *drain_out = 6 + addrlen;
      return 1;
  }

  /* LCOV_EXCL_START */
  /* shouldn't get here if the input state is one we know about... */
  tor_assert(0);

  return -1;
  /* LCOV_EXCL_STOP */
}

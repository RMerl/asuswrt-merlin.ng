/* Copyright (c) 2004-2006, Roger Dingledine, Nick Mathewson.
 * Copyright (c) 2019-2020, The Tor Project, Inc. */
/* See LICENSE for licensing information */

/**
 * \file control_hs.c
 *
 * \brief Implement commands for Tor's control-socket interface that are
 *        related to onion services.
 **/

#include "core/or/or.h"
#include "feature/control/control_cmd.h"
#include "feature/control/control_hs.h"
#include "feature/control/control_proto.h"
#include "feature/hs/hs_client.h"
#include "lib/encoding/confline.h"

#include "feature/control/control_cmd_args_st.h"

/** Parse the 'KeyType ":" PrivateKey' from <b>client_privkey_str</b> and store
 *  it into <b>privkey</b>. Use <b>conn</b> to output any errors if needed.
 *
 *  Return 0 if all went well, -1 otherwise. */
static int
parse_private_key_from_control_port(const char *client_privkey_str,
                                    curve25519_secret_key_t *privkey,
                                    control_connection_t *conn)
{
  int retval = -1;
  smartlist_t *key_args = smartlist_new();

  tor_assert(privkey);

  smartlist_split_string(key_args, client_privkey_str, ":",
                         SPLIT_IGNORE_BLANK, 0);
  if (smartlist_len(key_args) != 2) {
    control_printf_endreply(conn, 512, "Invalid key type/blob");
    goto err;
  }

  const char *key_type = smartlist_get(key_args, 0);
  const char *key_blob = smartlist_get(key_args, 1);

  if (strcasecmp(key_type, "x25519")) {
    control_printf_endreply(conn, 552,
                            "Unrecognized key type \"%s\"", key_type);
    goto err;
  }

  if (base64_decode((char*)privkey->secret_key, sizeof(privkey->secret_key),
                    key_blob,
                    strlen(key_blob)) != sizeof(privkey->secret_key)) {
    control_printf_endreply(conn, 512, "Failed to decode x25519 private key");
    goto err;
  }

  if (fast_mem_is_zero((const char*)privkey->secret_key,
                       sizeof(privkey->secret_key))) {
    control_printf_endreply(conn, 553,
                            "Invalid private key \"%s\"", key_blob);
    goto err;
  }

  retval = 0;

 err:
  SMARTLIST_FOREACH(key_args, char *, c, tor_free(c));
  smartlist_free(key_args);
  return retval;
}

/** Syntax details for ONION_CLIENT_AUTH_ADD */
const control_cmd_syntax_t onion_client_auth_add_syntax = {
  .max_args = 2,
  .accept_keywords = true,
};

/** Called when we get an ONION_CLIENT_AUTH_ADD command; parse the body, and
 *  register the new client-side client auth credentials:
 *  "ONION_CLIENT_AUTH_ADD" SP HSAddress
 *                          SP KeyType ":" PrivateKeyBlob
 *                          [SP "Type=" TYPE] CRLF
 */
int
handle_control_onion_client_auth_add(control_connection_t *conn,
                                     const control_cmd_args_t *args)
{
  int retval = -1;
  smartlist_t *flags = smartlist_new();
  hs_client_service_authorization_t *creds = NULL;

  tor_assert(args);

  int argc = smartlist_len(args->args);
  /* We need at least 'HSAddress' and 'PrivateKeyBlob' */
  if (argc < 2) {
    control_printf_endreply(conn, 512,
                            "Incomplete ONION_CLIENT_AUTH_ADD command");
    goto err;
  }

  creds = tor_malloc_zero(sizeof(hs_client_service_authorization_t));

  const char *hsaddress = smartlist_get(args->args, 0);
  if (!hs_address_is_valid(hsaddress)) {
    control_printf_endreply(conn, 512, "Invalid v3 address \"%s\"",hsaddress);
    goto err;
  }
  strlcpy(creds->onion_address, hsaddress, sizeof(creds->onion_address));

  /* Parse the client private key */
  const char *client_privkey = smartlist_get(args->args, 1);
  if (parse_private_key_from_control_port(client_privkey,
                                          &creds->enc_seckey, conn) < 0) {
    goto err;
  }

  /* Now let's parse the remaining arguments (variable size) */
  for (const config_line_t *line = args->kwargs; line; line = line->next) {
    if (!strcasecmpstart(line->key, "Flags")) {
      smartlist_split_string(flags, line->value, ",", SPLIT_IGNORE_BLANK, 0);
      if (smartlist_len(flags) < 1) {
        control_write_endreply(conn, 512, "Invalid 'Flags' argument");
        goto err;
      }
      SMARTLIST_FOREACH_BEGIN(flags, const char *, flag) {
        if (!strcasecmp(flag, "Permanent")) {
          creds->flags |= CLIENT_AUTH_FLAG_IS_PERMANENT;
        } else {
          control_printf_endreply(conn, 512, "Invalid 'Flags' argument: %s",
                                  escaped(flag));
          goto err;
        }
      } SMARTLIST_FOREACH_END(flag);
    }
    if (!strcasecmp(line->key, "ClientName")) {
      if (strlen(line->value) > REND_CLIENTNAME_MAX_LEN) {
        control_printf_endreply(conn, 512, "ClientName longer than %d chars",
                                REND_CLIENTNAME_MAX_LEN);
      }
      creds->client_name = tor_strdup(line->value);
    }
  }

  hs_client_register_auth_status_t register_status;
  /* Register the credential (register func takes ownership of cred.) */
  register_status = hs_client_register_auth_credentials(creds);
  switch (register_status) {
  case REGISTER_FAIL_BAD_ADDRESS:
    /* It's a bug because the service addr has already been validated above */
    control_printf_endreply(conn, 512, "Invalid v3 address \"%s\"", hsaddress);
    break;
  case REGISTER_FAIL_PERMANENT_STORAGE:
    control_printf_endreply(conn, 553, "Unable to store creds for \"%s\"",
                            hsaddress);
    break;
  case REGISTER_SUCCESS_ALREADY_EXISTS:
    control_printf_endreply(conn, 251,"Client for onion existed and replaced");
    break;
  case REGISTER_SUCCESS_AND_DECRYPTED:
    control_printf_endreply(conn, 252,"Registered client and decrypted desc");
    break;
  case REGISTER_SUCCESS:
    control_printf_endreply(conn, 250, "OK");
    break;
  default:
    tor_assert_nonfatal_unreached();
  }

  retval = 0;
  goto done;

 err:
  client_service_authorization_free(creds);

 done:
  SMARTLIST_FOREACH(flags, char *, s, tor_free(s));
  smartlist_free(flags);
  return retval;
}

/** Syntax details for ONION_CLIENT_AUTH_REMOVE */
const control_cmd_syntax_t onion_client_auth_remove_syntax = {
  .max_args = 1,
  .accept_keywords = true,
};

/** Called when we get an ONION_CLIENT_AUTH_REMOVE command; parse the body, and
 *  register the new client-side client auth credentials.
 *    "ONION_CLIENT_AUTH_REMOVE" SP HSAddress
 */
int
handle_control_onion_client_auth_remove(control_connection_t *conn,
                                        const control_cmd_args_t *args)
{
  int retval = -1;

  tor_assert(args);

  int argc = smartlist_len(args->args);
  if (argc < 1) {
    control_printf_endreply(conn, 512,
                            "Incomplete ONION_CLIENT_AUTH_REMOVE command");
    goto err;
  }

  const char *hsaddress = smartlist_get(args->args, 0);
  if (!hs_address_is_valid(hsaddress)) {
    control_printf_endreply(conn, 512, "Invalid v3 address \"%s\"",hsaddress);
    goto err;
  }

  hs_client_removal_auth_status_t removal_status;
  removal_status = hs_client_remove_auth_credentials(hsaddress);
  switch (removal_status) {
  case REMOVAL_BAD_ADDRESS:
    /* It's a bug because the service addr has already been validated above */
    control_printf_endreply(conn, 512, "Invalid v3 address \"%s\"",hsaddress);
    break;
  case REMOVAL_SUCCESS_NOT_FOUND:
    control_printf_endreply(conn, 251, "No credentials for \"%s\"",hsaddress);
    break;
  case REMOVAL_SUCCESS:
    control_printf_endreply(conn, 250, "OK");
    break;
  default:
    tor_assert_nonfatal_unreached();
  }

  retval = 0;

 err:
  return retval;
}

/** Helper: Return a newly allocated string with the encoding of client
 *  authorization credentials */
static char *
encode_client_auth_cred_for_control_port(
                                       hs_client_service_authorization_t *cred)
{
  smartlist_t *control_line = smartlist_new();
  char x25519_b64[128];
  char *msg_str = NULL;

  tor_assert(cred);

  if (base64_encode(x25519_b64, sizeof(x25519_b64),
                    (char *)cred->enc_seckey.secret_key,
                    sizeof(cred->enc_seckey.secret_key), 0) < 0) {
    tor_assert_nonfatal_unreached();
    goto err;
  }

  smartlist_add_asprintf(control_line, "CLIENT %s x25519:%s",
                         cred->onion_address, x25519_b64);

  if (cred->flags) { /* flags are also optional */
    if (cred->flags & CLIENT_AUTH_FLAG_IS_PERMANENT) {
      smartlist_add_asprintf(control_line, " Flags=Permanent");
    }
  }

  if (cred->client_name) {
    smartlist_add_asprintf(control_line, " ClientName=%s", cred->client_name);
  }

  /* Join all the components into a single string */
  msg_str = smartlist_join_strings(control_line, "", 0, NULL);

 err:
  SMARTLIST_FOREACH(control_line, char *, cp, tor_free(cp));
  smartlist_free(control_line);

  return msg_str;
}

/** Syntax details for ONION_CLIENT_AUTH_VIEW */
const control_cmd_syntax_t onion_client_auth_view_syntax = {
  .max_args = 1,
  .accept_keywords = true,
};

/** Called when we get an ONION_CLIENT_AUTH_VIEW command; parse the body, and
 *  register the new client-side client auth credentials.
 *        "ONION_CLIENT_AUTH_VIEW" [SP HSAddress] CRLF
 */
int
handle_control_onion_client_auth_view(control_connection_t *conn,
                                      const control_cmd_args_t *args)
{
  int retval = -1;
  const char *hsaddress = NULL;
  /* We are gonna put all the credential strings into a smartlist, and sort it
     before printing, so that we can get a guaranteed order of printing. */
  smartlist_t *creds_str_list = smartlist_new();

  tor_assert(args);

  int argc = smartlist_len(args->args);
  if (argc >= 1) {
    hsaddress = smartlist_get(args->args, 0);
    if (!hs_address_is_valid(hsaddress)) {
      control_printf_endreply(conn, 512, "Invalid v3 address \"%s\"",
                              hsaddress);
      goto err;
    }
  }

  if (hsaddress) {
    control_printf_midreply(conn, 250, "ONION_CLIENT_AUTH_VIEW %s", hsaddress);
  } else {
    control_printf_midreply(conn, 250, "ONION_CLIENT_AUTH_VIEW");
  }

  /* Create an iterator out of the digest256map */
  digest256map_t *client_auths = get_hs_client_auths_map();
  digest256map_iter_t *itr = digest256map_iter_init(client_auths);
  while (!digest256map_iter_done(itr)) {
    const uint8_t *service_pubkey;
    void *valp;
    digest256map_iter_get(itr, &service_pubkey, &valp);
    tor_assert(valp);
    hs_client_service_authorization_t *cred = valp;

    /* If a specific HS address was requested, only print creds for that one */
    if (hsaddress && strcmp(cred->onion_address, hsaddress)) {
      itr = digest256map_iter_next(client_auths, itr);
      continue;
    }

    char *encoding_str = encode_client_auth_cred_for_control_port(cred);
    tor_assert_nonfatal(encoding_str);
    smartlist_add(creds_str_list, encoding_str);

    itr = digest256map_iter_next(client_auths, itr);
  }

  /* We got everything: Now sort the strings and print them */
  smartlist_sort_strings(creds_str_list);
  SMARTLIST_FOREACH_BEGIN(creds_str_list, char *, c) {
    control_printf_midreply(conn, 250, "%s", c);
  } SMARTLIST_FOREACH_END(c);

  send_control_done(conn);

  retval = 0;

 err:
  SMARTLIST_FOREACH(creds_str_list, char *, cp, tor_free(cp));
  smartlist_free(creds_str_list);
  return retval;
}

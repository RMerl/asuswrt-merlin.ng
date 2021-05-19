/* Copyright (c) 2001 Matej Pfajfar.
 * Copyright (c) 2001-2004, Roger Dingledine.
 * Copyright (c) 2004-2006, Roger Dingledine, Nick Mathewson.
 * Copyright (c) 2007-2020, The Tor Project, Inc. */
/* See LICENSE for licensing information */

/**
 * \file rendparse.c
 * \brief Code to parse and validate v2 hidden service descriptors.
 **/

#include "core/or/or.h"
#include "core/or/extendinfo.h"
#include "feature/dirparse/parsecommon.h"
#include "feature/dirparse/sigcommon.h"
#include "feature/rend/rendcommon.h"
#include "feature/rend/rendparse.h"
#include "lib/memarea/memarea.h"

#include "core/or/extend_info_st.h"
#include "feature/rend/rend_authorized_client_st.h"
#include "feature/rend/rend_intro_point_st.h"
#include "feature/rend/rend_service_descriptor_st.h"

/** List of tokens recognized in rendezvous service descriptors */
static token_rule_t desc_token_table[] = {
  T1_START("rendezvous-service-descriptor", R_RENDEZVOUS_SERVICE_DESCRIPTOR,
           EQ(1), NO_OBJ),
  T1("version", R_VERSION, EQ(1), NO_OBJ),
  T1("permanent-key", R_PERMANENT_KEY, NO_ARGS, NEED_KEY_1024),
  T1("secret-id-part", R_SECRET_ID_PART, EQ(1), NO_OBJ),
  T1("publication-time", R_PUBLICATION_TIME, CONCAT_ARGS, NO_OBJ),
  T1("protocol-versions", R_PROTOCOL_VERSIONS, EQ(1), NO_OBJ),
  T01("introduction-points", R_INTRODUCTION_POINTS, NO_ARGS, NEED_OBJ),
  T1_END("signature", R_SIGNATURE, NO_ARGS, NEED_OBJ),
  END_OF_TABLE
};

/** List of tokens recognized in the (encrypted) list of introduction points of
 * rendezvous service descriptors */
static token_rule_t ipo_token_table[] = {
  T1_START("introduction-point", R_IPO_IDENTIFIER, EQ(1), NO_OBJ),
  T1("ip-address", R_IPO_IP_ADDRESS, EQ(1), NO_OBJ),
  T1("onion-port", R_IPO_ONION_PORT, EQ(1), NO_OBJ),
  T1("onion-key", R_IPO_ONION_KEY, NO_ARGS, NEED_KEY_1024),
  T1("service-key", R_IPO_SERVICE_KEY, NO_ARGS, NEED_KEY_1024),
  END_OF_TABLE
};

/** List of tokens recognized in the (possibly encrypted) list of introduction
 * points of rendezvous service descriptors */
static token_rule_t client_keys_token_table[] = {
  T1_START("client-name", C_CLIENT_NAME, CONCAT_ARGS, NO_OBJ),
  T1("descriptor-cookie", C_DESCRIPTOR_COOKIE, EQ(1), NO_OBJ),
  T01("client-key", C_CLIENT_KEY, NO_ARGS, NEED_SKEY_1024),
  END_OF_TABLE
};

/** Parse and validate the ASCII-encoded v2 descriptor in <b>desc</b>,
 * write the parsed descriptor to the newly allocated *<b>parsed_out</b>, the
 * binary descriptor ID of length DIGEST_LEN to <b>desc_id_out</b>, the
 * encrypted introduction points to the newly allocated
 * *<b>intro_points_encrypted_out</b>, their encrypted size to
 * *<b>intro_points_encrypted_size_out</b>, the size of the encoded descriptor
 * to *<b>encoded_size_out</b>, and a pointer to the possibly next
 * descriptor to *<b>next_out</b>; return 0 for success (including validation)
 * and -1 for failure.
 *
 * If <b>as_hsdir</b> is 1, we're parsing this as an HSDir, and we should
 * be strict about time formats.
 */
int
rend_parse_v2_service_descriptor(rend_service_descriptor_t **parsed_out,
                                 char *desc_id_out,
                                 char **intro_points_encrypted_out,
                                 size_t *intro_points_encrypted_size_out,
                                 size_t *encoded_size_out,
                                 const char **next_out, const char *desc,
                                 int as_hsdir)
{
  rend_service_descriptor_t *result =
                            tor_malloc_zero(sizeof(rend_service_descriptor_t));
  char desc_hash[DIGEST_LEN];
  const char *eos;
  smartlist_t *tokens = smartlist_new();
  directory_token_t *tok;
  char secret_id_part[DIGEST_LEN];
  int i, version, num_ok=1;
  smartlist_t *versions;
  char public_key_hash[DIGEST_LEN];
  char test_desc_id[DIGEST_LEN];
  memarea_t *area = NULL;
  const int strict_time_fmt = as_hsdir;

  tor_assert(desc);
  /* Check if desc starts correctly. */
  if (strcmpstart(desc, "rendezvous-service-descriptor ")) {
    log_info(LD_REND, "Descriptor does not start correctly.");
    goto err;
  }
  /* Compute descriptor hash for later validation. */
  if (router_get_hash_impl(desc, strlen(desc), desc_hash,
                           "rendezvous-service-descriptor ",
                           "\nsignature", '\n', DIGEST_SHA1) < 0) {
    log_warn(LD_REND, "Couldn't compute descriptor hash.");
    goto err;
  }
  /* Determine end of string. */
  eos = strstr(desc, "\nrendezvous-service-descriptor ");
  if (!eos)
    eos = desc + strlen(desc);
  else
    eos = eos + 1;
  /* Check length. */
  if (eos-desc > REND_DESC_MAX_SIZE) {
    /* XXXX+ If we are parsing this descriptor as a server, this
     * should be a protocol warning. */
    log_warn(LD_REND, "Descriptor length is %d which exceeds "
             "maximum rendezvous descriptor size of %d bytes.",
             (int)(eos-desc), REND_DESC_MAX_SIZE);
    goto err;
  }
  /* Tokenize descriptor. */
  area = memarea_new();
  if (tokenize_string(area, desc, eos, tokens, desc_token_table, 0)) {
    log_warn(LD_REND, "Error tokenizing descriptor.");
    goto err;
  }
  /* Set next to next descriptor, if available. */
  *next_out = eos;
  /* Set length of encoded descriptor. */
  *encoded_size_out = eos - desc;
  /* Check min allowed length of token list. */
  if (smartlist_len(tokens) < 7) {
    log_warn(LD_REND, "Impossibly short descriptor.");
    goto err;
  }
  /* Parse base32-encoded descriptor ID. */
  tok = find_by_keyword(tokens, R_RENDEZVOUS_SERVICE_DESCRIPTOR);
  tor_assert(tok == smartlist_get(tokens, 0));
  tor_assert(tok->n_args == 1);
  if (!rend_valid_descriptor_id(tok->args[0])) {
    log_warn(LD_REND, "Invalid descriptor ID: '%s'", tok->args[0]);
    goto err;
  }
  if (base32_decode(desc_id_out, DIGEST_LEN,
                    tok->args[0], REND_DESC_ID_V2_LEN_BASE32) != DIGEST_LEN) {
    log_warn(LD_REND,
             "Descriptor ID has wrong length or illegal characters: %s",
             tok->args[0]);
    goto err;
  }
  /* Parse descriptor version. */
  tok = find_by_keyword(tokens, R_VERSION);
  tor_assert(tok->n_args == 1);
  result->version =
    (int) tor_parse_long(tok->args[0], 10, 0, INT_MAX, &num_ok, NULL);
  if (result->version != 2 || !num_ok) {
    /* If it's <2, it shouldn't be under this format.  If the number
     * is greater than 2, we bumped it because we broke backward
     * compatibility.  See how version numbers in our other formats
     * work. */
    log_warn(LD_REND, "Unrecognized descriptor version: %s",
             escaped(tok->args[0]));
    goto err;
  }
  /* Parse public key. */
  tok = find_by_keyword(tokens, R_PERMANENT_KEY);
  result->pk = tok->key;
  tok->key = NULL; /* Prevent free */
  /* Parse secret ID part. */
  tok = find_by_keyword(tokens, R_SECRET_ID_PART);
  tor_assert(tok->n_args == 1);
  if (strlen(tok->args[0]) != REND_SECRET_ID_PART_LEN_BASE32 ||
      strspn(tok->args[0], BASE32_CHARS) != REND_SECRET_ID_PART_LEN_BASE32) {
    log_warn(LD_REND, "Invalid secret ID part: '%s'", tok->args[0]);
    goto err;
  }
  if (base32_decode(secret_id_part, DIGEST_LEN, tok->args[0], 32) !=
      DIGEST_LEN) {
    log_warn(LD_REND,
             "Secret ID part has wrong length or illegal characters: %s",
             tok->args[0]);
    goto err;
  }
  /* Parse publication time -- up-to-date check is done when storing the
   * descriptor. */
  tok = find_by_keyword(tokens, R_PUBLICATION_TIME);
  tor_assert(tok->n_args == 1);
  if (parse_iso_time_(tok->args[0], &result->timestamp,
                      strict_time_fmt, 0) < 0) {
    log_warn(LD_REND, "Invalid publication time: '%s'", tok->args[0]);
    goto err;
  }
  /* Parse protocol versions. */
  tok = find_by_keyword(tokens, R_PROTOCOL_VERSIONS);
  tor_assert(tok->n_args == 1);
  versions = smartlist_new();
  smartlist_split_string(versions, tok->args[0], ",",
                         SPLIT_SKIP_SPACE|SPLIT_IGNORE_BLANK, 0);
  for (i = 0; i < smartlist_len(versions); i++) {
    version = (int) tor_parse_long(smartlist_get(versions, i),
                                   10, 0, INT_MAX, &num_ok, NULL);
    if (!num_ok) /* It's a string; let's ignore it. */
      continue;
    if (version >= REND_PROTOCOL_VERSION_BITMASK_WIDTH)
      /* Avoid undefined left-shift behaviour. */
      continue;
    result->protocols |= 1 << version;
  }
  SMARTLIST_FOREACH(versions, char *, cp, tor_free(cp));
  smartlist_free(versions);
  /* Parse encrypted introduction points. Don't verify. */
  tok = find_opt_by_keyword(tokens, R_INTRODUCTION_POINTS);
  if (tok) {
    if (strcmp(tok->object_type, "MESSAGE")) {
      log_warn(LD_DIR, "Bad object type: introduction points should be of "
               "type MESSAGE");
      goto err;
    }
    *intro_points_encrypted_out = tor_memdup(tok->object_body,
                                             tok->object_size);
    *intro_points_encrypted_size_out = tok->object_size;
  } else {
    *intro_points_encrypted_out = NULL;
    *intro_points_encrypted_size_out = 0;
  }
  /* Parse and verify signature. */
  tok = find_by_keyword(tokens, R_SIGNATURE);
  if (check_signature_token(desc_hash, DIGEST_LEN, tok, result->pk, 0,
                            "v2 rendezvous service descriptor") < 0)
    goto err;
  /* Verify that descriptor ID belongs to public key and secret ID part. */
  if (crypto_pk_get_digest(result->pk, public_key_hash) < 0) {
    log_warn(LD_REND, "Unable to compute rend descriptor public key digest");
    goto err;
  }
  rend_get_descriptor_id_bytes(test_desc_id, public_key_hash,
                               secret_id_part);
  if (tor_memneq(desc_id_out, test_desc_id, DIGEST_LEN)) {
    log_warn(LD_REND, "Parsed descriptor ID does not match "
             "computed descriptor ID.");
    goto err;
  }
  goto done;
 err:
  rend_service_descriptor_free(result);
  result = NULL;
 done:
  if (tokens) {
    SMARTLIST_FOREACH(tokens, directory_token_t *, t, token_clear(t));
    smartlist_free(tokens);
  }
  if (area)
    memarea_drop_all(area);
  *parsed_out = result;
  if (result)
    return 0;
  return -1;
}

/** Decrypt the encrypted introduction points in <b>ipos_encrypted</b> of
 * length <b>ipos_encrypted_size</b> using <b>descriptor_cookie</b> and
 * write the result to a newly allocated string that is pointed to by
 * <b>ipos_decrypted</b> and its length to <b>ipos_decrypted_size</b>.
 * Return 0 if decryption was successful and -1 otherwise. */
int
rend_decrypt_introduction_points(char **ipos_decrypted,
                                 size_t *ipos_decrypted_size,
                                 const char *descriptor_cookie,
                                 const char *ipos_encrypted,
                                 size_t ipos_encrypted_size)
{
  tor_assert(ipos_encrypted);
  tor_assert(descriptor_cookie);
  if (ipos_encrypted_size < 2) {
    log_warn(LD_REND, "Size of encrypted introduction points is too "
                      "small.");
    return -1;
  }
  if (ipos_encrypted[0] == (int)REND_BASIC_AUTH) {
    char iv[CIPHER_IV_LEN], client_id[REND_BASIC_AUTH_CLIENT_ID_LEN],
         session_key[CIPHER_KEY_LEN], *dec;
    int declen, client_blocks;
    size_t pos = 0, len, client_entries_len;
    crypto_digest_t *digest;
    crypto_cipher_t *cipher;
    client_blocks = (int) ipos_encrypted[1];
    client_entries_len = client_blocks * REND_BASIC_AUTH_CLIENT_MULTIPLE *
                         REND_BASIC_AUTH_CLIENT_ENTRY_LEN;
    if (ipos_encrypted_size < 2 + client_entries_len + CIPHER_IV_LEN + 1) {
      log_warn(LD_REND, "Size of encrypted introduction points is too "
                        "small.");
      return -1;
    }
    memcpy(iv, ipos_encrypted + 2 + client_entries_len, CIPHER_IV_LEN);
    digest = crypto_digest_new();
    crypto_digest_add_bytes(digest, descriptor_cookie, REND_DESC_COOKIE_LEN);
    crypto_digest_add_bytes(digest, iv, CIPHER_IV_LEN);
    crypto_digest_get_digest(digest, client_id,
                             REND_BASIC_AUTH_CLIENT_ID_LEN);
    crypto_digest_free(digest);
    for (pos = 2; pos < 2 + client_entries_len;
         pos += REND_BASIC_AUTH_CLIENT_ENTRY_LEN) {
      if (tor_memeq(ipos_encrypted + pos, client_id,
                  REND_BASIC_AUTH_CLIENT_ID_LEN)) {
        /* Attempt to decrypt introduction points. */
        cipher = crypto_cipher_new(descriptor_cookie);
        if (crypto_cipher_decrypt(cipher, session_key, ipos_encrypted
                                  + pos + REND_BASIC_AUTH_CLIENT_ID_LEN,
                                  CIPHER_KEY_LEN) < 0) {
          log_warn(LD_REND, "Could not decrypt session key for client.");
          crypto_cipher_free(cipher);
          return -1;
        }
        crypto_cipher_free(cipher);

        len = ipos_encrypted_size - 2 - client_entries_len - CIPHER_IV_LEN;
        dec = tor_malloc_zero(len + 1);
        declen = crypto_cipher_decrypt_with_iv(session_key, dec, len,
            ipos_encrypted + 2 + client_entries_len,
            ipos_encrypted_size - 2 - client_entries_len);

        if (declen < 0) {
          log_warn(LD_REND, "Could not decrypt introduction point string.");
          tor_free(dec);
          return -1;
        }
        if (fast_memcmpstart(dec, declen, "introduction-point ")) {
          log_warn(LD_REND, "Decrypted introduction points don't "
                            "look like we could parse them.");
          tor_free(dec);
          continue;
        }
        *ipos_decrypted = dec;
        *ipos_decrypted_size = declen;
        return 0;
      }
    }
    log_warn(LD_REND, "Could not decrypt introduction points. Please "
             "check your authorization for this service!");
    return -1;
  } else if (ipos_encrypted[0] == (int)REND_STEALTH_AUTH) {
    char *dec;
    int declen;
    if (ipos_encrypted_size < CIPHER_IV_LEN + 2) {
      log_warn(LD_REND, "Size of encrypted introduction points is too "
                        "small.");
      return -1;
    }
    dec = tor_malloc_zero(ipos_encrypted_size - CIPHER_IV_LEN - 1 + 1);

    declen = crypto_cipher_decrypt_with_iv(descriptor_cookie, dec,
                                           ipos_encrypted_size -
                                               CIPHER_IV_LEN - 1,
                                           ipos_encrypted + 1,
                                           ipos_encrypted_size - 1);

    if (declen < 0) {
      log_warn(LD_REND, "Decrypting introduction points failed!");
      tor_free(dec);
      return -1;
    }
    *ipos_decrypted = dec;
    *ipos_decrypted_size = declen;
    return 0;
  } else {
    log_warn(LD_REND, "Unknown authorization type number: %d",
             ipos_encrypted[0]);
    return -1;
  }
}

/** Parse the encoded introduction points in <b>intro_points_encoded</b> of
 * length <b>intro_points_encoded_size</b> and write the result to the
 * descriptor in <b>parsed</b>; return the number of successfully parsed
 * introduction points or -1 in case of a failure. */
int
rend_parse_introduction_points(rend_service_descriptor_t *parsed,
                               const char *intro_points_encoded,
                               size_t intro_points_encoded_size)
{
  const char *current_ipo, *end_of_intro_points;
  smartlist_t *tokens = NULL;
  directory_token_t *tok;
  rend_intro_point_t *intro;
  extend_info_t *info;
  int result, num_ok=1;
  memarea_t *area = NULL;
  tor_assert(parsed);
  /** Function may only be invoked once. */
  tor_assert(!parsed->intro_nodes);
  if (!intro_points_encoded || intro_points_encoded_size == 0) {
    log_warn(LD_REND, "Empty or zero size introduction point list");
    goto err;
  }
  /* Consider one intro point after the other. */
  current_ipo = intro_points_encoded;
  end_of_intro_points = intro_points_encoded + intro_points_encoded_size;
  tokens = smartlist_new();
  parsed->intro_nodes = smartlist_new();
  area = memarea_new();

  while (!fast_memcmpstart(current_ipo, end_of_intro_points-current_ipo,
                      "introduction-point ")) {
    /* Determine end of string. */
    const char *eos = tor_memstr(current_ipo, end_of_intro_points-current_ipo,
                                 "\nintroduction-point ");
    if (!eos)
      eos = end_of_intro_points;
    else
      eos = eos+1;
    tor_assert(eos <= intro_points_encoded+intro_points_encoded_size);
    /* Free tokens and clear token list. */
    SMARTLIST_FOREACH(tokens, directory_token_t *, t, token_clear(t));
    smartlist_clear(tokens);
    memarea_clear(area);
    /* Tokenize string. */
    if (tokenize_string(area, current_ipo, eos, tokens, ipo_token_table, 0)) {
      log_warn(LD_REND, "Error tokenizing introduction point");
      goto err;
    }
    /* Advance to next introduction point, if available. */
    current_ipo = eos;
    /* Check minimum allowed length of introduction point. */
    if (smartlist_len(tokens) < 5) {
      log_warn(LD_REND, "Impossibly short introduction point.");
      goto err;
    }
    /* Allocate new intro point and extend info. */
    intro = tor_malloc_zero(sizeof(rend_intro_point_t));
    info = intro->extend_info =
      extend_info_new(NULL, NULL, NULL, NULL, NULL, NULL, 0);
    /* Parse identifier. */
    tok = find_by_keyword(tokens, R_IPO_IDENTIFIER);
    if (base32_decode(info->identity_digest, DIGEST_LEN,
                      tok->args[0], REND_INTRO_POINT_ID_LEN_BASE32) !=
        DIGEST_LEN) {
      log_warn(LD_REND,
               "Identity digest has wrong length or illegal characters: %s",
               tok->args[0]);
      rend_intro_point_free(intro);
      goto err;
    }
    /* Write identifier to nickname. */
    info->nickname[0] = '$';
    base16_encode(info->nickname + 1, sizeof(info->nickname) - 1,
                  info->identity_digest, DIGEST_LEN);
    /* Parse IP address. */
    tok = find_by_keyword(tokens, R_IPO_IP_ADDRESS);
    tor_addr_t addr;
    if (tor_addr_parse(&addr, tok->args[0])<0) {
      log_warn(LD_REND, "Could not parse introduction point address.");
      rend_intro_point_free(intro);
      goto err;
    }
    if (tor_addr_family(&addr) != AF_INET) {
      log_warn(LD_REND, "Introduction point address was not ipv4.");
      rend_intro_point_free(intro);
      goto err;
    }

    /* Parse onion port. */
    tok = find_by_keyword(tokens, R_IPO_ONION_PORT);
    uint16_t port = (uint16_t) tor_parse_long(tok->args[0],10,1,65535,
                                           &num_ok,NULL);
    if (!port || !num_ok) {
      log_warn(LD_REND, "Introduction point onion port %s is invalid",
               escaped(tok->args[0]));
      rend_intro_point_free(intro);
      goto err;
    }

    /* Add the address and port. */
    extend_info_add_orport(info, &addr, port);

    /* Parse onion key. */
    tok = find_by_keyword(tokens, R_IPO_ONION_KEY);
    if (!crypto_pk_public_exponent_ok(tok->key)) {
      log_warn(LD_REND,
               "Introduction point's onion key had invalid exponent.");
      rend_intro_point_free(intro);
      goto err;
    }
    info->onion_key = tok->key;
    tok->key = NULL; /* Prevent free */
    /* Parse service key. */
    tok = find_by_keyword(tokens, R_IPO_SERVICE_KEY);
    if (!crypto_pk_public_exponent_ok(tok->key)) {
      log_warn(LD_REND,
               "Introduction point key had invalid exponent.");
      rend_intro_point_free(intro);
      goto err;
    }
    intro->intro_key = tok->key;
    tok->key = NULL; /* Prevent free */
    /* Add extend info to list of introduction points. */
    smartlist_add(parsed->intro_nodes, intro);
  }
  result = smartlist_len(parsed->intro_nodes);
  goto done;

 err:
  result = -1;

 done:
  /* Free tokens and clear token list. */
  if (tokens) {
    SMARTLIST_FOREACH(tokens, directory_token_t *, t, token_clear(t));
    smartlist_free(tokens);
  }
  if (area)
    memarea_drop_all(area);

  return result;
}

/** Parse the content of a client_key file in <b>ckstr</b> and add
 * rend_authorized_client_t's for each parsed client to
 * <b>parsed_clients</b>. Return the number of parsed clients as result
 * or -1 for failure. */
int
rend_parse_client_keys(strmap_t *parsed_clients, const char *ckstr)
{
  int result = -1;
  smartlist_t *tokens;
  directory_token_t *tok;
  const char *current_entry = NULL;
  memarea_t *area = NULL;
  char *err_msg = NULL;
  if (!ckstr || strlen(ckstr) == 0)
    return -1;
  tokens = smartlist_new();
  /* Begin parsing with first entry, skipping comments or whitespace at the
   * beginning. */
  area = memarea_new();
  current_entry = eat_whitespace(ckstr);
  while (!strcmpstart(current_entry, "client-name ")) {
    rend_authorized_client_t *parsed_entry;
    /* Determine end of string. */
    const char *eos = strstr(current_entry, "\nclient-name ");
    if (!eos)
      eos = current_entry + strlen(current_entry);
    else
      eos = eos + 1;
    /* Free tokens and clear token list. */
    SMARTLIST_FOREACH(tokens, directory_token_t *, t, token_clear(t));
    smartlist_clear(tokens);
    memarea_clear(area);
    /* Tokenize string. */
    if (tokenize_string(area, current_entry, eos, tokens,
                        client_keys_token_table, 0)) {
      log_warn(LD_REND, "Error tokenizing client keys file.");
      goto err;
    }
    /* Advance to next entry, if available. */
    current_entry = eos;
    /* Check minimum allowed length of token list. */
    if (smartlist_len(tokens) < 2) {
      log_warn(LD_REND, "Impossibly short client key entry.");
      goto err;
    }
    /* Parse client name. */
    tok = find_by_keyword(tokens, C_CLIENT_NAME);
    tor_assert(tok == smartlist_get(tokens, 0));
    tor_assert(tok->n_args == 1);

    if (!rend_valid_client_name(tok->args[0])) {
      log_warn(LD_CONFIG, "Illegal client name: %s. (Length must be "
               "between 1 and %d, and valid characters are "
               "[A-Za-z0-9+-_].)", tok->args[0], REND_CLIENTNAME_MAX_LEN);
      goto err;
    }
    /* Check if client name is duplicate. */
    if (strmap_get(parsed_clients, tok->args[0])) {
      log_warn(LD_CONFIG, "HiddenServiceAuthorizeClient contains a "
               "duplicate client name: '%s'. Ignoring.", tok->args[0]);
      goto err;
    }
    parsed_entry = tor_malloc_zero(sizeof(rend_authorized_client_t));
    parsed_entry->client_name = tor_strdup(tok->args[0]);
    strmap_set(parsed_clients, parsed_entry->client_name, parsed_entry);
    /* Parse client key. */
    tok = find_opt_by_keyword(tokens, C_CLIENT_KEY);
    if (tok) {
      parsed_entry->client_key = tok->key;
      tok->key = NULL; /* Prevent free */
    }

    /* Parse descriptor cookie. */
    tok = find_by_keyword(tokens, C_DESCRIPTOR_COOKIE);
    tor_assert(tok->n_args == 1);
    if (rend_auth_decode_cookie(tok->args[0], parsed_entry->descriptor_cookie,
                                NULL, &err_msg) < 0) {
      tor_assert(err_msg);
      log_warn(LD_REND, "%s", err_msg);
      tor_free(err_msg);
      goto err;
    }
  }
  result = strmap_size(parsed_clients);
  goto done;
 err:
  result = -1;
 done:
  /* Free tokens and clear token list. */
  SMARTLIST_FOREACH(tokens, directory_token_t *, t, token_clear(t));
  smartlist_free(tokens);
  if (area)
    memarea_drop_all(area);
  return result;
}

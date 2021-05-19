/* Copyright (c) 2001 Matej Pfajfar.
 * Copyright (c) 2001-2004, Roger Dingledine.
 * Copyright (c) 2004-2006, Roger Dingledine, Nick Mathewson.
 * Copyright (c) 2007-2020, The Tor Project, Inc. */
/* See LICENSE for licensing information */

/**
 * \file describe.c
 * \brief Format short descriptions of relays.
 */

#define DESCRIBE_PRIVATE

#include "core/or/or.h"
#include "core/or/extendinfo.h"
#include "feature/nodelist/describe.h"
#include "feature/nodelist/nodelist.h"
#include "feature/nodelist/routerinfo.h"
#include "lib/crypt_ops/crypto_ed25519.h"
#include "lib/crypt_ops/crypto_format.h"

#include "core/or/extend_info_st.h"
#include "feature/nodelist/node_st.h"
#include "feature/nodelist/routerinfo_st.h"
#include "feature/nodelist/routerstatus_st.h"
#include "feature/nodelist/microdesc_st.h"

/** Use <b>buf</b> (which must be at least NODE_DESC_BUF_LEN bytes long) to
 * hold a human-readable description of a node with identity digest
 * <b>id_digest</b>, nickname <b>nickname</b>, and addresses <b>addr32h</b> and
 * <b>addr</b>.
 *
 * The <b>nickname</b>, <b>ipv6_addr</b> and <b>ipv4_addr</b> fields are
 * optional and may be set to NULL or the null address.
 *
 * Return a pointer to the front of <b>buf</b>.
 * If buf is NULL, return a string constant describing the error.
 */
STATIC const char *
format_node_description(char *buf,
                        const char *rsa_id_digest,
                        const ed25519_public_key_t *ed25519_id,
                        const char *nickname,
                        const tor_addr_t *ipv4_addr,
                        const tor_addr_t *ipv6_addr)
{
  size_t rv = 0;
  bool has_ipv6 = ipv6_addr && !tor_addr_is_null(ipv6_addr);
  bool valid_ipv4 = false;

  if (!buf)
    return "<NULL BUFFER>";

  memset(buf, 0, NODE_DESC_BUF_LEN);

  if (!rsa_id_digest) {
    /* strlcpy() returns the length of the source string it attempted to copy,
     * ignoring any required truncation due to the buffer length. */
    rv = strlcpy(buf, "<NULL ID DIGEST>", NODE_DESC_BUF_LEN);
    tor_assert_nonfatal(rv < NODE_DESC_BUF_LEN);
    return buf;
  }

  /* strlcat() returns the length of the concatenated string it attempted to
   * create, ignoring any required truncation due to the buffer length.  */
  rv = strlcat(buf, "$", NODE_DESC_BUF_LEN);
  tor_assert_nonfatal(rv < NODE_DESC_BUF_LEN);

  {
    char hex_digest[HEX_DIGEST_LEN+1];
    memset(hex_digest, 0, sizeof(hex_digest));

    base16_encode(hex_digest, sizeof(hex_digest),
                  rsa_id_digest, DIGEST_LEN);
    rv = strlcat(buf, hex_digest, NODE_DESC_BUF_LEN);
    tor_assert_nonfatal(rv < NODE_DESC_BUF_LEN);
  }

  if (nickname) {
    rv = strlcat(buf, "~", NODE_DESC_BUF_LEN);
    tor_assert_nonfatal(rv < NODE_DESC_BUF_LEN);
    rv = strlcat(buf, nickname, NODE_DESC_BUF_LEN);
    tor_assert_nonfatal(rv < NODE_DESC_BUF_LEN);
  }
  if (ed25519_id) {
    char ed_base64[ED25519_BASE64_LEN+1];
    ed25519_public_to_base64(ed_base64, ed25519_id);
    rv = strlcat(buf, " [", NODE_DESC_BUF_LEN);
    tor_assert_nonfatal(rv < NODE_DESC_BUF_LEN);
    rv = strlcat(buf, ed_base64, NODE_DESC_BUF_LEN);
    tor_assert_nonfatal(rv < NODE_DESC_BUF_LEN);
    rv = strlcat(buf, "]", NODE_DESC_BUF_LEN);
    tor_assert_nonfatal(rv < NODE_DESC_BUF_LEN);
  }
  if (ipv4_addr || has_ipv6) {
    rv = strlcat(buf, " at ", NODE_DESC_BUF_LEN);
    tor_assert_nonfatal(rv < NODE_DESC_BUF_LEN);
  }
  if (ipv4_addr) {
    const char *str_rv = NULL;
    char addr_str[TOR_ADDR_BUF_LEN];
    memset(addr_str, 0, sizeof(addr_str));

    str_rv = tor_addr_to_str(addr_str, ipv4_addr, sizeof(addr_str), 0);
    if (str_rv) {
      rv = strlcat(buf, addr_str, NODE_DESC_BUF_LEN);
      tor_assert_nonfatal(rv < NODE_DESC_BUF_LEN);
      valid_ipv4 = true;
    }
  }
  /* Both addresses are valid */
  if (valid_ipv4 && has_ipv6) {
    rv = strlcat(buf, " and ", NODE_DESC_BUF_LEN);
    tor_assert_nonfatal(rv < NODE_DESC_BUF_LEN);
  }
  if (has_ipv6) {
    const char *str_rv = NULL;
    char addr_str[TOR_ADDR_BUF_LEN];
    memset(addr_str, 0, sizeof(addr_str));

    str_rv = tor_addr_to_str(addr_str, ipv6_addr, sizeof(addr_str), 1);
    if (str_rv) {
      rv = strlcat(buf, addr_str, NODE_DESC_BUF_LEN);
      tor_assert_nonfatal(rv < NODE_DESC_BUF_LEN);
    }
  }

  return buf;
}

/** Return a human-readable description of the routerinfo_t <b>ri</b>.
 *
 * This function is not thread-safe.  Each call to this function invalidates
 * previous values returned by this function.
 */
const char *
router_describe(const routerinfo_t *ri)
{
  static char buf[NODE_DESC_BUF_LEN];

  if (!ri)
    return "<null>";

  const ed25519_public_key_t *ed25519_id = routerinfo_get_ed25519_id(ri);

  return format_node_description(buf,
                                 ri->cache_info.identity_digest,
                                 ed25519_id,
                                 ri->nickname,
                                 &ri->ipv4_addr,
                                 &ri->ipv6_addr);
}

/** Return a human-readable description of the node_t <b>node</b>.
 *
 * This function is not thread-safe.  Each call to this function invalidates
 * previous values returned by this function.
 */
const char *
node_describe(const node_t *node)
{
  static char buf[NODE_DESC_BUF_LEN];
  const char *nickname = NULL;
  const tor_addr_t *ipv6_addr = NULL, *ipv4_addr = NULL;

  if (!node)
    return "<null>";

  if (node->rs) {
    nickname = node->rs->nickname;
    ipv4_addr = &node->rs->ipv4_addr;
    ipv6_addr = &node->rs->ipv6_addr;
    /* Support consensus versions less than 28, when IPv6 addresses were in
     * microdescs. This code can be removed when 0.2.9 is no longer supported,
     * and the MIN_METHOD_FOR_NO_A_LINES_IN_MICRODESC macro is removed. */
    if (node->md && tor_addr_is_null(ipv6_addr)) {
      ipv6_addr = &node->md->ipv6_addr;
    }
  } else if (node->ri) {
    nickname = node->ri->nickname;
    ipv4_addr = &node->ri->ipv4_addr;
    ipv6_addr = &node->ri->ipv6_addr;
  } else {
    return "<null rs and ri>";
  }

  const ed25519_public_key_t *ed25519_id = node_get_ed25519_id(node);

  return format_node_description(buf,
                                 node->identity,
                                 ed25519_id,
                                 nickname,
                                 ipv4_addr,
                                 ipv6_addr);
}

/** Return a human-readable description of the routerstatus_t <b>rs</b>.
 *
 * This function is not thread-safe.  Each call to this function invalidates
 * previous values returned by this function.
 */
const char *
routerstatus_describe(const routerstatus_t *rs)
{
  static char buf[NODE_DESC_BUF_LEN];

  if (!rs)
    return "<null>";

  return format_node_description(buf,
                                 rs->identity_digest,
                                 NULL,
                                 rs->nickname,
                                 &rs->ipv4_addr,
                                 &rs->ipv6_addr);
}

/** Return a human-readable description of the extend_info_t <b>ei</b>.
 *
 * This function is not thread-safe.  Each call to this function invalidates
 * previous values returned by this function.
 */
const char *
extend_info_describe(const extend_info_t *ei)
{
  static char buf[NODE_DESC_BUF_LEN];

  if (!ei)
    return "<null>";

  const tor_addr_port_t *ap4 = extend_info_get_orport(ei, AF_INET);
  const tor_addr_port_t *ap6 = extend_info_get_orport(ei, AF_INET6);
  const tor_addr_t *addr4 = ap4 ? &ap4->addr : NULL;
  const tor_addr_t *addr6 = ap6 ? &ap6->addr : NULL;

  const ed25519_public_key_t *ed25519_id = &ei->ed_identity;
  if (ed25519_public_key_is_zero(ed25519_id))
    ed25519_id = NULL;

  return format_node_description(buf,
                                 ei->identity_digest,
                                 ed25519_id,
                                 ei->nickname,
                                 addr4,
                                 addr6);
}

/** Set <b>buf</b> (which must have MAX_VERBOSE_NICKNAME_LEN+1 bytes) to the
 * verbose representation of the identity of <b>router</b>.  The format is:
 *  A dollar sign.
 *  The upper-case hexadecimal encoding of the SHA1 hash of router's identity.
 *  A "=" if the router is named (no longer implemented); a "~" if it is not.
 *  The router's nickname.
 **/
void
router_get_verbose_nickname(char *buf, const routerinfo_t *router)
{
  size_t rv = 0;

  if (!buf)
    return;

  memset(buf, 0, MAX_VERBOSE_NICKNAME_LEN+1);

  if (!router) {
    /* strlcpy() returns the length of the source string it attempted to copy,
     * ignoring any required truncation due to the buffer length. */
    rv = strlcpy(buf, "<null>", MAX_VERBOSE_NICKNAME_LEN+1);
    tor_assert_nonfatal(rv < MAX_VERBOSE_NICKNAME_LEN+1);
    return;
  }

  /* strlcat() returns the length of the concatenated string it attempted to
   * create, ignoring any required truncation due to the buffer length.  */
  rv = strlcat(buf, "$", MAX_VERBOSE_NICKNAME_LEN+1);
  tor_assert_nonfatal(rv < MAX_VERBOSE_NICKNAME_LEN+1);

  {
    char hex_digest[HEX_DIGEST_LEN+1];
    memset(hex_digest, 0, sizeof(hex_digest));

    base16_encode(hex_digest, sizeof(hex_digest),
                  router->cache_info.identity_digest, DIGEST_LEN);
    rv = strlcat(buf, hex_digest, MAX_VERBOSE_NICKNAME_LEN+1);
    tor_assert_nonfatal(rv < MAX_VERBOSE_NICKNAME_LEN+1);
  }

  rv = strlcat(buf, "~", MAX_VERBOSE_NICKNAME_LEN+1);
  tor_assert_nonfatal(rv < MAX_VERBOSE_NICKNAME_LEN+1);

  rv = strlcat(buf, router->nickname, MAX_VERBOSE_NICKNAME_LEN+1);
  tor_assert_nonfatal(rv < MAX_VERBOSE_NICKNAME_LEN+1);
}

/* Copyright (c) 2001 Matej Pfajfar.
 * Copyright (c) 2001-2004, Roger Dingledine.
 * Copyright (c) 2004-2006, Roger Dingledine, Nick Mathewson.
 * Copyright (c) 2007-2019, The Tor Project, Inc. */
/* See LICENSE for licensing information */

/**
 * \file describe.c
 * \brief Format short descriptions of relays.
 */

#include "core/or/or.h"
#include "feature/nodelist/describe.h"
#include "feature/nodelist/routerinfo.h"

#include "core/or/extend_info_st.h"
#include "feature/nodelist/node_st.h"
#include "feature/nodelist/routerinfo_st.h"
#include "feature/nodelist/routerstatus_st.h"

/**
 * Longest allowed output of format_node_description, plus 1 character for
 * NUL.  This allows space for:
 * "$FFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFF~xxxxxxxxxxxxxxxxxxx at"
 * " [ffff:ffff:ffff:ffff:ffff:ffff:255.255.255.255]"
 * plus a terminating NUL.
 */
#define NODE_DESC_BUF_LEN (MAX_VERBOSE_NICKNAME_LEN+4+TOR_ADDR_BUF_LEN)

/** Use <b>buf</b> (which must be at least NODE_DESC_BUF_LEN bytes long) to
 * hold a human-readable description of a node with identity digest
 * <b>id_digest</b>, named-status <b>is_named</b>, nickname <b>nickname</b>,
 * and address <b>addr</b> or <b>addr32h</b>.
 *
 * The <b>nickname</b> and <b>addr</b> fields are optional and may be set to
 * NULL.  The <b>addr32h</b> field is optional and may be set to 0.
 *
 * Return a pointer to the front of <b>buf</b>.
 */
static const char *
format_node_description(char *buf,
                        const char *id_digest,
                        int is_named,
                        const char *nickname,
                        const tor_addr_t *addr,
                        uint32_t addr32h)
{
  char *cp;

  if (!buf)
    return "<NULL BUFFER>";

  buf[0] = '$';
  base16_encode(buf+1, HEX_DIGEST_LEN+1, id_digest, DIGEST_LEN);
  cp = buf+1+HEX_DIGEST_LEN;
  if (nickname) {
    buf[1+HEX_DIGEST_LEN] = is_named ? '=' : '~';
    strlcpy(buf+1+HEX_DIGEST_LEN+1, nickname, MAX_NICKNAME_LEN+1);
    cp += strlen(cp);
  }
  if (addr32h || addr) {
    memcpy(cp, " at ", 4);
    cp += 4;
    if (addr) {
      tor_addr_to_str(cp, addr, TOR_ADDR_BUF_LEN, 0);
    } else {
      struct in_addr in;
      in.s_addr = htonl(addr32h);
      tor_inet_ntoa(&in, cp, INET_NTOA_BUF_LEN);
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
  return format_node_description(buf,
                                 ri->cache_info.identity_digest,
                                 0,
                                 ri->nickname,
                                 NULL,
                                 ri->addr);
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
  uint32_t addr32h = 0;
  int is_named = 0;

  if (!node)
    return "<null>";

  if (node->rs) {
    nickname = node->rs->nickname;
    is_named = node->rs->is_named;
    addr32h = node->rs->addr;
  } else if (node->ri) {
    nickname = node->ri->nickname;
    addr32h = node->ri->addr;
  }

  return format_node_description(buf,
                                 node->identity,
                                 is_named,
                                 nickname,
                                 NULL,
                                 addr32h);
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
                                 rs->is_named,
                                 rs->nickname,
                                 NULL,
                                 rs->addr);
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
  return format_node_description(buf,
                                 ei->identity_digest,
                                 0,
                                 ei->nickname,
                                 &ei->addr,
                                 0);
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
  buf[0] = '$';
  base16_encode(buf+1, HEX_DIGEST_LEN+1, router->cache_info.identity_digest,
                DIGEST_LEN);
  buf[1+HEX_DIGEST_LEN] = '~';
  strlcpy(buf+1+HEX_DIGEST_LEN+1, router->nickname, MAX_NICKNAME_LEN+1);
}

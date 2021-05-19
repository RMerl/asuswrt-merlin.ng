/* Copyright (c) 2001 Matej Pfajfar.
 * Copyright (c) 2001-2004, Roger Dingledine.
 * Copyright (c) 2004-2006, Roger Dingledine, Nick Mathewson.
 * Copyright (c) 2007-2020, The Tor Project, Inc. */
/* See LICENSE for licensing information */

/**
 * \file policy_parse.c
 * \brief Code to parse address policies.
 **/

#define  ROUTERDESC_TOKEN_TABLE_PRIVATE

#include "core/or/or.h"

#include "core/or/policies.h"
#include "feature/dirparse/parsecommon.h"
#include "feature/dirparse/policy_parse.h"
#include "feature/dirparse/routerparse.h"
#include "feature/dirparse/unparseable.h"
#include "lib/memarea/memarea.h"

#include "core/or/addr_policy_st.h"

static addr_policy_t *router_parse_addr_policy_private(directory_token_t *tok);

/** Parse the addr policy in the string <b>s</b> and return it.  If
 * assume_action is nonnegative, then insert its action (ADDR_POLICY_ACCEPT or
 * ADDR_POLICY_REJECT) for items that specify no action.
 *
 * Returns NULL on policy errors.
 *
 * Set *<b>malformed_list</b> to true if the entire policy list should be
 * discarded. Otherwise, set it to false, and only this item should be ignored
 * on error - the rest of the policy list can continue to be processed and
 * used.
 *
 * The addr_policy_t returned by this function can have its address set to
 * AF_UNSPEC for '*'.  Use policy_expand_unspec() to turn this into a pair
 * of AF_INET and AF_INET6 items.
 */
MOCK_IMPL(addr_policy_t *,
router_parse_addr_policy_item_from_string,(const char *s, int assume_action,
                                           int *malformed_list))
{
  directory_token_t *tok = NULL;
  const char *cp, *eos;
  /* Longest possible policy is
   * "accept6 [ffff:ffff:..255]/128:10000-65535",
   * which contains a max-length IPv6 address, plus 26 characters.
   * But note that there can be an arbitrary amount of space between the
   * accept and the address:mask/port element.
   * We don't need to multiply TOR_ADDR_BUF_LEN by 2, as there is only one
   * IPv6 address. But making the buffer shorter might cause valid long lines,
   * which parsed in previous versions, to fail to parse in new versions.
   * (These lines would have to have excessive amounts of whitespace.) */
  char line[TOR_ADDR_BUF_LEN*2 + 32];
  addr_policy_t *r;
  memarea_t *area = NULL;

  tor_assert(malformed_list);
  *malformed_list = 0;

  s = eat_whitespace(s);
  /* We can only do assume_action on []-quoted IPv6, as "a" (accept)
   * and ":" (port separator) are ambiguous */
  if ((*s == '*' || *s == '[' || TOR_ISDIGIT(*s)) && assume_action >= 0) {
    if (tor_snprintf(line, sizeof(line), "%s %s",
               assume_action == ADDR_POLICY_ACCEPT?"accept":"reject", s)<0) {
      log_warn(LD_DIR, "Policy %s is too long.", escaped(s));
      return NULL;
    }
    cp = line;
    tor_strlower(line);
  } else { /* assume an already well-formed address policy line */
    cp = s;
  }

  eos = cp + strlen(cp);
  area = memarea_new();
  tok = get_next_token(area, &cp, eos, routerdesc_token_table);
  if (tok->tp == ERR_) {
    log_warn(LD_DIR, "Error reading address policy: %s", tok->error);
    goto err;
  }
  if (tok->tp != K_ACCEPT && tok->tp != K_ACCEPT6 &&
      tok->tp != K_REJECT && tok->tp != K_REJECT6) {
    log_warn(LD_DIR, "Expected 'accept' or 'reject'.");
    goto err;
  }

  /* Use the extended interpretation of accept/reject *,
   * expanding it into an IPv4 wildcard and an IPv6 wildcard.
   * Also permit *4 and *6 for IPv4 and IPv6 only wildcards. */
  r = router_parse_addr_policy(tok, TAPMP_EXTENDED_STAR);
  if (!r) {
    goto err;
  }

  /* Ensure that accept6/reject6 fields are followed by IPv6 addresses.
   * AF_UNSPEC addresses are only permitted on the accept/reject field type.
   * Unlike descriptors, torrcs exit policy accept/reject can be followed by
   * either an IPv4 or IPv6 address. */
  if ((tok->tp == K_ACCEPT6 || tok->tp == K_REJECT6) &&
       tor_addr_family(&r->addr) != AF_INET6) {
    /* This is a non-fatal error, just ignore this one entry. */
    *malformed_list = 0;
    log_warn(LD_DIR, "IPv4 address '%s' with accept6/reject6 field type in "
             "exit policy. Ignoring, but continuing to parse rules. (Use "
             "accept/reject with IPv4 addresses.)",
             tok->n_args == 1 ? tok->args[0] : "");
    addr_policy_free(r);
    r = NULL;
    goto done;
  }

  goto done;
 err:
  *malformed_list = 1;
  r = NULL;
 done:
  token_clear(tok);
  if (area) {
    DUMP_AREA(area, "policy item");
    memarea_drop_all(area);
  }
  return r;
}

/** Given a K_ACCEPT[6] or K_REJECT[6] token and a router, create and return
 * a new exit_policy_t corresponding to the token. If TAPMP_EXTENDED_STAR
 * is set in fmt_flags, K_ACCEPT6 and K_REJECT6 tokens followed by *
 * expand to IPv6-only policies, otherwise they expand to IPv4 and IPv6
 * policies */
addr_policy_t *
router_parse_addr_policy(directory_token_t *tok, unsigned fmt_flags)
{
  addr_policy_t newe;
  char *arg;

  tor_assert(tok->tp == K_REJECT || tok->tp == K_REJECT6 ||
             tok->tp == K_ACCEPT || tok->tp == K_ACCEPT6);

  if (tok->n_args != 1)
    return NULL;
  arg = tok->args[0];

  if (!strcmpstart(arg,"private"))
    return router_parse_addr_policy_private(tok);

  memset(&newe, 0, sizeof(newe));

  if (tok->tp == K_REJECT || tok->tp == K_REJECT6)
    newe.policy_type = ADDR_POLICY_REJECT;
  else
    newe.policy_type = ADDR_POLICY_ACCEPT;

  /* accept6/reject6 * produces an IPv6 wildcard address only.
   * (accept/reject * produces rules for IPv4 and IPv6 wildcard addresses.) */
  if ((fmt_flags & TAPMP_EXTENDED_STAR)
      && (tok->tp == K_ACCEPT6 || tok->tp == K_REJECT6)) {
    fmt_flags |= TAPMP_STAR_IPV6_ONLY;
  }

  if (tor_addr_parse_mask_ports(arg, fmt_flags, &newe.addr, &newe.maskbits,
                                &newe.prt_min, &newe.prt_max) < 0) {
    log_warn(LD_DIR,"Couldn't parse line %s. Dropping", escaped(arg));
    return NULL;
  }

  addr_policy_t *result = addr_policy_get_canonical_entry(&newe);
  /* It would be a nasty error to return 'newe', and sometimes
   * addr_policy_get_canonical_entry() can return its argument.  But in this
   * case, it won't, since newe is *not* canonical.  We assert here to make
   * sure that the compiler knows it too.
   */
  tor_assert(result != &newe);
  return result;
}

/** Parse an exit policy line of the format "accept[6]/reject[6] private:...".
 * This didn't exist until Tor 0.1.1.15, so nobody should generate it in
 * router descriptors until earlier versions are obsolete.
 *
 * accept/reject and accept6/reject6 private all produce rules for both
 * IPv4 and IPv6 addresses.
 */
static addr_policy_t *
router_parse_addr_policy_private(directory_token_t *tok)
{
  const char *arg;
  uint16_t port_min, port_max;
  addr_policy_t result;

  arg = tok->args[0];
  if (strcmpstart(arg, "private"))
    return NULL;

  arg += strlen("private");
  arg = (char*) eat_whitespace(arg);
  if (!arg || *arg != ':')
    return NULL;

  if (parse_port_range(arg+1, &port_min, &port_max)<0)
    return NULL;

  memset(&result, 0, sizeof(result));
  if (tok->tp == K_REJECT || tok->tp == K_REJECT6)
    result.policy_type = ADDR_POLICY_REJECT;
  else
    result.policy_type = ADDR_POLICY_ACCEPT;
  result.is_private = 1;
  result.prt_min = port_min;
  result.prt_max = port_max;

  if (tok->tp == K_ACCEPT6 || tok->tp == K_REJECT6) {
    log_warn(LD_GENERAL,
             "'%s' expands into rules which apply to all private IPv4 and "
             "IPv6 addresses. (Use accept/reject private:* for IPv4 and "
             "IPv6.)", tok->n_args == 1 ? tok->args[0] : "");
  }

  return addr_policy_get_canonical_entry(&result);
}

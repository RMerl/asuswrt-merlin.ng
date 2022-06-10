/* Copyright (c) 2001 Matej Pfajfar.
 * Copyright (c) 2001-2004, Roger Dingledine.
 * Copyright (c) 2004-2006, Roger Dingledine, Nick Mathewson.
 * Copyright (c) 2007-2021, The Tor Project, Inc. */
/* See LICENSE for licensing information */

/**
 * \file routerset.c
 *
 * \brief Functions and structures to handle set-type selection of routers
 *  by name, ID, address, etc.
 *
 * This module implements the routerset_t data structure, whose purpose
 * is to specify a set of relays based on a list of their identities or
 * properties.  Routersets can restrict relays by IP address mask,
 * identity fingerprint, country codes, and nicknames (deprecated).
 *
 * Routersets are typically used for user-specified restrictions, and
 * are created by invoking routerset_new and routerset_parse from
 * config.c and confmgt.c.  To use a routerset, invoke one of
 * routerset_contains_...() functions , or use
 * routerstatus_get_all_nodes() / routerstatus_subtract_nodes() to
 * manipulate a smartlist of node_t pointers.
 *
 * Country-code restrictions are implemented in geoip.c.
 */

#define ROUTERSET_PRIVATE

#include "core/or/or.h"
#include "core/or/policies.h"
#include "feature/client/bridges.h"
#include "feature/dirparse/policy_parse.h"
#include "feature/nodelist/nickname.h"
#include "feature/nodelist/nodelist.h"
#include "feature/nodelist/routerset.h"
#include "lib/conf/conftypes.h"
#include "lib/confmgt/typedvar.h"
#include "lib/encoding/confline.h"
#include "lib/geoip/geoip.h"

#include "core/or/addr_policy_st.h"
#include "core/or/extend_info_st.h"
#include "feature/nodelist/node_st.h"
#include "feature/nodelist/routerinfo_st.h"
#include "feature/nodelist/routerstatus_st.h"
#include "lib/confmgt/var_type_def_st.h"

/** Return a new empty routerset. */
routerset_t *
routerset_new(void)
{
  routerset_t *result = tor_malloc_zero(sizeof(routerset_t));
  result->list = smartlist_new();
  result->names = strmap_new();
  result->digests = digestmap_new();
  result->policies = smartlist_new();
  result->country_names = smartlist_new();
  result->fragile = 0;
  return result;
}

/** If <b>c</b> is a country code in the form {cc}, return a newly allocated
 * string holding the "cc" part.  Else, return NULL. */
STATIC char *
routerset_get_countryname(const char *c)
{
  char *country;

  if (strlen(c) < 4 || c[0] !='{' || c[3] !='}')
    return NULL;

  country = tor_strndup(c+1, 2);
  tor_strlower(country);
  return country;
}

/** Update the routerset's <b>countries</b> bitarray_t. Called whenever
 * the GeoIP IPv4 database is reloaded.
 */
void
routerset_refresh_countries(routerset_t *target)
{
  int cc;
  bitarray_free(target->countries);

  if (!geoip_is_loaded(AF_INET)) {
    target->countries = NULL;
    target->n_countries = 0;
    return;
  }
  target->n_countries = geoip_get_n_countries();
  target->countries = bitarray_init_zero(target->n_countries);
  SMARTLIST_FOREACH_BEGIN(target->country_names, const char *, country) {
    cc = geoip_get_country(country);
    if (cc >= 0) {
      tor_assert(cc < target->n_countries);
      bitarray_set(target->countries, cc);
    } else {
      log_warn(LD_CONFIG, "Country code '%s' is not recognized.",
          country);
    }
  } SMARTLIST_FOREACH_END(country);
}

/** Parse the string <b>s</b> to create a set of routerset entries, and add
 * them to <b>target</b>.  In log messages, refer to the string as
 * <b>description</b>.  Return 0 on success, -1 on failure.
 *
 * Three kinds of elements are allowed in routersets: nicknames, IP address
 * patterns, and fingerprints.  They may be surrounded by optional space, and
 * must be separated by commas.
 */
int
routerset_parse(routerset_t *target, const char *s, const char *description)
{
  int r = 0;
  int added_countries = 0;
  char *countryname;
  smartlist_t *list = smartlist_new();
  int malformed_list;
  smartlist_split_string(list, s, ",",
                         SPLIT_SKIP_SPACE | SPLIT_IGNORE_BLANK, 0);
  SMARTLIST_FOREACH_BEGIN(list, char *, nick) {
      addr_policy_t *p;
      /* if it doesn't pass our validation, assume it's malformed */
      malformed_list = 1;
      if (is_legal_hexdigest(nick)) {
        char d[DIGEST_LEN];
        if (*nick == '$')
          ++nick;
        log_debug(LD_CONFIG, "Adding identity %s to %s", nick, description);
        base16_decode(d, sizeof(d), nick, HEX_DIGEST_LEN);
        digestmap_set(target->digests, d, (void*)1);
      } else if (is_legal_nickname(nick)) {
        log_debug(LD_CONFIG, "Adding nickname %s to %s", nick, description);
        strmap_set_lc(target->names, nick, (void*)1);
      } else if ((countryname = routerset_get_countryname(nick)) != NULL) {
        log_debug(LD_CONFIG, "Adding country %s to %s", nick,
                  description);
        smartlist_add(target->country_names, countryname);
        added_countries = 1;
      } else if ((strchr(nick,'.') || strchr(nick, ':') ||  strchr(nick, '*'))
                 && (p = router_parse_addr_policy_item_from_string(
                                     nick, ADDR_POLICY_REJECT,
                                     &malformed_list))) {
        /* IPv4 addresses contain '.', IPv6 addresses contain ':',
         * and wildcard addresses contain '*'. */
        log_debug(LD_CONFIG, "Adding address %s to %s", nick, description);
        smartlist_add(target->policies, p);
      } else if (malformed_list) {
        log_warn(LD_CONFIG, "Entry '%s' in %s is malformed. Discarding entire"
                 " list.", nick, description);
        r = -1;
        tor_free(nick);
        SMARTLIST_DEL_CURRENT(list, nick);
      } else {
        log_notice(LD_CONFIG, "Entry '%s' in %s is ignored. Using the"
                   " remainder of the list.", nick, description);
        tor_free(nick);
        SMARTLIST_DEL_CURRENT(list, nick);
      }
  } SMARTLIST_FOREACH_END(nick);
  policy_expand_unspec(&target->policies);
  smartlist_add_all(target->list, list);
  smartlist_free(list);
  if (added_countries)
    routerset_refresh_countries(target);
  return r;
}

/** Add all members of the set <b>source</b> to <b>target</b>. */
void
routerset_union(routerset_t *target, const routerset_t *source)
{
  char *s;
  tor_assert(target);
  if (!source || !source->list)
    return;
  s = routerset_to_string(source);
  routerset_parse(target, s, "other routerset");
  tor_free(s);
}

/** Return true iff <b>set</b> lists only nicknames and digests, and includes
 * no IP ranges or countries. */
int
routerset_is_list(const routerset_t *set)
{
  return smartlist_len(set->country_names) == 0 &&
    smartlist_len(set->policies) == 0;
}

/** Return true iff we need a GeoIP IP-to-country database to make sense of
 * <b>set</b>. */
int
routerset_needs_geoip(const routerset_t *set)
{
  return set && smartlist_len(set->country_names);
}

/** Return true iff there are no entries in <b>set</b>. */
int
routerset_is_empty(const routerset_t *set)
{
  return !set || smartlist_len(set->list) == 0;
}

/** Return the number of entries in <b>set</b>. This does NOT return a
 * negative value. */
int
routerset_len(const routerset_t *set)
{
  if (!set) {
    return 0;
  }
  return smartlist_len(set->list);
}

/** Helper.  Return true iff <b>set</b> contains a router based on the other
 * provided fields.  Return higher values for more specific subentries: a
 * single router is more specific than an address range of routers, which is
 * more specific in turn than a country code.
 *
 * (If country is -1, then we take the country
 * from addr.) */
static int
routerset_contains2(const routerset_t *set, const tor_addr_t *addr,
                    uint16_t orport, const tor_addr_t *addr2,
                    uint16_t orport2, const char *nickname,
                    const char *id_digest, country_t country)
{
  if (!set || !set->list)
    return 0;
  if (nickname && strmap_get_lc(set->names, nickname))
    return 4;
  if (id_digest && digestmap_get(set->digests, id_digest))
    return 4;
  if (addr && compare_tor_addr_to_addr_policy(addr, orport, set->policies)
      == ADDR_POLICY_REJECTED)
    return 3;
  if (addr2 && compare_tor_addr_to_addr_policy(addr2, orport2, set->policies)
      == ADDR_POLICY_REJECTED)
    return 3;
  if (set->countries) {
    if (country < 0 && addr)
      country = geoip_get_country_by_addr(addr);

    if (country >= 0 && country < set->n_countries &&
        bitarray_is_set(set->countries, country))
      return 2;
  }
  return 0;
}

/** Helper. Like routerset_contains2() but for a single IP/port combo.
 */
STATIC int
routerset_contains(const routerset_t *set, const tor_addr_t *addr,
                   uint16_t orport, const char *nickname,
                   const char *id_digest, country_t country)
{
  return routerset_contains2(set, addr, orport, NULL, 0,
                             nickname, id_digest, country);
}

/** If *<b>setp</b> includes at least one country code, or if
 * <b>only_some_cc_set</b> is 0, add the ?? and A1 country codes to
 * *<b>setp</b>, creating it as needed.  Return true iff *<b>setp</b> changed.
 */
int
routerset_add_unknown_ccs(routerset_t **setp, int only_if_some_cc_set)
{
  routerset_t *set;
  int add_unknown, add_a1;
  if (only_if_some_cc_set) {
    if (!*setp || smartlist_len((*setp)->country_names) == 0)
      return 0;
  }
  if (!*setp)
    *setp = routerset_new();

  set = *setp;

  add_unknown = ! smartlist_contains_string_case(set->country_names, "??") &&
    geoip_get_country("??") >= 0;
  add_a1 = ! smartlist_contains_string_case(set->country_names, "a1") &&
    geoip_get_country("A1") >= 0;

  if (add_unknown) {
    smartlist_add_strdup(set->country_names, "??");
    smartlist_add_strdup(set->list, "{??}");
  }
  if (add_a1) {
    smartlist_add_strdup(set->country_names, "a1");
    smartlist_add_strdup(set->list, "{a1}");
  }

  if (add_unknown || add_a1) {
    routerset_refresh_countries(set);
    return 1;
  }
  return 0;
}

/** Return true iff we can tell that <b>ei</b> is a member of <b>set</b>. */
int
routerset_contains_extendinfo(const routerset_t *set, const extend_info_t *ei)
{
  const tor_addr_port_t *ap1 = NULL, *ap2 = NULL;
  if (! tor_addr_is_null(&ei->orports[0].addr))
    ap1 = &ei->orports[0];
  if (! tor_addr_is_null(&ei->orports[1].addr))
    ap2 = &ei->orports[1];
  return routerset_contains2(set,
                             ap1 ? &ap1->addr : NULL,
                             ap1 ? ap1->port : 0,
                             ap2 ? &ap2->addr : NULL,
                             ap2 ? ap2->port : 0,
                             ei->nickname,
                             ei->identity_digest,
                             -1 /*country*/);
}

/** Return true iff <b>ri</b> is in <b>set</b>.  If country is <b>-1</b>, we
 * look up the country. */
int
routerset_contains_router(const routerset_t *set, const routerinfo_t *ri,
                          country_t country)
{
  return routerset_contains2(set, &ri->ipv4_addr, ri->ipv4_orport,
                             &ri->ipv6_addr, ri->ipv6_orport, ri->nickname,
                             ri->cache_info.identity_digest, country);
}

/** Return true iff <b>rs</b> is in <b>set</b>.  If country is <b>-1</b>, we
 * look up the country. */
int
routerset_contains_routerstatus(const routerset_t *set,
                                const routerstatus_t *rs,
                                country_t country)
{
  return routerset_contains(set,
                            &rs->ipv4_addr,
                            rs->ipv4_orport,
                            rs->nickname,
                            rs->identity_digest,
                            country);
}

/** Return true iff <b>node</b> is in <b>set</b>. */
int
routerset_contains_node(const routerset_t *set, const node_t *node)
{
  if (node->rs)
    return routerset_contains_routerstatus(set, node->rs, node->country);
  else if (node->ri)
    return routerset_contains_router(set, node->ri, node->country);
  else
    return 0;
}

/** Return true iff <b>routerset</b> contains the bridge <b>bridge</b>. */
int
routerset_contains_bridge(const routerset_t *set, const bridge_info_t *bridge)
{
  const char *id = (const char*)bridge_get_rsa_id_digest(bridge);
  const tor_addr_port_t *addrport = bridge_get_addr_port(bridge);

  tor_assert(addrport);
  return routerset_contains(set, &addrport->addr, addrport->port,
                            NULL, id, -1);
}

/** Add every known node_t that is a member of <b>routerset</b> to
 * <b>out</b>, but never add any that are part of <b>excludeset</b>.
 * If <b>running_only</b>, only add the running ones. */
void
routerset_get_all_nodes(smartlist_t *out, const routerset_t *routerset,
                        const routerset_t *excludeset, int running_only)
{
  tor_assert(out);
  if (!routerset || !routerset->list)
    return;

  if (routerset_is_list(routerset)) {
    /* No routers are specified by type; all are given by name or digest.
     * we can do a lookup in O(len(routerset)). */
    SMARTLIST_FOREACH(routerset->list, const char *, name, {
        const node_t *node = node_get_by_nickname(name, 0);
        if (node) {
          if (!running_only || node->is_running)
            if (!routerset_contains_node(excludeset, node))
              smartlist_add(out, (void*)node);
        }
    });
  } else {
    /* We need to iterate over the routerlist to get all the ones of the
     * right kind. */
    const smartlist_t *nodes = nodelist_get_list();
    SMARTLIST_FOREACH(nodes, const node_t *, node, {
        if (running_only && !node->is_running)
          continue;
        if (routerset_contains_node(routerset, node) &&
            !routerset_contains_node(excludeset, node))
          smartlist_add(out, (void*)node);
    });
  }
}

/** Remove every node_t from <b>lst</b> that is in <b>routerset</b>. */
void
routerset_subtract_nodes(smartlist_t *lst, const routerset_t *routerset)
{
  tor_assert(lst);
  if (!routerset)
    return;
  SMARTLIST_FOREACH(lst, const node_t *, node, {
      if (routerset_contains_node(routerset, node)) {
        //log_debug(LD_DIR, "Subtracting %s",r->nickname);
        SMARTLIST_DEL_CURRENT(lst, node);
      }
    });
}

/** Return a new string that when parsed by routerset_parse_string() will
 * yield <b>set</b>. */
char *
routerset_to_string(const routerset_t *set)
{
  if (!set || !set->list)
    return tor_strdup("");
  return smartlist_join_strings(set->list, ",", 0, NULL);
}

/** Helper: return true iff old and new are both NULL, or both non-NULL
 * equal routersets. */
int
routerset_equal(const routerset_t *old, const routerset_t *new)
{
  if (routerset_is_empty(old) && routerset_is_empty(new)) {
    /* Two empty sets are equal */
    return 1;
  } else if (routerset_is_empty(old) || routerset_is_empty(new)) {
    /* An empty set is equal to nothing else. */
    return 0;
  }
  tor_assert(old != NULL);
  tor_assert(new != NULL);

  if (smartlist_len(old->list) != smartlist_len(new->list))
    return 0;

  SMARTLIST_FOREACH(old->list, const char *, cp1, {
    const char *cp2 = smartlist_get(new->list, cp1_sl_idx);
    if (strcmp(cp1, cp2))
      return 0;
  });

  return 1;
}

/** Free all storage held in <b>routerset</b>. */
void
routerset_free_(routerset_t *routerset)
{
  if (!routerset)
    return;

  SMARTLIST_FOREACH(routerset->list, char *, cp, tor_free(cp));
  smartlist_free(routerset->list);
  SMARTLIST_FOREACH(routerset->policies, addr_policy_t *, p,
                    addr_policy_free(p));
  smartlist_free(routerset->policies);
  SMARTLIST_FOREACH(routerset->country_names, char *, cp, tor_free(cp));
  smartlist_free(routerset->country_names);

  strmap_free(routerset->names, NULL);
  digestmap_free(routerset->digests, NULL);
  bitarray_free(routerset->countries);
  tor_free(routerset);
}

/**
 * config helper: parse a routerset-typed variable.
 *
 * Takes as input as a single line in <b>line</b>; writes its results into a
 * routerset_t** passed as <b>target</b>.  On success return 0; on failure
 * return -1 and store an error message into *<b>errmsg</b>.
 **/
/*
 * Warning: For this type, the default value (NULL) and "" are sometimes
 * considered different values.  That is generally risky, and best avoided for
 * other types in the future.  For cases where we want the default to be "all
 * routers" (like EntryNodes) we should add a new routerset value indicating
 * "all routers" (see #31908)
 */
static int
routerset_kv_parse(void *target, const config_line_t *line, char **errmsg,
                  const void *params)
{
  (void)params;
  routerset_t **lines = target;

  if (*lines && (*lines)->fragile) {
    if (line->command == CONFIG_LINE_APPEND) {
      (*lines)->fragile = 0;
    } else {
      routerset_free(*lines); // Represent empty sets as NULL
    }
  }

  int ret;
  routerset_t *rs = routerset_new();
  if (routerset_parse(rs, line->value, line->key) < 0) {
    *errmsg = tor_strdup("Invalid router list.");
    ret = -1;
  } else {
    if (!routerset_is_empty(rs)) {
      if (!*lines) {
        *lines = routerset_new();
      }
      routerset_union(*lines, rs);
    }
    ret = 0;
  }
  routerset_free(rs);
  return ret;
}

/**
 * config helper: encode a routerset-typed variable.
 *
 * Return a newly allocated string containing the value of the
 * routerset_t** passed as <b>value</b>.
 */
static char *
routerset_encode(const void *value, const void *params)
{
  (void)params;
  const routerset_t **p = (const routerset_t**)value;
  return routerset_to_string(*p);
}

/**
 * config helper: free and clear a routerset-typed variable.
 *
 * Clear the routerset_t** passed as <b>value</b>.
 */
static void
routerset_clear(void *value, const void *params)
{
  (void)params;
  routerset_t **p = (routerset_t**)value;
  routerset_free(*p); // sets *p to NULL.
}

/**
 * config helper: copy a routerset-typed variable.
 *
 * Takes it input from a routerset_t** in <b>src</b>; writes its output to a
 * routerset_t** in <b>dest</b>.  Returns 0 on success, -1 on (impossible)
 * failure.
 **/
static int
routerset_copy(void *dest, const void *src, const void *params)
{
  (void)params;
  routerset_t **output = (routerset_t**)dest;
  const routerset_t *input = *(routerset_t**)src;
  routerset_free(*output); // sets *output to NULL
  if (! routerset_is_empty(input)) {
    *output = routerset_new();
    routerset_union(*output, input);
  }
  return 0;
}

static void
routerset_mark_fragile(void *target, const void *params)
{
  (void)params;
  routerset_t **ptr = (routerset_t **)target;
  if (*ptr)
    (*ptr)->fragile = 1;
}

/**
 * Function table to implement a routerset_t-based configuration type.
 **/
static const var_type_fns_t routerset_type_fns = {
  .kv_parse = routerset_kv_parse,
  .encode = routerset_encode,
  .clear = routerset_clear,
  .copy = routerset_copy,
  .mark_fragile = routerset_mark_fragile,
};

/**
 * Definition of a routerset_t-based configuration type.
 *
 * Values are mapped to and from strings using the format defined in
 * routerset_parse(): nicknames, IP address patterns, and fingerprints--with
 * optional space, separated by commas.
 *
 * Empty sets are represented as NULL.
 **/
const var_type_def_t ROUTERSET_type_defn = {
  .name = "RouterList",
  .fns = &routerset_type_fns,
  .flags = CFLG_NOREPLACE
};

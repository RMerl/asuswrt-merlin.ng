/* Copyright (c) 2007-2020, The Tor Project, Inc. */
/* See LICENSE for licensing information */

/**
 * \file geoip.c
 * \brief Functions related to maintaining an IP-to-country database;
 * to summarizing client connections by country to entry guards, bridges,
 * and directory servers; and for statistics on answering network status
 * requests.
 *
 * There are two main kinds of functions in this module: geoip functions,
 * which map groups of IPv4 and IPv6 addresses to country codes, and
 * statistical functions, which collect statistics about different kinds of
 * per-country usage.
 *
 * The geoip lookup tables are implemented as sorted lists of disjoint address
 * ranges, each mapping to a singleton geoip_country_t.  These country objects
 * are also indexed by their names in a hashtable.
 *
 * The tables are populated from disk at startup by the geoip_load_file()
 * function.  For more information on the file format they read, see that
 * function.  See the scripts and the README file in src/config for more
 * information about how those files are generated.
 *
 * Tor uses GeoIP information in order to implement user requests (such as
 * ExcludeNodes {cc}), and to keep track of how much usage relays are getting
 * for each country.
 */

#define GEOIP_PRIVATE
#include "lib/geoip/geoip.h"
#include "lib/container/map.h"
#include "lib/container/order.h"
#include "lib/container/smartlist.h"
#include "lib/crypt_ops/crypto_digest.h"
#include "lib/ctime/di_ops.h"
#include "lib/encoding/binascii.h"
#include "lib/fs/files.h"
#include "lib/log/escape.h"
#include "lib/malloc/malloc.h"
#include "lib/net/address.h" //????
#include "lib/net/inaddr.h"
#include "lib/string/compat_ctype.h"
#include "lib/string/compat_string.h"
#include "lib/string/scanf.h"
#include "lib/string/util_string.h"

#include <stdio.h>
#include <string.h>

static void init_geoip_countries(void);

/** An entry from the GeoIP IPv4 file: maps an IPv4 range to a country. */
typedef struct geoip_ipv4_entry_t {
  uint32_t ip_low; /**< The lowest IP in the range, in host order */
  uint32_t ip_high; /**< The highest IP in the range, in host order */
  intptr_t country; /**< An index into geoip_countries */
} geoip_ipv4_entry_t;

/** An entry from the GeoIP IPv6 file: maps an IPv6 range to a country. */
typedef struct geoip_ipv6_entry_t {
  struct in6_addr ip_low; /**< The lowest IP in the range, in host order */
  struct in6_addr ip_high; /**< The highest IP in the range, in host order */
  intptr_t country; /**< An index into geoip_countries */
} geoip_ipv6_entry_t;

/** A list of geoip_country_t */
static smartlist_t *geoip_countries = NULL;
/** A map from lowercased country codes to their position in geoip_countries.
 * The index is encoded in the pointer, and 1 is added so that NULL can mean
 * not found. */
static strmap_t *country_idxplus1_by_lc_code = NULL;
/** List of all known geoip_ipv4_entry_t sorted
 * by their respective ip_low values. */
static smartlist_t *geoip_ipv4_entries = NULL;
/** List of all known geoip_ipv6_entry_t, sorted by their respective
 * ip_low values. */
static smartlist_t *geoip_ipv6_entries = NULL;

/** SHA1 digest of the IPv4 GeoIP file to include in extra-info
 * descriptors. */
static char geoip_digest[DIGEST_LEN];
/** SHA1 digest of the IPv6 GeoIP file to include in extra-info
 * descriptors. */
static char geoip6_digest[DIGEST_LEN];

/** Return a list of geoip_country_t for all known countries. */
const smartlist_t *
geoip_get_countries(void)
{
  if (geoip_countries == NULL) {
    init_geoip_countries();
  }
  return geoip_countries;
}

/** Return the index of the <b>country</b>'s entry in the GeoIP
 * country list if it is a valid 2-letter country code, otherwise
 * return -1. */
MOCK_IMPL(country_t,
geoip_get_country,(const char *country))
{
  void *idxplus1_;
  intptr_t idx;

  idxplus1_ = strmap_get_lc(country_idxplus1_by_lc_code, country);
  if (!idxplus1_)
    return -1;

  idx = ((uintptr_t)idxplus1_)-1;
  return (country_t)idx;
}

/** Add an entry to a GeoIP table, mapping all IP addresses between <b>low</b>
 * and <b>high</b>, inclusive, to the 2-letter country code <b>country</b>. */
static void
geoip_add_entry(const tor_addr_t *low, const tor_addr_t *high,
                const char *country)
{
  intptr_t idx;
  void *idxplus1_;

  IF_BUG_ONCE(tor_addr_family(low) != tor_addr_family(high))
    return;
  IF_BUG_ONCE(tor_addr_compare(high, low, CMP_EXACT) < 0)
    return;

  idxplus1_ = strmap_get_lc(country_idxplus1_by_lc_code, country);

  if (!idxplus1_) {
    geoip_country_t *c = tor_malloc_zero(sizeof(geoip_country_t));
    strlcpy(c->countrycode, country, sizeof(c->countrycode));
    tor_strlower(c->countrycode);
    smartlist_add(geoip_countries, c);
    idx = smartlist_len(geoip_countries) - 1;
    strmap_set_lc(country_idxplus1_by_lc_code, country, (void*)(idx+1));
  } else {
    idx = ((uintptr_t)idxplus1_)-1;
  }
  {
    geoip_country_t *c = smartlist_get(geoip_countries, (int)idx);
    tor_assert(!strcasecmp(c->countrycode, country));
  }

  if (tor_addr_family(low) == AF_INET) {
    geoip_ipv4_entry_t *ent = tor_malloc_zero(sizeof(geoip_ipv4_entry_t));
    ent->ip_low = tor_addr_to_ipv4h(low);
    ent->ip_high = tor_addr_to_ipv4h(high);
    ent->country = idx;
    smartlist_add(geoip_ipv4_entries, ent);
  } else if (tor_addr_family(low) == AF_INET6) {
    geoip_ipv6_entry_t *ent = tor_malloc_zero(sizeof(geoip_ipv6_entry_t));
    ent->ip_low = *tor_addr_to_in6_assert(low);
    ent->ip_high = *tor_addr_to_in6_assert(high);
    ent->country = idx;
    smartlist_add(geoip_ipv6_entries, ent);
  }
}

/** Add an entry to the GeoIP table indicated by <b>family</b>,
 * parsing it from <b>line</b>. The format is as for geoip_load_file(). */
STATIC int
geoip_parse_entry(const char *line, sa_family_t family)
{
  tor_addr_t low_addr, high_addr;
  char c[3];
  char *country = NULL;

  if (!geoip_countries)
    init_geoip_countries();
  if (family == AF_INET) {
    if (!geoip_ipv4_entries)
      geoip_ipv4_entries = smartlist_new();
  } else if (family == AF_INET6) {
    if (!geoip_ipv6_entries)
      geoip_ipv6_entries = smartlist_new();
  } else {
    log_warn(LD_GENERAL, "Unsupported family: %d", family);
    return -1;
  }

  while (TOR_ISSPACE(*line))
    ++line;
  if (*line == '#')
    return 0;

  char buf[512];
  if (family == AF_INET) {
    unsigned int low, high;
    if (tor_sscanf(line,"%u,%u,%2s", &low, &high, c) == 3 ||
        tor_sscanf(line,"\"%u\",\"%u\",\"%2s\",", &low, &high, c) == 3) {
      tor_addr_from_ipv4h(&low_addr, low);
      tor_addr_from_ipv4h(&high_addr, high);
    } else
      goto fail;
    country = c;
  } else {                      /* AF_INET6 */
    char *low_str, *high_str;
    struct in6_addr low, high;
    char *strtok_state;
    strlcpy(buf, line, sizeof(buf));
    low_str = tor_strtok_r(buf, ",", &strtok_state);
    if (!low_str)
      goto fail;
    high_str = tor_strtok_r(NULL, ",", &strtok_state);
    if (!high_str)
      goto fail;
    country = tor_strtok_r(NULL, "\n", &strtok_state);
    if (!country)
      goto fail;
    if (strlen(country) != 2)
      goto fail;
    if (tor_inet_pton(AF_INET6, low_str, &low) <= 0)
      goto fail;
    tor_addr_from_in6(&low_addr, &low);
    if (tor_inet_pton(AF_INET6, high_str, &high) <= 0)
      goto fail;
    tor_addr_from_in6(&high_addr, &high);
  }
  geoip_add_entry(&low_addr, &high_addr, country);
  return 0;

  fail:
  log_warn(LD_GENERAL, "Unable to parse line from GEOIP %s file: %s",
           family == AF_INET ? "IPv4" : "IPv6", escaped(line));
  return -1;
}

/** Sorting helper: return -1, 1, or 0 based on comparison of two
 * geoip_ipv4_entry_t */
static int
geoip_ipv4_compare_entries_(const void **_a, const void **_b)
{
  const geoip_ipv4_entry_t *a = *_a, *b = *_b;
  if (a->ip_low < b->ip_low)
    return -1;
  else if (a->ip_low > b->ip_low)
    return 1;
  else
    return 0;
}

/** bsearch helper: return -1, 1, or 0 based on comparison of an IP (a pointer
 * to a uint32_t in host order) to a geoip_ipv4_entry_t */
static int
geoip_ipv4_compare_key_to_entry_(const void *_key, const void **_member)
{
  /* No alignment issue here, since _key really is a pointer to uint32_t */
  const uint32_t addr = *(uint32_t *)_key;
  const geoip_ipv4_entry_t *entry = *_member;
  if (addr < entry->ip_low)
    return -1;
  else if (addr > entry->ip_high)
    return 1;
  else
    return 0;
}

/** Sorting helper: return -1, 1, or 0 based on comparison of two
 * geoip_ipv6_entry_t */
static int
geoip_ipv6_compare_entries_(const void **_a, const void **_b)
{
  const geoip_ipv6_entry_t *a = *_a, *b = *_b;
  return fast_memcmp(a->ip_low.s6_addr, b->ip_low.s6_addr,
                     sizeof(struct in6_addr));
}

/** bsearch helper: return -1, 1, or 0 based on comparison of an IPv6
 * (a pointer to a in6_addr) to a geoip_ipv6_entry_t */
static int
geoip_ipv6_compare_key_to_entry_(const void *_key, const void **_member)
{
  const struct in6_addr *addr = (struct in6_addr *)_key;
  const geoip_ipv6_entry_t *entry = *_member;

  if (fast_memcmp(addr->s6_addr, entry->ip_low.s6_addr,
             sizeof(struct in6_addr)) < 0)
    return -1;
  else if (fast_memcmp(addr->s6_addr, entry->ip_high.s6_addr,
                  sizeof(struct in6_addr)) > 0)
    return 1;
  else
    return 0;
}

/** Set up a new list of geoip countries with no countries (yet) set in it,
 * except for the unknown country.
 */
static void
init_geoip_countries(void)
{
  geoip_country_t *geoip_unresolved;
  geoip_countries = smartlist_new();
  /* Add a geoip_country_t for requests that could not be resolved to a
   * country as first element (index 0) to geoip_countries. */
  geoip_unresolved = tor_malloc_zero(sizeof(geoip_country_t));
  strlcpy(geoip_unresolved->countrycode, "??",
          sizeof(geoip_unresolved->countrycode));
  smartlist_add(geoip_countries, geoip_unresolved);
  country_idxplus1_by_lc_code = strmap_new();
  strmap_set_lc(country_idxplus1_by_lc_code, "??", (void*)(1));
}

/** Clear appropriate GeoIP database, based on <b>family</b>, and
 * reload it from the file <b>filename</b>. Return 0 on success, -1 on
 * failure.
 *
 * Recognized line formats for IPv4 are:
 *   INTIPLOW,INTIPHIGH,CC
 * and
 *   "INTIPLOW","INTIPHIGH","CC","CC3","COUNTRY NAME"
 * where INTIPLOW and INTIPHIGH are IPv4 addresses encoded as 4-byte unsigned
 * integers, and CC is a country code.
 *
 * Recognized line format for IPv6 is:
 *   IPV6LOW,IPV6HIGH,CC
 * where IPV6LOW and IPV6HIGH are IPv6 addresses and CC is a country code.
 *
 * It also recognizes, and skips over, blank lines and lines that start
 * with '#' (comments).
 */
int
geoip_load_file(sa_family_t family, const char *filename, int severity)
{
  FILE *f;
  crypto_digest_t *geoip_digest_env = NULL;

  tor_assert(family == AF_INET || family == AF_INET6);

  if (!(f = tor_fopen_cloexec(filename, "r"))) {
    log_fn(severity, LD_GENERAL, "Failed to open GEOIP file %s.",
           filename);
    return -1;
  }
  if (!geoip_countries)
    init_geoip_countries();

  if (family == AF_INET) {
    if (geoip_ipv4_entries) {
      SMARTLIST_FOREACH(geoip_ipv4_entries, geoip_ipv4_entry_t *, e,
                        tor_free(e));
      smartlist_free(geoip_ipv4_entries);
    }
    geoip_ipv4_entries = smartlist_new();
  } else { /* AF_INET6 */
    if (geoip_ipv6_entries) {
      SMARTLIST_FOREACH(geoip_ipv6_entries, geoip_ipv6_entry_t *, e,
                        tor_free(e));
      smartlist_free(geoip_ipv6_entries);
    }
    geoip_ipv6_entries = smartlist_new();
  }
  geoip_digest_env = crypto_digest_new();

  log_notice(LD_GENERAL, "Parsing GEOIP %s file %s.",
             (family == AF_INET) ? "IPv4" : "IPv6", filename);
  while (!feof(f)) {
    char buf[512];
    if (fgets(buf, (int)sizeof(buf), f) == NULL)
      break;
    crypto_digest_add_bytes(geoip_digest_env, buf, strlen(buf));
    /* FFFF track full country name. */
    geoip_parse_entry(buf, family);
  }
  /*XXXX abort and return -1 if no entries/illformed?*/
  fclose(f);

  /* Sort list and remember file digests so that we can include it in
   * our extra-info descriptors. */
  if (family == AF_INET) {
    smartlist_sort(geoip_ipv4_entries, geoip_ipv4_compare_entries_);
    crypto_digest_get_digest(geoip_digest_env, geoip_digest, DIGEST_LEN);
  } else {
    /* AF_INET6 */
    smartlist_sort(geoip_ipv6_entries, geoip_ipv6_compare_entries_);
    crypto_digest_get_digest(geoip_digest_env, geoip6_digest, DIGEST_LEN);
  }
  crypto_digest_free(geoip_digest_env);

  return 0;
}

/** Given an IP address in host order, return a number representing the
 * country to which that address belongs, -1 for "No geoip information
 * available", or 0 for the 'unknown country'.  The return value will always
 * be less than geoip_get_n_countries().  To decode it, call
 * geoip_get_country_name().
 */
int
geoip_get_country_by_ipv4(uint32_t ipaddr)
{
  geoip_ipv4_entry_t *ent;
  if (!geoip_ipv4_entries)
    return -1;
  ent = smartlist_bsearch(geoip_ipv4_entries, &ipaddr,
                          geoip_ipv4_compare_key_to_entry_);
  return ent ? (int)ent->country : 0;
}

/** Given an IPv6 address, return a number representing the country to
 * which that address belongs, -1 for "No geoip information available", or
 * 0 for the 'unknown country'.  The return value will always be less than
 * geoip_get_n_countries().  To decode it, call geoip_get_country_name().
 */
int
geoip_get_country_by_ipv6(const struct in6_addr *addr)
{
  geoip_ipv6_entry_t *ent;

  if (!geoip_ipv6_entries)
    return -1;
  ent = smartlist_bsearch(geoip_ipv6_entries, addr,
                          geoip_ipv6_compare_key_to_entry_);
  return ent ? (int)ent->country : 0;
}

/** Given an IP address, return a number representing the country to which
 * that address belongs, -1 for "No geoip information available", or 0 for
 * the 'unknown country'.  The return value will always be less than
 * geoip_get_n_countries().  To decode it, call geoip_get_country_name().
 */
MOCK_IMPL(int,
geoip_get_country_by_addr,(const tor_addr_t *addr))
{
  if (tor_addr_family(addr) == AF_INET) {
    return geoip_get_country_by_ipv4(tor_addr_to_ipv4h(addr));
  } else if (tor_addr_family(addr) == AF_INET6) {
    return geoip_get_country_by_ipv6(tor_addr_to_in6(addr));
  } else {
    return -1;
  }
}

/** Return the number of countries recognized by the GeoIP country list. */
MOCK_IMPL(int,
geoip_get_n_countries,(void))
{
  if (!geoip_countries)
    init_geoip_countries();
  return (int) smartlist_len(geoip_countries);
}

/** Return the two-letter country code associated with the number <b>num</b>,
 * or "??" for an unknown value. */
const char *
geoip_get_country_name(country_t num)
{
  if (geoip_countries && num >= 0 && num < smartlist_len(geoip_countries)) {
    geoip_country_t *c = smartlist_get(geoip_countries, num);
    return c->countrycode;
  } else
    return "??";
}

/** Return true iff we have loaded a GeoIP database.*/
MOCK_IMPL(int,
geoip_is_loaded,(sa_family_t family))
{
  tor_assert(family == AF_INET || family == AF_INET6);
  if (geoip_countries == NULL)
    return 0;
  if (family == AF_INET)
    return geoip_ipv4_entries != NULL;
  else                          /* AF_INET6 */
    return geoip_ipv6_entries != NULL;
}

/** Return the hex-encoded SHA1 digest of the loaded GeoIP file. The
 * result does not need to be deallocated, but will be overwritten by the
 * next call of hex_str(). */
const char *
geoip_db_digest(sa_family_t family)
{
  tor_assert(family == AF_INET || family == AF_INET6);
  if (family == AF_INET)
    return hex_str(geoip_digest, DIGEST_LEN);
  else                          /* AF_INET6 */
    return hex_str(geoip6_digest, DIGEST_LEN);
}

/** Release all storage held by the GeoIP databases and country list. */
STATIC void
clear_geoip_db(void)
{
  if (geoip_countries) {
    SMARTLIST_FOREACH(geoip_countries, geoip_country_t *, c, tor_free(c));
    smartlist_free(geoip_countries);
  }

  strmap_free(country_idxplus1_by_lc_code, NULL);
  if (geoip_ipv4_entries) {
    SMARTLIST_FOREACH(geoip_ipv4_entries, geoip_ipv4_entry_t *, ent,
                      tor_free(ent));
    smartlist_free(geoip_ipv4_entries);
  }
  if (geoip_ipv6_entries) {
    SMARTLIST_FOREACH(geoip_ipv6_entries, geoip_ipv6_entry_t *, ent,
                      tor_free(ent));
    smartlist_free(geoip_ipv6_entries);
  }
  geoip_countries = NULL;
  country_idxplus1_by_lc_code = NULL;
  geoip_ipv4_entries = NULL;
  geoip_ipv6_entries = NULL;
}

/** Release all storage held in this file. */
void
geoip_free_all(void)
{
  clear_geoip_db();

  memset(geoip_digest, 0, sizeof(geoip_digest));
  memset(geoip6_digest, 0, sizeof(geoip6_digest));
}

/* Copyright (c) 2016-2021, The Tor Project, Inc. */
/* See LICENSE for licensing information */

/**
 * \file protover.c
 * \brief Versioning information for different pieces of the Tor protocol.
 *
 * Starting in version 0.2.9.3-alpha, Tor places separate version numbers on
 * each of the different components of its protocol. Relays use these numbers
 * to advertise what versions of the protocols they can support, and clients
 * use them to find what they can ask a given relay to do.  Authorities vote
 * on the supported protocol versions for each relay, and also vote on the
 * which protocols you should have to support in order to be on the Tor
 * network. All Tor instances use these required/recommended protocol versions
 * to tell what level of support for recent protocols each relay has, and
 * to decide whether they should be running given their current protocols.
 *
 * The main advantage of these protocol versions numbers over using Tor
 * version numbers is that they allow different implementations of the Tor
 * protocols to develop independently, without having to claim compatibility
 * with specific versions of Tor.
 **/

#define PROTOVER_PRIVATE

#include "core/or/or.h"
#include "core/or/protover.h"
#include "core/or/versions.h"
#include "lib/tls/tortls.h"

static const smartlist_t *get_supported_protocol_list(void);
static int protocol_list_contains(const smartlist_t *protos,
                                  protocol_type_t pr, uint32_t ver);
static const proto_entry_t *find_entry_by_name(const smartlist_t *protos,
                                               const char *name);

/** Mapping between protocol type string and protocol type. */
/// C_RUST_COUPLED: src/rust/protover/protover.rs `PROTOCOL_NAMES`
static const struct {
  protocol_type_t protover_type;
  const char *name;
/* If you add a new protocol here, you probably also want to add
 * parsing for it in summarize_protover_flags(), so that it has a
 * summary flag in routerstatus_t */
} PROTOCOL_NAMES[] = {
  { PRT_LINK, "Link" },
  { PRT_LINKAUTH, "LinkAuth" },
  { PRT_RELAY, "Relay" },
  { PRT_DIRCACHE, "DirCache" },
  { PRT_HSDIR, "HSDir" },
  { PRT_HSINTRO, "HSIntro" },
  { PRT_HSREND, "HSRend" },
  { PRT_DESC, "Desc" },
  { PRT_MICRODESC, "Microdesc"},
  { PRT_PADDING, "Padding"},
  { PRT_CONS, "Cons" },
  { PRT_FLOWCTRL, "FlowCtrl"},
  { PRT_CONFLUX, "Conflux"},
};

#define N_PROTOCOL_NAMES ARRAY_LENGTH(PROTOCOL_NAMES)

/* Maximum allowed length of any single subprotocol name. */
// C_RUST_COUPLED: src/rust/protover/protover.rs
//                 `MAX_PROTOCOL_NAME_LENGTH`
static const unsigned MAX_PROTOCOL_NAME_LENGTH = 100;

/**
 * Given a protocol_type_t, return the corresponding string used in
 * descriptors.
 */
STATIC const char *
protocol_type_to_str(protocol_type_t pr)
{
  unsigned i;
  for (i=0; i < N_PROTOCOL_NAMES; ++i) {
    if (PROTOCOL_NAMES[i].protover_type == pr)
      return PROTOCOL_NAMES[i].name;
  }
  /* LCOV_EXCL_START */
  tor_assert_nonfatal_unreached_once();
  return "UNKNOWN";
  /* LCOV_EXCL_STOP */
}

/**
 * Release all space held by a single proto_entry_t structure
 */
STATIC void
proto_entry_free_(proto_entry_t *entry)
{
  if (!entry)
    return;
  tor_free(entry->name);
  tor_free(entry);
}

/** The largest possible protocol version. */
#define MAX_PROTOCOL_VERSION (63)

/**
 * Given a string <b>s</b> and optional end-of-string pointer
 * <b>end_of_range</b>, parse the protocol range and store it in
 * <b>low_out</b> and <b>high_out</b>.  A protocol range has the format U, or
 * U-U, where U is an unsigned integer between 0 and 63 inclusive.
 */
static int
parse_version_range(const char *s, const char *end_of_range,
                    uint32_t *low_out, uint32_t *high_out)
{
  uint32_t low, high;
  char *next = NULL;
  int ok;

  tor_assert(high_out);
  tor_assert(low_out);

  if (BUG(!end_of_range))
    end_of_range = s + strlen(s); // LCOV_EXCL_LINE

  /* A range must start with a digit. */
  if (!TOR_ISDIGIT(*s)) {
    goto error;
  }

  /* Note that this wouldn't be safe if we didn't know that eventually,
   * we'd hit a NUL */
  low = (uint32_t) tor_parse_ulong(s, 10, 0, MAX_PROTOCOL_VERSION, &ok, &next);
  if (!ok)
    goto error;
  if (next > end_of_range)
    goto error;
  if (next == end_of_range) {
    high = low;
    goto done;
  }

  if (*next != '-')
    goto error;
  s = next+1;

  /* ibid */
  if (!TOR_ISDIGIT(*s)) {
    goto error;
  }
  high = (uint32_t) tor_parse_ulong(s, 10, 0,
                                    MAX_PROTOCOL_VERSION, &ok, &next);
  if (!ok)
    goto error;
  if (next != end_of_range)
    goto error;

  if (low > high)
    goto error;

 done:
  *high_out = high;
  *low_out = low;
  return 0;

 error:
  return -1;
}

static int
is_valid_keyword(const char *s, size_t n)
{
  for (size_t i = 0; i < n; i++) {
    if (!TOR_ISALNUM(s[i]) && s[i] != '-')
      return 0;
  }
  return 1;
}

/** The x'th bit in a bitmask. */
#define BIT(x) (UINT64_C(1)<<(x))

/**
 * Return a bitmask so that bits 'low' through 'high' inclusive are set,
 * and all other bits are cleared.
 **/
static uint64_t
bitmask_for_range(uint32_t low, uint32_t high)
{
  uint64_t mask = ~(uint64_t)0;
  mask <<= 63 - high;
  mask >>= 63 - high + low;
  mask <<= low;
  return mask;
}

/** Parse a single protocol entry from <b>s</b> up to an optional
 * <b>end_of_entry</b> pointer, and return that protocol entry. Return NULL
 * on error.
 *
 * A protocol entry has a keyword, an = sign, and zero or more ranges. */
static proto_entry_t *
parse_single_entry(const char *s, const char *end_of_entry)
{
  proto_entry_t *out = tor_malloc_zero(sizeof(proto_entry_t));
  const char *equals;

  if (BUG (!end_of_entry))
    end_of_entry = s + strlen(s); // LCOV_EXCL_LINE

  /* There must be an =. */
  equals = memchr(s, '=', end_of_entry - s);
  if (!equals)
    goto error;

  /* The name must be nonempty */
  if (equals == s)
    goto error;

  /* The name must not be longer than MAX_PROTOCOL_NAME_LENGTH. */
  if (equals - s > (int)MAX_PROTOCOL_NAME_LENGTH) {
    log_warn(LD_NET, "When parsing a protocol entry, I got a very large "
             "protocol name. This is possibly an attack or a bug, unless "
             "the Tor network truly supports protocol names larger than "
             "%ud characters. The offending string was: %s",
             MAX_PROTOCOL_NAME_LENGTH, escaped(out->name));
    goto error;
  }

  /* The name must contain only alphanumeric characters and hyphens. */
  if (!is_valid_keyword(s, equals-s))
    goto error;

  out->name = tor_strndup(s, equals-s);

  tor_assert(equals < end_of_entry);

  s = equals + 1;
  while (s < end_of_entry) {
    const char *comma = memchr(s, ',', end_of_entry-s);
    if (! comma)
      comma = end_of_entry;

    uint32_t low=0, high=0;
    if (parse_version_range(s, comma, &low, &high) < 0) {
      goto error;
    }

    out->bitmask |= bitmask_for_range(low,high);

    s = comma;
    // Skip the comma separator between ranges. Don't ignore a trailing comma.
    if (s < (end_of_entry - 1))
      ++s;
  }

  return out;

 error:
  proto_entry_free(out);
  return NULL;
}

/**
 * Parse the protocol list from <b>s</b> and return it as a smartlist of
 * proto_entry_t
 */
STATIC smartlist_t *
parse_protocol_list(const char *s)
{
  smartlist_t *entries = smartlist_new();

  while (*s) {
    /* Find the next space or the NUL. */
    const char *end_of_entry = strchr(s, ' ');
    proto_entry_t *entry;
    if (!end_of_entry)
      end_of_entry = s + strlen(s);

    entry = parse_single_entry(s, end_of_entry);

    if (! entry)
      goto error;

    smartlist_add(entries, entry);

    s = end_of_entry;
    while (*s == ' ')
      ++s;
  }

  return entries;

 error:
  SMARTLIST_FOREACH(entries, proto_entry_t *, ent, proto_entry_free(ent));
  smartlist_free(entries);
  return NULL;
}

/**
 * Return true if the unparsed protover list in <b>s</b> contains a
 * parsing error, such as extra commas, a bad number, or an over-long
 * name.
 */
bool
protover_list_is_invalid(const char *s)
{
  smartlist_t *list = parse_protocol_list(s);
  if (!list)
    return true; /* yes, has a dangerous name */
  SMARTLIST_FOREACH(list, proto_entry_t *, ent, proto_entry_free(ent));
  smartlist_free(list);
  return false; /* no, looks fine */
}

/**
 * Given a protocol type and version number, return true iff we know
 * how to speak that protocol.
 */
int
protover_is_supported_here(protocol_type_t pr, uint32_t ver)
{
  const smartlist_t *ours = get_supported_protocol_list();
  return protocol_list_contains(ours, pr, ver);
}

/**
 * Return true iff "list" encodes a protocol list that includes support for
 * the indicated protocol and version.
 *
 * If the protocol list is unparseable, treat it as if it defines no
 * protocols, and return 0.
 */
int
protocol_list_supports_protocol(const char *list, protocol_type_t tp,
                                uint32_t version)
{
  /* NOTE: This is a pretty inefficient implementation. If it ever shows
   * up in profiles, we should memoize it.
   */
  smartlist_t *protocols = parse_protocol_list(list);
  if (!protocols) {
    return 0;
  }
  int contains = protocol_list_contains(protocols, tp, version);

  SMARTLIST_FOREACH(protocols, proto_entry_t *, ent, proto_entry_free(ent));
  smartlist_free(protocols);
  return contains;
}

/**
 * Return true iff "list" encodes a protocol list that includes support for
 * the indicated protocol and version, or some later version.
 *
 * If the protocol list is unparseable, treat it as if it defines no
 * protocols, and return 0.
 */
int
protocol_list_supports_protocol_or_later(const char *list,
                                         protocol_type_t tp,
                                         uint32_t version)
{
  /* NOTE: This is a pretty inefficient implementation. If it ever shows
   * up in profiles, we should memoize it.
   */
  smartlist_t *protocols = parse_protocol_list(list);
  if (!protocols) {
    return 0;
  }
  const char *pr_name = protocol_type_to_str(tp);

  int contains = 0;
  const uint64_t mask = bitmask_for_range(version, 63);

  SMARTLIST_FOREACH_BEGIN(protocols, proto_entry_t *, proto) {
    if (strcasecmp(proto->name, pr_name))
      continue;
    if (0 != (proto->bitmask & mask)) {
      contains = 1;
      goto found;
    }
  } SMARTLIST_FOREACH_END(proto);

 found:
  SMARTLIST_FOREACH(protocols, proto_entry_t *, ent, proto_entry_free(ent));
  smartlist_free(protocols);
  return contains;
}

/*
 * XXX START OF HAZARDOUS ZONE XXX
 */
/* All protocol version that this relay version supports. */
#define PR_CONFLUX_V   "1"
#define PR_CONS_V      "1-2"
#define PR_DESC_V      "1-2"
#define PR_DIRCACHE_V  "2"
#define PR_FLOWCTRL_V  "1-2"
#define PR_HSDIR_V     "2"
#define PR_HSINTRO_V   "4-5"
#define PR_HSREND_V    "1-2"
#define PR_LINK_V      "1-5"
#ifdef HAVE_WORKING_TOR_TLS_GET_TLSSECRETS
#define PR_LINKAUTH_V  "1,3"
#else
#define PR_LINKAUTH_V  "3"
#endif
#define PR_MICRODESC_V "1-2"
#define PR_PADDING_V   "2"
#define PR_RELAY_V     "1-4"

/** Return the string containing the supported version for the given protocol
 * type. */
const char *
protover_get_supported(const protocol_type_t type)
{
  switch (type) {
  case PRT_CONFLUX: return PR_CONFLUX_V;
  case PRT_CONS: return PR_CONS_V;
  case PRT_DESC: return PR_DESC_V;
  case PRT_DIRCACHE: return PR_DIRCACHE_V;
  case PRT_FLOWCTRL: return PR_FLOWCTRL_V;
  case PRT_HSDIR: return PR_HSDIR_V;
  case PRT_HSINTRO:  return PR_HSINTRO_V;
  case PRT_HSREND: return PR_HSREND_V;
  case PRT_LINK: return PR_LINK_V;
  case PRT_LINKAUTH: return PR_LINKAUTH_V;
  case PRT_MICRODESC: return PR_MICRODESC_V;
  case PRT_PADDING: return PR_PADDING_V;
  case PRT_RELAY: return PR_RELAY_V;
  default:
    tor_assert_unreached();
  }
}

/** Return the canonical string containing the list of protocols
 * that we support.
 **/
/// C_RUST_COUPLED: src/rust/protover/protover.rs `SUPPORTED_PROTOCOLS`
const char *
protover_get_supported_protocols(void)
{
  /* WARNING!
   *
   * Remember to edit the SUPPORTED_PROTOCOLS list in protover.rs if you
   * are editing this list.
   */

  /*
   * XXX: WARNING!
   *
   * Be EXTREMELY CAREFUL when *removing* versions from this list.  If you
   * remove an entry while it still appears as "recommended" in the consensus,
   * you'll cause all the instances without it to warn.
   *
   * If you remove an entry while it still appears as "required" in the
   * consensus, you'll cause all the instances without it to refuse to connect
   * to the network, and shut down.
   *
   * If you need to remove a version from this list, you need to make sure that
   * it is not listed in the _current consensuses_: just removing it from the
   * required list below is NOT ENOUGH.  You need to remove it from the
   * required list, and THEN let the authorities upgrade and vote on new
   * consensuses without it. Only once those consensuses are out is it safe to
   * remove from this list.
   *
   * One concrete example of a very dangerous race that could occur:
   *
   * Suppose that the client supports protocols "HsDir=1-2" and the consensus
   * requires protocols "HsDir=1-2.  If the client supported protocol list is
   * then changed to "HSDir=2", while the consensus stills lists "HSDir=1-2",
   * then these clients, even very recent ones, will shut down because they
   * don't support "HSDir=1".
   *
   * And so, changes need to be done in strict sequence as described above.
   *
   * XXX: WARNING!
   */

  return
    "Conflux=" PR_CONFLUX_V " "
    "Cons=" PR_CONS_V " "
    "Desc=" PR_DESC_V " "
    "DirCache=" PR_DIRCACHE_V " "
    "FlowCtrl=" PR_FLOWCTRL_V " "
    "HSDir=" PR_HSDIR_V " "
    "HSIntro=" PR_HSINTRO_V " "
    "HSRend=" PR_HSREND_V " "
    "Link=" PR_LINK_V " "
    "LinkAuth=" PR_LINKAUTH_V " "
    "Microdesc=" PR_MICRODESC_V " "
    "Padding=" PR_PADDING_V " "
    "Relay=" PR_RELAY_V;
}

/*
 * XXX: WARNING!
 *
 * The recommended and required values are hardwired, to avoid disaster. Voting
 * on the wrong subprotocols here has the potential to take down the network.
 *
 * In particular, you need to be EXTREMELY CAREFUL before adding new versions
 * to the required protocol list.  Doing so will cause every relay or client
 * that doesn't support those versions to refuse to connect to the network and
 * shut down.
 *
 * Note that this applies to versions, not just protocols!  If you say that
 * Foobar=8-9 is required, and the client only has Foobar=9, it will shut down.
 *
 * It is okay to do this only for SUPER OLD relays that are not supported on
 * the network anyway.  For clients, we really shouldn't kick them off the
 * network unless their presence is causing serious active harm.
 *
 * The following required and recommended lists MUST be changed BEFORE the
 * supported list above is changed, so that these lists appear in the
 * consensus BEFORE clients need them.
 *
 * Please, see the warning in protocol_get_supported_versions().
 *
 * XXX: WARNING!
 */

/** Return the recommended client protocols list that directory authorities
 * put in the consensus. */
const char *
protover_get_recommended_client_protocols(void)
{
  return "Cons=2 Desc=2 DirCache=2 HSDir=2 HSIntro=4 HSRend=2 "
         "Link=4-5 Microdesc=2 Relay=2";
}

/** Return the recommended relay protocols list that directory authorities
 * put in the consensus. */
const char *
protover_get_recommended_relay_protocols(void)
{
  return "Cons=2 Desc=2 DirCache=2 HSDir=2 HSIntro=4 HSRend=2 "
         "Link=4-5 LinkAuth=3 Microdesc=2 Relay=2";
}

/** Return the required client protocols list that directory authorities
 * put in the consensus. */
const char *
protover_get_required_client_protocols(void)
{
  return "Cons=2 Desc=2 Link=4 Microdesc=2 Relay=2";
}

/** Return the required relay protocols list that directory authorities
 * put in the consensus. */
const char *
protover_get_required_relay_protocols(void)
{
  return "Cons=2 Desc=2 DirCache=2 HSDir=2 HSIntro=4 HSRend=2 "
         "Link=4-5 LinkAuth=3 Microdesc=2 Relay=2";
}

/*
 * XXX END OF HAZARDOUS ZONE XXX
 */

/** The protocols from protover_get_supported_protocols(), as parsed into a
 * list of proto_entry_t values. Access this via
 * get_supported_protocol_list. */
static smartlist_t *supported_protocol_list = NULL;

/** Return a pointer to a smartlist of proto_entry_t for the protocols
 * we support. */
static const smartlist_t *
get_supported_protocol_list(void)
{
  if (PREDICT_UNLIKELY(supported_protocol_list == NULL)) {
    supported_protocol_list =
      parse_protocol_list(protover_get_supported_protocols());
  }
  return supported_protocol_list;
}

/** Return the number of trailing zeros in x.  Undefined if x is 0. */
static int
trailing_zeros(uint64_t x)
{
#ifdef __GNUC__
  return __builtin_ctzll((unsigned long long)x);
#else
  int i;
  for (i = 0; i <= 64; ++i) {
    if (x&1)
      return i;
    x>>=1;
  }
  return i;
#endif /* defined(__GNUC__) */
}

/**
 * Given a protocol entry, encode it at the end of the smartlist <b>chunks</b>
 * as one or more newly allocated strings.
 */
static void
proto_entry_encode_into(smartlist_t *chunks, const proto_entry_t *entry)
{
  smartlist_add_asprintf(chunks, "%s=", entry->name);

  uint64_t mask = entry->bitmask;
  int shift = 0; // how much have we shifted by so far?
  bool first = true;
  while (mask) {
    const char *comma = first ? "" : ",";
    if (first) {
      first = false;
    }
    int zeros = trailing_zeros(mask);
    mask >>= zeros;
    shift += zeros;
    int ones = !mask ? 64 : trailing_zeros(~mask);
    if (ones == 1) {
      smartlist_add_asprintf(chunks, "%s%d", comma, shift);
    } else {
      smartlist_add_asprintf(chunks, "%s%d-%d", comma,
                             shift, shift + ones - 1);
    }
    if (ones == 64) {
      break; // avoid undefined behavior; can't shift by 64.
    }
    mask >>= ones;
    shift += ones;
  }
}

/** Given a list of space-separated proto_entry_t items,
 * encode it into a newly allocated space-separated string. */
STATIC char *
encode_protocol_list(const smartlist_t *sl)
{
  const char *separator = "";
  smartlist_t *chunks = smartlist_new();
  SMARTLIST_FOREACH_BEGIN(sl, const proto_entry_t *, ent) {
    smartlist_add_strdup(chunks, separator);

    proto_entry_encode_into(chunks, ent);

    separator = " ";
  } SMARTLIST_FOREACH_END(ent);

  char *result = smartlist_join_strings(chunks, "", 0, NULL);

  SMARTLIST_FOREACH(chunks, char *, cp, tor_free(cp));
  smartlist_free(chunks);

  return result;
}

/**
 * Protocol voting implementation.
 *
 * Given a list of strings describing protocol versions, return a newly
 * allocated string encoding all of the protocols that are listed by at
 * least <b>threshold</b> of the inputs.
 *
 * The string is minimal and sorted according to the rules of
 * contract_protocol_list above.
 */
char *
protover_compute_vote(const smartlist_t *list_of_proto_strings,
                      int threshold)
{
  // we use u8 counters below.
  tor_assert(smartlist_len(list_of_proto_strings) < 256);

  if (smartlist_len(list_of_proto_strings) == 0) {
    return tor_strdup("");
  }

  smartlist_t *parsed = smartlist_new(); // smartlist of smartlist of entries
  smartlist_t *proto_names = smartlist_new(); // smartlist of strings
  smartlist_t *result = smartlist_new(); // smartlist of entries

  // First, parse the inputs, and accumulate a list of protocol names.
  SMARTLIST_FOREACH_BEGIN(list_of_proto_strings, const char *, vote) {
    smartlist_t *unexpanded = parse_protocol_list(vote);
    if (! unexpanded) {
      log_warn(LD_NET, "I failed with parsing a protocol list from "
               "an authority. The offending string was: %s",
               escaped(vote));
      continue;
    }
    SMARTLIST_FOREACH_BEGIN(unexpanded, const proto_entry_t *, ent) {
      if (!smartlist_contains_string(proto_names,ent->name)) {
        smartlist_add(proto_names, ent->name);
      }
    } SMARTLIST_FOREACH_END(ent);
    smartlist_add(parsed, unexpanded);
  } SMARTLIST_FOREACH_END(vote);

  // Sort the list of names.
  smartlist_sort_strings(proto_names);

  // For each named protocol, compute the consensus.
  //
  // This is not super-efficient, but it's not critical path.
  SMARTLIST_FOREACH_BEGIN(proto_names, const char *, name) {
    uint8_t counts[64];
    memset(counts, 0, sizeof(counts));
    // Count how many votes we got for each bit.
    SMARTLIST_FOREACH_BEGIN(parsed, const smartlist_t *, vote) {
      const proto_entry_t *ent = find_entry_by_name(vote, name);
      if (! ent)
        continue;

      for (int i = 0; i < 64; ++i) {
        if ((ent->bitmask & BIT(i)) != 0) {
          ++ counts[i];
        }
      }
    } SMARTLIST_FOREACH_END(vote);

    uint64_t result_bitmask = 0;
    for (int i = 0; i < 64; ++i) {
      if (counts[i] >= threshold) {
        result_bitmask |= BIT(i);
      }
    }
    if (result_bitmask != 0) {
      proto_entry_t *newent = tor_malloc_zero(sizeof(proto_entry_t));
      newent->name = tor_strdup(name);
      newent->bitmask = result_bitmask;
      smartlist_add(result, newent);
    }
  } SMARTLIST_FOREACH_END(name);

  char *consensus = encode_protocol_list(result);

  SMARTLIST_FOREACH(result, proto_entry_t *, ent, proto_entry_free(ent));
  smartlist_free(result);
  smartlist_free(proto_names); // no need to free members; they are aliases.
  SMARTLIST_FOREACH_BEGIN(parsed, smartlist_t *, v) {
    SMARTLIST_FOREACH(v, proto_entry_t *, ent, proto_entry_free(ent));
    smartlist_free(v);
  } SMARTLIST_FOREACH_END(v);
  smartlist_free(parsed);

  return consensus;
}

/** Return true if every protocol version described in the string <b>s</b> is
 * one that we support, and false otherwise.  If <b>missing_out</b> is
 * provided, set it to the list of protocols we do not support.
 *
 * If the protocol version string is unparseable, treat it as if it defines no
 * protocols, and return 1.
 **/
int
protover_all_supported(const char *s, char **missing_out)
{
  if (!s) {
    return 1;
  }

  smartlist_t *entries = parse_protocol_list(s);
  if (BUG(entries == NULL)) {
    log_warn(LD_NET, "Received an unparseable protocol list %s"
             " from the consensus", escaped(s));
    return 1;
  }
  const smartlist_t *supported = get_supported_protocol_list();
  smartlist_t *missing = smartlist_new();

  SMARTLIST_FOREACH_BEGIN(entries, const proto_entry_t *, ent) {
    const proto_entry_t *mine = find_entry_by_name(supported, ent->name);
    if (mine == NULL) {
      if (ent->bitmask != 0) {
        proto_entry_t *m = tor_malloc_zero(sizeof(proto_entry_t));
        m->name = tor_strdup(ent->name);
        m->bitmask = ent->bitmask;
        smartlist_add(missing, m);
      }
      continue;
    }

    uint64_t missing_mask = ent->bitmask & ~mine->bitmask;
    if (missing_mask != 0) {
      proto_entry_t *m = tor_malloc_zero(sizeof(proto_entry_t));
      m->name = tor_strdup(ent->name);
      m->bitmask = missing_mask;
      smartlist_add(missing, m);
    }
  } SMARTLIST_FOREACH_END(ent);

  const int all_supported = (smartlist_len(missing) == 0);
  if (!all_supported && missing_out) {
    *missing_out = encode_protocol_list(missing);
  }

  SMARTLIST_FOREACH(missing, proto_entry_t *, ent, proto_entry_free(ent));
  smartlist_free(missing);

  SMARTLIST_FOREACH(entries, proto_entry_t *, ent, proto_entry_free(ent));
  smartlist_free(entries);

  return all_supported;
}

/** Helper: return the member of 'protos' whose name is
 * 'name', or NULL if there is no such member. */
static const proto_entry_t *
find_entry_by_name(const smartlist_t *protos, const char *name)
{
  if (!protos) {
    return NULL;
  }
  SMARTLIST_FOREACH_BEGIN(protos, const proto_entry_t *, ent) {
    if (!strcmp(ent->name, name)) {
      return ent;
    }
  } SMARTLIST_FOREACH_END(ent);

  return NULL;
}

/** Helper: Given a list of proto_entry_t, return true iff
 * <b>pr</b>=<b>ver</b> is included in that list. */
static int
protocol_list_contains(const smartlist_t *protos,
                       protocol_type_t pr, uint32_t ver)
{
  if (BUG(protos == NULL)) {
    return 0; // LCOV_EXCL_LINE
  }
  const char *pr_name = protocol_type_to_str(pr);
  if (BUG(pr_name == NULL)) {
    return 0; // LCOV_EXCL_LINE
  }
  if (ver > MAX_PROTOCOL_VERSION) {
    return 0;
  }

  const proto_entry_t *ent = find_entry_by_name(protos, pr_name);
  if (ent) {
    return (ent->bitmask & BIT(ver)) != 0;
  }
  return 0;
}

/** Return a string describing the protocols supported by tor version
 * <b>version</b>, or an empty string if we cannot tell.
 *
 * Note that this is only used to infer protocols for Tor versions that
 * can't declare their own.
 **/
/// C_RUST_COUPLED: src/rust/protover/protover.rs `compute_for_old_tor`
const char *
protover_compute_for_old_tor(const char *version)
{
  if (version == NULL) {
    /* No known version; guess the oldest series that is still supported. */
    version = "0.2.5.15";
  }

  if (tor_version_as_new_as(version,
                            FIRST_TOR_VERSION_TO_ADVERTISE_PROTOCOLS)) {
    return "";
  } else if (tor_version_as_new_as(version, "0.2.9.1-alpha")) {
    /* 0.2.9.1-alpha HSRend=2 */
    return "Cons=1-2 Desc=1-2 DirCache=1 HSDir=1 HSIntro=3 HSRend=1-2 "
      "Link=1-4 LinkAuth=1 "
      "Microdesc=1-2 Relay=1-2";
  } else if (tor_version_as_new_as(version, "0.2.7.5")) {
    /* 0.2.7-stable added Desc=2, Microdesc=2, Cons=2, which indicate
     * ed25519 support.  We'll call them present only in "stable" 027,
     * though. */
    return "Cons=1-2 Desc=1-2 DirCache=1 HSDir=1 HSIntro=3 HSRend=1 "
      "Link=1-4 LinkAuth=1 "
      "Microdesc=1-2 Relay=1-2";
  } else if (tor_version_as_new_as(version, "0.2.4.19")) {
    /* No currently supported Tor server versions are older than this, or
     * lack these protocols. */
    return "Cons=1 Desc=1 DirCache=1 HSDir=1 HSIntro=3 HSRend=1 "
      "Link=1-4 LinkAuth=1 "
      "Microdesc=1 Relay=1-2";
  } else {
    /* Cannot infer protocols. */
    return "";
  }
}

/**
 * Release all storage held by static fields in protover.c
 */
void
protover_free_all(void)
{
  if (supported_protocol_list) {
    smartlist_t *entries = supported_protocol_list;
    SMARTLIST_FOREACH(entries, proto_entry_t *, ent, proto_entry_free(ent));
    smartlist_free(entries);
    supported_protocol_list = NULL;
  }
}

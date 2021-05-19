/* Copyright (c) 2003-2004, Roger Dingledine
 * Copyright (c) 2004-2006, Roger Dingledine, Nick Mathewson.
 * Copyright (c) 2007-2020, The Tor Project, Inc. */
/* See LICENSE for licensing information */

/**
 * \file resolve.c
 * \brief Use the libc DNS resolver to convert hostnames into addresses.
 **/

#define RESOLVE_PRIVATE
#include "lib/net/resolve.h"

#include "lib/net/address.h"
#include "lib/net/inaddr.h"
#include "lib/malloc/malloc.h"
#include "lib/string/parse_int.h"
#include "lib/string/util_string.h"

#include "ext/siphash.h"
#include "ext/ht.h"

#ifdef HAVE_SYS_TYPES_H
#include <sys/types.h>
#endif
#ifdef HAVE_SYS_SOCKET_H
#include <sys/socket.h>
#endif
#ifdef HAVE_NETDB_H
#include <netdb.h>
#endif

#include <string.h>

/** Similar behavior to Unix gethostbyname: resolve <b>name</b>, and set
 * *<b>addr</b> to the proper IP address, in host byte order.  Returns 0
 * on success, -1 on failure; 1 on transient failure.
 *
 * This function only accepts IPv4 addresses.
 *
 * (This function exists because standard windows gethostbyname
 * doesn't treat raw IP addresses properly.)
 */

MOCK_IMPL(int,
tor_lookup_hostname,(const char *name, uint32_t *addr))
{
  tor_addr_t myaddr;
  int ret;

  if (BUG(!addr))
    return -1;

  *addr = 0;

  if ((ret = tor_addr_lookup(name, AF_INET, &myaddr)))
    return ret;

  if (tor_addr_family(&myaddr) == AF_INET) {
    *addr = tor_addr_to_ipv4h(&myaddr);
    return ret;
  }

  return -1;
}

#ifdef HAVE_GETADDRINFO

/* Host lookup helper for tor_addr_lookup(), when getaddrinfo() is
 * available on this system.
 *
 * See tor_addr_lookup() for details.
 */
MOCK_IMPL(STATIC int,
tor_addr_lookup_host_impl,(const char *name,
                          uint16_t family,
                          tor_addr_t *addr))
{
  int err;
  struct addrinfo *res=NULL, *res_p;
  struct addrinfo *best=NULL;
  struct addrinfo hints;
  int result = -1;
  memset(&hints, 0, sizeof(hints));
  hints.ai_family = family;
  hints.ai_socktype = SOCK_STREAM;
  err = tor_getaddrinfo(name, NULL, &hints, &res);
  /* The check for 'res' here shouldn't be necessary, but it makes static
   * analysis tools happy. */
  if (!err && res) {
    best = NULL;
    for (res_p = res; res_p; res_p = res_p->ai_next) {
      if (family == AF_UNSPEC) {
        if (res_p->ai_family == AF_INET) {
          best = res_p;
          break;
        } else if (res_p->ai_family == AF_INET6 && !best) {
          best = res_p;
        }
      } else if (family == res_p->ai_family) {
        best = res_p;
        break;
      }
    }
    if (!best)
      best = res;
    if (best->ai_family == AF_INET) {
      tor_addr_from_in(addr,
                       &((struct sockaddr_in*)best->ai_addr)->sin_addr);
      result = 0;
    } else if (best->ai_family == AF_INET6) {
      tor_addr_from_in6(addr,
                        &((struct sockaddr_in6*)best->ai_addr)->sin6_addr);
      result = 0;
    }
    tor_freeaddrinfo(res);
    return result;
  }
  return (err == EAI_AGAIN) ? 1 : -1;
}

#else /* !defined(HAVE_GETADDRINFO) */

/* Host lookup helper for tor_addr_lookup(), which calls gethostbyname().
 * Used when getaddrinfo() is not available on this system.
 *
 * See tor_addr_lookup() for details.
 */
MOCK_IMPL(STATIC int,
tor_addr_lookup_host_impl,(const char *name,
                          uint16_t family,
                           tor_addr_t *addr))
{
  (void) family;
  struct hostent *ent;
  int err;
#ifdef HAVE_GETHOSTBYNAME_R_6_ARG
  char buf[2048];
  struct hostent hostent;
  int r;
  r = gethostbyname_r(name, &hostent, buf, sizeof(buf), &ent, &err);
#elif defined(HAVE_GETHOSTBYNAME_R_5_ARG)
  char buf[2048];
  struct hostent hostent;
  ent = gethostbyname_r(name, &hostent, buf, sizeof(buf), &err);
#elif defined(HAVE_GETHOSTBYNAME_R_3_ARG)
  struct hostent_data data;
  struct hostent hent;
  memset(&data, 0, sizeof(data));
  err = gethostbyname_r(name, &hent, &data);
  ent = err ? NULL : &hent;
#else
  ent = gethostbyname(name);
#ifdef _WIN32
  err = WSAGetLastError();
#else
  err = h_errno;
#endif /* defined(_WIN32) */
#endif /* defined(HAVE_GETHOSTBYNAME_R_6_ARG) || ... */
  if (ent) {
    if (ent->h_addrtype == AF_INET) {
      tor_addr_from_in(addr, (struct in_addr*) ent->h_addr);
    } else if (ent->h_addrtype == AF_INET6) {
      tor_addr_from_in6(addr, (struct in6_addr*) ent->h_addr);
    } else {
      tor_assert(0); // LCOV_EXCL_LINE: gethostbyname() returned bizarre type
    }
    return 0;
  }
#ifdef _WIN32
  return (err == WSATRY_AGAIN) ? 1 : -1;
#else
  return (err == TRY_AGAIN) ? 1 : -1;
#endif
}
#endif /* defined(HAVE_GETADDRINFO) */

/** Similar behavior to Unix gethostbyname: resolve <b>name</b>, and set
 * *<b>addr</b> to the proper IP address and family. The <b>family</b>
 * argument (which must be AF_INET, AF_INET6, or AF_UNSPEC) declares a
 * <i>preferred</i> family, though another one may be returned if only one
 * family is implemented for this address.
 *
 * Like tor_addr_parse(), this function accepts IPv6 addresses with or without
 * square brackets.
 *
 * Return 0 on success, -1 on failure; 1 on transient failure.
 */
MOCK_IMPL(int,
tor_addr_lookup,(const char *name, uint16_t family, tor_addr_t *addr))
{
  /* Perhaps eventually this should be replaced by a tor_getaddrinfo or
   * something.
   */
  int parsed_family = 0;
  int result = -1;

  tor_assert(name);
  tor_assert(addr);
  tor_assert(family == AF_INET || family == AF_INET6 || family == AF_UNSPEC);

  if (!*name) {
    /* Empty address is an error. */
    goto permfail;
  }

  /* Is it an IP address? */
  parsed_family = tor_addr_parse(addr, name);

  if (parsed_family >= 0) {
    /* If the IP address family matches, or was unspecified */
    if (parsed_family == family || family == AF_UNSPEC) {
      goto success;
    } else {
      goto permfail;
    }
  } else {
    /* Clear the address after a failed tor_addr_parse(). */
    memset(addr, 0, sizeof(tor_addr_t));
    result = tor_addr_lookup_host_impl(name, family, addr);
    goto done;
  }

 /* If we weren't successful, and haven't already set the result,
  * assume it's a permanent failure */
 permfail:
  result = -1;
  goto done;
 success:
  result = 0;

 /* We have set the result, now it's time to clean up */
 done:
  if (result) {
    /* Clear the address on error */
    memset(addr, 0, sizeof(tor_addr_t));
  }
  return result;
}

/** Parse an address or address-port combination from <b>s</b>, resolve the
 * address as needed, and put the result in <b>addr_out</b> and (optionally)
 * <b>port_out</b>.
 *
 * Like tor_addr_port_parse(), this function accepts:
 *  - IPv6 address and port, when the IPv6 address is in square brackets,
 *  - IPv6 address with square brackets,
 *  - IPv6 address without square brackets.
 *
 * Return 0 on success, negative on failure. */
int
tor_addr_port_lookup(const char *s, tor_addr_t *addr_out, uint16_t *port_out)
{
  tor_addr_t addr;
  uint16_t portval = 0;
  char *tmp = NULL;
  int rv = 0;
  int result;

  tor_assert(s);
  tor_assert(addr_out);

  s = eat_whitespace(s);

  /* Try parsing s as an address:port first, so we don't have to duplicate
   * the logic that rejects IPv6:Port with no square brackets. */
  rv = tor_addr_port_parse(LOG_WARN, s, &addr, &portval, 0);
  /* That was easy, no DNS required. */
  if (rv == 0)
    goto success;

  /* Now let's check for malformed IPv6 addresses and ports:
   * tor_addr_port_parse() requires squared brackes if there is a port,
   * and we want tor_addr_port_lookup() to have the same requirement.
   * But we strip the port using tor_addr_port_split(), so tor_addr_lookup()
   * only sees the address, and will accept it without square brackets. */
  int family = tor_addr_parse(&addr, s);
  /* If tor_addr_parse() succeeds where tor_addr_port_parse() failed, we need
   * to reject this address as malformed. */
  if (family >= 0) {
    /* Double-check it's an IPv6 address. If not, we have a parsing bug.
     */
    tor_assertf_nonfatal(family == AF_INET6,
                         "Wrong family: %d (should be IPv6: %d) which "
                         "failed IP:port parsing, but passed IP parsing. "
                         "input string: '%s'; parsed address: '%s'.",
                         family, AF_INET6, s, fmt_addr(&addr));
    goto err;
  }

  /* Now we have a hostname. Let's split off the port, if any. */
  rv = tor_addr_port_split(LOG_WARN, s, &tmp, &portval);
  if (rv < 0)
    goto err;

  /* And feed the hostname to the lookup function. */
  if (tor_addr_lookup(tmp, AF_UNSPEC, &addr) != 0)
    goto err;

 success:
  if (port_out)
    *port_out = portval;
  tor_addr_copy(addr_out, &addr);
  result = 0;
  goto done;

 err:
  /* Clear the address and port on error */
  memset(addr_out, 0, sizeof(tor_addr_t));
  if (port_out)
    *port_out = 0;
  result = -1;

 /* We have set the result, now it's time to clean up */
 done:
  tor_free(tmp);
  return result;
}

#ifdef USE_SANDBOX_GETADDRINFO
/** True if we should only return cached values */
static int sandbox_getaddrinfo_is_active = 0;

/** Cache entry for getaddrinfo results; used when sandboxing is implemented
 * so that we can consult the cache when the sandbox prevents us from doing
 * getaddrinfo.
 *
 * We support only a limited range of getaddrinfo calls, where servname is null
 * and hints contains only socktype=SOCK_STREAM, family in INET,INET6,UNSPEC.
 */
typedef struct cached_getaddrinfo_item_t {
  HT_ENTRY(cached_getaddrinfo_item_t) node;
  char *name;
  int family;
  /** set if no error; otherwise NULL */
  struct addrinfo *res;
  /** 0 for no error; otherwise an EAI_* value */
  int err;
} cached_getaddrinfo_item_t;

static unsigned
cached_getaddrinfo_item_hash(const cached_getaddrinfo_item_t *item)
{
  return (unsigned)siphash24g(item->name, strlen(item->name)) + item->family;
}

static unsigned
cached_getaddrinfo_items_eq(const cached_getaddrinfo_item_t *a,
                            const cached_getaddrinfo_item_t *b)
{
  return (a->family == b->family) && 0 == strcmp(a->name, b->name);
}

#define cached_getaddrinfo_item_free(item)              \
  FREE_AND_NULL(cached_getaddrinfo_item_t,              \
                cached_getaddrinfo_item_free_, (item))

static void
cached_getaddrinfo_item_free_(cached_getaddrinfo_item_t *item)
{
  if (item == NULL)
    return;

  tor_free(item->name);
  if (item->res)
    freeaddrinfo(item->res);
  tor_free(item);
}

static HT_HEAD(getaddrinfo_cache, cached_getaddrinfo_item_t)
     getaddrinfo_cache = HT_INITIALIZER();

HT_PROTOTYPE(getaddrinfo_cache, cached_getaddrinfo_item_t, node,
             cached_getaddrinfo_item_hash,
             cached_getaddrinfo_items_eq);
HT_GENERATE2(getaddrinfo_cache, cached_getaddrinfo_item_t, node,
             cached_getaddrinfo_item_hash,
             cached_getaddrinfo_items_eq,
             0.6, tor_reallocarray_, tor_free_);

/** If true, don't try to cache getaddrinfo results. */
static int sandbox_getaddrinfo_cache_disabled = 0;

/** Tell the sandbox layer not to try to cache getaddrinfo results. Used as in
 * tor-resolve, when we have no intention of initializing crypto or of
 * installing the sandbox.*/
void
sandbox_disable_getaddrinfo_cache(void)
{
  sandbox_getaddrinfo_cache_disabled = 1;
}

void
tor_freeaddrinfo(struct addrinfo *ai)
{
  if (sandbox_getaddrinfo_cache_disabled)
    freeaddrinfo(ai);
}

int
tor_getaddrinfo(const char *name, const char *servname,
                const struct addrinfo *hints,
                struct addrinfo **res)
{
  int err;
  struct cached_getaddrinfo_item_t search, *item;

  if (sandbox_getaddrinfo_cache_disabled) {
    return getaddrinfo(name, NULL, hints, res);
  }

  if (servname != NULL) {
    log_warn(LD_BUG, "called with non-NULL servname");
    return EAI_NONAME;
  }
  if (name == NULL) {
    log_warn(LD_BUG, "called with NULL name");
    return EAI_NONAME;
  }

  *res = NULL;

  memset(&search, 0, sizeof(search));
  search.name = (char *) name;
  search.family = hints ? hints->ai_family : AF_UNSPEC;
  item = HT_FIND(getaddrinfo_cache, &getaddrinfo_cache, &search);

  if (! sandbox_getaddrinfo_is_active) {
    /* If the sandbox is not turned on yet, then getaddrinfo and store the
       result. */

    err = getaddrinfo(name, NULL, hints, res);
    log_info(LD_NET,"(Sandbox) getaddrinfo %s.", err ? "failed" : "succeeded");

    if (! item) {
      item = tor_malloc_zero(sizeof(*item));
      item->name = tor_strdup(name);
      item->family = hints ? hints->ai_family : AF_UNSPEC;
      HT_INSERT(getaddrinfo_cache, &getaddrinfo_cache, item);
    }

    if (item->res) {
      freeaddrinfo(item->res);
      item->res = NULL;
    }
    item->res = *res;
    item->err = err;
    return err;
  }

  /* Otherwise, the sandbox is on.  If we have an item, yield its cached
     result. */
  if (item) {
    *res = item->res;
    return item->err;
  }

  /* getting here means something went wrong */
  log_err(LD_BUG,"(Sandbox) failed to get address %s!", name);
  return EAI_NONAME;
}

int
tor_add_addrinfo(const char *name)
{
  struct addrinfo *res;
  struct addrinfo hints;
  int i;
  static const int families[] = { AF_INET, AF_INET6, AF_UNSPEC };

  memset(&hints, 0, sizeof(hints));
  hints.ai_socktype = SOCK_STREAM;
  for (i = 0; i < 3; ++i) {
    hints.ai_family = families[i];

    res = NULL;
    (void) tor_getaddrinfo(name, NULL, &hints, &res);
    if (res)
      tor_freeaddrinfo(res);
  }

  return 0;
}

void
tor_free_getaddrinfo_cache(void)
{
  cached_getaddrinfo_item_t **next, **item, *this;

  for (item = HT_START(getaddrinfo_cache, &getaddrinfo_cache);
       item;
       item = next) {
    this = *item;
    next = HT_NEXT_RMV(getaddrinfo_cache, &getaddrinfo_cache, item);
    cached_getaddrinfo_item_free(this);
  }

  HT_CLEAR(getaddrinfo_cache, &getaddrinfo_cache);
}

void
tor_make_getaddrinfo_cache_active(void)
{
  sandbox_getaddrinfo_is_active = 1;
}
#else /* !defined(USE_SANDBOX_GETADDRINFO) */
void
sandbox_disable_getaddrinfo_cache(void)
{
}
void
tor_make_getaddrinfo_cache_active(void)
{
}
#endif /* defined(USE_SANDBOX_GETADDRINFO) */

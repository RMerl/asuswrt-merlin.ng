/* Copyright (c) 2001-2004, Roger Dingledine.
 * Copyright (c) 2004-2006, Roger Dingledine, Nick Mathewson.
 * Copyright (c) 2007-2021, The Tor Project, Inc. */
/* See LICENSE for licensing information */

#include "core/or/or.h"

#include "app/config/config.h"
#include "core/mainloop/connection.h"
#include "feature/dircache/conscache.h"
#include "feature/dircache/consdiffmgr.h"
#include "feature/dircommon/directory.h"
#include "feature/dircache/dirserv.h"
#include "feature/nodelist/microdesc.h"
#include "feature/nodelist/routerlist.h"
#include "feature/relay/router.h"
#include "feature/relay/routermode.h"
#include "feature/stats/predict_ports.h"

#include "feature/dircache/cached_dir_st.h"
#include "feature/dircommon/dir_connection_st.h"
#include "feature/nodelist/extrainfo_st.h"
#include "feature/nodelist/microdesc_st.h"
#include "feature/nodelist/routerinfo_st.h"
#include "feature/nodelist/routerlist_st.h"

#include "lib/compress/compress.h"

/**
 * \file dirserv.c
 * \brief Directory server core implementation. Manages directory
 * contents and generates directory documents.
 *
 * This module implements most of directory cache functionality, and some of
 * the directory authority functionality.  The directory.c module delegates
 * here in order to handle incoming requests from clients, via
 * connection_dirserv_flushed_some() and its kin.  In order to save RAM, this
 * module is responsible for spooling directory objects (in whole or in part)
 * onto buf_t instances, and then closing the dir_connection_t once the
 * objects are totally flushed.
 *
 * The directory.c module also delegates here for handling descriptor uploads
 * via dirserv_add_multiple_descriptors().
 *
 * Additionally, this module handles some aspects of voting, including:
 * deciding how to vote on individual flags (based on decisions reached in
 * rephist.c), of formatting routerstatus lines, and deciding what relays to
 * include in an authority's vote.  (TODO: Those functions could profitably be
 * split off.  They only live in this file because historically they were
 * shared among the v1, v2, and v3 directory code.)
 */

static void clear_cached_dir(cached_dir_t *d);
static const signed_descriptor_t *get_signed_descriptor_by_fp(
                                                        const uint8_t *fp,
                                                        int extrainfo);

static int spooled_resource_lookup_body(const spooled_resource_t *spooled,
                                        int conn_is_encrypted,
                                        const uint8_t **body_out,
                                        size_t *size_out,
                                        time_t *published_out);
static cached_dir_t *spooled_resource_lookup_cached_dir(
                                   const spooled_resource_t *spooled,
                                   time_t *published_out);
static cached_dir_t *lookup_cached_dir_by_fp(const uint8_t *fp);

/********************************************************************/

/* A set of functions to answer questions about how we'd like to behave
 * as a directory mirror */

/** Return true iff we want to serve certificates for authorities
 * that we don't acknowledge as authorities ourself.
 * Use we_want_to_fetch_unknown_auth_certs to check if we want to fetch
 * and keep these certificates.
 */
int
directory_caches_unknown_auth_certs(const or_options_t *options)
{
  return dir_server_mode(options) || options->BridgeRelay;
}

/** Return 1 if we want to fetch and serve descriptors, networkstatuses, etc
 * Else return 0.
 * Check options->DirPort_set and directory_permits_begindir_requests()
 * to see if we are willing to serve these directory documents to others via
 * the DirPort and begindir-over-ORPort, respectively.
 *
 * To check if we should fetch documents, use we_want_to_fetch_flavor and
 * we_want_to_fetch_unknown_auth_certs instead of this function.
 */
int
directory_caches_dir_info(const or_options_t *options)
{
  if (options->BridgeRelay || dir_server_mode(options))
    return 1;
  if (!server_mode(options) || !advertised_server_mode())
    return 0;
  /* We need an up-to-date view of network info if we're going to try to
   * block exit attempts from unknown relays. */
  return ! router_my_exit_policy_is_reject_star() &&
    should_refuse_unknown_exits(options);
}

/** Return 1 if we want to allow remote clients to ask us directory
 * requests via the "begin_dir" interface, which doesn't require
 * having any separate port open. */
int
directory_permits_begindir_requests(const or_options_t *options)
{
  return options->BridgeRelay != 0 || dir_server_mode(options);
}

/********************************************************************/

/** Map from flavor name to the cached_dir_t for the v3 consensuses that we're
 * currently serving. */
static strmap_t *cached_consensuses = NULL;

/** Decrement the reference count on <b>d</b>, and free it if it no longer has
 * any references. */
void
cached_dir_decref(cached_dir_t *d)
{
  if (!d || --d->refcnt > 0)
    return;
  clear_cached_dir(d);
  tor_free(d);
}

/** Allocate and return a new cached_dir_t containing the string <b>s</b>,
 * published at <b>published</b>. */
cached_dir_t *
new_cached_dir(char *s, time_t published)
{
  cached_dir_t *d = tor_malloc_zero(sizeof(cached_dir_t));
  d->refcnt = 1;
  d->dir = s;
  d->dir_len = strlen(s);
  d->published = published;
  if (tor_compress(&(d->dir_compressed), &(d->dir_compressed_len),
                   d->dir, d->dir_len, ZLIB_METHOD)) {
    log_warn(LD_BUG, "Error compressing directory");
  }
  return d;
}

/** Remove all storage held in <b>d</b>, but do not free <b>d</b> itself. */
static void
clear_cached_dir(cached_dir_t *d)
{
  tor_free(d->dir);
  tor_free(d->dir_compressed);
  memset(d, 0, sizeof(cached_dir_t));
}

/** Free all storage held by the cached_dir_t in <b>d</b>. */
static void
free_cached_dir_(void *_d)
{
  cached_dir_t *d;
  if (!_d)
    return;

  d = (cached_dir_t *)_d;
  cached_dir_decref(d);
}

/** Replace the v3 consensus networkstatus of type <b>flavor_name</b> that
 * we're serving with <b>networkstatus</b>, published at <b>published</b>.  No
 * validation is performed. */
void
dirserv_set_cached_consensus_networkstatus(const char *networkstatus,
                                           size_t networkstatus_len,
                                           const char *flavor_name,
                                           const common_digests_t *digests,
                                           const uint8_t *sha3_as_signed,
                                           time_t published)
{
  cached_dir_t *new_networkstatus;
  cached_dir_t *old_networkstatus;
  if (!cached_consensuses)
    cached_consensuses = strmap_new();

  new_networkstatus =
    new_cached_dir(tor_memdup_nulterm(networkstatus, networkstatus_len),
                   published);
  memcpy(&new_networkstatus->digests, digests, sizeof(common_digests_t));
  memcpy(&new_networkstatus->digest_sha3_as_signed, sha3_as_signed,
         DIGEST256_LEN);
  old_networkstatus = strmap_set(cached_consensuses, flavor_name,
                                 new_networkstatus);
  if (old_networkstatus)
    cached_dir_decref(old_networkstatus);
}

/** Return the latest downloaded consensus networkstatus in encoded, signed,
 * optionally compressed format, suitable for sending to clients. */
MOCK_IMPL(cached_dir_t *,
dirserv_get_consensus,(const char *flavor_name))
{
  if (!cached_consensuses)
    return NULL;
  return strmap_get(cached_consensuses, flavor_name);
}

/** As dir_split_resource_into_fingerprints, but instead fills
 * <b>spool_out</b> with a list of spoolable_resource_t for the resource
 * identified through <b>source</b>. */
int
dir_split_resource_into_spoolable(const char *resource,
                                  dir_spool_source_t source,
                                  smartlist_t *spool_out,
                                  int *compressed_out,
                                  int flags)
{
  smartlist_t *fingerprints = smartlist_new();

  tor_assert(flags & (DSR_HEX|DSR_BASE64));
  const size_t digest_len =
    (flags & DSR_DIGEST256) ? DIGEST256_LEN : DIGEST_LEN;

  int r = dir_split_resource_into_fingerprints(resource, fingerprints,
                                               compressed_out, flags);
  /* This is not a very efficient implementation XXXX */
  SMARTLIST_FOREACH_BEGIN(fingerprints, uint8_t *, digest) {
    spooled_resource_t *spooled =
      spooled_resource_new(source, digest, digest_len);
    if (spooled)
      smartlist_add(spool_out, spooled);
    tor_free(digest);
  } SMARTLIST_FOREACH_END(digest);

  smartlist_free(fingerprints);
  return r;
}

/** As dirserv_get_routerdescs(), but instead of getting signed_descriptor_t
 * pointers, adds copies of digests to fps_out, and doesn't use the
 * /tor/server/ prefix.  For a /d/ request, adds descriptor digests; for other
 * requests, adds identity digests.
 */
int
dirserv_get_routerdesc_spool(smartlist_t *spool_out,
                             const char *key,
                             dir_spool_source_t source,
                             int conn_is_encrypted,
                             const char **msg_out)
{
  *msg_out = NULL;

  if (!strcmp(key, "all")) {
    const routerlist_t *rl = router_get_routerlist();
    SMARTLIST_FOREACH_BEGIN(rl->routers, const routerinfo_t *, r) {
      spooled_resource_t *spooled;
      spooled = spooled_resource_new(source,
                              (const uint8_t *)r->cache_info.identity_digest,
                              DIGEST_LEN);
      /* Treat "all" requests as if they were unencrypted */
      conn_is_encrypted = 0;
      smartlist_add(spool_out, spooled);
    } SMARTLIST_FOREACH_END(r);
  } else if (!strcmp(key, "authority")) {
    const routerinfo_t *ri = router_get_my_routerinfo();
    if (ri)
      smartlist_add(spool_out,
                    spooled_resource_new(source,
                             (const uint8_t *)ri->cache_info.identity_digest,
                             DIGEST_LEN));
  } else if (!strcmpstart(key, "d/")) {
    key += strlen("d/");
    dir_split_resource_into_spoolable(key, source, spool_out, NULL,
                                  DSR_HEX|DSR_SORT_UNIQ);
  } else if (!strcmpstart(key, "fp/")) {
    key += strlen("fp/");
    dir_split_resource_into_spoolable(key, source, spool_out, NULL,
                                  DSR_HEX|DSR_SORT_UNIQ);
  } else {
    *msg_out = "Not found";
    return -1;
  }

  if (! conn_is_encrypted) {
    /* Remove anything that insists it not be sent unencrypted. */
    SMARTLIST_FOREACH_BEGIN(spool_out, spooled_resource_t *, spooled) {
      const uint8_t *body = NULL;
      size_t bodylen = 0;
      int r = spooled_resource_lookup_body(spooled, conn_is_encrypted,
                                           &body, &bodylen, NULL);
      if (r < 0 || body == NULL || bodylen == 0) {
        SMARTLIST_DEL_CURRENT(spool_out, spooled);
        spooled_resource_free(spooled);
      }
    } SMARTLIST_FOREACH_END(spooled);
  }

  if (!smartlist_len(spool_out)) {
    *msg_out = "Servers unavailable";
    return -1;
  }
  return 0;
}

/* ==========
 * Spooling code.
 * ========== */

spooled_resource_t *
spooled_resource_new(dir_spool_source_t source,
                     const uint8_t *digest, size_t digestlen)
{
  spooled_resource_t *spooled = tor_malloc_zero(sizeof(spooled_resource_t));
  spooled->spool_source = source;
  switch (source) {
    case DIR_SPOOL_NETWORKSTATUS:
      spooled->spool_eagerly = 0;
      break;
    case DIR_SPOOL_SERVER_BY_DIGEST:
    case DIR_SPOOL_SERVER_BY_FP:
    case DIR_SPOOL_EXTRA_BY_DIGEST:
    case DIR_SPOOL_EXTRA_BY_FP:
    case DIR_SPOOL_MICRODESC:
    default:
      spooled->spool_eagerly = 1;
      break;
    case DIR_SPOOL_CONSENSUS_CACHE_ENTRY:
      tor_assert_unreached();
      break;
  }
  tor_assert(digestlen <= sizeof(spooled->digest));
  if (digest)
    memcpy(spooled->digest, digest, digestlen);
  return spooled;
}

/**
 * Create a new spooled_resource_t to spool the contents of <b>entry</b> to
 * the user.  Return the spooled object on success, or NULL on failure (which
 * is probably caused by a failure to map the body of the item from disk).
 *
 * Adds a reference to entry's reference counter.
 */
spooled_resource_t *
spooled_resource_new_from_cache_entry(consensus_cache_entry_t *entry)
{
  spooled_resource_t *spooled = tor_malloc_zero(sizeof(spooled_resource_t));
  spooled->spool_source = DIR_SPOOL_CONSENSUS_CACHE_ENTRY;
  spooled->spool_eagerly = 0;
  consensus_cache_entry_incref(entry);
  spooled->consensus_cache_entry = entry;

  int r = consensus_cache_entry_get_body(entry,
                                         &spooled->cce_body,
                                         &spooled->cce_len);
  if (r == 0) {
    return spooled;
  } else {
    spooled_resource_free(spooled);
    return NULL;
  }
}

/** Release all storage held by <b>spooled</b>. */
void
spooled_resource_free_(spooled_resource_t *spooled)
{
  if (spooled == NULL)
    return;

  if (spooled->cached_dir_ref) {
    cached_dir_decref(spooled->cached_dir_ref);
  }

  if (spooled->consensus_cache_entry) {
    consensus_cache_entry_decref(spooled->consensus_cache_entry);
  }

  tor_free(spooled);
}

/** When spooling data from a cached_dir_t object, we always add
 * at least this much. */
#define DIRSERV_CACHED_DIR_CHUNK_SIZE 8192

/** Return an compression ratio for compressing objects from <b>source</b>.
 */
static double
estimate_compression_ratio(dir_spool_source_t source)
{
  /* We should put in better estimates here, depending on the number of
     objects and their type */
  (void) source;
  return 0.5;
}

/** Return an estimated number of bytes needed for transmitting the
 * resource in <b>spooled</b> on <b>conn</b>
 *
 * As a convenient side-effect, set *<b>published_out</b> to the resource's
 * publication time.
 */
static size_t
spooled_resource_estimate_size(const spooled_resource_t *spooled,
                               dir_connection_t *conn,
                               int compressed,
                               time_t *published_out)
{
  if (spooled->spool_eagerly) {
    const uint8_t *body = NULL;
    size_t bodylen = 0;
    int r = spooled_resource_lookup_body(spooled,
                                         connection_dir_is_encrypted(conn),
                                         &body, &bodylen,
                                         published_out);
    if (r == -1 || body == NULL || bodylen == 0)
      return 0;
    if (compressed) {
      double ratio = estimate_compression_ratio(spooled->spool_source);
      bodylen = (size_t)(bodylen * ratio);
    }
    return bodylen;
  } else {
    cached_dir_t *cached;
    if (spooled->consensus_cache_entry) {
      if (published_out) {
        consensus_cache_entry_get_valid_after(
            spooled->consensus_cache_entry, published_out);
      }

      return spooled->cce_len;
    }
    if (spooled->cached_dir_ref) {
      cached = spooled->cached_dir_ref;
    } else {
      cached = spooled_resource_lookup_cached_dir(spooled,
                                                  published_out);
    }
    if (cached == NULL) {
      return 0;
    }
    size_t result = compressed ? cached->dir_compressed_len : cached->dir_len;
    return result;
  }
}

/** Return code for spooled_resource_flush_some */
typedef enum {
  SRFS_ERR = -1,
  SRFS_MORE = 0,
  SRFS_DONE
} spooled_resource_flush_status_t;

/** Flush some or all of the bytes from <b>spooled</b> onto <b>conn</b>.
 * Return SRFS_ERR on error, SRFS_MORE if there are more bytes to flush from
 * this spooled resource, or SRFS_DONE if we are done flushing this spooled
 * resource.
 */
static spooled_resource_flush_status_t
spooled_resource_flush_some(spooled_resource_t *spooled,
                            dir_connection_t *conn)
{
  if (spooled->spool_eagerly) {
    /* Spool_eagerly resources are sent all-at-once. */
    const uint8_t *body = NULL;
    size_t bodylen = 0;
    int r = spooled_resource_lookup_body(spooled,
                                         connection_dir_is_encrypted(conn),
                                         &body, &bodylen, NULL);
    if (r == -1 || body == NULL || bodylen == 0) {
      /* Absent objects count as "done". */
      return SRFS_DONE;
    }

    connection_dir_buf_add((const char*)body, bodylen, conn, 0);

    return SRFS_DONE;
  } else {
    cached_dir_t *cached = spooled->cached_dir_ref;
    consensus_cache_entry_t *cce = spooled->consensus_cache_entry;
    if (cached == NULL && cce == NULL) {
      /* The cached_dir_t hasn't been materialized yet. So let's look it up. */
      cached = spooled->cached_dir_ref =
        spooled_resource_lookup_cached_dir(spooled, NULL);
      if (!cached) {
        /* Absent objects count as done. */
        return SRFS_DONE;
      }
      ++cached->refcnt;
      tor_assert_nonfatal(spooled->cached_dir_offset == 0);
    }

    if (BUG(!cached && !cce))
      return SRFS_DONE;

    int64_t total_len;
    const char *ptr;
    if (cached) {
      total_len = cached->dir_compressed_len;
      ptr = cached->dir_compressed;
    } else {
      total_len = spooled->cce_len;
      ptr = (const char *)spooled->cce_body;
    }
    /* How many bytes left to flush? */
    int64_t remaining;
    remaining = total_len - spooled->cached_dir_offset;
    if (BUG(remaining < 0))
      return SRFS_ERR;
    ssize_t bytes = (ssize_t) MIN(DIRSERV_CACHED_DIR_CHUNK_SIZE, remaining);

    connection_dir_buf_add(ptr + spooled->cached_dir_offset,
                           bytes, conn, 0);

    spooled->cached_dir_offset += bytes;
    if (spooled->cached_dir_offset >= (off_t)total_len) {
      return SRFS_DONE;
    } else {
      return SRFS_MORE;
    }
  }
}

/** Helper: find the cached_dir_t for a spooled_resource_t, for
 * sending it to <b>conn</b>. Set *<b>published_out</b>, if provided,
 * to the published time of the cached_dir_t.
 *
 * DOES NOT increase the reference count on the result.  Callers must do that
 * themselves if they mean to hang on to it.
 */
static cached_dir_t *
spooled_resource_lookup_cached_dir(const spooled_resource_t *spooled,
                                   time_t *published_out)
{
  tor_assert(spooled->spool_eagerly == 0);
  cached_dir_t *d = lookup_cached_dir_by_fp(spooled->digest);
  if (d != NULL) {
    if (published_out)
      *published_out = d->published;
  }
  return d;
}

/** Helper: Look up the body for an eagerly-served spooled_resource.  If
 * <b>conn_is_encrypted</b> is false, don't look up any resource that
 * shouldn't be sent over an unencrypted connection.  On success, set
 * <b>body_out</b>, <b>size_out</b>, and <b>published_out</b> to refer
 * to the resource's body, size, and publication date, and return 0.
 * On failure return -1. */
static int
spooled_resource_lookup_body(const spooled_resource_t *spooled,
                             int conn_is_encrypted,
                             const uint8_t **body_out,
                             size_t *size_out,
                             time_t *published_out)
{
  tor_assert(spooled->spool_eagerly == 1);

  const signed_descriptor_t *sd = NULL;

  switch (spooled->spool_source) {
    case DIR_SPOOL_EXTRA_BY_FP: {
      sd = get_signed_descriptor_by_fp(spooled->digest, 1);
      break;
    }
    case DIR_SPOOL_SERVER_BY_FP: {
      sd = get_signed_descriptor_by_fp(spooled->digest, 0);
      break;
    }
    case DIR_SPOOL_SERVER_BY_DIGEST: {
      sd = router_get_by_descriptor_digest((const char *)spooled->digest);
      break;
    }
    case DIR_SPOOL_EXTRA_BY_DIGEST: {
      sd = extrainfo_get_by_descriptor_digest((const char *)spooled->digest);
      break;
    }
    case DIR_SPOOL_MICRODESC: {
      microdesc_t *md = microdesc_cache_lookup_by_digest256(
                                  get_microdesc_cache(),
                                  (const char *)spooled->digest);
      if (! md || ! md->body) {
        return -1;
      }
      *body_out = (const uint8_t *)md->body;
      *size_out = md->bodylen;
      if (published_out)
        *published_out = TIME_MAX;
      return 0;
    }
    case DIR_SPOOL_NETWORKSTATUS:
    case DIR_SPOOL_CONSENSUS_CACHE_ENTRY:
    default:
      /* LCOV_EXCL_START */
      tor_assert_nonfatal_unreached();
      return -1;
      /* LCOV_EXCL_STOP */
  }

  /* If we get here, then we tried to set "sd" to a signed_descriptor_t. */

  if (sd == NULL) {
    return -1;
  }
  if (sd->send_unencrypted == 0 && ! conn_is_encrypted) {
    /* we did this check once before (so we could have an accurate size
     * estimate and maybe send a 404 if somebody asked for only bridges on
     * a connection), but we need to do it again in case a previously
     * unknown bridge descriptor has shown up between then and now. */
    return -1;
  }
  *body_out = (const uint8_t *) signed_descriptor_get_body(sd);
  *size_out = sd->signed_descriptor_len;
  if (published_out)
    *published_out = sd->published_on;
  return 0;
}

/** Given a fingerprint <b>fp</b> which is either set if we're looking for a
 * v2 status, or zeroes if we're looking for a v3 status, or a NUL-padded
 * flavor name if we want a flavored v3 status, return a pointer to the
 * appropriate cached dir object, or NULL if there isn't one available. */
static cached_dir_t *
lookup_cached_dir_by_fp(const uint8_t *fp)
{
  cached_dir_t *d = NULL;
  if (tor_digest_is_zero((const char *)fp) && cached_consensuses) {
    d = strmap_get(cached_consensuses, "ns");
  } else if (memchr(fp, '\0', DIGEST_LEN) && cached_consensuses) {
    /* this here interface is a nasty hack: we're shoving a flavor into
     * a digest field. */
    d = strmap_get(cached_consensuses, (const char *)fp);
  }
  return d;
}

/** Try to guess the number of bytes that will be needed to send the
 * spooled objects for <b>conn</b>'s outgoing spool.  In the process,
 * remove every element of the spool that refers to an absent object, or
 * which was published earlier than <b>cutoff</b>.  Set *<b>size_out</b>
 * to the number of bytes, and *<b>n_expired_out</b> to the number of
 * objects removed for being too old. */
void
dirserv_spool_remove_missing_and_guess_size(dir_connection_t *conn,
                                            time_t cutoff,
                                            int compression,
                                            size_t *size_out,
                                            int *n_expired_out)
{
  if (BUG(!conn))
    return;

  smartlist_t *spool = conn->spool;
  if (!spool) {
    if (size_out)
      *size_out = 0;
    if (n_expired_out)
      *n_expired_out = 0;
    return;
  }
  int n_expired = 0;
  uint64_t total = 0;
  SMARTLIST_FOREACH_BEGIN(spool, spooled_resource_t *, spooled) {
    time_t published = TIME_MAX;
    size_t sz = spooled_resource_estimate_size(spooled, conn,
                                               compression, &published);
    if (published < cutoff) {
      ++n_expired;
      SMARTLIST_DEL_CURRENT(spool, spooled);
      spooled_resource_free(spooled);
    } else if (sz == 0) {
      SMARTLIST_DEL_CURRENT(spool, spooled);
      spooled_resource_free(spooled);
    } else {
      total += sz;
    }
  } SMARTLIST_FOREACH_END(spooled);

  if (size_out) {
    *size_out = (total > SIZE_MAX) ? SIZE_MAX : (size_t)total;
  }
  if (n_expired_out)
    *n_expired_out = n_expired;
}

/** Helper: used to sort a connection's spool. */
static int
dirserv_spool_sort_comparison_(const void **a_, const void **b_)
{
  const spooled_resource_t *a = *a_;
  const spooled_resource_t *b = *b_;
  return fast_memcmp(a->digest, b->digest, sizeof(a->digest));
}

/** Sort all the entries in <b>conn</b> by digest. */
void
dirserv_spool_sort(dir_connection_t *conn)
{
  if (conn->spool == NULL)
    return;
  smartlist_sort(conn->spool, dirserv_spool_sort_comparison_);
}

/** Return the cache-info for identity fingerprint <b>fp</b>, or
 * its extra-info document if <b>extrainfo</b> is true. Return
 * NULL if not found or if the descriptor is older than
 * <b>publish_cutoff</b>. */
static const signed_descriptor_t *
get_signed_descriptor_by_fp(const uint8_t *fp, int extrainfo)
{
  if (router_digest_is_me((const char *)fp)) {
    if (extrainfo)
      return &(router_get_my_extrainfo()->cache_info);
    else
      return &(router_get_my_routerinfo()->cache_info);
  } else {
    const routerinfo_t *ri = router_get_by_id_digest((const char *)fp);
    if (ri) {
      if (extrainfo)
        return extrainfo_get_by_descriptor_digest(
                                     ri->cache_info.extra_info_digest);
      else
        return &ri->cache_info;
    }
  }
  return NULL;
}

/** When we're spooling data onto our outbuf, add more whenever we dip
 * below this threshold. */
#define DIRSERV_BUFFER_MIN 16384

/**
 * Called whenever we have flushed some directory data in state
 * SERVER_WRITING, or whenever we want to fill the buffer with initial
 * directory data (so that subsequent writes will occur, and trigger this
 * function again.)
 *
 * Return 0 on success, and -1 on failure.
 */
int
connection_dirserv_flushed_some(dir_connection_t *conn)
{
  tor_assert(conn->base_.state == DIR_CONN_STATE_SERVER_WRITING);
  if (conn->spool == NULL)
    return 0;

  while (connection_get_outbuf_len(TO_CONN(conn)) < DIRSERV_BUFFER_MIN &&
         smartlist_len(conn->spool)) {
    spooled_resource_t *spooled =
      smartlist_get(conn->spool, smartlist_len(conn->spool)-1);
    spooled_resource_flush_status_t status;
    status = spooled_resource_flush_some(spooled, conn);
    if (status == SRFS_ERR) {
      return -1;
    } else if (status == SRFS_MORE) {
      return 0;
    }
    tor_assert(status == SRFS_DONE);

    /* If we're here, we're done flushing this resource. */
    tor_assert(smartlist_pop_last(conn->spool) == spooled);
    spooled_resource_free(spooled);
  }

  if (smartlist_len(conn->spool) > 0) {
    /* We're still spooling something. */
    return 0;
  }

  /* If we get here, we're done. */
  smartlist_free(conn->spool);
  conn->spool = NULL;
  if (conn->compress_state) {
    /* Flush the compression state: there could be more bytes pending in there,
     * and we don't want to omit bytes. */
    connection_buf_add_compress("", 0, conn, 1);
    tor_compress_free(conn->compress_state);
    conn->compress_state = NULL;
  }
  return 0;
}

/** Remove every element from <b>conn</b>'s outgoing spool, and delete
 * the spool. */
void
dir_conn_clear_spool(dir_connection_t *conn)
{
  if (!conn || ! conn->spool)
    return;
  SMARTLIST_FOREACH(conn->spool, spooled_resource_t *, s,
                    spooled_resource_free(s));
  smartlist_free(conn->spool);
  conn->spool = NULL;
}

/** Release all storage used by the directory server. */
void
dirserv_free_all(void)
{
  strmap_free(cached_consensuses, free_cached_dir_);
  cached_consensuses = NULL;
}

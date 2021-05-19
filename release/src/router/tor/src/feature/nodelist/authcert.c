/* Copyright (c) 2001 Matej Pfajfar.
 * Copyright (c) 2001-2004, Roger Dingledine.
 * Copyright (c) 2004-2006, Roger Dingledine, Nick Mathewson.
 * Copyright (c) 2007-2020, The Tor Project, Inc. */
/* See LICENSE for licensing information */

/**
 * \file authcert.c
 * \brief Code to maintain directory authorities' certificates.
 *
 * Authority certificates are signed with authority identity keys; they
 * are used to authenticate shorter-term authority signing keys. We
 * fetch them when we find a consensus or a vote that has been signed
 * with a signing key we don't recognize.  We cache them on disk and
 * load them on startup.  Authority operators generate them with the
 * "tor-gencert" utility.
 */

#include "core/or/or.h"

#include "app/config/config.h"
#include "core/mainloop/connection.h"
#include "core/mainloop/mainloop.h"
#include "core/or/policies.h"
#include "feature/client/bridges.h"
#include "feature/dirauth/authmode.h"
#include "feature/dirclient/dirclient.h"
#include "feature/dirclient/dlstatus.h"
#include "feature/dircommon/directory.h"
#include "feature/dircommon/fp_pair.h"
#include "feature/dirparse/authcert_parse.h"
#include "feature/nodelist/authcert.h"
#include "feature/nodelist/dirlist.h"
#include "feature/nodelist/networkstatus.h"
#include "feature/nodelist/node_select.h"
#include "feature/nodelist/nodelist.h"
#include "feature/nodelist/routerlist.h"
#include "feature/relay/routermode.h"

#include "core/or/connection_st.h"
#include "feature/dirclient/dir_server_st.h"
#include "feature/dircommon/dir_connection_st.h"
#include "feature/nodelist/authority_cert_st.h"
#include "feature/nodelist/document_signature_st.h"
#include "feature/nodelist/networkstatus_st.h"
#include "feature/nodelist/networkstatus_voter_info_st.h"
#include "feature/nodelist/node_st.h"

DECLARE_TYPED_DIGESTMAP_FNS(dsmap, digest_ds_map_t, download_status_t)
#define DSMAP_FOREACH(map, keyvar, valvar) \
  DIGESTMAP_FOREACH(dsmap_to_digestmap(map), keyvar, download_status_t *, \
                    valvar)
#define dsmap_free(map, fn) MAP_FREE_AND_NULL(dsmap, (map), (fn))

/* Forward declaration for cert_list_t */
typedef struct cert_list_t cert_list_t;

static void download_status_reset_by_sk_in_cl(cert_list_t *cl,
                                              const char *digest);
static int download_status_is_ready_by_sk_in_cl(cert_list_t *cl,
                                                const char *digest,
                                                time_t now);
static void list_pending_fpsk_downloads(fp_pair_map_t *result);

/** List of certificates for a single authority, and download status for
 * latest certificate.
 */
struct cert_list_t {
  /*
   * The keys of download status map are cert->signing_key_digest for pending
   * downloads by (identity digest/signing key digest) pair; functions such
   * as authority_cert_get_by_digest() already assume these are unique.
   */
  struct digest_ds_map_t *dl_status_map;
  /* There is also a dlstatus for the download by identity key only */
  download_status_t dl_status_by_id;
  smartlist_t *certs;
};
/** Map from v3 identity key digest to cert_list_t. */
static digestmap_t *trusted_dir_certs = NULL;

/** True iff any key certificate in at least one member of
 * <b>trusted_dir_certs</b> has changed since we last flushed the
 * certificates to disk. */
static int trusted_dir_servers_certs_changed = 0;

/** Initialise schedule, want_authority, and increment_on in the download
 * status dlstatus, then call download_status_reset() on it.
 * It is safe to call this function or download_status_reset() multiple times
 * on a new dlstatus. But it should *not* be called after a dlstatus has been
 * used to count download attempts or failures. */
static void
download_status_cert_init(download_status_t *dlstatus)
{
  dlstatus->schedule = DL_SCHED_CONSENSUS;
  dlstatus->want_authority = DL_WANT_ANY_DIRSERVER;
  dlstatus->increment_on = DL_SCHED_INCREMENT_FAILURE;
  dlstatus->last_backoff_position = 0;
  dlstatus->last_delay_used = 0;

  /* Use the new schedule to set next_attempt_at */
  download_status_reset(dlstatus);
}

/** Reset the download status of a specified element in a dsmap */
static void
download_status_reset_by_sk_in_cl(cert_list_t *cl, const char *digest)
{
  download_status_t *dlstatus = NULL;

  tor_assert(cl);
  tor_assert(digest);

  /* Make sure we have a dsmap */
  if (!(cl->dl_status_map)) {
    cl->dl_status_map = dsmap_new();
  }
  /* Look for a download_status_t in the map with this digest */
  dlstatus = dsmap_get(cl->dl_status_map, digest);
  /* Got one? */
  if (!dlstatus) {
    /* Insert before we reset */
    dlstatus = tor_malloc_zero(sizeof(*dlstatus));
    dsmap_set(cl->dl_status_map, digest, dlstatus);
    download_status_cert_init(dlstatus);
  }
  tor_assert(dlstatus);
  /* Go ahead and reset it */
  download_status_reset(dlstatus);
}

/**
 * Return true if the download for this signing key digest in cl is ready
 * to be re-attempted.
 */
static int
download_status_is_ready_by_sk_in_cl(cert_list_t *cl,
                                     const char *digest,
                                     time_t now)
{
  int rv = 0;
  download_status_t *dlstatus = NULL;

  tor_assert(cl);
  tor_assert(digest);

  /* Make sure we have a dsmap */
  if (!(cl->dl_status_map)) {
    cl->dl_status_map = dsmap_new();
  }
  /* Look for a download_status_t in the map with this digest */
  dlstatus = dsmap_get(cl->dl_status_map, digest);
  /* Got one? */
  if (dlstatus) {
    /* Use download_status_is_ready() */
    rv = download_status_is_ready(dlstatus, now);
  } else {
    /*
     * If we don't know anything about it, return 1, since we haven't
     * tried this one before.  We need to create a new entry here,
     * too.
     */
    dlstatus = tor_malloc_zero(sizeof(*dlstatus));
    download_status_cert_init(dlstatus);
    dsmap_set(cl->dl_status_map, digest, dlstatus);
    rv = 1;
  }

  return rv;
}

/** Helper: Return the cert_list_t for an authority whose authority ID is
 * <b>id_digest</b>, allocating a new list if necessary. */
static cert_list_t *
get_cert_list(const char *id_digest)
{
  cert_list_t *cl;
  if (!trusted_dir_certs)
    trusted_dir_certs = digestmap_new();
  cl = digestmap_get(trusted_dir_certs, id_digest);
  if (!cl) {
    cl = tor_malloc_zero(sizeof(cert_list_t));
    download_status_cert_init(&cl->dl_status_by_id);
    cl->certs = smartlist_new();
    cl->dl_status_map = dsmap_new();
    digestmap_set(trusted_dir_certs, id_digest, cl);
  }
  return cl;
}

/** Return a list of authority ID digests with potentially enumerable lists
 * of download_status_t objects; used by controller GETINFO queries.
 */

MOCK_IMPL(smartlist_t *,
list_authority_ids_with_downloads, (void))
{
  smartlist_t *ids = smartlist_new();
  digestmap_iter_t *i;
  const char *digest;
  char *tmp;
  void *cl;

  if (trusted_dir_certs) {
    for (i = digestmap_iter_init(trusted_dir_certs);
         !(digestmap_iter_done(i));
         i = digestmap_iter_next(trusted_dir_certs, i)) {
      /*
       * We always have at least dl_status_by_id to query, so no need to
       * probe deeper than the existence of a cert_list_t.
       */
      digestmap_iter_get(i, &digest, &cl);
      tmp = tor_malloc(DIGEST_LEN);
      memcpy(tmp, digest, DIGEST_LEN);
      smartlist_add(ids, tmp);
    }
  }
  /* else definitely no downloads going since nothing even has a cert list */

  return ids;
}

/** Given an authority ID digest, return a pointer to the default download
 * status, or NULL if there is no such entry in trusted_dir_certs */

MOCK_IMPL(download_status_t *,
id_only_download_status_for_authority_id, (const char *digest))
{
  download_status_t *dl = NULL;
  cert_list_t *cl;

  if (trusted_dir_certs) {
    cl = digestmap_get(trusted_dir_certs, digest);
    if (cl) {
      dl = &(cl->dl_status_by_id);
    }
  }

  return dl;
}

/** Given an authority ID digest, return a smartlist of signing key digests
 * for which download_status_t is potentially queryable, or NULL if no such
 * authority ID digest is known. */

MOCK_IMPL(smartlist_t *,
list_sk_digests_for_authority_id, (const char *digest))
{
  smartlist_t *sks = NULL;
  cert_list_t *cl;
  dsmap_iter_t *i;
  const char *sk_digest;
  char *tmp;
  download_status_t *dl;

  if (trusted_dir_certs) {
    cl = digestmap_get(trusted_dir_certs, digest);
    if (cl) {
      sks = smartlist_new();
      if (cl->dl_status_map) {
        for (i = dsmap_iter_init(cl->dl_status_map);
             !(dsmap_iter_done(i));
             i = dsmap_iter_next(cl->dl_status_map, i)) {
          /* Pull the digest out and add it to the list */
          dsmap_iter_get(i, &sk_digest, &dl);
          tmp = tor_malloc(DIGEST_LEN);
          memcpy(tmp, sk_digest, DIGEST_LEN);
          smartlist_add(sks, tmp);
        }
      }
    }
  }

  return sks;
}

/** Given an authority ID digest and a signing key digest, return the
 * download_status_t or NULL if none exists. */

MOCK_IMPL(download_status_t *,
download_status_for_authority_id_and_sk,(const char *id_digest,
                                         const char *sk_digest))
{
  download_status_t *dl = NULL;
  cert_list_t *cl = NULL;

  if (trusted_dir_certs) {
    cl = digestmap_get(trusted_dir_certs, id_digest);
    if (cl && cl->dl_status_map) {
      dl = dsmap_get(cl->dl_status_map, sk_digest);
    }
  }

  return dl;
}

#define cert_list_free(val) \
  FREE_AND_NULL(cert_list_t, cert_list_free_, (val))

/** Release all space held by a cert_list_t */
static void
cert_list_free_(cert_list_t *cl)
{
  if (!cl)
    return;

  SMARTLIST_FOREACH(cl->certs, authority_cert_t *, cert,
                    authority_cert_free(cert));
  smartlist_free(cl->certs);
  dsmap_free(cl->dl_status_map, tor_free_);
  tor_free(cl);
}

/** Wrapper for cert_list_free so we can pass it to digestmap_free */
static void
cert_list_free_void(void *cl)
{
  cert_list_free_(cl);
}

/** Reload the cached v3 key certificates from the cached-certs file in
 * the data directory. Return 0 on success, -1 on failure. */
int
trusted_dirs_reload_certs(void)
{
  char *filename;
  char *contents;
  int r;

  filename = get_cachedir_fname("cached-certs");
  contents = read_file_to_str(filename, RFTS_IGNORE_MISSING, NULL);
  tor_free(filename);
  if (!contents)
    return 0;
  r = trusted_dirs_load_certs_from_string(
        contents,
        TRUSTED_DIRS_CERTS_SRC_FROM_STORE, 1, NULL);
  tor_free(contents);
  return r;
}

/** Helper: return true iff we already have loaded the exact cert
 * <b>cert</b>. */
static inline int
already_have_cert(authority_cert_t *cert)
{
  cert_list_t *cl = get_cert_list(cert->cache_info.identity_digest);

  SMARTLIST_FOREACH(cl->certs, authority_cert_t *, c,
  {
    if (tor_memeq(c->cache_info.signed_descriptor_digest,
                cert->cache_info.signed_descriptor_digest,
                DIGEST_LEN))
      return 1;
  });
  return 0;
}

/** Load a bunch of new key certificates from the string <b>contents</b>.  If
 * <b>source</b> is TRUSTED_DIRS_CERTS_SRC_FROM_STORE, the certificates are
 * from the cache, and we don't need to flush them to disk.  If we are a
 * dirauth loading our own cert, source is TRUSTED_DIRS_CERTS_SRC_SELF.
 * Otherwise, source is download type: TRUSTED_DIRS_CERTS_SRC_DL_BY_ID_DIGEST
 * or TRUSTED_DIRS_CERTS_SRC_DL_BY_ID_SK_DIGEST.  If <b>flush</b> is true, we
 * need to flush any changed certificates to disk now.  Return 0 on success,
 * -1 if any certs fail to parse.
 *
 * If source_dir is non-NULL, it's the identity digest for a directory that
 * we've just successfully retrieved certificates from, so try it first to
 * fetch any missing certificates.
 */
int
trusted_dirs_load_certs_from_string(const char *contents, int source,
                                    int flush, const char *source_dir)
{
  dir_server_t *ds;
  const char *s, *eos;
  int failure_code = 0;
  int from_store = (source == TRUSTED_DIRS_CERTS_SRC_FROM_STORE);
  int added_trusted_cert = 0;

  for (s = contents; *s; s = eos) {
    authority_cert_t *cert = authority_cert_parse_from_string(s, strlen(s),
                                                              &eos);
    cert_list_t *cl;
    if (!cert) {
      failure_code = -1;
      break;
    }
    ds = trusteddirserver_get_by_v3_auth_digest(
                                       cert->cache_info.identity_digest);
    log_debug(LD_DIR, "Parsed certificate for %s",
              ds ? ds->nickname : "unknown authority");

    if (already_have_cert(cert)) {
      /* we already have this one. continue. */
      log_info(LD_DIR, "Skipping %s certificate for %s that we "
               "already have.",
               from_store ? "cached" : "downloaded",
               ds ? ds->nickname : "an old or new authority");

      /*
       * A duplicate on download should be treated as a failure, so we call
       * authority_cert_dl_failed() to reset the download status to make sure
       * we can't try again.  Since we've implemented the fp-sk mechanism
       * to download certs by signing key, this should be much rarer than it
       * was and is perhaps cause for concern.
       */
      if (!from_store) {
        if (authdir_mode(get_options())) {
          log_warn(LD_DIR,
                   "Got a certificate for %s, but we already have it. "
                   "Maybe they haven't updated it. Waiting for a while.",
                   ds ? ds->nickname : "an old or new authority");
        } else {
          log_info(LD_DIR,
                   "Got a certificate for %s, but we already have it. "
                   "Maybe they haven't updated it. Waiting for a while.",
                   ds ? ds->nickname : "an old or new authority");
        }

        /*
         * This is where we care about the source; authority_cert_dl_failed()
         * needs to know whether the download was by fp or (fp,sk) pair to
         * twiddle the right bit in the download map.
         */
        if (source == TRUSTED_DIRS_CERTS_SRC_DL_BY_ID_DIGEST) {
          authority_cert_dl_failed(cert->cache_info.identity_digest,
                                   NULL, 404);
        } else if (source == TRUSTED_DIRS_CERTS_SRC_DL_BY_ID_SK_DIGEST) {
          authority_cert_dl_failed(cert->cache_info.identity_digest,
                                   cert->signing_key_digest, 404);
        }
      }

      authority_cert_free(cert);
      continue;
    }

    if (ds) {
      added_trusted_cert = 1;
      log_info(LD_DIR, "Adding %s certificate for directory authority %s with "
               "signing key %s", from_store ? "cached" : "downloaded",
               ds->nickname, hex_str(cert->signing_key_digest,DIGEST_LEN));
    } else {
      int adding = we_want_to_fetch_unknown_auth_certs(get_options());
      log_info(LD_DIR, "%s %s certificate for unrecognized directory "
               "authority with signing key %s",
               adding ? "Adding" : "Not adding",
               from_store ? "cached" : "downloaded",
               hex_str(cert->signing_key_digest,DIGEST_LEN));
      if (!adding) {
        authority_cert_free(cert);
        continue;
      }
    }

    cl = get_cert_list(cert->cache_info.identity_digest);
    smartlist_add(cl->certs, cert);
    if (ds && cert->cache_info.published_on > ds->addr_current_at) {
      /* Check to see whether we should update our view of the authority's
       * address. */
      if (!tor_addr_is_null(&cert->ipv4_addr) && cert->ipv4_dirport &&
          (!tor_addr_eq(&ds->ipv4_addr, &cert->ipv4_addr) ||
           ds->ipv4_dirport != cert->ipv4_dirport)) {
        log_notice(LD_DIR, "Updating address for directory authority %s "
                   "from %s:%"PRIu16" to %s:%"PRIu16" based on certificate.",
                   ds->nickname, ds->address, ds->ipv4_dirport,
                   fmt_addr(&cert->ipv4_addr), cert->ipv4_dirport);
        tor_addr_copy(&ds->ipv4_addr, &cert->ipv4_addr);
        ds->ipv4_dirport = cert->ipv4_dirport;
      }
      ds->addr_current_at = cert->cache_info.published_on;
    }

    if (!from_store)
      trusted_dir_servers_certs_changed = 1;
  }

  if (flush)
    trusted_dirs_flush_certs_to_disk();

  /* call this even if failure_code is <0, since some certs might have
   * succeeded, but only pass source_dir if there were no failures,
   * and at least one more authority certificate was added to the store.
   * This avoids retrying a directory that's serving bad or entirely duplicate
   * certificates. */
  if (failure_code == 0 && added_trusted_cert) {
    networkstatus_note_certs_arrived(source_dir);
  } else {
    networkstatus_note_certs_arrived(NULL);
  }

  return failure_code;
}

/** Save all v3 key certificates to the cached-certs file. */
void
trusted_dirs_flush_certs_to_disk(void)
{
  char *filename;
  smartlist_t *chunks;

  if (!trusted_dir_servers_certs_changed || !trusted_dir_certs)
    return;

  chunks = smartlist_new();
  DIGESTMAP_FOREACH(trusted_dir_certs, key, cert_list_t *, cl) {
    SMARTLIST_FOREACH(cl->certs, authority_cert_t *, cert,
          {
            sized_chunk_t *c = tor_malloc(sizeof(sized_chunk_t));
            c->bytes = cert->cache_info.signed_descriptor_body;
            c->len = cert->cache_info.signed_descriptor_len;
            smartlist_add(chunks, c);
          });
  } DIGESTMAP_FOREACH_END;

  filename = get_cachedir_fname("cached-certs");
  if (write_chunks_to_file(filename, chunks, 0, 0)) {
    log_warn(LD_FS, "Error writing certificates to disk.");
  }
  tor_free(filename);
  SMARTLIST_FOREACH(chunks, sized_chunk_t *, c, tor_free(c));
  smartlist_free(chunks);

  trusted_dir_servers_certs_changed = 0;
}

static int
compare_certs_by_pubdates(const void **_a, const void **_b)
{
  const authority_cert_t *cert1 = *_a, *cert2=*_b;

  if (cert1->cache_info.published_on < cert2->cache_info.published_on)
    return -1;
  else if (cert1->cache_info.published_on >  cert2->cache_info.published_on)
    return 1;
  else
    return 0;
}

/** Remove all expired v3 authority certificates that have been superseded for
 * more than 48 hours or, if not expired, that were published more than 7 days
 * before being superseded. (If the most recent cert was published more than 48
 * hours ago, then we aren't going to get any consensuses signed with older
 * keys.) */
void
trusted_dirs_remove_old_certs(void)
{
  time_t now = time(NULL);
#define DEAD_CERT_LIFETIME (2*24*60*60)
#define SUPERSEDED_CERT_LIFETIME (2*24*60*60)
  if (!trusted_dir_certs)
    return;

  DIGESTMAP_FOREACH(trusted_dir_certs, key, cert_list_t *, cl) {
    /* Sort the list from first-published to last-published */
    smartlist_sort(cl->certs, compare_certs_by_pubdates);

    SMARTLIST_FOREACH_BEGIN(cl->certs, authority_cert_t *, cert) {
      if (cert_sl_idx == smartlist_len(cl->certs) - 1) {
        /* This is the most recently published cert.  Keep it. */
        continue;
      }
      authority_cert_t *next_cert = smartlist_get(cl->certs, cert_sl_idx+1);
      const time_t next_cert_published = next_cert->cache_info.published_on;
      if (next_cert_published > now) {
        /* All later certs are published in the future. Keep everything
         * we didn't discard. */
        break;
      }
      int should_remove = 0;
      if (cert->expires + DEAD_CERT_LIFETIME < now) {
        /* Certificate has been expired for at least DEAD_CERT_LIFETIME.
         * Remove it. */
        should_remove = 1;
      } else if (next_cert_published + SUPERSEDED_CERT_LIFETIME < now) {
        /* Certificate has been superseded for OLD_CERT_LIFETIME.
         * Remove it.
         */
        should_remove = 1;
      }
      if (should_remove) {
        SMARTLIST_DEL_CURRENT_KEEPORDER(cl->certs, cert);
        authority_cert_free(cert);
        trusted_dir_servers_certs_changed = 1;
      }
    } SMARTLIST_FOREACH_END(cert);

  } DIGESTMAP_FOREACH_END;
#undef DEAD_CERT_LIFETIME
#undef OLD_CERT_LIFETIME

  trusted_dirs_flush_certs_to_disk();
}

/** Return the newest v3 authority certificate whose v3 authority identity key
 * has digest <b>id_digest</b>.  Return NULL if no such authority is known,
 * or it has no certificate. */
authority_cert_t *
authority_cert_get_newest_by_id(const char *id_digest)
{
  cert_list_t *cl;
  authority_cert_t *best = NULL;
  if (!trusted_dir_certs ||
      !(cl = digestmap_get(trusted_dir_certs, id_digest)))
    return NULL;

  SMARTLIST_FOREACH(cl->certs, authority_cert_t *, cert,
  {
    if (!best || cert->cache_info.published_on > best->cache_info.published_on)
      best = cert;
  });
  return best;
}

/** Return the newest v3 authority certificate whose directory signing key has
 * digest <b>sk_digest</b>. Return NULL if no such certificate is known.
 */
authority_cert_t *
authority_cert_get_by_sk_digest(const char *sk_digest)
{
  authority_cert_t *c;
  if (!trusted_dir_certs)
    return NULL;

  if ((c = get_my_v3_authority_cert()) &&
      tor_memeq(c->signing_key_digest, sk_digest, DIGEST_LEN))
    return c;
  if ((c = get_my_v3_legacy_cert()) &&
      tor_memeq(c->signing_key_digest, sk_digest, DIGEST_LEN))
    return c;

  DIGESTMAP_FOREACH(trusted_dir_certs, key, cert_list_t *, cl) {
    SMARTLIST_FOREACH(cl->certs, authority_cert_t *, cert,
    {
      if (tor_memeq(cert->signing_key_digest, sk_digest, DIGEST_LEN))
        return cert;
    });
  } DIGESTMAP_FOREACH_END;
  return NULL;
}

/** Return the v3 authority certificate with signing key matching
 * <b>sk_digest</b>, for the authority with identity digest <b>id_digest</b>.
 * Return NULL if no such authority is known. */
authority_cert_t *
authority_cert_get_by_digests(const char *id_digest,
                              const char *sk_digest)
{
  cert_list_t *cl;
  if (!trusted_dir_certs ||
      !(cl = digestmap_get(trusted_dir_certs, id_digest)))
    return NULL;
  SMARTLIST_FOREACH(cl->certs, authority_cert_t *, cert,
    if (tor_memeq(cert->signing_key_digest, sk_digest, DIGEST_LEN))
      return cert; );

  return NULL;
}

/** Add every known authority_cert_t to <b>certs_out</b>. */
void
authority_cert_get_all(smartlist_t *certs_out)
{
  tor_assert(certs_out);
  if (!trusted_dir_certs)
    return;

  DIGESTMAP_FOREACH(trusted_dir_certs, key, cert_list_t *, cl) {
    SMARTLIST_FOREACH(cl->certs, authority_cert_t *, c,
                      smartlist_add(certs_out, c));
  } DIGESTMAP_FOREACH_END;
}

/** Called when an attempt to download a certificate with the authority with
 * ID <b>id_digest</b> and, if not NULL, signed with key signing_key_digest
 * fails with HTTP response code <b>status</b>: remember the failure, so we
 * don't try again immediately. */
void
authority_cert_dl_failed(const char *id_digest,
                         const char *signing_key_digest, int status)
{
  cert_list_t *cl;
  download_status_t *dlstatus = NULL;
  char id_digest_str[2*DIGEST_LEN+1];
  char sk_digest_str[2*DIGEST_LEN+1];

  if (!trusted_dir_certs ||
      !(cl = digestmap_get(trusted_dir_certs, id_digest)))
    return;

  /*
   * Are we noting a failed download of the latest cert for the id digest,
   * or of a download by (id, signing key) digest pair?
   */
  if (!signing_key_digest) {
    /* Just by id digest */
    download_status_failed(&cl->dl_status_by_id, status);
  } else {
    /* Reset by (id, signing key) digest pair
     *
     * Look for a download_status_t in the map with this digest
     */
    dlstatus = dsmap_get(cl->dl_status_map, signing_key_digest);
    /* Got one? */
    if (dlstatus) {
      download_status_failed(dlstatus, status);
    } else {
      /*
       * Do this rather than hex_str(), since hex_str clobbers
       * old results and we call twice in the param list.
       */
      base16_encode(id_digest_str, sizeof(id_digest_str),
                    id_digest, DIGEST_LEN);
      base16_encode(sk_digest_str, sizeof(sk_digest_str),
                    signing_key_digest, DIGEST_LEN);
      log_warn(LD_BUG,
               "Got failure for cert fetch with (fp,sk) = (%s,%s), with "
               "status %d, but knew nothing about the download.",
               id_digest_str, sk_digest_str, status);
    }
  }
}

static const char *BAD_SIGNING_KEYS[] = {
  "09CD84F751FD6E955E0F8ADB497D5401470D697E", // Expires 2015-01-11 16:26:31
  "0E7E9C07F0969D0468AD741E172A6109DC289F3C", // Expires 2014-08-12 10:18:26
  "57B85409891D3FB32137F642FDEDF8B7F8CDFDCD", // Expires 2015-02-11 17:19:09
  "87326329007AF781F587AF5B594E540B2B6C7630", // Expires 2014-07-17 11:10:09
  "98CC82342DE8D298CF99D3F1A396475901E0D38E", // Expires 2014-11-10 13:18:56
  "9904B52336713A5ADCB13E4FB14DC919E0D45571", // Expires 2014-04-20 20:01:01
  "9DCD8E3F1DD1597E2AD476BBA28A1A89F3095227", // Expires 2015-01-16 03:52:30
  "A61682F34B9BB9694AC98491FE1ABBFE61923941", // Expires 2014-06-11 09:25:09
  "B59F6E99C575113650C99F1C425BA7B20A8C071D", // Expires 2014-07-31 13:22:10
  "D27178388FA75B96D37FA36E0B015227DDDBDA51", // Expires 2014-08-04 04:01:57
  NULL,
};

/** Return true iff <b>cert</b> authenticates some atuhority signing key
 * which, because of the old openssl heartbleed vulnerability, should
 * never be trusted. */
int
authority_cert_is_denylisted(const authority_cert_t *cert)
{
  char hex_digest[HEX_DIGEST_LEN+1];
  int i;
  base16_encode(hex_digest, sizeof(hex_digest),
                cert->signing_key_digest, sizeof(cert->signing_key_digest));

  for (i = 0; BAD_SIGNING_KEYS[i]; ++i) {
    if (!strcasecmp(hex_digest, BAD_SIGNING_KEYS[i])) {
      return 1;
    }
  }
  return 0;
}

/** Return true iff when we've been getting enough failures when trying to
 * download the certificate with ID digest <b>id_digest</b> that we're willing
 * to start bugging the user about it. */
int
authority_cert_dl_looks_uncertain(const char *id_digest)
{
#define N_AUTH_CERT_DL_FAILURES_TO_BUG_USER 2
  cert_list_t *cl;
  int n_failures;
  if (!trusted_dir_certs ||
      !(cl = digestmap_get(trusted_dir_certs, id_digest)))
    return 0;

  n_failures = download_status_get_n_failures(&cl->dl_status_by_id);
  return n_failures >= N_AUTH_CERT_DL_FAILURES_TO_BUG_USER;
}

/* Fetch the authority certificates specified in resource.
 * If we are a bridge client, and node is a configured bridge, fetch from node
 * using dir_hint as the fingerprint. Otherwise, if rs is not NULL, fetch from
 * rs. Otherwise, fetch from a random directory mirror. */
static void
authority_certs_fetch_resource_impl(const char *resource,
                                    const char *dir_hint,
                                    const node_t *node,
                                    const routerstatus_t *rs)
{
  const or_options_t *options = get_options();
  int get_via_tor = purpose_needs_anonymity(DIR_PURPOSE_FETCH_CERTIFICATE, 0,
                                            resource);

  /* Make sure bridge clients never connect to anything but a bridge */
  if (options->UseBridges) {
    if (node && !node_is_a_configured_bridge(node)) {
      /* If we're using bridges, and node is not a bridge, use a 3-hop path. */
      get_via_tor = 1;
    } else if (!node) {
      /* If we're using bridges, and there's no node, use a 3-hop path. */
      get_via_tor = 1;
    }
  }

  const dir_indirection_t indirection = get_via_tor ? DIRIND_ANONYMOUS
                                                    : DIRIND_ONEHOP;

  directory_request_t *req = NULL;
  /* If we've just downloaded a consensus from a bridge, re-use that
   * bridge */
  if (options->UseBridges && node && node->ri && !get_via_tor) {
    /* clients always make OR connections to bridges */
    tor_addr_port_t or_ap;
    /* we are willing to use a non-preferred address if we need to */
    reachable_addr_choose_from_node(node, FIREWALL_OR_CONNECTION, 0,
                                         &or_ap);

    req = directory_request_new(DIR_PURPOSE_FETCH_CERTIFICATE);
    directory_request_set_or_addr_port(req, &or_ap);
    if (dir_hint)
      directory_request_set_directory_id_digest(req, dir_hint);
  } else if (rs) {
    /* And if we've just downloaded a consensus from a directory, re-use that
     * directory */
    req = directory_request_new(DIR_PURPOSE_FETCH_CERTIFICATE);
    directory_request_set_routerstatus(req, rs);
  }

  if (req) {
    /* We've set up a request object -- fill in the other request fields, and
     * send the request.  */
    directory_request_set_indirection(req, indirection);
    directory_request_set_resource(req, resource);
    directory_initiate_request(req);
    directory_request_free(req);
    return;
  }

  /* Otherwise, we want certs from a random fallback or directory
   * mirror, because they will almost always succeed. */
  directory_get_from_dirserver(DIR_PURPOSE_FETCH_CERTIFICATE, 0,
                               resource, PDS_RETRY_IF_NO_SERVERS,
                               DL_WANT_ANY_DIRSERVER);
}

/** Try to download any v3 authority certificates that we may be missing.  If
 * <b>status</b> is provided, try to get all the ones that were used to sign
 * <b>status</b>.  Additionally, try to have a non-expired certificate for
 * every V3 authority in trusted_dir_servers.  Don't fetch certificates we
 * already have.
 *
 * If dir_hint is non-NULL, it's the identity digest for a directory that
 * we've just successfully retrieved a consensus or certificates from, so try
 * it first to fetch any missing certificates.
 **/
void
authority_certs_fetch_missing(networkstatus_t *status, time_t now,
                              const char *dir_hint)
{
  /*
   * The pending_id digestmap tracks pending certificate downloads by
   * identity digest; the pending_cert digestmap tracks pending downloads
   * by (identity digest, signing key digest) pairs.
   */
  digestmap_t *pending_id;
  fp_pair_map_t *pending_cert;
  /*
   * The missing_id_digests smartlist will hold a list of id digests
   * we want to fetch the newest cert for; the missing_cert_digests
   * smartlist will hold a list of fp_pair_t with an identity and
   * signing key digest.
   */
  smartlist_t *missing_cert_digests, *missing_id_digests;
  char *resource = NULL;
  cert_list_t *cl;
  const or_options_t *options = get_options();
  const int keep_unknown = we_want_to_fetch_unknown_auth_certs(options);
  fp_pair_t *fp_tmp = NULL;
  char id_digest_str[2*DIGEST_LEN+1];
  char sk_digest_str[2*DIGEST_LEN+1];

  if (should_delay_dir_fetches(options, NULL))
    return;

  pending_cert = fp_pair_map_new();
  pending_id = digestmap_new();
  missing_cert_digests = smartlist_new();
  missing_id_digests = smartlist_new();

  /*
   * First, we get the lists of already pending downloads so we don't
   * duplicate effort.
   */
  list_pending_downloads(pending_id, NULL,
                         DIR_PURPOSE_FETCH_CERTIFICATE, "fp/");
  list_pending_fpsk_downloads(pending_cert);

  /*
   * Now, we download any trusted authority certs we don't have by
   * identity digest only.  This gets the latest cert for that authority.
   */
  SMARTLIST_FOREACH_BEGIN(router_get_trusted_dir_servers(),
                          dir_server_t *, ds) {
    int found = 0;
    if (!(ds->type & V3_DIRINFO))
      continue;
    if (smartlist_contains_digest(missing_id_digests,
                                  ds->v3_identity_digest))
      continue;
    cl = get_cert_list(ds->v3_identity_digest);
    SMARTLIST_FOREACH_BEGIN(cl->certs, authority_cert_t *, cert) {
      if (now < cert->expires) {
        /* It's not expired, and we weren't looking for something to
         * verify a consensus with.  Call it done. */
        download_status_reset(&(cl->dl_status_by_id));
        /* No sense trying to download it specifically by signing key hash */
        download_status_reset_by_sk_in_cl(cl, cert->signing_key_digest);
        found = 1;
        break;
      }
    } SMARTLIST_FOREACH_END(cert);
    if (!found &&
        download_status_is_ready(&(cl->dl_status_by_id), now) &&
        !digestmap_get(pending_id, ds->v3_identity_digest)) {
      log_info(LD_DIR,
               "No current certificate known for authority %s "
               "(ID digest %s); launching request.",
               ds->nickname, hex_str(ds->v3_identity_digest, DIGEST_LEN));
      smartlist_add(missing_id_digests, ds->v3_identity_digest);
    }
  } SMARTLIST_FOREACH_END(ds);

  /*
   * Next, if we have a consensus, scan through it and look for anything
   * signed with a key from a cert we don't have.  Those get downloaded
   * by (fp,sk) pair, but if we don't know any certs at all for the fp
   * (identity digest), and it's one of the trusted dir server certs
   * we started off above or a pending download in pending_id, don't
   * try to get it yet.  Most likely, the one we'll get for that will
   * have the right signing key too, and we'd just be downloading
   * redundantly.
   */
  if (status) {
    SMARTLIST_FOREACH_BEGIN(status->voters, networkstatus_voter_info_t *,
                            voter) {
      if (!smartlist_len(voter->sigs))
        continue; /* This authority never signed this consensus, so don't
                   * go looking for a cert with key digest 0000000000. */
      if (!keep_unknown &&
          !trusteddirserver_get_by_v3_auth_digest(voter->identity_digest))
        continue; /* We don't want unknown certs, and we don't know this
                   * authority.*/

      /*
       * If we don't know *any* cert for this authority, and a download by ID
       * is pending or we added it to missing_id_digests above, skip this
       * one for now to avoid duplicate downloads.
       */
      cl = get_cert_list(voter->identity_digest);
      if (smartlist_len(cl->certs) == 0) {
        /* We have no certs at all for this one */

        /* Do we have a download of one pending? */
        if (digestmap_get(pending_id, voter->identity_digest))
          continue;

        /*
         * Are we about to launch a download of one due to the trusted
         * dir server check above?
         */
        if (smartlist_contains_digest(missing_id_digests,
                                      voter->identity_digest))
          continue;
      }

      SMARTLIST_FOREACH_BEGIN(voter->sigs, document_signature_t *, sig) {
        authority_cert_t *cert =
          authority_cert_get_by_digests(voter->identity_digest,
                                        sig->signing_key_digest);
        if (cert) {
          if (now < cert->expires)
            download_status_reset_by_sk_in_cl(cl, sig->signing_key_digest);
          continue;
        }
        if (download_status_is_ready_by_sk_in_cl(
              cl, sig->signing_key_digest, now) &&
            !fp_pair_map_get_by_digests(pending_cert,
                                        voter->identity_digest,
                                        sig->signing_key_digest)) {
          /*
           * Do this rather than hex_str(), since hex_str clobbers
           * old results and we call twice in the param list.
           */
          base16_encode(id_digest_str, sizeof(id_digest_str),
                        voter->identity_digest, DIGEST_LEN);
          base16_encode(sk_digest_str, sizeof(sk_digest_str),
                        sig->signing_key_digest, DIGEST_LEN);

          if (voter->nickname) {
            log_info(LD_DIR,
                     "We're missing a certificate from authority %s "
                     "(ID digest %s) with signing key %s: "
                     "launching request.",
                     voter->nickname, id_digest_str, sk_digest_str);
          } else {
            log_info(LD_DIR,
                     "We're missing a certificate from authority ID digest "
                     "%s with signing key %s: launching request.",
                     id_digest_str, sk_digest_str);
          }

          /* Allocate a new fp_pair_t to append */
          fp_tmp = tor_malloc(sizeof(*fp_tmp));
          memcpy(fp_tmp->first, voter->identity_digest, sizeof(fp_tmp->first));
          memcpy(fp_tmp->second, sig->signing_key_digest,
                 sizeof(fp_tmp->second));
          smartlist_add(missing_cert_digests, fp_tmp);
        }
      } SMARTLIST_FOREACH_END(sig);
    } SMARTLIST_FOREACH_END(voter);
  }

  /* Bridge clients look up the node for the dir_hint */
  const node_t *node = NULL;
  /* All clients, including bridge clients, look up the routerstatus for the
   * dir_hint */
  const routerstatus_t *rs = NULL;

  /* If we still need certificates, try the directory that just successfully
   * served us a consensus or certificates.
   * As soon as the directory fails to provide additional certificates, we try
   * another, randomly selected directory. This avoids continual retries.
   * (We only ever have one outstanding request per certificate.)
   */
  if (dir_hint) {
    if (options->UseBridges) {
      /* Bridge clients try the nodelist. If the dir_hint is from an authority,
       * or something else fetched over tor, we won't find the node here, but
       * we will find the rs. */
      node = node_get_by_id(dir_hint);
    }

    /* All clients try the consensus routerstatus, then the fallback
     * routerstatus */
    rs = router_get_consensus_status_by_id(dir_hint);
    if (!rs) {
      /* This will also find authorities */
      const dir_server_t *ds = router_get_fallback_dirserver_by_digest(
                                                                    dir_hint);
      if (ds) {
        rs = &ds->fake_status;
      }
    }

    if (!node && !rs) {
      log_warn(LD_BUG, "Directory %s delivered a consensus, but %s"
               "no routerstatus could be found for it.",
               options->UseBridges ? "no node and " : "",
               hex_str(dir_hint, DIGEST_LEN));
    }
  }

  /* Do downloads by identity digest */
  if (smartlist_len(missing_id_digests) > 0) {
    int need_plus = 0;
    smartlist_t *fps = smartlist_new();

    smartlist_add_strdup(fps, "fp/");

    SMARTLIST_FOREACH_BEGIN(missing_id_digests, const char *, d) {
      char *fp = NULL;

      if (digestmap_get(pending_id, d))
        continue;

      base16_encode(id_digest_str, sizeof(id_digest_str),
                    d, DIGEST_LEN);

      if (need_plus) {
        tor_asprintf(&fp, "+%s", id_digest_str);
      } else {
        /* No need for tor_asprintf() in this case; first one gets no '+' */
        fp = tor_strdup(id_digest_str);
        need_plus = 1;
      }

      smartlist_add(fps, fp);
    } SMARTLIST_FOREACH_END(d);

    if (smartlist_len(fps) > 1) {
      resource = smartlist_join_strings(fps, "", 0, NULL);
      /* node and rs are directories that just gave us a consensus or
       * certificates */
      authority_certs_fetch_resource_impl(resource, dir_hint, node, rs);
      tor_free(resource);
    }
    /* else we didn't add any: they were all pending */

    SMARTLIST_FOREACH(fps, char *, cp, tor_free(cp));
    smartlist_free(fps);
  }

  /* Do downloads by identity digest/signing key pair */
  if (smartlist_len(missing_cert_digests) > 0) {
    int need_plus = 0;
    smartlist_t *fp_pairs = smartlist_new();

    smartlist_add_strdup(fp_pairs, "fp-sk/");

    SMARTLIST_FOREACH_BEGIN(missing_cert_digests, const fp_pair_t *, d) {
      char *fp_pair = NULL;

      if (fp_pair_map_get(pending_cert, d))
        continue;

      /* Construct string encodings of the digests */
      base16_encode(id_digest_str, sizeof(id_digest_str),
                    d->first, DIGEST_LEN);
      base16_encode(sk_digest_str, sizeof(sk_digest_str),
                    d->second, DIGEST_LEN);

      /* Now tor_asprintf() */
      if (need_plus) {
        tor_asprintf(&fp_pair, "+%s-%s", id_digest_str, sk_digest_str);
      } else {
        /* First one in the list doesn't get a '+' */
        tor_asprintf(&fp_pair, "%s-%s", id_digest_str, sk_digest_str);
        need_plus = 1;
      }

      /* Add it to the list of pairs to request */
      smartlist_add(fp_pairs, fp_pair);
    } SMARTLIST_FOREACH_END(d);

    if (smartlist_len(fp_pairs) > 1) {
      resource = smartlist_join_strings(fp_pairs, "", 0, NULL);
      /* node and rs are directories that just gave us a consensus or
       * certificates */
      authority_certs_fetch_resource_impl(resource, dir_hint, node, rs);
      tor_free(resource);
    }
    /* else they were all pending */

    SMARTLIST_FOREACH(fp_pairs, char *, p, tor_free(p));
    smartlist_free(fp_pairs);
  }

  smartlist_free(missing_id_digests);
  SMARTLIST_FOREACH(missing_cert_digests, fp_pair_t *, p, tor_free(p));
  smartlist_free(missing_cert_digests);
  digestmap_free(pending_id, NULL);
  fp_pair_map_free(pending_cert, NULL);
}

void
authcert_free_all(void)
{
  if (trusted_dir_certs) {
    digestmap_free(trusted_dir_certs, cert_list_free_void);
    trusted_dir_certs = NULL;
  }
}

/** Free storage held in <b>cert</b>. */
void
authority_cert_free_(authority_cert_t *cert)
{
  if (!cert)
    return;

  tor_free(cert->cache_info.signed_descriptor_body);
  crypto_pk_free(cert->signing_key);
  crypto_pk_free(cert->identity_key);

  tor_free(cert);
}

/** For every certificate we are currently downloading by (identity digest,
 * signing key digest) pair, set result[fp_pair] to (void *1).
 */
static void
list_pending_fpsk_downloads(fp_pair_map_t *result)
{
  const char *pfx = "fp-sk/";
  smartlist_t *tmp;
  smartlist_t *conns;
  const char *resource;

  tor_assert(result);

  tmp = smartlist_new();
  conns = get_connection_array();

  SMARTLIST_FOREACH_BEGIN(conns, connection_t *, conn) {
    if (conn->type == CONN_TYPE_DIR &&
        conn->purpose == DIR_PURPOSE_FETCH_CERTIFICATE &&
        !conn->marked_for_close) {
      resource = TO_DIR_CONN(conn)->requested_resource;
      if (!strcmpstart(resource, pfx))
        dir_split_resource_into_fingerprint_pairs(resource + strlen(pfx),
                                                  tmp);
    }
  } SMARTLIST_FOREACH_END(conn);

  SMARTLIST_FOREACH_BEGIN(tmp, fp_pair_t *, fp) {
    fp_pair_map_set(result, fp, (void*)1);
    tor_free(fp);
  } SMARTLIST_FOREACH_END(fp);

  smartlist_free(tmp);
}

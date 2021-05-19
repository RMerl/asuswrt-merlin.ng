/* Copyright (c) 2017-2020, The Tor Project, Inc. */
/* See LICENSE for licensing information */

/**
 * \file consdiffmgr.c
 *
 * \brief consensus diff manager functions
 *
 * This module is run by directory authorities and caches in order
 * to remember a number of past consensus documents, and to generate
 * and serve the diffs from those documents to the latest consensus.
 */

#define CONSDIFFMGR_PRIVATE

#include "core/or/or.h"
#include "app/config/config.h"
#include "feature/dircache/conscache.h"
#include "feature/dircommon/consdiff.h"
#include "feature/dircache/consdiffmgr.h"
#include "core/mainloop/cpuworker.h"
#include "feature/nodelist/networkstatus.h"
#include "feature/dirparse/ns_parse.h"
#include "lib/evloop/compat_libevent.h"
#include "lib/evloop/workqueue.h"
#include "lib/compress/compress.h"
#include "lib/encoding/confline.h"

#include "feature/nodelist/networkstatus_st.h"
#include "feature/nodelist/networkstatus_voter_info_st.h"

/**
 * Labels to apply to items in the conscache object.
 *
 * @{
 */
/* One of DOCTYPE_CONSENSUS or DOCTYPE_CONSENSUS_DIFF */
#define LABEL_DOCTYPE "document-type"
/* The valid-after time for a consensus (or for the target consensus of a
 * diff), encoded as ISO UTC. */
#define LABEL_VALID_AFTER "consensus-valid-after"
/* The fresh-until time for a consensus (or for the target consensus of a
 * diff), encoded as ISO UTC. */
#define LABEL_FRESH_UNTIL "consensus-fresh-until"
/* The valid-until time for a consensus (or for the target consensus of a
 * diff), encoded as ISO UTC. */
#define LABEL_VALID_UNTIL "consensus-valid-until"
/* Comma-separated list of hex-encoded identity digests for the voting
 * authorities. */
#define LABEL_SIGNATORIES "consensus-signatories"
/* A hex encoded SHA3 digest of the object, as compressed (if any) */
#define LABEL_SHA3_DIGEST "sha3-digest"
/* A hex encoded SHA3 digest of the object before compression. */
#define LABEL_SHA3_DIGEST_UNCOMPRESSED "sha3-digest-uncompressed"
/* A hex encoded SHA3 digest-as-signed of a consensus */
#define LABEL_SHA3_DIGEST_AS_SIGNED "sha3-digest-as-signed"
/* The flavor of the consensus or consensuses diff */
#define LABEL_FLAVOR "consensus-flavor"
/* Diff only: the SHA3 digest-as-signed of the source consensus. */
#define LABEL_FROM_SHA3_DIGEST "from-sha3-digest"
/* Diff only: the SHA3 digest-in-full of the target consensus. */
#define LABEL_TARGET_SHA3_DIGEST "target-sha3-digest"
/* Diff only: the valid-after date of the source consensus. */
#define LABEL_FROM_VALID_AFTER "from-valid-after"
/* What kind of compression was used? */
#define LABEL_COMPRESSION_TYPE "compression"
/** @} */

#define DOCTYPE_CONSENSUS "consensus"
#define DOCTYPE_CONSENSUS_DIFF "consensus-diff"

/**
 * Underlying directory that stores consensuses and consensus diffs.  Don't
 * use this directly: use cdm_cache_get() instead.
 */
static consensus_cache_t *cons_diff_cache = NULL;
/**
 * If true, we have learned at least one new consensus since the
 * consensus cache was last up-to-date.
 */
static int cdm_cache_dirty = 0;
/**
 * If true, we have scanned the cache to update our hashtable of diffs.
 */
static int cdm_cache_loaded = 0;

/**
 * Possible status values for cdm_diff_t.cdm_diff_status
 **/
typedef enum cdm_diff_status_t {
  CDM_DIFF_PRESENT=1,
  CDM_DIFF_IN_PROGRESS=2,
  CDM_DIFF_ERROR=3,
} cdm_diff_status_t;

/** Which methods do we use for precompressing diffs? */
static const compress_method_t compress_diffs_with[] = {
  NO_METHOD,
  GZIP_METHOD,
#ifdef HAVE_LZMA
  LZMA_METHOD,
#endif
#ifdef HAVE_ZSTD
  ZSTD_METHOD,
#endif
};

/**
 * Event for rescanning the cache.
 */
static mainloop_event_t *consdiffmgr_rescan_ev = NULL;

static void consdiffmgr_rescan_cb(mainloop_event_t *ev, void *arg);
static void mark_cdm_cache_dirty(void);

/** How many different methods will we try to use for diff compression? */
STATIC unsigned
n_diff_compression_methods(void)
{
  return ARRAY_LENGTH(compress_diffs_with);
}

/** Which methods do we use for precompressing consensuses? */
static const compress_method_t compress_consensus_with[] = {
  ZLIB_METHOD,
#ifdef HAVE_LZMA
  LZMA_METHOD,
#endif
#ifdef HAVE_ZSTD
  ZSTD_METHOD,
#endif
};

/** How many different methods will we try to use for diff compression? */
STATIC unsigned
n_consensus_compression_methods(void)
{
  return ARRAY_LENGTH(compress_consensus_with);
}

/** For which compression method do we retain old consensuses?  There's no
 * need to keep all of them, since we won't be serving them.  We'll
 * go with ZLIB_METHOD because it's pretty fast and everyone has it.
 */
#define RETAIN_CONSENSUS_COMPRESSED_WITH_METHOD ZLIB_METHOD

/** Handles pointing to the latest consensus entries as compressed and
 * stored. */
static consensus_cache_entry_handle_t *
                  latest_consensus[N_CONSENSUS_FLAVORS]
                                  [ARRAY_LENGTH(compress_consensus_with)];

/** Hashtable node used to remember the current status of the diff
 * from a given sha3 digest to the current consensus.  */
typedef struct cdm_diff_t {
  HT_ENTRY(cdm_diff_t) node;

  /** Consensus flavor for this diff (part of ht key) */
  consensus_flavor_t flavor;
  /** SHA3-256 digest of the consensus that this diff is _from_. (part of the
   * ht key) */
  uint8_t from_sha3[DIGEST256_LEN];
  /** Method by which the diff is compressed. (part of the ht key */
  compress_method_t compress_method;

  /** One of the CDM_DIFF_* values, depending on whether this diff
   * is available, in progress, or impossible to compute. */
  cdm_diff_status_t cdm_diff_status;
  /** SHA3-256 digest of the consensus that this diff is _to. */
  uint8_t target_sha3[DIGEST256_LEN];

  /** Handle to the cache entry for this diff, if any.  We use a handle here
   * to avoid thinking too hard about cache entry lifetime issues. */
  consensus_cache_entry_handle_t *entry;
} cdm_diff_t;

/** Hashtable mapping flavor and source consensus digest to status. */
static HT_HEAD(cdm_diff_ht, cdm_diff_t) cdm_diff_ht = HT_INITIALIZER();

#ifdef _WIN32
   // XXX(ahf): For tor#24857, a contributor suggested that on Windows, the CPU
   // begins to spike at 100% once the number of files handled by the consensus
   // diff manager becomes larger than 64. To see if the issue goes away, we
   // hardcode this value to 64 now while we investigate a better solution.
#  define CACHE_MAX_NUM 64
#else
#  define CACHE_MAX_NUM 128
#endif

/**
 * Configuration for this module
 */
static consdiff_cfg_t consdiff_cfg = {
  // XXXX I'd like to make this number bigger, but it interferes with the
  // XXXX seccomp2 syscall filter, which tops out at BPF_MAXINS (4096)
  // XXXX rules.
  /* .cache_max_num = */ CACHE_MAX_NUM
};

static int consdiffmgr_ensure_space_for_files(int n);
static int consensus_queue_compression_work(const char *consensus,
                                            size_t consensus_len,
                                            const networkstatus_t *as_parsed);
static int consensus_diff_queue_diff_work(consensus_cache_entry_t *diff_from,
                                          consensus_cache_entry_t *diff_to);
static void consdiffmgr_set_cache_flags(void);

/* =====
 * Hashtable setup
 * ===== */

/** Helper: hash the key of a cdm_diff_t. */
static unsigned
cdm_diff_hash(const cdm_diff_t *diff)
{
  uint8_t tmp[DIGEST256_LEN + 2];
  memcpy(tmp, diff->from_sha3, DIGEST256_LEN);
  tmp[DIGEST256_LEN] = (uint8_t) diff->flavor;
  tmp[DIGEST256_LEN+1] = (uint8_t) diff->compress_method;
  return (unsigned) siphash24g(tmp, sizeof(tmp));
}
/** Helper: compare two cdm_diff_t objects for key equality */
static int
cdm_diff_eq(const cdm_diff_t *diff1, const cdm_diff_t *diff2)
{
  return fast_memeq(diff1->from_sha3, diff2->from_sha3, DIGEST256_LEN) &&
    diff1->flavor == diff2->flavor &&
    diff1->compress_method == diff2->compress_method;
}

HT_PROTOTYPE(cdm_diff_ht, cdm_diff_t, node, cdm_diff_hash, cdm_diff_eq);
HT_GENERATE2(cdm_diff_ht, cdm_diff_t, node, cdm_diff_hash, cdm_diff_eq,
             0.6, tor_reallocarray, tor_free_);

#define cdm_diff_free(diff) \
  FREE_AND_NULL(cdm_diff_t, cdm_diff_free_, (diff))

/** Release all storage held in <b>diff</b>. */
static void
cdm_diff_free_(cdm_diff_t *diff)
{
  if (!diff)
    return;
  consensus_cache_entry_handle_free(diff->entry);
  tor_free(diff);
}

/** Create and return a new cdm_diff_t with the given values.  Does not
 * add it to the hashtable. */
static cdm_diff_t *
cdm_diff_new(consensus_flavor_t flav,
             const uint8_t *from_sha3,
             const uint8_t *target_sha3,
             compress_method_t method)
{
  cdm_diff_t *ent;
  ent = tor_malloc_zero(sizeof(cdm_diff_t));
  ent->flavor = flav;
  memcpy(ent->from_sha3, from_sha3, DIGEST256_LEN);
  memcpy(ent->target_sha3, target_sha3, DIGEST256_LEN);
  ent->compress_method = method;
  return ent;
}

/**
 * Examine the diff hashtable to see whether we know anything about computing
 * a diff of type <b>flav</b> between consensuses with the two provided
 * SHA3-256 digests.  If a computation is in progress, or if the computation
 * has already been tried and failed, return 1.  Otherwise, note the
 * computation as "in progress" so that we don't reattempt it later, and
 * return 0.
 */
static int
cdm_diff_ht_check_and_note_pending(consensus_flavor_t flav,
                                   const uint8_t *from_sha3,
                                   const uint8_t *target_sha3)
{
  struct cdm_diff_t search, *ent;
  unsigned u;
  int result = 0;
  for (u = 0; u < n_diff_compression_methods(); ++u) {
    compress_method_t method = compress_diffs_with[u];
    memset(&search, 0, sizeof(cdm_diff_t));
    search.flavor = flav;
    search.compress_method = method;
    memcpy(search.from_sha3, from_sha3, DIGEST256_LEN);
    ent = HT_FIND(cdm_diff_ht, &cdm_diff_ht, &search);
    if (ent) {
      tor_assert_nonfatal(ent->cdm_diff_status != CDM_DIFF_PRESENT);
      result = 1;
      continue;
    }
    ent = cdm_diff_new(flav, from_sha3, target_sha3, method);
    ent->cdm_diff_status = CDM_DIFF_IN_PROGRESS;
    HT_INSERT(cdm_diff_ht, &cdm_diff_ht, ent);
  }
  return result;
}

/**
 * Update the status of the diff of type <b>flav</b> between consensuses with
 * the two provided SHA3-256 digests, so that its status becomes
 * <b>status</b>, and its value becomes the <b>handle</b>.  If <b>handle</b>
 * is NULL, then the old handle (if any) is freed, and replaced with NULL.
 */
static void
cdm_diff_ht_set_status(consensus_flavor_t flav,
                       const uint8_t *from_sha3,
                       const uint8_t *to_sha3,
                       compress_method_t method,
                       int status,
                       consensus_cache_entry_handle_t *handle)
{
  if (handle == NULL) {
    tor_assert_nonfatal(status != CDM_DIFF_PRESENT);
  }

  struct cdm_diff_t search, *ent;
  memset(&search, 0, sizeof(cdm_diff_t));
  search.flavor = flav;
  search.compress_method = method,
  memcpy(search.from_sha3, from_sha3, DIGEST256_LEN);
  ent = HT_FIND(cdm_diff_ht, &cdm_diff_ht, &search);
  if (!ent) {
    ent = cdm_diff_new(flav, from_sha3, to_sha3, method);
    ent->cdm_diff_status = CDM_DIFF_IN_PROGRESS;
    HT_INSERT(cdm_diff_ht, &cdm_diff_ht, ent);
  } else if (fast_memneq(ent->target_sha3, to_sha3, DIGEST256_LEN)) {
    // This can happen under certain really pathological conditions
    // if we decide we don't care about a diff before it is actually
    // done computing.
    return;
  }

  tor_assert_nonfatal(ent->cdm_diff_status == CDM_DIFF_IN_PROGRESS);

  ent->cdm_diff_status = status;
  consensus_cache_entry_handle_free(ent->entry);
  ent->entry = handle;
}

/**
 * Helper: Remove from the hash table every present (actually computed) diff
 * of type <b>flav</b> whose target digest does not match
 * <b>unless_target_sha3_matches</b>.
 *
 * This function is used for the hash table to throw away references to diffs
 * that do not lead to the most given consensus of a given flavor.
 */
static void
cdm_diff_ht_purge(consensus_flavor_t flav,
                  const uint8_t *unless_target_sha3_matches)
{
  cdm_diff_t **diff, **next;
  for (diff = HT_START(cdm_diff_ht, &cdm_diff_ht); diff; diff = next) {
    cdm_diff_t *this = *diff;

    if ((*diff)->cdm_diff_status == CDM_DIFF_PRESENT &&
        flav == (*diff)->flavor) {

      if (BUG((*diff)->entry == NULL) ||
          consensus_cache_entry_handle_get((*diff)->entry) == NULL) {
        /* the underlying entry has gone away; drop this. */
        next = HT_NEXT_RMV(cdm_diff_ht, &cdm_diff_ht, diff);
        cdm_diff_free(this);
        continue;
      }

      if (unless_target_sha3_matches &&
          fast_memneq(unless_target_sha3_matches, (*diff)->target_sha3,
                      DIGEST256_LEN)) {
        /* target hash doesn't match; drop this. */
        next = HT_NEXT_RMV(cdm_diff_ht, &cdm_diff_ht, diff);
        cdm_diff_free(this);
        continue;
      }
    }
    next = HT_NEXT(cdm_diff_ht, &cdm_diff_ht, diff);
  }
}

/**
 * Helper: initialize <b>cons_diff_cache</b>.
 */
static void
cdm_cache_init(void)
{
  unsigned n_entries = consdiff_cfg.cache_max_num * 2;

  tor_assert(cons_diff_cache == NULL);
  cons_diff_cache = consensus_cache_open("diff-cache", n_entries);
  if (cons_diff_cache == NULL) {
    // LCOV_EXCL_START
    log_err(LD_FS, "Error: Couldn't open storage for consensus diffs.");
    tor_assert_unreached();
    // LCOV_EXCL_STOP
  } else {
    consdiffmgr_set_cache_flags();
  }
  consdiffmgr_rescan_ev =
    mainloop_event_postloop_new(consdiffmgr_rescan_cb, NULL);
  mark_cdm_cache_dirty();
  cdm_cache_loaded = 0;
}

/**
 * Helper: return the consensus_cache_t * that backs this manager,
 * initializing it if needed.
 */
STATIC consensus_cache_t *
cdm_cache_get(void)
{
  if (PREDICT_UNLIKELY(cons_diff_cache == NULL)) {
    cdm_cache_init();
  }
  return cons_diff_cache;
}

/**
 * Helper: given a list of labels, prepend the hex-encoded SHA3 digest
 * of the <b>bodylen</b>-byte object at <b>body</b> to those labels,
 * with <b>label</b> as its label.
 */
static void
cdm_labels_prepend_sha3(config_line_t **labels,
                        const char *label,
                        const uint8_t *body,
                        size_t bodylen)
{
  uint8_t sha3_digest[DIGEST256_LEN];
  char hexdigest[HEX_DIGEST256_LEN+1];
  crypto_digest256((char *)sha3_digest,
                   (const char *)body, bodylen, DIGEST_SHA3_256);
  base16_encode(hexdigest, sizeof(hexdigest),
                (const char *)sha3_digest, sizeof(sha3_digest));

  config_line_prepend(labels, label, hexdigest);
}

/** Helper: if there is a sha3-256 hex-encoded digest in <b>ent</b> with the
 * given label, set <b>digest_out</b> to that value (decoded), and return 0.
 *
 * Return -1 if there is no such label, and -2 if it is badly formatted. */
STATIC int
cdm_entry_get_sha3_value(uint8_t *digest_out,
                         consensus_cache_entry_t *ent,
                         const char *label)
{
  if (ent == NULL)
    return -1;

  const char *hex = consensus_cache_entry_get_value(ent, label);
  if (hex == NULL)
    return -1;

  int n = base16_decode((char*)digest_out, DIGEST256_LEN, hex, strlen(hex));
  if (n != DIGEST256_LEN)
    return -2;
  else
    return 0;
}

/**
 * Helper: look for a consensus with the given <b>flavor</b> and
 * <b>valid_after</b> time in the cache. Return that consensus if it's
 * present, or NULL if it's missing.
 */
STATIC consensus_cache_entry_t *
cdm_cache_lookup_consensus(consensus_flavor_t flavor, time_t valid_after)
{
  char formatted_time[ISO_TIME_LEN+1];
  format_iso_time_nospace(formatted_time, valid_after);
  const char *flavname = networkstatus_get_flavor_name(flavor);

  /* We'll filter by valid-after time first, since that should
   * match the fewest documents. */
  /* We could add an extra hashtable here, but since we only do this scan
   * when adding a new consensus, it probably doesn't matter much. */
  smartlist_t *matches = smartlist_new();
  consensus_cache_find_all(matches, cdm_cache_get(),
                           LABEL_VALID_AFTER, formatted_time);
  consensus_cache_filter_list(matches, LABEL_FLAVOR, flavname);
  consensus_cache_filter_list(matches, LABEL_DOCTYPE, DOCTYPE_CONSENSUS);

  consensus_cache_entry_t *result = NULL;
  if (smartlist_len(matches)) {
    result = smartlist_get(matches, 0);
  }
  smartlist_free(matches);

  return result;
}

/** Return the maximum age (in seconds) of consensuses that we should consider
 * storing. The available space in the directory may impose additional limits
 * on how much we store. */
static int32_t
get_max_age_to_cache(void)
{
  const int32_t DEFAULT_MAX_AGE_TO_CACHE = 8192;
  const int32_t MIN_MAX_AGE_TO_CACHE = 0;
  const int32_t MAX_MAX_AGE_TO_CACHE = 8192;
  const char MAX_AGE_TO_CACHE_NAME[] = "max-consensus-age-to-cache-for-diff";

  const or_options_t *options = get_options();

  if (options->MaxConsensusAgeForDiffs) {
    const int v = options->MaxConsensusAgeForDiffs;
    if (v >= MAX_MAX_AGE_TO_CACHE * 3600)
      return MAX_MAX_AGE_TO_CACHE;
    else
      return v;
  }

  /* The parameter is in hours, so we multiply */
  return 3600 * networkstatus_get_param(NULL,
                                        MAX_AGE_TO_CACHE_NAME,
                                        DEFAULT_MAX_AGE_TO_CACHE,
                                        MIN_MAX_AGE_TO_CACHE,
                                        MAX_MAX_AGE_TO_CACHE);
}

#ifdef TOR_UNIT_TESTS
/** As consdiffmgr_add_consensus, but requires a nul-terminated input. For
 * testing. */
int
consdiffmgr_add_consensus_nulterm(const char *consensus,
                                  const networkstatus_t *as_parsed)
{
  size_t len = strlen(consensus);
  /* make a non-nul-terminated copy so that we can have a better chance
   * of catching errors. */
  char *ctmp = tor_memdup(consensus, len);
  int r = consdiffmgr_add_consensus(ctmp, len, as_parsed);
  tor_free(ctmp);
  return r;
}
#endif /* defined(TOR_UNIT_TESTS) */

/**
 * Given a buffer containing a networkstatus consensus, and the results of
 * having parsed that consensus, add that consensus to the cache if it is not
 * already present and not too old.  Create new consensus diffs from or to
 * that consensus as appropriate.
 *
 * Return 0 on success and -1 on failure.
 */
int
consdiffmgr_add_consensus(const char *consensus,
                          size_t consensus_len,
                          const networkstatus_t *as_parsed)
{
  if (BUG(consensus == NULL) || BUG(as_parsed == NULL))
    return -1; // LCOV_EXCL_LINE
  if (BUG(as_parsed->type != NS_TYPE_CONSENSUS))
    return -1; // LCOV_EXCL_LINE

  const consensus_flavor_t flavor = as_parsed->flavor;
  const time_t valid_after = as_parsed->valid_after;

  if (valid_after < approx_time() - get_max_age_to_cache()) {
    log_info(LD_DIRSERV, "We don't care about this consensus document; it's "
             "too old.");
    return -1;
  }

  /* Do we already have this one? */
  consensus_cache_entry_t *entry =
    cdm_cache_lookup_consensus(flavor, valid_after);
  if (entry) {
    log_info(LD_DIRSERV, "We already have a copy of that consensus");
    return -1;
  }

  /* We don't have it. Add it to the cache. */
  return consensus_queue_compression_work(consensus, consensus_len, as_parsed);
}

/**
 * Helper: used to sort two smartlists of consensus_cache_entry_t by their
 * LABEL_VALID_AFTER labels.
 */
static int
compare_by_valid_after_(const void **a, const void **b)
{
  const consensus_cache_entry_t *e1 = *a;
  const consensus_cache_entry_t *e2 = *b;
  /* We're in luck here: sorting UTC iso-encoded values lexically will work
   * fine (until 9999). */
  return strcmp_opt(consensus_cache_entry_get_value(e1, LABEL_VALID_AFTER),
                    consensus_cache_entry_get_value(e2, LABEL_VALID_AFTER));
}

/**
 * Helper: Sort <b>lst</b> by LABEL_VALID_AFTER and return the most recent
 * entry.
 */
static consensus_cache_entry_t *
sort_and_find_most_recent(smartlist_t *lst)
{
  smartlist_sort(lst, compare_by_valid_after_);
  if (smartlist_len(lst)) {
    return smartlist_get(lst, smartlist_len(lst) - 1);
  } else {
    return NULL;
  }
}

/** Return i such that compress_consensus_with[i] == method. Return
 * -1 if no such i exists. */
static int
consensus_compression_method_pos(compress_method_t method)
{
  unsigned i;
  for (i = 0; i < n_consensus_compression_methods(); ++i) {
    if (compress_consensus_with[i] == method) {
      return i;
    }
  }
  return -1;
}

/**
 * If we know a consensus with the flavor <b>flavor</b> compressed with
 * <b>method</b>, set *<b>entry_out</b> to that value.  Return values are as
 * for consdiffmgr_find_diff_from().
 */
consdiff_status_t
consdiffmgr_find_consensus(struct consensus_cache_entry_t **entry_out,
                           consensus_flavor_t flavor,
                           compress_method_t method)
{
  tor_assert(entry_out);
  tor_assert((int)flavor < N_CONSENSUS_FLAVORS);

  int pos = consensus_compression_method_pos(method);
  if (pos < 0) {
     // We don't compress consensuses with this method.
    return CONSDIFF_NOT_FOUND;
  }
  consensus_cache_entry_handle_t *handle = latest_consensus[flavor][pos];
  if (!handle)
    return CONSDIFF_NOT_FOUND;
  *entry_out = consensus_cache_entry_handle_get(handle);
  if (*entry_out)
    return CONSDIFF_AVAILABLE;
  else
    return CONSDIFF_NOT_FOUND;
}

/**
 * Look up consensus_cache_entry_t for the consensus of type <b>flavor</b>,
 * from the source consensus with the specified digest (which must be SHA3).
 *
 * If the diff is present, store it into *<b>entry_out</b> and return
 * CONSDIFF_AVAILABLE. Otherwise return CONSDIFF_NOT_FOUND or
 * CONSDIFF_IN_PROGRESS.
 */
consdiff_status_t
consdiffmgr_find_diff_from(consensus_cache_entry_t **entry_out,
                           consensus_flavor_t flavor,
                           int digest_type,
                           const uint8_t *digest,
                           size_t digestlen,
                           compress_method_t method)
{
  if (BUG(digest_type != DIGEST_SHA3_256) ||
      BUG(digestlen != DIGEST256_LEN)) {
    return CONSDIFF_NOT_FOUND; // LCOV_EXCL_LINE
  }

  // Try to look up the entry in the hashtable.
  cdm_diff_t search, *ent;
  memset(&search, 0, sizeof(search));
  search.flavor = flavor;
  search.compress_method = method;
  memcpy(search.from_sha3, digest, DIGEST256_LEN);
  ent = HT_FIND(cdm_diff_ht, &cdm_diff_ht, &search);

  if (ent == NULL ||
      ent->cdm_diff_status == CDM_DIFF_ERROR) {
    return CONSDIFF_NOT_FOUND;
  } else if (ent->cdm_diff_status == CDM_DIFF_IN_PROGRESS) {
    return CONSDIFF_IN_PROGRESS;
  } else if (BUG(ent->cdm_diff_status != CDM_DIFF_PRESENT)) {
    return CONSDIFF_IN_PROGRESS;
  }

  if (BUG(ent->entry == NULL)) {
    return CONSDIFF_NOT_FOUND;
  }
  *entry_out = consensus_cache_entry_handle_get(ent->entry);
  return (*entry_out) ? CONSDIFF_AVAILABLE : CONSDIFF_NOT_FOUND;

#if 0
  // XXXX Remove this.  I'm keeping it around for now in case we need to
  // XXXX debug issues in the hashtable.
  char hex[HEX_DIGEST256_LEN+1];
  base16_encode(hex, sizeof(hex), (const char *)digest, digestlen);
  const char *flavname = networkstatus_get_flavor_name(flavor);

  smartlist_t *matches = smartlist_new();
  consensus_cache_find_all(matches, cdm_cache_get(),
                           LABEL_FROM_SHA3_DIGEST, hex);
  consensus_cache_filter_list(matches, LABEL_FLAVOR, flavname);
  consensus_cache_filter_list(matches, LABEL_DOCTYPE, DOCTYPE_CONSENSUS_DIFF);

  *entry_out = sort_and_find_most_recent(matches);
  consdiff_status_t result =
    (*entry_out) ? CONSDIFF_AVAILABLE : CONSDIFF_NOT_FOUND;
  smartlist_free(matches);

  return result;
#endif /* 0 */
}

/**
 * Perform periodic cleanup tasks on the consensus diff cache.  Return
 * the number of objects marked for deletion.
 */
int
consdiffmgr_cleanup(void)
{
  smartlist_t *objects = smartlist_new();
  smartlist_t *consensuses = smartlist_new();
  smartlist_t *diffs = smartlist_new();
  int n_to_delete = 0;

  log_debug(LD_DIRSERV, "Looking for consdiffmgr entries to remove");

  // 1. Delete any consensus or diff or anything whose valid_after is too old.
  const time_t valid_after_cutoff = approx_time() - get_max_age_to_cache();

  consensus_cache_find_all(objects, cdm_cache_get(),
                           NULL, NULL);
  SMARTLIST_FOREACH_BEGIN(objects, consensus_cache_entry_t *, ent) {
    const char *lv_valid_after =
      consensus_cache_entry_get_value(ent, LABEL_VALID_AFTER);
    if (! lv_valid_after) {
      log_debug(LD_DIRSERV, "Ignoring entry because it had no %s label",
                LABEL_VALID_AFTER);
      continue;
    }
    time_t valid_after = 0;
    if (parse_iso_time_nospace(lv_valid_after, &valid_after) < 0) {
      log_debug(LD_DIRSERV, "Ignoring entry because its %s value (%s) was "
                "unparseable", LABEL_VALID_AFTER, escaped(lv_valid_after));
      continue;
    }
    if (valid_after < valid_after_cutoff) {
      log_debug(LD_DIRSERV, "Deleting entry because its %s value (%s) was "
                "too old", LABEL_VALID_AFTER, lv_valid_after);
      consensus_cache_entry_mark_for_removal(ent);
      ++n_to_delete;
    }
  } SMARTLIST_FOREACH_END(ent);

  // 2. Delete all diffs that lead to a consensus whose valid-after is not the
  // latest.
  for (int flav = 0; flav < N_CONSENSUS_FLAVORS; ++flav) {
    const char *flavname = networkstatus_get_flavor_name(flav);
    /* Determine the most recent consensus of this flavor */
    consensus_cache_find_all(consensuses, cdm_cache_get(),
                             LABEL_DOCTYPE, DOCTYPE_CONSENSUS);
    consensus_cache_filter_list(consensuses, LABEL_FLAVOR, flavname);
    consensus_cache_entry_t *most_recent =
      sort_and_find_most_recent(consensuses);
    if (most_recent == NULL)
      continue;
    const char *most_recent_sha3 =
      consensus_cache_entry_get_value(most_recent,
                                      LABEL_SHA3_DIGEST_UNCOMPRESSED);
    if (BUG(most_recent_sha3 == NULL))
      continue; // LCOV_EXCL_LINE

    /* consider all such-flavored diffs, and look to see if they match. */
    consensus_cache_find_all(diffs, cdm_cache_get(),
                             LABEL_DOCTYPE, DOCTYPE_CONSENSUS_DIFF);
    consensus_cache_filter_list(diffs, LABEL_FLAVOR, flavname);
    SMARTLIST_FOREACH_BEGIN(diffs, consensus_cache_entry_t *, diff) {
      const char *this_diff_target_sha3 =
        consensus_cache_entry_get_value(diff, LABEL_TARGET_SHA3_DIGEST);
      if (!this_diff_target_sha3)
        continue;
      if (strcmp(this_diff_target_sha3, most_recent_sha3)) {
        consensus_cache_entry_mark_for_removal(diff);
        ++n_to_delete;
      }
    } SMARTLIST_FOREACH_END(diff);
    smartlist_clear(consensuses);
    smartlist_clear(diffs);
  }

  // 3. Delete all consensuses except the most recent that are compressed with
  // an un-preferred method.
  for (int flav = 0; flav < N_CONSENSUS_FLAVORS; ++flav) {
    const char *flavname = networkstatus_get_flavor_name(flav);
    /* Determine the most recent consensus of this flavor */
    consensus_cache_find_all(consensuses, cdm_cache_get(),
                             LABEL_DOCTYPE, DOCTYPE_CONSENSUS);
    consensus_cache_filter_list(consensuses, LABEL_FLAVOR, flavname);
    consensus_cache_entry_t *most_recent =
      sort_and_find_most_recent(consensuses);
    if (most_recent == NULL)
      continue;
    const char *most_recent_sha3_uncompressed =
      consensus_cache_entry_get_value(most_recent,
                                      LABEL_SHA3_DIGEST_UNCOMPRESSED);
    const char *retain_methodname = compression_method_get_name(
                               RETAIN_CONSENSUS_COMPRESSED_WITH_METHOD);

    if (BUG(most_recent_sha3_uncompressed == NULL))
      continue;
    SMARTLIST_FOREACH_BEGIN(consensuses, consensus_cache_entry_t *, ent) {
      const char *lv_sha3_uncompressed =
        consensus_cache_entry_get_value(ent, LABEL_SHA3_DIGEST_UNCOMPRESSED);
      if (BUG(! lv_sha3_uncompressed))
        continue;
      if (!strcmp(lv_sha3_uncompressed, most_recent_sha3_uncompressed))
        continue; // This _is_ the most recent.
      const char *lv_methodname =
        consensus_cache_entry_get_value(ent, LABEL_COMPRESSION_TYPE);
      if (! lv_methodname || strcmp(lv_methodname, retain_methodname)) {
        consensus_cache_entry_mark_for_removal(ent);
        ++n_to_delete;
      }
    } SMARTLIST_FOREACH_END(ent);
  }

  smartlist_free(objects);
  smartlist_free(consensuses);
  smartlist_free(diffs);

  // Actually remove files, if they're not used.
  consensus_cache_delete_pending(cdm_cache_get(), 0);
  return n_to_delete;
}

/**
 * Initialize the consensus diff manager and its cache, and configure
 * its parameters based on the latest torrc and networkstatus parameters.
 */
void
consdiffmgr_configure(const consdiff_cfg_t *cfg)
{
  if (cfg)
    memcpy(&consdiff_cfg, cfg, sizeof(consdiff_cfg));

  (void) cdm_cache_get();
}

/**
 * Tell the sandbox (if any) configured by <b>cfg</b> to allow the
 * operations that the consensus diff manager will need.
 */
int
consdiffmgr_register_with_sandbox(struct sandbox_cfg_elem_t **cfg)
{
  return consensus_cache_register_with_sandbox(cdm_cache_get(), cfg);
}

/**
 * Scan the consensus diff manager's cache for any grossly malformed entries,
 * and mark them as deletable.  Return 0 if no problems were found; 1
 * if problems were found and fixed.
 */
int
consdiffmgr_validate(void)
{
  /* Right now, we only check for entries that have bad sha3 values */
  int problems = 0;

  smartlist_t *objects = smartlist_new();
  consensus_cache_find_all(objects, cdm_cache_get(),
                           NULL, NULL);
  SMARTLIST_FOREACH_BEGIN(objects, consensus_cache_entry_t *, obj) {
    uint8_t sha3_expected[DIGEST256_LEN];
    uint8_t sha3_received[DIGEST256_LEN];
    int r = cdm_entry_get_sha3_value(sha3_expected, obj, LABEL_SHA3_DIGEST);
    if (r == -1) {
      /* digest isn't there; that's allowed */
      continue;
    } else if (r == -2) {
      /* digest is malformed; that's not allowed */
      problems = 1;
      consensus_cache_entry_mark_for_removal(obj);
      continue;
    }
    const uint8_t *body;
    size_t bodylen;
    consensus_cache_entry_incref(obj);
    r = consensus_cache_entry_get_body(obj, &body, &bodylen);
    if (r == 0) {
      crypto_digest256((char *)sha3_received, (const char *)body, bodylen,
                       DIGEST_SHA3_256);
    }
    consensus_cache_entry_decref(obj);
    if (r < 0)
      continue;

    // Deconfuse coverity about the possibility of sha3_received being
    // uninitialized
    tor_assert(r <= 0);

    if (fast_memneq(sha3_received, sha3_expected, DIGEST256_LEN)) {
      problems = 1;
      consensus_cache_entry_mark_for_removal(obj);
      continue;
    }

  } SMARTLIST_FOREACH_END(obj);
  smartlist_free(objects);
  return problems;
}

/**
 * Helper: build new diffs of <b>flavor</b> as needed
 */
static void
consdiffmgr_rescan_flavor_(consensus_flavor_t flavor)
{
  smartlist_t *matches = NULL;
  smartlist_t *diffs = NULL;
  smartlist_t *compute_diffs_from = NULL;
  strmap_t *have_diff_from = NULL;

  // look for the most recent consensus, and for all previous in-range
  // consensuses.  Do they all have diffs to it?
  const char *flavname = networkstatus_get_flavor_name(flavor);

  // 1. find the most recent consensus, and the ones that we might want
  //    to diff to it.
  const char *methodname = compression_method_get_name(
                             RETAIN_CONSENSUS_COMPRESSED_WITH_METHOD);

  matches = smartlist_new();
  consensus_cache_find_all(matches, cdm_cache_get(),
                           LABEL_FLAVOR, flavname);
  consensus_cache_filter_list(matches, LABEL_DOCTYPE, DOCTYPE_CONSENSUS);
  consensus_cache_filter_list(matches, LABEL_COMPRESSION_TYPE, methodname);
  consensus_cache_entry_t *most_recent = sort_and_find_most_recent(matches);
  if (!most_recent) {
    log_info(LD_DIRSERV, "No 'most recent' %s consensus found; "
             "not making diffs", flavname);
    goto done;
  }
  tor_assert(smartlist_len(matches));
  smartlist_del(matches, smartlist_len(matches) - 1);

  const char *most_recent_valid_after =
    consensus_cache_entry_get_value(most_recent, LABEL_VALID_AFTER);
  if (BUG(most_recent_valid_after == NULL))
    goto done; //LCOV_EXCL_LINE
  uint8_t most_recent_sha3[DIGEST256_LEN];
  if (BUG(cdm_entry_get_sha3_value(most_recent_sha3, most_recent,
                                   LABEL_SHA3_DIGEST_UNCOMPRESSED) < 0))
    goto done; //LCOV_EXCL_LINE

  // 2. Find all the relevant diffs _to_ this consensus. These are ones
  //    that we don't need to compute.
  diffs = smartlist_new();
  consensus_cache_find_all(diffs, cdm_cache_get(),
                           LABEL_VALID_AFTER, most_recent_valid_after);
  consensus_cache_filter_list(diffs, LABEL_DOCTYPE, DOCTYPE_CONSENSUS_DIFF);
  consensus_cache_filter_list(diffs, LABEL_FLAVOR, flavname);
  have_diff_from = strmap_new();
  SMARTLIST_FOREACH_BEGIN(diffs, consensus_cache_entry_t *, diff) {
    const char *va = consensus_cache_entry_get_value(diff,
                                                     LABEL_FROM_VALID_AFTER);
    if (BUG(va == NULL))
      continue; // LCOV_EXCL_LINE
    strmap_set(have_diff_from, va, diff);
  } SMARTLIST_FOREACH_END(diff);

  // 3. See which consensuses in 'matches' don't have diffs yet.
  smartlist_reverse(matches); // from newest to oldest.
  compute_diffs_from = smartlist_new();
  SMARTLIST_FOREACH_BEGIN(matches, consensus_cache_entry_t *, ent) {
    const char *va = consensus_cache_entry_get_value(ent, LABEL_VALID_AFTER);
    if (BUG(va == NULL))
      continue; // LCOV_EXCL_LINE
    if (strmap_get(have_diff_from, va) != NULL)
      continue; /* we already have this one. */
    smartlist_add(compute_diffs_from, ent);
    /* Since we are not going to serve this as the most recent consensus
     * any more, we should stop keeping it mmap'd when it's not in use.
     */
    consensus_cache_entry_mark_for_aggressive_release(ent);
  } SMARTLIST_FOREACH_END(ent);

  log_info(LD_DIRSERV,
           "The most recent %s consensus is valid-after %s. We have diffs to "
           "this consensus for %d/%d older %s consensuses. Generating diffs "
           "for the other %d.",
           flavname,
           most_recent_valid_after,
           smartlist_len(matches) - smartlist_len(compute_diffs_from),
           smartlist_len(matches),
           flavname,
           smartlist_len(compute_diffs_from));

  // 4. Update the hashtable; remove entries in this flavor to other
  //    target consensuses.
  cdm_diff_ht_purge(flavor, most_recent_sha3);

  // 5. Actually launch the requests.
  SMARTLIST_FOREACH_BEGIN(compute_diffs_from, consensus_cache_entry_t *, c) {
    if (BUG(c == most_recent))
      continue; // LCOV_EXCL_LINE

    uint8_t this_sha3[DIGEST256_LEN];
    if (cdm_entry_get_sha3_value(this_sha3, c,
                                 LABEL_SHA3_DIGEST_AS_SIGNED)<0) {
      // Not actually a bug, since we might be running with a directory
      // with stale files from before the #22143 fixes.
      continue;
    }
    if (cdm_diff_ht_check_and_note_pending(flavor,
                                           this_sha3, most_recent_sha3)) {
      // This is already pending, or we encountered an error.
      continue;
    }
    consensus_diff_queue_diff_work(c, most_recent);
  } SMARTLIST_FOREACH_END(c);

 done:
  smartlist_free(matches);
  smartlist_free(diffs);
  smartlist_free(compute_diffs_from);
  strmap_free(have_diff_from, NULL);
}

/**
 * Scan the cache for the latest consensuses and add their handles to
 * latest_consensus
 */
static void
consdiffmgr_consensus_load(void)
{
  smartlist_t *matches = smartlist_new();
  for (int flav = 0; flav < N_CONSENSUS_FLAVORS; ++flav) {
    const char *flavname = networkstatus_get_flavor_name(flav);
    smartlist_clear(matches);
    consensus_cache_find_all(matches, cdm_cache_get(),
                             LABEL_FLAVOR, flavname);
    consensus_cache_filter_list(matches, LABEL_DOCTYPE, DOCTYPE_CONSENSUS);
    consensus_cache_entry_t *most_recent = sort_and_find_most_recent(matches);
    if (! most_recent)
      continue; // no consensuses.
    const char *most_recent_sha3 =
      consensus_cache_entry_get_value(most_recent,
                                      LABEL_SHA3_DIGEST_UNCOMPRESSED);
    if (BUG(most_recent_sha3 == NULL))
      continue; // LCOV_EXCL_LINE
    consensus_cache_filter_list(matches, LABEL_SHA3_DIGEST_UNCOMPRESSED,
                                most_recent_sha3);

    // Everything that remains matches the most recent consensus of this
    // flavor.
    SMARTLIST_FOREACH_BEGIN(matches, consensus_cache_entry_t *, ent) {
      const char *lv_compression =
        consensus_cache_entry_get_value(ent, LABEL_COMPRESSION_TYPE);
      compress_method_t method =
        compression_method_get_by_name(lv_compression);
      int pos = consensus_compression_method_pos(method);
      if (pos < 0)
        continue;
      consensus_cache_entry_handle_free(latest_consensus[flav][pos]);
      latest_consensus[flav][pos] = consensus_cache_entry_handle_new(ent);
    } SMARTLIST_FOREACH_END(ent);
  }
  smartlist_free(matches);
}

/**
 * Scan the cache for diffs, and add them to the hashtable.
 */
static void
consdiffmgr_diffs_load(void)
{
  smartlist_t *diffs = smartlist_new();
  consensus_cache_find_all(diffs, cdm_cache_get(),
                           LABEL_DOCTYPE, DOCTYPE_CONSENSUS_DIFF);
  SMARTLIST_FOREACH_BEGIN(diffs, consensus_cache_entry_t *, diff) {
    const char *lv_flavor =
      consensus_cache_entry_get_value(diff, LABEL_FLAVOR);
    if (!lv_flavor)
      continue;
    int flavor = networkstatus_parse_flavor_name(lv_flavor);
    if (flavor < 0)
      continue;
    const char *lv_compression =
      consensus_cache_entry_get_value(diff, LABEL_COMPRESSION_TYPE);
    compress_method_t method = NO_METHOD;
    if (lv_compression) {
      method = compression_method_get_by_name(lv_compression);
      if (method == UNKNOWN_METHOD) {
        continue;
      }
    }

    uint8_t from_sha3[DIGEST256_LEN];
    uint8_t to_sha3[DIGEST256_LEN];
    if (cdm_entry_get_sha3_value(from_sha3, diff, LABEL_FROM_SHA3_DIGEST)<0)
      continue;
    if (cdm_entry_get_sha3_value(to_sha3, diff, LABEL_TARGET_SHA3_DIGEST)<0)
      continue;

    cdm_diff_ht_set_status(flavor, from_sha3, to_sha3,
                           method,
                           CDM_DIFF_PRESENT,
                           consensus_cache_entry_handle_new(diff));
  } SMARTLIST_FOREACH_END(diff);
  smartlist_free(diffs);
}

/**
 * Build new diffs as needed.
 */
void
consdiffmgr_rescan(void)
{
  if (cdm_cache_dirty == 0)
    return;

  // Clean up here to make room for new diffs, and to ensure that older
  // consensuses do not have any entries.
  consdiffmgr_cleanup();

  if (cdm_cache_loaded == 0) {
    consdiffmgr_diffs_load();
    consdiffmgr_consensus_load();
    cdm_cache_loaded = 1;
  }

  for (int flav = 0; flav < N_CONSENSUS_FLAVORS; ++flav) {
    consdiffmgr_rescan_flavor_((consensus_flavor_t) flav);
  }

  cdm_cache_dirty = 0;
}

/** Callback wrapper for consdiffmgr_rescan */
static void
consdiffmgr_rescan_cb(mainloop_event_t *ev, void *arg)
{
  (void)ev;
  (void)arg;
  consdiffmgr_rescan();
}

/** Mark the cache as dirty, and schedule a rescan event. */
static void
mark_cdm_cache_dirty(void)
{
  cdm_cache_dirty = 1;
  tor_assert(consdiffmgr_rescan_ev);
  mainloop_event_activate(consdiffmgr_rescan_ev);
}

/**
 * Helper: compare two files by their from-valid-after and valid-after labels,
 * trying to sort in ascending order by from-valid-after (when present) and
 * valid-after (when not).  Place everything that has neither label first in
 * the list.
 */
static int
compare_by_staleness_(const void **a, const void **b)
{
  const consensus_cache_entry_t *e1 = *a;
  const consensus_cache_entry_t *e2 = *b;
  const char *va1, *fva1, *va2, *fva2;
  va1 = consensus_cache_entry_get_value(e1, LABEL_VALID_AFTER);
  va2 = consensus_cache_entry_get_value(e2, LABEL_VALID_AFTER);
  fva1 = consensus_cache_entry_get_value(e1, LABEL_FROM_VALID_AFTER);
  fva2 = consensus_cache_entry_get_value(e2, LABEL_FROM_VALID_AFTER);

  if (fva1)
    va1 = fva1;
  if (fva2)
    va2 = fva2;

  /* See note about iso-encoded values in compare_by_valid_after_.  Also note
   * that missing dates will get placed first. */
  return strcmp_opt(va1, va2);
}

/** If there are not enough unused filenames to store <b>n</b> files, then
 * delete old consensuses until there are.  (We have to keep track of the
 * number of filenames because of the way that the seccomp2 cache works.)
 *
 * Return 0 on success, -1 on failure.
 **/
static int
consdiffmgr_ensure_space_for_files(int n)
{
  consensus_cache_t *cache = cdm_cache_get();
  if (consensus_cache_get_n_filenames_available(cache) >= n) {
    // there are already enough unused filenames.
    return 0;
  }
  // Try a cheap deletion of stuff that's waiting to get deleted.
  consensus_cache_delete_pending(cache, 0);
  if (consensus_cache_get_n_filenames_available(cache) >= n) {
    // okay, _that_ made enough filenames available.
    return 0;
  }
  // Let's get more assertive: clean out unused stuff, and force-remove
  // the files that we can.
  consdiffmgr_cleanup();
  consensus_cache_delete_pending(cache, 1);
  const int n_to_remove = n - consensus_cache_get_n_filenames_available(cache);
  if (n_to_remove <= 0) {
    // okay, finally!
    return 0;
  }

  // At this point, we're going to have to throw out objects that will be
  // missed.  Too bad!
  smartlist_t *objects = smartlist_new();
  consensus_cache_find_all(objects, cache, NULL, NULL);
  smartlist_sort(objects, compare_by_staleness_);
  int n_marked = 0;
  SMARTLIST_FOREACH_BEGIN(objects, consensus_cache_entry_t *, ent) {
    consensus_cache_entry_mark_for_removal(ent);
    if (++n_marked >= n_to_remove)
      break;
  } SMARTLIST_FOREACH_END(ent);
  smartlist_free(objects);

  consensus_cache_delete_pending(cache, 1);

  if (consensus_cache_may_overallocate(cache)) {
    /* If we're allowed to throw extra files into the cache, let's do so
     * rather getting upset.
     */
    return 0;
  }

  if (BUG(n_marked < n_to_remove))
    return -1;
  else
    return 0;
}

/**
 * Set consensus cache flags on the objects in this consdiffmgr.
 */
static void
consdiffmgr_set_cache_flags(void)
{
  /* Right now, we just mark the consensus objects for aggressive release,
   * so that they get mmapped for as little time as possible. */
  smartlist_t *objects = smartlist_new();
  consensus_cache_find_all(objects, cdm_cache_get(), LABEL_DOCTYPE,
                           DOCTYPE_CONSENSUS);
  SMARTLIST_FOREACH_BEGIN(objects, consensus_cache_entry_t *, ent) {
    consensus_cache_entry_mark_for_aggressive_release(ent);
  } SMARTLIST_FOREACH_END(ent);
  smartlist_free(objects);
}

/**
 * Called before shutdown: drop all storage held by the consdiffmgr.c module.
 */
void
consdiffmgr_free_all(void)
{
  cdm_diff_t **diff, **next;
  for (diff = HT_START(cdm_diff_ht, &cdm_diff_ht); diff; diff = next) {
    cdm_diff_t *this = *diff;
    next = HT_NEXT_RMV(cdm_diff_ht, &cdm_diff_ht, diff);
    cdm_diff_free(this);
  }
  int i;
  unsigned j;
  for (i = 0; i < N_CONSENSUS_FLAVORS; ++i) {
    for (j = 0; j < n_consensus_compression_methods(); ++j) {
      consensus_cache_entry_handle_free(latest_consensus[i][j]);
    }
  }
  memset(latest_consensus, 0, sizeof(latest_consensus));
  consensus_cache_free(cons_diff_cache);
  cons_diff_cache = NULL;
  mainloop_event_free(consdiffmgr_rescan_ev);
}

/* =====
   Thread workers
   =====*/

typedef struct compressed_result_t {
  config_line_t *labels;
  /**
   * Output: Body of the diff, as compressed.
   */
  uint8_t *body;
  /**
   * Output: length of body_out
   */
  size_t bodylen;
} compressed_result_t;

/**
 * Compress the bytestring <b>input</b> of length <b>len</b> using the
 * <b>n_methods</b> compression methods listed in the array <b>methods</b>.
 *
 * For each successful compression, set the fields in the <b>results_out</b>
 * array in the position corresponding to the compression method. Use
 * <b>labels_in</b> as a basis for the labels of the result.
 *
 * Return 0 if all compression succeeded; -1 if any failed.
 */
static int
compress_multiple(compressed_result_t *results_out, int n_methods,
                  const compress_method_t *methods,
                  const uint8_t *input, size_t len,
                  const config_line_t *labels_in)
{
  int rv = 0;
  int i;
  for (i = 0; i < n_methods; ++i) {
    compress_method_t method = methods[i];
    const char *methodname = compression_method_get_name(method);
    char *result;
    size_t sz;
    if (0 == tor_compress(&result, &sz, (const char*)input, len, method)) {
      results_out[i].body = (uint8_t*)result;
      results_out[i].bodylen = sz;
      results_out[i].labels = config_lines_dup(labels_in);
      cdm_labels_prepend_sha3(&results_out[i].labels, LABEL_SHA3_DIGEST,
                              results_out[i].body,
                              results_out[i].bodylen);
      config_line_prepend(&results_out[i].labels,
                          LABEL_COMPRESSION_TYPE,
                          methodname);
    } else {
      rv = -1;
    }
  }
  return rv;
}

/**
 * Given an array of <b>n</b> compressed_result_t in <b>results</b>,
 * as produced by compress_multiple, store them all into the
 * consdiffmgr, and store handles to them in the <b>handles_out</b>
 * array.
 *
 * Return CDM_DIFF_PRESENT if any was stored, and CDM_DIFF_ERROR if none
 * was stored.
 */
static cdm_diff_status_t
store_multiple(consensus_cache_entry_handle_t **handles_out,
               int n,
               const compress_method_t *methods,
               const compressed_result_t *results,
               const char *description)
{
  cdm_diff_status_t status = CDM_DIFF_ERROR;
  consdiffmgr_ensure_space_for_files(n);

  int i;
  for (i = 0; i < n; ++i) {
    compress_method_t method = methods[i];
    uint8_t *body_out = results[i].body;
    size_t bodylen_out = results[i].bodylen;
    config_line_t *labels = results[i].labels;
    const char *methodname = compression_method_get_name(method);
    if (body_out && bodylen_out && labels) {
      /* Success! Store the results */
      log_info(LD_DIRSERV, "Adding %s, compressed with %s",
               description, methodname);

      consensus_cache_entry_t *ent =
        consensus_cache_add(cdm_cache_get(),
                            labels,
                            body_out,
                            bodylen_out);
      if (ent == NULL) {
        static ratelim_t cant_store_ratelim = RATELIM_INIT(5*60);
        log_fn_ratelim(&cant_store_ratelim, LOG_WARN, LD_FS,
                       "Unable to store object %s compressed with %s.",
                       description, methodname);
        continue;
      }

      status = CDM_DIFF_PRESENT;
      handles_out[i] = consensus_cache_entry_handle_new(ent);
      consensus_cache_entry_decref(ent);
    }
  }
  return status;
}

/**
 * An object passed to a worker thread that will try to produce a consensus
 * diff.
 */
typedef struct consensus_diff_worker_job_t {
  /**
   * Input: The consensus to compute the diff from.  Holds a reference to the
   * cache entry, which must not be released until the job is passed back to
   * the main thread. The body must be mapped into memory in the main thread.
   */
  consensus_cache_entry_t *diff_from;
  /**
   * Input: The consensus to compute the diff to.  Holds a reference to the
   * cache entry, which must not be released until the job is passed back to
   * the main thread. The body must be mapped into memory in the main thread.
   */
  consensus_cache_entry_t *diff_to;

  /** Output: labels and bodies */
  compressed_result_t out[ARRAY_LENGTH(compress_diffs_with)];
} consensus_diff_worker_job_t;

/** Given a consensus_cache_entry_t, check whether it has a label claiming
 * that it was compressed.  If so, uncompress its contents into *<b>out</b> and
 * set <b>outlen</b> to hold their size, and set *<b>owned_out</b> to a pointer
 * that the caller will need to free.  If not, just set *<b>out</b> and
 * <b>outlen</b> to its extent in memory.  Return 0 on success, -1 on failure.
 **/
STATIC int
uncompress_or_set_ptr(const char **out, size_t *outlen,
                      char **owned_out,
                      consensus_cache_entry_t *ent)
{
  const uint8_t *body;
  size_t bodylen;

  *owned_out = NULL;

  if (consensus_cache_entry_get_body(ent, &body, &bodylen) < 0)
    return -1;

  const char *lv_compression =
    consensus_cache_entry_get_value(ent, LABEL_COMPRESSION_TYPE);
  compress_method_t method = NO_METHOD;

  if (lv_compression)
    method = compression_method_get_by_name(lv_compression);

  int rv;
  if (method == NO_METHOD) {
    *out = (const char *)body;
    *outlen = bodylen;
    rv = 0;
  } else {
    rv = tor_uncompress(owned_out, outlen, (const char *)body, bodylen,
                        method, 1, LOG_WARN);
    *out = *owned_out;
  }
  return rv;
}

/**
 * Worker function. This function runs inside a worker thread and receives
 * a consensus_diff_worker_job_t as its input.
 */
static workqueue_reply_t
consensus_diff_worker_threadfn(void *state_, void *work_)
{
  (void)state_;
  consensus_diff_worker_job_t *job = work_;
  const uint8_t *diff_from, *diff_to;
  size_t len_from, len_to;
  int r;
  /* We need to have the body already mapped into RAM here.
   */
  r = consensus_cache_entry_get_body(job->diff_from, &diff_from, &len_from);
  if (BUG(r < 0))
    return WQ_RPL_REPLY; // LCOV_EXCL_LINE
  r = consensus_cache_entry_get_body(job->diff_to, &diff_to, &len_to);
  if (BUG(r < 0))
    return WQ_RPL_REPLY; // LCOV_EXCL_LINE

  const char *lv_to_valid_after =
    consensus_cache_entry_get_value(job->diff_to, LABEL_VALID_AFTER);
  const char *lv_to_fresh_until =
    consensus_cache_entry_get_value(job->diff_to, LABEL_FRESH_UNTIL);
  const char *lv_to_valid_until =
    consensus_cache_entry_get_value(job->diff_to, LABEL_VALID_UNTIL);
  const char *lv_to_signatories =
    consensus_cache_entry_get_value(job->diff_to, LABEL_SIGNATORIES);
  const char *lv_from_valid_after =
    consensus_cache_entry_get_value(job->diff_from, LABEL_VALID_AFTER);
  const char *lv_from_digest =
    consensus_cache_entry_get_value(job->diff_from,
                                    LABEL_SHA3_DIGEST_AS_SIGNED);
  const char *lv_from_flavor =
    consensus_cache_entry_get_value(job->diff_from, LABEL_FLAVOR);
  const char *lv_to_flavor =
    consensus_cache_entry_get_value(job->diff_to, LABEL_FLAVOR);
  const char *lv_to_digest =
    consensus_cache_entry_get_value(job->diff_to,
                                    LABEL_SHA3_DIGEST_UNCOMPRESSED);

  if (! lv_from_digest) {
    /* This isn't a bug right now, since it can happen if you're migrating
     * from an older version of master to a newer one.  The older ones didn't
     * annotate their stored consensus objects with sha3-digest-as-signed.
    */
    return WQ_RPL_REPLY; // LCOV_EXCL_LINE
  }

  /* All these values are mandatory on the input */
  if (BUG(!lv_to_valid_after) ||
      BUG(!lv_from_valid_after) ||
      BUG(!lv_from_flavor) ||
      BUG(!lv_to_flavor)) {
    return WQ_RPL_REPLY; // LCOV_EXCL_LINE
  }
  /* The flavors need to match */
  if (BUG(strcmp(lv_from_flavor, lv_to_flavor))) {
    return WQ_RPL_REPLY; // LCOV_EXCL_LINE
  }

  char *consensus_diff;
  {
    const char *diff_from_nt = NULL, *diff_to_nt = NULL;
    char *owned1 = NULL, *owned2 = NULL;
    size_t diff_from_nt_len, diff_to_nt_len;

    if (uncompress_or_set_ptr(&diff_from_nt, &diff_from_nt_len, &owned1,
                              job->diff_from) < 0) {
      return WQ_RPL_REPLY;
    }
    if (uncompress_or_set_ptr(&diff_to_nt, &diff_to_nt_len, &owned2,
                              job->diff_to) < 0) {
      tor_free(owned1);
      return WQ_RPL_REPLY;
    }
    tor_assert(diff_from_nt);
    tor_assert(diff_to_nt);

    // XXXX ugh; this is going to calculate the SHA3 of both its
    // XXXX inputs again, even though we already have that. Maybe it's time
    // XXXX to change the API here?
    consensus_diff = consensus_diff_generate(diff_from_nt,
                                             diff_from_nt_len,
                                             diff_to_nt,
                                             diff_to_nt_len);
    tor_free(owned1);
    tor_free(owned2);
  }
  if (!consensus_diff) {
    /* Couldn't generate consensus; we'll leave the reply blank. */
    return WQ_RPL_REPLY;
  }

  /* Compress the results and send the reply */
  tor_assert(compress_diffs_with[0] == NO_METHOD);
  size_t difflen = strlen(consensus_diff);
  job->out[0].body = (uint8_t *) consensus_diff;
  job->out[0].bodylen = difflen;

  config_line_t *common_labels = NULL;
  if (lv_to_valid_until)
    config_line_prepend(&common_labels, LABEL_VALID_UNTIL, lv_to_valid_until);
  if (lv_to_fresh_until)
    config_line_prepend(&common_labels, LABEL_FRESH_UNTIL, lv_to_fresh_until);
  if (lv_to_signatories)
    config_line_prepend(&common_labels, LABEL_SIGNATORIES, lv_to_signatories);
  cdm_labels_prepend_sha3(&common_labels,
                          LABEL_SHA3_DIGEST_UNCOMPRESSED,
                          job->out[0].body,
                          job->out[0].bodylen);
  config_line_prepend(&common_labels, LABEL_FROM_VALID_AFTER,
                      lv_from_valid_after);
  config_line_prepend(&common_labels, LABEL_VALID_AFTER,
                      lv_to_valid_after);
  config_line_prepend(&common_labels, LABEL_FLAVOR, lv_from_flavor);
  config_line_prepend(&common_labels, LABEL_FROM_SHA3_DIGEST,
                      lv_from_digest);
  config_line_prepend(&common_labels, LABEL_TARGET_SHA3_DIGEST,
                      lv_to_digest);
  config_line_prepend(&common_labels, LABEL_DOCTYPE,
                      DOCTYPE_CONSENSUS_DIFF);

  job->out[0].labels = config_lines_dup(common_labels);
  cdm_labels_prepend_sha3(&job->out[0].labels,
                          LABEL_SHA3_DIGEST,
                          job->out[0].body,
                          job->out[0].bodylen);

  compress_multiple(job->out+1,
                    n_diff_compression_methods()-1,
                    compress_diffs_with+1,
                    (const uint8_t*)consensus_diff, difflen, common_labels);

  config_free_lines(common_labels);
  return WQ_RPL_REPLY;
}

#define consensus_diff_worker_job_free(job)             \
  FREE_AND_NULL(consensus_diff_worker_job_t,            \
                consensus_diff_worker_job_free_, (job))

/**
 * Helper: release all storage held in <b>job</b>.
 */
static void
consensus_diff_worker_job_free_(consensus_diff_worker_job_t *job)
{
  if (!job)
    return;
  unsigned u;
  for (u = 0; u < n_diff_compression_methods(); ++u) {
    config_free_lines(job->out[u].labels);
    tor_free(job->out[u].body);
  }
  consensus_cache_entry_decref(job->diff_from);
  consensus_cache_entry_decref(job->diff_to);
  tor_free(job);
}

/**
 * Worker function: This function runs in the main thread, and receives
 * a consensus_diff_worker_job_t that the worker thread has already
 * processed.
 */
static void
consensus_diff_worker_replyfn(void *work_)
{
  tor_assert(in_main_thread());
  tor_assert(work_);

  consensus_diff_worker_job_t *job = work_;

  const char *lv_from_digest =
    consensus_cache_entry_get_value(job->diff_from,
                                    LABEL_SHA3_DIGEST_AS_SIGNED);
  const char *lv_to_digest =
    consensus_cache_entry_get_value(job->diff_to,
                                    LABEL_SHA3_DIGEST_UNCOMPRESSED);
  const char *lv_flavor =
    consensus_cache_entry_get_value(job->diff_to, LABEL_FLAVOR);
  if (BUG(lv_from_digest == NULL))
    lv_from_digest = "???"; // LCOV_EXCL_LINE
  if (BUG(lv_to_digest == NULL))
    lv_to_digest = "???"; // LCOV_EXCL_LINE

  uint8_t from_sha3[DIGEST256_LEN];
  uint8_t to_sha3[DIGEST256_LEN];
  int flav = -1;
  int cache = 1;
  if (BUG(cdm_entry_get_sha3_value(from_sha3, job->diff_from,
                                   LABEL_SHA3_DIGEST_AS_SIGNED) < 0))
    cache = 0;
  if (BUG(cdm_entry_get_sha3_value(to_sha3, job->diff_to,
                                   LABEL_SHA3_DIGEST_UNCOMPRESSED) < 0))
    cache = 0;
  if (BUG(lv_flavor == NULL)) {
    cache = 0;
  } else if ((flav = networkstatus_parse_flavor_name(lv_flavor)) < 0) {
    cache = 0;
  }

  consensus_cache_entry_handle_t *handles[ARRAY_LENGTH(compress_diffs_with)];
  memset(handles, 0, sizeof(handles));

  char description[128];
  tor_snprintf(description, sizeof(description),
               "consensus diff from %s to %s",
               lv_from_digest, lv_to_digest);

  int status = store_multiple(handles,
                              n_diff_compression_methods(),
                              compress_diffs_with,
                              job->out,
                              description);

  if (status != CDM_DIFF_PRESENT) {
    /* Failure! Nothing to do but complain */
    log_warn(LD_DIRSERV,
             "Worker was unable to compute consensus diff "
             "from %s to %s", lv_from_digest, lv_to_digest);
    /* Cache this error so we don't try to compute this one again. */
    status = CDM_DIFF_ERROR;
  }

  unsigned u;
  for (u = 0; u < ARRAY_LENGTH(handles); ++u) {
    compress_method_t method = compress_diffs_with[u];
    if (cache) {
      consensus_cache_entry_handle_t *h = handles[u];
      int this_status = status;
      if (h == NULL) {
        this_status = CDM_DIFF_ERROR;
      }
      tor_assert_nonfatal(h != NULL || this_status == CDM_DIFF_ERROR);
      cdm_diff_ht_set_status(flav, from_sha3, to_sha3, method, this_status, h);
    } else {
      consensus_cache_entry_handle_free(handles[u]);
    }
  }

  consensus_diff_worker_job_free(job);
}

/**
 * Queue the job of computing the diff from <b>diff_from</b> to <b>diff_to</b>
 * in a worker thread.
 */
static int
consensus_diff_queue_diff_work(consensus_cache_entry_t *diff_from,
                               consensus_cache_entry_t *diff_to)
{
  tor_assert(in_main_thread());

  consensus_cache_entry_incref(diff_from);
  consensus_cache_entry_incref(diff_to);

  consensus_diff_worker_job_t *job = tor_malloc_zero(sizeof(*job));
  job->diff_from = diff_from;
  job->diff_to = diff_to;

  /* Make sure body is mapped. */
  const uint8_t *body;
  size_t bodylen;
  int r1 = consensus_cache_entry_get_body(diff_from, &body, &bodylen);
  int r2 = consensus_cache_entry_get_body(diff_to, &body, &bodylen);
  if (r1 < 0 || r2 < 0)
    goto err;

  workqueue_entry_t *work;
  work = cpuworker_queue_work(WQ_PRI_LOW,
                              consensus_diff_worker_threadfn,
                              consensus_diff_worker_replyfn,
                              job);
  if (!work)
    goto err;

  return 0;
 err:
  consensus_diff_worker_job_free(job); // includes decrefs.
  return -1;
}

/**
 * Holds requests and replies for consensus_compress_workers.
 */
typedef struct consensus_compress_worker_job_t {
  char *consensus;
  size_t consensus_len;
  consensus_flavor_t flavor;
  config_line_t *labels_in;
  compressed_result_t out[ARRAY_LENGTH(compress_consensus_with)];
} consensus_compress_worker_job_t;

#define consensus_compress_worker_job_free(job) \
  FREE_AND_NULL(consensus_compress_worker_job_t, \
                consensus_compress_worker_job_free_, (job))

/**
 * Free all resources held in <b>job</b>
 */
static void
consensus_compress_worker_job_free_(consensus_compress_worker_job_t *job)
{
  if (!job)
    return;
  tor_free(job->consensus);
  config_free_lines(job->labels_in);
  unsigned u;
  for (u = 0; u < n_consensus_compression_methods(); ++u) {
    config_free_lines(job->out[u].labels);
    tor_free(job->out[u].body);
  }
  tor_free(job);
}
/**
 * Worker function. This function runs inside a worker thread and receives
 * a consensus_compress_worker_job_t as its input.
 */
static workqueue_reply_t
consensus_compress_worker_threadfn(void *state_, void *work_)
{
  (void)state_;
  consensus_compress_worker_job_t *job = work_;
  consensus_flavor_t flavor = job->flavor;
  const char *consensus = job->consensus;
  size_t bodylen = job->consensus_len;

  config_line_t *labels = config_lines_dup(job->labels_in);
  const char *flavname = networkstatus_get_flavor_name(flavor);

  cdm_labels_prepend_sha3(&labels, LABEL_SHA3_DIGEST_UNCOMPRESSED,
                          (const uint8_t *)consensus, bodylen);
  {
    const char *start, *end;
    if (router_get_networkstatus_v3_signed_boundaries(consensus, bodylen,
                                                      &start, &end) < 0) {
      start = consensus;
      end = consensus+bodylen;
    }
    cdm_labels_prepend_sha3(&labels, LABEL_SHA3_DIGEST_AS_SIGNED,
                            (const uint8_t *)start,
                            end - start);
  }
  config_line_prepend(&labels, LABEL_FLAVOR, flavname);
  config_line_prepend(&labels, LABEL_DOCTYPE, DOCTYPE_CONSENSUS);

  compress_multiple(job->out,
                    n_consensus_compression_methods(),
                    compress_consensus_with,
                    (const uint8_t*)consensus, bodylen, labels);
  config_free_lines(labels);
  return WQ_RPL_REPLY;
}

/**
 * Worker function: This function runs in the main thread, and receives
 * a consensus_diff_compress_job_t that the worker thread has already
 * processed.
 */
static void
consensus_compress_worker_replyfn(void *work_)
{
  consensus_compress_worker_job_t *job = work_;

  consensus_cache_entry_handle_t *handles[
                               ARRAY_LENGTH(compress_consensus_with)];
  memset(handles, 0, sizeof(handles));

  store_multiple(handles,
                 n_consensus_compression_methods(),
                 compress_consensus_with,
                 job->out,
                 "consensus");
  mark_cdm_cache_dirty();

  unsigned u;
  consensus_flavor_t f = job->flavor;
  tor_assert((int)f < N_CONSENSUS_FLAVORS);
  for (u = 0; u < ARRAY_LENGTH(handles); ++u) {
    if (handles[u] == NULL)
      continue;
    consensus_cache_entry_handle_free(latest_consensus[f][u]);
    latest_consensus[f][u] = handles[u];
  }

  consensus_compress_worker_job_free(job);
}

/**
 * If true, we compress in worker threads.
 */
static int background_compression = 0;

/**
 * Queue a job to compress <b>consensus</b> and store its compressed
 * text in the cache.
 */
static int
consensus_queue_compression_work(const char *consensus,
                                 size_t consensus_len,
                                 const networkstatus_t *as_parsed)
{
  tor_assert(consensus);
  tor_assert(as_parsed);

  consensus_compress_worker_job_t *job = tor_malloc_zero(sizeof(*job));
  job->consensus = tor_memdup_nulterm(consensus, consensus_len);
  job->consensus_len = strlen(job->consensus);
  job->flavor = as_parsed->flavor;

  char va_str[ISO_TIME_LEN+1];
  char vu_str[ISO_TIME_LEN+1];
  char fu_str[ISO_TIME_LEN+1];
  format_iso_time_nospace(va_str, as_parsed->valid_after);
  format_iso_time_nospace(fu_str, as_parsed->fresh_until);
  format_iso_time_nospace(vu_str, as_parsed->valid_until);
  config_line_append(&job->labels_in, LABEL_VALID_AFTER, va_str);
  config_line_append(&job->labels_in, LABEL_FRESH_UNTIL, fu_str);
  config_line_append(&job->labels_in, LABEL_VALID_UNTIL, vu_str);
  if (as_parsed->voters) {
    smartlist_t *hexvoters = smartlist_new();
    SMARTLIST_FOREACH_BEGIN(as_parsed->voters,
                            networkstatus_voter_info_t *, vi) {
      if (smartlist_len(vi->sigs) == 0)
        continue; // didn't sign.
      char d[HEX_DIGEST_LEN+1];
      base16_encode(d, sizeof(d), vi->identity_digest, DIGEST_LEN);
      smartlist_add_strdup(hexvoters, d);
    } SMARTLIST_FOREACH_END(vi);
    char *signers = smartlist_join_strings(hexvoters, ",", 0, NULL);
    config_line_prepend(&job->labels_in, LABEL_SIGNATORIES, signers);
    tor_free(signers);
    SMARTLIST_FOREACH(hexvoters, char *, cp, tor_free(cp));
    smartlist_free(hexvoters);
  }

  if (background_compression) {
    workqueue_entry_t *work;
    work = cpuworker_queue_work(WQ_PRI_LOW,
                                consensus_compress_worker_threadfn,
                                consensus_compress_worker_replyfn,
                                job);
    if (!work) {
      consensus_compress_worker_job_free(job);
      return -1;
    }

    return 0;
  } else {
    consensus_compress_worker_threadfn(NULL, job);
    consensus_compress_worker_replyfn(job);
    return 0;
  }
}

/**
 * Tell the consdiffmgr backend to compress consensuses in worker threads.
 */
void
consdiffmgr_enable_background_compression(void)
{
  // This isn't the default behavior because it would break unit tests.
  background_compression = 1;
}

/** Read the set of voters from the cached object <b>ent</b> into
 * <b>out</b>, as a list of hex-encoded digests. Return 0 on success,
 * -1 if no signatories were recorded. */
int
consensus_cache_entry_get_voter_id_digests(const consensus_cache_entry_t *ent,
                                           smartlist_t *out)
{
  tor_assert(ent);
  tor_assert(out);
  const char *s;
  s = consensus_cache_entry_get_value(ent, LABEL_SIGNATORIES);
  if (s == NULL)
    return -1;
  smartlist_split_string(out, s, ",", SPLIT_SKIP_SPACE|SPLIT_STRIP_SPACE, 0);
  return 0;
}

/** Read the fresh-until time of cached object <b>ent</b> into *<b>out</b>
 * and return 0, or return -1 if no such time was recorded. */
int
consensus_cache_entry_get_fresh_until(const consensus_cache_entry_t *ent,
                                      time_t *out)
{
  tor_assert(ent);
  tor_assert(out);
  const char *s;
  s = consensus_cache_entry_get_value(ent, LABEL_FRESH_UNTIL);
  if (s == NULL || parse_iso_time_nospace(s, out) < 0)
    return -1;
  else
    return 0;
}

/** Read the valid until timestamp from the cached object <b>ent</b> into
 * *<b>out</b> and return 0, or return -1 if no such time was recorded. */
int
consensus_cache_entry_get_valid_until(const consensus_cache_entry_t *ent,
                                      time_t *out)
{
  tor_assert(ent);
  tor_assert(out);

  const char *s;
  s = consensus_cache_entry_get_value(ent, LABEL_VALID_UNTIL);
  if (s == NULL || parse_iso_time_nospace(s, out) < 0)
    return -1;
  else
    return 0;
}

/** Read the valid after timestamp from the cached object <b>ent</b> into
 * *<b>out</b> and return 0, or return -1 if no such time was recorded. */
int
consensus_cache_entry_get_valid_after(const consensus_cache_entry_t *ent,
                                      time_t *out)
{
  tor_assert(ent);
  tor_assert(out);

  const char *s;
  s = consensus_cache_entry_get_value(ent, LABEL_VALID_AFTER);

  if (s == NULL || parse_iso_time_nospace(s, out) < 0)
    return -1;
  else
    return 0;
}

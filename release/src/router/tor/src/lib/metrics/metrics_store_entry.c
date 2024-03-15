/* Copyright (c) 2020-2021, The Tor Project, Inc. */
/* See LICENSE for licensing information */

/**
 * @file metrics_store_entry.c
 * @brief Metrics store entry which contains the gathered data.
 **/

#include "metrics_common.h"
#define METRICS_STORE_ENTRY_PRIVATE

#include <string.h>

#include "orconfig.h"

#include "lib/container/smartlist.h"
#include "lib/log/util_bug.h"
#include "lib/malloc/malloc.h"

#include "lib/metrics/metrics_store_entry.h"

/*
 * Public API.
 */

/** Return newly allocated store entry of the specified type. */
metrics_store_entry_t *
metrics_store_entry_new(const metrics_type_t type, const char *name,
                        const char *help, size_t bucket_count,
                        const int64_t *buckets)
{
  metrics_store_entry_t *entry = tor_malloc_zero(sizeof(*entry));

  tor_assert(name);

  entry->type = type;
  entry->name = tor_strdup(name);
  entry->labels = smartlist_new();
  if (help) {
    entry->help = tor_strdup(help);
  }

  if (type == METRICS_TYPE_HISTOGRAM && bucket_count > 0) {
    tor_assert(buckets);

    entry->u.histogram.bucket_count = bucket_count;
    entry->u.histogram.buckets =
        tor_malloc_zero(sizeof(metrics_histogram_bucket_t) * bucket_count);

    for (size_t i = 0; i < bucket_count; ++i) {
      entry->u.histogram.buckets[i].bucket = buckets[i];
    }
  }

  return entry;
}

/** Free a store entry. */
void
metrics_store_entry_free_(metrics_store_entry_t *entry)
{
  if (!entry) {
    return;
  }
  SMARTLIST_FOREACH(entry->labels, char *, l, tor_free(l));
  smartlist_free(entry->labels);
  tor_free(entry->name);
  tor_free(entry->help);

  if (entry->type == METRICS_TYPE_HISTOGRAM) {
    tor_free(entry->u.histogram.buckets);
  }

  tor_free(entry);
}

/** Update a store entry with value. */
void
metrics_store_entry_update(metrics_store_entry_t *entry, const int64_t value)
{
  tor_assert(entry);

  /* Histogram values are updated using metrics_store_hist_entry_update */
  if (BUG(entry->type == METRICS_TYPE_HISTOGRAM)) {
    return;
  }

  switch (entry->type) {
  case METRICS_TYPE_COUNTER:
    /* Counter can ONLY be positive. */
    if (BUG(value < 0)) {
      return;
    }
    entry->u.counter.value += value;
    break;
  case METRICS_TYPE_GAUGE:
    /* Gauge can increment or decrement. And can be positive or negative. */
    entry->u.gauge.value += value;
    break;
  case METRICS_TYPE_HISTOGRAM:
    tor_assert_unreached();
  }
}

/** Update a store entry with value for the specified observation obs.
 *
 * Note: entry **must** be a histogram. */
void
metrics_store_hist_entry_update(metrics_store_entry_t *entry,
                                const int64_t value, const int64_t obs)
{
  if (BUG(entry->type != METRICS_TYPE_HISTOGRAM)) {
    return;
  }

  /* Counter can ONLY be positive for histograms. */
  if (BUG(value < 0)) {
    return;
  }

  /* If we're about to overflow or underflow the sum, reset all counters back
   * to 0 before recording the observation. */
  if (PREDICT_UNLIKELY(
          (obs > 0 && entry->u.histogram.sum > INT64_MAX - obs) ||
          (obs < 0 && entry->u.histogram.sum < INT64_MIN - obs))) {
    metrics_store_entry_reset(entry);
  }

  entry->u.histogram.count += value;
  entry->u.histogram.sum += obs;

  for (size_t i = 0; i < entry->u.histogram.bucket_count; ++i) {
    metrics_histogram_bucket_t *hb = &entry->u.histogram.buckets[i];
    if (obs <= hb->bucket) {
      hb->value += value;
    }
  }
}

/** Reset a store entry that is set its metric data to 0. */
void
metrics_store_entry_reset(metrics_store_entry_t *entry)
{
  tor_assert(entry);

  switch (entry->type) {
  case METRICS_TYPE_COUNTER: FALLTHROUGH;
  case METRICS_TYPE_GAUGE:
    /* Everything back to 0. */
    memset(&entry->u, 0, sizeof(entry->u));
    break;
  case METRICS_TYPE_HISTOGRAM:
    for (size_t i = 0; i < entry->u.histogram.bucket_count; ++i) {
      metrics_histogram_bucket_t *hb = &entry->u.histogram.buckets[i];
      hb->value = 0;
    }
    entry->u.histogram.sum = 0;
    entry->u.histogram.count = 0;
    break;
  }
}

/** Return store entry value. */
int64_t
metrics_store_entry_get_value(const metrics_store_entry_t *entry)
{
  tor_assert(entry);

  /* Histogram values are accessed using metrics_store_hist_entry_get_value. */
  if (BUG(entry->type == METRICS_TYPE_HISTOGRAM)) {
    return 0;
  }

  switch (entry->type) {
  case METRICS_TYPE_COUNTER:
    if (entry->u.counter.value > INT64_MAX) {
      return INT64_MAX;
    }
    return entry->u.counter.value;
  case METRICS_TYPE_GAUGE:
    return entry->u.gauge.value;
  case METRICS_TYPE_HISTOGRAM:
    tor_assert_unreached();
    return 0;
  }

  // LCOV_EXCL_START
  tor_assert_unreached();
  // LCOV_EXCL_STOP
}

/** Return store entry value for the specified bucket.
 *
 * Note: entry **must** be a histogram. */
uint64_t
metrics_store_hist_entry_get_value(const metrics_store_entry_t *entry,
                                   const int64_t bucket)
{
  tor_assert(entry);

  if (BUG(entry->type != METRICS_TYPE_HISTOGRAM)) {
    return 0;
  }

  for (size_t i = 0; i <= entry->u.histogram.bucket_count; ++i) {
    metrics_histogram_bucket_t hb = entry->u.histogram.buckets[i];
    if (bucket == hb.bucket) {
      if (hb.value > INT64_MAX) {
        return INT64_MAX;
      } else {
        return hb.value;
      }
    }
  }

  tor_assertf_nonfatal(false, "attempted to get the value of non-existent "
                       "bucket %" PRId64, bucket);
  return 0;
}

/** Add a label into the given entry.*/
void
metrics_store_entry_add_label(metrics_store_entry_t *entry,
                              const char *label)
{
  tor_assert(entry);
  tor_assert(label);

  smartlist_add(entry->labels, tor_strdup(label));
}

/** Return true iff the given entry has the given label. */
bool
metrics_store_entry_has_label(const metrics_store_entry_t *entry,
                              const char *label)
{
  tor_assert(entry);
  tor_assert(label);

  return smartlist_contains_string(entry->labels, label);
}

/** Return the first entry that has the given label, or NULL if none
 * of the entries have the label. */
metrics_store_entry_t *
metrics_store_find_entry_with_label(const smartlist_t *entries,
                                    const char *label)
{
  tor_assert(entries);
  tor_assert(label);

  SMARTLIST_FOREACH_BEGIN(entries, metrics_store_entry_t *, entry) {
    tor_assert(entry);

    if (smartlist_contains_string(entry->labels, label)) {
      return entry;
    }
  } SMARTLIST_FOREACH_END(entry);

  return NULL;
}

/** Return true iff the specified entry is a histogram. */
bool
metrics_store_entry_is_histogram(const metrics_store_entry_t *entry)
{
  if (entry->type == METRICS_TYPE_HISTOGRAM) {
    return true;
  }

  return false;
}

/** Return the total number of observations for the specified histogram. */
uint64_t
metrics_store_hist_entry_get_count(const metrics_store_entry_t *entry)
{
  tor_assert(entry);

  if (BUG(entry->type != METRICS_TYPE_HISTOGRAM)) {
    return 0;
  }

  return entry->u.histogram.count;
}

/** Return the sum of all observations for the specified histogram. */
int64_t
metrics_store_hist_entry_get_sum(const metrics_store_entry_t *entry)
{
  tor_assert(entry);

  if (BUG(entry->type != METRICS_TYPE_HISTOGRAM)) {
    return 0;
  }

  return entry->u.histogram.sum;
}

/* Copyright (c) 2020-2021, The Tor Project, Inc. */
/* See LICENSE for licensing information */

/**
 * @file metrics_store.c
 * @brief Metrics interface to store them based on specific store type and get
 *        their MetricsPort output.
 **/

#define METRICS_STORE_ENTRY_PRIVATE

#include "orconfig.h"

#include "lib/container/map.h"
#include "lib/log/util_bug.h"
#include "lib/malloc/malloc.h"

#include "lib/metrics/metrics_store.h"
#include "lib/metrics/metrics_store_entry.h"

/* Format Drivers. */
#include "lib/metrics/prometheus.h"

/** A metric store which contains a map of entries. */
struct metrics_store_t {
  /** Indexed by metrics entry name. An entry is a smartlist_t of one or more
   * metrics_store_entry_t allowing for multiple metrics of the same name.
   *
   * The reason we allow multiple entries is because there are cases where one
   * metrics can be used twice by the same entity but with different labels.
   * One example is an onion service with multiple ports, the port specific
   * metrics will have a port value as a label. */
  strmap_t *entries;
};

/** Function pointer to the format function of a specific driver. */
typedef void (fmt_driver_fn_t)(const metrics_store_entry_t *, buf_t *,
                               bool no_comment);

/** Helper: Free a single entry in a metrics_store_t taking a void pointer
 * parameter. */
static void
metrics_store_free_void(void *p)
{
  smartlist_t *list = p;
  SMARTLIST_FOREACH(list, metrics_store_entry_t *, entry,
                    metrics_store_entry_free(entry));
  smartlist_free(list);
}

#include <stdio.h>

/** Put the given store output in the buffer data and use the format function
 * given in fmt to get it for each entry. */
static void
get_output(const metrics_store_t *store, buf_t *data, fmt_driver_fn_t fmt)
{
  tor_assert(store);
  tor_assert(data);
  tor_assert(fmt);

  STRMAP_FOREACH(store->entries, key, const smartlist_t *, entries) {
    /* Indicate that we've formatted the comment already for the entries. */
    bool comment_formatted = false;
    SMARTLIST_FOREACH_BEGIN(entries, const metrics_store_entry_t *, entry) {
      fmt(entry, data, comment_formatted);
      comment_formatted = true;
    } SMARTLIST_FOREACH_END(entry);
  } STRMAP_FOREACH_END;
}

/** Return a newly allocated and initialized store of the given type. */
metrics_store_t *
metrics_store_new(void)
{
  metrics_store_t *store = tor_malloc_zero(sizeof(*store));

  store->entries = strmap_new();

  return store;
}

/** Free the given store including all its entries. */
void
metrics_store_free_(metrics_store_t *store)
{
  if (store == NULL) {
    return;
  }

  strmap_free(store->entries, metrics_store_free_void);
  tor_free(store);
}

/** Find all metrics entry in the given store identified by name. If not found,
 * NULL is returned. */
smartlist_t *
metrics_store_get_all(const metrics_store_t *store, const char *name)
{
  tor_assert(store);
  tor_assert(name);

  return strmap_get(store->entries, name);
}

/** Add a new metrics entry to the given store and type. The name MUST be the
 * unique identifier. The help string can be omitted. */
metrics_store_entry_t *
metrics_store_add(metrics_store_t *store, metrics_type_t type,
                  const char *name, const char *help, size_t bucket_count,
                  const int64_t *buckets)

{
  smartlist_t *entries;
  metrics_store_entry_t *entry;

  tor_assert(store);
  tor_assert(name);

  entries = metrics_store_get_all(store, name);
  if (!entries) {
    entries = smartlist_new();
    strmap_set(store->entries, name, entries);
  }
  entry = metrics_store_entry_new(type, name, help, bucket_count, buckets);
  smartlist_add(entries, entry);

  return entry;
}

/** Set the output of the given store of the format fmt into the given buffer
 * data. */
void
metrics_store_get_output(const metrics_format_t fmt,
                         const metrics_store_t *store, buf_t *data)
{
  tor_assert(store);

  switch (fmt) {
  case METRICS_FORMAT_PROMETHEUS:
    get_output(store, data, prometheus_format_store_entry);
    break;
  default:
    // LCOV_EXCL_START
    tor_assert_unreached();
    // LCOV_EXCL_STOP
  }
}

/** Reset a store as in free its content. */
void
metrics_store_reset(metrics_store_t *store)
{
  if (store == NULL) {
    return;
  }
  strmap_free(store->entries, metrics_store_free_void);
  store->entries = strmap_new();
}
